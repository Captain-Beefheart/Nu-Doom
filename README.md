# Nu-Doom

**Nu-Doom** is an enhanced DOOM source port for modern x64 systems, built on the
minimal [doomgeneric](https://github.com/ozkl/doomgeneric) core with a full
**SDL2 audio backend** wired in.

The goal of the project is to keep doomgeneric's small, portable, low-footprint
engine while progressively removing the original engine's static limits and
enhancing graphics on modern hardware.

> Status: early. The gameplay/renderer come from doomgeneric (vanilla DOOM
> lineage); sound (SFX + music) is provided by SDL2 / SDL2_mixer. Builds and runs
> on Windows x64 via MSYS2/MinGW.

## Why doomgeneric?

doomgeneric strips DOOM down to a tiny platform interface (`DG_Init`,
`DG_DrawFrame`, `DG_GetKey`, `DG_GetTicksMs`, `DG_SleepMs`), which makes it an
ideal, low-memory base to build enhancements on. Nu-Doom uses the SDL2 platform
backend (`doomgeneric_sdl.c`) and enables the sound path that doomgeneric leaves
optional.

## Features

- **Minimal, low-footprint core** from doomgeneric (vanilla DOOM gameplay).
- **640×400, 32-bit true-color framebuffer** (2× internal scale of the classic
  320×200), rendered through SDL2.
- **SDL2 audio backend** — sound effects via `i_sdlsound.c` and music via
  `i_sdlmusic.c` (SDL2_mixer, `Mix_OpenAudioDevice`), enabled with
  `-DFEATURE_SOUND`.
- Cross-platform SDL2 video/input.

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
   `SDL2_mixer.dll`.

### Packaging a runnable folder

`setup_dist.sh` copies the executable and all required runtime DLLs (SDL2,
SDL2_mixer, and the music codec libraries) into a `dist/` folder:

```sh
./setup_dist.sh
```

## Running

You need a DOOM IWAD (`DOOM1.WAD`, `DOOM.WAD`, `DOOM2.WAD`, ...). The freely
redistributable DOOM shareware WAD, or the fully-free
[Freedoom](https://freedoom.github.io/) IWAD, both work.

```sh
dist/doomgeneric.exe -iwad /path/to/doom2.wad
```

If you run from outside the MinGW shell, keep the DLLs from `setup_dist.sh` next
to the executable.

## Roadmap

- Remove classic static limits (dynamic visplanes / drawsegs / vissprites).
- Higher internal resolution and rendering enhancements.
- Incremental C++ modernization of subsystems.

## Credits & License

Nu-Doom is derived from **doomgeneric** by ozkl, which is in turn derived from
id Software's DOOM. This project is free software licensed under the **GNU
General Public License v2** (see [COPYING](COPYING)); it comes with NO warranty.

- DOOM — Copyright © 1993-1996 id Software
- doomgeneric — https://github.com/ozkl/doomgeneric
- Upstream is tracked as the `upstream` git remote for pulling future
  doomgeneric fixes.
