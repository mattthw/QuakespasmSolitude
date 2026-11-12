// Solitude Items: weapons, ammo, misc.

// GetPlayerWeapons: 
// return how many weapons a player is holding.
// also used for finding out if the player is holding a certain weapon.
float (float pweapons) GetPlayerWeapons =
{
	local float i;
	
	if (pweapons & WEAPON_PISTOL)
		i++;
	if (pweapons & WEAPON_GHOST)
		i++;
	if (pweapons & WEAPON_FLAG)
		i++;
	if (pweapons & WEAPON_SKULL)
		i++;
	if (pweapons & WEAPON_SHOT)
		i++;
	if (pweapons & WEAPON_AR)
		i++;
	if (pweapons & WEAPON_UZI)
		i++;
	if (pweapons & WEAPON_RL)
		i++;
	if (pweapons & WEAPON_SNIPER)
		i++;
	if (pweapons & WEAPON_PPISTOL)
		i++;
	if (pweapons & WEAPON_SWORD)
		i++;
	if (pweapons & WEAPON_NEEDLER)
		i++;
	if (pweapons & WEAPON_PRIFLE)
		i++;
    if (pweapons & WEAPON_BR)
        i++;
/*
	if (pweapons & WEAPON_PISTOL)
		i++;
	if (pweapons & WEAPON_PISTOL)
		i++;
*/
	return i;
};

// Returns the amount of ammo an entity or player has either in the primary or secondary clip
// PS. It's hackish, but it works :)
float (entity t_plyr, float t_wep, float cliptype) GetAmmoCount =
{
	local float oldweapon, rtval;
	
	oldweapon = t_plyr.weapon;
	t_plyr.weapon = t_wep;
	W_UpdateAmmoCounts( t_plyr );
	if (cliptype)
		rtval = t_plyr.currentammo;
	else
		rtval = t_plyr.armorvalue;
		
	t_plyr.weapon = oldweapon;
	W_UpdateAmmoCounts( t_plyr );
	
	return rtval;
};

// Sets the ammo of a new or dropped weapon, as opposed to the function below which...
void ( entity t_wep, float weptype ) SetWeaponAmmo =
{
	local float stammo, stotal;
    stammo = W_GetMaxClip( weptype, 1 );
    stotal = W_GetMaxClip( weptype, 0 );
	switch( weptype ) {
		case WEAPON_PISTOL:
			t_wep.exshells = stotal;
			t_wep.ammo_shells = stammo;
			break;
		case WEAPON_GHOST:
			t_wep.exshells = stotal; //0
			t_wep.ammo_shells = stammo;
			break;
		case WEAPON_FLAG:
			t_wep.exshells = stotal; //0
			t_wep.ammo_shells = stammo;
			break;
		case WEAPON_SKULL:
			t_wep.exshells = stotal; //0
			t_wep.ammo_shells = stammo;
			break;
		case WEAPON_SHOT:
			t_wep.exshells2 = stotal;
			t_wep.ammo_shells2 = stammo;
			break;
		case WEAPON_AR:
			t_wep.exnails = stotal;
			t_wep.ammo_nails = stammo;
			break;
		case WEAPON_UZI:
			t_wep.exhshells = stotal;
			t_wep.ammo_hshells = stammo;
			break;
		case WEAPON_RL:
			t_wep.exrockets = stotal;
			t_wep.ammo_rockets = stammo;
			break;
		case WEAPON_SNIPER:
			t_wep.exsniper = stotal;
			t_wep.ammo_sniper = stammo;
			break;
		case WEAPON_PPISTOL:
			t_wep.exppistol = stotal;
			t_wep.ammo_ppistol = stammo;
			break;
		case WEAPON_SWORD:
			t_wep.exppistol = stotal;
			t_wep.ammo_ppistol = stammo;
			break;
		case WEAPON_NEEDLER:
			t_wep.exneedler = stotal;
			t_wep.ammo_needler = stammo;
			break;
		case WEAPON_PRIFLE:
			t_wep.exprifle = stotal;
			t_wep.ammo_prifle = stammo;
			break;			
        case WEAPON_BR:
            t_wep.exbr = stotal;
            t_wep.ammo_br = stammo;
            break;
		default:
			break;
	}
};

