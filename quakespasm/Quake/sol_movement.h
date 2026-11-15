/*
sol_movement.h -- perf B11: Halo-3 movement model (GitHub issue #48).

SINGLE SOURCE OF TRUTH for the movement physics model. All tuning lives
here; nothing about the model is a runtime cvar, per the batch decision
("I dont want other players changing their physics mid game"). Flip
SOL_MOVEMENT_HALO to 0 and rebuild to restore the classic Quake
dot-product model for A/B comparison.

The QuakeC side mirrors these numbers for bot physics (frikbot's
bot_phys.qc is a hand-port of sv_user.c and must stay in lockstep):
the matching block lives at the top of qcc-src/progs.src
(SOL_MOVEMENT_HALO / SOL_ACCEL_TIME / SOL_DECEL_TIME / SOL_AIR_PCT).
A mismatch means bots and humans live under different physics.

Ships ON. Landed 2026-11-14 alongside host.c sol_gravity/sol_jump
defaults, menu.c Custom Game Rules defaults, world.qc fallback, and the
bot_phys.qc/bot_move.qc mirrors.
*/
#ifndef SOL_MOVEMENT_H
#define SOL_MOVEMENT_H

#define SOL_MOVEMENT_HALO 1

#if SOL_MOVEMENT_HALO

/* Ground: linear ramp, SOL_ACCEL_TIME seconds to make a full-speed
 * change in any direction; stop is a constant decel ramp of
 * SOL_DECEL_TIME seconds from sv_maxspeed. No edge-friction trace. */
#define SOL_HALO_ACCEL_TIME 0.15f
#define SOL_HALO_DECEL_TIME 0.12f

/* Air: acceleration toward the wish direction scaled to 8% of the
 * ground formula -- ledge-nudge only, strafe-jumping is dead. The
 * radial cap below pins airborne horizontal speed at sv_maxspeed. */
#define SOL_HALO_AIR_CONTROL 0.08f

/* Radial speed cap: horizontal velocity is clamped to sv_maxspeed every
 * frame (MOVETYPE_WALK only). Vertical velocity is NOT capped, so
 * rocket/super-jump launch height survives; horizontal launch push and
 * damage knockback are clamped like everything else. */

/* View bob softening (backlog item on the same issue): gentler vertical
 * bob amplitude and a much smaller fore-aft gun push. cl_bob stays a
 * user cvar; only the compiled-in default changes. */
#define SOL_HALO_CLBOB_DEFAULT "0.01"
#define SOL_HALO_GUN_BOB 0.15f

/* Model-matched shipping defaults for the existing sol_* trio (host.c
 * registration + menu.c Custom Game Rules + world.qc fallback):
 * g=230 (68% of the legacy 340), jump=172 -> apex 172^2/(2*230)=64.3u,
 * hangtime 1.50s, ~300u full-speed jump distance. */
#define SOL_DEFAULT_GRAVITY_S "230"
#define SOL_DEFAULT_GRAVITY_I 230
#define SOL_DEFAULT_JUMP_S "172"
#define SOL_DEFAULT_JUMP_I 172

#else /* classic Quake model */

#define SOL_HALO_CLBOB_DEFAULT "0.02"
#define SOL_HALO_GUN_BOB 0.4f

#define SOL_DEFAULT_GRAVITY_S "340"
#define SOL_DEFAULT_GRAVITY_I 340
#define SOL_DEFAULT_JUMP_S "225"
#define SOL_DEFAULT_JUMP_I 225

#endif /* SOL_MOVEMENT_HALO */

#endif /* SOL_MOVEMENT_H */
