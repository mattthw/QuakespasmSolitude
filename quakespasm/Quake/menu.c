/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2010-2014 QuakeSpasm developers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "quakedef.h"
#include "bgmusic.h"
#ifndef _WIN32
#include <dirent.h>	// perf B7: Custom Maps tab folder scan (same precedent as host_cmd.c)
#endif

qpic_t *menu_bg;

extern cvar_t scr_fov;

// perf B7 - cvars surfaced on the new Performance menu page
extern cvar_t	r_dlight_cull, r_max_dlights, r_part_maxrad, r_subpixel_cull, r_light_cache, r_lerpskip_dist;
extern cvar_t	r_perfhud, sv_max_needles, sv_bot_thinkrate, sv_particle_scale, sv_max_gibs;
extern cvar_t	r_particles, r_dynamic, gl_flashblend, r_shadows, r_lerpmodels;
// unified Options menu (perf B7): the cvars its tabs bind to
extern cvar_t	crosshair, gl_texturemode, vid_vsync, r_viewmodeloffset;
extern cvar_t	host_maxfps;	// Performance tab "Max FPS" (host.c)
extern cvar_t	cl_cambob, cl_gunbob;	// Graphics tab bob sliders (defined view.c)
extern cvar_t	pstv_rumble, retrotouch, motioncam, motion_horizontal_sensitivity, motion_vertical_sensitivity;
// r101 - Editor tab: map debug cvars (defined gl_rmain.c) + live cheat state (host_cmd.c)
extern cvar_t	r_showtris, r_speeds, r_novis, r_lightmap, r_fullbright, r_drawflat;
extern int	Host_CheatState (int which);
extern cvar_t	sv_noclip_speed;	//r101 noclip fly speed (sv_user.c)

enum
{
	UNI_TAB_EDIT, UNI_TAB_PERF, UNI_TAB_GFX, UNI_TAB_CTRL, UNI_TAB_SND, UNI_TAB_GEN, UNI_TABS
};

static const char	*uni_tab_names[UNI_TABS] = {"Editor", "Performance", "Graphics", "Controls", "Sound", "General"}; // r101: Editor first, but only visible in editor mode
static int	 uni_tab, uni_cursor, uni_first;
static enum m_state_e uni_return_state;

static void M_Performance_Body (struct MenuCoords *coords, int rowh, size_t shown);
static void M_Performance_ResetWindow (void);
static void M_Options_CycleTab (int dir);
static void M_Options_SetTab (int tab);		// r101: tab + remembered row
static void M_Options_OpenTab (void);		// r101: first-open heuristic
static qboolean UNI_EditorVisible (void);	// r101
static qboolean UNI_TabVisible (int tab);	// r101
static void M_Draw_Scrollbar (struct MenuCoords *coords, int rowh, int total, size_t first, int shown);
static int M_HintIconW (qpic_t *icon);	// perf B7: reuse matchmaking hint icons in the tab strip
static void M_HintIcon (int *x, int y, qpic_t *icon);
cvar_t	perf_preset = {"perf_preset", "2", CVAR_ARCHIVE};	// perf B7 - 1/2/3 = last preset applied, 0 = custom; ships Balanced
cvar_t	perf_custom_backup = {"perf_custom_backup", "", CVAR_ARCHIVE};	// perf B9b: the 12 preset-controlled knob values x100, space separated -- "Custom remembers your mix"

void (*vid_menucmdfn)(void); //johnfitz
void (*vid_menudrawfn)(void);
void (*vid_menukeyfn)(int key);

enum m_state_e m_state;

bool mm_rentry = false;

void M_Menu_Main_f (void);
	void M_Menu_SinglePlayer_f (void);
		void M_Menu_Load_f (void);
		void M_Menu_Save_f (void);
    void M_Matchmaking_f (void);
        void M_Matchmaking_Gamerules_f (void);
        void M_Matchmaking_Map_f (void);
		void M_Menu_Setup_f (void);
		void M_Menu_LanConfig_f (void);
		void M_Menu_Search_f (enum slistScope_e scope);
		void M_Menu_ServerList_f (void);
    void M_Menu_Firefight_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_Keys_f (void);
		void M_Menu_Video_f (void);
		void M_Menu_Performance_f (void);	// perf B7
		void M_Menu_Editor_f (void);	// r101
	void M_Menu_Help_f (void);
	void M_Menu_Quit_f (void);
    void M_Menu_Pause_f (void);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
    void M_Main_Multi_Draw (void);
    void M_Matchmaking_Draw (void);
void M_Lobbies_f (void);
void M_Lobbies_Draw (void);
void M_Lobbies_Key (int key);
        void M_Matchmaking_Gamerules_Draw (void);
        void M_Matchmaking_Map_Draw (void);
		void M_Setup_Draw (void);
		void M_LanConfig_Draw (void);
		void M_Search_Draw (void);
		void M_ServerList_Draw (void);
    void M_Firefight_Draw (void);
	void M_Options_Draw (void);
		void M_Keys_Draw (void);
		void M_Video_Draw (void);
	void M_Help_Draw (void);
	void M_Quit_Draw (void);
    void M_Pause_Draw (void);

#ifdef VITA
	void M_Mods_Key (int key);
	void M_Mods_Draw (void);
	void M_Menu_Mods_f (void);

	void M_Maps_Key (int key);
	void M_Maps_Draw (void);
	void M_Menu_Maps_f (void);
#endif

void M_Main_Key (int key);
	void M_SinglePlayer_Key (int key);
		void M_Load_Key (int key);
		void M_Save_Key (int key);
    void M_Matchmaking_Key (int key);
        void M_Matchmaking_Gamerules_Key (int key);
        void M_Matchmaking_Map_Key (int key);
		void M_Setup_Key (int key);
		void M_LanConfig_Key (int key);
		void M_Search_Key (int key);
		void M_ServerList_Key (int key);
    void M_Firefight_Key (int key);
	void M_Options_Key (int key);
		void M_Keys_Key (int key);
		void M_Video_Key (int key);
	void M_Help_Key (int key);
	void M_Quit_Key (int key);
    void M_Pause_Key (int key);

qboolean	m_entersound;		// play after drawing a frame, so caching
								// won't disrupt the sound
qboolean	m_recursiveDraw;

enum m_state_e	m_return_state;
/* r164 (user: "exiting custom games and matchmaking should go back to their
 * respective menu, not always custom"): the mode a match was launched from.
 * Set at every start/join entry point, consumed by the disconnect paths so
 * Find Match games land back in the lobby and Custom Games in Custom Games. */
enum m_state_e	match_return_state = m_gameoptions;
qboolean	m_return_onerror;
char		m_return_reason [32];
int m_multiplayer_cursor = 1;
#define StartingGame	(m_multiplayer_cursor == 1)
#define JoiningGame		(m_multiplayer_cursor == 0)
#define	IPXConfig		false //(m_net_cursor == 0)
#define	TCPIPConfig		(m_net_cursor == 1)


/*
===============
scroll the string inside a glscissor region
===============
*/
void M_DrawScrollString (int x, int y, int width, const char *str)
{
    int len, ofs, left;

    glEnable (GL_SCISSOR_TEST);
    glScissor (0, 0, glwidth*scr_menuscale.value, glheight);

    len = strlen(str)*8 + 40;
    ofs = ((int)(realtime*30))%len;
    Sbar_DrawString (x - ofs, y, str);
    Sbar_DrawCharacter (x - ofs + len - 32, y, '/');
    Sbar_DrawCharacter (x - ofs + len - 24, y, '/');
    Sbar_DrawCharacter (x - ofs + len - 16, y, '/');
    Sbar_DrawString (x - ofs + len, y, str);

    glDisable (GL_SCISSOR_TEST);
}

void M_ConfigureNetSubsystem(void);

/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character (cx, line, num);
}

void M_DrawCharacterColored (int cx, int line, int num)
{
    Draw_Character (cx, line, num+128);
}

/* P2a: classic Quake palette row 0 (48 bytes, measured from retail
 * gfx/palette.lmp in id1 pak0 — this fork has NO runtime palette array).
 * ^N menu tokens map here; 11/12 are the greys M_Print/M_PrintWhite use. */
/* TTF path has NO palette limitation (white texture x glColor4f — any blend).
 * These 16 are curated Halo-flavored UI colors for ^0..^9 menu tokens, not the
 * quake row-0 ramp (that read "off"); ^10..^15 round out a useful set. */
static const unsigned char	menu_palette[48] =
{ 20,21,23,   /* 0  near-black      */
  235,240,242,/* 1  warm white      */
  208,74,60,  /* 2  alert red        */
  124,196,110,/* 3  friendly green   */
  88,160,220, /* 4  team blue        */
  230,170,70, /* 5  amber            */
  108,210,240,/* 6  halo teal        */
  160,120,210,/* 7  purple           */
  120,122,126,/* 8  mid grey         */
  60,64,68,   /* 9  dark grey        */
  170,172,176,/* 10 slate grey       */
  203,219,219,/* 11 light grey       */
  235,235,235,/* 12 white            */
  235,15,11,  /* 13 hot red          */
  7,23,15,    /* 14 dark green       */
  11,31,23 }; /* 15 deep             */

void M_Print (int cx, int cy, const char *str)
{
	/* P2a: menu font — Archivo TTF, with the classic '^N' palette-number
	 * tokens honored (the old main menu is literally "3NEW GAME" etc —
	 * dropping them made everything white, user 2026-11-12).
	 * Menus draw in the stretched canvas space (8-unit cells); size 14
	 * matches the old glyphs' visual weight there. y is the TTF baseline;
	 * old Draw_Character y was the cell top, so +8 lines the text where
	 * conchars sat. */
	const char	*p = str;
	char		 tmpl[256];
	int			ti = 0, col = 11;  /* P2a: palette row0 grey 11 (menu grey) */

	while (*p && ti < (int)sizeof(tmpl) - 3)
	{
		if (p[0] == '^' && p[1] >= '0' && p[1] <= '9')
		{
			col = p[1] - '0';
			p += 2;
			continue;
		}
		if (p[0] == '%' && p[1] && strchr("cdiogsxXEGF", p[1]))
		{
			tmpl[ti++] = '%';   /* forward fmt token to no-op %s later */
			p += 2;
			continue;
		}
		if (p[0] == '%' && p[1] == '%')
		{
			tmpl[ti++] = '%';
			p += 2;
			continue;
		}
		tmpl[ti++] = *p++;
	}
	tmpl[ti] = '\0';

	Draw_TTFString (cx, cy + 8, tmpl,
		(menu_palette[col*3] + 0.1f) / 255.0f,
		(menu_palette[col*3+1] + 0.1f) / 255.0f,
		(menu_palette[col*3+2] + 0.1f) / 255.0f, 1.0f, 14.0f);
}

void M_PrintWhite (int cx, int cy, const char *str)
{
	/* P2a: menu font — see M_Print. White default variant (palette 15
	 * unless a '^N' overrides), matching the old per-char white loop. */
	const char	*p = str;
	char		 tmpl[256];
	int			ti = 0, col = 12;

	while (*p && ti < (int)sizeof(tmpl) - 3)
	{
		if (p[0] == '^' && p[1] >= '0' && p[1] <= '9')
		{
			col = p[1] - '0';
			p += 2;
			continue;
		}
		if (p[0] == '%' && p[1] && strchr("cdiogsxXEGF", p[1]))
		{
			tmpl[ti++] = '%';
			p += 2;
			continue;
		}
		if (p[0] == '%' && p[1] == '%')
		{
			tmpl[ti++] = '%';
			p += 2;
			continue;
		}
		tmpl[ti++] = *p++;
	}
	tmpl[ti] = '\0';

	/* r112: main menu rows read small vs their bands (user: slight bump,
	 * stay inside the row). 16 vs row height MVS≈19; other menus keep 14. */
	Draw_TTFString (cx, cy + 8, tmpl,
		(menu_palette[col*3] + 0.1f) / 255.0f,
		(menu_palette[col*3+1] + 0.1f) / 255.0f,
		(menu_palette[col*3+2] + 0.1f) / 255.0f, 1.0f,
		m_state == m_main ? 16.0f : 14.0f);
}

void M_PrintCentered (int cy, char *str)
{
	int cx = (320*MENU_SCALE)/2 - strlen(str) * 4;
	
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_DrawTransPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y, pic); //johnfitz -- simplified becuase centering is handled elsewhere
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y, pic); //johnfitz -- simplified becuase centering is handled elsewhere
}

void M_DrawTransPicTranslate (int x, int y, qpic_t *pic, int top, int bottom) //johnfitz -- more parameters
{
	Draw_TransPicTranslate (x, y, pic, top, bottom); //johnfitz -- simplified becuase centering is handled elsewhere
}

void M_DrawTextBox (int x, int y, int width, int lines)
{
	/* P2a: same Halo navy style as the menu panels — the classic box_*
	 * WAD-art tiles read grey against the #171d3c windows. */
	int w = (width + 2) * 8;
	int h = (lines + 2) * 8;
	Draw_FillRGBA (x - 1, y - 1, w + 2, h + 2, 20/255.0f, 25/255.0f, 44/255.0f, 0.85f);
	Draw_FillGradient (x, y, w, h, 23/255.0f, 29/255.0f, 60/255.0f, 0.75f);
}

//=============================================================================

int m_save_demonum;

int mvs(double cursorpos)
{
    return cursorpos*MVS;
}

//=============================================================================
/* MAIN MENU */

int	m_main_cursor;
#define	MAIN_ITEMS	5
#define MAIN_MULTI_ITEMS 2

int	m_main_cursor;
int m_main_multi_cursor;
bool main_multi = false;
float main_percentwidth = 0.3;
float main_x_offset_percent = 0.1;

#define CURSOR_HEIGHT MVS

//main
#define main_pixel_height (MAIN_ITEMS + 1) * MVS
#define main_pixel_width PixWidth(main_percentwidth)
#define main_y_offset_pixels PixHeight(1) - main_pixel_height
#define main_x_offset_pixels PixWidth(main_x_offset_percent)

//main-multi submenu
#define multi_pixel_height (MAIN_MULTI_ITEMS) * MVS
#define multi_pixel_width 17*CHARZ

#define multi_x_offset_pixels main_x_offset_pixels+main_pixel_width

bool theme_started = false;
void PlayMenuTheme(bool override) {
    //    Cbuf_AddText ("playdemo title.dem\n"); broken since there is a missing audio track. causes loop. do in config if wanted
    /* r178: theme_started alone was not enough. It is only cleared on the
     * pause-menu disconnect (menu.c:1175), while every other way out of a
     * match -- joining from the lobby, "disconnect" in the console, a map
     * change -- stops the track through CL_Disconnect/BGM_Stop and leaves the
     * flag set, so the theme never came back. Ask the mixer instead: if no
     * stream is open, the music is not actually running and must restart. */
    if (!theme_started || override || !BGM_IsPlaying()) {
        /* r106: menu theme moved from the one-shot "play music/theme.wav"
         * sample to the bgmusic streamer - music/<file>.mp3 decoded by
         * snd_mp3 (libmad on Vita) and rewound at EOF (bgmloop default 1),
         * so the ambient track loops under the Music Volume slider. The old
         * theme.wav stays in the pak untouched. CL_Disconnect/BGM_Stop kills
         * it on map start; BGM_IsPlaying() above notices and restarts it the
         * next time a menu that owns the theme is entered. */
        Cbuf_AddText("music solitude_ambient_1_justin_medford.mp3\n");
        theme_started = true;
    }
}

void M_Menu_Main_f (void)
{
	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;
    main_multi = false;
    m_main_cursor = 1;      /* r139 (user): open on Matchmaking */
    PlayMenuTheme(false);

	IN_UpdateGrabs();
}

void M_Main_Draw (void)
{
    // Menu Background
    Draw_MenuBg();
    Draw_CachePic ("gfx/loading.tga"); //don't need yet, just want to cache
    //background of menu
    Draw_FillGradient(main_x_offset_pixels, main_y_offset_pixels, main_pixel_width, main_pixel_height, 23/255.0f, 29/255.0f, 60/255.0f, 0.75f); //P2a Halo #171d3c + edge gradient
    Draw_FillRGBA(main_x_offset_pixels, PixHeight(1)-MVS, main_pixel_width, MVS, 23/255.0f, 29/255.0f, 60/255.0f, 0.9f); //P2a #171d3c solid-ish footer
    /* hint lives on the footer band, right-aligned WITHIN the main menu
     * panel's bounds + padding (user): measure icon+text, subtract from the
     * panel's right edge. M_HintIcon keeps the art native-aspect. */
    {  int hintw = 16 + 2 + (int)Draw_TTFWidth ("Select", 16.0f); // r112: funnel draws main at 16
    int hintx = main_x_offset_pixels + main_pixel_width - 8 - hintw;
    int tx = hintx;
    M_HintIcon (&tx, PixHeight(1)-MVS+TEXT_YMARGIN, b_abutton);
    tx += 2;
    M_PrintWhite(tx, PixHeight(1)-MVS+TEXT_YMARGIN, "Select"); //tip
    }
    //cursor
    Draw_Cursor(main_x_offset_pixels, main_y_offset_pixels+mvs(m_main_cursor), main_pixel_width, CURSOR_HEIGHT, true);
    //menu items
    /* r138 (user): flat five-row menu; Matchmaking is the online lobby browser,
     * Custom Games is the local hosting screen, Spartan editing moved into the
     * Options tabs, so the old main_multi fold-out is gone. */
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(0), "Firefight");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(1), "Matchmaking");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(2), "Custom Games");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(3), "Options");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(4), "Quit");

    /* r94: welcome marquee disabled per user */

}

void M_Main_Multi_Draw (void)
{
    //background
    Draw_FillGradient(multi_x_offset_pixels, main_y_offset_pixels, multi_pixel_width, multi_pixel_height, 23/255.0f, 29/255.0f, 60/255.0f, 0.75f); //P2a Halo #171d3c + edge gradient
    //cursor in focus
    Draw_Cursor(multi_x_offset_pixels, main_y_offset_pixels+mvs(m_main_multi_cursor), multi_pixel_width, CURSOR_HEIGHT, true);
    //menu items
    M_PrintWhite(multi_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(0), "Custom Games");
    M_PrintWhite(multi_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(1), "Find Match");
}

void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
    case K_YBUTTON:
	case K_BBUTTON:
        {
            m_main_cursor = 4;
            M_Menu_Quit_f ();
        }
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
            if (++m_main_cursor >= MAIN_ITEMS)
                m_main_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
            if (--m_main_cursor < 0)
                m_main_cursor = MAIN_ITEMS - 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
        m_entersound = true;

        switch (m_main_cursor)
        {
            case 0:
                M_Menu_Firefight_f ();
                break;

            case 1:
                M_Lobbies_f ();          /* r137/r138: online lobbies */
                break;

            case 2:
                M_Matchmaking_f ();      /* r138: local custom game */
                break;

            case 3:
                M_Menu_Options_f ();     /* r138: Spartan tab lives in here */
                break;

            case 4:
                M_Menu_Quit_f ();
                break;
        }
	}
}

//=============================================================================
/* SINGLE PLAYER MENU */

int	m_singleplayer_cursor;
#define	SINGLEPLAYER_ITEMS	3


void M_Menu_SinglePlayer_f (void)
{
	key_dest = key_menu;
	m_state = m_singleplayer;
	m_entersound = true;

	IN_UpdateGrabs();
}


void M_SinglePlayer_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawTransPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_sgl.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawTransPic (72, 32, Draw_CachePic ("gfx/sp_menu.lmp") );

	f = (int)(realtime * 10)%6;

	M_DrawTransPic (54, 32 + m_singleplayer_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_SinglePlayer_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Main_f ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (++m_singleplayer_cursor >= SINGLEPLAYER_ITEMS)
			m_singleplayer_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (--m_singleplayer_cursor < 0)
			m_singleplayer_cursor = SINGLEPLAYER_ITEMS - 1;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;

		switch (m_singleplayer_cursor)
		{
		case 0:
			if (sv.active)
				if (!SCR_ModalMessage("Are you sure you want to\nstart a new game?\n", 0.0f))
					break;
			key_dest = key_game;
			IN_UpdateGrabs();
			if (sv.active)
				Cbuf_AddText ("disconnect\n");
			Cbuf_AddText ("maxplayers 1\n");
			Cbuf_AddText ("samelevel 0\n"); //spike -- you'd be amazed how many qw players have this setting breaking their singleplayer experience...
			Cbuf_AddText ("deathmatch 0\n"); //johnfitz
			Cbuf_AddText ("coop 0\n"); //johnfitz
			Cbuf_AddText ("startmap_sp\n");
			break;

		case 1:
			M_Menu_Load_f ();
			break;

		case 2:
			M_Menu_Save_f ();
			break;
		}
	}
}

//=============================================================================
/* LOAD/SAVE MENU */

int		load_cursor;		// 0 < load_cursor < MAX_SAVEGAMES

#define	MAX_SAVEGAMES		20	/* johnfitz -- increased from 12 */
char	m_filenames[MAX_SAVEGAMES][SAVEGAME_COMMENT_LENGTH+1];
int		loadable[MAX_SAVEGAMES];

void M_ScanSaves (void)
{
	int	i, j;
	char	name[MAX_OSPATH];
	FILE	*f;
	int	version;

	for (i = 0; i < MAX_SAVEGAMES; i++)
	{
		strcpy (m_filenames[i], "--- UNUSED SLOT ---");
		loadable[i] = false;
		q_snprintf (name, sizeof(name), "%s/s%i.sav", com_gamedir, i);
		f = fopen (name, "r");
		if (!f)
			continue;
		fscanf (f, "%i\n", &version);
		fscanf (f, "%79s\n", name);
		q_strlcpy (m_filenames[i], name, SAVEGAME_COMMENT_LENGTH+1);

	// change _ back to space
		for (j = 0; j < SAVEGAME_COMMENT_LENGTH; j++)
		{
			if (m_filenames[i][j] == '_')
				m_filenames[i][j] = ' ';
		}
		loadable[i] = true;
		fclose (f);
	}
}

void M_Menu_Load_f (void)
{
	m_entersound = true;
	m_state = m_load;

	key_dest = key_menu;
	M_ScanSaves ();

	IN_UpdateGrabs();
}


void M_Menu_Save_f (void)
{
	if (!sv.active)
		return;
	if (cl.intermission)
		return;
	if (svs.maxclients != 1)
		return;
	m_entersound = true;
	m_state = m_save;

	key_dest = key_menu;
	IN_UpdateGrabs();
	M_ScanSaves ();
}


void M_Load_Draw (void)
{
	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_load.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i = 0; i < MAX_SAVEGAMES; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Save_Draw (void)
{
	int		i;
	qpic_t	*p;

	p = Draw_CachePic ("gfx/p_save.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	for (i = 0; i < MAX_SAVEGAMES; i++)
		M_Print (16, 32 + 8*i, m_filenames[i]);

// line cursor
	M_DrawCharacter (8, 32 + load_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Load_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_SinglePlayer_f ();
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		S_LocalSound ("misc/menu2.wav");
		if (!loadable[load_cursor])
			return;
		m_state = m_none;
		key_dest = key_game;
		IN_UpdateGrabs();

	// Host_Loadgame_f can't bring up the loading plaque because too much
	// stack space has been used, so do it now
		SCR_BeginLoadingPlaque ();

	// issue the load command
		Cbuf_AddText (va ("load s%i\n", load_cursor) );
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}


void M_Save_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_SinglePlayer_f ();
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_state = m_none;
		key_dest = key_game;
		IN_UpdateGrabs();
		Cbuf_AddText (va("save s%i\n", load_cursor));
		return;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor--;
		if (load_cursor < 0)
			load_cursor = MAX_SAVEGAMES-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		load_cursor++;
		if (load_cursor >= MAX_SAVEGAMES)
			load_cursor = 0;
		break;
	}
}

//=============================================================================
/* MULTIPLAYER MENU */

int	m_multiplayer_cursor;

//=============================================================================
/* SETUP MENU */

int		setup_cursor = 2;
int		setup_cursor_table[] = {40, 56, 80, 104, 140};

/* r96: TDM pins teams red/blue; remember the player's profile colors the
 * first time the lobby opens so FFA matches can restore them.
 * r113: hoisted above the setup page so M_Setup_Key can keep the capture
 * in sync with deliberate profile edits (the old capture went stale and
 * the next lobby open silently stomped a freshly-chosen color back to the
 * value captured on first entry -- the "I set green, roster shows red"
 * bug). */
static qboolean	lobby_profile_saved = false;
static char		lobby_saved_bottom[8] = "";
static char		lobby_saved_top[8] = "";

/* r114: Team Slayer pins bottomcolor/topcolor to 4 for the match (QC team
 * ids and model skins read those cvars). The pin used to leak permanently:
 * quitting straight from a TDM match wrote 4 into config.cfg before any
 * lobby open could restore the profile. This helper re-applies the captured
 * Spartan-page profile synchronously; Host_WriteConfiguration calls it as
 * the single config-persist chokepoint, and M_Matchmaking_f uses it for the
 * live repair (Cvar_Set takes effect this frame, so the roster preview
 * cannot flash the stale pin). */
void M_Lobby_RestoreProfile (void)
{
	if (!lobby_profile_saved || !lobby_saved_bottom[0])
		return;
	if (strcmp (Cvar_VariableString ("bottomcolor"), lobby_saved_bottom)
	    || strcmp (Cvar_VariableString ("topcolor"), lobby_saved_top))
	{
		Cvar_Set ("bottomcolor", lobby_saved_bottom);
		Cvar_Set ("topcolor", lobby_saved_top);
	}
}

char	setup_hostname[16];
char	setup_myname[16];
int		setup_oldtop;
int		setup_oldbottom;
int		setup_top;
int		setup_bottom;

#define	NUM_SETUP_CMDS	4

void M_Menu_Setup_f (void)
{
	key_dest = key_menu;
	m_state = m_setup;
	m_entersound = true;
	Q_strcpy(setup_myname, cl_name.string);
	Q_strcpy(setup_hostname, hostname.string);
	setup_top = setup_oldtop = ((int)cl_topcolor.value) >> 4;
	setup_bottom = setup_oldbottom = ((int)cl_bottomcolor.value) & 15;

	IN_UpdateGrabs();
}

void M_Setup_Draw (void)
{
    qpic_t	*p;
    Draw_MenuBg();
    // window
    struct MenuCoords mc = Draw_WindowGridOffset("Spartan Customization", 4, MVS, 2, 0.267, 0.8, setup_cursor, true, -0.19);
    //footer
    switch (setup_cursor) {
        case 0:
        case 1:
            M_Draw_Hint(INPUT, mc);
            break;
        case 2:
            M_Draw_Hint(SCROLL, mc);
            break;
        default:
            M_Draw_Hint(SELECT, mc);
    }

    M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][0].yp, "Hostname");
	M_DrawTextBox (mc.grid[1][0].x, mc.grid[1][0].y, 16, 1);
    M_PrintWhite (mc.grid[1][0].xp, mc.grid[1][0].yp, setup_hostname);

    M_PrintWhite (mc.grid[0][1].xp, mc.grid[0][1].yp, "Your name");
	M_DrawTextBox (mc.grid[1][1].x, mc.grid[1][1].y, 16, 1);
    M_PrintWhite (mc.grid[1][1].xp, mc.grid[1][1].yp, setup_myname);

    M_PrintWhite (mc.grid[0][2].xp, mc.grid[0][2].yp, "Spartan color");

    M_PrintWhite (mc.grid[0][3].xp, mc.grid[0][3].yp, "Save Changes");

	p = Draw_CachePic ("gfx/menuspartan.lmp");
    int spartanx = ((320 * MENU_SCALE) - p->width)/2 + (0.2667 * (320 * MENU_SCALE));
    int spartany = ((200 * MENU_SCALE) - p->height)/2;
	M_DrawTransPicTranslate (spartanx, spartany, p, setup_top, setup_bottom);

	if (setup_cursor == 0)
		M_DrawCharacter (mc.grid[1][0].xp + 8*strlen(setup_hostname), mc.grid[1][0].yp, 10+((int)(realtime*4)&1));

	if (setup_cursor == 1)
		M_DrawCharacter (mc.grid[1][1].xp + 8*strlen(setup_myname), mc.grid[1][1].yp, 10+((int)(realtime*4)&1));
}


void M_Setup_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Main_f();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		setup_cursor--;
		if (setup_cursor < 0)
			setup_cursor = NUM_SETUP_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		setup_cursor++;
		if (setup_cursor >= NUM_SETUP_CMDS)
			setup_cursor = 0;
		break;
		
#ifdef VITA
	case K_YBUTTON:
		if (setup_cursor == 0)
			IN_SwitchKeyboard(setup_hostname, 16);
		else if (setup_cursor == 1)
			IN_SwitchKeyboard(setup_myname, 16);
		break;
#endif

	case K_LEFTARROW:
		if (setup_cursor < 2)
			return;
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 2)
            setup_bottom = setup_bottom - 1;
            setup_top = setup_bottom; // spartan has one color
		break;
	case K_RIGHTARROW:
		if (setup_cursor < 2)
			return;
forward:
		S_LocalSound ("misc/menu3.wav");
		if (setup_cursor == 2)
            setup_bottom = setup_bottom + 1;
            setup_top = setup_bottom; // spartan has one color
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
        if (setup_cursor == 0)
            IN_SwitchKeyboard(setup_hostname, 16);
        else if (setup_cursor == 1)
            IN_SwitchKeyboard(setup_myname, 16);

		if (setup_cursor == 2)
			goto forward;

		// setup_cursor == 4 (OK)
		if (Q_strcmp(cl_name.string, setup_myname) != 0)
			Cbuf_AddText ( va ("name \"%s\"\n", setup_myname) );
		if (Q_strcmp(hostname.string, setup_hostname) != 0)
			Cvar_Set("hostname", setup_hostname);
		if (setup_top != setup_oldtop || setup_bottom != setup_oldbottom)
			Cbuf_AddText( va ("color %i %i\n", setup_top, setup_bottom) );
		/* r113: the lobby profile memory follows deliberate edits (and is
		 * armed even if the lobby has not been opened yet this session). */
		lobby_profile_saved = true;
		q_strlcpy (lobby_saved_top, va("%i", setup_top), sizeof (lobby_saved_top));
		q_strlcpy (lobby_saved_bottom, va("%i", setup_bottom), sizeof (lobby_saved_bottom));
        if (setup_cursor == 3) {
            m_entersound = true;
            M_Menu_Main_f();
        }
		break;
	
	case K_XBUTTON:
	case K_BACKSPACE:
		if (setup_cursor == 0)
		{
			if (strlen(setup_hostname))
				setup_hostname[strlen(setup_hostname)-1] = 0;
		}

		if (setup_cursor == 1)
		{
			if (strlen(setup_myname))
				setup_myname[strlen(setup_myname)-1] = 0;
		}
		break;
	}

	if (setup_top > 13)
		setup_top = 0;
	if (setup_top < 0)
		setup_top = 13;
	if (setup_bottom > 13)
		setup_bottom = 0;
	if (setup_bottom < 0)
		setup_bottom = 13;
}


void M_Setup_Char (int k)
{
	int l;

	switch (setup_cursor)
	{
	case 0:
		l = strlen(setup_hostname);
		if (l < 15)
		{
			setup_hostname[l+1] = 0;
			setup_hostname[l] = k;
		}
		break;
	case 1:
		l = strlen(setup_myname);
		if (l < 15)
		{
			setup_myname[l+1] = 0;
			setup_myname[l] = k;
		}
		break;
	}
}


qboolean M_Setup_TextEntry (void)
{
	return (setup_cursor == 0 || setup_cursor == 1);
}

//=============================================================================
/* NET MENU */
//
int	m_net_cursor;

//======================
/* PAUSE MENU */

int PauseOptsCount(void)
{
    /* r75b: Add/Remove Bot dropped from the pause menu (user) --
     * bot count lives in the lobby; console keeps impulse 100/101/102.
     * r75d: three rows in every gametype: Resume, Disconnect, Settings
     * (user order). */
    return 3;
}

int		pause_cursor;

void M_Menu_Pause_f (void)
{
    key_dest = key_menu;
    m_state = m_pause;
    m_entersound = true;
    pause_cursor = 0;
}

//pause
void M_Pause_Draw (void)
{

    /* r75b: pause menu attaches bottom-left (user) and lost its bot rows.
     * r75c: match the start menu's side distance + width (user). */
    draw_grid_anchor = 1;
    draw_grid_anchor_x = (int)main_x_offset_pixels;
    if ((int)deathmatch.value == 2) //firefight
    {
        struct MenuCoords coords = Draw_WindowGrid("Options", 3, MVS_P, 1, main_percentwidth, 1, pause_cursor, false);
        M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "Return to game");
        M_PrintWhite (coords.grid[0][1].xp, coords.grid[0][1].yp, "Settings");
        M_PrintWhite (coords.grid[0][2].xp, coords.grid[0][2].yp, "Disconnect");
    } else {
        struct MenuCoords coords = Draw_WindowGrid("Options", 3, MVS_P, 1, main_percentwidth, 1, pause_cursor, false);
        M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "Return to game");
        M_PrintWhite (coords.grid[0][1].xp, coords.grid[0][1].yp, "Settings");
        M_PrintWhite (coords.grid[0][2].xp, coords.grid[0][2].yp, "Disconnect");
    }
    draw_grid_anchor = 0;
    draw_grid_anchor_x = -1;
}

