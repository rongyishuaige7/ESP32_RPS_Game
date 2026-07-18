#!/usr/bin/env python3
"""历史 EDA 脱敏衍生文件的窄范围发布门禁。"""
from __future__ import annotations
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCHEMATIC = ROOT / "hardware/eda/schematic.pdf"
EVIDENCE = ROOT / "docs/MEDIA_EVIDENCE.md"
PRIVATE = re.compile(rb"(?:192\.168\.\d{1,3}\.\d{1,3}|10(?:\.\d{1,3}){3}|172\.(?:1[6-9]|2\d|3[01])(?:\.\d{1,3}){2}|-----BEGIN [A-Z ]*PRIVATE KEY-----)")


def main() -> int:
    failures: list[str] = []
    for path in (SCHEMATIC, EVIDENCE):
        if not path.is_file():
            failures.append(f"missing public material: {path.relative_to(ROOT)}")
    if SCHEMATIC.is_file():
        raw = SCHEMATIC.read_bytes()
        if not raw.startswith(b"%PDF-"):
            failures.append("schematic is not a PDF")
        if len(raw) > 2 * 1024 * 1024:
            failures.append("schematic exceeds 2 MiB")
        if PRIVATE.search(raw):
            failures.append("schematic contains private-network or credential material")
    if EVIDENCE.is_file():
        text = EVIDENCE.read_text(encoding="utf-8")
        for required in ("权威原件保持不变", "当前未进行真机复测"):
            if required not in text:
                failures.append(f"evidence boundary missing: {required}")
    if failures:
        print("Public material check: FAIL", file=sys.stderr)
        print("\n".join(f"- {item}" for item in failures), file=sys.stderr)
        return 1
    print("Public material check: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
