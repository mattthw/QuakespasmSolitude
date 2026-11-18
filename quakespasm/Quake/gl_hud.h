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
// gl_hud.h -- status bar code

#include "quakedef.h"

// buttons
extern qpic_t      *b_up;
extern qpic_t      *b_down;
extern qpic_t      *b_left;
extern qpic_t      *b_right;
extern qpic_t      *b_lthumb;
extern qpic_t      *b_rthumb;
extern qpic_t      *b_lshoulder;
extern qpic_t      *b_rshoulder;
extern qpic_t      *b_abutton;
extern qpic_t      *b_bbutton;
extern qpic_t      *b_ybutton;
extern qpic_t      *b_xbutton;
extern qpic_t      *b_lt;
extern qpic_t      *b_rt;

//health
extern qpic_t *health_blue;
extern qpic_t *health_red;

void HUD_Draw (void);
void HUD_DrawShield (void);
void MiniDeathmatchOverlay (void);

void HUD_Init (void);

// visor warp (issue #40, HUD half) -- emit-time barrel warp, gl_hud.c
extern cvar_t visor_warp;
extern cvar_t visor_zoom;
// r101 editor menu: runtime editor exception + photo switch, defined gl_hud.c
extern cvar_t editor_mode;
extern cvar_t hud_mapshot;
// graphics tab toggles: HUD overlay hide + minimal fps chip (defined gl_hud.c)
extern cvar_t hide_hud;
extern cvar_t hud_fps;
// r112 LB melee: Controls-tab switch, defined gl_hud.c, read by QC by name
extern cvar_t lb_melee;
void     HUD_SetWarpStrength (float k);
qboolean HUD_WarpActive (void);
void     HUD_WarpPoint (float x, float y, float *ox, float *oy);
void     HUD_WarpQuad (float x, float y, float w, float h, const float uv[4], int tess);
void     HUD_WarpPic (float x, float y, float w, float h, qpic_t *pic);

//void HUD_NewMap (void);
// perf B1 - per-frame timing buckets behind r_perfhud
typedef enum
{
    PERF_QC,
    PERF_WLD,
    PERF_ENT,
    PERF_PRT,
    PERF_TOTAL,
    PERF_N
} perf_bucket_t;

extern cvar_t         r_perfhud;
extern const char     *perf_names[PERF_N];
extern float          perf_fps;                // smoothed frames per second

void Perf_Begin (int bucket);
void Perf_End (int bucket);
// copies up to 'want' recent 50ms samples (oldest->newest), returns n
int  Perf_RingWindow (int bucket, double *out, int want);
int  Perf_PresentWindow (double *out, int want);
// perf B11: per-model entity draw accounting for `perfreport` (top-N table).
// t0 = Perf_ModelT0() before the draw (0 when r_perfhud is off = no cost).
double Perf_ModelT0 (void);
void   Perf_ModelDraw (const char *name, double t0);
// mode 2: per-50ms-window maxima of r_speeds counters (oldest->newest)
#define PERF_C_WPOLY  0
#define PERF_C_EPOLY  1
#define PERF_C_ELIGHT 2	// live dlights processed (mode-independent: post-cull count)
#define PERF_C_PARTS  3
#define PERF_C_PASS   4	// draw batches (rs_brushpasses + rs_aliaspasses)
#define PERF_C_EENTS  5	// server-visible entities (cl_numvisedicts)
#define PERF_C_N      6
int  Perf_PolyWindow (int which, int *out, int want);
void HUD_DrawPerf (void);
