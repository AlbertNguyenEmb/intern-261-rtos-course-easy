# Dự đoán — Week 4 / Ex3

> **Commit TRƯỚC khi đo tải cao nhất.**

`student_id`: 2412477

1. Dự đoán CPU% của `logger_task` khi hệ thống ở tải cao nhất — ước lượng
   dựa trên `SAMPLE_RATE_HZ` và thời gian xử lý 1 mẫu bạn đã ước lượng ở
   `week4/ex1`.
   Phần trăm cpu dự đoán:
   Process (p3) : 30us thì 1 giây lấy được 500 mẫu => 500 x 30 = 15000 us thời gian dùng để lấy mẫu trong 1s => khoảng 1.5%
   tương tự ta tính được
   Sampler: 20%
   Noise 50%
   Phần còn lại của logger_task.
2. Dự đoán stack HWM (bytes còn trống) của từng task — task nào bạn nghĩ
   sẽ có margin thấp nhất? Vì sao?
   Byte vẫn còn nhiều byte trống của từng task nhưng margin nhiều nhất là measure bởi vừa gọi snprintf và vTaskGetRunTimeStats tốn nhiều stack nhất
3. Dự đoán latency ISR→xử lý ở worst-case có còn nằm trong
   `ISR_TO_PROC_DEADLINE_US` không khi có task nhiễu priority cao hơn
   chạy cùng? Nếu không đạt, bạn dự đoán lệch bao nhiêu %?
   Nếu worst_case của process_task thì sẽ bị tính của noise_task cùng p2 thì thằng sampler_task phải đợi chạy xong thì mới tới lượt gửi isr để process_task chạy và sampler_task đọc mpu 14 byte phải mất thời gian tiếp => khó mà thỏa mãn deadline ~450 – 1450 µs > 300 µs → dự đoán KHÔNG đạt.