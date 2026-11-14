# Report — Week 3 / Ex4

`student_id`: ______    Commit của PREDICTION.md: ______

## 1. Bảng CPU %
| Task | CPU % (lần đo 1) | CPU % (lần đo 2) |
|---|---|---|
| stats |<1% |2% |
| IDLE |99% | 97%|
| beta |<1% | <1%|
| alpha | <1%| <1%|

Giải thích Idle task chiếm bao nhiêu % và vì sao.
Idle task chiếm phần trăm cao bởi vì alpha và beta đã bị chiếm mutex lẫn nhau và bị log, còn stats task sẽ tăng dần phần trăm sau cửa sổ window khi lấy idwg.
## 2. Bằng chứng tìm deadlock
Log/ảnh `uxTaskGetSystemState` **trước khi sửa**: trạng thái từng task, chỉ
rõ `task_alpha`/`task_beta` đang `eBlocked` và đang chờ mutex nào. Giải thích
chính xác thứ tự lấy mutex nào gây ra deadlock (AB-BA).
Log state:
STATE tick=7852
Task	State	Priority
stats	eRunning	1
IDLE	eReady	    0
beta	eBlocked	2
alpha	eBlocked	2
task_alpha lấy mutex A còn task_beta lấy mutex B sau đó B cần mutex A nhưng không có mà còn task_alpha lại cần mutex A => gây ra deadlock.
## 3. Sửa lỗi
Mô tả thay đổi bạn làm để hết deadlock (thứ tự lấy mutex thống nhất). Xác
nhận chạy ổn định sau khi sửa (thời gian chạy thử).
Để làm hết deadlock ta chỉnh thứ tự thay đổi lại cả 2 task cùng lấy theo thứ tự A B và trả theo thứ tự B A. Sau khi nạp và in uart ta nhận thấy cả 2 hoàn thành chu kỳ và tăng cycle.
## 4. Watchdog
Phép tính reload IWDG. Log/video board tự reset khi deadlock được tái tạo lại
có chủ đích, kèm thời gian từ lúc treo đến lúc reset — so với
`IWDG_TIMEOUT_MS`.
CPU tick=505
Task	Cycles	CPU%
stats          	335344		<1%
IDLE           	35795642		98%
beta           	91103		<1%
alpha          	65171		<1%
supervisor     	7174		<1%

STATE tick=520
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=22 beta=21
WDG feeds=0 misses=0 last_feed_tick=0

CPU tick=1040
Task	Cycles	CPU%
stats          	2853860		3%
IDLE           	71631459		95%
alpha          	129701		<1%
beta           	187851		<1%
supervisor     	8995		<1%

STATE tick=1055
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=43 beta=43
WDG feeds=1 misses=0 last_feed_tick=539

CPU tick=1575
Task	Cycles	CPU%
stats          	5399459		4%
IDLE           	107449546		94%
alpha          	192507		<1%
beta           	280097		<1%
supervisor     	10257		<1%

STATE tick=1591
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=64 beta=64
WDG feeds=2 misses=0 last_feed_tick=1074

CPU tick=2110
Task	Cycles	CPU%
stats          	7960438		5%
IDLE           	143247015		94%
beta           	374985		<1%
alpha          	258060		<1%
supervisor     	11481		<1%

STATE tick=2126
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=86 beta=86
WDG feeds=3 misses=0 last_feed_tick=1609

CPU tick=2645
Task	Cycles	CPU%
stats          	10527328		5%
IDLE           	179042060		94%
alpha          	320532		<1%
beta           	469313		<1%
supervisor     	12633		<1%

STATE tick=2661
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=107 beta=107
WDG feeds=4 misses=0 last_feed_tick=2144

CPU tick=3180
Task	Cycles	CPU%
stats          	13110273		5%
IDLE           	214822245		93%
beta           	561559		<1%
alpha          	384253		<1%
supervisor     	13674		<1%

STATE tick=3196
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=129 beta=128
WDG feeds=5 misses=0 last_feed_tick=2679

CPU tick=3716
Task	Cycles	CPU%
stats          	15691995		5%
IDLE           	250669843		93%
alpha          	448783		<1%
beta           	658419		<1%
supervisor     	14826		<1%

STATE tick=3732
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=150 beta=150
WDG feeds=6 misses=0 last_feed_tick=3215

CPU tick=4251
Task	Cycles	CPU%
stats          	18273439		5%
IDLE           	286452263		93%
beta           	750665		<1%
alpha          	512501		<1%
supervisor     	15978		<1%

STATE tick=4267
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=171 beta=171
WDG feeds=7 misses=0 last_feed_tick=3750

CPU tick=4787
Task	Cycles	CPU%
stats          	20849982		6%
IDLE           	322304241		93%
alpha          	577142		<1%
beta           	847525		<1%
supervisor     	17202		<1%

STATE tick=4803
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=193 beta=193
WDG feeds=8 misses=0 last_feed_tick=4285

CPU tick=5322
Task	Cycles	CPU%
stats          	23433074		6%
IDLE           	358084608		93%
alpha          	639948		<1%
beta           	939771		<1%
supervisor     	18465		<1%

