void() normal_elite;
void() normal_grunt;
void() red_elite;
void() white_elite;
/*
---------------------------------------------------------
Chooses an Enemy to Spawn.
---------------------------------------------------------
*/
void() Spawn_enemy =
{
	local float r;
	r = floor(random()*5);

  	if (r == 1)
	normal_elite();
	
	else if (r == 2)
	normal_grunt();
	
	else if (r == 3)
	if (round < 5)
	normal_elite() || normal_grunt();
	else
	red_elite();
	
	else if (r == 4)
	if (round < 7)
	normal_elite() || normal_grunt() || red_elite();
	else
	white_elite();
	
	else if (r == 5)
	normal_grunt();
};

/*
---------------------------------------------------------
Sets Firefight Information up like lives and things.
---------------------------------------------------------
*/
void() Firefight_settings =
{
	CATCH = 0;
	F_LIVES = 10;
	Check_Round();
	Update_Lives();

	localcmd ("timelimit 60\n");
};
/*
---------------------------------------------------------
Updates Firefight HUD Info for Lives
---------------------------------------------------------
*/
void() Update_Lives =
{
	localcmd ("cl_life ");
	localcmd (ftos(F_LIVES));
	localcmd (" \n");
};
/*
---------------------------------------------------------
Updates Firefight HUD Info for Rounds
---------------------------------------------------------
*/
void() Update_Rounds =
{
	localcmd ("cl_round ");
	localcmd (ftos(round));
	localcmd (" \n");
};
/*
------------------------
Catch skull Activate
-----------------------
*/
void() Catch_On =
{
CATCH = 1;
localcmd ("cl_round 100\n");
};

/*
-----------------------------------
Round Settings. Updates the rounds.
-----------------------------------
*/
void() Check_Round =
{
local float r;
r = random();
if (round_kills == MAX_MONS)
{
if (r < 0.1)
{
F_LIVES += 1;
Update_Lives();
}
round += 1;
Update_Rounds();
MAX_MONS += 2;
TOTAL_MONS = 0;
round_kills = 0;
}

};
void() normal_elite =
{
	local entity oldself;
	
	if (self.enemy != world)
	{
        if (self.enemy.health <= 0)
		{
            self.enemy = world;
		}
	}
	if (self.enemy == world)
	{ 
	oldself = self;
	self = spawn ();
	self.classname = "Needle-spikes";
      setorigin (self, oldself.origin);
      self.origin_z = self.origin_z + 2;
      self.solid = SOLID_SLIDEBOX;
      self.movetype = MOVETYPE_STEP;
	setmodel (self, "progs/elite.mdl");
	self.skin = 0;
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
      self.health = 150;
	self.elite = 0;	
	self.selnade = GRENADE_STICKY;
	self.th_stand = elite_stand1;
	self.th_walk = elite_walk1;
	self.th_run = elite_run1;
	self.th_die = elite_bdie1;
	self.th_melee = elite_melee;
	self.th_missile = elite_attack;
	self.th_pain = elite_pain;

        if (oldself.target != string_null)
        {
            self.target = oldself.target;
        }

        walkmonster_start();
	
	  oldself.enemy = self;
	  self = oldself;
		TOTAL_MONS += 1;		//Adds a monster to the world, so that waves can be set.
	}

};

void() red_elite =
{
	local entity oldself;
	
	if (self.enemy != world)
	{
        if (self.enemy.health <= 0)
		{
            self.enemy = world;
		}
	}
	if (self.enemy == world)
	{ 
	oldself = self;
	self = spawn ();
	self.classname = "Needle-spikes";
      setorigin (self, oldself.origin);
      self.origin_z = self.origin_z + 2;
      self.solid = SOLID_SLIDEBOX;
      self.movetype = MOVETYPE_STEP;
	setmodel (self, "progs/elite.mdl");
	self.skin = 1;
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
      self.health = 150;
	self.elite = 1;		//States what Elite type it is, so Different stats/weapons can be set.
	self.selnade = GRENADE_STICKY;
	self.th_stand = elite_stand1;
	self.th_walk = elite_walk1;
	self.th_run = elite_run1;
	self.th_die = elite_bdie1;
	self.th_melee = elite_melee;
	self.th_missile = elite_attack;
	self.th_pain = elite_pain;

        if (oldself.target != string_null)
        {
            self.target = oldself.target;
        }

        walkmonster_start();
	
	  oldself.enemy = self;
	  self = oldself;
	TOTAL_MONS += 1;
	}

};
void() white_elite =
{
	local entity oldself;
	
	if (self.enemy != world)
	{
        if (self.enemy.health <= 0)
		{
            self.enemy = world;
		}
	}
	if (self.enemy == world)
	{ 
	oldself = self;
	self = spawn ();
	self.classname = "Needle-spikes";
      setorigin (self, oldself.origin);
      self.origin_z = self.origin_z + 2;
      self.solid = SOLID_SLIDEBOX;
      self.movetype = MOVETYPE_STEP;
	setmodel (self, "progs/elite.mdl");
	self.skin = 2;
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
      self.health = 170;
	self.elite = 2;
	self.selnade = GRENADE_STICKY;
	self.th_stand = elite_stand1;
	self.th_walk = elite_walk1;
	self.th_run = elite_run1;
	self.th_die = elite_bdie1;
	self.th_melee = elite_melee;
	self.th_missile = elite_attack;
	self.th_pain = elite_pain;

        if (oldself.target != string_null)
        {
            self.target = oldself.target;
        }

        walkmonster_start();
	
	  oldself.enemy = self;
	  self = oldself;
	TOTAL_MONS += 1;
	}

};

void() normal_grunt =
{
	local entity oldself;
	
	if (self.enemy != world)
	{
        if (self.enemy.health <= 0)
		{
            self.enemy = world;
		}
	}
	if (self.enemy == world)
	{ 
	oldself = self;
	self = spawn ();
	self.classname = "Needle-spikes";
      setorigin (self, oldself.origin);
      self.origin_z = self.origin_z + 2;
      self.solid = SOLID_SLIDEBOX;
      self.movetype = MOVETYPE_STEP;
	setmodel (self, "progs/soldier.mdl");
	self.skin = 0;
	self.elite = 1;		//For Melee damage
	self.selnade = GRENADE_STICKY;
	setsize (self, VEC_HULL_MIN, VEC_HULL_MAX);
      self.health = 50;
	self.th_stand = grunt_stand1;
	self.th_walk = grunt_walk1;
	self.th_run = grunt_run1;
	self.th_missile = grunt_attack;
	self.th_pain = grunt_pain;
	self.th_die = grunt_die;
	self.th_melee = grunt_melee;

        if (oldself.target != string_null)
        {
            self.target = oldself.target;
        }

        walkmonster_start();
	
	  oldself.enemy = self;
	  self = oldself;
	TOTAL_MONS += 1;
	}

};