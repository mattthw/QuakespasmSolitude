/* ALL LIGHTS SHOULD BE 0 1 0 IN COLOR ALL OTHER ITEMS SHOULD
BE .8 .3 .4 IN COLOR */
void () W_SetCurrentAmmo;
void (float weap) W_WeaponSwitch;
void (entity ent) W_UpdateAmmoCounts;
void (entity client) pre_player_pull;

void() SUB_regen =
{
	self.model = self.mdl;          // restore original model
	self.solid = SOLID_TRIGGER;     // allow it to be touched again
	sound (self, CHAN_VOICE, "items/itembk2.wav", 1, ATTN_NORM);    // play respawn sound
	setorigin (self, self.origin);
};



/*
============
PlaceItem

plants the object on the floor
============
*/
void() PlaceItem =
{
	local float     oldz;

	self.mdl = self.model;          // so it can be restored on respawn
	self.flags = FL_ITEM;           // make extra wide
	self.solid = SOLID_TRIGGER;
	self.movetype = MOVETYPE_TOSS;  
	self.velocity = '0 0 0';
	self.origin_z = self.origin_z + 6;
	oldz = self.origin_z;
	if (!droptofloor())
	{
		dprint ("Bonus item fell out of level at ");
		dprint (vtos(self.origin));
		dprint ("\n");
		remove(self);
		return;
	}
};

/*
============
StartItem

Sets the clipping size and plants the object on the floor
============
*/
void() StartItem =
{
	self.nextthink = time + 0.2;    // items start after other solids
	self.think = PlaceItem;
};

/*
===============================================================================

WEAPONS

===============================================================================
*/

void() bound_other_ammo =
{
	if (other.ammo_shells_real > 100)
		other.ammo_shells_real = 100;
	if (other.ammo_nails_real > 200)
		other.ammo_nails_real = 200;
	if (other.ammo_rockets_real > 100)
		other.ammo_rockets_real = 100;               
	if (other.ammo_cells_real > 100)
		other.ammo_cells_real = 100;         

	if ((other.exshells > 100))
	{
		other.exshells = 100;
	}
	if ((other.exnails > 200))
	{
		other.exnails = 200;
	}
	if ((other.exrockets > 100))
	{
		other.exrockets = 100;
	}
	if ((other.excells > 100))
	{
		other.excells = 100;
	}
};


/*
=============
weapon_touch
=============
*/
float() W_BestWeapon;
void() weapon_touch;		// sol_items.qc




/*
===============================================================================

AMMO

===============================================================================
*/

#define WEAPON_SHOTGUN 	1
float WEAPON_ROCKET = 2;
float WEAPON_SPIKES = 4;
float WEAPON_MASK = 7;
float WEAPON_BIG = 8;

/*
===============================================================================

PLAYER BACKPACKS

===============================================================================
*/

void () BackpackTouch =
{
	local string s;
	local float best;
	local float old;
	local float new;
	local entity stemp;
	local float acount;

	if ((other.classname != "player"))
	{
		return;
	}
	if ((other.health <= MSG_BROADCAST))
	{
		return;
	}
	acount = MSG_BROADCAST;
	sprint (other, "You get ");
	if (self.items)
	{
		if (((other.items & self.items) == MSG_BROADCAST))
		{
			acount = WEAPON_SHOTGUN;
			sprint (other, "the ");
			sprint (other, self.netname);
		}
	}
	stemp = self;
	self = other;
	best = W_BestWeapon ();
	self = stemp;
	other.exshells = (other.exshells + self.exshells);
	other.exnails = (other.exnails + self.exnails);
	other.exrockets = (other.exrockets + self.exrockets);
	other.excells = (other.excells + self.excells);
	new = self.items;
	if (!new)
	{
		new = other.weapon;
	}
	old = other.items;
	other.items = (other.items | new);
	bound_other_ammo ();
	if (self.exshells)
	{
		if (acount)
		{
			sprint (other, ", ");
		}
		acount = WEAPON_SHOTGUN;
		s = ftos (self.exshells);
		sprint (other, s);
		sprint (other, " shells");
	}
	if (self.exnails)
	{
		if (acount)
		{
			sprint (other, ", ");
		}
		acount = WEAPON_SHOTGUN;
		s = ftos (self.exnails);
		sprint (other, s);
		sprint (other, " nails");
	}
	if (self.exrockets)
	{
		if (acount)
		{
			sprint (other, ", ");
		}
		acount = WEAPON_SHOTGUN;
		s = ftos (self.exrockets);
		sprint (other, s);
		sprint (other, " rockets");
	}
	if (self.excells)
	{
		if (acount)
		{
			sprint (other, ", ");
		}
		acount = WEAPON_SHOTGUN;
		s = ftos (self.excells);
		sprint (other, s);
		sprint (other, " cells");
	}
	sprint (other, "\n");
	sound (other, CHAN_ITEM, "weapons/lock4.wav", WEAPON_SHOTGUN, ATTN_NORM);
	stuffcmd (other, "bf\n");
	remove (self);
	self = other;
};

