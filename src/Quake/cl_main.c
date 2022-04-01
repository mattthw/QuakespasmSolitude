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
// cl_main.c  -- client main loop

#include "quakedef.h"
#include "bgmusic.h"

#include "arch_def.h"
#ifdef PLATFORM_UNIX
//for unlink
#include <unistd.h>
#endif

// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t	cl_name = {"name", "player", CVAR_ARCHIVE | CVAR_USERINFO};
cvar_t	cl_topcolor = {"topcolor", "", CVAR_ARCHIVE | CVAR_USERINFO};
cvar_t	cl_bottomcolor = {"bottomcolor", "", CVAR_ARCHIVE | CVAR_USERINFO};

cvar_t	cl_shownet = {"cl_shownet","0",CVAR_NONE};	// can be 0, 1, or 2
cvar_t	cl_nolerp = {"cl_nolerp","0",CVAR_NONE};

cvar_t	cfg_unbindall = {"cfg_unbindall", "1", CVAR_ARCHIVE};

cvar_t	lookspring = {"lookspring","0", CVAR_ARCHIVE};
cvar_t	lookstrafe = {"lookstrafe","0", CVAR_ARCHIVE};
cvar_t	sensitivity = {"sensitivity","3", CVAR_ARCHIVE};

cvar_t	m_pitch = {"m_pitch","0.022", CVAR_ARCHIVE};
cvar_t	m_yaw = {"m_yaw","0.022", CVAR_ARCHIVE};
cvar_t	m_forward = {"m_forward","1", CVAR_ARCHIVE};
cvar_t	m_side = {"m_side","0.8", CVAR_ARCHIVE};

cvar_t	cl_maxpitch = {"cl_maxpitch", "90", CVAR_ARCHIVE}; //johnfitz -- variable pitch clamping
cvar_t	cl_minpitch = {"cl_minpitch", "-90", CVAR_ARCHIVE}; //johnfitz -- variable pitch clamping

cvar_t cl_recordingdemo = {"cl_recordingdemo", "", CVAR_ROM};	//the name of the currently-recording demo.

client_static_t	cls;
client_state_t	cl;
// FIXME: put these on hunk?
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

int				cl_numvisedicts;
int				cl_maxvisedicts;
entity_t		**cl_visedicts;

extern cvar_t	r_lerpmodels, r_lerpmove; //johnfitz
extern float	host_netinterval;	//Spike

void CL_ClearTrailStates(void)
{
	int i;
	for (i = 0; i < cl.num_statics; i++)
	{
		PScript_DelinkTrailstate(&(cl.static_entities[i]->trailstate));
		PScript_DelinkTrailstate(&(cl.static_entities[i]->emitstate));
	}
	for (i = 0; i < cl.max_edicts; i++)
	{
		PScript_DelinkTrailstate(&(cl.entities[i].trailstate));
		PScript_DelinkTrailstate(&(cl.entities[i].emitstate));
	}
	for (i = 0; i < MAX_BEAMS; i++)
	{
		PScript_DelinkTrailstate(&(cl_beams[i].trailstate));
	}
}

void CL_FreeState(void)
{
	int i;
	for (i = 0; i < MAX_CL_STATS; i++)
		free(cl.statss[i]);
	CL_ClearTrailStates();
	PR_ClearProgs(&cl.qcvm);
	free(cl.static_entities);
	free(cl.ssqc_to_csqc);
	memset (&cl, 0, sizeof(cl));
}

/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	if (cl.qcvm.extfuncs.CSQC_Shutdown)
	{
		PR_SwitchQCVM(&cl.qcvm);
		PR_ExecuteProgram(qcvm->extfuncs.CSQC_Shutdown);
		qcvm->extfuncs.CSQC_Shutdown = 0;
		PR_SwitchQCVM(NULL);
	}

	if (!sv.active)
		Host_ClearMemory ();

// wipe the entire cl structure
	CL_FreeState();

	SZ_Clear (&cls.message);

// clear other arrays
	memset (cl_dlights, 0, sizeof(cl_dlights));
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	memset (cl_temp_entities, 0, sizeof(cl_temp_entities));
	memset (cl_beams, 0, sizeof(cl_beams));

	//johnfitz -- cl_entities is now dynamically allocated
	cl.max_edicts = CLAMP (MIN_EDICTS,(int)max_edicts.value,MAX_EDICTS);
	cl.entities = (entity_t *) Hunk_AllocName (cl.max_edicts*sizeof(entity_t), "cl_entities");
	//johnfitz

	//Spike -- this stuff needs to get reset to defaults.
	cl.csqc_sensitivity = 1;

	cl.viewent.netstate = nullentitystate;
#ifdef PSET_SCRIPT
	PScript_Shutdown();
#endif

	if (!sv.active)
		Draw_ReloadTextures(false);
}

/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
	if (key_dest == key_message)
		Key_EndChat ();	// don't get stuck in chat mode

// stop sounds (especially looping!)
	S_StopAllSounds (true);
	BGM_Stop();
	CDAudio_Stop();

// if running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);
		cls.netcon = NULL;

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.demopaused = false;
	cls.signon = 0;
	cls.netcon = NULL;
	if (cls.download.file)
		fclose(cls.download.file);
	memset(&cls.download, 0, sizeof(cls.download));
	cl.intermission = 0;
	cl.worldmodel = NULL;
	cl.sendprespawn = false;
}

void CL_Disconnect_f (void)
{
	CL_Disconnect ();
	if (sv.active)
		Host_ShutdownServer (false);
}


/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (const char *host)
{
	static char lasthost[NET_NAMELEN];
	if (cls.state == ca_dedicated)
		return;

	if (cls.demoplayback)
		return;
	if (!host)
	{
		host = lasthost;
		if (!*host)
			return;
	}
	else
		q_strlcpy(lasthost, host, sizeof(lasthost));

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("CL_Connect: connect failed\n");
	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);

	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing
	MSG_WriteByte (&cls.message, clc_nop);	// NAT Fix from ProQuake
}

