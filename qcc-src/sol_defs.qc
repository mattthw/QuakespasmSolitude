// Solitude defs
#define DEFAULT_GRAVITY			340
#define DEFAULT_MAXSPEED		170			// max walk speed on ground
.float next_jump_time;		// next time the player can jump (usually after landing)
.float setup_death;			// setup respawn timer on player death
void () Player_SetupDeath;

.float needles;				// how many needler needles a player has in him

float (float wep, float cliptype) W_GetMaxClip;
void (float wtype, float ttype) ThrowWeapon;
void () player_throw_grenade;
void () pre_player_throw_grenade;
void () pre_player_melee;
void ( entity t_ent, float weptype, float cliptype, float newamt ) SetAmmoFor;
void () NeedlerHome;


.float selnade; // the type of nade being thrown
#define GRENADE_REGULAR		0
#define GRENADE_STICKY		1

.float weaponheat;

.entity pp_cooldown;

.float pickup_time, canpickup_time;

.float lives;		// times an item can respawn

float           teamlimit; // used to determine when to end team based games

.float exshells;
.float exshells2;
.float exhshells;
.float exnails;
.float exbr;
.float exrockets;
.float excells;

//sniper rifle
.float ammo_sniper;
.float exsniper;

// plasma pistol
.float ammo_ppistol;
.float exppistol;

// plasma rifle
.float ammo_prifle;
.float exprifle;
.float prifle_heat;			// plasma rifle fires faster the longer it's fired

// energy sword
.float ammo_sword;
.float exsword;

// needler
.float ammo_needler;
.float exneedler;
.float needler_heat;		// needler fires faster the longer it's held down
.float live_needles;		// B2: shooter's in-flight W_FireNeedler count (sv_max_needles cap)
.float counted_needle;		// B2: 1 = projectile tracked in owner.live_needles (Mario needles untracked)

.float ammo_shells2;
.float ammo_hshells;

.float axhitme;

.float ohealth;		// for regen
.float regen;		///


// animations
//weapon frame
.float donefire;
.float wepanim;
//player frame
.float fireframe;
.float jump_time;
.float pain_time;
.float reload_time;
.float pull_time;
.float nade_time, melee_time;
.float death_type;

.float ammo_br;
float   IT_BR                           = 90248441;         // solitude


#define MOD_SOLITUDE		666			// undefined mode of death

// weapons
#define WEAPON_SWORD		IT_AXE
#define WEAPON_PISTOL		IT_SHOTGUN
#define WEAPON_SHOT			IT_SUPER_SHOTGUN
#define WEAPON_AR			IT_NAILGUN
#define WEAPON_UZI			IT_SUPER_NAILGUN
#define WEAPON_SNIPER		IT_GRENADE_LAUNCHER
#define WEAPON_RL			IT_ROCKET_LAUNCHER
#define WEAPON_PPISTOL		IT_LIGHTNING
#define WEAPON_NEEDLER		IT_EXTRA_WEAPON
#define WEAPON_PRIFLE		IT_EXTRA_WEAPON2
#define WEAPON_SKULL		IT_SKULL
#define WEAPON_BR           IT_BR
#define WEAPON_GHOST        IT_GHOST
#define WEAPON_FLAG         IT_FLAG

// Zooming
.float pfov;
#define FOV_DEFAULT			90

// Frikbot
void(float to, float f) WriteByte;
void(float to, string s) WriteString;
void () reload;

// perf B6: server-side QC perf knobs (sv_bot_thinkrate / sv_particle_scale /
// sv_max_gibs). Cvars registered engine-side in host.c - QC cvar() is read-only
// in this fork (B2 / sv_max_needles precedent). All default 0 = off.
.float b_ai_nextthink;		// B6: per-bot AI decision clock (sv_bot_thinkrate)
.float gib_serial;			// B6: spawn-order stamp; >0 only on live tracked gibs
float gib_count;			// B6: number of live tracked gib entities
float gib_serial_next;		// B6: monotonic stamp source

void() BotAI;				// frikbot/bot_ai.qc

// B6: stagger wrapper for the BotAI calls in the physics chain (bot_phys.qc).
// rate 0 (default) = pre-B6 behavior (BotAI every frame, its internal per-skill
// throttle aside). rate > 0: skip unless due on the shared per-bot clock that
// the BotFrame obstacle gate (bot.qc) uses too, so decisions throttle together.
void() BotAI_Staggered =
{
	local float rate;

	rate = cvar("sv_bot_thinkrate");
	if (rate <= 0)
	{
		BotAI();
		return;
	}
	if (time < self.b_ai_nextthink)
		return;
	self.b_ai_nextthink = time + 1 / rate;
	BotAI();
};

// B6: sv_particle_scale = percent (50/75) applied to QC particle() counts.
// 0 or >= 100 = pass-through; rint-rounded with floor 1 so hits keep feedback.
float(float n) ParticleScaled =
{
	local float s;

	s = cvar("sv_particle_scale");
	if (s <= 0 || s >= 100)
		return n;
	n = rint(n * s / 100);
	if (n < 1)
		return 1;
	return n;
};

// B6: sv_max_gibs = live gib cap, remove-oldest. GibDespawn replaces SUB_Remove
// on tracked gibs so the counter stays honest; gib_serial is zeroed on EVERY
// removal path, so a recycled edict slot can never masquerade as a live gib.
void() GibDespawn =
{
	self.gib_serial = 0;
	gib_count = gib_count - 1;
	remove(self);
};

void(entity g) GibTrackNew =
{
	local float cap;
	local entity e, oldest;
	local float best;

	cap = cvar("sv_max_gibs");
	gib_serial_next = gib_serial_next + 1;
	g.gib_serial = gib_serial_next;
	gib_count = gib_count + 1;
	g.think = GibDespawn;
	if (cap <= 0 || gib_count <= cap)
		return;
	oldest = world;
	best = 0;
	for (e = nextent(world); e != world; e = nextent(e))
	{
		if (e.gib_serial <= 0 || e.think != GibDespawn)
			continue;
		if (oldest == world || e.gib_serial < best)
		{
			oldest = e;
			best = e.gib_serial;
		}
	}
	if (oldest != world)
	{
		oldest.gib_serial = 0;
		gib_count = gib_count - 1;
		remove(oldest);
	}
};
