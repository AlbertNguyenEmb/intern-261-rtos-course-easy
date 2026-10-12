# Report — Week 1 / Ex1

`student_id`: 2412477    Commit của PREDICTION.md: commit 041152959f65e3d708358d8cb8565bbb5fcf1bb7

## 1. Số đo
| Đại lượng | Giá trị |
|---|---|
| `BLINK_PERIOD_MS` (từ params.h) | |
| Chu kỳ đo được — min | |
| Chu kỳ đo được — max | |
| Chu kỳ đo được — trung bình | |
| Sai số trung bình vs `BLINK_PERIOD_MS` | |
| `uxTaskGetStackHighWaterMark()` của bringup_task | words |

Ảnh PulseView (>= 10 chu kỳ, đọc được giá trị): `![](do_ex1.png)`

## 2. Ba handler của kernel
| Tên trong port ARM_CM3 | Tên trong vector table | Nhiệm vụ |
|---|---|---|
| `vPortSVCHandler` | | |
| `xPortPendSVHandler` | | |
| `xPortSysTickHandler` | | |

Log lỗi link trước khi sửa (dán nguyên văn):

```
```

Cách sửa:

## 3. Tại sao HAL timebase phải sang TIM4
Phải trả lời được: `HAL_IncTick()` chạy ở đâu, `configKERNEL_INTERRUPT_PRIORITY`
ảnh hưởng gì đến nó, và `HAL_Delay()` / `vTaskDelay()` khác nhau chỗ nào.

## 4. Lệch so với dự đoán
Với mỗi mục trong `PREDICTION.md` lệch so đo: lệch bao nhiêu, và **nguyên nhân vật lý**
(không được trả lời "do sai số").

## 5. Câu hỏi đánh đổi
`configTICK_RATE_HZ` = 1000 trong bài này. Dựa vào số đo của **bạn**, nếu hạ xuống
100 Hz thì chu kỳ LED và sai số thay đổi thế nào? Trả lời bằng con số, không bằng lý thuyết.
