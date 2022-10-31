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
// sbar.c -- status bar code

#include "quakedef.h"
#include <pspgu.h>

int			sb_updates;		// if >= vid.numpages, no update needed

cvar_t cl_killmedals = {"cl_killmedals", "0"};
cvar_t cl_plasmanade = {"cl_plasmanade", "2"};
cvar_t cl_nadenum = {"cl_nadenum", "5"};
cvar_t cl_ww = {"cl_ww", "0"};		//World weapon print message



#define STAT_MINUS		10	// num frame for '-' stats digit
qpic_t		*sb_nums[2][11];
qpic_t		*sb_colon, *sb_slash;
qpic_t		*sb_ibar;
qpic_t		*sb_sbar;
qpic_t		*sb_scorebar;

qpic_t      *sb_weapons[7][8];   // 0 is active, 1 is owned, 2-5 are flashes
qpic_t      *sb_ammo[4];
qpic_t		*sb_sigil[4];
qpic_t		*sb_armor[3];
qpic_t		*sb_items[32];

qpic_t	*sb_faces[7][2];		// 0 is gibbed, 1 is dead, 2-6 are alive
							// 0 is static, 1 is temporary animation
qpic_t	*sb_face_invis;
qpic_t	*sb_face_quad;
qpic_t	*sb_face_invuln;
qpic_t	*sb_face_invis_invuln;

qboolean	sb_showscores;

int			sb_lines;			// scan lines to draw

qpic_t		*ksb_ammo[1];

qpic_t      *rsb_invbar[2];
qpic_t      *rsb_weapons[5];
qpic_t      *rsb_items[2];
qpic_t      *rsb_ammo[3];
qpic_t      *rsb_teambord;		// PGM 01/19/97 - team color border
//Weapon Icons
qpic_t		*AR;
qpic_t		*SMG;
qpic_t		*Sniper;
qpic_t		*RL;
qpic_t		*Needler;
qpic_t		*ppist;
qpic_t		*Pistol;
qpic_t		*Shottie;
//World weapons
qpic_t		*ARW;
qpic_t		*SMGW;
qpic_t		*SniperW;
qpic_t		*RLW;
qpic_t		*NeedlerW;
qpic_t		*ppistW;
qpic_t		*PistolW;
qpic_t		*ShottieW;

//Kill medals
qpic_t		*doublek;
qpic_t		*triple;
qpic_t		*killtrocity;
qpic_t		*killtacular;

//Grenade types
qpic_t		*plasma;
qpic_t		*frag;


//Health
qpic_t		*life90;
qpic_t		*health_bar;
qpic_t		*health_bar_red;

//Ammo counter 
qpic_t		*pistolb;
qpic_t		*shotgunb;
qpic_t		*arb;
qpic_t		*needleb;
qpic_t		*rocketb;
//Grenade Numbers
qpic_t		*numbers[5];


//MED 01/04/97 added two more weapons + 3 alternates for grenade launcher
qpic_t      *hsb_weapons[7][5];   // 0 is active, 1 is owned, 2-5 are flashes
//MED 01/04/97 added array to simplify weapon parsing
int         hipweapons[4] = {HIT_LASER_CANNON_BIT,HIT_MJOLNIR_BIT,4,HIT_PROXIMITY_GUN_BIT};
//MED 01/04/97 added hipnotic items array
qpic_t      *hsb_items[2];

void Sbar_MiniDeathmatchOverlay (void);
void Sbar_TimeOverlay (void);
void Sbar_DeathmatchOverlay (void);
void Sbar_DeathmatchOverlay2 (void);
void M_DrawPic (int x, int y, qpic_t *pic);

/*
===============
Sbar_ShowScores

Tab key down
===============
*/
void Sbar_ShowScores (void)
{
	if (sb_showscores)
		return;
	sb_showscores = true;
	sb_updates = 0;
}

/*
===============
Sbar_DontShowScores

Tab key up
===============
*/
void Sbar_DontShowScores (void)
{
	sb_showscores = false;
	sb_updates = 0;
}

/*
===============
Sbar_Changed
===============
*/
void Sbar_Changed (void)
{
	sb_updates = 0;	// update next frame
}

