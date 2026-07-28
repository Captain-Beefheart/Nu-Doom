# Nu-Doom

**Nu-Doom** is an enhanced DOOM source port for modern x64 systems, built on the
minimal [doomgeneric](https://github.com/ozkl/doomgeneric) core with the classic
static limits removed, a **native high-resolution renderer**, an **SDL2 audio
backend**, and a Crispy-Doom-inspired **Nu-Doom Options** menu.

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

### Mod support
doomgeneric shipped with DeHackEd and WAD-merging compiled out; Nu-Doom restores
both (the Chocolate Doom subsystems), which is what most classic vanilla and
limit-removing mods need:

- **DeHackEd (`-deh`)** — load one or more `.deh` / `.bex` patches. The full
  vanilla section set is supported: Thing, Frame, Pointer (code pointers),
  Weapon, Ammo, Sound, Cheat, Misc, and `[Text]` string replacements. Embedded
  `DEHACKED` lumps are loaded from PWADs with `-dehlump`, and automatically for
  IWADs that require them (Freedoom, HACX, Chex Quest).
- **Chocolate-style WAD merging (`-merge`)** — merge a PWAD's sprites, flats and
  textures *into* the IWAD namespace (plus the NWT-style `-nwtmerge` / `-af` /
  `-as` / `-aa`). This makes mods that replace individual sprites or flats —
  without a full `S_START`/`F_START` marker set — render correctly, which plain
  `-file` appending can't do.

Because Nu-Doom's renderer/playsim is **vanilla + limit-removing** (not
Boom/MBF/ZDoom), this targets vanilla-compatible and limit-removing mods;
Boom/MBF/UMAPINFO and GZDoom (DECORATE/ZScript) mods are out of scope.

### Nu-Doom Options menu & config
A dedicated **Nu-Doom Options** submenu (Options → Nu-Doom Options) spanning **six pages**
of working toggles, all saved to **`nudoom.cfg`** (config persistence,
which the base doomgeneric had disabled, is enabled here):

- **Rendering / display:** Uncapped framerate (sub-tic interpolation of camera,
  sprites, and moving sectors), Smooth pixel scaling, VSync, Framerate limit,
  Gamma correction, 4:3 aspect-ratio correction, Brightmaps (self-lit screens
  and lights), Smooth diminishing lighting, Translucency, Colored blood, Sound
  channels (8/16/32).
- **Weapon & crosshair:** Centered weapon, Configurable weapon bob, Weapon
  squat on landing, Weapon recoil pitch, Crosshair with shape (cross/X/dot),
  health tint, and target-highlight.
- **HUD:** Show FPS, coordinates, level stats, level time, demo timer,
  Colored HUD numbers (by value), "secret revealed" message. At the fullscreen
  view size (screenblocks 11) an **extended HUD** overlays big-number
  health/armor (bottom-left) and ready ammo (bottom-right), value-tinted when
  Colored HUD numbers is enabled.
- **Automap:** Overlay mode, rotate mode, secret-sector highlighting, extended
  (exit-line) colors.
- **Audio:** Mono SFX, full-length sounds (no cut-offs), SFX pitch shifting.
- **Demos / misc:** Demo progress bar, 8 savegame slots.

### Controls
- **WASD** move/strafe by default (W/S forward-back, A/D strafe; arrow keys
  still turn).
- **Configurable bindings** — **Options → Bind Keys** lists every gameplay
  action (fire, use, movement, run/strafe, jump, weapons 1–7) with its current
  key and mouse button. Select an action, press Enter, then press a key
  (arrow keys included) or a mouse button to rebind it; bindings persist to
  `default.cfg`.
- A **Controls** toggle (Nu-Doom Options page 4) switches between **Keyboard** and
  **Keyboard + Mouse**; with the mouse on, it is captured to the window and
  turns you left/right (and fires).
- A **Mouselook** toggle beneath it tilts the view up/down via horizon
  y-shear, with the weapon kept anchored to the HUD. Free look is view-only —
  shots keep vanilla auto-aim, so demo playback stays bit-for-bit deterministic.

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

   This produces `NuDoom.exe` linked against `SDL2.dll` and
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
dist/NuDoom.exe -iwad /path/to/doom2.wad

# shareware DOOM (episode + map)
dist/NuDoom.exe -iwad /path/to/doom1.wad -warp 1 1
```

Loading mods (see **Mod support** above):

```sh
# a map/megawad PWAD
dist/NuDoom.exe -iwad /path/to/doom2.wad -file mymegawad.wad

