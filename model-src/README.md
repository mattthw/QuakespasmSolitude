# model-src — Blender → MDL/MD3 sources

Editable 3D sources (.blend, skin work PNGs, decimated exports) for models that
ship compiled into `solitude/progs/`. Nothing here is loaded by the engine;
ship the .mdl/.md3 (+ external skin PNGs for md3) into `solitude/progs/`.

## Engine support (this fork, verified in source)

- `misc_model` point entities work WITHOUT any QuakeC (pr_edict.c:1093 →
  pr_cmds.c:1748 `PR_spawnfunc_misc_model`): reads `model` (or `mdl`) key,
  precaches, makes the edict static. `angle -1` = random yaw per spawn
  (scatter forest for free). Origin is the model's 0 0 0.
- Classic Quake 1 **MDL** = vertex-animated, 8-bit palette skins, one skin
  per surface — effectively opaque skins. Fine for trunks/props.
- **MD3** is loadable (gl_model.c IDP3 header dispatch → Mod_LoadMD3Model)
  with external PNG/TGA skins, real alpha + `_luma` fullbright overlays.
  This is the path for see-through foliage.

## Toolchain

- Blender add-on: khreathor/mdl-for-blender (`io_mesh_qfmdl`), maintained
  through Blender 4.1+; robrohan/blender_ie_quake_mdl tracks newer Blender.
  Export = File → Export → Quake 1 (MDL).
- Blender-unit to quake-unit is 1:1; floor convention at z = -24 for
  viewmodels/monsters (map props: put the base at origin z 0 and use the
  entity origin to plant it).
- Keep skins inside the first 224 palette entries — the last 32 are
  fullbright and pop as glowing speckles if used by accident.

## Exporting existing brush trees out of TrenchBroom

1. Copy the tree brushes into a throwaway map (`tools/scratch-tree.map`)
   as worldspawn, compile with qbsp (from repo root so `wad` keys resolve).
2. `toolchain/ericw-tools/bsputil -export scratch-tree.bsp -firstface 0
   -numfaces <n>` gives OBJ + miptex PNGs of the alpha foliage you can
   reuse as skins (or take the {textures straight from the wads).
3. Import the OBJ into Blender, `Decimate` (collapse ~0.3–0.5 is normal
   for foliage crosses), join to one mesh, one UV atlas, export.

## Layout

- `model-src/<name>/<name>.blend` — source of record (tracked)
- `model-src/<name>/skins/` — authored PNG palettes before quantization
- ship → `solitude/progs/<name>.mdl` (+ `solitude/progs/<name>.md3`
  and `<name>/*.png` external skins if md3 route)

## TODO

- Add `misc_model` (+ `mdl`, `scale`, random-yaw note) to
  `assets/models/QuakespasmSolitude.fgd` so TB offers it in the entity browser.
