#!/usr/bin/env python3
"""reconcile.py — join bsp_refs.json + progs_strings.json + disk/pak inventories
into per-asset verdicts. Output: reconcile.json + REPORT.md in this dir.
v2: adds changelevel-target + play-script evidence, and a name-loaded category
for engine-convention artwork, replacing the blunt 'orphan' verdict."""
import json, os, re, subprocess, sys
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

    # --- v2 evidence sources --------------------------------------------
    # changelevel targets: "map"/"changelevel" entity keys in runtime bsp lumps
    map_stems = set()
    for rel in loose:
        if rel.endswith('.bsp'):
            try:
                d = open(os.path.join(REPO, rel), 'rb').read(600000).decode('latin-1')
            except OSError:
                continue
            for m in re.finditer(r'"(?:map|changelevel)"\s+"([^"]+)"', d):
                v = m.group(1).lower().strip()
                if v and '/' not in v and ';' not in v and ' ' not in v:
                    map_stems.add(re.sub(r'\.bsp$', '', v))
    # `play <stem>` console-script refs in runtime cfg/rc -> sound/<stem>.*
    play_refs = set()
    for rel in loose:
        if rel.endswith(('.cfg', '.rc')):
            t = open(os.path.join(REPO, rel), encoding='latin-1', errors='replace').read()
            for m in re.finditer(r'\bplay\s+([A-Za-z0-9_/]+)', t):
                s = m.group(1).lower()
                if not s.startswith('sound/'):
                    s = 'sound/' + s
                for e in ('.mp3', '.wav', '.ogg'):
                    play_refs.add(s + e)
    # engine-by-name conventions: HUD/crosshair/menu/particle artwork loads by
    # filename pattern — scans over progs.dat/bsp can never prove it.
    NAME_LOADED = ('solitude/gfx/', 'solitude/textures/', 'solitude/particles/',
                   'solitude/sprites/', 'solitude/sound/music/')
    NAME_LOADED_EXT = ('.lmp', '.pcx', '.dat')

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
        if kind == 'map' and name[:-4] in map_stems:
            refs.append('changelevel-target')
        if rel[len('solitude/'):] in play_refs:
            refs.append('play-script')
        dup = name in pak_of
        if refs:
            v = 'referenced'
        elif dup:
            v = 'pak-covered'        # loose copy of something also inside a pak
        elif (rel.startswith(NAME_LOADED) or rel.endswith(NAME_LOADED_EXT)
              or kind == 'map'):
            v = 'name-loaded'        # loaded by engine convention/name; unprovable by scan
        else:
            v = 'unverified'         # no scan evidence — still NOT proof of unused
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
                      for k in ('referenced', 'pak-covered', 'name-loaded', 'unverified')},
           'dead_progs_refs': dead, 'maps': maps,
           'tracked_derived_files': tracked_derived}
    json.dump(out, open(os.path.join(HERE, 'reconcile.json'), 'w'), indent=1)

    nl = [x for x in verdicts if x['verdict'] == 'name-loaded']
    orph = [x for x in verdicts if x['verdict'] == 'unverified']
    cov = [x for x in verdicts if x['verdict'] == 'pak-covered']
    nl_bytes = sum(os.path.getsize(os.path.join(REPO, x['path'])) for x in nl)
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
             '- referenced (progs.dat / bsp entities / changelevel / play): %d' % len(ref),
             '- pak-covered loose duplicates: %d (%.1f MB also inside PAKs)' % (len(cov), cov_bytes/1048576),
             '- name-loaded by engine convention (HUD/gfx/menu/music/maps — unprovable by scan): %d (%.1f MB)' % (len(nl), nl_bytes/1048576),
             '- unverified (no scan evidence; still NOT proof of unused): %d (%.1f MB)' % (len(orph), orph_bytes/1048576),
             '',
             '## Dead progs.dat references',
             *([('- %s' % d) for d in dead] or ['- none']),
             '',
             '## Maps tracked as .map source with no runtime .bsp',
             *[('- %s (%s)' % (m['map'], ', '.join(m['sources']))) for m in no_bsp],
             '',
             '## Tracked derived files (should become untracked per decision #5): %d' % len(tracked_derived),
             '', '### Top 30 unverified by size',
             *sorted(('- %.2f MB %s' % (os.path.getsize(os.path.join(REPO, x['path']))/1048576, x['path'])
                      for x in orph), key=lambda s: -float(s.split(' MB')[0].lstrip('- ')))[:30]]
    lines += ['', '### Scan limitations / interpretation notes',
              '- v2 matcher counts changelevel-target (bsp entities), play-script (cfg/rc) and',
              '  map-source evidence. Runtime .bsp are grandfathered tracked copies (decision #5',
              '  targets future additions; .githooks/pre-commit enforces).',
              '- cdtrack exists NOWHERE in runtime bsp entities/qc (measured 2026-11-11); mp3s load',
              '  via `play music/<stem>` scripts and the vitaQuake engine track table.',
              '- "name-loaded" is a POSITIVE category: gfx/ HUD .lmp/.pcx, menu .dat, textures/ +',
              '  particles/ + sprites/ art, sound/music/. Engine loads by filename convention.',
              '- 109 .qc files inside runtime solitude/ = stray source copies, not assets.',
              '- progs/player*.mdl, v_nailgun.mdl not in shipped dat strings: either mod-era '
              'leftovers or loaded only by alternate cvars — verify before any pruning.',
              '- verdict "unverified" NEVER authorises deletion by itself (decision: drop-nothing).']
    open(os.path.join(HERE, 'REPORT.md'), 'w').write('\n'.join(lines) + '\n')
    print('\n'.join(lines[:22]))
    print('full report -> REPORT.md, data -> reconcile.json')

if __name__ == '__main__':
    main()
