/*
==============================================================================

SOLDIER / PLAYER

==============================================================================
*/

$cd id1/models/soldier3
$origin 0 -6 24
$base base		
$skin skin
$frame	stand1 stand2 stand3 stand4 stand5 stand6 stand7 stand8 stand9

$frame grenade1 grenade2 grenade3 grenade4 grenade5 grenade6 grenade7 grenade8 grenade9

$frame run1 run2 run3 run4 run5 run6 run7 run8

$frame melee1 melee2 melee3 melee4 melee5 melee6 melee7 

$frame shoot1 shoot2 shoot3 shoot4

$frame death1

$frame prowl_1 prowl_2 prowl_3 prowl_4 prowl_5 prowl_6 prowl_7 prowl_8
$frame prowl_9 prowl_10 prowl_11 prowl_12 prowl_13 prowl_14 prowl_15 prowl_16
$frame prowl_17 prowl_18 prowl_19 prowl_20 prowl_21 prowl_22 prowl_23 prowl_24

/*
==============================================================================
SOLDIER CODE
==============================================================================
*/

void() laughs =
{
local float r;
r = random();
if (self.attack_finished > time)
return;

if (r <= 0.025)
{
sound (self, CHAN_AUTO, "soldier/laugh1.wav", 0.8, ATTN_NORM);
self.attack_finished = time + 5;
}
else if ( r <= 0.05)
{
sound (self, CHAN_AUTO, "soldier/laugh2.wav", 0.8, ATTN_NORM);
self.attack_finished = time + 5;
}
else
{}
};

void() g_attack =
{
local float r;
r = random();

if (self.attack_finished > time)
return;
if (r <= 0.005)
{
sound (self, CHAN_AUTO, "soldier/unstoppable.wav", 0.9, ATTN_NORM);
self.attack_finished = time + 5;
}
else if ( r <= 0.01)
{
sound (self, CHAN_AUTO, "soldier/kill_it.wav", 0.9, ATTN_NORM);
self.attack_finished = time + 5;
}
else {}
};
void()	grunt_stand1	=[	$stand1,	grunt_stand2	] {ai_stand();};
void()	grunt_stand2	=[	$stand2,	grunt_stand3	] {ai_stand();};
void()	grunt_stand3	=[	$stand3,	grunt_stand4	] {ai_stand();};
void()	grunt_stand4	=[	$stand4,	grunt_stand5	] {ai_stand();};
void()	grunt_stand5	=[	$stand5,	grunt_stand6	] {ai_stand();};
void()	grunt_stand6	=[	$stand6,	grunt_stand7	] {ai_stand();laughs();};
void()	grunt_stand7	=[	$stand7,	grunt_stand8	] {ai_stand();};
void()	grunt_stand8	=[	$stand8,	grunt_stand1	] {ai_stand();};

void()	grunt_walk1	=[	$prowl_1,	grunt_walk2	] {
ai_walk(1);};
void()	grunt_walk2	=[	$prowl_2,	grunt_walk3	] {ai_walk(1);};
void()	grunt_walk3	=[	$prowl_3,	grunt_walk4	] {ai_walk(1);};
void()	grunt_walk4	=[	$prowl_4,	grunt_walk5	] {ai_walk(1);};
void()	grunt_walk5	=[	$prowl_5,	grunt_walk6	] {ai_walk(2);};
void()	grunt_walk6	=[	$prowl_6,	grunt_walk7	] {ai_walk(3);};
void()	grunt_walk7	=[	$prowl_7,	grunt_walk8	] {ai_walk(4);};
void()	grunt_walk8	=[	$prowl_8,	grunt_walk9	] {ai_walk(4);};
void()	grunt_walk9	=[	$prowl_9,	grunt_walk10	] {ai_walk(2);};
void()	grunt_walk10	=[	$prowl_10,	grunt_walk11	] {ai_walk(2);};
void()	grunt_walk11	=[	$prowl_11,	grunt_walk12	] {ai_walk(2);};
void()	grunt_walk12	=[	$prowl_12,	grunt_walk13	] {ai_walk(1);};
void()	grunt_walk13	=[	$prowl_13,	grunt_walk14	] {ai_walk(0);};
void()	grunt_walk14	=[	$prowl_14,	grunt_walk15	] {ai_walk(1);};
void()	grunt_walk15	=[	$prowl_15,	grunt_walk16	] {ai_walk(1);};
void()	grunt_walk16	=[	$prowl_16,	grunt_walk17	] {ai_walk(1);};
void()	grunt_walk17	=[	$prowl_17,	grunt_walk18	] {ai_walk(3);};
void()	grunt_walk18	=[	$prowl_18,	grunt_walk19	] {ai_walk(3);};
void()	grunt_walk19	=[	$prowl_19,	grunt_walk20	] {ai_walk(3);};
void()	grunt_walk20	=[	$prowl_20,	grunt_walk21	] {ai_walk(3);};
void()	grunt_walk21	=[	$prowl_21,	grunt_walk22	] {ai_walk(2);};
void()	grunt_walk22	=[	$prowl_22,	grunt_walk23	] {ai_walk(1);};
void()	grunt_walk23	=[	$prowl_23,	grunt_walk24	] {ai_walk(1);};
void()	grunt_walk24	=[	$prowl_24,	grunt_walk1	] {ai_walk(1);};

