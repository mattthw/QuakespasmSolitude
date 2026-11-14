# B0 map audit - 24 matchmaking-menu maps (BSP29, read-only)

Score = leak(50) + 20*PVS-weakness + 0.5*lightmap-MB + 2*count(>=1024 tex) + 0.2*gl_max_size512-savings-MB + 5*hotspot-density-ratio(cap6)
Hotspots: 256-unit cells; 'Nd (Xx avg) Mf' = N verts, X times the map's mean verts/cell, M faces anchored in cell; tex = top textures there.

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
| 15 | pit2 | WEAK-PVS (0.65 of all-visible) | 0 | 0.6 | 0.0 | 0.4 | 12218 | 15858 | 3857 |
| 16 | +bloodgulch2 | WEAK-PVS (0.62 of all-visible) | 0 | 0.3 | 0.0 | 0.2 | 3239 | 4238 | 892 |
| 17 | pit3 | WEAK-PVS (0.61 of all-visible) | 0 | 0.6 | 0.0 | 0.4 | 11179 | 14480 | 3436 |
| 18 | citadel | WEAK-PVS (0.66 of all-visible) | 0 | 0.2 | 0.0 | 0.1 | 3222 | 4387 | 860 |
| 19 | Wizard | ok | 0 | 0.2 | 0.0 | 0.2 | 2776 | 3641 | 760 |
| 20 | fire | ok | 0 | 0.1 | 0.0 | 0.1 | 982 | 1425 | 249 |
| 21 | highground | ok | 0 | 0.6 | 0.0 | 0.2 | 10184 | 11911 | 6365 |
| 22 | narrows | ok | 0 | 0.2 | 0.0 | 0.6 | 10995 | 15021 | 3140 |