struct teamcounts {
    int colors;
    int playerCount;
};
void addBot () {
    if (teamplay.value == 1) {
        struct teamcounts a; // my team
        struct teamcounts b; // other team
        a.playerCount = 0;
        b.playerCount = 0;
        a.colors = cl.scores[0].colors;
        b.colors = a.colors;
        for (int i = 0; i < cl.maxclients; i++)
        {
            if (cl.scores[i].name[0])
            {
                if (a.colors == cl.scores[i].colors) {
                    a.playerCount = a.playerCount + 1;
                } else {
                    b.playerCount = b.playerCount + 1;
                    b.colors = cl.scores[i].colors;
                }
            }
        }
        if (a.playerCount >= b.playerCount) {
            Cbuf_AddText ("impulse 101\n");
            Cbuf_AddText("say teams: adding enemy\n");
        } else {
            Cbuf_AddText ("impulse 100\n");
            Cbuf_AddText("say teams: adding friend\n");
        }
    } else {
        Cbuf_AddText("say slayer: adding enemy\n");
        Cbuf_AddText ("impulse 100\n");
    }
}

void M_Pause_Key (int key)
{
    switch (key)
    {
#ifdef VITA
        case K_ENTER:
#endif
        case K_ESCAPE:
        case K_YBUTTON:
        case K_BBUTTON:
            key_dest = key_game;
            m_state = m_none;
            break;

        case K_UPARROW:
            S_LocalSound ("misc/menuoption.wav");
            pause_cursor--;
            if (pause_cursor < 0)
                pause_cursor = PauseOptsCount()-1;
            break;

        case K_DOWNARROW:
            S_LocalSound ("misc/menuoption.wav");
            pause_cursor++;
            if (pause_cursor >= PauseOptsCount())
                pause_cursor = 0;
            break;

        case K_ABUTTON:
#ifndef VITA
        case K_ENTER:
        case K_KP_ENTER:
            /* Mac/PC port: Enter selects, like the Vita X button. */
#endif
            S_LocalSound ("misc/menuenter.wav");
            /* perf B7: rows are Resume / Settings / Disconnect (user order). */
            if(pause_cursor == 0)
            {
                key_dest = key_game;
                m_state = m_none;
            }
            if(pause_cursor == 1)
            {
                M_Menu_Options_f ();
            }
            if(pause_cursor == 2)
            {
                Cbuf_AddText ("disconnect\n");
                theme_started = 0;
                M_ReturnToMatchMenu ();	/* r164: back where the match started */
            }
            break;
    }
}

//=============================================================================
/* OPTIONS MENU */

enum
{
	OPT_CUSTOMIZE = 0,
	OPT_CONSOLE,	// 1
	OPT_DEFAULTS,	// 2
	OPT_SCALE,
	OPT_SCRSIZE,
	OPT_GAMMA,
#ifdef VITA
	OPT_FOV,
#else
	OPT_CONTRAST,
#endif
	OPT_MOUSESPEED,
	OPT_SBALPHA,
	OPT_SNDVOL,
	OPT_MUSICVOL,
	OPT_MUSICEXT,
	OPT_ALWAYRUN,
	OPT_INVMOUSE,
	OPT_ALWAYSMLOOK,
	OPT_LOOKSPRING,
	OPT_LOOKSTRAFE,
#ifdef VITA
	OPT_MODS,
#endif
//#ifdef _WIN32
//	OPT_USEMOUSE,
//#endif
	OPT_PERFORMANCE,	// perf B7
	OPT_VIDEO,	// This is the last before OPTIONS_ITEMS
	OPTIONS_ITEMS
};

enum
{
	ALWAYSRUN_OFF = 0,
	ALWAYSRUN_VANILLA,
	ALWAYSRUN_QUAKESPASM,
	ALWAYSRUN_ITEMS
};

#define	SLIDER_RANGE	10

int		options_cursor;

void M_Menu_Options_f (void)
{
	uni_return_state = m_state;		// m_main or m_pause at entry
	//r101: remember tab + row across opens; first open picks the tab
	M_Options_OpenTab ();

	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;

	IN_UpdateGrabs();
}


void M_AdjustSliders (int dir)
{
	int	curr_alwaysrun, target_alwaysrun;
	float	f, l;

	S_LocalSound ("misc/menu3.wav");

	switch (options_cursor)
	{
	case OPT_SCALE:	// console and menu scale
		l = ((vid.width + 31) / 32) / 10.0;
		f = scr_conscale.value + dir * .1;
		if (f < 1)	f = 1;
		else if(f > l)	f = l;
		Cvar_SetValue ("scr_conscale", f);
		Cvar_SetValue ("scr_menuscale", f);
		Cvar_SetValue ("scr_sbarscale", f);
		break;
	case OPT_SCRSIZE:	// screen size
		f = scr_viewsize.value + dir * 10;
		if (f > 120)	f = 120;
		else if(f < 30)	f = 30;
		Cvar_SetValue ("viewsize", f);
		break;
	case OPT_GAMMA:	// gamma
		f = vid_gamma.value - dir * 0.05;
		if (f < 0.5)	f = 0.5;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("gamma", f);
		break;
#ifdef VITA
	case OPT_FOV:	// fov
		scr_fov.value += dir * 5;
		if (scr_fov.value > 130) scr_fov.value = 130;
		else if (scr_fov.value < 75) scr_fov.value = 75;
		Cvar_SetValue ("fov",scr_fov.value);
		break;
#else
	case OPT_CONTRAST:	// contrast
		f = vid_contrast.value + dir * 0.1;
		if (f < 1)	f = 1;
		else if (f > 2)	f = 2;
		Cvar_SetValue ("contrast", f);
		break;
#endif
	case OPT_MOUSESPEED:	// mouse speed
		f = sensitivity.value + dir * 0.5;
		if (f > 11)	f = 11;
		else if (f < 1)	f = 1;
		Cvar_SetValue ("sensitivity", f);
		break;
	case OPT_SBALPHA:	// statusbar alpha
		f = scr_sbaralpha.value - dir * 0.05;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("scr_sbaralpha", f);
		break;
	case OPT_MUSICVOL:	// music volume
		f = bgmvolume.value + dir * 0.1;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("bgmvolume", f);
		break;
	case OPT_MUSICEXT:	// enable external music vs cdaudio
		Cvar_Set ("bgm_extmusic", bgm_extmusic.value ? "0" : "1");
		break;
	case OPT_SNDVOL:	// sfx volume
		f = sfxvolume.value + dir * 0.1;
		if (f < 0)	f = 0;
		else if (f > 1)	f = 1;
		Cvar_SetValue ("volume", f);
		break;

	case OPT_ALWAYRUN:	// always run
		if (cl_alwaysrun.value)
			curr_alwaysrun = ALWAYSRUN_QUAKESPASM;
		else if (cl_forwardspeed.value > 200)
			curr_alwaysrun = ALWAYSRUN_VANILLA;
		else
			curr_alwaysrun = ALWAYSRUN_OFF;
			
		target_alwaysrun = (ALWAYSRUN_ITEMS + curr_alwaysrun + dir) % ALWAYSRUN_ITEMS;
			
		if (target_alwaysrun == ALWAYSRUN_VANILLA)
		{
			Cvar_SetValue ("cl_alwaysrun", 0);
			Cvar_SetValue ("cl_forwardspeed", 400);
			Cvar_SetValue ("cl_backspeed", 400);
		}
		else if (target_alwaysrun == ALWAYSRUN_QUAKESPASM)
		{
			Cvar_SetValue ("cl_alwaysrun", 1);
			Cvar_SetValue ("cl_forwardspeed", 200);
			Cvar_SetValue ("cl_backspeed", 200);
		}
		else // ALWAYSRUN_OFF
		{
			Cvar_SetValue ("cl_alwaysrun", 0);
			Cvar_SetValue ("cl_forwardspeed", 200);
			Cvar_SetValue ("cl_backspeed", 200);
		}
		break;

	case OPT_INVMOUSE:	// invert mouse
		Cvar_SetValue ("m_pitch", -m_pitch.value);
		break;

	case OPT_ALWAYSMLOOK:
		if (in_mlook.state & 1)
			Cbuf_AddText("-mlook");
		else
			Cbuf_AddText("+mlook");
		break;

	case OPT_LOOKSPRING:	// lookspring
		Cvar_Set ("lookspring", lookspring.value ? "0" : "1");
		break;

	case OPT_LOOKSTRAFE:	// lookstrafe
		Cvar_Set ("lookstrafe", lookstrafe.value ? "0" : "1");
		break;
	}
}


void M_DrawSlider (int x, int y, float range)
{
	int	i;
	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 128);
	for (i = 0; i < SLIDER_RANGE; i++)
		M_DrawCharacter (x + i*8, y, 129);
	M_DrawCharacter (x+i*8, y, 130);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 131);
}

void M_DrawCheckbox (int x, int y, int on)
{
#if 0
	if (on)
		M_DrawCharacter (x, y, 131);
	else
		M_DrawCharacter (x, y, 129);
#endif
	if (on)
		M_PrintWhite (x, y, "on");
	else
        M_PrintWhite (x, y, "off");
}

// unified Options menu (perf B7): the flat pre-tab page below is kept as the
// legacy reference but is no longer dispatched; M_Options_Draw/Key are the tabbed ones.
void M_Options_DrawOld (void)
{
	float		r, l;
    float alpha = 0.8;
    if (sv.active)
        alpha = 1;
    if (!sv.active)
        Draw_MenuBg();

    struct MenuCoords mc = Draw_WindowGrid("Options", OPTIONS_ITEMS, MVS*0.6, 2, 0.4, alpha, options_cursor, true);
    //footer
    if (options_cursor > 2 && options_cursor <= 10) {
        M_Draw_Hint(SCROLL, mc);
    } else {
        M_Draw_Hint(SELECT, mc);
    }
	// Draw the items in the order of the enum defined above:
	// OPT_CUSTOMIZE:
    M_PrintWhite (mc.grid[0][0].xp ,mc.grid[0][0].yp, "Controls");
	// OPT_CONSOLE:
    M_PrintWhite (mc.grid[0][1].xp ,mc.grid[0][1].yp, "Goto console");
	// OPT_DEFAULTS:
    M_PrintWhite (mc.grid[0][2].xp ,mc.grid[0][2].yp, "Reset config");

	// OPT_SCALE:
    M_PrintWhite (mc.grid[0][3].xp ,mc.grid[0][3].yp, "Scale");
	l = (vid.width / 320.0) - 1;
	r = l > 0 ? (scr_conscale.value - 1) / l : 0;
	M_DrawSlider (mc.grid[1][3].xp ,mc.grid[0][3].yp, r);

	// OPT_SCRSIZE:
    M_PrintWhite (mc.grid[0][4].xp ,mc.grid[0][4].yp, "Screen size");
	r = (scr_viewsize.value - 30) / (120 - 30);
	M_DrawSlider (mc.grid[1][4].xp ,mc.grid[0][4].yp, r);

	// OPT_GAMMA:
    M_PrintWhite (mc.grid[0][5].xp ,mc.grid[0][5].yp, "Brightness");
	r = (1.0 - vid_gamma.value) / 0.5;
	M_DrawSlider (mc.grid[1][5].xp ,mc.grid[0][5].yp, r);

	// OPT_CONTRAST:
#ifdef VITA
    M_PrintWhite (mc.grid[0][6].xp ,mc.grid[0][6].yp, "Field of View");
	r = (scr_fov.value - 75) / 55;
	M_DrawSlider (mc.grid[1][6].xp ,mc.grid[0][6].yp, r);
#else
    M_PrintWhite (mc.grid[0][6].xp ,mc.grid[0][6].yp, "Contrast");
	r = vid_contrast.value - 1.0;
	M_DrawSlider (mc.grid[1][6].xp ,mc.grid[0][6].yp, r);
#endif

	// OPT_MOUSESPEED:
    M_PrintWhite (mc.grid[0][7].xp ,mc.grid[0][7].yp, "Mouse Speed");
	r = (sensitivity.value - 1)/10;
	M_DrawSlider (mc.grid[1][7].xp ,mc.grid[0][7].yp, r);

	// OPT_SBALPHA:
    M_PrintWhite (mc.grid[0][8].xp ,mc.grid[0][8].yp, "Statusbar alpha");
	r = (1.0 - scr_sbaralpha.value) ; // scr_sbaralpha range is 1.0 to 0.0
	M_DrawSlider (mc.grid[1][8].xp ,mc.grid[0][8].yp, r);

	// OPT_SNDVOL:
    M_PrintWhite (mc.grid[0][9].xp ,mc.grid[0][9].yp, "Sound Volume");
	r = sfxvolume.value;
	M_DrawSlider (mc.grid[1][9].xp ,mc.grid[0][9].yp, r);

	// OPT_MUSICVOL:
    M_PrintWhite (mc.grid[0][10].xp ,mc.grid[0][10].yp, "Music Volume");
	r = bgmvolume.value;
	M_DrawSlider (mc.grid[1][10].xp ,mc.grid[0][10].yp, r);

	// OPT_MUSICEXT:
    M_PrintWhite (mc.grid[0][11].xp ,mc.grid[0][11].yp, "External Music");
	M_DrawCheckbox (mc.grid[1][11].xp ,mc.grid[0][11].yp, bgm_extmusic.value);

	// OPT_ALWAYRUN:
    M_PrintWhite (mc.grid[0][12].xp ,mc.grid[0][12].yp, "Always Run");
	if (cl_alwaysrun.value)
		M_PrintWhite (mc.grid[1][12].xp ,mc.grid[0][12].yp, "quakespasm");
	else if (cl_forwardspeed.value > 200.0)
        M_PrintWhite (mc.grid[1][12].xp ,mc.grid[0][12].yp, "vanilla");
	else
        M_PrintWhite (mc.grid[1][12].xp ,mc.grid[0][12].yp, "off");

	// OPT_INVMOUSE:
    M_PrintWhite (mc.grid[0][13].xp ,mc.grid[0][13].yp, "Invert Mouse");
	M_DrawCheckbox (mc.grid[1][13].xp ,mc.grid[0][13].yp, m_pitch.value < 0);

	// OPT_ALWAYSMLOOK:
    M_PrintWhite (mc.grid[0][14].xp ,mc.grid[0][14].yp, "Mouse Look");
	M_DrawCheckbox (mc.grid[1][14].xp ,mc.grid[0][14].yp, in_mlook.state & 1);

	// OPT_LOOKSPRING:
    M_PrintWhite (mc.grid[0][15].xp ,mc.grid[0][15].yp, "Lookspring");
	M_DrawCheckbox (mc.grid[1][15].xp ,mc.grid[0][15].yp, lookspring.value);

	// OPT_LOOKSTRAFE:
    M_PrintWhite (mc.grid[0][16].xp ,mc.grid[0][16].yp, "Lookstrafe");
	M_DrawCheckbox (mc.grid[1][16].xp ,mc.grid[0][16].yp, lookstrafe.value);
	
#ifdef VITA
	// OPT_MODS
	M_PrintWhite (mc.grid[0][17].xp ,mc.grid[0][17].yp, "Select Mod");
#endif

	// OPT_PERFORMANCE:	// perf B7
	M_PrintWhite (mc.grid[0][OPT_PERFORMANCE].xp ,mc.grid[0][OPT_PERFORMANCE].yp, "Performance");

	// OPT_VIDEO:
	if (vid_menudrawfn)
        M_PrintWhite (mc.grid[0][OPTIONS_ITEMS-1].xp ,mc.grid[0][OPTIONS_ITEMS-1].yp, "Advanced Options");

}


// legacy: see M_Options_Key (unified, perf B7)
void M_Options_KeyOld (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
    case K_YBUTTON:
        if (sv.active) { //user is in pause menu
            M_Menu_Pause_f();
        } else {
            Host_WriteConfiguration();
            M_Menu_Main_f ();
        }
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;
		switch (options_cursor)
		{
		case OPT_CUSTOMIZE:
			M_Menu_Keys_f ();
			break;
		case OPT_CONSOLE:
			m_state = m_none;
			Con_ToggleConsole_f ();
			break;
		case OPT_DEFAULTS:
			if (SCR_ModalMessage("This will reset all controls\n"
					"and stored cvars. Continue? (y/n)\n", 15.0f))
			{
				Cbuf_AddText ("resetcfg\n");
				Cbuf_AddText ("exec default.cfg\n");
			}
			break;
#ifdef VITA
		case OPT_MODS:
			M_Menu_Mods_f ();
			break;
#endif
		case OPT_PERFORMANCE:	// perf B7
			M_Menu_Performance_f ();
			break;
		case OPT_VIDEO:
			M_Menu_Video_f ();
			break;
		default:
			M_AdjustSliders (1);
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;

	case K_LEFTARROW:
		M_AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_AdjustSliders (1);
		break;
	}

	if (options_cursor == OPTIONS_ITEMS - 1 && vid_menudrawfn == NULL)
	{
		if (k == K_UPARROW)
			options_cursor = OPTIONS_ITEMS - 2;
		else
			options_cursor = 0;
	}
}

//=============================================================================
/* KEYS MENU */

char *quakebindnames[][2] =
{
        {"+attack", 		"Attack"},
        {"impulse 10", 		"Cycle weapons"},
        {"impulse 13", 		"Pickup / Reload"},
        {"impulse 27", 		"Change Grenade"},
        {"impulse 28", 		"Throw Grenade"},
        {"impulse 29", 		"Melee"},
        {"impulse 11", 		"Zoom"},
        {"+jump", 			"Jump / Swim up"},
        {"+forward", 		"Forward"},
        {"+back", 			"Back"},
        {"+moveleft", 		"Left" },
        {"+moveright", 		"Right" }
};

#define	NUMQUAKECOMMANDS	(sizeof(quakebindnames)/sizeof(quakebindnames[0]))

static struct
{
	char *cmd;
	char *desc;
} *bindnames;
static size_t numbindnames;

static size_t	keys_first;
static size_t	keys_cursor;
qboolean	bind_grab;

void M_Keys_Close (void)
{
	while (numbindnames>0)
	{
		numbindnames--;
		Z_Free(bindnames[numbindnames].cmd);
		Z_Free(bindnames[numbindnames].desc);
	}
	Z_Free(bindnames);
	bindnames = NULL;
	numbindnames = 0;
}
void M_Keys_Populate(void)
{
	FILE *file;
	char line[1024];
	if (numbindnames)
		return;

	if (COM_FOpenFile("bindlist.lst", &file, NULL) >= 0)
	{
		while (fgets(line, sizeof(line), file))
		{
			const char *cmd, *desc/*, tip*/;
			Cmd_TokenizeString(line);
			cmd = Cmd_Argv(0);
			desc = Cmd_Argv(1);
			/*tip = Cmd_Argv(2); unused in quakespasm*/

			if (*cmd)
			{
				bindnames = Z_Realloc(bindnames, sizeof(*bindnames)*(numbindnames+1));
				bindnames[numbindnames].cmd = strcpy(Z_Malloc(strlen(cmd)+1), cmd);
				bindnames[numbindnames].desc = strcpy(Z_Malloc(strlen(desc)+1), desc);
				numbindnames++;
			}
		}
		fclose(file);
	}

	if (!numbindnames)
	{
		bindnames = Z_Realloc(bindnames, sizeof(*bindnames)*NUMQUAKECOMMANDS);
		while(numbindnames < NUMQUAKECOMMANDS)
		{
			bindnames[numbindnames].cmd = strcpy(Z_Malloc(strlen(quakebindnames[numbindnames][0])+1), quakebindnames[numbindnames][0]);
			bindnames[numbindnames].desc = strcpy(Z_Malloc(strlen(quakebindnames[numbindnames][1])+1), quakebindnames[numbindnames][1]);
			numbindnames++;
		}
	}

	//don't start with it on text
	keys_first = keys_cursor = 0;
	while (keys_cursor < numbindnames-1 && !strcmp(bindnames[keys_cursor].cmd, "-"))
		keys_cursor++;
}

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
	IN_UpdateGrabs();

	M_Keys_Populate();
}


void M_FindKeysForCommand (const char *command, int *threekeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;
	int		bindmap = 0;

	threekeys[0] = threekeys[1] = threekeys[2] = -1;
	l = strlen(command);
	count = 0;

	for (j = 0; j < MAX_KEYS; j++)
	{
		b = keybindings[bindmap][j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			threekeys[count] = j;
			count++;
			if (count == 3)
				break;
		}
	}
}

void M_UnbindCommand (const char *command)
{
	int		j;
	int		l;
	char	*b;
	int		bindmap = 0;

	l = strlen(command);

	for (j = 0; j < MAX_KEYS; j++)
	{
		b = keybindings[bindmap][j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, NULL, bindmap);
	}
}

extern qpic_t	*pic_up, *pic_down;

void M_Keys_Draw (void)
{
	size_t		i;
	int x, y;
	int		keys[3];
	const char	*name;
	size_t keys_shown;
    int vertical_spacing = MVS*0.6;
    float alpha = 0.8;
    if (sv.active)
        alpha = 1;
    if (!sv.active)
        Draw_MenuBg();

    // window
    struct MenuCoords mc = Draw_WindowGrid("Controls", 19, vertical_spacing, 2, 0.4, alpha, keys_cursor-keys_first, true);
    // footer
    if (bind_grab) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "Press a button for this action");
    } else {
        M_Draw_Hint(SELECT_DELETE, mc);
    }

	keys_shown = numbindnames;
	if (keys_shown > (200-48)/8)
		keys_shown = (200-48)/8;
	if (keys_first+keys_shown-1 < keys_cursor)
		keys_first = keys_cursor-(keys_shown-1);
	if (keys_first > keys_cursor)
		keys_first = keys_cursor;

// search for known bindings
	for (i = keys_first; i < keys_first+keys_shown; i++)
	{
		y = mc.grid[0][0].yp + vertical_spacing*(i-keys_first);

		if (!strcmp(bindnames[i].cmd, "-"))
		{
			M_PrintWhite (mc.grid[0][0].xp, y, bindnames[i].desc);
			continue;
		}

        M_PrintWhite (mc.grid[0][0].xp, y, bindnames[i].desc);

		M_FindKeysForCommand (bindnames[i].cmd, keys);

		if (keys[0] == -1)
		{
			M_PrintWhite(mc.grid[1][0].xp, y, "???");
		}
		else
		{
			name = Key_KeynumToString (keys[0]);
            M_PrintWhite (mc.grid[1][0].xp, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				name = Key_KeynumToString (keys[1]);
                M_PrintWhite (mc.grid[1][0].xp + x + 8, y, "or");
                M_PrintWhite (mc.grid[1][0].xp + x + 32, y, name);
				x = x + 32 + strlen(name) * 8;
				if (keys[2] != -1)
				{
                    M_PrintWhite (mc.grid[1][0].xp + x + 8, y, "or");
                    M_PrintWhite (mc.grid[1][0].xp + x + 32, y, Key_KeynumToString (keys[2]));
				}
			}
		}
	}

	if (bind_grab) {
        M_DrawCharacterColored (mc.grid[1][0].x, mc.grid[1][0].y + (keys_cursor-keys_first)*vertical_spacing, '=');
    }
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[3];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menuoption.wav");
		if ((k != K_ESCAPE) && (k != '`'))
		{
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor].cmd);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		IN_UpdateGrabs();
		return;
	}

	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		do
		{
			keys_cursor--;
			if (keys_cursor >= numbindnames)
			{
				if (keys_first && strcmp(bindnames[keys_first].cmd, "-"))
				{	//some weirdness, so the user can re-view any leading titles
					keys_cursor = keys_first;
					keys_first = 0;
				}
				else
					keys_cursor = numbindnames-1;
				break;
			}
		} while (!strcmp(bindnames[keys_cursor].cmd, "-"));
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		do
		{
			keys_cursor++;
			if (keys_cursor >= numbindnames)
				keys_cursor = keys_first = 0;
			else if (keys_cursor == numbindnames-1)
				break;
		} while (!strcmp(bindnames[keys_cursor].cmd, "-"));
		break;

	case K_ENTER:		// go into bind mode
	case K_KP_ENTER:
	case K_ABUTTON:
		M_FindKeysForCommand (bindnames[keys_cursor].cmd, keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[2] != -1)
			M_UnbindCommand (bindnames[keys_cursor].cmd);
		bind_grab = true;
		IN_UpdateGrabs(); // activate to allow mouse key binding
		break;
	
	case K_XBUTTON:
	case K_BACKSPACE:	// delete bindings
	case K_DEL:
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor].cmd);
		break;
	}
}

//=============================================================================
/* VIDEO MENU */

void M_Menu_Video_f (void)
{
	(*vid_menucmdfn) (); //johnfitz
}


void M_Video_Draw (void)
{
	(*vid_menudrawfn) ();
}


void M_Video_Key (int key)
{
	(*vid_menukeyfn) (key);
}

//=============================================================================
/* HELP MENU */

int		help_page;
#define	NUM_HELP_PAGES	6


void M_Menu_Help_f (void)
{
	key_dest = key_menu;
	m_state = m_help;
	m_entersound = true;
	help_page = 0;
	IN_UpdateGrabs();
}



void M_Help_Draw (void)
{
	M_DrawPic (0, 0, Draw_CachePic ( va("gfx/help%i.lmp", help_page)) );
}


void M_Help_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Main_f ();
		break;

	case K_UPARROW:
	case K_RIGHTARROW:
		m_entersound = true;
		if (++help_page >= NUM_HELP_PAGES)
			help_page = 0;
		break;

	case K_DOWNARROW:
	case K_LEFTARROW:
		m_entersound = true;
		if (--help_page < 0)
			help_page = NUM_HELP_PAGES-1;
		break;
	}

}

//=============================================================================
/* QUIT MENU */

int		msgNumber;
enum m_state_e	m_quit_prevstate;
qboolean	wasInMenus;

char *quitMessage [] =
{
/* .........1.........2.... */
        "    Ready to retire     ",
        "         Chief?         ",
        "                        ",
        "   O NO             X YES   ",

        " Milord, methinks that  ",
        "   thou art a lowly     ",
        " quitter. Is this true? ",
        "                        ",
        " Do I need to bust your ",
        "  face open for trying  ",
        "        to quit?        ",
        "                        ",
        " Man, I oughta smack you",
        "   for trying to quit!  ",
        "     Press X to get     ",
        "      smacked out.      ",
        " What, you want to stop ",
        "   playing VitaQuake?   ",
        "     Press X or O to    ",
        "   return to LiveArea.  ",
        " Press X to quit like a ",
        "   big loser in life.   ",
        "  Return back to stay   ",
        "  proud and successful! ",
        "   If you press X to    ",
        "  quit, I will summon   ",
        "  Satan all over your   ",
        "      memory card!      ",
        "  Um, Asmodeus dislikes ",
        " his children trying to ",
        " quit. Press X to return",
        "   to your Tinkertoys.  ",
        "  If you quit now, I'll ",
        "  throw a blanket-party ",
        "   for you next time!   ",
        "                        "
};

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = 0;

	IN_UpdateGrabs();
}

void M_Quit_Char (int key)
{
	switch (key)
	{
    case K_YBUTTON: // triangle
    case K_BBUTTON: // circle
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
			IN_UpdateGrabs();
		}
		break;

    case K_ABUTTON: // x
	case 'y':
	case 'Y':
		key_dest = key_console;
		Host_Quit_f ();
		IN_UpdateGrabs();
		break;

	default:
		break;
	}
}

void M_Quit_Key (int key)
{
    /* keyboard parity (user): Enter confirms like A/X, ESC declines like
     * B/circle. M_Quit_Char already maps those buttons; nothing upstream
     * swallows ESC in this state, so translating here is enough. */
    if (key == K_ENTER)
        key = K_ABUTTON;
    else if (key == K_ESCAPE)
        key = K_BBUTTON;
    M_Quit_Char(key);
}

qboolean M_Quit_TextEntry (void)
{
	return true;
}


void M_Quit_Draw (void)
{
    if (wasInMenus)
    {
        m_state = m_quit_prevstate;
        m_recursiveDraw = true;
        M_Draw ();
        m_state = m_quit;
    }
    int xoff = 80;
    int yoff = 20;

    Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 0.7f);
    struct MenuCoords coords = Draw_WindowGrid("Quit", 2, MVS_P, 2, 0.2, 1.0, -1, false);
    M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "Ready to retire, Chief?");
    M_PrintWhite (coords.grid[0][1].xp, coords.grid[0][1].yp, "Yes");
    Draw_Button(coords.grid[0][1].xp + 4*CHARZ, coords.grid[0][1].yp, b_abutton);
    M_PrintWhite (coords.grid[1][1].xp, coords.grid[1][1].yp, "No");
    Draw_Button(coords.grid[1][1].xp + 3*CHARZ, coords.grid[1][1].yp, b_bbutton);
}

//=============================================================================
/* LAN CONFIG MENU */

int		lanConfig_cursor = -1;
#define NUM_LANCONFIG_CMDS	4

int 	lanConfig_port;
char	lanConfig_portname[6];
char	lanConfig_joinname[22];

/***
 * This method creates and/or joins a game over IPX or TCP/IP
 *  - Initializer component
 */
/* r163 (user: "exiting custom games and matchmaking should go back to their
 * respective menu, not always custom"): this screen is opened from Custom
 * Games, the Matchmaking lobby and the main menu, but always returned to
 * Custom Games. Latch who opened it and go back there. */
static enum m_state_e	lanconfig_origin = m_main;

static void M_LanConfig_Back (void)
{
	if (lanconfig_origin == m_lobbies)
		M_Lobbies_f ();		/* Find Match */
	else if (lanconfig_origin == m_gameoptions)
		M_Matchmaking_f ();	/* Custom Games */
	else if (m_multiplayer_cursor == 0)
		M_Menu_Main_f ();
	else
		M_Matchmaking_f ();
}

void M_Menu_LanConfig_f (void)
{
	lanconfig_origin = m_state;
	if (lanconfig_origin != m_lobbies && lanconfig_origin != m_gameoptions)
		lanconfig_origin = m_main;
	key_dest = key_menu;
	m_state = m_lanconfig;
	m_entersound = true;
	if (lanConfig_cursor == -1)
	{
		if (JoiningGame && TCPIPConfig)
			lanConfig_cursor = 2;
		else
			lanConfig_cursor = 1;
	}
	if (StartingGame && lanConfig_cursor >= 2)
		lanConfig_cursor = 1;
	lanConfig_port = DEFAULTnet_hostport;
	sprintf(lanConfig_portname, "%u", lanConfig_port);

	m_return_onerror = false;
	m_return_reason[0] = 0;
    mm_rentry = true; //used in matchmaking menu
	IN_UpdateGrabs();
}

const char	*protocol = "null";

/***
 * This method creates and/or joins a game over IPX or TCP/IP
 *  - Draw component
 */
void M_LanConfig_Draw (void)
{
    qpic_t	*p;
    int		basex;
    int		y;
    int		numaddresses, i;
    qhostaddr_t addresses[16];
    const char	*startJoin;
    float width = 0.4;
    int rows = 6;
    float alpha = 0.667;
    if (StartingGame) {
        width = 0.25;
        rows = 4;
        // set to matchmaking for recursive draw
        m_state = m_gameoptions;
        m_recursiveDraw = true;
        M_Draw ();
        // now draw this menu
        m_state = m_lanconfig;
        // background tint
        Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 0.7f);
        alpha = 1.0;
    } else {
        Draw_MenuBg();
    }

    struct MenuCoords mc = Draw_WindowGrid("Matchmaking", rows, MVS_P, 2, width, alpha, -1, false);

	if (StartingGame)
		startJoin = "New Game";
	else
		startJoin = "Join Game";
	if (IPXConfig)
		protocol = "IPX";
	else
		protocol = "TCP/IP";
	M_PrintWhite(mc.grid[0][0].xp, mc.grid[0][0].yp, va ("%s - %s", startJoin, protocol));

	M_Print (mc.grid[0][1].xp, mc.grid[0][1].yp, "Address:");
    y = mc.grid[0][1].yp;
#if 1
	numaddresses = NET_ListAddresses(addresses, sizeof(addresses)/sizeof(addresses[0]));
	if (!numaddresses)
	{
		M_PrintWhite(mc.grid[1][1].xp, y, "NONE KNOWN");
		y += MVS;
	}
	else for (i = 0; i < numaddresses; i++)
	{
		M_PrintWhite(mc.grid[1][1].xp, y, addresses[i]);
		y += MVS;
	}
#else
	if (IPXConfig)
	{
		M_Print (mc.grid[1][1].xp, y, my_ipx_address);
		y+=MVS;
	}
	else
	{
		if (ipv4Available && ipv6Available)
		{
			M_Print (mc.grid[1][1].xp, y, my_ipv4_address);
			y+=MVS;
			M_Print (mc.grid[1][1].xp, y, my_ipv6_address);
			y+=MVS;
		}
		else
		{
			if (ipv4Available)
				M_Print (mc.grid[1][1].xp, y, my_ipv4_address);
			if (ipv6Available)
				M_Print (mc.grid[1][1].xp, y, my_ipv6_address);
			y+=MVS;
		}
	}
#endif

