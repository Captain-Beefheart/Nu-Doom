//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	DOOM selection menu, options, episode etc.
//	Sliders and icons. Kinda widget stuff.
//


#include <stdlib.h>
#include <ctype.h>


#include "doomdef.h"
#include "doomkeys.h"
#include "dstrings.h"

#include "d_main.h"
#include "deh_main.h"

#include "i_swap.h"
#include "i_system.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#include "r_local.h"


#include "hu_stuff.h"

#include "g_game.h"

#include "m_argv.h"
#include "m_controls.h"
#include "crispy.h"
#include "doomgeneric.h"
#include "p_saveg.h"

#include "s_sound.h"

#include "doomstat.h"

// Data.
#include "sounds.h"

#include "m_menu.h"


extern patch_t*		hu_font[HU_FONTSIZE];
extern boolean		message_dontfuckwithme;

extern boolean		chat_on;		// in heads-up code

//
// defaulted values
//
int			mouseSensitivity = 5;

// Show messages has default, 0 = off, 1 = on
int			showMessages = 1;
	

// Blocky mode, has default, 0 = high, 1 = normal
int			detailLevel = 0;
int			screenblocks = 10;

// temp for screenblocks (0-9)
int			screenSize;

// -1 = no quicksave slot picked!
int			quickSaveSlot;

 // 1 = message to be printed
int			messageToPrint;
// ...and here is the message string!
char*			messageString;

// message x & y
int			messx;
int			messy;
int			messageLastMenuActive;

// timed message = no input from user
boolean			messageNeedsInput;

void    (*messageRoutine)(int response);

char gammamsg[5][26] =
{
    GAMMALVL0,
    GAMMALVL1,
    GAMMALVL2,
    GAMMALVL3,
    GAMMALVL4
};

// we are going to be entering a savegame string
int			saveStringEnter;              
int             	saveSlot;	// which slot to save in
int			saveCharIndex;	// which char we're editing
// old save description before edit
char			saveOldString[SAVESTRINGSIZE];  

boolean			inhelpscreens;
boolean			menuactive;

#define SKULLXOFF		-32
#define LINEHEIGHT		16

extern boolean		sendpause;
char			savegamestrings[10][SAVESTRINGSIZE];

char	endstring[160];

//static boolean opldev;

//
// MENU TYPEDEFS
//
typedef struct
{
    // 0 = no cursor here, 1 = ok, 2 = arrows ok
    short	status;
    
    char	name[10];
    
    // choice = menu item #.
    // if status = 2,
    //   choice=0:leftarrow,1:rightarrow
    void	(*routine)(int choice);
    
    // hotkey in menu
    char	alphaKey;			
} menuitem_t;



typedef struct menu_s
{
    short		numitems;	// # of menu items
    struct menu_s*	prevMenu;	// previous menu
    menuitem_t*		menuitems;	// menu items
    void		(*routine)();	// draw routine
    short		x;
    short		y;		// x,y of menu
    short		lastOn;		// last item user was on in menu
} menu_t;

short		itemOn;			// menu item skull is on
short		skullAnimCounter;	// skull animation counter
short		whichSkull;		// which skull to draw

// graphic name of skulls
// warning: initializer-string for array of chars is too long
char    *skullName[2] = {"M_SKULL1","M_SKULL2"};

// current menudef
menu_t*	currentMenu;                          

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_ChangeDetail(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);
void M_Crispness(int choice);
void M_DrawCrispness(void);
void M_BindKeys(int choice);

void M_FinishReadThis(int choice);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);

void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, char *string);
static void M_WriteTextBig(int x, int y, char *string);
int  M_StringWidth(char *string);
int  M_StringHeight(char *string);
void M_StartMessage(char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);




//
// DOOM MENU
//
enum
{
    newgame = 0,
    options,
    loadgame,
    savegame,
    readthis,
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
    {1,"M_NGAME",M_NewGame,'n'},
    {1,"M_OPTION",M_Options,'o'},
    {1,"M_LOADG",M_LoadGame,'l'},
    {1,"M_SAVEG",M_SaveGame,'s'},
    // Another hickup with Special edition.
    {1,"M_RDTHIS",M_ReadThis,'r'},
    {1,"M_QUITG",M_QuitDOOM,'q'}
};

menu_t  MainDef =
{
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};


//
// EPISODE SELECT
//
enum
{
    ep1,
    ep2,
    ep3,
    ep4,
    ep_end
} episodes_e;

menuitem_t EpisodeMenu[]=
{
    {1,"M_EPI1", M_Episode,'k'},
    {1,"M_EPI2", M_Episode,'t'},
    {1,"M_EPI3", M_Episode,'i'},
    {1,"M_EPI4", M_Episode,'t'}
};

menu_t  EpiDef =
{
    ep_end,		// # of menu items
    &MainDef,		// previous menu
    EpisodeMenu,	// menuitem_t ->
    M_DrawEpisode,	// drawing routine ->
    48,63,              // x,y
    ep1			// lastOn
};

//
// NEW GAME
//
enum
{
    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
    {1,"M_JKILL",	M_ChooseSkill, 'i'},
    {1,"M_ROUGH",	M_ChooseSkill, 'h'},
    {1,"M_HURT",	M_ChooseSkill, 'h'},
    {1,"M_ULTRA",	M_ChooseSkill, 'u'},
    {1,"M_NMARE",	M_ChooseSkill, 'n'}
};

menu_t  NewDef =
{
    newg_end,		// # of menu items
    &EpiDef,		// previous menu
    NewGameMenu,	// menuitem_t ->
    M_DrawNewGame,	// drawing routine ->
    48,63,              // x,y
    hurtme		// lastOn
};



//
// OPTIONS MENU
//
enum
{
    bindkeys,
    messages,
    detail,
    scrnsize,
    option_empty1,
    mousesens,
    option_empty2,
    soundvol,
    crispness,
    opt_end
} options_e;

menuitem_t OptionsMenu[]=
{
    {1,"",		M_BindKeys,'b'},	// text item, drawn by M_DrawOptions
    {1,"M_MESSG",	M_ChangeMessages,'m'},
    {1,"M_DETAIL",	M_ChangeDetail,'g'},
    {2,"M_SCRNSZ",	M_SizeDisplay,'s'},
    {-1,"",0,'\0'},
    {2,"M_MSENS",	M_ChangeSensitivity,'m'},
    {-1,"",0,'\0'},
    {1,"M_SVOL",	M_Sound,'s'},
    {1,"",		M_Crispness,'c'}	// text item, drawn by M_DrawOptions
};

menu_t  OptionsDef =
{
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};

//
// CRISPNESS MENU (Nu-Doom enhancement toggles, saved to crispy-doom.cfg)
// The full set spans three pages; the last item on each page advances to the
// next (wrapping). Backspace/ESC steps back through the prevMenu chain.
//
enum
{
    crisp_uncapped,
    crisp_smoothscaling,
    crisp_vsync,
    crisp_gamma,
    crisp_translucency,
    crisp_coloredblood,
    crisp_soundchannels,
    crisp_nextpage,
    crisp_end
} crispness_e;

enum
{
    crisp2_centerweapon,
    crisp2_weaponbob,
    crisp2_weaponsquat,
    crisp2_crosshair,
    crisp2_crosshairhealth,
    crisp2_crosshairtype,
    crisp2_showfps,
    crisp2_nextpage,
    crisp2_end
} crispness2_e;

enum
{
    crisp3_showcoords,
    crisp3_showstats,
    crisp3_showleveltime,
    crisp3_secretmessage,
    crisp3_automapoverlay,
    crisp3_automaprotate,
    crisp3_automapsecrets,
    crisp3_nextpage,
    crisp3_end
} crispness3_e;

enum
{
    crisp4_automapcolors,
    crisp4_fpslimit,
    crisp4_monosfx,
    crisp4_fullsounds,
    crisp4_demotimer,
    crisp4_crosshairtarget,
    crisp4_mousecontrol,
    crisp4_mouselook,
    crisp4_nextpage,
    crisp4_end
} crispness4_e;

enum
{
    crisp5_recoilpitch,
    crisp5_demobar,
    crisp5_aspectratio,
    crisp5_sfxpitch,
    crisp5_coloredhud,
    crisp5_brightmaps,
    crisp5_nextpage,
    crisp5_end
} crispness5_e;

enum
{
    crisp6_smoothlight,
    crisp6_nextpage,
    crisp6_end
} crispness6_e;

