/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2010-2014 QuakeSpasm developers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// sv_user.c -- server code for moving users

#include "quakedef.h"

edict_t	*sv_player;

extern	cvar_t	sv_friction;
cvar_t	sv_edgefriction = {"edgefriction", "2", CVAR_NONE};
extern	cvar_t	sv_stopspeed;

static	vec3_t		forward, right, up;

// world
float	*angles;
float	*origin;
float	*velocity;

qboolean	onground;

usercmd_t	cmd;

cvar_t	sv_idealpitchscale = {"sv_idealpitchscale","0.8",CVAR_NONE};
cvar_t	sv_altnoclip = {"sv_altnoclip","1",CVAR_ARCHIVE}; //johnfitz

qboolean SV_RunThink (edict_t *ent);

/*
===============
SV_SetIdealPitch
===============
*/
#define	MAX_FORWARD	6
void SV_SetIdealPitch (void)
{
	float	angleval, sinval, cosval;
	trace_t	tr;
	vec3_t	top, bottom;
	float	z[MAX_FORWARD];
	int		i, j;
	int		step, dir, steps;

	if (!((int)sv_player->v.flags & FL_ONGROUND))
		return;

	angleval = sv_player->v.angles[YAW] * M_PI*2 / 360;
	sinval = sin(angleval);
	cosval = cos(angleval);

	for (i=0 ; i<MAX_FORWARD ; i++)
	{
		top[0] = sv_player->v.origin[0] + cosval*(i+3)*12;
		top[1] = sv_player->v.origin[1] + sinval*(i+3)*12;
		top[2] = sv_player->v.origin[2] + sv_player->v.view_ofs[2];

		bottom[0] = top[0];
		bottom[1] = top[1];
		bottom[2] = top[2] - 160;

		tr = SV_Move (top, vec3_origin, vec3_origin, bottom, 1, sv_player);
		if (tr.allsolid)
			return;	// looking at a wall, leave ideal the way is was

		if (tr.fraction == 1)
			return;	// near a dropoff

		z[i] = top[2] + tr.fraction*(bottom[2]-top[2]);
	}

	dir = 0;
	steps = 0;
	for (j=1 ; j<i ; j++)
	{
		step = z[j] - z[j-1];
		if (step > -ON_EPSILON && step < ON_EPSILON)
			continue;

		if (dir && ( step-dir > ON_EPSILON || step-dir < -ON_EPSILON ) )
			return;		// mixed changes

		steps++;
		dir = step;
	}

	if (!dir)
	{
		sv_player->v.idealpitch = 0;
		return;
	}

	if (steps < 2)
		return;
	sv_player->v.idealpitch = -dir * sv_idealpitchscale.value;
}


/*
==================
SV_UserFriction

==================
*/
void SV_UserFriction (void)
{
	float	*vel;
	float	speed, newspeed, control;
	vec3_t	start, stop;
	float	friction;
	trace_t	trace;

	vel = velocity;

	speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1]);
	if (!speed)
		return;

// if the leading edge is over a dropoff, increase friction
	start[0] = stop[0] = origin[0] + vel[0]/speed*16;
	start[1] = stop[1] = origin[1] + vel[1]/speed*16;
	start[2] = origin[2] + sv_player->v.mins[2];
	stop[2] = start[2] - 34;

	trace = SV_Move (start, vec3_origin, vec3_origin, stop, true, sv_player);

	if (trace.fraction == 1.0)
		friction = sv_friction.value*sv_edgefriction.value;
	else
		friction = sv_friction.value;

// apply friction
	control = speed < sv_stopspeed.value ? sv_stopspeed.value : speed;
	newspeed = speed - host_frametime*control*friction;

	if (newspeed < 0)
		newspeed = 0;
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

/*
==============
SV_Accelerate
==============
*/
cvar_t	sv_maxspeed = {"sv_maxspeed", "320", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	sv_accelerate = {"sv_accelerate", "10", CVAR_NONE};
void SV_Accelerate (float wishspeed, const vec3_t wishdir)
{
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = sv_accelerate.value*host_frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i=0 ; i<3 ; i++)
		velocity[i] += accelspeed*wishdir[i];
}

