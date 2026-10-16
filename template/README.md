# STM32F103 board template

Thư mục này chứa phần board-support do STM32CubeMX sinh ra: startup, linker
script, CMSIS, HAL, clock, GPIO và HAL timebase TIM4. Không đặt source ứng dụng
hoặc FreeRTOS kernel trong thư mục này.

Không build trực tiếp từ `template/`. Dùng `CMakeLists.txt` ở root và chọn ứng
dụng bằng `-DEXERCISE=weekN/exM`.