// Boolean toggles.
static void M_CrispUncapped(int choice)       { crispy.uncapped        = !crispy.uncapped; }
static void M_CrispSmoothScaling(int choice)  { crispy.smoothscaling   = !crispy.smoothscaling; }
static void M_CrispVSync(int choice)          { crispy.vsync = !crispy.vsync; DG_SetVSync(crispy.vsync); }
static void M_CrispTranslucency(int choice)   { crispy.translucency    = !crispy.translucency; }
static void M_CrispColoredBlood(int choice)   { crispy.coloredblood    = !crispy.coloredblood; }
static void M_CrispCenterWeapon(int choice)   { crispy.centerweapon    = !crispy.centerweapon; }
static void M_CrispWeaponSquat(int choice)    { crispy.weaponsquat     = !crispy.weaponsquat; }
static void M_CrispCrosshair(int choice)      { crispy.crosshair       = !crispy.crosshair; }
static void M_CrispCrosshairHealth(int choice){ crispy.crosshairhealth = !crispy.crosshairhealth; }
static void M_CrispShowFPS(int choice)        { crispy.showfps         = !crispy.showfps; }
static void M_CrispShowCoords(int choice)     { crispy.showcoords      = !crispy.showcoords; }
static void M_CrispShowStats(int choice)      { crispy.showstats       = !crispy.showstats; }
static void M_CrispShowLevelTime(int choice)  { crispy.showleveltime   = !crispy.showleveltime; }
static void M_CrispSecretMessage(int choice)  { crispy.secretmessage   = !crispy.secretmessage; }
static void M_CrispAutomapOverlay(int choice) { crispy.automapoverlay  = !crispy.automapoverlay; }
static void M_CrispAutomapRotate(int choice)  { crispy.automaprotate   = !crispy.automaprotate; }
static void M_CrispAutomapSecrets(int choice) { crispy.automapsecrets  = !crispy.automapsecrets; }
static void M_CrispAutomapColors(int choice)  { crispy.automapcolors   = !crispy.automapcolors; }
static void M_CrispMonoSFX(int choice)        { crispy.monosfx         = !crispy.monosfx; }
static void M_CrispFullSounds(int choice)     { crispy.fullsounds      = !crispy.fullsounds; }
static void M_CrispDemoTimer(int choice)      { crispy.demotimer       = !crispy.demotimer; }
static void M_CrispCrosshairTarget(int choice){ crispy.crosshairtarget = !crispy.crosshairtarget; }
static void M_CrispMouseControl(int choice)   { crispy.mousecontrol   = !crispy.mousecontrol; }
static void M_CrispMouselook(int choice)
{
    extern int mlookpitch;
    crispy.mouselook = !crispy.mouselook;
    if (!crispy.mouselook)
	mlookpitch = 0;   // recenter the view when turning mouselook off
}
static void M_CrispRecoilPitch(int choice)    { crispy.recoilpitch    = !crispy.recoilpitch; }
static void M_CrispDemoBar(int choice)        { crispy.demobar        = !crispy.demobar; }
static void M_CrispAspectRatio(int choice)    { crispy.aspectratio    = !crispy.aspectratio; }
static void M_CrispSfxPitch(int choice)       { crispy.sfxpitch       = !crispy.sfxpitch; }
static void M_CrispColoredHUD(int choice)     { crispy.coloredhud     = !crispy.coloredhud; }
static void M_CrispBrightmaps(int choice)     { crispy.brightmaps     = !crispy.brightmaps; }
static void M_CrispSmoothLight(int choice)    { crispy.smoothlight    = !crispy.smoothlight; R_SetSmoothLight(); }

// Multi-value settings.
static void M_CrispGamma(int choice)
{
    usegamma = (usegamma + 1) % 5;
    players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
    I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"), PU_CACHE));
}
static void M_CrispWeaponBob(int choice)      { crispy.weaponbob     = (crispy.weaponbob + 1) % 5; }
static void M_CrispCrosshairType(int choice)  { crispy.crosshairtype = (crispy.crosshairtype + 1) % 3; }
static void M_CrispSoundChannels(int choice)
{
    int n = (snd_channels < 16) ? 16 : (snd_channels < 32) ? 32 : 8;
    S_ReallocChannels(n);
}
static void M_CrispFpsLimit(int choice)
{
    static const int steps[6] = { 0, 35, 50, 60, 100, 200 };
    int i, n = 0;
    for (i = 0; i < 6; i++) { if (crispy.fpslimit == steps[i]) { n = i; break; } }
    crispy.fpslimit = steps[(n + 1) % 6];
}

void M_DrawCrispness2(void);
void M_DrawCrispness3(void);
void M_DrawCrispness4(void);
void M_DrawCrispness5(void);
void M_DrawCrispness6(void);
static void M_CrispnessPage2(int choice);
static void M_CrispnessPage3(int choice);
static void M_CrispnessPage4(int choice);
static void M_CrispnessPage5(int choice);
static void M_CrispnessPage6(int choice);
static void M_CrispnessPage1(int choice);

menuitem_t CrispnessMenu[]=
{
    {1,"",M_CrispUncapped,'u'},
    {1,"",M_CrispSmoothScaling,'s'},
    {1,"",M_CrispVSync,'v'},
    {1,"",M_CrispGamma,'g'},
    {1,"",M_CrispTranslucency,'t'},
    {1,"",M_CrispColoredBlood,'b'},
    {1,"",M_CrispSoundChannels,'c'},
    {1,"",M_CrispnessPage2,'n'}
};

menuitem_t Crispness2Menu[]=
{
    {1,"",M_CrispCenterWeapon,'w'},
    {1,"",M_CrispWeaponBob,'b'},
    {1,"",M_CrispWeaponSquat,'q'},
    {1,"",M_CrispCrosshair,'c'},
    {1,"",M_CrispCrosshairHealth,'h'},
    {1,"",M_CrispCrosshairType,'y'},
    {1,"",M_CrispShowFPS,'f'},
    {1,"",M_CrispnessPage3,'n'}
};

menuitem_t Crispness3Menu[]=
{
    {1,"",M_CrispShowCoords,'o'},
    {1,"",M_CrispShowStats,'l'},
    {1,"",M_CrispShowLevelTime,'i'},
    {1,"",M_CrispSecretMessage,'m'},
    {1,"",M_CrispAutomapOverlay,'a'},
    {1,"",M_CrispAutomapRotate,'r'},
    {1,"",M_CrispAutomapSecrets,'e'},
    {1,"",M_CrispnessPage4,'n'}
};

menuitem_t Crispness4Menu[]=
{
    {1,"",M_CrispAutomapColors,'x'},
    {1,"",M_CrispFpsLimit,'l'},
    {1,"",M_CrispMonoSFX,'m'},
    {1,"",M_CrispFullSounds,'u'},
    {1,"",M_CrispDemoTimer,'d'},
    {1,"",M_CrispCrosshairTarget,'t'},
    {1,"",M_CrispMouseControl,'c'},
    {1,"",M_CrispMouselook,'k'},
    {1,"",M_CrispnessPage5,'n'}
};

menuitem_t Crispness5Menu[]=
{
    {1,"",M_CrispRecoilPitch,'r'},
    {1,"",M_CrispDemoBar,'d'},
    {1,"",M_CrispAspectRatio,'a'},
    {1,"",M_CrispSfxPitch,'p'},
    {1,"",M_CrispColoredHUD,'h'},
    {1,"",M_CrispBrightmaps,'b'},
    {1,"",M_CrispnessPage6,'n'}
};

menuitem_t Crispness6Menu[]=
{
    {1,"",M_CrispSmoothLight,'s'},
    {1,"",M_CrispnessPage1,'n'}
};

menu_t  CrispnessDef =
{
    crisp_end,
    &OptionsDef,	// back returns to Options
    CrispnessMenu,
    M_DrawCrispness,
    48,32,
    0
};

menu_t  Crispness2Def =
{
    crisp2_end,
    &CrispnessDef,	// back returns to page 1
    Crispness2Menu,
    M_DrawCrispness2,
    48,32,
    0
};

menu_t  Crispness3Def =
{
    crisp3_end,
    &Crispness2Def,	// back returns to page 2
    Crispness3Menu,
    M_DrawCrispness3,
    48,32,
    0
};

menu_t  Crispness4Def =
{
    crisp4_end,
    &Crispness3Def,	// back returns to page 3
    Crispness4Menu,
    M_DrawCrispness4,
    48,32,
    0
};

menu_t  Crispness5Def =
{
    crisp5_end,
    &Crispness4Def,	// back returns to page 4
    Crispness5Menu,
    M_DrawCrispness5,
    48,32,
    0
};

menu_t  Crispness6Def =
{
    crisp6_end,
    &Crispness5Def,	// back returns to page 5
    Crispness6Menu,
    M_DrawCrispness6,
    48,32,
    0
};

