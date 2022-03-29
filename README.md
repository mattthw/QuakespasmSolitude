# Solitude

> ⚠️ **WARNING:** I am looking for the QC quake code source files to [Halo Revamped](https://www.youtube.com/watch?v=B_GB9LLBATQ) (r17?) for N3DS! Without this, the changes I am able to make are extremely limited. If you happen to have this laying around on your computer or know someone who has this, please let me know.

> ⚠️ **NOTICE:** This port is starting from scratch. See the original release [here](https://github.com/mmccoy37/SolitudeVita). The old version was build ontop of vitaQuake. While this new version is built ontop of Quakespasm-spiked.
## Introduction
Quakespasm-spiked is a Quake engine, ported to Vita by Rinnegatamante

QuakespasmSolitude is a port of Solitude for the vita, built ontop of vitaQuake. Solitude was originally made for the PSP, which was abandoned, then rebooted as Solitude Revamped, then abandoned, then ported to N3DS, then forgotten. This port is still a work in progress. QuakeC code is blocked, unless someone is able to decompile it into a clean, workable state.

# Issues
![](https://img.shields.io/github/issues-raw/mmccoy37/QuakespasmSolitude) ![](https://img.shields.io/github/issues-closed-raw/mmccoy37/QuakespasmSolitude)

Search [issues](https://github.com/mmccoy37/QuakespasmSolitude/issues) for existing bugs and feature requests before submitting a new one.

# Installation on Playstation Vita
1. Download the latest release: [releases](https://github.com/mmccoy37/QuakespasmSolitude/releases/)
1. Extract the archive
1. Install ``quakehalo.vpk`` on the Playstation Vita
1. Copy the ```/Solitude``` folder to ```<ux0 or alternative:>/data/Quakespasm/```.
    - You will also need the original quake files or shareware files as a .PAK filetype inside ```<ux0 or alternative:>/data/Quake/id1/```. You should be able to use the data files from VITADB for vitaQuake.


The end result will look something like this:
```
/data/Quakespasm/
            ./id1/*.PAK  # Legally acquired PAK files from original quake game
            ./Solitude/*
```

# Development

## Editing & Compiling Quake

### QuakeC Code

Use [FTEQCC](https://www.fteqcc.org/) ([mac](https://github.com/BryanHaley/fteqcc-mac)) to compile or decompile QuakeC code.

Sorry, but unless someone wants to decompile the progs.dat, or if someone can get TCPixel to locate the source of Solitude Revamped R17, we are out of luck.
### Compile SolitudeVita
1. Install the VitaSDK and set up building with the instructions here; https://vitasdk.org/
2. Install VitaGL (https://github.com/Rinnegatamante/vitaGl) with "make" then "make install"
3. Run ``make`` in the this repository's ``./Quake/`` directory, which should produce a working quakehalo.vpk

## Mapping / Level Design

- [Quake Level Design Starter Kit](https://github.com/jonathanlinat/quake-leveldesign-starterkit#readme)
- For decompilation of maps, use **QuArk.**
- [parsing of old solitude maps](https://docs.google.com/forms/d/e/1FAIpQLSeWVJZsibrkOvGFM-eU71NBU7y_i1WiL9-np2pGlpMS5n62mw/viewform?usp=sf_link)
- [Frank's Craptacular House of Mac Quake Stuffs
  ](http://quake.chaoticbox.com/) - Tools for quake on macOS
- https://quakewiki.org/wiki/Getting_Started_Mapping

### General Workflow
1. Setup **TrenchBroom**
   1. Install wherever. Make a shortcut to the installation folder. Make sure to add ```./id1/*``` quake source contents to ``<your directory>/Quake/id1/`` 
   2. Add WADs from solitude and quake under Entities menu
   3. Edit and save as .map
2. Use **ne_q1spCompilingGui** to compile the map as a .bsp with basic lighting
   1. use the **ericw-tools** command line tools for advanced lighting ([tutorial](https://www.youtube.com/watch?v=xtq_SY2R8MQ&t=297s))
3. Play the map using Quakespasm-spiked. ne_q1spCompilingGui can launch it directly for you if you point it to the .exe
   1. Manual steps: open the Quake folder belonging to Quakespasm (your PC copy, not this vita build/folder).
   2. Copy your ``<mapname>.bsp`` to ``./Quake/id1/maps/``
   3. Run your ``Quakespasm-sdl12.exe`` (or other name, if you have a different quake fork)
   4. Once it starts, press `` ` `` to open the console and type ``map <namename>`` to test your map
   
### WAD; Tooling
- [Download WAD pack for map editing](https://github.com/mmccoy37/QuakespasmSolitude/releases?q=wad&expanded=true)
- [quake-cli-tools](https://github.com/joshuaskelly/quake-cli-tools)

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