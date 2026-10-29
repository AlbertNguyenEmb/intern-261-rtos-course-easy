# Report — Week 2 / Ex3

`student_id`: 2412477    Commit của PREDICTION.md: caad200

## 1. Ảnh đo
Binary semaphore: `![](inversion_binsem.png)`

Mutex: `![](inversion_mutex.png)`

Trên mỗi ảnh, đánh dấu: thời điểm `task_high` sẵn sàng chạy (t0), thời điểm nó
thực sự chạy (t1). Thời gian block = t1 − t0.

## 2. Bảng thời gian block
| Cấu hình | t0 (ms) | t1 (ms) | Block (ms) | Đạt `DEADLINE_MS`? |
|---|---|---|---|---|
| Binary semaphore | 29|77 |48 | Không |
| Mutex | 30|38 |8 | Có |

## 3. Vì sao mutex ngắn hơn
Giải thích bằng priority inheritance: `task_low` được nâng lên ưu tiên nào, khi
nào, và việc đó ngăn `task_mid` chen vào như thế nào.
task_low được nâng lên ưu tiên 3 bằng với task_high đang chờ mutex, khi đó việc ngăn task_mid do tạm thời task_high có độ ưu tiên cao hơn task_mid.
## 4. task_mid trong lúc inversion
Mô tả trạng thái kênh `task_mid` trên ảnh binary semaphore trong khoảng
inversion — chạy bao nhiêu lần, mỗi lần bao lâu.

## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý. Không có lệch

## 6. Câu hỏi đánh đổi
Với bộ tham số của **bạn**, nếu `MID_WORK_MS` tăng gấp đôi thì thời gian block ở
cấu hình binary semaphore tăng thêm bao nhiêu? Trả lời bằng số từ dữ liệu của
bạn, rồi đo lại để kiểm chứng. `task_high` còn đạt `DEADLINE_MS` không?

Ở phép đo ban đầu, với `MID_WORK_MS = 40 ms`, thời gian
`task_high` bị block khi dùng binary semaphore là 48 ms.

Khi tăng `MID_WORK_MS` lên 80 ms, thời gian CPU mà `task_mid`
chiếm thêm là:

`80 - 40 = 40 ms`

Do đó, thời gian block dự đoán tăng khoảng 40 ms, từ 48 ms lên
khoảng 88 ms. Kết quả đo lại là ... ms, lệch ... ms so với dự đoán.

Vì thời gian block lớn hơn `DEADLINE_MS = 15 ms`, `task_high`
không đạt deadline.
