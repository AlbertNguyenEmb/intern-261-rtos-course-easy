# Dự đoán — Week 3 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477

1. Chạy `app/deadlock_demo.c` y nguyên như được cấp. Dự đoán: hệ thống sẽ
   treo sau khoảng bao lâu (ngay lập tức, hay sau vài chu kỳ)? Vì sao không
   phải ngay lập tức (gợi ý: cần cả 2 task "đua" vào đúng thời điểm)?
   2 task cùng lấy mutex ở ngay ban đầu nhưng sau khoảng cả 2 cùng nhận mutex thì sẽ bị deadlock còn phụ thuộc bài scheduler.
2. Khi treo, `uxTaskGetSystemState` sẽ báo trạng thái gì cho `task_alpha` và
   `task_beta`? Dự đoán tên mutex mỗi task đang chờ (dựa trên đọc code, chưa
   Dự đoán cả task_alpha và task_beta đều ở trạng thái eBlocked. Alpha giữ s_mutex_a và chờ s_mutex_b; beta giữ s_mutex_b và chờ s_mutex_a, tạo thành vòng chờ không thể tự giải phóng.
   chạy công cụ).
3. Tính reload value cho IWDG với `IWDG_TIMEOUT_MS` của bạn (LSI ~40 kHz).
   Với prescaler = 64 và T_timeout = 1s thì `Reload value = (T_timeout x f_lsi)/prescaler` = 625.
4. Sau khi sửa lỗi deadlock và thêm supervisor + IWDG, nếu bạn cố tình tái
   tạo lại tình huống deadlock cũ (tạm thời), dự đoán board sẽ phản ứng thế
   nào và sau bao lâu.
   Khi tái tạo deadlock, alpha và beta ngừng check-in. Supervisor không thu đủ bit nên ngừng refresh watchdog. Board dự kiến reset khoảng 1000 ms kể từ lần refresh cuối cùng, có sai số do LSI. Thời gian tính từ lúc deadlock không nhất thiết đúng 1000 ms vì watchdog có thể đã đếm một phần thời gian trước đó.

