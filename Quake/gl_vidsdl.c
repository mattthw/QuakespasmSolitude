/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
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
// gl_vidsdl.c -- SDL GL vid component

#include "quakedef.h"
#include "cfgfile.h"
#include "bgmusic.h"
#include "resource.h"
#ifdef VITA
#include <vitasdk.h>
#endif
#if defined(SDL_FRAMEWORK) || defined(NO_SDL_CONFIG)
#if defined(USE_SDL2)
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif
#else
#include "SDL.h"
#endif

//ericw -- for putting the driver into multithreaded mode
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#define MAX_MODE_LIST	600 //johnfitz -- was 30
#define MAX_BPPS_LIST	5
#define MAX_RATES_LIST	20
#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000

#define DEFAULT_SDL_FLAGS	SDL_OPENGL

#define DEFAULT_REFRESHRATE	60

// FIXME: If we don't re-define this here, there seems to be issues related to softfp vs hardfp
#define SLIDER_RANGE 10
static void M_DrawSlider (int x, int y, float range)
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

typedef struct {
	int			width;
	int			height;
	int			refreshrate;
	int			bpp;
} vmode_t;

static const char *gl_vendor;
static const char *gl_renderer;
static const char *gl_version;
static int gl_version_major;
static int gl_version_minor;
static const char *gl_extensions;

qboolean gl_texture_s3tc, gl_texture_rgtc, gl_texture_bptc, gl_texture_etc2, gl_texture_astc;

extern cvar_t gl_texturemode;
extern cvar_t crosshair;
extern cvar_t pstv_rumble;
extern cvar_t retrotouch;
extern cvar_t	motioncam;
extern cvar_t	motion_horizontal_sensitivity;
extern cvar_t	motion_vertical_sensitivity;
extern cvar_t	r_lerpmodels;
extern cvar_t	r_lerpmoves;
extern cvar_t r_viewmodeloffset;
extern cvar_t scr_showfps;

char *w_pos[] = {"Center", "Right", "Left", "Hidden"};
int w_pos_idx = -1;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;

static qboolean	vid_initialized = false;

static SDL_Cursor	*vid_cursor;
#if defined(USE_SDL2)
static SDL_Window	*draw_context;
static SDL_GLContext	gl_context;
#else
static SDL_Surface	*draw_context;
#endif

static qboolean	vid_locked = false; //johnfitz
static qboolean	vid_changed = false;

static void VID_Menu_Init (void); //johnfitz
static void VID_Menu_f (void); //johnfitz
static void VID_MenuDraw (void);
static void VID_MenuKey (int key);

static void ClearAllStates (void);
static void GL_Init (void);
static void GL_SetupState (void); //johnfitz

viddef_t	vid;				// global video state
modestate_t	modestate = MS_UNINIT;
qboolean	scr_skipupdate;

qboolean gl_mtexable = false;
qboolean gl_texture_env_combine = false; //johnfitz
qboolean gl_texture_env_add = false; //johnfitz
qboolean gl_swap_control = false; //johnfitz
qboolean gl_anisotropy_able = false; //johnfitz
float gl_max_anisotropy; //johnfitz
qboolean gl_texture_NPOT = false; //ericw
qboolean gl_vbo_able = false; //ericw
qboolean gl_glsl_able = false; //ericw
GLint gl_max_texture_units = 0; //ericw
qboolean gl_glsl_gamma_able = false; //ericw
qboolean gl_glsl_alias_able = false; //ericw
int gl_stencilbits;

PFNGLMULTITEXCOORD2FARBPROC GL_MTexCoord2fFunc = NULL; //johnfitz
PFNGLACTIVETEXTUREARBPROC GL_SelectTextureFunc = NULL; //johnfitz
PFNGLCLIENTACTIVETEXTUREARBPROC GL_ClientActiveTextureFunc = NULL; //ericw
PFNGLBINDBUFFERARBPROC GL_BindBufferFunc = NULL; //ericw
PFNGLBUFFERDATAARBPROC GL_BufferDataFunc = NULL; //ericw
PFNGLBUFFERSUBDATAARBPROC GL_BufferSubDataFunc = NULL; //ericw
PFNGLDELETEBUFFERSARBPROC GL_DeleteBuffersFunc = NULL; //ericw
PFNGLGENBUFFERSARBPROC GL_GenBuffersFunc = NULL; //ericw

QS_PFNGLCREATESHADERPROC GL_CreateShaderFunc = NULL; //ericw
QS_PFNGLDELETESHADERPROC GL_DeleteShaderFunc = NULL; //ericw
QS_PFNGLDELETEPROGRAMPROC GL_DeleteProgramFunc = NULL; //ericw
QS_PFNGLSHADERSOURCEPROC GL_ShaderSourceFunc = NULL; //ericw
QS_PFNGLCOMPILESHADERPROC GL_CompileShaderFunc = NULL; //ericw
QS_PFNGLGETSHADERIVPROC GL_GetShaderivFunc = NULL; //ericw
QS_PFNGLGETSHADERINFOLOGPROC GL_GetShaderInfoLogFunc = NULL; //ericw
QS_PFNGLGETPROGRAMIVPROC GL_GetProgramivFunc = NULL; //ericw
QS_PFNGLGETPROGRAMINFOLOGPROC GL_GetProgramInfoLogFunc = NULL; //ericw
QS_PFNGLCREATEPROGRAMPROC GL_CreateProgramFunc = NULL; //ericw
QS_PFNGLATTACHSHADERPROC GL_AttachShaderFunc = NULL; //ericw
QS_PFNGLLINKPROGRAMPROC GL_LinkProgramFunc = NULL; //ericw
QS_PFNGLBINDATTRIBLOCATIONFUNC GL_BindAttribLocationFunc = NULL; //ericw
QS_PFNGLUSEPROGRAMPROC GL_UseProgramFunc = NULL; //ericw
QS_PFNGLGETATTRIBLOCATIONPROC GL_GetAttribLocationFunc = NULL; //ericw
QS_PFNGLVERTEXATTRIBPOINTERPROC GL_VertexAttribPointerFunc = NULL; //ericw
QS_PFNGLENABLEVERTEXATTRIBARRAYPROC GL_EnableVertexAttribArrayFunc = NULL; //ericw
QS_PFNGLDISABLEVERTEXATTRIBARRAYPROC GL_DisableVertexAttribArrayFunc = NULL; //ericw
QS_PFNGLGETUNIFORMLOCATIONPROC GL_GetUniformLocationFunc = NULL; //ericw
QS_PFNGLUNIFORM1IPROC GL_Uniform1iFunc = NULL; //ericw
QS_PFNGLUNIFORM1FPROC GL_Uniform1fFunc = NULL; //ericw
QS_PFNGLUNIFORM3FPROC GL_Uniform3fFunc = NULL; //ericw
QS_PFNGLUNIFORM4FPROC GL_Uniform4fFunc = NULL; //ericw
QS_PFNGLUNIFORM4FVPROC GL_Uniform4fvFunc = NULL; //spike (for iqms)

QS_PFNGLCOMPRESSEDTEXIMAGE2DPROC GL_CompressedTexImage2D = NULL;	//spike

//====================================

//johnfitz -- new cvars
static cvar_t	vid_fullscreen = {"vid_fullscreen", "1", CVAR_ARCHIVE};	// QuakeSpasm, was "1"
static cvar_t	vid_width = {"vid_width", "960", CVAR_ARCHIVE};		// QuakeSpasm, was 640
static cvar_t	vid_height = {"vid_height", "544", CVAR_ARCHIVE};	// QuakeSpasm, was 480
static cvar_t	vid_bpp = {"vid_bpp", "32", CVAR_ARCHIVE};
static cvar_t	vid_refreshrate = {"vid_refreshrate", "60", CVAR_ARCHIVE};
static cvar_t	vid_vsync = {"vid_vsync", "1", CVAR_ARCHIVE};
static cvar_t	vid_fsaa = {"vid_fsaa", "0", CVAR_ARCHIVE}; // QuakeSpasm
static cvar_t	vid_desktopfullscreen = {"vid_desktopfullscreen", "1", CVAR_ARCHIVE}; // QuakeSpasm
static cvar_t	vid_borderless = {"vid_borderless", "0", CVAR_ARCHIVE}; // QuakeSpasm
//johnfitz

cvar_t		vid_gamma = {"gamma", "1", CVAR_ARCHIVE}; //johnfitz -- moved here from view.c
cvar_t		vid_contrast = {"contrast", "1", CVAR_ARCHIVE}; //QuakeSpasm, MarkV

//==========================================================================
//
//  HARDWARE GAMMA -- johnfitz
//
//==========================================================================

#define	USE_GAMMA_RAMPS			0

#if USE_GAMMA_RAMPS
static unsigned short vid_gamma_red[256];
static unsigned short vid_gamma_green[256];
static unsigned short vid_gamma_blue[256];

static unsigned short vid_sysgamma_red[256];
static unsigned short vid_sysgamma_green[256];
static unsigned short vid_sysgamma_blue[256];
#endif

static qboolean	gammaworks = false;	// whether hw-gamma works
static int fsaa;

/*
================
VID_Gamma_SetGamma -- apply gamma correction
================
*/
static void VID_Gamma_SetGamma (void)
{
	if (gl_glsl_gamma_able)
		return;

	if (draw_context && gammaworks)
	{
		float	value;

		if (vid_gamma.value > (1.0f / GAMMA_MAX))
			value = 1.0f / vid_gamma.value;
		else
			value = GAMMA_MAX;

#if defined(USE_SDL2)
# if USE_GAMMA_RAMPS
		if (SDL_SetWindowGammaRamp(draw_context, vid_gamma_red, vid_gamma_green, vid_gamma_blue) != 0)
			Con_Printf ("VID_Gamma_SetGamma: failed on SDL_SetWindowGammaRamp\n");
# else
		if (SDL_SetWindowBrightness(draw_context, value) != 0)
			Con_Printf ("VID_Gamma_SetGamma: failed on SDL_SetWindowBrightness\n");
# endif
#else /* USE_SDL2 */
# if USE_GAMMA_RAMPS
		if (SDL_SetGammaRamp(vid_gamma_red, vid_gamma_green, vid_gamma_blue) == -1)
			Con_Printf ("VID_Gamma_SetGamma: failed on SDL_SetGammaRamp\n");
# else
		if (SDL_SetGamma(value,value,value) == -1)
			Con_Printf ("VID_Gamma_SetGamma: failed on SDL_SetGamma\n");
# endif
#endif /* USE_SDL2 */
	}
}

