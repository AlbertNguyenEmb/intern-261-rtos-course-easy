# Dự đoán — Week 1 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    Ưu tiên của tôi: A = 2 B = 1 C = 3

1. Bước 2 (`configUSE_PREEMPTION`=1, ưu tiên khác nhau): task nào chạy? task nào
   **không bao giờ** chạy? CPU share dự đoán:
   Dự đoán là task C sẽ chạy liên tục nếu task không bị blocked.
   | Task | CPU share dự đoán (%) |
   |---|---|
   | A | 0|
   | B | 0|
   | C | 100|

2. Bước 3 (`configUSE_PREEMPTION`=0): dự đoán thay đổi gì so với bước 2? Vì sao?
Gần như không thay đổi vì C vẫn chiếm CPU do k bị block và yield
3. Bước 4 (3 task cùng ưu tiên): time slice dự đoán = ______ ms. Dựa vào đâu?
3 Task cùng ưu tiên nó sẽ thay nhay chạy nếu tick rate = 1000 -> time slice = 1ms (Thời gian mỗi task được chạy trước khi chuyển sang task khác cùng mức ưu tiên)
4. Bước 5: tôi sẽ sửa starvation bằng cách: ______ . Cách này đánh đổi gì?
Thêm vTaskDelay(1) để tạm thời Task đang chạy bị block. Đánh đổi là tăng độ trễ và giảm tần suất thực hiện công việc của task.
5. Chân nào sẽ không bao giờ đổi trạng thái trên logic analyzer ở bước 2? ______
GPIO của Task A và Task B sẽ không đổi trạng thái trong giai đoạn đo ổn định, vì cả hai không được chạy.