void CL_SendInitialUserinfo(void *ctx, const char *key, const char *val)
{
	if (*key == '*')
		return;	//servers don't like that sort of userinfo key
	if (!strcmp(key, "name"))
		return;	//already unconditionally sent earlier.
	MSG_WriteByte (&cls.message, clc_stringcmd);
	MSG_WriteString (&cls.message, va("setinfo \"%s\" \"%s\"\n", key, val));
}
/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char 	str[8192];

	Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_name.string));

		cl.sendprespawn = true;
		break;

	case 2:

		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", (int)cl_topcolor.value, (int)cl_bottomcolor.value));

		if (*cl.serverinfo)
			Info_Enumerate(cls.userinfo, CL_SendInitialUserinfo, NULL);

		MSG_WriteByte (&cls.message, clc_stringcmd);
		sprintf (str, "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);
		break;

	case 3:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
		break;

	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
void CL_NextDemo (void)
{
	char	str[1024];

	if (cls.demonum == -1)
		return;		// don't play demos

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			CL_Disconnect();
			return;
		}
	}

	SCR_BeginLoadingPlaque ();

	sprintf (str,"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int			i;

	if (cls.state != ca_connected)
		return;

	for (i=0,ent=cl.entities ; i<cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}

/*
===============
CL_AllocDlight

===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int		i;
	dlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				dl->color[0] = dl->color[1] = dl->color[2] = 1; //johnfitz -- lit support via lordhavoc
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			dl->color[0] = dl->color[1] = dl->color[2] = 1; //johnfitz -- lit support via lordhavoc
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	dl->color[0] = dl->color[1] = dl->color[2] = 1; //johnfitz -- lit support via lordhavoc
	return dl;
}


/*
===============
CL_DecayLights

===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	float		time;

	time = cl.time - cl.oldtime;
	if (time < 0)
		return;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;

		dl->radius -= time*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}


/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
float	CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];

	if (!f || cls.timedemo || (sv.active && !host_netinterval))
	{
		cl.time = cl.mtime[0];
		return 1;
	}

	if (f > 0.1) // dropped packet, or start of demo
	{
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = 0.1;
	}

	frac = (cl.time - cl.mtime[1]) / f;

	if (frac < 0)
	{
		if (frac < -0.01)
			cl.time = cl.mtime[1];
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
			cl.time = cl.mtime[0];
		frac = 1;
	}

	//johnfitz -- better nolerp behavior
	if (cl_nolerp.value)
		return 1;
	//johnfitz

	return frac;
}

static qboolean CL_LerpEntity(entity_t *ent, vec3_t org, vec3_t ang, float frac)
{
	float f, d;
	int j;
	vec3_t delta;
	qboolean teleported = false;
	//figure out the pos+angles of the parent
	if (ent->forcelink)
	{	// the entity was not updated in the last message
		// so move to the final spot
		VectorCopy (ent->msg_origins[0], org);
		VectorCopy (ent->msg_angles[0], ang);
	}
	else
	{	// if the delta is large, assume a teleport and don't lerp
		f = frac;
		for (j=0 ; j<3 ; j++)
		{
			delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
			if (delta[j] > 100 || delta[j] < -100)
			{
				f = 1;		// assume a teleportation, not a motion
				teleported = true;	//johnfitz -- don't lerp teleports
			}
		}

		//johnfitz -- don't cl_lerp entities that will be r_lerped
		if (r_lerpmove.value && (ent->lerpflags & LERP_MOVESTEP))
			f = 1;
		//johnfitz

	// interpolate the origin and angles
		for (j=0 ; j<3 ; j++)
		{
			org[j] = ent->msg_origins[1][j] + f*delta[j];

			d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			ang[j] = ent->msg_angles[1][j] + f*d;
		}
	}
	return teleported;
}

static qboolean CL_AttachEntity(entity_t *ent, float frac)
{
	entity_t *parent;
	vec3_t porg, pang;
	vec3_t paxis[3];
	vec3_t tmp, fwd, up;
	unsigned int tagent = ent->netstate.tagentity;
	int runaway = 0;

	while(1)
	{
		if (!tagent)
			return true;	//nothing to do.
		if (runaway++==10 || tagent >= (unsigned int)cl.num_entities)
			return false;	//parent isn't valid
		parent = &cl.entities[tagent];

		if (tagent == cl.viewentity)
			ent->eflags |= EFLAGS_EXTERIORMODEL;

		if (!parent->model)
			return false;
		if (0)//tagent < ent-cl_entities)
		{
			tagent = parent->netstate.tagentity;
			VectorCopy(parent->origin, porg);
			VectorCopy(parent->angles, pang);
		}
		else
		{
			tagent = parent->netstate.tagentity;
			CL_LerpEntity(parent, porg, pang, frac);
		}

		//FIXME: this code needs to know the exact lerp info of the underlaying model.
		//however for some idiotic reason, someone decided to figure out what should be displayed somewhere far removed from the code that deals with timing
		//so we have absolutely no way to get a reliable origin
		//in the meantime, r_lerpmove 0; r_lerpmodels 0
		//you might be able to work around it by setting the attached entity to movetype_step to match the attachee, and to avoid EF_MUZZLEFLASH.
		//personally I'm just going to call it a quakespasm bug that I cba to fix.

		//FIXME: update porg+pang according to the tag index (we don't support md3s/iqms, so we don't need to do anything here yet)

		if (parent->model && parent->model->type == mod_alias)
			pang[0] *= -1;
		AngleVectors(pang, paxis[0], paxis[1], paxis[2]);

		if (ent->model && ent->model->type == mod_alias)
			ent->angles[0] *= -1;
		AngleVectors(ent->angles, fwd, tmp, up);

		//transform the origin
		VectorMA(parent->origin, ent->origin[0], paxis[0], tmp);
		VectorMA(tmp, -ent->origin[1], paxis[1], tmp);
		VectorMA(tmp, ent->origin[2], paxis[2], ent->origin);

		//transform the forward vector
		VectorMA(vec3_origin, fwd[0], paxis[0], tmp);
		VectorMA(tmp, -fwd[1], paxis[1], tmp);
		VectorMA(tmp, fwd[2], paxis[2], fwd);
		//transform the up vector
		VectorMA(vec3_origin, up[0], paxis[0], tmp);
		VectorMA(tmp, -up[1], paxis[1], tmp);
		VectorMA(tmp, up[2], paxis[2], up);
		//regenerate the new angles.
		VectorAngles(fwd, up, ent->angles);
		if (ent->model && ent->model->type == mod_alias)
			ent->angles[0] *= -1;

		ent->eflags |= parent->netstate.eflags & (EFLAGS_VIEWMODEL|EFLAGS_EXTERIORMODEL);
	}
}

/*
===============
CL_RelinkEntities
===============
*/
void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, d;
	float		bobjrotate;
	vec3_t		oldorg;
	dlight_t	*dl;
	float		frametime;
	int			modelflags;

// determine partial update time
	frac = CL_LerpPoint ();

	frametime = cl.time - cl.oldtime;
	if (frametime < 0)
		frametime = 0;
	if (frametime > 0.1)
		frametime = 0.1;

	if (cl_numvisedicts + 64 > cl_maxvisedicts)
	{
		cl_maxvisedicts = cl_maxvisedicts+64;
		cl_visedicts = realloc(cl_visedicts, sizeof(*cl_visedicts)*cl_maxvisedicts);
	}
	cl_numvisedicts = 0;

//
// interpolate player info
//
	for (i=0 ; i<3 ; i++)
		cl.velocity[i] = cl.mvelocity[1][i] +
			frac * (cl.mvelocity[0][i] - cl.mvelocity[1][i]);

	if (cls.demoplayback)
	{
	// interpolate the angles
		for (j=0 ; j<3 ; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}

	bobjrotate = anglemod(100*cl.time);

// start on the entity after the world
	for (i=1,ent=cl.entities+1 ; i<cl.num_entities ; i++,ent++)
	{
		if (!ent->model)
		{	// empty slot, ish.
			
			// ericw -- efrags are only used for static entities in GLQuake
			// ent can't be static, so this is a no-op.
			//if (ent->forcelink)
			//	R_RemoveEfrags (ent);	// just became empty
			continue;
		}
		ent->eflags = ent->netstate.eflags;

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0])
		{
			ent->model = NULL;
			ent->lerpflags |= LERP_RESETMOVE|LERP_RESETANIM; //johnfitz -- next time this entity slot is reused, the lerp will need to be reset
			continue;
		}

		VectorCopy (ent->origin, oldorg);

		if (CL_LerpEntity(ent, ent->origin, ent->angles, frac))
			ent->lerpflags |= LERP_RESETMOVE;

		if (ent->netstate.tagentity)
		if (!CL_AttachEntity(ent, frac))
		{
			//can't draw it if we don't know where its parent is.
			continue;
		}

		modelflags = (ent->effects>>24)&0xff;
		if (!(ent->effects & EF_NOMODELFLAGS))
			modelflags |= ent->model->flags;

// rotate binary objects locally
		if (modelflags & EF_ROTATE)
			ent->angles[1] = bobjrotate;

		if (ent->effects & EF_BRIGHTFIELD)
			R_EntityParticles (ent);

		if (ent->effects & EF_MUZZLEFLASH)
		{
			vec3_t		fv, rv, uv;

			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			AngleVectors (ent->angles, fv, rv, uv);

			VectorMA (dl->origin, 18, fv, dl->origin);
			dl->radius = 200 + (rand()&31);
			dl->minlight = 32;
			dl->die = cl.time + 0.1;

			//johnfitz -- assume muzzle flash accompanied by muzzle flare, which looks bad when lerped
			if (r_lerpmodels.value != 2)
			{
				if (ent == &cl.entities[cl.viewentity])
					cl.viewent.lerpflags |= LERP_RESETANIM|LERP_RESETANIM2; //no lerping for two frames
				else
					ent->lerpflags |= LERP_RESETANIM|LERP_RESETANIM2; //no lerping for two frames
			}
			//johnfitz
		}
		if (ent->effects & EF_BRIGHTLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->origin[2] += 16;
			dl->radius = 400 + (rand()&31);
			dl->die = cl.time + 0.001;
		}
		if (ent->effects & EF_DIMLIGHT)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin,  dl->origin);
			dl->radius = 200 + (rand()&31);
			dl->die = cl.time + 0.001;
		}

