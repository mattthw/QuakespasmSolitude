float p_count;
void () Pickup_think =
{
	if (self.owner.health < 1) {
		stuffcmd (self.owner, "cl_ww 0");
		remove(self);
		return;
	}
	self.health -= 1;
	if (self.health < 0) {
		stuffcmd (self.owner, "cl_ww 0");
		remove(self);
		p_count = 0;
		return;
	}
	
	self.nextthink = time + 1;
};

void (entity client) Pickup =
{
	local entity dt;
	if (p_count > 1)
	return;
	dt = spawn();
	p_count = 1;
	dt.health = 2;
	dt.owner = client;
	dt.think = Pickup_think;
	dt.nextthink = time + 0;
};

void(entity client) Setup_Pickup =
{
other = client;
Pickup(client);
client.pickup_flags = PICKUP_UNABLE;
};
