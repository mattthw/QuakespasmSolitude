void () Activate_Ghost;
void () Activate_Normal;

/*
==============
Vehicles
==============
*/
/* This is a WIP but it *should* hold most of the vehicle code.
*/

void () Vehicle_Index = 
{
	if (self.weapon == IT_GHOST)
	{
		Activate_Ghost ();
//        bprint2 (PRINT_HIGH, self.netname, " Entered the Ghost.\n");  //Team Xlink - Entered the Ghost text on screen
	}
	else
	{
		Activate_Normal ();
	}
}


void () Activate_Ghost =
{
//Team Xlink - I finally decided to comment on some of my code.
	if (self.weapon == IT_GHOST) //The Ghost Vehicle
	{
        setmodel(self,"progs/v_ghost.mdl");
        modelindex_player = self.modelindex;
        self.impulse = 0;
        centerprint(self,"\n"); 
		stuffcmd(self, "cl_yawspeed 1\n");
		stuffcmd(self, "chase_active 1\n");
		self.flags = self.flags - (self.flags & FL_ONGROUND);
	}
}
void () Activate_Normal =
{
	{
        setmodel(self,"progs/player.mdl");
        modelindex_player = self.modelindex;
        self.impulse = 0;
        centerprint(self,"\n"); 
		stuffcmd(self, "chase_active 0\n");
		self.flags = self.flags - (self.flags & FL_ONGROUND);
	}
}

void () Ghost_Speed =
{
	self.speed = 999;
	self.maxspeed = 999; 
	stuffcmd(self,"cl_forwardspeed 999 \n");
	stuffcmd(self,"cl_backspeed 999 \n");
	stuffcmd(self,"cl_sidespeed 999 \n");
}

/*==============================================================
Warthog by Arkage
================================================================*/

// some defs
entity use_target;

// Positions
vector wart_driver = '-20.993 3.569 44.978'; // the spot where the driver sits	'-20.993,3.569,44.978'
vector wart_gunner;// = '2.665,53.994,51.296'; // the gunners position	'0.506, -51.909, 51.296'
vector wart_passanger = '16.795 3.569 44.978'; // the passanger poition	'16.795,3.569,44.978'

// enter positions
vector wart_driver_enter = '-56.67 26.836 44.978'; // location to check to seee what seat to enter	'-56.67,26.836,44.978'
vector wart_gunner_enter; // = '2.081, -119.883, 44.978';		//'2.081,-119.883,44.978'
vector wart_passanger_enter = '56.73 26.836 44.978';	//'56.73,26.836,44.978'

.vector c_wart_driver_enter;
.vector c_wart_gunner_enter;
.vector c_wart_passanger_enter;

// speed and stuff
float wart_Mspeed_f = 40; // Maxium speed forward
float wart_Mspeed_b; // maxium speed backwards
.float wart_speed;

// Player info
.entity follow_veh; // The vechile that the player is in,
.string veh_in; // What vechiele is the player in
.string veh_pos; // What position id the player in  eg. driver or gunner
.vector loc_dist; // how far to offset the players origin

// Seats
.float wart_D_seat_taken; // Is the driver seat taken in this warthog
.float wart_P_seat_taken;
.float wart_G_seat_taken;

void set_pos ()
{
// weird problem with bad vectors so these are set here
wart_gunner_x =+ 0.506;
wart_gunner_y =+ -51.909;
wart_gunner_z =+ 51.296;

wart_gunner_enter_x =+ 2.081;
wart_gunner_enter_y =+ -119.883;
wart_gunner_enter_z =+ 44.978;

};

void wart_driver_exit ()
{
self.origin = self.follow_veh.c_wart_driver_enter;
use_target.wart_D_seat_taken = 0;
self.veh_in = "";
self.veh_pos = "";
};

void wart_passanger_exit ()
{
self.origin = self.follow_veh.c_wart_passanger_enter;
self.movetype = MOVETYPE_WALK;
self.follow_veh.wart_P_seat_taken = 0;
self.veh_in = "";
self.veh_pos = "";
};

void wart_gunner_exit ()
{
self.origin = self.follow_veh.c_wart_gunner_enter;
self.follow_veh.wart_G_seat_taken = 0;
self.movetype = MOVETYPE_WALK;
self.veh_in = "";
self.veh_pos = "";
};

void wart_driver_entering(entity wart)
{
if (wart.wart_D_seat_taken == 1)
return;
self.veh_in = "warthog";
self.veh_pos = "driver";
wart.wart_D_seat_taken = 1;
self.follow_veh = wart;
wart.origin = self.origin;
self.loc_dist = wart_driver;
use_target = wart;
sprint(self,"enter the warthog should work if this is shown");
};

void wart_passanger_entering(entity wart)
{
if (wart.wart_P_seat_taken == 1)
return;
self.veh_in = "warthog";
self.veh_pos = "passanger";
wart.wart_P_seat_taken = 1;
self.follow_veh = wart;
self.loc_dist = wart_passanger;
//self.origin = self.follow_veh.passanger_origin;
sprint(self,"Passanger/n");
};

void wart_gunner_entering(entity wart)
{
if (wart.wart_G_seat_taken == 1)
return;
self.veh_in = "warthog";
self.veh_pos = "gunner"; 
wart.wart_G_seat_taken = 1;
self.follow_veh = wart;
self.loc_dist = wart_gunner;
self.origin = self.follow_veh.origin + wart_gunner;
sprint(self,"Passanger/n");
};

