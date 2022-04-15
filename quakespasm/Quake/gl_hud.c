/*
Copyright (C) 1996-1997 Id Software, Inc.

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
// gl_hud.c -- status bar code

#include "quakedef.h"

extern cvar_t	waypoint_mode;

qpic_t      *b_up;
qpic_t      *b_down;
qpic_t      *b_left;
qpic_t      *b_right;
qpic_t      *b_lthumb;
qpic_t      *b_rthumb;
qpic_t      *b_lshoulder;
qpic_t      *b_rshoulder;
qpic_t      *b_abutton;
qpic_t      *b_bbutton;
qpic_t      *b_ybutton;
qpic_t      *b_xbutton;
qpic_t      *b_lt;
qpic_t      *b_rt;
// health
qpic_t  *health_red;
qpic_t  *health_blue;
/*
 * Solitude vars
 */
//
// custom cvars for solitude
cvar_t cl_killmedals = {"cl_killmedals", "0"};
cvar_t cl_plasmanade = {"cl_plasmanade", "2"};
cvar_t cl_nadenum = {"cl_nadenum", "5"};
cvar_t cl_round = {"cl_round", "0"};
cvar_t cl_life = {"cl_life", "1"};
cvar_t cl_slowmo = {"cl_slowmo", "5"};
cvar_t cl_respawn = {"cl_respawn", "0"};
cvar_t cl_activate = {"cl_activate", "0"};	//For slowmo
cvar_t cl_scope = {"cl_scope", "0"};
cvar_t cl_ww = {"cl_ww", "0"};		//World weapon print message
cvar_t cl_ch_red = {"cl_ch_red", "0"};
cvar_t cl_ch_blue = {"cl_ch_blue", "1"};
cvar_t cl_overlay = {"cl_overlay", "0"};
cvar_t hide_hud = {"hide_hud", "0"};
//
////Weapon Icons
//qpic_t		*AR;
//qpic_t		*SMG;
//qpic_t		*Sniper;
//qpic_t		*RL;
//qpic_t		*Needler;
//qpic_t		*ppist;
//qpic_t		*Pistol;
//qpic_t		*Shottie;
////World weapons
//qpic_t		*ARW;
//qpic_t		*SMGW;
//qpic_t		*SniperW;
//qpic_t		*RLW;
//qpic_t		*NeedlerW;
//qpic_t		*ppistW;
//qpic_t		*PistolW;
//qpic_t		*ShottieW;
//
////Kill medals
qpic_t		*doublek;
qpic_t		*triple;
qpic_t		*killtrocity;
qpic_t		*killtacular;
//
////Grenade types
//qpic_t		*plasma;
//qpic_t		*frag;
//
//
////Health
//qpic_t		*life90;
//qpic_t		*lifered;
//qpic_t		*health_bar;
//qpic_t		*health_bar_red;
//
////Ammo counter
//qpic_t		*pistolb;
//qpic_t		*shotgunb;
//qpic_t		*arb;
//qpic_t		*needleb;
//qpic_t		*rocketb;
//qpic_t		*smgb;
//qpic_t		*sniperb;
//
////Slowmo graphic
//qpic_t		*clock[9];
//
////Grenade Numbers
//qpic_t		*numbers[5];
//
////Crosshairs
//
////red
//qpic_t		*arred;
//qpic_t		*smgred;
//qpic_t		*sniperred;
//qpic_t		*rlred;
//qpic_t		*pistolred;
//qpic_t		*ppistred;
//qpic_t		*needlerred;
//qpic_t		*plriflered;
//qpic_t		*shotgunred;
//qpic_t		*swordred;
//
////blue
//qpic_t		*arblue;
//qpic_t		*smgblue;
//qpic_t		*sniperblue;
//qpic_t		*rlblue;
//qpic_t		*pistolblue;
//qpic_t		*ppistblue;
//qpic_t		*needlerblue;
//qpic_t		*plrifleblue;
//qpic_t		*shotgunblue;
//qpic_t		*swordblue;
//
//qpic_t		*plasmabar;
//
//typedef struct
//{
//    int points;
//    int negative;
//    float x;
//    float y;
//    float move_x;
//    float move_y;
//    double alive_time;
//} point_change_t;
//
//int  x_value, y_value;
//
//point_change_t point_change[10];

