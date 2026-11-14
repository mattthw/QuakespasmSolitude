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
extern cvar_t	pstv_rumble, retrotouch, motioncam, motion_horizontal_sensitivity, motion_vertical_sensitivity;

enum
{
	UNI_TAB_PERF, UNI_TAB_GFX, UNI_TAB_CTRL, UNI_TAB_SND, UNI_TAB_GEN, UNI_TABS
};

static const char	*uni_tab_names[UNI_TABS] = {"Performance", "Graphics", "Controls", "Sound", "General"};
static int	 uni_tab, uni_cursor, uni_first;
static enum m_state_e uni_return_state;

static void M_Performance_Body (struct MenuCoords *coords, int rowh, size_t shown);
static void M_Performance_ResetWindow (void);
static void M_Options_CycleTab (int dir);
static void M_Draw_Scrollbar (struct MenuCoords *coords, int rowh, int total, size_t first, int shown);
static int M_HintIconW (qpic_t *icon);	// perf B7: reuse matchmaking hint icons in the tab strip
static void M_HintIcon (int *x, int y, qpic_t *icon);
cvar_t	perf_preset = {"perf_preset", "2", CVAR_ARCHIVE};	// perf B7 - 1/2/3 = last preset applied, 0 = custom; ships Balanced

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
	void M_Menu_Help_f (void);
	void M_Menu_Quit_f (void);
    void M_Menu_Pause_f (void);