void SV_AirAccelerate (float wishspeed, vec3_t wishveloc)
{
	int			i;
	float		addspeed, wishspd, accelspeed, currentspeed;

	wishspd = VectorNormalize (wishveloc);
	if (wishspd > 30)
		wishspd = 30;
	currentspeed = DotProduct (velocity, wishveloc);
	addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;
//	accelspeed = sv_accelerate.value * host_frametime;
	accelspeed = sv_accelerate.value*wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i=0 ; i<3 ; i++)
		velocity[i] += accelspeed*wishveloc[i];
}


void DropPunchAngle (void)
{
	float	len;

	len = VectorNormalize (sv_player->v.punchangle);

	len -= 10*host_frametime;
	if (len < 0)
		len = 0;
	VectorScale (sv_player->v.punchangle, len, sv_player->v.punchangle);
}

/*
===================
SV_WaterMove

===================
*/
void SV_WaterMove (void)
{
	int		i;
	vec3_t	wishvel;
	float	speed, newspeed, wishspeed, addspeed, accelspeed;

//
// user intentions
//
	AngleVectors (sv_player->v.v_angle, forward, right, up);

	for (i=0 ; i<3 ; i++)
		wishvel[i] = forward[i]*cmd.forwardmove + right[i]*cmd.sidemove;

	if (sv_player->onladder)
	{
		wishvel[2] *= 1+fabs(wishvel[2]/200)*9;	//exaggerate vertical movement.
		if (sv_player->v.button2)
			wishvel[2] += 400; //make jump climb (you can turn around and move off to fall)
	}

	if (!cmd.forwardmove && !cmd.sidemove && !cmd.upmove && !sv_player->onladder)
		wishvel[2] -= 60;		// drift towards bottom
	else
		wishvel[2] += cmd.upmove;

	wishspeed = VectorLength(wishvel);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}
	wishspeed *= 0.7;

//
// water friction
//
	speed = VectorLength (velocity);
	if (speed)
	{
		newspeed = speed - host_frametime * speed * sv_friction.value;
		if (newspeed < 0)
			newspeed = 0;
		VectorScale (velocity, newspeed/speed, velocity);
	}
	else
		newspeed = 0;

//
// water acceleration
//
	if (!wishspeed)
		return;

	addspeed = wishspeed - newspeed;
	if (addspeed <= 0)
		return;

	VectorNormalize (wishvel);
	accelspeed = sv_accelerate.value * wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i=0 ; i<3 ; i++)
		velocity[i] += accelspeed * wishvel[i];
}

void SV_WaterJump (void)
{
	if (qcvm->time > sv_player->v.teleport_time
	|| !sv_player->v.waterlevel)
	{
		sv_player->v.flags = (int)sv_player->v.flags & ~FL_WATERJUMP;
		sv_player->v.teleport_time = 0;
	}
	sv_player->v.velocity[0] = sv_player->v.movedir[0];
	sv_player->v.velocity[1] = sv_player->v.movedir[1];
}

/*
===================
SV_NoclipMove -- johnfitz

new, alternate noclip. old noclip is still handled in SV_AirMove
===================
*/
void SV_NoclipMove (void)
{
	AngleVectors (sv_player->v.v_angle, forward, right, up);

	velocity[0] = forward[0]*cmd.forwardmove + right[0]*cmd.sidemove;
	velocity[1] = forward[1]*cmd.forwardmove + right[1]*cmd.sidemove;
	velocity[2] = forward[2]*cmd.forwardmove + right[2]*cmd.sidemove;
	velocity[2] += cmd.upmove*2; //doubled to match running speed

	if (VectorLength (velocity) > sv_maxspeed.value)
	{
		VectorNormalize (velocity);
		VectorScale (velocity, sv_maxspeed.value, velocity);
	}
}

