# Report — Week 4 / Ex2

`student_id`: 2412477    Commit của PREDICTION.md: `59980c1`

## 1. Mô tả lỗi non-FromISR đã sửa

Lỗi gốc nằm trong `sampler_timer_callback()` ở `app/sampler_isr.c`: gọi
`xQueueSend(sample_queue, &s, 0)` trong ngữ cảnh timer ISR. Đây là API dành
cho task, không được dùng trong ISR ngay cả khi thời gian chờ bằng 0.

Trên port Cortex-M3, API task dùng cơ chế critical section và quản lý
chuyển ngữ cảnh dành cho task. `vPortEnterCritical()` kiểm tra IPSR bằng
`configASSERT` khi vào critical section ngoài cùng; gọi từ ISR có thể dừng
ở assert. Không khẳng định chip chắc chắn reset hoặc HardFault. Với đối số
chờ bằng 0 của lỗi gốc, queue đầy trả về thất bại, không phải tự đưa task
vào Blocked như khi dùng thời gian chờ khác 0.

Bản sửa trung gian dùng `xQueueSendFromISR()` và `portYIELD_FROM_ISR()`.
Bản hiện tại theo design doc: ISR chỉ gọi `vTaskNotifyGiveFromISR()` để
đánh thức Sampler, rồi `portYIELD_FROM_ISR()` để yêu cầu chuyển ngữ cảnh
khi cần. Sampler đọc I2C và gọi `xQueueSend()` trong task nên hợp lệ.
TIM2 dùng NVIC priority 5, bằng ngưỡng
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` của FreeRTOS.

**Quan sát thực tế trước khi sửa:** chưa ghi nhận trên board; chưa có ảnh
assert, log treo hoặc reset. Phân tích trên là đối chiếu source, không phải
kết quả thực nghiệm. Cần bổ sung bằng chứng nếu rubric yêu cầu tái hiện lỗi.

## 2. Kiến trúc hiện thực

Đối chiếu design doc thực tế tại `week4/ex1/REPORT_week4.md`:

```text
TIM2 ISR, 500 Hz
    → task notification
Sampler, priority 2
    → sample_queue, 32 phần tử
Process, priority 3
    → log_queue, 32 phần tử
Logger, priority 1
    → USART1, từng block 32 mẫu