/*
================
VID_Gamma_Restore -- restore system gamma
================
*/
static void VID_Gamma_Restore (void)
{
	if (gl_glsl_gamma_able)
		return;

	if (draw_context && gammaworks)
	{
#if defined(USE_SDL2)
# if USE_GAMMA_RAMPS
		if (SDL_SetWindowGammaRamp(draw_context, vid_sysgamma_red, vid_sysgamma_green, vid_sysgamma_blue) != 0)
			Con_Printf ("VID_Gamma_Restore: failed on SDL_SetWindowGammaRamp\n");
# else
		if (SDL_SetWindowBrightness(draw_context, 1) != 0)
			Con_Printf ("VID_Gamma_Restore: failed on SDL_SetWindowBrightness\n");
# endif
#else /* USE_SDL2 */
# if USE_GAMMA_RAMPS
		if (SDL_SetGammaRamp(vid_sysgamma_red, vid_sysgamma_green, vid_sysgamma_blue) == -1)
			Con_Printf ("VID_Gamma_Restore: failed on SDL_SetGammaRamp\n");
# else
		if (SDL_SetGamma(1, 1, 1) == -1)
			Con_Printf ("VID_Gamma_Restore: failed on SDL_SetGamma\n");
# endif
#endif /* USE_SDL2 */
	}
}

/*
================
VID_Gamma_Shutdown -- called on exit
================
*/
static void VID_Gamma_Shutdown (void)
{
	VID_Gamma_Restore ();
}

/*
================
VID_Gamma_f -- callback when the cvar changes
================
*/
static void VID_Gamma_f (cvar_t *var)
{
	if (gl_glsl_gamma_able)
		return;

#if USE_GAMMA_RAMPS
	int i;

	for (i = 0; i < 256; i++)
	{
		vid_gamma_red[i] =
			CLAMP(0, (int) ((255 * pow((i + 0.5)/255.5, vid_gamma.value) + 0.5) * vid_contrast.value), 255) << 8;
		vid_gamma_green[i] = vid_gamma_red[i];
		vid_gamma_blue[i] = vid_gamma_red[i];
	}
#endif
	VID_Gamma_SetGamma ();
}

/*
================
VID_Gamma_Init -- call on init
================
*/
static void VID_Gamma_Init (void)
{
	Cvar_RegisterVariable (&vid_gamma);
	Cvar_RegisterVariable (&vid_contrast);
	Cvar_SetCallback (&vid_gamma, VID_Gamma_f);
	Cvar_SetCallback (&vid_contrast, VID_Gamma_f);

	if (gl_glsl_gamma_able)
		return;

#if defined(USE_SDL2)
# if USE_GAMMA_RAMPS
	gammaworks	= (SDL_GetWindowGammaRamp(draw_context, vid_sysgamma_red, vid_sysgamma_green, vid_sysgamma_blue) == 0);
	if (gammaworks)
	    gammaworks	= (SDL_SetWindowGammaRamp(draw_context, vid_sysgamma_red, vid_sysgamma_green, vid_sysgamma_blue) == 0);
# else
	gammaworks	= (SDL_SetWindowBrightness(draw_context, 1) == 0);
# endif
#else /* USE_SDL2 */
# if USE_GAMMA_RAMPS
	gammaworks	= (SDL_GetGammaRamp(vid_sysgamma_red, vid_sysgamma_green, vid_sysgamma_blue) == 0);
	if (gammaworks)
	    gammaworks	= (SDL_SetGammaRamp(vid_sysgamma_red, vid_sysgamma_green, vid_sysgamma_blue) == 0);
# else
	gammaworks	= (SDL_SetGamma(1, 1, 1) == 0);
# endif
#endif /* USE_SDL2 */

	if (!gammaworks)
		Con_SafePrintf("gamma adjustment not available\n");
}

/*
======================
VID_GetCurrentWidth
======================
*/
static int VID_GetCurrentWidth (void)
{
#ifdef VITA
	return 960;
#else
#if defined(USE_SDL2)
	int w = 0, h = 0;
	SDL_GetWindowSize(draw_context, &w, &h);
	return w;
#else
	return draw_context->w;
#endif
#endif
}

/*
=======================
VID_GetCurrentHeight
=======================
*/
static int VID_GetCurrentHeight (void)
{
#ifdef VITA
	return 544;
#else
#if defined(USE_SDL2)
	int w = 0, h = 0;
	SDL_GetWindowSize(draw_context, &w, &h);
	return h;
#else
	return draw_context->h;
#endif
#endif
}

/*
====================
VID_GetCurrentRefreshRate
====================
*/
static int VID_GetCurrentRefreshRate (void)
{
#if defined(USE_SDL2)
	SDL_DisplayMode mode;
	int current_display;
	
	current_display = SDL_GetWindowDisplayIndex(draw_context);
	
	if (0 != SDL_GetCurrentDisplayMode(current_display, &mode))
		return DEFAULT_REFRESHRATE;
	
	return mode.refresh_rate;
#else
	// SDL1.2 doesn't support refresh rates
	return DEFAULT_REFRESHRATE;
#endif
}


/*
====================
VID_GetCurrentBPP
====================
*/
static int VID_GetCurrentBPP (void)
{
#ifdef VITA
	return 32;
#else
#if defined(USE_SDL2)
	const Uint32 pixelFormat = SDL_GetWindowPixelFormat(draw_context);
	return SDL_BITSPERPIXEL(pixelFormat);
#else
	return draw_context->format->BitsPerPixel;
#endif
#endif
}

/*
====================
VID_GetFullscreen
 
returns true if we are in regular fullscreen or "desktop fullscren"
====================
*/
static qboolean VID_GetFullscreen (void)
{
#if defined(USE_SDL2)
	return (SDL_GetWindowFlags(draw_context) & SDL_WINDOW_FULLSCREEN) != 0;
#else
	return (draw_context->flags & SDL_FULLSCREEN) != 0;
#endif
}

/*
====================
VID_GetDesktopFullscreen
 
returns true if we are specifically in "desktop fullscreen" mode
====================
*/
static qboolean VID_GetDesktopFullscreen (void)
{
#ifdef VITA
	return true;
#else
#if defined(USE_SDL2)
	return (SDL_GetWindowFlags(draw_context) & SDL_WINDOW_FULLSCREEN_DESKTOP) == SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
	return false;
#endif
#endif
}

/*
====================
VID_GetVSync
====================
*/
static qboolean VID_GetVSync (void)
{
#ifdef VITA
	return true;
#elif defined(USE_SDL2)
	return SDL_GL_GetSwapInterval() == 1;
#else
	int swap_control;
	if (SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &swap_control) == 0)
		return swap_control > 0;
	return false;
#endif
}

/*
====================
VID_GetWindow

used by pl_win.c
====================
*/
void *VID_GetWindow (void)
{
#if defined(USE_SDL2)
	return draw_context;
#else
	return NULL;
#endif
}

/*
====================
VID_HasMouseOrInputFocus
====================
*/
qboolean VID_HasMouseOrInputFocus (void)
{
#if defined(USE_SDL2)
	return (SDL_GetWindowFlags(draw_context) & (SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS)) != 0;
#else
	return (SDL_GetAppState() & (SDL_APPMOUSEFOCUS | SDL_APPINPUTFOCUS)) != 0;
#endif
}

/*
====================
VID_IsMinimized
====================
*/
qboolean VID_IsMinimized (void)
{
#if defined(USE_SDL2)
	return !(SDL_GetWindowFlags(draw_context) & SDL_WINDOW_SHOWN);
#else
	/* SDL_APPACTIVE in SDL 1.x means "not minimized" */
	return !(SDL_GetAppState() & SDL_APPACTIVE);
#endif
}

#if defined(USE_SDL2)
/*
================
VID_SDL2_GetDisplayMode

Returns a pointer to a statically allocated SDL_DisplayMode structure
if there is one with the requested params on the default display.
Otherwise returns NULL.

This is passed to SDL_SetWindowDisplayMode to specify a pixel format
with the requested bpp. If we didn't care about bpp we could just pass NULL.
================
*/
static SDL_DisplayMode *VID_SDL2_GetDisplayMode(int width, int height, int refreshrate, int bpp)
{
	static SDL_DisplayMode mode;
	const int sdlmodes = SDL_GetNumDisplayModes(0);
	int i;

	for (i = 0; i < sdlmodes; i++)
	{
		if (SDL_GetDisplayMode(0, i, &mode) != 0)
			continue;
		
		if (mode.w == width && mode.h == height
			&& SDL_BITSPERPIXEL(mode.format) == bpp
			&& mode.refresh_rate == refreshrate)
		{
			return &mode;
		}
	}
	return NULL;
}
#endif /* USE_SDL2 */

/*
================
VID_ValidMode
================
*/
static qboolean VID_ValidMode (int width, int height, int refreshrate, int bpp, qboolean fullscreen)
{
// ignore width / height / bpp if vid_desktopfullscreen is enabled
	if (fullscreen && vid_desktopfullscreen.value)
		return true;

	if (width < 320)
		return false;

	if (height < 200)
		return false;
	
#ifdef VITA
	return true;
#endif

#if defined(USE_SDL2)
	if (fullscreen && VID_SDL2_GetDisplayMode(width, height, refreshrate, bpp) == NULL)
		bpp = 0;
#else
	{
		Uint32 flags = DEFAULT_SDL_FLAGS;
		if (fullscreen)
			flags |= SDL_FULLSCREEN;

		bpp = SDL_VideoModeOK(width, height, bpp, flags);
	}
#endif

	switch (bpp)
	{
	case 16:
	case 24:
	case 32:
		break;
	default:
		return false;
	}

	return true;
}

/*
================
VID_SetMode
================
*/
static qboolean VID_SetMode (int width, int height, int refreshrate, int bpp, qboolean fullscreen)
{
	int		temp;
	Uint32	flags;
	char		caption[50];
	int		depthbits, stencilbits;
	int		fsaa_obtained;
#if defined(USE_SDL2)
	int		previous_display;
#endif

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	CDAudio_Pause ();
	BGM_Pause ();

	/* z-buffer depth */
	if (bpp == 16)
	{
		depthbits = 16;
		stencilbits = 0;
	}
	else
	{
		depthbits = 24;
		stencilbits = 8;
	}
#ifndef VITA
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthbits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilbits);

	/* fsaa */
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, fsaa > 0 ? 1 : 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa);
#endif
	q_snprintf(caption, sizeof(caption), ENGINE_NAME_AND_VER);

