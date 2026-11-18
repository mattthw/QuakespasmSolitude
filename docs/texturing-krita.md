# Texturing (Krita → game)

Verified: 2026-11-17 against mainline tip `ac83c967`; engine behavior checked
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

## Which rules apply where (read this first)

The Quake palette is a **WAD-lane-only** concept. External sidecars never touch
it — full 24-bit color, no palette, no palette-index rules. Concretely:

| Rule | WAD (`assets/textures/wads/*.wad`) | External sidecar (`solitude/textures/`) |
|---|---|---|
| Quake 256-color palette | **applies** — pixels are palette indexes | does **not** apply — real RGB stored |
| Index **255** = transparent | **applies** (the only alpha a wad has) | irrelevant — use a real **alpha channel** instead |
| Fullbright = bottom 32 palette rows | **applies** (wad + MDL skins) | does **not** apply — use a `<name>_glow.png` sibling instead |
| Dithering to fake soft edges | needed (binary alpha only) | **not needed** — smooth alpha gradients are free |
| Name prefix (`{`, `*`, `+`) | applies | applies — the prefix is on the *face*, the sidecar just replaces its art |
| RGBA alpha honored | n/a | **only on `{` and `*` faces** — plain faces treat the PNG as opaque |

So: working purely on sidecars (PNG/TGA overrides), you can ignore palette
imports entirely — paint freely, add an alpha layer, export RGBA. The palette
only matters again when a converter quantizes your art into a wad.

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

## Working in Krita from scratch (first-run path)

No prior Krita needed. Krita is a free raster editor (krita.org; installed on
this Mac as `/Applications/krita.app`). For Quake work you touch about five
features total:

1. **New document** — `File > New`. Set **Width/Height to 64, 128, or 256 px**
   (always power-of-two; 64 is the house unit). Set color model **RGB, 8
   bits/channel integer**. Leave DPI irrelevant, background **transparent**.
2. **Zoom to pixels** — `View > Zoom > 800%` (or ⌘+/⌘-); turn on
   `Settings > Krita Settings > Preferences > General > Pixel Snap` if you
   draw rectangles/hard shapes, and the pixel grid under `Settings > Dockers >
   …` is optional. Work one screen-zoomed tile at a time.
3. **Layers** — right panel. One `PaintLayer` per material detail is fine;
   erase with `E` (eraser) on a transparent background. For a cutout, just
   delete pixels — the checker pattern IS the alpha you export.
4. **Seamless tiling check** — `View > Wrap-Around Mode` (shortcut **W**)
   paints edges opposite-side live so seams are visible while you work. Turn
   it off with W again before export.
5. **Reference swatches** — for wad-lane art, drop the Quake palette on screen:
   open `assets/quake_palette/quake1palette.png` as a second document
   (`File > Open as New Image` is fine) and Alt-click to sample, OR load the
   real palette: `Settings > Dockers > Palettes` docker → hamburger menu →
   `Load Global Palette…` accepts **`.ase`, `.pal`, `.act`** — all three ship
   in `assets/quake_palette/` (`quake1palette.ase` / `.pal` / `.act`). Pick any
   — same 256 colors. Sidecar-only artists can skip this entirely (see "Which
   rules apply where").

## Import — what Krita reads

| Input | Format | How |
|---|---|---|
| Existing game texture as PNG | any | `bsputil -export <map>.bsp` dumps every miptex (see [mapping-tb.md](mapping-tb.md)); or grab the sidecar `.tga`/`.png` straight from `solitude/textures/` |
| Quake palette | `.ase` / `.pal` / `.act` | Palettes docker → Load Global Palette (files under `assets/quake_palette/`) |
| Photo / concept reference | PNG/JPG/TGA | File > Open; desaturate + grade toward palette by eye for wad work |

Krita cannot open `.wad` directly — nothing to import from wads; always go
through the exported PNGs above.

## Export — the settings that matter

Krita never writes `.wad`; you produce **PNG (or TGA)** and either ship it as
a sidecar or run it through [wad-conversion.md](wad-conversion.md).

| Decision | Value | Why |
|---|---|---|
| Format | **PNG** default; **TGA (32-bit uncompressed)** for large wall art | PNG = pak-friendly; uncompressed TGA skips the Vita's zlib decode at map load (budget section below) |
| Bit depth | **8 bits/channel**, RGBA (alpha on if `{`/`*` lane) | engine ignores 16-bit; 8-bit RGBA is exactly the upload format |
| Interlace | **off** | progressive PNGs decode slower and gain nothing here |
| Compression level | anything | only affects file size/decode ms marginally — never quality |
| Flatten | yes (single layer) | `Image > Flatten Image` before `File > Export` |
| Dimensions | **64/128/256 max**, power-of-two | non-square or non-POT sidecars get resampled/padded by the engine at load (wasted CPU) |
| Alpha content | `{`/`*` lanes: real gradient alpha is fine; everything else: fully opaque | RGBA alpha is only honored on `{` and `*` faces |
| Export path | straight into `solitude/textures/<mapname>/` (or `/textures/` to override on EVERY map — intentional only), named exactly the face name (`{`/`+N` prefixes included; `*tex` → `#tex.png`) | case-sensitive resolution; a misnamed sidecar silently does nothing |

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
