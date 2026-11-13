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

/* r71: right-pointing filled triangle (mini-overlay caret). Fixed-function,
 * Vita-safe; premul canvas so bake the opacity into rgb at the callsite. */
static void HUD_FillTriRight (float x, float y, float w, float h, float cr, float cg, float cb, float a)
{
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_ALPHA_TEST);
    glEnable (GL_BLEND);
    glBegin (GL_TRIANGLES);
    glColor4f (cr, cg, cb, a);
    glVertex2f (x, y);
    glVertex2f (x, y+h);
    glVertex2f (x+w, y+h*0.5f);
    glEnd ();
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);
    glColor3f (1,1,1);
}

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
cvar_t hud_nameplates = {"hud_nameplates", "1"};	//r63 issue #39: nameplates-over-reticle
cvar_t cl_debugweapon = {"cl_debugweapon", "0"};	//r65b: print STAT_ACTIVEWEAPON on change (crosshair-shape debugging)
cvar_t hud_mapshot = {"hud_mapshot", "0"};	//r83: one-switch hide of all HUD for map thumbnail shots

/* r66 Tracker-0: Halo-style ping motion radar (bottom-left). No movement
 * gating, no equipment - blips refresh on a ping every radar_ping seconds and
 * stay visible (frozen position) for radar_hold seconds. */
cvar_t hud_radar   = {"hud_radar", "1"};     // master toggle
cvar_t radar_range = {"radar_range", "2000"};  // r66b: ~50 m, 1000 was too short
cvar_t radar_ping  = {"radar_ping", "1.5"};   // r66h: was 2.5 (too delayed); live (0) was too accurate for foes
cvar_t radar_hold  = {"radar_hold", "1.0"};   // r66h: was 1.2 - blips now blink off briefly between pings
cvar_t hud_crosshair = {"hud_crosshair", "1"};	//r63 issue #39: dynamic Halo tick crosshair
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
    Cvar_RegisterVariable (&hud_nameplates);	//r63
    Cvar_RegisterVariable (&hud_crosshair);	//r63
    Cvar_RegisterVariable (&cl_debugweapon);	//r65b
    Cvar_RegisterVariable (&hud_mapshot);	//r83
    Cvar_RegisterVariable (&hud_radar);	//r66
    Cvar_RegisterVariable (&radar_range);	//r66
    Cvar_RegisterVariable (&radar_ping);	//r66
    Cvar_RegisterVariable (&radar_hold);	//r66
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

/* ==================== r63: dynamic crosshair + nameplates (issue #39) ====================
 * CL_TraceLine (r_part_fte.c) only traces against BRUSH models - players are
 * alias models - so the reticle "pick" is an angular hit-test against the
 * player entities, and CL_TraceLine does occlusion + world/sky classification.
 * Other players' health is not networked to the client, so plates are
 * name + team tint only (health bar needs a server-side stat - future work).
 * Team test mirrors MiniDeathmatchOverlay's colors-nibble equality heuristic.
 */
extern float CL_TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal, int *entnum);

#define HUD_MAXPLATES 8
static struct {
    float sx, sy, factor;   /* screen px + px-per-world-unit at its depth */
    float dist;             /* depth along view forward */
    qboolean teammate;
    qboolean occluded;
    qboolean nearret;       /* r64: crosshair within the target's pixel extent */
    const char *name;
} hud_plate[HUD_MAXPLATES];
static int hud_plate_n;
static int hud_plate_reticle = -1;   /* plate index under the reticle, -1 none */
static qboolean hud_worldhit;        /* crosshair ray hits a brush model */

