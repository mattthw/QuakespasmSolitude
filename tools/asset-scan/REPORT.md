# Asset reconciliation report

Inputs: bsp_refs.json (362 bsp), progs_strings.json (337 refs, md5 c43d6d2c75a15276b489ccaf2bd11d83), loose runtime 1843 files, paks parsed.

## Runtime solitude/ verdicts
- referenced (progs.dat / bsp entities / changelevel / play): 284
- pak-covered loose duplicates: 131 (5.5 MB also inside PAKs)
- name-loaded by engine convention (HUD/gfx/menu/music/maps — unprovable by scan): 893 (215.8 MB)
- unverified (no scan evidence; still NOT proof of unused): 533 (74.3 MB)

## Dead progs.dat references
- effects/bodyhit2.wav
- soldier/kill_it.wav

## Maps tracked as .map source with no runtime .bsp
- apdm3.map (solitude/maps/apdm3.map, legacy/solitude-vita-backup/Solitude/maps/apdm3.map)
- babybloodgulch.map (assets/maps/babybloodgulch/babybloodgulch.map)
- bigbloodgulch.map (assets/maps/bigbloodgulch/bigbloodgulch.map)
- cestra.map (assets/maps/cestra/cestra.map)
- citadel.map (solitude/maps/citadel.map, assets/maps/citadel/citadel.map)
- highground.map (assets/maps/highground/highground.map)
- lane.map (assets/maps/lane/lane.map)
- lockdown.map (assets/maps/narrows/lockdown.map, assets/maps/lockdown/lockdown.map)
- narrows.map (assets/maps/narrows/narrows.map, legacy/solitude-vita-backup/Solitude/maps/narrows.map, solitude/maps/narrows.map, legacy/solitude-production/Solitude/maps/narrows.map)
- narrows2.map (assets/maps/narrows/narrows2.map)
- narrows3.map (assets/maps/narrows/narrows3.map)
- narsp.map (solitude/maps/narsp.map, assets/maps/narrowed/narsp.map, legacy/solitude-production/Solitude/maps/narsp.map, legacy/solitude-vita-backup/Solitude/maps/narsp.map)
- narspfixed.map (solitude/maps/narspfixed.map, legacy/solitude-vita-backup/Solitude/maps/narspfixed.map)
- narspfixed2.map (solitude/maps/narspfixed2.map, legacy/solitude-vita-backup/Solitude/maps/narspfixed2.map)
- pit2.map (assets/maps/pit/pit2.map)
- pit3.map (assets/maps/pit/pit3.map)
- skyringbl2.map (legacy/solitude-production/Solitude/maps/skyringbl2.map, legacy/solitude-vita-backup/Solitude/maps/skyringbl2.map, assets/maps/skyring/skyringbl2.map, solitude/maps/skyringbl2.map)
- solitude_props.map (assets/maps/solitude_props/solitude_props.map)
- wizard.map (assets/maps/wizard/wizard.map)

## Tracked derived files (should become untracked per decision #5): 428

### Top 30 unverified by size
- 4.76 MB solitude/progs/playerr.mdl
- 4.53 MB solitude/progs/playerult.mdl
- 4.34 MB solitude/progs/playercolors.mdl
- 4.01 MB solitude/progs/v_nailgun.mdl
- 3.23 MB solitude/maps/narrows.map
- 2.59 MB solitude/maps/cestra.lit
- 2.10 MB solitude/maps/+frostbite.lit
- 1.73 MB solitude/maps/narrows.lit
- 1.56 MB solitude/maps/pit2q.lit
- 1.32 MB solitude/maps/pit2.lit
- 1.29 MB solitude/sound/player/sonic2.wav
- 1.28 MB solitude/maps/pit3.lit
- 1.28 MB solitude/progs/v_ar2.mdl
- 1.06 MB solitude/maps/washedup.lit
- 1.06 MB solitude/sound/sonic2.wav
- 1.05 MB solitude/progs/player_sngun.mdl
- 1.01 MB solitude/progs/copy of v_nail.mdl
- 0.99 MB solitude/progs/player_nail.mdl
- 0.99 MB solitude/progs/player_nail2.mdl
- 0.99 MB solitude/progs/player_nailold.mdl
- 0.99 MB solitude/progs/playerbu.mdl
- 0.96 MB solitude/progs/v_shotter.mdl
- 0.94 MB solitude/maps/+trapbridgesctf.lit
- 0.91 MB solitude/maps/skyringbl2.lit
- 0.88 MB solitude/maps/narspfixed.lit
- 0.71 MB solitude/maps/babybloodgulch.lit
- 0.71 MB solitude/maps/highground.lit
- 0.71 MB solitude/progs/v_rock3.mdl
- 0.70 MB solitude/maps/narsp.lit
- 0.67 MB solitude/progs/progs.rar

### Scan limitations / interpretation notes
- v2 matcher counts changelevel-target (bsp entities), play-script (cfg/rc) and
  map-source evidence. Runtime .bsp are grandfathered tracked copies (decision #5
  targets future additions; .githooks/pre-commit enforces).
- cdtrack exists NOWHERE in runtime bsp entities/qc (measured 2026-11-11); mp3s load
  via `play music/<stem>` scripts and the vitaQuake engine track table.
- "name-loaded" is a POSITIVE category: gfx/ HUD .lmp/.pcx, menu .dat, textures/ +
  particles/ + sprites/ art, sound/music/. Engine loads by filename convention.
- 109 .qc files inside runtime solitude/ = stray source copies, not assets.
- progs/player*.mdl, v_nailgun.mdl not in shipped dat strings: either mod-era leftovers or loaded only by alternate cvars — verify before any pruning.
- verdict "unverified" NEVER authorises deletion by itself (decision: drop-nothing).
