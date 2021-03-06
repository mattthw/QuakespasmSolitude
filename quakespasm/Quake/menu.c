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

qpic_t *menu_bg;

extern cvar_t scr_fov;

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

void M_Print (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
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
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p);
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
    Draw_Fill(main_x_offset_pixels, main_y_offset_pixels, main_pixel_width, main_pixel_height, BG_COLOR, 0.5); //blue bg
    Draw_Fill(main_x_offset_pixels, PixHeight(1)-MVS, main_pixel_width, MVS, 1, 0.5); //black bar bottom
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

    M_DrawScrollString(0,0,320*MENU_SCALE, "Welcome to Halo Solitude! This port is a work in progress.");

    if (main_multi)
        M_Main_Multi_Draw ();
}

void M_Main_Multi_Draw (void)
{
    //background
    Draw_Fill(multi_x_offset_pixels, main_y_offset_pixels, multi_pixel_width, multi_pixel_height, GREY, 0.5);
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
    if (deathmatch.value == 2) {
        return 2;
    } else {
        return 4;
    }
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

    if ((int)deathmatch.value == 2) //firefight
    {
        struct MenuCoords coords = Draw_WindowGrid("Options", 2, MVS_P, 1, 0.35, 1, pause_cursor, false);
        M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "Settings");
        M_PrintWhite (coords.grid[0][1].xp, coords.grid[0][1].yp, "Disconnect");
    } else {
        struct MenuCoords coords = Draw_WindowGrid("Options", 4, MVS_P, 1, 0.35, 1, pause_cursor, false);
        M_PrintWhite (coords.grid[0][0].xp, coords.grid[0][0].yp, "Add Bot");
        M_PrintWhite (coords.grid[0][1].xp, coords.grid[0][1].yp, "Remove Bot");
        M_PrintWhite (coords.grid[0][2].xp, coords.grid[0][2].yp, "Settings");
        M_PrintWhite (coords.grid[0][3].xp, coords.grid[0][3].yp, "Disconnect");
    }
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
        case K_ENTER:
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
            S_LocalSound ("misc/menuenter.wav");
            if ((int)deathmatch.value == 2) //firefight
            {
                if(pause_cursor == 0)
                {
                    M_Menu_Options_f ();
                }
                if(pause_cursor == 1)
                {
                    Cbuf_AddText ("disconnect\n");
                    theme_started = 0;
                    M_Menu_Firefight_f(); // todo: do I need to do more than this (probably). Why not use entry menu f?
                }
            } else {
                if(pause_cursor == 0)
                {
                    addBot();
                }
                if(pause_cursor == 1)
                {
                    Cbuf_AddText ("impulse 102\n");
                }
                if(pause_cursor == 2)
                {
                    M_Menu_Options_f ();
                }
                if(pause_cursor == 3)
                {
                    Cbuf_AddText ("disconnect\n");
                    theme_started = 0;
                    M_Matchmaking_f(); // todo: same as above
                }
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

void M_Options_Draw (void)
{
	float		r, l;
    float alpha = 0.8;
    if (sv.active)
        alpha = 1;
    if (!sv.active)
        Draw_MenuBg();

    struct MenuCoords mc = Draw_WindowGrid("Options", 19, MVS*0.6, 2, 0.4, alpha, options_cursor, true);
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

	// OPT_VIDEO:
	if (vid_menudrawfn)
        M_PrintWhite (mc.grid[0][18].xp ,mc.grid[0][18].yp, "Advanced Options");

}


void M_Options_Key (int k)
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

    Draw_Fill(0,0,1000,1000, BLACK, 0.667);
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
        Draw_Fill(0,0,1000,1000, BLACK, 0.667);
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
    const char  *mapBio[4];
    const char  *author;
} level_t;