void HUD_Init (void) {
    //Client variables
    Cvar_RegisterVariable (&cl_killmedals);	//Kill medals
    Cvar_RegisterVariable (&cl_plasmanade); //Grenade types
    Cvar_RegisterVariable (&cl_nadenum);	//A counter for your grenades.
    Cvar_RegisterVariable (&cl_ww);	//World weapons
    Cvar_RegisterVariable (&cl_round);
    Cvar_RegisterVariable (&cl_life);
    Cvar_RegisterVariable (&cl_slowmo);
    Cvar_RegisterVariable (&cl_activate);
    Cvar_RegisterVariable (&cl_scope);
    Cvar_RegisterVariable (&cl_respawn);
    Cvar_RegisterVariable (&cl_ch_red);
    Cvar_RegisterVariable (&cl_ch_blue);
    Cvar_RegisterVariable (&cl_overlay);
    Cvar_RegisterVariable (&hide_hud);

    // buttons
    if (sceKernelGetModel() == 0x20000) { // PSTV
		b_lt = Draw_CachePic ("gfx/butticons/backl1_pstv.tga");
		b_rt = Draw_CachePic ("gfx/butticons/backr1_pstv.tga");
	} else {
		b_lt = Draw_CachePic ("gfx/butticons/backl1.tga");
		b_rt = Draw_CachePic ("gfx/butticons/backr1.tga");
	}
	b_lthumb = Draw_CachePic ("gfx/butticons/backl1.tga"); // Not existent
	b_rthumb = Draw_CachePic ("gfx/butticons/backl1.tga"); // Not existent
	b_lshoulder = Draw_CachePic ("gfx/butticons/backl1.tga"); // Not existent
	b_rshoulder = Draw_CachePic ("gfx/butticons/backl1.tga"); // Not existent
	b_left = Draw_CachePic ("gfx/butticons/dpadleft.tga");
	b_right = Draw_CachePic ("gfx/butticons/dpadright.tga");
	b_up = Draw_CachePic ("gfx/butticons/dpadup.tga");
	b_down = Draw_CachePic ("gfx/butticons/dpaddown.tga");
	b_abutton = Draw_CachePic ("gfx/butticons/fbtncross.tga");
	b_bbutton = Draw_CachePic ("gfx/butticons/fbtncircle.tga");
	b_ybutton = Draw_CachePic ("gfx/butticons/fbtnsquare.tga");
	b_xbutton = Draw_CachePic ("gfx/butticons/fbtntriangle.tga");

    // health
    health_blue = Draw_CachePic ("gfx/hud/health/health_blue.lmp");
    health_red = Draw_CachePic ("gfx/hud/health/health_red.lmp");

    //Kill medals
    doublek = Draw_CachePic ("gfx/medals/double.lmp");
    triple = Draw_CachePic ("gfx/medals/triple.lmp");
    killtacular = Draw_CachePic ("gfx/medals/killtacular.lmp");
    killtrocity = Draw_CachePic ("gfx/medals/killtrocity.lmp");
}

float * rgb(float r, float g, float b) {
    static float rgb[4];
    rgb[0] = r/255.0;
    rgb[1] = g/255.0;
    rgb[2] = b/255.0;
    return rgb;
}

/*
=============
HUD_itoa
=============
*/
int HUD_itoa (int num, char *buf)
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

char *getWeaponName(int w) {
    switch (w) {
        case IT_NAILGUN:
            return "Assault Rifle";
        case IT_LIGHTNING:
            return "Plasma Pistol";
        case IT_SUPER_SHOTGUN:
            return "Shotgun";
        case IT_SUPER_LIGHTNING:
            return "Needler";
        case IT_ROCKET_LAUNCHER:
            return "Rocket Launcher";
        case IT_SHOTGUN:
            return "Pistol";
        case IT_SUPER_NAILGUN:
            return "SMG";
        case IT_GRENADE_LAUNCHER:
            return "Sniper";
        case IT_AXE:
            return "Energy Sword";
        default:
            return "Unknown";
    }
}