void()	grunt_run1	=[	$run1,		grunt_run2	] {
if (random() < 0.2)
	sound (self, CHAN_VOICE, "soldier/idle.wav", 1, ATTN_IDLE);
ai_run(11);};
void()	grunt_run2	=[	$run2,		grunt_run3	] {ai_run(15);};
void()	grunt_run3	=[	$run3,		grunt_run4	] {ai_run(10);};
void()	grunt_run4	=[	$run4,		grunt_run5	] {ai_run(10);};
void()	grunt_run5	=[	$run5,		grunt_run6	] {ai_run(8);};
void()	grunt_run6	=[	$run6,		grunt_run7	] {ai_run(15);};
void()	grunt_run7	=[	$run7,		grunt_run8	] {ai_run(10);};
void()	grunt_run8	=[	$run8,		grunt_run1	] {ai_run(8);};

void()	grunt_grenade1	=[	$grenade1,	grunt_grenade2	] {ai_face();};
void()	grunt_grenade2	=[	$grenade2,	grunt_grenade3	] {ai_face();};
void()	grunt_grenade3	=[	$grenade3,	grunt_grenade4	] {ai_face();};
void()	grunt_grenade4	=[	$grenade4,	grunt_grenade5	] {ai_face();};
void()	grunt_grenade5	=[	$grenade5,	grunt_grenade6	] {ai_face();Enemy_Fire_Nade();};
void()	grunt_grenade6	=[	$grenade6,	grunt_grenade7	] {ai_face();};
void()	grunt_grenade7	=[	$grenade7,	grunt_grenade8	] {ai_face();};
void()	grunt_grenade8	=[	$grenade8,	grunt_grenade9	] {ai_face();};
void()	grunt_grenade9	=[	$grenade9,	grunt_run1	] {ai_face();};

void()	grunt_atk1	=[	$shoot1,	grunt_atk2	] {ai_face();};
void()	grunt_atk2	=[	$shoot2,	grunt_atk3	] {ai_face();Enemy_Fire_PPist();};
void()	grunt_atk3	=[	$shoot3,	grunt_atk4	] {ai_face();};
void()	grunt_atk4	=[	$shoot4,	grunt_run1	] {ai_face();};

void() grunt_melee1	=[	$melee1,		grunt_melee2	] {ai_charge(1); };
void() grunt_melee2	=[	$melee2,		grunt_melee3	] {ai_charge(1);ai_melee(MOD_SOLDIER);};
void() grunt_melee3	=[	$melee3,		grunt_melee4	] {ai_charge(4); };
void() grunt_melee4	=[	$melee4,		grunt_melee5	] {ai_charge(13);};
void() grunt_melee5	=[	$melee5,		grunt_melee6	] {ai_charge(9); };
void() grunt_melee6	=[	$melee6,		grunt_melee7	] {};
void() grunt_melee7	=[	$melee7,		grunt_run1	] {};

void()	grunt_pain1	=[	$stand1,		grunt_run1	] {ai_painforward(8);g_attack();};

void() grunt_melee =
{
grunt_melee1();
};
void(entity attacker, float damage)	grunt_pain =
{
	
	if (self.pain_finished > time)
		return;

		self.pain_finished = time + 1;
		grunt_pain1 ();
		sound (self, CHAN_VOICE, "soldier/pain1.wav", 1, ATTN_NORM);
};


void(float enemy_range) grunt_attack =
{
	local float r;
	r = random();
	if (CATCH == 1)
	{
if (r <= 0.7)
grunt_grenade1();
else
grunt_atk1();
	}
	else
	{
if (r <= 0.15 && enemy_range <= RANGE_MID)
grunt_grenade1();
else
grunt_atk1();
	}
};


void()	grunt_die1	=[	$death1,	grunt_die1	] {remove(self); round_kills += 1;Check_Round();};



void() grunt_die =
{
	sound (self, CHAN_VOICE, "soldier/death1.wav", 1, ATTN_NORM);
		grunt_die1 ();
};


/*QUAKED monster_grunt (1 0 0) (-16 -16 -24) (16 16 40) Ambush
*/
void() monster_grunt =
{	

	precache_model ("progs/soldier.mdl");
	precache_model ("progs/gib1.mdl");
	precache_model ("progs/gib2.mdl");
	precache_model ("progs/gib3.mdl");

	precache_sound ("soldier/death1.wav");
	precache_sound ("soldier/idle.wav");
	precache_sound ("soldier/pain1.wav");
	precache_sound ("soldier/pain2.wav");
	precache_sound ("soldier/sattck1.wav");
	precache_sound ("soldier/sight1.wav");
	self.solid = SOLID_SLIDEBOX;
	self.movetype = MOVETYPE_STEP;

	setmodel (self, "progs/soldier.mdl");

	setsize (self, '-16 -16 -24', '16 16 40');
	self.health = 130;

	self.th_stand = grunt_stand1;
	self.th_walk = grunt_walk1;
	self.th_run = grunt_run1;
	self.th_missile = grunt_atk1;
	self.th_pain = grunt_pain;
	self.th_die = grunt_die;

	//self.ammo_shells_real = 5; // drop 5 shells on death

	walkmonster_start ();
};