#ifdef PSET_SCRIPT
		if (cl.paused)
			;
		else if (ent->netstate.traileffectnum > 0 && ent->netstate.traileffectnum < MAX_PARTICLETYPES)
		{
			vec3_t axis[3];
			AngleVectors(ent->angles, axis[0], axis[1], axis[2]);
			PScript_ParticleTrail(oldorg, ent->origin, cl.particle_precache[ent->netstate.traileffectnum].index, frametime, i, axis, &ent->trailstate);
		}
		else if (ent->model->traileffect >= 0)
		{
			vec3_t axis[3];
			AngleVectors(ent->angles, axis[0], axis[1], axis[2]);
			PScript_ParticleTrail(oldorg, ent->origin, ent->model->traileffect, frametime, i, axis, &ent->trailstate);
		}
		else
#endif
			if (modelflags & EF_GIB)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_BLOOD"))
				R_RocketTrail (oldorg, ent->origin, 2);
		}
		else if (modelflags & EF_ZOMGIB)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_SLIGHTBLOOD"))
				R_RocketTrail (oldorg, ent->origin, 4);
		}
		else if (modelflags & EF_TRACER)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_WIZSPIKE"))
				R_RocketTrail (oldorg, ent->origin, 3);
		}
		else if (modelflags & EF_TRACER2)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_KNIGHTSPIKE"))
				R_RocketTrail (oldorg, ent->origin, 5);
		}
		else if (modelflags & EF_ROCKET)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_ROCKET"))
				R_RocketTrail (oldorg, ent->origin, 0);
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = 200;
			dl->die = cl.time + 0.01;
		}
		else if (modelflags & EF_GRENADE)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_GRENADE"))
				R_RocketTrail (oldorg, ent->origin, 1);
		}
		else if (modelflags & EF_TRACER3)
		{
			if (PScript_EntParticleTrail(oldorg, ent, "TR_VORESPIKE"))
				R_RocketTrail (oldorg, ent->origin, 6);
		}

		ent->forcelink = false;

