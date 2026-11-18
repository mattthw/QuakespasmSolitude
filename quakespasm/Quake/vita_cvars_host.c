/* Host-build layer (macOS, Linux): cvar definitions that live inside
 * `#ifdef VITA` blocks in in_sdl.c but are referenced from unguarded code
 * (gl_vidsdl.c, cl_input.c, menu.c option tables). Defaults copied from
 * in_sdl.c:43-47. Compiled into every non-Vita build -- the generic Makefile
 * lists it in GLOBJS and tools/mac.sh compiles it by hand for the Darwin link.
 */
#if !defined(VITA)

#include "quakedef.h"

cvar_t pstv_rumble = {"pstv_rumble", "1", CVAR_ARCHIVE};
cvar_t retrotouch = {"retrotouch", "0", CVAR_ARCHIVE};
cvar_t motioncam = {"motioncam", "0", CVAR_ARCHIVE};
cvar_t motion_horizontal_sensitivity = {"motion_horizontal_sensitivity", "0", CVAR_ARCHIVE};
cvar_t motion_vertical_sensitivity = {"motion_vertical_sensitivity", "0", CVAR_ARCHIVE};

#endif /* !VITA */

#ifdef __APPLE__
/* Mac-only: sys_sdl_unix.c Sys_Init() dereferences host_parms->argv[0], but on
 * Mac host_parms is only assigned later by Host_Init -- the Vita main() that
 * seeds it early is #ifdef VITA'd out. Pre-seed a valid quakeparms_t whose
 * argv[0] sits INSIDE the repo dir so Sys_GetBasedir() resolves basedir to the
 * repo root. Linux does not need this: pl_linux/main_sdl set it up normally. */
static char s_argv0[] = "/Users/matt_1/workspace/projects-vita/solitude/repo/quakespasm";
static char *s_argv[] = { s_argv0, NULL };
static int s_argc = 1;
static quakeparms_t s_parms;

__attribute__((constructor))
static void seed_host_parms(void)
{
	memset(&s_parms, 0, sizeof(s_parms));
	s_parms.argc = s_argc;
	s_parms.argv = s_argv;
	host_parms = &s_parms;
}
#endif
