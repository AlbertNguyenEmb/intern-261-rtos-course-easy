# Report — Week 2 / Ex2

`student_id`: 2412477    Commit của PREDICTION.md: 

## 1. Lỗi tái hiện (UART_GUARD_MODE = 0)
Log UART dán nguyên văn, đánh dấu vị trí ký tự lẫn lộn:
[W0 seq=001450]AAAAAAAAAAAAAAA
[W1 seq=001449]BBBBBBBBBBBBBBB
[W2 seq=001442]CCCCCCCCCCCCCCC
[W0 seq=001451]AAAAAAAAAAAAAAA
[W1 seq=001450]BBBBBBBBBBBBBBB
[W2 seq=001443]CCCCCCCCCCCCCCC
[W0 seq=001452]AAAAAAAAAAAAAAA
[W1 seq=001451]BBBBB[W2 seq=001BBBBBBBBBB
444]CCCCCC
[W0 seq=0CCCCCCCCC
01453]AAAA[W1 seq=00AAAAAAAAAAA1452]BBBBB
[W2 seq=BBBBBBBBBB
001445]CCC
[W0 seq=0CCCCCCCCCCC01454]AAAAC
[W1 seqAAAAAAAAAAA=001453]BB
[W2 seq=BBBBBBBBBBB001446]CCCBB
[W0 seCCCCCCCCCCCq=001455]AC
[W1 seqAAAAAAAAAAA=001454]BBAAA
[W2 sBBBBBBBBBBeq=001447]CBBB
[W0 sCCCCCCCCCCCeq=001456]CCC
[W1 sAAAAAAAAAAeq=001455]BAAAAA
[W2BBBBBBBBBBB seq=00144BBB
[W0 s8]CCCCCCCCeq=001457]ACCCCCCC
[AAAAAAAAAAW1 seq=0014AAAA
[W2 56]BBBBBBBseq=001449]BBBBBBBB
CCCCCCCCCCC[W0 seq=00CCCC
1458]AAAAAAAAAAAAAAA
[W1 seq=001457]BBBBBBBBBBBBBBB
[W2 seq=001450]CCCCCCCCCCCCCCC
[W0 seq=001459]AAAAAAAAAAAAAAA
[W1 seq=001458]BBBBBBBBBBBBBBB
[W2 seq=001451]CCCCCCCCCCCCCCC
[W0 seq=001460]AAAAAAAAAAAAAAA
[W1 seq=001459]BBBBBBBBBBBBBBB
[W2 seq=001452]CCCCCCCCCCCCCCC

Tái hiện lại bao nhiêu lần / bao nhiêu lần thấy lỗi: 3 / 3.

## 2. Bảng so sánh ba cách

| Cách | Độ trễ trung bình (µs) | Độ trễ max (µs) | Ảnh hưởng task khác | RAM thêm |
|---|---:|---:|---|---:|
| Mutex | 6734 | 7223 | Writer không giữ mutex chuyển sang Blocked; scheduler và interrupt vẫn hoạt động | 80 byte |
| Critical section | 6178 | 6204 | Scheduler và các IRQ nằm dưới ngưỡng `BASEPRI` bị trì hoãn trong lúc truyền | 0 byte |
| Gatekeeper | 9372 | 9466 | Writer block chờ queue/notification; interrupt vẫn hoạt động, UART chỉ có một owner | 1304 byte |

Cách đo độ trễ in (mô tả ngắn, kèm ảnh logic analyzer nếu dùng pin đánh dấu):
Latency được đo bằng bộ đếm cycle DWT của Cortex-M3. Mỗi writer đọc
`DWT->CYCCNT` ngay trước khi gọi `uart_guard_send()` và đọc lại sau khi
message đã truyền xong.

Công thức quy đổi:

`latency_us = elapsed_cycles / (SystemCoreClock / 1,000,000)`.

Với `SystemCoreClock = 72 MHz`, một microsecond tương ứng 72 cycle.

Các biến `g_latency_sum_us`, `g_latency_count`, `g_latency_avg_us` và
`g_latency_max_us` được tách theo từng writer để hệ thống đo không tạo
thêm race condition.

