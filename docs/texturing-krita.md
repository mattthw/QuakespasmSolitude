# Texturing (Krita → game)

Verified: 2026-11-17 against mainline tip `1a1d74c4`; engine behavior checked
Mac + Vita. Prefix/sidecar rules transcribed from the repo README
("Texture Effects") which was itself verified against `gl_model.c` /
`r_brush.c`.

## Two texture lanes

| | WAD lane (classic) | External PNG lane (sidecar) |
|---|---|---|
| Lives in | `assets/textures/wads/*.wad` (tracked) | `solitude/textures/<mapname>/<name>.png` |
| Color | 8-bit indexed, Quake palette | full RGB, RGBA alpha honored (see prefix rules) |
| Needs qbsp | yes — face names resolve from `wad` keys | no recompile for art changes |
| qbsp/TB still needs a stub | — | yes: keep a same-named lump (proxy) in a wad |
| Glow | compile-time `_surface` light entities | `<name>_glow.png` / `_luma.png` sibling, additive |

`gl_load24bit` defaults to **1** (archive cvar, `gl_model.c`) — sidecar
loading is on out of the box. The palette at `solitude/gfx/PALETTE.LMP`
(768-byte RGB) has authoring provenance under `assets/quake_palette/`;
`gfx/PALETTE.LMP` is the runtime one — keep them in sync if either changes.

**In WADs, alpha is binary**: palette index **255** = transparent pixel,
everything else opaque (`gl_texmgr.c:551`). There is no partial alpha inside a
WAD texture — soft edges need dithering to index 255 (see
`tools/gen_muzzleflash.py` for the Bayer-dither pattern we generate) or the
`{` prefix + sidecar PNG route below.

## Name prefixes drive rendering (first character)

- `+0name` … `+9name` — animated group, 5 fps, engine cycles at runtime.
  Faces reference frame-0 (`+0beam`). `A`–`J` is an alternate sequence.
  **`+` is opaque** — cannot combine with `{`. Example: lockfx.wad ships the
  first in-pack animation (`+0beam_yl`…); highground's waterfall
  (`+0_fall`…`+7_fall`) predates this repo era.
- `*name` — warp (sine-warped each frame; classic water/lava). Trap: **qbsp
  stamps `*`-faced leaves CONTENTS_WATER** → swim physics + splash sfx. For a
  warped *effect* (beam, force field), put the geometry in a
  `func_illusionary` with `"_mirrorinside" "1"` so it never gets liquid
  contents and renders from inside.
- `{name` — alpha/fence texture, drawn in the transparent pass, **no warp**.
  This is the ONLY transparency marker this renderer checks — the WAD2 miptex
  flag byte is ignored.
- `sky*` skybox; `skip`/`clip` compile-time only.

Sidecar file naming: `{tex` → `{tex.png`; `*tex` → **`#tex.png`** (star
becomes hash). Both `textures/<mapname>/` then `textures/` resolve, across the
engine searchpath. RGBA alpha respected on `{` and `*` faces only — a plain
sidecar PNG is treated opaque.

## Working in Krita (workflow notes)

Canvas at the wad texel size — **64×64** is the house unit (everything in our
wads; TB grid aligns at 16). *Unverified against a specific Krita version:*
Krita's indexed/gamma workflow changes between releases, so treat this as
policy, not clicks:

1. Work RGB, snap colors toward the Quake palette by eye (or sample from
   existing wad exports — `bsputil -export` dumps miptex PNGs of any bsp).
2. Reserve one swatch = palette entry 255 for transparency; when exporting
   indexed, "background/transparent" index must land on **255** — many
   converters default it to 255 automatically (check yours).
3. Keep non-fullbright art out of the bottom 32 palette rows — in MDL skins
   those are the glowing band (see [../model-src/README.md](../model-src/README.md)).
4. Export PNG → convert via [wad-conversion.md](wad-conversion.md), or ship
   as a sidecar with proxy lumps in a wad.

## External sidecar budget (size discipline)

Rule of thumb: **≤256 px max edge for anything the player can press their face
against, 128 px otherwise.** At the Vita's 960×544 you will not see more, and
oversize sidecars cost real money. Measured 2026-11-17 (code + scan):

- **Per-frame GPU cost: zero difference.** Wad and external textures converge
  in `TexMgr_LoadImage32` — both upload as RGBA8. A big texture displayed small
  samples via mips ≈ the same texels as a small one.
- **Load-time cost: externals pay extra.** Wad lumps ship pre-built mips that
  upload straight through; every sidecar gets its pixels decoded (zlib inflate
  for PNG, none for uncompressed TGA type 2 — the reason our sidecars are TGA)
  *and* a CPU box-filter mip chain *plus* an `AlphaEdgeFix` pass per mip level
  for `{` textures. Cost scales with pixel count: a 512² sidecar is ~64× the
  load work of a 64px wad lump.
- **VRAM on Vita is a cliff, not a slope.** 128 MB total, shared with OS, game
  heap, wad textures and lightmaps (lightmaps are the actual big eaters). The
  SGX driver does not thrash-and-swap like a desktop GPU — over budget means
  upload hitches, black textures or crash, not gradual fps loss. Keep sidecars
  trimmed so the headroom goes to entities/particles/dlights.
- A root-level `textures/<name>.<ext>` overrides that wad lump on **every map**,
  not just one — that's a feature; treat root placement as intentional, put
  experiments in `textures/<map>/`.
- Current inventory: 93 sidecars, ~9.3 MB on disk, ~14 MB worst-case VRAM.
  Known offenders from the 2026-11-17 scan: `textures/groundsm_red02.tga`
  (512², ROOT-level global override — suspected stray), and
  `textures/highground1/{map_hg_grnd_01,groundsm_red02}.tga` (512²) +
  `en_256_cliff_02.tga` (256×512). Downscale to ≤256² before the next
  highground1 ship. (`shield_overlay_*` at 256² are HUD-drawn near-fullscreen —
  correctly sized, not offenders.)

## Alpha-by-purpose cheat sheet

| Want | Use |
|---|---|
| Hard cutout (grating, vines) | `{name` + index-255 pixels in-wad |
| Soft fade (beam, smoke, force field) | `{name` + RGBA sidecar PNG (real alpha) |
| Motion without animating art | `*name` warp (mind CONTENTS_WATER trap) |
| Flipbook animation | `+0..+9name` group (each frame may be its own sidecar PNG too — every frame resolves its own texture name) |
| Emissive parts | `<name>_glow.png` sidecar |
