#!/usr/bin/env python3
import os
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
EXTS = {".c", ".cpp", ".cc", ".h", ".hpp"}

files = []
for base in [ROOT / "src", ROOT / "examples"]:
    if not base.exists():
        continue
    for path, dirs, fs in os.walk(base):
        for name in fs:
            p = Path(path) / name
            if p.suffix.lower() in EXTS:
                files.append(p)

files.sort()

total_lines = 0
print("File;Lines;Bytes")
for p in files:
    try:
        with p.open("r", encoding="utf-8", errors="ignore") as f:
            lines = sum(1 for _ in f)
    except Exception:
        lines = 0
    size = p.stat().st_size
    total_lines += lines
    print(f"{p.relative_to(ROOT)};{lines};{size}")

print(f"TOTAL_LINES;{total_lines}") 