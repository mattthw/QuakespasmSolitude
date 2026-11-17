# Solitude production docs

Human docs for making content that ships in this mod: textures, maps, models,
and how the pieces get into the game. Written 2026-11-17.

> Convention for every page here: a **Verified** line saying what it was
> checked against. Tool flags rot — trust the date, and re-check anything
> marked *Unverified* before blaming the engine when it differs.

## Pages

| Page | About |
|---|---|
| [texturing-krita.md](texturing-krita.md) | Palette policy, name prefixes (`+ * {`), alpha rules, glow, external PNG sidecars |
| [wad-conversion.md](wad-conversion.md) | Getting art from Krita/PS/PNG into a `.wad` qbsp can read; WAD2 format facts; proxy-lump convention |
| [mapping-tb.md](mapping-tb.md) | TrenchBroom + ericw-tools setup on this repo, compile profiles, engine launch, traps |
| [modeling-3dsmax.md](modeling-3dsmax.md) | 3ds Max → MDL/MD3 (stub) |
| [modeling-blender.md](modeling-blender.md) | Blender route (stub — see also [../model-src/README.md](../model-src/README.md)) |
| [runtime-assets.md](runtime-assets.md) | Where files live at runtime: searchpath, loose-vs-pak, textures/, data/, music/ |
| [waypointing-guide.md](waypointing-guide.md) | Frikbot waypoint workflow (existing page) |
| [hud-rework-spec.md](hud-rework-spec.md) | HUD rework spec (existing page, historical) |

## The asset pipeline at a glance

```
 art (Krita PNG, 8-bit palette or RGBA)      3D model (Max / Blender)
        │                                            │
        ▼                                            ▼
 wad converter (any2wad / Quake Texture Tool)   MDL/MD3 exporter
        │                                            │
        ▼                                            ▼
 assets/textures/wads/*.wad ──► TrenchBroom ◄── progs/v_*.mdl, misc_model props
 (or textures/<map>/*.png sidecar)   map .map          solitude/progs/
                                       │
                                       ▼
                        ericw-tools qbsp → vis → light
                        (run from repo ROOT — wad keys are relative)
                                       │
                                       ▼
                    solitude/maps/<name>.bsp (+ .lit tracked sidecar!)
                    + <name>.tga map icon (200×117) → solitude/gfx/maps/
                                       │
                                       ▼
                     ./mac.sh launch <name>   |   Vita: USB push (never FTP)
```

## Browsing these docs

- **VS Code**: open the repo folder; click any `.md`, `⌘K V` for preview.
  Relative links above work in the preview and in GitHub's web renderer.
- **GitHub**: renders `docs/` natively, same files.
- **True wiki feel later** (optional phase 2): `mkdocs serve` or
  `python -m http.server` + a markdown-viewer page — not required to use these.