#ifdef PSET_SCRIPT
		if (ent->netstate.emiteffectnum > 0)
		{
			vec3_t axis[3];
			AngleVectors(ent->angles, axis[0], axis[1], axis[2]);
			if (ent->model->type == mod_alias)
				axis[0][2] *= -1;	//stupid vanilla bug
			PScript_RunParticleEffectState(ent->origin, axis[0], frametime, cl.particle_precache[ent->netstate.emiteffectnum].index, &ent->emitstate);
		}
		else if (ent->model->emiteffect >= 0)
		{
			vec3_t axis[3];
			AngleVectors(ent->angles, axis[0], axis[1], axis[2]);
			if (ent->model->flags & MOD_EMITFORWARDS)
			{
				if (ent->model->type == mod_alias)
					axis[0][2] *= -1;	//stupid vanilla bug
			}
			else
				VectorScale(axis[2], -1, axis[0]);
			PScript_RunParticleEffectState(ent->origin, axis[0], frametime, ent->model->emiteffect, &ent->emitstate);
			if (ent->model->flags & MOD_EMITREPLACE)
				continue;
		}
#endif

		if (i == cl.viewentity && !chase_active.value)
			continue;

		if (cl_numvisedicts < cl_maxvisedicts)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}

#ifdef PSET_SCRIPT
int CL_GenerateRandomParticlePrecache(const char *pname)
{	//for dpp7 compat
	size_t i;
	pname = va("%s", pname);
	for (i = 1; i < MAX_PARTICLETYPES; i++)
	{
		if (!cl.particle_precache[i].name)
		{
			cl.particle_precache[i].name = strcpy(Hunk_Alloc(strlen(pname)+1), pname);
			cl.particle_precache[i].index = PScript_FindParticleType(cl.particle_precache[i].name);
			return i;
		}
		if (!strcmp(cl.particle_precache[i].name, pname))
			return i;
	}
	return 0;
}
#endif

//sent by the server to let us know that dp downloads can be used
void CL_ServerExtension_Download_f(void)
{
	if (Cmd_Argc() == 2)
		cl.protocol_dpdownload = atoi(Cmd_Argv(1));
}

//sent by the server to let us know when its finished sending the entire file
void CL_Download_Finished_f(void)
{
	if (cls.download.file)
	{
		char finalpath[MAX_OSPATH];
		unsigned int size = strtoul(Cmd_Argv(1), NULL, 0);
		unsigned int hash = strtoul(Cmd_Argv(2), NULL, 0);
		//const char *fname = Cmd_Argv(3);
		qboolean hashokay = false;
		if (size == cls.download.size)
		{
			byte *tmp = malloc(size);
			if (tmp)
			{
				fseek(cls.download.file, 0, SEEK_SET);
				fread(tmp, 1, size, cls.download.file);
				hashokay = (hash == CRC_Block(tmp, size));
				free(tmp);

				if (!hashokay) Con_Warning("Download hash failure\n");
			}
			else Con_Warning("Download size too large\n");
		}
		else Con_Warning("Download size mismatch\n");

		fclose(cls.download.file);
		cls.download.file = NULL;
		if (hashokay)
		{
			q_snprintf (finalpath, sizeof(finalpath), "%s/%s", com_gamedir, cls.download.current);
			rename(cls.download.temp, finalpath);
			Con_SafePrintf("Downloaded %s: %u bytes\n", cls.download.current, cls.download.size);
		}
		else
		{
			Con_Warning("Download of %s failed\n", cls.download.current);
			unlink(cls.download.temp);	//kill the temp
		}
	}

	cls.download.active = false;
}
//sent by the server (or issued by the user) to stop the current download for any reason.
void CL_StopDownload_f(void)
{
	if (cls.download.file)
	{
		fclose(cls.download.file);
		cls.download.file = NULL;
		unlink(cls.download.temp);

//		Con_SafePrintf("Download cancelled\n", cl.download_current, cl.download_size);
	}
	cls.download.active = false;
}
//sent by the server to let us know that its going to start spamming us now.
void CL_Download_Begin_f(void)
{
	if (!cls.download.active)
		return;

	if (cls.download.file)
		CL_StopDownload_f();

	//cl_downloadbegin size "name"
	cls.download.size = strtoul(Cmd_Argv(1), NULL, 0);

	COM_CreatePath(cls.download.temp);
	cls.download.file = fopen(cls.download.temp, "wb+");	//+ so we can read the data back to validate it

	MSG_WriteByte (&cls.message, clc_stringcmd);
	MSG_WriteString (&cls.message, "sv_startdownload\n");
}

void CL_Download_Data(void)
{
	byte *data;
	unsigned int start, size;
	start = MSG_ReadLong();
	size = (unsigned short)MSG_ReadShort();
	data = MSG_ReadData(size);
	if (msg_badread)
		return;
	if (!cls.download.file)
		return;	//demo started mid-record? something weird anyway

	fseek(cls.download.file, start, SEEK_SET);
	fwrite(data, 1, size, cls.download.file);

	Con_SafePrintf("Downloading %s: %g%%\r", cls.download.current, 100*(start+size) / (double)cls.download.size);

	//should maybe use unreliables, but whatever, shouldn't matter too much, it'll still complete
	MSG_WriteByte(&cls.message, clcdp_ackdownloaddata);
	MSG_WriteLong(&cls.message, start);
	MSG_WriteShort(&cls.message, size);
}