void M_Main_Draw (void);
	void M_SinglePlayer_Draw (void);
		void M_Load_Draw (void);
		void M_Save_Draw (void);
    void M_Main_Multi_Draw (void);
    void M_Matchmaking_Draw (void);
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

	Draw_TTFString (cx, cy + 8, tmpl,
		(menu_palette[col*3] + 0.1f) / 255.0f,
		(menu_palette[col*3+1] + 0.1f) / 255.0f,
		(menu_palette[col*3+2] + 0.1f) / 255.0f, 1.0f, 14.0f);
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
    if (!theme_started || override) {
        Cbuf_AddText("play music/theme.wav\n");
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
    M_PrintWhite(main_x_offset_pixels+main_pixel_width-(8*CHARZ), PixHeight(1)-MVS+TEXT_YMARGIN, " Select"); //tip
    Draw_Button(main_x_offset_pixels+main_pixel_width-(10*CHARZ), PixHeight(1)-MVS+TEXT_YMARGIN, b_abutton);
    //cursor
    Draw_Cursor(main_x_offset_pixels, main_y_offset_pixels+mvs(m_main_cursor), main_pixel_width, CURSOR_HEIGHT, !main_multi);
    //menu items
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(0), "Matchmaking");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(1), "Firefight");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(2), "Options");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(3), "Spartan");
    M_PrintWhite(main_x_offset_pixels+TEXT_XMARGIN, main_y_offset_pixels+TEXT_YMARGIN+mvs(4), "Quit");

    /* r94: welcome marquee disabled per user */

    if (main_multi)
        M_Main_Multi_Draw ();
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
        if (main_multi == true)
        {
            main_multi = false;
        }
        else
        {
            m_main_cursor = 4;
            M_Menu_Quit_f ();
        }
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
            if (main_multi)
            {
                if (++m_main_multi_cursor >= MAIN_MULTI_ITEMS)
                    m_main_multi_cursor = 0;
            }
            else
            {
                if (++m_main_cursor >= MAIN_ITEMS)
                    m_main_cursor = 0;
            }
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
            if (main_multi)
            {
                if (--m_main_multi_cursor < 0)
                    m_main_multi_cursor = MAIN_MULTI_ITEMS - 1;
            }
            else
            {
                if (--m_main_cursor < 0)
                    m_main_cursor = MAIN_ITEMS - 1;
            }
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
        if (m_main_cursor != 0) //submenu
        {
            S_LocalSound ("misc/menuoption.wav");
        }
        else
        {
            m_entersound = true;
        }

        if (main_multi)
        {
            switch (m_main_multi_cursor)
            {
                case 0:
                    M_Matchmaking_f();
                    break;
                case 1:
                    m_multiplayer_cursor = 0;
                    M_Menu_LanConfig_f();
                    break;
            }
        }
        else
        {
            switch (m_main_cursor)
            {
                case 0:
                    main_multi = true;
                    break;
                case 1:
                    M_Menu_Firefight_f ();
                    break;

                case 2:
                    M_Menu_Options_f ();
                    break;

                case 3:
                    M_Menu_Setup_f ();
                    break;

                case 4:
                    M_Menu_Quit_f ();
                    break;
            }
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
                if ((int)deathmatch.value == 2)
                    M_Menu_Firefight_f(); // todo: do I need to do more than this (probably). Why not use entry menu f?
                else
                    M_Matchmaking_f(); // todo: same as above
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
	uni_tab = UNI_TAB_PERF;			// performance is the default tab (user)
	uni_cursor = uni_first = 0;
	M_Performance_ResetWindow ();

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
void M_Menu_LanConfig_f (void)
{
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
        if (m_multiplayer_cursor == 0)
        {
            M_Menu_Main_f ();
        }
        else
        {
            M_Matchmaking_f ();
        }
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
                M_Matchmaking_f ();
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
				key_dest = key_game;
				m_state = m_none;
				IN_UpdateGrabs();
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
        {"pit3", "pit3", 1, "Pit", "A deep UNSC training hole ringed by gantries. Drop in with a shotgun and climb out with a story.", "Jukki, mattthw"},
        {"narsp", "narsp", 1, "Narrows", "The Ark's cooling channels, still running cold. Tight catwalks over open water make every duel a knife fight.", "Scifiknux"},
        {"skyringbl2", "skyringbl2", 1, "Heavenly Suspense", "A Halo ring facility under repair, open sky on every side. Snipers own the scaffolds; everyone else hides.", "Arkage"},
        {"wizard", "wizard", 1, "Wizard", "A small ring arena that never stops turning. Round and round and round - the middle belongs to no one.", "IlDucci"},
        {"lockdown", "lockdown", 1, "Lockdown", "A prison block on full lockdown. Corridors funnel every push into one bad doorway at a time.", "Team Xlink, mattthw"},
        {"cestra", "cestra", 1, "Cestra", "A buried research lab the brass never expected anyone to fight through. Every hall tests something new.", "Sam"},
        {"lane", "lane", 1, "Lane", "An outdoor human training lane: one long corridor of cover with flanking shelves above it.", "Sam"},
        // Classic map pack [6]
        {"Longest", "Longest", 0, "Longest", "The longest lane in the rotation. Mid-range duels down an endless aisle, with just enough cover to feint.", "Unknown"},
        {"chill", "chill", 0, "Chill Out", "A frozen outpost of short halls and shorter tempers. Indoor fights end before you find your breath.", "Unknown"},
        {"pri2", "pri2", 0, "Prisoner 2", "A compact prison complex. Three levels of balconies turn every escort into an ambush.", "Unknown"},
        {"base", "base", 0, "Minibase", "A small two-flag outpost built for duels. One ridge, two bases, and nowhere to hide.", "Unknown"},
        {"plaza", "plaza", 0, "Plaza", "An open civic plaza ringed by shops. The middle is a graveyard; own the rooftops instead.", "Unknown"},
        {"spider", "spider", 0, "Spiderweb", "Towers strung together on narrow webbing. Fall off a bridge and the match is already lost.", "Unknown"},
        // Bonus map pack
        {"pit", "random", 0, "Pit (original)", "The first cut of the Pit: three towers, one hole, and no forgiveness for bad jumps.", "Jukki"},
        {"pit2", "pit2", 1, "Pit (v2)", "The Pit rebuilt - more gantries, more corners, and more reasons to watch your footing.", "Jukki, mattthw"},
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
                {"New & Remastered", 0, 10},
                {"Classic Maps", 10, 6},
                {"Bonus Maps", 16, 5},
                {"Custom Maps", 0, 0}	// perf B7: loose .bsp files from maps/custom, counted at runtime
        };
int NUM_EPISODES_FF = 1;
episode_t	episodes_ff[] =
        {
                {"Firefight Maps", 21, 2}
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

    // background tint (perf B7: opaque, like the unified Options)
    Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 1.0f);

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
        case K_LTRIGGER:
        case K_LEFTARROW:
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Gamerules_Submenu_Key(-1);
            break;
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
	PERF_KNOB_ROWS   = 16,
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
// rows 12..15 are passthrough knobs the presets leave alone.
static perf_knob_t	perf_knobs[PERF_KNOB_ROWS] =
{
	{"Perf HUD",         &r_perfhud,         0, 1,    1,    "",  0, 0, 0, "Framerate info on screen. Impact: none."},
	{"Cull Lights",      &r_dlight_cull,     0, 1,    1,    "",  0, 0, 0, "Ignores light flashes hidden by walls. Impact: high."},
	{"Max Lights",       &r_max_dlights,     0, 32,   4,    "",  0, 0, 1, "How many lights from gunfire and blasts. Impact: med."},
	{"Light Cache",      &r_light_cache,     0, 4,    1,    "",  0, 1, 0, "Lighting redraws every few frames. Impact: med."},
	{"Far Detail",       &r_subpixel_cull,   0, 8,    1,    " px", 0, 1, 0, "Tiny far-off wall details are skipped. Impact: med."},
	{"Big Particles",    &r_part_maxrad,     0, 128,  8,    " px", 0, 1, 0, "Huge particle clouds are skipped. Impact: med."},
	{"Particles",        &r_particles,       0, 1,    0.25, "%", 1, 0, 0, "How much smoke, sparks and blood you see. Impact: high."},
	{"Blood & Debris",   &sv_particle_scale, 0, 100,  25,   "%", 0, 1, 0, "Blood and debris from multiplayer kills. Impact: high."},
	{"Gib Cap",          &sv_max_gibs,       0, 32,   4,    "",  0, 1, 0, "Body parts flying at once. Impact: med."},
	{"Bot Think Hz",     &sv_bot_thinkrate,  0, 20,   5,    " Hz", 0, 1, 0, "How often bots make decisions. Impact: high."},
	{"Nail Cap",         &sv_max_needles,    0, 64,   8,    "",  0, 1, 0, "Nailgun shots flying at once. Impact: high."},
	{"Far Smoothing",    &r_lerpskip_dist,   0, 2400, 400,  "",  0, 1, 0, "Distant players move in steps, not smoothly. Impact: low."},
	{"Dynamic Lights",   &r_dynamic,         0, 1,    1,    "",  0, 0, 0, "Muzzle flashes light up the room. Impact: high."},
	{"Simple Flashes",   &gl_flashblend,     0, 1,    1,    "",  0, 0, 0, "Flashes drawn as simple colored glows. Impact: med."},
	{"Shadows",          &r_shadows,         0, 1,    1,    "",  0, 0, 0, "Shadow blobs under players and items. Impact: low."},
	{"Smooth Anim",      &r_lerpmodels,      0, 1,    1,    "",  0, 0, 0, "Smooths character animation between frames. Impact: low."},
};

static const char	*perf_preset_desc[4] =
{
	"Each option below is set by hand.",
	"Best looks, lowest framerate.",
	"Small limits on lights only, looks nearly the same.",
	"Big limits on effects and bots, plays smoothest.",
};

// Quality: every B1-B6 limiter off (pure stock behaviour), max fidelity.
// Balanced: safe caps only -- no bot throttle, no needle cap.
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

static void M_Performance_ApplyPreset (int p)	// 1..3
{
	int	i;
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
			Cvar_SetValueQuick (&perf_preset, 0);	// back to Custom: knobs stay where they are
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
}

static const char *M_Performance_Value (int i)
{
	const perf_knob_t	*k = &perf_knobs[i];
	float	v = k->cvar->value;
	if (k->maxval <= 1.0f)
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

void M_Performance_Key (int key)
{
	switch (key)
	{
	case K_YBUTTON: // triangle
#ifdef VITA
	case K_ENTER:
#endif
	case K_ESCAPE:
	case K_BBUTTON: // circle
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

// weapon position: same 4-state cycle as the old video menu (r75 era)
static const char *UNI_ShowWeaponPos (char *buf, int size)
{
	if (!r_drawviewmodel.value)
		q_strlcpy (buf, "Hidden", size);
	else if (r_viewmodeloffset.value > 0)
		q_strlcpy (buf, "Right", size);
	else if (r_viewmodeloffset.value < 0)
		q_strlcpy (buf, "Left", size);
	else
		q_strlcpy (buf, "Center", size);
	return buf;
}

static void UNI_SetWeaponPos (int dir)
{
	int idx;
	if (!r_drawviewmodel.value)
		idx = 3;
	else if (r_viewmodeloffset.value > 0)
		idx = 1;
	else if (r_viewmodeloffset.value < 0)
		idx = 2;
	else
		idx = 0;
	idx += dir;
	if (idx > 3)
		idx = 0;
	if (idx < 0)
		idx = 3;
	switch (idx)
	{
	case 1:
		Cvar_SetValueQuick (&r_drawviewmodel, 1);
		Cvar_SetValueQuick (&r_viewmodeloffset, 8);
		break;
	case 2:
		Cvar_SetValueQuick (&r_drawviewmodel, 1);
		Cvar_SetValueQuick (&r_viewmodeloffset, -8);
		break;
	case 3:
		Cvar_SetValueQuick (&r_drawviewmodel, 0);
		break;
	default:
		Cvar_SetValueQuick (&r_drawviewmodel, 1);
		Cvar_SetValueQuick (&r_viewmodeloffset, 0);
		break;
	}
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

static void UNI_ActionConsole (int dir)
{
	(void)dir;
	m_state = m_none;
	Con_ToggleConsole_f ();
}

static void UNI_ActionDefaults (int dir)
{
	(void)dir;
	if (SCR_ModalMessage("This will reset all controls\n"
			"and stored cvars. Continue? (y/n)\n", 15.0f))
	{
		Cbuf_AddText ("resetcfg\n");
		Cbuf_AddText ("exec default.cfg\n");
	}
}

static const uni_row_t uni_gfx_rows[] =
{
	{"Screen Size",     &scr_viewsize,  30, 120, 10,   0, 0, NULL, NULL, "Size of the 3D view in the window. Impact: none."},
	{"UI Scale",        &scr_menuscale,    1, 2.5, 0.1,  0, 0, UNI_SetScale, UNI_ShowScale, "Size of menus and status text. Impact: none."},
	{"Brightness",      &vid_gamma,   0.5,   1, 0.05,  1, 0, NULL, NULL, "Overall screen lightness. Impact: none."},
#ifndef VITA
	{"Contrast",        &vid_contrast,   1,   2, 0.1,  1, 0, NULL, NULL, "Difference between dark and bright. Impact: none."},
#endif
#ifdef VITA
	{"Field of View",   &scr_fov,       75, 130, 5,    0, 0, NULL, NULL, "How wide your view is. Impact: none."},
#endif
	{"Water Opacity",   &r_wateralpha,   0,   1, 0.1,  1, 0, NULL, NULL, "How clear the water looks. Impact: low."},
	{"Texture Filter",  NULL,            0,   1, 1,    0, 1, UNI_SetTexMode, UNI_ShowTexMode, "Smooth vs crisp textures. Impact: low."},
	{"Vertical Sync",   &vid_vsync,      0,   1, 1,    0, 0, NULL, NULL, "Match the display refresh, no tearing. Impact: none."},
	{"Weapon Position", NULL,            0,   3, 1,    0, 1, UNI_SetWeaponPos, UNI_ShowWeaponPos, "Where the gun sits on screen."},
	{"Show Crosshair",  &crosshair,      0,   1, 1,    0, 0, NULL, NULL, "Aim dot in the center of the screen. Impact: none."},
#ifdef VITA
	{"Rumble",              &pstv_rumble, 0, 1, 1, 0, 0, NULL, NULL, "Controller vibration on fire and hits."},
	{"Retro Touch",         &retrotouch,  0, 1, 1, 0, 0, NULL, NULL, "Old-style touchscreen aiming."},
	{"Gyroscope Aim",       &motioncam,   0, 1, 1, 0, 0, NULL, NULL, "Look by tilting the system."},
	{"Gyro Speed X",  &motion_horizontal_sensitivity, 0, 10, 1, 0, 0, NULL, NULL, "Tilt sensitivity left/right."},
	{"Gyro Speed Y",  &motion_vertical_sensitivity,   0, 10, 1, 0, 0, NULL, NULL, "Tilt sensitivity up/down."},
#endif
};

static const uni_row_t uni_ctrl_rows[] =
{
	{"Mouse Speed",    &sensitivity, 1, 11, 1, 0, 0, NULL, UNI_ShowSens, "Look speed with mouse or sticks. Impact: none."},
	{"Invert Mouse",   &m_pitch,    -1,  1, 1, 0, 1, UNI_SetInvert, UNI_ShowInvert, "Flip vertical look direction."},
	{"Mouse Look",     NULL,         0,  1, 1, 0, 1, UNI_SetMlook, UNI_ShowMlook, "Mouse turns the player (vs strafing)."},
	{"Lookspring",     &lookspring,  0,  1, 1, 0, 0, NULL, NULL, "View drifts back to center after looking. Impact: none."},
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
	{"Restore Defaults", NULL, 0, 1, 1, 0, 1, UNI_ActionDefaults, NULL, "Reset all controls and stored settings."},
};

static const uni_row_t	*uni_tab_rows[UNI_TABS] =
{ NULL, uni_gfx_rows, uni_ctrl_rows, uni_snd_rows, uni_gen_rows };
static const int uni_tab_nrows[UNI_TABS] =
{ 0,
  (int)(sizeof(uni_gfx_rows)   / sizeof(uni_gfx_rows[0])),
  (int)(sizeof(uni_ctrl_rows)  / sizeof(uni_ctrl_rows[0])),
  (int)(sizeof(uni_snd_rows)   / sizeof(uni_snd_rows[0])),
  (int)(sizeof(uni_gen_rows)   / sizeof(uni_gen_rows[0])) };

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

static void M_Options_CycleTab (int dir)
{
	S_LocalSound ("misc/menuoption.wav");
	uni_tab = (uni_tab + dir + UNI_TABS) % UNI_TABS;
	uni_cursor = uni_first = 0;
	M_Performance_ResetWindow ();
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

	// recursive background = the page we came from (same pattern as perf page)
	saved = m_state;
	m_state = uni_return_state;
	m_recursiveDraw = true;
	M_Draw ();
	m_state = saved;

	// opaque background for the whole options page (user: no translucency)
	Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 1.0f);

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
		segw = iw / UNI_TABS;
		for (i = 0; i < UNI_TABS; i++)
			if (i == uni_tab)
				Draw_FillRGBA (ix0 + i * segw, bandy + 2, segw, ih - 2,
					0.055f, 0.075f, 0.135f, 1.0f);	// solid, matches the body strip
		{	// same art + size as the matchmaking hints (M_HintIcon: native
			// aspect at 16px tall, 0.6 fade reads as "disabled tab" per user)
			int		lx = coords.x0 + (slotw - M_HintIconW (b_lt)) / 2;
			int		rx = coords.x0 + coords.panelw - slotw + (slotw - M_HintIconW (b_rt)) / 2;
			M_HintIcon (&lx, bandy + ih - 15, b_lt);
			M_HintIcon (&rx, bandy + ih - 15, b_rt);
		}
		for (i = 0; i < UNI_TABS; i++)
		{
			int	tw = (int)Draw_TTFHeaderWidth (uni_tab_names[i], 14.0f);
			int	tx = ix0 + i * segw + (segw - tw) / 2;
			int	ty = bandy + ih - 15;	// glyphs sit on the band's bottom edge
			if (i == uni_tab)
			{
				Draw_TTFStringHeader (tx, ty, uni_tab_names[i], 1, 1, 1, 1.0f, 14.0f);
				Draw_FillRGBA (ix0 + i * segw + 4, bandy + ih - 2, segw - 8, 2, 0.85f, 0.68f, 0.25f, 1.0f);
			}
			else
				Draw_TTFStringHeader (tx, ty, uni_tab_names[i], 0.55f, 0.58f, 0.65f, 1.0f, 14.0f);
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
	switch (k)
	{
	case K_ESCAPE:
	case K_BBUTTON: // circle
	case K_YBUTTON: // triangle
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

	case K_LTRIGGER:
		M_Options_CycleTab (1);
		return;
	case K_RTRIGGER:
		M_Options_CycleTab (-1);
		return;
#ifndef VITA
	// Mac convenience: Tab cycles, Q/E jump either way
	case K_TAB:
	case 'e':
	case 'E':
		M_Options_CycleTab (1);
		return;
	case 'q':
	case 'Q':
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
static char	custom_names[MAX_CUSTOM_MAPS][32];
static int	custom_count = -1;	// -1 = not scanned yet

static int custom_cmp (const void *a, const void *b)
{
	return q_strcasecmp ((const char *)a, (const char *)b);
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
		COM_StripExtension (dir_t->d_name, custom_names[custom_count], sizeof(custom_names[0]));
		custom_count++;
	}
	closedir (dir_p);
	if (custom_count > 1)
		qsort (custom_names, custom_count, sizeof(custom_names[0]), custom_cmp);
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
        q_snprintf(string, sizeof(string), "%-18.18s\n", custom_names[index]);
        return string;
    }
    if (index >= episodes[startepisode].levels)
        return "";

    q_snprintf(string, sizeof(string), "%-18.18s\n", levels[episodes[startepisode].firstLevel + index].description);

    return string;
}

void M_Matchmaking_Map_f (void) {
    key_dest = key_menu;
    m_state = m_matchmaking_map;
    m_entersound = true;
    cursorfocused = false;
    mlist_cursor = 0;
    mlist_first = 0;
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

    // background tint (perf B7: opaque, like the unified Options)
    Draw_FillRGBA(0,0,1000,1000, 0.02f, 0.02f, 0.025f, 1.0f);
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
     * (perf B7: custom pack has no info column) */
    if (startepisode != NUM_EPISODES - 1)
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
        M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "no custom maps: drop .bsp files in maps/custom");
    // perf B7: left gutter scrollbar (Options treatment)
    {
        int	rowh = coords.grid[1][1].y - coords.grid[1][0].y;
        if (rowh <= 0)
            rowh = MVS;
        M_Draw_Scrollbar (&coords, rowh, levcount, mlist_first, (int)mlist_shown);
    }
    // map thumbnail (perf B7: custom pack is list-only -- no art, bio or author;
    // its data does not live in levels[] so the whole info column would read garbage)
    if (startepisode == NUM_EPISODES - 1)
        return;
    char format[10];
    if (levels[episodes[startepisode].firstLevel + (mlist_cursor)].targaThumbnail) {
        strcpy(format, "tga"); // why not just bake it into the filename? why did I do it like this? testing? idk
    } else {
        strcpy(format, "lmp");
    }
    qpic_t *mappic = NULL;   /* r90: PNG mapshot if it exists, else legacy lmp/tga (Draw_TryCachePic never returns NULL -- gate on COM_FileExists) */
    {
        const char *mspath = va("gfx/maps/%s.png", levels[episodes[startepisode].firstLevel + (mlist_cursor)].thumbnail);
        if (COM_FileExists (mspath, NULL))
            mappic = Draw_TryCachePic (mspath, TEXPREF_ALPHA | TEXPREF_PAD | TEXPREF_NOPICMIP);
    }
    if (!mappic)
        mappic = Draw_CachePic(va("gfx/maps/%s.%s", levels[episodes[startepisode].firstLevel + (mlist_cursor)].thumbnail, format));
    Draw_StretchPic(coords.grid[1][0].x + 1, coords.grid[1][0].y, mappic, coords.colw, coords.rowh*5);

    // map bio (r92: single string, wrapped on words to the column width)
    {
        int bio_x = coords.grid[1][5].xp + CHARZ;   /* r93: padding off the column divider */
        int bio_maxpx = (int)coords.grid[coords.cols - 1][0].x + (int)coords.colw - bio_x - 16;   /* r93: wrap to the window edge, not the screen */
        const char *bio = levels[episodes[startepisode].firstLevel + (mlist_cursor)].mapBio;
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
            coords.grid[1][8].yp, levels[episodes[startepisode].firstLevel + (mlist_cursor)].author);
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
        case K_LTRIGGER:
        case K_LEFTARROW:
#ifndef VITA
        case 'q':	// perf B7: same tab aliases as unified Options
        case 'Q':
#endif
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor = 0;
            mlist_first = 0;
            M_Matchmaking_Map_Submenu_Key(-1,0);
            if (startepisode == NUM_EPISODES - 1)
                M_Maps_ScanCustom ();	// perf B7: refresh the folder list on tab entry
            break;
        case K_RTRIGGER:
        case K_RIGHTARROW:
#ifndef VITA
        case K_TAB:
        case 'e':
        case 'E':
#endif
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor = 0;
            mlist_first = 0;
            M_Matchmaking_Map_Submenu_Key(1,0);
            if (startepisode == NUM_EPISODES - 1)
                M_Maps_ScanCustom ();	// perf B7: refresh the folder list on tab entry
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
int matchmaking_cursor = 0; //indexing off 0

/* P2a lobby roster: bot count lives in the engine cvar lobby_bots so the
 * matchmaking menu can adjust it before a match exists (cl.scores is only
 * allocated on svc_serverinfo, see issue #41). Created in M_Init via
 * Cvar_Create so it always exists (QC cvar() on unset cvars reads 0).
 * Default 3 bots + you = 4 players of 8. Names come from bots.cfg
 * (bot_nameK cvars); QC reads the same cvars at BotConnect, so lobby and
 * in-game names match. */
static cvar_t	*lobby_bots;

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

/* r96: TDM pins teams red/blue; remember the player's profile colors the
 * first time the lobby opens so FFA matches can restore them. */
static qboolean lobby_profile_saved = false;
static char lobby_saved_bottom[8] = "";
static char lobby_saved_top[8] = "";

void M_Matchmaking_f (void)
{
    key_dest = key_menu;
    m_state = m_gameoptions;
    m_entersound = true;
    draw_grid_translucent = 1;   /* r92: overlay windows opt into opaque */
    if (!lobby_profile_saved)
    {
        lobby_profile_saved = true;
        q_strlcpy (lobby_saved_bottom, Cvar_VariableString ("bottomcolor"),
                   sizeof(lobby_saved_bottom));
        q_strlcpy (lobby_saved_top, Cvar_VariableString ("topcolor"),
                   sizeof(lobby_saved_top));
    }
    /* r97: leaving a TDM match re-pins nothing -- repair the live profile
     * back to what the lobby captured, so the ultrabright pin (era: 14)
     * never sticks in menus, rosters or config.cfg. */
    else if (strcmp (Cvar_VariableString ("bottomcolor"), lobby_saved_bottom)
          || strcmp (Cvar_VariableString ("topcolor"), lobby_saved_top))
    {
        Cbuf_AddText (va ("bottomcolor %s\n", lobby_saved_bottom));
        Cbuf_AddText (va ("topcolor %s\n", lobby_saved_top));
    }
    Cbuf_AddText ("exec bots.cfg\nlobby_shufflenames\n");   // P2a lobby roster names (r64: + shuffle)
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
     * canvas ignores bare alpha, so rgb is darkened along with it. */
    glColor4f (0.6f, 0.6f, 0.6f, 0.6f);
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
    qpic_t *icons[10];
    const char *words[10];
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
    } else if (type == SELECT_DELETE) {
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_abutton); HINTW("Edit");
        HINTI(b_ybutton); HINTW("Delete");
    } else if (type == PERF) {	// perf B7
        HINTI(b_bbutton); HINTW("Back");
        HINTI(b_left); HINTI(b_right); HINTW("Change");
        HINTI(b_up); HINTI(b_down); HINTW("Scroll");
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

void M_Draw_Hint (int type, struct MenuCoords mc)
{
    M_Draw_HintY (type, mc, mc.grid[0][mc.rows].yp);
}

void M_Matchmaking_Draw (void)
{
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
            const char *cname = (chosen_level.level < custom_count) ? custom_names[chosen_level.level] : "custom";
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
        matchmaking_cursor = 0;   /* r77: list shrank to 3 rows */
    draw_grid_inset = 1;          /* r77: 20% side margin (user) */
    struct MenuCoords mc = Draw_WindowGridLeft("Matchmaking",14,MVS*0.8f,2,0.2, 0.8, matchmaking_cursor, 1.0, 3.0);   /* r79b: edge to edge */
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
        if (matchmaking_cursor == 1)
            M_Draw_HintY(MATCH_SCROLL_TOP, mc, botbar);   /* nav on bottom */
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
            ? ((int)chosen_level.level < custom_count ? custom_names[chosen_level.level] : "Custom map")
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
        /* r62: FFA band pool mirroring HaloColorAt in frikbot/bot.qc; the
         * player owns band 14 (bottomcolor), so bots start at blue. */
        static const int ffa_bands[6] = {13, 12, 11, 6, 2, 4};   /* r97: 4 = safe red */
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
        Draw_TTFString (rx + CHARZ, y + nfs, "You", 1, 1, 1, 1, (float)nfs);
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
            Cvar_SetValue ("lobby_bots", opts.gametype == 1 ? 5 : 3);   /* r94: TDM lobby defaults to 6 total, slayer/swat to 4 */
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
            S_LocalSound ("misc/menuenter.wav");
            if (matchmaking_cursor == 2)
            {
//                S_LocalSound ("misc/menuback.wav"); // why was this here?

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
                Cbuf_AddText ( va ("map custom/%s\n", custom_names[chosen_level.level]) );
        }
        else
            Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + chosen_level.level].name) );
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

        /* bottom hint bar (window footer): Back + Select/Change */
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

    M_PrintWhite (mc.grid[0][1].xp, mc.grid[0][1].yp, "No Quake servers found");
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
	slist_sorted = false;
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
		slist_sorted = false;
		key_dest = key_game;
		m_state = m_none;
		IN_UpdateGrabs();
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
        if(sv.active)
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
	Cmd_AddCommand ("lobby_addbot", M_Lobby_AddBot_f);
	Cmd_AddCommand ("lobby_delbot", M_Lobby_DelBot_f);
	Cmd_AddCommand ("lobby_shufflenames", M_Lobby_ShuffleNames_f);
	// perf B7 - Performance menu: preset memory cvar + console shortcut
	Cvar_RegisterVariable (&perf_preset);
	Cmd_AddCommand ("perfmenu", M_Menu_Performance_f);
	Cmd_AddCommand ("menu_cmd", MQC_Command_f);
	Cmd_AddCommand ("menu_restart", M_MenuRestart_f);	//qss still loads progs on hunk, so we can't do this safely.

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
