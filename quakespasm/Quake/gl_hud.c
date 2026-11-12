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
qpic_t  *healthbar_red;
qpic_t  *healthbar_inside;
qpic_t  *healthbar_br;
/*
 * Solitude vars
 */
//
// custom cvars for solitude
cvar_t cl_killmedals = {"cl_killmedals", "0"};
cvar_t cl_plasmanade = {"cl_plasmanade", "2"};
cvar_t cl_nadenum = {"cl_nadenum", "4"};
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
    healthbar_inside = Draw_CachePic ("gfx/hud/health/healthbar_inside.tga");
    healthbar_br = Draw_CachePic ("gfx/hud/health/healthbar_br.tga");
    healthbar_red = Draw_CachePic ("gfx/hud/health/healthbar_red.tga");

    //Kill medals
    doublek = Draw_CachePic ("gfx/medals/double.lmp");
    triple = Draw_CachePic ("gfx/medals/triple.lmp");
    killtacular = Draw_CachePic ("gfx/medals/killtacular.lmp");
    killtrocity = Draw_CachePic ("gfx/medals/killtrocity.lmp");

    GLFont_Init ();	/* P2a: TTF HUD text (gl_font.c); conchars fallback if absent */
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
        case IT_EXTRA_WEAPON2:
            return "Plasma Rifle";
        default:
            return "Unknown";
    }
}

char *getPickupWeaponString (int value) {
    switch (value) {
        case 1:
            return "Assault Rifle";
        case 2:
            return "Plasma Pistol";
        case 3:
            return "Shotgun";
        case 4:
            return "Needler";
        case 5:
            return "Rocket Launcher";
        case 6:
            return "Pistol";
        case 7:
            return "SMG";
        case 8:
            return "Sniper";
        case 9:
            return "Battle Rifle";
        case 10:
            return "Energy Sword";
        case 11:
            return "Plasma Rifle";
        default:
            return "";
    }
}

void drawCrosshair(void) {
    //nothing
}

void drawPickupWeapon(void) {
    char *string = getPickupWeaponString((int)cl_ww.value);
    if (strcmp(string,"") != 0) {
        Draw_ButtonScaled(glwidth*0.67, glheight*0.90, b_ybutton, 0.8f*HUD_SCALE());
    }
    Draw_TTFString(glwidth*0.7, glheight*0.90 - 8*HUD_SCALE(), string, 1, 1, 1,1, 14.0f*HUD_SCALE());
}

void drawWeapon(void) {
    char primary[30];
    char rounds[10];
    char ammo[10];
    HUD_itoa((int)cl.stats[STAT_AMMO], rounds);
    HUD_itoa((int)cl.stats[STAT_ARMOR], ammo);
    strcpy(primary, getWeaponName((int)cl.stats[STAT_ACTIVEWEAPON]));
    Draw_TTFString(glwidth*0.7, glheight*0.10, primary, 127/255.0, 191/255.0, 255/255.0,1, 16.0f*HUD_SCALE());
    if (strcmp(primary, "Energy Sword") != 0 /* 0 means equal*/) { // dont draw ammo for energy sword
        Draw_TTFString(glwidth*0.7, glheight*0.16 - 8*HUD_SCALE(), rounds, 171/255.0, 231/255.0, 255/255.0,1, 14.0f*HUD_SCALE());
        Draw_TTFString(glwidth*0.77, glheight*0.16 - 8*HUD_SCALE(), ammo, 171/255.0, 231/255.0, 255/255.0,1, 11.0f*HUD_SCALE());
    }
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
    /* halo3 text-state rule (no icon art exists): zero count dims to gray */
    if (count == 0)
    {
        Draw_TTFString(glwidth*0.03, glheight*0.12 - 8*HUD_SCALE(), nadecount, 0.45, 0.45, 0.45, 0.75, 14.0f*HUD_SCALE());
        Draw_TTFString(glwidth*0.08, glheight*0.12 - 8*HUD_SCALE(), nadename, 0.45, 0.45, 0.45, 0.75, 14.0f*HUD_SCALE());
    }
    else
    {
        Draw_TTFString(glwidth*0.03, glheight*0.12 - 8*HUD_SCALE(), nadecount, 171/255.0, 231/255.0, 255/255.0,1, 14.0f*HUD_SCALE());
        Draw_TTFString(glwidth*0.08, glheight*0.12 - 8*HUD_SCALE(), nadename, 171/255.0, 231/255.0, 255/255.0,1, 14.0f*HUD_SCALE());
    }
}

int oldsheilds;
int sheildrechargeplaying = 0;
int sheildbleepplaying = 0;
int nullplaying = 0;
int sheilds_channel;