/*
===============
Sbar_Init
===============
*/
void Sbar_Init (void)
{
	int		i;

	for (i=0 ; i<10 ; i++)
	{
		sb_nums[0][i] = Draw_PicFromWad (va("num_%i",i));
		sb_nums[1][i] = Draw_PicFromWad (va("anum_%i",i));
	}

	sb_nums[0][10] = Draw_PicFromWad ("num_minus");
	sb_nums[1][10] = Draw_PicFromWad ("anum_minus");

	sb_colon = Draw_PicFromWad ("num_colon");
	sb_slash = Draw_PicFromWad ("num_slash");

/*
=================================================
Weapon Icons for weapons.
=================================================
*/
	
	//Ammo counter
	pistolb = Draw_CachePic ("gfx/ammo/pistolb.lmp");	//Pistolb
	shotgunb = Draw_CachePic ("gfx/ammo/shotgunb.lmp");	//shotgunb
	arb = Draw_CachePic ("gfx/ammo/arb.lmp");		//arb
	needleb = Draw_CachePic ("gfx/ammo/needleb.lmp");		//needleb
	rocketb = Draw_CachePic ("gfx/ammo/rocketb.lmp");		//rocketb

	//World Weapons
	ARW = Draw_CachePic ("gfx/worldweapons/ar.lmp");	//Assault rifle
	SMGW = Draw_CachePic ("gfx/worldweapons/smg.lmp");	//SMG
	SniperW = Draw_CachePic ("gfx/worldweapons/sniper.lmp");	//Sniper Rifle
	RLW = Draw_CachePic ("gfx/worldweapons/RL.lmp");	//Rocket Launcher
	NeedlerW = Draw_CachePic ("gfx/worldweapons/Needler.lmp");	//Needler
	PistolW = Draw_CachePic ("gfx/worldweapons/Pistol.lmp");	//Pistol
	ShottieW = Draw_CachePic ("gfx/worldweapons/Shottie.lmp");	//Shottie
	ppistW = Draw_CachePic ("gfx/worldweapons/ppist.lmp");	//Plasma Pistol

	//Weapons
	AR = Draw_CachePic ("gfx/weapons/ar.lmp");	//Assault rifle
	SMG = Draw_CachePic ("gfx/weapons/smg.lmp");	//SMG
	Sniper = Draw_CachePic ("gfx/weapons/sniper.lmp");	//Sniper Rifle
	RL = Draw_CachePic ("gfx/weapons/RL.lmp");	//Rocket Launcher
	Needler = Draw_CachePic ("gfx/weapons/Needler.lmp");	//Needler
	Pistol = Draw_CachePic ("gfx/weapons/Pistol.lmp");	//Pistol
	Shottie = Draw_CachePic ("gfx/weapons/Shottie.lmp");	//Shottie
	ppist = Draw_CachePic ("gfx/weapons/ppist.lmp");	//Plasma Pistol


	//Health
	life90 = Draw_CachePic ("gfx/healthb.lmp");
	health_bar = Draw_CachePic ("gfx/healthback.lmp");
	health_bar_red = Draw_CachePic ("gfx/healthbackr.lmp");

	//Kill medals
	doublek = Draw_CachePic ("gfx/medals/double.lmp");
	triple = Draw_CachePic ("gfx/medals/triple.lmp");
	killtacular = Draw_CachePic ("gfx/medals/killtacular.lmp");
	killtrocity = Draw_CachePic ("gfx/medals/killtrocity.lmp");

	//Nade types
	plasma = Draw_CachePic ("gfx/plasmanade.lmp");
	frag = Draw_CachePic ("gfx/fraggrenade.lmp");

	//Nade number counter
	numbers[0] = Draw_CachePic ("gfx/numbers/num_0.lmp");
	numbers[1] = Draw_CachePic ("gfx/numbers/num_1.lmp");
	numbers[2] = Draw_CachePic ("gfx/numbers/num_2.lmp");
	numbers[3] = Draw_CachePic ("gfx/numbers/num_3.lmp");
	numbers[4] = Draw_CachePic ("gfx/numbers/num_4.lmp");

	//Client variables
	Cvar_RegisterVariable (&cl_killmedals);	//Kill medals
	Cvar_RegisterVariable (&cl_plasmanade); //Grenade types
	Cvar_RegisterVariable (&cl_nadenum);	//A counter for your grenades.
	Cvar_RegisterVariable (&cl_ww);	//World weapons

	sb_weapons[0][0] = Draw_PicFromWad ("inv_shotgun");
	sb_weapons[0][1] = Draw_PicFromWad ("inv_sshotgun");
	sb_weapons[0][2] = Draw_PicFromWad ("inv_nailgun");
	sb_weapons[0][3] = Draw_PicFromWad ("inv_snailgun");
	sb_weapons[0][4] = Draw_PicFromWad ("inv_rlaunch");
	sb_weapons[0][5] = Draw_PicFromWad ("inv_srlaunch");
	sb_weapons[0][6] = Draw_PicFromWad ("inv_lightng");

	sb_weapons[1][0] = Draw_PicFromWad ("inv2_shotgun");
	sb_weapons[1][1] = Draw_PicFromWad ("inv2_sshotgun");
	sb_weapons[1][2] = Draw_PicFromWad ("inv2_nailgun");
	sb_weapons[1][3] = Draw_PicFromWad ("inv2_snailgun");
	sb_weapons[1][4] = Draw_PicFromWad ("inv2_rlaunch");
	sb_weapons[1][5] = Draw_PicFromWad ("inv2_srlaunch");
	sb_weapons[1][6] = Draw_PicFromWad ("inv2_lightng");

	for (i=0 ; i<5 ; i++)
	{
		sb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_shotgun",i+1));
		sb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_sshotgun",i+1));
		sb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_nailgun",i+1));
		sb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_snailgun",i+1));
		sb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_rlaunch",i+1));
		sb_weapons[2+i][5] = Draw_PicFromWad (va("inva%i_srlaunch",i+1));
		sb_weapons[2+i][6] = Draw_PicFromWad (va("inva%i_lightng",i+1));
	}

	sb_ammo[0] = Draw_PicFromWad ("sb_shells");
	sb_ammo[1] = Draw_PicFromWad ("sb_nails");
	sb_ammo[2] = Draw_PicFromWad ("sb_rocket");
	sb_ammo[3] = Draw_PicFromWad ("sb_cells");


	sb_armor[0] = Draw_PicFromWad ("sb_armor1");
	sb_armor[1] = Draw_PicFromWad ("sb_armor2");
	sb_armor[2] = Draw_PicFromWad ("sb_armor3");

	sb_items[0] = Draw_PicFromWad ("sb_key1");
	sb_items[1] = Draw_PicFromWad ("sb_key2");
	sb_items[2] = Draw_PicFromWad ("sb_invis");
	sb_items[3] = Draw_PicFromWad ("sb_invuln");
	sb_items[4] = Draw_PicFromWad ("sb_suit");
	sb_items[5] = Draw_PicFromWad ("sb_quad");

	sb_sigil[0] = Draw_PicFromWad ("sb_sigil1");
	sb_sigil[1] = Draw_PicFromWad ("sb_sigil2");
	sb_sigil[2] = Draw_PicFromWad ("sb_sigil3");
	sb_sigil[3] = Draw_PicFromWad ("sb_sigil4");

	sb_faces[4][0] = Draw_PicFromWad ("face1");
	sb_faces[4][1] = Draw_PicFromWad ("face_p1");
	sb_faces[3][0] = Draw_PicFromWad ("face2");
	sb_faces[3][1] = Draw_PicFromWad ("face_p2");
	sb_faces[2][0] = Draw_PicFromWad ("face3");
	sb_faces[2][1] = Draw_PicFromWad ("face_p3");
	sb_faces[1][0] = Draw_PicFromWad ("face4");
	sb_faces[1][1] = Draw_PicFromWad ("face_p4");
	sb_faces[0][0] = Draw_PicFromWad ("face5");
	sb_faces[0][1] = Draw_PicFromWad ("face_p5");

	sb_face_invis = Draw_PicFromWad ("face_invis");
	sb_face_invuln = Draw_PicFromWad ("face_invul2");
	sb_face_invis_invuln = Draw_PicFromWad ("face_inv2");
	sb_face_quad = Draw_PicFromWad ("face_quad");

	Cmd_AddCommand ("+showscores", Sbar_ShowScores);
	Cmd_AddCommand ("-showscores", Sbar_DontShowScores);

	sb_sbar = Draw_PicFromWad ("sbar");
	sb_ibar = Draw_PicFromWad ("ibar");
	sb_scorebar = Draw_PicFromWad ("scorebar");

//MED 01/04/97 added new hipnotic weapons
	if (hipnotic)
	{
	  hsb_weapons[0][0] = Draw_PicFromWad ("inv_laser");
	  hsb_weapons[0][1] = Draw_PicFromWad ("inv_mjolnir");
	  hsb_weapons[0][2] = Draw_PicFromWad ("inv_gren_prox");
	  hsb_weapons[0][3] = Draw_PicFromWad ("inv_prox_gren");
	  hsb_weapons[0][4] = Draw_PicFromWad ("inv_prox");

	  hsb_weapons[1][0] = Draw_PicFromWad ("inv2_laser");
	  hsb_weapons[1][1] = Draw_PicFromWad ("inv2_mjolnir");
	  hsb_weapons[1][2] = Draw_PicFromWad ("inv2_gren_prox");
	  hsb_weapons[1][3] = Draw_PicFromWad ("inv2_prox_gren");
	  hsb_weapons[1][4] = Draw_PicFromWad ("inv2_prox");
	  for (i=0 ; i<5 ; i++)
	  {
		 hsb_weapons[2+i][0] = Draw_PicFromWad (va("inva%i_laser",i+1));
		 hsb_weapons[2+i][1] = Draw_PicFromWad (va("inva%i_mjolnir",i+1));
		 hsb_weapons[2+i][2] = Draw_PicFromWad (va("inva%i_gren_prox",i+1));
		 hsb_weapons[2+i][3] = Draw_PicFromWad (va("inva%i_prox_gren",i+1));
		 hsb_weapons[2+i][4] = Draw_PicFromWad (va("inva%i_prox",i+1));
	  }

	  hsb_items[0] = Draw_PicFromWad ("sb_wsuit");
	  hsb_items[1] = Draw_PicFromWad ("sb_eshld");
	}

	if (rogue)
	{
		rsb_invbar[0] = Draw_PicFromWad ("r_invbar1");
		rsb_invbar[1] = Draw_PicFromWad ("r_invbar2");

		rsb_weapons[0] = Draw_PicFromWad ("r_lava");
		rsb_weapons[1] = Draw_PicFromWad ("r_superlava");
		rsb_weapons[2] = Draw_PicFromWad ("r_gren");
		rsb_weapons[3] = Draw_PicFromWad ("r_multirock");
		rsb_weapons[4] = Draw_PicFromWad ("r_plasma");

		rsb_items[0] = Draw_PicFromWad ("r_shield1");
        rsb_items[1] = Draw_PicFromWad ("r_agrav1");

// PGM 01/19/97 - team color border
        rsb_teambord = Draw_PicFromWad ("r_teambord");
// PGM 01/19/97 - team color border

		rsb_ammo[0] = Draw_PicFromWad ("r_ammolava");
		rsb_ammo[1] = Draw_PicFromWad ("r_ammomulti");
		rsb_ammo[2] = Draw_PicFromWad ("r_ammoplasma");
	}
}


//=============================================================================

// drawing routines are relative to the status bar location

/*
=============
Sbar_DrawAlphaPic
Alpha = 1 :  full alpha which is fully solid
Alpha = 0.5 ... 50% transparency
Alpha = 0.25 ... 75% transparency
Alpha = 0 ... invisible .. not drawn essentially
This function calls Draw_AlphaPic in video_hardware_draw.cpp (PSP)/gl_draw.c (GLQuake)
instead of Draw_Pic (Draw_Pic draws fully solid images).  Draw_AlphaPic offers an extra field "alpha"
which can draw fully solid or with transparency/translucency or whatever you like to call it.
=============
*/
void Sbar_DrawAlphaPic (int x, int y, qpic_t *pic, float alpha)
{
  if(kurok)
     Draw_AlphaPic (x /* + ((vid.width - 320)>>1)*/, y + (vid.height-SBAR_HEIGHT), pic, alpha);
  else
  {
   if (cl.gametype == GAME_DEATHMATCH)
      Draw_AlphaPic (x /* + ((vid.width - 320)>>1)*/, y +

(vid.height-SBAR_HEIGHT), pic, alpha);
   else
      Draw_AlphaPic (x + ((vid.width - 320)>>1), y + (vid.height-SBAR_HEIGHT), pic, alpha);
  }
}
/*
=============
Sbar_DrawPic
=============
*/
void Sbar_DrawPic (int x, int y, qpic_t *pic)
{
  if(kurok)
  		Draw_Pic (x /* + ((vid.width - 320)>>1)*/, y + (vid.height-SBAR_HEIGHT), pic);
  else
  {
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_Pic (x /* + ((vid.width - 320)>>1)*/, y + (vid.height-SBAR_HEIGHT), pic);
	else
		Draw_Pic (x + ((vid.width - 320)>>1), y + (vid.height-SBAR_HEIGHT), pic);
  }
}
/*
=============
Sbar_DrawTransPic
=============
*/
void Sbar_DrawTransPic (int x, int y, qpic_t *pic)
{
  if(kurok)
  		Draw_TransPic (x /*+ ((vid.width - 320)>>1)*/, y + (vid.height-SBAR_HEIGHT), pic);
  else
  {
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_TransPic (x /*+ ((vid.width - 320)>>1)*/, y + (vid.height-SBAR_HEIGHT), pic);
	else
		Draw_TransPic (x + ((vid.width - 320)>>1), y + (vid.height-SBAR_HEIGHT), pic);
  }
}