//	y+=MVS;	//for the port's box
	M_Print (mc.grid[0][2].xp, y, "Port");
	M_DrawTextBox (mc.grid[1][2].xp, y-CHARZ, 6, 1);
	M_PrintWhite (mc.grid[1][2].xp + CHARZ, y, lanConfig_portname);
	if (lanConfig_cursor == 0)
	{
		M_DrawCharacter (mc.grid[1][2].xp + 8*strlen(lanConfig_portname), y, 10+((int)(realtime*4)&1));
		M_DrawCharacter (mc.grid[1][2].x, y, 12+((int)(realtime*4)&1));
	}
	y += MVS;

	if (JoiningGame)
	{
		M_Print (mc.grid[0][3].xp, y, "Search for local games...");
		if (lanConfig_cursor == 1)
			M_DrawCharacter (mc.grid[0][3].x, y, 12+((int)(realtime*4)&1));
		y+=MVS;

		M_Print (mc.grid[0][3].xp, y, "Search for public games...");
		if (lanConfig_cursor == 2)
			M_DrawCharacter (mc.grid[0][3].x, y, 12+((int)(realtime*4)&1));
		y+=MVS;

		M_Print (mc.grid[0][4].xp, y, "Join game at:");
		M_DrawTextBox (mc.grid[1][4].xp, y-CHARZ, 22, 1);
		M_PrintWhite (mc.grid[1][4].xp + CHARZ, y, lanConfig_joinname);
		if (lanConfig_cursor == 3)
		{
			M_DrawCharacter (mc.grid[1][4].xp + CHARZ + CHARZ*strlen(lanConfig_joinname), y, 10+((int)(realtime*4)&1));
			M_DrawCharacter (mc.grid[1][4].x-CHARZ, y, 12+((int)(realtime*4)&1));
		}
		y += MVS;
	}
	else
	{
//		M_DrawTextBox (mc.grid[0][3].xp, y-CHARZ, 2, 1);
		M_Print (mc.grid[0][3].xp, y, "OK");
		if (lanConfig_cursor == 1)
			M_DrawCharacter (mc.grid[0][3].x, y, 12+((int)(realtime*4)&1));
		y += MVS;
	}

	if (*m_return_reason)
		M_PrintWhite (mc.grid[0][5].xp, mc.grid[0][5].yp, m_return_reason);
}

/***
 * This method creates and/or joins a game over IPX or TCP/IP
 *  - Key input component
 */
void M_LanConfig_Key (int key)
{
	int		l;

	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_LanConfig_Back ();	/* r163: back where we came from */
//		M_Menu_Net_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		lanConfig_cursor--;
		if (lanConfig_cursor < 0)
			lanConfig_cursor = NUM_LANCONFIG_CMDS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		lanConfig_cursor++;
		if (lanConfig_cursor >= NUM_LANCONFIG_CMDS)
			lanConfig_cursor = 0;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		if (lanConfig_cursor == 0)
			break;

		m_entersound = true;

		M_ConfigureNetSubsystem ();

		if (StartingGame)
		{
			if (lanConfig_cursor == 1)
				M_LanConfig_Back ();	/* r163 */
		}
		else
		{
			if (lanConfig_cursor == 1)
				M_Menu_Search_f(SLIST_LAN);
			else if (lanConfig_cursor == 2)
				M_Menu_Search_f(SLIST_INTERNET);
			else if (lanConfig_cursor == 3)
			{
				m_return_state = m_state;
				m_return_onerror = true;
				/* r164: remember which screen asked for the join */
				match_return_state = (lanconfig_origin == m_lobbies)
					? m_lobbies : m_gameoptions;
				key_dest = key_game;
				m_state = m_none;
				IN_UpdateGrabs();
				SCR_ShowLoadingScreen ();
				Cbuf_AddText ( va ("connect \"%s\"\n", lanConfig_joinname) );
			}
		}

		break;
#ifdef VITA
	case K_YBUTTON:
		if (lanConfig_cursor == 3)
			IN_SwitchKeyboard(lanConfig_joinname, 22);
		else if (lanConfig_cursor == 0)
			IN_SwitchKeyboard(lanConfig_portname, 6);
		break;
#endif
	case K_BACKSPACE:
		if (lanConfig_cursor == 0)
		{
			if (strlen(lanConfig_portname))
				lanConfig_portname[strlen(lanConfig_portname)-1] = 0;
		}

		if (lanConfig_cursor == 3)
		{
			if (strlen(lanConfig_joinname))
				lanConfig_joinname[strlen(lanConfig_joinname)-1] = 0;
		}
		break;
	}

	if (StartingGame && lanConfig_cursor >= 2)
	{
		if (key == K_UPARROW)
			lanConfig_cursor = 1;
		else
			lanConfig_cursor = 0;
	}

	l =  Q_atoi(lanConfig_portname);
	if (l > 65535)
		l = lanConfig_port;
	else
		lanConfig_port = l;
	sprintf(lanConfig_portname, "%u", lanConfig_port);
}

/***
 * This method creates and/or joins a game over IPX or TCP/IP
 *  - helper method
 */
void M_LanConfig_Char (int key)
{
	int l;

	switch (lanConfig_cursor)
	{
	case 0:
		if (key < '0' || key > '9')
			return;
		l = strlen(lanConfig_portname);
		if (l < 5)
		{
			lanConfig_portname[l+1] = 0;
			lanConfig_portname[l] = key;
		}
		break;
	case 3:
		l = strlen(lanConfig_joinname);
		if (l < 21)
		{
			lanConfig_joinname[l+1] = 0;
			lanConfig_joinname[l] = key;
		}
		break;
	}
}

/***
 * This method creates and/or joins a game over IPX or TCP/IP
 *  - helper method
 */
qboolean M_LanConfig_TextEntry (void)
{
	return (lanConfig_cursor == 0 || lanConfig_cursor == 3);
}

//=============================================================================
/* GAME OPTIONS MENU */
typedef struct {
    int episode;
    int level;
} eplev; // chosen level & episode
eplev chosen_level = {0,0};
typedef struct
{
	const char	*name;
    const char  *thumbnail;
    float targaThumbnail;
	const char	*description;
    const char  *mapBio;   /* r92: single blurb, word-wrapped at draw time */
    const char  *author;
} level_t;

level_t		levels[] =
{
        // Refined map pack
        {"highground", "highground", 0, "High Ground", "A vertical fight across high ground above the valley. Control the lift and the overlook and the low ground never comes for you.", "mattthw"},
        {"citadel", "citadel", 0, "Citadel", "A vast Forerunner monument stacked floor upon floor. Long sightlines reward patience while the stairwells punish it.", "mattthw"},
        {"babybloodgulch", "babybloodgulch", 1, "Blood Gutch", "Two bases split by a wide open trench. Cross it fast, hug the walls, and never walk the ridge in the open.", "Unknown"},
        {"pit_optimized", "pit3", 1, "Pit", "A deep UNSC training hole ringed by gantries. Drop in with a shotgun and climb out with a story.", "Jukki, mattthw"},
        {"narsp", "narsp", 1, "Narrows", "The Ark's cooling channels, still running cold. Tight catwalks over open water make every duel a knife fight.", "Scifiknux"},
        {"skyringbl2", "skyringbl2", 1, "Heavenly Suspense", "A Halo ring facility under repair, open sky on every side. Snipers own the scaffolds; everyone else hides.", "Arkage"},
        {"lockdown", "lockdown", 1, "Lockdown", "A prison block on full lockdown. Corridors funnel every push into one bad doorway at a time.", "Team Xlink, mattthw"},
        {"cestra", "cestra", 1, "Cestra", "A buried research lab the brass never expected anyone to fight through. Every hall tests something new.", "Sam"},
        // Classic map pack [8]
        {"Longest", "Longest", 0, "Longest", "The longest lane in the rotation. Mid-range duels down an endless aisle, with just enough cover to feint.", "Unknown"},
        {"chill", "chill", 0, "Chill Out", "A frozen outpost of short halls and shorter tempers. Indoor fights end before you find your breath.", "Unknown"},
        {"pri2", "pri2", 0, "Prisoner 2", "A compact prison complex. Three levels of balconies turn every escort into an ambush.", "Unknown"},
        {"base", "base", 0, "Minibase", "A small two-flag outpost built for duels. One ridge, two bases, and nowhere to hide.", "Unknown"},
        {"plaza", "plaza", 0, "Plaza", "An open civic plaza ringed by shops. The middle is a graveyard; own the rooftops instead.", "Unknown"},
        {"spider", "spider", 0, "Spiderweb", "Towers strung together on narrow webbing. Fall off a bridge and the match is already lost.", "Unknown"},
        {"wizard", "wizard", 1, "Wizard", "A small ring arena that never stops turning. Round and round and round - the middle belongs to no one.", "IlDucci"},
        {"lane", "lane", 1, "Lane", "An outdoor human training lane: one long corridor of cover with flanking shelves above it.", "Sam"},
        // Bonus map pack
        {"pit", "random", 0, "Pit (original)", "The first cut of the Pit: three towers, one hole, and no forgiveness for bad jumps.", "Jukki"},
        {"pit2", "pit2", 1, "Pit (v2)", "The Pit rebuilt - more gantries, more corners, and more reasons to watch your footing.", "Jukki, mattthw"},
        {"pit3", "pit3", 1, "Pit HD", "The full-weight cut of the Pit: same deep hole, every gantry detailed, no budget cuts. The one that started it.", "Jukki, mattthw"},
        {"lock", "lockout", 0, "Lockout2", "A tight refinery platform where the fight always comes back to the ramp.", "Unknown"},
        {"+bloodgulch2", "bgulch", 0, "Blood Gutch", "The full-width Gulch. Same trench, bigger mistake if you stand in it too long.", "Unknown"},
        {"narrows", "narrows", 0, "Narrows Big", "The Ark channel stretched out big. The water is still cold and the catwalks are still narrow.", "mattthw"},
        // Firefight map pack
        {"construction", "construction", 0, "Construction", "An unfinished site of scaffolds and drop-shafts. High ground is free if you survive the climb.", "Unknown"},
        {"fire", "fire", 0, "Fire!", "A refinery gone up in smoke. Short sightlines, hot decks, and no second chances.", "Unknown"},

        //{"lockout", "lockout", "Lockout"},
};


typedef struct
{
	const char	*description;
	int		firstLevel;
	int		levels;
} episode_t;
int NUM_EPISODES = 4;
episode_t	episodes[] =
        {
                {"New & Remastered", 0, 8},
                {"Classic Maps", 8, 8},
                {"Bonus Maps", 16, 6},
                {"Custom Maps", 0, 0}	// perf B7: loose .bsp files from maps/custom, counted at runtime
        };
int NUM_EPISODES_FF = 1;
episode_t	episodes_ff[] =
        {
                {"Firefight Maps", 22, 2}
        };

extern cvar_t sv_public;



//========= || MATCHMAKING || ============

int	startepisode;
int	startlevel,prevlevel,nextlevel;
int maxplayers;
qboolean cursorfocused = 1;

// gametypes available
typedef enum gametype_t {
    SLAYER = 0,
    TEAM_SLAYER = 1,
    SWAT = 2
} gametype_t;

typedef struct opts_t {
    int teamplay;
    int coop;
    int deathmatch;
    int skill;
    int timelimit;
    int fraglimit;
    int teamlimit;
    enum gametype_t gametype;
    // custom game options
    int custom_primary;
    int custom_secondary;
    int map_weapons_disable;
    int infinite_ammo;
    int max_nades;
    int grunt_birthday_party;
    int sol_jump;
    int sol_maxspeed;
    int sol_gravity;

} opts_t;
#define DEFAULT_GRAVITY 340
#define DEFAULT_SPEED 200
#define DEFAULT_JUMP 225
// default options
struct opts_t opts = {
        .teamplay = 0,
        .coop = 0,
        .deathmatch = 1,
        .skill = 1,
        .timelimit = 10,
        .fraglimit = 20,   /* r77: Team Slayer default (slayer modes get 10 on switch) */
        .teamlimit = 0,
        .gametype = TEAM_SLAYER,
        /* CUSTOM GAME OPTIONS */
        .custom_primary = 0,
        .custom_secondary = 1,
        .map_weapons_disable = 0,
        .max_nades = 2,	/* perf B7 */
        .infinite_ammo = 0,
        .grunt_birthday_party = 0,
        .sol_gravity = DEFAULT_GRAVITY, // todo: pull from cvar_t
        .sol_maxspeed = DEFAULT_SPEED, // todo: pull from cvar_t
        .sol_jump = DEFAULT_JUMP // todo: pull from cvar_t
};

void resetGameRules() {
    opts.custom_primary = 0;
    opts.custom_secondary = 1;
    opts.map_weapons_disable = 0;
    opts.infinite_ammo = 0;
    opts.fraglimit = (opts.gametype == TEAM_SLAYER) ? 20 : 10;   /* r77 per-mode defaults */
    opts.timelimit = 10;
    opts.skill = 1;
    opts.max_nades = 2;	/* perf B7 */
    opts.grunt_birthday_party = 0;
    opts.sol_jump = DEFAULT_JUMP;
    opts.sol_gravity = DEFAULT_GRAVITY;
    opts.sol_maxspeed = DEFAULT_SPEED;
}

typedef struct custom_weapon_t {
    int weapon;
    char *name;
} custom_weapon_t;
struct custom_weapon_t gamerule_weapons[] = {
        {WEAPON_AR, "Assault Rifle"},
        {WEAPON_PISTOL, "Pistol"},
        {WEAPON_SWORD, "Energy Sword"},
        {WEAPON_SHOT, "Shotgun"},
        {WEAPON_UZI, "SMG"},
        {WEAPON_SNIPER, "Sniper"},
        {WEAPON_RL, "Rocket Launcher"},
        {WEAPON_PPISTOL, "Plasma Pistol"},
        {WEAPON_PRIFLE, "Plasma Rifle"},
        {WEAPON_SKULL, "Skull"}
};
struct custom_weapon_t gamerule_weapons_secondary[] = {
        {WEAPON_AR, "Assault Rifle"},
        {WEAPON_PISTOL, "Pistol"},
        {WEAPON_SWORD, "Energy Sword"},
        {WEAPON_SHOT, "Shotgun"},
        {WEAPON_UZI, "SMG"},
        {WEAPON_SNIPER, "Sniper"},
        {WEAPON_RL, "Rocket Launcher"},
        {WEAPON_PPISTOL, "Plasma Pistol"},
        {WEAPON_PRIFLE, "Plasma Rifle"},
        {WEAPON_SKULL, "Skull"},
        {134217728, "None"}
};

/*
=============
M_itoa
=============
*/
int M_itoa (int num, char *buf)
{
    char	*str;
    int		pow10;
    int		dig;

    str = buf;

    if (num < 0)
    {
        *str++ = '-';
        num = -num;
    }

    for (pow10 = 10 ; num >= pow10 ; pow10 *= 10)
        ;

    do
    {
        pow10 /= 10;
        dig = num/pow10;
        *str++ = '0'+dig;
        num -= dig*pow10;
    } while (pow10 != 1);

    *str = 0;

    return str-buf;
}
char* drawRuleData(float f) {
    if (f)
        return "Enabled";
    else
        return "Disabled";
}
char* drawDifficulty(int f) {
    switch (f) {
        case 0:
            return "Easy";
        case 1:
            return "Normal";
        case 2:
            return "Heroic";
        case 3:
            return "Legendary";
        default:
            return "BROKEN!";

    }
}

/**
 * Custom rules for the custom games lobby such as snipers only, infinite ammo, etc.
 */
size_t		gamerules_cursor;
size_t		gamerules_first;
float vertical_gamerules_affordance = 8;
float rule_list_size = 13; //Should equal sizeof(rule_titles)/sizeof(rule_titles[0])
void M_Matchmaking_Gamerules_f (void) {
    key_dest = key_menu;
    m_state = m_matchmaking_gamerules;
    m_entersound = true;
    gamerules_cursor = 0;
    gamerules_first = 0;
    IN_UpdateGrabs();
}
void M_Matchmaking_Gamerules_Draw (void) {
    // set to matchmaking for recursive draw
    m_state = m_gameoptions;
    m_recursiveDraw = true;
    M_Draw ();
    // now draw this menu
    m_state = m_matchmaking_gamerules;

    // create game option display data before drawing
    char str_nades[20];
    char str_grav[20];
    char str_speed[20];
    char str_jump[20];
    M_itoa(opts.max_nades, str_nades);
    M_itoa(opts.sol_gravity, str_grav);
    M_itoa(opts.sol_maxspeed, str_speed);
    M_itoa(opts.sol_jump, str_jump);
    struct rule_option_t {
        char* name;
        char* value;
    } rule_option_t;
    struct rule_option_t rule_titles[13] = {
            {"Score Limit", opts.fraglimit ? va("%i Kills", (int)opts.fraglimit) : "Unlimited"},
            {"Time Limit", opts.timelimit ? va("%i Minutes", (int)opts.timelimit) : "Unlimited"},
            {"Primary Weapon",gamerule_weapons[opts.custom_primary].name},
            {"Secondary Weapon",gamerule_weapons_secondary[opts.custom_secondary].name},
            {"Disable Ground Weapons",drawRuleData(opts.map_weapons_disable)},
            {"Difficulty", drawDifficulty(opts.skill)},
            {"Maximum Grenades", str_nades},
            {"Infinite Ammo", drawRuleData(opts.infinite_ammo)},
            {"Grunt Birthday Party", drawRuleData(opts.grunt_birthday_party)},
            {"Gravity", str_grav},
            {"Max Speed", str_speed},
            {"Jump", str_jump},
            {"Reset to Defaults", ""}};

    // setup scrolling cursor
    size_t	gamerules_shown;
    gamerules_shown = sizeof(rule_titles)/sizeof(rule_titles[0]);
    if (gamerules_shown > vertical_gamerules_affordance)
        gamerules_shown = vertical_gamerules_affordance;
    if (gamerules_first+gamerules_shown-1 < gamerules_cursor)
        gamerules_first = gamerules_cursor-(gamerules_shown-1);
    if (gamerules_first > gamerules_cursor)
        gamerules_first = gamerules_cursor;

    // r101: dim shade behind the menu (user) -- panels themselves are opaque
    Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 0.7f);

    // window
    struct MenuCoords coords = Draw_WindowGrid("Custom Game Rules", vertical_gamerules_affordance, MVS_P, 2, 0.35, 1.0, gamerules_cursor-gamerules_first, true);
    // footer
    switch (gamerules_cursor) {
        case 12:
            M_Draw_Hint(SELECT_RULES, coords);
            break;
        default:
            M_Draw_Hint(SCROLL_RULES, coords);
    }

    // draw rules
    for (int n = 0; n < gamerules_shown; n++) {
        M_PrintWhite (coords.grid[0][n].xp, coords.grid[0][n].yp, rule_titles[gamerules_first+n].name);        M_Print (coords.grid[1][n].xp, coords.grid[1][n].yp, rule_titles[gamerules_first+n].value);
    }

    // perf B7: left gutter scrollbar (Options treatment)
    {
        int	rowh = coords.grid[1][1].y - coords.grid[1][0].y;
        if (rowh <= 0)
            rowh = MVS;
        M_Draw_Scrollbar (&coords, rowh, (int)(sizeof(rule_titles)/sizeof(rule_titles[0])), gamerules_first, (int)gamerules_shown);
    }
}

void M_Matchmaking_Gamerules_Submenu_Key (int dir) {
    switch (gamerules_cursor)
    {
        case 0: //score limit (r77: moved here from the main matchmaking page)
            opts.fraglimit += dir*5;
            if (opts.fraglimit > 100)
                opts.fraglimit = 0;
            if (opts.fraglimit < 0)
                opts.fraglimit = 100;
            break;
        case 1: //time limit
            opts.timelimit += dir*5;
            if (opts.timelimit > 60)
                opts.timelimit = 0;
            if (opts.timelimit < 0)
                opts.timelimit = 60;
            break;
        case 2: //custom primary
            opts.custom_primary += dir;
            if (opts.custom_primary >= sizeof(gamerule_weapons)/sizeof(gamerule_weapons[0]))
                opts.custom_primary = 0;
            if (opts.custom_primary < 0 && dir < 0)
                opts.custom_primary = sizeof(gamerule_weapons)/sizeof(gamerule_weapons[0])-1;
            break;
        case 3: //custom secondary
//            opts.custom_secondary += dir; todo: fix custom secondary QuakeC code
            if (opts.custom_secondary >= sizeof(gamerule_weapons_secondary)/sizeof(gamerule_weapons_secondary[0]))
                opts.custom_secondary = 0;
            if (opts.custom_secondary < 0 && dir < 0)
                opts.custom_secondary = sizeof(gamerule_weapons_secondary)/sizeof(gamerule_weapons_secondary[0])-1;
            break;
        case 4: //ground weapons
            opts.map_weapons_disable += dir;
            if (opts.map_weapons_disable > 1)
                opts.map_weapons_disable = 0;
            if (opts.map_weapons_disable < 0)
                opts.map_weapons_disable = 1;
            break;
        case 5: //difficulty
            opts.skill += dir;
            if (opts.skill > 3)
                opts.skill = 0;
            if (opts.skill < 0)
                opts.skill = 3;
            break;
        case 6: //max grenades
            opts.max_nades += dir;
            if (opts.max_nades > 8)
                opts.max_nades = 0;
            if (opts.max_nades < 0)
                opts.max_nades = 8;
            break;
        case 7: //infinite ammo
            opts.infinite_ammo += dir;
            if (opts.infinite_ammo > 1)
                opts.infinite_ammo = 0;
            if (opts.infinite_ammo < 0)
                opts.infinite_ammo = 1;
            break;
        case 8: //grunt birthday
            opts.grunt_birthday_party += dir;
            if (opts.grunt_birthday_party > 1)
                opts.grunt_birthday_party = 0;
            if (opts.grunt_birthday_party < 0)
                opts.grunt_birthday_party = 1;
            break;
        case 9: //gravity
            opts.sol_gravity += dir*10;
            if (opts.sol_gravity > 500)
                opts.sol_gravity = 100;
            if (opts.sol_gravity < 100)
                opts.sol_gravity = 500;
            break;
        case 10: //max speed
            opts.sol_maxspeed += dir*10;
            if (opts.sol_maxspeed > 500)
                opts.sol_maxspeed = 100;
            if (opts.sol_maxspeed < 100)
                opts.sol_maxspeed = 500;
            break;
        case 11: //jump
            opts.sol_jump += dir*10;
            if (opts.sol_jump > 500)
                opts.sol_jump = 100;
            if (opts.sol_jump < 100)
                opts.sol_jump = 500;
            break;
    }
}
void M_Matchmaking_Gamerules_Key (int key) {
    switch (key)
    {
        case K_YBUTTON: // triangle
            resetGameRules();
            break;
#ifdef VITA
        case K_ENTER:
#endif
        case K_ESCAPE:
        case K_BBUTTON: // circle
        case 'n':
        case 'N':
            if (opts.custom_primary == opts.custom_secondary) {
                // do not allow two of the same weapons
                if (opts.custom_secondary == sizeof(gamerule_weapons)/sizeof(gamerule_weapons[0])-1) {
                    opts.custom_secondary = 0;
                } else {
                    opts.custom_secondary++;
                }
            } else {
                m_state = m_gameoptions;
                m_entersound = true;
                cursorfocused = true;
            }
            break;
        case K_ABUTTON: // x
        case 'y':
        case 'Y':
#ifndef VITA
        case K_ENTER:
        case K_KP_ENTER:
            /* Mac/PC port: Enter selects, like the Vita X button. */
#endif
            switch (gamerules_cursor) {
                case 12:
                {
                    resetGameRules();
                    break;
                }
            }
//            cursorfocused = true;
            S_LocalSound ("misc/menuoption.wav");
            break;
        case K_UPARROW:
            S_LocalSound ("misc/menuoption.wav");
            gamerules_cursor--;
            if (gamerules_cursor >= rule_list_size)
                gamerules_cursor = 12;
            break;
        case K_DOWNARROW:
            S_LocalSound ("misc/menuoption.wav");
            gamerules_cursor++;
            if (gamerules_cursor >= rule_list_size)
                gamerules_cursor = 0;
            break;
        case K_LSHOULDER:	// perf fix: Vita L/R report as shoulders, not triggers
        case K_LTRIGGER:
        case K_LEFTARROW:
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Gamerules_Submenu_Key(-1);
            break;
        case K_RSHOULDER:	// perf fix: Vita L/R report as shoulders, not triggers
        case K_RTRIGGER:
        case K_RIGHTARROW:
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Gamerules_Submenu_Key(1);
            break;

        default:
            break;
    }
}

//=============================================================================
/* perf B7 - PERFORMANCE MENU: presets + direct control of the B1-B6 cvars
 * and the classic render knobs. One scrolling page, styled slider widget. */

enum
{
	PERF_PRESET_ROWS = 1,	// one cycling row: Preset [Custom/Quality/Balanced/Performance]
	PERF_KNOB_ROWS   = 17,
	PERF_ROWS        = PERF_PRESET_ROWS + PERF_KNOB_ROWS
};

static char	*perf_preset_names[4] = {"Custom", "Quality", "Balanced", "Performance"};
static float	perf_preset_color[4][3] = {	// Custom gray, Quality blue, Balanced gold, Performance orange
	{0.60f, 0.62f, 0.68f},
	{0.40f, 0.65f, 1.00f},
	{0.92f, 0.75f, 0.30f},
	{1.00f, 0.50f, 0.25f},
};

typedef struct
{
	const char	*name;
	cvar_t		*cvar;
	float		minval, maxval, step;
	const char	*suffix;	// appended to numeric values
	int			scale100;	// stored 0..1, shown as percent
	int			zeroOff;	// show "Off" at 0
	int			zeroUnltd;	// show "Unlimited" at 0
	const char	*desc;		// shown under the panel for the cursor row
} perf_knob_t;

// rows 0..11 are set by the presets (order matches perf_preset_vals below);
// rows 12..16 are passthrough knobs the presets leave alone.
static perf_knob_t	perf_knobs[PERF_KNOB_ROWS] =
{
	{"Perf HUD",         &r_perfhud,         0, 2,    1,    "",  0, 0, 0, "Charts frame times (1) or load vs fps (2). Impact: none."},
	{"Cull Lights",      &r_dlight_cull,     0, 1,    1,    "",  0, 0, 0, "Ignores light flashes hidden by walls. Impact: high."},
	{"Max Lights",       &r_max_dlights,     0, 32,   4,    "",  0, 0, 1, "How many lights from gunfire and blasts. Impact: med."},
	{"Light Cache",      &r_light_cache,     0, 4,    1,    "",  0, 1, 0, "Lighting redraws every few frames. Impact: med."},
	{"Far Detail",       &r_subpixel_cull,   0, 8,    1,    " px", 0, 1, 0, "Tiny far-off wall details are skipped. Impact: med."},
	{"Big Particles",    &r_part_maxrad,     0, 128,  8,    " px", 0, 1, 0, "Huge particle clouds are skipped. Impact: med."},
	{"Particles",        &r_particles,       0, 1,    0.25, "%", 1, 0, 0, "How much smoke, sparks and blood you see. Impact: high."},
	{"Blood & Debris",   &sv_particle_scale, 0, 100,  25,   "%", 0, 1, 0, "Blood and debris from multiplayer kills. Impact: high."},
	{"Gib Cap",          &sv_max_gibs,       0, 32,   4,    "",  0, 1, 0, "Body parts flying at once. Impact: med."},
	{"Bot Think Hz",     &sv_bot_thinkrate,  0, 20,   5,    " Hz", 0, 1, 0, "How often bots make decisions. Impact: high."},
	{"Needle FX",        &sv_max_needles,    0, 64,   8,    "",  0, 1, 0, "Thins needle sparkle bursts. Never blocks firing. Impact: med."},
	{"Far Smoothing",    &r_lerpskip_dist,   0, 2400, 400,  "",  0, 1, 0, "Distant players move in steps, not smoothly. Impact: low."},
	{"Dynamic Lights",   &r_dynamic,         0, 1,    1,    "",  0, 0, 0, "Muzzle flashes light up the room. Impact: high."},
	{"Simple Flashes",   &gl_flashblend,     0, 1,    1,    "",  0, 0, 0, "Flashes drawn as simple colored glows. Impact: med."},
	{"Shadows",          &r_shadows,         0, 1,    1,    "",  0, 0, 0, "Shadow blobs under players and items. Impact: low."},
	{"Smooth Anim",      &r_lerpmodels,      0, 1,    1,    "",  0, 0, 0, "Smooths character animation between frames. Impact: low."},
	{"3D Resolution",    &r_renderscale,     0.25, 1, 0.25, "%", 1, 0, 0, "Draws the world smaller, then stretches it. Impact: high."},
};

static const char	*perf_preset_desc[4] =
{
	"Each option below is set by hand.",
	"Best looks, lowest framerate.",
	"Small limits on lights only, looks nearly the same.",
	"Big limits on effects and bots, plays smoothest.",
};

// Quality: every B1-B6 limiter off (pure stock behaviour), max fidelity.
// Balanced: safe caps only -- no bot throttle; needle row is FX-only (r103).
// Performance: aggressive caps for the Vita.
// Columns track the first 12 knobs above (Perf HUD first, then B1-B6 caps).
static const float	perf_preset_vals[3][12] =
{
	{0, 1, 0,  0, 0, 0,  1,    100, 0,  0,  0,    0},	// Quality
	{0, 1, 16, 2, 2, 64, 1,     75, 12,  0,  0, 1600},	// Balanced
	{0, 1, 8,  3, 4, 32, 0.5f,  50, 8,  10, 16, 1000},	// Performance
};

size_t		perf_cursor;
size_t		perf_first;
float		vertical_perf_affordance = 11;	// perf B7: 10 data rows + footnote band
static enum m_state_e	perf_return_state = m_main;	// page we came from (options / pause / main)

// perf B9b (user): Custom remembers your mix. The 12 preset-controlled knobs
// are mirrored into an archived backup string (value*100, every value fits
// exactly) whenever you tweak one or make the first preset jump, so cycling
// the Preset row away by mistake and back to Custom restores what you built.
static void PERF_Custom_Store (void)
{
	char	sbuf[192];
	int		n = 0, i;

	for (i = 0; i < 12; i++)
		n += q_snprintf (sbuf + n, sizeof(sbuf) - n, "%s%i",
				i ? " " : "", (int)(perf_knobs[i].cvar->value * 100.0f + 0.5f));
	Cvar_Set ("perf_custom_backup", sbuf);
}

static qboolean PERF_Custom_Restore (void)
{
	int	vals[12];
	int	i, got;

	if (!perf_custom_backup.string[0])
		return false;
	got = sscanf (perf_custom_backup.string,
			"%i %i %i %i %i %i %i %i %i %i %i %i",
			&vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5],
			&vals[6], &vals[7], &vals[8], &vals[9], &vals[10], &vals[11]);
	if (got != 12)
		return false;	// stale/garbled backup: leave the knobs alone
	for (i = 0; i < 12; i++)
		Cvar_SetValueQuick (perf_knobs[i].cvar, vals[i] / 100.0f);
	return true;
}

static void M_Performance_ApplyPreset (int p)	// 1..3
{
	int	i;
	if (!perf_custom_backup.string[0])	// first preset jump: keep what we are about to overwrite
		PERF_Custom_Store ();
	for (i = 0; i < 12; i++)
		Cvar_SetValueQuick (perf_knobs[i].cvar, perf_preset_vals[p-1][i]);
	Cvar_SetValueQuick (&perf_preset, p);
	Host_WriteConfiguration ();	// persist immediately -- user picked a whole profile
}

static void M_Performance_Step (int dir)
{
	perf_knob_t	*k;
	float		v;
	int			p;
	if (perf_cursor < PERF_PRESET_ROWS)
	{	// preset row cycles Custom -> Quality -> Balanced -> Performance
		p = (int)perf_preset.value + dir;
		if (p > 3)
			p = 0;
		if (p < 0)
			p = 3;
		if (p > 0)
		{
			M_Performance_ApplyPreset (p);	// selecting a preset applies it at once
			S_LocalSound ("misc/menu2.wav");	// confirm sound: values just moved
		}
		else
		{	// perf B9b: Custom restores the remembered mix when there is one
			if (PERF_Custom_Restore ())
			{
				S_LocalSound ("misc/menu2.wav");	// values just moved
				Host_WriteConfiguration ();		// mirror ApplyPreset's immediate persist
			}
			Cvar_SetValueQuick (&perf_preset, 0);	// no backup yet: knobs stay where they are
		}
		return;
	}
	k = &perf_knobs[perf_cursor - PERF_PRESET_ROWS];
	v = k->cvar->value + dir * k->step;
	if (v > k->maxval + 0.001f)
		v = k->minval;	// wrap: fastest travel to the other end
	if (v < k->minval - 0.001f)
		v = k->maxval;
	Cvar_SetValueQuick (k->cvar, v);
	if (perf_preset.value)	// manual edit = custom mix
		Cvar_SetValueQuick (&perf_preset, 0);
	PERF_Custom_Store ();	// perf B9b: mirror the mix (includes this knob's new value)
}

static const char *M_Performance_Value (int i)
{
	const perf_knob_t	*k = &perf_knobs[i];
	float	v = k->cvar->value;
	if (k->maxval <= 1.0f && !k->scale100)	// percent knobs (scale100) must fall through
		return v ? "On" : "Off";
	if (k->zeroUnltd && v == 0)
		return "Unlimited";
	if (k->zeroOff && v == 0)
		return "Off";
	if (k->scale100)
		return va("%i%s", (int)(v * 100.0f + 0.5f), k->suffix);
	return va("%i%s", (int)(v + 0.5f), k->suffix);
}