// sets ammo to a specific clip aspect of a weapon (current clip or extended clip)
void ( entity t_ent, float weptype, float cliptype, float newamt ) SetAmmoFor =
{
    local float curr_ammo = GetAmmoCount(t_ent, weptype, cliptype);
    local float max_clip = W_GetMaxClip(weptype, cliptype);

//    _bprint("curr ammo: ", ftos(curr_ammo), "\n");
//    _bprint("newamt: ", ftos(newamt), "\n");
//    _bprint("max clip: ", ftos(max_clip), "\n");

	if ((curr_ammo + newamt) >  max_clip) {
//        _bprint("new amount ", ftos(newamt), " exceeds max clip. correcting... \n");
        newamt = max_clip - curr_ammo;
        if (newamt < 0)
            newamt = 0;
	}
    // fix (bandaid patch?) shotgun bug, where we get stuck in a loop of negative ammo after
    // switching from the SMG.
    if (curr_ammo < 0) {
//        _bprint("new curr_ammo ", ftos(newamt), " is invalid! correcting... \n");
        newamt = (-curr_ammo) + newamt;
    }

	switch ( weptype ) {
		case WEAPON_PISTOL:
			if (!cliptype || cliptype > 1)
				t_ent.exshells += newamt;
			if (cliptype)
				t_ent.ammo_shells += newamt;
			break;
		case WEAPON_GHOST:
			if (!cliptype || cliptype > 1)
				t_ent.exshells += newamt;
			if (cliptype)
				t_ent.ammo_shells += newamt;
			break;
		case WEAPON_FLAG:
			if (!cliptype || cliptype > 1)
				t_ent.exshells += newamt;
			if (cliptype)
				t_ent.ammo_shells += newamt;
			break;
		case WEAPON_SKULL:
			if (!cliptype || cliptype > 1)
				t_ent.exshells += newamt;
			if (cliptype)
				t_ent.ammo_shells += newamt;
			break;
		case WEAPON_SHOT:
			if (!cliptype || cliptype > 1)
				t_ent.exshells2 += newamt;
			if (cliptype)
				t_ent.ammo_shells2 += newamt;
			break;
		case WEAPON_AR:
			if (!cliptype || cliptype > 1)
				t_ent.exnails += newamt;
			if (cliptype)
				t_ent.ammo_nails += newamt;
			break;
		case WEAPON_UZI:
			if (!cliptype || cliptype > 1)
				t_ent.exhshells += newamt;
			if (cliptype)
				t_ent.ammo_hshells += newamt;
			break;
		case WEAPON_RL:
			if (!cliptype || cliptype > 1)
				t_ent.exrockets += newamt;
			if (cliptype)
				t_ent.ammo_rockets += newamt;
			break;
		case WEAPON_SNIPER:
			if (!cliptype || cliptype > 1)
				t_ent.exsniper += newamt;
			if (cliptype)
				t_ent.ammo_sniper += newamt;
			break;
		case WEAPON_PPISTOL:
			if (!cliptype || cliptype > 1)
				t_ent.exppistol += newamt;
			if (cliptype)
				t_ent.ammo_ppistol += newamt;
			break;
		case WEAPON_SWORD:
			if (!cliptype || cliptype > 1)
				t_ent.exsword += newamt;
			if (cliptype)
				t_ent.ammo_sword += newamt;
			break;
		case WEAPON_NEEDLER:
			if (!cliptype || cliptype > 1)
				t_ent.exneedler += newamt;
			if (cliptype)
				t_ent.ammo_needler += newamt;
			break;
		case WEAPON_PRIFLE:
			if (!cliptype || cliptype > 1)
				t_ent.exprifle += newamt;
			if (cliptype)
				t_ent.ammo_prifle += newamt;
			break;			
        case WEAPON_BR:
            if ((!cliptype || (cliptype > 1)))
            {
                t_ent.exbr = (t_ent.exbr + newamt);
            }
            if (cliptype)
            {
                t_ent.ammo_br += newamt;
            }
            break;
		default:
			_bprint("SetAmmoFor: invalid weapon type specified\n");
			break;
	}
};