#if defined(USE_SDL2)
#ifndef VITA
	/* Create the window if needed, hidden */
	if (!draw_context)
	{
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;

		if (vid_borderless.value)
			flags |= SDL_WINDOW_BORDERLESS;
		else if (!fullscreen)
			flags |= SDL_WINDOW_RESIZABLE;
		
		draw_context = SDL_CreateWindow (caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		if (!draw_context) { // scale back fsaa
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			draw_context = SDL_CreateWindow (caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		}
		if (!draw_context) { // scale back SDL_GL_DEPTH_SIZE
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			draw_context = SDL_CreateWindow (caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		}
		if (!draw_context) { // scale back SDL_GL_STENCIL_SIZE
			SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
			draw_context = SDL_CreateWindow (caption, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		}
		if (!draw_context)
			Sys_Error ("Couldn't create window");

		previous_display = -1;
	}
	else
	{
		previous_display = SDL_GetWindowDisplayIndex(draw_context);
	}
#else
	draw_context = SDL_CreateWindow("dummy", 0, 0, 960, 544, SDL_WINDOW_FULLSCREEN);
#endif
	/* Ensure the window is not fullscreen */
	if (VID_GetFullscreen ())
	{
		if (SDL_SetWindowFullscreen (draw_context, 0) != 0)
			Sys_Error("Couldn't set fullscreen state mode");
	}
#ifndef VITA
	/* Set window size and display mode */
	SDL_SetWindowSize (draw_context, width, height);
	if (previous_display >= 0)
		SDL_SetWindowPosition (draw_context, SDL_WINDOWPOS_CENTERED_DISPLAY(previous_display), SDL_WINDOWPOS_CENTERED_DISPLAY(previous_display));
	else
		SDL_SetWindowPosition(draw_context, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_SetWindowDisplayMode (draw_context, VID_SDL2_GetDisplayMode(width, height, refreshrate, bpp));
	SDL_SetWindowBordered (draw_context, vid_borderless.value ? SDL_FALSE : SDL_TRUE);

	/* Make window fullscreen if needed, and show the window */

	if (fullscreen) {
		Uint32 flags = vid_desktopfullscreen.value ?
			SDL_WINDOW_FULLSCREEN_DESKTOP :
			SDL_WINDOW_FULLSCREEN;
		if (SDL_SetWindowFullscreen (draw_context, flags) != 0)
			Sys_Error ("Couldn't set fullscreen state mode");
	}

	SDL_ShowWindow (draw_context);

	/* Create GL context if needed */
	if (!gl_context) {
		gl_context = SDL_GL_CreateContext(draw_context);
		if (!gl_context)
			Sys_Error("Couldn't create GL context");
	}
#endif
	gl_swap_control = true;
#ifndef VITA
	if (SDL_GL_SetSwapInterval ((vid_vsync.value) ? 1 : 0) == -1)
		gl_swap_control = false;
#endif
#else /* !defined(USE_SDL2) */

	flags = DEFAULT_SDL_FLAGS;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;
	if (vid_borderless.value)
		flags |= SDL_NOFRAME;
	
	gl_swap_control = true;
	if (SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (vid_vsync.value) ? 1 : 0) == -1)
		gl_swap_control = false;

	bpp = SDL_VideoModeOK(width, height, bpp, flags);

	draw_context = SDL_SetVideoMode(width, height, bpp, flags);
	if (!draw_context) { // scale back fsaa
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		draw_context = SDL_SetVideoMode(width, height, bpp, flags);
	}
	if (!draw_context) { // scale back SDL_GL_DEPTH_SIZE
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		draw_context = SDL_SetVideoMode(width, height, bpp, flags);
	}
	if (!draw_context) { // scale back SDL_GL_STENCIL_SIZE
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
		draw_context = SDL_SetVideoMode(width, height, bpp, flags);
		if (!draw_context)
			Sys_Error ("Couldn't set video mode");
	}

	SDL_WM_SetCaption(caption, caption);
#endif /* !defined(USE_SDL2) */

	vid.width = VID_GetCurrentWidth();
	vid.height = VID_GetCurrentHeight();
	vid.conwidth = vid.width & 0xFFFFFFF8;
	vid.conheight = vid.conwidth * vid.height / vid.width;
	vid.numpages = 2;

#ifdef VITA
	depthbits = 32;
	gl_stencilbits = 8;
#else
// read the obtained z-buffer depth
	if (SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthbits) == -1)
		depthbits = 0;

// read obtained fsaa samples
	if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fsaa_obtained) == -1)
		fsaa_obtained = 0;

// read stencil bits
	if (SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &gl_stencilbits) == -1)
		gl_stencilbits = 0;
#endif
	modestate = VID_GetFullscreen() ? MS_FULLSCREEN : MS_WINDOWED;

	CDAudio_Resume ();
	BGM_Resume ();
	scr_disabled_for_loading = temp;

// fix the leftover Alt from any Alt-Tab or the like that switched us away
	ClearAllStates ();

	Con_SafePrintf ("Video mode %dx%dx%d %dHz (%d-bit z-buffer, %dx FSAA) initialized\n",
				VID_GetCurrentWidth(),
				VID_GetCurrentHeight(),
				VID_GetCurrentBPP(),
				VID_GetCurrentRefreshRate(),
				depthbits,
				fsaa_obtained);

	vid.recalc_refdef = 1;

// no pending changes
	vid_changed = false;

	return true;
}

/*
===================
VID_Changed_f -- kristian -- notify us that a value has changed that requires a vid_restart
===================
*/
static void VID_Changed_f (cvar_t *var)
{
	vid_changed = true;
	vglWaitVblankStart((int)vid_vsync.value);
}

/*
===================
VID_Restart -- johnfitz -- change video modes on the fly
===================
*/
static void VID_Restart (void)
{
	int width, height, refreshrate, bpp;
	qboolean fullscreen;

	if (vid_locked || !vid_changed)
		return;

	width = (int)vid_width.value;
	height = (int)vid_height.value;
	refreshrate = (int)vid_refreshrate.value;
	bpp = (int)vid_bpp.value;
	fullscreen = vid_fullscreen.value ? true : false;

//
// validate new mode
//
	if (!VID_ValidMode (width, height, refreshrate, bpp, fullscreen))
	{
		Con_Printf ("%dx%dx%d %dHz %s is not a valid mode\n",
				width, height, bpp, refreshrate, fullscreen? "fullscreen" : "windowed");
		return;
	}
	
// ericw -- OS X, SDL1: textures, VBO's invalid after mode change
//          OS X, SDL2: still valid after mode change
// To handle both cases, delete all GL objects (textures, VBO, GLSL) now.
// We must not interleave deleting the old objects with creating new ones, because
// one of the new objects could be given the same ID as an invalid handle
// which is later deleted.

	TexMgr_DeleteTextureObjects ();
	GLSLGamma_DeleteTexture ();
	R_ScaleView_DeleteTexture ();
	R_DeleteShaders ();
	GL_DeleteBModelVertexBuffer ();
	GLMesh_DeleteVertexBuffers ();

//
// set new mode
//
	VID_SetMode (width, height, refreshrate, bpp, fullscreen);

	GL_Init ();
	TexMgr_ReloadImages ();
	GL_BuildBModelVertexBuffer ();
	GLMesh_LoadVertexBuffers ();
	GL_SetupState ();
	Fog_SetupState ();

	//warpimages needs to be recalculated
	TexMgr_RecalcWarpImageSize ();

	//conwidth and conheight need to be recalculated
	vid.conwidth = (scr_conwidth.value > 0) ? (int)scr_conwidth.value : (scr_conscale.value > 0) ? (int)(vid.width/scr_conscale.value) : vid.width;
	vid.conwidth = CLAMP (320, vid.conwidth, vid.width);
	vid.conwidth &= 0xFFFFFFF8;
	vid.conheight = vid.conwidth * vid.height / vid.width;
//
// keep cvars in line with actual mode
//
	VID_SyncCvars();
//
// update mouse grab
//
	IN_UpdateGrabs();
}

/*
================
VID_Test -- johnfitz -- like vid_restart, but asks for confirmation after switching modes
================
*/
static void VID_Test (void)
{
	int old_width, old_height, old_refreshrate, old_bpp, old_fullscreen;

	if (vid_locked || !vid_changed)
		return;
//
// now try the switch
//
	old_width = VID_GetCurrentWidth();
	old_height = VID_GetCurrentHeight();
	old_refreshrate = VID_GetCurrentRefreshRate();
	old_bpp = VID_GetCurrentBPP();
	old_fullscreen = VID_GetFullscreen() ? true : false;

	VID_Restart ();

	//pop up confirmation dialoge
	if (!SCR_ModalMessage("Would you like to keep this\nvideo mode? (y/n)\n", 5.0f))
	{
		//revert cvars and mode
		Cvar_SetValueQuick (&vid_width, old_width);
		Cvar_SetValueQuick (&vid_height, old_height);
		Cvar_SetValueQuick (&vid_refreshrate, old_refreshrate);
		Cvar_SetValueQuick (&vid_bpp, old_bpp);
		Cvar_SetQuick (&vid_fullscreen, old_fullscreen ? "1" : "0");
		VID_Restart ();
	}
}

/*
================
VID_Unlock -- johnfitz
================
*/
static void VID_Unlock (void)
{
	vid_locked = false;
	VID_SyncCvars();
}

/*
================
VID_Lock -- ericw

Subsequent changes to vid_* mode settings, and vid_restart commands, will
be ignored until the "vid_unlock" command is run.

Used when changing gamedirs so the current settings override what was saved
in the config.cfg.
================
*/
void VID_Lock (void)
{
	vid_locked = true;
}

//==============================================================================
//
//	OPENGL STUFF
//
//==============================================================================

/*
===============
GL_MakeNiceExtensionsList -- johnfitz
===============
*/
static void GL_PrintNiceExtensionsList (const char *in)
{
	char *copy, *token;
	if (!in) return Con_SafePrintf("(none)");

	copy = (char *) Z_Strdup(in);
	for (token = strtok(copy, " "); token; token = strtok(NULL, " "))
		Con_SafePrintf("\n   %s", token);
	Z_Free (copy);
}