//returns true if we should block waiting for a download, false if there's no point.
qboolean CL_CheckDownload(const char *filename)
{
	if (sv.active)
		return false;	//no point downloading if we're the server...
	if (*filename == '*')
		return false;	//don't download these...
	if (cls.download.active)
		return true;	//block while we're already downloading something
	if (!cl.protocol_dpdownload)
		return false;	//can't download anyway
	if (*cls.download.current && !strcmp(cls.download.current, filename))
		return false;	//if the previous download failed, don't endlessly retry.
	if (COM_FileExists(filename, NULL))
		return false;	//no need to download anything.
	if (!COM_DownloadNameOkay(filename))
		return false;	//diediedie

	cls.download.active = true;
	q_strlcpy(cls.download.current, filename, sizeof(cls.download.current));
	q_snprintf (cls.download.temp, sizeof(cls.download.temp), "%s/%s.tmp", com_gamedir, filename);
	Con_Printf("Downloading %s...\r", filename);
	MSG_WriteByte (&cls.message, clc_stringcmd);
	MSG_WriteString (&cls.message, va("download \"%s\"\n", filename));
	return true;
}

//download+load models and sounds as needed, once complete let the server know we're ready for the next stage.
//returning false will trigger nops.
qboolean CL_CheckDownloads(void)
{
	int i;
	if (cl.model_download == 0 && cl.model_count && cl.model_name[1])
	{	//haxors, download the lit first, but only if we don't already have the bsp
		//this ensures that we don't keep requesting the lit for maps that just don't have one (although may be problematic if the first server we find deleted them all, but oh well)
		char litname[MAX_QPATH];
		char *ext;
		q_strlcpy(litname, cl.model_name[1], sizeof(litname));
		ext = (char*)COM_FileGetExtension(litname);
		if (!q_strcasecmp(ext, "bsp"))
		{
			if (!COM_FileExists(litname, NULL))
			{
				strcpy(ext, "lit");
				if (CL_CheckDownload(litname))
					return false;
			}
		}
		cl.model_download++;
	}
	for (; cl.model_download < cl.model_count; )
	{
		if (*cl.model_name[cl.model_download])
		{
			if (CL_CheckDownload(cl.model_name[cl.model_download]))
				return false;
			cl.model_precache[cl.model_download] = Mod_ForName (cl.model_name[cl.model_download], false);
			if (cl.model_precache[cl.model_download] == NULL)
			{
				Host_Error ("Model %s not found", cl.model_name[cl.model_download]);
			}
		}
		cl.model_download++;
	}

	for (; cl.sound_download < cl.sound_count; )
	{
		if (*cl.sound_name[cl.sound_download])
		{
			if (CL_CheckDownload(va("sound/%s", cl.sound_name[cl.sound_download])))
				return false;
			cl.sound_precache[cl.sound_download] = S_PrecacheSound (cl.sound_name[cl.sound_download]);
		}
		cl.sound_download++;
	}

	if (!cl.worldmodel && cl.model_count >= 2)
	{
	// local state
		cl.entities[0].model = cl.worldmodel = cl.model_precache[1];
		if (cl.worldmodel->type != mod_brush)
		{
			if (cl.worldmodel->type == mod_ext_invalid)
				Host_Error ("Worldmodel %s was not loaded", cl.model_name[1]);
			else
				Host_Error ("Worldmodel %s is not a brushmodel", cl.model_name[1]);
		}

		//fixme: deal with skybox somehow

		R_NewMap ();

#ifdef PSET_SCRIPT
		//the protocol changing depending upon files found on the client's computer is of course a really shit way to design things
		//especially when users have a nasty habit of changing config files.
		if (cl.protocol == PROTOCOL_VERSION_DP7)
		{
			PScript_FindParticleType("effectinfo.");	//make sure this is implicitly loaded.
			COM_Effectinfo_Enumerate(CL_GenerateRandomParticlePrecache);
			cl.protocol_particles = true;
		}
		else if (cl.protocol_pext2 || (cl.protocol_pext1&PEXT1_CSQC))
			cl.protocol_particles = true;	//doesn't have a pext flag of its own, but at least we know what it is.
#endif
	}

	//make sure ents have the correct models, now that they're actually loaded.
	for (i = 0; i < cl.num_statics; i++)
	{
		if (cl.static_entities[i]->model)
			continue;
		cl.static_entities[i]->model = cl.model_precache[cl.static_entities[i]->netstate.modelindex];
		R_AddEfrags (cl.static_entities[i]);
	}
	return true;
}


/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int			ret;
	extern int	num_temp_entities; //johnfitz
	int			num_beams = 0; //johnfitz
	int			num_dlights = 0; //johnfitz
	beam_t		*b; //johnfitz
	dlight_t	*l; //johnfitz
	int			i; //johnfitz


	cl.oldtime = cl.time;
	cl.time += host_frametime;

	do
	{
		ret = CL_GetMessage ();
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");
		if (!ret)
			break;

		cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && cls.state == ca_connected);

	if (cl_shownet.value)
		Con_Printf ("\n");

	CL_RelinkEntities ();
	CL_UpdateTEnts ();

//johnfitz -- devstats

	//visedicts
	if (cl_numvisedicts > 256 && dev_peakstats.visedicts <= 256)
		Con_DWarning ("%i visedicts exceeds standard limit of 256.\n", cl_numvisedicts);
	dev_stats.visedicts = cl_numvisedicts;
	dev_peakstats.visedicts = q_max(cl_numvisedicts, dev_peakstats.visedicts);

	//temp entities
	if (num_temp_entities > 64 && dev_peakstats.tempents <= 64)
		Con_DWarning ("%i tempentities exceeds standard limit of 64 (max = %d).\n", num_temp_entities, MAX_TEMP_ENTITIES);
	dev_stats.tempents = num_temp_entities;
	dev_peakstats.tempents = q_max(num_temp_entities, dev_peakstats.tempents);

	//beams
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->model && b->endtime >= cl.time)
			num_beams++;
	if (num_beams > 24 && dev_peakstats.beams <= 24)
		Con_DWarning ("%i beams exceeded standard limit of 24 (max = %d).\n", num_beams, MAX_BEAMS);
	dev_stats.beams = num_beams;
	dev_peakstats.beams = q_max(num_beams, dev_peakstats.beams);

	//dlights
	for (i=0, l=cl_dlights ; i<MAX_DLIGHTS ; i++, l++)
		if (l->die >= cl.time && l->radius)
			num_dlights++;
	if (num_dlights > 32 && dev_peakstats.dlights <= 32)
		Con_DWarning ("%i dlights exceeded standard limit of 32 (max = %d).\n", num_dlights, MAX_DLIGHTS);
	dev_stats.dlights = num_dlights;
	dev_peakstats.dlights = q_max(num_dlights, dev_peakstats.dlights);

