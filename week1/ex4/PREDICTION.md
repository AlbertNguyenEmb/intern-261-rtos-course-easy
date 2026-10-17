# Dự đoán — Week 1 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    `HEAP_SIZE_BYTES` = 8192 bytes    `DYN_TASK_STACK_WORDS` =  160 words
`STATIC_TASK_STACK_WORDS` = 128 words   `HOG_DEPTH_BYTES` = 640 bytes    `TARGET_FREE_HEAP_BYTES` = 4096 bytes

1. Ngân sách RAM dự đoán (tổng phải = 20480 bytes):

| Mục | Bytes dự đoán |
|---|---|
| `.data` | 20|
| `.bss` (không kể heap FreeRTOS) |2200 |
| Heap FreeRTOS (`ucHeap`) | 8192|
| Stack của main (MSP) | 1024|
| Còn lại / chưa dùng | 9044|
| **Tổng** | **20480** |

2. `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy: khoảng 6000 bytes.
   `HEAP_SIZE_BYTES` − số này = khoảng 2192 bytes. Số đó là của những gì?
   Phần heap đã dùng gồm stack và TCB của `dyn_task`,
`ram_report_task`, timer queue và metadata của `heap_4`.
3. High water mark dự đoán (words): dyn 90, stat 58, idle 90, timer 80.
| Task | HWM dự đoán |
|---|---:|
| `dyn_task` | khoảng 90 words |
| `stat_task` | khoảng 58 words |
| Idle | khoảng 90 words |
| Timer | khoảng 80 words |

Hai task làm cùng công việc nên lượng stack thực dùng gần giống nhau.
Do stack ban đầu khác nhau 32 words, HWM dự kiến cũng lệch khoảng 32 words.
4. `dyn_task` có stack 160 words = 640 bytes. Mảng `HOG_DEPTH_BYTES` = 640 bytes.
   Hook tràn stack **có** chạy không? Vì sao?
   `dyn_task` có 160 words = 640 bytes. Mảng cục bộ có kích thước 640 bytes.
Hook tràn stack được dự đoán sẽ chạy vì mảng đã chiếm toàn bộ stack, chưa kể
context switch, địa chỉ trả về và các biến cục bộ khác.

5. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì:
Trong `.map`, `ucHeap` luôn chiếm đủ 8192 bytes trong `.bss`.
`xPortGetFreeHeapSize()` chỉ trả về phần chưa được cấp phát bên trong `ucHeap`.
Chênh lệch gồm stack động, TCB động, timer queue và metadata của `heap_4`.

6. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: 96 words. Dựa vào đâu?
Dự đoán stack nhỏ nhất an toàn của `dyn_task` là 96 words.
Giá trị cuối cùng sẽ được chọn từ stack thực dùng cộng biên an toàn ít nhất
32 words.