# a mod that replaces individual sprites/flats (merge into the IWAD namespace)
dist/NuDoom.exe -iwad /path/to/doom2.wad -merge sprites.wad

# a DeHackEd patch (gameplay/text changes)
dist/NuDoom.exe -iwad /path/to/doom2.wad -file maps.wad -deh patch.deh

# a PWAD that embeds its own DEHACKED lump
dist/NuDoom.exe -iwad /path/to/doom2.wad -file mod.wad -dehlump
```

Settings — including the Nu-Doom Options toggles — are saved to `nudoom.cfg` and
`default.cfg` in the working directory. If you run from outside the MinGW shell,
keep the DLLs from `setup_dist.sh` next to the executable.

## Compatible mods to try

Because Nu-Doom is a **vanilla + limit-removing** engine (with DeHackEd and WAD
merging — see **Mod support**), it runs the huge back catalogue of classic
vanilla and limit-removing WADs, but **not** Boom/MBF/UMAPINFO or GZDoom
(ZScript/DECORATE) mods. Here are ten popular, verified-compatible classics —
all free from the [Doomworld /idgames archive](https://www.doomworld.com/idgames/)
(or the author). Grab the `.zip`, unpack the `.wad`/`.deh`, and load as shown.

| Mod | Kind | IWAD | Load with |
|-----|------|------|-----------|
| **[SIGIL](https://romero.com/sigil)** (2019) — John Romero's official 5th episode for Doom | limit-removing | Doom 1 | `-merge SIGIL.wad -deh SIGIL.deh` |
| **[SIGIL II](https://www.doomworld.com/idgames/levels/doom/Ports/s-u/sigil_ii_v1_0)** (2023) — Romero's 30th-anniversary follow-up | limit-removing | Doom 1 | `-merge SIGIL_II.wad -deh SIGIL_II.deh` |
| **[Alien Vendetta](https://www.doomworld.com/idgames/levels/doom2/megawads/av)** (2001) — one of the most influential megawads ever made | limit-removing | Doom II | `-merge av.wad -deh av.deh` |
| **[Hell Revealed](https://www.doomworld.com/idgames/themes/hr/hr)** (1997) — the original high-monster-count "hard" megawad | vanilla | Doom II | `-file hr.wad` |
| **[Scythe](https://www.doomworld.com/idgames/levels/doom2/megawads/scythe)** (2003) — Erik Alm's bite-sized, hugely influential 32-map set | vanilla | Doom II | `-file scythe.wad` |
| **[Requiem](https://www.doomworld.com/idgames/levels/doom2/megawads/requiem)** (1997) — a defining late-'90s community megawad | vanilla | Doom II | `-merge requiem.wad reqmus.wad` |
| **[Memento Mori](https://www.doomworld.com/idgames/themes/mm/mm_allup)** (1995) — foundational international community megawad | vanilla | Doom II | `-merge mm.wad` |
| **[Icarus: Alien Vanguard](https://www.doomworld.com/idgames/themes/TeamTNT/icarus/icarus)** (1996) — TeamTNT's themed 32-map sister to TNT | vanilla | Doom II | `-merge icarus.wad` |
| **[Doom the Way id Did](https://www.doomworld.com/idgames/levels/doom/megawads/dtwid)** (2012) — community tribute recreating id's original style | vanilla | Doom 1 | `-file dtwid.wad -deh dtwid.deh` |
| **[Plutonia 2](https://www.doomworld.com/idgames/levels/doom2/megawads/pl2)** (2008) — the fan sequel to Final Doom's Plutonia | limit-removing | Doom II | `-merge pl2.wad -deh pl2.deh` |

Notes:
- **Filenames vary by release** — check each archive's `.txt`; the flags above
  use the typical lump names. Use `-merge` when a mod adds textures/sprites/flats
  (so they merge into the IWAD namespace), plain `-file` for pure map sets.
- **SIGIL / SIGIL II** add Episodes 5/6. On a strict vanilla-lineage engine, if a
  new episode doesn't appear, load the bundled `SIGIL_COMPAT.wad` instead (it
  *replaces* Episode 3). Both variants ship in the romero.com download.
- **Bonus DeHackEd showcase:** [Batman Doom](https://www.doomworld.com/idgames/themes/batman/batman)
  (1999) is a landmark DeHackEd total conversion. Run it vanilla-style with the
  [vbatman fix](https://www.doomworld.com/idgames/themes/batman/vbatman):
  `-merge batman.wad -deh batman.deh vbatman.deh`.

## Roadmap

- [x] Remove classic static renderer limits (visplanes / drawsegs / vissprites /
  openings).
- [x] Remove playsim limits (intercepts / spechit / plats / buttons / ceilings).
- [x] Native 640×400 high-resolution rendering.
- [x] Nu-Doom Options menu + persistent `nudoom.cfg`.
- [x] Nu-Doom Options menu (initial set): Crosshair, Show FPS, Translucency, Colored
  blood, Smooth pixel scaling, Uncapped/interpolated framerate (camera +
  sprites + moving sectors).
- [x] Nu-Doom Options menu (expanded set, two pages): Health-colored crosshair,
  Show coordinates, Show level stats, Centered weapon, Automap overlay,
  Automap rotate, "Secret revealed" message.
- [x] Nu-Doom Options menu (third page): VSync, Gamma correction, Sound channels
  (8/16/32), Configurable weapon bob, Weapon squat on landing, Crosshair
  type (cross/X/dot), Show level time, Automap secret highlighting.
- [x] Nu-Doom Options menu (fourth page): Framerate limit, Mono SFX, Full-length
  sounds, Demo timer, Target-highlight crosshair, Automap extended colors
  (exit lines), Mouselook.
- [x] Modern controls: WASD move/strafe by default, and a **Mouselook**
  toggle (mouse turns left/right, mouse up/down tilts the view via y-shear;
  the weapon stays anchored to the HUD). View-only free look — shots keep
  vanilla auto-aim, so demos stay deterministic.
- [x] **Bind Keys** menu (Options → Bind Keys): rebind every action to a key
  (arrows included) or mouse button, saved to `default.cfg`.
- [x] Extended fullscreen HUD (big-number health / armor / ammo at
  screenblocks 11).

### Nu-Doom Options menu — build out the full feature set

Goal: implement the complete Crispy Doom-style Nu-Doom Options menu. Remaining
features, grouped by menu category:

**Rendering / Visual**
- [x] Widescreen rendering (16:9 / 21:9 field of view) — true wider FOV (not
  stretched), selectable on Nu-Doom Options page 6, applied on next launch. UI
  stays 4:3-centered; demo playback is unchanged (`timedemo demo1` = 1205
  gametics).
- [x] Aspect-ratio correction (4:3 letterbox)
- [x] VSync toggle
- [x] Framerate limit
- [x] Brightmaps (self-lit texture pixels)
- [x] Smooth diminishing lighting
- [x] Colored HUD numbers (health / armor / ammo by value)
- [x] Extended "Crispy" fullscreen HUD layout (screenblocks 11)
- [x] Gamma / level brightness control

**Tactical**
- [x] Health-colored crosshair
- [x] Crosshair type / shape (cross / X / dot)
- [x] Crosshair target highlight
- [x] Configurable weapon bob
- [x] Weapon recoil pitch
- [x] Centered weapon when firing
- [x] Squat weapon on hard landing

**Audio**
- [x] More sound channels (8 / 16 / 32)
- [x] Full sound-effect pitch shifting
- [x] Mono SFX toggle
- [x] Play sounds in full length (no cutoffs)

**Navigational / Automap**
- [x] Automap overlay mode
- [x] Automap rotate mode
- [x] Secret-sector highlighting on the automap
- [x] Automap extended colors (exit-line highlighting)
- [x] "Secret revealed" notification
- [x] Show player coordinates and level stats
- [x] Show level time

**Demos / misc**
- [x] Demo timer
- [x] Demo progress bar
- [x] Extended savegame slots (8)

## Credits & License

Nu-Doom is derived from **doomgeneric** by ozkl, which is in turn derived from
id Software's DOOM. The Nu-Doom Options menu is modelled on **Crispy Doom** by Fabian
Greffrath. This project is free software licensed under the **GNU General Public
License v2** (see [COPYING](COPYING)); it comes with NO warranty.

- DOOM — Copyright © 1993-1996 id Software
- doomgeneric — https://github.com/ozkl/doomgeneric
- Upstream (`upstream` git remote) tracks ozkl/doomgeneric for pulling fixes.
