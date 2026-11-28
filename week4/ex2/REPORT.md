# Report — Week 4 / Ex2

`student_id`: 2412477    Commit của PREDICTION.md: 59980c1

## 1. Mô tả lỗi non-FromISR đã sửa
Vị trí trong code, thông điệp/hành vi quan sát được trước khi sửa, và giải
thích chính xác vì sao đó là lỗi (tên macro assert nếu có, cơ chế kernel).
Vị trí nằm ở dòng thứ 43 của sampler_isr. Khi gọi hàm send bên trong ngữ cảnh callback ngắt của timer.
Chương trình bị treo tại vòng lặp vô hạn của configASSERT() ngay ở nhịp ngắt timer đầu tiên bởi vì cơ chế của kernel. Khi vào vùng critial section thì nó sẽ kiểm tra thanh ghi trạng thái ngắt là VECTACTIVE nên nó sẽ bị configassert khóa và bị treo chương trình. Cũng như API bình thường sẽ không có taskyield để context switch để nhảy task nếu có task mới có ưu tiên cao hơn khi nó bị đánh thức.
## 2. Kiến trúc hiện thực
So với design doc `week4/ex1/REPORT.md`: có điểm nào phải đổi so với thiết
kế ban đầu không? Vì sao?
Không có thay đổi.
## 3. Thông lượng UART
| Đại lượng | Dự đoán | Đo được |
|---|---|---|
| Khối/giây | 15.625 khối/giây |5.08 khối /s  |
| Thời gian truyền 1 khối (ms) |44.44 ms |186.262 ms |

## 4. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
Lệch do em đổi từ binary sang text csv để dễ nhìn mỗi mẫu tốt hơn khoảng 40-50 byte nên thời gian truyền uart đã tăng lên. Hơn nữa là vẫn đề preempt giữa chừng khi đang chời UART logger_task ở priority 1, dùng HAL_UART_Transmit (blocking/polling). Cứ mỗi 2 ms (500 Hz), sampler_task (prio 2) và process_task (prio 3) lại preempt logger_task để đọc I2C MPU6050 và đẩy queue.
Còn phần truyền khối/s thì thực tế truyền 1 khối mất 186.26 ms > 64 ms, nên logger_task bị nghẽn cổ chai ở UART: trong lúc đang gửi khối cũ, log_queue (32 phần tử) bị đầy và process_task phải drop bớt mẫu (log_drop_count tăng).
## 5. AI_LOG.md
Xác nhận file tồn tại và đầy đủ: ______