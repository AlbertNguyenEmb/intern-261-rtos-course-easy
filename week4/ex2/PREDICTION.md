# Dự đoán — Week 4 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477

1. Lỗi gọi API queue không phải `FromISR` bên trong ISR sẽ gây ra hiện
   tượng gì khi chạy (configASSERT? treo? reset?). Giải thích bằng cơ chế
   (ngăn xếp ISR, gọi `vTaskSuspendAll`/scheduler từ ngữ cảnh ngắt), không
   chỉ "nó bị lỗi".
   Khi gọi api queue không trong FromISR thì có thể sẽ xảy ra các trường hợp:
   - Ngắn xếp ISR: 
   Bình thường các api không chứa FromISR thường được thiết kế cho task. Nếu Queue đầy hàm sẽ gọi bộ lập lịch để đưa task hiện tại vào trạng thái blocked;
   Còn ISR thì không thể vào trạng thái đó nên nếu khi queue đầy và nhảy và isr, RTOS kernel sẽ lấy nhầm Task đang bị ngắt tạm dừng để đưa vào trạng thái Blocked, làm sai lệch hoàn toàn danh sách quản lý Task của Scheduler.
   - Xung đột ngăn xếp: 
   Task bình thường sử dụng ngăn xếp PSP trên Cortex-M còn ISR dùng ngăn xếp riêng của nó khi chuyển ngữ cảnh trong ngắt xong muốn khôi phục context cũ thì cần truy cập vào ngăn xếp của task thường chứ không phải của isr => gây ra lỗi HardFault và Reset chip
   - Ngữ cảnh ngắt: 
   + Khi sửa dữ liệu Queue, hàm thường dùng taskENTER_CRITICAL() để tạm tắt ngắt và taskEXIT_CRITICAL() để bật lại ngắt. Cơ chế này mặc định chỉ chạy ở Task nên nó không lưu lại trạng thái ngắt trước đó, mà chỉ đếm số lần vào/ra để quyết định khi nào bật lại toàn bộ ngắt.
   + Hậu quả khi ISR gọi hàm thường: Nếu một Task đang ở trong vùng khóa (tắt ngắt) để sửa dữ liệu mà bị một ngắt ưu tiên cao hơn nhảy vào, việc ISR gọi hàm thường sẽ làm sai lệch bộ đếm khóa và tự ý bật lại ngắt quá sớm khi kết thúc.
   + Kết quả: Khi ISR thoát ra, Task gốc mất lớp bảo vệ, bị các ngắt khác chen vào giữa chừng khi đang sửa dữ liệu dở dang, dẫn đến xung đột dữ liệu (race condition) và làm hỏng bộ nhớ Queue.
2. Sau khi sửa đúng API `FromISR`, dự đoán số mẫu ghi UART mỗi giây =
   `SAMPLE_RATE_HZ` / `LOG_BLOCK_SAMPLES` khối/giây. Tính ra con số.
   Khi sửa đúng api thì mỗi khối mất số mẫu ghi UART mỗi giây =
   `500` / `32` = 15.625 khối/giây
3. Với `UART_BAUD` của bạn, một khối `LOG_BLOCK_SAMPLES` mẫu mất bao lâu để
   truyền xong? Có nguy cơ UART chưa gửi xong khối cũ mà khối mới đã sẵn
   sàng không?
   Thời gian sinh một khối mới là 32 mẫu / 500 mẫu/s = 0.064s
   Mỗi byte uart truyền mất 10/115200 = 86.81us => mỗi khối gồm 32 mẫu
   $$T_{\text{tx}} = 32 \times B_{\text{sample}} \times 86.81\ \mu\text{s}$$ = $44.44\text{ ms}$ 
   => < 64 ms thỏa mãn điều kiện.