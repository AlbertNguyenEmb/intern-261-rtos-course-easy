# Dự đoán — Week 2 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    `WRITER_COUNT` =  3  `MSG_LEN` = 32 bytes

1. Ở `UART_GUARD_MODE = 0`: dự đoán lỗi lẫn lộn sẽ xuất hiện sau khoảng bao
   nhiêu dòng log? Vì sao MSG_LEN và WRITER_PERIOD_MS của bạn ảnh hưởng đến
   con số này?
   Với msg_len 32 byte tại 115200 baud thì cần khoảng 2.7ms giây để truyền mà chu kỳ ghi của 3 writer khoảng 3 ms do đó sau khoảng 9 đến 10 chu kỳ thì sẽ bị lẫn lộn.
   Nếu msg_len tăng độ dài chuỗi lên hoặc period giảm xuống thì dữ liệu sẽ dài hơn và ghi nhiều hơn từ đó dẫn đến sẽ tăng nguy cơ lẫn lộn.
2. Dự đoán thứ tự **độ trễ in** (latency) từ thấp đến cao giữa mutex / critical
   section / gatekeeper. Vì sao?
   Dự đoán latency từ thấp đến cao là critical section, mutex, gatekeeper.
Critical section ngăn scheduler/ngắt can thiệp nên message hiện tại được
truyền liên tục. Mutex cho phép task chờ ở trạng thái Blocked nên có thêm
thời gian chờ chủ sở hữu hiện tại. Gatekeeper thêm thời gian enqueue và
thời gian nằm sau các message khác trong queue.
3. Với critical section, task nào trong hệ thống có nguy cơ bị ảnh hưởng xấu
   nhất nếu đoạn in quá dài? (gợi ý: nghĩ đến ngắt, không chỉ task)
   Critical section nguy hiểm nhất đối với các interrupt thời gian thực.
Một task dài có thể bị ảnh hưởng xấu vì task cần thực hiện nhiều deadline ngắn hoặc có chu kỳ chính xác sẽ bị các task dài hơn chèn từ đó dẫn đến k đúng chu kỳ.
Một message dài 32 byte tại 115200 baud giữ critical section khoảng
2.778 ms, tương đương gần ba tick FreeRTOS. SysTick/PendSV và các IRQ có
priority bị mask sẽ bị trì hoãn, làm tăng jitter và có thể gây mất dữ
liệu ở peripheral có yêu cầu phục vụ nhanh.
4. RAM dùng thêm của gatekeeper so với mutex: dự đoán tăng hay giảm, vì sao?
Gatekeeper dùng nhiều RAM hơn mutex vì ngoài queue control block còn có
queue storage `queue_length × 32 byte`, một TCB và stack riêng cho
gatekeeper task. Mutex chỉ cần một kernel object và không cần thêm task.
5. Nếu `WRITER_COUNT` tăng gấp đôi, cách nào trong 3 cách bị ảnh hưởng về độ trễ
   nhiều nhất? Vì sao?
Khi tăng từ 3 lên 6 writer, dự đoán gatekeeper tăng latency end-to-end
nhiều nhất vì tốc độ enqueue tăng gấp đôi trong khi UART không đổi.
Message phải chờ nhiều phần tử hơn trong queue. Mutex và critical
section tạo backpressure trực tiếp lên writer; gatekeeper tách writer
khỏi UART trong thời gian ngắn nên backlog thể hiện rõ thành queue
latency.