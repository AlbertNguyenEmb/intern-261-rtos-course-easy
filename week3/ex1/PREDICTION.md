# Dự đoán — Week 3 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477

1. Với `ISR_NVIC_PRIO` của bạn và nhóm priority `NVIC_PRIORITYGROUP_4`, số này
   nằm trong khoảng cho phép (`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` ..
   `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`) không? Trình bày phép so sánh.
   Giá trị cao nhất của priority trong nvic_prioritygroup_4 là 15 và lowest là 15 do càng lớn thì độ ưu tiên càng thấp. Còn max_syscall của interupt là 5 nên giá trị priority của tôi thuộc khoảng 7 là hợp lý.
2. Khi đặt priority EXTI cao hơn ngưỡng cho phép rồi gọi
   `vTaskNotifyGiveFromISR`, bạn dự đoán hành vi quan sát được là gì (reset?
   treo? thông báo gì trên UART/debugger)?
   Hành vi quan sát được là chắc chắn hệ thống bị lỗi, hệ thống sẽ bị reset liên tục và trên uart sẽ bị hiện printf("Assert failed at %s:%d\n", __FILE__, __LINE__). 
3. Dự đoán latency ISR → task (có `portYIELD_FROM_ISR`) theo đơn vị us, dựa
   trên tần số CPU 72 MHz và số lệnh ước lượng trong đường đi ISR → context
   switch. So với `LATENCY_BUDGET_US`: đạt hay không?
   Ta muốn biết có kịp hay không dựa trên quá trình đi từ ISR tới Task
   Giai đoạn 1:
   Khi ngắt xảy ra, CPU Cortex-M mất cố định 12 chu kỳ lệnh để tự động đẩy các thanh ghi hiện tại (R0-R3, R12, LR, PC, xPSR) vào Stack và nhảy tới vector ngắt cần 12 chu kỳ.
   Giai đoạn 2:
   Thực thi hàm ngắt của bạn, gọi hàm vTaskNotifyGiveFromISR() và kích hoạt portYIELD_FROM_ISR().
   Hàm vTaskNotifyGiveFromISR của FreeRTOS sẽ kiểm tra, mở khóa Task đang đợi, và đặt cờ yêu cầu Context Switch (bằng cách set bit PendSV). Đoạn mã này tốn khoảng 40 - 60 lệnh.
   Giai đoạn 3:
   Khi ISR kết thúc, CPU nhận diện cờ PendSV đang bật và nhảy vào ngắt PendSV để thực hiện Context Switch.
   FreeRTOS lưu các thanh ghi còn lại (R4-R11), cập nhật TCB (Task Control Block), tìm Task có ưu tiên cao nhất vừa được Notify, rồi nạp Stack của Task mới. Đoạn code Assembly này mất khoảng 40 - 50 chu kỳ.
   CPU mất thêm 12 chu kỳ (Hardware Exit) để tự động pop các thanh ghi của Task mới từ Stack ra lại CPU.
   Thì tốn khoảng 132 chu kỳ mất khoảng 2.5us.
4. Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ chạy **khi nào** (ngay lập tức
   hay phải chờ tick ngắt kế tiếp)? Latency tăng thêm cỡ bao nhiêu?
   Khi bỏ portYIELD_FROM_ISR, ack_task chỉ chuyển sang Ready và thường phải chờ tick kế tiếp để được chạy. Với tick rate 1000 Hz, tôi dự đoán latency tăng thêm khoảng 0–1000 µs, trung bình khoảng 500 µs nếu sự kiện xảy ra ngẫu nhiên giữa hai tick. Giả sử ack_task có priority cao nhất trong các task Ready, bật preemption và không có tác nhân khác gây chuyển task sớm hoặc trì hoãn scheduler.