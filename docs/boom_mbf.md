# Boom / MBF compatibility — scoping

*Status: planning / not started. Baseline: Nu-Doom v0.6.0 (vanilla + limit-removing,
with DeHackEd and Chocolate-style WAD merge). Written 2026-07-25.*

## TL;DR

Everything Nu-Doom has shipped so far — hi-res, uncapped framerate, the Crispness
toggles, colored HUD, brightmaps, even DeHackEd/WAD-merge — was **render/input-only
or purely additive data**. That is exactly why `timedemo demo1` has stayed at **1205
gametics** through every feature: none of it changed the simulation.

**Boom/MBF is the opposite.** It rewrites the *playsim* — line/sector specials,
thinkers, monster AI, physics — and therefore changes simulation results *by design*.
You cannot simply "add" it; it must go in **behind a compatibility-level (complevel)
gate** so the vanilla code path still simulates bit-for-bit like vanilla. In effect
this converts Nu-Doom from a *vanilla + limit-removing* port into a *Boom/MBF* port,
which is the single largest item on the roadmap — larger than everything else combined.

This document scopes what that takes so the decision can be made with eyes open.

## Why this is categorically different

| Prior feature type | Touches | Determinism |
|---|---|---|
| Crispness / hi-res / HUD / brightmaps | renderer, HUD, input | preserved (render/input only) |
| DeHackEd / WAD merge | data tables at load time, gated when unused | preserved (inert unless a mod is loaded) |
| **Boom / MBF** | **the playsim: specials, thinkers, AI, physics** | **changed on purpose — must be complevel-gated** |

The whole *point* of Boom is to run Boom maps and demos **correctly**. Boom deliberately
fixed vanilla bugs and changed physics; MBF then added ~20 `comp[]` toggles precisely
because behavior differs per era. So Boom/MBF support is inseparable from a
**compatibility-level system** that selects vanilla-vs-Boom-vs-MBF behavior at dozens
of decision points and is threaded through demo I/O and savegames.

## Current state of the tree (grounded, as of v0.6.0)

| Subsystem | Today (doomgeneric) | Boom needs | MBF adds |
|---|---|---|---|
| **Playsim specials** | vanilla `p_spec.c` = **1,509 lines** | generalized linedefs + scrollers / friction / pushers / deep-water / transfers (Woof `p_spec.c` = **4,147** + `p_genlin.c` = **1,209**) | a few extra |
| **`line_t` / `sector_t`** | vanilla only (`short special; short tag;`) | many new fields (control sectors, scroll data, friction, colormap index, preserved special bits) | friend / comp bits |
| **`mobj_t.flags`** | **no Boom/MBF flags present** | `MF_TRANSLUCENT` / `TOUCHY` / `BOUNCES` | `MF_FRIEND` (+ helper AI) |
| **Animations** | **hardcoded** `animdefs[]` table in `p_spec.c` | parse `ANIMATED` / `SWITCHES` lumps | — |
| **Colormap / translucency** | present: `R_InitTranMap` + `COLORMAP` load (from Crispy translucency) | reuse for `TRANMAP` + multi-`COLORMAP` transfers | — |
| **DeHackEd** | vanilla `Pointer` section + `[STRINGS]` only (Chocolate 2.2.0) | full **BEX**: `[CODEPTR]`, `[PARS]`, DEHEXTRA (extra states / things) | MBF codepointers |
| **Compat / demo** | `gameversion` (vanilla exe variants) + longtics + `DEMOMARKER` | a real **complevel** system + Boom demo format | `comp[]` (~20 flags) + MBF demos |
| **Nodes** | vanilla only | **extended nodes** (DeePBSP / ZDBSP) — many Boom maps ship them | — |

**Donor code available locally:** Woof (`C:\Users\computer\CODE\woof`) is a modern
MBF-lineage port and has `p_genlin.c`, the full Boom `p_spec.c`, and a complete BEX
suite (`deh_bex_pointers/strings/sprites/sounds/music/partimes/helper/includes`).
Caveat: those files assume Woof's *data model* throughout, so they do **not** drop in
the way Chocolate Doom's DeHackEd did — this was the reason grafting Woof gameplay was
rejected earlier in the project.

## The three things that make this big (not merely tedious)

1. **Data-structure surgery.** New fields on `line_t` / `sector_t` / `mobj_t` ripple
   into the renderer (`r_bsp` / `r_segs` / `r_plane` read sector fields at ~30 sites),
   into `p_saveg.c` (everything new must serialize — and be *versioned*), and into
   `p_setup.c` (loading Boom maps). This change touches the most files.

2. **The complevel spine.** A global compatibility level gating vanilla / Boom / MBF
   behavior at dozens of playsim decisions, plumbed into demo I/O and savegames. Getting
   this bug-for-bug right is the real *correctness* boss (it is why DSDA-Doom exists as
   the demo-compat reference), and it is distinct from the sheer volume of code.

3. **DeHackEd → full BEX + DEHEXTRA.** The ported deh is vanilla `Pointer` + BEX
   strings only. Boom/MBF mods need `[CODEPTR]` with mnemonic pointers, `[PARS]`, MBF's
   new codepointers (`A_Mushroom`, `A_Spawn`, `A_LineEffect`, …), and DEHEXTRA's extra
   state/thing slots. The framework already added is the right base; Woof's `deh_bex_*`
   files are the donor.

## Boom feature inventory (what actually needs implementing)

- **Generalized linedefs** (`p_genlin`) — parameterized doors / floors / ceilings /
  lifts / stairs / crushers encoded in linedef-value bitfields. Self-contained,
  ~1,200 lines; the single biggest Boom feature.
