
// Effects code (EFF) --

// TE effects/muzzleflash/messages

// modular efs
var float ef_red  = 0;           // used for glow on pent item
var float ef_pent = EF_DIMLIGHT; // used for pent effect on player
var float ef_blue = 0;           // used for glow on quad
var float ef_quad = EF_DIMLIGHT; // used for quad effect on player

// base te write functions
void WriteVector (float msg, vector org) =
{
	WriteCoord (msg, org_x);
	WriteCoord (msg, org_y);
	WriteCoord (msg, org_z);
};

#ifdef NETQUAKE
void(vector org, float effect) _EFF_PointEffect =
{
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, effect);
	WriteVector (MSG_BROADCAST, org);
};

void(entity ent, vector org, vector org2, float effect) _EFF_BeamEffect =
{
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, effect);
	WriteEntity (MSG_BROADCAST, ent);
	WriteVector (MSG_BROADCAST, org);
	WriteVector (MSG_BROADCAST, org2);
};

void(vector org, float damage, float effect) _EFF_CountEffect =
{
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);
	WriteByte (MSG_BROADCAST, effect);
	WriteByte (MSG_BROADCAST, damage);
	WriteVector (MSG_BROADCAST, org);
};

#define EFF_PointEffect(a,b,c) _EFF_PointEffect(a,b)
#define EFF_BeamEffect(a,b,c,d,e) _EFF_BeamEffect(a,b,c,d)
#define EFF_CountEffect(a,b,c,d) _EFF_CountEffect(a,b,c)
#else
void(vector org, float effect, float mcast) EFF_PointEffect =
{
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, effect);
	WriteVector (MSG_MULTICAST, org);
	multicast (org, mcast);
};

void(entity ent, vector org, vector org2, float effect, float mcast) EFF_BeamEffect =
{
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, effect);
	WriteEntity (MSG_MULTICAST, ent);
	WriteVector (MSG_MULTICAST, org);
	WriteVector (MSG_MULTICAST, org2);
	multicast (org, mcast);
};

void(vector org, float damage, float effect, float mcast) EFF_CountEffect =
{
	WriteByte (MSG_MULTICAST, SVC_TEMPENTITY);
	WriteByte (MSG_MULTICAST, effect);
	WriteByte (MSG_MULTICAST, damage);
	WriteVector (MSG_MULTICAST, org);
	multicast (org, mcast);
};
#endif

/*
================
SpawnBlood
================
*/
void(vector org, float damage) _SpawnBlood_TEBlood =
{
	te_blood(org, '0 0 0', damage*2);
};

void(vector org, float damage) _SpawnBlood =
{
#ifdef NETQUAKE
	particle (org, '0 0 0', 73, damage*2);
#else
	EFF_CountEffect(org, damage, TE_BLOOD, MULTICAST_PVS);
#endif
};
/*
void() NadeTouch =
{
if (other.nade_lives > 4)
{
sprint (other, "Can't pick up anymore grenades/n");
return;
}
other.nade_lives = other.nade_lives + 1;
};

*/
void() s_smokee6 = [5,		SUB_Remove] {};
void() s_smokee5 = [4,		s_smokee6] {};
void() s_smokee4 = [3,		s_smokee5] {};
void() s_smokee3 = [2,		s_smokee4] {};
void() s_smokee2 = [1,		s_smokee3] {};
void() s_smokee1 = [0,		s_smokee2] {};

void() s_needle3 = [2,		SUB_Remove] {};
void() s_needle2 = [1,		s_needle3] {};
void() s_needle1 = [0,		s_needle2] {};