// styled slider: dark track, blue fill, gold handle -- Draw_FillRGBA quads.
static void M_Draw_PerfSlider (struct MenuCoords *mc, int row, int rowh, float frac, const char *value, int toggle)
{
	int		bary = mc->grid[1][row].y + (rowh - 10) / 2;	// band top
	int		x, barw, hw, hx, tx;
	if (frac < 0) frac = 0;
	if (frac > 1) frac = 1;
	if (toggle)
	{	// compact pill, clearly different from value sliders
		x = mc->grid[1][row].x + 12;
		barw = 56; hw = 22;
		Draw_FillRGBA (x, bary + 2, barw, 6, 0.16f, 0.18f, 0.26f, 1.0f);
		Draw_FillRGBA (x - 3, bary, 2, 10, 0.70f, 0.73f, 0.80f, 1.0f);
		Draw_FillRGBA (x + barw + 1, bary, 2, 10, 0.70f, 0.73f, 0.80f, 1.0f);
		hx = (frac >= 0.999f) ? x + barw - hw : x;
		Draw_FillRGBA (hx, bary, hw, 10,
			frac >= 0.999f ? 0.30f : 0.42f, frac >= 0.999f ? 0.55f : 0.44f, frac >= 0.999f ? 1.0f : 0.50f, 1.0f);
	}
	else
	{	// uniform-width value slider
		x = mc->grid[1][row].x + 12;
		barw = (int)(mc->colw * 0.58f); hw = 10;
		Draw_FillRGBA (x, bary + 2, barw, 6, 0.36f, 0.39f, 0.46f, 1.0f);	// solid brighter grey
		if (frac > 0)
			Draw_FillRGBA (x, bary + 2, (int)(barw * frac), 6, 0.22f, 0.42f, 0.75f, 1.0f);
		// edge ticks: gold when the handle is pinned at that end
		Draw_FillRGBA (x - 3, bary, 2, 10, frac <= 0.001f ? 1.0f : 0.70f, frac <= 0.001f ? 0.80f : 0.73f, frac <= 0.001f ? 0.30f : 0.80f, 1.0f);
		Draw_FillRGBA (x + barw + 1, bary, 2, 10, frac >= 0.999f ? 1.0f : 0.70f, frac >= 0.999f ? 0.80f : 0.73f, frac >= 0.999f ? 0.30f : 0.80f, 1.0f);
		hx = x + (int)((barw - hw) * frac);
		Draw_FillRGBA (hx, bary, hw, 10, 0.85f, 0.68f, 0.25f, 1.0f);
	}
	tx = mc->grid[1][row].x + mc->colw - (int)Draw_TTFWidth (value, 14.0f) - 12;
	M_PrintWhite (tx, mc->grid[1][row].yp, value);
}

void M_Menu_Performance_f (void)
{
	// unified Options menu (perf B7): perfmenu now lands on the tabbed menu's
	// Performance tab instead of the old standalone page.
	M_Menu_Options_f ();
}

void M_Menu_Editor_f (void)
{
	//r101: editmenu lands on the tabbed Options menu's Editor tab. Opening
	//it is an explicit request for the editor context, so the runtime
	//switch comes on too (keeps the tab visible while inside).
	Cvar_SetValue ("editor_mode", 1);	// makes the tab visible, then land on it
	M_Menu_Options_f ();
	M_Options_SetTab (UNI_TAB_EDIT);
}

void M_Performance_Draw (void)
{
	size_t			n, shown;
	enum m_state_e	saved;
	int				rowh;
	struct MenuCoords	coords;

	// recursive background = the page we came from
	saved = m_state;
	m_state = perf_return_state;
	m_recursiveDraw = true;
	M_Draw ();
	m_state = saved;

	// background tint
	Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 0.7f);

	// scrolling window setup (gamerules pattern); last grid slot = description band
	shown = PERF_ROWS;
	if (shown > (size_t)vertical_perf_affordance - 1)
		shown = (size_t)vertical_perf_affordance - 1;
	if (perf_first + shown - 1 < perf_cursor)
		perf_first = perf_cursor - (shown - 1);
	if (perf_first > perf_cursor)
		perf_first = perf_cursor;

	coords = Draw_WindowGrid("Performance", vertical_perf_affordance, MVS_P, 2, 0.4, 1.0, (int)(perf_cursor - perf_first), true);
	M_Draw_Hint(PERF, coords);

	rowh = coords.grid[1][1].y - coords.grid[1][0].y;
	if (rowh <= 0)
		rowh = MVS;

	M_Performance_Body (&coords, rowh, shown);
}

static void UNI_ActionDefaults (int dir);	//defined with the General tab rows; the triangle handlers use it too

void M_Performance_Key (int key)
{
	switch (key)
	{
	case K_YBUTTON: // triangle -- restore defaults (confirm modal, like the rules page "Defaults")
		S_LocalSound ("misc/menu2.wav");
		UNI_ActionDefaults (0);
		break;	//stay on the page: sliders read the cvars live, no refresh needed

	case K_XBUTTON: // square -- save the shown values as the Custom mix
#ifndef VITA
	case 'g':	// Mac alias for square (matchmaking submenu precedent)
	case 'G':
#endif
		S_LocalSound ("misc/menu2.wav");
		PERF_Custom_Store ();
		Host_WriteConfiguration ();	// archive now; do not wait for the page to close
		break;

	case K_ESCAPE:
	case K_BBUTTON: // circle
#ifdef VITA
	case K_ENTER:
#endif
		if (perf_return_state != m_options)
			Host_WriteConfiguration ();	// console entry point: persist before leaving the menu flow
		if (perf_return_state == m_options)
		{
			m_state = m_options;
			m_entersound = true;
		}
		else if (perf_return_state == m_pause)
			M_Menu_Pause_f ();
		else
			M_Menu_Main_f ();
		break;

	case K_ABUTTON: // x
	case 'y':
	case 'Y':
#ifndef VITA
	case K_ENTER:
	case K_KP_ENTER:
		/* Mac/PC port: Enter selects, like the Vita X button. */
#endif
		if (perf_cursor < PERF_PRESET_ROWS && (int)perf_preset.value > 0)
		{	// re-apply the current preset (undoes manual tweaks)
			S_LocalSound ("misc/menu2.wav");
			M_Performance_ApplyPreset ((int)perf_preset.value);
		}
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		perf_cursor--;
		if (perf_cursor >= PERF_ROWS)
			perf_cursor = PERF_ROWS - 1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		perf_cursor++;
		if (perf_cursor >= PERF_ROWS)
			perf_cursor = 0;
		break;

	// NOTE: L/R triggers deliberately NOT bound here (perf B7 design:
	// triggers are reserved for page changes; arrows step the values).
	case K_LEFTARROW:
		S_LocalSound ("misc/menu3.wav");
		M_Performance_Step (-1);
		break;

	case K_RIGHTARROW:
		S_LocalSound ("misc/menu3.wav");
		M_Performance_Step (1);
		break;

	default:
		break;
	}
}

//=============================================================================
// unified Options menu (perf B7): tabbed Options; Performance tab reuses the
// exact perf rows via M_Performance_Body. Rows bind EXISTING cvars (name-string
// Cvar_SetValue is not used here: Cvar_SetValueQuick takes the pointer).

typedef struct
{
	const char	*label;
	cvar_t		*cvar;			// NULL for action/text rows
	float		 minval, maxval, stepval;
	int			 percent;		// display value*100 with a %% suffix
	int			 noslider;		// right-aligned text only, no track
	void		 (*set)(int dir);	// custom adjuster (dir==0 on select)
	const char	*(*show)(char *buf, int size);
	const char	*desc;
} uni_row_t;

static void UNI_SetScale (int dir)
{
	float v = scr_menuscale.value + dir * 0.1f;
	if (v < 1.0f)
		v = 2.5f;
	if (v > 2.5f)
		v = 1.0f;
	Cvar_SetValueQuick (&scr_menuscale, v);
	Cvar_SetValueQuick (&scr_conscale, v);
	Cvar_SetValueQuick (&scr_sbarscale, v);
}

static const char *UNI_ShowScale (char *buf, int size)
{
	q_snprintf (buf, size, "%i%%", (int)(scr_menuscale.value * 100.0f + 0.5f));
	return buf;
}

/* r122 (user): Show FPS is three-way -- Off / Native (the engine's own
 * bottom-right counter) / New (our top-right chip). */
static const char *UNI_ShowFps (char *buf, int size)
{
	int v = (int)(hud_fps.value + 0.5f);
	q_snprintf (buf, size, "%s", v <= 0 ? "Off" : (v == 1 ? "Native" : "New"));
	return buf;
}

static const char *UNI_ShowSens (char *buf, int size)
{
	q_snprintf (buf, size, "%g", sensitivity.value);
	return buf;
}

static const char *UNI_ShowTexMode (char *buf, int size)
{
	q_strlcpy (buf, strstr (gl_texturemode.string, "NEAREST") ? "Nearest" : "Smooth", size);
	return buf;
}

static void UNI_SetTexMode (int dir)
{
	(void)dir;
	if (strstr (gl_texturemode.string, "NEAREST"))
		Cbuf_AddText ("gl_texturemode GL_LINEAR_MIPMAP_LINEAR\n");
	else
		Cbuf_AddText ("gl_texturemode GL_NEAREST_MIPMAP_LINEAR\n");
}

static void UNI_SetInvert (int dir)
{
	(void)dir;
	Cvar_SetValueQuick (&m_pitch, -m_pitch.value);
}

static const char *UNI_ShowInvert (char *buf, int size)
{
	q_strlcpy (buf, m_pitch.value < 0 ? "On" : "Off", size);
	return buf;
}

static void UNI_SetMlook (int dir)
{
	(void)dir;
	Cbuf_AddText ((in_mlook.state & 1) ? "-mlook\n" : "+mlook\n");
}

static const char *UNI_ShowMlook (char *buf, int size)
{
	q_strlcpy (buf, (in_mlook.state & 1) ? "On" : "Off", size);
	return buf;
}

static void UNI_ActionKeys (int dir)
{
	(void)dir;
	M_Menu_Keys_f ();
}

/* r112: LB Melee (Options > Controls). dir==0 (X/Enter) flips; L/R set
 * explicitly. cvar defined gl_hud.c; QC reads it by name in weapons.qc. */
static void UNI_SetLbMelee (int dir)
{
	if (dir == 0)
		Cvar_SetValueQuick (&lb_melee, lb_melee.value > 0 ? 0.0f : 1.0f);
	else
		Cvar_SetValueQuick (&lb_melee, dir > 0 ? 1.0f : 0.0f);
}

static void UNI_ActionConsole (int dir)
{
	(void)dir;
	m_state = m_none;
	Con_ToggleConsole_f ();
}

// Scoped default restore (user incident 2026-11-18): the old recipe ran
// resetcfg + exec default.cfg, which reset the ENTIRE cvar space (names,
// binds, everything archived) and the menu-close Host_WriteConfiguration
// then persisted the wreckage over config.cfg. This table covers ONLY what
// the Options pages can change, each with its built-in engine default (the
// documented shipping values; sensitivity per user is a menu default of 5).
typedef struct { cvar_t *cvar; float def; } uni_def_t;
static const uni_def_t uni_defaults[] =
{
	// Performance tab knobs (order mirrors perf_knobs)
	{ &r_perfhud,           0     },
	{ &r_dlight_cull,       1     },
	{ &r_max_dlights,       16    },
	{ &r_light_cache,       2     },
	{ &r_subpixel_cull,     2     },
	{ &r_part_maxrad,       64    },
	{ &r_particles,         1     },
	{ &sv_particle_scale,   75    },
	{ &sv_max_gibs,         12    },
	{ &sv_bot_thinkrate,    0     },
	{ &sv_max_needles,      0     },
	{ &r_lerpskip_dist,     1600  },
	{ &r_dynamic,           1     },
	{ &gl_flashblend,       0     },
	{ &r_shadows,           0     },
	{ &r_lerpmodels,        1     },
	{ &r_renderscale,       1     },
	{ &perf_preset,         2     },	// the defaults above ARE Balanced
	// Graphics tab
	{ &scr_viewsize,        100   },
	{ &scr_menuscale,       1.4f  },
	{ &vid_gamma,           1     },
#ifndef VITA
	{ &vid_contrast,        1     },
#endif
	{ &scr_fov,             75    },
	{ &cl_cambob,           0.1f  },
	{ &cl_gunbob,           0.2f  },
	{ &r_wateralpha,        1     },
	{ &visor_warp,          0.18f },
	{ &visor_zoom,          0.65f },
	{ &vid_vsync,           1     },
	{ &host_maxfps,         60    },
	{ &hide_hud,            0     },
	{ &hud_fps,             0     },
	// Controls tab
	{ &sensitivity,         5     },
	{ &m_pitch,             0.022f },
	{ &lookspring,          0     },
	{ &lb_melee,            1     },
#ifdef VITA
	{ &pstv_rumble,         1     },
	{ &retrotouch,          0     },
	{ &motioncam,           0     },
	{ &motion_horizontal_sensitivity, 0 },
	{ &motion_vertical_sensitivity,   0 },
#endif
	// Sound tab
	{ &sfxvolume,           0.7f  },
	{ &bgmvolume,           1     },
};

static void UNI_ActionDefaults (int dir)
{
	size_t	i;
	(void)dir;
	if (SCR_ModalMessage("This will restore the default\n"
			"values of every menu setting.\n"
			"Binds and saves are untouched.\n"
			"Continue? (y/n)\n", 15.0f))
	{
		for (i = 0; i < sizeof(uni_defaults)/sizeof(uni_defaults[0]); i++)
			Cvar_SetValueQuick (uni_defaults[i].cvar, uni_defaults[i].def);
		PERF_Custom_Store ();	// perf B9b: defaults are the fresh custom mix
	}
}

//r101 Editor tab: cheat rows exec the engine command (it toggles and prints
//its own ON/OFF); the state column mirrors Host_CheatState (-1 = n/a, no
//live player). Rows deliberately ignore dir so arrows and select all toggle,
//matching the Texture Filter / Mouse Look pill pattern.
static void UNI_SetGod (int dir)      { (void)dir; Cbuf_AddText ("god\n"); }
static void UNI_SetNotarget (int dir) { (void)dir; Cbuf_AddText ("notarget\n"); }
static void UNI_SetFly (int dir)      { (void)dir; Cbuf_AddText ("fly\n"); }
static void UNI_SetNoclip (int dir)   { (void)dir; Cbuf_AddText ("noclip\n"); }
static const char *UNI_CheatShow (char *buf, int size, int which)
{
	int s = Host_CheatState (which);
	q_strlcpy (buf, s < 0 ? "n/a" : (s > 0 ? "On" : "Off"), size);
	return buf;
}
static const char *UNI_ShowGod (char *buf, int size)       { return UNI_CheatShow (buf, size, 0); }
static const char *UNI_ShowNotarget (char *buf, int size)  { return UNI_CheatShow (buf, size, 1); }
static const char *UNI_ShowFly (char *buf, int size)       { return UNI_CheatShow (buf, size, 2); }
static const char *UNI_ShowNoclip (char *buf, int size)    { return UNI_CheatShow (buf, size, 3); }
static void UNI_ActionReview (int dir)
{
	(void)dir;
	//review re-launches the current/last map with editor_mode + queued
	//cheats; drop out of the menu first so the map takes over cleanly.
	m_state = m_none;
	Cbuf_AddText ("review\n");
}

static const uni_row_t uni_gfx_rows[] =
{
	{"Screen Size",     &scr_viewsize,  30, 120, 10,   0, 0, NULL, NULL, "Size of the 3D view in the window. Impact: none."},
	{"Hide HUD",        &hide_hud,         0,   1, 1,   0, 0, NULL, NULL, "Hide the combat overlay for clean screenshots. Impact: none."},
	{"Show FPS",        &hud_fps,          0,   2, 1,   0, 0, NULL, UNI_ShowFps, "Framerate readout: Off, Native (engine counter) or New (top-right chip). Impact: none."},
	{"Max FPS",        &host_maxfps,   30,  60,  5,   0, 0, NULL, NULL, "Frame rate cap in steps of 5. Impact: none."},
	{"UI Scale",        &scr_menuscale,    1, 2.5, 0.1,  0, 0, UNI_SetScale, UNI_ShowScale, "Size of menus and status text. Impact: none."},
	{"Brightness",      &vid_gamma,   0.5,   1, 0.05,  1, 0, NULL, NULL, "Overall screen lightness. Impact: none."},
#ifndef VITA
	{"Contrast",        &vid_contrast,   1,   2, 0.1,  1, 0, NULL, NULL, "Difference between dark and bright. Impact: none."},
#endif
	{"Field of View",   &scr_fov,       75, 130, 5,    0, 0, NULL, NULL, "How wide your view is. Impact: none."},
	{"View Bob",        &cl_cambob,      0, 0.4, 0.05, 1, 0, NULL, NULL, "Camera sway while walking. Zero keeps the view rock steady. Impact: none."},
	{"Weapon Bob",      &cl_gunbob,      0, 0.5, 0.05, 1, 0, NULL, NULL, "Rhythm your gun carries with your steps. Impact: none."},
	{"Water Opacity",   &r_wateralpha,   0,   1, 0.1,  1, 0, NULL, NULL, "How clear the water looks. Impact: low."},
	{"Visor Warp",      &visor_warp,     0, 0.25, 0.01, 1, 0, NULL, NULL, "Curves the HUD onto the visor. Impact: none."},
	{"Visor Zoom",      &visor_zoom,     0, 1, 0.05, 1, 0, NULL, NULL, "Pulls the warped HUD back into view. Impact: none."},
	{"Texture Filter",  NULL,            0,   1, 1,    0, 1, UNI_SetTexMode, UNI_ShowTexMode, "Smooth vs crisp textures. Impact: low."},
	{"Vertical Sync",   &vid_vsync,      0,   1, 1,    0, 0, NULL, NULL, "Match the display refresh, no tearing. Impact: none."},
};

static const uni_row_t uni_ctrl_rows[] =
{
	{"Mouse Speed",    &sensitivity, 1, 11, 1, 0, 0, NULL, UNI_ShowSens, "Look speed with mouse or sticks. Impact: none."},
	{"Invert Mouse",   &m_pitch,    -1,  1, 1, 0, 1, UNI_SetInvert, UNI_ShowInvert, "Flip vertical look direction."},
	{"Mouse Look",     NULL,         0,  1, 1, 0, 1, UNI_SetMlook, UNI_ShowMlook, "Mouse turns the player (vs strafing)."},
	{"Lookspring",     &lookspring,  0,  1, 1, 0, 0, NULL, NULL, "View drifts back to center after looking. Impact: none."},
	{"LB Melee",       &lb_melee,    0,  1, 1, 0, 0, UNI_SetLbMelee, NULL, "With an enemy close, LB swings your weapon instead of throwing a grenade. Impact: none."},
#ifdef VITA
	{"Rumble",              &pstv_rumble, 0, 1, 1, 0, 0, NULL, NULL, "Controller vibration on fire and hits."},
	{"Retro Touch",         &retrotouch,  0, 1, 1, 0, 0, NULL, NULL, "Old-style touchscreen aiming."},
	{"Gyroscope Aim",       &motioncam,   0, 1, 1, 0, 0, NULL, NULL, "Look by tilting the system."},
	{"Gyro Speed X",  &motion_horizontal_sensitivity, 0, 10, 1, 0, 0, NULL, NULL, "Tilt sensitivity left/right."},
	{"Gyro Speed Y",  &motion_vertical_sensitivity,   0, 10, 1, 0, 0, NULL, NULL, "Tilt sensitivity up/down."},
#endif
	{"Key Bindings",   NULL,         0,  1, 1, 0, 1, UNI_ActionKeys, NULL, "Rebind buttons on the legacy keys page."},
};

static const uni_row_t uni_snd_rows[] =
{
	{"Sound Volume",   &sfxvolume,   0, 1, 0.05, 1, 0, NULL, NULL, "Effects and weapons volume. Impact: none."},
	{"Music Volume",   &bgmvolume,   0, 1, 0.05, 1, 0, NULL, NULL, "Background music volume. Impact: none."},
};

static const uni_row_t uni_gen_rows[] =
{
	{"Goto Console",   NULL, 0, 1, 1, 0, 1, UNI_ActionConsole, NULL, "Open the developer console."},
	{"Restore Defaults", NULL, 0, 1, 1, 0, 1, UNI_ActionDefaults, NULL, "Restore the default value of every menu setting."},
};

//r101 Editor tab (console: editmenu, or the review command): cheat toggles +
//map debug draws. editor_mode/hud_mapshot defined gl_hud.c (externs gl_hud.h).
static const uni_row_t uni_edit_rows[] =
{
	{"Editor Mode",   &editor_mode,  0, 1, 1, 0, 0, NULL, NULL, "Unlocks cheats and debug draws outside -editor. Session-only, never saved."},
	{"God Mode",      NULL,          0, 1, 1, 0, 1, UNI_SetGod, UNI_ShowGod, "Cannot be hurt. Toggles."},
	{"Notarget",      NULL,          0, 1, 1, 0, 1, UNI_SetNotarget, UNI_ShowNotarget, "Enemies ignore you. Toggles."},
	{"Fly",           NULL,          0, 1, 1, 0, 1, UNI_SetFly, UNI_ShowFly, "Free flight. Toggles."},
	{"Noclip",        NULL,          0, 1, 1, 0, 1, UNI_SetNoclip, UNI_ShowNoclip, "Walk through walls. Toggles."},
	{"Noclip Speed",  &sv_noclip_speed, 0, 8000, 100, 0, 0, NULL, NULL, "Fly speed while noclip is on. 0 = normal run cap. Impact: none."},
	{"Hide HUD",      &hud_mapshot,  0, 1, 1, 0, 0, NULL, NULL, "Photo mode: hides the HUD and bots, unlocks noclip/fly."},
	{"Reload Map",    NULL,          0, 1, 1, 0, 1, UNI_ActionReview, NULL, "Re-launch the current map with editor cheats."},
	{"Show Tris",     &r_showtris,   0, 2, 1, 0, 0, NULL, NULL, "1 = lines through walls (hidden too); 2 = visible faces only. Costs a second pass."},
	{"Draw Flat",     &r_drawflat,   0, 1, 1, 0, 0, NULL, NULL, "Untextured faces, one color each -- spot mis-painted textures. Runs faster."},
	{"Lightmap View", &r_lightmap,   0, 1, 1, 0, 0, NULL, NULL, "Raw light data in color -- check density and bleeds. Big lightmaps cost VRAM."},
	{"Fullbright",    &r_fullbright, 0, 1, 1, 0, 0, NULL, NULL, "No lighting at all -- see textures true-color. Authoring view, not a perf switch."},
	{"No Vis",        &r_novis,      0, 1, 1, 0, 0, NULL, NULL, "Bypass PVS: see worst-case overdraw. Watch Speeds to gauge what vis saves you."},
	{"Speeds",        &r_speeds,     0, 2, 1, 0, 0, NULL, NULL, "On-screen poly/texture counts per frame (2 = per-pass). Measures cost, adds none."},
};

static const uni_row_t	*uni_tab_rows[UNI_TABS] =
{ uni_edit_rows, NULL, uni_gfx_rows, uni_ctrl_rows, uni_snd_rows, uni_gen_rows };
static const int uni_tab_nrows[UNI_TABS] =
{ (int)(sizeof(uni_edit_rows)  / sizeof(uni_edit_rows[0])), 0,
  (int)(sizeof(uni_gfx_rows)   / sizeof(uni_gfx_rows[0])),
  (int)(sizeof(uni_ctrl_rows)  / sizeof(uni_ctrl_rows[0])),
  (int)(sizeof(uni_snd_rows)   / sizeof(uni_snd_rows[0])),
  (int)(sizeof(uni_gen_rows)   / sizeof(uni_gen_rows[0])) };

//r101: Options remembers where you were -- last row per tab survives closing
//the menu; the first open of a session picks the tab (Editor if the editor
//context is live, else Performance).
static int		uni_tab_memory[UNI_TABS];
static qboolean	uni_options_opened;

static void M_Options_SetTab (int tab)
{
	int	nrows = (tab == UNI_TAB_PERF) ? PERF_ROWS : uni_tab_nrows[tab];
	int	row = uni_tab_memory[tab];

	uni_tab = tab;
	if (row >= nrows)
		row = nrows - 1;
	if (row < 0)
		row = 0;
	uni_tab_memory[tab] = row;
	if (tab == UNI_TAB_PERF)
	{
		perf_cursor = row;
		perf_first = 0;		// the draw pass scrolls the window to the cursor
	}
	else
	{
		uni_cursor = row;
		uni_first = 0;
	}
}

static void M_Options_OpenTab (void)
{
	if (!uni_options_opened)
	{	// first open of the session: Editor when the editor context is
		// live (review/-editor), Performance otherwise.
		uni_options_opened = true;
		M_Options_SetTab (UNI_EditorVisible () ? UNI_TAB_EDIT : UNI_TAB_PERF);
		return;
	}
	if (!UNI_TabVisible (uni_tab))
		M_Options_SetTab (UNI_TAB_PERF);	// the editor context went away
	else
		M_Options_SetTab (uni_tab);
}

static const char *UNI_Value (const uni_row_t *r, char *buf, int size)
{
	if (r->show)
		return r->show (buf, size);
	if (!r->cvar)
	{
		q_strlcpy (buf, "Open", size);
		return buf;
	}
	if (r->percent)
	{
		q_snprintf (buf, size, "%i%%", (int)(r->cvar->value * 100.0f + 0.5f));
		return buf;
	}
	if (r->maxval <= 1.0f)
	{
		q_strlcpy (buf, r->cvar->value > 0 ? "On" : "Off", size);
		return buf;
	}
	q_snprintf (buf, size, "%i", (int)(r->cvar->value + 0.001f));
	return buf;
}

static void UNI_Step (const uni_row_t *r, int dir)
{
	float v = r->cvar->value + dir * r->stepval;
	if (v < r->minval)
		v = r->maxval;
	if (v > r->maxval)
		v = r->minval;
	Cvar_SetValueQuick (r->cvar, v);
}

// thin scrollbar on the panel's right edge: shows rows hidden off-screen (perf B7)
static void M_Draw_Scrollbar (struct MenuCoords *coords, int rowh, int total, size_t first, int shown)
{
	int		top, trackh, ty, th, sx;
	if (total <= shown)
		return;
	top = coords->grid[0][0].y;
	trackh = rowh * shown;
	// left gutter INSIDE the panel, on a matching backing strip (user: never
	// float it over the page background). Grid text starts at x0+15, so a
	// 12-unit gutter never collides with rows or the gold cursor.
	Draw_FillRGBA (coords->x0, top, 12, trackh, 0.055f, 0.075f, 0.135f, 1.0f);
	sx = coords->x0 + 4;
	Draw_FillRGBA (sx, top, 4, trackh, 0.10f, 0.12f, 0.18f, 1.0f);
	ty = top + (int)((float)first / total * trackh);
	th = (int)((float)shown / total * trackh);
	if (th < 6)
		th = 6;
	if (ty + th > top + trackh)
		ty = top + trackh - th;
	Draw_FillRGBA (sx, ty, 4, th, 0.55f, 0.58f, 0.65f, 1.0f);
}

static void M_Performance_ResetWindow (void)
{
	perf_cursor = 0;
	perf_first = 0;
}

//r101: the Editor tab exists only inside an editing context -- editor_mode
//runtime switch (review/editmenu) or -editor at launch. Hidden elsewhere;
//tab cycles skip it.
static qboolean UNI_EditorVisible (void)
{
	return Cvar_VariableValue ("editor_mode") != 0 || COM_CheckParm ("-editor") != 0;
}

static qboolean UNI_TabVisible (int tab)
{
	return tab != UNI_TAB_EDIT || UNI_EditorVisible ();
}

static void UNI_ValidateTab (void)
{
	if (UNI_TabVisible (uni_tab))
		return;
	uni_tab_memory[uni_tab] = (uni_tab == UNI_TAB_PERF) ? (int)perf_cursor : uni_cursor;	//r101
	M_Options_SetTab (UNI_TAB_PERF);
}

static void M_Options_CycleTab (int dir)
{
	int	i;
	S_LocalSound ("misc/menuoption.wav");
	uni_tab_memory[uni_tab] = (uni_tab == UNI_TAB_PERF) ? (int)perf_cursor : uni_cursor;	//r101
	for (i = 0; i < UNI_TABS; i++)
	{
		uni_tab = (uni_tab + dir + UNI_TABS) % UNI_TABS;
		if (UNI_TabVisible (uni_tab))
			break;
	}
	M_Options_SetTab (uni_tab);	//r101: each tab reopens on its remembered row
}

// the Performance tab's rows + footnote band, shared by the old standalone
// page (legacy) and the unified menu. Identical pixels, edge-to-edge band fix.
static void M_Performance_Body (struct MenuCoords *coords, int rowh, size_t shown)
{
	size_t	n;

	for (n = 0; n < shown; n++)
	{
		size_t idx = perf_first + n;
		if (idx < PERF_PRESET_ROWS)
		{	// the single preset row: dark navy strip, colored accent bar +
			// oversized colored preset name (no brackets, no gold-on-gold)
			int		px = coords->grid[0][n].x;
			int		py = coords->grid[0][n].y;
			int		pw = coords->colw * coords->cols;
			int		p = (int)perf_preset.value;
			int		vw;
			float	cr = perf_preset_color[p][0], cg = perf_preset_color[p][1], cb = perf_preset_color[p][2];
			Draw_FillRGBA (px, py, pw, rowh, 0.055f, 0.075f, 0.135f, 1.0f);
			// caret (left accent bar) removed per user; the gold row cursor IS
			// wanted here -- re-strike it on top of our opaque strip, before text
			if ((int)(perf_cursor - perf_first) == (int)n)
				Draw_Cursor (px, py, coords->colw, rowh, true);
			Draw_TTFStringHeader (px + 14, coords->grid[0][n].yp + 8, "Preset", 1, 1, 1, 1.0f, 14.0f);
			vw = (int)Draw_TTFHeaderWidth (perf_preset_names[p], 14.0f);
			Draw_TTFStringHeader (px + pw - vw - 14, coords->grid[0][n].yp + 8, perf_preset_names[p], cr, cg, cb, 1.0f, 14.0f);
		}
		else
		{
			const perf_knob_t	*k = &perf_knobs[idx - PERF_PRESET_ROWS];
			float	frac = 0;
			if (k->maxval > k->minval)
				frac = (k->cvar->value - k->minval) / (k->maxval - k->minval);
			M_PrintWhite (coords->grid[0][n].xp, coords->grid[0][n].yp, k->name);
			M_Draw_PerfSlider (coords, (int)n, rowh, frac, M_Performance_Value((int)idx - PERF_PRESET_ROWS), k->maxval <= 1.0f && !k->scale100);
		}
	}

	// description for the row under the cursor, in its own band above the footer.
	// styled as a footnote (grey, smaller, indented) so it does not read as a row.
	// perf B7 (user): pinned to the bottom slot, not right under short tabs.
	{
		const char	*d = (perf_cursor < PERF_PRESET_ROWS)
			? perf_preset_desc[(int)perf_preset.value]
			: perf_knobs[perf_cursor - PERF_PRESET_ROWS].desc;
		int	bn = (int)vertical_perf_affordance - 1;
		Draw_FillRGBA (coords->x0, coords->grid[0][bn].y,
			coords->panelw, rowh, 0.04f, 0.05f, 0.09f, 0.75f);
		Draw_FillRGBA (coords->x0, coords->grid[0][bn].y,
			coords->panelw, 2, 0.30f, 0.33f, 0.40f, 0.9f);
		Draw_TTFString (coords->grid[0][bn].xp + 12, coords->grid[0][bn].y + rowh / 2 + 6, d,
			0.62f, 0.65f, 0.72f, 1.0f, 12.0f);
	}
}