void wart_enter ()
{
set_pos();

local entity e;

e = findradius(use_target.c_wart_driver_enter, 30);
  while(e)
  {
  if (e == self)
  {
	self.origin = use_target.origin + wart_driver;
	wart_driver_entering(use_target);
	return;
  }
  e = e.chain;
  }
  
  e = findradius(use_target.c_wart_passanger_enter, 30);
  while(e)
  {
  if (e == self)
  {
	sprint(self, "Passanger by findradius/n");
	self.origin = use_target.origin + wart_passanger;
	wart_passanger_entering(use_target);
	return;
  }
  e = e.chain;
  }
  
  e = findradius(use_target.c_wart_gunner_enter, 30);
  while(e)
  {
  if (e == self)
  {
	sprint(self, "Gunner by findradius/n");
	self.origin = use_target.origin + wart_gunner;
	wart_gunner_entering(use_target);
	return;
  }
  e = e.chain;
  }
};

/*==============================================================
Mongoouse by Arkage
================================================================*/

// Positions
vector mong_driver; // the spot where the driver sits
vector mong_passanger; // the passanger poition

// enter positions
vector mong_driver_enter; // location to check to seee what seat to enter
vector mong_passanger_enter;

// speed and stuff
float mong_Mspeed_f; // Maxium speed forward
float mong_Mspeed_b; // maxium speed backwards
.float mong_speed;

void mong_enter ()
{
local float pos_weight;
local float mong_driver_weight;
local float mong_passanger_weight;

pos_weight = self.origin_x + self.origin_y;
mong_driver_weight = use_target.origin_x + use_target.origin_y;
mong_passanger_weight = use_target.origin_x + use_target.origin_y;

mong_driver_weight = mong_driver_weight + mong_driver_enter;
mong_passanger_weight = mong_passanger_weight + mong_passanger_enter;

if (pos_weight > mong_passanger_weight)
{

}

};

/*==============================================================
Ghost by Arkage
================================================================*/
// Ghosts max speed
float ghost_max_speed = 2; // was 4


void() ghost_de_accel =
{


if (time_check_set != 1)
{
time_check = time + 0.5;
time_check_set = 1;
}

if (self.speed >= 1 && move_fwd == 0 /*&& time == time_check*/)
{
self.speed = 1; 
//self.velocity_x = self.velocity_x + self.speed;
stuffcmd(self,"cl_movespeedkey ");
stuffcmd(self,ftos(self.speed));
stuffcmd(self,"\n");
stuffcmd(self,"+speed\n");

time_check = time + 0.5;
}

//if (self.speed == 0)
//stuffcmd(self,"-speed\n");
};

void() ghost_accel =
{


if (time_check_set != 1)
{
time_check = time + 0.5;
time_check_set = 1;
}


if (self.speed < ghost_max_speed && move_fwd == 1 /*&& time == time_check*/)
{
self.speed = self.speed + 0.4;  //was 0.4
/*
self.velocity_x = (self.velocity_x) * (self.speed);
self.velocity_y = (self.velocity_y) * (self.speed);
*/
//self.velocity_x = self.velocity_x + self.speed;

stuffcmd(self,"cl_movespeedkey ");
if (self.button1 == 1)
{
stuffcmd(self,ftos(self.speed * 1.5));
}
else
{
stuffcmd(self,ftos(self.speed));
}
stuffcmd(self,"\n");
stuffcmd(self,"+speed\n");

time_check = time + 0.5;


/*
stuffcmd(self,"cl_forwardspeed 200\n");
stuffcmd(self,"cl_movespeedkey ");
stuffcmd(self,ftos(self.speed));
stuffcmd(self,"\n");
*/

}
ghost_de_accel();
};

void() ghost_enter =
{
local entity e;

stuffcmd(self,"sv_maxspeed 99999\n");
stuffcmd(self,"sv_friction 3\n");
e = findradius(use_target.origin, 30);
  while(e)
  {
  if (e == self)
  {
	self.veh_in = "ghost";
	self.follow_veh = use_target;
	stuffcmd(self, "chase_active 1");
	return;
  } 
  e = e.chain;
  }
};

void() ghost_exit =
{
stuffcmd(self,"sv_friction 4\n");
self.veh_in = "";
stuffcmd(self, "chase_active 0");
self.origin = self.origin + '30 30 0';
stuffcmd(self,"-speed\n");
ghost_de_accel();
};


// The use key is golbal for any veichale or iteam for that matter, placed here for handyness
void use_key ()
{
local entity e;

// Check if we are in a vechile if so get out
if (self.veh_in == "warthog")
{
// get out of the hog cheif
if (self.veh_pos == "driver")
{
wart_driver_exit();
return;
}
else if (self.veh_pos == "passanger")
{
wart_passanger_exit();
return;
}
else if (self.veh_pos == "gunner")
{
wart_gunner_exit();
return;
}
}
else if (self.veh_in == "ghost")
{
ghost_exit();
return;
}
	
	// check to see  if we should do some thing
	e = findradius(self.origin, 200);
  while(e)
  {
  if (e.classname == "warthog")
  {
  use_target = e;
  	wart_enter();
  }
  else if (e.classname == "mongoouse")
  {
    use_target = e;
  	mong_enter();
  }
  else if (e.classname == "ghost")
  {
    use_target = e;
  	ghost_enter();
  }
  e = e.chain;
  }
};

/*
	self.button0 = 0;
	self.button1 = 0;
	self.button2 = 0;
	*/