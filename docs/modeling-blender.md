# Modeling (Blender)

Future-route page, created 2026-11-17. Most content here is **verified via
[../model-src/README.md](../model-src/README.md)** — that file is the
detailed, authoritative notes for the Blender path; this page just indexes it.

## Quick facts (from model-src, verified in engine source)

- Add-ons: `khreathor/mdl-for-blender` (`io_mesh_qfmdl`, maintained through
  Blender 4.1+) or `robrohan/blender_ie_quake_mdl`. Export: File → Export →
  Quake 1 (MDL).
- MDL = vertex-animated, 8-bit palette skins (keep art out of the last 32
  palette entries — they're fullbright and glow).
- MD3 + external PNG skins = real alpha (see-through foliage route).
- 1 Blender unit = 1 quake unit; viewmodel/monster floor convention z = -24;
  map props base at z = 0 planted by entity origin.
- Reuse brush trees as models: compile a throwaway map, `bsputil -export`
  → OBJ + miptex PNGs → Decimate 0.3–0.5 → join → one UV atlas → export
  (full recipe in model-src README).

## Layout / shipping

- `model-src/<name>/<name>.blend` = source of record (tracked),
  `skins/` = pre-quantization authoring PNGs.
- Ship compiled to `solitude/progs/` (+ external skin PNGs for the MD3
  route). Nothing in `model-src/` is loaded at runtime.

## Status

Placeholder until the first Blender-made asset ships; then replace with a
worked example walkthrough and update the Verified line.
