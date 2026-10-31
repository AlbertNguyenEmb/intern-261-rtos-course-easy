# Report — Week 2 / Ex4

`student_id`: 2412477    Commit của PREDICTION.md: `d63e402`

## 1. Bảng so sánh RAM

| Cấu hình | Bytes thực tế | Cách tính |
|---|---:|---|
| Event Group (4 producer) | 24-byte object / 32-byte heap | `sizeof(StaticEventGroup_t) = 24`; `8184 - 8152 = 32` |
| Event Group (3 producer) + notification (1 producer) | 24-byte object / 32-byte heap | Event Group vẫn chiếm 32 byte heap; notification thêm 0 byte |
| Chênh lệch | 0 | `32 - 32 = 0` byte |

## 2. Bảng so sánh tốc độ đánh thức
| Cách | Thời gian đánh thức (cycle) | Thời gian đánh thức (us) |
|---|---|---|
| Event-group-bit (producer_1 hoặc 2) | | |
| Task notification (producer_0) | 33,241 (trung bình 500 lần) | 462 |

Mô tả cách đo: bật bộ đếm DWT `CYCCNT`. Producer lưu mốc bắt đầu ngay trước
khi gọi `xTaskNotifyGive()` hoặc `xEventGroupSetBits()`. Sau khi `sync_task`
được đánh thức và hàm chờ trả về, lấy `DWT->CYCCNT - mốc bắt đầu`. Chạy 500
lần và dùng `wake_avg_cycles` làm kết quả đại diện. Với STM32 chạy ở 72 MHz,
thời gian trung bình của notification là `33241 / 72 = 461.68 us`, làm tròn
thành 462 us. `wake_cycles=70347` và `wake_us=977` trong dòng thứ 500 chỉ là
giá trị tức thời của riêng mẫu cuối, không phải giá trị trung bình.

## 3. Khi nào KHÔNG dùng được notification
Tình huống bạn dự đoán ở PREDICTION.md, kiểm chứng lại: đúng hay sai, vì sao.
Không nên dùng Task Notification khi một sự kiện cần đánh thức nhiều task cùng lúc. Task Notification thuộc riêng về một task nhận, nên producer phải gửi notification lần lượt cho từng task và tự quản lý danh sách task. Trong khi đó, Event Group cho phép nhiều task cùng chờ một bit sự kiện và được đánh thức khi bit đó được set. Vì vậy, trường hợp broadcast sự kiện cho nhiều task phù hợp với Event Group hơn.
## 4. Timeout
Log/ảnh một lần `sync_task` bị timeout (cố tình làm 1 producer chậm hơn
`SYNC_TIMEOUT_MS`), và giá trị trả về của `xEventGroupWaitBits` lúc đó.
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
TIMEOUT returned=0x00000007 missing=0x00000008
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.

- Dự đoán Event Group có kích thước 24 byte là đúng khi kiểm tra bằng `sizeof`. Tuy nhiên, heap thực tế giảm 32 byte do `heap_4` cần thêm bộ nhớ để quản lý và căn chỉnh vùng nhớ.
- Dự đoán Notification không làm tăng dung lượng heap là đúng, vì dữ liệu notification đã được lưu sẵn trong TCB của task `sync_task`, không cần tạo thêm object mới.
- Dự đoán Notification nhanh hơn Event Group chưa khớp với kết quả đo. Notification có thời gian trung bình 33.241 cycles, trong khi Event Group chỉ khoảng 1.079 cycles. Kết quả có thể bị ảnh hưởng bởi việc chuyển đổi task, ngắt tick và thời điểm task được chạy. Một số mẫu Notification lên đến khoảng 72.000 cycles (xấp xỉ 1 ms), làm tăng giá trị trung bình. Ngoài ra, mốc thời gian DWT có thể bị task producer khác ghi đè. Vì vậy, chưa thể kết luận Notification luôn chậm hơn Event Group.
- Dự đoán về timeout là đúng. Khi hết thời gian chờ, Event Group trả về các bit đã nhận được thay vì một giá trị báo timeout riêng. Do đó, cần so sánh kết quả trả về với toàn bộ mask mong đợi để kiểm tra xem đã nhận đủ các sự kiện hay chưa.

## 6. Câu hỏi đánh đổi
Với `EVENT_BIT_COUNT` của **bạn**, nếu tăng thêm 1 bit (1 producer nữa) thì chu
kỳ đồng bộ trung bình đo được thay đổi thế nào? Trả lời bằng số từ dữ liệu của
bạn, rồi đo lại để kiểm chứng.
Với 4 producer, 13 chu kỳ khác 0 trong log hiện có là:
`6, 6, 5, 5, 6, 5, 6, 6, 5, 6, 5, 5, 5 ms`. Trung bình:

```text
T4 = 71 / 13 = 5.46 ms
```

Khi tăng lên 5 producer, task đồng bộ phải đợi thêm một nguồn và thời gian
thường do producer đến cuối cùng quyết định. Nếu xem thời điểm đến phân bố gần
đều, kỳ vọng cực đại tăng theo tỉ lệ `(5/6) / (4/5) = 25/24`. 
```text
EX4 mode=event-group producers=5 force_timeout=0 object_bytes=24 heap_before=8184 heap_after=8152 heap_bytes=32 notify_extra=0
SYNC count=500 tick=2845 period_ms=6 wake_cycles=1120 wake_avg_cycles=1100 wake_us=15
```

Kết luận: thêm producer thứ 5 được dự đoán làm chu kỳ đồng bộ tăng từ
5.46 ms lên khoảng 5.69 ms. 