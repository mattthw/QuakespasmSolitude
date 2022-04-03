void() info_elite_think;
void() Check_Round;

/*
===========================================================

WEAPON SHOOTING
This Enables Various Weapons to be used by Monster AI
============================================================
*/
//Shoot the Plasma Pistol
void() Enemy_Fire_PPist =
{
    local vector dir;
    local entity en;
	local float r;
	r = random();

    ai_face();

    //sound (self, CHAN_WEAPON, "soldier/.wav", 1,ATTN_NORM);
    en = self.enemy;
    dir = en.origin - en.velocity*0.2;
    dir = normalize (dir - self.origin);

     W_FirePPistol( self.weaponheat );
};

//Shoot the Needler
void() Enemy_Fire_Needler =
{
    local vector dir;
    local entity en;
	local float r;
	r = random();

    ai_face();

    //sound (self, CHAN_WEAPON, "soldier/.wav", 1,ATTN_NORM);
    en = self.enemy;
    dir = en.origin - en.velocity*0.2;
    dir = normalize (dir - self.origin);

	if (round < 10 && round > 0)
   W_FireNeedler();
	else if (round < 20 && round > 10)
	{
   W_FireNeedler();
   W_FireNeedler();
	}
else if (round > 30)
	{
   W_FireNeedler();
   W_FireNeedler();
   W_FireNeedler();
	}
};

void() Enemy_Fire_Nade =
{
    local entity missile;

    sound (self, CHAN_WEAPON, "weapons/plasma_throw.wav", 1, ATTN_NORM);

    missile = spawn ();
    missile.owner = self;
    missile.movetype = MOVETYPE_BOUNCE;
    missile.solid = SOLID_BBOX;
    missile.effects = EF_FLAG2;
    missile.classname ="missile";
// set missile speed
    makevectors (self.angles);

    missile.velocity = normalize(self.enemy.origin - self.origin);
    missile.velocity = missile.velocity * 400;
    missile.velocity_z = 200;
    missile.avelocity = '300 300 300';
    missile.angles = vectoangles(missile.velocity);
    missile.selnade = self.selnade;
    missile.touch = GrenadeBounce;

    setmodel (missile, "progs/plasgren.mdl");
    setsize (missile, '0 0 0', '0 0 0');
    setorigin (missile, self.origin);
};
/*
==============================================================================

ELITE

==============================================================================
*/

$cd id1/models/elite_c
$origin 0 0 24
$base base		
$skin base

$frame	stand1 

$frame walk1 walk2 walk3 walk4 walk5 walk6 walk7
$frame walk8 walk9 walk10 walk11 walk12 walk13 walk14 walk15 walk16

$frame run1 run2 run3 run4 run5 run6 run7 run8

$frame swing1 swing2 swing3 swing4 swing5 swing6 swing7
$frame swing8 swing9 swing10 swing11 swing12 swing13 swing14

$frame smash1 smash2 smash3 smash4 smash5 smash6 smash7
$frame smash8 smash9 smash10 smash11 smash12 smash13 smash14
	
$frame nade1 nade2 nade3 nade4 nade5 nade6

$frame pain1 pain2 pain3 pain4 pain5

$frame bdeath1 bdeath2 bdeath3 bdeath4 bdeath5 bdeath6
$frame bdeath7 bdeath8 bdeath9 bdeath10
//=============================================================================

//=============================================================================

void() laugh =
{
local float r;
r = random();
if (self.attack_finished > time)
return;
if (r <= 0.025)
{
sound (self, CHAN_AUTO, "elite/laugh1.wav", 1, ATTN_NORM);
self.attack_finished = time + 5;
}
else if ( r <= 0.05)
{
sound (self, CHAN_AUTO, "elite/laugh2.wav", 1, ATTN_NORM);
self.attack_finished = time + 5;
}
else
{}
};


void() elite_stand1	=[	$stand1,	elite_stand1	] {ai_stand(); laugh();};