//johnfitz

//
// bring the links up to date
//
	return 0;
}

/*
=================
CL_UpdateViewAngles

Spike: split from CL_SendCmd, to do clientside viewangle changes separately from outgoing packets.
=================
*/
void CL_AccumulateCmd (void)
{
	if (cls.signon == SIGNONS)
	{
		//basic keyboard looking
		CL_AdjustAngles ();

		//accumulate movement from other devices
		IN_Move (&cl.pendingcmd);
	}

	cl.pendingcmd.seconds		= cl.mtime[0] - cl.pendingcmd.servertime;
}

void CL_CSQC_SetInputs(usercmd_t *cmd, qboolean set)
{
	if (set)
	{
		if (qcvm->extglobals.input_timelength)
			*qcvm->extglobals.input_timelength = cmd->seconds;
		if (qcvm->extglobals.input_angles)
			VectorCopy(cmd->viewangles, qcvm->extglobals.input_angles);
		if (qcvm->extglobals.input_movevalues)
		{
			qcvm->extglobals.input_movevalues[0] = cmd->forwardmove;
			qcvm->extglobals.input_movevalues[1] = cmd->sidemove;
			qcvm->extglobals.input_movevalues[2] = cmd->upmove;
		}
		if (qcvm->extglobals.input_buttons)
			*qcvm->extglobals.input_buttons = cmd->buttons;
		if (qcvm->extglobals.input_impulse)
			*qcvm->extglobals.input_impulse = cmd->impulse;

		if (qcvm->extglobals.input_weapon)
			*qcvm->extglobals.input_weapon = cmd->weapon;
		if (qcvm->extglobals.input_cursor_screen)
			qcvm->extglobals.input_cursor_screen[0] = cmd->cursor_screen[0], qcvm->extglobals.input_cursor_screen[1] = cmd->cursor_screen[1];
		if (qcvm->extglobals.input_cursor_trace_start)
			VectorCopy(cmd->cursor_start, qcvm->extglobals.input_cursor_trace_start);
		if (qcvm->extglobals.input_cursor_trace_endpos)
			VectorCopy(cmd->cursor_impact, qcvm->extglobals.input_cursor_trace_endpos);
		if (qcvm->extglobals.input_cursor_entitynumber)
			*qcvm->extglobals.input_cursor_entitynumber = cmd->cursor_entitynumber;
	}
	else
	{
		if (qcvm->extglobals.input_timelength)
			cmd->seconds = *qcvm->extglobals.input_timelength;
		if (qcvm->extglobals.input_angles)
			VectorCopy(qcvm->extglobals.input_angles, cmd->viewangles);
		if (qcvm->extglobals.input_movevalues)
		{
			cmd->forwardmove = qcvm->extglobals.input_movevalues[0];
			cmd->sidemove = qcvm->extglobals.input_movevalues[1];
			cmd->upmove = qcvm->extglobals.input_movevalues[2];
		}
		if (qcvm->extglobals.input_buttons)
			cmd->buttons = *qcvm->extglobals.input_buttons;
		if (qcvm->extglobals.input_impulse)
			cmd->impulse = *qcvm->extglobals.input_impulse;

		if (qcvm->extglobals.input_weapon)
			cmd->weapon = *qcvm->extglobals.input_weapon;
		if (qcvm->extglobals.input_cursor_screen)
			cmd->cursor_screen[0] = qcvm->extglobals.input_cursor_screen[0], cmd->cursor_screen[1] = qcvm->extglobals.input_cursor_screen[1];
		if (qcvm->extglobals.input_cursor_trace_start)
			VectorCopy(qcvm->extglobals.input_cursor_trace_start, cmd->cursor_start);
		if (qcvm->extglobals.input_cursor_trace_endpos)
			VectorCopy(qcvm->extglobals.input_cursor_trace_endpos, cmd->cursor_impact);
		if (qcvm->extglobals.input_cursor_entitynumber)
			cmd->cursor_entitynumber = *qcvm->extglobals.input_cursor_entitynumber;
	}
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t		cmd;

	if (cls.state != ca_connected)
		return;

	// get basic movement from keyboard
	CL_BaseMove (&cmd);

	// allow mice or other external controllers to add to the move
	cmd.forwardmove	+= cl.pendingcmd.forwardmove;
	cmd.sidemove	+= cl.pendingcmd.sidemove;
	cmd.upmove		+= cl.pendingcmd.upmove;
	cmd.sequence	= cl.movemessages;
	cmd.servertime	= cl.time;
	cmd.seconds		= cmd.servertime - cl.pendingcmd.servertime;

	CL_FinishMove(&cmd);

	if (cl.qcvm.extfuncs.CSQC_Input_Frame && !cl.qcvm.nogameaccess)
	{
		PR_SwitchQCVM(&cl.qcvm);
		CL_CSQC_SetInputs(&cmd, true);
		PR_ExecuteProgram(cl.qcvm.extfuncs.CSQC_Input_Frame);
		CL_CSQC_SetInputs(&cmd, false);
		PR_SwitchQCVM(NULL);
	}

	if (cls.signon == SIGNONS)
		CL_SendMove (&cmd);	// send the unreliable message
	else
		CL_SendMove (NULL);
	memset(&cl.pendingcmd, 0, sizeof(cl.pendingcmd));
	cl.pendingcmd.servertime = cmd.servertime;

	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}

// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all

	if (!NET_CanSendMessage (cls.netcon))
	{
		Con_DPrintf ("CL_SendCmd: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("CL_SendCmd: lost server connection");

	SZ_Clear (&cls.message);
}

/*
=============
CL_Tracepos_f -- johnfitz

display impact point of trace along VPN
=============
*/
void CL_Tracepos_f (void)
{
	vec3_t	v, w;

	if (cls.state != ca_connected)
		return;

	VectorMA(r_refdef.vieworg, 8192.0, vpn, v);
	TraceLine(r_refdef.vieworg, v, w);

	if (VectorLength(w) == 0)
		Con_Printf ("Tracepos: trace didn't hit anything\n");
	else
		Con_Printf ("Tracepos: (%i %i %i)\n", (int)w[0], (int)w[1], (int)w[2]);
}

/*
=============
CL_Viewpos_f -- johnfitz

display client's position and angles
=============
*/
void CL_Viewpos_f (void)
{
	if (cls.state != ca_connected)
		return;
#if 0
	//camera position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)r_refdef.vieworg[0],
		(int)r_refdef.vieworg[1],
		(int)r_refdef.vieworg[2],
		(int)r_refdef.viewangles[PITCH],
		(int)r_refdef.viewangles[YAW],
		(int)r_refdef.viewangles[ROLL]);
#else
	//player position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)cl.entities[cl.viewentity].origin[0],
		(int)cl.entities[cl.viewentity].origin[1],
		(int)cl.entities[cl.viewentity].origin[2],
		(int)cl.viewangles[PITCH],
		(int)cl.viewangles[YAW],
		(int)cl.viewangles[ROLL]);
#endif
}

static void CL_ServerExtension_FullServerinfo_f(void)
{
	const char *newserverinfo = Cmd_Argv(1);
	Q_strncpy(cl.serverinfo, newserverinfo, sizeof(cl.serverinfo));	//just replace it
}
static void CL_ServerExtension_ServerinfoUpdate_f(void)
{
	const char *newserverkey = Cmd_Argv(1);
	const char *newservervalue = Cmd_Argv(2);
	Info_SetKey(cl.serverinfo, sizeof(cl.serverinfo), newserverkey, newservervalue);
}

static void CL_UserinfoChanged(scoreboard_t *sb)
{
	char tmp[64];
	Info_GetKey(sb->userinfo, "name", sb->name, sizeof(sb->name));

	Info_GetKey(sb->userinfo, "topcolor", tmp, sizeof(tmp));
	sb->colors = (atoi(tmp)&15)<<4;
	Info_GetKey(sb->userinfo, "bottomcolor", tmp, sizeof(tmp));
	sb->colors |= (atoi(tmp)&15);
}
static void CL_ServerExtension_FullUserinfo_f(void)
{
	size_t slot = atoi(Cmd_Argv(1));
	const char *newserverinfo = Cmd_Argv(2);
	if (slot < cl.maxclients)
	{
		scoreboard_t *sb = &cl.scores[slot];
		Q_strncpy(sb->userinfo, newserverinfo, sizeof(sb->userinfo));	//just replace it
		CL_UserinfoChanged(sb);
	}
}
static void CL_ServerExtension_UserinfoUpdate_f(void)
{
	size_t slot = atoi(Cmd_Argv(1));
	const char *newserverkey = Cmd_Argv(2);
	const char *newservervalue = Cmd_Argv(3);
	if (slot < cl.maxclients)
	{
		scoreboard_t *sb = &cl.scores[slot];
		Info_SetKey(sb->userinfo, sizeof(sb->userinfo), newserverkey, newservervalue);
		CL_UserinfoChanged(sb);
	}
}
static void SV_DecodeUserInfo(client_t *client)
{
	char tmp[64];
	int top, bot;

	//figure out the player's colours
	Info_GetKey(client->userinfo, "topcolor", tmp, sizeof(tmp));
	top = atoi(tmp)&15;
	if (top > 13)
		top = 13;
	Info_GetKey(client->userinfo, "bottomcolor", tmp, sizeof(tmp));
	bot = atoi(tmp)&15;
	if (bot > 13)
		bot = 13;
	//update their entity
	client->edict->v.team = bot+1;
	client->colors = (top<<4) | bot;

	//pick out a name and try to clean it up a little.
	Info_GetKey(client->userinfo, "name", tmp, sizeof(tmp));
	if (!*tmp)
		q_strlcpy(tmp, "unnamed", sizeof(tmp));

	if (Q_strcmp(client->name, tmp) != 0)
	{	//name changed.
		if (client->name[0] && strcmp(client->name, "unconnected") )
			Con_Printf ("%s renamed to %s\n", host_client->name, tmp);
		Q_strcpy (host_client->name, tmp);
		client->edict->v.netname = PR_SetEngineString(client->name);
	}
}
void SV_UpdateInfo(int edict, const char *keyname, const char *value)
{
	char oldvalue[1024];

	char *info;
	size_t infosize;
	const char *pre;
	client_t *cl;
	client_t *infoplayer = NULL;

	if (!edict)
	{
		cvar_t *var = Cvar_FindVar(keyname);
		if (var && var->flags & CVAR_SERVERINFO)
		{
			Cvar_Set(var->name, value);
			return;
		}
		info = svs.serverinfo;
		infosize = sizeof(svs.serverinfo);
		pre = "//svi ";
	}
	else if (edict <= svs.maxclients)
	{
		edict-=1;
		infoplayer = &svs.clients[edict];
		info = infoplayer->userinfo;
		infosize = sizeof(infoplayer->userinfo);
		pre = va("//ui %i", edict);
	}
	else
		return;

	Info_GetKey(info, keyname, oldvalue, sizeof(oldvalue));
	if (strcmp(value, oldvalue))
	{	//its changed. actually broadcast it.
		Info_SetKey(info, infosize, keyname, value);
		if (infoplayer)
			SV_DecodeUserInfo(infoplayer);

		if (*keyname == '_' || !sv.active)
			return;	//underscore means private (user) keys. these are not networked to clients.

		Info_GetKey(info, keyname, oldvalue, sizeof(oldvalue));
		value = oldvalue;

		for (cl = svs.clients; cl < svs.clients+svs.maxclients; cl++)
		{
			if (cl->active)
			{
				if (cl->protocol_pext2 & PEXT2_PREDINFO)
				{
					MSG_WriteByte (&cl->message, svc_stufftext);
					MSG_WriteString (&cl->message, va("%s \"%s\" \"%s\"\n", pre, keyname, value));
				}
				else if (infoplayer && !strcmp(keyname, "name"))
				{
					MSG_WriteByte (&cl->message, svc_updatename);
					MSG_WriteByte (&cl->message, edict);
					MSG_WriteString (&cl->message, value);
				}
				else if (infoplayer && (!strcmp(keyname, "topcolor") || !strcmp(keyname, "bottomcolor")))
				{
					MSG_WriteByte (&cl->message, svc_updatecolors);
					MSG_WriteByte (&cl->message, edict);
					MSG_WriteByte (&cl->message, infoplayer->colors);
				}
			}
		}
	}
}
static void CL_ServerExtension_Ignore_f(void)
{
	Con_DPrintf2("Ignoring stufftext: %s\n", Cmd_Argv(0));
}
static void CL_LegacyColor_f(void)
{	//spike -- code to handle the legacy _cl_color cvar (we now use separate qw-style topcolor/bottomcolor userinfo cvars)
	int col = atoi(Cmd_Argv(1));
	Cvar_SetValue("topcolor",		(col>>4)&0xf);
	Cvar_SetValue("bottomcolor",	(col>>0)&0xf);
}

