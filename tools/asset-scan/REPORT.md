# Asset reconciliation report

Inputs: bsp_refs.json (362 bsp), progs_strings.json (337 refs, md5 c43d6d2c75a15276b489ccaf2bd11d83), loose runtime 1843 files, paks parsed.

## Runtime solitude/ verdicts
- referenced (progs.dat or bsp entities): 276
- pak-covered loose duplicates: 131 (5.5 MB also inside PAKs)
- orphan (no scan evidence; NOT proof of unused — name-loaded assets exist): 1434 (297.2 MB)

## Dead progs.dat references
- effects/bodyhit2.wav
- soldier/kill_it.wav

## Maps tracked as .map source with no runtime .bsp
- apdm3.map (solitude-vita-backup/Solitude/maps/apdm3.map, solitude/maps/apdm3.map)
- babybloodgulch.map (assets/maps/babybloodgulch/babybloodgulch.map)
- bigbloodgulch.map (assets/maps/bigbloodgulch/bigbloodgulch.map)
- cestra.map (assets/maps/cestra/cestra.map)
- citadel.map (assets/maps/citadel/citadel.map, solitude/maps/citadel.map)
- highground.map (assets/maps/highground/highground.map)
- lane.map (assets/maps/lane/lane.map)
- lockdown.map (assets/maps/lockdown/lockdown.map, assets/maps/narrows/lockdown.map)
- narrows.map (solitude/maps/narrows.map, solitude-vita-backup/Solitude/maps/narrows.map, solitude-production/Solitude/maps/narrows.map, assets/maps/narrows/narrows.map)
- narrows2.map (assets/maps/narrows/narrows2.map)
- narrows3.map (assets/maps/narrows/narrows3.map)
- narsp.map (solitude-vita-backup/Solitude/maps/narsp.map, solitude-production/Solitude/maps/narsp.map, solitude/maps/narsp.map, assets/maps/narrowed/narsp.map)
- narspfixed.map (solitude/maps/narspfixed.map, solitude-vita-backup/Solitude/maps/narspfixed.map)
- narspfixed2.map (solitude-vita-backup/Solitude/maps/narspfixed2.map, solitude/maps/narspfixed2.map)
- pit2.map (assets/maps/pit/pit2.map)
- pit3.map (assets/maps/pit/pit3.map)
- skyringbl2.map (solitude-vita-backup/Solitude/maps/skyringbl2.map, solitude/maps/skyringbl2.map, solitude-production/Solitude/maps/skyringbl2.map, assets/maps/skyring/skyringbl2.map)
- solitude_props.map (assets/maps/solitude_props/solitude_props.map)
- wizard.map (assets/maps/wizard/wizard.map)

## Tracked derived files (should become untracked per decision #5): 428

### Top 30 orphans by size
- 19.09 MB solitude/sound/music/03.mp3
- 4.90 MB solitude/maps/cestra.bsp
- 4.76 MB solitude/progs/playerr.mdl
- 4.53 MB solitude/progs/playerult.mdl
- 4.40 MB solitude/maps/off.bsp
- 4.34 MB solitude/progs/playercolors.mdl
- 4.01 MB solitude/progs/v_nailgun.mdl
- 3.94 MB solitude/maps/pit2.bsp
- 3.51 MB solitude/maps/highground.bsp
- 3.50 MB solitude/maps/pit3.bsp
- 3.23 MB solitude/maps/narrows.map
- 3.12 MB solitude/maps/pit2q.bsp
- 2.82 MB solitude/maps/skyringbl2.bsp
- 2.59 MB solitude/maps/cestra.lit
- 2.31 MB solitude/maps/narrows.bsp
- 2.27 MB solitude/maps/+allout.bsp
- 2.10 MB solitude/maps/+frostbite.lit
- 2.06 MB solitude/maps/lockdown.bsp
- 2.03 MB solitude/gfx/env.rar
- 2.01 MB solitude/maps/+separation.bsp
- 2.00 MB solitude/maps/+narrowedctf.bsp
- 1.99 MB solitude/gfx/loading.tga
- 1.99 MB solitude/gfx/menu/menu_bg.tga
- 1.96 MB solitude/maps/+field.bsp
- 1.86 MB solitude/maps/+maze.bsp
- 1.82 MB solitude/maps/z.bsp
- 1.78 MB solitude/maps/+lane.bsp
- 1.73 MB solitude/maps/narrows.lit
- 1.72 MB solitude/maps/skyringbl1.bsp
- 1.68 MB solitude/maps/washedup.bsp

### Scan limitations / interpretation notes
- changelevel + cdtrack refs in QC are extensionless names -> most .bsp/.mp3 "orphans" are false negatives (e.g. cestra.bsp, sound/music/03.mp3).
- 2024 HUD .png/.lmp art loads by engine-side name, never appears in progs.dat.
- 109 .qc files inside runtime solitude/ = stray source copies, not assets.
- progs/player*.mdl, v_nailgun.mdl not in shipped dat strings: either mod-era leftovers or loaded only by alternate cvars — verify before any pruning.
- verdict "orphan" NEVER authorises deletion by itself (decision: drop-nothing).
