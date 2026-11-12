# Report — Week 3 / Ex3

`student_id`: ______    Commit của PREDICTION.md: ______

## 1. Thời gian byte & độ sâu stream buffer
Phép tính thời gian 1 byte ở `RX_BAUD`, và số lệnh tối đa chứa được trong
`STREAM_BUFFER_BYTES`.
Thời gian truyền 1 byte là `10/115200 10^6` = 86,81us
Mà mỗi lệnh chiếm 16 byte trong khi buffer có 96 byte ô chứa
=> có  thể chứa khoảng 6 lệnh.
## 2. Demo 3 loại lệnh
Bằng chứng (log/ảnh) `demo_task` phản ứng đúng với `P`, `S`, `T` — mỗi loại ít
nhất 1 ví dụ có số liệu trước/sau.

READY baud=115200 tick_hz=1000 stream=96 trigger=4 initial_period_ms=200 initial_priority=1 end=CR
ERR_TOO_LONG cmd=P200P200P20P20P... tick=45307 period_ms=200->200 delay_ticks=200->200 priority=1->1 state=BLOCKED->BLOCKED rx_drop=0 log_drop=0
OK cmd=P200 tick=53833 period_ms=200->200 delay_ticks=200->200 priority=1->1 state=BLOCKED->BLOCKED rx_drop=0 log_drop=0
OK cmd=P20 tick=59466 period_ms=200->20 delay_ticks=200->20 priority=1->1 state=BLOCKED->BLOCKED rx_drop=0 log_drop=0
OK cmd=T0 tick=65343 period_ms=20->20 delay_ticks=20->20 priority=1->1 state=BLOCKED->SUSPENDED rx_drop=0 log_drop=0
OK cmd=T1 tick=98400 period_ms=20->20 delay_ticks=20->20 priority=1->1 state=SUSPENDED->READY rx_drop=0 log_drop=0
OK cmd=S3 tick=117317 period_ms=20->20 delay_ticks=20->20 priority=1->3 state=BLOCKED->BLOCKED rx_drop=0 log_drop=0

## 3. Test gửi nhanh
| | Số lệnh gửi | Số lệnh nhận đúng | Mất/lỗi |
|---|---|---|---|
| Gõ tay bình thường | 10 | 10 | Không có |
| Gửi liên tục (script, SYNC_PARSER, buffer 96 byte) | 100 | 15 | 85 lệnh không nhận/xử lý đúng; rx_drop=646 byte |

Nếu có mất byte: giải thích bằng cơ chế (stream buffer đầy, parser bị trễ,
hay nguyên nhân khác) — không chỉ nói "bị mất".
Trong lần thử SYNC_PARSER này, parser truyền log UART đồng bộ nên phải chờ
truyền xong mới đọc tiếp stream buffer. RX ISR vẫn tiếp tục nhận dữ liệu,
khiến stream buffer đầy và bỏ 646 byte. `log_drop=0`, `uart_err=0` và `ore=0`.
Kết quả này khác với thí nghiệm ASYNC_TASK trước đó: bỏ 77 bản ghi log TX
không đồng nghĩa với mất 77 lệnh RX.
## 4. Vì sao stream buffer, không phải queue
Giải thích bằng lời của bạn.
stream buffer thường dùng cho các tác vụ yêu cầu số byte thay đổi phù hợp với việc nhận dữ liệu thô hơn là tác vụ có số lượng item cố định như queue. Cũng như là ở queue cần một số thao tác sao chép và cập nhập con trỏ hàng đợi và xử lý danh sách chờ. Từ đó stream buffer có thao tác tiết kiệm đơn giản hơn trong case này.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
Khi truyền log đồng bộ trong parser, task phải chờ UART truyền xong mỗi bản ghi trước khi đọc tiếp stream buffer. Trong thời gian đó, ISR tiếp tục nhận dữ liệu, khiến buffer đầy và bỏ 646 byte. Parser chỉ xử lý được 15/100 lệnh hợp lệ. Các bộ đếm lỗi UART và overrun phần cứng đều bằng 0, phù hợp với nguyên nhân mất dữ liệu ở stream buffer do parser xử lý không kịp.
## 6. Câu hỏi đánh đổi
Với `RX_BAUD` của bạn, nếu tăng gấp đôi `STREAM_BUFFER_BYTES`, tốc độ gửi tối
đa không mất byte tăng thêm bao nhiêu (đo thật, không suy luận)?

### Điều kiện và số liệu đã đo

So sánh hai lần gửi 100 lệnh có sequence ID từ 0 đến 99 ở `RX_BAUD=115200`,
8N1, dùng chế độ `SYNC_PARSER`. Lần thứ hai đổi lời gọi tạo stream buffer
từ 96 byte sang `xStreamBufferCreate(192, TRIGGER_LEVEL)`. 

Log:

```text
STATS mode=SYNC_PARSER ok=15 bad=2 too_long=1 seq_count=15 first=0 last=94 seq_gap=80 seq_order=0 rx_bytes=792 rx_drop=646 uart_err=0 ore=0 rearm_err=0 log_drop=0 tx_err=0
STATS mode=SYNC_PARSER ok=28 bad=4 too_long=0 seq_count=28 first=0 last=33 seq_gap=6 seq_order=0 rx_bytes=792 rx_drop=550 uart_err=0 ore=0 rearm_err=0 log_drop=0 tx_err=0
```

### Giải thích

Khi tăng buffer gấp đôi, số lệnh hợp lệ tăng thêm 13 lệnh, từ 15/100 lên
28/100; tỷ lệ nhận đúng tăng 13 điểm phần trăm. Số byte bị bỏ giảm 96 byte,
từ 646 xuống 550. Với cùng 792 byte RX được ghi nhận, số byte đưa được vào
stream tăng từ `792-646=146` lên `792-550=242` byte.

Kết quả phù hợp với việc buffer lớn hơn chứa được nhiều dữ liệu hơn trong
lúc parser chờ truyền log. Tuy nhiên, chi phí truyền UART đồng bộ vẫn còn,
nên cả hai cấu hình vẫn bị đầy buffer và mất dữ liệu.