static void M_CrispnessPage2(int choice) { M_SetupNextMenu(&Crispness2Def); }
static void M_CrispnessPage3(int choice) { M_SetupNextMenu(&Crispness3Def); }
static void M_CrispnessPage4(int choice) { M_SetupNextMenu(&Crispness4Def); }
static void M_CrispnessPage5(int choice) { M_SetupNextMenu(&Crispness5Def); }
static void M_CrispnessPage6(int choice) { M_SetupNextMenu(&Crispness6Def); }
static void M_CrispnessPage1(int choice) { M_SetupNextMenu(&CrispnessDef); }

//
// Read This! MENU 1 & 2
//
enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {1,"",M_ReadThis2,0}
};

menu_t  ReadDef1 =
{
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {1,"",M_FinishReadThis,0}
};

menu_t  ReadDef2 =
{
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};

//
// SOUND VOLUME MENU
//
enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {2,"M_SFXVOL",M_SfxVol,'s'},
    {-1,"",0,'\0'},
    {2,"M_MUSVOL",M_MusicVol,'m'},
    {-1,"",0,'\0'}
};

menu_t  SoundDef =
{
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawSound,
    80,64,
    0
};

//
// LOAD GAME MENU
//
enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,        // Nu-Doom: extended savegame slots (8 total)
    load8,
    load_end
} load_e;

menuitem_t LoadMenu[]=
{
    {1,"", M_LoadSelect,'1'},
    {1,"", M_LoadSelect,'2'},
    {1,"", M_LoadSelect,'3'},
    {1,"", M_LoadSelect,'4'},
    {1,"", M_LoadSelect,'5'},
    {1,"", M_LoadSelect,'6'},
    {1,"", M_LoadSelect,'7'},
    {1,"", M_LoadSelect,'8'}
};

menu_t  LoadDef =
{
    load_end,
    &MainDef,
    LoadMenu,
    M_DrawLoad,
    80,44,
    0
};

//
// SAVE GAME MENU
//
menuitem_t SaveMenu[]=
{
    {1,"", M_SaveSelect,'1'},
    {1,"", M_SaveSelect,'2'},
    {1,"", M_SaveSelect,'3'},
    {1,"", M_SaveSelect,'4'},
    {1,"", M_SaveSelect,'5'},
    {1,"", M_SaveSelect,'6'},
    {1,"", M_SaveSelect,'7'},
    {1,"", M_SaveSelect,'8'}
};

menu_t  SaveDef =
{
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,44,
    0
};


//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
    FILE   *handle;
    int     i;
    char    name[256];

    for (i = 0;i < load_end;i++)
    {
        M_StringCopy(name, P_SaveGameFile(i), sizeof(name));

	handle = fopen(name, "rb");
        if (handle == NULL)
        {
            M_StringCopy(savegamestrings[i], EMPTYSTRING, SAVESTRINGSIZE);
            LoadMenu[i].status = 0;
            continue;
        }
	fread(&savegamestrings[i], 1, SAVESTRINGSIZE, handle);
	fclose(handle);
	LoadMenu[i].status = 1;
    }
}


//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;
	
    V_DrawPatchDirect(72, 28, 
                      W_CacheLumpName(DEH_String("M_LOADG"), PU_CACHE));

    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
}



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;
	
    V_DrawPatchDirect(x - 8, y + 7,
                      W_CacheLumpName(DEH_String("M_LSLEFT"), PU_CACHE));
	
    for (i = 0;i < 24;i++)
    {
	V_DrawPatchDirect(x, y + 7,
                          W_CacheLumpName(DEH_String("M_LSCNTR"), PU_CACHE));
	x += 8;
    }

    V_DrawPatchDirect(x, y + 7, 
                      W_CacheLumpName(DEH_String("M_LSRGHT"), PU_CACHE));
}



//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    char    name[256];
	
    M_StringCopy(name, P_SaveGameFile(choice), sizeof(name));

    G_LoadGame (name);
    M_ClearMenus ();
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
    if (netgame)
    {
	M_StartMessage(DEH_String(LOADNET),NULL,false);
	return;
    }
	
    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int             i;
	
    V_DrawPatchDirect(72, 28, W_CacheLumpName(DEH_String("M_SAVEG"), PU_CACHE));
    for (i = 0;i < load_end; i++)
    {
	M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
	M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }
	
    if (saveStringEnter)
    {
	i = M_StringWidth(savegamestrings[saveSlot]);
	M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus ();

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
	quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;
    
    saveSlot = choice;
    M_StringCopy(saveOldString,savegamestrings[choice], SAVESTRINGSIZE);
    if (!strcmp(savegamestrings[choice], EMPTYSTRING))
	savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if (!usergame)
    {
	M_StartMessage(DEH_String(SAVEDEAD),NULL,false);
	return;
    }
	
    if (gamestate != GS_LEVEL)
	return;
	
    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}



//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int key)
{
    if (key == key_menu_confirm)
    {
	M_DoSave(quickSaveSlot);
	S_StartSound(NULL,sfx_swtchx);
    }
}

void M_QuickSave(void)
{
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }

    if (gamestate != GS_LEVEL)
	return;
	
    if (quickSaveSlot < 0)
    {
	M_StartControlPanel();
	M_ReadSaveStrings();
	M_SetupNextMenu(&SaveDef);
	quickSaveSlot = -2;	// means to pick a slot now
	return;
    }
    DEH_snprintf(tempstring, 80, QSPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,true);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int key)
{
    if (key == key_menu_confirm)
    {
	M_LoadSelect(quickSaveSlot);
	S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
	M_StartMessage(DEH_String(QLOADNET),NULL,false);
	return;
    }
	
    if (quickSaveSlot < 0)
    {
	M_StartMessage(DEH_String(QSAVESPOT),NULL,false);
	return;
    }
    DEH_snprintf(tempstring, 80, QLPROMPT, savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,true);
}




//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    char *lumpname = "CREDIT";
    int skullx = 330, skully = 175;

    inhelpscreens = true;
    
    // Different versions of Doom 1.9 work differently

    switch (gameversion)
    {
        case exe_doom_1_666:
        case exe_doom_1_7:
        case exe_doom_1_8:
        case exe_doom_1_9:
        case exe_hacx:

            if (gamemode == commercial)
            {
                // Doom 2

                lumpname = "HELP";

                skullx = 330;
                skully = 165;
            }
            else
            {
                // Doom 1
                // HELP2 is the first screen shown in Doom 1
                
                lumpname = "HELP2";

                skullx = 280;
                skully = 185;
            }
            break;

        case exe_ultimate:
        case exe_chex:

            // Ultimate Doom always displays "HELP1".

            // Chex Quest version also uses "HELP1", even though it is based
            // on Final Doom.

            lumpname = "HELP1";

            break;

        case exe_final:
        case exe_final2:

            // Final Doom always displays "HELP".

            lumpname = "HELP";

            break;

        default:
            I_Error("Unhandled game version");
            break;
    }

    lumpname = DEH_String(lumpname);
    
    V_DrawPatchDirect (0, 0, W_CacheLumpName(lumpname, PU_CACHE));

    ReadDef1.x = skullx;
    ReadDef1.y = skully;
}



//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    inhelpscreens = true;

    // We only ever draw the second page if this is 
    // gameversion == exe_doom_1_9 and gamemode == registered

    V_DrawPatchDirect(0, 0, W_CacheLumpName(DEH_String("HELP1"), PU_CACHE));
}


//
// Change Sfx & Music volumes
//
void M_DrawSound(void)
{
    V_DrawPatchDirect (60, 38, W_CacheLumpName(DEH_String("M_SVOL"), PU_CACHE));

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),
		 16,sfxVolume);

    M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),
		 16,musicVolume);
}

void M_Sound(int choice)
{
    M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
    switch(choice)
    {
      case 0:
	if (sfxVolume)
	    sfxVolume--;
	break;
      case 1:
	if (sfxVolume < 15)
	    sfxVolume++;
	break;
    }
	
    S_SetSfxVolume(sfxVolume * 8);
}

void M_MusicVol(int choice)
{
    switch(choice)
    {
      case 0:
	if (musicVolume)
	    musicVolume--;
	break;
      case 1:
	if (musicVolume < 15)
	    musicVolume++;
	break;
    }
	
    S_SetMusicVolume(musicVolume * 8);
}




//
// M_DrawMainMenu
//
void M_DrawMainMenu(void)
{
    V_DrawPatchDirect(94, 2,
                      W_CacheLumpName(DEH_String("M_DOOM"), PU_CACHE));
}




//
// M_NewGame
//
void M_DrawNewGame(void)
{
    V_DrawPatchDirect(96, 14, W_CacheLumpName(DEH_String("M_NEWG"), PU_CACHE));
    V_DrawPatchDirect(54, 38, W_CacheLumpName(DEH_String("M_SKILL"), PU_CACHE));
}