void() s_plasma14 = [13,		SUB_Remove] {};
void() s_plasma13 = [12,		s_plasma14] {};
void() s_plasma12 = [11,		s_plasma13] {};
void() s_plasma11 = [10,		s_plasma12] {};
void() s_plasma10 = [9,		s_plasma11] {};
void() s_plasma9 = [8,		s_plasma10] {};
void() s_plasma8 = [7,		s_plasma9] {};
void() s_plasma7 = [6,		s_plasma8] {};
void() s_plasma6 = [5,		s_plasma7] {};
void() s_plasma5 = [4,		s_plasma6] {};
void() s_plasma4 = [3,		s_plasma5] {};
void() s_plasma3 = [2,		s_plasma4] {};
void() s_plasma2 = [1,		s_plasma3] {};
void() s_plasma1 = [0,		s_plasma2] {};

void(vector org) createpinkdust =
{
	local entity s;


	s = self; // save old self

	self = spawn();

	self.movetype = MOVETYPE_NONE;
	self.velocity = '0 0 0';
	self.touch = SUB_Null;
	setmodel (self, "progs/needler_dust.spr");
	setorigin (self, org);
	self.solid = SOLID_NOT;
	s_needle1();
	self = s; // restore old self
	
};
void(vector org) createplasma =
{
	local entity s;


	s = self; // save old self

	self = spawn();

	self.movetype = MOVETYPE_NONE;
	self.velocity = '0 0 0';
	self.touch = SUB_Null;
	setmodel (self, "sprites/s_plasma4.spr");
	setorigin (self, org);
	self.solid = SOLID_NOT;
	s_plasma1();
	self = s; // restore old self
	
};

void() s_plasmaa5 = [4,		SUB_Remove] {};
void() s_plasmaa4 = [3,		s_plasmaa5] {createplasma(self.origin);};
void() s_plasmaa3 = [2,		s_plasmaa4] {};
void() s_plasmaa2 = [1,		s_plasmaa3] {};
void() s_plasmaa1 = [0,		s_plasmaa2] {};

void(vector org) plasmaexplo =
{
	local entity s;
	local float r;
	r = random();


	s = self; // save old self

	self = spawn();

	self.movetype = MOVETYPE_NONE;
	self.velocity = '0 0 0';
	self.touch = SUB_Null;
	setmodel (self, "sprites/plasma3.spr");
	setorigin (self, org);
	self.solid = SOLID_NOT;
	
	// So that debris comes out in random spurts over It coming out in the same number every time --Mexicouger

	s_plasmaa1 ();

	self = s; // restore old self
};
void(vector org) pistolsmoke =
{
	local entity s;


	s = self; // save old self

	self = spawn();

	self.movetype = MOVETYPE_FLY;
	

	self.velocity = v_forward;
	self.velocity = (self.velocity * 80);
	self.touch = SUB_Null;
	setmodel (self, "sprites/s_smoke2.spr");
	setorigin (self, ((org + (v_forward * 15)) + '0 0 12'));
	self.solid = SOLID_NOT;
	s_smokee1();
	self = s; // restore old self
};

// #if defined(NETQUAKE) || defined(MONSTERS)


void() s_explode14 = [13,		SUB_Remove] {};
void() s_explode13 = [12,		s_explode14] {};
void() s_explode12 = [11,		s_explode13] {};
void() s_explode11 = [10,		s_explode12] {};
void() s_explode10 = [9,		s_explode11] {};
void() s_explode9 = [8,		s_explode10] {};
void() s_explode8 = [7,		s_explode9] {};
void() s_explode7 = [6,		s_explode8] {};
void() s_explode6 = [5,		s_explode7] {};
void() s_explode5 = [4,		s_explode6] {};
void() s_explode4 = [3,		s_explode5] {};
void() s_explode3 = [2,		s_explode4] {};
void() s_explode2 = [1,		s_explode3] {};
void() s_explode1 = [0,		s_explode2] {};

