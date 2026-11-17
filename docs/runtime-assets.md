# Runtime assets (where the game actually looks)

Verified: 2026-11-17 against this fork's source + device behavior.

## Layout under the game dir (`solitude/`)

```
solitude/
├── quake.rc          # startup script: exec default.cfg + config.cfg, gl commands
├── default.cfg       # shipped defaults (don't put personal binds here)
├── config.cfg        # PER-USER archived cvars/bindings — rewritten at quit,
│                     #  never committed, NEVER pushed to device
├── config-mac.cfg    # Mac-only dev overrides — never committed/pushed
├── PAK0.PAK PAK1.PAK # released pak = deterministic build, sorted names
├── gfx/              # HUD/menu art, fonts (gfx/fonts/*.ttf), maps/<icon>.tga,
│                     #  MENU/ page backgrounds, palette.lmp
├── progs/            # models (v_*.mdl viewmodels, *.md3 + external skins)
├── progs.dat         # compiled QuakeC (from qcc-src/, tracked binary)
├── maps/             # <name>.bsp (+ <name>.lit sidecar — TRACKED, not derived)
├── data/maps/        # bot waypoint files (<name>.way) — see below
├── textures/         # external PNG sidecar overrides (textures/<map>/<name>.png)
├── music/            # bgm_extmusic files (console: music <file>)
└── gfx/wads...       # (wads are compile-time only; runtime uses embedded face data)
```

Load order facts: `quake.rc` runs at engine init **after** menu cvars are
created, so `CVAR_ARCHIVE` cvars defined in `menu.c` do receive `config.cfg`
values at boot. Loose files override pak contents — a loose `quake.rc` or
`progs.dat` on the device shadows the packed copy (that's how the double-music
fix ships).

## File-sandbox rules (QuakeC builtins)

`fopen`/file writes from QC (extension builtins, `pr_ext.c`) go through
`QC_FixFileName`: absolute paths / `..` are rejected and **`data/` is
prepended**. So QC-saved files (bot waypoints via `PrintWaypoint`) land in
`data/…`; the console/loader side must `exec data/maps/<name>.way`. When
adding any QC file feature, save+load must both use the `data/`-prefixed
form.

## Screenshot / console capture

- `screenshot png` → `spasmNNNN.png` in the game dir (gitignored).
- Mac runs with `-condebug`: the console log lives at
  `toolchain/fork-engine-mac/qconsole.log` (NOT the game dir).
- Photo mode: `hud_mapshot 1` unlocks noclip/fly and suppresses bot populate;
  comma bound to screenshot in that mode.

## Vita device layout

`ux0:/data/Quakespasm/Solitude/` mirrors the game dir above; vpk installs from
`ux0:/download/`. Transfer = **USB only** (VitaShell Select → `diskutil mount`
→ cp → readback md5 → unmount → Select). The FTP suprx (ports 1337/1338) has
crashed the device repeatedly — do not use it. `lb_scale` differs by platform
(`#ifdef` in `gl_screen.c`: Vita 1.05, Mac 1.3).