/*
===================
SV_AirMove
===================
*/
void SV_AirMove (void)
{
	int			i;
	vec3_t		wishvel, wishdir;
	float		wishspeed;
	float		fmove, smove;

	AngleVectors (sv_player->v.angles, forward, right, up);

	fmove = cmd.forwardmove;
	smove = cmd.sidemove;

// hack to not let you back into teleporter
	if (qcvm->time < sv_player->v.teleport_time && fmove < 0)
		fmove = 0;

	for (i=0 ; i<3 ; i++)
		wishvel[i] = forward[i]*fmove + right[i]*smove;

	if ( (int)sv_player->v.movetype != MOVETYPE_WALK)
		wishvel[2] = cmd.upmove;
	else
		wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}

	if ( sv_player->v.movetype == MOVETYPE_NOCLIP)
	{	// noclip
		VectorCopy (wishvel, velocity);
	}
	else if ( onground )
	{
		SV_UserFriction ();
		SV_Accelerate (wishspeed, wishdir);
	}
	else
	{	// not on ground, so little effect on velocity
		SV_AirAccelerate (wishspeed, wishvel);
	}
}

/*
===================
SV_ClientThink

the move fields specify an intended velocity in pix/sec
the angle fields specify an exact angular motion in degrees
===================
*/
void SV_ClientThink (void)
{
	vec3_t		v_angle;

	if (sv_player->v.movetype == MOVETYPE_NONE)
		return;

	onground = (int)sv_player->v.flags & FL_ONGROUND;

	origin = sv_player->v.origin;
	velocity = sv_player->v.velocity;

	DropPunchAngle ();

//
// if dead, behave differently
//
	if (sv_player->v.health <= 0)
		return;

//
// angles
// show 1/3 the pitch angle and all the roll angle
	cmd = host_client->cmd;
	angles = sv_player->v.angles;

	VectorAdd (sv_player->v.v_angle, sv_player->v.punchangle, v_angle);
	angles[ROLL] = V_CalcRoll (sv_player->v.angles, sv_player->v.velocity)*4;
	if (!sv_player->v.fixangle)
	{
		angles[PITCH] = -v_angle[PITCH]/3;
		angles[YAW] = v_angle[YAW];
	}

	if ( (int)sv_player->v.flags & FL_WATERJUMP )
	{
		SV_WaterJump ();
		return;
	}
//
// walk
//
	//johnfitz -- alternate noclip
	if (sv_player->v.movetype == MOVETYPE_NOCLIP && sv_altnoclip.value)
		SV_NoclipMove ();
	else if ((sv_player->v.waterlevel >= 2||sv_player->onladder) && sv_player->v.movetype != MOVETYPE_NOCLIP)
		SV_WaterMove ();
	else
		SV_AirMove ();
	//johnfitz
}