void(vector org) CreateExplosion =
{
	local entity s;
	local float r;
	r = random();


	s = self; // save old self

	self = spawn();

	self.movetype = MOVETYPE_NONE;
	self.velocity = '0 0 0';
	self.touch = SUB_Null;
	setmodel (self, "progs/s_explod.spr");
	setorigin (self, org);
	self.solid = SOLID_NOT;
	sound (self, CHAN_WEAPON, "weapons/r_exp3.wav", 1, ATTN_NORM);
	// So that debris comes out in random spurts over It coming out in the same number every time --Mexicouger
		s_explode1 ();

	self = s; // restore old self
};
// #endif

void(vector org) _TE_explosion =
{
	EFF_PointEffect(org, TE_EXPLOSION, MULTICAST_PHS);
#ifdef NETQUAKE
	CreateExplosion(org);
#endif
};

void(vector org) _TE_gunshot =
{
#ifdef NETQUAKE
	EFF_PointEffect(org, TE_GUNSHOT, MULTICAST_PVS);
#else
	EFF_CountEffect(org, 3, TE_GUNSHOT, MULTICAST_PVS);
#endif
};

void(vector org) _TE_spike =
{
	EFF_PointEffect(org, TE_SPIKE, MULTICAST_PHS);
};

void(vector org) _TE_knightspike =
{
	EFF_PointEffect(org, TE_KNIGHTSPIKE, MULTICAST_PHS);
};

void(vector org) _TE_wizspike =
{
	EFF_PointEffect(org, TE_WIZSPIKE, MULTICAST_PHS);
};

void(vector org) _TE_superspike =
{
	EFF_PointEffect(org, TE_SUPERSPIKE, MULTICAST_PHS);
};

void(vector org) _TE_teleport =
{
	EFF_PointEffect(org, TE_TELEPORT, MULTICAST_PHS);
};

void(vector org) _TE_lavasplash = 
{
	EFF_PointEffect(org, TE_LAVASPLASH, MULTICAST_PVS);
};

void(vector org) _TE_tarexplosion = 
{
	EFF_PointEffect(org, TE_TAREXPLOSION, MULTICAST_PHS);
};

void(entity ent, vector start, vector end) _TE_lightning1 =
{
	EFF_BeamEffect (ent, start, end, TE_LIGHTNING1, MULTICAST_PHS);
};

void(entity ent, vector start, vector end) _TE_lightning2 =
{
	EFF_BeamEffect (ent, start, end, TE_LIGHTNING2, MULTICAST_PHS);
};

void(entity ent, vector start, vector end) _TE_lightning3 =
{
	EFF_BeamEffect (ent, start, end, TE_LIGHTNING3, MULTICAST_PHS);
};

void(vector org) TE_lightningblood =
{
#ifdef NETQUAKE
	particle(org, '0 0 100', 225, 120);
#else
	EFF_PointEffect(org, TE_LIGHTNINGBLOOD, MULTICAST_PVS);
#endif
};

// function pointers for TE calls
var void(vector org, float damage) SpawnBlood = _SpawnBlood;
var void(vector org) TE_explosion = _TE_explosion;
var void(vector org) TE_gunshot = _TE_gunshot;
var void(vector org) TE_spike = _TE_spike;
var void(vector org) TE_knightspike = _TE_knightspike;
var void(vector org) TE_wizspike = _TE_wizspike;
var void(vector org) TE_superspike = _TE_superspike;
var void(vector org) TE_teleport = _TE_teleport;
var void(vector org) TE_lavasplash = _TE_lavasplash;
var void(vector org) TE_tarexplosion = _TE_tarexplosion;
var void(entity ent, vector start, vector end) TE_lightning1 = _TE_lightning1;
var void(entity ent, vector start, vector end) TE_lightning2 = _TE_lightning2;
var void(entity ent, vector start, vector end) TE_lightning3 = _TE_lightning3;