Trong mode gatekeeper, writer chờ task notification từ gatekeeper sau khi
UART truyền đủ 32 byte. Vì vậy số đo bao gồm toàn bộ thời gian từ lúc writer
muốn in đến lúc UART truyền xong, không chỉ thời gian enqueue.
## 3. Vì sao critical section nguy hiểm nhất
Giải thích bằng số đo của bạn: đoạn in dài bao nhiêu us khi ở trong critical
section, và điều đó chặn ngắt nào trong hệ thống bao lâu.
## 3. Vì sao critical section nguy hiểm nhất

Critical section truyền đủ 32 byte trong khoảng trung bình
`3054 µs`, lớn nhất `3073 µs`.

Trong khoảng này FreeRTOS sử dụng `BASEPRI` để ngăn context switch và mask
các interrupt có priority nằm trong vùng được quản lý bởi kernel. SysTick
và PendSV bị trì hoãn; các IRQ ngoại vi priority thấp cũng không được phục
vụ đúng thời điểm.

Critical section không nhất thiết tắt mọi interrupt của Cortex-M3. Các IRQ
có priority khẩn cấp hơn `configMAX_SYSCALL_INTERRUPT_PRIORITY` vẫn có thể
chạy. Tuy nhiên giữ critical section gần 2.8 ms vẫn quá dài so với tick
FreeRTOS 1 ms.

Hậu quả cụ thể:

- scheduler có thể trễ gần ba tick;
- `vTaskDelay()` và timeout được xử lý muộn;
- jitter của task định kỳ tăng;
- peripheral có FIFO nhỏ có thể mất dữ liệu;
- nhiều writer liên tục vào critical section khiến CPU ít có cơ hội phục vụ
  phần còn lại của hệ thống.

Mutex và gatekeeper cho phép task đang chờ chuyển sang trạng thái Blocked,
trong khi critical section trì hoãn hoạt động của toàn hệ thống. Vì vậy
critical section là phương án nguy hiểm nhất dù không dùng thêm RAM.
## 4. Lệch so với dự đoán

1. Thứ tự latency đo được là:

   `Critical section < Mutex < Gatekeeper`

   Kết quả này khớp dự đoán. Critical section không cho scheduler chuyển
   writer trong quá trình truyền; mutex thêm thời gian chờ owner; gatekeeper
   thêm queue, notification và thời gian chờ task chuyên trách.

2. Gatekeeper dùng 1304 byte, lớn hơn mutex 80 byte và critical section
   0 byte, khớp dự đoán. Phần tăng đến từ queue, TCB và stack 256 words của
   gatekeeper.

3. Thời gian truyền vật lý dự đoán là 2778 µs nhưng latency end-to-end đều
   lớn hơn. Phép tính lý thuyết chỉ tính thời gian trên đường UART, không
   gồm thời gian chờ mutex, context switch, queue và task notification.

4. Mode không bảo vệ tái hiện lỗi trong 3/3 lần. Thời điểm xuất hiện cụ thể
   phụ thuộc pha khởi động của ba writer và thời điểm tick interrupt, nên
   không nhất thiết xuất hiện đúng tại số dòng đã dự đoán.
## 5. Câu hỏi đánh đổi
Với `WRITER_COUNT` của **bạn**, nếu tăng gấp đôi thì cách nào bị ảnh hưởng độ trễ
nhiều nhất? Trả lời bằng số từ dữ liệu của bạn, rồi đo lại để kiểm chứng.
Với ba writer, gatekeeper có latency trung bình 9372 µs. Khi tăng lên sáu
writer, một writer có thể phải chờ gatekeeper phục vụ gần sáu message:

`6 × 32 × 10 / 115200 ≈ 16668 µs`.

Sau khi cộng queue và context switch, dự đoán latency
gatekeeper khoảng 18–20 ms. Gatekeeper chịu ảnh hưởng rõ nhất vì toàn bộ
message được xếp hàng tại một consumer duy nhất.