void M_NewGame(int choice)
{
    if (netgame && !demoplayback)
    {
	M_StartMessage(DEH_String(NEWGAME),NULL,false);
	return;
    }
	
    // Chex Quest disabled the episode select screen, as did Doom II.

    if (gamemode == commercial || gameversion == exe_chex)
	M_SetupNextMenu(&NewDef);
    else
	M_SetupNextMenu(&EpiDef);
}


//
//      M_Episode
//
int     epi;

void M_DrawEpisode(void)
{
    V_DrawPatchDirect(54, 38, W_CacheLumpName(DEH_String("M_EPISOD"), PU_CACHE));
}

void M_VerifyNightmare(int key)
{
    if (key != key_menu_confirm)
	return;
		
    G_DeferedInitNew(nightmare,epi+1,1);
    M_ClearMenus ();
}

void M_ChooseSkill(int choice)
{
    if (choice == nightmare)
    {
	M_StartMessage(DEH_String(NIGHTMARE),M_VerifyNightmare,true);
	return;
    }
	
    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();
}

void M_Episode(int choice)
{
    if ( (gamemode == shareware)
	 && choice)
    {
	M_StartMessage(DEH_String(SWSTRING),NULL,false);
	M_SetupNextMenu(&ReadDef1);
	return;
    }

    // Yet another hack...
    if ( (gamemode == registered)
	 && (choice > 2))
    {
      fprintf( stderr,
	       "M_Episode: 4th episode requires UltimateDOOM\n");
      choice = 0;
    }
	 
    epi = choice;
    M_SetupNextMenu(&NewDef);
}



//
// M_Options
//
static char *detailNames[2] = {"M_GDHIGH","M_GDLOW"};
static char *msgNames[2] = {"M_MSGOFF","M_MSGON"};

void M_DrawOptions(void)
{
    V_DrawPatchDirect(108, 15, W_CacheLumpName(DEH_String("M_OPTTTL"),
                                               PU_CACHE));

    // "Bind Keys" submenu entry (drawn as big text; replaces the End Game item,
    // which stays reachable via its F-key shortcut).
    M_WriteTextBig(OptionsDef.x, OptionsDef.y + LINEHEIGHT*bindkeys, "Bind Keys");

    V_DrawPatchDirect(OptionsDef.x + 175, OptionsDef.y + LINEHEIGHT * detail,
		      W_CacheLumpName(DEH_String(detailNames[detailLevel]),
			              PU_CACHE));

    V_DrawPatchDirect(OptionsDef.x + 120, OptionsDef.y + LINEHEIGHT * messages,
                      W_CacheLumpName(DEH_String(msgNames[showMessages]),
                                      PU_CACHE));

    M_DrawThermo(OptionsDef.x, OptionsDef.y + LINEHEIGHT * (mousesens + 1),
		 10, mouseSensitivity);

    M_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
		 9,screenSize);

    // "Nu-Doom Options" submenu entry (drawn as big text; no WAD patch for it,
    // but sized to match the other option items).
    M_WriteTextBig(OptionsDef.x, OptionsDef.y + LINEHEIGHT*crispness, "Nu-Doom Options");
}

void M_Options(int choice)
{
    M_SetupNextMenu(&OptionsDef);
}

//
// M_Crispness
//
static char *crispOnOff[2] = { "Off", "On" };
static char *crispBob[5]   = { "Off", "25%", "50%", "75%", "Full" };
static char *crispXhair[3] = { "Cross", "X", "Dot" };

// The Crispness submenu is drawn with the standard small hu_font (its original
// size). Labels sit on the left; values line up in a fixed right-hand column.
static void M_DrawCrispnessStr(int row, char *label, char *value)
{
    int y = CrispnessDef.y + LINEHEIGHT * row;
    M_WriteText(CrispnessDef.x, y, label);
    M_WriteText(CrispnessDef.x + 176, y, value);
}

static void M_DrawCrispnessItem(int row, char *label, int value)
{
    M_DrawCrispnessStr(row, label, crispOnOff[value != 0]);
}

static void M_DrawCrispnessNav(int row, char *label)
{
    M_WriteText(CrispnessDef.x, CrispnessDef.y + LINEHEIGHT * row, label);
}

void M_DrawCrispness(void)
{
    char buf[16];

    M_WriteText(CrispnessDef.x, CrispnessDef.y - 16, "NU-DOOM OPTIONS  1/6");

    M_DrawCrispnessItem(crisp_uncapped,      "Uncapped framerate",   crispy.uncapped);
    M_DrawCrispnessItem(crisp_smoothscaling, "Smooth pixel scaling", crispy.smoothscaling);
    M_DrawCrispnessItem(crisp_vsync,         "Vertical sync",        crispy.vsync);
    snprintf(buf, sizeof(buf), "%d", usegamma);
    M_DrawCrispnessStr (crisp_gamma,         "Gamma correction",     usegamma ? buf : "Off");
    M_DrawCrispnessItem(crisp_translucency,  "Translucency",         crispy.translucency);
    M_DrawCrispnessItem(crisp_coloredblood,  "Colored blood",        crispy.coloredblood);
    snprintf(buf, sizeof(buf), "%d", snd_channels);
    M_DrawCrispnessStr (crisp_soundchannels, "Sound channels",       buf);
    M_DrawCrispnessNav (crisp_nextpage,      "Next page >");
}

void M_DrawCrispness2(void)
{
    M_WriteText(Crispness2Def.x, Crispness2Def.y - 16, "NU-DOOM OPTIONS  2/6");

    M_DrawCrispnessItem(crisp2_centerweapon,   "Center weapon",    crispy.centerweapon);
    M_DrawCrispnessStr (crisp2_weaponbob,      "Weapon bob",       crispBob[crispy.weaponbob % 5]);
    M_DrawCrispnessItem(crisp2_weaponsquat,    "Weapon squat",     crispy.weaponsquat);
    M_DrawCrispnessItem(crisp2_crosshair,      "Crosshair",        crispy.crosshair);
    M_DrawCrispnessItem(crisp2_crosshairhealth,"Health crosshair", crispy.crosshairhealth);
    M_DrawCrispnessStr (crisp2_crosshairtype,  "Crosshair type",   crispXhair[crispy.crosshairtype % 3]);
    M_DrawCrispnessItem(crisp2_showfps,        "Show FPS",         crispy.showfps);
    M_DrawCrispnessNav (crisp2_nextpage,       "Next page >");
}

void M_DrawCrispness3(void)
{
    M_WriteText(Crispness3Def.x, Crispness3Def.y - 16, "NU-DOOM OPTIONS  3/6");

    M_DrawCrispnessItem(crisp3_showcoords,     "Show coordinates", crispy.showcoords);
    M_DrawCrispnessItem(crisp3_showstats,      "Show level stats", crispy.showstats);
    M_DrawCrispnessItem(crisp3_showleveltime,  "Show level time",  crispy.showleveltime);
    M_DrawCrispnessItem(crisp3_secretmessage,  "Report secrets",   crispy.secretmessage);
    M_DrawCrispnessItem(crisp3_automapoverlay, "Automap overlay",  crispy.automapoverlay);
    M_DrawCrispnessItem(crisp3_automaprotate,  "Automap rotate",   crispy.automaprotate);
    M_DrawCrispnessItem(crisp3_automapsecrets, "Automap secrets",  crispy.automapsecrets);
    M_DrawCrispnessNav (crisp3_nextpage,       "Next page >");
}

void M_DrawCrispness4(void)
{
    char buf[16];

    M_WriteText(Crispness4Def.x, Crispness4Def.y - 16, "NU-DOOM OPTIONS  4/6");

    M_DrawCrispnessItem(crisp4_automapcolors,   "Automap ext. colors", crispy.automapcolors);
    if (crispy.fpslimit > 0)
        snprintf(buf, sizeof(buf), "%d", crispy.fpslimit);
    M_DrawCrispnessStr (crisp4_fpslimit,        "Framerate limit",     crispy.fpslimit ? buf : "Off");
    M_DrawCrispnessItem(crisp4_monosfx,         "Mono SFX",            crispy.monosfx);
    M_DrawCrispnessItem(crisp4_fullsounds,      "Full-length sounds",  crispy.fullsounds);
    M_DrawCrispnessItem(crisp4_demotimer,       "Demo timer",          crispy.demotimer);
    M_DrawCrispnessItem(crisp4_crosshairtarget, "Target crosshair",    crispy.crosshairtarget);
    M_DrawCrispnessStr (crisp4_mousecontrol,    "Controls",
			crispy.mousecontrol ? "Kb+Mouse" : "Keyboard");
    M_DrawCrispnessItem(crisp4_mouselook,       "Mouselook",           crispy.mouselook);
    M_DrawCrispnessNav (crisp4_nextpage,        "Next page >");
}