void ( entity t_from, entity t_to, float wtype, float aremove ) CopyAmmo =
{
	W_UpdateAmmoCounts( t_from );
	SetAmmoFor( t_to, wtype, 0, t_from.armorvalue );
	SetAmmoFor( t_to, wtype, 1, t_from.currentammo );

	if (aremove == 1) {
		SetAmmoFor( t_from, wtype, 0, (-1 * t_from.armorvalue) );
		SetAmmoFor( t_from, wtype, 1, (-1 * t_from.currentammo) );
	}
};

void () weapon_becometouch =
{
	self.think = SUB_Remove;
	self.nextthink = time + 30;
	self.touch = weapon_touch;
};

string ( float wtype ) weapon_getlaymodel =
{
	switch (wtype)
	{
		case WEAPON_SHOT:
			return ("progs/g_shot.mdl");
			break;
		case WEAPON_PISTOL:
			return ("progs/g_pistol.mdl");
			break;
		case WEAPON_GHOST:
			return ("progs/g_ghost.mdl");
			break;
		case WEAPON_FLAG:
			return ("progs/g_flag.mdl");
			break;
		case WEAPON_SKULL:
			return ("progs/g_skull.mdl");
			break;
		case WEAPON_AR:
			return ("progs/g_nail.mdl");
			break;
		case WEAPON_UZI:
			return ("progs/g_uzi.mdl");
			break;
		case WEAPON_RL:
			return ("progs/g_rocket.mdl");
			break;
		case WEAPON_SNIPER:
			return ("progs/g_sniper.mdl");
			break;
		case WEAPON_PPISTOL:
			return ("progs/g_plpist.mdl");
			break;
		case WEAPON_SWORD:
			return ("progs/g_axe.mdl");
			break;
		case WEAPON_NEEDLER:
			return ("progs/g_needler.mdl");
			break;
		case WEAPON_PRIFLE:
			return ("progs/g_plrifl.mdl");
			break;
        case WEAPON_BR:
            return ("progs/g_nail.mdl");
            break;
		default:
			return ("progs/player.mdl");
			break;
	}
};
	
float drop_time;

void() t_sound =
{
    local float r;

    if ((drop_time > time))
    {
        return;
    }

    if ((other.classname == "player"))
    {
        return;
    }

    r = floor((random() * 3));
    if ((r == 0))
    {
        sound(self, 0, "weapons/w_drop1.wav", 0.7, 1);
    }
    else if ((r == 1))
    {
        sound(self, 0, "weapons/w_drop2.wav", 0.7, 1);
    }
    else if ((r == 2))
    {
        sound(self, 0, "weapons/w_drop3.wav", 0.7, 1);
    }

    drop_time = time + 0.1;
};
void (float wtype, float ttype) ThrowWeapon =
{
	local entity newthrow;
	newthrow = spawn();
    newthrow.origin = ((self.origin + '0 0 16') + (v_right * CONTENT_SKY));
    setmodel(newthrow, weapon_getlaymodel(wtype));
    setsize(newthrow, VEC_ORIGIN, VEC_ORIGIN);
    makevectors(self.v_angle);
    if (self.v_angle_x)
    {
        newthrow.velocity = ((((v_forward * 45) + (v_up * 200)) + (((2 * (random() + -0.5)) * v_right) * 10)) + (((2 * (random() + -0.5)) * v_up) * 10));
    }
    else
    {
        newthrow.velocity = aim(self, 10000);
        newthrow.velocity = (newthrow.velocity * 45);
        newthrow.velocity_z = 200;
    }
	newthrow.owner = self;
	newthrow.movetype = MOVETYPE_BOUNCE;
	newthrow.solid = SOLID_TRIGGER;
    newthrow.touch = t_sound; //copied over
	newthrow.classname = "weapon_pickup";
	newthrow.flags = FL_ITEM;
    newthrow.gravity = 5; // copied over
	newthrow.think = weapon_becometouch;
	newthrow.nextthink = time + 1;
	
	newthrow.weapon = wtype;
	
	if (ttype == 1)	{	// throwing an existing weapon
		CopyAmmo(self, newthrow, wtype, 1);
		newthrow.lives = 1;
	}
	else
    {
		SetWeaponAmmo(newthrow, wtype);
	}
};