/*
===================
SV_ReadClientMove
===================
*/
void SV_ReadClientMove (usercmd_t *move)
{
	int		i;
	vec3_t	angle;
	int		buttonbits;
	int		newimpulse;
	unsigned int inweapon;
	float curs_screen[2];
	vec3_t curs_start, curs_impact;
	unsigned int curs_entity;
	eval_t *eval;
	qboolean drop = false;
	float timestamp;
	vec3_t movevalues;
	int sequence;
	eval_t *val;

	if (host_client->protocol_pext2 & PEXT2_PREDINFO)
	{
		i = (unsigned short)MSG_ReadShort();
		sequence = (host_client->lastmovemessage & 0xffff0000) | (i&0xffff);

		//tollerance of a few old frames, so we can have redundancy for packetloss
		if (sequence+0x100 < host_client->lastmovemessage)
			sequence += 0x10000;

		if (sequence <= host_client->lastmovemessage)
			drop = true;
	}
	else
		sequence = 0;

	//read the data
	timestamp = MSG_ReadFloat();
	for (i=0 ; i<3 ; i++)
	{
		if (sv.protocol == PROTOCOL_NETQUAKE && !(host_client->protocol_pext2 & PEXT2_PREDINFO) && !NET_QSocketGetProQuakeAngleHack(host_client->netconnection))
			angle[i] = MSG_ReadAngle (sv.protocolflags);
		else
			angle[i] = MSG_ReadAngle16 (sv.protocolflags);	//johnfitz -- 16-bit angles for PROTOCOL_FITZQUAKE
	}
	movevalues[0] = MSG_ReadShort ();
	movevalues[1] = MSG_ReadShort ();
	movevalues[2] = MSG_ReadShort ();
	if (host_client->protocol_pext2 & PEXT2_PRYDONCURSOR)
		buttonbits = MSG_ReadLong();
	else
		buttonbits = MSG_ReadByte();
	newimpulse = MSG_ReadByte();

	inweapon = (buttonbits & (1u<<30))?MSG_ReadLong():0;
	curs_screen[0] = (buttonbits & (1u<<31))?MSG_ReadShort()/32767.0:0;
	curs_screen[1] = (buttonbits & (1u<<31))?MSG_ReadShort()/32767.0:0;
	curs_start[0] = (buttonbits & (1u<<31))?MSG_ReadFloat():0;
	curs_start[1] = (buttonbits & (1u<<31))?MSG_ReadFloat():0;
	curs_start[2] = (buttonbits & (1u<<31))?MSG_ReadFloat():0;
	curs_impact[0] = (buttonbits & (1u<<31))?MSG_ReadFloat():0;
	curs_impact[1] = (buttonbits & (1u<<31))?MSG_ReadFloat():0;
	curs_impact[2] = (buttonbits & (1u<<31))?MSG_ReadFloat():0;
	curs_entity = (buttonbits & (1u<<31))?MSG_ReadEntity(host_client->protocol_pext2):0;
	buttonbits &= ~((1u<<30)|(1u<<31));

	if (drop)
		return;	//okay, we don't care about that then

// calc ping times
	host_client->lastmovemessage = sequence;
	if (!(host_client->protocol_pext2 & PEXT2_PREDINFO))
	{
		host_client->ping_times[host_client->num_pings%NUM_PING_TIMES]
			= qcvm->time - timestamp;
		host_client->num_pings++;
	}	//otherwise time is still useful for determining the input frame's time value

	// read movement
	VectorCopy (angle, host_client->edict->v.v_angle);
	move->forwardmove = movevalues[0];
	move->sidemove = movevalues[1];
	move->upmove = movevalues[2];

// read buttons
	host_client->edict->v.button0 = (buttonbits & 1)>>0;
	//button1 was meant to be 'use', but got reused by too many mods to get implemented now
	host_client->edict->v.button2 = (buttonbits & 2)>>1;
	if ((val = GetEdictFieldValue(host_client->edict, qcvm->extfields.button3)))
		val->_float = (buttonbits & 4)>>2;
	if ((val = GetEdictFieldValue(host_client->edict, qcvm->extfields.button4)))
		val->_float = (buttonbits & 8)>>3;
	if ((val = GetEdictFieldValue(host_client->edict, qcvm->extfields.button5)))
		val->_float = (buttonbits & 0x10)>>4;
	if ((val = GetEdictFieldValue(host_client->edict, qcvm->extfields.button6)))
		val->_float = (buttonbits & 0x20)>>5;
	if ((val = GetEdictFieldValue(host_client->edict, qcvm->extfields.button7)))
		val->_float = (buttonbits & 0x40)>>6;
	if ((val = GetEdictFieldValue(host_client->edict, qcvm->extfields.button8)))
		val->_float = (buttonbits & 0x80)>>7;

	if (newimpulse)
		host_client->edict->v.impulse = newimpulse;

	eval = GetEdictFieldValue(host_client->edict, qcvm->extfields.movement);
	if (eval)
	{
		eval->vector[0] = move->forwardmove;
		eval->vector[1] = move->sidemove;
		eval->vector[2] = move->upmove;
	}

	//FIXME: attempt to apply physics command now, if the mod has custom physics+csqc-prediction


	if (qcvm->extfuncs.SV_RunClientCommand)
	{
		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram(pr_global_struct->PlayerPreThink);

		if (!SV_RunThink (host_client->edict))
			return;	//ent was removed? o.O

		if (timestamp > qcvm->time)
			timestamp = qcvm->time;					//don't let the client exceed the current time
		if (timestamp < qcvm->time-0.5)
			timestamp = qcvm->time-0.5;				//don't let the client bank too much time for bursts...
		if (timestamp < host_client->lastmovetime)
			timestamp = host_client->lastmovetime;	//don't let the client report times in the past (to get extra time with the next clc_move)
		if (qcvm->extglobals.input_timelength)
			*qcvm->extglobals.input_timelength = timestamp - host_client->lastmovetime;
		host_client->lastmovetime = timestamp;

		if (qcvm->extglobals.input_buttons)
			*qcvm->extglobals.input_buttons = buttonbits;
		if (qcvm->extglobals.input_impulse)
			*qcvm->extglobals.input_impulse = newimpulse;
		if (qcvm->extglobals.input_movevalues)
			VectorCopy(movevalues, qcvm->extglobals.input_movevalues);
		if (qcvm->extglobals.input_angles)
			VectorCopy(angle, qcvm->extglobals.input_angles);
		if (qcvm->extglobals.input_weapon)
			*qcvm->extglobals.input_weapon = inweapon;
		if (qcvm->extglobals.input_cursor_screen)
			qcvm->extglobals.input_cursor_screen[0] = curs_screen[0], qcvm->extglobals.input_cursor_screen[1] = curs_screen[1];
		if (qcvm->extglobals.input_cursor_trace_start)
			VectorCopy(curs_start, qcvm->extglobals.input_cursor_trace_start);
		if (qcvm->extglobals.input_cursor_trace_endpos)
			VectorCopy(curs_impact, qcvm->extglobals.input_cursor_trace_endpos);
		if (qcvm->extglobals.input_cursor_entitynumber)
			*qcvm->extglobals.input_cursor_entitynumber = curs_entity;

		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram(qcvm->extfuncs.SV_RunClientCommand);


		pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
		PR_ExecuteProgram(pr_global_struct->PlayerPostThink);
	}
}

