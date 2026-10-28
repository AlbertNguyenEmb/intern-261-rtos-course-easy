# Dự đoán — Week 2 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    `EVENT_BIT_COUNT` = 4  `SYNC_TIMEOUT_MS` = 80

1. RAM event group chiếm (bytes) dự đoán: ______ . Dựa vào đâu (cấu trúc dữ
   liệu nào trong FreeRTOS)?
   Dự đoán RAM event group khoảng 24 bytes. Gồm 4 byte uxEventBits và 20 bytes xTasksWaitingForBits.
2. RAM thêm khi dùng task notification cho 1 điểm đồng bộ: dự đoán = ______
   bytes. Vì sao?
   0 RAM bytes. Bởi vì notification thường được tích hợp state và value trong TCB không cần một object cụ thể để quản lý.
3. Dự đoán thời gian đánh thức (producer gọi → sync_task chạy) của notification
   so với event-group-bit: nhanh hơn / chậm hơn / xấp xỉ? Vì sao (nghĩ đến số
   lớp hàm bên dưới mỗi API)?
   Notification nhanh hơn event group vì 
   - Notification cập nhập trực tiếp tcv của task đích
   - Event group phải cập nhập event bít, kiểm tra điều kiện mask hay các task chờ cũng như nhiều lớp xử lý hơn
4. Nếu `sync_task` bị timeout (`SYNC_TIMEOUT_MS` hết hạn) vì một producer
   không set bit kịp, `xEventGroupWaitBits` trả về gì? Làm sao phân biệt được
   "đủ bit" với "timeout"?
   xEventGroupWaitBits() không trả về một hằng số timeout riêng. Ta kiểm tra các bit được trả về để xác định đủ điều kiện đồng bộ hay chưa, nếu chưa đủ sau khi hết thời gian chờ thì xử lý timeout.
5. Nêu trước một tình huống bạn nghĩ **không thể** thay toàn bộ event group
   bằng notification — sẽ kiểm chứng lại ở REPORT.md.
   Một sự kiện cần đánh thức nhiều task cùng chờ một bit. Event Group cho nhiều task chờ cùng event bit, trong khi notification thuộc riêng từng task, nên phải notify từng task riêng và tự quản lý danh sách task nhận.