void M_DrawCrispness5(void)
{
    M_WriteText(Crispness5Def.x, Crispness5Def.y - 16, "NU-DOOM OPTIONS  5/6");

    M_DrawCrispnessItem(crisp5_recoilpitch, "Weapon recoil",    crispy.recoilpitch);
    M_DrawCrispnessItem(crisp5_demobar,     "Demo progress bar",crispy.demobar);
    M_DrawCrispnessItem(crisp5_aspectratio, "Aspect ratio 4:3", crispy.aspectratio);
    M_DrawCrispnessItem(crisp5_sfxpitch,    "SFX pitch shift",  crispy.sfxpitch);
    M_DrawCrispnessItem(crisp5_coloredhud,  "Colored HUD nums", crispy.coloredhud);
    M_DrawCrispnessItem(crisp5_brightmaps,  "Brightmaps",       crispy.brightmaps);
    M_DrawCrispnessNav (crisp5_nextpage,    "Next page >");
}

void M_DrawCrispness6(void)
{
    M_WriteText(Crispness6Def.x, Crispness6Def.y - 16, "NU-DOOM OPTIONS  6/6");

    M_DrawCrispnessItem(crisp6_smoothlight,   "Smooth lighting", crispy.smoothlight);
    M_DrawCrispnessNav (crisp6_nextpage,      "< First page");
}

void M_Crispness(int choice)
{
    M_SetupNextMenu(&CrispnessDef);
}

//
// BIND KEYS MENU (Options -> Bind Keys)
// Lists the gameplay actions and the key each is bound to. Move the cursor with
// up/down and press Enter to (re)bind: the row shows a prompt and the next key
// pressed becomes the new binding (ESC cancels). Two pages; the last item on
// each steps to the other. Rebindings are the same key_* config variables the
// game reads, so they persist to default.cfg on exit.
//

// The bindable actions, in menu order. Page 1 shows [0..8], page 2 [9..17].
// `mouseb` is the matching mouse-button variable (a button number, -1 = none),
// or NULL for actions the engine can't drive from the mouse. Pressing a key
// while binding sets `key`; pressing a mouse button sets `mouseb`.
static struct
{
    char	*label;
    int		*key;
    int		*mouseb;
} bindkey_actions[] =
{
    { "Fire",         &key_fire,        &mousebfire        },
    { "Use / Open",   &key_use,         &mousebuse         },
    { "Move Forward", &key_up,          &mousebforward     },
    { "Move Back",    &key_down,        &mousebbackward    },
    { "Strafe Left",  &key_strafeleft,  &mousebstrafeleft  },
    { "Strafe Right", &key_straferight, &mousebstraferight },
    { "Turn Left",    &key_left,        NULL               },
    { "Turn Right",   &key_right,       NULL               },
    { "Run",          &key_speed,       NULL               },
    { "Strafe On",    &key_strafe,      &mousebstrafe      },
    { "Jump",         &key_jump,        &mousebjump        },
    { "Weapon 1",     &key_weapon1,     NULL               },
    { "Weapon 2",     &key_weapon2,     NULL               },
    { "Weapon 3",     &key_weapon3,     NULL               },
    { "Weapon 4",     &key_weapon4,     NULL               },
    { "Weapon 5",     &key_weapon5,     NULL               },
    { "Weapon 6",     &key_weapon6,     NULL               },
    { "Weapon 7",     &key_weapon7,     NULL               },
};

#define BINDKEY_PAGE1	9	// actions on page 1 (rest go on page 2)
#define BINDKEY_COUNT	(int)(sizeof(bindkey_actions)/sizeof(bindkey_actions[0]))

// While true, M_Responder captures the next input and assigns it to the
// selected action instead of driving the menu. A keyboard press sets
// *bindkey_target; a mouse button sets *bindkey_mouse (NULL = action has no
// mouse binding).
boolean		askforkey = false;
static int	*bindkey_target = NULL;
static int	*bindkey_mouse  = NULL;

static void M_BindKeyPage1(int choice);
static void M_BindKeyPage2(int choice);
static void M_BindKeyStartA(int choice);
static void M_BindKeyStartB(int choice);
static void M_DrawBindKeys1(void);
static void M_DrawBindKeys2(void);

menuitem_t BindKeys1Menu[]=
{
    {1,"",M_BindKeyStartA,'f'},
    {1,"",M_BindKeyStartA,'u'},
    {1,"",M_BindKeyStartA,'w'},
    {1,"",M_BindKeyStartA,'b'},
    {1,"",M_BindKeyStartA,'a'},
    {1,"",M_BindKeyStartA,'d'},
    {1,"",M_BindKeyStartA,'l'},
    {1,"",M_BindKeyStartA,'t'},
    {1,"",M_BindKeyStartA,'r'},
    {1,"",M_BindKeyPage2,  'n'}
};

menuitem_t BindKeys2Menu[]=
{
    {1,"",M_BindKeyStartB,'s'},
    {1,"",M_BindKeyStartB,'j'},
    {1,"",M_BindKeyStartB,'1'},
    {1,"",M_BindKeyStartB,'2'},
    {1,"",M_BindKeyStartB,'3'},
    {1,"",M_BindKeyStartB,'4'},
    {1,"",M_BindKeyStartB,'5'},
    {1,"",M_BindKeyStartB,'6'},
    {1,"",M_BindKeyStartB,'7'},
    {1,"",M_BindKeyPage1,  'n'}
};

menu_t  BindKeys1Def =
{
    10,
    &OptionsDef,	// back returns to Options
    BindKeys1Menu,
    M_DrawBindKeys1,
    48,28,
    0
};

menu_t  BindKeys2Def =
{
    10,
    &BindKeys1Def,	// back returns to page 1
    BindKeys2Menu,
    M_DrawBindKeys2,
    48,28,
    0
};

static void M_BindKeyPage1(int choice) { M_SetupNextMenu(&BindKeys1Def); }
static void M_BindKeyPage2(int choice) { M_SetupNextMenu(&BindKeys2Def); }

// Begin capturing an input for action `idx` (global index into bindkey_actions).
static void M_BindKeyStart(int idx)
{
    if (idx < 0 || idx >= BINDKEY_COUNT)
	return;
    askforkey      = true;
    bindkey_target = bindkey_actions[idx].key;
    bindkey_mouse  = bindkey_actions[idx].mouseb;
}

static void M_BindKeyStartA(int choice) { M_BindKeyStart(choice); }
static void M_BindKeyStartB(int choice) { M_BindKeyStart(BINDKEY_PAGE1 + choice); }

// Human-readable name for a DOOM key code (as stored in the key_* variables).
static char *M_KeyName(int key)
{
    static char buf[8];

    switch (key)
    {
      case 0:              return "---";
      case KEY_RIGHTARROW: return "RIGHT";
      case KEY_LEFTARROW:  return "LEFT";
      case KEY_UPARROW:    return "UP";
      case KEY_DOWNARROW:  return "DOWN";
      case KEY_ENTER:      return "ENTER";
      case KEY_TAB:        return "TAB";
      case KEY_ESCAPE:     return "ESC";
      case KEY_BACKSPACE:  return "BKSP";
      case KEY_PAUSE:      return "PAUSE";
      case KEY_RSHIFT:     return "SHIFT";
      case KEY_RCTRL:      return "CTRL";
      case KEY_RALT:       return "ALT";
      case ' ':            return "SPACE";
      // doomgeneric's platform layer sends these action codes for real keys
      // (Space -> USE, Ctrl -> FIRE); name them for what the player presses.
      case KEY_FIRE:       return "CTRL";
      case KEY_USE:        return "SPACE";
      case KEY_STRAFE_L:   return "STR.L";
      case KEY_STRAFE_R:   return "STR.R";
      case KEY_F1:  return "F1";  case KEY_F2:  return "F2";
      case KEY_F3:  return "F3";  case KEY_F4:  return "F4";
      case KEY_F5:  return "F5";  case KEY_F6:  return "F6";
      case KEY_F7:  return "F7";  case KEY_F8:  return "F8";
      case KEY_F9:  return "F9";  case KEY_F10: return "F10";
      case KEY_F11: return "F11"; case KEY_F12: return "F12";
      case KEY_HOME: return "HOME"; case KEY_END: return "END";
      case KEY_PGUP: return "PGUP"; case KEY_PGDN: return "PGDN";
      case KEY_INS:  return "INS";  case KEY_DEL: return "DEL";
      case KEY_CAPSLOCK: return "CAPS";
    }

    if (key > ' ' && key < KEY_BACKSPACE)   // printable ASCII
    {
	buf[0] = toupper(key);
	buf[1] = '\0';
	return buf;
    }

    M_snprintf(buf, sizeof(buf), "#%d", key);
    return buf;
}

