#!/usr/bin/env python3
"""Generate iOS app icon PNGs without storing binaries in git.

Creates solid-color icons for all sizes defined in AppIcon.appiconset/Contents.json.
The PNG encoder is implemented with the stdlib (struct+zlib) to avoid extra deps.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import struct
import zlib

# Simple RGBA color for the background (blue).
BACKGROUND = (0x1E, 0x88, 0xE5, 0xFF)  # Blue 600


def _png_bytes(width: int, height: int, color: tuple[int, int, int, int]) -> bytes:
    signature = b"\x89PNG\r\n\x1a\n"

    def chunk(tag: bytes, payload: bytes) -> bytes:
        return (
            struct.pack("!I", len(payload))
            + tag
            + payload
            + struct.pack("!I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
        )

    ihdr = chunk(b"IHDR", struct.pack("!IIBBBBB", width, height, 8, 6, 0, 0, 0))
    # filter type 0 per row, uncompressed RGBA pixels
    scanline = bytes(color * width)
    raw = bytearray()
    for _ in range(height):
        raw.append(0)
        raw.extend(scanline)
    idat = chunk(b"IDAT", zlib.compress(bytes(raw), level=9))
    iend = chunk(b"IEND", b"")
    return signature + ihdr + idat + iend


def generate_icon(path: Path, size: int) -> None:
    data = _png_bytes(size, size, BACKGROUND)
    path.write_bytes(data)


def load_icons_spec(appiconset: Path) -> list[tuple[str, int]]:
    contents = json.loads((appiconset / "Contents.json").read_text())
    outputs: list[tuple[str, int]] = []
    for image in contents.get("images", []):
        filename = image.get("filename")
        size_str = image.get("size")
        scale = image.get("scale", "1x")
        if not filename or not size_str:
            continue
        base = float(size_str.split("x")[0])
        factor = int(scale.rstrip("x"))
        outputs.append((filename, int(base * factor)))
    return outputs


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate iOS app icons")
    parser.add_argument(
        "--appiconset",
        type=Path,
        default=Path("ios/Runner/Assets.xcassets/AppIcon.appiconset"),
        help="Path to AppIcon.appiconset directory",
    )
    args = parser.parse_args()

    appiconset = args.appiconset
    appiconset.mkdir(parents=True, exist_ok=True)

    outputs = load_icons_spec(appiconset)
    if not outputs:
        raise SystemExit("No icon specifications found in Contents.json")

    for filename, size in outputs:
        destination = appiconset / filename
        destination.parent.mkdir(parents=True, exist_ok=True)
        generate_icon(destination, size)
        print(f"generated {destination} ({size}x{size})")


if __name__ == "__main__":
    main()
