# Muzzle flash generation (B10-muzzle)

Sprites in `solitude/sprites/` are **generated**, not hand-painted. The
tool is `tools/gen_muzzleflash.py` (pure stdlib Python 3, no PIL). It
renders a procedural powder-burn field (bright symmetric core + tapered
tongues, per-frame decay and turbulence), maps it through palette ramps,
and writes a Quake SPR group sprite plus a PNG contact-sheet preview.
Every output is self-validated by re-parsing the file with a strict
reader that mirrors `Mod_LoadSpriteModel` in `quakespasm/Quake/gl_model.c`
-- if `_check_spr` prints `self-check passed`, the engine will load it.

## Reproduce the shipped set (current look)

Run from the repo root. All five use the 128px canvas, orientation 4
(`SPR_VP_PARALLEL_ORIENTED` -- obeys `angles_z` roll jitter set in QC)
and `--rgba` (SPR version 32: identical container, 4-byte RGBA pixels,
drawn with per-sprite alpha blending by `r_sprite.c`; indexed v1 sprites
keep the old global alphatest path instead).

```sh
P="--size 128 --orientation 4 --rgba"
/usr/bin/python3 tools/gen_muzzleflash.py $P --shape claw    --ramp gunpowder --cscale 0.50 --frames 6 --interval 0.0167 --seed 7  --out build/sprites/muzzle_ar.spr      --preview build/sprites/muzzle_ar.png
/usr/bin/python3 tools/gen_muzzleflash.py $P --shape radial  --ramp gunpowder --cscale 0.58 --frames 5 --interval 0.018  --seed 2  --out build/sprites/muzzle_shotgun.spr --preview build/sprites/muzzle_shotgun.png
/usr/bin/python3 tools/gen_muzzleflash.py $P --shape claw    --ramp gunpowder --cscale 0.60 --frames 5 --interval 0.018  --seed 12 --out build/sprites/muzzle_sshotgun.spr  --preview build/sprites/muzzle_sshotgun.png
/usr/bin/python3 tools/gen_muzzleflash.py $P --shape claw    --ramp gunpowder --cscale 0.42 --frames 4 --interval 0.0167 --seed 3  --out build/sprites/muzzle_pistol.spr    --preview build/sprites/muzzle_pistol.png
/usr/bin/python3 tools/gen_muzzleflash.py $P --shape radial  --ramp needler   --cscale 0.50 --frames 4 --interval 0.0167 --seed 11 --out build/sprites/muzzle_needler.spr   --preview build/sprites/muzzle_needler.png
```

Machinegun reuses `muzzleflash.spr` (shotgun art) -- no separate file.
`--old <file.spr>` adds a bottom row of the previous sprite's frames to
the preview for side-by-side scale comparison (the 32px originals are
still on disk as `*-old.spr`).

## Deploy mapping (build/sprites -> solitude/sprites)

| generated | runtime file | used by |
|---|---|---|
| `muzzle_ar.spr` | `muzzleflash1.spr` | AR (`muzzleflash2`) + sniper (`muzzleflashsniper` - same art, own long-barrel offset) |
| `muzzle_shotgun.spr` | `muzzleflash.spr` | shotgun (`muzzleflash3` @1277), shottie (@1234), rocket launcher (@1674), machinegun (`muzzleflash7` via the per-weapon `muzzleflash()` dispatcher in `effects.qc:421`) |
| `muzzle_sshotgun.spr` | `muzzleflash2.spr` | super shotgun (`muzzleflash4`) |
| `muzzle_pistol.spr` | `pistolmuzzleflash.spr` | pistol (`muzzleflash5`) |
| `muzzle_needler.spr` | `muzzleN.spr` | needler (`muzzleflash6`) |

**Names are exact and case-sensitive** -- QC looks up
`sprites/pistolmuzzleflash.spr` lowercase (the old tracked file was
capital-`P` for years, so the pistol had no flash at all; fixed in
0d8ad903). After `cp`, just relaunch -- sprites load at runtime, no
rebuild needed. Vita pickup needs a device sync of `solitude/sprites/`
(USB method; never push `config-mac.cfg`).

## Tuning knobs

- `--cscale` art size *within* the canvas (wastes texels if <1; the real
  on-screen size knob is the pair below).
- `r_spritemuzzlescale` (engine cvar, CVAR_ARCHIVE, default 1 = stock):
  draw-time scale applied in `R_DrawSpriteModel` only to models whose
  name contains "muzzle". Mac ships 0.65 in `config-mac.cfg`; on-screen
  footprint = 128 world units x this. Sprite pixel density and world
  footprint are coupled at load time -- this cvar is the decoupler.
- Alpha feather: `alpha = ((v - 0.03) / 0.30)^0.6` in
  `render_frame_rgba`; raise the 0.30 for a softer edge.
- Tongue geometry (length/spread/wobble): `make_tongues` + `v_claw`
  constants, tuned per rounds 6-7 feedback (short uniform spikes,
  symmetric core so roll jitter reads straight).
- Per-shot variation: header `synctype=rand` gives each entity a random
  animation start phase; QC adds +-20 deg roll (`angles_z`).
- Animation speed: the loader **cumulates** the per-frame interval
  floats; each must be > 0 (shipped 0.0167 ~= 10 fps-ish).

## Format recap (SPR group, what the tool writes and engine reads)

```
int   IDSP magic + version (1 indexed | 32 RGBA)
int   orientation (4 = VP_PARALLEL_ORIENTED)
float beamlength = hypot(size,size)/2   <- the "odd float" in headers
int   maxwidth, maxheight, numframes=1, synctype
int   frametype=1 (SPR_GROUP), numsubframes
float intervals[numsubframes]           (cumulated, all > 0)
per subframe: int x, y, width, height; pixels (w*h | w*h*4 if v32)
```

Palette source: `solitude/gfx/palette.lmp` (768 raw bytes) for the v1
path; RGBA mode interpolates the same ramp colors directly.
