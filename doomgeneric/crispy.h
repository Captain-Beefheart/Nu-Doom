//
// Copyright(C) 2023-2026 Nu-Doom contributors
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Nu-Doom "Crispness" settings. These are the tunables exposed by the
//	Crispness menu and persisted to crispy-doom.cfg. Modelled on the
//	crispy_t settings block used by Crispy Doom.
//

#ifndef __CRISPY_H__
#define __CRISPY_H__

#include "doomtype.h"
#include "m_fixed.h"

typedef struct
{
    int uncapped;       // uncapped / interpolated framerate
    int smoothscaling;  // smooth (linear) pixel scaling to the display
    int translucency;   // translucent sprites / effects
    int coloredblood;   // colored blood and corpses
    int crosshair;      // draw a crosshair in the center of the view
    int crosshairhealth;// tint the crosshair by the player's health
    int crosshairtype;  // crosshair shape: 0 cross, 1 X, 2 dot
    int showfps;        // show a framerate counter
    int showcoords;     // show player x/y/z/angle
    int showstats;      // show level kills / items / secrets
    int showleveltime;  // show elapsed level time
    int centerweapon;   // center the weapon while firing (no side bob)
    int weaponbob;      // view/weapon bob amount: 0 off .. 4 full
    int weaponsquat;    // dip the weapon on a hard landing
    int automapoverlay; // draw the automap over the game view
    int automaprotate;  // rotate the automap to the player's facing
    int automapsecrets; // highlight unfound secret sectors on the automap
    int automapcolors;  // extended automap colors (highlight exit lines)
    int secretmessage;  // print a message when a secret is revealed
    int vsync;          // vertical sync on the display
    int fpslimit;       // render framerate cap: 0 = off, else target fps
    int monosfx;        // force mono (centered) sound effects
    int fullsounds;     // let sound effects play in full (no cutoffs)
    int demotimer;      // show a demo-playback timer
    int crosshairtarget;// highlight the crosshair when aiming at a target
} crispy_t;

// The single global crispness settings block.
extern crispy_t crispy;

// Bind the crispness variables into the config system so they load from
// and save to crispy-doom.cfg. Called from D_BindVariables().
void M_BindCrispnessVariables(void);

// Per-frame overlays gated by the crispness toggles (called from D_Display).
void Crispy_DrawCrosshair(void);  // crispy.crosshair (+ crispy.crosshairhealth)
void Crispy_DrawFPS(void);        // crispy.showfps
void Crispy_DrawCoords(void);     // crispy.showcoords
void Crispy_DrawStats(void);      // crispy.showstats (+ crispy.showleveltime)
void Crispy_DrawDemoTimer(void);  // crispy.demotimer (during demo playback)

// Fixed-point bob multiplier (FRACUNIT = full) from crispy.weaponbob; used to
// scale the render-only view/weapon bob without touching the playsim.
fixed_t Crispy_BobFactor(void);

// Colored-blood translation tables (crispy.coloredblood).
extern byte crispy_bloodtrans_blue[256];
extern byte crispy_bloodtrans_green[256];

#endif // __CRISPY_H__
