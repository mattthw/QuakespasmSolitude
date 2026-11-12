/* Apple shim for Vita-fork vglGetProcAddress calls on Mac builds.
 * Injected via CFLAGS -include (untracked; build-portability only). */
#ifdef __APPLE__
#ifndef VGL_SHIM_APPLE_H
#define VGL_SHIM_APPLE_H
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_timer.h>
static void *vglGetProcAddress(const char *name)
{
    return (void *)SDL_GL_GetProcAddress(name);
}
static void vglWaitVblankStart(int wait)
{
    SDL_GL_SetSwapInterval(wait);
}
#define sceTouchSetSamplingState(...) ((void)0)
#define sceMotionReset() ((void)0)
#define sceMotionStartSampling() ((void)0)
#define sceKernelGetModel() (0) /* not PSTV: skip PSTV-only HUD tweaks */
static void IN_StartRumble(void) { }
static void IN_StopRumble(void) { }
static long long sceKernelGetProcessTimeWide(void)
{
    return (long long)SDL_GetTicks() * 1000; /* us-ish; rumble_tick stays 0 on Mac anyway */
}
static int IN_SwitchKeyboard(char *out, int out_len) { if (out && out_len) out[0] = 0; return 0; }
extern void Sbar_DrawString (int x, int y, const char *str);
extern void Sbar_DrawCharacter (int x, int y, int num);
#endif
#endif
