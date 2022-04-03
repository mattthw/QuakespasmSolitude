
//Credit to Frag.machine for partial code!
void () info_elite_think =
{
	if (TOTAL_MONS < MAX_MONS)
	{
	Spawn_enemy();
	}
    // finally, let's schedule the execution of our "think" function
    self.think = info_elite_think;
    self.nextthink = (time + 5);
}; 

void () info_elite_start =
{
	if (deathmatch != FIREFIGHT)
	{
	remove(self);
	return;
	}
	precache_sound ("firefight.wav");
	
	precache_model ("progs/elite.mdl");

	precache_sound ("elite/ogdrag.wav");
	precache_sound ("elite/ogdth.wav");
	precache_sound ("elite/ogpain1.wav");
	precache_sound ("elite/ogsawatk.wav");
	precache_sound ("elite/ogwake.wav");
	precache_sound ("elite/laugh1.wav");
	precache_sound ("elite/laugh2.wav");
	
	//Grunts
	precache_model ("progs/soldier.mdl");

	precache_sound ("soldier/death1.wav");
	precache_sound ("soldier/idle.wav");
	precache_sound ("soldier/pain1.wav");
	precache_sound ("soldier/pain2.wav");
	precache_sound ("soldier/sight1.wav");
	precache_sound ("soldier/laugh1.wav");
	precache_sound ("soldier/laugh2.wav");
	
	precache_sound ("soldier/unstoppable.wav");
	precache_sound ("soldier/kill_all.wav");
	
	Firefight_settings();	//Set up Firefight settings
	self.enemy = world;
	
	
    self.think = info_elite_think;
    self.nextthink = time + 5;
}; 