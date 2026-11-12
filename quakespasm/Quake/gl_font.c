/*
 * gl_font.c -- TTF HUD font (P2a of the HUD rework: docs/hud-rework-spec.md)
 *
 * Bakes ASCII bitmap atlases once at HUD_Init via stb_truetype and draws
 * scaled quads in the same immediate-mode style as Draw_ColoredStringScale.
 * Font family: Archivo Narrow (SIL OFL, see solitude/gfx/fonts/OFL.txt),
 * a metric cousin of Halo's condensed grotesque UI face. Real Halo UI font
 * (Conduit ITC) is commercial and deliberately not bundled.
 *
 * Two atlases (P2a round 4, user wanted a "second, thicker font for menu
 * headers"): body = SemiBold for HUD/menu text, header = ExtraBold for menu
 * titles. Same ASCII range and bake size, so one struct+bake+draw path
 * serves both and the public entry points are thin wrappers.
 *
 * If a TTF is missing the draw call falls back to conchars so the HUD
 * never blanks out on devices with an incomplete data dir.
 */

#include "quakedef.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define GLFONT_FILE			"gfx/fonts/archivo-narrow-semibold.ttf"
#define GLFONT_FILE_HEADER	"gfx/fonts/archivo-narrow-extrabold.ttf"
#define GLFONT_BAKEPX		40					/* bake height in px == size 1.0   */
#define GLFONT_FIRST		32
#define GLFONT_LAST			126
#define GLFONT_W			512
#define GLFONT_H			256

typedef struct
{
	const char		*file;
	const char		*label;	/* TexMgr name */
	qboolean		 dbgdump;	/* P2a bring-up: write the 8-bit atlas PGM (body only) */
	stbtt_bakedchar	cdata[GLFONT_LAST - GLFONT_FIRST + 1];
	gltexture_t		*texture;	/* engine-managed atlas via TexMgr, bound with GL_Bind */
	qboolean		 ready;
} glfont_atlas_t;

static glfont_atlas_t glfont_body =
{
	GLFONT_FILE, "glfont_atlas", true
};

static glfont_atlas_t glfont_header =
{
	GLFONT_FILE_HEADER, "glfont_header_atlas", false
};

/*
================
GLFont_Bake -- load one TTF, bake the ASCII atlas, upload via TexMgr.
================
*/
static void GLFont_Bake (glfont_atlas_t *f)
{
	byte			*data;
	unsigned char	*pixels;
	int				used_h;

	f->ready = false;

	data = COM_LoadMallocFile (f->file, NULL);
	if (!data)
	{
		Con_Printf ("GLFont_Init: %s not found, text stays on conchars\n", f->file);
		return;
	}

	pixels = (unsigned char *) malloc (GLFONT_W * GLFONT_H * 4);
	if (!pixels)
	{
		free (data);
		return;
	}

	/* stbtt_BakeFontBitmap is 8 BIT PER PIXEL (it memsets and writes only
	 * pw*ph bytes) -- baking straight into an RGBA buffer aliases the glyph
	 * raster across the first quarter of the texture. Bake into a real
	 * 8-bit raster, then expand to premultiply-ready RGBA: white glyphs,
	 * coverage in alpha, which TexMgr_PREMULTIPLY + our
	 * GL_ONE/ONE_MINUS_SRC_ALPHA blend consume correctly. */
	{
		unsigned char *raster = (unsigned char *) malloc (GLFONT_W * GLFONT_H);
		int px;

		if (!raster)
		{
			free (pixels);
			free (data);
			return;
		}

		used_h = stbtt_BakeFontBitmap (data, 0, (float)GLFONT_BAKEPX, raster,
		                               GLFONT_W, GLFONT_H,
		                               GLFONT_FIRST, GLFONT_LAST - GLFONT_FIRST + 1, f->cdata);
		free (data);

		for (px = 0; px < GLFONT_W * GLFONT_H; px++)
		{
			pixels[px*4 + 0] = 255;
			pixels[px*4 + 1] = 255;
			pixels[px*4 + 2] = 255;
			pixels[px*4 + 3] = raster[px];
		}

		if (f->dbgdump)
		{	/* P2a bring-up: dump the raw 8-bit atlas so the cells are
		     * human-inspectable: white letters on black = healthy. */
			FILE *fp = fopen ("/Users/matt_1/workspace/projects-vita/toolchain/fork-engine-mac/glfont_atlas.pgm", "wb");
			if (fp)
			{
				fprintf (fp, "P5\n%d %d\n255\n", GLFONT_W, GLFONT_H);
				fwrite (raster, 1, GLFONT_W * GLFONT_H, fp);
				fclose (fp);
				Con_Printf ("GLFont: atlas dumped (8-bit raster, used rows=%d)\n", used_h);
			}
		}
		free (raster);
	}
	if (used_h <= 0)		/* stb returns negative row-count on failure */
	{
		Con_Printf ("GLFont_Init: bake failed for %s\n", f->file);
		free (pixels);
		return;
	}

	/* Upload through the texture manager, like notexture/nulltexture are made:
	 * - SRC_RGBA keeps our coverage-in-alpha pixels
	 * - TEXPREF_PREMULTIPLY folds rgb*=alpha so the premultiplied blend
	 *   (GL_ONE, GL_ONE_MINUS_SRC_ALPHA) below matches conchars behavior
	 * - a raw glGenTextures bypassing GL_Bind's state cache sampled whatever
	 *   texture the last GL_Bind left bound -> "rainbow dots" on the HUD
	 * - PERSIST so level loads / tex_reloads never purge the atlas */
	f->texture = TexMgr_LoadImage (NULL, f->label, GLFONT_W, GLFONT_H,
		SRC_RGBA, pixels, "", (src_offset_t) pixels,
		TEXPREF_ALPHA | TEXPREF_LINEAR | TEXPREF_NOPICMIP
		| TEXPREF_PERSIST | TEXPREF_PREMULTIPLY);
	free (pixels);
	if (!f->texture)
	{
		Con_Printf ("GLFont_Init: TexMgr could not upload the baked atlas (%s)\n", f->label);
		return;
	}

	f->ready = true;
}