level_t		levels[] =
{
        // Refined map pack [4]
        {"narrows", "random", 0, "Narrows (wip)", {"","","",""}, "Mattthw"},
        {"highground", "random", 0, "High Ground", {"","","",""}, "Mattthw"},
        {"citadel", "citadel", 0, "Citadel",{"A Forerunner","structure built","in Epitaph.",""}, "Matt"}, //0
        {"babybloodgulch", "babybloodgulch", 1, "Blood Gutch", {"You don't know","Blood Gulch? Hey!","This guy doesn't","know Blood Gulch!"}, "Unknown"},
        {"pit3", "pit2", 1, "Pit", {"Training ground","for UNSC forces.","Located somewhere","in Africa."}, "Jukki, Matt"},
        {"narsp", "narsp", 1, "Narrows", {"One of the Ark's","cooling systems.","Enables life","on the Construct"}, "Scifiknux"},
        {"skyringbl2", "skyringbl2", 1, "Heavenly Suspense", {"One of the many","Sky ring facilities.","Damaged by","recent conflict"}, "Arkage"},
        {"wizard", "wizard", 1, "Wizard", {"Round and Round","and Round.","",""}, "IlDucci"},
        {"lockdown", "lockdown", 1, "Lockdown", {"","","", ""}, "Team Xlink, Matt"},
        {"lane", "lane", 1, "Lane", {"Outdoor human facility","used for USMC","training.",""}, "Sam"},
        {"cestra", "cestra", 1, "Cestra", {"Experimental lab","used by USMC","researchers.",""}, "Sam"},
        // Classic map pack [6]
        {"Longest", "Longest", 0, "Longest", {"","","",""}, "Unknown"},
        {"chill", "random", 0, "Chill Out", {"","","",""}, "Unknown"},
        {"pri2", "random", 0, "Prisoner 2", {"","","",""}, "Unknown"},
        {"base", "base", 0, "Minibase", {"","","",""}, "Unknown"},
        {"plaza", "plaza", 0, "Plaza", {"","","",""}, "Unknown"},
        {"spider", "spider", 0, "Spiderweb", {"","","",""}, "Unknown"},
        // Bonus map pack
        {"pit", "random", 0, "Pit (original)", {"","","",""}, "Jukki"},
        {"pit2", "pit2", 1, "Pit (v2)", {"","","",""}, "Jukki, Matt"},
        {"lock", "lockout", 0, "Lockout2", {"","","",""}, "Unknown"},
        {"+bloodgulch2", "bgulch", 0, "Blood Gutch", {"You don't know","Blood Gulch? Hey!","This guy doesn't","know Blood Gulch!"}, "Unknown"},

        //{"lockout", "lockout", "Lockout"},

        // Firefight map pack
        {"construction", "construction", 0, "Construction", {"","","",""}, "Unknown"},
        {"fire", "fire", 0, "Fire!", {"","","",""}, "Unknown"}
};

