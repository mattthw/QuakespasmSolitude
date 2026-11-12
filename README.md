# Solitude

> ⚠️ **NOTICE #1:** I am looking for the QC quake code source files to [Halo Revamped](https://www.youtube.com/watch?v=B_GB9LLBATQ) (r17?) for N3DS! Without this, I must use decompilation of lost code to preserve the firefight features.

> ⚠️ **NOTICE #2:** This port is starting from scratch. See the original release [here](https://github.com/mattthw/SolitudeVita). The old version was built ontop of vitaQuake. While this new version is built ontop of Quakespasm-spiked.
## Introduction
QuakespasmSolitude is a port of Solitude for the vita, built ontop of Quakespasm-spiked (a Quake engine ported to Vita by Rinnegatamante). Solitude was originally made for the PSP and released in 2010 by FlamingIce. The forums went down and the game & assets became shareware. Since then several projects started with a goal of releasing on new platforms or improving the base game. This port is still a work in progress.

The goals of this project are to:
1. Have fun.
2. Make Solitude on PS Vita be a game we actually enjoy playing. Not just a demo of what is possible.
3. Upgrade old PSP maps and create new maps. I want to have a solid map pack of maps we actually enjoy playing on.

## Other versions of Solitude:

| Game                                                           | State                                                           | Started In | Last Update | Target Platforms | Authors                      | QuakeC Source                                        | Engine Source                                          |
|----------------------------------------------------------------|-----------------------------------------------------------------|------------|-------------|------------------|------------------------------|------------------------------------------------------|--------------------------------------------------------|
| [Solitude](https://www.moddb.com/games/solitude-game)          | ![](https://img.shields.io/badge/-alpha%20released-green)       | 2008       | 2010        | PSP, Windows     | FlamingIce                   | ![](https://img.shields.io/badge/-released**-yellow) | ![](https://img.shields.io/badge/-released**-yellow)   |
| [Solitude Reborn](https://www.moddb.com/games/solitude-reborn) | ![](https://img.shields.io/badge/-abandoned-red)                | 2014       | 2018        | Vita             | Jukki, Ghost_Fang, Dr_Mabuse | ![](https://img.shields.io/badge/-private-red)       | ![](https://img.shields.io/badge/-closed%20source-red) |
| [Halo Revamped](https://github.com/CollinScripter/Revamped3DS) | ![](https://img.shields.io/badge/-released-green)               | 2017       | 2017        | 3DS              | TCPixel                      | ![](https://img.shields.io/badge/-lost-yellow)       | ![](https://img.shields.io/badge/-public-green)        |
| (Quakespasm) Solitude Vita                                     | ![](https://img.shields.io/badge/-in%20development-yellowgreen) | 2020       | 2022        | Vita             | mattthw                      | ![](https://img.shields.io/badge/-none-grey)         | ![](https://img.shields.io/badge/-public-green)        |

**Once released publicly on now dead forums. Now shareware.

# Issues
![](https://img.shields.io/github/issues-raw/mattthw/QuakespasmSolitude) ![](https://img.shields.io/github/issues-closed-raw/mattthw/QuakespasmSolitude)

Search [issues](https://github.com/mattthw/QuakespasmSolitude/issues) for existing bugs and feature requests before submitting a new one.

# Installation on Playstation Vita
> **Beta release.** The v0.1.0 assets below are the same files that run on the
> developer's Vita; the mod is playable start to finish, but expect rough
> edges and see [Issues](https://github.com/mattthw/QuakespasmSolitude/issues).

1. Download the three release assets from
   [releases](https://github.com/mattthw/QuakespasmSolitude/releases/):
   `quakehalo.vpk`, `Solitude-vX.Y.Z.zip`, and `quakespasm.pak`
   (`SHA256SUMS.txt` has checksums).
1. Install ``quakehalo.vpk`` on the Playstation Vita (VitaShell → select the
   vpk → Install).
1. Create the folder ```ux0:/data/Quakespasm/``` and put ```quakespasm.pak```
   in it — it carries the engine defaults (console background, `default.cfg`,
   vanilla entity files).
1. Create ```ux0:/data/Quakespasm/Solitude/``` and extract the **contents**
   of ```Solitude-vX.Y.Z.zip``` into it.

The end result will look something like this:
```
ux0:/data/Quakespasm/
            ./quakespasm.pak   # engine defaults
            ./Solitude/        # the whole mod: maps models sound gfx,
                               # progs.dat, gfx.wad, PAK1.PAK, PAK2.PAK
```

You do **not** need the original Quake files or the shareware `pak0.pak`:
Solitude ships every asset its code references. (Want vanilla Quake maps in
this engine anyway? Drop your legally acquired `id1/pak0.pak` into
`ux0:/data/Quakespasm/id1/`.)

# Development

## Compiling Quake Source

### Compile SolitudeVita Engine Source
1. Install the VitaSDK and set up building with the instructions here; https://vitasdk.org/
2. Install VitaGL (https://github.com/Rinnegatamante/vitaGl) with "make" then "make install"
3. Run the following command in this repository's ``./Quake/`` directory, which should produce a working quakehalo.vpk
   ```make clean && make -f Makefile.vita```
   (If you built for macOS in the same tree first, stale host `.o` files
   break the ARM link — `make clean` or `rm *.o` is mandatory.)
4. (Optional) After installing quakehalo.vpk for the first time, you can install the changes faster by enabling FTP on the vita then running:
   ```curl --ftp-method nocwd -T ./build/eboot.bin ftp://<local IP of your vita>:1337/ux0:/app/SOLITUDE0/```
   > **Warnings:** hot-pushing `eboot.bin` is only for iterating on a build
   > you already know boots on the device — otherwise install the full vpk.
   > Keep FTP sessions inside data paths: LIST/MKD on `ux0:/app/` or the
   > `ux0:/` root can crash VitaShell's FTP server outright.

### Compile SolitudeVite Quake Code Source
1. Install [FTEQCC](https://www.fteqcc.org/) ([mac](https://github.com/BryanHaley/fteqcc-mac)) to compile or decompile QuakeC code.
2. Navigate to the `qcc-src`` directory.
3. run ``fteqcc`` command.
4. Copy ``progs.dat`` and ``progs.lno`` to your vita directory ``ux0:/data/Quakespasm/Solitude/``.

Example (run from `qcc-src/`; `<vita-ip>` is your device's address):
```
fteqcc && curl --ftp-method nocwd -T ../progs.dat "ftp://<vita-ip>:1337/ux0:/data/Quakespasm/Solitude/progs.dat"
```

## Mapping / Level Design

#### [Map Status Tracker](https://docs.google.com/spreadsheets/d/1HBcGHUBthl3Qb_umI5GIVykRjpyiiFUFz325e6HpJVI/edit?usp=sharing)

### WADs, Assets, Tooling
- [Skyboxes](https://www.quaddicted.com/webarchive/kell.quaddicted.com/skyboxes.html)
   - extract using [SLADE](https://slade.mancubus.net/index.php?page=downloads)
   - place all 6 images under <mod>/gfx/env/. If they are called skybox-day_[left | right | etc].tga then set ``sky`` to ``skybox-day_`` in trenchbroom's worldspawn entity.
- [Download WAD pack for map editing](https://github.com/mattthw/QuakespasmSolitude/releases?q=wad&expanded=true)
   - Use [Wally](https://valvedev.info/tools/wally/) to edit textures and build/package/unpackage the WAD
- [Quake Level Design Starter Kit](https://github.com/jonathanlinat/quake-leveldesign-starterkit#readme)
   - [Trenchbroom](https://github.com/TrenchBroom/TrenchBroom/releases)
   - [Quakespasm](https://sourceforge.net/projects/quakespasm/), [alt  silicon build](https://github.com/BryanHaley/Quakespasm-AppleSilicon)
   - [ericw-tools](https://github.com/ericwa/ericw-tools)
     >**NOTE** ericw-tools did not build for me on macOS 12.1 (Silicon). The issue was resolved by changing the build script: https://github.com/ericwa/ericw-tools/issues/329
   - [quake-cli-tools](https://github.com/joshuaskelly/quake-cli-tools): used for managing WADs
     - pak: Add files to a PAK file. 
     - unpak: Extract files from a PAK file. 
     - wad: Add file to a WAD file. 
     - unwad: Extract files from a WAD file. 
     - bsp2wad: Create a WAD file from a BSP file. 
     - qmount: Mount a PAK file as a drive. 
     - image2spr: Create an SPR from image files. 
     - spr2image: Extract frames from an SPR. 
     - bsp2svg: Create an SVG file from a BSP file.
- For decompilation of maps, use **[WinBSPC](https://gamebanana.com/tools/download/5030)**, or bsp2map
- [Parsing of old solitude maps](https://docs.google.com/forms/d/e/1FAIpQLSeWVJZsibrkOvGFM-eU71NBU7y_i1WiL9-np2pGlpMS5n62mw/viewform?usp=sf_link)
- [Frank's Craptacular House of Mac Quake Stuffs
  ](http://quake.chaoticbox.com/) - Tools for quake on macOS
- [Marco's Quake Tools](http://icculus.org/~marco/sources/q1tools.html) C programs for Targa <-> Lmp format
  - the programs are rehosted in *this* repository under ``/solitude/artwork`` in case the website goes down.
- https://quakewiki.org/wiki/Getting_Started_Mapping
- Map Icon creation: 
  - must be .tga and size 200x117. filename must match map name. Save from photoshop using *Save as Copy...* -> Targa -> ``<mapname>.tga``
  - code in menu.c must be updated to use image and image must be placed in ``Solitude/gx/maps/``
- [Frikbot waypointing](https://www.insideqc.com/frikbot/fbx/readme.html)
- How to take screenshots (so I remember later):
    ```
  r_drawviewmodel 0;viewsize 120;fov 120;noclip 1;crosshair 0;bind , screenshot
  ```

### Mapping Workflow Setup (macOS Silicon)
> Note: you must launch trenchbroom from the terminal with command ``cd /Applications/TrenchBroom.app && ./Contents/MacOS/TrenchBroom`` (or run directly with ``/Applications/TrenchBroom.app/Contents/MacOS/TrenchBroom``. Otherwise the compilation tools will fail. This is because they rely on terminal variables.
1. Install [Quakespasm](https://sourceforge.net/projects/quakespasm/), [alt  silicon build](https://github.com/BryanHaley/Quakespasm-AppleSilicon) to some location
   1. Make sure to add ```./id1/``` and ``pak1.pak`` to ``<your game directory>/Quake/id1/``. I cannot provide pak1.pak, you can get it from Quake source code by purchasing a retail copy.
2. Create a folder inside ``Quakespasm`` called ``./maps/`` if it does not yet exist. This is where Quake looks for maps.
3. Create a folder inside ``Quakespasm`` called ``./working/``. This is where we will create/edit/save maps
4. Create ``wads`` folder in ``Quakespasm/working/wads`` and add [project wads](https://github.com/mattthw/QuakespasmSolitude/releases?q=wad&expanded=true). These are used by TrenchBroom to load textures
5. Install [ericw-tools](https://github.com/ericwa/ericw-tools) in ``Quakespasm/working/``
6. Setup **TrenchBroom**
   1. Go to preferences and click on *Quake 1*
      1. point the engine to the ``./Quakespasm/`` directory which contains ``./id1/maps/``, ``./working/`` etc.
   2. In the map editor, go to **Faces**, then at the bottom right click on *Texture Collections*. Add wads from ``Quakespasm/working/wads``.
   3. Edit and save your map
   4. Go to *Run -> Compile Map...*.
      1. Use the following settings:
         - Working directory: ``${GAME_DIR_PATH}/working/``
         - Export Map: ``${WORK_DIR_PATH}/${MAP_BASE_NAME}.map``
         - Run Tool: ``${WORK_DIR_PATH}/ericw-tools/build/qbsp/qbsp``
            - Parameters: ``-wadpath  ${WORK_DIR_PATH}/wads/ ${MAP_BASE_NAME}``
         - Run Tool: ``${WORK_DIR_PATH}/ericw-tools/build/vis/vis``
            - Parameters: ``${MAP_BASE_NAME}``
         - Run Tool: ``${WORK_DIR_PATH}/ericw-tools/build/light/light``
            - Parameters: ``<your settings> ${MAP_BASE_NAME}``
               - example ``-bounce -bouncescale 2 ${MAP_BASE_NAME}``
               - docs: http://ericwa.github.io/ericw-tools/doc/light.html
               - examples: http://ericwa.github.io/ericw-tools/
         - Copy Files: source: ``${WORK_DIR_PATH}/${MAP_BASE_NAME}.bsp``, target: ``${GAME_DIR_PATH}/id1/maps/``
         - Copy Files: source: ``${WORK_DIR_PATH}/${MAP_BASE_NAME}.lit``, target: ``${GAME_DIR_PATH}/id1/maps/``
      2. Click *Launch...*
         1. use the parameters ``-basedir ${GAME_DIR_PATH} +map ${MAP_BASE_NAME} ``

#### Updated 2026-11-11 — post path-move + relative wad keys
The layout above (the old `Quakespasm/working/` game dir) predates the project move. Current truth:
- **Game dir (``${GAME_DIR_PATH}``) = this repo** (`solitude/repo`). Map `wad` keys are
  repo-relative, so **TB's compile working directory must be the repo root** or qbsp
  cannot resolve textures. Vendored wads arrive via the `wads/` symlink
  → `solitude/local/vendored/wads`; in-repo wads at `assets/textures/wads/`.
- **Tools:** prebuilt ericw-tools **2.0.0-alpha11** at `../toolchain/ericw-tools/`
  (there is no Homebrew formula). alpha11 dropped `qbsp -splitturb`; `-wadpath`, `vis -fast`,
  `light -surflight_subdivide` verified present.
- Compile profile as configured now:
  - Working directory: ``${GAME_DIR_PATH}``
  - qbsp: ``-wadpath ${GAME_DIR_PATH}/wads ${MAP_BASE_NAME}``
  - vis: ``${MAP_BASE_NAME}``  ·  light: ``-surflight_subdivide 64 ${MAP_BASE_NAME}``
  - Copy bsp/lit/map to ``${GAME_DIR_PATH}/solitude/maps/`` **and**
    ``${GAME_DIR_PATH}/assets/maps/`` — always with trailing slash (directory targets).
    The old extension-less targets are how strays like `assets/maps/narrows2` were born.
- Root-level compile intermediates (`/*.bsp /*.lit /*.vis /*.map /*.prt`) are gitignored.
- **Engine launch profile is intentionally NOT updated:** it still points at the archived
  Mac build `solitude/local/archive/resources/Quakespasm/QuakeSpasm-SDL2.app`, which is
  the correct binary for local test play. Don't "fix" this path.
- If you hand-edit TB's own files in `~/Library/Application Support/TrenchBroom/`,
  quit TrenchBroom first — it rewrites `Preferences.json` on exit.

# Credits
- idSoftware for winQuake sourcecode
- MasterFeizz for ctrQuake sourcecode i studied to understand how winQuake works
- EasyRPG Team for the audio decoder used for CDAudio support
- Ch0wW for various improvements and code cleanup
- JPG for ProQuake and some various fixes.
- Cuevavirus for 1920x1080 rendering

continued...
- Credit to Rinne for porting Quake to Vita
- Credit to FlamingIce team for the original Solitude and Solitude Revamped
- Ghost_Fang for menu & scoreboard inspiration
- Credit to TCPixel for the [N3DS port](https://github.com/CollinScripter/Revamped3DS), which was the only place I was able to obtain the source code
- Also credit to all the unnamed and unknown contributors to Solitudes several maps and many assets