STATE tick=5338
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=214 beta=214
WDG feeds=9 misses=0 last_feed_tick=4821

CPU tick=5857
Task	Cycles	CPU%
stats          	26010899		6%
IDLE           	393865304		93%
beta           	1034547		<1%
alpha          	705723		<1%
supervisor     	19506		<1%

STATE tick=5873
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=236 beta=236
WDG feeds=10 misses=0 last_feed_tick=5356

CPU tick=6393
Task	Cycles	CPU%
stats          	28605212		6%
IDLE           	429704814		93%
alpha          	768306		<1%
beta           	1128764		<1%
supervisor     	20770		<1%

STATE tick=6409
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=257 beta=257
WDG feeds=11 misses=0 last_feed_tick=5892

CPU tick=6929
Task	Cycles	CPU%
stats          	31202775		6%
IDLE           	465542019		93%
beta           	1221010		<1%
alpha          	832140		<1%
supervisor     	21922		<1%

STATE tick=6945
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=279 beta=278
WDG feeds=12 misses=0 last_feed_tick=6428

CPU tick=7465
Task	Cycles	CPU%
stats          	33792505		6%
IDLE           	501381859		93%
alpha          	896781		<1%
beta           	1317760		<1%
supervisor     	23074		<1%

STATE tick=7481
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=300 beta=300
WDG feeds=13 misses=0 last_feed_tick=6964

CPU tick=8001
Task	Cycles	CPU%
stats          	36387807		6%
IDLE           	537222308		93%
beta           	1410006		<1%
alpha          	960499		<1%
supervisor     	24226		<1%

STATE tick=8017
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=321 beta=321
WDG feeds=14 misses=0 last_feed_tick=7500

CPU tick=8537
Task	Cycles	CPU%
stats          	38977466		6%
IDLE           	573061240		93%
alpha          	1025140		<1%
beta           	1506866		<1%
supervisor     	25267		<1%

STATE tick=8553
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=343 beta=343
WDG feeds=15 misses=0 last_feed_tick=8036

CPU tick=9073
Task	Cycles	CPU%
stats          	41578720		6%
IDLE           	608895558		93%
alpha          	1087946		<1%
beta           	1599112		<1%
supervisor     	26530		<1%

STATE tick=9089
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=364 beta=364
WDG feeds=16 misses=0 last_feed_tick=8572

CPU tick=9609
Task	Cycles	CPU%
stats          	44175716		6%
IDLE           	644729083		93%
beta           	1693888		<1%
alpha          	1153721		<1%
supervisor     	27571		<1%

STATE tick=9625
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
beta	eBlocked	2
alpha	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=386 beta=386
WDG feeds=17 misses=0 last_feed_tick=9108

CPU tick=10145
Task	Cycles	CPU%
stats          	46776801		6%
IDLE           	680561821		93%
alpha          	1216194		<1%
beta           	1788215		<1%
supervisor     	28835		<1%

STATE tick=10161
Task	State	Priority
stats	eRunning	1
IDLE	eReady	0
alpha	eBlocked	2
beta	eBlocked	2
supervisor	eBlocked	3
PROGRESS alpha=407 beta=407
WDG feeds=18 misses=0 last_feed_tick=9644

CPU tick=10681
Task	Cycles	CPU%
stats          	49385233		6%
IDLE           	716388519		93%
beta           	1882994		<1%
alpha          	1282085		<1%
supervisor     	29987		<1%

Khi chạy bản đã sửa deadlock, tại tick 10161, alpha và beta đều hoàn thành 407 chu kỳ. Supervisor đã refresh IWDG 18 lần và chưa ghi nhận lần timeout nào. Khoảng cách giữa các lần refresh quan sát được là 535–536 ms.

Khi tái tạo deadlock, tại tick 520, alpha và beta đều ở trạng thái eBlocked, số chu kỳ hoàn thành của cả hai bằng 0. Supervisor chưa refresh watchdog lần nào. Sau đó board tự khởi động lại và log ghi BOOT iwdg_reset=1, xác nhận nguyên nhân reset là IWDG. Hiện tượng lặp lại qua nhiều lần khởi động.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
Chu kỳ thống kê thực tế khoảng 535–536 ms, dài hơn dự đoán 35–36 ms. Nguyên nhân là vTaskDelay(500 ms) cộng thêm thời gian lấy thống kê, định dạng và truyền UART trong mỗi vòng lặp. Task stats chiếm khoảng 6% CPU ở các mẫu cuối log.
## 6. Câu hỏi đánh đổi
Với `SUPERVISOR_BITS` và chu kỳ check-in của bạn, nếu một task thật sự chạy
chậm (không deadlock, chỉ chậm) trong đúng 1 chu kỳ, supervisor có false-trip
(reset oan) không? Trả lời bằng số đo thời gian thực tế của chu kỳ check-in.
Trong log, alpha tạm dừng ở 100 chu kỳ rồi tiếp tục chạy. Đến tick 21955, alpha hoàn thành 855 chu kỳ, beta hoàn thành 879 chu kỳ, supervisor refresh 40 lần và ghi nhận 0 lần timeout. Vì vậy, cấu hình không gây false-trip trong lần thử này.