// respawn the weapon
void () weapon_activate =
{
	if (self.lives == 1) {
		remove( self );
		return;
	}
	if (self.mdl)
		setmodel(self, self.mdl);
	self.touch = weapon_touch;
}

void () weapon_touch =
{
	local entity oself;
	local float addweapon, newweapon;
	
	oself = self;
	//

	if (!(other.flags & FL_CLIENT))
		return;

	if (other.ishuman != #TRUE)
		if (other.attack_finished < time)
			other.canpickup_time = time + .1;
		
	self.mdl = self.model;
		
	if ( (other.items & self.weapon) ) {
		if (self.weapon & WEAPON_PPISTOL || self.weapon & WEAPON_PRIFLE)				// weapons that don't have an extended clip
			if ( W_GetMaxClip(self.weapon, 1) <= GetAmmoCount(other, self.weapon, 1) )
				return;
	
		if ( W_GetMaxClip(self.weapon, 0) <= GetAmmoCount(other, self.weapon, 0) )
			return;

		W_UpdateAmmoCounts( self );
		if (self.weapon & WEAPON_PPISTOL || self.weapon & WEAPON_PRIFLE)
			SetAmmoFor( other, self.weapon, 1, self.currentammo );
		else
			SetAmmoFor( other, self.weapon, 0, self.armorvalue );
		//SetAmmoFor( other, self.weapon, 1, self.currentammo );
		
		W_UpdateAmmoCounts( other );
		self.touch = SUB_Null;
		setmodel( self, string_null );
		self.nextthink = time + 30;
		self.think = weapon_activate;
		return;
	}
	
	if (other.canpickup_time > time) {
		if (other.weapon == self.weapon || GetPlayerWeapons(other.items) >= 2 ) {
			addweapon = 2;		// drop current weapon and replace it
        } else {
			addweapon = 1;		// add other weapon and switch to it
		}

		other.canpickup_time = 0;
	}
	else {
		if (time > other.pickup_time) {

//			if ( GetPlayerWeapons(other.items) == 2 )
//				centerprint(other, "Press reload button to swap for this weapon\n");
//			else
//				centerprint(other, "Press reload button to pick up weapon\n");
			other.pickup_time = time + .5;
			return;
		}
	}
	
	if (other.ishuman == TRUE && other.classname == "player")
    {
        if (icon_time > time)
        stuffcmd (other, "cl_ww 0\n");
        else
            {
        if (time > icon_time)
        icon_time = time + 1;
        other.canpick = 1;
        return;
            }
        if (self.weapon == WEAPON_AR)	//Assault rifle
        {
        stuffcmd (other, "cl_ww 1\n");
        }

        else if (self.weapon == WEAPON_PPISTOL)//Plasma Pistol
        {
        stuffcmd (other, "cl_ww 2\n");
        }

        else if (self.weapon == IT_SUPER_SHOTGUN)	//Shootie
        {
        stuffcmd (other, "cl_ww 3\n");
        }

        else if (self.weapon == WEAPON_NEEDLER)
        {
        stuffcmd (other, "cl_ww 4\n");
        }

        else if (self.weapon == IT_ROCKET_LAUNCHER)
        {
        stuffcmd (other, "cl_ww 5\n");
        }

        else if (self.weapon == WEAPON_PISTOL)
        {
        stuffcmd (other, "cl_ww 6\n");
        }

        else if (self.weapon == WEAPON_UZI)
        {
        stuffcmd (other, "cl_ww 7\n");
        }

        else if (self.weapon == WEAPON_SNIPER)
        {
        stuffcmd (other, "cl_ww 8\n");
        }
        else if ((self.weapon == WEAPON_BR))
        {
            stuffcmd(other, "cl_ww 9\n");
        }
        else if (self.weapon == WEAPON_SWORD) {
            stuffcmd(other, "cl_ww 10\n");
        }
        else if (self.weapon == WEAPON_PRIFLE) {
            stuffcmd(other, "cl_ww 11\n");
        }
    }

	other.canpick = 0;
	if (addweapon)
	{
		other.attack_finished = time + .1;	// so the player doesnt reload when picking up a new weapon
		if (addweapon == 1) {
			other.items = other.items | self.weapon;
			sprint(other,"New weapon added to your inv\n");
		}
		else if (addweapon == 2) {			// replace current weapon
			newweapon = self.weapon;
			other.items = other.items - other.weapon;
			other.items = other.items | self.weapon;
			self = other;
			ThrowWeapon(self.weapon, 1);
			W_WeaponSwitch(newweapon);
			W_SetCurrentAmmo();
			pre_player_pull(other);
			self = oself;
		}
		CopyAmmo( self, other, self.weapon, 0 );
		W_UpdateAmmoCounts( other );
		self.touch = SUB_Null;
		setmodel( self, string_null );
		self.nextthink = time + 30;
		self.think = weapon_activate;
		return;
	}
};

void () weapon_start =
{
	if (!self.weapon) {
		_bprint("weapon_start: weapon with no .weapon set!\n");
		remove(self);
	}
	if (deathmatch == 3)
		{
	remove(self);
	return;
		}
	self.classname = "weapon_pickup";			// so bots can recognize it
	self.flags = FL_ITEM;
	
	precache_model( weapon_getlaymodel( self.weapon ) );
	setmodel( self, weapon_getlaymodel( self.weapon ) );
	
	self.gravity = 3;
	self.solid = SOLID_TRIGGER;
	
	setsize (self, '-16 -16 0', '16 16 56');
	
	SetWeaponAmmo( self, self.weapon );
	self.touch = weapon_touch;
};

// WEAPONS
void () weapon_shotgun = //for maps using old quake weapon names
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_PISTOL;
	weapon_start ();

};
void () weapon_pistol =
{
	weapon_shotgun();
};
void() weapon_ghost =
{
	self.weapon = WEAPON_GHOST;
	weapon_start ();
};
void() weapon_flag =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_FLAG;
	weapon_start ();
};
void() weapon_skull =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_SKULL;
	weapon_start ();
};
void() weapon_supershotgun =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_SHOT;
	weapon_start ();
};

