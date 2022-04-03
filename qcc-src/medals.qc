.float medal_count;
/*
====================================================
This gets repeatedly called, and decreases 
your slowmo while you're in slowmo mode.
=====================================================
*/
void () Medal_think =
{
	if (self.owner.medal_count > 1)
	{
	self.owner.medal_count -= 1;
	remove(self);
	return;
	}
		
	if (self.owner.health < 1) {
		stuffcmd (self.owner, "cl_killmedals 0\n");
		self.owner.medal_count -= 1;
		remove(self);
		return;
	}
	self.health -= 1;
	if (self.health < 0) {
		stuffcmd (self.owner, "cl_killmedals 0\n");
		self.owner.medal_count -= 1;
		remove(self);
		return;
	}
	
	self.nextthink = time + 1;
};
/*
====================================================
Creates the Slowmo ticker
=====================================================
*/
void (entity client) Medal =
{
	local entity medal_timer;
	client.medal_count += 1;
	medal_timer = spawn();
	medal_timer.health = 4;
	medal_timer.owner = client;
	medal_timer.think = Medal_think;
	medal_timer.nextthink = time + 0;
};
