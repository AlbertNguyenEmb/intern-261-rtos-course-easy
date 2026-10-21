# Dự đoán — Week 2 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477

1. Phép tính kích thước queue tối thiểu (dùng `SAMPLE_PERIOD_MS`, thời gian xử lý
   ước lượng của `filter_task` và `uart_task` ở `UART_BAUD`):

   ```
   Với T = 10 ms thì mỗi giây sensor sẽ gửi 'fsample = 1000/10 = 100 mẫu/s'
   ```

   Kích thước tối thiểu dự đoán: `queue_raw` = 1 , `queue_filtered` = 1 .
   So với `QUEUE_LENGTH` được cấp: đủ hay không? Vì sao?
   "seq,ax,ay,az\r\n"
   Ta nhận thấy seq có kiểu dữ liệu uint32_t thì số ký tự lớn nhất của kiểu dữ liệu là 10 ký tự, 3 trục mỗi trục có kiểu dữ liệu là `int16_t` sẽ có nhiều nhất `6x3 = 18` ký tự cuối cùng là 3 dấu phẩy và \r\n => Tổng 33 bytes
   1 đường truyền uart cần 10 bit gồm 8 bit data, 1 bit start và 1 bit stop => uart cần 10 bit/byte => tổng 33 x 10 = 330 bit
   `Tuart = 330 / 230400 giây
      = 0.001432 giây
      = 1.432 ms`
   => uart truyền xong thì mẫu kia đã tới mà queue_filter lại nhanh hơn lấy mẫu rất nhiều => 1 mẫu là đủ
2. Khi hạ `UART_BAUD` xuống 9600 (tạm thời), queue nào sẽ đầy trước? Vì sao?
Khi giảm baud xuống 9600:

`Tuart = 33 × 10 / 9600 = 34.375 ms`.

UART chỉ xử lý khoảng `29.1 mẫu/s`, trong khi filter tạo khoảng
`100 mẫu/s`. Backlog tăng khoảng `70.9 mẫu/s`.

Với queue dài 4, thời gian đầy ước lượng là:

`4 / 70.9 ≈ 56 ms`.

Dự đoán queue_filtered đầy trước sau khoảng 50–70 ms vì uart_task
là nút thắt của pipeline.
3. Với `xQueueSend(..., 0)` (timeout = 0) khi queue đầy: điều gì xảy ra với mẫu đó?
Với timeout bằng 0, khi queue đầy, xQueueSend trả thất bại ngay và
task không chuyển sang Blocked. Mẫu bị mất nếu chương trình không tự
lưu hoặc gửi lại. Cần kiểm tra return value và tăng bộ đếm drop.
4. Với `xQueueSend(..., portMAX_DELAY)` khi queue đầy: `sensor_task` sẽ ở trạng thái
   nào? Có nguy cơ gì cho chu kỳ lấy mẫu?
Với timeout hữu hạn, task bị Blocked tối đa số tick đã chọn. Nếu có
chỗ trước khi hết timeout thì send thành công; nếu không thì send thất
bại. Chờ queue có thể làm tăng jitter của chu kỳ lấy mẫu.
5. Độ đầy `queue_raw` theo thời gian ở cấu hình bình thường sẽ trông như thế nào
   (tăng dần / dao động quanh một mức / luôn gần 0)? Vì sao?
Với portMAX_DELAY, sensor_task chuyển sang Blocked cho tới khi queue có
chỗ. Mẫu đang gửi không bị bỏ, nhưng chu kỳ lấy mẫu 10 ms không còn
được đảm bảo. Backpressure có thể truyền từ UART về filter rồi tới
sensor.