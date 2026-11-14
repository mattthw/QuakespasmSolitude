# B0 map audit - 24 matchmaking-menu maps (BSP29, read-only)

Score = leak(50) + 20*PVS-weakness + 0.5*lightmap-MB + 2*count(>=1024 tex) + 0.2*gl_max_size512-savings-MB

| # | map | vis | >=1024 tex | tex DXT MB | cap512 saves MB | lightmaps MB | faces | verts | leafs |
|---|-----|-----|-----------|------------|-----------------|--------------|-------|-------|-------|
| 1 | Cestra | NO-VIS (leaked/unwritten) | 1 | 1.5 | 2.0 | 0.9 | 9473 | 11634 | 2604 |
| 2 | lane | NO-VIS (leaked/unwritten) | 0 | 0.3 | 0.0 | 0.2 | 4379 | 4977 | 1425 |
| 3 | pit | NO-VIS (leaked/unwritten) | 0 | 0.1 | 0.0 | 0.2 | 2959 | 4188 | 480 |
| 4 | chill | NO-VIS (leaked/unwritten) | 0 | 0.2 | 0.0 | 0.1 | 2486 | 3198 | 853 |
| 5 | plaza | WEAK-PVS (1.02 of all-visible) | 0 | 0.1 | 0.0 | 0.0 | 633 | 889 | 172 |
| 6 | longest | WEAK-PVS (0.99 of all-visible) | 0 | 0.1 | 0.0 | 0.0 | 982 | 1378 | 214 |
| 7 | pri2 | WEAK-PVS (0.93 of all-visible) | 0 | 0.3 | 0.0 | 0.2 | 4076 | 5873 | 924 |
| 8 | babybloodgulch | WEAK-PVS (0.93 of all-visible) | 0 | 0.3 | 0.0 | 0.2 | 4983 | 6417 | 1392 |
| 9 | narsp | WEAK-PVS (0.93 of all-visible) | 0 | 0.2 | 0.0 | 0.2 | 4468 | 5876 | 1352 |
| 10 | skyRingbl2 | WEAK-PVS (0.92 of all-visible) | 0 | 0.3 | 0.0 | 0.3 | 9194 | 11695 | 2762 |
| 11 | lock | WEAK-PVS (0.92 of all-visible) | 0 | 0.3 | 0.0 | 0.1 | 3941 | 5532 | 1008 |
| 12 | Spider | WEAK-PVS (0.91 of all-visible) | 0 | 0.2 | 0.0 | 0.2 | 1539 | 2200 | 147 |
| 13 | lockdown | WEAK-PVS (0.87 of all-visible) | 0 | 0.4 | 0.0 | 0.2 | 5901 | 7588 | 2142 |
| 14 | base | WEAK-PVS (0.85 of all-visible) | 0 | 0.2 | 0.0 | 0.0 | 785 | 998 | 170 |
| 15 | citadel | WEAK-PVS (0.66 of all-visible) | 0 | 0.2 | 0.0 | 0.1 | 3222 | 4387 | 860 |
| 16 | pit2 | WEAK-PVS (0.65 of all-visible) | 0 | 0.6 | 0.0 | 0.4 | 12218 | 15858 | 3857 |
| 17 | +bloodgulch2 | WEAK-PVS (0.62 of all-visible) | 0 | 0.3 | 0.0 | 0.2 | 3239 | 4238 | 892 |
| 18 | pit3 | WEAK-PVS (0.61 of all-visible) | 0 | 0.6 | 0.0 | 0.4 | 11179 | 14480 | 3436 |
| 19 | fire | ok | 0 | 0.1 | 0.0 | 0.1 | 982 | 1425 | 249 |
| 20 | Wizard | ok | 0 | 0.2 | 0.0 | 0.2 | 2776 | 3641 | 760 |
| 21 | highground | ok | 0 | 0.6 | 0.0 | 0.2 | 10184 | 11911 | 6365 |
| 22 | narrows | ok | 0 | 0.2 | 0.0 | 0.6 | 10995 | 15021 | 3140 |

