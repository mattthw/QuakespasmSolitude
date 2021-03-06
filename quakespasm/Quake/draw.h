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

#ifndef _QUAKE_DRAW_H
#define _QUAKE_DRAW_H

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

extern	qpic_t		*draw_disc;	// also used on sbar
extern canvastype currentcanvas;

struct Coord {
    int x;
    int xp; // with padding for text
    int y;
    int yp; // with padding for text
};
struct MenuCoords {
    struct Coord grid[10][20];
    int cols;
    int rows;
    int colw;
    int rowh;
};

struct PicAttr {
    float width;
    float height;
    int x;
    int y;
    float xpercent;
    float ypercent;
};

void Draw_Init (void);
void Draw_Character (int x, int y, int num);
void Draw_CharacterScale (int x, int y, int num, float s);
void Draw_ColorCharacterScale (int x, int y, int num, float r, float g, float b, float a, float s);
int PixWidth(float percent);
int PixHeight(float percent);
void Draw_WindowInsCol(int x, int y, float width, float height, int color, float alpha);
void Draw_WindowIns(int x, int y, float width, float height, float alpha);
void Draw_OffCenterWindow(int x, int y, float width, float height, char *str, float alpha);
void Draw_OffCenterWindowPix(int x, int y, int width, int height, char *str, float alpha);
struct MenuCoords Draw_WindowGrid(char* title, int rows, float rowheight, int cols, float colwidth, float alpha, int cursor, float incl_footer);
struct MenuCoords Draw_WindowGridOffset(char* title, int rows, float rowheight, int cols, float colwidth, float alpha, int cursor, float incl_footer, float off);
struct MenuCoords Draw_WindowGridLeft(char* title, int rows, float rowheight, int cols, float colwidth, float alpha, int cursor, float incl_footer, float incl_header);
void Draw_WindowPix(int x, int y, int bgwidth, int bgheight, char *str, float alpha);
void Draw_CenterWindow(float width, float height, char *str, float alpha);
void Draw_DebugChar (char num);
void Draw_Button(int x, int y, qpic_t *pic);
void Draw_ButtonScaled(int x, int y, qpic_t *pic, float scale);
void Draw_MenuBg();
void Draw_StretchPic (int x, int y, qpic_t *pic, int x_value, int y_value);
struct PicAttr getHudPicAttr(float xpercent, float ypercent, qpic_t *pic);
void Draw_HudPic (struct PicAttr attr, qpic_t *pic);
void Draw_HudSubPic (struct PicAttr attr, qpic_t *pic, float s1, float t1, float s2, float t2);
void Draw_Pic (int x, int y, qpic_t *pic);
void Draw_SubPic (float x, float y, float w, float h, qpic_t *pic, float s1, float t1, float s2, float t2);
void Draw_TransPicTranslate (int x, int y, qpic_t *pic, int top, int bottom); //johnfitz -- more parameters
void Draw_ConsoleBackground (void); //johnfitz -- removed parameter int lines
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c, float alpha); //johnfitz -- added alpha
void Draw_Cursor (int x, int y, int w, int h, bool focused); //johnfitz -- added alpha
void Draw_FadeScreen (void);
void Draw_String (int x, int y, const char *str);
void Draw_ColoredString (int x, int y, const char *str, float r, float g, float b, float a);
float Draw_ColoredStringScale (int x, int y, const char *str, float r, float g, float b, float a, float s);
qpic_t *Draw_PicFromWad2 (const char *name, unsigned int texflags);
qpic_t *Draw_PicFromWad (const char *name);
qpic_t *Draw_CachePic (const char *path);
qpic_t *Draw_TryCachePic (const char *path, unsigned int texflags);
void Draw_NewGame (void);
qboolean Draw_ReloadTextures(qboolean force);

//Spike -- this is for csqc
typedef struct
{
	vec_t xy[2];
	vec_t st[2];
	vec4_t rgba;
} polygonvert_t;
void Draw_PicPolygon(qpic_t *pic, unsigned int numverts, polygonvert_t *verts);

void GL_SetCanvas (canvastype newcanvas); //johnfitz

#endif	/* _QUAKE_DRAW_H */