void () DropBackpack =
{
	local entity item;

	if (!(((self.exshells + self.exnails) + self.exrockets) + self.excells))
	{
		return;
	}
	item = spawn ();
	item.origin = (self.origin - '0 0 24');
	item.items = self.weapon;
	if ((item.items == IT_AXE))
	{
		item.netname = "Axe";
	}
	else
	{
		if ((item.items == IT_SHOTGUN))
		{
			item.netname = "Pistol";
		}
		else
		{
			if ((item.items == IT_SUPER_SHOTGUN))
			{
				item.netname = "SMG";
			}
			else
			{
				if ((item.items == IT_NAILGUN))
				{
					item.netname = "Rifle";
				}
				else
				{
					if ((item.items == IT_SUPER_NAILGUN))
					{
						item.netname = "Super Nailgun";
					}
					else
					{
						if ((item.items == IT_GRENADE_LAUNCHER))
						{
							item.netname = "Grenade Launcher";
						}
						else
						{
							if ((item.items == IT_ROCKET_LAUNCHER))
							{
								item.netname = "Rocket Launcher";
							}
							else
							{
								if ((item.items == IT_LIGHTNING))
								{
									item.netname = "Thunderbolt";
								}
								else
								{
									item.netname = "";
								}
							}
						}
					}
				}
			}
		}
	}
	item.exshells = self.exshells;
	item.exnails = self.exnails;
	item.exrockets = self.exrockets;
	item.excells = self.excells;
	item.velocity_z = 300;
	item.velocity_x = (-100 + (random () * 200));
	item.velocity_y = (-100 + (random () * 200));
	item.flags = FL_ITEM;
	item.solid = SOLID_TRIGGER;
	item.movetype = MOVETYPE_TOSS;
	setmodel (item, "progs/backpack.mdl");
	setsize (item, '-16 -16 0', '16 16 56');
	item.touch = BackpackTouch;
	item.nextthink = (time + 120);
	item.think = SUB_Remove;
};

/*
========================
Grenades By mexicouger
------------------------
*/
void() plasma_nade_touch =
{
if (other.classname != "player")
		return;
	
	if (other.plasma_lives >= 4) {
	}
	
	else {
	other.plasma_lives += 1;
	NadeCounter(other);
	sprint (other, "picked up a Plasma grenade\n");
	sound (self, CHAN_WEAPON, "weapons/pbounce.wav", 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.mdl = self.model;
	self.model = string_null;
	self.solid = SOLID_NOT;
	
	if (deathmatch) {
	self.nextthink = time + 20;
	self.think = SUB_regen;
	}
	activator = other;
	SUB_UseTargets();           
}                    
};

void() nade_touch =
{
if (other.classname != "player")
		return;
	
	if (other.nade_lives >= 4) {
	}
	
	else {
	other.nade_lives += 1;
	NadeCounter(other);
	sprint (other, "picked up a Frag grenade\n");
	sound (self, CHAN_WEAPON, "weapons/grenpick.wav", 1, ATTN_NORM);
	stuffcmd (other, "bf\n");
	self.mdl = self.model;
	self.model = string_null;
	self.solid = SOLID_NOT;
	
	if (deathmatch) {
	self.nextthink = time + 20;
	self.think = SUB_regen;
	}
	activator = other;
	SUB_UseTargets();           
}                    
};

void() pod_touch =
{
if (other.classname != "player")
		return;
	if (other.impulse == 13)
	{
	other.ammo_nails = 60;
	other.exnails = 560;
	other.weapon = WEAPON_AR;
	W_UpdateAmmoCounts(other);
	pre_player_pull(other);
	
	stuffcmd (other, "bf\n");
	self.mdl = self.model;
	self.model = string_null;
	self.solid = SOLID_NOT;
	
	if (deathmatch) {
	self.nextthink = time + 20;
	self.think = SUB_regen;
	}
	activator = other;
	SUB_UseTargets();    
	}                    
};

void() item_nade =
{
	if (deathmatch == SWAT)
	remove(self);
	self.owner = other;
	self.touch = nade_touch;
	setmodel(self, "progs/fraggren.mdl");
	setsize (self, '0 0 0', ' 2 2 2');
	StartItem();
};

void() item_plasma_nade =
{
	if (deathmatch == SWAT)
	remove(self);
	self.owner = other;
	self.touch = plasma_nade_touch;
	setmodel(self, "progs/plasgren.mdl");
	setsize (self, '0 0 0', ' 2 2 2');
	StartItem();
};

void() item_weapon_pod =
{
	if (deathmatch != FIREFIGHT)
	remove(self);
	self.mdl = self.model;
	self.movetype = MOVETYPE_NONE;
	self.solid = SOLID_BBOX;
	self.owner = other;
	self.touch = pod_touch;	
	self.velocity = '0 0 0';
	self.origin_z = self.origin_z + 6;
	setmodel(self, "progs/pod.mdl");
	setsize (self, '10 10 10', '10 10 10');
};