void SV_ReadQCRequest(void)
{
	int e;
	char args[8];
	const char *rname, *fname;
	func_t f;
	int i;
	client_t *cl = host_client;

	for (i = 0; ; )
	{
		byte ev = MSG_ReadByte();
		/*if (ev >= 200 && ev < 200+MAX_SPLITS)
		{
			ev -= 200;
			while (ev-- && cl)
				cl = cl->controlled;
			continue;
		}*/
		if (i >= sizeof(args)-1)
		{
			if (ev != ev_void)
			{
				msg_badread = true;
				return;
			}
			goto done;
		}
		switch(ev)
		{
		default:
			args[i] = '?';
			G_INT(OFS_PARM0+i*3) = MSG_ReadLong();
			break;
		case ev_void:
			goto done;
		case ev_float:
			args[i] = 'f';
			G_FLOAT(OFS_PARM0+i*3) = MSG_ReadFloat();
			break;
		case ev_vector:
			args[i] = 'v';
			G_FLOAT(OFS_PARM0+i*3+0) = MSG_ReadFloat();
			G_FLOAT(OFS_PARM0+i*3+1) = MSG_ReadFloat();
			G_FLOAT(OFS_PARM0+i*3+2) = MSG_ReadFloat();
			break;
		case ev_ext_integer:
			args[i] = 'i';
			G_INT(OFS_PARM0+i*3) = MSG_ReadLong();
			break;
		case ev_string:
			args[i] = 's';
			G_INT(OFS_PARM0+i*3) = PR_MakeTempString(MSG_ReadString());
			break;
		case ev_entity:
			args[i] = 'e';
			e = MSG_ReadEntity(host_client->protocol_pext2);
			if (e < 0 || e >= qcvm->num_edicts)
				e = 0;
			G_INT(OFS_PARM0+i*3) = EDICT_TO_PROG(EDICT_NUM(e));
			break;
		}
		i++;
	}

done:
	args[i] = 0;
	rname = MSG_ReadString();
	if (i)
		fname = va("CSEv_%s_%s", rname, args);
	else
		fname = va("CSEv_%s", rname);
	f = PR_FindExtFunction(fname);
	/*if (!f)
	{
		if (i)
			rname = va("Cmd_%s_%s", rname, args);
		else
			rname = va("Cmd_%s", rname);
		f = PR_FindExtFunction(rname);
	}*/
	if (!cl)
		;	//bad seat! not going to warn as they might have been removed recently
	else if (f)
	{
		pr_global_struct->self = EDICT_TO_PROG(cl->edict);
		PR_ExecuteProgram(f);
	}
	else
		SV_ClientPrintf("qcrequest \"%s\" not supported\n", fname);
}

