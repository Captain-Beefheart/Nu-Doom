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
//	Nu-Doom "Crispness" settings storage and config bindings.
//

#include <stdio.h>

#include "crispy.h"
#include "m_config.h"
#include "i_video.h"
#include "i_timer.h"
#include "m_fixed.h"
#include "doomstat.h"
#include "p_local.h"   // P_AimLineAttack + linetarget for the target crosshair

// State owned by the renderer / menu that the overlays need.
extern int  viewwindowx, viewwindowy, viewwidth, viewheight;
extern void M_WriteText(int x, int y, char *string);

// All settings default to 0 (off), matching vanilla behaviour.
crispy_t crispy = { 0 };

// Colored-blood translation tables: identity except the red blood gradient
// (palette 0xB0-0xBF) remapped to blue / green.
byte crispy_bloodtrans_blue[256];
byte crispy_bloodtrans_green[256];

static void Crispy_InitColoredBlood(void)
{
    int i;

    for (i = 0; i < 256; i++)
        crispy_bloodtrans_blue[i] = crispy_bloodtrans_green[i] = (byte) i;

    for (i = 0; i < 16; i++)
    {
        crispy_bloodtrans_blue[0xB0 + i]  = (byte) (0xC0 + i);  // -> blue
        crispy_bloodtrans_green[0xB0 + i] = (byte) (0x70 + i);  // -> green
    }
}

void M_BindCrispnessVariables(void)
{
    M_BindVariable("crispy_uncapped",      &crispy.uncapped);
    M_BindVariable("crispy_smoothscaling", &crispy.smoothscaling);
    M_BindVariable("crispy_translucency",  &crispy.translucency);
    M_BindVariable("crispy_coloredblood",  &crispy.coloredblood);
    M_BindVariable("crispy_crosshair",      &crispy.crosshair);
    M_BindVariable("crispy_crosshairhealth",&crispy.crosshairhealth);
    M_BindVariable("crispy_crosshairtype",  &crispy.crosshairtype);
    M_BindVariable("crispy_showfps",        &crispy.showfps);
    M_BindVariable("crispy_showcoords",     &crispy.showcoords);
    M_BindVariable("crispy_showstats",      &crispy.showstats);
    M_BindVariable("crispy_showleveltime",  &crispy.showleveltime);
    M_BindVariable("crispy_centerweapon",   &crispy.centerweapon);
    M_BindVariable("crispy_weaponbob",      &crispy.weaponbob);
    M_BindVariable("crispy_weaponsquat",    &crispy.weaponsquat);
    M_BindVariable("crispy_automapoverlay", &crispy.automapoverlay);
    M_BindVariable("crispy_automaprotate",  &crispy.automaprotate);
    M_BindVariable("crispy_automapsecrets", &crispy.automapsecrets);
    M_BindVariable("crispy_automapcolors",  &crispy.automapcolors);
    M_BindVariable("crispy_secretmessage",  &crispy.secretmessage);
    M_BindVariable("crispy_vsync",          &crispy.vsync);
    M_BindVariable("crispy_fpslimit",       &crispy.fpslimit);
    M_BindVariable("crispy_monosfx",        &crispy.monosfx);
    M_BindVariable("crispy_fullsounds",     &crispy.fullsounds);
    M_BindVariable("crispy_demotimer",      &crispy.demotimer);
    M_BindVariable("crispy_crosshairtarget",&crispy.crosshairtarget);

    // Weapon bob defaults to full (4) so gameplay is unchanged out of the box.
    crispy.weaponbob = 4;

    Crispy_InitColoredBlood();
}

//
// Crispy_DrawCrosshair
// Plots a simple plus-shaped crosshair at the center of the 3D view.
// Drawn directly into the 8-bit render buffer (palette index 4 ~ white).
//
void Crispy_DrawCrosshair(void)
{
    int cx, cy, i;
    byte col = 4;  // near-white
    byte *fb = I_VideoBuffer;

    if (!crispy.crosshair)
        return;

    // Optionally tint the crosshair by the player's health.
    if (crispy.crosshairhealth)
    {
        int h = players[displayplayer].health;
        if (h < 34)         col = 176;  // red
        else if (h < 67)    col = 231;  // yellow
        else                col = 112;  // green
    }

    // Optionally highlight the crosshair when a shootable target is in view.
    // P_AimLineAttack only queries the world (it rebuilds its own globals),
    // so calling it from the render path does not perturb the playsim.
    if (crispy.crosshairtarget && players[displayplayer].mo)
    {
        mobj_t *mo = players[displayplayer].mo;
        P_AimLineAttack(mo, mo->angle, 16 * 64 * FRACUNIT);
        if (linetarget)
            col = 112;  // green: on target
    }

    cx = viewwindowx + (viewwidth >> 1);
    cy = viewwindowy + (viewheight >> 1);

    switch (crispy.crosshairtype)
    {
      case 1:  // diagonal X
        for (i = -4; i <= 4; i++)
        {
            fb[(cy + i) * SCREENWIDTH + (cx + i)] = col;
            fb[(cy + i) * SCREENWIDTH + (cx - i)] = col;
        }
        break;

      case 2:  // small dot
        for (i = -1; i <= 1; i++)
        {
            fb[cy * SCREENWIDTH + (cx + i)] = col;
            fb[(cy + i) * SCREENWIDTH + cx] = col;
        }
        break;

      default: // plus-shaped cross
        for (i = -4; i <= 4; i++)
        {
            fb[cy * SCREENWIDTH + (cx + i)] = col;
            fb[(cy + i) * SCREENWIDTH + cx] = col;
        }
        break;
    }
}