void() elite_walk1	=[	$walk1,		elite_walk2	] {ai_walk(3);};
void() elite_walk2	=[	$walk2,		elite_walk3	] {ai_walk(2);};
void() elite_walk3	=[	$walk3,		elite_walk4	] {
ai_walk(2);
};
void() elite_walk4	=[	$walk4,		elite_walk5	] {ai_walk(2);};
void() elite_walk5	=[	$walk5,		elite_walk6	] {ai_walk(2);};
void() elite_walk6	=[	$walk6,		elite_walk7	] {
ai_walk(5);
if (random() < 0.1)
	sound (self, CHAN_VOICE, "elite/ogdrag.wav", 1, ATTN_IDLE);
};
void() elite_walk7	=[	$walk7,		elite_walk8	] {ai_walk(6);};
void() elite_walk8	=[	$walk8,		elite_walk9	] {ai_walk(4);};
void() elite_walk9	=[	$walk9,		elite_walk10	] {ai_walk(6);};
void() elite_walk10	=[	$walk10,	elite_walk11	] {ai_walk(2);};
void() elite_walk11	=[	$walk11,	elite_walk12	] {ai_walk(4);};
void() elite_walk12	=[	$walk12,	elite_walk13	] {ai_walk(6);};
void() elite_walk13	=[	$walk13,	elite_walk14	] {ai_walk(6);};
void() elite_walk14	=[	$walk14,	elite_walk15	] {ai_walk(6);};
void() elite_walk15	=[	$walk15,	elite_walk16	] {ai_walk(6);};
void() elite_walk16	=[	$walk16,	elite_walk1	] {ai_walk(8);};

void() elite_run1	=[	$run1,		elite_run2	] {ai_run(18);
};
void() elite_run2	=[	$run2,		elite_run3	] {ai_run(15);};
void() elite_run3	=[	$run3,		elite_run4	] {ai_run(15);};
void() elite_run4	=[	$run4,		elite_run5	] {ai_run(15);};
void() elite_run5	=[	$run5,		elite_run6	] {ai_run(15);};
void() elite_run6	=[	$run6,		elite_run7	] {ai_run(15);};
void() elite_run7	=[	$run7,		elite_run8	] {ai_run(15);};
void() elite_run8	=[	$run8,		elite_run1	] {ai_run(15);};

void() elite_swing1	=[	$swing1,		elite_swing2	] {ai_charge(11);
sound (self, CHAN_WEAPON, "elite/ogsawatk.wav", 1, ATTN_NORM);
};
void() elite_swing2	=[	$swing2,		elite_swing3	] {ai_charge(1);};
void() elite_swing3	=[	$swing3,		elite_swing4	] {ai_charge(4);};
void() elite_swing4	=[	$swing4,		elite_swing5	] {ai_charge(13);};
void() elite_swing5	=[	$swing5,		elite_swing6	] {ai_charge(9);ai_melee(MOD_OGRE); };
void() elite_swing6	=[	$swing6,		elite_swing7	] {};
void() elite_swing7	=[	$swing7,		elite_swing8	] {};
void() elite_swing8	=[	$swing8,		elite_swing9	] {};
void() elite_swing9	=[	$swing9,		elite_swing10 ] {};
void() elite_swing10	=[	$swing10,		elite_swing11 ] {};
void() elite_swing11	=[	$swing11,		elite_swing12 ] {};
void() elite_swing12	=[	$swing12,		elite_swing13 ] {ai_charge(3);};
void() elite_swing13	=[	$swing13,		elite_swing14 ] {ai_charge(8);};
void() elite_swing14	=[	$swing14,		elite_run1	] {ai_charge(9);};

void() elite_smash1	=[	$smash1,		elite_smash2	] {ai_charge(6);
sound (self, CHAN_WEAPON, "elite/ogsawatk.wav", 1, ATTN_NORM);
};
void() elite_smash2	=[	$smash2,		elite_smash3	] {ai_charge(0);};
void() elite_smash3	=[	$smash3,		elite_smash4	] {ai_charge(0);};
void() elite_smash4	=[	$smash4,		elite_smash5	] {ai_charge(1);};
void() elite_smash5	=[	$smash5,		elite_smash6	] {ai_charge(4);ai_melee(MOD_OGRE);};
void() elite_smash6	=[	$smash6,		elite_smash7	] {ai_charge(4);};
void() elite_smash7	=[	$smash7,		elite_smash8	] {ai_charge(4);};
void() elite_smash8	=[	$smash8,		elite_smash9	] {ai_charge(10);};
void() elite_smash9	=[	$smash9,		elite_smash10 ] {ai_charge(13);};
void() elite_smash10	=[	$smash10,		elite_smash11 ] {};
void() elite_smash11	=[	$smash11,		elite_smash12 ] {ai_charge(2);
self.nextthink = time + 0.1 + random()*0.1;};	// slight variation
//void() elite_smash12	=[	$smash12,		elite_smash13 ] {ai_charge();};
void() elite_smash12 =[$smash12, elite_smash13] {ai_charge(0);};
void() elite_smash13	=[	$smash13,		elite_smash14 ] {ai_charge(4);};
void() elite_smash14	=[	$smash14,		elite_run1	] {ai_charge(12);};