/* P2a: Halo-style procedural shield capsule (chamfered octagon fan).
 * Replaces the health_blue/health_red .lmp frames + the old per-hitpoint
 * overdraw loop (up to 130 Draw_Fill passes per frame). Same GL state
 * recipe as Draw_Fill so it composites identically under CANVAS_DEFAULT. */
static void Draw_GeomCapsule (float x, float y, float w, float h, float c,
                              float r, float g, float b, float a)
{
    glDisable (GL_TEXTURE_2D);
    glEnable (GL_BLEND);
    glDisable (GL_ALPHA_TEST);
    glColor4f (r, g, b, a);
    glBegin (GL_TRIANGLE_FAN);
        glVertex2f (x + w*0.5f, y + h*0.5f);
        glVertex2f (x + c,     y);
        glVertex2f (x + w - c, y);
        glVertex2f (x + w,     y + c);
        glVertex2f (x + w,     y + h - c);
        glVertex2f (x + w - c, y + h);
        glVertex2f (x + c,     y + h);
        glVertex2f (x,         y + h - c);
        glVertex2f (x,         y + c);
        glVertex2f (x + c,     y);
    glEnd ();
    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);
}

void drawHealth(void)
{
    if(cl.stats <= 0 || deathmatch.value == DM_SWAT || sb_showscores) {
        return;
    }
    int sheilds = (int)cl.stats[STAT_HEALTH];
    struct PicAttr health_blue_attr = getHudPicAttr(0.5, 0.1, health_blue);

    /* Halo 3 manual, shields: solid blue when healthy, flash red when
     * depleted. The mod's bar has always been fed by STAT_HEALTH; keep the
     * old 130-oversize fraction so >100 looks the same as before. */
    {
        float hp    = sheilds;
        float frac  = hp / 130.0f;
        float inset, cham, tflash;
        qboolean hurt;
        if (frac > 1.0f) frac = 1.0f;
        if (frac < 0.0f) frac = 0.0f;
        inset = health_blue_attr.height * 0.18f;
        cham  = health_blue_attr.height * 0.34f;
        hurt  = (sheilds <= 30);
        tflash = realtime * 3.0f;
        tflash -= (float)((int)tflash);

        /* dark navy backing capsule */
        Draw_GeomCapsule(health_blue_attr.x, health_blue_attr.y,
                         health_blue_attr.width, health_blue_attr.height, cham,
                         0.05f, 0.06f, 0.12f, 0.80f);
        /* value fill: Halo blue, or red flashing when depleted */
        if (frac > 0)
            Draw_GeomCapsule(health_blue_attr.x + inset, health_blue_attr.y + inset,
                             (health_blue_attr.width - 2*inset) * frac,
                             health_blue_attr.height - 2*inset, cham * 0.7f,
                             hurt ? 0.85f : 0.25f,
                             hurt ? (tflash > 0.5f ? 0.20f : 0.10f) : 0.62f,
                             hurt ? 0.18f : 0.95f,
                             1.0f);
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
    /* P2a: the .lmp box frames (health_blue/health_red) are retired —
     * the geometry capsule above draws frame + fill; art stays cached for
     * nothing else now, removed from HUD_Init in the relocation pass. */
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
    float			boxw, SX;
    int f;
    char			num[12];
    scoreboard_t	*s;

    y = 0.0;

    /* P2a relocation: score cluster anchors bottom-RIGHT (Halo 3 manual
     * multiplayer-info position), mirroring the author's 8% margin. */
    boxw = (960.0*0.08)*HUD_SCALE();
    SX   = glwidth*0.92 - boxw;

//    scr_copyeverything = 1;
//    scr_fullupdate = 0;


    // draw the text
    l = scoreboardlines;
    if (l > 2)
    {
        l = 2;
    }
    // time
    char	str[80];
    int		minutes, seconds, tens, units, timex;
    if (timelimit.value > 0) {
        minutes = (timelimit.value*60 - cl.time) / 60;
        seconds = (timelimit.value*60 - cl.time) - 60*minutes;
        tens = seconds / 10;
        units = seconds - 10*tens;
        sprintf (str,"%3i:%i%i", minutes, tens, units);
        timex = SX;
        if (minutes < 10) {
            timex += - (CHARZ*2);
        } else if (minutes < 100) {
            timex += - (CHARZ*1);
        }
        Draw_TTFString(timex, glheight*(0.71), str, 171/255.0, 231/255.0, 255/255.0, 1, 12.0f*HUD_SCALE());
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
    Draw_TTFString(SX, glheight*(0.75), gametype, 127/255.0, 191/255.0, 255/255.0, 1, 12.0f*HUD_SCALE());

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
        // draw scores. only show two lines
        for (i=0; i < 2 && teamB.color >= 0; i++)
        {
            // draw background
            bottom = (teamscores[i].color & 15) <<4;
            bottom = ColorForMap (bottom);
            Draw_Fill (SX-1, glheight*(0.8+y)-1, boxw+2, 1.5*CHARZ*HUD_SCALE()+2, bottom, 0.3);
            Draw_Fill (SX, glheight*(0.8+y), boxw, 1.5*CHARZ*HUD_SCALE(), bottom, 1);
            // draw foreground
            Draw_Fill (SX, glheight*(0.8+y), boxw*CLAMP (new_max (teamscores[i].scoreTotal/(fraglimit.value > 0 ? fraglimit.value : new_max(teamscores[0].scoreTotal, 1.0f)), 0), 0.0f, 1.0f), 1.5*CHARZ*HUD_SCALE(), bottom, 0.65);
            // draw score
            f = teamscores[i].scoreTotal;
            HUD_itoa(f, num);
            Draw_TTFString(SX + 9*HUD_SCALE(), glheight*(0.8+y) + 1.0f*HUD_SCALE(), num, 1, 1, 1, 1, 11.0f*HUD_SCALE());
            // draw caret to indicate own team
            if (teamscores[i].color == cl.scores[0].colors)
                Draw_CharacterScale(SX + boxw + (glwidth*0.08 > 9.0f*HUD_SCALE() + 2.0f ? 9.0f*HUD_SCALE() : 2.0f), glheight*(0.8+y)+(1.1f*HUD_SCALE()*0.5), 13, 1.1f*HUD_SCALE());
            y += (544.0*0.03)*HUD_SCALE()/glheight;
        }
        // send cvar to QuakeC to enter intermission/NextMap logic, once score limit reached
        if (teamscores[0].scoreTotal >= fraglimit.value) {
            teamlimit.value = 1;
            Cbuf_AddText ( va ("teamlimit %u\n", (int)teamlimit.value) );
        }
    } else { // slayer (ffa) or swat
        SortFrags ();
        for (i=0; i<l; i++) //only show two lines
        {
            k = fragsort[i];
            s = &cl.scores[k];
            if (!s->name[0])
                continue;


            bottom = (s->colors & 15) <<4;
            bottom = ColorForMap (bottom);
            f = s->frags;
            HUD_itoa(f, num);
            // draw background
            Draw_Fill (SX-1, glheight*(0.8+y)-1, boxw+2, 1.5*CHARZ*HUD_SCALE()+2, bottom, 0.3);
            Draw_Fill (SX, glheight*(0.8+y), boxw, 1.5*CHARZ*HUD_SCALE(), bottom, 1);
            // draw foreground
            Draw_Fill (SX, glheight*(0.8+y), boxw*CLAMP (new_max (f/(fraglimit.value > 0 ? fraglimit.value : new_max(cl.scores[fragsort[0]].frags, 1.0f)), 0), 0.0f, 1.0f), 1.5*CHARZ*HUD_SCALE(), bottom, 0.65);
            // draw score
            Draw_TTFString(SX + 9*HUD_SCALE(), glheight*(0.8+y) + 1.0f*HUD_SCALE(), num, 1, 1, 1, 1, 11.0f*HUD_SCALE());
            // draw caret to indicate own team
            if (k == cl.viewentity - 1)
                Draw_CharacterScale(SX + boxw + (glwidth*0.08 > 9.0f*HUD_SCALE() + 2.0f ? 9.0f*HUD_SCALE() : 2.0f), glheight*(0.8+y)+(1.1f*HUD_SCALE()*0.5), 13, 1.1f*HUD_SCALE());
            y += (544.0*0.03)*HUD_SCALE()/glheight;
        }
    }
}

void drawMedal (void)
{
    qpic_t *drawmedal = NULL;
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
            break;
        default:
            return;	/* no medal queued (cl_killmedals==0) */
    }
    if (!drawmedal)
        return;		/* missing art must not crash the HUD */
    /* scaled with the sbar, per the old todo: same anchor model as the
     * health pics (fraction of free space, art scaled by HUD_SCALE) */
    Draw_HudPic (getHudPicAttr (0.08, 0.5, drawmedal), drawmedal);
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
//    if (!*cl.scores[1].name) {
//        Draw_ColoredStringScale(0, glheight-(8*0.8f*HUD_SCALE()), "Its too quiet, Chief! Add bots to get started.", 1,1,1,1,0.8f*HUD_SCALE());
//    }
//    if (deathmatch.value == DM_SLAYER || deathmatch.value == DM_SWAT)
//        MiniDeathmatchOverlay ();
    GL_SetCanvas(oldcanvas);
}