char *getPickupWeaponString (int value) {
    switch (value) {
        case 1:
            return "pickup Assault Rifle";
        case 2:
            return "pickup Plasma Pistol";
        case 3:
            return "pickup Shotgun";
        case 4:
            return "pickup Needler";
        case 5:
            return "pickup Rocket Launcher";
        case 6:
            return "pickup Pistol";
        case 7:
            return "pickup SMG";
        case 8:
            return "pickup Sniper";
        case 9: //todo: add pickup CL QuakeC code for energy sword
            return "pickup Energy Sword";
        default:
            return "";
    }
}

void drawCrosshair(void) {
    //nothing
}

void drawPickupWeapon(void) {
    char *string = getPickupWeaponString((int)cl_ww.value);
    Draw_ColoredStringScale(glwidth*0.65, glheight*0.6, string, 127/255.0, 191/255.0, 255/255.0,1,0.8f*scr_sbarscale.value);
}
void drawWeapon(void) {
    char primary[30];
    char rounds[10];
    char ammo[10];
    HUD_itoa((int)cl.stats[STAT_AMMO], rounds);
    HUD_itoa((int)cl.stats[STAT_ARMOR], ammo);
    strcpy(primary, getWeaponName((int)cl.stats[STAT_ACTIVEWEAPON]));
    Draw_ColoredStringScale(glwidth*0.7, glheight*0.8, primary, 127/255.0, 191/255.0, 255/255.0,1,1.0f*scr_sbarscale.value);
    Draw_ColoredStringScale(glwidth*0.7, glheight*0.75, rounds, 171/255.0, 231/255.0, 255/255.0,1,1.0f*scr_sbarscale.value);
    Draw_ColoredStringScale(glwidth*0.77, glheight*0.75, ammo, 171/255.0, 231/255.0, 255/255.0,1,0.8f*scr_sbarscale.value);
}

void drawGrenades() {
    char nadename[10];
    char nadecount[2];
    int count = (int)cl_nadenum.value;
    if ((int)cl_plasmanade.value == 1) {
        strcpy(nadename, "Plasma");
    } else if ((int)cl_plasmanade.value == 2) {
        strcpy(nadename, "Frags");
    }
    HUD_itoa(count, nadecount);
    Draw_ColoredStringScale(glwidth*0.7, glheight*0.85, nadecount, 171/255.0, 231/255.0, 255/255.0,1,1.0f*scr_sbarscale.value);
    Draw_ColoredStringScale(glwidth*0.75, glheight*0.85, nadename, 171/255.0, 231/255.0, 255/255.0,1,1.0f*scr_sbarscale.value);
}

int oldsheilds;
int sheildrechargeplaying = 0;
int sheildbleepplaying = 0;
int nullplaying = 0;
int sheilds_channel;

void drawHealth(void)
{
    if(cl.stats <= 0 || deathmatch.value == DM_SWAT) {
        return;
    }
    int sheilds = (int)cl.stats[STAT_HEALTH];
    struct PicAttr health_blue_attr = getHudPicAttr(0.5, 0.1, health_blue);
    struct PicAttr health_red_attr = getHudPicAttr(0.5, 0.1, health_red);

    for (float i = 0; i < (int)cl.stats[STAT_HEALTH]; i++) {
        float color = i <= 30 ? 230 /* red */: 214 /* blue */;
        Draw_Fill(health_blue_attr.x + health_blue_attr.width*0.05, health_blue_attr.y + health_blue_attr.height*0.1, (i/130.0f)*health_blue_attr.width*0.9, health_blue_attr.height*0.80, color, 1);
    }

    if ((int)deathmatch.value == DM_SLAYER || (int)deathmatch.value == DM_FIREFIGHT)
    {
        // start sounds
        if (sheilds > oldsheilds && !sheildrechargeplaying)
        {
            S_LocalSound("hud/sheild_recharge.wav");
            sheildrechargeplaying = 1;
        }
        if (sheilds > oldsheilds && !nullplaying)
        {
            S_LocalSound("hud/null.wav");
            nullplaying = 1;
        }
        if (sheilds < 40 && !sheildbleepplaying && sheilds > 20)
        {
            S_LocalSound("hud/sheilds_low_bleep.wav");
            sheildbleepplaying = 1;
        }
        // stop sounds
        if (sheilds == 40 && sheildrechargeplaying)
        {
            sheildrechargeplaying = 0;
            S_StopSound (cl.viewentity, 1);
        }
        if (sheilds == 100 && nullplaying)
        {
            nullplaying = 0;
            S_StopSound (cl.viewentity, 1);
        }
        if (sheilds > 40 && sheildbleepplaying)
        {
            sheildbleepplaying = 0;
            S_StopSound (cl.viewentity, 0);
        }
        oldsheilds = sheilds;
    }
    // draw health box
    if ((int)cl.stats[STAT_HEALTH] <= 30)
    {
        Draw_HudPic(health_red_attr, health_red);
    }
    else
    {
//        Draw_ColoredStringScale(glwidth*0.5, glheight*0.4, "test", 1,1,1,1,3.0f);
        Draw_HudPic(health_blue_attr, health_blue);
    }
}