## Details (worst first)
- **Cestra.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | big tex: sand_red102 1024x1024(58 faces) | hotspots(256u): (-2560,3328,-512):175; (-3584,2560,-512):170; (-3584,2048,-512):166
- **lane.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | big tex: none | hotspots(256u): (-512,768,-1536):150; (-256,-768,-1536):149; (-512,-512,-1536):133
- **pit.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | big tex: none | hotspots(256u): (768,256,-256):178; (-768,256,-256):140; (-256,768,-512):128
- **chill.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | big tex: none | hotspots(256u): (-256,-1280,0):146; (0,256,-256):83; (-256,256,0):63
- **plaza.bsp** vis-ratio 1.01 WEAK-PVS (1.02 of all-visible) | big tex: none | hotspots(256u): (256,0,-256):88; (256,-1024,-256):77; (0,0,-256):55
- **longest.bsp** vis-ratio 0.99 WEAK-PVS (0.99 of all-visible) | big tex: none | hotspots(256u): (1280,-256,-512):108; (2048,0,-512):63; (1792,0,-512):53
- **pri2.bsp** vis-ratio 0.93 WEAK-PVS (0.93 of all-visible) | big tex: none | hotspots(256u): (-768,-256,0):207; (-1280,-512,256):170; (-512,-1024,0):158
- **babybloodgulch.bsp** vis-ratio 0.93 WEAK-PVS (0.93 of all-visible) | big tex: none | hotspots(256u): (0,-512,-256):62; (512,512,-256):61; (-512,-1536,-256):55
- **narsp.bsp** vis-ratio 0.93 WEAK-PVS (0.93 of all-visible) | big tex: none | hotspots(256u): (1536,-2048,-2048):132; (1024,-2304,-2048):106; (1024,-1024,-2048):89
- **skyRingbl2.bsp** vis-ratio 0.92 WEAK-PVS (0.92 of all-visible) | big tex: none | hotspots(256u): (-2048,-512,768):956; (-512,-512,768):838; (-2304,-1024,768):334
- **lock.bsp** vis-ratio 0.92 WEAK-PVS (0.92 of all-visible) | big tex: none | hotspots(256u): (0,-1280,256):121; (256,-512,0):82; (-768,-1280,0):73
- **Spider.bsp** vis-ratio 0.91 WEAK-PVS (0.91 of all-visible) | big tex: none | hotspots(256u): (768,-256,256):28; (1024,-256,256):25; (768,-768,512):22

## Read-out / recommended actions
- TEXTURE SIZE IS A NON-ISSUE for this set: zero menu maps use >1024px textures (Cestra: one 1024). gl_max_size caps save <2MB total; not worth a batch.
- **4 maps have an EMPTY visibility lump (no PVS at all)**: Cestra.bsp, lane.bsp, pit.bsp, chill.bsp. Engine falls back to all-leafs-visible (gl_rmain R_MarkLeaves) -> world culling AND dlight MarkLights walks degrade to whole-map every frame. Re-running `vis` on these .map sources is the single biggest map-content fix and amplifies B3 (dlight cull) and B9 (sub-pixel cull).
- WEAK-PVS ratios ~0.6-1.0 on small open DM maps are largely geometric truth, not leaks; the NO-VIS rows above are the real defect class.
- 'construction' is in the matchmaking menu but no runtime .bsp exists in solitude/maps (only in legacy backup) - menu selection errors out; fix menu or ship map.
- Worst overall content map: Cestra (no PVS + 9473 faces + 0.9MB lightmaps + 1024 tex). Heaviest well-built maps: narrows/pit2/pit3/highground (10-12k faces, good vis).
- Vertex hotspots (256u cells, coords in Details) are TB-jumpable for face-merge passes if a specific area tests slow in B1 - don't blanket-remap.

## In menu but NO runtime BSP found: construction
