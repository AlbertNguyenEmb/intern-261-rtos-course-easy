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
Queue mẫu — dự kiến 32 phần tử
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
| Chu kỳ lấy mẫu | 1 / 500 Hz | 2000 µs |
| Latency ISR → Task | Đo thực tế Week 3/Ex1 | 10.67 µs |
| Thời gian đọc I2C | 9 × 9 × 2.5 | 202.5 µs | Đo 9 byte mỗi khung byte 9 bit mỗi lần truyền mất 2.5 us (Ngoài deadline)
| Context switch | Ước lượng | 8 µs |
| Tính toán và lọc | Ước lượng | 30 µs |
| Tổng so với `ISR_TO_PROC_DEADLINE_US` |  **10.67 + 8 + 30 = 48.67 µs**| Đạt |
Khi đọc queue thì chỉ có 8 mẫu nhưng lại đó đến 9 byte bởi vì HAL_I2C_Mem_Read thì giao thức I2C thực chất chỉ đọc 6 byte data và 3 byte đầu là 3 byte header. Do đó trong queue chỉ đọc 6 byte dữ liệu và 2 byte số thứ tự seq là đủ 8 byte.
T_proc và context switch chỉ là ước lượng.
## 3. RAM budget
| Thành phần | Kích thước | Ghi chú |
|---|---|---|
  | Stack task lấy mẫu | 768 B | |
   | Stack task xử lý | 1024 B | |
   | Stack task UART | 768 B | |
   | Stack Idle task | 512 B | |
   | Stack timer task | 512 B | | 
   | Queue mẫu: 32 × 8 B | 256 B | |
   | Queue log: 32 × 16 B | 512 B | |
   | TCB | 5 x 96 B | 480 B | 5 task mỗi task cần một pcb_t để quản lý thông tin |
   | Overhead queue | 2 x 80 | 160 B | 2 queue mỗi queue cần queue_t để quản lý cũng như vùng lưu trữ dữ liệu |
   | Block buffer UART | 32 × 16 B | 512 B |Bộ đệm gom 1 block 32 mẫu trước khi truyền UART |
   | **Tổng queue + stack mọi task** | **4352 B** | |
| **Tổng** | 4352 | So với `MAX_HEAP_BYTES` : 11264 B dư 6912 B |

## 4. Lựa chọn API đồng bộ
Queue / event group / stream buffer — vì sao chọn cái này cho pipeline này.
Dùng task notification từ ISR để đánh thức task đọc, vì ISR chỉ cần báo sự kiện cho một task nhận. Dùng queue giữa các task vì cần truyền từng bản ghi mẫu có ranh giới rõ ràng. Task xử lý chờ queue mẫu khi chưa có dữ liệu; task UART chờ queue log. Chỉ task UART sở hữu đường truyền log nên không cần mutex UART giữa ba task này.
## 5. Xử lý khi hàng đợi đầy
Drop mẫu mới hay mẫu cũ? Có đếm số mẫu bị rớt để báo cáo ở ex3 không?
Nếu queue mẫu đầy, task đọc không chờ kéo dài; bỏ mẫu mới và tăng raw_drop_count. Nếu queue log đầy, task xử lý không chờ UART; bỏ bản ghi mới và tăng log_drop_count. Mẫu có sequence/timestamp để nhận biết khoảng dữ liệu bị thiếu.
Khi hàng đợi đầy drop mẫu mới vì trong cơ chế rtos queue hoạt động theo fifo phải nhận rút khi receive ra thì mới lấy mẫu ra được cũng như là không dùng pormaxdelay để bảo toàn flow hoạt động.
## 6. Kiểm tra băng thông UART
UART sử dụng baud rate 115200, cấu hình 8N1
(1 start bit, 8 data bits, 1 stop bit).

| Đại lượng | Công thức | Giá trị |
|---|---|---:|
| Tần số lấy mẫu | SAMPLE_RATE_HZ | 500 mẫu/s |
| Kích thước bản ghi | 16 bytes/mẫu | 16 B |
| Băng thông UART tối đa | 115200 / 10 | 11520 B/s |
| Băng thông cần thiết | 500 × 16 | 8000 B/s |
| Kích thước một block | 32 × 16 | 512 B |
| Thời gian gom một block | 32 / 500 | 64 ms |
| Thời gian truyền một block | 512 × 10 / 115200 | 44.44 ms |
| Thời gian dự phòng | 64 - 44.44 | 19.56 ms |
| Mức sử dụng băng thông | 8000 / 11520 × 100% | 69.44% |
| Băng thông còn lại | 11520 - 8000 | 3520 B/s |

**Kết luận:** UART 115200 baud đáp ứng tốc độ 500 mẫu/s.
Thời gian truyền một block nhỏ hơn thời gian tạo một block mới
(44.44 ms < 64 ms), với băng thông dự phòng 30.56%.
## 7. Mentor duyệt
Chữ ký / ghi chú của mentor sau buổi duyệt thiết kế: ______