static void HUD_ScanTargets (void)
{
    vec3_t fwd, right, up, rel, to, impact, normal;
    float fwdamt, latx, laty, tanx, tany, fx, fy, cx, cy, pfrac;
    int me_colors;
    int i, hument;
    entity_t *ent;
    scoreboard_t *sc;

    hud_plate_n = 0;
    hud_plate_reticle = -1;
    hud_worldhit = false;
    if (!deathmatch.value || cl.intermission)
        return;

    AngleVectors (r_refdef.viewangles, fwd, right, up);

    /* where the crosshair ray lands in the world (brushes/inline models only) */
    VectorMA (r_refdef.vieworg, 8192.0f, fwd, to);
    pfrac = CL_TraceLine (r_refdef.vieworg, to, impact, normal, &hument);
    hud_worldhit = (pfrac < 1.0f);

    cx = glwidth * 0.5f;
    cy = glheight * 0.5f;
    tanx = tanf (r_refdef.fov_x * 0.008726646f);   /* 0.5 * pi/180 */
    tany = tanf (r_refdef.fov_y * 0.008726646f);
    /* r65b: teammate test mirrors MiniDeathmatchOverlay - identical FULL
     * colors byte (both nibbles) while teamplay is on. In FFA everyone is
     * a foe; the old bottom-nibble equality turned random bots green just
     * because they shared a palette band. (self.team on the server IS
     * bottom-nibble+1, so in team games byte equality == same team.) */
    me_colors = cl.scores[cl.viewentity - 1].colors;

    for (i = 1; i <= cl.maxclients && i < cl.num_entities && hud_plate_n < HUD_MAXPLATES; i++)
    {
        ent = &cl.entities[i];
        sc = &cl.scores[i - 1];
        if (!ent->model || !*sc->name)
            continue;
        /* r64: dead players swap to the QC corpse model (progs/plyr.mdl =
         * modelindex_dead, Gametypes.qc) - drop the plate the instant they
         * die instead of riding out the respawn timer (user). */
        if (!q_strcasecmp (ent->model->name, "progs/plyr.mdl"))
            continue;

        rel[0] = ent->origin[0] - r_refdef.vieworg[0];
        rel[1] = ent->origin[1] - r_refdef.vieworg[1];
        rel[2] = ent->origin[2] - r_refdef.vieworg[2] + 20.0f;  /* chest/head-ish */
        fwdamt = DotProduct (rel, fwd);
        if (fwdamt < 32.0f || fwdamt > 3200.0f)
            continue;
        latx = DotProduct (rel, right);
        laty = DotProduct (rel, up);

        /* occlusion: trace to the player's chest; a brush model in between hides them */
        to[0] = ent->origin[0]; to[1] = ent->origin[1];
        to[2] = ent->origin[2] + 20.0f;   /* chest target == the rel we just measured */
        if (CL_TraceLine (r_refdef.vieworg, to, impact, normal, &hument) < 0.98f)
            continue;   /* behind geometry */

        fx = cx + latx * ((glwidth * 0.5f) / (fwdamt * tanx));
        fy = cy - laty * ((glheight * 0.5f) / (fwdamt * tany));

        /* r64: per-plate proximity test - crosshair within the player's pixel
         * extent + tick ring (same radius the reticle pick uses). Nameplates
         * only render for players inside this gate. */
        {
            float pdx = fx - cx;
            float pdy = fy - cy;
            float pr  = 24.0f * ((glwidth * 0.5f) / (fwdamt * tanx)) + (float)(14 * HUD_SCALE());
            hud_plate[hud_plate_n].nearret = (pdx * pdx + pdy * pdy < pr * pr);
        }

        hud_plate[hud_plate_n].sx = fx;
        hud_plate[hud_plate_n].sy = fy;
        hud_plate[hud_plate_n].factor = (glwidth * 0.5f) / (fwdamt * tanx);
        hud_plate[hud_plate_n].dist = fwdamt;
        hud_plate[hud_plate_n].teammate = (teamplay.value != 0 && (int)sc->colors == me_colors);
        hud_plate[hud_plate_n].occluded = false;
        hud_plate[hud_plate_n].name = sc->name;
        hud_plate_n++;
    }

    /* reticle pick: smallest |screen offset| within the player's pixel extent + tick ring */
    {
        float best = 1e9f;
        int k;
        for (k = 0; k < hud_plate_n; k++)
        {
            float dx = hud_plate[k].sx - cx;
            float dy = hud_plate[k].sy - cy;   /* r64 fix: was fy (last target's y), not center */
            float r  = 24.0f * hud_plate[k].factor + (float)(14 * HUD_SCALE());
            float d2 = dx * dx + dy * dy;
            if (d2 < r * r && hud_plate[k].dist < best)
            {
                best = hud_plate[k].dist;
                hud_plate_reticle = k;
            }
        }
    }
}

/* r65: stroked circle for weapon-specific crosshair shapes. Same GL state
 * sequence as Draw_TrapezoidGrad; premul 2D canvas ignores alpha, so colors
 * arrive pre-baked (cr/cg/cb carry the brightness). */
static void HUD_DrawCircle (float x, float y, float r, float cr, float cg, float cb)
{
    int i, n = 28;
    const float tau = 6.28318530718f;

    glDisable (GL_TEXTURE_2D);
    glEnable (GL_BLEND);
    glDisable (GL_ALPHA_TEST);
    glColor3f (cr, cg, cb);
    glBegin (GL_LINE_LOOP);
    for (i = 0; i < n; i++)
    {
        float a = (float)i * (tau / (float)n);
        glVertex2f (x + r * cosf (a), y + r * sinf (a));
    }
    glEnd ();
    glColor3f (1,1,1);
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);
}