//=============================================================================

struct teaminfo {
    int scoreTotal;
    int color;
};
int		fragsort[MAX_SCOREBOARD];
struct teaminfo     teamscores[2];
extern char	scoreboardtext[MAX_SCOREBOARD][20];
extern int		scoreboardtop[MAX_SCOREBOARD];
extern int		scoreboardbottom[MAX_SCOREBOARD];
extern int		scoreboardcount[MAX_SCOREBOARD];
extern int		scoreboardlines;

void SortFrags (void)
{
    int		i, j, k;

// sort by frags
    scoreboardlines = 0;
    for (i = 0; i < cl.maxclients; i++)
    {
        if (cl.scores[i].name[0])
        {
            fragsort[scoreboardlines] = i;
            scoreboardlines++;
        }
    }

    for (i = 0; i < scoreboardlines; i++)
    {
        for (j = 0; j < scoreboardlines - 1 - i; j++)
        {
            if (cl.scores[fragsort[j]].frags < cl.scores[fragsort[j+1]].frags)
            {
                k = fragsort[j];
                fragsort[j] = fragsort[j+1];
                fragsort[j+1] = k;
            }
        }
    }
}

// broken
void TeamSort (void)
{
    int		i, j, k;

    //setup players to sort
    scoreboardlines = 0;
    for (i = 0; i < cl.maxclients; i++)
    {
        if (cl.scores[i].name[0])
        {
            fragsort[scoreboardlines] = i;
            scoreboardlines++;
        }
    }

    // sort by color
    for (i = 0; i < scoreboardlines; i++)
    {
        for (j = 0; j < scoreboardlines - 1 - i; j++)
        {
            if (cl.scores[fragsort[j]].colors < cl.scores[fragsort[j+1]].colors)
            {
                k = fragsort[j];
                fragsort[j] = fragsort[j+1];
                fragsort[j+1] = k;
            }
        }
    }
//    char text[256];
//    HUD_itoa(scoreboardlines, text);
//    q_snprintf(text, sizeof (text), "scoreboardlines=%d\n", scoreboardlines);
//    Cbuf_AddText(text);
}

