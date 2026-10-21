# Dự đoán — Week 1 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    `HEAP_SIZE_BYTES` = 8192    `DYN_TASK_STACK_WORDS` = 160
`STATIC_TASK_STACK_WORDS` = 128    `HOG_DEPTH_BYTES` = 640   `TARGET_FREE_HEAP_BYTES` = 4096

1. Ngân sách RAM dự đoán (tổng phải = 20480 bytes):

| Mục | Bytes dự đoán |
|---|---|
| `.data` | 20 |
| `.bss` (không kể heap FreeRTOS) | 2200|
| Heap FreeRTOS (`ucHeap`) | 8192|
| Stack của main (MSP) | 1024 |
| Còn lại / chưa dùng | 9044|
| **Tổng** | **20480** |

2. Giả sử `ram_report_task` dùng stack 320 words, còn Idle/Timer task dùng
   cấp phát tĩnh, `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy dự đoán
   khoảng 6000 bytes. `8192 - 6000 = 2192 bytes` là stack và TCB của
   `dyn_task`, stack và TCB của `ram_report_task`, cộng metadata/alignment của
   `heap_4`. `stat_task`, Idle và Timer không lấy từ heap vì được tạo tĩnh.
3. High water mark dự đoán (words): dyn 96 words , stat 64 words , idle 80 words , timer 72 words .
4. `dyn_task` có stack 160 words = 640 bytes. Mảng `HOG_DEPTH_BYTES` = 640 bytes.
   Hook tràn stack **có** chạy không? Vì sao?
   Hook gây tràn có thể chạy vì mảng đã chiếm đủ 640 bytes stack nhưng task
   còn cần stack cho thanh ghi context, địa chỉ trả về và biến vòng lặp.
5. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì:
Vì .map sinh ra để biết bộ nhớ ban đầu khi được compile là bao nhiêu nhưng sau khi chúng ta cấp phát một số task dyn thì sẽ bị chiếm dụng bộ nhớ trong heap từ đó khi gọi xPortGetFreeHeapSize() tại thời điểm chương trình thực thi còn lại bao nhiêu từ đó dẫn đến không bằng nhau.
6. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: 96 words. Dựa vào đâu?
Với HWM dự đoán 96 trên stack 160 words, mức dùng cực đại dự đoán là
`160 - 96 = 64 words`. Cộng biên an toàn 32 words được
`64 + 32 = 96 words`. Giá trị cuối phải tính lại từ HWM đo thật.
