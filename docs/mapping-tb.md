# Mapping (TrenchBroom + ericw-tools, this repo)

Verified: 2026-11-17 against the repo README "Mapping Workflow Setup (Updated
2026-11-11)" + live TB config; ericw-tools **2.0.0-alpha11** at
`../toolchain/ericw-tools/` (no Homebrew formula exists — prebuilt Darwin zip,
`libtbb`/`libembree` dylibs must sit beside the binaries, runs under Rosetta).

## Launch

TB must be started from a terminal (or a launcher that carries your shell env)
— its compile tools rely on terminal variables:

```sh
/Applications/TrenchBroom.app/Contents/MacOS/TrenchBroom
```

Game path in TB preferences = **this repo** (`solitude/repo`). If you hand-edit
TB files under `~/Library/Application Support/TrenchBroom/`, quit TB first —
it rewrites `Preferences.json` on exit.

## Compile profile (current truth)

Map `wad` keys are repo-relative → **working directory must be the repo root**.

| Step | Command / params |
|---|---|
| Export | `${GAME_DIR_PATH}/${MAP_BASE_NAME}.map` (maps live under `assets/maps/<name>/`) |
| qbsp | `toolchain/ericw-tools/qbsp -wadpath …` — **do NOT use `-wadpath`**: it *prepends* to each `wad` key, and with repo-relative keys that produces bogus `archive … not found` warnings. Plain `${MAP_BASE_NAME}` from repo root resolves everything (verified: highground 52/52 textures, 0 warnings) |
| vis | `vis ${MAP_BASE_NAME}` (`-fast` exists; `-O` does **not** in alpha11) |
| light | `light -surflight_subdivide 64 ${MAP_BASE_NAME}` (advanced lighting → `.lit`) |
| Copy | bsp/lit/map → `solitude/maps/` **and** `assets/maps/…/` — targets need the **trailing slash** (directory targets; extension-less targets are how strays like `assets/maps/narrows2` were born) |

alpha11 flag deltas vs older docs: `qbsp -splitturb` and `qbsp -hide` are
GONE. `/usr/bin/vis` is the BSD text editor — always call the toolchain binary
by path.

`.lit` is **tracked content**, not derived: a map shipping a sidecar has
advanced lighting NOT baked into the bsp; without the matching `.lit` (size-
checked by the engine) it renders unlit. Commit bsp+lit+map together.

## Engine launch profile (in TB)

Launches the Mac fork bundle:

```
-nolauncher -basedir ${GAME_DIR_PATH} -game solitude -window -condebug
+exec config-mac.cfg +hud_mapshot 0 +map ${MAP_BASE_NAME}
```

Stock Quakespasm 0.97 / the 2021 archived app are `./mac.sh stock` fallbacks
only — they spam `Unknown command` on fork cvars and lack the HUD/perf work.

## Entity/content notes

- Map icon: `200×117` **`.tga`** named `<mapname>` → `solitude/gfx/maps/`.
- `func_illusionary` + `"_mirrorinside" "1"` is the pattern for warp-effect
  geometry (avoids the `*` CONTENTS_WATER trap — see
  [texturing-krita.md](texturing-krita.md)).
- `misc_model` works engine-side with zero QuakeC (`model` key, `angle -1` =
  random yaw) — details in [../model-src/README.md](../model-src/README.md).
- Background audio in maps = speaker entities; worldspawn `"sounds"` is
  cd-track convention this pack doesn't ship.
- Fog/weather worldspawn keys in use: `fog` "r g b density" (color currently
  ignored on Vita shaders — see TODO fog section), `skyfog` "0.55".
- Waypointing bots into a new map: [waypointing-guide.md](waypointing-guide.md).
- Crash/autosave maps from TB land beside the source — when scripting over
  `assets/maps/`, filter with `'-crash' in basename` (excludes
  `highground-crash-1.map`, which an `-crash.map` suffix test misses).

## Ship checklist for a new map

1. `assets/maps/<name>/` source `.map` (+ `.lit` when advanced-lighted) —
   tracked; built `.bsp` to `solitude/maps/` per current practice committed
   alongside.
2. New/proxy wads tracked under `assets/textures/wads/`, `wad` key updated.
3. Any `textures/<name>.png` sidecars → `solitude/textures/`. Respect the size
   budget in `texturing-krita.md` (≤256 px max edge; root-level files override
   that lump on **every** map, so map-specific art belongs in
   `textures/<map>/`).
4. Map icon `.tga` + the menu/`levels` wiring (matchmaking list lives in
   `menu.c` levels table).
5. Mac play-through, then Vita USB sync + device pass (never FTP — it has
   crashed the device).