/*
================
Sbar_DrawCharacter

Draws one solid graphics character
================
*/
void Sbar_DrawCharacter (int x, int y, int num)
{
  if(kurok)
		Draw_Character ( x /*+ ((vid.width - 320)>>1) */ + 4 , y + vid.height-SBAR_HEIGHT, num);
  else
  {
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_Character ( x /*+ ((vid.width - 320)>>1) */ + 4 , y + vid.height-SBAR_HEIGHT, num);
	else
		Draw_Character ( x + ((vid.width - 320)>>1) + 4 , y + vid.height-SBAR_HEIGHT, num);
  }
}

/*
================
Sbar_DrawString
================
*/
void Sbar_DrawString (int x, int y, char *str)
{
  if(kurok)
		Draw_String (x /*+ ((vid.width - 320)>>1)*/, y + vid.height-SBAR_HEIGHT, str);
  else
  {
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_String (x /*+ ((vid.width - 320)>>1)*/, y+ vid.height-SBAR_HEIGHT, str);
	else
		Draw_String (x + ((vid.width - 320)>>1), y+ vid.height-SBAR_HEIGHT, str);
  }
}

/*
===============
Sbar_DrawScrollString -- johnfitz

scroll the string inside a glscissor region
===============
*/
void Sbar_DrawScrollString (int x, int y, int width, char* str)
{
	int len, ofs;

	y += vid.height-SBAR_HEIGHT;
//	if (cl.gametype != GAME_DEATHMATCH) x += ((vid.width - 320)>>1);

	len = strlen(str)*8 + 40;
	ofs = ((int)(realtime*30))%len;

//	sceGuEnable(GU_SCISSOR_TEST);
//	sceGuViewport (x, y, glwidth, glheight);
//	sceGuScissor (x, vid.height - y - 8, 160, 8);

	Draw_String (x - ofs, y, str);
	Draw_String (x - ofs + len - 32, y, "///");
	Draw_String (x - ofs + len, y, str);

//	sceGuDisable(GU_SCISSOR_TEST);
}

/*
=============
Sbar_itoa
=============
*/
int Sbar_itoa (int num, char *buf)
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


/*
=============
Sbar_DrawNum
=============
*/
void Sbar_DrawNum (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);

    if (l < digits)
    {
        if (kurok)
	        x += (digits-l)*16;
        else
            x += (digits-l)*24;
    }

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Sbar_DrawTransPic (x,y,sb_nums[color][frame]);

        if (kurok)
		    x += 16;
		else
		    x += 24;
		ptr++;
	}
}

/*
=============
Sbar_DrawNum2
=============
*/
void Sbar_DrawNum2 (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);

    if (l < digits)
	    x += (digits-l)*0;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		M_DrawPic (x,y,sb_nums[color][frame]);

		x += 10;
		ptr++;
	}
}

//=============================================================================

int		fragsort[MAX_SCOREBOARD];

char	scoreboardtext[MAX_SCOREBOARD][20];
int		scoreboardtop[MAX_SCOREBOARD];
int		scoreboardbottom[MAX_SCOREBOARD];
int		scoreboardcount[MAX_SCOREBOARD];
int		scoreboardlines;

