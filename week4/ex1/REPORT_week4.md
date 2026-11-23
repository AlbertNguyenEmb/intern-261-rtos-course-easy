# Report — Week 4 / Ex1 (Design doc)

`student_id`: 2412477    Commit của PREDICTION.md: ______

> File này ĐÓNG VAI TRÒ design doc 2–3 trang nộp cho mentor duyệt trước khi
> viết code. Điền đủ số liệu — không mô tả chung chung.

## 1. Task diagram
Mô tả (sơ đồ ASCII hoặc ảnh chèn vào) luồng: ISR lấy mẫu MPU6050 → hàng đợi
→ task xử lý → task ghi UART. Ghi rõ priority từng task.
Ngắt lấy mẫu: mỗi 2 ms
        │ notification
        ▼
Task đọc MPU6050 — priority 2
        │ đọc dữ liệu, gắn timestamp/sequence
        ▼
Queue mẫu — dự kiến 8 phần tử
        │
        ▼
Task xử lý — priority 3
        │ xử lý ngay từng mẫu
        ▼
Queue log — dự kiến 32 phần tử
        │
        ▼
Task UART — priority 1
        │ gom 32 mẫu thành một block
        ▼
UART
Task xử lý có priority 3 để xử lý ngay khi queue có mẫu. Task đọc có priority 2, được đánh thức bởi notification mỗi kỳ lấy mẫu. Task UART có priority 1 vì việc đóng block và truyền log được thực hiện sau khi mẫu đã xử lý xong. Task đọc và task xử lý block khi chưa có công việc, nhường CPU cho logger.
## 2. Timing budget
| Đại lượng | Công thức / nguồn số liệu | Giá trị |
|---|---|---|
| Chu kỳ lấy mẫu | 1 / `SAMPLE_RATE_HZ` | 2.000 µs |
| Latency ISR→task (đo thật, week3/ex1) | | 10.67 us |
| Thời gian xử lý 1 mẫu (ước lượng) | | 242,5 µs|
| Tổng so với `ISR_TO_PROC_DEADLINE_US` | | Đạt / Không đạt |

## 3. RAM budget
| Thành phần | Kích thước | Ghi chú |
|---|---|---|
| Queue (độ dài × cỡ phần tử) | 8 × 16 = 128 B| |
| Stack task lấy mẫu |256 × 4 = 1.024 B| 256 phần tử stack|
| Stack task xử lý |192 × 4 = 768 B | |
| Stack task ghi UART | 256 × 4 = 1.024 B| |
| **Tổng** | 2176 | So với `MAX_HEAP_BYTES` | dư 

## 4. Lựa chọn API đồng bộ
Queue / event group / stream buffer — vì sao chọn cái này cho pipeline này.
Dùng task notification từ ISR để đánh thức task đọc, vì ISR chỉ cần báo sự kiện cho một task nhận. Dùng queue giữa các task vì cần truyền từng bản ghi mẫu có ranh giới rõ ràng. Task xử lý chờ queue mẫu khi chưa có dữ liệu; task UART chờ queue log. Chỉ task UART sở hữu đường truyền log nên không cần mutex UART giữa ba task này.
## 5. Xử lý khi hàng đợi đầy
Drop mẫu mới hay mẫu cũ? Có đếm số mẫu bị rớt để báo cáo ở ex3 không?
Nếu queue mẫu đầy, task đọc không chờ kéo dài; bỏ mẫu mới và tăng raw_drop_count. Nếu queue log đầy, task xử lý không chờ UART; bỏ bản ghi mới và tăng log_drop_count. Mẫu có sequence/timestamp để nhận biết khoảng dữ liệu bị thiếu.
## 6. Mentor duyệt
Chữ ký / ghi chú của mentor sau buổi duyệt thiết kế: ______