/*
================
GLFont_Init
================
*/
void GLFont_Init (void)
{
	GLFont_Bake (&glfont_body);
	GLFont_Bake (&glfont_header);
}

/*
================
TTF_AtlasWidth -- advance width of str at the given draw size, for centering.
================
*/
static float TTF_AtlasWidth (glfont_atlas_t *f, const char *str, float size)
{
	float sc, w = 0;

	if (!f->ready)
		return strlen (str) * 8.0f * (size / (float)GLFONT_BAKEPX);

	sc = size / (float)GLFONT_BAKEPX;
	for (; *str; str++)
	{
		if (*str < GLFONT_FIRST || *str > GLFONT_LAST)
			continue;
		w += f->cdata[*str - GLFONT_FIRST].xadvance * sc;
	}
	return w;
}

/*
================
TTF_AtlasDraw -- shared quad renderer for one atlas.

x,y are screen px (top-left of the text line, HUD_SCALE model).
size is the drawn cap height in px (pass design_px * HUD_SCALE()).
Falls back to Draw_ColoredStringScale at a matched visual size.
================
*/
static float TTF_AtlasDraw (glfont_atlas_t *f, int x, int y, const char *str, float r, float g, float b, float a, float size)
{
	float sc, x0, y0, x1, y1, u0, v0, u1, v1;
	int startx = x;

	if (!f->ready)
		return Draw_ColoredStringScale (x, y + (int)(size/8.0f) - 8, str, r, g, b, a, size / 8.0f);

	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_TEXTURE_2D);
	GL_Bind (f->texture);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable (GL_ALPHA_TEST);
	glColor4f (r, g, b, a);

	sc = size / (float)GLFONT_BAKEPX;
	glBegin (GL_QUADS);
	for (; *str; str++)
	{
		if (*str < GLFONT_FIRST || *str > GLFONT_LAST)
			continue;
		{
			stbtt_bakedchar *ch = &f->cdata[*str - GLFONT_FIRST];
			x0 = x + ch->xoff * sc;
			y0 = y + ch->yoff * sc;
			x1 = x + (ch->xoff + (ch->x1 - ch->x0)) * sc;
			y1 = y + (ch->yoff + (ch->y1 - ch->y0)) * sc;
			u0 = (float)ch->x0; v0 = (float)ch->y0;
			u1 = (float)ch->x1; v1 = (float)ch->y1;
			glTexCoord2f (u0/GLFONT_W, v0/GLFONT_H); glVertex2f (x0, y0);
			glTexCoord2f (u1/GLFONT_W, v0/GLFONT_H); glVertex2f (x1, y0);
			glTexCoord2f (u1/GLFONT_W, v1/GLFONT_H); glVertex2f (x1, y1);
			glTexCoord2f (u0/GLFONT_W, v1/GLFONT_H); glVertex2f (x0, y1);
			x += ch->xadvance * sc;
		}
	}
	glEnd ();

	glColor4f (1, 1, 1, 1);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable (GL_ALPHA_TEST);
	glDisable (GL_BLEND);
	return x - startx;
}

/*
================
Draw_TTFWidth -- advance width of str at the given draw size, for centering.
================
*/
float Draw_TTFWidth (const char *str, float size)
{
	return TTF_AtlasWidth (&glfont_body, str, size);
}

/*
================
Draw_TTFString -- halo-style HUD text (body face).

x,y are screen px (top-left of the text line, HUD_SCALE model).
size is the drawn cap height in px (pass design_px * HUD_SCALE()).
Falls back to Draw_ColoredStringScale at a matched visual size.
================
*/
float Draw_TTFString (int x, int y, const char *str, float r, float g, float b, float a, float size)
{
	return TTF_AtlasDraw (&glfont_body, x, y, str, r, g, b, a, size);
}

/* P2a round 4: header face (Archivo Narrow ExtraBold) for menu titles.
 * Same coord model and fallback behavior as the body entry points. */
float Draw_TTFHeaderWidth (const char *str, float size)
{
	return TTF_AtlasWidth (&glfont_header, str, size);
}

float Draw_TTFStringHeader (int x, int y, const char *str, float r, float g, float b, float a, float size)
{
	return TTF_AtlasDraw (&glfont_header, x, y, str, r, g, b, a, size);
}
