# Modeling (3ds Max)

Stub, created 2026-11-17. Max-side export specifics are **Unverified** — fill
in what you actually use; the engine-side facts below are verified
([../model-src/README.md](../model-src/README.md) is the reference).

## What the engine loads

- **MDL** (Quake 1 classic): vertex-animated, 8-bit palette skins, one skin
  per surface — effectively opaque. Fine for props/trunk geometry.
- **MD3**: loadable in this fork with **external PNG/TGA skins** — real
  alpha + `_luma` fullbright overlays. This is the route for anything
  translucent (foliage, glass, force-field shapes). Skins resolve next to the
  model / documented override paths in `model-src/README.md`.
- Placement: point entities via `misc_model` (`model` key — works with no
  QuakeC; `angle -1` = random yaw), or weapon/view models precached by QC
  (`progs/v_*.mdl` convention).
- Scale: Blender uses 1 unit = 1 quake unit; if Max exports differently,
  fix at export, not with entity `scale`.

## To fill in

- Max version + which exporter/plug-in (or intermediate FBX→Blender→export
  path) actually round-trips animation into Quake 1 MDL here.
- Skin pipeline from Max render/export to the palette (link to
  [texturing-krita.md](texturing-krita.md) rules once proven).
- One worked example commit to point at (e.g. rebuild of an existing
  `progs/v_*.mdl` from source, byte-checked in-game).
