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

#include "crispy.h"
#include "m_config.h"

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
