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
    char prompt[64];
    float s = HUD_SCALE();
    if (strcmp(string,"") == 0)
        return;
    sprintf(prompt, "Pick up/Replace with %s.", string);
    /* Y button box is 16*0.8 design px; center it on the text line instead of
     * Draw_ButtonScaled's -4 fudge (that was for 8px conchars rows).
     * TTF baseline sits at glheight*0.24 - 8*s; cap-center ~ 5*s above it. */
    Draw_StretchPic((int)(glwidth*0.67), (int)(glheight*0.26 - 8*s - 4*s - 7.2f*s),
                    b_ybutton, (int)(14.0f*s), (int)(14.0f*s));
    Draw_TTFString(glwidth*0.7, glheight*0.26 - 8*s, prompt, 1, 1, 1, 1, 16.0f*s);
}

void drawWeapon(void) {
    char primary[30];
    char rounds[10];
    char ammo[10];
    HUD_itoa((int)cl.stats[STAT_AMMO], rounds);
    HUD_itoa((int)cl.stats[STAT_ARMOR], ammo);
    strcpy(primary, getWeaponName((int)cl.stats[STAT_ACTIVEWEAPON]));
    /* r19: nudged right (0.70/0.77 -> 0.72/0.79) per user 2026-11-12 */
    Draw_TTFString(glwidth*0.72, glheight*0.10, primary, 127/255.0, 191/255.0, 255/255.0,1, 22.0f*HUD_SCALE());
    if (strcmp(primary, "Energy Sword") != 0 /* 0 means equal*/) { // dont draw ammo for energy sword
        Draw_TTFString(glwidth*0.72, glheight*0.19 - 8*HUD_SCALE(), rounds, 171/255.0, 231/255.0, 255/255.0,1, 19.0f*HUD_SCALE());
        Draw_TTFString(glwidth*0.79, glheight*0.19 - 8*HUD_SCALE(), ammo, 171/255.0, 231/255.0, 255/255.0,1, 15.0f*HUD_SCALE());
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
        Draw_TTFString(glwidth*0.03, glheight*0.12 - 8*HUD_SCALE(), nadecount, 0.45, 0.45, 0.45, 0.75, 19.0f*HUD_SCALE());
        Draw_TTFString(glwidth*0.08, glheight*0.12 - 8*HUD_SCALE(), nadename, 0.45, 0.45, 0.45, 0.75, 19.0f*HUD_SCALE());
    }
    else
    {
        Draw_TTFString(glwidth*0.03, glheight*0.12 - 8*HUD_SCALE(), nadecount, 171/255.0, 231/255.0, 255/255.0,1, 19.0f*HUD_SCALE());
        Draw_TTFString(glwidth*0.08, glheight*0.12 - 8*HUD_SCALE(), nadename, 171/255.0, 231/255.0, 255/255.0,1, 19.0f*HUD_SCALE());
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

/* P2a r15 (user: "its still an octogon... I said follow the / and \\ with
 * the cut", "diagonal glitch lines", "weird neon blue hue"): the cell is a
 * MIRROR TRAPEZOID — one full-height / on the left end (x,y+h)->(x+d,y) and
 * its mirror \ on the right. No vertical end faces, no extra corner cuts.
 * Every layer passes the SAME d and layers nest by plain insets, so all
 * diagonals are parallel — r14's per-layer mismatched bevels were the
 * glitch lines. One shared vertical ramp (ry, rh); calm Halo blues, not
 * neon.
 * r17 (user: "the trapezoid should have rounded corners"): every corner is
 * replaced by a 3-point quadratic-Bezier fillet (entry, mid, exit), giving
 * a 12-gon fanned from vertex 0. Entry/exit points lie ON the original
 * edges, so nested layers keep parallel slants; s is clamped to 38% of the
 * shorter of top-flat/diagonal so thin fills stay near-sharp instead of
 * self-overlapping. Vertex colors are still exact samples of one global
 * vertical ramp, so the fan stays linear-accurate (no radial artifacts). */
static void Draw_TrapezoidGrad (float x, float y, float w, float h, float d,
                                float rt, float gt, float bt,
                                float rb, float gb, float bb, float a,
                                float ry, float rh)
{
    float cx[4], cy[4];
    float px[12], py[12];
    int i, n = 0;
    float s, top, diag;

    if (d * 2.0f > w) d = w * 0.5f;

    cx[0] = x + d;     cy[0] = y;
    cx[1] = x + w - d; cy[1] = y;
    cx[2] = x + w;     cy[2] = y + h;
    cx[3] = x;         cy[3] = y + h;

    top  = w - 2.0f * d;
    diag = sqrtf (d * d + h * h);
    s = h * 0.18f;
    if (s > 0.38f * top)  s = 0.38f * top;
    if (s < 0.0f) s = 0.0f;
    if (s > 0.38f * diag) s = 0.38f * diag;

    for (i = 0; i < 4; i++)
    {
        int p = (i + 3) & 3, q = (i + 1) & 3;
        float ux = cx[p] - cx[i], uy = cy[p] - cy[i];
        float vx = cx[q] - cx[i], vy = cy[q] - cy[i];
        float ul = sqrtf (ux*ux + uy*uy), vl = sqrtf (vx*vx + vy*vy);
        float ex, ey, xx, xy;
        if (ul > 0.0f) { ux /= ul; uy /= ul; }
        if (vl > 0.0f) { vx /= vl; vy /= vl; }
        ex = cx[i] + ux * s;  ey = cy[i] + uy * s;
        xx = cx[i] + vx * s;  xy = cy[i] + vy * s;
        px[n] = ex; py[n] = ey; n++;
        px[n] = (ex + 2.0f*cx[i] + xx) * 0.25f;   /* Bezier t=0.5 */
        py[n] = (ey + 2.0f*cy[i] + xy) * 0.25f; n++;
        px[n] = xx; py[n] = xy; n++;
    }

    glDisable (GL_TEXTURE_2D);
    glEnable (GL_BLEND);
    glDisable (GL_ALPHA_TEST);
    glBegin (GL_TRIANGLE_FAN);
        for (i = 0; i <= n; i++)
        {
            int k = (i == n) ? 0 : i;
            float t = (py[k] - ry) / rh;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            glColor4f (rt + (rb-rt)*t, gt + (gb-gt)*t, bt + (bb-bt)*t, a);
            glVertex2f (px[k], py[k]);
        }
    glEnd ();
    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);
}

/* P2a r9: all 10%% segment ticks in ONE quad batch (no per-tick state
 * churn; user: "should not make the game lag"). r15: ticks are slanted
 * parallel to the trapezoid ends ("/" like the left edge, same d over the
 * fill height), spaced 1/10 of the full inner width, stopping at the fill
 * edge. r16: width + color are params — the dark 2px ladder read as
 * "ghetto", now 1px light-blue hairlines at low alpha. */
static void Draw_CapsuleTicks (float x, float y, float fw, float h, float fullw, float d,
                               float r, float g, float b, float al)
{
    int i;
    glDisable (GL_TEXTURE_2D);
    glEnable (GL_BLEND);
    glDisable (GL_ALPHA_TEST);
    glColor4f (r, g, b, al);
    glBegin (GL_QUADS);
    for (i = 1; i < 10; i++)
    {
        float tx = x + fullw * (i * 0.1f);
        if (tx >= x + fw - 1.0f)
            break;
        glVertex2f (tx,         y + h);
        glVertex2f (tx + 1,     y + h);
        glVertex2f (tx + 1 + d, y);
        glVertex2f (tx + d,     y);
    }
    glEnd ();
    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);
}