void M_Options_Draw (void)
{
	size_t			n, shown;
	enum m_state_e	saved;
	int				rowh;
	int				i, segw, bandy;
	struct MenuCoords	coords;

	UNI_ValidateTab ();	//r101: Editor tab can hide itself out from under us
	uni_tab_memory[uni_tab] = (uni_tab == UNI_TAB_PERF) ? (int)perf_cursor : uni_cursor;	//r101: live row memory

	// recursive background = the page we came from (same pattern as perf page)
	saved = m_state;
	m_state = uni_return_state;
	m_recursiveDraw = true;
	M_Draw ();
	m_state = saved;

	// r101: dim shade behind the menu (user) -- panels themselves are opaque
	Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 0.7f);

	if (uni_tab == UNI_TAB_PERF)
	{
		shown = PERF_ROWS;
		if (shown > (size_t)vertical_perf_affordance - 1)
			shown = (size_t)vertical_perf_affordance - 1;
		if (perf_first + shown - 1 < perf_cursor)
			perf_first = perf_cursor - (shown - 1);
		if (perf_first > perf_cursor)
			perf_first = perf_cursor;
	}
	else
	{
		int nrows = uni_tab_nrows[uni_tab];
		shown = (size_t)nrows;
		if (shown > (size_t)vertical_perf_affordance - 1)
			shown = (size_t)vertical_perf_affordance - 1;
		if ((size_t)uni_first + shown - 1 < (size_t)uni_cursor)
			uni_first = uni_cursor - ((int)shown - 1);
		if (uni_first > uni_cursor)
			uni_first = uni_cursor;
	}

	// header scale 2.0 -> the double-height header band hosts the tab strip
	coords = Draw_WindowGridBase ("", vertical_perf_affordance, MVS_P, 2, 0.4, 1.0, 1.0, 0, 2,
		(uni_tab == UNI_TAB_PERF) ? (int)(perf_cursor - perf_first) : (int)(uni_cursor - uni_first),
		1, 1.0, 1.5, 0.0);
	M_Draw_Hint(PERF, coords);

	rowh = coords.grid[1][1].y - coords.grid[1][0].y;
	if (rowh <= 0)
		rowh = MVS;

		// tab strip: L/R cycling arrows occupy their own edge slots, drawn like
	// disabled tabs (user: not boxes floating over the strip)
	{
		extern qpic_t	*b_lt, *b_rt;	// L/R trigger button art
		int	ih = coords.headerh;
		int	slotw = 34;
		int	ix0 = coords.x0 + slotw;
		int	iw = coords.panelw - 2 * slotw;
		bandy = coords.grid[0][0].y - ih;
		{
			//r101: hidden tabs take no slot -- the strip packs visible ones
			int	nvis = 0, slot;
			for (i = 0; i < UNI_TABS; i++)
				if (UNI_TabVisible (i))
					nvis++;
			segw = iw / (nvis ? nvis : UNI_TABS);
			slot = 0;
			for (i = 0; i < UNI_TABS; i++)
			{
				if (!UNI_TabVisible (i))
					continue;
				if (i == uni_tab)
					Draw_FillRGBA (ix0 + slot * segw, bandy + 2, segw, ih - 2,
						0.055f, 0.075f, 0.135f, 1.0f);	// solid, matches the body strip
				slot++;
			}
		}
		{	// same art + size as the matchmaking hints (M_HintIcon: native
			// aspect at 16px tall, 0.6 fade reads as "disabled tab" per user)
			int		lx = coords.x0 + (slotw - M_HintIconW (b_lt)) / 2;
			int		rx = coords.x0 + coords.panelw - slotw + (slotw - M_HintIconW (b_rt)) / 2;
			M_HintIcon (&lx, bandy + ih - 15, b_lt);
			M_HintIcon (&rx, bandy + ih - 15, b_rt);
		}
		{
			//r101: labels ride the packed slot positions too
			int	slot = 0;
			for (i = 0; i < UNI_TABS; i++)
			{
				int	tw, tx, ty;
				if (!UNI_TabVisible (i))
					continue;
				tw = (int)Draw_TTFHeaderWidth (uni_tab_names[i], 14.0f);
				tx = ix0 + slot * segw + (segw - tw) / 2;
				ty = bandy + ih - 15;	// glyphs sit on the band's bottom edge
				if (i == uni_tab)
				{
					Draw_TTFStringHeader (tx, ty, uni_tab_names[i], 1, 1, 1, 1.0f, 14.0f);
					Draw_FillRGBA (ix0 + slot * segw + 4, bandy + ih - 2, segw - 8, 2, 0.85f, 0.68f, 0.25f, 1.0f);
				}
				else
					Draw_TTFStringHeader (tx, ty, uni_tab_names[i], 0.55f, 0.58f, 0.65f, 1.0f, 14.0f);
				slot++;
			}
		}
	}

	if (uni_tab == UNI_TAB_PERF)
	{
		M_Performance_Body (&coords, rowh, shown);
		M_Draw_Scrollbar (&coords, rowh, PERF_ROWS, perf_first, (int)shown);
		return;
	}

	for (n = 0; n < shown; n++)
	{
		const uni_row_t	*r = &uni_tab_rows[uni_tab][uni_first + n];
		char			 vbuf[64];
		const char		*v;
		float			 frac = 0;

		v = UNI_Value (r, vbuf, sizeof(vbuf));
		M_PrintWhite (coords.grid[0][n].xp, coords.grid[0][n].yp, r->label);
		if (!r->noslider && r->cvar && r->maxval > r->minval)
		{
			frac = (r->cvar->value - r->minval) / (r->maxval - r->minval);
			M_Draw_PerfSlider (&coords, (int)n, rowh, frac, v, r->maxval <= 1.0f && !r->percent && !r->show);
		}
		else if (r->show && r->maxval <= 1.0f)
		{	// two-state custom rows (invert, mlook, texture filter) get the pill too
			frac = (!strcmp (v, "On") || !strcmp (v, "Smooth")) ? 1 : 0;
			M_Draw_PerfSlider (&coords, (int)n, rowh, frac, v, 1);
		}
		else
		{
			int vx = coords.grid[0][n].x + coords.colw * coords.cols
				- (int)Draw_TTFWidth (v, 14.0f) - 12;
			M_PrintWhite (vx, coords.grid[0][n].yp, v);
		}
	}

	// footnote band, edge to edge, same styling as the perf tab; pinned to the bottom slot
	M_Draw_Scrollbar (&coords, rowh, uni_tab_nrows[uni_tab], uni_first, (int)shown);
	{
		const char *d = uni_tab_rows[uni_tab][uni_cursor].desc;
		int	bn = (int)vertical_perf_affordance - 1;
		Draw_FillRGBA (coords.x0, coords.grid[0][bn].y,
			coords.panelw, rowh, 0.04f, 0.05f, 0.09f, 0.75f);
		Draw_FillRGBA (coords.x0, coords.grid[0][bn].y,
			coords.panelw, 2, 0.30f, 0.33f, 0.40f, 0.9f);
		Draw_TTFString (coords.grid[0][bn].xp + 12, coords.grid[0][bn].y + rowh / 2 + 6, d,
			0.62f, 0.65f, 0.72f, 1.0f, 12.0f);
	}
}

void M_Options_Key (int k)
{
	UNI_ValidateTab ();	//r101
	switch (k)
	{
	case K_YBUTTON: // triangle -- restore defaults (confirm modal, like the rules page "Defaults")
		S_LocalSound ("misc/menu2.wav");
		UNI_ActionDefaults (0);
		return;	//stay on the page; X/O inside the modal, no persistence here

	case K_XBUTTON: // square -- save the shown values as the Custom mix
#ifndef VITA
	case 'g':	// Mac alias for square (matchmaking submenu precedent)
	case 'G':
#endif
		if (uni_tab == UNI_TAB_PERF)
		{	// the custom mix covers only the Performance knobs; other tabs no-op
			S_LocalSound ("misc/menu2.wav");
			PERF_Custom_Store ();
			Host_WriteConfiguration ();	// archive now; do not wait for Options to close
		}
		return;

	case K_ESCAPE:
	case K_BBUTTON: // circle
#ifdef VITA
	case K_ENTER:	// Vita: enter closes, like the old standalone perf page
#endif
		Host_WriteConfiguration ();	// archived cvars live here now: always persist
		if (uni_return_state == m_pause)
			M_Menu_Pause_f ();
		else
			M_Menu_Main_f ();
		return;

	case K_ABUTTON: // x
	case 'y':
	case 'Y':
#ifndef VITA
	case K_ENTER:
	case K_KP_ENTER:
		/* Mac/PC port: Enter selects, like the Vita X button. */
#endif
		if (uni_tab == UNI_TAB_PERF)
		{
			if (perf_cursor == 0 && (int)perf_preset.value > 0)
			{	// re-apply the current preset (undoes manual tweaks)
				S_LocalSound ("misc/menu2.wav");
				M_Performance_ApplyPreset ((int)perf_preset.value);
			}
		}
		else
		{
			const uni_row_t *r = &uni_tab_rows[uni_tab][uni_cursor];
			if (r->set)
			{
				S_LocalSound ("misc/menu2.wav");
				r->set (0);
			}
		}
		return;

	case K_LSHOULDER:	// perf fix: Vita L/R report as shoulders, not triggers
	case K_LTRIGGER:
		M_Options_CycleTab (-1);
		return;
	case K_RSHOULDER:
	case K_RTRIGGER:
		M_Options_CycleTab (1);
		return;
#ifndef VITA
	// Mac convenience: Tab/] /E cycle right, Q/[ left -- r101: brackets
	// alias the L/R triggers per user
	case K_TAB:
	case 'e':
	case 'E':
	case ']':
		M_Options_CycleTab (1);
		return;
	case 'q':
	case 'Q':
	case '[':
		M_Options_CycleTab (-1);
		return;
#endif

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (uni_tab == UNI_TAB_PERF)
		{
			perf_cursor--;
			if (perf_cursor >= PERF_ROWS)
				perf_cursor = PERF_ROWS - 1;
		}
		else
		{
			int nrows = uni_tab_nrows[uni_tab];
			uni_cursor--;
			if (uni_cursor < 0)
				uni_cursor = nrows - 1;
		}
		return;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (uni_tab == UNI_TAB_PERF)
		{
			perf_cursor++;
			if (perf_cursor >= PERF_ROWS)
				perf_cursor = 0;
		}
		else
		{
			int nrows = uni_tab_nrows[uni_tab];
			uni_cursor++;
			if (uni_cursor >= nrows)
				uni_cursor = 0;
		}
		return;

	case K_LEFTARROW:
		S_LocalSound ("misc/menu3.wav");
		if (uni_tab == UNI_TAB_PERF)
			M_Performance_Step (-1);
		else
		{
			const uni_row_t *r = &uni_tab_rows[uni_tab][uni_cursor];
			if (r->set)
				r->set (-1);
			else if (r->cvar)
				UNI_Step (r, -1);
		}
		return;

	case K_RIGHTARROW:
		S_LocalSound ("misc/menu3.wav");
		if (uni_tab == UNI_TAB_PERF)
			M_Performance_Step (1);
		else
		{
			const uni_row_t *r = &uni_tab_rows[uni_tab][uni_cursor];
			if (r->set)
				r->set (1);
			else if (r->cvar)
				UNI_Step (r, 1);
		}
		return;

	default:
		break;
	}
}

size_t		mlist_cursor;
size_t		mlist_first;
float vertical_mlist_affordance = 9;	// perf B7: 10 -> 9, pack tabs ride the header band; first pack (10 maps) scrolls

// perf B7: Custom Maps tab -- plain .bsp files the user drops into <gamedir>/maps/custom
#define MAX_CUSTOM_MAPS 64
typedef struct {
	char	name[32];
	char	disp[32];	// perf B7: title-cased from the filename
	char	bio[192];	// perf B7: worldspawn "message" pulled from the .bsp
} custom_map_t;
static custom_map_t	custom_maps[MAX_CUSTOM_MAPS];
static int	custom_count = -1;	// -1 = not scanned yet

static int custom_cmp (const void *a, const void *b)
{
	return q_strcasecmp (((const custom_map_t *)a)->name, ((const custom_map_t *)b)->name);
}

static void	M_Maps_ScanCustom (void);

/* perf B7: turn a dropped filename into a tidy title - underscores and
 * dashes collapse to single spaces, each word starts capitalized, the rest
 * is left as typed (so "dm2_e1m2" reads "Dm2 E1m2" but "ZTest-Pit" keeps
 * its caps). */
static void M_CustomDisplayName (const char *fn, char *out, int outsize)
{
	int	o = 0, newword = 1;
	char c;

	for (; *fn && o < outsize - 2; fn++)
	{
		c = *fn;
		if (c == '_' || c == '-' || c == ' ')
		{
			if (o && out[o-1] != ' ')
				out[o++] = ' ';
			newword = 1;
			continue;
		}
		if (newword && c >= 'a' && c <= 'z')
			c -= 32;
		newword = 0;
		out[o++] = c;
	}
	while (o && out[o-1] == ' ')
		o--;
	out[o] = 0;
}

/* perf B7: read the worldspawn "message" out of a custom .bsp for the blurb.
 * On disk a BSP is int version + a lump directory of {ofs,len} pairs, and
 * entities is lump 0 in every BSP20..29 layout. Only the header and a
 * clamped slice of the entity text are read, so multi-MB maps cost nothing;
 * any mismatch just leaves an empty string. */
static void M_CustomBspBio (const char *name, char *out, int outsize)
{
	char	path[MAX_OSPATH];
	byte	hdr[4 + 15*8];
	static byte	ents[8192];
	int	h, vers, ofs, len, n;
	char	*p, *e;

	out[0] = 0;
	q_snprintf (path, sizeof (path), "%s/maps/custom/%s.bsp", com_gamedir, name);
	if (Sys_FileOpenRead (path, &h) == -1)
		return;
	if (Sys_FileRead (h, hdr, sizeof (hdr)) != (int) sizeof (hdr))
	{
		Sys_FileClose (h);
		return;
	}
	vers = *(int *)hdr;
	ofs = *(int *)(hdr + 4);
	len = *(int *)(hdr + 8);
	if (vers < 20 || vers > 30 || ofs < (int)sizeof (hdr) || len <= 0 || len > 0x100000)
	{
		Sys_FileClose (h);
		return;
	}
	if (len > (int)sizeof (ents) - 1)
		len = (int)sizeof (ents) - 1;
	Sys_FileSeek (h, ofs);
	n = Sys_FileRead (h, ents, len);
	Sys_FileClose (h);
	if (n <= 0)
		return;
	ents[n] = 0;

	p = q_strcasestr ((char *)ents, "\"message\"");
	if (!p)
		return;
	p += sizeof ("\"message\"") - 1;
	while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
		p++;
	if (*p != '"')
		return;
	p++;
	for (e = out; *p && *p != '"' && e - out < outsize - 1; p++)
	{
		if (*p == '^')	// color codes would print literally in the menu font
			continue;
		*e++ = *p;
	}
	*e = 0;
}

/* perf B7: manual refresh for when someone drops a new .bsp mid-session */
static void M_Maps_Rescan_f (void)
{
	M_Maps_ScanCustom ();
	Con_Printf ("%i custom map(s) in maps/custom\n", custom_count);
}

static void M_Maps_ScanCustom (void)
{
	char			path[MAX_OSPATH];
	DIR			*dir_p;
	struct dirent		*dir_t;
	custom_count = 0;
	q_snprintf (path, sizeof(path), "%s/maps/custom", com_gamedir);
	dir_p = opendir (path);
	if (!dir_p)
		return;
	while (custom_count < MAX_CUSTOM_MAPS && (dir_t = readdir (dir_p)) != NULL)
	{
		if (q_strcasecmp (COM_FileGetExtension (dir_t->d_name), "bsp") != 0)
			continue;
		COM_StripExtension (dir_t->d_name, custom_maps[custom_count].name, sizeof(custom_maps[0].name));
		M_CustomDisplayName (custom_maps[custom_count].name, custom_maps[custom_count].disp, sizeof(custom_maps[0].disp));
		M_CustomBspBio (custom_maps[custom_count].name, custom_maps[custom_count].bio, sizeof(custom_maps[0].bio));
		custom_count++;
	}
	closedir (dir_p);
	if (custom_count > 1)
		qsort (custom_maps, custom_count, sizeof(custom_maps[0]), custom_cmp);
}

static int M_Maps_LevelCount (void)
{
	if (startepisode == NUM_EPISODES - 1)
	{
		if (custom_count < 0)
			M_Maps_ScanCustom ();
		return custom_count;
	}
	return episodes[startepisode].levels;
}

/* perf B7: a custom pack entry viewed as a level_t so the map browser can
 * treat both packs alike in the info column (its thumbnail is resolved
 * separately: gfx/maps/custom/<name>.png first, placeholder card after). */
static const level_t *M_Maps_CustomLevel (int idx)
{
	static level_t	cv;
	static char		cfile[64];

	if (idx < 0 || idx >= custom_count)
		idx = 0;
	q_strlcpy (cfile, custom_maps[idx].name, sizeof (cfile));
	q_strlcat (cfile, ".bsp", sizeof (cfile));
	cv.name = custom_maps[idx].name;
	cv.thumbnail = custom_maps[idx].name;
	cv.targaThumbnail = 0;
	cv.description = custom_maps[idx].disp;
	cv.mapBio = custom_maps[idx].bio[0] ? custom_maps[idx].bio : NULL;
	cv.author = cfile;
	return &cv;
}

// perf B7: the matchmaking summary page wants art/bio for the chosen map;
// custom picks have neither -- borrow the first refined-pack entry for the
// picture and never index levels[] with a custom number.
static const level_t *M_Maps_ChosenDisplay (void)
{
	int	idx;
	if (chosen_level.episode == NUM_EPISODES - 1)
		return &levels[0];
	idx = episodes[chosen_level.episode].firstLevel + chosen_level.level;
	if (idx < 0 || idx >= (int)(sizeof(levels) / sizeof(levels[0])))
		return &levels[0];
	return &levels[idx];
}

const char *M_listPrintMap (size_t index) {

    static char	string[64];
    if (startepisode == NUM_EPISODES - 1)	// perf B7: custom pack = folder names
    {
        if ((int)index >= custom_count)
            return "";
        q_snprintf(string, sizeof(string), "%-18.18s\n", custom_maps[index].disp);
        return string;
    }
    if (index >= episodes[startepisode].levels)
        return "";

    q_snprintf(string, sizeof(string), "%-18.18s\n", levels[episodes[startepisode].firstLevel + index].description);

    return string;
}

/* r120 (user): map thumbnails used to decode+upload on the exact frame the
 * cursor first landed on a row -- visible pop-in lag while browsing.
 * menu_cachepics never evicts, so warming each shot once makes every later
 * visit free; one image per frame keeps any hitch imperceptible (~24 shots
 * + custom pack). Reset on browser entry so shots dropped in mid-session
 * (fresh mapshots, new custom .bsp) are still picked up. */
static int	mappic_warm = 0;	// levels[] idx, then custom placeholder, then custom maps, -1 done

static void M_Maps_WarmOne (void)
{
	if (mappic_warm < 0)
		return;
	if (mappic_warm < (int)(sizeof(levels)/sizeof(levels[0])))
	{
		const level_t *lv = &levels[mappic_warm];
		const char *mspath = va("gfx/maps/%s.png", lv->thumbnail);
		if (COM_FileExists (mspath, NULL))
			Draw_TryCachePic (mspath, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
		else
			Draw_CachePic (va("gfx/maps/%s.%s", lv->thumbnail, lv->targaThumbnail ? "tga" : "lmp"));
		mappic_warm++;
		return;
	}
	if (mappic_warm == (int)(sizeof(levels)/sizeof(levels[0])))
	{
		if (COM_FileExists ("gfx/maps/custom.png", NULL))
			Draw_TryCachePic ("gfx/maps/custom.png", TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
		mappic_warm++;
		return;
	}
	{
		int ci = mappic_warm - (int)(sizeof(levels)/sizeof(levels[0])) - 1;
		if (custom_count >= 0 && ci < custom_count)
		{
			const char *cp = va("gfx/maps/custom/%s.png", custom_maps[ci].name);
			if (COM_FileExists (cp, NULL))
				Draw_TryCachePic (cp, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
			mappic_warm++;
			return;
		}
		mappic_warm = -1;	// whole set resident
	}
}

void M_Matchmaking_Map_f (void) {
    key_dest = key_menu;
    m_state = m_matchmaking_map;
    m_entersound = true;
    cursorfocused = false;
    mlist_cursor = 0;
    mlist_first = 0;
    mappic_warm = 0;	// r120: trickle-load every thumbnail in the background
    IN_UpdateGrabs();
}

/* r92: greedy word-wrap for map blurbs -- splits on spaces, never mid-word
 * (user request). Prints through the M_PrintWhite funnel at 14px with a
 * CHARZ*1.5 baseline step, capped at maxlines. */
static void M_WrapText (const char *text, int x, int y, int maxpx, int maxlines)
{
        char line[128];
        char word[64];
        char probe[160];
        const char *p = text;
        int lines = 0;

        while (*p && lines < maxlines)
        {
                int li = 0;
                line[0] = 0;
                while (*p == ' ')
                        p++;
                while (*p && lines < maxlines)
                {
                        int wi = 0, k;
                        while (*p && *p != ' ')
                        {
                                if (wi < 62)
                                        word[wi++] = *p;
                                p++;
                        }
                        word[wi] = 0;
                        if (li + (li ? 1 : 0) + wi > 126)
                                break;
                        q_strlcpy (probe, line, sizeof(probe));
                        if (li)
                                q_strlcat (probe, " ", sizeof(probe));
                        q_strlcat (probe, word, sizeof(probe));
                        if (Draw_TTFWidth (probe, 14.0f) > (float)maxpx && li)
                        {
                                M_PrintWhite (x, y, line);
                                y += CHARZ * 3 / 2;
                                lines++;
                                li = 0;
                                line[0] = 0;
                                if (lines >= maxlines)
                                        break;
                        }
                        if (li)
                                line[li++] = ' ';
                        for (k = 0; k < wi; k++)
                                line[li++] = word[k];
                        line[li] = 0;
                        while (*p == ' ')
                                p++;
                }
                if (li && lines < maxlines)
                {
                        M_PrintWhite (x, y, line);
                        lines++;
                }
        }
}

void M_Matchmaking_Map_Draw (void) {

    M_Maps_WarmOne ();	// r120: one thumbnail per frame until the set is resident
    // set to matchmaking for recursive draw
    m_state = m_gameoptions;
    m_recursiveDraw = true;
    M_Draw ();
    // now draw this menu
    m_state = m_matchmaking_map;

    // setup scrolling cursor (perf B7: custom pack counted live from disk)
    size_t	mlist_shown;
    int		levcount = M_Maps_LevelCount ();
    if ((int)mlist_cursor >= levcount)
        mlist_cursor = levcount > 0 ? levcount - 1 : 0;
    mlist_shown = (size_t)levcount;
    if (mlist_shown > (size_t)vertical_mlist_affordance)
        mlist_shown = vertical_mlist_affordance;
    if (mlist_shown && mlist_first+mlist_shown-1 < mlist_cursor)
        mlist_first = mlist_cursor-(mlist_shown-1);
    if (mlist_first > mlist_cursor)
        mlist_first = mlist_cursor;

    // r101: dim shade behind the menu (user) -- panels themselves are opaque
    Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 0.7f);
    // title
    static char title[128];
    q_snprintf(title, sizeof(title), "Map Selection\n");   /* r91: pack name drawn separately in blue */
    // window (perf B7: Options-identical band: 1.5 header, no Base title -- the tabs are the header)
    struct MenuCoords coords = Draw_WindowGridBase ("", vertical_mlist_affordance, MVS_P, 2, 0.35, 1.0, 1.0, 0, 2, mlist_cursor-mlist_first, 1, 1.0, 1.5, 0.0);
    draw_grid_translucent = 0;   /* r92: opaque popup (perf B7: hoisted from the old pack-name block) */
    /* perf B7: the packs as a tab strip, exact Options geometry. */
    {
        extern qpic_t	*b_lt, *b_rt;
        int	bandy = coords.grid[0][0].y - coords.headerh;
        int	ih = coords.headerh;
        int	slotw = 34;
        int	ix0 = coords.x0 + slotw;
        int	iw = coords.panelw - 2 * slotw;
        int	segw = iw / NUM_EPISODES;
        int	i;
        for (i = 0; i < NUM_EPISODES; i++)
            if (i == startepisode)
                Draw_FillRGBA (ix0 + i * segw, bandy + 2, segw, ih - 2,
                    0.055f, 0.075f, 0.135f, 1.0f);
        {
            int	lx = coords.x0 + (slotw - M_HintIconW (b_lt)) / 2;
            int	rx = coords.x0 + coords.panelw - slotw + (slotw - M_HintIconW (b_rt)) / 2;
            M_HintIcon (&lx, bandy + ih - 15, b_lt);
            M_HintIcon (&rx, bandy + ih - 15, b_rt);
        }
        for (i = 0; i < NUM_EPISODES; i++)
        {
            int	tw = (int)Draw_TTFHeaderWidth (episodes[i].description, 14.0f);
            int	tx = ix0 + i * segw + (segw - tw) / 2;
            int	ty = bandy + ih - 15;
            if (i == startepisode)
            {
                Draw_TTFStringHeader (tx, ty, episodes[i].description, 1, 1, 1, 1.0f, 14.0f);
                Draw_FillRGBA (ix0 + i * segw + 4, bandy + ih - 2, segw - 8, 2, 0.85f, 0.68f, 0.25f, 1.0f);
            }
            else
                Draw_TTFStringHeader (tx, ty, episodes[i].description, 0.55f, 0.58f, 0.65f, 1.0f, 14.0f);
        }
    }

    /* r92: vertical divider between the list column and the info column
     * (perf B7: the custom pack has a real info column now too) */
    Draw_FillRGBA ((int)coords.grid[1][0].x - 2, (int)coords.grid[0][0].y, 2,
                   (int)(coords.grid[0][coords.rows].yp + coords.rowh - coords.grid[0][0].y),
                   20/255.0f, 25/255.0f, 44/255.0f, 1.0f);
    //footer
    switch (mlist_cursor) {
        default:
            M_Draw_Hint(SELECT_SCROLL_MAPS, coords);
    }
    // map name
    for (int n = 0; n < mlist_shown; n++) {
        M_PrintWhite (coords.grid[0][n].xp, coords.grid[0][n].yp, M_listPrintMap (mlist_first+n));
    }
    if (levcount == 0)	// perf B7: empty custom folder hint
        M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "Drop .bsp files into maps/custom to play them here");
    // perf B7: left gutter scrollbar (Options treatment)
    {
        int	rowh = coords.grid[1][1].y - coords.grid[1][0].y;
        if (rowh <= 0)
            rowh = MVS;
        M_Draw_Scrollbar (&coords, rowh, levcount, mlist_first, (int)mlist_shown);
    }
    // map thumbnail + info column (perf B7: the custom pack gets the same
    // treatment now -- its data comes from the folder scan instead of levels[])
    const level_t *lv;
    char format[10];
    if (startepisode == NUM_EPISODES - 1)
    {
        if (custom_count == 0)
		return;	// empty folder: no info to show
        lv = M_Maps_CustomLevel ((int)mlist_cursor);
    }
    else
        lv = &levels[episodes[startepisode].firstLevel + (mlist_cursor)];
    if (lv->targaThumbnail) {
        strcpy(format, "tga"); // why not just bake it into the filename? why did I do it like this? testing? idk
    } else {
        strcpy(format, "lmp");
    }
    qpic_t *mappic = NULL;   /* r90: PNG mapshot if it exists, else legacy lmp/tga (Draw_TryCachePic never returns NULL -- gate on COM_FileExists) */
    if (startepisode == NUM_EPISODES - 1)
	{// perf B7: per-map mapshot first, then the shared placeholder card
        const char *cp = va("gfx/maps/custom/%s.png", custom_maps[(int)mlist_cursor].name);
        if (COM_FileExists (cp, NULL))
            mappic = Draw_TryCachePic (cp, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
        else if (COM_FileExists ("gfx/maps/custom.png", NULL))
            mappic = Draw_TryCachePic ("gfx/maps/custom.png", TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
    }
    else
    {
        const char *mspath = va("gfx/maps/%s.png", lv->thumbnail);
        if (COM_FileExists (mspath, NULL))
            mappic = Draw_TryCachePic (mspath, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
        if (!mappic)
            mappic = Draw_CachePic(va("gfx/maps/%s.%s", lv->thumbnail, format));
    }
    if (mappic)
        Draw_StretchPic(coords.grid[1][0].x + 1, coords.grid[1][0].y, mappic, coords.colw, coords.rowh*5);

    // map bio (r92: single string, wrapped on words to the column width)
    {
        int bio_x = coords.grid[1][5].xp + CHARZ;   /* r93: padding off the column divider */
        int bio_maxpx = (int)coords.grid[coords.cols - 1][0].x + (int)coords.colw - bio_x - 16;   /* r93: wrap to the window edge, not the screen */
        const char *bio = lv->mapBio;
        M_Print(coords.grid[1][5].x + CHARZ, coords.grid[1][5].yp, "Map Description:");
        if (bio && bio[0])
            M_WrapText (bio, bio_x, coords.grid[1][5].yp + CHARZ * 3 / 2, bio_maxpx, 4);
        else
            M_PrintWhite (bio_x, coords.grid[1][5].yp + CHARZ * 3 / 2, "No description available.");
    }
    /* perf B7: author on one inline row -- the two-row version spilled into
     * the footer hint band. */
    {
        int abx = coords.grid[1][8].x + CHARZ;
        M_Print(abx, coords.grid[1][8].yp, "Author:");
        M_PrintWhite (abx + (int)Draw_TTFWidth ("Author:", 14.0f) + 8,
            coords.grid[1][8].yp, lv->author);
    }
}


void M_Matchmaking_Map_Submenu_Key (int xdir, int ydir)
{
    int epcount, levcount;
    startepisode += xdir;
    startlevel += ydir;
    epcount = NUM_EPISODES;
    levcount = episodes[startepisode].levels;

    if (startepisode < 0)
        startepisode = epcount - 1;

    if (startepisode >= epcount)
        startepisode = 0;
}

void M_Matchmaking_Map_Key (int key) {
    switch (key)
    {
        case K_YBUTTON: // triangle
#ifdef VITA
        case K_ENTER:
#endif
        case K_ESCAPE:
        case K_BBUTTON: // circle
        case 'n':
        case 'N':
            m_state = m_gameoptions;
            m_entersound = true;
            cursorfocused = true;
            break;
        case K_ABUTTON: // x
        case 'y':
        case 'Y':
#ifndef VITA
        case K_ENTER:
        case K_KP_ENTER:
            /* Mac/PC port: Enter selects, like the Vita X button. */
#endif
            if (startepisode == NUM_EPISODES - 1 && custom_count == 0)
                break;	// perf B7: nothing to pick in an empty custom folder
            chosen_level.episode = startepisode;
            chosen_level.level = mlist_cursor;
            m_state = m_gameoptions;
            cursorfocused = true;
            S_LocalSound ("misc/menuoption.wav");
            break;
        case K_UPARROW:
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor--;
            if (mlist_cursor >= (size_t)M_Maps_LevelCount ())
                mlist_cursor = M_Maps_LevelCount () - 1;
            break;
        case K_DOWNARROW:
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor++;
            if (mlist_cursor >= (size_t)M_Maps_LevelCount ())
                mlist_cursor = 0;
            break;
        case K_LSHOULDER:	// perf fix: Vita L/R report as shoulders, not triggers
        case K_LTRIGGER:
        case K_LEFTARROW:
#ifndef VITA
        case 'q':	// perf B7: same tab aliases as unified Options
        case '[':	// r101: bracket alias of L trigger
#endif
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor = 0;
            mlist_first = 0;
            M_Matchmaking_Map_Submenu_Key(-1,0);
            break;
        case K_RSHOULDER:	// perf fix: Vita L/R report as shoulders, not triggers
        case K_RTRIGGER:
        case K_RIGHTARROW:
#ifndef VITA
        case K_TAB:
        case 'e':
        case ']':	// r101: bracket alias of R trigger
#endif
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor = 0;
            mlist_first = 0;
            M_Matchmaking_Map_Submenu_Key(1,0);
            break;

        default:
            break;
    }
}

#define MM_HEIGHT_PERCENT 1
#define MM_HEIGHT_PIX PixHeight(MM_HEIGHT_PERCENT)
#define MM_HEADER_HEIGHT mvs(3)
#define MM_FOOTER_HEIGHT mvs(1)

#define	NUM_GAMEOPTIONS	3 // r77: NETWORK / PLAYLIST / START MATCHMAKING
int matchmaking_cursor = NUM_GAMEOPTIONS - 1; // perf B7: open on START MATCHMAKING (user)

/* P2a lobby roster: bot count lives in the engine cvar lobby_bots so the
 * matchmaking menu can adjust it before a match exists (cl.scores is only
 * allocated on svc_serverinfo, see issue #41). Created in M_Init via
 * Cvar_Create so it always exists (QC cvar() on unset cvars reads 0).
 * Default 3 bots + you = 4 players of 8. Names come from bots.cfg
 * (bot_nameK cvars); QC reads the same cvars at BotConnect, so lobby and
 * in-game names match. */
static cvar_t	*lobby_bots;
static cvar_t	*lobby_gametype;	/* r117: remembered mode (0 slayer, 1 tdm, 2 swat) */

void M_Lobby_AddBot_f (void)
{
	/* r65e: give each freshly added bot a random unused name. The roster preview
	 * reads slots in order, so swap a random tail slot into the one about to be
	 * shown; previously every re-add in the same lobby showed the same name
	 * because the shuffle only ran when the menu opened. */
	int n = lobby_bots ? (int)lobby_bots->value : 0;
	char a[32], b[32];

	if (key_dest == key_menu && m_state == m_gameoptions && lobby_bots && lobby_bots->value < 7)
	{
		if (n < 7)
		{
			int j;
			srand ((unsigned int)(Sys_DoubleTime () * 1000.0));
			j = n + rand() % (7 - n);
			q_strlcpy (a, Cvar_VariableString (va("bot_name%d", n)), sizeof(a));
			q_strlcpy (b, Cvar_VariableString (va("bot_name%d", j)), sizeof(b));
			Cvar_Set (va("bot_name%d", n), b);
			Cvar_Set (va("bot_name%d", j), a);
		}
		Cvar_SetValue ("lobby_bots", lobby_bots->value + 1);
	}
}

void M_Lobby_DelBot_f (void)
{
	if (key_dest == key_menu && m_state == m_gameoptions && lobby_bots && lobby_bots->value > 0)
		Cvar_SetValue ("lobby_bots", lobby_bots->value - 1);
}

/* r64: shuffle the bot_nameK cvars (loaded by bots.cfg) each time the lobby
 * opens. The roster preview reads them in slot order and QC's BotConnect
 * reads them by connect-order slot, so shuffling HERE makes lobby names and
 * in-game names agree while still being random per session. Runs as a cbuf
 * command right after "exec bots.cfg" so the cfg values are already in. */
void M_Lobby_ShuffleNames_f (void)
{
	char names[7][32];
	char tmp[32];
	int i, j;

	for (i = 0; i < 7; i++)
		q_strlcpy (names[i], Cvar_VariableString (va("bot_name%d", i)), sizeof(names[i]));

	srand ((unsigned int)(Sys_DoubleTime () * 1000.0));
	for (i = 6; i > 0; i--)	/* Fisher-Yates */
	{
		j = rand() % (i + 1);
		q_strlcpy (tmp, names[i], sizeof(tmp));
		q_strlcpy (names[i], names[j], sizeof(names[i]));
		q_strlcpy (names[j], tmp, sizeof(names[j]));
	}

	for (i = 0; i < 7; i++)
		Cvar_Set (va("bot_name%d", i), names[i]);
}

/* r96/r113: lobby_profile_saved / lobby_saved_bottom / lobby_saved_top
 * live above the setup page now (M_Setup_Key keeps them fresh). */

/* r62: FFA band pool mirroring HaloColorAt in frikbot/bot.qc; the
 * player owns band 14 (bottomcolor), so bots start at blue.
 * r115: hoisted to file scope -- M_Matchmaking_f pins these into the
 * bot_color<slot> cvars the QC BotConnect ladder reads, making the
 * preview truth instead of a guess. */
static const int ffa_bands[6] = {13, 12, 11, 6, 5, 4};   /* r97: 4 = safe red. r115c: was band 2 — the QC ladder rejects 2, so that row's color never matched */

/* r118 (user): opening Matchmaking could instantly START a match -- the
 * cursor is static (still on row 2 from last time) and the press that opens
 * the page can deliver one more down event. Ignore confirms this long. */
static double mm_opened_time = -1000.0;

void M_Matchmaking_f (void)
{
    key_dest = key_menu;
    m_state = m_gameoptions;
    m_entersound = true;
    mm_opened_time = realtime;
    draw_grid_translucent = 0;   /* r101: opaque panels are the house style (user) */
    /* r117 (user): reopen where we left off. lobby_bots needs no work (it IS
     * the stored count); the mode lives in opts only, so pull it back from
     * the archived cvar. Clamp guards a stale/hand-edited config value. */
    if (lobby_gametype)
    {
        int gt = (int) lobby_gametype->value;
        if (gt < 0 || gt > 2)
            gt = TEAM_SLAYER;
        if ((int) opts.gametype != gt)
        {
            opts.gametype = (enum gametype_t) gt;
            opts.fraglimit = (gt == 1) ? 20 : 10;
        }
    }
    if (!lobby_profile_saved)
    {
        lobby_profile_saved = true;
        q_strlcpy (lobby_saved_bottom, Cvar_VariableString ("bottomcolor"),
                   sizeof(lobby_saved_bottom));
        q_strlcpy (lobby_saved_top, Cvar_VariableString ("topcolor"),
                   sizeof(lobby_saved_top));
    }
    /* r97/r114: leaving a TDM match re-pins nothing -- repair the live profile
     * back to what the lobby captured (or the Spartan page last set), so the
     * pin never sticks in menus, rosters or config.cfg. */
    M_Lobby_RestoreProfile ();
    Cbuf_AddText ("exec bots.cfg\nlobby_shufflenames\n");   // P2a lobby roster names (r64: + shuffle)
    /* r115: pin the preview's FFA bands into bot_color<slot> so the QC
     * BotConnect ladder gives each bot exactly the row color shown.
     * TDM ignores these (hard red vs blue there, by design).
     * r115b: create inline — the M_Init loop shipped in no deployed build
     * (stale menu.o relink), so relying on init-order creation left every
     * lobby open failing with "Cvar_Set: variable bot_colorN not found".
     * Cvar_Create is idempotent; not CVAR_ARCHIVE, so config.cfg stays clean. */
    {
        int ci;
        for (ci = 0; ci < 7; ci++)
        {
            Cvar_Create (va("bot_color%i", ci), "0");
            Cvar_Set (va("bot_color%i", ci), va("%i", ffa_bands[ci % 6]));
        }
    }
    if (maxplayers == 0)
        maxplayers = svs.maxclients;
    if (maxplayers < 2)
        maxplayers = svs.maxclientslimit;
    startepisode = 0;
    if (!mm_rentry) {
        startlevel = rand() % episodes[startepisode].levels;
    }
    chosen_level.episode = startepisode;
    chosen_level.level = startlevel;
    mm_rentry = false;
    cursorfocused = true;
    PlayMenuTheme(false);
}

/* P2a: footer-hint layout helpers — icon first, then its word. The old
 * single padded string only aligned while M_PrintWhite drew 8px monospace
 * conchars; it is proportional Archivo TTF now, so the words drifted off
 * their icons (user 2026-11-12). Draw_Button stretches each glyph to 16
 * canvas px; CHARZ (8) is the gap between elements. */
static int M_HintIconW (qpic_t *icon)
{
    /* r80 (user: "L/R images squished horizontally"): backl1/backr1 trigger
     * art is 32x16 but Draw_Button forced a 16x16 box. Keep height 16 and
     * scale the width from the source aspect. */
    if (icon && icon->height && icon->width != icon->height)
        return (int)(16.0 * icon->width / icon->height + 0.5);
    return 16;
}

static void M_HintIcon (int *x, int y, qpic_t *icon)
{
    int w = M_HintIconW (icon);
    /* r79b: buttons fade to 0.6 (user: "kind of bright"). The premul 2D
     * canvas ignores bare alpha, so rgb is darkened along with it.
     * r108: alpha must stay 1.0 — Mac's GL_ONE,ONE_MINUS_SRC_ALPHA canvas
     * multiplies it too and the double-halved texels vanished entirely
     * (main-menu Select icon "disappeared"); rgb 0.6 alone is the dim. */
    glColor4f (0.6f, 0.6f, 0.6f, 1.0f);
    Draw_StretchPic (*x, y - 4, icon, w, 16);
    glColor4f (1, 1, 1, 1);
    *x += w;
}

static void M_HintWord (int *x, int y, const char *word)
{
    *x += CHARZ;
    M_PrintWhite (*x, y, word);
    *x += (int) Draw_TTFWidth (word, 14.0f) + CHARZ;
}

static void M_Draw_HintY (int type, struct MenuCoords mc, int y) {
    // footer text hint -- each group is icon(s)+word. r76: measure the
    // whole group first, then draw it centered in the window; it used to
    // start pinned at the left cell padding (user 2026-11-12).
    qpic_t *icons[14];
    const char *words[14];
    int n = 0, i, total = 0, x;
    int lx = mc.grid[0][mc.rows].x;
    int rx = mc.grid[mc.cols - 1][mc.rows].x + mc.colw;

    #define HINTI(ic) do { icons[n] = (ic); words[n] = NULL; n++; } while (0)
    #define HINTW(wd) do { icons[n] = NULL; words[n] = (wd); n++; } while (0)

    if (type == SCROLL) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
    } else if (type == SELECT) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Select");
    } else if (type == SCROLL_RULES) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
        HINTI(b_up); HINTI(b_down); HINTW("Scroll");
        HINTI(b_xbutton); HINTW("Defaults"); // triangle
    } else if (type == SELECT_RULES) {
        HINTI(b_bbutton); HINTW("Back"); // o
        HINTI(b_abutton); HINTW("Select"); // x
        HINTI(b_up); HINTI(b_down); HINTW("Scroll");
        HINTI(b_xbutton); HINTW("Defaults"); // triangle
    } else if (type == INPUT) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_xbutton); HINTW("Edit");
        HINTI(b_ybutton); HINTW("Delete");
    } else if (type == SELECT_SCROLL) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Select");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
    } else if (type == SELECT_SCROLL_MAPS) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Select");
        HINTI(b_left); HINTI(b_right); HINTW("Change Map Pack");
    } else if (type == MATCH_MAIN) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Select");
        HINTI(b_ybutton); HINTW("Game Options");   /* square (b_ybutton holds fbtnsquare art) */
        HINTI(b_xbutton); HINTW("Maps");           /* triangle (b_xbutton holds fbtntriangle art) */
    } else if (type == MATCH_SCROLL) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
        HINTI(b_ybutton); HINTW("Game Options");
        HINTI(b_xbutton); HINTW("Maps");
    } else if (type == MATCH_MAIN_TOP) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Select");
    } else if (type == MATCH_SCROLL_TOP) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
    } else if (type == MATCH_BOTTOM) {
        HINTI(b_ybutton); HINTW("Game Options");   /* square */
        HINTI(b_xbutton); HINTW("Maps");           /* triangle */
    } else if (type == MATCH_LOBBY_BOTTOM) {
        /* r165 (user): Find Match has its own two actions. The square opens
         * the lobby list (Browse) and the triangle the advanced connect form;
         * MATCH_BOTTOM's "Game Options"/"Maps" belong to Custom Games and were
         * simply the wrong words here. b_ybutton holds the square art,
         * b_xbutton the triangle art (see M_HintIcon). */
        HINTI(b_ybutton); HINTW("Browse");         /* square -> K_XBUTTON */
        HINTI(b_xbutton); HINTW("Advanced");       /* triangle -> K_YBUTTON */
    } else if (type == MATCH_BROWSE_TOP) {
        /* r167 (user): the lobby popup gained a playlist filter and a sort
         * cycle, plus a rescan that does not need a keyboard.
         * r179 (user: "vita is not showing the bumper image icons for the
         * lobby menu"): the shoulders were the only groups in the menu spelled
         * out as text ("LB Rescan"/"RB Sort"), so the popup had no arm art for
         * them at all. Draw the real backl1/backr1 bumper art like every other
         * hint, and keep the words short -- the popup panel is only 392
         * virtual px wide and this row is centred inside it. */
        extern qpic_t *b_lt, *b_rt;   /* L/R shoulder art (gl_hud.c) */
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Select");
        HINTI(b_left); HINTI(b_right); HINTW("Filter");
        HINTI(b_lt); HINTW("Scan");
        HINTI(b_rt); HINTW("Sort");
    } else if (type == SELECT_DELETE) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Edit");
        HINTI(b_ybutton); HINTW("Delete");
    } else if (type == PERF) {	// perf B7
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
        HINTI(b_up); HINTI(b_down); HINTW("Scroll");
        HINTI(b_xbutton); HINTW("Defaults"); // triangle (b_xbutton holds fbtntriangle art)
        if (m_state == m_performance || uni_tab == UNI_TAB_PERF)
        {   // r102: braces -- without them only HINTW was guarded, so the
            // word leaked onto every tab while the icon stayed perf-only
            HINTI(b_ybutton); HINTW("Save Custom"); // square (b_ybutton holds fbnsquare art)
        }
    }

    for (i = 0; i < n; i++)
    {
        if (words[i])
            total += CHARZ + (int) Draw_TTFWidth (words[i], 14.0f) + CHARZ;
        else
            total += 16;
    }

    x = lx + ((rx - lx) - total) / 2;
    if (x < mc.grid[0][mc.rows].xp + CHARZ)
        x = mc.grid[0][mc.rows].xp + CHARZ;
    for (i = 0; i < n; i++)
    {
        if (words[i])
            M_HintWord (&x, y, words[i]);
        else
            M_HintIcon (&x, y, icons[i]);
    }

    #undef HINTI
    #undef HINTW
}

