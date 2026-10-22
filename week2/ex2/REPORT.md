# Report — Week 2 / Ex2

`student_id`: 2412477    Commit của PREDICTION.md: 

## 1. Lỗi tái hiện (UART_GUARD_MODE = 0)
Log UART dán nguyên văn, đánh dấu vị trí ký tự lẫn lộn:
[W0 seq=001450]AAAAAAAAAAAAAAA
[W1 seq=001449]BBBBBBBBBBBBBBB
[W2 seq=001442]CCCCCCCCCCCCCCC
[W0 seq=001451]AAAAAAAAAAAAAAA
[W1 seq=001450]BBBBBBBBBBBBBBB
[W2 seq=001443]CCCCCCCCCCCCCCC
[W0 seq=001452]AAAAAAAAAAAAAAA
[W1 seq=001451]BBBBB[W2 seq=001BBBBBBBBBB
444]CCCCCC
[W0 seq=0CCCCCCCCC
01453]AAAA[W1 seq=00AAAAAAAAAAA1452]BBBBB
[W2 seq=BBBBBBBBBB
001445]CCC
[W0 seq=0CCCCCCCCCCC01454]AAAAC
[W1 seqAAAAAAAAAAA=001453]BB
[W2 seq=BBBBBBBBBBB001446]CCCBB
[W0 seCCCCCCCCCCCq=001455]AC
[W1 seqAAAAAAAAAAA=001454]BBAAA
[W2 sBBBBBBBBBBeq=001447]CBBB
[W0 sCCCCCCCCCCCeq=001456]CCC
[W1 sAAAAAAAAAAeq=001455]BAAAAA
[W2BBBBBBBBBBB seq=00144BBB
[W0 s8]CCCCCCCCeq=001457]ACCCCCCC
[AAAAAAAAAAW1 seq=0014AAAA
[W2 56]BBBBBBBseq=001449]BBBBBBBB
CCCCCCCCCCC[W0 seq=00CCCC
1458]AAAAAAAAAAAAAAA
[W1 seq=001457]BBBBBBBBBBBBBBB
[W2 seq=001450]CCCCCCCCCCCCCCC
[W0 seq=001459]AAAAAAAAAAAAAAA
[W1 seq=001458]BBBBBBBBBBBBBBB
[W2 seq=001451]CCCCCCCCCCCCCCC
[W0 seq=001460]AAAAAAAAAAAAAAA
[W1 seq=001459]BBBBBBBBBBBBBBB
[W2 seq=001452]CCCCCCCCCCCCCCC

Tái hiện lại bao nhiêu lần / bao nhiêu lần thấy lỗi: 3 / 3.

## 2. Bảng so sánh 3 cách
| Cách | Độ trễ in trung bình (us) | Độ trễ in max (us) | Ảnh hưởng task khác | RAM thêm (bytes) |
|---|---|---|---|---|
| Mutex | 6731|7225 | có | |
| Critical section | | | | |
| Gatekeeper | | | | |

Cách đo độ trễ in (mô tả ngắn, kèm ảnh logic analyzer nếu dùng pin đánh dấu):

## 3. Vì sao critical section nguy hiểm nhất
Giải thích bằng số đo của bạn: đoạn in dài bao nhiêu us khi ở trong critical
section, và điều đó chặn ngắt nào trong hệ thống bao lâu.

## 4. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.

## 5. Câu hỏi đánh đổi
Với `WRITER_COUNT` của **bạn**, nếu tăng gấp đôi thì cách nào bị ảnh hưởng độ trễ
nhiều nhất? Trả lời bằng số từ dữ liệu của bạn, rồi đo lại để kiểm chứng.
