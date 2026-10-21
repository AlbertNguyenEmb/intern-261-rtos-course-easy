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

2. `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy: 8192 - 640 = 7552 bytes.
   `HEAP_SIZE_BYTES` − số này = ____ . Số đó là của những gì?
số đó chính là số lượng bytes mà lập trình viên cấp cho task động sử dụng. Từ đó tiêu tốn một lượng bộ nhớ của Heap.
3. High water mark dự đoán (words): dyn 96 words , stat 64 words , idle 80 words , timer 72 words .
4. `dyn_task` có stack 160 words = 640 bytes. Mảng `HOG_DEPTH_BYTES` = 640 bytes.
   Hook tràn stack **có** chạy không? Vì sao?
   Hook gây tràn có theer chạy vì mảng đã chiếm khoảng 640 bytes bộ nhớ stack nhưng task còn cần các RAM stack cho thanh ghi context, địa chỉ trả về, biến vòng lặp,...
5. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì:
Vì .map sinh ra để biết bộ nhớ ban đầu khi được compile là bao nhiêu nhưng sau khi chúng ta cấp phát một số task dyn thì sẽ bị chiếm dụng bộ nhớ trong heap từ đó khi gọi xPortGetFreeHeapSize() tại thời điểm chương trình thực thi còn lại bao nhiêu từ đó dẫn đến không bằng nhau.
6. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: 128 words. Dựa vào đâu?
dyn_task thông thường chỉ thực hiện các thao tác đơn giản, ít biến cục bộ, nên dự đoán mức sử dụng Stack khoảng 80 words.
Dự phòng thêm 32 words cho lời gọi hàm và biến động Stack.
