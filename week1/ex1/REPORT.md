# Report — Week 1 / Ex1

`student_id`: 2412477    Commit của PREDICTION.md: commit 041152959f65e3d708358d8cb8565bbb5fcf1bb7

## 1. Số đo
| Đại lượng | Giá trị |
|---|---|
| `BLINK_PERIOD_MS` (từ params.h) | 149ms|
| Chu kỳ đo được — min | 148.95 ms|
| Chu kỳ đo được — max | 149.00 ms|
| Chu kỳ đo được — trung bình |148.9525 ms |
| Sai số trung bình vs `BLINK_PERIOD_MS` | 0.03185%|
| `uxTaskGetStackHighWaterMark()` của bringup_task | 109 words |

Ảnh PulseView (>= 10 chu kỳ, đọc được giá trị): `![](do_ex1.png)`

## 2. Ba handler của kernel
| Tên trong port ARM_CM3 | Tên trong vector table | Nhiệm vụ |
|---|---|---|
| `vPortSVCHandler` | SVC_Handler| Khởi động task đầu tiên khi scheduler bắt đầu, lưu context hiện tại của task |
| `xPortPendSVHandler` |PendSV_Handler | Thực hiện contextswitch lưu context hiện tại và khôi phục context task tiếp theo |
| `xPortSysTickHandler` |SysTick_Handler | Tăng tick của FreeRTOS và yêu cầu chuyển task nếu có task ưu tiên cao hơn sẵn sàng chạy.|

Log lỗi link trước khi sửa (dán nguyên văn):

C:/PROGRA~2/ARMGNU~1/14EFD8~1.2RE/bin/../lib/gcc/arm-none-eabi/14.2.1/../../../../arm-none-eabi/bin/ld.exe: third_party/FreeRTOS-Kernel/libfreertos_kernel.a(port.c.obj): in function `SVC_Handler':
D:/ml_iot_lab_intern/intern-261-rtos-course-easy/week1/ex1/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:230: multiple definition of `SVC_Handler'; CMakeFiles/week1_ex1.dir/Core/Src/stm32f1xx_it.c.obj:D:/ml_iot_lab_intern/intern-261-rtos-course-easy/week1/ex1/Core/Src/stm32f1xx_it.c:146: first defined here
C:/PROGRA~2/ARMGNU~1/14EFD8~1.2RE/bin/../lib/gcc/arm-none-eabi/14.2.1/../../../../arm-none-eabi/bin/ld.exe: third_party/FreeRTOS-Kernel/libfreertos_kernel.a(port.c.obj): in function `PendSV_Handler':
D:/ml_iot_lab_intern/intern-261-rtos-course-easy/week1/ex1/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c:459: multiple definition of `PendSV_Handler'; CMakeFiles/week1_ex1.dir/Core/Src/stm32f1xx_it.c.obj:D:/ml_iot_lab_intern/intern-261-rtos-course-easy/week1/ex1/Core/Src/stm32f1xx_it.c:172: first defined here
C:/PROGRA~2/ARMGNU~1/14EFD8~1.2RE/bin/../lib/gcc/arm-none-eabi/14.2.1/../../../../arm-none-eabi/bin/ld.exe: third_party/FreeRTOS-Kernel/libfreertos_kernel.a(port.c.obj): in function `SysTick_Handler':

Cách sửa:
xóa 3 khối code định nghĩa 3 hàm tương ứng đi trong khối stm32f1xx_it.c
## 3. Tại sao HAL timebase phải sang TIM4
Phải trả lời được: `HAL_IncTick()` chạy ở đâu, `configKERNEL_INTERRUPT_PRIORITY`
ảnh hưởng gì đến nó, và `HAL_Delay()` / `vTaskDelay()` khác nhau chỗ nào.
HAL_IncTick() thường được gọi và chạy trong hàm SysTick_Handler, thường sử dụng biến tăng thời gian uwTick.
Nhưng khi tích hợp freeRTOS thì freeRTOS sử dụng để tạo kernel phục vụ cho bộ lập lịch. Từ đó dẫn tới HAL_IncTick phải được gọi trong hàm callback của TIM4
configKERNEL_INTERRUPT_PRIORITY xác định mới ưu tiên ngắt của kernel bằng cách sử dụng ngắt PendSV
Vì thế khi sử dụng SysTick thì có thể thể bị trì hoãn khi CPU đang thực hiện ISR, đông thời HAL cũng phụ thuộc vào SysTick bộ đếm thời gian HAL cũng sẽ bị ảnh hưởng bởi cách RTOS quản lý task.  
2 Hàm đó khác nhau ở nhiều điểm:  
Một là, sử dụng khác nhau bởi đơn vị đếm thời gian 1 bên là uwTick 1 bên là FreeRTOS tick
Hai là, khi HAL_Delay CPU bị đứng hoàn toàn và không thể chạy task tiếp theo còn đối với vTaskDelay thì có thể chạy task khác trong khi task hiện tại đang bị treo.  
## 4. Lệch so với dự đoán
Với mỗi mục trong `PREDICTION.md` lệch so đo: lệch bao nhiêu, và **nguyên nhân vật lý**
(không được trả lời "do sai số").
Sai số khoảng 1 tick hoặc hơn.
Nguyên nhân vật lý có thể gồm chênh lệch tần số thực tế giữa clock của STM32 và clock lấy mẫu của logic analyzer, độ phân giải lấy mẫu 50 µs, thời điểm ngắt SysTick và độ trễ khi scheduler đánh thức task.
## 5. Câu hỏi đánh đổi
`configTICK_RATE_HZ` = 1000 trong bài này. Dựa vào số đo của **bạn**, nếu hạ xuống
100 Hz thì chu kỳ LED và sai số thay đổi thế nào? Trả lời bằng con số, không bằng lý thuyết.
Khi configTICK_RATE_HZ = 1000, chu kỳ đảo trạng thái LED đo được trung bình là 148.9525 ms, sai lệch 0.0475 ms so với giá trị cấu hình 149 ms, tương ứng sai số 0.03185%.  

Nếu giảm configTICK_RATE_HZ xuống 100 mà giữ nguyên vTaskDelay(pdMS_TO_TICKS(149)), số tick delay sẽ giảm từ 149 xuống 14. Khoảng đảo trạng thái LED dự kiến giảm xuống khoảng 140 ms và chu kỳ HIGH–LOW đầy đủ còn khoảng 280 ms. Sai số so với mục tiêu 149 ms dự kiến tăng lên khoảng 6.04%, chủ yếu do phép chuyển đổi mili giây sang tick bị làm tròn xuống.