// set effects function, takes function pointers and assigns them based on detected builtins
void() EFF_SetEffects =
{
	if (eng_support & ENG_EFRED)
	{
		ef_pent = 128;
		ef_red = 128;
	}

	if (eng_support & ENG_EFBLUE)
	{
		ef_blue = 64;
		ef_quad = 64;
	}

	if (eng_support & ENG_TEBUILTINS)
	{
		// use TE_ builtins instead
		TE_explosion = _te_explosion;
		TE_gunshot = _te_gunshot;
		TE_spike = _te_spike;
		TE_knightspike = _te_knightspike;
		TE_wizspike = _te_wizspike;
		TE_superspike = _te_superspike;
		TE_teleport = _te_teleport;
		TE_lavasplash = _te_lavasplash;
		TE_tarexplosion = _te_tarexplosion;
		TE_lightning1 = _te_lightning1;
		TE_lightning2 = _te_lightning2;
		TE_lightning3 = _te_lightning3;
	}

	if (eng_support & ENG_TEBLOOD)
		SpawnBlood = _SpawnBlood_TEBlood; // use TE_Blood builtin instead
};

// view kicks
void(entity ent) VK_smallkick =
{
#ifdef NETQUAKE
	self.punchangle_x = -2;
#else
	msg_entity = ent;
	WriteByte (MSG_ONE, SVC_SMALLKICK);
#endif
}

void(entity ent) VK_bigkick =
{
#ifdef NETQUAKE
	self.punchangle_x = -4;
#else
	msg_entity = ent;
	WriteByte (MSG_ONE, SVC_BIGKICK);
#endif
}
void(entity ent) VK_hugekick =
{
#ifdef NETQUAKE
	self.punchangle_x = -8;
#else
	msg_entity = ent;
	WriteByte (MSG_ONE, SVC_HUGEKICK);
#endif
}

// muzzle flash
void() muzzleflash =
{
#ifdef NETQUAKE
	//self.effects |= EF_MUZZLEFLASH;
#else
	WriteByte (MSG_MULTICAST, SVC_MUZZLEFLASH);
	WriteEntity (MSG_MULTICAST, self);
	multicast (self.origin, MULTICAST_PVS);
#endif
};

// touch blood functions
void(float damage) spawn_touchblood =
{
	local vector    vel;

	vel = normalize (self.velocity);
	vel = normalize(vel + v_up*(random()- 0.5) + v_right*(random()- 0.5));
	vel = vel + 2*trace_plane_normal;
	vel = vel * 0.4;
	SpawnBlood (self.origin + vel, damage);
}; 
/*
============================================
HUD weapon ammo counter. Counts ammo and displays 
Number of bullets.

============================================
*/
/*
==========================================

Gernade counter for the HUD. Displays a players Number 
Of nades.

==========================================
*/
void(entity client) NadeCounter =
{
other = self;
if (client.selnade == GRENADE_REGULAR)
	{
	stuffcmd (client, "cl_plasmanade 2\n");
	if (client.nade_lives == 4)
	stuffcmd (client, "cl_nadenum 5\n");
	if (client.nade_lives == 3)
	stuffcmd (client, "cl_nadenum 4\n");
	if (client.nade_lives == 2)
	stuffcmd (client, "cl_nadenum 3\n");
	if (client.nade_lives == 1)
	stuffcmd (client, "cl_nadenum 2\n");
	if (client.nade_lives == 0)
	stuffcmd (client, "cl_nadenum 1\n");
	}
else if (client.selnade == GRENADE_STICKY)
	{
	stuffcmd (client, "cl_plasmanade 1\n");
	if (client.plasma_lives == 4)
	stuffcmd (client, "cl_nadenum 5\n");
	if (client.plasma_lives == 3)
	stuffcmd (client, "cl_nadenum 4\n");
	if (client.plasma_lives == 2)
	stuffcmd (client, "cl_nadenum 3\n");
	if (client.plasma_lives == 1)
	stuffcmd (client, "cl_nadenum 2\n");
	if (client.plasma_lives == 0)
	stuffcmd (client, "cl_nadenum 1\n");
	}


};