/* r156 (user: "the hintes arent centered?"): M_Draw_HintY measures its whole
 * group and centres it in the window (r76). The Mac lobby screen prints plain
 * text instead of Vita arm art, so it needs the same arithmetic. */
static void M_PrintHintCentred (struct MenuCoords mc, int y, const char *text)
{
    int lx = (int)mc.grid[0][mc.rows].x;
    int rx = (int)(mc.grid[mc.cols - 1][mc.rows].x + mc.colw);
    int w = (int)Draw_TTFWidth (text, 14.0f);
    int x = lx + ((rx - lx) - w) / 2;

    if (x < mc.grid[0][mc.rows].xp + CHARZ)
        x = mc.grid[0][mc.rows].xp + CHARZ;
    M_PrintWhite (x, y, text);
}

void M_Draw_Hint (int type, struct MenuCoords mc)
{
    M_Draw_HintY (type, mc, mc.grid[0][mc.rows].yp);
}

void M_Matchmaking_Draw (void)
{
    M_Maps_WarmOne ();	// r120: the chosen-map thumbnail should already be loaded here
    Draw_MenuBg();
    char format[10];
    if ((*M_Maps_ChosenDisplay ()).targaThumbnail) {
        strcpy(format, "tga");
    } else {
        strcpy(format, "lmp");
    }
    qpic_t *mappic = NULL;   /* r90: PNG mapshot if it exists, else legacy lmp (see map select) */
    {
        const char *mspath;
        int customsel = (chosen_level.episode == NUM_EPISODES - 1);
        if (customsel)
        {   /* perf B7: a custom map may ship its own gfx/maps/<name>.png;
             * without one the summary page shows the placeholder card. */
            const char *cname = (chosen_level.level < custom_count) ? custom_maps[chosen_level.level].name : "custom";
            mspath = va("gfx/maps/%s.png", cname);
            if (COM_FileExists (mspath, NULL))
                mappic = Draw_TryCachePic (mspath, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
            else
                mappic = Draw_TryCachePic ("gfx/maps/custom.png", TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
        }
        else
        {
            mspath = va("gfx/maps/%s.png", (*M_Maps_ChosenDisplay ()).thumbnail);
            if (COM_FileExists (mspath, NULL))
                mappic = Draw_TryCachePic (mspath, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
        }
    }
    if (!mappic)
        mappic = Draw_CachePic(va("gfx/maps/%s.%s", (*M_Maps_ChosenDisplay ()).thumbnail, format));
    int fg_height = MM_HEIGHT_PIX - (MM_HEADER_HEIGHT + MM_FOOTER_HEIGHT +mappic->height);

    //background
    if (matchmaking_cursor >= NUM_GAMEOPTIONS)
        matchmaking_cursor = NUM_GAMEOPTIONS - 1;   /* perf B7: stale index lands on START MATCHMAKING */
    draw_grid_inset = 1;          /* r77: 20% side margin (user) */
    struct MenuCoords mc = Draw_WindowGridLeft("Custom Games",14,MVS*0.8f,2,0.2, 0.8, matchmaking_cursor, 1.0, 3.0);   /* r79b: edge to edge */
    draw_grid_inset = 0;
    // map thumbnail -- r79/r80 (user): panel stretches top to bottom;
    // nav hints (Back/Select/Change) live in the footer bar and the map +
    // action-hint bar stack up from it, hugging the bottom.
    double bar_x = mc.grid[0][0].x;
    double bar_w = (mc.grid[mc.cols - 1][0].x + mc.colw) - bar_x + 2;
    /* r80 (user: hint rows + map should "stack hugging the bottom" while
     * the three rows "cling to the top"): lay the block out from the
     * footer upward -- bottom hint bar (the footer itself), the full 16:9
     * map touching it from above, and the action-hint bar touching the
     * map. Any slack lands between the menu rows and the top hint bar.
     * r79b's no-distortion rule stands: the whole image is shown; the pic
     * height only clamps if the window can't fit bars + art at all. */
    int pic_bottom = (int)mc.grid[1][mc.rows].y;
    int pic_fit = (int)((bar_w - 2) / 1.7);
    int room = pic_bottom - (int)mc.grid[0][3].y - 2 * (int)mc.rowh;
    int pic_yfit;
    int topbar;
    double pic_x = bar_x + 1;
    if (pic_fit > room)
        pic_fit = room;
    pic_yfit = pic_bottom - pic_fit;
    topbar = pic_yfit - (int)mc.rowh;
    /* r78 note kept: pic_x centers in the REAL window bounds (r77 margin).
     * r79 stretches the pic to the interior instead of keeping 16:9 so it
     * can touch the bars (user). */
    Draw_FillRGBA ((int)bar_x, topbar, (int)bar_w, (int)mc.rowh,
                   20/255.0f, 25/255.0f, 44/255.0f, 0.85f);
    Draw_StretchPic(pic_x, pic_yfit, mappic, bar_w - 2, pic_fit);
    M_Draw_HintY(MATCH_BOTTOM, mc, topbar + 5);   /* actions on top */
    {
        int botbar = (int)mc.grid[0][mc.rows].yp;
        /* r161 (user: "the arrows and select are still not conditional"): the
         * bar follows the row again -- PLAYLIST is changed with left/right,
         * every other row is entered with A. */
        if (matchmaking_cursor == 1)
            M_Draw_HintY(MATCH_SCROLL_TOP, mc, botbar);
        else
            M_Draw_HintY(MATCH_MAIN_TOP, mc, botbar);
    }

    //======================== 0
    //force update protocol char[]
    M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][0].yp, "NETWORK");
    switch (IPXConfig) {
        case true:
            protocol = "IPX";
            break;
        case false:
            protocol = "TCP/IP";
            break;
    }
    M_Print (mc.grid[1][0].xp, mc.grid[1][0].yp, protocol);
    //======================== 1
    M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][1].yp, "PLAYLIST");
    switch(opts.gametype)
    {
        case 0:
            M_Print(mc.grid[1][0].xp, mc.grid[1][1].yp, "Slayer");
            break;
        case 1:
            M_Print(mc.grid[1][0].xp, mc.grid[1][1].yp, "Team Slayer");
            break;
        case 2:
            M_Print(mc.grid[1][0].xp, mc.grid[1][1].yp, "Swat");
            break;
    }
    //======================== 2 (r77: score/time/options/map rows removed
    //                     -- they live behind square/triangle now)
    /* r78: full map title under the thumbnail -- the old fixed 12-char
     * buffer chopped "Narrows (wip)" to "Narrows (" (user: "cut off too
     * short"). Header face, auto-shrinks until it fits the panel width,
     * never truncated. */
    {
        const char *mapdesc = chosen_level.episode == NUM_EPISODES - 1
            ? ((int)chosen_level.level < custom_count ? custom_maps[chosen_level.level].disp : "Custom map")
            : (*M_Maps_ChosenDisplay ()).description;
        float mapname_size = 16.0f;
        int mapname_x = (int)pic_x + CHARZ;
        double mapname_avail = (mc.grid[mc.cols - 1][0].x + mc.colw) - mapname_x - 8;
        while (mapname_size > 9.0f && Draw_TTFHeaderWidth (mapdesc, mapname_size) > mapname_avail)
            mapname_size -= 0.5f;
        /* r79: overlaps the picture now that the pic touches the bars --
         * shadow pass keeps it legible on bright thumbnails. */
        Draw_TTFStringHeader (mapname_x, pic_yfit + pic_fit - 9, mapdesc, 1, 1, 1, 1, mapname_size);   /* r90: single solid draw -- the +1px black ghost read as a duplicate name (user) */
    }
    //========================= 6
    /* P2a round 4b: was a raw Draw_ColoredStringScale conchars run (the old
     * blocky look, user 2026-11-12); route through the TTF funnel like the
     * SELECT MAP row above and the Firefight START GAME row. */
    M_PrintWhite (mc.grid[0][2].xp, mc.grid[1][2].yp, "START MATCHMAKING");

    /* P2a lobby roster: virtual menu space is (320*MENU_SCALE) x
     * (200*MENU_SCALE) stretched to the window (NOT raw glwidth pixels;
     * coords past 560x350 project off-screen). Spec (user 2026-11-12):
     * top-right, 20% below the top, right edge 15% in from the right
     * limit. Default 3 bots + you = 4/8; friends = floor(n/2) mirrors
     * populate_bots in client.qc so team games balance (2v2 by default).
     * Names from bot_nameK cvars (bots.cfg execed on open); unset slots
     * fall back to "Bot k". */
    {
        extern qpic_t *b_lt, *b_rt;   /* L/R trigger butticon art */
        int rw = (int)(80 * MENU_SCALE);
        int rx = (int)(320 * MENU_SCALE) - 55 - rw;   /* r58: right-side gap 84 -> 55 (-34%) */
        int hh = 24;          /* r77: header bigger again (user) */
        int ry = (int)mc.grid[0][0].y - hh;   /* r76b: strip bottom flush with the Matchmaking header bottom */
        int n = (int)(lobby_bots ? lobby_bots->value : 3);
        int fr = (opts.gametype == 1) ? n / 2 : n;
        int en = n - fr;
        int pitch = 20;       /* r76b: was 29 */
        int band = 17;        /* r76b: was 25 */
        int nfs = 13;         /* r76b: was 19 */
        int y;
        int i;
        int hx, hy;
        /* r62/r115: FFA band pool now lives at file scope (M_Matchmaking_f
         * pins it into bot_color<slot> so QC matches this preview). */
        /* r96: FFA preview follows the player's profile band (bottomcolor).
         * 15 is the fullbright glitch band and >14 parses garbage: pin to 14. */
        int prof = atoi (Cvar_VariableString ("bottomcolor"));
        if (prof < 0 || prof > 13)    /* r97: 14/15 ultrabright tail */
            prof = 4;

        Draw_FillRGBA (rx, ry, rw, hh, 20/255.0f*0.85f, 25/255.0f*0.85f, 44/255.0f*0.85f, 0.85f);
        Draw_TTFStringHeader (rx + CHARZ - 2, ry + 18, "ROSTER", 1, 1, 1, 0.9f, 14.0f);
        Draw_TTFString (rx + rw - 8 - (int)Draw_TTFWidth (va("%i/8", n + 1), 14.0f), ry + 18,
                        va("%i/8", n + 1), 1, 1, 1, 1, 14.0f);

        // you are always on the friends (red) side; r58: the team band
        // spans the full column behind the name; r76: rows sized to the
        // cursor height, names drawn straight in 19px Archivo.
        y = ry + hh + 2;
        if (opts.gametype == 1)
            Draw_FillRGBA (rx, y, rw, band, 0.78f*0.35f, 0.12f*0.35f, 0.14f*0.35f, 0.35f);
        else
        {   /* r96: Slayer roster shows the player's chosen profile color */
            byte *pcc = (byte *)&d_8to24table[prof * 16 + 8];
            Draw_FillRGBA (rx, y, rw, band, pcc[0]/255.0f*0.35f, pcc[1]/255.0f*0.35f, pcc[2]/255.0f*0.35f, 0.35f);
        }
        /* r113: roster shows the player's real name (Spartan page "Your
         * name" / cl_name), not a hardcoded "You". */
        {
            const char *me = cl_name.string;
            Draw_TTFString (rx + CHARZ, y + nfs, (*me) ? me : "You", 1, 1, 1, 1, (float)nfs);
        }
        y += pitch;

        for (i = 0; i < fr; i++)
        {
            const char *nm = Cvar_VariableString (va("bot_name%d", i));
            if (opts.gametype == 1)
            {
                Draw_FillRGBA (rx, y, rw, band, 0.78f*0.35f, 0.12f*0.35f, 0.14f*0.35f, 0.35f);
            }
            else
            {   /* r62: Slayer/FFA rows use each bot's own palette band
                 * instead of one flat team red. Actual in-game colors come
                 * from FindAnotherColor (collision-avoiding) so a row may
                 * occasionally differ from this preview. */
                int bandx = ffa_bands[i % 6];
                if (bandx == prof)
                    bandx = 8;   /* r96: never preview the player's own band */
                byte *pc = (byte *)&d_8to24table[bandx * 16 + 8];
                Draw_FillRGBA (rx, y, rw, band, pc[0]/255.0f*0.35f, pc[1]/255.0f*0.35f, pc[2]/255.0f*0.35f, 0.35f);
            }
            Draw_TTFString (rx + CHARZ, y + nfs, (*nm) ? nm : va("Bot %i", i + 1), 1, 1, 1, 1, (float)nfs);
            y += pitch;
        }
        for (i = 0; i < en; i++)
        {
            const char *nm = Cvar_VariableString (va("bot_name%d", fr + i));
            Draw_FillRGBA (rx, y, rw, band, 0.13f*0.35f, 0.36f*0.35f, 0.85f*0.35f, 0.35f);
            Draw_TTFString (rx + CHARZ, y + nfs, (*nm) ? nm : va("Bot %i", fr + i + 1), 1, 1, 1, 1, (float)nfs);
            y += pitch;
        }

        hx = rx + (rw - (M_HintIconW (b_lt) + M_HintIconW (b_rt)) - (int)(Draw_TTFWidth ("Remove", 14.0f) + Draw_TTFWidth ("Add", 14.0f)) - 4*CHARZ) / 2;
        hy = y + 2;
        M_HintIcon(&hx, hy, b_lt); M_HintWord(&hx, hy, "Remove");
        M_HintIcon(&hx, hy, b_rt); M_HintWord(&hx, hy, "Add");
    }

#ifndef VITA
    /* r81: PC key legend -- what the Vita pad buttons map to here */
    {
        float ls = 12.0f;
        int lrx = (int)(320 * MENU_SCALE) - 8;
        const char *leg1 = "Enter:Cross  Esc:Circle  g:Square";
        const char *leg2 = "m:Triangle  [:L1  ]:R1";
        Draw_TTFString (lrx - (int)Draw_TTFWidth (leg1, ls), (int)(200 * MENU_SCALE) - 22, leg1, 0.65, 0.70, 0.80, 0.85f, ls);
        Draw_TTFString (lrx - (int)Draw_TTFWidth (leg2, ls), (int)(200 * MENU_SCALE) - 8, leg2, 0.65, 0.70, 0.80, 0.85f, ls);
    }
#endif
}

void M_Matchmaking_Submenu_Key (int dir)
{
    int epcount, levcount;
    switch (matchmaking_cursor)
    {
        case 1: //gametype
            opts.gametype += dir;
            if (opts.gametype > 2)
                opts.gametype = 0;
            if (opts.gametype < 0 && dir < 0)
                opts.gametype = 2;
            opts.fraglimit = (opts.gametype == 1) ? 20 : 10;   /* r77: per-mode score defaults */
            /* r94 used to reset the player count here (TDM 5 / others 3); r117
             * (user): switching modes KEEPS the chosen count, and the picked
             * mode is archived so relaunches reopen the lobby where you left. */
            Cvar_SetValue ("lobby_gametype", (float)opts.gametype);
            break;
        case 2: //score limit
            opts.fraglimit += dir*5;
            if (opts.fraglimit > 100)
                opts.fraglimit = 0;
            if (opts.fraglimit < 0)
                opts.fraglimit = 100;
            break;
        case 3: //time limit
            opts.timelimit += dir*5;
            if (opts.timelimit > 60)
                opts.timelimit = 0;
            if (opts.timelimit < 0)
                opts.timelimit = 60;
            break;
        case 5: //map
            startlevel += dir;
            levcount = episodes[startepisode].levels;

            if (startlevel < 0)
                startlevel = levcount - 1;

            if (startlevel >= levcount)
                startlevel = 0;
            break;
    }
}

void M_Matchmaking_Key (int key)
{
    switch (key)
    {
#ifdef VITA
        case K_ENTER:
#endif
        case K_ESCAPE:
        case K_BBUTTON:
            M_Menu_Main_f ();
            break;

        /* r77 (user): square opens game options, triangle opens the map
         * list; those two rows are gone from the main page. */
        case K_XBUTTON:
            M_Matchmaking_Gamerules_f ();
            break;
        case K_YBUTTON:
            M_Matchmaking_Map_f ();
            break;
#ifndef VITA
        /* r78: Mac/PC has no square/triangle keys -- g/m are unbound in
         * default.cfg + config.cfg so they reach M_Keydown raw and open
         * the same two submenus the Vita buttons do. */
        case 'g':
        case 'G':
            M_Matchmaking_Gamerules_f ();
            break;
        case 'm':
        case 'M':
            M_Matchmaking_Map_f ();
            break;
#endif

        case K_UPARROW:
            S_LocalSound ("misc/menuoption.wav");
            matchmaking_cursor--;
            if (matchmaking_cursor < 0)
                matchmaking_cursor = NUM_GAMEOPTIONS-1;
            break;

        case K_DOWNARROW:
            S_LocalSound ("misc/menuoption.wav");
            matchmaking_cursor++;
            if (matchmaking_cursor >= NUM_GAMEOPTIONS)
                matchmaking_cursor = 0;
            break;

        case K_LEFTARROW:
            if (matchmaking_cursor != 1)
                break;
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Submenu_Key (-1);
            break;

        case K_RIGHTARROW:
            if (matchmaking_cursor != 1)
                break;
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Submenu_Key (1);
            break;

        case K_ABUTTON:
#ifndef VITA
        case K_ENTER:
        case K_KP_ENTER:
            /* Mac/PC port: Enter selects, like the Vita X button. */
#endif
            if (realtime - mm_opened_time < 0.35)
                break;      /* r118: swallow the open press / pad repeat */
            S_LocalSound ("misc/menuenter.wav");
            if (matchmaking_cursor == 2)
            {
//                S_LocalSound ("misc/menuback.wav"); // why was this here?
                match_return_state = m_gameoptions;	/* r164: exit lands in Custom Games */

                /*
                 * Pre-setup. Disconnect form last match, etc.
                 */
                // make sure we disconnected from lat match
                if (sv.active) {
                    Cbuf_AddText ("disconnect\n");
                }
                Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
                Cbuf_AddText ( va ("maxplayers %u\n", 8) );
                SCR_BeginLoadingPlaque ();
                Cbuf_AddText ("chase_active 0\n");
                /*
                 * Setup based on user selected settings
                 */
                switch (opts.gametype) {
                    case 0: // Slayer
                        opts.deathmatch = 1;
                        opts.teamplay = 0;
                        opts.coop = 0;
                        break;
                    case 1: // Team Slayer
                        opts.deathmatch = 1;
                        opts.teamplay = 1;
                        opts.coop = 0;
                        break;
                    case 2: // Swat
                        opts.deathmatch = 3;
                        opts.teamplay = 0;
                        opts.coop = 0;
                        break;
                }
                Cbuf_AddText ( va ("deathmatch %u\n", (int)opts.deathmatch) );
                Cbuf_AddText ( va ("teamplay %u\n", (int)opts.teamplay) );
                /* r96: Team Slayer is always red vs blue (user). Pin the human
                 * to band 14; bots hold 14/13 to match, so .team ids line up.
                 * FFA/Swat restore the saved profile colors. */
                if (opts.gametype == 1)
                {
                    Cbuf_AddText ("bottomcolor 4\n");
                    Cbuf_AddText ("topcolor 4\n");
                }
                else if (lobby_profile_saved && lobby_saved_bottom[0])
                {
                    Cbuf_AddText (va ("bottomcolor %s\n", lobby_saved_bottom));
                    Cbuf_AddText (va ("topcolor %s\n", lobby_saved_top));
                }
                /* r67: in Team Slayer the menu "score limit" drives the TEAM
                 * score; an individual fraglimit would race it and end the
                 * match on one player's streak. */
                if (opts.gametype == 1)
                    Cbuf_AddText ( va ("fraglimit %u\n", 0) );
                else
                    Cbuf_AddText ( va ("fraglimit %u\n", (int)opts.fraglimit) );
                Cbuf_AddText ( va ("timelimit %u\n", (int)opts.timelimit) );
                if (opts.gametype == 1)
                    Cbuf_AddText ( va ("teamlimit %u\n", (int)opts.fraglimit) );
                else
                    Cbuf_AddText ( va ("teamlimit %u\n", (int)opts.teamlimit) );
                // custom game options
                Cbuf_AddText(va ("infinite_ammo %u\n", (int)opts.infinite_ammo));
                Cbuf_AddText(va ("map_weapons_disable %u\n", (int)opts.map_weapons_disable));
                Cbuf_AddText(va ("grunt_birthday_party %u\n", (int)opts.grunt_birthday_party));
                Cbuf_AddText(va ("max_nades %u\n", (int)opts.max_nades));
                Cbuf_AddText(va ("custom_primary %u\n", gamerule_weapons[(int)opts.custom_primary].weapon));
                Cbuf_AddText(va ("custom_secondary %u\n", gamerule_weapons_secondary[(int)opts.custom_secondary].weapon));
                Cbuf_AddText(va ("sol_gravity %u\n", (int)opts.sol_gravity));
                Cbuf_AddText(va ("sol_maxspeed %u\n", (int)opts.sol_maxspeed));
                Cbuf_AddText(va ("sol_jump %u\n", (int)opts.sol_jump));
                // difficulty
                Cbuf_AddText ( va ("skill %u\n", (int)opts.skill) );
                // map
                if (chosen_level.episode == NUM_EPISODES - 1)	// perf B7: custom pack -- launch from the folder scan
        {
            if (chosen_level.level < custom_count)
                Cbuf_AddText ( va ("map custom/%s\n", custom_maps[chosen_level.level].name) );
        }
        else
            Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + chosen_level.level].name) );
                // r123: dismiss the menu the moment Start is chosen (vanilla
                // M_ChangeLevel pattern). Leaving key_dest == key_menu until the
                // queued `map` executes meant any ESC/pause in that window hit
                // M_ToggleMenu's "submenu -> main menu" branch.
                m_entersound = false;
                key_dest = key_game;
                m_state = m_none;

                return;
            } else if (matchmaking_cursor == 0) {
                m_multiplayer_cursor = 1;
                M_Menu_LanConfig_f();
            }

            // todo: safe to remove the below line?
            M_Matchmaking_Submenu_Key (1);
            break;

        /* P2a lobby roster: shoulders (Vita) / brackets (Mac) adjust the
         * bot count in the matchmaking menu only; the config.cfg binds
         * keep them no-ops elsewhere and stop "is unbound" spam. Brackets
         * arrive raw here because bound keys do not execute in menus. */
        case K_LSHOULDER:
        case '[':
            if (lobby_bots && lobby_bots->value > 0)
            {
                Cvar_SetValue ("lobby_bots", lobby_bots->value - 1);
                S_LocalSound ("misc/menuoption.wav");
            }
            break;

        case K_RSHOULDER:
        case ']':
            if (lobby_bots && lobby_bots->value < 7)
            {
                Cvar_SetValue ("lobby_bots", lobby_bots->value + 1);
                S_LocalSound ("misc/menuoption.wav");
            }
            break;
    }
}

