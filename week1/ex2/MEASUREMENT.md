# Week 1 / Ex2 — Checklist thí nghiệm

## Kết nối

| Task | STM32 | Logic analyzer |
|---|---|---|
| A | PA0 | D0 |
| B | PA5 | D1 |
| C | PA6 | D2 |
| GND | GND | GND |

PulseView: sample rate 5 MHz hoặc cao hơn, bật đồng thời D0/D1/D2.

## Các firmware

| Lượt | Cấu hình VS Code | Thời gian capture | File ảnh |
|---|---|---:|---|
| B2 | `Week 1 Ex2 - B2 Starvation` | 20 ms | `evidence/b2_starvation.png` |
| B3 | `Week 1 Ex2 - B3 Cooperative` | 20 ms | `evidence/b3_cooperative.png` |
| B4 | `Week 1 Ex2 - B4 Round Robin` | 10 ms | `evidence/b4_round_robin.png` |
| B5 | `Week 1 Ex2 - B5 Fixed` | 20 ms | `evidence/b5_fixed.png` |
| C×3 | `Week 1 Ex2 - B2 C Work x3` | 20 ms | `evidence/b2_c_work_x3.png` |

Mỗi lượt: chọn cấu hình, F5, F5 lần nữa nếu dừng ở `main`, rồi Run PulseView.
Không đặt breakpoint trong task khi đang đo.

## Số phải ghi

- B2/B3: kênh nào chạy và kênh nào phẳng.
- B4: ít nhất hai mốc đổi task và khoảng cách time slice bằng cursor.
- B5: tổng độ rộng HIGH của từng kênh trong cùng một cửa sổ đo.
- C×3: CPU share và độ rộng pulse C để so với B2.

CPU share trong cửa sổ `T`:

`share_i = 100 × CPU_time_i / T`.

Ở B4, pin của task bị preempt có thể còn HIGH nên không cộng trực tiếp duty
cycle của ba kênh. Dùng các mốc task mới bắt đầu chạy để chia time slice.