void() elite_nade1	=[	$nade1,		elite_nade2	] {ai_face();};
void() elite_nade2	=[	$nade2,		elite_nade3	] {ai_face();};
void() elite_nade3	=[	$nade3,		elite_nade4	] {ai_face(); Enemy_Fire_Nade();};
void() elite_nade4	=[	$nade4,		elite_nade5	] {ai_face();};
void() elite_nade5	=[	$nade5,		elite_nade6	] {ai_face();};
void() elite_nade6	=[	$nade6,		elite_run1	] {ai_face();};

void() elite_nail1	=[	$run1,		elite_nail2	] {ai_face();};
void() elite_nail2	=[	$run2,		elite_nail3	] {ai_face();};
void() elite_nail3	=[	$run2,		elite_nail4	] {ai_face();};
void() elite_nail4	=[	$run3,		elite_nail5	] {ai_face();
if (self.elite == 0 || self.elite == 2)
Enemy_Fire_PPist();
if (self.elite == 1)
Enemy_Fire_Needler();
};
void() elite_nail5	=[	$run4,		elite_nail6	] {ai_face();};
void() elite_nail6	=[	$run5,		elite_nail7	] {ai_face();};
void() elite_nail7	=[	$run6,		elite_run1	] {ai_face();};

void()	elite_pain1	=[	$pain1,		elite_pain2	] {ai_pain(3); };
void()	elite_pain2	=[	$pain2,		elite_pain3	] {};
void()	elite_pain3	=[	$pain3,		elite_pain4	] {};
void()	elite_pain4	=[	$pain4,		elite_pain5	] {};
void()	elite_pain5	=[	$pain5,		elite_run1	] {};

void()	elite_bdie1	=[	$bdeath1,	elite_bdie2	] {sound (self, CHAN_VOICE, "elite/ogdth.wav", 1, ATTN_NORM);};
void()	elite_bdie2	=[	$bdeath2,	elite_bdie3	] {};
void()	elite_bdie3	=[	$bdeath3,	elite_bdie4	]
{self.solid = SOLID_NOT;};
void()	elite_bdie4	=[	$bdeath4,	elite_bdie5	] {};
void()	elite_bdie5	=[	$bdeath5,	elite_bdie6	] {};
void()	elite_bdie6	=[	$bdeath6,	elite_bdie7	] {};
void()	elite_bdie7	=[	$bdeath7,	elite_bdie8	] {};
void()	elite_bdie8	=[	$bdeath8,	elite_bdie9	] {};
void()	elite_bdie9	=[	$bdeath9,	elite_bdie10	] {};
void()	elite_bdie10	=[	$bdeath10,	elite_bdie10	] { self.think = SUB_Remove; self.nextthink = time + 5;round_kills += 1;Check_Round();};

void(float enemy_range) elite_attack =
{
	local float r;
	r = random();
	if (CATCH == 1)
	{
if (r <= 0.5)
elite_nade1();
else
elite_nail1();
	}
	else
	{
if (r <= 0.15 && enemy_range <= RANGE_MID)
elite_nade1();
else
elite_nail1();
	}
};


void(float enemy_range) elite_melee =
{
	if (random() > 0.5)
		{
		elite_smash1 ();
		}
	else
		{
		elite_swing1 ();
		}
};
void(entity attacker, float damage)	elite_pain =
{
	local float	r;

// don't make multiple pain sounds right after each other
	if (self.pain_finished > time)
		return;

	sound (self, CHAN_VOICE, "elite/ogpain1.wav", 1, ATTN_NORM);		

	r = random();
	
		elite_pain2 ();
		self.pain_finished = time + 1.5;
};


