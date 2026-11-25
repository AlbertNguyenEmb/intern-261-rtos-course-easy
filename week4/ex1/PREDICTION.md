# Dự đoán — Week 4 / Ex1

> **Commit TRƯỚC khi gửi design doc cho mentor duyệt.**

`student_id`: 2412477

1. Với `SAMPLE_RATE_HZ` của bạn, chu kỳ lấy mẫu là bao nhiêu us? Dựa trên
   latency ISR→task thật bạn đã đo ở `week3/ex1`, dự đoán task xử lý có kịp
   `ISR_TO_PROC_DEADLINE_US` không? Trình bày phép tính, không đoán.
   Sample_rate_hz là 500 hz -> chu kỳ lấy mẫu là 1/500 = 2ms. Latency ISR to task đo được khi có yield là khoảng 10.67 us < 300us => đạt isr to proc.
2. Dự đoán tổng RAM (queue + stack mọi task) theo thiết kế của bạn, tính
   bằng bytes. So với `MAX_HEAP_BYTES`: còn dư bao nhiêu?
   | Thành phần | RAM |
   | :--- | :---: |
   | Stack task lấy mẫu | 768 B |
   | Stack task xử lý | 1024 B |
   | Stack task UART | 768 B |
   | Stack Idle task | 512 B |
   | Stack timer task | 512 B |
   | Queue mẫu: 32 × 8 B | 256 B |
   | Queue log: 32 × 16 B | 512 B |
   | **Tổng queue + stack mọi task** | **4352 B** |
   So với MAX_HEAP_BYTES = 11264 B thì
   Phần còn lại = 11264 − 4352 = 6912 B
3. Nếu task xử lý tạm thời chậm hơn ISR lấy mẫu (ví dụ do một task khác
   chiếm CPU), hàng đợi của bạn đầy sau bao nhiêu mẫu? Điều gì xảy ra với
   mẫu tiếp theo trong thiết kế của bạn?
   Hàng đợi của em sẽ đầy sau 32 mẫu. Mẫu tiếp theo nếu vẫn không xử lý kịp sẽ bị drop.
4. Bạn dự đoán priority nào (ISR task xử lý vs. task ghi UART) sẽ cho kết
   quả đạt deadline tốt hơn? Vì sao?
   Priority của task isr xử lý sẽ tốt hơn so với task ghi uart vì ghi uart là xuất log ra ngoài hình chứ không ảnh hưởng nhiều bằng độ chính xác khi lấy mẫu và ít rủi ro drop mẫu hơn.