void drawCrosshair(void) {
    float s = HUD_SCALE();
    int gap = (int)(8.0f * s) + 2;
    int len = (int)(5.0f * s) + 2;
    int th  = q_max (2, (int)(2.0f * s));
    int cx  = glwidth / 2;
    int cy  = glheight / 2;
    float cr = 0.35f, cg = 0.65f, cb = 1.00f;   /* r66e: default blue (was white) */

    /* r65d: weapon debug - the Con_Printf-only version was invisible in play
     * (console not open), so draw the raw held-weapon stat on screen now. */
    if (cl_debugweapon.value)
    {
        static char dbg_str[64] = "AW stat: --";
        static int dbg_last = -999;
        int dbg_aw = (int)cl.stats[STAT_ACTIVEWEAPON];
        if (dbg_aw != dbg_last)
        {
            dbg_last = dbg_aw;
            sprintf (dbg_str, "AW=%i (%s)", dbg_aw, getWeaponName (dbg_aw));
            Con_Printf ("drawCrosshair: STAT_ACTIVEWEAPON = %i (%s)\n",
                        dbg_aw, getWeaponName (dbg_aw));
        }
        Draw_TTFString (cx - 60, cy + 46, dbg_str, 1, 1, 0, 1, 12.0f * s);
    }

    if (!hud_crosshair.value)
        return;
    HUD_ScanTargets ();   /* fills hud_plate[] for both crosshair + nameplates */

    /* r66e: reticle is ALWAYS blue unless looking at an enemy - teammate
     * green was wrong (user); ally/world/sky all read blue now. */
    if (hud_plate_reticle >= 0 && !hud_plate[hud_plate_reticle].teammate)
        { cr = 0.90f; cg = 0.25f; cb = 0.25f; }       /* foe red */
    if (cl_ch_red.value)                              /* server push (#399 style) */
        { cr = 0.90f; cg = 0.25f; cb = 0.25f; }

    /* r65b: weapon-specific shapes (user), keyed off the HELD weapon item
     * flags in STAT_ACTIVEWEAPON (same source as drawWeapon's getWeaponName
     * - cl_ww is only the world/pickup weapon and was the earlier bug).
     *   SMG / Sniper            - small ring, tighter than tick spread
     *   Rocket Launcher         - ring at tick spread + small '+' center */
    switch ((int)cl.stats[STAT_ACTIVEWEAPON])
    {
    case IT_SHOTGUN:         /* Pistol - r65e: tiny ring (user: ticks read as
                              * 'no crosshair' and wrong vs sniper/SMG) */
        HUD_DrawCircle (cx, cy, q_max (2.5f, (float)(gap + len) * 0.18f), cr, cg, cb);
        return;
    case IT_ROCKET_LAUNCHER:
        {
            int plen = q_max (3, (int)(3.0f * s));
            HUD_DrawCircle (cx, cy, (float)(gap + len), cr, cg, cb);
            Draw_FillRGBA (cx - plen, cy - 1, plen * 2, 2, cr, cg, cb, 1.0f);
            Draw_FillRGBA (cx - 1, cy - plen, 2, plen * 2, cr, cg, cb, 1.0f);
            return;
        }
    case IT_SUPER_NAILGUN:    /* SMG */
        HUD_DrawCircle (cx, cy, q_max (4.0f, (float)(gap + len) * 0.45f), cr, cg, cb);
        return;
    case IT_GRENADE_LAUNCHER: /* Sniper - r65f: smaller than pistol (user) */
        HUD_DrawCircle (cx, cy, q_max (2.0f, (float)(gap + len) * 0.12f), cr, cg, cb);
        return;
    default:
        break;
    }

    /* premul 2D canvas ignores alpha - bake brightness into rgb */
    Draw_FillRGBA (cx - gap - len, cy - th / 2, len, th, cr, cg, cb, 1.0f);   /* left  */
    Draw_FillRGBA (cx + gap,       cy - th / 2, len, th, cr, cg, cb, 1.0f);   /* right */
    Draw_FillRGBA (cx - th / 2, cy - gap - len, th, len, cr, cg, cb, 1.0f);   /* up    */
    Draw_FillRGBA (cx - th / 2, cy + gap,       th, len, cr, cg, cb, 1.0f);   /* down  */
}

