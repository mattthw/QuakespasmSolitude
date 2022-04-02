void() Slowmo_Deactivate =
{
		localcmd ("cl_overlay 0\n");
		localcmd ("cl_activate 0\n");
		localcmd ("host_framerate 0\n");
};
/*
====================================================
Clock for the HUD.
=====================================================
*/
void(entity client) Update_Slowmo =
{

	stuffcmd (client, "cl_slowmo ");
	stuffcmd (client, ftos(client.slowmo));
	stuffcmd (client, " \n");

};
/*
====================================================
This gets repeatedly called, and decreases 
your slowmo while you're in slowmo mode.
=====================================================
*/
void () Slowmo_think =
{
	self.owner.slowmo = self.health;

	if (self.owner.health < 1 || SLOWMO_TOGGLE == OFF) {
		SLOWMO_TOGGLE = OFF;
		Slowmo_Deactivate();
		remove(self);
		return;
	}
	if (SLOWMO_TOGGLE == ON)
	{
	localcmd ("cl_overlay 1\n");
	localcmd ("cl_activate 1\n");
	localcmd ("host_framerate 0.008\n");
	}
	self.health -= 1;
	Update_Slowmo(self.owner);
	if (self.health < 0) {
		SLOWMO_TOGGLE = OFF;
		Slowmo_Deactivate();
		remove(self);
		return;
	}
	if (SLOWMO_TOGGLE == ON && self.health > 0)
	localcmd ("play misc/talk.wav\n");
	
	self.nextthink = time + 1;
};
/*
====================================================
Creates the Slowmo ticker
=====================================================
*/
void (entity client) Slowmo =
{
	local entity slowmo_timer;
	slowmo_timer = spawn();
	slowmo_timer.health = client.slowmo;
	slowmo_timer.owner = client;
	slowmo_timer.think = Slowmo_think;
	slowmo_timer.nextthink = time + 0;
};
/*
====================================================
Used to Create a time ticker to count a clients slowmo.
The flag switches to unable, so it player_Pre_think doesn't keep
calling the function.
=====================================================
*/
void(entity client) Setup_Slowmo =
{
Slowmo(client);
client.slowmo_flags = SLOWMO_UNABLE;
};

/*
====================================================
Used to Increase Slowmotion.
Client: Thing that the function is refering to. Can be self, other, etc.
howmuch: A number value. Indicates how much slowmo something gets.
=====================================================
*/
void(entity client, float howmuch) Add_Slowmo =
{
if (deathmatch != SWAT || client.slowmo > 8)
return;
if (client.slowmo < 8)
client.slowmo += howmuch;
Update_Slowmo(client);
};