void drawHealth(void)
{
    /* r19: the death guard was always false ('cl.stats' is an array — the
     * pointer compare never fires); mirror sbar.c:841 and test STAT_HEALTH.
     * Leaderboard (sb_showscores) hide was already intended. */
    if(cl.stats[STAT_HEALTH] <= 0 || deathmatch.value == DM_SWAT || sb_showscores) {
        return;
    }
    int sheilds = (int)cl.stats[STAT_HEALTH];
    struct PicAttr health_blue_attr = getHudPicAttr(0.5, 0.1, health_blue);

    /* Halo 3 manual, shields: solid blue when healthy, flash red when
     * depleted. The mod's bar has always been fed by STAT_HEALTH; keep the
     * old 130-oversize fraction so >100 looks the same as before.
     * P2a tier-1: gradient fill, 1/10 segment ticks, recharge sweep and a
     * white hit-flash (user 2026-11-12). */
    {
        static float shownfrac   = -1.0f;
        static float dmgflash    = 0.0f;
        static int   prevsheilds = -1;
        float hp    = sheilds;
        float frac  = hp / 130.0f;
        float inset, slant, tflash, fw;
        qboolean depleted;
        if (frac > 1.0f) frac = 1.0f;
        if (frac < 0.0f) frac = 0.0f;
        inset = health_blue_attr.height * 0.18f;
        slant = health_blue_attr.height * 0.35f;
        depleted = (sheilds <= 30);
        tflash = realtime * 3.0f;
        tflash -= (float)((int)tflash);

        if (shownfrac < 0.0f) shownfrac = frac;
        if (prevsheilds >= 0 && sheilds < prevsheilds) dmgflash = 0.18f;
        prevsheilds = sheilds;
        dmgflash -= host_frametime;
        if (dmgflash < 0.0f) dmgflash = 0.0f;
        { float k = (float)host_frametime * 8.0f; if (k > 1.0f) k = 1.0f;
          shownfrac += (frac - shownfrac) * k; }
        fw = (health_blue_attr.width - 2*inset) * shownfrac;

        /* P2a r15: mirror-trapezoid layers sharing ONE slant d so every
         * diagonal is parallel (kills the "glitch lines"), calm translucent
         * blues ("neon" was too hot). Depleted: hollow bar, outline itself
         * flashes red at 3 Hz — r14b behavior kept. All ramp via ry/rh over
         * the full bar height — r14 kept. */
        if (depleted)
        {
            float pk = (tflash > 0.5f) ? 1.0f : 0.35f;
            Draw_TrapezoidGrad (health_blue_attr.x, health_blue_attr.y,
                                health_blue_attr.width, health_blue_attr.height, slant,
                                1.00f, 0.30f + 0.20f*pk, 0.26f + 0.10f*pk,
                                0.62f, 0.08f, 0.10f,
                                0.28f + 0.30f*pk,
                                health_blue_attr.y, health_blue_attr.height);
        }
        else
        {
            Draw_TrapezoidGrad (health_blue_attr.x, health_blue_attr.y,
                                health_blue_attr.width, health_blue_attr.height, slant,
                                0.35f, 0.55f, 0.78f, 0.12f, 0.28f, 0.48f, 0.40f,
                                health_blue_attr.y, health_blue_attr.height);
        }
        Draw_TrapezoidGrad (health_blue_attr.x + 2, health_blue_attr.y + 2,
                            health_blue_attr.width - 4, health_blue_attr.height - 4, slant,
                            0.05f, 0.09f, 0.18f, 0.02f, 0.05f, 0.12f, 0.50f,
                            health_blue_attr.y, health_blue_attr.height);
        /* value fill: deeper calmer blue (only while not depleted) + r16
         * gloss/shadow bands top & bottom 30% for cylinder-like depth */
        if (!depleted && fw > 1.0f)
        {
            Draw_TrapezoidGrad (health_blue_attr.x + inset, health_blue_attr.y + inset,
                                fw, health_blue_attr.height - 2*inset, slant,
                                0.32f, 0.55f, 0.80f,
                                0.07f, 0.24f, 0.45f,
                                0.38f,
                                health_blue_attr.y, health_blue_attr.height);
            /* r18: gloss/shadow as flat inset strips, NOT mini-trapezoids.
             * The short bands reused the full slant, so their diagonals
             * were far shallower than the fill's and stuck out past the
             * edge (user 2026-11-12). Strips start/end past the slant so
             * they can never protrude. */
            {
                int bx = (int)(health_blue_attr.x + inset + slant + 2.0f);
                int bw = (int)(fw - 2.0f*slant - 4.0f);
                if (bw > 2)
                {
                    Draw_FillRGBA (bx, (int)(health_blue_attr.y + inset + 1.0f), bw, 2,
                                   0.45f, 0.75f, 1.00f, 0.35f);
                    Draw_FillRGBA (bx, (int)(health_blue_attr.y + health_blue_attr.height - inset - 3.0f), bw, 2,
                                   0.02f, 0.04f, 0.10f, 0.35f);
                }
            }
            /* r17: segment ticks removed — Halo 3 bars don't have them
             * (user 2026-11-12: "I dont need ticks"). Draw_CapsuleTicks
             * stays in the file, unused (Wno-unused-function covers it). */
            /* hit flash: pale ghost of the fill fading over 0.18s */
            if (dmgflash > 0.0f)
                Draw_TrapezoidGrad (health_blue_attr.x + inset, health_blue_attr.y + inset,
                                    fw, health_blue_attr.height - 2*inset, slant,
                                    0.75f, 0.85f, 0.95f, 0.55f, 0.70f, 0.88f,
                                    0.30f * (dmgflash / 0.18f),
                                    health_blue_attr.y, health_blue_attr.height);
        }
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
    SX   = glwidth*0.92 - boxw - 18*HUD_SCALE();   /* room for the right caret */

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
    {
        float total = (timelimit.value > 0) ? timelimit.value*60 - cl.time : cl.time;
        minutes = total / 60;
        seconds = total - 60*minutes;
        tens = seconds / 10;
        units = seconds - 10*tens;
        sprintf (str,"%3i:%i%i", minutes, tens, units);
        timex = SX + (int)(boxw - Draw_TTFWidth(str, 12.0f*HUD_SCALE()));   /* right-aligned, above the rows */
        Draw_TTFString(timex, glheight*(0.775), str, 171/255.0, 231/255.0, 255/255.0, 1, 16.0f*HUD_SCALE());
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
    Draw_TTFString(SX, glheight*(0.775), gametype, 127/255.0, 191/255.0, 255/255.0, 1, 16.0f*HUD_SCALE());

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
            Draw_Fill (SX-1, glheight*(0.8+y)-1, boxw+2, 2.0*CHARZ*HUD_SCALE()+2, bottom, 0.3);
            Draw_Fill (SX, glheight*(0.8+y), boxw, 2.0*CHARZ*HUD_SCALE(), bottom, 1);
            // draw foreground
            Draw_Fill (SX, glheight*(0.8+y), boxw*CLAMP (new_max (teamscores[i].scoreTotal/(fraglimit.value > 0 ? fraglimit.value : new_max(teamscores[0].scoreTotal, 1.0f)), 0), 0.0f, 1.0f), 2.0*CHARZ*HUD_SCALE(), bottom, 0.65);
            // draw score
            f = teamscores[i].scoreTotal;
            HUD_itoa(f, num);
            Draw_TTFString(SX + 4*HUD_SCALE(), glheight*(0.8+y) + 13.0f*HUD_SCALE(), num, 1, 1, 1, 1, 15.0f*HUD_SCALE());
            y += (22.0f*HUD_SCALE())/glheight;   /* box height + 2px gap in design px */
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
            Draw_Fill (SX-1, glheight*(0.8+y)-1, boxw+2, 2.0*CHARZ*HUD_SCALE()+2, bottom, 0.3);
            Draw_Fill (SX, glheight*(0.8+y), boxw, 2.0*CHARZ*HUD_SCALE(), bottom, 1);
            // draw foreground
            Draw_Fill (SX, glheight*(0.8+y), boxw*CLAMP (new_max (f/(fraglimit.value > 0 ? fraglimit.value : new_max(cl.scores[fragsort[0]].frags, 1.0f)), 0), 0.0f, 1.0f), 2.0*CHARZ*HUD_SCALE(), bottom, 0.65);
            // draw score
            Draw_TTFString(SX + 4*HUD_SCALE(), glheight*(0.8+y) + 13.0f*HUD_SCALE(), num, 1, 1, 1, 1, 15.0f*HUD_SCALE());
            y += (22.0f*HUD_SCALE())/glheight;   /* box height + 2px gap in design px */
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

/* P2a: shield bubble stays visible while the TAB leaderboard covers the
 * rest of the sbar (Sbar_DeathmatchOverlay calls this explicitly). */
void HUD_DrawShield (void) {
    int oldcanvas = currentcanvas;
    GL_SetCanvas(CANVAS_DEFAULT);
    drawHealth();
    GL_SetCanvas(oldcanvas);
}

void HUD_Draw (void) {
    int oldcanvas = currentcanvas;
    if (key_dest != key_game)   /* P2a: HUD hides for pause/menus/console */
        return;
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