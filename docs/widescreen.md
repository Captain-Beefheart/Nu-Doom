# Widescreen rendering — scoping & plan

*Status: planned, not started. Blocked on the fullscreen bottom-region fix (see
below). Baseline: Nu-Doom v0.6.0 + smooth lighting. Written 2026-07-25.*

## Goal

True widescreen (16:9 / 21:9): render a **wider horizontal field of view** — more
world on the sides — **not** a horizontally stretched 4:3 image. Vertical FOV stays
identical to 4:3, so geometry keeps its proportions. The 2D UI (status bar, menus,
HUD, messages) stays 4:3 and centered, with the extra width showing game view.

Render-only: FOV/width never touch the playsim (auto-aim and hitscans are
angle-based, not screen-based), so `timedemo demo1` must stay **1205 gametics**.

## Why this is a core-renderer rewrite (not a toggle)

`SCREENWIDTH` (= `ORIGWIDTH << HIRES` = 640) is a **compile-time constant baked in
across the renderer**:

- **~16 render arrays sized `[SCREENWIDTH]`**: `xtoviewangle` (r_main/r_state),
  `floorclip`/`ceilingclip`/`distscale` (r_plane), `negonearray`/`screenheightarray`/
  `clipbot`/`cliptop` (r_things), visplane `top[]`/`bottom[]` (r_defs), plus
  `columnofs[MAXWIDTH]` in r_draw.
- **Fixed framebuffer**: `DG_ScreenBuffer = malloc(DOOMGENERIC_RESX*DOOMGENERIC_RESY*4)`
  (640×400); the SDL window and texture are sized to `DOOMGENERIC_RESX`/`RESY`
  (doomgeneric_sdl.c), and `I_FinishUpdate` blits with a `SCREENWIDTH` stride.
- **Projection**: `R_InitTextureMapping` builds `xtoviewangle` so `FIELDOFVIEW`
  (90°) covers the view width; `R_ExecuteSetViewSize` sets `viewwidth`/`centerx`/
  `projection` from `scaledviewwidth`.
- **2D UI centering** assumes `SCREENWIDTH == ORIGWIDTH<<HIRES` (V_DrawPatch places
  logical-x at `x<<HIRES`, i.e. flush-left in the buffer).

Naïvely widening `viewwidth` with `FIELDOFVIEW` fixed **zooms in** (bigger
focallength → everything scales up), it does not widen the FOV. Correct widescreen
requires the projection change below.

## Dependency: the fullscreen bottom-region fix

Widescreen needs a correct **full-size view** to build on, and today the max-size
(screenblocks 11) view has a bug: the bottom ~40% renders as a black region and 2D
overlays below logical y≈115 don't reach the screen (root cause: `D_Display` still
branches on the now-dead `viewheight == 200`, and the visplane/openings bottom clip
doesn't cover the full 400-row view). That fix (tracked separately) rewrites the
same `R_ExecuteSetViewSize` / view-size code widescreen touches. **Do widescreen
after it lands** — otherwise it's built on broken view handling and conflicts.

## Implementation plan (phased, each verified `timedemo demo1` = 1205)

1. **Width parameterization.** Pick a widescreen width (e.g. 16:9 ≈ 854, 21:9 ≈
   1120 — align to a multiple; confirm `MAXWIDTH` ≥ it and bump if needed). Decide
   runtime vs compile-time buffer (see Decisions). Resize the `[SCREENWIDTH]` render
   arrays to the max width.
2. **Framebuffer + present.** Size `DG_ScreenBuffer`, the SDL window and texture,
   and the `I_FinishUpdate` blit stride to the wide width. If runtime-toggleable,
   reallocate the buffer and recreate the SDL texture on toggle (doomgeneric_sdl.c).
3. **Projection (the crux).** In `R_InitTextureMapping`, base `focallength` on the
   **4:3-equivalent half-width** (tied to the view height), not the actual wide
   half-width, so the extra columns map to angles beyond ±45° — widening horizontal
   FOV at the same vertical scale. Mirror PrBoom+/Crispy's widescreen
   `R_InitTextureMapping`/`R_SetupFrame`. Verify no fisheye and that the center 4:3
   region matches vanilla exactly.
4. **2D centering.** Introduce `WIDESCREENDELTA = (SCREENWIDTH - ORIGWIDTH<<HIRES)/2`
   and offset V_DrawPatch/V_DrawPatchDirect/V_CopyRect and the status-bar / menu /
   fullscreen-HUD / message positioning by it, so the UI stays 4:3-centered.
5. **Toggle + config.** `crispy.widescreen` (0 = 4:3, 1 = 16:9, 2 = 21:9) with
   config var + a Crispness menu item. Applying it re-runs the view-size setup
   (and, if runtime buffer, the realloc + SDL recreate).
6. **Verify.** Determinism (1205); center-region parity vs 4:3 (screenshot diff of
   the middle 640 columns should be ~identical); HUD/menus centered; no stretch.

## Design decisions to make first

- **Runtime toggle vs compile-time wide buffer.** Runtime (realloc `DG_ScreenBuffer`
  + recreate SDL texture on toggle) keeps the default 640 window and is the nicer
  UX, but touches the SDL backend and buffer lifecycle. Compile-time-wide (buffer
  always e.g. 854, toggle only chooses render width, 4:3 shown pillarboxed) is
  simpler but changes the default window. **Lean runtime** for a clean default.
- **Aspect handling.** Interacts with the existing 4:3 aspect-ratio-correction
  Crispness toggle — decide precedence (widescreen supersedes 4:3 letterbox).

## References

- PrBoom+ / DSDA-Doom and Crispy Doom widescreen implementations
  (`R_InitTextureMapping`, `R_ExecuteSetViewSize`, `WIDESCREENDELTA`).
- Existing in-tree precedent: the hi-res work already split `ORIGWIDTH`/`SCREENWIDTH`
  and the 2D scaling in `V_DrawPatch`, which is the seam widescreen extends.