void() weapon_nailgun =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_AR;
	weapon_start ();
};
void() weapon_ar =
{
	weapon_nailgun();
};

void() weapon_supernailgun =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_UZI;
	weapon_start ();
};
void() weapon_uzi =
{
    if (cvar("map_weapons_disable")) return;
	weapon_supernailgun ();
};

void() weapon_grenadelauncher =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_PPISTOL;
	weapon_start ();
};
void() weapon_ppistol =
{
    if (cvar("map_weapons_disable")) return;
	weapon_grenadelauncher ();
};

void() weapon_rocketlauncher =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_RL;
	weapon_start ();
};

void() weapon_lightning =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_SNIPER;
	weapon_start ();
};
void() weapon_sniperrifle =
{
    if (cvar("map_weapons_disable")) return;
	weapon_lightning ();
};

void() weapon_sword =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_SWORD;
	weapon_start ();
};

void() weapon_plasmapistol =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_PPISTOL;
	weapon_start ();
};

void() weapon_plasmarifle =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_PRIFLE;
	weapon_start ();
};

void() weapon_battlerifle =
{
    if (cvar("map_weapons_disable")) return;
    self.weapon = IT_BR;
    weapon_start();
};

void() weapon_needler =
{
    if (cvar("map_weapons_disable")) return;
	self.weapon = WEAPON_NEEDLER;
	weapon_start ();
};

void veh_ghost ()
{
	precache_model ("progs/ghost.mdl");
	//precache_sound ("items/protect.wav");
	//precache_sound ("items/protect2.wav");
	//precache_sound ("items/protect3.wav");
	setmodel (self, "progs/ghost.mdl");
	self.classname = "ghost";
	setsize (self, '-16 -16 -24', '16 16 32');
	self.movetype = MOVETYPE_FLY/*MOVETYPE_WALK*/;
	self.solid = SOLID_SLIDEBOX;
};