// Current binding for action `idx`, as shown in the right-hand column:
// the key name, plus "MBn" for a bound mouse button ("---" if neither).
static char *M_BindValue(int idx)
{
    static char buf[24];

    buf[0] = '\0';

    if (*bindkey_actions[idx].key != 0)
	M_StringCopy(buf, M_KeyName(*bindkey_actions[idx].key), sizeof(buf));

    if (bindkey_actions[idx].mouseb != NULL && *bindkey_actions[idx].mouseb >= 0)
    {
	char mb[8];
	M_snprintf(mb, sizeof(mb), "%sMB%d",
		   buf[0] ? " " : "", *bindkey_actions[idx].mouseb + 1);
	M_StringConcat(buf, mb, sizeof(buf));
    }

    if (buf[0] == '\0')
	M_StringCopy(buf, "---", sizeof(buf));

    return buf;
}

// Draw one page: title, then `count` action rows starting at global index
// `base`, then the page-flip line.
static void M_DrawBindKeysPage(menu_t *def, char *title, int base, int count,
			       char *nav)
{
    int i, y;

    M_WriteText(def->x, def->y - 16, title);

    for (i = 0; i < count; i++)
    {
	int idx = base + i;
	y = def->y + LINEHEIGHT * i;
	M_WriteText(def->x, y, bindkey_actions[idx].label);
	M_WriteText(def->x + 176, y,
		    (askforkey && bindkey_target == bindkey_actions[idx].key)
			? "<press>" : M_BindValue(idx));
    }

    M_WriteText(def->x, def->y + LINEHEIGHT * count, nav);

    if (askforkey)
	M_WriteText(def->x, def->y + LINEHEIGHT * (count + 1) + 4,
		    "Press key or mouse button  -  ESC cancels");
}

static void M_DrawBindKeys1(void)
{
    M_DrawBindKeysPage(&BindKeys1Def, "BIND KEYS  1/2",
		       0, BINDKEY_PAGE1, "Next page >");
}

static void M_DrawBindKeys2(void)
{
    M_DrawBindKeysPage(&BindKeys2Def, "BIND KEYS  2/2",
		       BINDKEY_PAGE1, BINDKEY_COUNT - BINDKEY_PAGE1,
		       "< First page");
}

void M_BindKeys(int choice)
{
    askforkey = false;
    M_SetupNextMenu(&BindKeys1Def);
}



//
//      Toggle messages on/off
//
void M_ChangeMessages(int choice)
{
    // warning: unused parameter `int choice'
    choice = 0;
    showMessages = 1 - showMessages;
	
    if (!showMessages)
	players[consoleplayer].message = DEH_String(MSGOFF);
    else
	players[consoleplayer].message = DEH_String(MSGON);

    message_dontfuckwithme = true;
}


//
// M_EndGame
//
void M_EndGameResponse(int key)
{
    if (key != key_menu_confirm)
	return;
		
    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
    D_StartTitle ();
}

void M_EndGame(int choice)
{
    choice = 0;
    if (!usergame)
    {
	S_StartSound(NULL,sfx_oof);
	return;
    }
	
    if (netgame)
    {
	M_StartMessage(DEH_String(NETEND),NULL,false);
	return;
    }
	
    M_StartMessage(DEH_String(ENDGAME),M_EndGameResponse,true);
}




//
// M_ReadThis
//
void M_ReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
    // Doom 1.9 had two menus when playing Doom 1
    // All others had only one

    if (gameversion <= exe_doom_1_9 && gamemode != commercial)
    {
        choice = 0;
        M_SetupNextMenu(&ReadDef2);
    }
    else
    {
        // Close the menu

        M_FinishReadThis(0);
    }
}

void M_FinishReadThis(int choice)
{
    choice = 0;
    M_SetupNextMenu(&MainDef);
}




//
// M_QuitDOOM
//
int     quitsounds[8] =
{
    sfx_pldeth,
    sfx_dmpain,
    sfx_popain,
    sfx_slop,
    sfx_telept,
    sfx_posit1,
    sfx_posit3,
    sfx_sgtatk
};

int     quitsounds2[8] =
{
    sfx_vilact,
    sfx_getpow,
    sfx_boscub,
    sfx_slop,
    sfx_skeswg,
    sfx_kntdth,
    sfx_bspact,
    sfx_sgtatk
};



void M_QuitResponse(int key)
{
    if (key != key_menu_confirm)
	return;
    if (!netgame)
    {
	if (gamemode == commercial)
	    S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
	else
	    S_StartSound(NULL,quitsounds[(gametic>>2)&7]);
	I_WaitVBL(105);
    }
    I_Quit ();
}


static char *M_SelectEndMessage(void)
{
    char **endmsg;

    if (logical_gamemission == doom)
    {
        // Doom 1

        endmsg = doom1_endmsg;
    }
    else
    {
        // Doom 2
        
        endmsg = doom2_endmsg;
    }

    return endmsg[gametic % NUM_QUITMESSAGES];
}


void M_QuitDOOM(int choice)
{
    DEH_snprintf(endstring, sizeof(endstring), "%s\n\n" DOSY,
                 DEH_String(M_SelectEndMessage()));

    M_StartMessage(endstring,M_QuitResponse,true);
}




void M_ChangeSensitivity(int choice)
{
    switch(choice)
    {
      case 0:
	if (mouseSensitivity)
	    mouseSensitivity--;
	break;
      case 1:
	if (mouseSensitivity < 9)
	    mouseSensitivity++;
	break;
    }
}




void M_ChangeDetail(int choice)
{
    choice = 0;
    detailLevel = 1 - detailLevel;

    R_SetViewSize (screenblocks, detailLevel);

    if (!detailLevel)
	players[consoleplayer].message = DEH_String(DETAILHI);
    else
	players[consoleplayer].message = DEH_String(DETAILLO);
}




void M_SizeDisplay(int choice)
{
    switch(choice)
    {
      case 0:
	if (screenSize > 0)
	{
	    screenblocks--;
	    screenSize--;
	}
	break;
      case 1:
	if (screenSize < 8)
	{
	    screenblocks++;
	    screenSize++;
	}
	break;
    }
	

    R_SetViewSize (screenblocks, detailLevel);
}




//
//      Menu Functions
//
void
M_DrawThermo
( int	x,
  int	y,
  int	thermWidth,
  int	thermDot )
{
    int		xx;
    int		i;

    xx = x;
    V_DrawPatchDirect(xx, y, W_CacheLumpName(DEH_String("M_THERML"), PU_CACHE));
    xx += 8;
    for (i=0;i<thermWidth;i++)
    {
	V_DrawPatchDirect(xx, y, W_CacheLumpName(DEH_String("M_THERMM"), PU_CACHE));
	xx += 8;
    }
    V_DrawPatchDirect(xx, y, W_CacheLumpName(DEH_String("M_THERMR"), PU_CACHE));

    V_DrawPatchDirect((x + 8) + thermDot * 8, y,
		      W_CacheLumpName(DEH_String("M_THERMO"), PU_CACHE));
}



void
M_DrawEmptyCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 
                      W_CacheLumpName(DEH_String("M_CELL1"), PU_CACHE));
}

void
M_DrawSelCell
( menu_t*	menu,
  int		item )
{
    V_DrawPatchDirect(menu->x - 10, menu->y + item * LINEHEIGHT - 1,
                      W_CacheLumpName(DEH_String("M_CELL2"), PU_CACHE));
}


void
M_StartMessage
( char*		string,
  void*		routine,
  boolean	input )
{
    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;
    return;
}


void M_StopMessage(void)
{
    menuactive = messageLastMenuActive;
    messageToPrint = 0;
}



//
// Find string width from hu_font chars
//
int M_StringWidth(char* string)
{
    size_t             i;
    int             w = 0;
    int             c;
	
    for (i = 0;i < strlen(string);i++)
    {
	c = toupper(string[i]) - HU_FONTSTART;
	if (c < 0 || c >= HU_FONTSIZE)
	    w += 4;
	else
	    w += SHORT (hu_font[c]->width);
    }
		
    return w;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    size_t             i;
    int             h;
    int             height = SHORT(hu_font[0]->height);
	
    h = height;
    for (i = 0;i < strlen(string);i++)
	if (string[i] == '\n')
	    h += height;
		
    return h;
}


