# Dự đoán — Week 1 / Ex1

> **Commit file này TRƯỚC khi nạp chương trình lần đầu.** Report sẽ phải giải thích
> mọi chỗ dự đoán lệch so đo. Dự đoán sai không bị trừ điểm; dự đoán sau khi đo thì bị.

`student_id`: 2412477    Ngày/giờ commit: 7:16 12/10/2026

1. Chu kỳ LED tôi sẽ đo được: 298 ms (`BLINK_PERIOD_MS` = 149)
2. Sai số giữa chu kỳ đo được và `BLINK_PERIOD_MS`:  us. Vì sao?
Sai số là ở khoảng 1 tick hoặc hơn tùy thời gian thực thi 
3. Nếu để HAL timebase ở lại SysTick, điều gì sẽ xảy ra khi `vTaskDelay()` được gọi?
   (chọn và giải thích: chạy bình thường / treo / tick sai nhịp / hard fault)
   tick sai nhịp hoặc hệ thống bị treo 
   Vì HAL và FreeRTOS sử dụng Systick chung thì có thể chúng có thể cấu hình thời gian can thiệp lẫn nhau
   dẫn đến vTaskDelay() không nhận giá trị tick đúng
4. `BRINGUP_STACK_WORDS` = 128 words = 128 x 4 bytes. Task này có đủ không? Vì sao?
Dự đoán đủ cho task đơn giản chỉ toggle GPIO và delay, nhưng cần kiểm chứng bằng stack high-water mark.
5. Trước khi map handler, linker sẽ báo lỗi gì (đoạn nguyên văn thông báo):
Em k biết sao nó k báo lỗi nữa anh Triều anh Huy ơi huhu có comment đoạn map rồi mà nó k lỗi 