# Nu-Doom

**Nu-Doom** is an enhanced DOOM source port for modern x64 systems, built on the
minimal [doomgeneric](https://github.com/ozkl/doomgeneric) core with the classic
static limits removed, a **native high-resolution renderer**, an **SDL2 audio
backend**, and a Crispy-Doom-style **Crispness** options menu.

The goal is to keep doomgeneric's small, portable engine while removing the
original engine's static limits and enhancing graphics on modern hardware.

> Status: playable. Runs the DOOM shareware (`DOOM1.WAD`), DOOM, and DOOM II at
> native 640×400 with sound. Builds and runs on Windows x64 via MSYS2/MinGW.

## Features

### Limit removing
The famous vanilla static-array overflows are gone — the arrays now grow
dynamically instead of crashing on complex maps:

- **Renderer:** visplanes, drawsegs, vissprites, openings
  (e.g. runs *nuts.wad*'s ~10,000 monsters — vissprites grow past 4,000).
- **Playsim:** intercepts, spechit, active plats, buttons, active ceilings.

### High-resolution rendering
- Native **640×400** software rendering (not a 2× upscale of 320×200): sharp
  walls, sprites, HUD, status bar and menus.
- Split logical (320×200) vs. buffer (640×400) coordinate spaces via a `HIRES`
  factor; `V_DrawPatch` and friends scale the 2D UI, visplane spans widened for
  the taller buffer.
- 32-bit true-color framebuffer presented through SDL2.

### Sound
- **SDL2 audio backend** — SFX via `i_sdlsound.c`, music via `i_sdlmusic.c`
  (SDL2_mixer, `Mix_OpenAudioDevice`), enabled with `-DFEATURE_SOUND`.

### Crispness menu & config
- A dedicated **Crispness** submenu (Options → Crispness) with toggles saved to
  **`crispy-doom.cfg`** (config persistence, which the base doomgeneric had
  disabled, is enabled here).
- Working toggles: **Crosshair**, **Show FPS**, **Translucency** (energy
  projectiles), **Colored blood** (cacodemon → blue, baron/knight → green),
  **Smooth pixel scaling** (linear filtering when the window is resized), and
  **Uncapped framerate** (sub-tic interpolation of the camera, sprites, and
  moving sectors, so all motion — including doors and lifts — is smooth above
  35 fps).

## Why doomgeneric?

doomgeneric strips DOOM down to a tiny platform interface (`DG_Init`,
`DG_DrawFrame`, `DG_GetKey`, `DG_GetTicksMs`, `DG_SleepMs`), making it an ideal,
low-footprint base to build enhancements on. Nu-Doom uses the SDL2 platform
backend (`doomgeneric_sdl.c`).

## Building on Windows (MSYS2 / MinGW-w64)

1. Install [MSYS2](https://www.msys2.org/).
2. From an **MSYS2 MinGW64** shell, install the toolchain and SDL2 stack:

   ```sh
   pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 \
       mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-pkgconf make
   ```

3. Build with the provided MinGW makefile (adds `-DFEATURE_SOUND`,
   `-lSDL2_mixer`, and `-std=gnu11` — doomgeneric predates C23, whose `true`/
   `false`/`bool` keywords clash with `doomtype.h`):

   ```sh
   cd doomgeneric
   make -f Makefile.win
   ```

   This produces `doomgeneric.exe` linked against `SDL2.dll` and
   `SDL2_mixer.dll`. (The makefile tracks header dependencies, so incremental
   rebuilds are correct after editing headers.)

### Packaging a runnable folder

`setup_dist.sh` copies the executable and all required runtime DLLs (SDL2,
SDL2_mixer, and the music codec libraries) into a `dist/` folder:

```sh
./setup_dist.sh
```

## Running

You need a DOOM IWAD. The freely-redistributable DOOM shareware `DOOM1.WAD`, or
the fully-free [Freedoom](https://freedoom.github.io/) IWAD, both work, as do
the commercial `DOOM.WAD` / `DOOM2.WAD`.

```sh
# DOOM II (flat map numbering)
dist/doomgeneric.exe -iwad /path/to/doom2.wad

# shareware DOOM (episode + map)
dist/doomgeneric.exe -iwad /path/to/doom1.wad -warp 1 1
```

Settings — including the Crispness toggles — are saved to `crispy-doom.cfg` and
`default.cfg` in the working directory. If you run from outside the MinGW shell,
keep the DLLs from `setup_dist.sh` next to the executable.

## Roadmap

- [x] Remove classic static renderer limits (visplanes / drawsegs / vissprites /
  openings).
- [x] Remove playsim limits (intercepts / spechit / plats / buttons / ceilings).
- [x] Native 640×400 high-resolution rendering.
- [x] Crispness menu + persistent `crispy-doom.cfg`.
- [x] Crispness features: Crosshair, Show FPS, Translucency, Colored blood,
  Smooth pixel scaling, Uncapped/interpolated framerate (camera + sprites +
  moving sectors).
- [ ] Further rendering and quality-of-life enhancements.

## Credits & License

Nu-Doom is derived from **doomgeneric** by ozkl, which is in turn derived from
id Software's DOOM. The Crispness menu is modelled on **Crispy Doom** by Fabian
Greffrath. This project is free software licensed under the **GNU General Public
License v2** (see [COPYING](COPYING)); it comes with NO warranty.

- DOOM — Copyright © 1993-1996 id Software
- doomgeneric — https://github.com/ozkl/doomgeneric
- Upstream (`upstream` git remote) tracks ozkl/doomgeneric for pulling fixes.