//
// Crispy_BobFactor
// Fixed-point multiplier for the render-only view/weapon bob.
// crispy.weaponbob: 0 = off .. 4 = full, in 25% steps.
//
fixed_t Crispy_BobFactor(void)
{
    int b = crispy.weaponbob;
    if (b < 0) b = 0;
    if (b > 4) b = 4;
    return b * FRACUNIT / 4;
}

//
// Crispy_DrawCoords
// Shows the display player's map coordinates (top-right).
//
void Crispy_DrawCoords(void)
{
    player_t *p = &players[displayplayer];
    char buf[24];

    if (!crispy.showcoords || gamestate != GS_LEVEL || !p->mo)
        return;

    snprintf(buf, sizeof(buf), "X %d", p->mo->x >> FRACBITS);
    M_WriteText(ORIGWIDTH - 72, 8, buf);
    snprintf(buf, sizeof(buf), "Y %d", p->mo->y >> FRACBITS);
    M_WriteText(ORIGWIDTH - 72, 16, buf);
    snprintf(buf, sizeof(buf), "Z %d", p->mo->z >> FRACBITS);
    M_WriteText(ORIGWIDTH - 72, 24, buf);
    snprintf(buf, sizeof(buf), "A %d",
             (int)(((uint64_t) p->mo->angle * 360) >> 32));
    M_WriteText(ORIGWIDTH - 72, 32, buf);
}

//
// Crispy_DrawStats
// Shows level kills / items / secrets and/or the elapsed level time
// (top-left, under the FPS counter).
//
void Crispy_DrawStats(void)
{
    player_t *p = &players[displayplayer];
    char buf[24];

    if (gamestate != GS_LEVEL)
        return;

    if (crispy.showstats)
    {
        snprintf(buf, sizeof(buf), "K %d/%d", p->killcount, totalkills);
        M_WriteText(2, 12, buf);
        snprintf(buf, sizeof(buf), "I %d/%d", p->itemcount, totalitems);
        M_WriteText(2, 20, buf);
        snprintf(buf, sizeof(buf), "S %d/%d", p->secretcount, totalsecret);
        M_WriteText(2, 28, buf);
    }

    if (crispy.showleveltime)
    {
        int secs = leveltime / TICRATE;
        snprintf(buf, sizeof(buf), "T %d:%02d", secs / 60, secs % 60);
        M_WriteText(2, 36, buf);
    }
}

//
// Crispy_DrawDemoTimer
// During demo playback, shows the elapsed demo time (bottom-left, above the
// status bar) so demo length is visible.
//
void Crispy_DrawDemoTimer(void)
{
    int secs;
    char buf[24];

    if (!crispy.demotimer || !demoplayback || gamestate != GS_LEVEL)
        return;

    secs = leveltime / TICRATE;
    snprintf(buf, sizeof(buf), "DEMO %d:%02d", secs / 60, secs % 60);
    M_WriteText(2, ORIGHEIGHT - 40, buf);
}

//
// Crispy_DrawFPS
// Shows a framerate counter (top-left), recomputed once per second.
//
void Crispy_DrawFPS(void)
{
    static int lasttime = 0, framecount = 0, fps = 0;
    int now;
    char buf[16];

    if (!crispy.showfps)
        return;

    framecount++;
    now = I_GetTimeMS();
    if (now - lasttime >= 1000)
    {
        fps = (now > lasttime) ? framecount * 1000 / (now - lasttime) : 0;
        framecount = 0;
        lasttime = now;
    }

    snprintf(buf, sizeof(buf), "%d FPS", fps);
    M_WriteText(2, 2, buf);
}