```

- `sampler_isr_init()` tạo queue và cấu hình TIM2; timer chưa chạy.
  `sampler_isr_start()` chạy trong Sampler, lưu task handle trước khi bật
  ngắt. Với timer clock 72 MHz: PSC=71, ARR=1999, chu kỳ tính toán 2 ms.
- ISR không đọc I2C blocking. Sampler chờ notification, đọc 14 byte cảm
  biến, gắn timestamp DWT, kiểm tra lỗi rồi gửi mẫu. Tick dồn được gộp
  thành một lần đọc hiện tại; không thể đọc lại lịch sử cảm biến đã bỏ qua.
- Process hiện chuyển nguyên mẫu sang queue log; chưa có thuật toán lọc
  được đặc tả. Logger chờ queue, gom đủ 32 mẫu và truyền UART blocking.
- I2C1: PB6=SCL, PB7=SDA, 400 kHz, cần pull-up ngoài; địa chỉ MPU6050
  0x68 khi AD0 thấp. USART1 TX: PA9, 115200 baud, 8N1.
- DWT và ngoại vi được khởi tạo trước khi tạo queue/task; delay khi khởi
  tạo cảm biến hoàn tất trước khi các API FreeRTOS có thể mask HAL tick.
- Heap: 11264 byte theo `MAX_HEAP_BYTES`. Ba task được cấp 256 words/task
  = 1024 byte/task; khác dự toán Ex1 về stack Sampler và UART. Có kiểm tra
  kết quả tạo queue/task bằng `configASSERT`. Bật trace và runtime stats.

Kiểu `sensor_sample_t` dùng chung từ Ex1 gồm timestamp 4 byte và 7 trường
`int16_t`: 18 byte dữ liệu, `sizeof` trên ARM là 20 byte do padding. Mỗi
queue có 640 byte payload, chưa tính control block.

Wire format hiện tại là binary struct dump little-endian: timestamp,
accel x/y/z, temp, gyro x/y/z, cuối bản ghi có 2 byte padding. Python có
thể giải mã bằng `<I7h2x`; không diễn giải padding là dữ liệu cảm biến.
Mỗi block 640 byte, không có header/checksum. UART lỗi sau khi truyền một
phần block có thể làm mất ranh giới bản ghi; cần restart capture.

## 3. Thông lượng UART

| Đại lượng | Dự đoán ban đầu | Tính lại theo code hiện tại | Đo được trên board |
|---|---:|---:|---|
| Khối/giây | 15,625 | 500 / 32 = 15,625 | Chưa đo |
| Thời gian truyền 1 khối (µs) | Khoảng 44444 | 640 × 10 / 115200 × 10^6 = 55555,6 | Chưa đo |
| Kích thước block (byte) | 512 | 32 × 20 = 640 | Chưa capture UART |

Thời gian tạo một block: 32/500 = 64 ms. Băng thông cần thiết là
500×20 = 10000 B/s; UART 8N1 cung cấp lý tưởng 115200/10 = 11520 B/s,
tương ứng sử dụng khoảng 86,8%. Dự phòng lý tưởng chỉ còn 8,44 ms/block,
chưa tính software overhead và preemption; chưa đủ bằng chứng kết luận
không rớt mẫu khi chạy tải.

Cách bổ sung số đo: đo delta `uart_blocks` trong cửa sổ thời gian đã biết;
dùng logic analyzer đo burst TX PA9 để lấy thời gian trên dây. Đo bao quanh
`HAL_UART_Transmit()` bằng DWT là thời gian gọi hàm có cả preemption, cần
phân biệt với thời gian truyền thuần trên dây.

## 4. Lệch so với dự đoán

1. **Kích thước mẫu:** dự toán dùng 16 byte/mẫu; hiện có 18 byte trường và
   struct 20 byte. Vì vậy block tăng 512→640 byte, UART tăng 44,44→55,56 ms.
2. **RAM:** queue không còn khớp dự toán 8/16 byte mỗi phần tử của Ex1.
   Hai queue hiện có tổng 1280 byte payload. Stack ba task tổng 3072 byte,
   chưa gồm Idle, TCB, queue control block và overhead cấp phát.
3. **Deadline:** nếu hiểu 300 µs là từ timer ISR đến khi Process nhận đủ
   mẫu mới, đường đọc hiện tại không đáp ứng. Đọc I2C gồm address-W,
   register, address-R và 14 data byte, tối thiểu 17×9/400000 = 382,5 µs,
   chưa kể overhead. Chuyển sang task giúp ISR ngắn, timeout HAL hoạt động,
   nhưng không rút ngắn thời gian trên dây. Cần thống nhất cách định nghĩa
   deadline với mentor; chưa tự thay đổi params.h hoặc tuyên bố đạt.
4. **Phép đo lỗi/drop:** có `sample_errors`, `sample_overruns`,
   `raw_drop_count`, `log_drop_count`, `uart_errors`, `uart_blocks`.
   `queue_high_water_mark` đo queue log sau khi nhận, có thể thấp hơn peak
   thực. Các giá trị runtime hiện chưa được ghi nhận.
5. **Stack:** xem `sampler_stack_free`, `process_stack_free`,
   `logger_stack_free`; đơn vị words, nhân 4 để đổi byte. Chưa có số đo
   stack HWM dưới tải. DWT 32 bit ở 72 MHz wrap khoảng 59,65 giây; phép đo
   delta phải dùng unsigned và chọn cửa sổ phù hợp.

**Đã kiểm tra trên máy:** lần build Debug trong phiên hỗ trợ thành công,
không warning; `tools/check_api.sh week4/ex2` PASS. Linker báo RAM
14056/20480 byte và FLASH 22328/65536 byte. Đây là kết quả build/link,
không phải số đo CPU, stack hay latency trên board.

**Chưa kiểm tra trên board:** lần kiểm tra kết nối gần nhất không phát
hiện ST-LINK, chỉ có cổng serial Bluetooth. Chưa flash, chưa đo chu kỳ
thực 2 ms, timestamp, UART capture, drop, CPU% hay deadline dưới tải.
Khi có board cần bổ sung kết quả và điều kiện đo vào các mục tương ứng.

## 5. AI_LOG.md

File `week4/ex2/AI_LOG.md` tồn tại và đã ghi các nội dung hỗ trợ chính trong
phiên này. Chưa xác nhận đầy đủ mọi lần sử dụng AI ngoài phiên này; người
làm bài cần rà soát và bổ sung trước khi nộp.

## 5. AI_LOG.md
Xác nhận file tồn tại và đầy đủ: ______
