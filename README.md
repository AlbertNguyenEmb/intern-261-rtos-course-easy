# FreeRTOS trên STM32F103 — Khóa tự học 4 tuần (Bản tiêu chuẩn)

> IoT Lab · Dành cho sinh viên năm 1–2 đã biết C cơ bản · ~11 giờ/tuần · tổng ~44 giờ

Khóa học giúp bạn nắm vững kernel FreeRTOS **ở mức API gốc** và chứng minh mọi kết luận bằng
**số đo trên phần cứng thật**. Mỗi bạn nhận một bộ tham số riêng, nên bài của hai người không bao giờ giống nhau.

**Kết quả cuối khóa:** tự thiết kế, hiện thực và chứng minh bằng số đo một hệ thống đa task có deadline cứng.

---

## 1. Chuẩn bị

### Phần cứng
| Thiết bị | Ghi chú |
|---|---|
| STM32F103C8T6 (Blue Pill) | 72 MHz, 20 KB RAM, 64 KB Flash. Chip clone (CKS32/CS32) có thể khiến ST-Link báo sai ID |
| ST-Link V2 | Nạp và debug qua SWD |
| USB-UART (CH340 / CP2102) | Nối USART1 (PA9 TX, PA10 RX) |
| Logic analyzer 8 kênh 24 MHz | Dùng với PulseView |
| MPU6050, LED, 2 nút nhấn, breadboard | |

### Phần mềm
- STM32CubeMX (xuất project với **Toolchain = CMake**)
- `arm-none-eabi-gcc`, CMake ≥ 3.20, Ninja
- VS Code + STM32 extension, hoặc dòng lệnh
- PulseView, Python 3

### Lấy mã nguồn
```bash
git clone --recurse-submodules <repo-url>
cd <repo>
python3 tools/gen_params.py <MSSV>      # sinh bộ tham số riêng của bạn
```
Lệnh `gen_params.py` tạo `weekN/exM/app/params.h` và `weekN/exM/PARAMS.md`. Mọi con số trong bài
(chu kỳ, priority, kích thước queue, deadline...) đều lấy từ đây. **Dùng sai MSSV = bài không được chấm.**

### Build và nạp
```bash
cmake --preset Debug
cmake --build --preset Debug
```

---

## 2. Quy tắc bắt buộc

| Quy tắc | Chi tiết |
|---|---|
| Chỉ dùng API gốc FreeRTOS | `task.h`, `queue.h`, `semphr.h`, `event_groups.h`, `timers.h`, `stream_buffer.h` |
| **Cấm CMSIS-RTOS** | Không `cmsis_os.h` / `cmsis_os2.h`, không bất kỳ hàm `os*` nào (`osThreadNew`, `osDelay`...) |
| CubeMX | Được dùng để sinh code HAL, nhưng **không** bật Middleware › FREERTOS (sẽ sinh wrapper CMSIS-RTOS) |
| Build | Chỉ CMake + Ninja. Không dùng Makefile |
| Cấu hình | HAL timebase = TIM4, `NVIC_PRIORITYGROUP_4`, bật `configASSERT` |
| Dùng AI | Được phép. Ghi lại prompt đã dùng vào `AI_LOG.md` và phải tự giải thích được mọi dòng code khi vấn đáp |

Trước khi nộp, chạy:
```bash
./tools/check_api.sh        # phải in ra PASS
```

---

## 3. Quy trình làm mỗi bài

1. Đọc `README.md` của bài và tài liệu được chỉ định.
2. Điền `PREDICTION.md` và **commit trước khi nạp chương trình**.
3. Viết code trong `app/`.
4. Đo trên board bằng logic analyzer / UART.
5. Điền `REPORT.md`: số đo, ảnh chụp, giải thích chỗ lệch so với dự đoán.
6. Vấn đáp với mentor.

---

## 4. Lộ trình

| Tuần | Bài | Chủ đề |
|---|---|---|
| **1** | ex1 | Bring-up: tích hợp FreeRTOS thủ công vào project CubeMX (CMake) |
| | ex2 | Task, priority, preemption, starvation |
| | ex3 | Timing & jitter: `vTaskDelay` vs `xTaskDelayUntil`, tick 100 / 1000 Hz |
| | ex4 | Ngân sách bộ nhớ 20 KB RAM: stack HWM, heap, overflow hook, cấp phát tĩnh |
| **2** | ex1 | Queue: pipeline MPU6050 → lọc → UART |
| | ex2 | Race condition: mutex vs critical section vs gatekeeper task |
| | ex3 | Priority inversion: tái hiện và sửa |
| | ex4 | Event group & task notification |
| **3** | ex1 | Ngắt → task, NVIC priority, `FromISR`, latency |
| | ex2 | Software timer & debounce |
| | ex3 | UART RX → stream buffer → bộ phân tích lệnh |
| | ex4 | Debug: CPU load, deadlock, watchdog |
| **4** | ex1–4 | Mini project: data logger MPU6050 có deadline cứng, design doc, đo đạc, bảo vệ |

Chi tiết từng bài (nội dung, API bắt buộc, output, tiêu chí đạt): xem [`SYLLABUS.md`](SYLLABUS.md).

---

## 5. Chấm điểm

| Thành phần (mỗi bài) | Trọng số |
|---|---|
| Code | 30% |
| Số đo + báo cáo | 40% |
| Vấn đáp | 30% |

| Tuần | 1 | 2 | 3 | 4 |
|---|---|---|---|---|
| Trọng số | 20% | 25% | 25% | 30% |

- **Đạt:** tổng ≥ 6.5/10 và không tuần nào dưới 5.
- **Điểm liệt:** dùng CMSIS-OS → phần code của bài đó = 0. Không có `AI_LOG.md` hoặc không giải thích được code mình nộp → phần vấn đáp = 0.
- Báo cáo không có số đo thật → 0 điểm phần đo.

---

## 6. Cấu trúc repo

```
.
├── README.md
├── SYLLABUS.md              # nội dung chi tiết 16 bài
├── template/                # project CubeMX (CMake) gốc, chưa có FreeRTOS
├── third_party/FreeRTOS-Kernel/
├── tools/
│   ├── gen_params.py        # sinh tham số theo MSSV
│   └── check_api.sh         # kiểm tra trước khi nộp
└── weekN/exM/
    ├── README.md            # đề bài
    ├── app/                 # code của bạn
    ├── PREDICTION.md        # dự đoán (commit trước khi chạy)
    ├── REPORT.md            # kết quả đo + phân tích
    └── RUBRIC.md            # thang điểm
```

## 7. Tài liệu

- *Mastering the FreeRTOS Real Time Kernel* (miễn phí tại freertos.org) + FreeRTOS API Reference
- RM0008 – Reference manual STM32F1 · PM0056 – Cortex-M3 programming manual
- freertos.org: *RTOS for ARM Cortex-M* (giải thích `configMAX_SYSCALL_INTERRUPT_PRIORITY`)
- Series video *Introduction to RTOS* của Digi-Key

## 8. Dùng MCU khác?

Được, nhưng bạn phải tự port và vẫn phải dùng đúng API gốc FreeRTOS. Mentor chỉ hỗ trợ STM32F1.

I hope that my project will be completed 
                    ---HCM 9/10/2026---