# Dự đoán — Week 2 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    `LOW_HOLD_MS` = 8 ms  `MID_WORK_MS` = 40 ms
`HIGH_PERIOD_MS` = 50 ms  `DEADLINE_MS` = 15 ms

1. Cấu hình binary semaphore: thời gian `task_high` bị block dự đoán = 48 ms.
   Phép tính dựa trên đâu (liệt kê các thành phần cộng lại)?
   Khi task low giữ semaphore ban đầu thì gọi task high đã không có semaphore để chạy từ đó task mid đã chiếm quyền chạy của task low sau đó task low mới chạy
   => Tdelay = mid_work + low_hold_ms = 48 ms
2. `task_high` có đạt `DEADLINE_MS` ở cấu hình binary semaphore không? Vì sao?
   task_high ở cấu hình binary semaphore thì nó sẽ chạy sau 48ms do sau khi low task chạy mới trả semaphore mà deadline_ms là 15ms => trễ 
3. Cấu hình mutex: thời gian `task_high` bị block dự đoán = 8 ms. Vì sao
   ngắn hơn (hoặc không) so với câu 1?
   Bởi vì khi cấu hình mutex thì có cơ chế khi task 1 giữ mutex thì nó sẽ tạm nâng priority lên mức cao nhất là 3 để chạy xong sau đó trả mutex để task 3 có thể chạy mà không bị chờ như binary semaphore 
    `T_block,mutex ≈ LOW_HOLD_MS - 2 ms ≈ 6 ms`.

   Giá trị này nhỏ hơn deadline 15 ms nên task_high dự kiến đạt deadline.
4. Trên ảnh logic analyzer ở cấu hình binary semaphore, kênh `task_mid` sẽ ở
   trạng thái nào trong lúc `task_high` bị chặn — chạy liên tục, chạy ngắt
   quãng, hay không chạy? Vì sao?
   task_mid sẽ chạy liên tục vì task_mid có priority cao hơn task_low từ đó scheduler sẽ chọn task_mid chạy liên tục từ đó làm cho task_low k chạy để trả mutex được.
5. Nếu đổi `task_mid` sang ưu tiên **thấp hơn** `task_low`, inversion còn xảy ra
   không? Vì sao (đây là cách phân biệt priority inversion với việc chỉ đơn
   giản là "task_low chạy lâu").
