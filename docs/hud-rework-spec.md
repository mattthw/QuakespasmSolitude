# Solitude HUD rework — spec

Status: P0 (Mac baseline runnable) and P1 (unified `HUD_SCALE` design-space
model) **done** — committed `d42a44af`. This document is the working scope
for the rework proper. Batch order set by user 2026-11-12:

> "we can do font and shield and relocating of items first. warp as a p1 not p0"

Everything renders through `HUD_Draw()` (`gl_hud.c`), under
`HUD_SCALE = fit(960×544) × clamp(0.5–4, scr_sbarscale)` (`gl_draw.c`).
Positions are screen fractions; sizes are design-pixels × `HUD_SCALE`.
Vita at 960×544 is the reference device; desktop is the same canvas scaled.

## P2a — HUD layer (this batch)

1. **TTF font atlas** (`stb_truetype`, baked once at `HUD_Init`)
   - New API: `Draw_TTFString(x, y, ptsize, r,g,b,a, s, text)` next to
     `Draw_ColoredStringScale`; callers pass design-space sizes.
   - **Licensing finding:** Halo 3's actual UI face is **Conduit ITC**
     (commercial, MyFonts). Not embeddable without purchase.
     Open candidates (pick one, OFL/Apache only):
     - Big Shoulders Text, SemiBold Italic — closest Halo character
     - Archivo Narrow SemiBold — Trade Gothic metrics substitute
     - Roboto Condensed Bold — safest license, least character
   - Bundle chosen TTF + its license file in `assets/artwork/fonts/`.
2. **Shield as geometry** (user: "I want shape as geometry")
   - Small polygon-fill helper in `gl_draw.c` (triangle-fan, same immediate
     mode as everything else; no new textures).
   - Halo 3 behavior for `drawHealth` shield bubble:
     solid blue when healthy → **flash red when fully depleted** (not when
     high, per manual); keep the existing recharge/bleep FSM.
3. **Relocation pass** (Halo 3 multiplayer HUD manual, 2007)
   — **no icon art exists for weapons/grenades (user 2026-11-12: "I am just
   using their name"), so every "icon state" below is a TEXT-STATE rule:**
   - SCORE / Slayer / time cluster → **lower-right** (manual §multiplayer
     info); currently bottom-left (`glwidth*0.08`).
   - Weapon + ammo text → top-right (already 0.7/0.8-ish; align panel).
   - **Low ammo → weapon/ammo text flashes red** (rule from manual icon
     behavior, applied to text; threshold from cl.stats vs ammo max).
   - Grenades → text pair (count + name): **zero count → dim gray**,
     selected type keeps the blue; counts left of name (manual order).
   - Killstreak medals → stacked below the grenade row (currently
     left-mid via `getHudPicAttr(0.08, 0.5)`); medals ARE art (.lmp) already.
   - Helper pickup/swap prompt → anchored above the weapon panel.
4. **Crosshair** (`drawCrosshair()` is EMPTY — greenfield)
   - Client-side eye trace, team-color classify: red foe / blue neutral /
     green friendly (manual crosshair rule). No new art required to start:
     four short geometry ticks + center dot.
5. Decide the commented-out pieces in `HUD_Draw`:
   `MiniDeathmatchOverlay` → becomes the P2b motion tracker or dies;
   `"Its too quiet Chief"` announcer line → keep as easter egg or delete.

## P2b — motion tracker mechanic (later, separate decision)

Radar bottom-left per manual: velocity-gated (slow/stationary targets do
not appear — on friend *and* foe trackers), red dots foes / yellow
friendlies, elevation not distinguished. Needs QC/engine decision (server
velocity in entity data is available client-side already).

## Not mapped from Halo 3 (deliberately)

Dual-wield restrictions, equipment hold-X-deploy, Spike/Flame grenade
types, RB reload semantics — Solitude's weapon set differs; do not force.

## P1 (deferred, below P2a) — visor warp pass

Render `HUD_Draw` into an offscreen FBO (960×544 RGBA ≈ 2 MB VRAM), then
composite a fullscreen quad through a small fragment shader (barrel /
vignette / RGB-split). Fork currently has **zero FBO usage** — validate the
FBO path on Vita (GL_OES_framebuffer_object via vitaGL/SGX543) with a plain
barrel shader before art is added. No per-element rewrites needed:
everything already funnels through `HUD_Draw`.

## Mac preview discipline

Iterate with `../mac.sh all` (build → vita vpk → launch). Screenshots from
the Mac fork are the review medium; **device writes stay frozen** until the
user explicitly opens the gate. `mac.sh` auto-cleans object files — never
mix x86_64 and ARM `.o` in this tree.
