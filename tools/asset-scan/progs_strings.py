#!/usr/bin/env python3
"""progs_strings.py — extract the string table from a Quake progs.dat and
resolve every path-like string against the runtime tree and the PAKs.

Why: some assets are loaded by name at RUNTIME (precached models/sounds),
so "unreferenced by BSP" does NOT mean unused. This is the safety net the
inventory's trap calls out ("Unreferenced != unused").

Header (quakespasm pr_comp.h dprograms_t, PROG_VERSION 6), little-endian ints:
  0 version | 4 crc | 8 totalsize
  12 ofs_statements  16 numstatements
  20 ofs_globaldefs  24 numglobaldefs
  28 ofs_fielddefs   32 numfielddefs
  36 ofs_functions   40 numfunctions
  44 ofs_strings     48 numstrings        <- string table
  52 ofs_globals     56 numglobals
  60 entityfields
Strings lump = [ofs_strings, ofs_globals); NUL-separated; first is "".

Output: progs_strings.json + stdout summary.
Usage: progs_strings.py [path/to/progs.dat]
"""
import json, os, re, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
RUNTIME = os.path.join(REPO, 'solitude')

PATH_RE = re.compile(r'^[A-Za-z0-9_./\\-]*\/[A-Za-z0-9_.$ -]+$')
ASSET_EXT = ('.mdl', '.spr', '.wav', '.pcx', '.tga', '.lit', '.bsp', '.dat')

def load_dat(path):
    data = open(path, 'rb').read()
    ver = struct.unpack_from('<i', data, 0)[0]
    if ver != 6:
        sys.exit(f'unexpected progs version {ver} (want 6) in {path}')
    # FTEQCC dprograms_t: version, crc, then (ofs, num) pairs —
    # statements, globaldefs, fielddefs, functions, strings, globals.
    # (NOT quakespasm's num-then-ofs layout.)
    ofs_statements = struct.unpack_from('<i', data, 8)[0]
    ofs_strings = struct.unpack_from('<i', data, 40)[0]
    numstrings = struct.unpack_from('<i', data, 44)[0]  # byte length of lump
    ofs_globals = struct.unpack_from('<i', data, 48)[0]
    numglobals = struct.unpack_from('<i', data, 52)[0]
    if ofs_strings + numstrings != ofs_statements:
        sys.exit(f'FTE header decode mismatch in {path}: strings lump '
                 f'[{ofs_strings}+{numstrings}] does not end at ofs_statements {ofs_statements}')
    if ofs_globals + numglobals * 4 != len(data):
        sys.exit(f'FTE header decode mismatch in {path}: globals do not end at EOF')
    blob = data[ofs_strings:ofs_strings + numstrings]
    parts = blob.split(b'\0')
    return [p.decode('latin1') for p in parts]

def pak_names(pakpath):
    if not os.path.exists(pakpath):
        return set()
    d = open(pakpath, 'rb').read()
    if d[:4] != b'PACK':
        return set()
    # Two header conventions: classic id (dirOfs@4, dirLen@8) and versioned
    # (version@4, dirOfs@8, dirLen@12). Validate by first-entry sanity.
    h = struct.unpack_from('<4i', d, 0)
    names = set()
    for dirstart, dirlen in ((h[1], h[2]), (h[2], h[3])):
        if not (dirlen % 64 == 0 and 0 < dirlen < len(d) and dirstart + dirlen <= len(d)):
            continue
        first = d[dirstart:dirstart + 56].split(b'\0')[0]
        if not (len(first) >= 6 and all(32 <= b < 127 for b in first)):
            continue
        for i in range(dirlen // 64):
            p = dirstart + i * 64
            nm = d[p:p + 56].split(b'\0')[0].decode('latin1').lower()
            if nm:
                names.add(nm)
        break
    return names

def main():
    target = sys.argv[1] if len(sys.argv) > 1 else os.path.join(RUNTIME, 'progs.dat')
    strings = load_dat(target)
    md5 = os.popen(f'md5 -q "{target}"').read().strip()
    paths = sorted({s for s in strings
                    if PATH_RE.match(s) and s.lower().endswith(ASSET_EXT)})
    # also keep precache-ish path strings without extension (sounds dirs etc.)
    paks = {}
    for pak in ('PAK0.PAK', 'PAK1.PAK', 'PAK2.PAK'):
        p = os.path.join(RUNTIME, pak)
        if os.path.exists(p):
            paks[pak] = pak_names(p)
    allpak = set().union(*paks.values()) if paks else set()
    # id1 base-game pak ships separately (user must own Quake) — refs that
    # resolve there are 'id1-dependency', NOT missing.
    id1_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
        REPO, '..', 'local', 'archive', 'resources', 'Quakespasm', 'id1', 'pak0.pak')
    id1 = pak_names(id1_path)
    if id1:
        print(f'id1 pak: {id1_path}  ({len(id1)} files)')

    resolved = {}
    for s in paths:
        loc = None
        for cand in (s.lower(), 'sound/' + s.lower()):
            disk = os.path.join(RUNTIME, cand)
            disk2 = os.path.join(REPO, cand)
            if os.path.exists(disk):
                loc = 'solitude/'
                break
            if os.path.exists(disk2):
                loc = 'repo/' + cand.split('/')[0]
                break
            if cand in allpak:
                loc = next(k for k, v in paks.items() if cand in v)
                break
            if cand in id1:
                loc = 'id1/pak0.pak'
                break
        resolved[s] = loc

    out = os.path.join(HERE, 'progs_strings.json')
    json.dump({'dat': os.path.relpath(target, REPO), 'md5': md5,
               'numstrings': len([s for s in strings if s]),
               'paths': {s: resolved[s] for s in paths}}, fh := open(out, 'w'))
    fh.close()

    missing = [s for s, v in resolved.items() if v is None]
    print(f'dat: {target}  md5 {md5}')
    print(f'strings: {len(strings)}  path-like asset refs: {len(paths)}')
    from collections import Counter
    c = Counter(v or 'MISSING' for v in resolved.values())
    for k, n in c.most_common():
        print(f'  {n:4d}  {k}')
    if missing:
        print('MISSING (referenced by progs.dat, not found anywhere):')
        for m in missing:
            print('   ', m)
    print(f'wrote {out}')

if __name__ == '__main__':
    main()