/*
===================
SV_ReadClientMessage

Returns false if the client should be killed
===================
*/
qboolean SV_ReadClientMessage (void)
{
	int		ccmd;
	const char	*s;

	MSG_BeginReading ();

	while (1)
	{
		if (!host_client->active)
			return false;	// a command caused an error

		if (msg_badread)
		{
			Sys_Printf ("SV_ReadClientMessage: badread\n");
			return false;
		}

		ccmd = MSG_ReadChar ();

		switch (ccmd)
		{
		case -1:
			return true;	//msg_badread, meaning we just hit eof.

		default:
			Sys_Printf ("SV_ReadClientMessage: unknown command char\n");
			return false;

		case clc_nop:
//			Sys_Printf ("clc_nop\n");
			break;

		case clc_stringcmd:
			s = MSG_ReadString ();
			if (q_strncasecmp(s, "spawn", 5) && q_strncasecmp(s, "begin", 5) && q_strncasecmp(s, "prespawn", 8) && qcvm->extfuncs.SV_ParseClientCommand)
			{	//the spawn/begin/prespawn are because of numerous mods that disobey the rules.
				//at a minimum, we must be able to join the server, so that we can see any sprints/bprints (because dprint sucks, yes there's proper ways to deal with this, but moders don't always know them).
				client_t *ohc = host_client;
				G_INT(OFS_PARM0) = PR_SetEngineString(s);
				pr_global_struct->time = qcvm->time;
				pr_global_struct->self = EDICT_TO_PROG(host_client->edict);
				PR_ExecuteProgram(qcvm->extfuncs.SV_ParseClientCommand);
				host_client = ohc;
			}
			else
				Cmd_ExecuteString (s, src_client);
			break;

		case clc_disconnect:
//			Sys_Printf ("SV_ReadClientMessage: client disconnected\n");
			return false;

		case clc_move:
			if (!host_client->spawned)
				return true;	//this is to suck up any stale moves on map changes, so we don't get confused (quite so easily) when protocols are changed between maps
			SV_ReadClientMove (&host_client->cmd);
			break;

		case clcdp_ackframe:
			SVFTE_Ack(host_client, MSG_ReadLong());
			break;

		case clcdp_ackdownloaddata:
			Host_DownloadAck(host_client);
			break;

		case clcfte_qcrequest:
			SV_ReadQCRequest();
			break;

		case clcfte_voicechat:
			SV_VoiceReadPacket(host_client);
			break;
		}
	}

	return true;
}


/*
==================
SV_RunClients
==================
*/
void SV_RunClients (void)
{
	int				i;

	//receive from clients first
	//Spike -- reworked this to query the network code for an active connection.
	//this allows the network code to serve multiple clients with the same listening port.
	//this solves server-side nats, which is important for coop etc.
	while(1)
	{
		struct qsocket_s *sock = NET_GetServerMessage();
		if (!sock)
			break;	//no more this frame

		for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
		{
			if (host_client->netconnection == sock)
			{
				sv_player = host_client->edict;
				if (!SV_ReadClientMessage ())
				{
					SV_DropClient (false);	// client misbehaved...
					break;
				}
			}
		}
	}

	//then do the per-frame stuff
	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		sv_player = host_client->edict;

		if (!host_client->spawned)
		{
		// clear client movement until a new packet is received
			memset (&host_client->cmd, 0, sizeof(host_client->cmd));
			continue;
		}

		if (!host_client->netconnection)
		{
			//botclients can't receive packets. don't even try.
			//not sure where to put this code, but here seems sane enough.
			//fill in the user's desired stuff according to a few things.
			eval_t *ev = GetEdictFieldValue(host_client->edict, qcvm->extfields.movement);
			if (ev)	//.movement normally works the other way around. oh well.
			{
				host_client->cmd.forwardmove = ev->vector[0];
				host_client->cmd.sidemove = ev->vector[1];
				host_client->cmd.upmove = ev->vector[2];
			}
			host_client->cmd.viewangles[0] = host_client->edict->v.v_angle[0];
			host_client->cmd.viewangles[1] = host_client->edict->v.v_angle[1];
			host_client->cmd.viewangles[2] = host_client->edict->v.v_angle[2];
		}

// always pause in single player if in console or menus
		if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
			SV_ClientThink ();
	}
}