typedef struct
{
	const char	*description;
	int		firstLevel;
	int		levels;
} episode_t;
int NUM_EPISODES = 3;
episode_t	episodes[] =
        {
                {"Remastered Maps", 0, 11},
                {"Classic Maps", 11, 6},
                {"Bonus Maps", 17, 4}
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
        .fraglimit = 10,
        .teamlimit = 0,
        .gametype = SLAYER,
        /* CUSTOM GAME OPTIONS */
        .custom_primary = 0,
        .custom_secondary = 1,
        .map_weapons_disable = 0,
        .max_nades = 4,
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
    opts.fraglimit = 4;
    opts.skill = 1;
    opts.max_nades = 4;
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
float rule_list_size = 11; //Should equal sizeof(rule_titles)/sizeof(rule_titles[0])
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
    struct rule_option_t rule_titles[11] = {
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

    // background tint
    Draw_Fill(0,0,1000,1000, BLACK, 0.667);

    // window
    struct MenuCoords coords = Draw_WindowGrid("Custom Game Rules", vertical_gamerules_affordance, MVS_P, 2, 0.35, 1.0, gamerules_cursor-gamerules_first, true);
    // footer
    switch (gamerules_cursor) {
        case 10:
            M_Draw_Hint(SELECT_RULES, coords);
            break;
        default:
            M_Draw_Hint(SCROLL_RULES, coords);
    }

    // draw rules
    for (int n = 0; n < gamerules_shown; n++) {
        M_PrintWhite (coords.grid[0][n].xp, coords.grid[0][n].yp, rule_titles[gamerules_first+n].name);        M_Print (coords.grid[1][n].xp, coords.grid[1][n].yp, rule_titles[gamerules_first+n].value);
    }
}

void M_Matchmaking_Gamerules_Submenu_Key (int dir) {
    switch (gamerules_cursor)
    {
        case 0: //custom primary
            opts.custom_primary += dir;
            if (opts.custom_primary >= sizeof(gamerule_weapons)/sizeof(gamerule_weapons[0]))
                opts.custom_primary = 0;
            if (opts.custom_primary < 0 && dir < 0)
                opts.custom_primary = sizeof(gamerule_weapons)/sizeof(gamerule_weapons[0])-1;
            break;
        case 1: //custom secondary
//            opts.custom_secondary += dir; todo: fix custom secondary QuakeC code
            if (opts.custom_secondary >= sizeof(gamerule_weapons_secondary)/sizeof(gamerule_weapons_secondary[0]))
                opts.custom_secondary = 0;
            if (opts.custom_secondary < 0 && dir < 0)
                opts.custom_secondary = sizeof(gamerule_weapons_secondary)/sizeof(gamerule_weapons_secondary[0])-1;
            break;
        case 2: //ground weapons
            opts.map_weapons_disable += dir;
            if (opts.map_weapons_disable > 1)
                opts.map_weapons_disable = 0;
            if (opts.map_weapons_disable < 0)
                opts.map_weapons_disable = 1;
            break;
        case 3: //difficulty
            opts.skill += dir;
            if (opts.skill > 3)
                opts.skill = 0;
            if (opts.skill < 0)
                opts.skill = 3;
            break;
        case 4: //max grenades
            opts.max_nades += dir;
            if (opts.max_nades > 8)
                opts.max_nades = 0;
            if (opts.max_nades < 0)
                opts.max_nades = 8;
            break;
        case 5: //infinite ammo
            opts.infinite_ammo += dir;
            if (opts.infinite_ammo > 1)
                opts.infinite_ammo = 0;
            if (opts.infinite_ammo < 0)
                opts.infinite_ammo = 1;
            break;
        case 6: //grunt birthday
            opts.grunt_birthday_party += dir;
            if (opts.grunt_birthday_party > 1)
                opts.grunt_birthday_party = 0;
            if (opts.grunt_birthday_party < 0)
                opts.grunt_birthday_party = 1;
            break;
        case 7: //gravity
            opts.sol_gravity += dir*10;
            if (opts.sol_gravity > 500)
                opts.sol_gravity = 100;
            if (opts.sol_gravity < 100)
                opts.sol_gravity = 500;
            break;
        case 8: //max speed
            opts.sol_maxspeed += dir*10;
            if (opts.sol_maxspeed > 500)
                opts.sol_maxspeed = 100;
            if (opts.sol_maxspeed < 100)
                opts.sol_maxspeed = 500;
            break;
        case 9: //jump
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
            switch (gamerules_cursor) {
                case 10:
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
                gamerules_cursor = 10;
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

size_t		mlist_cursor;
size_t		mlist_first;
float vertical_mlist_affordance = 10;
const char *M_listPrintMap (size_t index) {

    static char	string[64];
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

void M_Matchmaking_Map_Draw (void) {

    // set to matchmaking for recursive draw
    m_state = m_gameoptions;
    m_recursiveDraw = true;
    M_Draw ();
    // now draw this menu
    m_state = m_matchmaking_map;

    // setup scrolling cursor
    size_t	mlist_shown;
    mlist_shown = episodes[startepisode].levels;
    if (mlist_shown > vertical_mlist_affordance)
        mlist_shown = vertical_mlist_affordance;
    if (mlist_first+mlist_shown-1 < mlist_cursor)
        mlist_first = mlist_cursor-(mlist_shown-1);
    if (mlist_first > mlist_cursor)
        mlist_first = mlist_cursor;

    // background tint
    Draw_Fill(0,0,1000,1000, BLACK, 0.667);
    // title
    static char title[128];
    q_snprintf(title, sizeof(title), "%-18.18s [%-15.15s]\n", "Map Selection", episodes[startepisode].description);
    // window
    struct MenuCoords coords = Draw_WindowGrid(title, vertical_mlist_affordance, MVS_P, 2, 0.35, 1.0, mlist_cursor-mlist_first, true);
    //footer
    switch (mlist_cursor) {
        default:
            M_Draw_Hint(SELECT_SCROLL_MAPS, coords);
    }
    // map name
    for (int n = 0; n < mlist_shown; n++) {
        M_PrintWhite (coords.grid[0][n].xp, coords.grid[0][n].yp, M_listPrintMap (mlist_first+n));
    }
    // map thumbnail
    char format[10];
    if (levels[episodes[startepisode].firstLevel + (mlist_cursor)].targaThumbnail) {
        strcpy(format, "tga"); // why not just bake it into the filename? why did I do it like this? testing? idk
    } else {
        strcpy(format, "lmp");
    }
    qpic_t *mappic = Draw_CachePic(va("gfx/maps/%s.%s", levels[episodes[startepisode].firstLevel + (mlist_cursor)].thumbnail, format));
    Draw_StretchPic(coords.grid[1][0].x + 1, coords.grid[1][0].y, mappic, coords.colw, coords.rowh*5);

    // map bio
    int map_y = coords.grid[1][5].yp;
    M_Print(coords.grid[1][5].x, coords.grid[1][5].yp, "Map Description:");
    map_y += CHARZ * 1.5;
    M_PrintWhite (coords.grid[1][5].xp, map_y,  levels[episodes[startepisode].firstLevel + (mlist_cursor)].mapBio[0]);
    map_y += CHARZ * 1.5;
    M_PrintWhite (coords.grid[1][5].xp, map_y,  levels[episodes[startepisode].firstLevel + (mlist_cursor)].mapBio[1]);
    map_y += CHARZ * 1.5;
    M_PrintWhite (coords.grid[1][5].xp, map_y, levels[episodes[startepisode].firstLevel + (mlist_cursor)].mapBio[2]);
    map_y += CHARZ * 1.5;
    M_PrintWhite (coords.grid[1][5].xp, map_y, levels[episodes[startepisode].firstLevel + (mlist_cursor)].mapBio[3]);
    M_Print(coords.grid[1][8].x, coords.grid[1][8].yp, "Author:");
    M_PrintWhite (coords.grid[1][9].xp, coords.grid[1][8].yp + CHARZ*1.5, levels[episodes[startepisode].firstLevel + (mlist_cursor)].author);
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
            chosen_level.episode = startepisode;
            chosen_level.level = mlist_cursor;
            m_state = m_gameoptions;
            cursorfocused = true;
            S_LocalSound ("misc/menuoption.wav");
            break;
        case K_UPARROW:
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor--;
            if (mlist_cursor >= episodes[startepisode].levels)
                mlist_cursor = episodes[startepisode].levels - 1;
            break;
        case K_DOWNARROW:
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor++;
            if (mlist_cursor >= episodes[startepisode].levels)
                mlist_cursor = 0;
            break;
        case K_LTRIGGER:
        case K_LEFTARROW:
            S_LocalSound ("misc/menuoption.wav");
            mlist_cursor = 0;
            mlist_first = 0;
            M_Matchmaking_Map_Submenu_Key(-1,0);
            break;
        case K_RTRIGGER:
        case K_RIGHTARROW:
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

#define	NUM_GAMEOPTIONS	7 //indexing off 1
int matchmaking_cursor = 0; //indexing off 0

void M_Matchmaking_f (void)
{
    key_dest = key_menu;
    m_state = m_gameoptions;
    m_entersound = true;
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

void M_Draw_Hint (int type, struct MenuCoords mc) {
    // footer text hint
    if (type == SCROLL) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back      Change");
        Draw_Button(mc.grid[0][mc.rows].xp + 1*CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9*CHARZ, mc.grid[0][mc.rows].yp, b_left);
        Draw_Button(mc.grid[0][mc.rows].xp + 11*CHARZ, mc.grid[0][mc.rows].yp, b_right);
    } else if (type == SELECT) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back    Select");
        Draw_Button(mc.grid[0][mc.rows].xp + 1*CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9*CHARZ, mc.grid[0][mc.rows].yp, b_abutton);
    }else if (type == SCROLL_RULES) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back      Change       Scroll     Defaults");
        Draw_Button(mc.grid[0][mc.rows].xp + 1*CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9*CHARZ, mc.grid[0][mc.rows].yp, b_left);
        Draw_Button(mc.grid[0][mc.rows].xp + 11*CHARZ, mc.grid[0][mc.rows].yp, b_right);
        Draw_Button(mc.grid[0][mc.rows].xp + 22*CHARZ, mc.grid[0][mc.rows].yp, b_up);
        Draw_Button(mc.grid[0][mc.rows].xp + 24*CHARZ, mc.grid[0][mc.rows].yp, b_down);
        Draw_Button(mc.grid[0][mc.rows].xp + 35*CHARZ, mc.grid[0][mc.rows].yp, b_xbutton); // triangle
    } else if (type == SELECT_RULES) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back      Select       Scroll     Defaults");
        Draw_Button(mc.grid[0][mc.rows].xp + 1*CHARZ, mc.grid[0][mc.rows].yp, b_bbutton); // o
        Draw_Button(mc.grid[0][mc.rows].xp + 11*CHARZ, mc.grid[0][mc.rows].yp, b_abutton); // x
        Draw_Button(mc.grid[0][mc.rows].xp + 22*CHARZ, mc.grid[0][mc.rows].yp, b_up);
        Draw_Button(mc.grid[0][mc.rows].xp + 24*CHARZ, mc.grid[0][mc.rows].yp, b_down);
        Draw_Button(mc.grid[0][mc.rows].xp + 35*CHARZ, mc.grid[0][mc.rows].yp, b_xbutton); // triangle
    } else if (type == INPUT) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back    Edit    Delete");
        Draw_Button(mc.grid[0][mc.rows].xp + 1*CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9*CHARZ, mc.grid[0][mc.rows].yp, b_xbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 17*CHARZ, mc.grid[0][mc.rows].yp, b_ybutton);
    } else if (type ==  SELECT_SCROLL) {
        M_PrintWhite(mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back    Select      Change");
        Draw_Button(mc.grid[0][mc.rows].xp + 1 * CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9 * CHARZ, mc.grid[0][mc.rows].yp, b_abutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 19 * CHARZ, mc.grid[0][mc.rows].yp, b_left);
        Draw_Button(mc.grid[0][mc.rows].xp + 21 * CHARZ, mc.grid[0][mc.rows].yp, b_right);
    } else if (type == SELECT_SCROLL_MAPS) {
        M_PrintWhite(mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back    Select      Change Map Pack");
        Draw_Button(mc.grid[0][mc.rows].xp + 1 * CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9 * CHARZ, mc.grid[0][mc.rows].yp, b_abutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 19 * CHARZ, mc.grid[0][mc.rows].yp, b_left);
        Draw_Button(mc.grid[0][mc.rows].xp + 21 * CHARZ, mc.grid[0][mc.rows].yp, b_right);
    } else if (type == SELECT_DELETE) {
        M_PrintWhite (mc.grid[0][mc.rows].xp, mc.grid[0][mc.rows].yp, "    Back    Edit    Delete");
        Draw_Button(mc.grid[0][mc.rows].xp + 1*CHARZ, mc.grid[0][mc.rows].yp, b_bbutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 9*CHARZ, mc.grid[0][mc.rows].yp, b_abutton);
        Draw_Button(mc.grid[0][mc.rows].xp + 17*CHARZ, mc.grid[0][mc.rows].yp, b_ybutton);
    }
}

void M_Matchmaking_Draw (void)
{
    Draw_MenuBg();
    char format[10];
    if (levels[episodes[chosen_level.episode].firstLevel + chosen_level.level].targaThumbnail) {
        strcpy(format, "tga");
    } else {
        strcpy(format, "lmp");
    }
    qpic_t *mappic = Draw_CachePic(va("gfx/maps/%s.%s", levels[episodes[chosen_level.episode].firstLevel + chosen_level.level].thumbnail, format));
    int fg_height = MM_HEIGHT_PIX - (MM_HEADER_HEIGHT + MM_FOOTER_HEIGHT +mappic->height);

    //background
    struct MenuCoords mc = Draw_WindowGridLeft("Matchmaking",11,MVS,2,0.2, 0.8, matchmaking_cursor, 1.0, 3.0);
    // map thumbnail
    double picheight = mc.rowh*4;
    double picwidth = (picheight)*1.7; //lets keep 16x9 ratio
    double pic_x = ((mc.colw*mc.cols)-picwidth)/2;
    Draw_StretchPic(pic_x, mc.grid[1][mc.rows].y-picheight, mappic, picwidth, picheight);

    switch (matchmaking_cursor) {
        case 1:
        case 2:
        case 3:
            M_Draw_Hint(SCROLL, mc);
            break;
        default:
            M_Draw_Hint(SELECT, mc);
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
    M_PrintWhite (mc.grid[0][0].xp, mc.grid[0][1].yp, "GAME");
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
    //======================== 2
    M_PrintWhite (mc.grid[0][2].xp, mc.grid[0][2].yp, "SCORE");
    if (opts.fraglimit == 0)
        M_Print (mc.grid[1][2].xp, mc.grid[1][2].yp, "Unlimited");
    else if (opts.fraglimit < 1000)
        M_Print (mc.grid[1][2].xp, mc.grid[1][2].yp, va("%i Kills", (int)opts.fraglimit));
    else
        M_Print (mc.grid[1][2].xp, mc.grid[1][2].yp, "Rounds");
    //======================== 3
    M_PrintWhite (mc.grid[0][3].xp, mc.grid[1][3].yp, "TIME");
    if (opts.timelimit == 0)
        M_Print (mc.grid[1][3].xp, mc.grid[1][3].yp, "Unlimited");
    else
        M_Print (mc.grid[1][3].xp, mc.grid[1][3].yp, va("%i Minutes", (int)opts.timelimit));
    //======================== 4
    M_PrintWhite(mc.grid[0][4].xp, mc.grid[0][4].yp, "Edit Game Options");
    //======================== 5
    M_PrintWhite (mc.grid[0][5].xp, mc.grid[1][5].yp, "SELECT MAP");
    char finalMapName[12];
    Q_strncpy(finalMapName, levels[episodes[chosen_level.episode].firstLevel + chosen_level.level].description, 12);
    if (Q_strlen(levels[episodes[chosen_level.episode].firstLevel + chosen_level.level].description) >= 12) {
        finalMapName[11] = '.';
        finalMapName[10] = '.';
        finalMapName[9] = '.';
    }
    float mapnamescale = 1.5f;
    Draw_ColoredStringScale((mc.colw*2 - picwidth)/2 + /*padding*/4, mc.grid[1][mc.rows].yp - mc.rowh*1 - 0.0625*8*mapnamescale, finalMapName, 0.5, 0.5, 0.5, 0.3, mapnamescale);
    //========================= 6
    Draw_ColoredStringScale(mc.grid[0][6].xp, mc.grid[1][6].yp, "START GAME", 1, 1, 1, 1, 1.1f);
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
        case K_ENTER:
        case K_ESCAPE:
        case K_BBUTTON:
        case K_YBUTTON:
            M_Menu_Main_f ();
            break;

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
            if (matchmaking_cursor >= 5 || matchmaking_cursor == 0)
                break;
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Submenu_Key (-1);
            break;

        case K_RIGHTARROW:
            if (matchmaking_cursor >= 5 || matchmaking_cursor == 0)
                break;
            S_LocalSound ("misc/menuoption.wav");
            M_Matchmaking_Submenu_Key (1);
            break;

        case K_ABUTTON:
            S_LocalSound ("misc/menuenter.wav");
            if (matchmaking_cursor == 6)
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
                Cbuf_AddText ( va ("fraglimit %u\n", (int)opts.fraglimit) );
                Cbuf_AddText ( va ("timelimit %u\n", (int)opts.timelimit) );
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
                Cbuf_AddText ( va ("map %s\n", levels[episodes[startepisode].firstLevel + chosen_level.level].name) );
                return;
            } else if (matchmaking_cursor == 5) {
                M_Matchmaking_Map_f();
            } else if (matchmaking_cursor == 0) {
                m_multiplayer_cursor = 1;
                M_Menu_LanConfig_f();
            } else if (matchmaking_cursor == 4) {
                M_Matchmaking_Gamerules_f();
            }

            // todo: safe to remove the below line?
            M_Matchmaking_Submenu_Key (1);
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
    qpic_t	*p, *gt, *bg, *bar, *skillp, *mapfire, *mapconstruction, *maprandom;
    int		x, chg;

    Draw_MenuBg();

    struct MenuCoords mc = Draw_WindowGridLeft("Firefight",11,MVS,2,0.2, 0.8, firefight_cursor, 1.0, 3.0);

    qpic_t *randmap = Draw_CachePic("gfx/maps/random.lmp");
    int fg_feight = MM_HEIGHT_PIX - (MM_HEADER_HEIGHT + MM_FOOTER_HEIGHT +randmap->height);

    // footer text hint
    switch (firefight_cursor) {
        case 0:
            M_Draw_Hint(SCROLL, mc);
            break;
        default:
            M_Draw_Hint(SELECT, mc);
    }

    M_Print (mc.grid[0][0].xp, mc.grid[0][0].yp, "MAP");
    M_PrintWhite (mc.grid[1][0].xp - mc.colw/2, mc.grid[1][0].yp, levels[episodes_ff[startepisode].firstLevel + startlevel].description);
    M_PrintWhite (mc.grid[0][1].xp, mc.grid[0][1].yp, "START GAME");
    // draw image
    qpic_t *mappic = Draw_CachePic(va("gfx/maps/%s.lmp", levels[episodes_ff[startepisode].firstLevel + startlevel].thumbnail));
    M_DrawTransPic(((mc.colw*mc.cols)-mappic->width)/2, mc.grid[0][mc.rows].y-mappic->height, mappic);
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
        case K_ENTER:
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