//
//      Write a string using the hu_font
//
void
M_WriteText
( int		x,
  int		y,
  char*		string)
{
    int		w;
    char*	ch;
    int		c;
    int		cx;
    int		cy;
		

    ch = string;
    cx = x;
    cy = y;
	
    while(1)
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = x;
	    cy += 12;
	    continue;
	}
		
	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c>= HU_FONTSIZE)
	{
	    cx += 4;
	    continue;
	}
		
	w = SHORT (hu_font[c]->width);
	if (cx+w > ORIGWIDTH)
	    break;
	V_DrawPatchDirect(cx, cy, hu_font[c]);
	cx+=w;
    }
}

// Write a string using the hu_font at 2x size (matches the big menu font
// height). Used by the Crispness menu so its text is the same size as the
// other option menus.
static void M_WriteTextBig(int x, int y, char *string)
{
    char *ch = string;
    int cx = x, cy = y, c, w;

    while (1)
    {
	c = *ch++;
	if (!c)
	    break;
	if (c == '\n')
	{
	    cx = x;
	    cy += 2 * SHORT(hu_font[0]->height);
	    continue;
	}

	c = toupper(c) - HU_FONTSTART;
	if (c < 0 || c >= HU_FONTSIZE)
	{
	    cx += 8;
	    continue;
	}

	w = SHORT(hu_font[c]->width);
	if (cx + 2*w > ORIGWIDTH)
	    break;
	V_DrawPatchBig(cx, cy, hu_font[c]);
	cx += 2*w;
    }
}

// These keys evaluate to a "null" key in Vanilla Doom that allows weird
// jumping in the menus. Preserve this behavior for accuracy.

static boolean IsNullKey(int key)
{
    return key == KEY_PAUSE || key == KEY_CAPSLOCK
        || key == KEY_SCRLCK || key == KEY_NUMLOCK;
}

//
// CONTROL PANEL
//

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             key;
    int             i;
    static  int     joywait = 0;
    static  int     mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;

    // In testcontrols mode, none of the function keys should do anything
    // - the only key is escape to quit.

    if (testcontrols)
    {
        if (ev->type == ev_quit
         || (ev->type == ev_keydown
          && (ev->data1 == key_menu_activate || ev->data1 == key_menu_quit)))
        {
            I_Quit();
            return true;
        }

        return false;
    }

    // "close" button pressed on window?
    if (ev->type == ev_quit)
    {
        // First click on close button = bring up quit confirm message.
        // Second click on close button = confirm quit

        if (menuactive && messageToPrint && messageRoutine == M_QuitResponse)
        {
            M_QuitResponse(key_menu_confirm);
        }
        else
        {
            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
        }

        return true;
    }

    // key is the key pressed, ch is the actual character typed
  
    ch = 0;
    key = -1;
	
    if (ev->type == ev_joystick && joywait < I_GetTime())
    {
	if (ev->data3 < 0)
	{
	    key = key_menu_up;
	    joywait = I_GetTime() + 5;
	}
	else if (ev->data3 > 0)
	{
	    key = key_menu_down;
	    joywait = I_GetTime() + 5;
	}
		
	if (ev->data2 < 0)
	{
	    key = key_menu_left;
	    joywait = I_GetTime() + 2;
	}
	else if (ev->data2 > 0)
	{
	    key = key_menu_right;
	    joywait = I_GetTime() + 2;
	}
		
	if (ev->data1&1)
	{
	    key = key_menu_forward;
	    joywait = I_GetTime() + 5;
	}
	if (ev->data1&2)
	{
	    key = key_menu_back;
	    joywait = I_GetTime() + 5;
	}
        if (joybmenu >= 0 && (ev->data1 & (1 << joybmenu)) != 0)
        {
            key = key_menu_activate;
	    joywait = I_GetTime() + 5;
        }
    }
    else
    {
	if (ev->type == ev_mouse && mousewait < I_GetTime())
	{
	    mousey += ev->data3;
	    if (mousey < lasty-30)
	    {
		key = key_menu_down;
		mousewait = I_GetTime() + 5;
		mousey = lasty -= 30;
	    }
	    else if (mousey > lasty+30)
	    {
		key = key_menu_up;
		mousewait = I_GetTime() + 5;
		mousey = lasty += 30;
	    }
		
	    mousex += ev->data2;
	    if (mousex < lastx-30)
	    {
		key = key_menu_left;
		mousewait = I_GetTime() + 5;
		mousex = lastx -= 30;
	    }
	    else if (mousex > lastx+30)
	    {
		key = key_menu_right;
		mousewait = I_GetTime() + 5;
		mousex = lastx += 30;
	    }
		
	    if (ev->data1&1)
	    {
		key = key_menu_forward;
		mousewait = I_GetTime() + 15;
	    }
			
	    if (ev->data1&2)
	    {
		key = key_menu_back;
		mousewait = I_GetTime() + 15;
	    }
	}
	else
	{
	    if (ev->type == ev_keydown)
	    {
		key = ev->data1;
		ch = ev->data2;
	    }
	}
    }
    
    if (key == -1)
	return false;

    // BIND KEYS: capture the next input as the new binding for the selected
    // action. A keyboard press (incl. arrows) sets the key; a mouse button sets
    // the mouse binding; ESC cancels. Everything else is swallowed so the menu
    // stays put until a choice is made.
    if (askforkey)
    {
	if (ev->type == ev_keydown)
	{
	    askforkey = false;
	    if (key != KEY_ESCAPE && bindkey_target != NULL)
	    {
		*bindkey_target = key;
		S_StartSound(NULL, sfx_pistol);
	    }
	    else
		S_StartSound(NULL, sfx_swtchx);
	}
	else if (ev->type == ev_mouse && ev->data1 != 0)
	{
	    // data1 is a bitmask of held buttons; bind the lowest one.
	    int b = 0;
	    while (b < 16 && !(ev->data1 & (1 << b)))
		b++;

	    askforkey = false;
	    if (bindkey_mouse != NULL && b < 16)
	    {
		*bindkey_mouse = b;
		S_StartSound(NULL, sfx_pistol);
	    }
	    else
		S_StartSound(NULL, sfx_swtchx);  // action takes no mouse button
	}
	return true;
    }

    // Save Game string input
    if (saveStringEnter)
    {
	switch(key)
	{
	  case KEY_BACKSPACE:
	    if (saveCharIndex > 0)
	    {
		saveCharIndex--;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;

          case KEY_ESCAPE:
            saveStringEnter = 0;
            M_StringCopy(savegamestrings[saveSlot], saveOldString,
                         SAVESTRINGSIZE);
            break;

	  case KEY_ENTER:
	    saveStringEnter = 0;
	    if (savegamestrings[saveSlot][0])
		M_DoSave(saveSlot);
	    break;

	  default:
            // This is complicated.
            // Vanilla has a bug where the shift key is ignored when entering
            // a savegame name. If vanilla_keyboard_mapping is on, we want
            // to emulate this bug by using 'data1'. But if it's turned off,
            // it implies the user doesn't care about Vanilla emulation: just
            // use the correct 'data2'.

            if (vanilla_keyboard_mapping)
            {
                ch = key;
            }

            ch = toupper(ch);

            if (ch != ' '
             && (ch - HU_FONTSTART < 0 || ch - HU_FONTSTART >= HU_FONTSIZE))
            {
                break;
            }

	    if (ch >= 32 && ch <= 127 &&
		saveCharIndex < SAVESTRINGSIZE-1 &&
		M_StringWidth(savegamestrings[saveSlot]) <
		(SAVESTRINGSIZE-2)*8)
	    {
		savegamestrings[saveSlot][saveCharIndex++] = ch;
		savegamestrings[saveSlot][saveCharIndex] = 0;
	    }
	    break;
	}
	return true;
    }
    
    // Take care of any messages that need input
    if (messageToPrint)
    {
	if (messageNeedsInput)
        {
            if (key != ' ' && key != KEY_ESCAPE
             && key != key_menu_confirm && key != key_menu_abort)
            {
                return false;
            }
	}

	menuactive = messageLastMenuActive;
	messageToPrint = 0;
	if (messageRoutine)
	    messageRoutine(key);

	menuactive = false;
	S_StartSound(NULL,sfx_swtchx);
	return true;
    }

    if ((devparm && key == key_menu_help) ||
        (key != 0 && key == key_menu_screenshot))
    {
	G_ScreenShot ();
	return true;
    }

    // F-Keys
    if (!menuactive)
    {
	if (key == key_menu_decscreen)      // Screen size down
        {
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(0);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
	}
        else if (key == key_menu_incscreen) // Screen size up
        {
	    if (automapactive || chat_on)
		return false;
	    M_SizeDisplay(1);
	    S_StartSound(NULL,sfx_stnmov);
	    return true;
	}
        else if (key == key_menu_help)     // Help key
        {
	    M_StartControlPanel ();

	    if ( gamemode == retail )
	      currentMenu = &ReadDef2;
	    else
	      currentMenu = &ReadDef1;

	    itemOn = 0;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
        else if (key == key_menu_save)     // Save
        {
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_SaveGame(0);
	    return true;
        }
        else if (key == key_menu_load)     // Load
        {
	    M_StartControlPanel();
	    S_StartSound(NULL,sfx_swtchn);
	    M_LoadGame(0);
	    return true;
        }
        else if (key == key_menu_volume)   // Sound Volume
        {
	    M_StartControlPanel ();
	    currentMenu = &SoundDef;
	    itemOn = sfx_vol;
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
        else if (key == key_menu_detail)   // Detail toggle
        {
	    M_ChangeDetail(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
        }
        else if (key == key_menu_qsave)    // Quicksave
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickSave();
	    return true;
        }
        else if (key == key_menu_endgame)  // End game
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_EndGame(0);
	    return true;
        }
        else if (key == key_menu_messages) // Toggle messages
        {
	    M_ChangeMessages(0);
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
        }
        else if (key == key_menu_qload)    // Quickload
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuickLoad();
	    return true;
        }
        else if (key == key_menu_quit)     // Quit DOOM
        {
	    S_StartSound(NULL,sfx_swtchn);
	    M_QuitDOOM(0);
	    return true;
        }
        else if (key == key_menu_gamma)    // gamma toggle
        {
	    usegamma++;
	    if (usegamma > 4)
		usegamma = 0;
	    players[consoleplayer].message = DEH_String(gammamsg[usegamma]);
            I_SetPalette (W_CacheLumpName (DEH_String("PLAYPAL"),PU_CACHE));
	    return true;
	}
    }

    // Pop-up menu?
    if (!menuactive)
    {
	if (key == key_menu_activate)
	{
	    M_StartControlPanel ();
	    S_StartSound(NULL,sfx_swtchn);
	    return true;
	}
	return false;
    }

    // Keys usable within menu

    if (key == key_menu_down)
    {
        // Move down to next item

        do
	{
	    if (itemOn+1 > currentMenu->numitems-1)
		itemOn = 0;
	    else itemOn++;
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);

	return true;
    }
    else if (key == key_menu_up)
    {
        // Move back up to previous item

	do
	{
	    if (!itemOn)
		itemOn = currentMenu->numitems-1;
	    else itemOn--;
	    S_StartSound(NULL,sfx_pstop);
	} while(currentMenu->menuitems[itemOn].status==-1);

	return true;
    }
    else if (key == key_menu_left)
    {
        // Slide slider left

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
	    currentMenu->menuitems[itemOn].routine(0);
	}
	return true;
    }
    else if (key == key_menu_right)
    {
        // Slide slider right

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status == 2)
	{
	    S_StartSound(NULL,sfx_stnmov);
	    currentMenu->menuitems[itemOn].routine(1);
	}
	return true;
    }
    else if (key == key_menu_forward)
    {
        // Activate menu item

	if (currentMenu->menuitems[itemOn].routine &&
	    currentMenu->menuitems[itemOn].status)
	{
	    currentMenu->lastOn = itemOn;
	    if (currentMenu->menuitems[itemOn].status == 2)
	    {
		currentMenu->menuitems[itemOn].routine(1);      // right arrow
		S_StartSound(NULL,sfx_stnmov);
	    }
	    else
	    {
		currentMenu->menuitems[itemOn].routine(itemOn);
		S_StartSound(NULL,sfx_pistol);
	    }
	}
	return true;
    }
    else if (key == key_menu_activate)
    {
        // Deactivate menu

	currentMenu->lastOn = itemOn;
	M_ClearMenus ();
	S_StartSound(NULL,sfx_swtchx);
	return true;
    }
    else if (key == key_menu_back)
    {
        // Go back to previous menu

	currentMenu->lastOn = itemOn;
	if (currentMenu->prevMenu)
	{
	    currentMenu = currentMenu->prevMenu;
	    itemOn = currentMenu->lastOn;
	    S_StartSound(NULL,sfx_swtchn);
	}
	return true;
    }

    // Keyboard shortcut?
    // Vanilla Doom has a weird behavior where it jumps to the scroll bars
    // when the certain keys are pressed, so emulate this.

    else if (ch != 0 || IsNullKey(key))
    {
	for (i = itemOn+1;i < currentMenu->numitems;i++)
        {
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
        }

	for (i = 0;i <= itemOn;i++)
        {
	    if (currentMenu->menuitems[i].alphaKey == ch)
	    {
		itemOn = i;
		S_StartSound(NULL,sfx_pstop);
		return true;
	    }
        }
    }

    return false;
}



