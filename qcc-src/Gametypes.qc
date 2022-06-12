void() Start_Weapon;

void() Setup_Gametype = {
        local entity spot;
        local float wtemp;
        Slowmo_Deactivate();
        // oh, this is a hack!
        setmodel(self, "progs/eyes.mdl");
        modelindex_eyes = self.modelindex;

        //vwep start
        setmodel(self, /*19073i*/"progs/eyes.mdl");
        modelindex_eyes = self.modelindex;
        setmodel(self, /*18876i*/"progs/plyr.mdl");
        modelindex_dead = self.modelindex;
        setmodel(self, /*18891i*/"progs/player.mdl");
        modelindex_playernail = self.modelindex;
        setmodel(self, /*18908i*/"progs/player_shot.mdl");
        modelindex_playershot = self.modelindex;
        setmodel(self, /*18930i*/"progs/player_snail.mdl");
        modelindex_playersnail = self.modelindex;
        setmodel(self, /*18953i*/"progs/player_sshot.mdl");
        modelindex_playersshot = self.modelindex;
        setmodel(self, /*18976i*/"progs/player_needler.mdl");
        modelindex_playerneedler = self.modelindex;
        setmodel(self, /*19001i*/"progs/player_rocket.mdl");
        modelindex_playerrocket = self.modelindex;
        setmodel(self, /*19025i*/"progs/player_sniper.mdl");
        modelindex_playersniper = self.modelindex;
        setmodel(self, /*19049i*/"progs/player_plasma.mdl");
        modelindex_playerplasma = self.modelindex;
        setmodel(self, /*18891i*/"progs/player.mdl");
        modelindex_playerbr = self.modelindex;
        modelindex_playersword = self.modelindex;
        //vwep end

        if (self.ishuman == TRUE)
        stuffcmd(self, "r_drawviewmodel 1\n");

        switch (deathmatch) {
            /*
            ==============================================================
            Gametype: Slayer
            ==============================================================
            */
            case SLAYER: {
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
                DecodeLevelParms();
                wtemp = self.weapon;
                self.weapon = 0;
                W_WeaponSwitch(wtemp);
                self.attack_finished = time;
                self.th_pain = player_pain;
                self.th_die = PlayerDie;
                self.deadflag = DEAD_NO;
                spot = SelectSpawnPoint();
                Start_Weapon();
                self.origin = spot.origin + '0 0 1';
                self.angles = spot.angles;
                self.fixangle = TRUE;
                setsize(self, VEC_HULL_MIN, VEC_HULL_MAX);
                self.view_ofs = '0 0 22';
                self.velocity = '0 0 0';
                player_stand1();
                makevectors(self.angles);
                spawn_tfog(self.origin + v_forward * 20);
                spawn_tdeath(self.origin, self);
                rj = numberserverinfokey("rj");
                self.needles = 0;
                self.pfov = FOV_DEFAULT;
                Set_FOV(self.pfov);
                //self.reset = 1.00;
                self.weaponheat = 0;
                stuffcmd(self, "chase_active 0\n");
                NadeCounter(self);
            }
            break;

            /*
            ==============================================================
            Gametype: Firefight
            ==============================================================
            */
            case FIREFIGHT: {
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
                DecodeLevelParms();
                wtemp = self.weapon;
                self.weapon = 0;
                W_WeaponSwitch(wtemp);
                self.attack_finished = time;
                self.th_pain = player_pain;
                self.th_die = PlayerDie;
                self.deadflag = DEAD_NO;
                spot = SelectSpawnPoint();
                Start_Weapon();
                self.origin = spot.origin + '0 0 1';
                self.angles = spot.angles;
                self.fixangle = TRUE;
                setsize(self, VEC_HULL_MIN, VEC_HULL_MAX);
                self.view_ofs = '0 0 22';
                self.velocity = '0 0 0';
                player_stand1();
                makevectors(self.angles);
                spawn_tfog(self.origin + v_forward * 20);
                spawn_tdeath(self.origin, self);
                rj = numberserverinfokey("rj");
                self.needles = 0;
                self.pfov = FOV_DEFAULT;
                Set_FOV(self.pfov);
                //self.reset = 1.00;
                self.weaponheat = 0;
                stuffcmd(self, "chase_active 0\n");
                NadeCounter(self);
            }
            break;
            /*
            ==============================================================
            Gametype: Swat
            ==============================================================
            */
            case SWAT: {
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
                DecodeLevelParms();
                wtemp = self.weapon;
                self.weapon = 0;
                self.slowmo = 5;
                MAX_SLOWMO = 8;
                self.slowmo_flags = SLOWMO_UNABLE;
                W_WeaponSwitch(wtemp);
                self.attack_finished = time;
                self.th_pain = player_pain;
                self.th_die = PlayerDie;
                self.deadflag = DEAD_NO;
                spot = SelectSpawnPoint();
                Start_Weapon();
                self.origin = spot.origin + '0 0 1';
                self.angles = spot.angles;
                self.fixangle = TRUE;
                setsize(self, VEC_HULL_MIN, VEC_HULL_MAX);
                self.view_ofs = '0 0 22';
                self.velocity = '0 0 0';
                player_stand1();
                makevectors(self.angles);
                spawn_tfog(self.origin + v_forward * 20);
                spawn_tdeath(self.origin, self);
                rj = numberserverinfokey("rj");
                self.needles = 0;
                self.pfov = FOV_DEFAULT;
                Set_FOV(self.pfov);
                //self.reset = 1.00;
                self.weaponheat = 0;
                stuffcmd(self, "chase_active 0\n");
                self.nade_lives = 0;
                self.plasma_lives = 0;
                NadeCounter(self);
                Update_Slowmo(self);
                localcmd("fraglimit 50\n");
            }
            break;
        }
};

// see also: SetChangeParms()
void() Start_Weapon = {
        switch (deathmatch) {
            /*
            ==============================================================
            Gametype: Slayer [Pistol & Assault Rifle]
            ==============================================================
            */
            case SLAYER: {
                // get from engine
                float primary_weapon = cvar("custom_primary");
                float secondary_weapon = cvar("custom_secondary");
                bprint("secondary=");bprint(ftos(secondary_weapon));bprint("\n");
                if (primary_weapon == WEAPON_NOT_SET) {
                    // mod being used with an engine which does not understand our custom cvars. Let's set them up with defaults.
                    primary_weapon = WEAPON_AR;
                    secondary_weapon = WEAPON_PISTOL;
                }
                // set weapon(s) and ammo to player items
                if (secondary_weapon == WEAPON_NONE) {
                    bprint("NO SECONDARY!=============\n");
                    self.items = primary_weapon;
                    SetWeaponAmmo(self, primary_weapon);
                } else {
                    self.items = primary_weapon + secondary_weapon;
                    SetWeaponAmmo(self, primary_weapon);
                    SetWeaponAmmo(self, secondary_weapon);
                }
                // set held weapon
                self.weapon = primary_weapon;
                W_UpdateWeapon();
                W_UpdateAmmoCounts(self);
                // sword has messed up default frame. Call after UpdateWeapon since it sets to 0
                // todo: fix v_sword.mdl animations/model
                if (self.weapon == WEAPON_SWORD)
                    self.weaponframe = 41;
            }
            break;

            /*
            ==============================================================
            Gametype: Firefight [SMG]
            ==============================================================
            */
            case FIREFIGHT: {
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
            case SWAT: {
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