## Hotspot details (all maps, worst first)
- **Cestra.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | hotspots: (-2560,3328,-512) 175v (17.5x avg) 79f [hum_barricadel x66; en_128_rock_03 x7] || (-3584,2560,-512) 170v (17.0x avg) 90f [en_128_cliff_02 x46; sky_blue_bright x13] || (-3584,2048,-512) 166v (16.6x avg) 76f [en_128_rock_03 x18; metal_black x14]
- **lane.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | hotspots: (-512,768,-1536) 150v (12.5x avg) 76f [? x26; en_128_cliff_01 x18] || (-256,-768,-1536) 149v (12.4x avg) 79f [en_128_cliff_01 x28; metal2_256 x24] || (-512,-512,-1536) 133v (11.1x avg) 67f [en_128_grass_01 x27; en_128_cliff_01 x15]
- **pit.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | hotspots: (768,256,-256) 178v (18.5x avg) 57f [floor_plain x45; wall_complex2 x7] || (-768,256,-256) 140v (14.6x avg) 51f [wall_plain x15; chain_fence x14] || (-256,768,-512) 128v (13.3x avg) 44f [floor_plain x12; wall_line_light x12]
- **chill.bsp** vis-ratio 99.00 NO-VIS (leaked/unwritten) | hotspots: (-256,-1280,0) 146v (13.0x avg) 67f [chillp x36; -0wlstucco x17] || (0,256,-256) 83v (7.4x avg) 47f [cemold4 x24; -0wlstucco x14] || (-256,256,0) 63v (5.6x avg) 30f [cemold4 x11; -0wlstucco x10]
- **plaza.bsp** vis-ratio 1.01 WEAK-PVS (1.02 of all-visible) | hotspots: (256,0,-256) 88v (8.0x avg) 22f [nm_concrete_flo x20; blue_concrete_p x2] || (256,-1024,-256) 77v (7.0x avg) 27f [blue_concrete_p x10; nm_concrete_flo x6] || (0,0,-256) 55v (5.0x avg) 17f [nm_concrete_flo x16; blue_concrete_p x1]
- **longest.bsp** vis-ratio 0.99 WEAK-PVS (0.99 of all-visible) | hotspots: (1280,-256,-512) 108v (12.5x avg) 42f [eshinytin x30; s20 x11] || (2048,0,-512) 63v (7.3x avg) 25f [eshinytin x20; text19 x4] || (1792,0,-512) 53v (6.1x avg) 18f [eshinytin x18]
- **pri2.bsp** vis-ratio 0.93 WEAK-PVS (0.93 of all-visible) | hotspots: (-768,-256,0) 207v (7.4x avg) 78f [cementwall x27; UnNamed4 x26] || (-1280,-512,256) 170v (6.0x avg) 71f [cementwall x22; unnamed20 x16] || (-512,-1024,0) 158v (5.6x avg) 57f [halo#143 x29; cementwall x17]
- **babybloodgulch.bsp** vis-ratio 0.93 WEAK-PVS (0.93 of all-visible) | hotspots: (0,-512,-256) 62v (7.7x avg) 18f [d_light_side x7; metal1_256 x5] || (512,512,-256) 61v (7.5x avg) 24f [en_128_grass_01 x6; d_wall1c_highqu x5] || (-512,-1536,-256) 55v (6.8x avg) 17f [d_light_side x6; d_wall1c_highqu x6]
- **narsp.bsp** vis-ratio 0.93 WEAK-PVS (0.93 of all-visible) | hotspots: (1536,-2048,-2048) 132v (10.3x avg) 55f [structuretex x25; sky_blue_cloudy x18] || (1024,-2304,-2048) 106v (8.3x avg) 42f [steel1_256 x21; met_floor_128 x16] || (1024,-1024,-2048) 89v (7.0x avg) 32f [steel1_256 x16; structuretex x9]
- **skyRingbl2.bsp** vis-ratio 0.92 WEAK-PVS (0.92 of all-visible) | hotspots: (-2048,-512,768) 956v (96.1x avg) 359f [metal1_256 x137; steel1_256 x72] || (-512,-512,768) 838v (84.3x avg) 376f [metal3_128 x177; sfloor1_2 x64] || (-2304,-1024,768) 334v (33.6x avg) 116f [metal3_128 x35; steel1_256 x32]
- **lock.bsp** vis-ratio 0.92 WEAK-PVS (0.92 of all-visible) | hotspots: (0,-1280,256) 121v (16.4x avg) 47f [base16_7 x40; snowup x5] || (256,-512,0) 82v (11.1x avg) 34f [base16_7 x27; snowup x5] || (-768,-1280,0) 73v (9.9x avg) 26f [base16_7 x17; snowup x6]
- **Spider.bsp** vis-ratio 0.91 WEAK-PVS (0.91 of all-visible) | hotspots: (768,-256,256) 28v (5.9x avg) 9f [metal1_256 x5; metal3_128 x2] || (1024,-256,256) 25v (5.2x avg) 9f [steel_1_128 x3; metal1_256 x3] || (768,-768,512) 22v (4.6x avg) 8f [metal1_256 x8]
- **lockdown.bsp** vis-ratio 0.87 WEAK-PVS (0.87 of all-visible) | hotspots: (512,0,0) 367v (12.8x avg) 143f [fore_128_wall_2 x99; sky_blue x10] || (512,-256,0) 334v (11.7x avg) 136f [fore_128_wall_2 x101; fore_256_wall_4 x15] || (-768,0,-256) 294v (10.3x avg) 106f [fore_128_wall_2 x61; sfloor1_2 x16]
- **base.bsp** vis-ratio 0.85 WEAK-PVS (0.85 of all-visible) | hotspots: (-768,0,0) 90v (10.6x avg) 44f [fore_128_door x18; metal4 x14] || (256,-256,0) 64v (7.6x avg) 28f [metal4 x17; steel_1_128 x7] || (0,512,0) 51v (6.0x avg) 26f [fore_128_door x19; metal4 x5]
- **pit2.bsp** vis-ratio 0.65 WEAK-PVS (0.65 of all-visible) | hotspots: (-512,0,-256) 455v (28.3x avg) 160f [en_32_number x52; wall_line_light x25] || (768,-256,-256) 450v (28.0x avg) 216f [en_32_number x71; fore_128_wall06 x42] || (-768,-768,-256) 450v (28.0x avg) 210f [human_128_bench x36; fore_128_wall06 x33]
- **+bloodgulch2.bsp** vis-ratio 0.62 WEAK-PVS (0.62 of all-visible) | hotspots: (-1280,-2048,-256) 59v (7.9x avg) 26f [metal1_256 x8; d_wall_no_strip x7] || (0,-512,-256) 59v (7.9x avg) 22f [metal2_256 x5; h_t1 x4] || (0,1536,-256) 56v (7.5x avg) 20f [h_t1 x9; c_cliff x7]
- **pit3.bsp** vis-ratio 0.61 WEAK-PVS (0.61 of all-visible) | hotspots: (-768,-768,-256) 449v (30.6x avg) 212f [human_128_bench x60; mmetal1_6 x33] || (512,-512,-256) 448v (30.6x avg) 207f [floor_plain x38; human_256_wall x23] || (768,-256,-256) 438v (29.9x avg) 201f [en_32_number x76; human_128_bench x27]
- **citadel.bsp** vis-ratio 0.66 WEAK-PVS (0.66 of all-visible) | hotspots: (-512,-512,0) 257v (4.7x avg) 98f [fore_256_floor1 x38; mmetal1_3 x31] || (-512,256,0) 233v (4.2x avg) 94f [fore_256_floor1 x43; mmetal1_3 x34] || (256,-512,0) 229v (4.2x avg) 90f [mmetal1_3 x40; fore_256_floor1 x30]
- **Wizard.bsp** vis-ratio 0.32  | hotspots: (1536,512,512) 46v (6.3x avg) 20f [hum_128_misc_02 x11; sky_blue x6] || (2048,-1536,512) 44v (6.0x avg) 25f [human_128_ceili x9; hum_128_misc_02 x7] || (2048,0,512) 42v (5.8x avg) 15f [hum_128_misc_02 x8; sky_blue x5]
- **fire.bsp** vis-ratio 0.41  | hotspots: (-768,-512,0) 60v (5.6x avg) 25f [hum128_concret_ x10; hum_wall_01 x4] || (-768,-1280,0) 55v (5.1x avg) 18f [hum_wall_01 x13; hum128_concret_ x3] || (-768,-1536,-256) 51v (4.7x avg) 19f [hum_wall_01 x10; hum128_concret_ x5]
- **highground.bsp** vis-ratio 0.10  | hotspots: (-1024,-1024,0) 519v (37.6x avg) 266f [{foliage x104; en_128_wood x73] || (-1024,-768,0) 509v (36.9x avg) 287f [en_128_rock_01 x73; groundsm_red02 x72] || (-3584,256,0) 430v (31.2x avg) 209f [en_128_cliff_01 x52; fore_128_wall_2 x26]
- **narrows.bsp** vis-ratio 0.01  | hotspots: (-1536,-768,-256) 130v (9.3x avg) 53f [fore_128_wall_3 x33; {hum128_ceil x9] || (1280,-768,-256) 122v (8.7x avg) 41f [hum_wall_01 x12; fore_256_wall_2 x8] || (3584,0,-256) 103v (7.4x avg) 39f [hum_wall_01 x21; floor_dark x5]

## Read-out / recommended actions
- TEXTURE SIZE IS A NON-ISSUE for this set: zero menu maps use >1024px textures (Cestra: one 1024). gl_max_size caps save <2MB total; not worth a batch.
- **4 maps have an EMPTY visibility lump (no PVS at all)**: Cestra.bsp, lane.bsp, pit.bsp, chill.bsp. Engine falls back to all-leafs-visible (gl_rmain R_MarkLeaves) -> world culling AND dlight MarkLights walks degrade to whole-map every frame. Re-running `vis` on these .map sources is the single biggest map-content fix and amplifies B3 (dlight cull) and B9 (sub-pixel cull).
- WEAK-PVS ratios ~0.6-1.0 on small open DM maps are largely geometric truth, not leaks; the NO-VIS rows above are the real defect class.
- 'construction' is in the matchmaking menu but no runtime .bsp exists in solitude/maps (only in legacy backup) - menu selection errors out; fix menu or ship map.
- Worst overall content map: Cestra (no PVS + 9473 faces + 0.9MB lightmaps + 1024 tex). Heaviest well-built maps: narrows/pit2/pit3/highground (10-12k faces, good vis).
- Vertex hotspots (256u cells, coords in Details) are TB-jumpable for face-merge passes if a specific area tests slow in B1 - don't blanket-remap.
- **Worst hotspot outliers** (verts/cell vs map mean): skyRingbl2 96x; highground 38x; pit3 31x; pit2 28x; pit 18x. skyRingbl2 is extreme (956 verts/359 faces in one 256u cell, z=768: the metal ring structure); highground 37x is {foliage/detail terrain; pit2+pit3 28-30x both driven by en_32_number (32px signage tex = hundreds of tiny faces); lockdown 13x fore_128_wall_2 carving. These inflate faces-in-PVS, draw calls and lightmap density in exactly the open-view lanes of outdoor maps -> prime targets for B4 sub-pixel cull AND map-side face merging with TB coords from Hotspot details.

## In menu but NO runtime BSP found: construction