/*
===============
Sbar_SortFrags
===============
*/
void Sbar_SortFrags (void)
{
	int		i, j, k;

// sort by frags
	scoreboardlines = 0;
	for (i=0 ; i<cl.maxclients ; i++)
	{
		if (cl.scores[i].name[0])
		{
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}

	for (i=0 ; i<scoreboardlines ; i++)
		for (j=0 ; j<scoreboardlines-1-i ; j++)
			if (cl.scores[fragsort[j]].frags < cl.scores[fragsort[j+1]].frags)
			{
				k = fragsort[j];
				fragsort[j] = fragsort[j+1];
				fragsort[j+1] = k;
			}
}

int	Sbar_ColorForMap (int m)
{
	return m < 128 ? m + 8 : m + 8;
}

/*
===============
Sbar_UpdateScoreboard
===============
*/
void Sbar_UpdateScoreboard (void)
{
	int		i, k;
	int		top, bottom;
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	memset (scoreboardtext, 0, sizeof(scoreboardtext));

	for (i=0 ; i<scoreboardlines; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		sprintf (&scoreboardtext[i][1], "%3i %s", s->frags, s->name);

		top = s->colors & 0xf0;
		bottom = (s->colors & 15) <<4;
		scoreboardtop[i] = Sbar_ColorForMap (top);
		scoreboardbottom[i] = Sbar_ColorForMap (bottom);
	}
}



/*
===============
Sbar_SoloScoreboard
===============
*/
void Sbar_SoloScoreboard (void)
{
	char	str[80];
	int		minutes, seconds, tens, units;
	int		len;

	if(kurok)
		sprintf (str,"Enemies :%3i /%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	else
		sprintf (str,"Monsters:%3i /%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	Sbar_DrawString (8, 4, str);

	if(kurok)
		sprintf (str,"Objectives :%3i /%3i", cl.stats[STAT_SECRETS], cl.stats[STAT_TOTALSECRETS]);
	else
		sprintf (str,"Secrets :%3i /%3i", cl.stats[STAT_SECRETS], cl.stats[STAT_TOTALSECRETS]);
	Sbar_DrawString (8, 12, str);

// time
	minutes = cl.time / 60;
	seconds = cl.time - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	sprintf (str,"Time :%3i:%i%i", minutes, tens, units);
	Sbar_DrawString (184, 4, str);

	//johnfitz -- scroll long levelnames
	len = strlen (cl.levelname);
#ifdef PSP_HARDWARE_VIDEO
	if (len > 22)
		Sbar_DrawScrollString (184, 12, 160, cl.levelname);
	else
#endif
//		Sbar_DrawString (232 - len*4, 12, cl.levelname);
		Sbar_DrawString (184, 12, cl.levelname);
	//johnfitz

// draw level name
//	l = strlen (cl.levelname);
//	Sbar_DrawString (232 - l*4, 12, cl.levelname);
}

/*
===============
Sbar_DrawScoreboard
===============
*/
void Sbar_DrawScoreboard (void)
{
	Sbar_SoloScoreboard ();
	if (cl.gametype == GAME_DEATHMATCH)
		Sbar_DeathmatchOverlay ();
#if 0
	int		i, j, c;
	int		x, y;
	int		l;
	int		top, bottom;
	scoreboard_t	*s;

	if (cl.gametype != GAME_DEATHMATCH)
	{
		Sbar_SoloScoreboard ();
		return;
	}

	Sbar_UpdateScoreboard ();

	l = scoreboardlines <= 6 ? scoreboardlines : 6;

	for (i=0 ; i<l ; i++)
	{
		x = 20*(i&1);
		y = i/2 * 8;

		s = &cl.scores[fragsort[i]];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x*8+10 + ((vid.width - 320)>>1), y + vid.height - SBAR_HEIGHT, 28, 4, top);
		Draw_Fill ( x*8+10 + ((vid.width - 320)>>1), y+4 + vid.height - SBAR_HEIGHT, 28, 4, bottom);

	// draw text
		for (j=0 ; j<20 ; j++)
		{
			c = scoreboardtext[i][j];
			if (c == 0 || c == ' ')
				continue;
			Sbar_DrawCharacter ( (x+j)*8, y, c);
		}
	}
#endif
}

//=============================================================================

/*
===============
Sbar_DrawInventory
===============
*/
void Sbar_DrawInventory (void)
{
	int		i;
	char	num[6];
	float	time;
	int		flashon;

	if (rogue)
	{
		if ( cl.stats[STAT_ACTIVEWEAPON] >= RIT_LAVA_NAILGUN )
			Sbar_DrawPic (0, -24, rsb_invbar[0]);
		else
			Sbar_DrawPic (0, -24, rsb_invbar[1]);
	}
	else if (!kurok)
		Sbar_DrawPic (0, -24, sb_ibar);

// weapons
    if (!kurok)
    {
	for (i=0 ; i<7 ; i++)
	{
		if (cl.items & (IT_SHOTGUN<<i) )
		{
			time = cl.item_gettime[i];
			flashon = (int)((cl.time - time)*10);
			if (flashon >= 10)
			{
				if ( cl.stats[STAT_ACTIVEWEAPON] == (IT_SHOTGUN<<i)  )
					flashon = 1;
				else
					flashon = 0;
			}
			else
				flashon = (flashon%5) + 2;

         Sbar_DrawPic (i*24, -16, sb_weapons[flashon][i]);

			if (flashon > 1)
				sb_updates = 0;		// force update to remove flash
		}
	}
    }
// MED 01/04/97
// hipnotic weapons
    if (hipnotic)
    {
      int grenadeflashing=0;
      for (i=0 ; i<4 ; i++)
      {
         if (cl.items & (1<<hipweapons[i]) )
         {
            time = cl.item_gettime[hipweapons[i]];
            flashon = (int)((cl.time - time)*10);
            if (flashon >= 10)
            {
               if ( cl.stats[STAT_ACTIVEWEAPON] == (1<<hipweapons[i])  )
                  flashon = 1;
               else
                  flashon = 0;
            }
            else
               flashon = (flashon%5) + 2;

            // check grenade launcher
            if (i==2)
            {
               if (cl.items & HIT_PROXIMITY_GUN)
               {
                  if (flashon)
                  {
                     grenadeflashing = 1;
                     Sbar_DrawPic (96, -16, hsb_weapons[flashon][2]);
                  }
               }
            }
            else if (i==3)
            {
               if (cl.items & (IT_SHOTGUN<<4))
               {
                  if (flashon && !grenadeflashing)
                  {
                     Sbar_DrawPic (96, -16, hsb_weapons[flashon][3]);
                  }
                  else if (!grenadeflashing)
                  {
                     Sbar_DrawPic (96, -16, hsb_weapons[0][3]);
                  }
               }
               else
                  Sbar_DrawPic (96, -16, hsb_weapons[flashon][4]);
            }
            else
               Sbar_DrawPic (176 + (i*24), -16, hsb_weapons[flashon][i]);
            if (flashon > 1)
               sb_updates = 0;      // force update to remove flash
         }
      }
    }

	if (rogue)
	{
    // check for powered up weapon.
		if ( cl.stats[STAT_ACTIVEWEAPON] >= RIT_LAVA_NAILGUN )
		{
			for (i=0;i<5;i++)
			{
				if (cl.stats[STAT_ACTIVEWEAPON] == (RIT_LAVA_NAILGUN << i))
				{
					Sbar_DrawPic ((i+2)*24, -16, rsb_weapons[i]);
				}
			}
		}
	}

    if (!kurok)
    {

// ammo counts
	for (i=0 ; i<4 ; i++)
	{
		sprintf (num, "%3i",cl.stats[STAT_SHELLS+i] );
		if (num[0] != ' ')
			Sbar_DrawCharacter ( (6*i+1)*8 - 2, -24, 18 + num[0] - '0');
		if (num[1] != ' ')
			Sbar_DrawCharacter ( (6*i+2)*8 - 2, -24, 18 + num[1] - '0');
		if (num[2] != ' ')
			Sbar_DrawCharacter ( (6*i+3)*8 - 2, -24, 18 + num[2] - '0');
	}
    }
	flashon = 0;

   // items
   for (i=0 ; i<6 ; i++)
      if (cl.items & (1<<(17+i)))
      {
         time = cl.item_gettime[17+i];
         if (time && time > cl.time - 2 && flashon )
         {  // flash frame
            sb_updates = 0;
         }
         else
         {
         //MED 01/04/97 changed keys
            if (!hipnotic || (i>1))
            {
				if(kurok)
					Sbar_DrawPic (80 + i*16, 0, sb_items[i]);
				else
					Sbar_DrawPic (192 + i*16, -16, sb_items[i]);
            }
         }
         if (time && time > cl.time - 2)
            sb_updates = 0;
      }

   //MED 01/04/97 added hipnotic items
   // hipnotic items
   if (hipnotic)
   {
      for (i=0 ; i<2 ; i++)
         if (cl.items & (1<<(24+i)))
         {
            time = cl.item_gettime[24+i];
            if (time && time > cl.time - 2 && flashon )
            {  // flash frame
               sb_updates = 0;
            }
            else
            {
               Sbar_DrawPic (288 + i*16, -16, hsb_items[i]);
            }
            if (time && time > cl.time - 2)
               sb_updates = 0;
         }
   }

	if (rogue)
	{
	// new rogue items
		for (i=0 ; i<2 ; i++)
		{
			if (cl.items & (1<<(29+i)))
			{
				time = cl.item_gettime[29+i];

				if (time &&	time > cl.time - 2 && flashon )
				{	// flash frame
					sb_updates = 0;
				}
				else
				{
					Sbar_DrawPic (288 + i*16, -16, rsb_items[i]);
				}

				if (time &&	time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}

	if (kurok)
	{
/*
	// new kurok items
		for (i=0 ; i<2 ; i++)
		{
			if (cl.items & (1<<(29+i)))
			{
				time = cl.item_gettime[29+i];

				if (time &&	time > cl.time - 2 && flashon )
				{	// flash frame
					sb_updates = 0;
				}
				else
				{
					Sbar_DrawPic (288 + i*16, -16, rsb_items[i]);
				}

				if (time &&	time > cl.time - 2)
					sb_updates = 0;
			}
		}
*/
	}

	else
	{
	// sigils
		for (i=0 ; i<4 ; i++)
		{
			if (cl.items & (1<<(28+i)))
			{
				time = cl.item_gettime[28+i];
				if (time &&	time > cl.time - 2 && flashon )
				{	// flash frame
					sb_updates = 0;
				}
				else
					Sbar_DrawPic (320-32 + i*8, -16, sb_sigil[i]);
				if (time &&	time > cl.time - 2)
					sb_updates = 0;
			}
		}
	}
}

//=============================================================================

/*
===============
Sbar_DrawFrags
===============
*/
void Sbar_DrawFrags (void)
{
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	int				xofs;
	char			num[12];
	scoreboard_t	*s;

	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines <= 4 ? scoreboardlines : 4;

	x = 23;

    if (kurok)
    {
		xofs = 0;
    }
    else
    {
	    if (cl.gametype == GAME_DEATHMATCH)
		    xofs = 0;
	    else
		    xofs = (vid.width - 320)>>1;
    }

	y = vid.height - SBAR_HEIGHT - 23;

	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill (xofs + x*8 + 10, y, 28, 4, top);
		Draw_Fill (xofs + x*8 + 10, y+4, 28, 3, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Sbar_DrawCharacter ( (x+1)*8 , -24, num[0]);
		Sbar_DrawCharacter ( (x+2)*8 , -24, num[1]);
		Sbar_DrawCharacter ( (x+3)*8 , -24, num[2]);

		if (k == cl.viewentity - 1)
		{
			Sbar_DrawCharacter (x*8+2, -24, 16);
			Sbar_DrawCharacter ( (x+4)*8-4, -24, 17);
		}
		x+=4;
	}
}

//=============================================================================


/*
===============
Sbar_DrawFace
===============
*/
void Sbar_DrawFace (void)
{
	int		f, anim;

// PGM 01/19/97 - team color drawing
// PGM 03/02/97 - fixed so color swatch only appears in CTF modes
	if (rogue &&
        (cl.maxclients != 1) &&
        (teamplay.value>3) &&
        (teamplay.value<7))
	{
		int				top, bottom;
		int				xofs;
		char			num[12];
		scoreboard_t	*s;

		s = &cl.scores[cl.viewentity - 1];
		// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		if (kurok)
		{
			xofs = 113;
        }
        else
        {
		    if (cl.gametype == GAME_DEATHMATCH)
			    xofs = 113;
		    else
		        xofs = ((vid.width - 320)>>1) + 113;
        }

		Sbar_DrawPic (112, 0, rsb_teambord);
		Draw_Fill (xofs, vid.height-SBAR_HEIGHT+3, 22, 9, top);
		Draw_Fill (xofs, vid.height-SBAR_HEIGHT+12, 22, 9, bottom);

		// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		if (top==8)
		{
			if (num[0] != ' ')
				Sbar_DrawCharacter(109, 3, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter(116, 3, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter(123, 3, 18 + num[2] - '0');
		}
		else
		{
			Sbar_DrawCharacter ( 109, 3, num[0]);
			Sbar_DrawCharacter ( 116, 3, num[1]);
			Sbar_DrawCharacter ( 123, 3, num[2]);
		}

		return;
	}
// PGM 01/19/97 - team color drawing

    if (!kurok)
    {

	if ( (cl.items & (IT_INVISIBILITY | IT_INVULNERABILITY) )
	== (IT_INVISIBILITY | IT_INVULNERABILITY) )
	{
        if (kurok)
		    Sbar_DrawPic (8, 0, sb_face_invis_invuln);
		else
		    Sbar_DrawPic (112, 0, sb_face_invis_invuln);
		return;
	}
	if (cl.items & IT_QUAD)
	{
        if (kurok)
		    Sbar_DrawPic (8, 0, sb_face_quad );
        else
		    Sbar_DrawPic (112, 0, sb_face_quad );
		return;
	}
	if (cl.items & IT_INVISIBILITY)
	{
        if (kurok)
		    Sbar_DrawPic (8, 0, sb_face_invis );
        else
		    Sbar_DrawPic (112, 0, sb_face_invis );
		return;
	}
	if (cl.items & IT_INVULNERABILITY)
	{
        if (kurok)
		    Sbar_DrawPic (8, 0, sb_face_invuln);
        else
		    Sbar_DrawPic (112, 0, sb_face_invuln);
		return;
	}

    }

/*
========================================
Health bar edges
========================================
*/
	if (cl.stats[STAT_HEALTH] <= 30)
	{
	M_DrawPic ( (324-health_bar_red->width)/2, 16, health_bar_red);
	}
	else
	{
	M_DrawPic ( (324-health_bar->width)/2, 16, health_bar);
	}



/*
===================================
Health bar 2 (322 = middle)
===================================
*/
if (cl.stats[STAT_HEALTH] >= 130)
	{
	M_DrawPic ( (422-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 129)
	{
	M_DrawPic ( (420-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 128)
	{
	M_DrawPic ( (418-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 127)
	{
	M_DrawPic ( (416-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 126)
	{
	M_DrawPic ( (414-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 125)
	{
	M_DrawPic ( (412-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 124)
	{
	M_DrawPic ( (410-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 123)
	{
	M_DrawPic ( (408-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 122)
	{
	M_DrawPic ( (406-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 121)
	{
	M_DrawPic ( (404-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 120)
	{
	M_DrawPic ( (402-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 119)
	{
	M_DrawPic ( (400-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 118)
	{
	M_DrawPic ( (398-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 117)
	{

	M_DrawPic ( (396-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 116)
	{

	M_DrawPic ( (394-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 115)
	{

	M_DrawPic ( (392-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 114)
	{

	M_DrawPic ( (390-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 113)
	{

	M_DrawPic ( (388-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 112)
	{

	M_DrawPic ( (386-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 111)
	{

	M_DrawPic ( (384-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 110)
	{

	M_DrawPic ( (382-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 109)
	{

	M_DrawPic ( (380-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 108)
	{

	M_DrawPic ( (378-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 107)
	{

	M_DrawPic ( (376-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 106)
	{

	M_DrawPic ( (374-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 105)
	{

	M_DrawPic ( (372-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 104)
	{

	M_DrawPic ( (370-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 103)
	{

	M_DrawPic ( (368-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 102)
	{

	M_DrawPic ( (366-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 101)
	{

	M_DrawPic ( (364-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 100)
	{

	M_DrawPic ( (362-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 99)
	{

	M_DrawPic ( (360-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 98)
	{

	M_DrawPic ( (358-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 97)
	{

	M_DrawPic ( (356-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 96)
	{

	M_DrawPic ( (354-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 95)
	{

	M_DrawPic ( (352-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 94)
	{

	M_DrawPic ( (350-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 93)
	{

	M_DrawPic ( (348-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 92)
	{

	M_DrawPic ( (346-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 91)
	{

	M_DrawPic ( (344-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 90)
	{

	M_DrawPic ( (342-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 89)
	{

	M_DrawPic ( (340-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 88)
	{

	M_DrawPic ( (338-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 87)
	{

	M_DrawPic ( (336-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 86)
	{

	M_DrawPic ( (334-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 85)
	{

	M_DrawPic ( (332-life90->width)/2, 20, life90 );
	}
		if (cl.stats[STAT_HEALTH] >= 84)
	{

	M_DrawPic ( (330-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 83)
	{

	M_DrawPic ( (328-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 82)
	{

	M_DrawPic ( (326-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 81)
	{

	M_DrawPic ( (324-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 80)
	{

	M_DrawPic ( (322-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 79)
	{

	M_DrawPic ( (320-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 78)
	{

	M_DrawPic ( (318-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 77)
	{

	M_DrawPic ( (316-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 76)
	{

	M_DrawPic ( (314-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 75)
	{

	M_DrawPic ( (312-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 74)
	{

	M_DrawPic ( (310-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 73)
	{

	M_DrawPic ( (308-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 72)			//Checkpoint
	{

	M_DrawPic ( (306-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 71)
	{

	M_DrawPic ( (304-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 70)
	{

	M_DrawPic ( (302-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 69)
	{

	M_DrawPic ( (300-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 68)
	{

	M_DrawPic ( (298-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 67)
	{

	M_DrawPic ( (296-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 66)
	{

	M_DrawPic ( (294-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 65)
	{

	M_DrawPic ( (292-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 64)
	{

	M_DrawPic ( (290-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 63)
	{

	M_DrawPic ( (288-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 62)
	{

	M_DrawPic ( (286-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 61)
	{

	M_DrawPic ( (284-life90->width)/2, 20, life90 );
	}

	if (cl.stats[STAT_HEALTH] > 60)
	{

	M_DrawPic ( (282-life90->width)/2, 20, life90 );
	}

	if (cl.stats[STAT_HEALTH] >= 59)
	{

	M_DrawPic ( (280-life90->width)/2, 20, life90 );
		}

	if (cl.stats[STAT_HEALTH] >= 58)
	{

	M_DrawPic ( (278-life90->width)/2, 20, life90 );
	}

	if (cl.stats[STAT_HEALTH] >= 57)
	{

	M_DrawPic ( (276-life90->width)/2, 20, life90 );
	}

	if (cl.stats[STAT_HEALTH] >= 56)
	{

	M_DrawPic ( (274-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 55)
	{

	M_DrawPic ( (272-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 54)
	{

	M_DrawPic ( (270-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 53)
	{

	M_DrawPic ( (268-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 52)
	{

	M_DrawPic ( (266-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 51)		//Checkpoint 2
	{

	M_DrawPic ( (264-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 50)
	{

	M_DrawPic ( (262-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 49)
	{

	M_DrawPic ( (260-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 48)
	{

	M_DrawPic ( (258-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 47)
	{

	M_DrawPic ( (256-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 46)
	{

	M_DrawPic ( (254-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 45)
	{

	M_DrawPic ( (252-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 44)
	{

	M_DrawPic ( (250-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 43)
	{

	M_DrawPic ( (248-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 42)
	{

	M_DrawPic ( (246-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 41)
	{

	M_DrawPic ( (244-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 40)
	{

	M_DrawPic ( (242-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 39)
	{

	M_DrawPic ( (240-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 38)
	{

	M_DrawPic ( (238-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 37)
	{

	M_DrawPic ( (236-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 36)
	{

	M_DrawPic ( (234-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 35)
	{

	M_DrawPic ( (232-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 34)
	{

	M_DrawPic ( (230-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 33)
	{

	M_DrawPic ( (228-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 32)
	{

	M_DrawPic ( (226-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 31)
	{

	M_DrawPic ( (224-life90->width)/2, 20, life90 );
	}
	if (cl.stats[STAT_HEALTH] >= 30)
	{
	}




/*
===================================
Displays weapons that are on the ground.
===================================
*/
	//Assault rifle
	if (cl_ww.value == 1)
	{

	M_DrawPic ( (680-ARW->width)/2, 55, ARW);
	}
	//plasma Pistol
	if (cl_ww.value == 2)
	{
	M_DrawPic ( (680-ppistW->width)/2, 55, ppistW );
	}
	//Shottie
	if (cl_ww.value == 3)
	{
	M_DrawPic ( (680-ShottieW->width)/2, 55, ShottieW );
	}
	//Needler
	if (cl_ww.value == 4)
	{
	M_DrawPic ( (680-NeedlerW->width)/2, 55, NeedlerW );
	}
	//Rocket Launcher
	if (cl_ww.value == 5)
	{
	M_DrawPic ( (680-RLW->width)/2, 55, RLW );
	}
	//Pistol
	if (cl_ww.value == 6)
	{
	M_DrawPic ( (680-PistolW->width)/2, 55, PistolW );
	}
	//SMG
	if (cl_ww.value == 7)
	{
	M_DrawPic ( (680-SMGW->width)/2, 55, SMGW );
	}
	//Sniper
	if (cl_ww.value == 8)
	{
	M_DrawPic ( (680-SniperW->width)/2, 55, SniperW );
	}
/*
===================================
Weapon HUD. Displays current weapon
===================================
*/
	//Assault rifle
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_NAILGUN)
	{
	M_DrawPic ( (680-AR->width)/2, 15, AR);
	}
	//plasma Pistol
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_LIGHTNING)
	{
	M_DrawPic ( (680-ppist->width)/2, 15, ppist );
	}
	//Shottie
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_SHOTGUN)
	{
	M_DrawPic ( (680-Shottie->width)/2, 15, Shottie );
	}
	//Needler
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_EXTRA_WEAPON)
	{
	M_DrawPic ( (680-Needler->width)/2, 15, Needler );
	}
	//Rocket Launcher
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_ROCKET_LAUNCHER)
	{
	M_DrawPic ( (680-RL->width)/2, 15, RL );
	}
	//Pistol
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_SHOTGUN)
	{
	M_DrawPic ( (680-Pistol->width)/2, 15, Pistol );
	}
	//SMG
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_NAILGUN)
	{
	M_DrawPic ( (680-SMG->width)/2, 15, SMG );
	}
	//Sniper
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_GRENADE_LAUNCHER)
	{
	M_DrawPic ( (680-Sniper->width)/2, 15, Sniper );
	}

/*
===================================
Needle ammo counter
===================================
*/

	if (cl.stats[STAT_ACTIVEWEAPON] == IT_EXTRA_WEAPON)
	{
	if (cl.stats[STAT_AMMO] >= 20)		
	{
	M_DrawPic ( (620-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 19)		
	{
	M_DrawPic ( (626-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 18)		
	{
	M_DrawPic ( (632-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 17)		
	{
	M_DrawPic ( (638-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 16)		
	{
	M_DrawPic ( (644-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 15)		
	{
	M_DrawPic ( (650-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 14)		
	{

	M_DrawPic ( (656-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 13)		
	{

	M_DrawPic ( (662-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 12)		
	{

	M_DrawPic ( (668-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 11)		
	{

	M_DrawPic ( (674-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 10)		
	{

	M_DrawPic ( (680-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 9)		
	{

	M_DrawPic ( (686-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 8)		
	{

	M_DrawPic ( (692-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 7)		
	{

	M_DrawPic ( (698-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 6)		
	{

	M_DrawPic ( (704-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 5)		
	{

	M_DrawPic ( (710-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 4)		
	{

	M_DrawPic ( (716-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 3)		
	{

	M_DrawPic ( (722-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 2)		
	{

	M_DrawPic ( (728-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 1)		
	{

	M_DrawPic ( (734-needleb->width)/2, 43, needleb);
	}
	if (cl.stats[STAT_AMMO] >= 0)		
	{
	}

	}
/*
===================================
Assault rifle counter
===================================
*/
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_NAILGUN)
	{
	if (cl.stats[STAT_AMMO] >= 32)		
	{

	M_DrawPic ( (650-arb->width)/2, 36, arb);
	}

	if (cl.stats[STAT_AMMO] >= 31)		
	{

	M_DrawPic ( (656-arb->width)/2, 36, arb);
	}
	
	if (cl.stats[STAT_AMMO] >= 30)		
	{

	M_DrawPic ( (662-arb->width)/2, 36, arb);
	}
	
	if (cl.stats[STAT_AMMO] >= 29)		
	{

	M_DrawPic ( (668-arb->width)/2, 36, arb);
	}

	if (cl.stats[STAT_AMMO] >= 28)		
	{

	M_DrawPic ( (674-arb->width)/2, 36, arb);
	}

	if (cl.stats[STAT_AMMO] >= 27)		
	{

	M_DrawPic ( (680-arb->width)/2, 36, arb);
	}

	if (cl.stats[STAT_AMMO] >= 26)		
	{

	M_DrawPic ( (686-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 25)		
	{

	M_DrawPic ( (692-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 24)		
	{

	M_DrawPic ( (698-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 23)		
	{

	M_DrawPic ( (704-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 22)		
	{

	M_DrawPic ( (710-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 21)		
	{

	M_DrawPic ( (716-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 20)		
	{

	M_DrawPic ( (722-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 19)		
	{

	M_DrawPic ( (728-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 18)		
	{

	M_DrawPic ( (734-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 17)		
	{

	M_DrawPic ( (740-arb->width)/2, 36, arb);
	}
	if (cl.stats[STAT_AMMO] >= 16)		
	{

	M_DrawPic ( (650-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 15)		
	{

	M_DrawPic ( (656-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 14)		
	{

	M_DrawPic ( (662-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 13)		
	{

	M_DrawPic ( (668-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 12)		
	{

	M_DrawPic ( (674-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 11)		
	{

	M_DrawPic ( (680-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 10)		
	{

	M_DrawPic ( (686-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 9)		
	{

	M_DrawPic ( (692-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 8)		
	{

	M_DrawPic ( (698-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 7)		
	{

	M_DrawPic ( (704-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 6)		
	{

	M_DrawPic ( (710-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 5)		
	{

	M_DrawPic ( (716-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 4)		
	{

	M_DrawPic ( (722-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 3)		
	{

	M_DrawPic ( (728-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 2)		
	{

	M_DrawPic ( (734-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 1)		
	{

	M_DrawPic ( (740-arb->width)/2, 43, arb);
	}
	if (cl.stats[STAT_AMMO] >= 0)		
	{
	}

	}
/*
===================================
Rocket Ammo counter
===================================
*/
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_ROCKET_LAUNCHER)
	{
	if (cl.stats[STAT_AMMO] >= 2)		
	{

	M_DrawPic ( (665-rocketb->width)/2, 47, rocketb);
	}

	if (cl.stats[STAT_AMMO] >= 1)		
	{

	M_DrawPic ( (735-rocketb->width)/2, 47, rocketb);
	}

	if (cl.stats[STAT_AMMO] >= 0)		
	{
	}
	
	}
/*
===================================
Shotgun Ammo counter
===================================
*/
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_SHOTGUN)
	{
	if (cl.stats[STAT_AMMO] >= 6)		
	{

	M_DrawPic ( (690-shotgunb->width)/2, 36, shotgunb);
	}

	if (cl.stats[STAT_AMMO] >= 5)		
	{

	M_DrawPic ( (700-shotgunb->width)/2, 36, shotgunb);
	}
	
	if (cl.stats[STAT_AMMO] >= 4)		
	{

	M_DrawPic ( (710-shotgunb->width)/2, 36, shotgunb);
	}
	
	if (cl.stats[STAT_AMMO] >= 3)		
	{

	M_DrawPic ( (720-shotgunb->width)/2, 36, shotgunb);
	}

	if (cl.stats[STAT_AMMO] >= 2)		
	{

	M_DrawPic ( (730-shotgunb->width)/2, 36, shotgunb);
	}

	if (cl.stats[STAT_AMMO] >= 1)		
	{

	M_DrawPic ( (740-shotgunb->width)/2, 36, shotgunb);
	}

	if (cl.stats[STAT_AMMO] >= 0)		
	{
	}
	
	}

/*
===================================
Pistol ammo counter
===================================
*/
	if (cl.stats[STAT_ACTIVEWEAPON] == IT_SHOTGUN)
	{
	//Pistol
	if (cl.stats[STAT_AMMO] >= 12)
	{

	M_DrawPic ( (690-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 11)	
	{

	M_DrawPic ( (696-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 10)		
	{

	M_DrawPic ( (702-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 9)		
	{

	M_DrawPic ( (708-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 8)		
	{

	M_DrawPic ( (714-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 7)		
	{

	M_DrawPic ( (720-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 6)		
	{

	M_DrawPic ( (726-pistolb->width)/2, 35, pistolb);
	}

	if (cl.stats[STAT_AMMO] >= 5)		
	{

	M_DrawPic ( (732-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 4)		
	{

	M_DrawPic ( (738-pistolb->width)/2, 35, pistolb);
	}
	
	if (cl.stats[STAT_AMMO] >= 3)		
	{

	M_DrawPic ( (744-pistolb->width)/2, 35, pistolb);
	}

	if (cl.stats[STAT_AMMO] >= 2)		
	{

	M_DrawPic ( (750-pistolb->width)/2, 35, pistolb);
	}

	if (cl.stats[STAT_AMMO] >= 1)		
	{

	M_DrawPic ( (756-pistolb->width)/2, 35, pistolb);
	}

	if (cl.stats[STAT_AMMO] >= 0)		
	{
	}
	
	}

/*
===================================
Grenade counter numbers
===================================
*/
	if (cl_nadenum.value == 1)
	{
	M_DrawPic ((-40 - numbers[0]->width)/2, 15, numbers[0]);
	}
	if (cl_nadenum.value == 2)
	{
	M_DrawPic ( (-40-numbers[1]->width)/2, 15, numbers[1] );
	}
	if (cl_nadenum.value == 3)
	{
	M_DrawPic ( (-40-numbers[2]->width)/2, 15, numbers[2] );
	}
	if (cl_nadenum.value == 4)
	{
	M_DrawPic ( (-40-numbers[3]->width)/2, 15, numbers[3] );
	}
	if (cl_nadenum.value == 5)
	{
	M_DrawPic ( (-40-numbers[4]->width)/2, 15, numbers[4] );
	}

/*
===================================
Displays what Grenade you currently have.
===================================
*/
	//For Plasma Nades.
	if (cl_plasmanade.value == 1)
	{
	M_DrawPic ( (-70-plasma->width)/2, 15, plasma);
	}

	//These are Actually Frag Grenades.
	if (cl_plasmanade.value == 2)
	{
	M_DrawPic ( (-70-frag->width)/2, 15, frag);
	}
/*
===================================
Kill medals
===================================
*/
	if (cl_killmedals.value == 1)
	{
	M_DrawPic ( (-70-doublek->width)/2, 165, doublek );
	}
	if (cl_killmedals.value == 2)
	{
	M_DrawPic ( (-70-triple->width)/2, 165, triple );
	}
	if (cl_killmedals.value == 3)
	{
	M_DrawPic ( (-70-killtacular->width)/2, 165, killtacular );
	}
	if (cl_killmedals.value == 4)
	{
	M_DrawPic ( (-70-killtrocity->width)/2, 165, killtrocity );
	}


}
/*
===============
Sbar_Draw
===============
*/
void Sbar_Draw (void)
{
    extern cvar_t cl_gunpitch;

	if (scr_con_current == vid.height)
		return;		// console is full screen

    if (scr_viewsize.value == 130)
        return;

    if (!kurok)
    {
	    if (sb_updates >= vid.numpages)
		   return;
    }
	scr_copyeverything = 1;

	sb_updates++;

	if (sb_lines && vid.width > 320)
		Draw_TileClear (0, vid.height - sb_lines, vid.width, sb_lines);

    if (kurok)
    {
	    if (scr_viewsize.value < 130)
	    {
		   Sbar_DrawInventory ();
               if (cl.maxclients != 1)
               {
               	   if (cl.stats[STAT_HEALTH] > 0)
               	   {
                       Sbar_DeathmatchOverlay2 ();
                       Sbar_TimeOverlay();
                   }
               }
        }
	    else// if (sb_lines < 24)
	    {
               if (cl.maxclients != 1)
               {
               	   if (cl.stats[STAT_HEALTH] > 0)
               	   {
                       Sbar_DeathmatchOverlay2 ();
                       Sbar_TimeOverlay();
                   }
               }
        }
    }
    else
    {
	    if (sb_lines > 24)
	    {
		   Sbar_DrawInventory ();
		       if (cl.maxclients != 1)
			       Sbar_DrawFrags ();
        }
	    else if (sb_lines < 48)
	    {
               if (cl.maxclients != 1)
               {
               	   if (cl.stats[STAT_HEALTH] > 0)
                   Sbar_DeathmatchOverlay2 ();
               }
        }
    }

	if (sb_showscores || cl.stats[STAT_HEALTH] <= 0)
	{
		if(!kurok)
			Sbar_DrawPic (0, 0, sb_scorebar);
		Sbar_DrawScoreboard ();
		sb_updates = 0;
	}
//	else if (sb_lines)
	else if (sb_lines >= 0)
	{

    if (!kurok)
    {
        if (sb_lines >= 24)
            Sbar_DrawPic (0, 0, sb_sbar);
    }

   // keys (hipnotic only)
      //MED 01/04/97 moved keys here so they would not be overwritten
      if (hipnotic)
      {
         if (cl.items & IT_KEY1)
            Sbar_DrawPic (209, 3, sb_items[0]);
         if (cl.items & IT_KEY2)
            Sbar_DrawPic (209, 12, sb_items[1]);
      }
   // armor
		if (cl.items & IT_INVULNERABILITY)
		{
			Sbar_DrawNum (24, 0, 666, 3, 1);
			Sbar_DrawPic (0, 0, draw_disc);
		}
		else
		{
			if (rogue)
			{
				Sbar_DrawNum (24, 0, cl.stats[STAT_ARMOR], 3,
								cl.stats[STAT_ARMOR] <= 25);
				if (cl.items & RIT_ARMOR3)
					Sbar_DrawPic (0, 0, sb_armor[2]);
				else if (cl.items & RIT_ARMOR2)
					Sbar_DrawPic (0, 0, sb_armor[1]);
				else if (cl.items & RIT_ARMOR1)
					Sbar_DrawPic (0, 0, sb_armor[0]);
			}
		}

	// face
		Sbar_DrawFace ();

	// ammo icon
		if (rogue)
		{
			if (cl.items & RIT_SHELLS)
				Sbar_DrawPic (224, 0, sb_ammo[0]);
			else if (cl.items & RIT_NAILS)
				Sbar_DrawPic (224, 0, sb_ammo[1]);
			else if (cl.items & RIT_ROCKETS)
				Sbar_DrawPic (224, 0, sb_ammo[2]);
			else if (cl.items & RIT_CELLS)
				Sbar_DrawPic (224, 0, sb_ammo[3]);
			else if (cl.items & RIT_LAVA_NAILS)
				Sbar_DrawPic (224, 0, rsb_ammo[0]);
			else if (cl.items & RIT_PLASMA_AMMO)
				Sbar_DrawPic (224, 0, rsb_ammo[1]);
			else if (cl.items & RIT_MULTI_ROCKETS)
				Sbar_DrawPic (224, 0, rsb_ammo[2]);
		}

	if (cl.stats[STAT_ARMOR] <= 0)
        {}
        else
        {
            if (kurok)
                Sbar_DrawNum2 (620/2, 10, cl.stats[STAT_ARMOR], 3, cl.stats[STAT_ARMOR] <= 25);
            else
                Sbar_DrawNum (620/2, 10, cl.stats[STAT_ARMOR], 3, cl.stats[STAT_ARMOR] <= 25);
        }
        //if (cl.stats[STAT_HEALTH] <= 0)
        //{}
        //else
        //{
             //if (kurok)
             //{
                //if (cl.stats[STAT_ARMOR] >= 1)
                //{}
               //else
              //  Sbar_DrawNum2 (24, 0, cl.stats[STAT_HEALTH], 3, cl.stats[STAT_HEALTH] <= 25);
            // }
             //else
             //Sbar_DrawNum (136, 0, cl.stats[STAT_HEALTH], 3, cl.stats[STAT_HEALTH] <= 25);
        //}

if (kurok)
        {
            if (!cl_gunpitch.value)
            {
                if (cl.stats[STAT_ACTIVEWEAPON] == IT_SHOTGUN)      // Pistol
                {
                  {}//  Sbar_DrawNum (352, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 1);
                   {}// Sbar_DrawNum2 (418, 0, cl.stats[STAT_NAILS], 3, cl.stats[STAT_NAILS] <= 10);
                }
                else if (cl.stats[STAT_ACTIVEWEAPON] == IT_NAILGUN) // Assualt rifle
                {
                   {}// Sbar_DrawNum (352, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 1);
                   {}// Sbar_DrawNum2 (418, 0, cl.stats[STAT_NAILS], 3, cl.stats[STAT_NAILS] <= 10);
                }
                else if (cl.stats[STAT_ACTIVEWEAPON] == KIT_UZI) // Uzi
                {
                   {}// Sbar_DrawNum (352, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 1);
                   {}// Sbar_DrawNum2 (418, 0, cl.stats[STAT_NAILS], 3, cl.stats[STAT_NAILS] <= 10);
                }
                else if (cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_SHOTGUN) // Shotgun
                 {}//   Sbar_DrawNum2 (418, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);

                else if (cl.stats[STAT_ACTIVEWEAPON] == KIT_M99) // Sniper
                  {}//  Sbar_DrawNum2 (418, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);

                else if (cl.stats[STAT_ACTIVEWEAPON] == IT_SUPER_NAILGUN) // Minigun
                   {}// Sbar_DrawNum2 (418, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);

                else if (cl.stats[STAT_ACTIVEWEAPON] == IT_GRENADE_LAUNCHER) // Grenade launcher
                  {}//  Sbar_DrawNum2 (418, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);

                else if (cl.stats[STAT_ACTIVEWEAPON] == IT_ROCKET_LAUNCHER) // Rocket launcher
                   {}// Sbar_DrawNum2 (418, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);

                else if (cl.stats[STAT_ACTIVEWEAPON] == IT_LIGHTNING) // Remote Mines
                   {}// Sbar_DrawNum2 (418, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);

                else if (cl.stats[STAT_AMMO] <= 0) // Axe / Bow
                {

                }
                else
                   {}// Sbar_DrawNum2 ((740-shotgunb->width)/2, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] < 0);
            }
        }
        else
            {}//Sbar_DrawNum (248, 0, cl.stats[STAT_AMMO], 3, cl.stats[STAT_AMMO] <= 10);
	}

//	Con_Printf ("I am an %i! woohoo!\n", cl.stats[STAT_ACTIVEWEAPON]);

//	if (vid.width > 320) {
		if (cl.gametype == GAME_DEATHMATCH)
			Sbar_MiniDeathmatchOverlay ();
//	}
}


//=============================================================================

/*
==================
Sbar_IntermissionNumber

==================
*/
void Sbar_IntermissionNumber (int x, int y, int num, int digits, int color)
{
	char			str[12];
	char			*ptr;
	int				l, frame;

	l = Sbar_itoa (num, str);
	ptr = str;
	if (l > digits)
		ptr += (l-digits);
	if (l < digits)
		x += (digits-l)*24;

	while (*ptr)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		Draw_TransPic (x,y,sb_nums[color][frame]);
		x += 24;
		ptr++;
	}
}


/*
==================
Sbar_DeathmatchOverlay

==================
*/
void Sbar_DeathmatchOverlay (void)
{
	qpic_t			*pic;
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	pic = Draw_CachePic ("gfx/ranking.lmp");
	M_DrawPic ((320-pic->width)/2, 8, pic);

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;

	x = 80 + ((vid.width - 320)>>1);
	y = 40;
	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y, 40, 4, top);
		Draw_Fill ( x, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Draw_Character ( x+8 , y, num[0]);
		Draw_Character ( x+16 , y, num[1]);
		Draw_Character ( x+24 , y, num[2]);

//		if (k == cl.viewentity - 1)
//			Draw_Character ( x - 8, y, 12);

		if (k == cl.viewentity - 1) {
			Draw_Character ( x, y, 16);
			Draw_Character ( x + 32, y, 17);
		}

#if 0
{
	int				total;
	int				n, minutes, tens, units;

	// draw time
		total = cl.completed_time - s->entertime;
		minutes = (int)total/60;
		n = total - minutes*60;
		tens = n/10;
		units = n%10;

		sprintf (num, "%3i:%i%i", minutes, tens, units);

		Draw_String ( x+48 , y, num);
}
#endif

	// draw name
		Draw_String (x+64, y, s->name);

		y += 10;
	}
}

/*
==================
Sbar_DeathmatchOverlay

==================
*/
void Sbar_MiniDeathmatchOverlay (void)
{
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;
	int				numlines;

	if (vid.width < 512 || !sb_lines)
		return;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;
	y = vid.height - sb_lines;
	numlines = sb_lines/8;
	if (numlines < 3)
		return;

	//find us
	for (i = 0; i < scoreboardlines; i++)
		if (fragsort[i] == cl.viewentity - 1)
			break;

    if (i == scoreboardlines) // we're not there
            i = 0;
    else // figure out start
            i = i - numlines/2;

    if (i > scoreboardlines - numlines)
            i = scoreboardlines - numlines;
    if (i < 0)
            i = 0;

	x = 324;
	for (/* */; i < scoreboardlines && y < vid.height - 8 ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y+1, 40, 3, top);
		Draw_Fill ( x, y+4, 40, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Draw_Character ( x+8 , y, num[0]);
		Draw_Character ( x+16 , y, num[1]);
		Draw_Character ( x+24 , y, num[2]);

		if (k == cl.viewentity - 1) {
			Draw_Character ( x, y, 16);
			Draw_Character ( x + 32, y, 17);
		}

#if 0
{
	int				total;
	int				n, minutes, tens, units;

	// draw time
		total = cl.completed_time - s->entertime;
		minutes = (int)total/60;
		n = total - minutes*60;
		tens = n/10;
		units = n%10;

		sprintf (num, "%3i:%i%i", minutes, tens, units);

		Draw_String ( x+48 , y, num);
}
#endif

	// draw name
		Draw_String (x+48, y, s->name);

		y += 8;
	}
}

/*
==================
Sbar_DeathmatchOverlay2

==================
*/
void Sbar_DeathmatchOverlay2 (void)
{
//	qpic_t			*pic;
	int				i, k, l;
	int				top, bottom;
	int				x, y, f;
	char			num[12];
	scoreboard_t	*s;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

// scores
	Sbar_SortFrags ();

// draw the text
	l = scoreboardlines;

	x = 16;
	y = 56;
	for (i=0 ; i<l ; i++)
	{
		k = fragsort[i];
		s = &cl.scores[k];
		if (!s->name[0])
			continue;

	// draw background
		top = s->colors & 0xf0;
		bottom = (s->colors & 15)<<4;
		top = Sbar_ColorForMap (top);
		bottom = Sbar_ColorForMap (bottom);

		Draw_Fill ( x, y, 24, 4, top);
		Draw_Fill ( x, y+4, 24, 4, bottom);

	// draw number
		f = s->frags;
		sprintf (num, "%3i",f);

		Draw_Character ( x, y, num[0]);
		Draw_Character ( x+8 , y, num[1]);
		Draw_Character ( x+16 , y, num[2]);

		if (k == cl.viewentity - 1)
			Draw_Character ( x - 8, y, 13);

		Draw_String (x+32, y, s->name);

		y += 10;
	}
}

/*
==================
Sbar_TimeOverlay

==================
*/

void Sbar_TimeOverlay (void)
{
	char	str[80];
	int		minutes, seconds, tens, units;
	int		x, y;

	x = 8;
	y = 56;

	minutes = cl.time / 60;
	seconds = cl.time - 60*minutes;
	tens = seconds / 10;
	units = seconds - 10*tens;
	sprintf (str,"Time %i:%i%i", minutes, tens, units);
    Draw_String ( x, y - 16, str);
}
/*
==================
Sbar_IntermissionOverlay

==================
*/
void Sbar_IntermissionOverlay (void)
{
	qpic_t	*pic;
	int		dig;
	int		num;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	if (cl.gametype == GAME_DEATHMATCH)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	pic = Draw_CachePic ("gfx/complete.lmp");
	Draw_Pic (64, 24, pic);

	pic = Draw_CachePic ("gfx/inter.lmp");
	Draw_TransPic (0, 56, pic);

// time
	dig = cl.completed_time/60;
	Sbar_IntermissionNumber (160, 64, dig, 3, 0);
	num = cl.completed_time - dig*60;
	Draw_TransPic (234,64,sb_colon);
	Draw_TransPic (246,64,sb_nums[0][num/10]);
	Draw_TransPic (266,64,sb_nums[0][num%10]);

	Sbar_IntermissionNumber (160, 104, cl.stats[STAT_SECRETS], 3, 0);
	Draw_TransPic (232,104,sb_slash);
	Sbar_IntermissionNumber (240, 104, cl.stats[STAT_TOTALSECRETS], 3, 0);

	Sbar_IntermissionNumber (160, 144, cl.stats[STAT_MONSTERS], 3, 0);
	Draw_TransPic (232,144,sb_slash);
	Sbar_IntermissionNumber (240, 144, cl.stats[STAT_TOTALMONSTERS], 3, 0);

}


/*
==================
Sbar_FinaleOverlay

==================
*/
void Sbar_FinaleOverlay (void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;

	pic = Draw_CachePic ("gfx/finale.lmp");
	Draw_TransPic ( (vid.width-pic->width)/2, 16, pic);
}
