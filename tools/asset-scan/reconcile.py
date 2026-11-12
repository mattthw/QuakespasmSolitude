#!/usr/bin/env python3
"""reconcile.py — join bsp_refs.json + progs_strings.json + disk/pak inventories
into per-asset verdicts. Output: reconcile.json + REPORT.md in this dir."""
import json, os, subprocess, sys
from progs_strings import pak_names, REPO, RUNTIME, HERE

def main():
    tracked = set(subprocess.run(['git', '-C', REPO, 'ls-files'],
                                 capture_output=True, text=True).stdout.split())
    bsp = json.load(open(os.path.join(HERE, 'bsp_refs.json')))
    progs = json.load(open(os.path.join(HERE, 'progs_strings.json')))

    prog_refs = {p.lower(): v for p, v in progs['paths'].items()}
    ent_refs = {}
    for r in bsp:
        for p in r['entity_paths']:
            pl = p.lower()
            if pl.startswith('/users/') or ';' in pl:
                continue  # stale wad-key / external: provenance junk, not runtime refs
            ent_refs.setdefault(pl, []).append(r['bsp'])

    # map name -> tracked .map source (by basename)
    map_src = {}
    for t in tracked:
        if t.lower().endswith('.map') and '-crash' not in os.path.basename(t).lower():
            map_src.setdefault(os.path.basename(t).lower(), []).append(t)

    # pak inventories (for loose-vs-pak duplicate detection); pak entries are
    # full paths ('sound/ambience/x.wav'), loose runtime paths carry extra
    # tiers ('solitude/sound/...'), so key by basename.
    pak_of = {}   # basename(lower) -> pakname
    for pk in ('PAK0.PAK', 'PAK1.PAK', 'PAK2.PAK'):
        p = os.path.join(RUNTIME, pk)
        for nm in pak_names(p):
            pak_of[os.path.basename(nm)] = pk

    verdicts = []
    loose = set()
    for dirpath, _dirs, files in os.walk(RUNTIME):
        for f in files:
            full = os.path.join(dirpath, f)
            rel = os.path.relpath(full, REPO).replace('/', os.sep).lower()
            loose.add(rel.replace(os.sep, '/'))

    for rel in sorted(loose):
        if rel.endswith('.pak'):
            continue
        name = os.path.basename(rel)
        refs = []
        parts = rel.split('/')
        suffixes = {'/'.join(parts[i:]) for i in range(len(parts) - 1)}
        for pr in suffixes & set(prog_refs):
            refs.append('progs(%s)' % prog_refs[pr])
        for er in suffixes & set(ent_refs):
            refs.append('bsp_entities:%d' % len(ent_refs[er]))
        if rel.endswith('.bsp') and name in map_src:
            refs.append('map-source')
        if rel.endswith('.bsp'):
            kind = 'map'
            buildable = name in map_src
        else:
            kind = 'asset'
            buildable = False
        dup = name in pak_of
        if refs:
            v = 'referenced'
        elif dup:
            v = 'pak-covered'        # loose copy of something also inside a pak
        else:
            v = 'orphan'             # not found by progs or bsp scans — MAY still be loaded by name; do not prune
        verdicts.append({'path': rel, 'class': kind, 'verdict': v,
                         'refs': refs, 'dup_pak': pak_of[name] if dup else None})

    # progs refs that resolve nowhere
    dead = [p for p, v in progs['paths'].items() if v in (None, 'MISSING')]

    # maps: tracked bsp with/without source, and changelevel maps missing bsp
    maps = []
    for name, srcs in sorted(map_src.items()):
        have_bsp = any(os.path.basename(v).lower() == name for v in loose if v.endswith('.bsp'))
        maps.append({'map': name, 'sources': srcs, 'runtime_bsp': have_bsp})

    tracked_derived = sorted(t for t in tracked
                             if t.lower().endswith(('.bsp', '.lit', '.vis', '.prt', 'progs.dat')))

    out = {'runtime_files': len(loose), 'verdicts': verdicts,
           'counts': {k: sum(1 for x in verdicts if x['verdict'] == k)
                      for k in ('referenced', 'pak-covered', 'orphan')},
           'dead_progs_refs': dead, 'maps': maps,
           'tracked_derived_files': tracked_derived}
    json.dump(out, open(os.path.join(HERE, 'reconcile.json'), 'w'), indent=1)

    orph = [x for x in verdicts if x['verdict'] == 'orphan']
    cov = [x for x in verdicts if x['verdict'] == 'pak-covered']
    orph_bytes = sum(os.path.getsize(os.path.join(REPO, x['path'])) for x in orph)
    cov_bytes = sum(os.path.getsize(os.path.join(REPO, x['path'])) for x in cov)
    ref = [x for x in verdicts if x['verdict'] == 'referenced']
    no_bsp = [m for m in maps if not m['runtime_bsp']]
    if not no_bsp:
        no_bsp = [{'map': '- none', 'sources': []}]
    lines = ['# Asset reconciliation report',
             '',
             'Inputs: bsp_refs.json (%d bsp), progs_strings.json (%d refs, md5 %s), '
             'loose runtime %d files, paks parsed.' % (len(bsp), len(prog_refs), progs['md5'], len(loose)),
             '',
             '## Runtime solitude/ verdicts',
             '- referenced (progs.dat or bsp entities): %d' % len(ref),
             '- pak-covered loose duplicates: %d (%.1f MB also inside PAKs)' % (len(cov), cov_bytes/1048576),
             '- orphan (no scan evidence; NOT proof of unused — name-loaded assets exist): %d (%.1f MB)' % (len(orph), orph_bytes/1048576),
             '',
             '## Dead progs.dat references',
             *([('- %s' % d) for d in dead] or ['- none']),
             '',
             '## Maps tracked as .map source with no runtime .bsp',
             *[('- %s (%s)' % (m['map'], ', '.join(m['sources']))) for m in no_bsp],
             '',
             '## Tracked derived files (should become untracked per decision #5): %d' % len(tracked_derived),
             '', '### Top 30 orphans by size',
             *sorted(('- %.2f MB %s' % (os.path.getsize(os.path.join(REPO, x['path']))/1048576, x['path'])
                      for x in orph), key=lambda s: -float(s.split(' MB')[0].lstrip('- ')))[:30]]
    lines += ['', '### Scan limitations / interpretation notes',
              '- changelevel + cdtrack refs in QC are extensionless names -> most .bsp/.mp3 '
              '"orphans" are false negatives (e.g. cestra.bsp, sound/music/03.mp3).',
              '- 2024 HUD .png/.lmp art loads by engine-side name, never appears in progs.dat.',
              '- 109 .qc files inside runtime solitude/ = stray source copies, not assets.',
              '- progs/player*.mdl, v_nailgun.mdl not in shipped dat strings: either mod-era '
              'leftovers or loaded only by alternate cvars — verify before any pruning.',
              '- verdict "orphan" NEVER authorises deletion by itself (decision: drop-nothing).']
    open(os.path.join(HERE, 'REPORT.md'), 'w').write('\n'.join(lines) + '\n')
    print('\n'.join(lines[:22]))
    print('full report -> REPORT.md, data -> reconcile.json')

if __name__ == '__main__':
    main()
