from pathlib import Path
import struct

import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent
QUEUE_LENGTH = 4


def load_run(prefix):
    time_bytes = (ROOT / f"{prefix}_time.bin").read_bytes()
    raw_bytes = (ROOT / f"{prefix}_raw.bin").read_bytes()
    filtered_bytes = (
        ROOT / f"{prefix}_filtered.bin"
    ).read_bytes()

    if len(time_bytes) % 2 != 0:
        raise ValueError("Invalid time data length")

    sample_count = len(time_bytes) // 2

    time_ms = list(
        struct.unpack(
            f"<{sample_count}H",
            time_bytes
        )
    )

    raw = list(raw_bytes[:sample_count])
    filtered = list(filtered_bytes[:sample_count])

    return time_ms, raw, filtered


def make_plot(prefix, output_name, title):
    time_ms, raw, filtered = load_run(prefix)

    plt.figure(figsize=(10, 5))

    plt.step(
        time_ms,
        raw,
        where="post",
        label="queue_raw",
        linewidth=1.5
    )

    plt.step(
        time_ms,
        filtered,
        where="post",
        label="queue_filtered",
        linewidth=1.5
    )

    plt.xlabel("Time (ms)")
    plt.ylabel("Queue depth (items)")
    plt.title(title)

    plt.yticks(range(QUEUE_LENGTH + 1))
    plt.ylim(-0.2, QUEUE_LENGTH + 0.2)
    plt.xlim(time_ms[0], time_ms[-1])

    plt.grid(True, alpha=0.3)
    plt.legend()
    plt.tight_layout()

    output_path = ROOT / output_name
    plt.savefig(output_path, dpi=180)
    plt.close()

    print(f"Created: {output_path}")
    print(f"Maximum queue_raw depth: {max(raw)}")
    print(
        "Maximum queue_filtered depth: "
        f"{max(filtered)}"
    )


make_plot(
    "normal",
    "queue_fill_normal.png",
    "Queue depth - UART 230400 baud"
)

make_plot(
    "slow",
    "queue_fill_slow_uart.png",
    "Queue depth - UART 9600 baud"
)