static void drawNameplates (void)
{
    float s = HUD_SCALE();
    float size = 13.0f * s;   /* r64: 11 -> 13, user wanted them a bit bigger */
    int i;

    if (!hud_nameplates.value)
        return;
    for (i = 0; i < hud_plate_n; i++)
    {
        float al, w, x, y;
        float nr, ng, nb;
        if (!hud_plate[i].nearret)
            continue;   /* r64: names only when the crosshair is near them (user) */
        al = 1.0f - hud_plate[i].dist / 3400.0f;
        if (al < 0.15f)
            continue;
        /* r64: enemy names red, teammate names light blue (was green - user);
         * no underline. */
        if (hud_plate[i].teammate)
            { nr = 0.45f; ng = 0.75f; nb = 1.00f; }
        else
            { nr = 0.90f; ng = 0.25f; nb = 0.25f; }
        w = Draw_TTFWidth (hud_plate[i].name, size);
        x = hud_plate[i].sx - w * 0.5f;
        y = hud_plate[i].sy - 15.0f * s;
        Draw_TTFString ((int)x, (int)y, hud_plate[i].name, nr * al, ng * al, nb * al, al, size);
    }
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
    Draw_TTFString(glwidth*0.804, glheight*0.10, primary, 127/255.0, 191/255.0, 255/255.0,1, 18.7f*HUD_SCALE());
    if (strcmp(primary, "Energy Sword") != 0 /* 0 means equal*/) { // dont draw ammo for energy sword
        Draw_TTFString(glwidth*0.804, glheight*0.19 - 8*HUD_SCALE(), rounds, 171/255.0, 231/255.0, 255/255.0,1, 16.15f*HUD_SCALE());
        Draw_TTFString(glwidth*0.853, glheight*0.19 - 8*HUD_SCALE(), ammo, 171/255.0, 231/255.0, 255/255.0,1, 12.75f*HUD_SCALE());
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
        Draw_TTFString(glwidth*0.03, glheight*0.12 - 8*HUD_SCALE(), nadecount, 0.45, 0.45, 0.45, 0.75, 16.15f*HUD_SCALE());
        Draw_TTFString(glwidth*0.08, glheight*0.12 - 8*HUD_SCALE(), nadename, 0.45, 0.45, 0.45, 0.75, 16.15f*HUD_SCALE());
    }
    else
    {
        Draw_TTFString(glwidth*0.03, glheight*0.12 - 8*HUD_SCALE(), nadecount, 171/255.0, 231/255.0, 255/255.0,1, 16.15f*HUD_SCALE());
        Draw_TTFString(glwidth*0.08, glheight*0.12 - 8*HUD_SCALE(), nadename, 171/255.0, 231/255.0, 255/255.0,1, 16.15f*HUD_SCALE());
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
    struct PicAttr health_blue_attr = getHudPicAttr(0.5, 0.07, health_blue);

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
        if (prevsheilds >= 0 && sheilds < prevsheilds) dmgflash = 0.30f;  /* r59c: longer */
        prevsheilds = sheilds;
        dmgflash -= host_frametime;
        if (dmgflash < 0.0f) dmgflash = 0.0f;
        { float k = (float)host_frametime * 8.0f; if (k > 1.0f) k = 1.0f;
          shownfrac += (frac - shownfrac) * k; }
        fw = (health_blue_attr.width - 2*inset) * shownfrac;

        /* r58e: all bar rgb premultiplied x0.7 (premul canvas ignores alpha)
         * for "30% more transparent" (user 2026-11-12); anchor 0.10 -> 0.07. */
        /* P2a r15: mirror-trapezoid layers sharing ONE slant d so every
         * diagonal is parallel (kills the "glitch lines"), calm translucent
         * blues ("neon" was too hot). Depleted: hollow bar, outline itself
         * flashes red at 3 Hz — r14b behavior kept. All ramp via ry/rh over
         * the full bar height — r14 kept. */
        if (depleted)
        {
            /* r59c: depleted keeps the BLUE identity (user 2026-11-12:
             * "it should always be blue, it should just get brighter and
             * flash more when damaged" -- the red outline read as a
             * different HUD). Danger = amplitude: same hue as the healthy
             * outline, gain 1.68..2.2 on a 3Hz flash, alpha 0.45..0.75. */
            float pk = (tflash > 0.5f) ? 1.0f : 0.35f;
            float k = 1.4f + 0.8f * pk;
            Draw_TrapezoidGrad (health_blue_attr.x, health_blue_attr.y,
                                health_blue_attr.width, health_blue_attr.height, slant,
                                0.245f*k, 0.385f*k, 0.546f*k,
                                0.084f*k, 0.196f*k, 0.336f*k,
                                0.45f + 0.30f*pk,
                                health_blue_attr.y, health_blue_attr.height);
        }
        else
        {
            Draw_TrapezoidGrad (health_blue_attr.x, health_blue_attr.y,
                                health_blue_attr.width, health_blue_attr.height, slant,
                                0.245f, 0.385f, 0.546f, 0.084f, 0.196f, 0.336f, 0.40f,
                                health_blue_attr.y, health_blue_attr.height);
        }
        Draw_TrapezoidGrad (health_blue_attr.x + 2, health_blue_attr.y + 2,
                            health_blue_attr.width - 4, health_blue_attr.height - 4, slant,
                            0.035f, 0.063f, 0.126f, 0.014f, 0.035f, 0.084f, 0.50f,
                            health_blue_attr.y, health_blue_attr.height);
        /* value fill: deeper calmer blue (only while not depleted) + r16
         * gloss/shadow bands top & bottom 30% for cylinder-like depth */
        if (!depleted && fw > 1.0f)
        {
            Draw_TrapezoidGrad (health_blue_attr.x + inset, health_blue_attr.y + inset,
                                fw, health_blue_attr.height - 2*inset, slant,
                                0.224f, 0.385f, 0.56f,
                                0.049f, 0.168f, 0.315f,
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
                                   0.315f, 0.525f, 0.70f, 0.35f);
                    Draw_FillRGBA (bx, (int)(health_blue_attr.y + health_blue_attr.height - inset - 3.0f), bw, 2,
                                   0.014f, 0.028f, 0.07f, 0.35f);
                }
            }
            /* r17: segment ticks removed — Halo 3 bars don't have them
             * (user 2026-11-12: "I dont need ticks"). Draw_CapsuleTicks
             * stays in the file, unused (Wno-unused-function covers it). */
            /* hit flash: pale ghost of the fill fading over 0.18s */
            if (dmgflash > 0.0f)
                Draw_TrapezoidGrad (health_blue_attr.x + inset, health_blue_attr.y + inset,
                                    fw, health_blue_attr.height - 2*inset, slant,
                                    0.525f, 0.595f, 0.665f, 0.385f, 0.49f, 0.616f,
                                    0.60f * (dmgflash / 0.30f),   /* r59c: stronger hit flash (user) */
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
extern unsigned int d_8to24table[256];
void Draw_FillRGBA (int x, int y, int w, int h, float r, float g, float b, float alpha);


void MiniDeathmatchOverlay (void)
{
    int				i, k, l;
    int				bottom;
    float			y;
    float			boxw, SX;
    int f;
    char			num[12];
    scoreboard_t	*s;

    if (hud_mapshot.value)      /* r83: hidden for map-photo capture */
        return;
    if (sb_showscores)          /* r73: score cluster hides under the leaderboard */
        return;

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
            /* r59d: translucent like the leaderboard rows -- the premul
             * 2D canvas ignores Draw_Fill's alpha (and alpha 1 was an
             * opaque slab), so opacity is baked into rgb here. */
            {
                byte *pc = (byte *)&d_8to24table[bottom];
                float tr = pc[0]/255.0f, tg = pc[1]/255.0f, tb = pc[2]/255.0f;
                float fr = tr*0.85f, fg = tg*0.85f, fb = tb*0.85f;   /* r59e: bright fill (user: mini-overlay "bad colors and no alpha") */
                Draw_FillRGBA (SX-1, glheight*(0.8+y)-1, boxw+2, 2.0*CHARZ*HUD_SCALE()+2, tr*0.18f, tg*0.18f, tb*0.18f, 0.18f);   /* r61: match LB border */
                Draw_FillRGBA (SX, glheight*(0.8+y), boxw, 2.0*CHARZ*HUD_SCALE(), tr*0.55f, tg*0.55f, tb*0.55f, 0.55f);   /* r61: match LB backing */
                // draw foreground
                Draw_FillRGBA (SX, glheight*(0.8+y), boxw*CLAMP (new_max (teamscores[i].scoreTotal/(teamlimit.value > 0 ? teamlimit.value : (fraglimit.value > 0 ? fraglimit.value : new_max(teamscores[0].scoreTotal, 1.0f))), 0), 0.0f, 1.0f), 2.0*CHARZ*HUD_SCALE(), fr, fg, fb, 0.65f);
            }
            // draw score
            /* r71: blue caret marks my team's bar (#43 addendum; user ask). */
            if (cl.viewentity >= 1 && cl.viewentity <= cl.maxclients &&
                (int)(teamscores[i].color & 15) == (int)(cl.scores[cl.viewentity-1].colors & 15))
                HUD_FillTriRight (SX - 11*HUD_SCALE(), glheight*(0.8+y) + 3.0f*HUD_SCALE(),
                                  7.0f*HUD_SCALE(), 10.0f*HUD_SCALE(),
                                  0.45f*0.95f, 0.75f*0.95f, 1.00f*0.95f, 0.95f);
            f = teamscores[i].scoreTotal;
            HUD_itoa(f, num);
            Draw_TTFString(SX + 4*HUD_SCALE(), glheight*(0.8+y) + 13.0f*HUD_SCALE(), num, 1, 1, 1, 1, 15.0f*HUD_SCALE());
            y += (22.0f*HUD_SCALE())/glheight;   /* box height + 2px gap in design px */
        }
        /* r67: client-side teamlimit hack removed -- team scores now live
         * server-side in QuakeC (TeamScore_Add/TopTeamScore) and NextLevel
         * fires from CheckRules against the real teamlimit. */
    } else { // slayer (ffa) or swat
        int myrow = -1, nrows = 0, rowidx[2], ri;
        SortFrags ();
        /* r75: the two visible rows are leader + me (user: last place must
         * still show own score). Rows past the second always clipped off
         * the bottom -- 0.8 anchor + 22px pitch only fits two. */
        for (i = 0; i < l; i++)
            if (fragsort[i] == cl.viewentity - 1)
            {
                myrow = i;
                break;
            }
        if (l > 0)
            rowidx[nrows++] = fragsort[0];
        if (l > 1)
            rowidx[nrows++] = (myrow >= 1) ? fragsort[myrow] : fragsort[1];
        for (ri = 0; ri < nrows; ri++)
        {
            k = rowidx[ri];
            s = &cl.scores[k];
            if (!s->name[0])
                continue;


            bottom = (s->colors & 15) <<4;
            bottom = ColorForMap (bottom);
            f = s->frags;
            HUD_itoa(f, num);
            /* r59d: translucent, as above. */
            {
                byte *pc = (byte *)&d_8to24table[bottom];
                float tr = pc[0]/255.0f, tg = pc[1]/255.0f, tb = pc[2]/255.0f;
                float fr = tr*0.85f, fg = tg*0.85f, fb = tb*0.85f;   /* r59e: bright fill (user: mini-overlay "bad colors and no alpha") */
                Draw_FillRGBA (SX-1, glheight*(0.8+y)-1, boxw+2, 2.0*CHARZ*HUD_SCALE()+2, tr*0.18f, tg*0.18f, tb*0.18f, 0.18f);   /* r61: match LB border */
                Draw_FillRGBA (SX, glheight*(0.8+y), boxw, 2.0*CHARZ*HUD_SCALE(), tr*0.55f, tg*0.55f, tb*0.55f, 0.55f);   /* r61: match LB backing */
                // draw foreground
                Draw_FillRGBA (SX, glheight*(0.8+y), boxw*CLAMP (new_max (f/(fraglimit.value > 0 ? fraglimit.value : new_max(cl.scores[fragsort[0]].frags, 1.0f)), 0), 0.0f, 1.0f), 2.0*CHARZ*HUD_SCALE(), fr, fg, fb, 0.65f);
            }
            /* r75: blue caret marks MY row in deathmatch too (user). */
            if (k == cl.viewentity - 1)
                HUD_FillTriRight (SX - 11*HUD_SCALE(), glheight*(0.8+y) + 3.0f*HUD_SCALE(),
                                  7.0f*HUD_SCALE(), 10.0f*HUD_SCALE(),
                                  0.45f*0.95f, 0.75f*0.95f, 1.00f*0.95f, 0.95f);
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

/* r66: translucent filled disc, same fixed-function/premul approach as the
 * shield-bar trapezoids - bake opacity into rgb, alpha passed for form. */
static void HUD_FillCircle (float x, float y, float r, float cr, float cg, float cb, float a)
{
    int i;
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_ALPHA_TEST);
    glEnable (GL_BLEND);
    glBegin (GL_TRIANGLE_FAN);
    glColor4f (cr, cg, cb, a);
    glVertex2f (x, y);
    for (i = 0; i <= 28; i++)
    {
        float ang = (float)i * (6.28318530718f / 28.0f);
        glVertex2f (x + cosf (ang) * r, y + sinf (ang) * r);
    }
    glEnd ();
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);
    glColor3f (1,1,1);
}

#define RADAR_MAXBLIPS 16
typedef struct { float px, py, born; qboolean teammate; } radar_blip_t;
static radar_blip_t radar_blips[RADAR_MAXBLIPS];
static int radar_blips_n;
static float radar_lastping = -1.0f;

static void drawRadar (void)
{
    float s = HUD_SCALE();
    float R, range, period, ph, age, al, d, scale, dx, dy;
    float rcx, rcy, ox, oy, ol, f;
    vec3_t fwd, right, up, rel;
    int me_colors, i, k, n;
    entity_t *ent;
    scoreboard_t *sc;

    if (!hud_radar.value || !deathmatch.value || cl.intermission)
        return;

    s = HUD_SCALE();
    R = 34.0f * s;   /* r66b: was 46 - disc too dominant (user) */
    rcx = 20.0f * s + R;
    rcy = (float)glheight - 20.0f * s - R;
    range = q_max (100.0f, radar_range.value);

    /* backing disc + rim (dark navy so it reads on bright sky, like the bar) */
    /* r66c: one light-blue family (0.45,0.75,1.00) everywhere, different
     * alpha strengths baked into rgb for the premul canvas (user). */
    HUD_FillCircle (rcx, rcy, R, 0.45f*0.22f, 0.75f*0.22f, 1.00f*0.22f, 0.55f);   /* r66e: +disc, closes the rim gap feel */

    /* FOV cone: screen up = view forward, +/-45 deg wedge */
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_ALPHA_TEST);
    glEnable (GL_BLEND);
    glBegin (GL_TRIANGLE_FAN);
    {
        /* r66c: fan pulsed like the menu cursor (same sinf recipe) since we
         * have no sweep yet - and widened to 150 deg, "still too small" (user) */
        float pulse = 0.06f + 0.14f * (0.5f + 0.5f * sinf ((float)realtime * 2.2f));   /* r66g: way darker - 0.06..0.20 (user: still too solid) */
        glColor4f (0.45f*pulse, 0.75f*pulse, 1.00f*pulse, pulse);
    }
    glVertex2f (rcx, rcy);
    for (k = 0; k <= 16; k++)
    {
        float ang = (-145.0f + (float)k * (110.0f / 16.0f)) * 0.0174532f;   /* r66e: mid -90, was tilted 10 deg left */
        glVertex2f (rcx + cosf (ang) * R * 0.95f, rcy + sinf (ang) * R * 0.95f);
    }
    glEnd ();

    /* r66d: view basis + team anchor computed every frame - ally blips
     * are live now, so they can't depend on the ping block.
     * r66h: yaw-only - AngleVectors folds PITCH into fwd/right, which
     * skewed every blip (and the ping snapshots) whenever the player
     * looked up or down; a top-down radar cares only about heading. */
    {
        float yv = (float)r_refdef.viewangles[1] * 0.0174532f;
        fwd[0] = cosf (yv);   fwd[1] = sinf (yv);   fwd[2] = 0.0f;
        right[0] = sinf (yv); right[1] = -cosf (yv); right[2] = 0.0f;
    }
    me_colors = cl.scores[cl.viewentity - 1].colors;
    scale = R / range;

    /* ping: freeze foes' relative position at screen px on each ping */
    period = q_max (0.5f, radar_ping.value);
    ph = floorf ((float)realtime / period);
    if (ph != radar_lastping)
    {
        radar_lastping = ph;
        n = 0;
        for (i = 1; i <= cl.maxclients && i < cl.num_entities && n < RADAR_MAXBLIPS; i++)
        {
            ent = &cl.entities[i];
            sc = &cl.scores[i - 1];
            if (!ent->model || !*sc->name)
                continue;
            if (i == cl.viewentity)
                continue;
            if (!q_strcasecmp (ent->model->name, "progs/plyr.mdl"))
                continue;   /* corpse */
            rel[0] = ent->origin[0] - r_refdef.vieworg[0];
            rel[1] = ent->origin[1] - r_refdef.vieworg[1];
            rel[2] = 0.0f;
            d = sqrtf (rel[0]*rel[0] + rel[1]*rel[1]);
            if (d > range)
                continue;
            dx = DotProduct (rel, right);
            dy = DotProduct (rel, fwd);
            ox = dx * scale;
            oy = -dy * scale;
            ol = sqrtf (ox*ox + oy*oy);
            if (ol > R * 0.9f)    /* pin far-but-in-range blips to the rim */
            {
                f = (R * 0.9f) / ol;
                ox *= f; oy *= f;
            }
            /* r66d: teammates are always visible via the live pass below -
             * skip them here so they don't double up as fading ping blips */
            if (teamplay.value != 0 && (int)sc->colors == me_colors)
                continue;
            radar_blips[n].px = rcx + ox;
            radar_blips[n].py = rcy + oy;
            radar_blips[n].born = (float)realtime;
            radar_blips[n].teammate = false;
            n++;
        }
        radar_blips_n = n;
    }

    /* blips live radar_hold sec after their ping, fading out */
    al = q_max (0.2f, radar_hold.value);
    for (i = 0; i < radar_blips_n; i++)
    {
        float life = 1.0f - ((float)realtime - radar_blips[i].born) / al;
        if (life <= 0.0f)
            continue;
        if (radar_blips[i].teammate)
            HUD_FillCircle (radar_blips[i].px, radar_blips[i].py,
                            q_max (1.6f, 2.2f * s),
                            0.85f * life, 0.75f * life, 0.20f * life, life);   /* yellow ally */
        else
            HUD_FillCircle (radar_blips[i].px, radar_blips[i].py,
                            q_max (1.6f, 2.2f * s),
                            0.90f * life, 0.25f * life, 0.25f * life, life);   /* red foe */
    }

    /* r66d: allies always visible - live blips every frame, no ping gating */
    if (teamplay.value != 0)
    {
        for (i = 1; i <= cl.maxclients && i < cl.num_entities; i++)
        {
            ent = &cl.entities[i];
            sc = &cl.scores[i - 1];
            if (!ent->model || !*sc->name || i == cl.viewentity)
                continue;
            if (!q_strcasecmp (ent->model->name, "progs/plyr.mdl"))
                continue;
            if ((int)sc->colors != me_colors)
                continue;
            rel[0] = ent->origin[0] - r_refdef.vieworg[0];
            rel[1] = ent->origin[1] - r_refdef.vieworg[1];
            rel[2] = 0.0f;
            d = sqrtf (rel[0]*rel[0] + rel[1]*rel[1]);
            if (d > range)
                continue;
            ox = DotProduct (rel, right) * scale;
            oy = -DotProduct (rel, fwd) * scale;
            ol = sqrtf (ox*ox + oy*oy);
            if (ol > R * 0.9f)
            {
                f = (R * 0.9f) / ol;
                ox *= f; oy *= f;
            }
            al = 1.0f - 0.35f * (d / range);
            HUD_FillCircle (rcx + ox, rcy + oy, q_max (1.6f, 2.2f * s),
                            0.85f * al, 0.75f * al, 0.20f * al, al);   /* yellow ally */
        }
    }

    /* cone left the pipeline states down; restore before the markers */
    glDisable (GL_BLEND);
    glEnable (GL_ALPHA_TEST);
    glEnable (GL_TEXTURE_2D);

    /* r66c: self marker = yellow dot inside blue dot (the axis), always on
     * regardless of blips - replaces the own-player triangle (user) */
    HUD_FillCircle (rcx, rcy, 6.0f * s, 0.45f*0.55f, 0.75f*0.55f, 1.00f*0.55f, 0.85f);
    HUD_FillCircle (rcx, rcy, 2.6f * s, 0.95f, 0.85f, 0.20f, 1.0f);
    glColor3f (1,1,1);

    /* rim last so it frames the content */
    HUD_DrawCircle (rcx, rcy, R, 0.45f*0.45f, 0.75f*0.45f, 1.00f*0.45f);   /* r66e: border was too harsh */
}

void HUD_Draw (void) {
    int oldcanvas = currentcanvas;
    if (key_dest != key_game)   /* P2a: HUD hides for pause/menus/console */
        return;
    if (sb_showscores)          /* r73: crosshair/nameplates/radar/weapon panel
                                 * hide under the leaderboard too (Halo) */
        return;
    if ((int)cl.stats[STAT_HEALTH] <= 0)   /* r95b: whole HUD hidden while dead (user) */
        return;
    if (hud_mapshot.value)        /* r83: master switch for map-photo capture */
        return;
    GL_SetCanvas(CANVAS_DEFAULT);
    drawHealth();
    drawWeapon();
    drawGrenades();
    drawMedal();
    drawCrosshair();
    drawNameplates();
    drawRadar();   /* r66 Tracker-0 */
    drawPickupWeapon();
//    if (!*cl.scores[1].name) {
//        Draw_ColoredStringScale(0, glheight-(8*0.8f*HUD_SCALE()), "Its too quiet, Chief! Add bots to get started.", 1,1,1,1,0.8f*HUD_SCALE());
//    }
//    if (deathmatch.value == DM_SLAYER || deathmatch.value == DM_SWAT)
//        MiniDeathmatchOverlay ();
    GL_SetCanvas(oldcanvas);
}