//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
	return;
    
    menuactive = 1;
    currentMenu = &MainDef;         // JDC
    itemOn = currentMenu->lastOn;   // JDC
}

// Display OPL debug messages - hack for GENMIDI development.

#if 0
static void M_DrawOPLDev(void)
{
    extern void I_OPL_DevMessages(char *, size_t);
    char debug[1024];
    char *curr, *p;
    int line;

    //XXX I_OPL_DevMessages(debug, sizeof(debug));
    curr = debug;
    line = 0;

    for (;;)
    {
        p = strchr(curr, '\n');

        if (p != NULL)
        {
            *p = '\0';
        }

        M_WriteText(0, line * 8, curr);
        ++line;

        if (p == NULL)
        {
            break;
        }

        curr = p + 1;
    }
}
#endif

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    static short	x;
    static short	y;
    unsigned int	i;
    unsigned int	max;
    char		string[80];
    char               *name;
    int			start;

    inhelpscreens = false;
    
    // Horiz. & Vertically center string and print it.
    if (messageToPrint)
    {
	start = 0;
	y = ORIGHEIGHT/2 - M_StringHeight(messageString) / 2;
	while (messageString[start] != '\0')
	{
	    int foundnewline = 0;

            for (i = 0; i < strlen(messageString + start); i++)
            {
                if (messageString[start + i] == '\n')
                {
                    M_StringCopy(string, messageString + start,
                                 sizeof(string));
                    if (i < sizeof(string))
                    {
                        string[i] = '\0';
                    }

                    foundnewline = 1;
                    start += i + 1;
                    break;
                }
            }

            if (!foundnewline)
            {
                M_StringCopy(string, messageString + start, sizeof(string));
                start += strlen(string);
            }

	    x = ORIGWIDTH/2 - M_StringWidth(string) / 2;
	    M_WriteText(x, y, string);
	    y += SHORT(hu_font[0]->height);
	}

	return;
    }

    //if (opldev)
    //{
    //    M_DrawOPLDev();
    //}

    if (!menuactive)
	return;

    if (currentMenu->routine)
	currentMenu->routine();         // call Draw routine
    
    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;

    for (i=0;i<max;i++)
    {
        name = DEH_String(currentMenu->menuitems[i].name);

	if (name[0])
	{
	    V_DrawPatchDirect (x, y, W_CacheLumpName(name, PU_CACHE));
	}
	y += LINEHEIGHT;
    }

    
    // DRAW SKULL
    V_DrawPatchDirect(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,
		      W_CacheLumpName(DEH_String(skullName[whichSkull]),
				      PU_CACHE));
}


//
// M_ClearMenus
//
void M_ClearMenus (void)
{
    menuactive = 0;
    // if (!netgame && usergame && paused)
    //       sendpause = true;
}




//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;
}


//
// M_Ticker
//
void M_Ticker (void)
{
    if (--skullAnimCounter <= 0)
    {
	whichSkull ^= 1;
	skullAnimCounter = 8;
    }
}


//
// M_Init
//
void M_Init (void)
{
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    // Here we could catch other version dependencies,
    //  like HELP1/2, and four episodes.

  
    switch ( gamemode )
    {
      case commercial:
        // Commercial has no "read this" entry.
	MainMenu[readthis] = MainMenu[quitdoom];
	MainDef.numitems--;
	MainDef.y += 8;
	NewDef.prevMenu = &MainDef;
	break;
      case shareware:
	// Episode 2 and 3 are handled,
	//  branching to an ad screen.
      case registered:
	break;
      case retail:
	// We are fine.
      default:
	break;
    }

    // Versions of doom.exe before the Ultimate Doom release only had
    // three episodes; if we're emulating one of those then don't try
    // to show episode four. If we are, then do show episode four
    // (should crash if missing).
    if (gameversion < exe_ultimate)
    {
	EpiDef.numitems--;
    }

    //opldev = M_CheckParm("-opldev") > 0;
}