- **Dynamic specials** — scrolling floors / walls / ceilings + conveyors, **friction**
  (ice / mud), **pushers** (wind / current / point source), **deep water** (line 242),
  **transfer** specials (floor/ceiling light, colormap, translucent lines), Boom
  elevators, silent and line-to-line teleporters.
- **Sector / line type decoding** — Boom turned sector `special` into a bitfield
  (damage / secret / friction / wind flags) and extended the line-trigger model.
- **Static data** — `ANIMATED` / `SWITCHES` lumps (replacing the hardcoded tables),
  `TRANMAP`, multiple `COLORMAP`s, and **extended nodes** (needed to even *load* many
  Boom maps).

## MBF additions on top of Boom

- MBF codepointers (`A_Detonate`, `A_Mushroom`, `A_Spawn`, `A_Turn`, `A_Face`,
  `A_Scratch`, `A_PlaySound`, `A_RandomJump`, `A_LineEffect`, `A_Die`, `A_FireOldBFG`,
  `A_BetaSkullAttack`).
- **Friend monsters + dogs** — `MF_FRIEND` plus helper AI in `p_enemy` (friend target
  selection, `P_HelpFriend`, player-following helpers).
- The `comp[]` compatibility array (~20 toggles: telefrag, dropoff, vile, pain, skull,
  blazing, doorlight, model, god, falloff, floors, skymap, pursuit, doorstuck, staylift,
  zombie, stairs, …).
- Sky-transfer specials (271 / 272), beta-emulation pointers.

## Modern reality: MBF21 + UMAPINFO

"Boom/MBF compatibility" in current practice usually means **complevel 21 (MBF21) +
UMAPINFO**. Several of the popular mods people actually want to run — Ancient Aliens,
Going Down, Valiant, Eviternity — depend on **MBF21 and/or UMAPINFO**, not merely Boom.
So implementing Boom + MBF alone unlocks the *older* Boom catalog; covering the popular
*modern* mods needs MBF21 (new thing flags/args, new codepointers, thing groups) plus
UMAPINFO (map names/order/music) as an additional phase. Decide the target up front.

## Two forks in the road

**Fork 1 — how faithful?**

- **(C) "Plays Boom maps."** Implement the specials so Boom/MBF *maps play*, but skip
  rigorous demo-compat. ~60–70% of the total effort, unlocks the mapping catalog, and
  is the fun path. Downside: Boom demos will not sync, and it must still be gated so
  the vanilla path stays at 1205 gametics.
- **(F) "Demo-compatible port."** The full complevel matrix, speedrun-grade. This is
  effectively *rebuilding PrBoom / DSDA-Doom inside doomgeneric*. Months of work.

**Fork 2 — where to source it?**

- **Port from Woof** — fastest to functional, but you inherit Woof's data model and
  drift from the clean doomgeneric base (the very thing deliberately avoided before).
- **Adapt PrBoom / Chocolate-lineage incrementally, complevel-gated** — keeps Nu-Doom's
  character, much more hand-work per subsystem.

## Suggested phasing (if we proceed)

| Phase | Work | Risk | Notes |
|---|---|---|---|
| 0 | complevel scaffold + extend `line_t` / `sector_t` / `mobj_t` + savegame versioning | med | the spine; nothing visible yet |
| 1 | `ANIMATED` / `SWITCHES`, `TRANMAP`, multi-`COLORMAP`, **extended nodes** | low | self-contained; unlocks map *loading* |
| 2 | generalized linedefs (`p_genlin`) | med | biggest single Boom feature |
| 3 | scrollers, friction, pushers, deep water, transfers, elevators | **high** | most Boom maps need these; physics-sensitive |
| 4 | full BEX + DEHEXTRA (extend the deh subsystem) | med | Woof `deh_bex_*` as donor |
| 5 | MBF: codepointers, friends/dogs, `comp[]` | high | |
| 6 *(optional)* | MBF21 + UMAPINFO | med–high | needed for the popular *modern* mods |

**Every phase:** re-verify `timedemo demo1 = 1205` (vanilla path untouched) **and** test
against representative Boom maps.

## Recommendation

- **Effort:** weeks-to-months, not the days-per-feature cadence so far. The volume of
  code is large and the *correctness* bar (demo compat) is the real difficulty.
- **If the goal is to enjoy Boom maps in Nu-Doom** → take **Fork 1-C**, phases 0–4,
  gated so vanilla stays at 1205. High value and tractable. Start with **Phase 1**
  (extended nodes + `ANIMATED`/`SWITCHES` + full BEX): the most self-contained pieces,
  lowest regression risk, and the fastest way to learn how much Woof code ports cleanly
  before committing to the big phases.
- **If the goal is a real demo-compatible Boom/MBF port** → recognize that this
  rebuilds what **Woof / DSDA-Doom already do excellently**. Nu-Doom's distinctive value
  is the *minimal doomgeneric base + hi-res + Crispness*; going full-compat trades that
  identity for a crowded field. A legitimate alternative is to **stay vanilla +
  limit-removing** (where the project is already strong) and point Boom users at
  Woof / DSDA-Doom.

## References

- **Donor code (local):** `C:\Users\computer\CODE\woof\src\` — `p_genlin.c`,
  `p_spec.c`, `p_map.c`, `p_enemy.c`, `deh_bex_*.c`.
- **Specs:** the Boom reference (`boomref.txt`), MBF docs (`mbfedit`/`mbf.txt`),
  the MBF21 spec, and the UMAPINFO spec.
- **Reference implementations:** PrBoom+, Woof, DSDA-Doom (the latter is the
  demo-compatibility gold standard).
