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
 * argv[0] is a path that exists, so Sys_GetBasedir()'s realpath() succeeds.
 *
 * The path used to be a hardcoded one into the developer's checkout, which
 * shipped a home directory in the source and in the binary. It is the running
 * executable now, so basedir falls back to the .app bundle's directory -- which
 * is only a fallback: every launch path passes -basedir (COM_InitFilesystem
 * prefers that parm), and Sys_Init merely needs argv[0] to be non-NULL and
 * realpath-able. Linux does not need this: pl_linux/main_sdl set it up normally.
 */
#include <mach-o/dyld.h>

#define HOST_ARGV0_MAX 1024
static char s_argv0[HOST_ARGV0_MAX];
static char *s_argv[] = { s_argv0, NULL };
static int s_argc = 1;
static quakeparms_t s_parms;

__attribute__((constructor))
static void seed_host_parms(void)
{
	uint32_t len = sizeof(s_argv0);

	if (_NSGetExecutablePath(s_argv0, &len) != 0)
	{
		/* buffer too small or no answer: "." is realpath-able everywhere */
		s_argv0[0] = '.';
		s_argv0[1] = '\0';
	}

	memset(&s_parms, 0, sizeof(s_parms));
	s_parms.argc = s_argc;
	s_parms.argv = s_argv;
	host_parms = &s_parms;
}
#endif
