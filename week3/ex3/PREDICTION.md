# Dự đoán — Week 3 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412411

1. Thời gian truyền 1 byte ở `RX_BAUD` của bạn là bao nhiêu us? (8N1, tính cả
   start/stop bit). Trong thời gian đó `parser_task` có kịp xử lý 1 byte từ
   stream buffer không, nếu nó là task ưu tiên cao nhất hệ thống?
   Số bit truyền 1 byte uart là 10 bit do truyền cả start và stop bit. Mà baud rate uart là 115200 nên thời gian truyền khoảng 10/115200 = 86.81us. Tương đương khoảng 6250 chu kỳ => parser_task có thể xử lý kịp thời.
2. `STREAM_BUFFER_BYTES` của bạn chứa tối đa bao nhiêu lệnh dài `MAX_CMD_LEN`
   byte trước khi đầy?
   Stream buffer có độ dài khoảng 96 bytes trong khi độ dài max len khoảng 16 bytes => sb có thể chưa khoảng 96/16 = 6 phần tử
3. Nếu `xStreamBufferSendFromISR` trả về 0 (stream buffer đầy) trong
   `uart_rx_isr_handler`, byte đó sẽ bị mất. Theo bạn, trường hợp nào thực tế
   khiến điều này xảy ra (gửi quá nhanh / `parser_task` ưu tiên quá thấp /
   cái khác)?
   Có nhiều khả năng khiến điều đó xảy ra.
   Một là parser_task bị chiếm quyền quá nhiều do ưu tiên thấp. Trong khoảng thời gian đó tác vụ bị chiếm quyền khiến parser_task không vào kịp trạng thái để xử lý buffer. Hoặc là là một task khác đang giữ mutex hoặc semaphore nên đã bị chặn lại.
   Hai là gửi quá nhanh khi mà gửi 1 số byte quá nhiều tốc độ ghi buffer không kịp xử lý.
   Ba là tác vụ parser_task đọc từng byte trong khi xử lý task đó cần quá nhiều thời gian thay vì gom theo cụm rồi mới xử lý.
4. Dự đoán: gửi lệnh bằng gõ tay bình thường có mất byte không? Gửi bằng
   script gửi liên tục không nghỉ thì sao?
   Gõ tay thì gần như không mất byte còn gửi bằng script thì có.