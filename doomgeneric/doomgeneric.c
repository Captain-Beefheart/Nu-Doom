#include <stdio.h>

#include "m_argv.h"

#include "doomgeneric.h"
#include "i_video.h"   // MAXWIDTH / SCREENHEIGHT

pixel_t* DG_ScreenBuffer = NULL;

void M_FindResponseFile(void);
void D_DoomMain (void);


void doomgeneric_Create(int argc, char **argv)
{
	// save arguments
    myargc = argc;
    myargv = argv;

	M_FindResponseFile();

	// Widescreen: size the present buffer to the widest supported render width.
	DG_ScreenBuffer = malloc(MAXWIDTH * DOOMGENERIC_RESY * 4);

	DG_Init();

	D_DoomMain ();
}

