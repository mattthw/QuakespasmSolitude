void() Start_Weapon;

void() Setup_Gametype =
{
	local   entity spot;
	local float wtemp;

Slowmo_Deactivate();
   // oh, this is a hack!
   setmodel (self, "progs/eyes.mdl");
   modelindex_eyes = self.modelindex;

   //vwep start
   setmodel (self, "progs/player_nail.mdl"); //vwep
   modelindex_playernail = self.modelindex;
   setmodel (self, "progs/player_shot.mdl"); //vwep
   modelindex_playershot = self.modelindex;
   setmodel (self, "progs/player_snail.mdl"); //vwep
   modelindex_playersnail = self.modelindex;
   setmodel (self, "progs/player_sshot.mdl"); //vwep
   modelindex_playersshot = self.modelindex;
   setmodel (self, "progs/player_needler.mdl"); //vwep
   modelindex_playerneedler = self.modelindex;
   setmodel (self, "progs/player_rocket.mdl"); //vwep
   modelindex_playerrocket = self.modelindex;
   setmodel (self, "progs/player_sniper.mdl"); //vwep
   modelindex_playersniper = self.modelindex;
   setmodel (self, "progs/player_plasma.mdl"); //vwep
   modelindex_playerplasma = self.modelindex;
   //vwep end 

	if (self.ishuman == TRUE)
	stuffcmd (self, "r_drawviewmodel 1\n");

switch(deathmatch)
	{
/*
==============================================================
Gametype: Slayer
==============================================================
*/
	case SLAYER:
	{	
	self.classname = "player";
	self.health = 130;
	self.takedamage = DAMAGE_AIM;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_WALK;
	self.max_health = 130;
	self.flags = FL_CLIENT;
	self.air_finished = time + 12;
	self.waterdmg = 2; // initial water damage
	self.effects = 0;
	self.suicide_time = time + 3;
	self.weaponstate = WS_IDLE;
	DecodeLevelParms ();
	wtemp = self.weapon;
	self.weapon = 0;
	W_WeaponSwitch (wtemp);
	self.attack_finished = time;
	self.th_pain = player_pain;
	self.th_die = PlayerDie;
	self.deadflag = DEAD_NO;
	spot = SelectSpawnPoint ();
	Start_Weapon ();
	self.origin = spot.origin + '0 0 1';
	self.angles = spot.angles;
	self.fixangle = TRUE;      
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
	self.view_ofs = '0 0 22';
	self.velocity = '0 0 0';
	player_stand1 ();
	makevectors(self.angles);
	spawn_tfog (self.origin + v_forward*20);
	spawn_tdeath (self.origin, self);
	rj = numberserverinfokey("rj");
	self.needles = 0;
	self.pfov = FOV_DEFAULT;
	Set_FOV( self.pfov );
	//self.reset = 1.00;
	self.weaponheat = 0;		
	stuffcmd(self,"chase_active 0\n");
	NadeCounter(self);
	}
	break;
	
/*
==============================================================
Gametype: Firefight
==============================================================
*/
	case FIREFIGHT:
	{
	self.classname = "player";
	self.health = 130;
	self.takedamage = DAMAGE_AIM;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_WALK;
	self.max_health = 130;
	self.flags = FL_CLIENT;
	self.air_finished = time + 12;
	self.waterdmg = 2; // initial water damage
	self.effects = 0;
	self.suicide_time = time + 3;
	self.weaponstate = WS_IDLE;
	DecodeLevelParms ();
	wtemp = self.weapon;
	self.weapon = 0;
	W_WeaponSwitch (wtemp);
	self.attack_finished = time;
	self.th_pain = player_pain;
	self.th_die = PlayerDie;
	self.deadflag = DEAD_NO;
	spot = SelectSpawnPoint ();
	Start_Weapon ();
	self.origin = spot.origin + '0 0 1';
	self.angles = spot.angles;
	self.fixangle = TRUE;      
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
	self.view_ofs = '0 0 22';
	self.velocity = '0 0 0';
	player_stand1 ();
	makevectors(self.angles);
	spawn_tfog (self.origin + v_forward*20);
	spawn_tdeath (self.origin, self);
	rj = numberserverinfokey("rj");
	self.needles = 0;
	self.pfov = FOV_DEFAULT;
	Set_FOV( self.pfov );
	//self.reset = 1.00;		
	self.weaponheat = 0;		
	stuffcmd(self,"chase_active 0\n");
	NadeCounter(self);
	}
	break;
/*
==============================================================
Gametype: Swat
==============================================================
*/
	case SWAT:
	{
	self.classname = "player";
	self.health = 30;
	self.takedamage = DAMAGE_AIM;
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_WALK;
	self.max_health = 30;
	self.flags = FL_CLIENT;
	self.air_finished = time + 12;
	self.waterdmg = 2; // initial water damage
	self.effects = 0;
	self.suicide_time = time + 3;
	self.weaponstate = WS_IDLE;
	DecodeLevelParms ();
	wtemp = self.weapon;
	self.weapon = 0;
	self.slowmo = 5;
	MAX_SLOWMO = 8;
	self.slowmo_flags = SLOWMO_UNABLE;
	W_WeaponSwitch (wtemp);
	self.attack_finished = time;
	self.th_pain = player_pain;
	self.th_die = PlayerDie;
	self.deadflag = DEAD_NO;
	spot = SelectSpawnPoint ();
	Start_Weapon ();
	self.origin = spot.origin + '0 0 1';
	self.angles = spot.angles;
	self.fixangle = TRUE;      
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
	self.view_ofs = '0 0 22';
	self.velocity = '0 0 0';
	player_stand1 ();
	makevectors(self.angles);
	spawn_tfog (self.origin + v_forward*20);
	spawn_tdeath (self.origin, self);
	rj = numberserverinfokey("rj");
	self.needles = 0;
	self.pfov = FOV_DEFAULT;
	Set_FOV( self.pfov );
	//self.reset = 1.00;		
	self.weaponheat = 0;		
	stuffcmd(self,"chase_active 0\n");
	self.nade_lives = 0;
	self.plasma_lives = 0;
	NadeCounter(self);
	Update_Slowmo(self);
	localcmd ("fraglimit 50\n");
	}
	break;
		}
};

void() Start_Weapon =
{
switch (deathmatch)
	{
/*
==============================================================
Gametype: Slayer [Pistol & Assault Rifle]
==============================================================
*/
	case SLAYER:
	{
	self.items = WEAPON_AR + WEAPON_PISTOL;
	self.ammo_nails = W_GetMaxClip(self.weapon, 4);
	self.exnails = 560;
	self.ammo_shells = 12;
	self.exshells = 60;
	self.currentammo = W_GetMaxClip(self.weapon, 4);
	self.weapon = WEAPON_AR;
	W_UpdateWeapon();
	W_UpdateAmmoCounts(self);
	}
	break;

/*
==============================================================
Gametype: Firefight [SMG]
==============================================================
*/
	case FIREFIGHT:
	{
	self.items = WEAPON_UZI;
	self.ammo_hshells = 60;
	self.exhshells = 320;
	self.weapon = WEAPON_UZI;
	W_UpdateWeapon();
	W_UpdateAmmoCounts(self);
	}
	break;

/*
==============================================================
Gametype: Swat [Pistol]
==============================================================
*/
	case SWAT:
	{
	self.items = WEAPON_PISTOL;
	self.ammo_shells = 12;
	self.exshells = 9999999;
	self.weapon = WEAPON_PISTOL;
	W_UpdateWeapon();
	W_UpdateAmmoCounts(self);
	}
	break;
	}
};