int	ColorForMap (int m)
{
    return m < 128 ? m + 8 : m + 8;
}
/*
==================
Sbar_DeathmatchOverlay
==================
*/
void MiniDeathmatchOverlay (void)
{
    int				i, k, l;
    int				bottom;
    float			y;
    int f;
    char			num[12];
    scoreboard_t	*s;

    y = 0.0;

//    scr_copyeverything = 1;
//    scr_fullupdate = 0;


    // draw the text
    l = scoreboardlines;
    if (l > 2)
    {
        l = 2;
    }

    // draw game type
    char gametype[20];
    if (deathmatch.value == DM_SLAYER && teamplay.value == 0) {
        strcpy(gametype, "Slayer");
    } else if (deathmatch.value == DM_SLAYER && teamplay.value == 1) {
        strcpy(gametype, "Team Slayer");
    } else if (deathmatch.value == DM_SWAT && teamplay.value == 0) {
        strcpy(gametype, "Swat");
    } else {
        strcpy(gametype, "You Broke My Code");
    }
    Draw_ColoredStringScale(glwidth*0.08, glheight*(0.75), gametype, 171/255.0, 231/255.0, 255/255.0,1,1.0f*scr_sbarscale.value);

    // begin draw scores
    if (teamplay.value) {
        TeamSort();
        struct teaminfo teamA;
        struct teaminfo teamB;
        teamA.color = cl.scores[0].colors;
        teamB.color = -1;
        teamA.scoreTotal = 0;
        teamB.scoreTotal = 0;
        // add up scores
        for (i = 0; i < cl.maxclients; i++)
        {
            if (cl.scores[i].name[0])
            {
                if (cl.scores[0].colors == cl.scores[i].colors) {
                    teamA.scoreTotal = teamA.scoreTotal + cl.scores[i].frags;
                } else {
                    teamB.scoreTotal = teamB.scoreTotal + cl.scores[i].frags;
                    teamB.color = cl.scores[i].colors;
                }
                scoreboardlines++;
            }
        }
        // sort. lol.
        if (teamA.scoreTotal > teamB.scoreTotal || (teamA.scoreTotal == teamB.scoreTotal && teamA.color == cl.scores[0].colors)) {
            teamscores[0] = teamA;
            teamscores[1] = teamB;
        } else {
            teamscores[0] = teamB;
            teamscores[1] = teamA;
        }
//        if (teamscores[0].scoreTotal == fraglimit.value || teamscores[1].scoreTotal == fraglimit.value) {
//            // HACK: force intermission once fraglimit is hit, since I cannot decompile the QuakeC code! :'(
//            cl.intermission = 1;
//            Cbuf_AddText("disconnect");
//        }

        for (i=0; i < 2 && teamB.color >= 0; i++) //only show two lines
        {
            // draw background
            bottom = (teamscores[i].color & 15) <<4;
            bottom = ColorForMap (bottom);

            Draw_Fill (glwidth*0.08, glheight*(0.8+y), glwidth*0.08*scr_sbarscale.value, 8*scr_sbarscale.value, bottom, 1);

            // draw number
            f = teamscores[i].scoreTotal;
            HUD_itoa(f, num);

            Draw_ColoredStringScale(glwidth*0.08 + 8*scr_sbarscale.value, glheight*(0.8+y), num, 1, 1, 1,1,0.8f*scr_sbarscale.value);

            if (teamscores[i].color == cl.scores[0].colors)
                Draw_CharacterScale( glwidth*0.08 - 9*scr_sbarscale.value, glheight*(0.8+y), 13, 1.0f*scr_sbarscale.value);
            y += 0.06;
        }
    } else { // slayer or swat
        SortFrags ();
        for (i=0; i<l; i++) //only show two lines
        {
            k = fragsort[i];
            s = &cl.scores[k];
            if (!s->name[0])
                continue;

            // draw background
            bottom = (s->colors & 15) <<4;
            bottom = ColorForMap (bottom);

            Draw_Fill (glwidth*0.08, glheight*(0.8+y), glwidth*0.08*scr_sbarscale.value, 8*scr_sbarscale.value, bottom, 1);

            // draw number
            f = s->frags;
            HUD_itoa(f, num);

            Draw_ColoredStringScale(glwidth*0.08 + 8*scr_sbarscale.value, glheight*(0.8+y), num, 1, 1, 1,1,0.8f*scr_sbarscale.value);

            if (k == cl.viewentity - 1)
                Draw_CharacterScale( glwidth*0.08 - 9*scr_sbarscale.value, glheight*(0.8+y), 13, 1.0f*scr_sbarscale.value);

            y += 0.06;
        }
    }
}

void drawMedal (void)
{
    qpic_t *drawmedal;
    switch ((int)cl_killmedals.value) {
        case 1:
            drawmedal = doublek;
            break;
        case 2:
            drawmedal = triple;
            break;
        case 3:
            drawmedal = killtacular;
            break;
        case 4:
            drawmedal = killtrocity;
    }
    // todo: this should stretch\scale with scr_sbarscale.value
    M_DrawTransPic(glwidth*0.08, glheight*0.5, drawmedal);
}

void HUD_Draw (void) {
    int oldcanvas = currentcanvas;
    GL_SetCanvas(CANVAS_DEFAULT);
    drawHealth();
    drawWeapon();
    drawGrenades();
    drawMedal();
    drawCrosshair();
    drawPickupWeapon();
    if (!*cl.scores[1].name) {
        Draw_ColoredStringScale(0, glheight-(8*0.8f*scr_sbarscale.value), "Its too quiet, Chief! Add bots to get started.", 1,1,1,1,0.8f*scr_sbarscale.value);
    }
    if (deathmatch.value == DM_SLAYER || deathmatch.value == DM_SWAT)
        MiniDeathmatchOverlay ();
    GL_SetCanvas(oldcanvas);
}