/*
===============
GL_Info_f -- johnfitz
===============
*/
static void GL_Info_f (void)
{
	Con_SafePrintf ("GL_VENDOR: %s\n", gl_vendor);
	Con_SafePrintf ("GL_RENDERER: %s\n", gl_renderer);
	Con_SafePrintf ("GL_VERSION: %s\n", gl_version);
	Con_SafePrintf ("GL_EXTENSIONS: ");
	GL_PrintNiceExtensionsList(gl_extensions);
	Con_SafePrintf ("\n");
}

/*
===============
GL_CheckExtensions
===============
*/
static qboolean GL_ParseExtensionList (const char *list, const char *name)
{
	const char	*start;
	const char	*where, *terminator;

	if (!list || !name || !*name)
		return false;
	if (strchr(name, ' ') != NULL)
		return false;	// extension names must not have spaces

	start = list;
	while (1) {
		where = strstr (start, name);
		if (!where)
			break;
		terminator = where + strlen (name);
		if (where == start || where[-1] == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return true;
		start = terminator;
	}
	return false;
}

static void GL_CheckExtensions (void)
{
	int swap_control;

	// ARB_vertex_buffer_object
	//
	if (COM_CheckParm("-novbo"))
		Con_Warning ("Vertex buffer objects disabled at command line\n");
	else if (gl_version_major < 1 || (gl_version_major == 1 && gl_version_minor < 5))
		Con_Warning ("OpenGL version < 1.5, skipping ARB_vertex_buffer_object check\n");
	else
	{
		GL_BindBufferFunc = (PFNGLBINDBUFFERARBPROC) vglGetProcAddress("glBindBufferARB");
		GL_BufferDataFunc = (PFNGLBUFFERDATAARBPROC) vglGetProcAddress("glBufferDataARB");
		GL_BufferSubDataFunc = (PFNGLBUFFERSUBDATAARBPROC) vglGetProcAddress("glBufferSubDataARB");
		GL_DeleteBuffersFunc = (PFNGLDELETEBUFFERSARBPROC) vglGetProcAddress("glDeleteBuffersARB");
		GL_GenBuffersFunc = (PFNGLGENBUFFERSARBPROC) vglGetProcAddress("glGenBuffersARB");
		if (GL_BindBufferFunc && GL_BufferDataFunc && GL_BufferSubDataFunc && GL_DeleteBuffersFunc && GL_GenBuffersFunc)
		{
			Con_Printf("FOUND: ARB_vertex_buffer_object\n");
			gl_vbo_able = true;
		}
		else
		{
			Con_Warning ("ARB_vertex_buffer_object not available\n");
		}
	}

	// multitexture
	//
	if (COM_CheckParm("-nomtex"))
		Con_Warning ("Mutitexture disabled at command line\n");
	else if (GL_ParseExtensionList(gl_extensions, "GL_ARB_multitexture"))
	{
		GL_MTexCoord2fFunc = (PFNGLMULTITEXCOORD2FARBPROC) vglGetProcAddress("glMultiTexCoord2fARB");
		GL_SelectTextureFunc = (PFNGLACTIVETEXTUREARBPROC) vglGetProcAddress("glActiveTextureARB");
		GL_ClientActiveTextureFunc = (PFNGLCLIENTACTIVETEXTUREARBPROC) vglGetProcAddress("glClientActiveTextureARB");
		if (GL_MTexCoord2fFunc && GL_SelectTextureFunc && GL_ClientActiveTextureFunc)
		{
			Con_Printf("FOUND: ARB_multitexture\n");
			gl_mtexable = true;
#ifdef VITA // Number of supported texunits is different between ffp and shaders pipeline in vitaGL
			gl_max_texture_units = 4;
#else
			glGetIntegerv(GL_MAX_TEXTURE_UNITS, &gl_max_texture_units);
			Con_Printf("GL_MAX_TEXTURE_UNITS: %d\n", (int)gl_max_texture_units);
#endif
		}
		else
		{
			Con_Warning ("Couldn't link to multitexture functions\n");
		}
	}
	else
	{
		Con_Warning ("multitexture not supported (extension not found)\n");
	}

	// texture_env_combine
	//
	if (COM_CheckParm("-nocombine"))
		Con_Warning ("texture_env_combine disabled at command line\n");
	else if (GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_env_combine"))
	{
		Con_Printf("FOUND: ARB_texture_env_combine\n");
		gl_texture_env_combine = true;
	}
	else if (GL_ParseExtensionList(gl_extensions, "GL_EXT_texture_env_combine"))
	{
		Con_Printf("FOUND: EXT_texture_env_combine\n");
		gl_texture_env_combine = true;
	}
	else
	{
		Con_Warning ("texture_env_combine not supported\n");
	}

	// texture_env_add
	//
	if (COM_CheckParm("-noadd"))
		Con_Warning ("texture_env_add disabled at command line\n");
	else if (GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_env_add"))
	{
		Con_Printf("FOUND: ARB_texture_env_add\n");
		gl_texture_env_add = true;
	}
	else if (GL_ParseExtensionList(gl_extensions, "GL_EXT_texture_env_add"))
	{
		Con_Printf("FOUND: EXT_texture_env_add\n");
		gl_texture_env_add = true;
	}
	else
	{
		Con_Warning ("texture_env_add not supported\n");
	}

	// swap control
	//
#ifndef VITA
	if (!gl_swap_control)
	{
#if defined(USE_SDL2)
		Con_Warning ("vertical sync not supported (SDL_GL_SetSwapInterval failed)\n");
#else
		Con_Warning ("vertical sync not supported (SDL_GL_SetAttribute failed)\n");
#endif
	}
#if defined(USE_SDL2)
	else if ((swap_control = SDL_GL_GetSwapInterval()) == -1)
#else
	else if (SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &swap_control) == -1)
#endif
	{
		gl_swap_control = false;
#if defined(USE_SDL2)
		Con_Warning ("vertical sync not supported (SDL_GL_GetSwapInterval failed)\n");
#else
		Con_Warning ("vertical sync not supported (SDL_GL_GetAttribute failed)\n");
#endif
	}
	else if ((vid_vsync.value && swap_control != 1) || (!vid_vsync.value && swap_control != 0))
	{
		gl_swap_control = false;
		Con_Warning ("vertical sync not supported (swap_control doesn't match vid_vsync)\n");
	}
	else
	{
#if defined(USE_SDL2)
		Con_Printf("FOUND: SDL_GL_SetSwapInterval\n");
#else
		Con_Printf("FOUND: SDL_GL_SWAP_CONTROL\n");
#endif
	}
#endif
	// anisotropic filtering
	//
	if (GL_ParseExtensionList(gl_extensions, "GL_EXT_texture_filter_anisotropic"))
	{
#ifndef VITA
		float test1,test2;
		GLuint tex;

		// test to make sure we really have control over it
		// 1.0 and 2.0 should always be legal values
		glGenTextures(1, &tex);
		glBindTexture (GL_TEXTURE_2D, tex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
		glGetTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &test1);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
		glGetTexParameterfv (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &test2);
		glDeleteTextures(1, &tex);

		if (test1 == 1 && test2 == 2)
		{
			Con_Printf("FOUND: EXT_texture_filter_anisotropic\n");
			gl_anisotropy_able = true;
		}
		else
		{
			Con_Warning ("anisotropic filtering locked by driver. Current driver setting is %f\n", test1);
		}

		//get max value either way, so the menu and stuff know it
		glGetFloatv (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_anisotropy);
		if (gl_max_anisotropy < 2)
		{
			gl_anisotropy_able = false;
			gl_max_anisotropy = 1;
			Con_Warning ("anisotropic filtering broken: disabled\n");
		}
#endif
	}
	else
	{
		gl_max_anisotropy = 1;
		Con_Warning ("texture_filter_anisotropic not supported\n");
	}

	// texture_non_power_of_two
	//
	if (COM_CheckParm("-notexturenpot"))
		Con_Warning ("texture_non_power_of_two disabled at command line\n");
	else if (GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_non_power_of_two"))
	{
		Con_Printf("FOUND: ARB_texture_non_power_of_two\n");
		gl_texture_NPOT = true;
	}
	else
	{
		Con_Warning ("texture_non_power_of_two not supported\n");
	}

	GL_CompressedTexImage2D = (gl_version_major>=2||(gl_version_major==1&&gl_version_major>=3))?(QS_PFNGLCOMPRESSEDTEXIMAGE2DPROC) vglGetProcAddress("glCompressedTexImage2D"):NULL;
	gl_texture_s3tc = GL_CompressedTexImage2D && (																			   GL_ParseExtensionList(gl_extensions, "GL_EXT_texture_compression_s3tc"));
	gl_texture_rgtc = GL_CompressedTexImage2D && (gl_version_major >= 3													|| GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_compression_rgtc"));
	gl_texture_bptc = GL_CompressedTexImage2D && (gl_version_major > 4 || (gl_version_major == 4 && gl_version_minor >= 2) || GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_compression_bptc"));
	gl_texture_etc2 = GL_CompressedTexImage2D && (gl_version_major > 4 || (gl_version_major == 4 && gl_version_minor >= 3) || GL_ParseExtensionList(gl_extensions, "GL_ARB_ES3_compatibility"));
	gl_texture_astc = GL_CompressedTexImage2D && (gl_version_major > 4 || (gl_version_major == 4 && gl_version_minor >= 3) || GL_ParseExtensionList(gl_extensions, "GL_ARB_ES3_2_compatibility") || GL_ParseExtensionList(gl_extensions, "GL_KHR_texture_compression_astc_ldr"));
	
	// GLSL
	//
	if (COM_CheckParm("-noglsl"))
		Con_Warning ("GLSL disabled at command line\n");
	else if (gl_version_major >= 2)
	{
		GL_CreateShaderFunc = (QS_PFNGLCREATESHADERPROC) vglGetProcAddress("glCreateShader");
		GL_DeleteShaderFunc = (QS_PFNGLDELETESHADERPROC) vglGetProcAddress("glDeleteShader");
		GL_DeleteProgramFunc = (QS_PFNGLDELETEPROGRAMPROC) vglGetProcAddress("glDeleteProgram");
		GL_ShaderSourceFunc = (QS_PFNGLSHADERSOURCEPROC) vglGetProcAddress("glShaderSource");
		GL_CompileShaderFunc = (QS_PFNGLCOMPILESHADERPROC) vglGetProcAddress("glCompileShader");
		GL_GetShaderivFunc = (QS_PFNGLGETSHADERIVPROC) vglGetProcAddress("glGetShaderiv");
		GL_GetShaderInfoLogFunc = (QS_PFNGLGETSHADERINFOLOGPROC) vglGetProcAddress("glGetShaderInfoLog");
		GL_GetProgramivFunc = (QS_PFNGLGETPROGRAMIVPROC) vglGetProcAddress("glGetProgramiv");
		GL_GetProgramInfoLogFunc = (QS_PFNGLGETPROGRAMINFOLOGPROC) vglGetProcAddress("glGetProgramInfoLog");
		GL_CreateProgramFunc = (QS_PFNGLCREATEPROGRAMPROC) vglGetProcAddress("glCreateProgram");
		GL_AttachShaderFunc = (QS_PFNGLATTACHSHADERPROC) vglGetProcAddress("glAttachShader");
		GL_LinkProgramFunc = (QS_PFNGLLINKPROGRAMPROC) vglGetProcAddress("glLinkProgram");
		GL_BindAttribLocationFunc = (QS_PFNGLBINDATTRIBLOCATIONFUNC) vglGetProcAddress("glBindAttribLocation");
		GL_UseProgramFunc = (QS_PFNGLUSEPROGRAMPROC) vglGetProcAddress("glUseProgram");
		GL_GetAttribLocationFunc = (QS_PFNGLGETATTRIBLOCATIONPROC) vglGetProcAddress("glGetAttribLocation");
		GL_VertexAttribPointerFunc = (QS_PFNGLVERTEXATTRIBPOINTERPROC) vglGetProcAddress("glVertexAttribPointer");
		GL_EnableVertexAttribArrayFunc = (QS_PFNGLENABLEVERTEXATTRIBARRAYPROC) vglGetProcAddress("glEnableVertexAttribArray");
		GL_DisableVertexAttribArrayFunc = (QS_PFNGLDISABLEVERTEXATTRIBARRAYPROC) vglGetProcAddress("glDisableVertexAttribArray");
		GL_GetUniformLocationFunc = (QS_PFNGLGETUNIFORMLOCATIONPROC) vglGetProcAddress("glGetUniformLocation");
		GL_Uniform1iFunc = (QS_PFNGLUNIFORM1IPROC) vglGetProcAddress("glUniform1i");
		GL_Uniform1fFunc = (QS_PFNGLUNIFORM1FPROC) vglGetProcAddress("glUniform1f");
		GL_Uniform3fFunc = (QS_PFNGLUNIFORM3FPROC) vglGetProcAddress("glUniform3f");
		GL_Uniform4fFunc = (QS_PFNGLUNIFORM4FPROC) vglGetProcAddress("glUniform4f");
		GL_Uniform4fvFunc = (QS_PFNGLUNIFORM4FVPROC) vglGetProcAddress("glUniform4fv");

		if (GL_CreateShaderFunc &&
			GL_DeleteShaderFunc &&
			GL_DeleteProgramFunc &&
			GL_ShaderSourceFunc &&
			GL_CompileShaderFunc &&
			GL_GetShaderivFunc &&
			GL_GetShaderInfoLogFunc &&
			GL_GetProgramivFunc &&
			GL_GetProgramInfoLogFunc &&
			GL_CreateProgramFunc &&
			GL_AttachShaderFunc &&
			GL_LinkProgramFunc &&
			GL_BindAttribLocationFunc &&
			GL_UseProgramFunc &&
			GL_GetAttribLocationFunc &&
			GL_VertexAttribPointerFunc &&
			GL_EnableVertexAttribArrayFunc &&
			GL_DisableVertexAttribArrayFunc &&
			GL_GetUniformLocationFunc &&
			GL_Uniform1iFunc &&
			GL_Uniform1fFunc &&
			GL_Uniform3fFunc &&
			GL_Uniform4fFunc &&
			GL_Uniform4fvFunc)
		{
			Con_Printf("FOUND: GLSL\n");
			gl_glsl_able = true;
		}
		else
		{
			Con_Warning ("GLSL not available\n");
		}
	}
	else
	{
		Con_Warning ("OpenGL version < 2, GLSL not available\n");
	}
	
	// GLSL gamma
	//
	if (COM_CheckParm("-noglslgamma"))
		Con_Warning ("GLSL gamma disabled at command line\n");
#ifndef VITA
	else if (gl_glsl_able)
	{
		gl_glsl_gamma_able = true;
	}
#endif
	else
	{
		Con_Warning ("GLSL gamma not available, using hardware gamma\n");
	}
    
    // GLSL alias model rendering
    //
	if (COM_CheckParm("-noglslalias"))
		Con_Warning ("GLSL alias model rendering disabled at command line\n");
	else if (gl_glsl_able && gl_vbo_able && gl_max_texture_units >= 3)
	{
		gl_glsl_alias_able = true;
	}
	else
	{
		Con_Warning ("GLSL alias model rendering not available, using Fitz renderer\n");
	}
}

/*
===============
GL_SetupState -- johnfitz

does all the stuff from GL_Init that needs to be done every time a new GL render context is created
===============
*/
static void GL_SetupState (void)
{
	glClearColor (0.15,0.15,0.15,0); //johnfitz -- originally 1,0,0,0
	glCullFace(GL_BACK); //johnfitz -- glquake used CCW with backwards culling -- let's do it right
	glFrontFace(GL_CW); //johnfitz -- glquake used CCW with backwards culling -- let's do it right
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
#ifndef VITA
	glShadeModel (GL_FLAT);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //johnfitz
#endif
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//spike -- these are invalid as there is no texture bound to receive this state.
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glDepthRange (0, 1); //johnfitz -- moved here becuase gl_ztrick is gone.
	glDepthFunc (GL_LEQUAL); //johnfitz -- moved here becuase gl_ztrick is gone.
}

/*
===============
GL_Init
===============
*/
static void GL_Init (void)
{
	gl_vendor = (const char *) glGetString (GL_VENDOR);
	gl_renderer = (const char *) glGetString (GL_RENDERER);
	gl_version = (const char *) glGetString (GL_VERSION);
	gl_extensions = (const char *) glGetString (GL_EXTENSIONS);

	Con_SafePrintf ("GL_VENDOR: %s\n", gl_vendor);
	Con_SafePrintf ("GL_RENDERER: %s\n", gl_renderer);
	Con_SafePrintf ("GL_VERSION: %s\n", gl_version);
#ifdef VITA // The code assumes Desktop GL versioning, so we enforce GL version to 2.0
	gl_version_major = 2;
#else
	if (gl_version == NULL || sscanf(gl_version, "%d.%d", &gl_version_major, &gl_version_minor) < 2)
	{
		gl_version_major = 0;
		gl_version_minor = 0;
	}
#endif
	GL_CheckExtensions (); //johnfitz

#ifdef __APPLE__
	// ericw -- enable multi-threaded OpenGL, gives a decent FPS boost.
	// https://developer.apple.com/library/mac/technotes/tn2085/
	if (host_parms->numcpus > 1 &&
	    kCGLNoError != CGLEnable(CGLGetCurrentContext(), kCGLCEMPEngine))
	{
		Con_Warning ("Couldn't enable multi-threaded OpenGL");
	}
#endif

	//johnfitz -- intel video workarounds from Baker
	if (!strcmp(gl_vendor, "Intel"))
	{
		Con_Printf ("Intel Display Adapter detected, enabling gl_clear\n");
		Cbuf_AddText ("gl_clear 1\n");	//Spike -- don't forget your \ns guys...
	}
	//johnfitz

	GLAlias_CreateShaders ();
	GLWorld_CreateShaders ();
	GL_ClearBufferBindings ();	
}

/*
=================
GL_BeginRendering -- sets values of glx, gly, glwidth, glheight
=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = *y = 0;
	*width = vid.width;
	*height = vid.height;
}

/*
=================
GL_EndRendering
=================
*/
void GL_EndRendering (void)
{
	if (!scr_skipupdate)
	{
#ifdef VITA
		vglSwapBuffers(GL_FALSE);
#elif defined(USE_SDL2)
		SDL_GL_SwapWindow(draw_context);
#else
		SDL_GL_SwapBuffers();
#endif
	}
}


void	VID_Shutdown (void)
{
	if (vid_initialized)
	{
		VID_Gamma_Shutdown (); //johnfitz

		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		draw_context = NULL;
#if defined(USE_SDL2)
		gl_context = NULL;
#endif
		PL_VID_Shutdown();
	}
}

void	VID_SetWindowCaption(const char *newcaption)
{
#if defined(USE_SDL2)
	SDL_SetWindowTitle(draw_context, newcaption);
#else
	SDL_WM_SetCaption(newcaption, newcaption);
#endif
}

/*
===================================================================

MAIN WINDOW

===================================================================
*/

/*
================
ClearAllStates
================
*/
static void ClearAllStates (void)
{
	Key_ClearStates ();
	IN_ClearStates ();
}


//==========================================================================
//
//  COMMANDS
//
//==========================================================================

/*
=================
VID_DescribeCurrentMode_f
=================
*/
static void VID_DescribeCurrentMode_f (void)
{
	if (draw_context)
		Con_Printf("%dx%dx%d %dHz %s\n",
			VID_GetCurrentWidth(),
			VID_GetCurrentHeight(),
			VID_GetCurrentBPP(),
			VID_GetCurrentRefreshRate(),
			VID_GetFullscreen() ? "fullscreen" : "windowed");
}

/*
=================
VID_DescribeModes_f -- johnfitz -- changed formatting, and added refresh rates after each mode.
=================
*/
static void VID_DescribeModes_f (void)
{
	int	i;
	int	lastwidth, lastheight, lastbpp, count;

	lastwidth = lastheight = lastbpp = count = 0;

	for (i = 0; i < nummodes; i++)
	{
		if (lastwidth != modelist[i].width || lastheight != modelist[i].height || lastbpp != modelist[i].bpp)
		{
			if (count > 0)
				Con_SafePrintf ("\n");
			Con_SafePrintf ("   %4i x %4i x %i : %i", modelist[i].width, modelist[i].height, modelist[i].bpp, modelist[i].refreshrate);
			lastwidth = modelist[i].width;
			lastheight = modelist[i].height;
			lastbpp = modelist[i].bpp;
			count++;
		}
	}
	Con_Printf ("\n%i modes\n", count);
}

/*
===================
VID_FSAA_f -- ericw -- warn that vid_fsaa requires engine restart
===================
*/
static void VID_FSAA_f (cvar_t *var)
{
	// don't print the warning if vid_fsaa is set during startup
	if (vid_initialized)
		Con_Printf("%s %d requires engine restart to take effect\n", var->name, (int)var->value);
}

//==========================================================================
//
//  INIT
//
//==========================================================================

/*
=================
VID_InitModelist
=================
*/
static void VID_InitModelist (void)
{
#if defined(USE_SDL2)
	const int sdlmodes = SDL_GetNumDisplayModes(0);
	int i;

	nummodes = 0;
	for (i = 0; i < sdlmodes; i++)
	{
		SDL_DisplayMode mode;

		if (nummodes >= MAX_MODE_LIST)
			break;
		if (SDL_GetDisplayMode(0, i, &mode) == 0)
		{
			modelist[nummodes].width = mode.w;
			modelist[nummodes].height = mode.h;
			modelist[nummodes].bpp = SDL_BITSPERPIXEL(mode.format);
			modelist[nummodes].refreshrate = mode.refresh_rate;
			nummodes++;
		}
	}
#else /* !defined(USE_SDL2) */
	SDL_PixelFormat	format;
	SDL_Rect	**modes;
	Uint32		flags;
	int		i, j, k, originalnummodes, existingmode;
	int		bpps[] = {16, 24, 32}; // enumerate >8 bpp modes

	originalnummodes = nummodes = 0;
	format.palette = NULL;

	// enumerate fullscreen modes
	flags = DEFAULT_SDL_FLAGS | SDL_FULLSCREEN;
	for (i = 0; i < (int)(sizeof(bpps)/sizeof(bpps[0])); i++)
	{
		if (nummodes >= MAX_MODE_LIST)
			break;

		format.BitsPerPixel = bpps[i];
		modes = SDL_ListModes(&format, flags);

		if (modes == (SDL_Rect **)0 || modes == (SDL_Rect **)-1)
			continue;

		for (j = 0; modes[j]; j++)
		{
			if (modes[j]->w > MAXWIDTH || modes[j]->h > MAXHEIGHT || nummodes >= MAX_MODE_LIST)
				continue;

			modelist[nummodes].width = modes[j]->w;
			modelist[nummodes].height = modes[j]->h;
			modelist[nummodes].bpp = bpps[i];
			modelist[nummodes].refreshrate = DEFAULT_REFRESHRATE;

			for (k=originalnummodes, existingmode = 0 ; k < nummodes ; k++)
			{
				if ((modelist[nummodes].width == modelist[k].width)   &&
				    (modelist[nummodes].height == modelist[k].height) &&
				    (modelist[nummodes].bpp == modelist[k].bpp))
				{
					existingmode = 1;
					break;
				}
			}

			if (!existingmode)
			{
				nummodes++;
			}
		}
	}

	if (nummodes == originalnummodes)
		Con_SafePrintf ("No fullscreen DIB modes found\n");
#endif /* !defined(USE_SDL2) */
}

/*
===================
VID_Init
===================
*/
void	VID_Init (void)
{
	static char vid_center[] = "SDL_VIDEO_CENTERED=center";
	int		p, width, height, refreshrate, bpp;
	int		display_width, display_height, display_refreshrate, display_bpp;
	qboolean	fullscreen;
	const char	*read_vars[] = { "vid_fullscreen",
					 "vid_width",
					 "vid_height",
					 "vid_refreshrate",
					 "vid_bpp",
					 "vid_vsync",
					 "vid_fsaa",
					 "vid_desktopfullscreen",
					 "vid_borderless"};
#define num_readvars	( sizeof(read_vars)/sizeof(read_vars[0]) )

	Cvar_RegisterVariable (&vid_fullscreen); //johnfitz
	Cvar_RegisterVariable (&vid_width); //johnfitz
	Cvar_RegisterVariable (&vid_height); //johnfitz
	Cvar_RegisterVariable (&vid_refreshrate); //johnfitz
	Cvar_RegisterVariable (&vid_bpp); //johnfitz
	Cvar_RegisterVariable (&vid_vsync); //johnfitz
	Cvar_RegisterVariable (&vid_fsaa); //QuakeSpasm
	Cvar_RegisterVariable (&vid_desktopfullscreen); //QuakeSpasm
	Cvar_RegisterVariable (&vid_borderless); //QuakeSpasm
	Cvar_SetCallback (&vid_fullscreen, VID_Changed_f);
	Cvar_SetCallback (&vid_width, VID_Changed_f);
	Cvar_SetCallback (&vid_height, VID_Changed_f);
	Cvar_SetCallback (&vid_refreshrate, VID_Changed_f);
	Cvar_SetCallback (&vid_bpp, VID_Changed_f);
	Cvar_SetCallback (&vid_vsync, VID_Changed_f);
	Cvar_SetCallback (&vid_fsaa, VID_FSAA_f);
	Cvar_SetCallback (&vid_desktopfullscreen, VID_Changed_f);
	Cvar_SetCallback (&vid_borderless, VID_Changed_f);
	
	Cmd_AddCommand ("vid_unlock", VID_Unlock); //johnfitz
	Cmd_AddCommand ("vid_restart", VID_Restart); //johnfitz
	Cmd_AddCommand ("vid_test", VID_Test); //johnfitz
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);

	putenv (vid_center);	/* SDL_putenv is problematic in versions <= 1.2.9 */
#ifndef VITA
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		Sys_Error("Couldn't init SDL video: %s", SDL_GetError());

#if defined(USE_SDL2)
	{
		SDL_DisplayMode mode;
		if (SDL_GetDesktopDisplayMode(0, &mode) != 0)
			Sys_Error("Could not get desktop display mode");

		display_width = mode.w;
		display_height = mode.h;
		display_refreshrate = mode.refresh_rate;
		display_bpp = SDL_BITSPERPIXEL(mode.format);
	}
#else
	{
		const SDL_VideoInfo *info = SDL_GetVideoInfo();
		display_width = info->current_w;
		display_height = info->current_h;
		display_refreshrate = DEFAULT_REFRESHRATE;
		display_bpp = info->vfmt->BitsPerPixel;
	}
#endif
#else
	display_width = 960;
	display_height = 544;
	display_refreshrate = DEFAULT_REFRESHRATE;
	display_bpp = 32;
#endif
	Cvar_SetValueQuick (&vid_bpp, (float)display_bpp);

	if (CFG_OpenConfig("config.cfg") == 0)
	{
		CFG_ReadCvars(read_vars, num_readvars);
		CFG_CloseConfig();
	}
	CFG_ReadCvarOverrides(read_vars, num_readvars);

	VID_InitModelist();

	width = (int)vid_width.value;
	height = (int)vid_height.value;
	refreshrate = (int)vid_refreshrate.value;
	bpp = (int)vid_bpp.value;
	fullscreen = (int)vid_fullscreen.value;
	fsaa = (int)vid_fsaa.value;

	if (COM_CheckParm("-current"))
	{
		width = display_width;
		height = display_height;
		refreshrate = display_refreshrate;
		bpp = display_bpp;
		fullscreen = true;
	}
	else
	{
		p = COM_CheckParm("-width");
		if (p && p < com_argc-1)
		{
			width = Q_atoi(com_argv[p+1]);

			if(!COM_CheckParm("-height"))
				height = width * 3 / 4;
		}

		p = COM_CheckParm("-height");
		if (p && p < com_argc-1)
		{
			height = Q_atoi(com_argv[p+1]);

			if(!COM_CheckParm("-width"))
				width = height * 4 / 3;
		}

		p = COM_CheckParm("-refreshrate");
		if (p && p < com_argc-1)
			refreshrate = Q_atoi(com_argv[p+1]);
		
		p = COM_CheckParm("-bpp");
		if (p && p < com_argc-1)
			bpp = Q_atoi(com_argv[p+1]);

		if (COM_CheckParm("-window") || COM_CheckParm("-w"))
			fullscreen = false;
		else if (COM_CheckParm("-fullscreen") || COM_CheckParm("-f"))
			fullscreen = true;
	}

	p = COM_CheckParm ("-fsaa");
	if (p && p < com_argc-1)
		fsaa = atoi(com_argv[p+1]);

	if (!VID_ValidMode(width, height, refreshrate, bpp, fullscreen))
	{
		width = (int)vid_width.value;
		height = (int)vid_height.value;
		refreshrate = (int)vid_refreshrate.value;
		bpp = (int)vid_bpp.value;
		fullscreen = (int)vid_fullscreen.value;
	}

	if (!VID_ValidMode(width, height, refreshrate, bpp, fullscreen))
	{
		width = 640;
		height = 480;
		refreshrate = display_refreshrate;
		bpp = display_bpp;
		fullscreen = false;
	}

	vid_initialized = true;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	// set window icon
	PL_SetWindowIcon();

	VID_SetMode (width, height, refreshrate, bpp, fullscreen);

	GL_Init ();
	GL_SetupState ();
	Cmd_AddCommand ("gl_info", GL_Info_f); //johnfitz

	//johnfitz -- removed code creating "glquake" subdirectory

	vid_menucmdfn = VID_Menu_f; //johnfitz
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

	VID_Gamma_Init(); //johnfitz
	VID_Menu_Init(); //johnfitz

	//QuakeSpasm: current vid settings should override config file settings.
	//so we have to lock the vid mode from now until after all config files are read.
	vid_locked = true;
}

// new proc by S.A., called by alt-return key binding.
void	VID_Toggle (void)
{
	// disabling the fast path completely because SDL_SetWindowFullscreen was changing
	// the window size on SDL2/WinXP and we weren't set up to handle it. --ericw
	//
	// TODO: Clear out the dead code, reinstate the fast path using SDL_SetWindowFullscreen
	// inside VID_SetMode, check window size to fix WinXP issue. This will
	// keep all the mode changing code in one place.
	static qboolean vid_toggle_works = false;
	qboolean toggleWorked;
#if defined(USE_SDL2)
	Uint32 flags = 0;
#endif

	S_ClearBuffer ();

	if (!vid_toggle_works)
		goto vrestart;
	else if (gl_vbo_able)
	{
		// disabling the fast path because with SDL 1.2 it invalidates VBOs (using them
		// causes a crash, sugesting that the fullscreen toggle created a new GL context,
		// although texture objects remain valid for some reason).
		//
		// SDL2 does promise window resizes / fullscreen changes preserve the GL context,
		// so we could use the fast path with SDL2. --ericw
		vid_toggle_works = false;
		goto vrestart;
	}

#if defined(USE_SDL2)
	if (!VID_GetFullscreen()) {
		flags = vid_desktopfullscreen.value ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
	}

	toggleWorked = SDL_SetWindowFullscreen(draw_context, flags) == 0;
#else
	toggleWorked = SDL_WM_ToggleFullScreen(draw_context) == 1;
#endif

	if (toggleWorked)
	{
		Sbar_Changed ();	// Sbar seems to need refreshing

		modestate = VID_GetFullscreen() ? MS_FULLSCREEN : MS_WINDOWED;

		VID_SyncCvars();

		IN_UpdateGrabs();
	}
	else
	{
		vid_toggle_works = false;
		Con_DPrintf ("SDL_WM_ToggleFullScreen failed, attempting VID_Restart\n");
	vrestart:
		Cvar_SetQuick (&vid_fullscreen, VID_GetFullscreen() ? "0" : "1");
		Cbuf_AddText ("vid_restart\n");
	}
}

/*
================
VID_SyncCvars -- johnfitz -- set vid cvars to match current video mode
================
*/
void VID_SyncCvars (void)
{
	if (draw_context)
	{
		if (!VID_GetDesktopFullscreen())
		{
			Cvar_SetValueQuick (&vid_width, VID_GetCurrentWidth());
			Cvar_SetValueQuick (&vid_height, VID_GetCurrentHeight());
		}
		Cvar_SetValueQuick (&vid_refreshrate, VID_GetCurrentRefreshRate());
		Cvar_SetValueQuick (&vid_bpp, VID_GetCurrentBPP());
		Cvar_SetQuick (&vid_fullscreen, VID_GetFullscreen() ? "1" : "0");
		// don't sync vid_desktopfullscreen, it's a user preference that
		// should persist even if we are in windowed mode.
#ifndef VITA
		Cvar_SetQuick (&vid_vsync, VID_GetVSync() ? "1" : "0");
#endif
	}

	vid_changed = false;
}

//==========================================================================
//
//  NEW VIDEO MENU -- johnfitz
//
//==========================================================================

enum {
	VID_OPT_FPS,
	VID_OPT_XHAIR,
	VID_OPT_BILINEAR,
	VID_OPT_WATER_OPACITY,
	VID_OPT_SMOOTH_ANIM,
	VID_OPT_VSYNC,
	VID_OPT_RUMBLE,
	VID_OPT_RETROTOUCH,
	VID_OPT_GYROSCOPE,
	VID_OPT_GYRO_HORI,
	VID_OPT_GYRO_VERT,
	VID_OPT_WEAPON_POS,
	VIDEO_OPTIONS_ITEMS
};

static int	video_options_cursor = 0;

typedef struct {
	int width,height;
} vid_menu_mode;

//TODO: replace these fixed-length arrays with hunk_allocated buffers
static vid_menu_mode vid_menu_modes[MAX_MODE_LIST];
static int vid_menu_nummodes = 0;

static int vid_menu_bpps[MAX_BPPS_LIST];
static int vid_menu_numbpps = 0;

static int vid_menu_rates[MAX_RATES_LIST];
static int vid_menu_numrates=0;

/*
================
VID_Menu_Init
================
*/
static void VID_Menu_Init (void)
{
	int i, j, h, w;

	for (i = 0; i < nummodes; i++)
	{
		w = modelist[i].width;
		h = modelist[i].height;

		for (j = 0; j < vid_menu_nummodes; j++)
		{
			if (vid_menu_modes[j].width == w &&
				vid_menu_modes[j].height == h)
				break;
		}

		if (j == vid_menu_nummodes)
		{
			vid_menu_modes[j].width = w;
			vid_menu_modes[j].height = h;
			vid_menu_nummodes++;
		}
	}
}

/*
================
VID_Menu_RebuildBppList

regenerates bpp list based on current vid_width and vid_height
================
*/
static void VID_Menu_RebuildBppList (void)
{
	int i, j, b;

	vid_menu_numbpps = 0;

	for (i = 0; i < nummodes; i++)
	{
		if (vid_menu_numbpps >= MAX_BPPS_LIST)
			break;

		//bpp list is limited to bpps available with current width/height
		if (modelist[i].width != vid_width.value ||
			modelist[i].height != vid_height.value)
			continue;

		b = modelist[i].bpp;

		for (j = 0; j < vid_menu_numbpps; j++)
		{
			if (vid_menu_bpps[j] == b)
				break;
		}

		if (j == vid_menu_numbpps)
		{
			vid_menu_bpps[j] = b;
			vid_menu_numbpps++;
		}
	}

	//if there are no valid fullscreen bpps for this width/height, just pick one
	if (vid_menu_numbpps == 0)
	{
		Cvar_SetValueQuick (&vid_bpp, (float)modelist[0].bpp);
		return;
	}

	//if vid_bpp is not in the new list, change vid_bpp
	for (i = 0; i < vid_menu_numbpps; i++)
		if (vid_menu_bpps[i] == (int)(vid_bpp.value))
			break;

	if (i == vid_menu_numbpps)
		Cvar_SetValueQuick (&vid_bpp, (float)vid_menu_bpps[0]);
}

/*
================
VID_Menu_RebuildRateList

regenerates rate list based on current vid_width, vid_height and vid_bpp
================
*/
static void VID_Menu_RebuildRateList (void)
{
	int i,j,r;
	
	vid_menu_numrates=0;
	
	for (i=0;i<nummodes;i++)
	{
		//rate list is limited to rates available with current width/height/bpp
		if (modelist[i].width != vid_width.value ||
		    modelist[i].height != vid_height.value ||
		    modelist[i].bpp != vid_bpp.value)
			continue;
		
		r = modelist[i].refreshrate;
		
		for (j=0;j<vid_menu_numrates;j++)
		{
			if (vid_menu_rates[j] == r)
				break;
		}
		
		if (j==vid_menu_numrates)
		{
			vid_menu_rates[j] = r;
			vid_menu_numrates++;
		}
	}
	
	//if there are no valid fullscreen refreshrates for this width/height, just pick one
	if (vid_menu_numrates == 0)
	{
		Cvar_SetValue ("vid_refreshrate",(float)modelist[0].refreshrate);
		return;
	}
	
	//if vid_refreshrate is not in the new list, change vid_refreshrate
	for (i=0;i<vid_menu_numrates;i++)
		if (vid_menu_rates[i] == (int)(vid_refreshrate.value))
			break;
	
	if (i==vid_menu_numrates)
		Cvar_SetValue ("vid_refreshrate",(float)vid_menu_rates[0]);
}

/*
================
VID_Menu_ChooseNextMode

chooses next resolution in order, then updates vid_width and
vid_height cvars, then updates bpp and refreshrate lists
================
*/
static void VID_Menu_ChooseNextMode (int dir)
{
	int i;

	if (vid_menu_nummodes)
	{
		for (i = 0; i < vid_menu_nummodes; i++)
		{
			if (vid_menu_modes[i].width == vid_width.value &&
				vid_menu_modes[i].height == vid_height.value)
				break;
		}

		if (i == vid_menu_nummodes) //can't find it in list, so it must be a custom windowed res
		{
			i = 0;
		}
		else
		{
			i += dir;
			if (i >= vid_menu_nummodes)
				i = 0;
			else if (i < 0)
				i = vid_menu_nummodes-1;
		}

		Cvar_SetValueQuick (&vid_width, (float)vid_menu_modes[i].width);
		Cvar_SetValueQuick (&vid_height, (float)vid_menu_modes[i].height);
		VID_Menu_RebuildBppList ();
		VID_Menu_RebuildRateList ();
	}
}

/*
================
VID_Menu_ChooseNextBpp

chooses next bpp in order, then updates vid_bpp cvar
================
*/
static void VID_Menu_ChooseNextBpp (int dir)
{
	int i;

	if (vid_menu_numbpps)
	{
		for (i = 0; i < vid_menu_numbpps; i++)
		{
			if (vid_menu_bpps[i] == vid_bpp.value)
				break;
		}

		if (i == vid_menu_numbpps) //can't find it in list
		{
			i = 0;
		}
		else
		{
			i += dir;
			if (i >= vid_menu_numbpps)
				i = 0;
			else if (i < 0)
				i = vid_menu_numbpps-1;
		}

		Cvar_SetValueQuick (&vid_bpp, (float)vid_menu_bpps[i]);
	}
}

/*
================
VID_Menu_ChooseNextRate

chooses next refresh rate in order, then updates vid_refreshrate cvar
================
*/
static void VID_Menu_ChooseNextRate (int dir)
{
	int i;
	
	for (i=0;i<vid_menu_numrates;i++)
	{
		if (vid_menu_rates[i] == vid_refreshrate.value)
			break;
	}
	
	if (i==vid_menu_numrates) //can't find it in list
	{
		i = 0;
	}
	else
	{
		i+=dir;
		if (i>=vid_menu_numrates)
			i = 0;
		else if (i<0)
			i = vid_menu_numrates-1;
	}
	
	Cvar_SetValue ("vid_refreshrate",(float)vid_menu_rates[i]);
}

/*
================
VID_MenuKey
================
*/
static void VID_MenuKey (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case K_BBUTTON:
		VID_SyncCvars (); //sync cvars before leaving menu. FIXME: there are other ways to leave menu
		S_LocalSound ("misc/menuoption.wav");
		M_Menu_Options_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menuoption.wav");
		video_options_cursor--;
		if (video_options_cursor < 0)
			video_options_cursor = VIDEO_OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menuoption.wav");
		video_options_cursor++;
		if (video_options_cursor >= VIDEO_OPTIONS_ITEMS)
			video_options_cursor = 0;
		break;

	case K_LEFTARROW:
		S_LocalSound ("misc/menu3.wav");
		switch (video_options_cursor)
		{
		case VID_OPT_FPS:
			Cvar_SetValue ("scr_showfps", !scr_showfps.value);
			break;
		case VID_OPT_XHAIR:
			Cvar_SetValue ("crosshair", !crosshair.value);
			break;
		case VID_OPT_BILINEAR:
			if (!strcmp(gl_texturemode.string, "GL_NEAREST"))
				Cbuf_AddText ("gl_texturemode GL_LINEAR\n");
			else
				Cbuf_AddText ("gl_texturemode GL_NEAREST\n");
			break;
		case VID_OPT_WATER_OPACITY:
			r_wateralpha.value -= 0.1;
			if (r_wateralpha.value < 0)
				r_wateralpha.value = 0;
			if (r_wateralpha.value > 1)
				r_wateralpha.value = 1;
			Cvar_SetValue ("r_wateralpha", r_wateralpha.value);
			break;
		case VID_OPT_VSYNC:
			Cvar_SetValue ("vid_vsync", !vid_vsync.value);
			break;
		case VID_OPT_WEAPON_POS:
			w_pos_idx--;
			if (w_pos_idx > 3) w_pos_idx = 0;
			if (w_pos_idx < 0) w_pos_idx = 3;
			switch (w_pos_idx) {
			case 1:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", 8);
				break;
			case 2:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", -8);
				break;
			case 3:
				Cvar_SetValue ("r_drawviewmodel", 0);
				break;
			default:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", 0);
				break;
			}
			break;
		default:
			break;
		}
		break;

	case K_RIGHTARROW:
		S_LocalSound ("misc/menu3.wav");
		switch (video_options_cursor)
		{
		case VID_OPT_FPS:
			Cvar_SetValue ("scr_showfps", !scr_showfps.value);
			break;
		case VID_OPT_XHAIR:
			Cvar_SetValue ("crosshair", !crosshair.value);
			break;
		case VID_OPT_BILINEAR:
			if (!strcmp(gl_texturemode.string, "GL_NEAREST"))
				Cbuf_AddText ("gl_texturemode GL_LINEAR\n");
			else
				Cbuf_AddText ("gl_texturemode GL_NEAREST\n");
			break;
		case VID_OPT_WATER_OPACITY:
			r_wateralpha.value += 0.1;
			if (r_wateralpha.value < 0)
				r_wateralpha.value = 0;
			if (r_wateralpha.value > 1)
				r_wateralpha.value = 1;
			Cvar_SetValue ("r_wateralpha", r_wateralpha.value);
			break;
		case VID_OPT_GYRO_HORI:
			motion_horizontal_sensitivity.value += 0.5;
			if (motion_horizontal_sensitivity.value < 0)
				motion_horizontal_sensitivity.value = 0;
			if (motion_horizontal_sensitivity.value > 10)
				motion_horizontal_sensitivity.value = 10;
			Cvar_SetValue ("motion_horizontal_sensitivity", motion_horizontal_sensitivity.value);
			break;
		case VID_OPT_GYRO_VERT:
			motion_vertical_sensitivity.value -= 0.5;
			if (motion_vertical_sensitivity.value < 0)
				motion_vertical_sensitivity.value = 0;
			if (motion_vertical_sensitivity.value > 10)
				motion_vertical_sensitivity.value = 10;
			Cvar_SetValue ("motion_vertical_sensitivity", motion_vertical_sensitivity.value);
			break;
		case VID_OPT_VSYNC:
			Cvar_SetValue ("vid_vsync", !vid_vsync.value);
			break;
		case VID_OPT_WEAPON_POS:
			w_pos_idx++;
			if (w_pos_idx > 3) w_pos_idx = 0;
			if (w_pos_idx < 0) w_pos_idx = 3;
			switch (w_pos_idx) {
			case 1:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", 8);
				break;
			case 2:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", -8);
				break;
			case 3:
				Cvar_SetValue ("r_drawviewmodel", 0);
				break;
			default:
				Cvar_SetValue ("r_drawviewmodel", 1);
				Cvar_SetValue ("r_viewmodeloffset", 0);
				break;
			}
			break;
		default:
			break;
		}
		break;

	case K_ENTER:
	case K_KP_ENTER:
	case K_ABUTTON:
		m_entersound = true;
		switch (video_options_cursor)
		{
		case VID_OPT_FPS:
			Cvar_SetValue ("scr_showfps", !scr_showfps.value);
			break;
		case VID_OPT_XHAIR:
			Cvar_SetValue ("crosshair", !crosshair.value);
			break;
		case VID_OPT_BILINEAR:
			if (!strcmp(gl_texturemode.string, "GL_NEAREST"))
				Cbuf_AddText ("gl_texturemode GL_LINEAR\n");
			else
				Cbuf_AddText ("gl_texturemode GL_NEAREST\n");
			break;
		case VID_OPT_VSYNC:
			Cvar_SetValue ("vid_vsync", !vid_vsync.value);
			break;
		case VID_OPT_RUMBLE:
			Cvar_SetValue ("pstv_rumble", !pstv_rumble.value);
			break;
		case VID_OPT_RETROTOUCH:
			Cvar_SetValue ("retrotouch", !retrotouch.value);
			break;
		case VID_OPT_GYROSCOPE:
			Cvar_SetValue ("motioncam", !motioncam.value);
			break;
		case VID_OPT_SMOOTH_ANIM:
			Cvar_SetValue ("r_lerpmodels", !r_lerpmodels.value);
			Cvar_SetValue ("r_lerpmoves", r_lerpmodels.value);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

/*
================
VID_MenuDraw
================
*/
static void VID_MenuDraw (void)
{
	int i, y;
	qpic_t *p;
	const char *title;
	float r;
	
	y = 4;

	//p = Draw_CachePic ("gfx/vidmodes.lmp");
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, y, p);

	y += 28;

	// title
	title = "Advanced Options";
	M_PrintWhite ((320-8*strlen(title))/2, y, title);

	y += 16;

	// options
	for (i = 0; i < VIDEO_OPTIONS_ITEMS; i++)
	{
		switch (i)
		{
		case VID_OPT_FPS:
			M_Print (16, y, "    Show Framerate");
			M_DrawCheckbox (220, y, scr_showfps.value);
			break;
		case VID_OPT_XHAIR:
			M_Print (16, y, "    Show Crosshair");
			M_DrawCheckbox (220, y, crosshair.value);
			break;
		case VID_OPT_BILINEAR:
			M_Print (16, y, "Bilinear Filtering");
			M_DrawCheckbox (220, y, strcmp(gl_texturemode.string, "GL_NEAREST"));
			break;
		case VID_OPT_WATER_OPACITY:
			M_Print (16, y, "     Water Opacity");
			r = r_wateralpha.value;
			M_DrawSlider (220, y, r);
			break;
		case VID_OPT_SMOOTH_ANIM:
			M_Print (16, y, " Smooth Animations");
			M_DrawCheckbox (220, y, (int)r_lerpmodels.value);
			break;
		case VID_OPT_VSYNC:
			M_Print (16, y, "     Vertical sync");
			M_DrawCheckbox (220, y, (int)vid_vsync.value);
			break;
		case VID_OPT_RUMBLE:
			M_Print (16, y, "     Rumble Effect");
			M_DrawCheckbox (220, y, pstv_rumble.value);
			break;
		case VID_OPT_RETROTOUCH:
			M_Print (16, y, "    Use Retrotouch");
			M_DrawCheckbox (220, y, retrotouch.value);
			break;
		case VID_OPT_GYROSCOPE:
			M_Print (16, y, "     Use Gyroscope");
			M_DrawCheckbox (220, y, motioncam.value);
			break;
		case VID_OPT_GYRO_HORI:
			M_Print (16, y, "Gyro X Sensitivity");
			r = motion_horizontal_sensitivity.value/10;
			M_DrawSlider (220, y, r);
			break;
		case VID_OPT_GYRO_VERT:
			M_Print (16, y, "Gyro Y Sensitivity");
			r = motion_vertical_sensitivity.value/10;
			M_DrawSlider (220, y, r);
			break;
		case VID_OPT_WEAPON_POS:
			M_Print (16, y, "   Weapon Position");
			if (w_pos_idx == -1) {
				if (!r_drawviewmodel.value) w_pos_idx = 3;
				else if (r_viewmodeloffset.value < 0) w_pos_idx = 2;
				else if (r_viewmodeloffset.value > 0) w_pos_idx = 1;
				else w_pos_idx = 0;
			}
			M_Print (220, y, w_pos[w_pos_idx]);
			break;
		}

		if (video_options_cursor == i)
			M_DrawCharacter (168, y, 12+((int)(realtime*4)&1));

		y += 8;
	}
}

/*
================
VID_Menu_f
================
*/
static void VID_Menu_f (void)
{
	key_dest = key_menu;
	m_state = m_video;
	m_entersound = true;
	IN_UpdateGrabs();

	//set all the cvars to match the current mode when entering the menu
	VID_SyncCvars ();

	//set up bpp and rate lists based on current cvars
	VID_Menu_RebuildBppList ();
	VID_Menu_RebuildRateList ();
}

void VID_UpdateCursor(void)
{
	SDL_Cursor *nc;

	qcvm_t *vm;
	if (key_dest == key_menu)
		vm = &cls.menu_qcvm;
	else if (key_dest == key_game)
		vm = &cl.qcvm;
	else
		vm = NULL;
	nc = vm?vm->cursorhandle:NULL;
	if (vid_cursor != nc)
	{
		vid_cursor = nc;
		if (nc)	//null is an invalid sdl cursor handle
			SDL_SetCursor(nc);
		else
			SDL_SetCursor(SDL_GetDefaultCursor());	//doesn't need freeing or anything.
	}
}
void VID_SetCursor(qcvm_t *vm, const char *cursorname, float hotspot[2], float cursorscale)
{
	SDL_Cursor *oldcursor;
	int mark = Hunk_LowMark();
	qboolean malloced = false;
	char npath[MAX_QPATH];
	SDL_Surface *surf = NULL;
	byte *imagedata = NULL;
	if (cursorname && *cursorname)
	{
		enum srcformat fmt;
		int width, height;
		COM_StripExtension(cursorname, npath, sizeof(npath));
		imagedata = Image_LoadImage(npath, &width, &height, &fmt, &malloced);
		if (imagedata && fmt == SRC_RGBA)
		{	//simple 32bit RGBA byte-ordered data.
			surf = SDL_CreateRGBSurfaceFrom(imagedata, width, height, 32, width*4, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
			if (cursorscale != 1 && surf)
			{	//rescale image by cursorscale
				int nwidth = q_max(1,width*cursorscale);
				int nheight = q_max(1,height*cursorscale);
				SDL_Surface *scaled = SDL_CreateRGBSurface(0, nwidth, nheight, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
				SDL_BlitScaled(surf, NULL, scaled, NULL);
				SDL_FreeSurface(surf);
				surf = scaled;
			}
		}
	}

	oldcursor = vm->cursorhandle;
	if (surf)
	{
		vm->cursorhandle = SDL_CreateColorCursor(surf, hotspot[0], hotspot[1]);
		SDL_FreeSurface(surf);
	}
	else
		vm->cursorhandle = NULL;

	Hunk_FreeToLowMark(mark);
	if (malloced)
		free(imagedata);

	VID_UpdateCursor();
	if (oldcursor)
		SDL_FreeCursor(oldcursor);
}