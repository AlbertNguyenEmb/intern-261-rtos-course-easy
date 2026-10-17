# Dự đoán — Week 1 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2412477    `PERIOD_MS` = 19    `WORK_JITTER_US` = 600    `SAMPLE_COUNT` = 500

1. Bảng dự đoán (điền trước khi đo):

| # | Delay | Tick | Chu kỳ tb (ms) | Jitter (us) | Drift tổng sau SAMPLE_COUNT (ms) |
|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 | 19 | >0 | 150 |
| 2 | `xTaskDelayUntil` | 1000 | 19 |0 | 0 |
| 3 | `vTaskDelay` | 100 | 10 | 6 | -4500 |
| 4 | `xTaskDelayUntil` | 100 | 10 |0 |-4500 |

2. Cấu hình nào có drift lớn nhất? Vì sao, viết bằng công thức có `WORK_JITTER_US`.
Cấu hình vTaskDelay với Tick = 1000Hz là có drift cao nhất 
$$ D_N\approx N\frac{\text{WORK\_JITTER\_US}}{2\times1000} $$

$$ D_{100}\approx100\times\frac{15000}{2000}=750\text{ ms} $$

3. `PERIOD_MS` = 37 ms . Với tick 100 Hz, một tick = 1ms ms, nên chu kỳ thực tế sẽ
   bị làm tròn thành 30 ms. Sai số hệ thống: ____ %.
   Với tick rate 100 Hz:

[
T_{\mathrm{tick}}=\frac{1000}{100}=10\text{ ms}
]

Số tick chuyển đổi:

[
N=\left\lfloor\frac{37}{10}\right\rfloor=3
]

Chu kỳ theo tick trở thành 30 ms.

Sai số hệ thống:

[
e=\frac{30-37}{37}\times100%\approx-18.92%
]

Độ lớn sai số: 18,92%.
4. Jitter ở tick 1000 Hz so với 100 Hz: lớn hơn / nhỏ hơn / bằng? Vì sao?
Jitter ở tick 1000 Hz nhỏ hơn Tick của 100 Hz bởi vì 1000 Hz cung cấp độ phân giải nhỏ hơn từ đó nếu Task có vượt phá một lượng chỉ cần chia hết 1ms thì sẽ phát hiện và nhận ra
còn ở vTaskDelayUntil() thì jitter gần như = 0;
5. Nếu task này **không** phải ưu tiên cao nhất, đại lượng nào trong bảng bị ảnh hưởng
   nhiều nhất? ______
Jitter dự kiến là đại lượng nhạy nhất với sự xuất hiện không đều của task ưu tiên cao hơn.

Khi bị preempt, task periodic phải chờ CPU, khiến một số chu kỳ dài hơn bình thường. Điều này có thể làm tăng (T_{\max}) và jitter.