void M_Menu_Firefight_f (void)
{
    key_dest = key_menu;
    m_state = m_firefight;
    m_entersound = true;
    //default firefight settings
    Cvar_SetValue ("skill", 0);
    Cvar_SetValue ("deathmatch", 2);
    startepisode = 0;
    startlevel = rand() % episodes_ff[startepisode].levels;
    PlayMenuTheme(false);
}

int firefight_cursor_table[] = {40, 56, 64, 72, 80, 88, 96, 112, 120};
#define	NUM_FIREFIGHT_OPTIONS	2
#define MM_FFOPS_HEIGHT mvs(NUM_FIREFIGHT_OPTIONS)
int		firefight_cursor;


void M_Firefight_Draw (void)
{
    M_Maps_WarmOne ();	// r120
    Draw_MenuBg();

    /* r82: Firefight restyled to match the r79/r80 matchmaking window --
     * edge-to-edge, rows top-hugged, map + hint bars bottom-stacked. */
    draw_grid_inset = 1;
    struct MenuCoords mc = Draw_WindowGridLeft("Firefight",14,MVS*0.8f,2,0.2, 0.8, firefight_cursor, 1.0, 3.0);
    draw_grid_inset = 0;

    M_Print (mc.grid[0][0].xp, mc.grid[0][0].yp, "MAP");
    M_PrintWhite (mc.grid[1][0].xp - mc.colw/2, mc.grid[1][0].yp, levels[episodes_ff[startepisode].firstLevel + startlevel].description);
    M_PrintWhite (mc.grid[0][1].xp, mc.grid[0][1].yp, "START FIREFIGHT");

    {
        double bar_x = mc.grid[0][0].x;
        double bar_w = (mc.grid[mc.cols-1][0].x + mc.colw) - bar_x + 2;
        int pic_bottom = (int)mc.grid[1][mc.rows].y;
        int pic_fit = (int)((bar_w - 2) / 1.7);
        int room = pic_bottom - (int)mc.grid[0][3].y - (int)(2*mc.rowh);
        int pic_yfit, topbar;
        qpic_t *mappic;
        const char *mapdesc;
        float mapname_size = 16.0f;
        int mapname_x;
        double mapname_avail;

        if (pic_fit > room)
            pic_fit = room;
        pic_yfit = pic_bottom - pic_fit;
        topbar = pic_yfit - (int)mc.rowh;

        Draw_FillRGBA ((int)bar_x, topbar, (int)bar_w, (int)mc.rowh, 20/255.0, 25/255.0, 44/255.0, 0.85);
        {
            const char *mspath = va("gfx/maps/%s.png", levels[episodes_ff[startepisode].firstLevel + startlevel].thumbnail);
            mappic = NULL;
            if (COM_FileExists (mspath, NULL))
                mappic = Draw_TryCachePic (mspath, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
        }
        if (!mappic)
            mappic = Draw_CachePic (va("gfx/maps/%s.lmp", levels[episodes_ff[startepisode].firstLevel + startlevel].thumbnail));
        Draw_StretchPic ((int)(bar_x + 1), pic_yfit, mappic, (int)(bar_w - 2), pic_fit);

        /* bottom hint bar (window footer): Back + Select/Change.
         * r161 (user): conditional -- row 0 is changed with left/right. */
        if (firefight_cursor == 0)
            M_Draw_HintY (MATCH_SCROLL_TOP, mc, (int)mc.grid[0][mc.rows].yp);
        else
            M_Draw_HintY (MATCH_MAIN_TOP, mc, (int)mc.grid[0][mc.rows].yp);

        /* top hint bar: no extra actions in Firefight -- skip MATCH_BOTTOM */

        /* map name overlapping the pic, auto-shrink (matchmaking style) */
        mapdesc = levels[episodes_ff[startepisode].firstLevel + startlevel].description;
        mapname_x = (int)(bar_x + 1) + CHARZ;
        mapname_avail = (mc.grid[mc.cols-1][0].x + mc.colw) - mapname_x - 8;
        while (mapname_size > 9.0f && Draw_TTFHeaderWidth (mapdesc, mapname_size) > mapname_avail)
            mapname_size -= 0.5f;
        Draw_TTFStringHeader (mapname_x, pic_yfit + pic_fit - 9, mapdesc, 1, 1, 1, 1, mapname_size);   /* r90: single solid draw -- the +1px black ghost read as a duplicate name (user) */
    }
}

void M_Firefight_Change (int dir)
{
    int epcount, levcount;
    switch (firefight_cursor)
    {
        case 0: //map
            startlevel += dir;
            levcount = episodes_ff[startepisode].levels;

            if (startlevel < 0)
                startlevel = levcount - 1;

            if (startlevel >= levcount)
                startlevel = 0;
            break;
    }
}

void M_Firefight_Key (int key)
{
    switch (key)
    {
#ifdef VITA
        case K_ENTER:
#endif
        case K_ESCAPE:
        case K_YBUTTON:
        case K_BBUTTON:
            M_Menu_Main_f ();
            break;

        case K_UPARROW:
            S_LocalSound ("misc/menuoption.wav");
            firefight_cursor--;
            if (firefight_cursor < 0)
                firefight_cursor = NUM_FIREFIGHT_OPTIONS-1;
            break;

        case K_DOWNARROW:
            S_LocalSound ("misc/menuoption.wav");
            firefight_cursor++;
            if (firefight_cursor >= NUM_FIREFIGHT_OPTIONS)
                firefight_cursor = 0;
            break;

        case K_LEFTARROW:
            if (firefight_cursor == 1)
                break;
            S_LocalSound ("misc/menuoption.wav");
            M_Firefight_Change (-1);
            break;

        case K_RIGHTARROW:
            if (firefight_cursor == 1)
                break;
            S_LocalSound ("misc/menuoption.wav");
            M_Firefight_Change (1);
            break;

        case K_ABUTTON:
#ifndef VITA
        case K_ENTER:
        case K_KP_ENTER:
            /* Mac/PC port: Enter selects, like the Vita X button. */
#endif
            S_LocalSound ("misc/menuenter.wav");
            if (firefight_cursor == 1) //on start options
            {
                match_return_state = m_firefight;	/* r164 */
                if (sv.active)
                    Cbuf_AddText ("disconnect\n");
                SCR_BeginLoadingPlaque (); //#TODO: fix loading screen not working.
                Cbuf_AddText ("timelimit 60\n");
                Cbuf_AddText ("fraglimit 9999\n");
                Cbuf_AddText ("listen 0\n");	// so host_netport will be re-examined
                Cbuf_AddText ("chase_active 0\n");
                Cbuf_AddText ( va ("maxplayers %u\n", 1) );
                Cbuf_AddText ( va ("deathmatch %u\n", (int)deathmatch.value) );
                Cbuf_AddText ( va ("map %s\n", levels[episodes_ff[startepisode].firstLevel + startlevel].name) );

                return;
            }
            break;
    }
}

//=============================================================================

//=============================================================================
/* r137: FIND MATCH -- online lobby browser.
 * r143 (user): the screen has to look like Custom Games: NETWORK, PLAYLIST and
 * START MATCHMAKING on three rows, then a map card for the best (or the
 * highlighted) lobby and the local player in the roster. Square opens a lobby
 * list on top of this screen, the way Map Selection layers its list over the
 * Custom Games card, so the preview follows the highlight. */

#define LOBBY_FILTERS   2
#define LOBBY_BROWSE_ROWS 8

static int	lobby_cursor = LOBBY_FILTERS;
static int	lobby_network = 0;		/* 0 Both, 1 Local, 2 Online */
static int	lobby_playlist = 0;		/* 0 All, 1 Slayer, 2 Team Slayer, 3 Swat */
static int	lobby_browse = 0;		/* square: lobby list overlay */
static int	lobby_browsecursor = 0;
static int	lobby_browsefirst = 0;
static int	lobby_selected = -1;	/* r153: slot picked in the browser */
static double	lobby_rescan_time = 0;	/* r147: auto-retry while empty */
static double	lobby_rescan_delay = 1.0;	/* r148: exponential backoff */

/* r167 (user): the browser can order and filter the list. The list is a
 * filtered index map rebuilt on demand; hostCacheCount is small (a LAN sweep
 * plus a master list), so sorting it per frame is cheaper than tracking
 * invalidation across the asynchronous replies. */
#define LOBBY_ORDER_MAX 96
static int	lobby_sort = 0;		/* 0 real players, 1 players+bots, 2 title, 3 mode */
static int	lobby_order[LOBBY_ORDER_MAX];
static int	lobby_ordercount = 0;

/* r137/r143: the browser exposes its rows through NET_SlistServerInfo, because
 * menu.c cannot include net_defs.h (it needs arch_def/q_stdinc first). */
static const char *M_Lobby_Name (int i)
{
	const char *name = "";
	const char *map = "";

	NET_SlistServerInfo ((size_t)i, &name, &map, NULL, NULL);
	return name;
}

static const char *M_Lobby_Map (int i)
{
	const char *name = "";
	const char *map = "";

	NET_SlistServerInfo ((size_t)i, &name, &map, NULL, NULL);
	return map;
}

static int M_Lobby_Users (int i)
{
	int	users = 0;

	NET_SlistServerInfo ((size_t)i, NULL, NULL, &users, NULL);
	return users;
}

static int M_Lobby_Max (int i)
{
	int	maxusers = 0;

	NET_SlistServerInfo ((size_t)i, NULL, NULL, NULL, &maxusers);
	return maxusers;
}

static const char *M_Lobby_NetworkName (void)
{
	switch (lobby_network)
	{
	case 1:		return "Local";
	case 2:		return "Online";
	default:	return "Both";
	}
}

static const char *M_Lobby_PlaylistName (void)
{
	switch (lobby_playlist)
	{
	case 1:		return "Slayer";
	case 2:		return "Team Slayer";
	case 3:		return "Swat";
	default:	return "All";
	}
}

/* Both = broadcast + masters, Local = broadcast, Online = masters only. */
static enum slistScope_e M_Lobby_Scope (void)
{
	switch (lobby_network)
	{
	case 1:		return SLIST_LAN;
	case 2:		return SLIST_ONLINE;
	default:	return SLIST_INTERNET;
	}
}

/* Servers advertise sv_hostname_fmt "%mode%", so the hostname IS the mode. */
static qboolean M_Lobby_NameIs (const char *name, const char *want)
{
	size_t n = strlen (want);

	if (!name || !*name)
		return false;
	if (q_strncasecmp (name, want, n) != 0)
		return false;
	return (name[n] == '\0' || name[n] == ' ' || name[n] == ':' || name[n] == '-');
}

static int M_Lobby_Matches (int i)
{
	if (i < 0 || (size_t)i >= hostCacheCount)
		return 0;
	if (lobby_playlist == 0)
		return 1;
	if (lobby_playlist == 1)
		return M_Lobby_NameIs (M_Lobby_Name (i), "Slayer");
	return M_Lobby_NameIs (M_Lobby_Name (i), M_Lobby_PlaylistName ());
}

/* r167: how many of the advertised users are bots. The server-info reply only
 * carries network clients today, so this is 0 until the broadcast update adds
 * a bot count; the sort mode exists now so the UI does not change shape later. */
/* r175: metadata helpers. Servers older than r175 do not send the trailing
 * browse block, so title/game/desc fall back to the hostname (which carries
 * the mode) and the map. */
static const char *M_Lobby_Title (int i)
{
	const char *title = "";

	NET_SlistServerMeta ((size_t)i, &title, NULL, NULL, NULL, NULL, NULL, NULL);
	return title[0] ? title : M_Lobby_Name (i);
}

static const char *M_Lobby_Game (int i)
{
	const char *game = "";

	NET_SlistServerMeta ((size_t)i, NULL, NULL, &game, NULL, NULL, NULL, NULL);
	return game[0] ? game : M_Lobby_Name (i);
}

static const char *M_Lobby_Desc (int i)
{
	const char *desc = "";

	NET_SlistServerMeta ((size_t)i, NULL, &desc, NULL, NULL, NULL, NULL, NULL);
	return desc;
}

static int M_Lobby_Bots (int i)
{
	int	bots = 0;

	NET_SlistServerMeta ((size_t)i, NULL, NULL, NULL, &bots, NULL, NULL, NULL);
	return bots;
}

static int M_Lobby_Rule (int i)
{
	int	rule = 0;

	NET_SlistServerMeta ((size_t)i, NULL, NULL, NULL, NULL, &rule, NULL, NULL);
	return rule;
}

/* r176 (user: "for game info we should also broadcast the game length and max
 * kills") -- the round shape the server advertises via \time\ and \limit\.
 * 0 means the server did not say (older build) or the limit is off. */
static int M_Lobby_Time (int i)
{
	int	minutes = 0;

	NET_SlistServerMeta ((size_t)i, NULL, NULL, NULL, NULL, NULL, &minutes, NULL);
	return minutes;
}

static int M_Lobby_MaxKills (int i)
{
	int	kills = 0;

	NET_SlistServerMeta ((size_t)i, NULL, NULL, NULL, NULL, NULL, NULL, &kills);
	return kills;
}

static int M_Lobby_ModeRank (int i)
{
	if (M_Lobby_NameIs (M_Lobby_Name (i), "Team Slayer"))
		return 0;
	if (M_Lobby_NameIs (M_Lobby_Name (i), "Slayer"))
		return 1;
	if (M_Lobby_NameIs (M_Lobby_Name (i), "Swat"))
		return 2;
	return 3;
}

static const char *M_Lobby_SortName (void)
{
	switch (lobby_sort)
	{
	case 1:		return "Bots";
	case 2:		return "Title";
	case 3:		return "Mode";
	default:	return "Players";
	}
}

static int M_Lobby_Compare (const void *pa, const void *pb)
{
	int	a = *(const int *)pa;
	int	b = *(const int *)pb;
	int	c = 0;

	switch (lobby_sort)
	{
	case 2:		/* title */
		c = q_strcasecmp (M_Lobby_Name (a), M_Lobby_Name (b));
		break;
	case 3:		/* gamemode, then title */
		c = (int)M_Lobby_ModeRank (a) - (int)M_Lobby_ModeRank (b);
		break;
	case 1:		/* players including bots, high to low */
		c = (M_Lobby_Users (b) + M_Lobby_Bots (b)) -
		    (M_Lobby_Users (a) + M_Lobby_Bots (a));
		break;
	default:	/* real players, high to low (r167 default) */
		c = M_Lobby_Users (b) - M_Lobby_Users (a);
		break;
	}
	if (!c)
		c = q_strcasecmp (M_Lobby_Name (a), M_Lobby_Name (b));
	if (!c)
		c = a - b;
	return c;
}

static void M_Lobby_Rebuild (void)
{
	int	i, n = 0;

	for (i = 0; (size_t)i < hostCacheCount && n < LOBBY_ORDER_MAX; i++)
		if (M_Lobby_Matches (i))
			lobby_order[n++] = i;
	lobby_ordercount = n;
	if (n > 1)
		qsort (lobby_order, n, sizeof (lobby_order[0]), M_Lobby_Compare);
}

static int M_Lobby_Count (void)
{
	M_Lobby_Rebuild ();
	return lobby_ordercount;
}

static int M_Lobby_IndexAt (int slot)
{
	M_Lobby_Rebuild ();
	if (slot < 0 || slot >= lobby_ordercount)
		return -1;
	return lobby_order[slot];
}

static int M_Lobby_Best (void)
{
	int	i, best = -1, bestusers = -1;

	for (i = 0; (size_t)i < hostCacheCount; i++)
		if (M_Lobby_Matches (i) && M_Lobby_Users (i) > bestusers)
		{
			bestusers = M_Lobby_Users (i);
			best = i;
		}
	return best;
}

static qpic_t *M_Lobby_MapPic (int i)
{
	const char	*map = (i >= 0 && (size_t)i < hostCacheCount) ? M_Lobby_Map (i) : NULL;
	const char	*path;
	int		k;

	if (map && *map)
	{
		for (k = 0; k < (int)(sizeof(levels) / sizeof(levels[0])); k++)
			if (!q_strcasecmp (levels[k].name, map))
			{
				qpic_t *png;

				path = va ("gfx/maps/%s.png", levels[k].thumbnail);
				if (COM_FileExists (path, NULL))
				{
					png = Draw_TryCachePic (path, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
					if (png)
						return png;
				}
				/* r162 (user: "the map art is not loading"): most of the
				 * pack ships only legacy art -- lockdown.tga, pit.lmp -- and
				 * PNG-only lookups dropped straight to the "?" custom card.
				 * Same fallback Custom Games' map select uses. */
				return Draw_CachePic (va ("gfx/maps/%s.%s", levels[k].thumbnail,
				                          levels[k].targaThumbnail ? "tga" : "lmp"));
			}
		path = va ("gfx/maps/%s.png", map);
		if (COM_FileExists (path, NULL))
			return Draw_TryCachePic (path, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
	}
	if (COM_FileExists ("gfx/maps/custom.png", NULL))
		return Draw_TryCachePic ("gfx/maps/custom.png", TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
	return Draw_CachePic ("gfx/loading.tga");
}

/* r146 (user): the browse card shows the map's blurb + author, same data the
 * map selection screen reads from levels[]. Unknown maps (custom packs) have
 * neither, so callers fall back to the placeholder art and no subtitle. */
static const char *M_Lobby_MapAuthor (int i)
{
	const char	*map = M_Lobby_Map (i);
	int		k;

	if (!map || !*map)
		return "";
	for (k = 0; k < (int)(sizeof(levels) / sizeof(levels[0])); k++)
		if (!q_strcasecmp (levels[k].name, map) && levels[k].author)
			return levels[k].author;
	return "";
}

static const char *M_Lobby_MapDesc (int i)
{
	const char	*map = M_Lobby_Map (i);
	int		k;

	if (!map || !*map)
		return "";
	for (k = 0; k < (int)(sizeof(levels) / sizeof(levels[0])); k++)
		if (!q_strcasecmp (levels[k].name, map) && levels[k].description)
			return levels[k].description;
	return "";
}

/* r169 (user: "there should be like boxes of your option below the header
 * before the list"): the filter and sort pickers are chips, one active, drawn
 * straight under the popup header. Widths follow the label so "Team Slayer"
 * is not truncated. */
static void M_Lobby_Chips (int x, int y, int h, const char **labels, int count, int active)
{
	int	i, cx = x;

	for (i = 0; i < count; i++)
	{
		int		w = (int)Draw_TTFWidth (labels[i], 11.0f) + 12;
		qboolean	on = (i == active);

		if (on)
			Draw_FillRGBA (cx, y, w, h, 0.24f, 0.37f, 0.72f, 0.95f);
		else
			Draw_FillRGBA (cx, y, w, h, 0.10f, 0.13f, 0.22f, 0.95f);
		Draw_TTFString (cx + 6, y + h - 5, labels[i], 1, 1, 1,
				on ? 1.0f : 0.6f, 11.0f);
		cx += w + 4;
	}
}

/* r171: the chip rows live in their own header module, so the layout needs to
 * know how wide a row is before drawing it (module is right-aligned and each
 * row is centred inside it). */
static int M_Lobby_ChipsWidth (const char **labels, int count)
{
	int	i, w = 0;

	for (i = 0; i < count; i++)
		w += (int)Draw_TTFWidth (labels[i], 11.0f) + 12 + (i ? 4 : 0);

	return w;
}

static void M_Lobby_Scan (void)
{
	slistSilent = true;
	slistScope = M_Lobby_Scope ();
	lobby_selected = -1;		/* r153: a rescan reorders the list */
	NET_Slist_f ();
}

static void M_Lobby_Join (int i)
{
	if (i < 0 || (size_t)i >= hostCacheCount)
		return;
	S_LocalSound ("misc/menu2.wav");
	m_return_state = m_lobbies;
	m_return_onerror = true;
	match_return_state = m_lobbies;	/* r164: a disconnect comes back here */
	key_dest = key_game;
	m_state = m_none;
	IN_UpdateGrabs ();
	/* r163/r164 (user: "connecting to a match freezes, draw the loading screen
	 * asap"): the connect text is only queued here, so put the loading art up
	 * before the first server packet. ShowLoadingScreen (not BeginLoadingPlaque)
	 * keeps drawing live and can be taken down if the connect fails. */
	SCR_ShowLoadingScreen ();
	Cbuf_AddText (va ("connect \"%s\"\n", NET_SlistPrintServerName (i)));
}

void M_Lobbies_f (void)
{
	key_dest = key_menu;
	m_state = m_lobbies;
	m_entersound = true;
	IN_UpdateGrabs ();
	lobby_cursor = LOBBY_FILTERS;
	lobby_rescan_delay = 1.0;
	lobby_browse = 0;
	lobby_browsecursor = lobby_browsefirst = 0;
	lobby_selected = -1;
	M_Lobby_Scan ();
}

/* r164: open the menu the finished match belongs to. Shared by the pause
 * menu's Disconnect row and CL_Disconnect_Multi_f (end of a deathmatch), so
 * both find their way back to Find Match or Custom Games as appropriate. */
void M_ReturnToMatchMenu (void)
{
	/* r178: this is where the pause menu drops you after a match, and none
	 * of the screens it lands on used to ask for the theme -- so leaving a
	 * server through the lobby put you back in a silent menu. */
	PlayMenuTheme (false);
	switch (match_return_state)
	{
	case m_lobbies:
		M_Lobbies_f ();
		break;
	case m_firefight:
		M_Menu_Firefight_f ();
		break;
	case m_slist:
		M_Menu_ServerList_f ();
		break;
	case m_gameoptions:
	default:
		M_Matchmaking_f ();
		break;
	}
}

/* the lobby the card is showing: highlighted one while browsing, else best */
static int M_Lobby_Focus (void)
{
	int	total, n;

	if (!lobby_browse)
	{
		n = M_Lobby_IndexAt (lobby_selected);
		if (n >= 0 && M_Lobby_Matches (n))
			return n;		/* r153: picked in the browser */
		return M_Lobby_Best ();
	}
	total = M_Lobby_Count ();
	if (total <= 0)
		return M_Lobby_Best ();
	if (lobby_browsecursor >= total)
		lobby_browsecursor = total - 1;
	n = M_Lobby_IndexAt (lobby_browsecursor);
	return (n >= 0) ? n : M_Lobby_Best ();
}

/* r176 (user: "the list can go back to just the title (marquee when too
 * long)"): one column of titles. The console font is fixed-width and the menu
 * layer has no scissor, so the scroll is a plain character window -- cheap and
 * clip-free. A three-cell blank gap marks where the title wraps. */
static void M_Lobby_DrawTitle (int x, int y, const char *title, int maxchars, int row)
{
	char	line[96];
	int	len = (int)strlen (title);
	int	i, off;

	if (maxchars < 4)
		maxchars = 4;
	if (maxchars > (int)sizeof (line) - 1)
		maxchars = (int)sizeof (line) - 1;

	if (len <= maxchars)
	{
		M_PrintWhite (x, y, title);
		return;
	}

	/* per-row phase so neighbouring marquees do not tick in lockstep;
	 * 24 px/s = 3 cells/s, the usual terminal-marquee pace */
	off = ((int)(realtime * 24.0) + row * 5) % (len + 3);
	for (i = 0; i < maxchars; i++)
	{
		int	p = (off + i) % (len + 3);

		line[i] = (p < len) ? title[p] : ' ';
	}
	line[maxchars] = 0;
	M_PrintWhite (x, y, line);
}

void M_Lobbies_Draw (void)
{
	int		rows = LOBBY_FILTERS + 1;	/* r143: filters + START, like Custom Games */
	int		total, i, hidx, botbar, topbar;
	int		pic_bottom, pic_fit, room, pic_yfit;
	struct MenuCoords mc;
	qpic_t		*mappic;
	double		bar_x, bar_w, pic_x;

	Draw_MenuBg ();

	total = M_Lobby_Count ();
	/* r147 (user): the scan started on entry can finish before the master
	 * or LAN replies land, leaving an empty list with no search running.
	 * Retry on a timer so the browser fills in without a keypress. */
	if (!slistInProgress && M_Lobby_Count () == 0 && realtime >= lobby_rescan_time)
	{
		lobby_rescan_time = realtime + lobby_rescan_delay;
		/* r149 (user): 1, 4, 8, 15, then settle into a one-minute poll */
		if (lobby_rescan_delay < 3.0)
			lobby_rescan_delay = 4.0;
		else if (lobby_rescan_delay < 7.0)
			lobby_rescan_delay = 8.0;
		else if (lobby_rescan_delay < 12.0)
			lobby_rescan_delay = 16.0;
		else
			lobby_rescan_delay = 60.0;
		M_Lobby_Scan ();
	}
	hidx = M_Lobby_Focus ();
	mappic = M_Lobby_MapPic (hidx);

	if (lobby_cursor >= rows)
		lobby_cursor = rows - 1;
	if (lobby_cursor < 0)
		lobby_cursor = 0;

	draw_grid_inset = 1;
	/* r144: the 2nd argument is the LAYOUT row count that sizes the panel, not
	 * the number of rows we fill -- Custom Games passes 14. Passing our own 4..7
	 * produced a stumpy panel and everything below collided. */
	mc = Draw_WindowGridLeft ("Matchmaking", 14, MVS * 0.8f, 2, 0.2, 0.8,
				  lobby_cursor, 1.0, 3.0);
	draw_grid_inset = 0;

	bar_x = mc.grid[0][0].x;
	bar_w = (mc.grid[mc.cols - 1][0].x + mc.colw) - bar_x + 2;
	pic_bottom = (int)mc.grid[1][mc.rows].y;
	pic_fit = (int)((bar_w - 2) / 1.7);
	/* r145: Custom Games measures the free space from row 3 (the row under the
	 * ones it fills); using our own last row left no room, so pic_fit collapsed
	 * and the map art never drew. */
	room = pic_bottom - (int)mc.grid[0][3].y - 2 * (int)mc.rowh;
	if (pic_fit > room)
		pic_fit = room;
	pic_x = bar_x + 1;
	pic_yfit = pic_bottom - pic_fit;
	topbar = pic_yfit - (int)mc.rowh;

	/* card + roster, exactly the Custom Games stack (room is guaranteed now
	 * that the list lives in the browse overlay instead of this grid) */
	Draw_FillRGBA ((int)bar_x, topbar, (int)bar_w, (int)mc.rowh,
		       20 / 255.0f, 25 / 255.0f, 44 / 255.0f, 0.85f);
	Draw_StretchPic (pic_x, pic_yfit, mappic, bar_w - 2, pic_fit);

	/* r153 (user): a roster on the right side, like the Custom Games lobby.
	 * That one lists the bots you are about to spawn; this lobby belongs to
	 * someone else, so what we can honestly show is yourself plus the player
	 * count the server advertises. Same virtual menu space and geometry as
	 * M_Matchmaking_Draw (past 560x350 projects off-screen). */
	{
		int	rrw = (int)(80 * MENU_SCALE);
		/* r156 (user: "the playerlist should be on the right side"): follow THIS
		 * panel's right edge instead of Custom Games' constant x, and sit
		 * under the header rather than above the grid. */
		int	rrx = (int)(320 * MENU_SCALE) - 55 - rrw;
		int	rrh = 24;
		int	rry = (int)mc.grid[0][0].y - rrh;
		int	rband = 17;
		int	rfs = 13;
		int	prof = atoi (Cvar_VariableString ("bottomcolor"));
		const char *me = cl_name.string;
		char	count[16];

		if (prof < 0 || prof > 13)
			prof = 4;
		if (hidx >= 0)
			q_snprintf (count, sizeof (count), "%i/%i",
				    M_Lobby_Users (hidx), M_Lobby_Max (hidx));
		else
			q_strlcpy (count, "-", sizeof (count));

		Draw_FillRGBA (rrx, rry, rrw, rrh, 20/255.0f*0.85f, 25/255.0f*0.85f,
			       44/255.0f*0.85f, 0.85f);
		Draw_TTFStringHeader (rrx + CHARZ - 2, rry + 18, "ROSTER",
				      1, 1, 1, 0.9f, 14.0f);
		Draw_TTFString (rrx + rrw - 8 - (int)Draw_TTFWidth (count, 14.0f),
				rry + 18, count, 1, 1, 1, 1, 14.0f);
		{
			byte *pcc = (byte *)&d_8to24table[prof * 16 + 8];
			Draw_FillRGBA (rrx, rry + rrh + 2, rrw, rband,
				       pcc[0]/255.0f*0.35f, pcc[1]/255.0f*0.35f,
				       pcc[2]/255.0f*0.35f, 0.35f);
		}
		Draw_TTFString (rrx + CHARZ, rry + rrh + 2 + rfs,
				(*me) ? me : "You", 1, 1, 1, 1, (float)rfs);
	}

	{
		const char *cap;
		float csize = 16.0f;
		int cx = (int)pic_x + CHARZ;
		double avail = (mc.grid[mc.cols - 1][0].x + mc.colw) - cx - 8;

		if (hidx >= 0)
			cap = va ("%s - %s", M_Lobby_Name (hidx), M_Lobby_Map (hidx));
		else
			if (slistInProgress)
			cap = "Searching...";
		else if (lobby_rescan_time > realtime)
			cap = va ("Searching again in %i s...",
				  (int)(lobby_rescan_time - realtime + 0.5));
		else
			cap = "No lobbies match";
		while (csize > 9.0f && Draw_TTFHeaderWidth (cap, csize) > avail)
			csize -= 0.5f;
		Draw_TTFStringHeader (cx, pic_yfit + pic_fit - 9, cap, 1, 1, 1, 1, csize);

		/* r146: artist + blurb on a second line, above the title */
		if (hidx >= 0)
		{
			const char *au = M_Lobby_MapAuthor (hidx);
			const char *de = M_Lobby_MapDesc (hidx);
			const char *sub = NULL;
			float ssize = 11.0f;

			if (*au && *de)
				sub = va ("%s - by %s", de, au);
			else if (*au)
				sub = va ("by %s", au);
			else if (*de)
				sub = de;
			if (sub)
			{
				while (ssize > 8.0f && Draw_TTFWidth (sub, ssize) > avail)
					ssize -= 0.5f;
				Draw_TTFString (cx, pic_yfit + pic_fit - 9
						- (int)(ssize + 6.0f), sub,
						0.75f, 0.82f, 0.92f, 0.9f, ssize);
			}
		}
	}

	M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][0].yp, "NETWORK");
	M_Print (mc.grid[1][0].xp, mc.grid[1][0].yp, M_Lobby_NetworkName ());
	M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][1].yp, "PLAYLIST");
	M_Print (mc.grid[1][1].xp, mc.grid[1][1].yp, M_Lobby_PlaylistName ());
	M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][2].yp, "START MATCHMAKING");

#ifdef VITA
	M_Draw_HintY (MATCH_LOBBY_BOTTOM, mc, topbar + 5);   /* r144: arm art is Vita-only */
#else
	/* r151 (user): browse/advanced belong on the bar above the map art,
	 * exactly where Custom Games puts its action hints. */
	M_PrintHintCentred (mc, topbar + 5, "B Browse    T Advanced");
