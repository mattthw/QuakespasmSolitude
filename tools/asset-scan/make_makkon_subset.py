#!/usr/bin/env python3
"""make_makkon_subset.py — build assets/textures/wads/makkon-subset.wad.

The Makkon0-3 texture packs (928 MB, author Ben 'Makkon' Hale, free for
community non-commercial use with credit — see LICENSE-makkon.txt) are too
large to track and >100 MB each so git/GitHub could never hold them.
Every Solitude map combined uses only these 18 lumps (~7.3 MB), including
animation sibling frames, so this subset replaces all four packs as the
only Makkon source any rebuild needs.

Lumps are copied verbatim (they are type 0x44 palette-less 512x512 miptex
that Makkon shipped; qbsp support is verified by rebuild, see REPORT.md).

Run from the repo root. Writes assets/textures/wads/makkon-subset.wad.
"""
import os
import struct
import sys

SRC_DIRS = [
    "../local/vendored/wads/Makkon0/Makkon0.wad",
    "../local/vendored/wads/Makkon1/Makkon1.wad",
    "../local/vendored/wads/Makkon2/Makkon2.wad",
    "../local/vendored/wads/Makkon3/Makkon3.wad",
]
OUT = "assets/textures/wads/makkon-subset.wad"

# name (lower) -> source file basename that holds it; verified 2026-11-12
WANTED = [
    ("metal_brass2_01", "Makkon0"),
    ("{metal_coppf01", "Makkon0"),
    ("{metal_coppf02", "Makkon0"),
    ("{metal_coppf03", "Makkon0"),
    ("cliff_tan_10", "Makkon1"),
    ("cliff_tan_16", "Makkon1"),
    ("groundsm_red02", "Makkon2"),
    ("sand_red102", "Makkon2"),
    ("woodc2_wwht01", "Makkon2"),
    ("techflr5_grey2", "Makkon3"),
    ("techlight_1", "Makkon3"),
    ("+0techlight_1", "Makkon3"),
    ("+atechlight_1", "Makkon3"),
    ("techlight_r1", "Makkon3"),
    ("+0techlight_r1", "Makkon3"),
    ("+atechlight_r1", "Makkon3"),
    ("techlzr_blu3", "Makkon3"),
    ("techpnl_grey1", "Makkon3"),
]


def read_wad(path):
    data = open(path, "rb").read()
    if data[:4] not in (b"WAD3", b"WAD2"):
        sys.exit(f"not a WAD3: {path}")
    n, base = struct.unpack("<ii", data[4:12])
    lumps = {}
    for i in range(n):
        rec = data[base + i * 32: base + i * 32 + 32]
        pos, size = struct.unpack("<ii", rec[:8])
        typ = rec[12]
        name = rec[16:32].split(b"\0")[0].decode("ascii", "replace")
        lumps[name.lower()] = (size, typ, data[pos:pos + size])
    return lumps


def main():
    sources = {os.path.basename(p): read_wad(p) for p in SRC_DIRS}
    records, payload = [], b""
    for name, src in WANTED:
        key = name.lower()
        lumps = sources[src + ".wad"]
        if key not in lumps:
            sys.exit(f"{name} not found in {src}")
        size, typ, blob = lumps[key]
        records.append((len(payload), size, typ, name))
        payload += blob
    infobase = 12 + len(payload)
    out = bytearray(struct.pack("<4sii", b"WAD3", len(records), infobase))
    out += payload
    for pos, size, typ, name in records:
        nb = name.encode("ascii").ljust(16, b"\0")[:16]
        out += struct.pack("<iiiBBh", pos, size, size, typ, 0, 0) + nb
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "wb") as f:
        f.write(out)
    print(f"{OUT}: {len(records)} lumps, {len(out)} bytes")


if __name__ == "__main__":
    main()
