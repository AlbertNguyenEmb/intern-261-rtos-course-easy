#!/usr/bin/env python3
"""Week 1 / Ex3: Calculate period statistics from PulseView digital CSV.

Input assumption: every data row is ONE consecutive sample acquired at the
sample rate passed with --sample-rate. This script deliberately does not use
CSV's Time column because some PulseView exports record it as all zeros.
The measured period is one gap between consecutive signal transitions.
This is a task period ONLY if firmware toggles the selected pin once per loop.
"""

import argparse
import csv
from pathlib import Path

CONFIGS = (
    ("vTaskDelay", 1000),
    ("xTaskDelayUntil", 1000),
    ("vTaskDelay", 100),
    ("xTaskDelayUntil", 100),
)


def find_edges(csv_path: Path):
    """Return (num_samples, edge_sample_indices, channel) from digital CSV."""
    with csv_path.open("r", newline="", encoding="utf-8-sig") as stream:
        reader = csv.reader(stream)
        header = next(reader, None)
        if not header or len(header) < 2:
            raise ValueError(f"{csv_path}: missing digital channel column")
        # PulseView sometimes exports repeated 'logic' column names.
        # The first digital channel is column 1 regardless of its label.
        signal_idx = header.index("D0") if "D0" in header else 1
        prev = None
        edges = []
        samples = 0
        for row in reader:
            if not row or len(row) <= signal_idx:
                continue
            value = row[signal_idx].strip()
            if value not in ("0", "1"):
                raise ValueError(
                    f"{csv_path}: unexpected signal value {value!r} "
                    f"at data row {samples + 1}"
                )
            if prev is not None and value != prev:
                edges.append(samples)
            prev = value
            samples += 1
    return samples, edges, header[signal_idx]


def measure(csv_path, sample_rate_hz, target_ms, n_periods):
    num_samples, edges, channel = find_edges(csv_path)
    if len(edges) < n_periods + 1:
        raise ValueError(
            f"{csv_path}: need {n_periods + 1} transitions, found {len(edges)}"
        )

    # First n_periods intervals need n_periods + 1 consecutive edges.
    selected = edges[: n_periods + 1]
    delta_samples = [b - a for a, b in zip(selected, selected[1:])]
    dt_ms = [1000.0 * d / sample_rate_hz for d in delta_samples]
    measured_ms = 1000.0 * (selected[-1] - selected[0]) / sample_rate_hz
    return {
        "source": csv_path.name,
        "channel": channel,
        "samples_total": num_samples,
        "edges_total": len(edges),
        "periods_used": n_periods,
        "min_ms": min(dt_ms),
        "max_ms": max(dt_ms),
        "avg_ms": measured_ms / n_periods,
        "jitter_us": (max(dt_ms) - min(dt_ms)) * 1000.0,
        "drift_ms": measured_ms - n_periods * target_ms,
        "gaps_in_samples": delta_samples,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv_files", nargs=4, type=Path,
                        help="CFG1, CFG2, CFG3, CFG4 CSVs in this exact order")
    parser.add_argument("--sample-rate", type=float, default=20000.0,
                        help="digital samples per second (default: 20000)")
    parser.add_argument("--period-ms", type=float, default=37.0,
                        help="desired task period in milliseconds (default: 37)")
    parser.add_argument("--count", type=int, default=100,
                        help="number of consecutive period intervals (default: 100)")
    parser.add_argument("--output", type=Path, default=Path("ex3_statistics.csv"),
                        help="summary CSV destination")
    args = parser.parse_args()
    if args.sample_rate <= 0 or args.period_ms <= 0 or args.count < 1:
        parser.error("sample rate, period and count must all be positive")

    results = []
    for cfg_id, (path, (delay, tick_hz)) in enumerate(
        zip(args.csv_files, CONFIGS), start=1
    ):
        r = measure(path, args.sample_rate, args.period_ms, args.count)
        r.update(cfg=cfg_id, delay=delay, tick_hz=tick_hz)
        results.append(r)

    print("Sample rate: {:.0f} Hz | target: {:.3f} ms | intervals: {}".format(
        args.sample_rate, args.period_ms, args.count
    ))
    print("CFG | Delay           | Tick | Min ms | Max ms | Avg ms  | Jitter us | Drift ms")
    for r in results:
        print("{cfg:>3} | {delay:<15} | {tick_hz:>4} | {min_ms:>6.3f} | "
              "{max_ms:>6.3f} | {avg_ms:>7.4f} | {jitter_us:>9.1f} | "
              "{drift_ms:>9.2f}".format(**r))
        print("      {}: {} edges in {} samples; first gaps: {}".format(
            r["source"], r["edges_total"], r["samples_total"],
            r["gaps_in_samples"][:8]
        ))

    args.output.parent.mkdir(parents=True, exist_ok=True)
    fields = ["cfg", "source", "delay", "tick_hz", "periods_used", "channel",
              "min_ms", "max_ms", "avg_ms", "jitter_us", "drift_ms",
              "samples_total", "edges_total"]
    with args.output.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        for r in results:
            writer.writerow({name: r[name] for name in fields})
    print("Summary saved to:", args.output)
    print("WARNING: the task period equals one edge-to-edge gap ONLY if "
          "GPIO toggles exactly once per task iteration.")


if __name__ == "__main__":
    main()