#endif
	botbar = (int)mc.grid[0][mc.rows].yp;
	/* r161 (user: "the arrows and select are still not conditional"): the bar
	 * follows the row. NETWORK/PLAYLIST are changed with left/right; the lobby
	 * rows and START MATCHMAKING are entered with A. */
	if (lobby_cursor < LOBBY_FILTERS)
	{
#ifdef VITA
		M_Draw_HintY (MATCH_SCROLL_TOP, mc, botbar);
#else
		M_PrintHintCentred (mc, botbar, "Left/Right Change    Esc Back");
#endif
	}
	else
	{
#ifdef VITA
		M_Draw_HintY (MATCH_MAIN_TOP, mc, botbar);
#else
		M_PrintHintCentred (mc, botbar, "Enter Select    Esc Back");
#endif
	}

	if (lobby_browse)
	{
		struct MenuCoords bc;
		int	shown, n;

		/* r152 (user): the browser is always openable, empty included. */
		if (total < 0)
			total = 0;
		if (lobby_browsecursor >= total)
			lobby_browsecursor = total - 1;
		if (lobby_browsecursor < 0)
			lobby_browsecursor = 0;
		/* r170: the chip rows moved into the header, so the popup body is
		 * all list again -- two more lobbies fit in the same panel. */
		shown = (total > LOBBY_BROWSE_ROWS + 2) ? LOBBY_BROWSE_ROWS + 2 : total;
		if (lobby_browsefirst + shown - 1 < lobby_browsecursor)
			lobby_browsefirst = lobby_browsecursor - (shown - 1);
		if (lobby_browsefirst > lobby_browsecursor)
			lobby_browsefirst = lobby_browsecursor;

		/* r165 (user: "the browse modal shrinks to minimum rows needed
		 * ... the map is squished"): size the popup for the FULL list, not
		 * for however many lobbies answered. Passing `shown` collapsed the
		 * right column (and with it the map art) whenever the browser held
		 * one or two servers -- same "layout rows, not filled rows" rule the
		 * main card already follows (r144). */
		/* r170 (user: "darken the screen behind the modal by a bit"):
		 * the same scrim the LAN config and rules pages already use, so
		 * the popup reads as a layer instead of part of the menu. */
		Draw_FillRGBA (0, 0, 1000, 1000, 0.02f, 0.02f, 0.025f, 0.45f);

		/* r170 (user: "filters go right of the modal title, which no longer
		 * needs to list your selections in the title"): plain screen name
		 * again; the chips moved up into the header band (below), so no
		 * body rows are spent on them. */
		bc = Draw_WindowGridBase ("Lobbies",
					  LOBBY_BROWSE_ROWS + 2, MVS_P, 2, 0.35, 1.0, 1.0,
					  0, 2, lobby_browsecursor - lobby_browsefirst,
					  1, 1.0, 2.2, 0.0);
		draw_grid_translucent = 0;

		/* r176 (user: "header needs some dark 50% opacity border on the
		 * bottom"): a dark rule closes the header band off from the list. */
		Draw_FillRGBA ((int)bc.x0, (int)bc.grid[0][0].y - 1, (int)bc.panelw, 1,
			       0, 0, 0, 0.5f);

		/* r172 (user: "looks kind of ugly"): the r171 module was crammed into
		 * a 1.5-row header. The header is taller now (2.2 rows) and the
		 * module is inset on all four sides with the chip rows centred, so
		 * it reads as a segmented control instead of a box glued to the
		 * edge. */
		{
			static const char *plabels[4] = {"All", "Slayer", "Team Slayer", "Swat"};
			static const char *slabels[4] = {"Players", "Bots", "Title", "Mode"};
			int	htop = (int)bc.grid[0][0].y - bc.headerh;
			int	wf = M_Lobby_ChipsWidth (plabels, 4);
			int	ws = M_Lobby_ChipsWidth (slabels, 4);
			int	lw = 38;			/* label gutter inside the module */
			/* r175 (user: "make the filter box width line up with the
			 * sidebar width"): the module now starts at the sidebar's left
			 * edge and ends at the panel border, so header and details
			 * column read as one column. */
			int	mx = (int)bc.grid[1][0].x;
			int	mw = (int)(bc.x0 + bc.panelw - 2) - mx;
			int	my = htop + 5;
			int	mh = bc.headerh - 10;
			int	r0 = my + (mh - 32) / 2;

			/* r176 (user: "I dont like the box around the filters, just
			 * keep them there with their title"): the r172/r173 frame and
			 * its gutter rule are gone -- the chips float on the header
			 * band and the row names do the labelling. */
			Draw_TTFString (mx + 7, r0 + 10, "Filter", 0.55f, 0.62f, 0.76f, 0.9f, 12.0f);
			Draw_TTFString (mx + 7, r0 + 27, "Sort", 0.55f, 0.62f, 0.76f, 0.9f, 12.0f);

			M_Lobby_Chips (mx + lw + (mw - lw - wf) / 2, r0, 15, plabels, 4, lobby_playlist);
			M_Lobby_Chips (mx + lw + (mw - lw - ws) / 2, r0 + 17, 15, slabels, 4, lobby_sort);
		}

		{
			/* r176 (user: "the list can go back to just the title
			 * (marquee when too long)"): Game and Players moved into the
			 * sidebar, so a row is the title at the full column width. */
			int	tmax = ((int)bc.colw - 15 - 6) / 8;	/* console font is 8 px */

			if (tmax < 8)
				tmax = 8;
			for (i = 0; i < shown; i++)
			{
				n = M_Lobby_IndexAt (lobby_browsefirst + i);
				if (n < 0)
					break;
				M_Lobby_DrawTitle (bc.grid[0][0].xp, bc.grid[0][i].yp,
						   M_Lobby_Title (n), tmax, i);
			}
		}
		if (!shown)
			M_Print (bc.grid[0][0].xp, bc.grid[0][0].yp,
				 slistInProgress ? "Searching..." : "No lobbies match this filter");

		/* r158 (user: "all I said was the browse modal needs art in it"):
		 * the highlighted lobby's map shot plus its name, artist and the
		 * game mode it is running, in the popup's right column -- what Map
		 * Selection shows for the highlighted map. r157 had moved the whole
		 * list out of the popup instead, which covered the menu rows. */
		/* r171 (user: "the right side of the modal with the map should also be
		 * a slightly different color of blue or a line vertically to
		 * differentiate the section"): tint the details column always, not
		 * only when a lobby is selected, plus a rule on its left edge. */
		{
			/* r173 (user: "the sidebar overlaps the cursor by a bit"):
			 * the tint started 8 px left of the column, on top of the
			 * highlighted row's last 8 px. Start it at the column edge. */
			int	dcol = (int)bc.grid[1][0].x;
			int	dright = (int)bc.x0 + (int)bc.panelw - 2;
			int	dtop = (int)bc.grid[1][0].y;
			int	dbot = (int)bc.grid[1][bc.rows].y;

			if (dbot > dtop)
			{
				Draw_FillRGBA (dcol, dtop, dright - dcol, dbot - dtop,
					       0.08f, 0.11f, 0.18f, 0.30f);
				Draw_FillRGBA (dcol, dtop, 1, dbot - dtop,
					       0.30f, 0.42f, 0.72f, 0.85f);
			}
		}

		n = M_Lobby_IndexAt (lobby_browsecursor);
		if (n >= 0)
		{
			qpic_t	*mp = M_Lobby_MapPic (n);
			char	by[80];
			char	rl[96];
			const char *ti = M_Lobby_Title (n);
			const char *gm = M_Lobby_Game (n);
			const char *ds = M_Lobby_Desc (n);
			const char *au = M_Lobby_MapAuthor (n);
			int	bt = M_Lobby_Bots (n);
			int	ru = M_Lobby_Rule (n);
			int	tm = M_Lobby_Time (n);
			int	mk = M_Lobby_MaxKills (n);
			/* r176 (user: "the content needs some left margin"): the
			 * sidebar text used to sit flush on the column rule. */
			int	ax = (int)bc.grid[1][0].x + 10;
			int	pright = (int)bc.x0 + (int)bc.panelw - 2;
			int	aw = pright - ax - 8;
			int	apt = (int)bc.grid[1][0].y;
			int	apb = (int)bc.grid[1][bc.rows].y;
			int	afit = (int)(aw / 1.7);
			int	texty = apt + 16;

			/* r176: how the match ends, from the server's own limits.
			 * Neither advertised (older build) draws nothing rather than
			 * claiming there are no limits. */
			rl[0] = 0;
			if (tm > 0 && mk > 0)
				q_snprintf (rl, sizeof (rl), "%i min    first to %i kills", tm, mk);
			else if (tm > 0)
				q_snprintf (rl, sizeof (rl), "%i min    no kill limit", tm);
			else if (mk > 0)
				q_snprintf (rl, sizeof (rl), "no time limit    first to %i kills", mk);

			/* r175: the sidebar carries the whole record -- title, mode
			 * and map, description, players and the bot rule, author --
			 * over the map shot. */
			if (afit > apb - apt - 112)
				afit = apb - apt - 112;
			if (aw > 0 && afit > 0)
			{
				Draw_TTFStringHeader (ax, texty, ti, 1, 1, 1, 1, 17.0f);
				Draw_TTFString (ax, texty + 20,
						va ("%s    %s", gm, M_Lobby_Map (n)),
						0.75f, 0.82f, 0.92f, 0.9f, 13.0f);
				if (*ds)
					Draw_TTFString (ax, texty + 38, ds,
							0.68f, 0.75f, 0.86f, 0.9f, 13.0f);
				Draw_TTFString (ax, texty + 58,
						va ("%i/%i players    %i bots (fill %i)",
						    M_Lobby_Users (n), M_Lobby_Max (n), bt, ru),
						0.75f, 0.82f, 0.92f, 0.9f, 13.0f);
				if (rl[0])
					Draw_TTFString (ax, texty + 76, rl,
							0.75f, 0.82f, 0.92f, 0.9f, 13.0f);
				if (*au)
				{
					q_snprintf (by, sizeof (by), "by %s", au);
					Draw_TTFString (ax, texty + 94, by,
							0.6f, 0.67f, 0.78f, 0.9f, 11.0f);
				}
				Draw_StretchPic (ax, apt + 112, mp, aw, afit);
			}
		}

		/* r152 (user): the browser needs hints of its own; r156 centred them. */
#ifdef VITA
		M_Draw_HintY (MATCH_BROWSE_TOP, bc, (int)bc.grid[0][bc.rows].yp);
#else
		/* r169 (user: "you cant say LB on mac but have the bind something
		 * else"): desktop hints name the desktop keys only -- arrow keys
		 * and F cycle the filter, S the sort, R rescans, T advanced. */
		M_PrintHintCentred (bc, (int)bc.grid[0][bc.rows].yp,
				    "Enter Select   Esc Back   F Filter   S Sort   R Rescan");
#endif
	}
}

/* r165 (user): the Find Match screen's Advanced action must open the JOIN
 * form. It used to call M_Menu_LanConfig_f directly, and that screen reads
 * m_multiplayer_cursor (default 1 = New Game, still 1 after hosting a Custom
 * Game), so the Vita showed hosting options it cannot use. Force the join
 * side; the LAN config screen then leads with "Join Game - TCP/IP". */
static void M_Lobby_AdvancedConnect (void)
{
	m_multiplayer_cursor = 0;
	M_Menu_LanConfig_f ();
}

void M_Lobbies_Key (int key)
{
	int	rows = LOBBY_FILTERS + 1;
	int	total = M_Lobby_Count ();

	if (lobby_cursor >= rows)
		lobby_cursor = rows - 1;

	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		if (lobby_browse)
		{
			lobby_browse = 0;
			S_LocalSound ("misc/menu1.wav");
			break;
		}
		M_Menu_Main_f ();
		break;
#ifdef VITA
	case K_ENTER:
		if (!lobby_browse)
		{
			M_Menu_Main_f ();
			break;
		}
		/* fall through to select while browsing */
#endif

	case K_XBUTTON:
		S_LocalSound ("misc/menu1.wav");
		if (!lobby_browse)
		{
			if (1)	/* r150: always openable, even with an empty list */
			{
				lobby_browse = 1;
				lobby_browsecursor = 0;
				lobby_browsefirst = 0;
			}
			else
				M_Lobby_Scan ();
		}
		else
			lobby_browse = 0;
		break;

	case K_YBUTTON:
		/* r138: advanced connection; r165: always the join form */
		S_LocalSound ("misc/menu1.wav");
		M_Lobby_AdvancedConnect ();
		break;
#ifndef VITA
	case 'r':
	case 'R':
		S_LocalSound ("misc/menu1.wav");
		M_Lobby_Scan ();
		break;
	case 'f':
	case 'F':
		lobby_playlist = (lobby_playlist + 1) % 4;
		lobby_browsecursor = lobby_browsefirst = 0;
		S_LocalSound ("misc/menuoption.wav");
		break;
	case 's':
	case 'S':
		lobby_sort = (lobby_sort + 1) % 4;
		S_LocalSound ("misc/menuoption.wav");
		break;
	case 't':
	case 'T':
		S_LocalSound ("misc/menu1.wav");
		M_Lobby_AdvancedConnect ();
		break;
	case 'b':
	case 'B':
		if (!lobby_browse && total > 0)
		{
			lobby_browse = 1;
			lobby_browsecursor = lobby_browsefirst = 0;
			S_LocalSound ("misc/menu1.wav");
		}
		break;
#endif

	/* r167 (user): Vita-friendly refresh/sort. The popup already uses the
	 * face buttons for open/close/select, so these live on the shoulders. */
	case K_LSHOULDER:
		S_LocalSound ("misc/menu1.wav");
		M_Lobby_Scan ();
		break;

	case K_RSHOULDER:
		lobby_sort = (lobby_sort + 1) % 4;
		S_LocalSound ("misc/menuoption.wav");
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (lobby_browse)
		{
			if (--lobby_browsecursor < 0)
				lobby_browsecursor = total > 0 ? total - 1 : 0;
		}
		else if (--lobby_cursor < 0)
			lobby_cursor = rows - 1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (lobby_browse)
		{
			if (++lobby_browsecursor >= total)
				lobby_browsecursor = 0;
		}
		else if (++lobby_cursor >= rows)
			lobby_cursor = 0;
		break;

	case K_LEFTARROW:
	case K_RIGHTARROW:
		{
			int dir = (key == K_RIGHTARROW) ? 1 : -1;

			if (lobby_browse)
			{
				/* r167 (user): playlist filter inside the popup. It is a
				 * client-side view over the cached list, so there is no
				 * rescan -- just move the highlight back to the top. */
				lobby_playlist += dir;
				if (lobby_playlist > 3) lobby_playlist = 0;
				if (lobby_playlist < 0) lobby_playlist = 3;
				lobby_browsecursor = 0;
				lobby_browsefirst = 0;
				S_LocalSound ("misc/menuoption.wav");
				break;
			}
			if (lobby_cursor == 0)
			{
				lobby_network += dir;
				if (lobby_network > 2) lobby_network = 0;
				if (lobby_network < 0) lobby_network = 2;
			}
			else if (lobby_cursor == 1)
			{
				lobby_playlist += dir;
				if (lobby_playlist > 3) lobby_playlist = 0;
				if (lobby_playlist < 0) lobby_playlist = 3;
			}
			else
				break;
			lobby_browsecursor = 0;
			S_LocalSound ("misc/menuoption.wav");
			M_Lobby_Scan ();
		}
		break;

	case K_ABUTTON:
#ifndef VITA
	case K_ENTER:
	case K_KP_ENTER:
#endif
		{
			int idx;

			if (lobby_browse)
			{
				/* r153 (user): picking a lobby in the browser only moves it
				 * onto the main screen; the join still needs START. */
				idx = M_Lobby_IndexAt (lobby_browsecursor);
				if (idx >= 0)
					lobby_selected = lobby_browsecursor;
				lobby_browse = 0;
				S_LocalSound ("misc/menuoption.wav");
				break;
			}
			idx = M_Lobby_IndexAt (lobby_selected);
			if (idx < 0 || !M_Lobby_Matches (idx))
				idx = M_Lobby_Best ();
			if (idx >= 0)
			{
				M_Lobby_Join (idx);
				break;
			}
			S_LocalSound ("misc/menu1.wav");
			M_Lobby_Scan ();
		}
		break;
	}
}

/* SEARCH MENU */

qboolean	searchComplete = false;
double		searchCompleteTime;
enum slistScope_e searchLastScope = SLIST_LAN;

void M_Menu_Search_f (enum slistScope_e scope)
{
	key_dest = key_menu;
	m_state = m_search;
	IN_UpdateGrabs();
	m_entersound = false;
	slistSilent = true;
	slistScope = searchLastScope = scope;
	searchComplete = false;
	NET_Slist_f();

}


void M_Search_Draw (void)
{
	qpic_t	*p;
	int x;

    Draw_MenuBg();
    struct MenuCoords mc = Draw_WindowGrid("Matchmaking", 2, MVS_P, 1, 0.4, 0.667, -1, false);
	M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][0].yp, "Finding match...");

	if(slistInProgress)
	{
		NET_Poll();
		return;
	}

	if (! searchComplete)
	{
		searchComplete = true;
		searchCompleteTime = realtime;
	}

	if (hostCacheCount)
	{
		M_Menu_ServerList_f ();
		return;
	}

    M_PrintWhite (mc.grid[0][1].xp, mc.grid[0][1].yp,
                  Cvar_VariableValue ("cl_slist_only_solitude") ?
                  "No Solitude lobbies found" : "No Quake servers found");
	if ((realtime - searchCompleteTime) < 3.0)
		return;

	M_Menu_LanConfig_f ();
}


void M_Search_Key (int key)
{
}

//=============================================================================
/* SLIST MENU */

size_t		slist_cursor;
size_t		slist_first;
qboolean slist_sorted;

void M_Menu_ServerList_f (void)
{
	key_dest = key_menu;
	m_state = m_slist;
	IN_UpdateGrabs();
	m_entersound = true;
	slist_cursor = 0;
	slist_first = 0;
	m_return_onerror = false;
	m_return_reason[0] = 0;
}


void M_ServerList_Draw (void)
{
	size_t	n, slist_shown;

    Draw_MenuBg();

	if (!slist_sorted)
	{
		slist_sorted = true;
		NET_SlistSort ();
	}

	slist_shown = hostCacheCount;
	if (slist_shown > (200-50)/CHARZ) // vertical size max
		slist_shown = (200-50)/CHARZ;
	if (slist_first+slist_shown-1 < slist_cursor)
		slist_first = slist_cursor-(slist_shown-1);
	if (slist_first > slist_cursor)
		slist_first = slist_cursor;

    struct MenuCoords mc = Draw_WindowGrid("Matchmaking", slist_shown, 14, 1, 0.667, 0.667, slist_cursor-slist_first, true);
    //footer
    switch (slist_cursor-slist_first) {
        default:
            M_Draw_Hint(SELECT, mc);
    }
    // draw servers
	for (n = 0; n < slist_shown; n++)
		M_PrintWhite (mc.grid[0][n].xp, mc.grid[0][n].yp, NET_SlistPrintServer (slist_first+n));

    // draw error
	if (*m_return_reason) {
        struct MenuCoords mce = Draw_WindowGrid("Error", 1, 14, 1, 40, 1.0, -1, false);
        M_PrintWhite (mce.grid[0][0].xp, mce.grid[0][0].yp, m_return_reason);
    }
}


void M_ServerList_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_LanConfig_f ();
		break;

	case K_SPACE:
		M_Menu_Search_f (searchLastScope);
		break;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		slist_cursor--;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = hostCacheCount - 1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		slist_cursor++;
		if (slist_cursor >= hostCacheCount)
			slist_cursor = 0;
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		S_LocalSound ("misc/menu2.wav");
		m_return_state = m_state;
		m_return_onerror = true;
		match_return_state = m_slist;	/* r164: disconnect -> server list */
		key_dest = key_game;
		m_state = m_none;
		IN_UpdateGrabs();
		SCR_ShowLoadingScreen ();
		Cbuf_AddText ( va ("connect \"%s\"\n", NET_SlistPrintServerName(slist_cursor)) );
		break;

	default:
		break;
	}

}

static struct
{
	const char *name;
	xcommand_t function;
	cmd_function_t *cmd;
} menucommands[] =
{
	{"menu_main", M_Menu_Main_f},
	{"menu_singleplayer", M_Menu_SinglePlayer_f},
	{"menu_load", M_Menu_Load_f},
	{"menu_save", M_Menu_Save_f},
//	{"menu_multiplayer", M_Menu_MultiPlayer_f},
	{"menu_setup", M_Menu_Setup_f},
	{"menu_options", M_Menu_Options_f},
	{"menu_editor", M_Menu_Editor_f},	//r101
	{"menu_keys", M_Menu_Keys_f},
	{"menu_video", M_Menu_Video_f},
	{"help", M_Menu_Help_f},
	{"menu_quit", M_Menu_Quit_f},
    {"menu_pause", M_Menu_Pause_f},
};

//=============================================================================
/* MenuQC Subsystem */
extern builtin_t pr_menubuiltins[];
extern int pr_menunumbuiltins;
#define MENUQC_PROGHEADER_CRC 10020
void MQC_End(void)
{
	PR_SwitchQCVM(NULL);
}
void MQC_Begin(void)
{
	PR_SwitchQCVM(&cls.menu_qcvm);
	pr_global_struct = NULL;
}
static qboolean MQC_Init(void)
{
	size_t i;
	qboolean success;
	PR_SwitchQCVM(&cls.menu_qcvm);
	if (COM_CheckParm("-qmenu") || fitzmode || !pr_checkextension.value)
		success = false;
	else
		success = PR_LoadProgs("menu.dat", false, MENUQC_PROGHEADER_CRC, pr_menubuiltins, pr_menunumbuiltins);
	if (success && qcvm->extfuncs.m_draw)
	{
		for (i = 0; i < sizeof(menucommands)/sizeof(menucommands[0]); i++)
			if (menucommands[i].cmd)
			{
				Cmd_RemoveCommand (menucommands[i].cmd);
				menucommands[i].cmd = NULL;
			}


		qcvm->max_edicts = CLAMP (MIN_EDICTS,(int)max_edicts.value,MAX_EDICTS);
		qcvm->edicts = (edict_t *) malloc (qcvm->max_edicts*qcvm->edict_size);
		qcvm->num_edicts = qcvm->reserved_edicts = 1;
		memset(qcvm->edicts, 0, qcvm->num_edicts*qcvm->edict_size);

		if (qcvm->extfuncs.m_init)
			PR_ExecuteProgram(qcvm->extfuncs.m_init);
	}
	PR_SwitchQCVM(NULL);
	return success;
}

void MQC_Shutdown(void)
{
	size_t i;
	if (key_dest == key_menu)
		key_dest = key_console;
	PR_ClearProgs(&cls.menu_qcvm);					//nuke it from orbit

	for (i = 0; i < sizeof(menucommands)/sizeof(menucommands[0]); i++)
		if (!menucommands[i].cmd)
			menucommands[i].cmd = Cmd_AddCommand (menucommands[i].name, menucommands[i].function);
}

static void MQC_Command_f(void)
{
	if (cls.menu_qcvm.extfuncs.GameCommand)
	{
		MQC_Begin();
		G_INT(OFS_PARM0) = PR_MakeTempString(Cmd_Args());
		PR_ExecuteProgram(qcvm->extfuncs.GameCommand);
		MQC_End();
	}
	else
		Con_Printf("menu_cmd: no menuqc GameCommand function available\n");
}

#ifdef VITA
//=============================================================================
/* Mod selector menu, see Modlist_* in host_cmd.c */

static filelist_item_t *mod_selected = NULL;

static void ChangeGame (char *mod)
{
	// change game
	Cbuf_AddText( va ("game %s\n", mod) );
}

void M_Mods_Key (int k)
{
	filelist_item_t *mod, *target;

	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		M_Menu_Options_f ();
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (mod_selected && mod_selected->next)
			mod_selected = mod_selected->next;
		else
			mod_selected = modlist;
		break;

	case K_UPARROW:
	case K_LEFTARROW:
		S_LocalSound ("misc/menuoption.wav");
		if (mod_selected)
		{
			target = mod_selected == modlist ? NULL : mod_selected;
			for (mod = modlist; mod && mod->next != target; mod = mod->next);
			mod_selected = mod;
		}
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		if (mod_selected)
		{
			S_LocalSound ("misc/menu2.wav");
			key_dest = key_game;
			m_state = m_none;
			ChangeGame (mod_selected->name);
		}
		break;

	default:
		break;
	}
}

void M_Mods_Draw (void)
{
	int	n, i = 0;
	char *tmp;
	filelist_item_t	*mod;

    if (!sv.active) {
        Draw_MenuBg();
    }

	M_PrintWhite (160 - 13*8, 0, "Detected game directories:");
	for (mod = modlist, n=0; mod; mod = mod->next, n++)
	{
		M_Print (160 - 13*8, 16 + 8*n, mod->name);
		if (mod == mod_selected) i = n;
	}

	if (n)
		M_DrawCharacter (160 - 15*8, 16 + i*8, 12+((int)(realtime*4)&1));
	else
		M_Print (160 - 13*8, 16, "No mods in current directory.");

	tmp = va("Current game directory: %s", com_gamedir);
	M_PrintWhite (160 - strlen(tmp)*4, 200 - 32, tmp);
	M_PrintWhite (160 - 13*8, 200 - 16, "Select new game directory");
	M_PrintWhite (160 - 13*8, 200 -  8, " and press A to restart.");
}

void M_Menu_Mods_f (void)
{
	key_dest = key_menu;
	m_state = m_mods;
	m_entersound = true;
	mod_selected = modlist;
}
#endif

//=============================================================================
/* Menu Subsystem */

/*
================
M_ToggleMenu_f
================
*/
void M_ToggleMenu (int mode)
{
	if (cls.menu_qcvm.extfuncs.m_toggle)
	{
		MQC_Begin();
		G_FLOAT(OFS_PARM0) = mode;
		PR_ExecuteProgram(qcvm->extfuncs.m_toggle);
		MQC_End();
		return;
	}

	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (mode != 0 && m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}

		key_dest = key_game;
		m_state = m_none;

		IN_UpdateGrabs();
		return;
	}
	else if (mode == 0)
		return;
	if (mode == -1 && key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}
	else
	{
        /* r123: a client on a REMOTE server has sv.active == 0, so this used to
         * send them to the main menu on every pause press. The in-game menu
         * belongs to anyone actually connected to a server (hosted or joined). */
        if (sv.active || (cls.state == ca_connected && !cls.demoplayback))
        {
            M_Menu_Pause_f ();
        } else {
            M_Menu_Main_f ();
        }
	}
}

static void M_ToggleMenu_f (void)
{
	M_ToggleMenu((Cmd_Argc() < 2) ? -1 : atoi(Cmd_Argv(1)));
}

static void M_MenuRestart_f (void)
{
	qboolean off = !strcmp(Cmd_Argv(1), "off");
	if (off || !MQC_Init())
		MQC_Shutdown();
}

void M_Init (void)
{
	Cmd_AddCommand ("togglemenu", M_ToggleMenu_f);

	// P2a lobby roster: bot count cvar + menu commands (see M_Matchmaking_Draw)
	lobby_bots = Cvar_Create ("lobby_bots", "5");   /* r94: TDM default = 6 players total (you + 5) */
	/* r117 (user): the lobby remembers mode + player count across matches and
	 * launches. Both cvars are the single source of truth already used by the
	 * roster/QC; archiving makes Host_WriteConfiguration persist them. */
	if (lobby_bots)
	    lobby_bots->flags |= CVAR_ARCHIVE;
	lobby_gametype = Cvar_Create ("lobby_gametype", "1");   /* default: Team Slayer, matches opts */
	if (lobby_gametype)
	    lobby_gametype->flags |= CVAR_ARCHIVE;
	/* r115b: lobby-owned bot color cvars (QC BotConnect ladder reads them by
	 * slot; 0 = never pinned). r115's version put this very call INSIDE the
	 * block comment (opened before it, closed after) — -O2 deleted the empty
	 * loop, cvars never existed, every lobby open logged "bot_colorN not
	 * found" and QC rolled random colors. Brace it, comment on its own lines. */
	for (int ci = 0; ci < 7; ci++)
	{
		Cvar_Create (va("bot_color%i", ci), "0");
	}
	Cmd_AddCommand ("lobby_addbot", M_Lobby_AddBot_f);
	Cmd_AddCommand ("lobby_delbot", M_Lobby_DelBot_f);
	Cmd_AddCommand ("lobby_shufflenames", M_Lobby_ShuffleNames_f);
	// perf B7 - Performance menu: preset memory cvar + console shortcut
	Cvar_RegisterVariable (&perf_preset);
	Cvar_RegisterVariable (&perf_custom_backup);	// perf B9b
	Cmd_AddCommand ("perfmenu", M_Menu_Performance_f);
	Cmd_AddCommand ("editmenu", M_Menu_Editor_f);	//r101 Editor tab
	Cmd_AddCommand ("menu_cmd", MQC_Command_f);
	Cmd_AddCommand ("menu_restart", M_MenuRestart_f);	//qss still loads progs on hunk, so we can't do this safely.

	// perf B7: the Custom Maps folder is scanned ONCE at startup (user);
	// the lazy first-use scan still covers a not-yet-set gamedir.
	M_Maps_ScanCustom ();
	Cmd_AddCommand ("rescanmaps", M_Maps_Rescan_f);

	if (!MQC_Init())
		MQC_Shutdown();
}


void M_Draw (void)
{
	if (cls.menu_qcvm.extfuncs.m_draw)
	{	//Spike -- menuqc
		float s = q_min((float)glwidth / 320.0, (float)glheight / 200.0);
		s = CLAMP (1.0, scr_menuscale.value, s);
		if (!host_initialized)
			return;
		MQC_Begin();

		if (scr_con_current && key_dest == key_menu)
		{	//make sure we don't have the console getting drawn in the background making the menu unreadable.
			//FIXME: rework console to show over the top of menuqc.
			Draw_ConsoleBackground ();
			S_ExtraUpdate ();
		}

		GL_SetCanvas (CANVAS_MENU_STRETCH);
		glEnable (GL_BLEND);	//in the finest tradition of glquake, we litter gl state calls all over the place. yay state trackers.
		glDisable (GL_ALPHA_TEST);	//in the finest tradition of glquake, we litter gl state calls all over the place. yay state trackers.
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (qcvm->extglobals.time)
			*qcvm->extglobals.time = realtime;
		if (qcvm->extglobals.frametime)
			*qcvm->extglobals.frametime = host_frametime;
		G_FLOAT(OFS_PARM0+0) = vid.width/s;
		G_FLOAT(OFS_PARM0+1) = vid.height/s;
		G_FLOAT(OFS_PARM0+2) = 0;
		PR_ExecuteProgram(qcvm->extfuncs.m_draw);

		glDisable (GL_BLEND);
		glEnable (GL_ALPHA_TEST);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);	//back to ignoring vertex colours.
		glDisable(GL_SCISSOR_TEST);
		glColor3f (1,1,1);

		MQC_End();
		return;
	}

	if (m_state == m_none || key_dest != key_menu)
		return;

	if (!m_recursiveDraw)
	{
		if (scr_con_current)
		{
			Draw_ConsoleBackground ();
			S_ExtraUpdate ();
		}

		Draw_FadeScreen (); //johnfitz -- fade even if console fills screen
	}
	else
	{
		m_recursiveDraw = false;
	}

	GL_SetCanvas (CANVAS_MENU_STRETCH); //johnfitz

	switch (m_state)
	{
	case m_none:
		break;

	case m_main:
		M_Main_Draw ();
		break;

	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;

	case m_load:
		M_Load_Draw ();
		break;

	case m_save:
		M_Save_Draw ();
		break;

    case m_firefight:
        M_Firefight_Draw();
        break;

    case m_pause:
        M_Pause_Draw ();
        break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_options:
		M_Options_Draw ();
		break;

	case m_keys:
		M_Keys_Draw ();
		break;

	case m_video:
		M_Video_Draw ();
		break;

	case m_help:
		M_Help_Draw ();
		break;

	case m_quit:
		M_Quit_Draw ();
		break;

	case m_lanconfig:
		M_LanConfig_Draw ();
		break;

	case m_gameoptions:
        M_Matchmaking_Draw ();
		break;

	case m_lobbies:
		M_Lobbies_Draw ();
		break;

    case m_matchmaking_map:
        M_Matchmaking_Map_Draw ();
        break;

    case m_matchmaking_gamerules:
        M_Matchmaking_Gamerules_Draw ();
        break;

    case m_performance:	// perf B7
        M_Performance_Draw ();
        break;

	case m_search:
		M_Search_Draw ();
		break;

	case m_slist:
		M_ServerList_Draw ();
		break;
#ifdef VITA
	case m_mods:
		M_Mods_Draw ();
		break;
#endif

	}

	if (m_entersound)
	{
		S_LocalSound ("misc/menu2.wav");
		m_entersound = false;
	}

	S_ExtraUpdate ();
}


void M_Keydown (int key)
{
	if (cls.menu_qcvm.extfuncs.m_draw)	//don't get confused.
		return;

	switch (m_state)
	{
	case m_none:
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_load:
		M_Load_Key (key);
		return;

	case m_save:
		M_Save_Key (key);
		return;

//	case m_multiplayer:
//		M_MultiPlayer_Key (key);
//		return;

    case m_pause:
        M_Pause_Key (key);
        return;

    case m_firefight:
        M_Firefight_Key (key);
        return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_options:
		M_Options_Key (key);
		return;

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_video:
		M_Video_Key (key);
		return;

	case m_help:
		M_Help_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_lanconfig:
		M_LanConfig_Key (key);
		return;

	case m_gameoptions:
        M_Matchmaking_Key (key);
		return;

	case m_lobbies:
		M_Lobbies_Key (key);
		return;

    case m_matchmaking_map:
        M_Matchmaking_Map_Key (key);
        break;

    case m_matchmaking_gamerules:
        M_Matchmaking_Gamerules_Key (key);
        break;

    case m_performance:	// perf B7
        M_Performance_Key (key);
        break;

	case m_search:
		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;
#ifdef VITA
	case m_mods:
		M_Mods_Key (key);
		break;
#endif
	}
}


void M_Charinput (int key)
{
	if (cls.menu_qcvm.extfuncs.m_draw)	//don't get confused.
		return;

	switch (m_state)
	{
	case m_setup:
		M_Setup_Char (key);
		return;
	case m_quit:
		M_Quit_Char (key);
		return;
	case m_lanconfig:
		M_LanConfig_Char (key);
		return;
	default:
		return;
	}
}


qboolean M_TextEntry (void)
{
	switch (m_state)
	{
	case m_setup:
		return M_Setup_TextEntry ();
	case m_quit:
		return M_Quit_TextEntry ();
	case m_lanconfig:
		return M_LanConfig_TextEntry ();
	default:
		return false;
	}
}


void M_ConfigureNetSubsystem(void)
{
// enable/disable net systems to match desired config
	Cbuf_AddText ("stopdemo\n");

	if (IPXConfig || TCPIPConfig)
		net_hostport = lanConfig_port;
}
