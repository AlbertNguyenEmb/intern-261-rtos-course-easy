# Dự đoán — Week 3 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477

1. Với nút nhấn cơ khí thông thường, nảy phím thường kéo dài bao lâu
   (tra cứu/ước lượng)? So với `DEBOUNCE_MS` của bạn: đủ lớn để lọc hết không?
   Khoảng 5ms đến 10ms so với deboune_ms của em là 25ms thì đủ để lọc hết.
2. Nếu `DEBOUNCE_MS` quá lớn, điều gì xảy ra khi người dùng nhấn nhả rất
   nhanh liên tiếp (double-click)?
   Thì khi người dùng nhấn thả quá nhanh thì sẽ có 2 click liên tiếp trong 1 khoảng thời gian nếu thời gian đó < debounce_ms thì nó sẽ bị mất 1 lần ở phía sau.
3. Khi callback của `debounce_timer` bị block 50 ms (thử nghiệm bước 4), bạn
   dự đoán `longpress_timer` sẽ bị trễ bao nhiêu? Giải thích bằng cơ chế
   daemon task + queue lệnh, không chỉ đoán số.
   Khi debounce_timer bị kích hoạt tại thời điểm t = 0 thì longpress_timer cũng bị kích hoạt theo và thời gian nhấn giữ của nó là 600ms lớn hơn rất nhiều so với 50ms delay của debounce_timer => không bị delay.
4. `LONGPRESS_MS` của bạn là bao nhiêu? Giữ phím đúng ngưỡng này, bạn dự đoán
   callback `longpress_callback` chạy lần đầu ở thời điểm nào tính từ lúc
   nhấn xuống?
là 600ms sẽ chạy lần đầu ở t = 600ms tính từ thời điểm nhấn xuống.