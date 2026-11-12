#!/usr/bin/env python3
"""bsp_refs.py — read-only census of every BSP in the repo.

For each .bsp (tracked or runtime): version, worldspawn "wad" key, embedded
vs external (file-texture) miptex, inline model count ("*N" entity keys),
external model references (progs/*.mdl etc), and any path-like entity values.

Output: bsp_refs.json (array of per-bsp records) + stdout summary.
"""
import json, os, struct, subprocess, sys
from collections import Counter

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def tracked_bsps():
    out = subprocess.run(['git', '-C', REPO, 'ls-files', '-z', '*.bsp'],
                         capture_output=True, check=True).stdout
    return set(os.path.relpath(p.decode('utf-8'), REPO) for p in out.split(b'\0') if p)

def all_bsps():
    found = []
    for root, dirs, files in os.walk(REPO):
        dirs[:] = [d for d in dirs if d not in ('.git', 'build', 'node_modules')]
        for f in files:
            if f.lower().endswith('.bsp'):
                found.append(os.path.relpath(os.path.join(root, f), REPO))
    return sorted(set(found))

LUMP_ENTITIES, LUMP_TEXTURES, LUMP_MODELS, LUMP_TEXDATA = 0, 2, 9, 12

def read_lumps(data):
    ver = struct.unpack_from('<i', data, 0)[0]
    if ver not in (20, 29, 30):
        return ver, None
    lumps = []
    for i in range(15):
        ofs, size = struct.unpack_from('<2i', data, 4 + i * 8)
        lumps.append((ofs, size))
    return ver, lumps

def lump_names(data, tofs, names_ofs, names_size):
    """TEXDATA lump (12): int nummiptex + offtexnames[nummiptex] rel. to its
    start; parallel to dataofs. Returns list of names (may be empty)."""
    if names_size <= 4:
        return []
    n = struct.unpack_from('<i', data, names_ofs)[0]
    if n <= 0 or names_ofs + 4 + n * 4 > len(data):
        return []
    out = []
    for i in range(n):
        so = struct.unpack_from('<i', data, names_ofs + 4 + i * 4)[0]
        p = names_ofs + so
        out.append(data[p:data.index(b'\0', p)].decode('latin1')
                   if p < len(data) and b'\0' in data[p:p + 256] else '')
    return out if len(out) == n else []

def parse_entities(ent):
    """Return (worldspawn_wad, model_keys Counter, path_values set)."""
    wad = None
    models = Counter()
    paths = set()
    in_world = False
    seen_key = False
    cur = {}
    def flush():
        nonlocal wad, in_world
        if cur.get('classname') == 'worldspawn' and 'wad' in cur:
            wad = cur['wad']
        for k, v in cur.items():
            if k == 'model':
                models['inline' if v.startswith('*') else 'external'] += 1
                if not v.startswith('*'):
                    paths.add(v)
            elif '/' in v and (('.' in v and v.rsplit('.', 1)[-1].isalnum())
                               or v.startswith('progs/')):
                paths.add(v)
    # crude entity block parser: strings between { }
    import re
    for m in re.finditer(r'\{([^{}]*)\}', ent, re.S):
        cur = dict(re.findall(r'"([^"]+)"\s+"([^"]*)"', m.group(1)))
        flush()
    return wad, models, paths

def parse_textures(data, tofs, tsz, names_ofs, names_size):
    """dmiptexlump_t: nummiptex + dataofs[nummiptex] (rel. to lump start).
    dataofs[i] == -1  <=>  external WAD texture; its name lives only in the
    TEXDATA (12) lump — the miptex record does not exist (verified against
    quakespasm gl_model.c Mod_LoadTextures)."""
    num = struct.unpack_from('<i', data, tofs)[0]
    if num <= 0 or tofs + 4 + num * 4 > len(data):
        return [], []
    names = lump_names(data, tofs, names_ofs, names_size)
    dataofs = struct.unpack_from('<%di' % num, data, tofs + 4)
    embedded, external = [], []
    for i, mo in enumerate(dataofs):
        nm = names[i] if i < len(names) else ''
        if mo == -1:
            external.append(nm)  # name only from TEXDATA lump ('' if absent)
        else:
            p = tofs + mo
            if not nm:
                nm = data[p:p + 16].split(b'\0')[0].decode('latin1')
            embedded.append(nm)
    return embedded, external

def main():
    trk = tracked_bsps()
    records = []
    tot_ext_tex = 0
    wad_counter = Counter()
    ext_models = Counter()
    bad = []
    for rel in all_bsps():
        p = os.path.join(REPO, rel)
        with open(p, 'rb') as fh:
            data = fh.read()
        ver, lumps = read_lumps(data)
        rec = {'bsp': rel, 'tracked': rel in trk, 'version': ver}
        if lumps is None:
            bad.append(rel)
            records.append(rec)
            continue
        eofs, esz = lumps[LUMP_ENTITIES]
        wad, models, paths = parse_entities(data[eofs:eofs + esz].decode('latin1'))
        rec['wad_key'] = wad
        rec['inline_models'] = models['inline']
        rec['external_models'] = models['external']
        rec['entity_paths'] = sorted(paths)
        if wad:
            for w in wad.split(';'):
                if w:
                    wad_counter[w] += 1
        for m in paths:
            if m.startswith('progs/'):
                ext_models[m] += 1
        tofs, tsz = lumps[LUMP_TEXTURES]
        ndofs, ndsz = lumps[LUMP_TEXDATA]
        embedded, external = parse_textures(data, tofs, tsz, ndofs, ndsz)
        rec['tex_total'] = len(embedded) + len(external)
        rec['tex_embedded'] = len(embedded)
        rec['tex_external'] = external  # names kept even when data lives in WAD
        tot_ext_tex += len(external)
        records.append(rec)

    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'bsp_refs.json')
    with open(out, 'w') as fh:
        json.dump(records, fh, indent=1)
    print(f'bsp files scanned: {len(records)}  (tracked: {sum(r["tracked"] for r in records)})')
    print(f'unparseable versions: {bad if bad else "none"}')
    print(f'self-contained (zero external miptex): '
          f'{sum(1 for r in records if not r.get("tex_external"))} / {len(records)}')
    print(f'total external-texture refs: {tot_ext_tex}')
    print('wad keys named (top 12):')
    for w, n in wad_counter.most_common(12):
        print(f'  {n:4d}  {w}')
    print('external progs/* model refs:')
    for m, n in ext_models.most_common():
        print(f'  {n:4d}  {m}')
    print(f'wrote {out}')

if __name__ == '__main__':
    main()
