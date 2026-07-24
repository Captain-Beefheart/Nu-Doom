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

// State owned by the renderer / menu that the overlays need.
extern int  viewwindowx, viewwindowy, viewwidth, viewheight;
extern void M_WriteText(int x, int y, char *string);

// All settings default to 0 (off), matching vanilla behaviour.
crispy_t crispy = { 0 };

void M_BindCrispnessVariables(void)
{
    M_BindVariable("crispy_uncapped",      &crispy.uncapped);
    M_BindVariable("crispy_smoothscaling", &crispy.smoothscaling);
    M_BindVariable("crispy_translucency",  &crispy.translucency);
    M_BindVariable("crispy_coloredblood",  &crispy.coloredblood);
    M_BindVariable("crispy_crosshair",     &crispy.crosshair);
    M_BindVariable("crispy_showfps",       &crispy.showfps);
}

//
// Crispy_DrawCrosshair
// Plots a simple plus-shaped crosshair at the center of the 3D view.
// Drawn directly into the 8-bit render buffer (palette index 4 ~ white).
//
void Crispy_DrawCrosshair(void)
{
    int cx, cy, i;
    byte *fb = I_VideoBuffer;

    if (!crispy.crosshair)
        return;

    cx = viewwindowx + (viewwidth >> 1);
    cy = viewwindowy + (viewheight >> 1);

    for (i = -4; i <= 4; i++)
    {
        fb[cy * SCREENWIDTH + (cx + i)] = 4;
        fb[(cy + i) * SCREENWIDTH + cx] = 4;
    }
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