/*
=================
CL_Init
=================
*/
void CL_Init (void)
{
	SZ_Alloc (&cls.message, 1024);

	CL_InitInput ();
	CL_InitTEnts ();

	Cvar_RegisterVariable (&cl_name);
	Cvar_RegisterAlias    (&cl_name, "_cl_name");	//spike -- for compat with configs now that 'name' is a cvar in its own right.
	Cvar_RegisterVariable (&cl_topcolor);
	Cvar_RegisterVariable (&cl_bottomcolor);
	Cmd_AddCommand ("_cl_color", CL_LegacyColor_f);	//for loading vanilla configs (we have separate qw-style topcolor/bottomcolor userinfo cvars instead)
	Cvar_RegisterVariable (&cl_upspeed);
	Cvar_RegisterVariable (&cl_forwardspeed);
	Cvar_RegisterVariable (&cl_backspeed);
	Cvar_RegisterVariable (&cl_sidespeed);
	Cvar_RegisterVariable (&cl_movespeedkey);
	Cvar_RegisterVariable (&cl_yawspeed);
	Cvar_RegisterVariable (&cl_pitchspeed);
	Cvar_RegisterVariable (&cl_anglespeedkey);
	Cvar_RegisterVariable (&cl_shownet);
	Cvar_RegisterVariable (&cl_nolerp);
	Cvar_RegisterVariable (&lookspring);
	Cvar_RegisterVariable (&lookstrafe);
	Cvar_RegisterVariable (&sensitivity);
	
	Cvar_RegisterVariable (&cl_alwaysrun);

	Cvar_RegisterVariable (&m_pitch);
	Cvar_RegisterVariable (&m_yaw);
	Cvar_RegisterVariable (&m_forward);
	Cvar_RegisterVariable (&m_side);

	Cvar_RegisterVariable (&cfg_unbindall);

	Cvar_RegisterVariable (&cl_maxpitch); //johnfitz -- variable pitch clamping
	Cvar_RegisterVariable (&cl_minpitch); //johnfitz -- variable pitch clamping
	Cvar_RegisterVariable (&cl_recordingdemo); //spike -- for mod hacks. combine with cvar_string or something

	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);

	Cmd_AddCommand ("tracepos", CL_Tracepos_f); //johnfitz
	Cmd_AddCommand ("viewpos", CL_Viewpos_f); //johnfitz

	//spike -- serverinfo stuff
	Cmd_AddCommand_ServerCommand ("fullserverinfo", CL_ServerExtension_FullServerinfo_f);
	Cmd_AddCommand_ServerCommand ("svi", CL_ServerExtension_ServerinfoUpdate_f);

	//spike -- userinfo stuff
	Cmd_AddCommand_ServerCommand ("fui", CL_ServerExtension_FullUserinfo_f);
	Cmd_AddCommand_ServerCommand ("ui", CL_ServerExtension_UserinfoUpdate_f);

	//spike -- add stubs to mute various invalid stuffcmds
	Cmd_AddCommand_ServerCommand ("paknames", CL_ServerExtension_Ignore_f); //package names in use by the server (including gamedir+extension)
	Cmd_AddCommand_ServerCommand ("paks", CL_ServerExtension_Ignore_f); //provides hashes to go with the paknames list
	//Cmd_AddCommand_ServerCommand ("vwep", CL_ServerExtension_Ignore_f); //invalid for nq, provides an alternative list of model precaches for vweps.
	//Cmd_AddCommand_ServerCommand ("at", CL_ServerExtension_Ignore_f); //invalid for nq, autotrack info for mvds
	Cmd_AddCommand_ServerCommand ("wps", CL_ServerExtension_Ignore_f); //ktx/cspree weapon stats
	Cmd_AddCommand_ServerCommand ("it", CL_ServerExtension_Ignore_f); //cspree item timers
	Cmd_AddCommand_ServerCommand ("tinfo", CL_ServerExtension_Ignore_f); //ktx team info
	Cmd_AddCommand_ServerCommand ("exectrigger", CL_ServerExtension_Ignore_f); //spike
	Cmd_AddCommand_ServerCommand ("csqc_progname", CL_ServerExtension_Ignore_f); //spike
	Cmd_AddCommand_ServerCommand ("csqc_progsize", CL_ServerExtension_Ignore_f); //spike
	Cmd_AddCommand_ServerCommand ("csqc_progcrc", CL_ServerExtension_Ignore_f); //spike
	Cmd_AddCommand_ServerCommand ("cl_fullpitch", CL_ServerExtension_Ignore_f); //spike
	Cmd_AddCommand_ServerCommand ("pq_fullpitch", CL_ServerExtension_Ignore_f); //spike

	Cmd_AddCommand_ServerCommand ("cl_serverextension_download", CL_ServerExtension_Download_f); //spike
	Cmd_AddCommand_ServerCommand ("cl_downloadbegin", CL_Download_Begin_f); //spike
	Cmd_AddCommand_ServerCommand ("cl_downloadfinished", CL_Download_Finished_f); //spike
	Cmd_AddCommand ("stopdownload", CL_StopDownload_f); //spike
}

