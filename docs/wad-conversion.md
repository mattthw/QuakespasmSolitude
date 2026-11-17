# WAD conversion (PNG/TGA → .wad)

Verified: 2026-11-17 for the format facts + this repo's conventions; the
**converter incantations are *Unverified*** — the tools named are candidates,
not tested on this machine. Record what you actually ran in the wad's commit
message so the next person inherits truth.

## WAD2 format (what qbsp/vis/light + the engine read)

- File: magic `WAD2`, 12-byte header, lump directory (32 bytes/lump), then
  miptex data. Each texture = one lump named exactly as the face references
  it (16 chars max, incl. the `+ * {` prefix characters).
- Miptex: 16-byte name, flags (ignored by our renderer), width/height,
  4 mip offsets, 16-byte **anim-name** field (`+N2 frames`-style metadata for
  `+` groups), nextoffset; pixel data = indexed bytes, then mips, then a
  768-byte palette + 2-byte terminator.
- Practical rules: **64×64** is our house size (power of two required);
  transparency = index **255** only; mip chain expected by editors even
  though the engine only uploads level 0 — let the converter generate it.

## Converters (candidates)

| Tool | Platform | Status |
|---|---|---|
| **any2wad** | Windows | *Unverified here* — common choice, batch PNG→wad |
| **Quake Texture Tool** | Windows | *Unverified here* — visual wad editor |
| Marco's q1tools (`tga2dm8`, lmp/targa converters) | C, macOS buildable | rehosted in `solitude/artwork/` (README links source) |
| Custom Python wad writer | this repo | **none committed yet** — a ~60-line writer (struct.pack miptex + naive mips) is the gap; if you add one, put it in `tools/` and note the name in the wad's commit message |

Export path check, any tool: open the produced wad in **TrenchBroom's texture
browser** (qbsp is the real consumer) and eyeball transparency — a converter
that puts transparency on the wrong index reads as a solid black quad in-game.

## This repo's conventions

- Tracked wads: `assets/textures/wads/*.wad`. Big vendored sets
  (Makkon0–3 etc.) stay local under `solitude/local/vendored/wads/`, reached
  via the repo-root `wads/` symlink; map `wad` keys are repo-relative —
  **qbsp must run from the repo root** or nothing resolves.
- **Proxy-lump convention** (external-PNG art, e.g. the lockdown beam kit):
  real art lives in `solitude/textures/<map>/`; the wad carries matching
  **64×64 stub lumps** with the exact face names (`{beam_bl`, `*beam_yl`,
  `+0beam_yl`…) purely so TB/qbsp can resolve faces. Stubs can be crude — the
  engine overrides the pixels with the sidecar PNG at load. See
  `assets/textures/wads/lockfx.wad` as the worked example.
- New wad referenced by a map → add its relative path to the map's `wad` key
  (worldspawn) or qbsp fails at the first face.
- Release paks: decision is one deterministic `PAK0.PAK` published as a
  GitHub release asset; wads themselves ship loose under the game dir
  (qbsp-time use) — runtime never needs the wad when faces are embedded.

## After converting

1. Face-check in TB (texture browser + apply to a test brush).
2. Compile the lab map (`beamtest` pattern) from repo root.
3. In-game on Mac (`./mac.sh launch <map>`), then Vita before trusting —
   animated `+` groups and fence alpha have historically rendered differently
   on vitaGL.
