/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, MovieDuels contributors

This file is part of the MovieDuels source code.

MovieDuels is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

//b_spawn.cpp
//added by MCG
#include "b_local.h"
#include "anims.h"
#include "w_saber.h"
#include "bg_saga.h"
#include "bg_vehicles.h"
#include "g_nav.h"

extern void G_DebugPrint(int level, const char* format, ...);

extern qboolean G_CheckInSolid(gentity_t* self, qboolean fix);
extern qboolean client_userinfo_changed(int clientNum);
extern qboolean SpotWouldTelefrag2(const gentity_t* mover, vec3_t dest);
extern void Jedi_Cloak(gentity_t* self);

extern void Q3_SetParm(int entID, int parmNum, const char* parmValue);
extern char* TeamNames[TEAM_NUM_TEAMS];
extern void bubble_shield_update(void);

extern void ST_ClearTimers(const gentity_t* ent);
extern void Jedi_ClearTimers(const gentity_t* ent);
extern void npc_shadow_trooper_precache(void);
extern void NPC_Gonk_Precache(void);
extern void NPC_Mouse_Precache(void);
extern void NPC_Seeker_Precache(void);
extern void NPC_Remote_Precache(void);
extern void NPC_R2D2_Precache(void);
extern void NPC_R5D2_Precache(void);
extern void NPC_Probe_Precache(void);
extern void NPC_Interrogator_Precache(void);
extern void NPC_MineMonster_Precache(void);
extern void NPC_Howler_Precache(void);
extern void NPC_ATST_Precache(void);
extern void NPC_Sentry_Precache(void);
extern void NPC_Mark1_Precache(void);
extern void NPC_Mark2_Precache(void);
extern void NPC_GalakMech_Precache(void);
extern void NPC_GalakMech_Init(gentity_t* ent);
extern void NPC_Protocol_Precache(void);
extern void Boba_Precache(void);
extern void NPC_Wampa_Precache(void);
extern void npc_rosh_dark_precache(void);
gentity_t* NPC_SpawnType(const gentity_t* ent, const char* npc_type, const char* targetname, qboolean isVehicle);
extern void Howler_ClearTimers(const gentity_t* self);

extern void Rancor_SetBolts(const gentity_t* self);
extern void Wampa_SetBolts(gentity_t* self);

#define	NSF_DROP_TO_FLOOR	16

// PAIN functions...
//
extern void funcBBrushPain(gentity_t* self, gentity_t* attacker, int damage);
extern void misc_model_breakable_pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void station_pain(gentity_t* self, gentity_t* attacker, int damage);
extern void func_usable_pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_ATST_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_ST_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Jedi_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Droid_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Probe_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_MineMonster_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Howler_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Seeker_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Remote_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void emplaced_gun_pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Mark1_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_GM_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Sentry_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Mark2_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void PlayerPain(gentity_t* self, int damage);
extern void GasBurst(gentity_t* self, gentity_t* attacker, int damage);
extern void CrystalCratePain(gentity_t* self, gentity_t* attacker, int damage);
extern void TurretPain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Wampa_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_Rancor_Pain(gentity_t* self, gentity_t* attacker, int damage);
extern void NPC_SandCreature_Pain(gentity_t* self, gentity_t* attacker, int damage);

int WP_SetSaberModel(gclient_t* client, class_t npcClass)
{
	//if (client)
	//{
	//	switch (npcClass)
	//	{
	//	case CLASS_DESANN://Desann
	//		client->ps.saber[0].model = "models/weapons2/saber_desann/saber_w.glm";
	//		break;
	//	case CLASS_VADER://Desann
	//		client->ps.saber[0].model = "models/weapons2/saber_VaderEp5/saber_w.glm";
	//		break;
	//	case CLASS_LUKE://Luke
	//		client->ps.saber[0].model = "models/weapons2/saber_luke/saber_w.glm";
	//		break;
	//	case CLASS_YODA://YODA
	//		client->ps.saber[0].model = "models/weapons2/saber_yoda/saber_w.glm";
	//		break;
	//	case CLASS_PLAYER://Kyle NPC and player
	//	case CLASS_KYLE://Kyle NPC and player
	//		client->ps.saber[0].model = DEFAULT_SABER_MODEL;
	//		break;
	//	default://reborn and tavion and everyone else
	//		client->ps.saber[0].model = DEFAULT_SABER_MODEL;
	//		break;
	//	}
	//	return (G_model_index(client->ps.saber[0].model));
	//}
	//else
	//{
	//	switch (npcClass)
	//	{
	//	case CLASS_DESANN://Desann
	//		return (G_model_index("models/weapons2/saber_desann/saber_w.glm"));
	//		break;
	//	case CLASS_VADER://Desann
	//		return (G_model_index("models/weapons2/saber_VaderEp5/saber_w.glm"));
	//		break;
	//	case CLASS_LUKE://Luke
	//		return (G_model_index("models/weapons2/saber_luke/saber_w.glm"));
	//		break;
	//	case CLASS_YODA://Luke
	//		return (G_model_index("models/weapons2/saber_yoda/saber_w.glm"));
	//		break;
	//	case CLASS_PLAYER://Kyle NPC and player
	//	case CLASS_KYLE://Kyle NPC and player
	//		return (G_model_index(DEFAULT_SABER_MODEL));
	//		break;
	//	default://reborn and tavion and everyone else
	//		return (G_model_index(DEFAULT_SABER_MODEL));
	//		break;
	//	}
	//}
	return 1;
}

/*
-------------------------
NPC_PainFunc
-------------------------
*/
typedef void (PAIN_FUNC)(gentity_t* self, gentity_t* attacker, int damage);

static PAIN_FUNC* NPC_PainFunc(const gentity_t* ent)
{
	void (*func)(gentity_t * self, gentity_t * attacker, int damage) = NULL;

	if (ent->client->ps.weapon == WP_SABER)
	{
		func = NPC_Jedi_Pain;
	}
	else
	{
		switch (ent->client->NPC_class)
		{
			// troopers get special pain
		case CLASS_STORMTROOPER:
		case CLASS_STORMCOMMANDO:
		case CLASS_CLONETROOPER:
		case CLASS_SWAMPTROOPER:
			func = NPC_ST_Pain;
			break;

		case CLASS_SEEKER:
			func = NPC_Seeker_Pain;
			break;

		case CLASS_REMOTE:
			func = NPC_Remote_Pain;
			break;

		case CLASS_MINEMONSTER:
			func = NPC_MineMonster_Pain;
			break;

		case CLASS_HOWLER:
			func = NPC_Howler_Pain;
			break;

			// all other droids, did I miss any?
		case CLASS_GONK:
		case CLASS_R2D2:
		case CLASS_R5D2:
		case CLASS_MOUSE:
		case CLASS_PROTOCOL:
		case CLASS_INTERROGATOR:
		case CLASS_SBD:
		case CLASS_DROIDEKA:
			func = NPC_Droid_Pain;
			break;
		case CLASS_PROBE:
			func = NPC_Probe_Pain;
			break;

		case CLASS_SENTRY:
			func = NPC_Sentry_Pain;
			break;
		case CLASS_MARK1:
			func = NPC_Mark1_Pain;
			break;
		case CLASS_MARK2:
			func = NPC_Mark2_Pain;
			break;
		case CLASS_ATST:
			func = NPC_ATST_Pain;
			break;
		case CLASS_GALAKMECH:
			func = NPC_GM_Pain;
			break;
		case CLASS_RANCOR:
			func = NPC_Rancor_Pain;
			break;
		case CLASS_WAMPA:
			func = NPC_Wampa_Pain;
			break;
		case CLASS_SAND_CREATURE:
			func = NPC_SandCreature_Pain;
			break;
			// everyone else gets the normal pain func
		default:
			func = NPC_Pain;
			break;
		}
	}

	return func;
}

/*
-------------------------
NPC_TouchFunc
-------------------------
*/
typedef void (TOUCH_FUNC)(gentity_t* self, gentity_t* other, trace_t* trace);

static TOUCH_FUNC* NPC_TouchFunc(gentity_t* ent)
{
	void(*func)(gentity_t * self, gentity_t * other, trace_t * trace) = NPC_Touch;

	return func;
}

/*
-------------------------
NPC_SetMiscDefaultData
-------------------------
*/
//this function sets the default fleeing behavior for this NPC
static void G_ClassSetDontFlee(const gentity_t* self)
{
	if (!self || !self->client || !self->NPC)
	{
		return;
	}
	switch (self->client->NPC_class)
	{
	case CLASS_ATST:
	case CLASS_CLAW:
	case CLASS_DESANN:
	case CLASS_FISH:
	case CLASS_FLIER2:
	case CLASS_GALAK:
	case CLASS_GLIDER:
	case CLASS_RANCOR:
	case CLASS_SAND_CREATURE:
	case CLASS_INTERROGATOR: // droid
	case CLASS_JAN:
	case CLASS_JEDI:
	case CLASS_KYLE:
	case CLASS_LANDO:
	case CLASS_LIZARD:
	case CLASS_LUKE:
	case CLASS_MARK1: // droid
	case CLASS_MARK2: // droid
	case CLASS_GALAKMECH: // droid
	case CLASS_MONMOTHA:
	case CLASS_MORGANKATARN:
	case CLASS_MURJJ:
	case CLASS_PROBE: // droid
	case CLASS_REBORN:
	case CLASS_REELO:
	case CLASS_REMOTE:
	case CLASS_SEEKER: // droid
	case CLASS_SENTRY:
	case CLASS_SHADOWTROOPER:
	case CLASS_SWAMP:
	case CLASS_TAVION:
	case CLASS_ALORA:
	case CLASS_BOBAFETT:
	case CLASS_SABER_DROID:
	case CLASS_ASSASSIN_DROID:
	case CLASS_DROIDEKA:
	case CLASS_PLAYER:
	case CLASS_VEHICLE:
	case CLASS_MANDO:
	case CLASS_SBD:
	case CLASS_WOOKIE:
		self->NPC->scriptFlags |= SCF_DONT_FLEE;
		break;
	default:
		break;
	}

	if (self->NPC->aiFlags & NPCAI_BOSS_CHARACTER)
	{
		self->NPC->scriptFlags |= SCF_DONT_FLEE;
	}
	if (self->NPC->aiFlags & NPCAI_SUBBOSS_CHARACTER)
	{
		self->NPC->scriptFlags |= SCF_DONT_FLEE;
	}
	if (self->NPC->aiFlags & NPCAI_ROSH)
	{
		self->NPC->scriptFlags |= SCF_DONT_FLEE;
	}
	if (self->NPC->aiFlags & NPCAI_HEAL_ROSH)
	{
		self->NPC->scriptFlags |= SCF_DONT_FLEE;
	}
}

extern void SandCreature_ClearTimers(void);
extern void Boba_SetBolts(const gentity_t* ent);
void DetermineDodgeMax(const gentity_t* ent);
extern void Saboteur_Cloak(gentity_t* self);

static void NPC_SetMiscDefaultData(gentity_t* ent)
{
	if (ent->spawnflags & SFB_CINEMATIC)
	{
		//if a cinematic guy, default us to wait b_state
		ent->NPC->behaviorState = BS_CINEMATIC;
	}
	if (ent->client->NPC_class == CLASS_BOBAFETT)
	{
		//set some stuff, precache
		Boba_Precache();
		ent->client->ps.fd.forcePowersKnown |= 1 << FP_LEVITATION;
		ent->client->ps.fd.forcePowerLevel[FP_LEVITATION] = FORCE_LEVEL_3;
		ent->client->ps.fd.forcePower = 100;
		ent->NPC->scriptFlags |= SCF_ALT_FIRE | SCF_NO_GROUPS;
	}
	if (ent->s.NPC_class == CLASS_VEHICLE && ent->m_pVehicle)
	{
		ent->s.g2radius = 255; //MAX for this value, was (ent->r.maxs[2]-ent->r.mins[2]), which is 272 or something

		if (ent->m_pVehicle->m_pVehicleInfo->type == VH_WALKER)
		{
			ent->mass = 2000; //???
			ent->flags |= FL_SHIELDED | FL_NO_KNOCKBACK;
			ent->pain = NPC_ATST_Pain;
		}
		//turn the damn hatch cover on and LEAVE it on
		trap->G2API_SetSurfaceOnOff(ent->ghoul2, "head_hatchcover", 0/*TURN_ON*/);
	}
	if (!Q_stricmp("wampa", ent->NPC_type))
	{
		//FIXME: extern this into NPC.cfg?
		Wampa_SetBolts(ent);
		ent->s.g2radius = 80; //???
		ent->mass = 300; //???
		ent->flags |= FL_NO_KNOCKBACK;
		ent->pain = NPC_Wampa_Pain;
	}
	if (ent->client->NPC_class == CLASS_RANCOR)
	{
		Rancor_SetBolts(ent);
		ent->s.g2radius = 255; //MAX for this value, was (ent->r.maxs[2]-ent->r.mins[2]), which is 272 or something
		ent->mass = 1000; //???
		ent->flags |= FL_NO_KNOCKBACK;
		ent->pain = NPC_Rancor_Pain;
		ent->health *= 4;
		ent->NPC->aiFlags |= NPCAI_NAV_THROUGH_BREAKABLES; // (SP)
		if (Q_stricmp("mutant_rancor", ent->NPC_type) == 0)
		{
			ent->spawnflags |= 1; //just so I know it's a mutant rancor as opposed to a normal one (SP: breath attack, NPC_AI_Rancor.c)
			ent->mass = 2000;
		}
	}
	if (ent->client->NPC_class == CLASS_SAND_CREATURE)
	{
		//???
		ent->clipmask = CONTENTS_SOLID | CONTENTS_MONSTERCLIP; //it can go through others
		ent->r.contents = 0; //can't be hit?
		ent->takedamage = qfalse; //can't be killed
		ent->flags |= FL_NO_KNOCKBACK;
		SandCreature_ClearTimers();
		Rancor_SetBolts(ent); //racc - need for the MP system for getting picked up by monsters.
	}

	if (Q_stricmpn(ent->NPC_type, "hazardtrooper", 13) == 0)
	{
		//hazard trooper
		ent->NPC->scriptFlags |= SCF_NO_GROUPS; //don't use combat points or group AI
		ent->flags |= FL_SHIELDED | FL_NO_KNOCKBACK; //low-level shots bounce off, no knockback
	}
	if (!Q_stricmp("Yoda", ent->NPC_type) || !Q_stricmp("Ep1_Yoda", ent->NPC_type) ||
		!Q_stricmp("Ep2_Yoda", ent->NPC_type) || !Q_stricmp("Ep3_Yoda", ent->NPC_type))
	{
		//FIXME: extern this into NPC.cfg?
		ent->NPC->scriptFlags |= SCF_NO_FORCE; //force powers don't work on him
		ent->NPC->aiFlags |= NPCAI_BOSS_CHARACTER; //added boss flag
	}
	if (!Q_stricmp("emperor", ent->NPC_type)
		|| !Q_stricmp("cultist_grip", ent->NPC_type)
		|| !Q_stricmp("cultist_drain", ent->NPC_type)
		|| !Q_stricmp("cultist_destroyer", ent->NPC_type)
		|| !Q_stricmp("cultist_lightning", ent->NPC_type))
	{
		//FIXME: extern this into NPC.cfg?
		ent->NPC->scriptFlags |= SCF_DONT_FIRE; //so he uses only force powers
	}
	if (!Q_stricmp("cultist_destroyer", ent->NPC_type))
	{
		ent->splashDamage = 1000;
		ent->splashRadius = 384;
		//ent->fxID = G_EffectIndex("explosions/droidexplosion1");
		ent->NPC->scriptFlags |= SCF_DONT_FLEE | SCF_IGNORE_ALERTS;
		ent->NPC->ignorePain = qtrue;
	}
	if (ent->client->NPC_class == CLASS_BOBAFETT)
	{
		Boba_SetBolts(ent);
	}
	//==================
	if (ent->client->ps.weapon == WP_SABER)
	{
		//if I'm equipped with a saber, initialize it (them)
		wp_saber_init_blade_data(ent);
		ent->client->ps.saberHolstered = 2;
		Jedi_ClearTimers(ent);
		ent->client->ps.fd.blockPoints = BLOCK_POINTS_MAX;
	}
	if (ent->client->ps.fd.forcePowersKnown != 0)
	{
		WP_InitForcePowers(ent);
		WP_SpawnInitForcePowers(ent); //rww
	}
	//allocate the NPC's skills based on their weapon selection
	if (ent->client->ps.weapon == WP_BLASTER)
	{
		ent->client->skillLevel[SK_BLASTER] = FORCE_LEVEL_3;
	}
	if (ent->client->ps.weapon == WP_BOWCASTER)
	{
		ent->client->skillLevel[SK_BOWCASTER] = FORCE_LEVEL_3;
	}
	if (ent->client->ps.weapon == WP_ROCKET_LAUNCHER)
	{
		ent->client->skillLevel[SK_ROCKET] = FORCE_LEVEL_3;
	}
	if (ent->client->ps.weapon == WP_THERMAL)
	{
		ent->client->skillLevel[SK_THERMAL] = FORCE_LEVEL_3;
		ent->client->skillLevel[SK_CRYOBAN] = FORCE_LEVEL_3;
	}
	if (ent->client->ps.weapon == WP_BRYAR_PISTOL || ent->client->ps.weapon == WP_BRYAR_OLD)
	{
		ent->client->skillLevel[SK_PISTOL] = FORCE_LEVEL_3;
	}
	if (ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE] > 0)
	{
		//just bump all the NPC's other saber styles to their saber offense skill level
		for (int count = SK_BLUESTYLE; count <= SK_STAFFSTYLE; count++)
		{
			ent->client->skillLevel[count] = ent->client->ps.fd.forcePowerLevel[FP_SABER_OFFENSE];
		}
	}
	//determine DodgeMax since NPCs kick out of InitForcePowers early.
	DetermineDodgeMax(ent);

	if (ent->client->NPC_class == CLASS_HOWLER)
	{
		Howler_ClearTimers(ent);
		ent->NPC->scriptFlags |= SCF_NO_FALLTODEATH;
		ent->flags |= FL_NO_IMPACT_DMG;
		ent->NPC->scriptFlags |= SCF_NAV_CAN_JUMP; // These jokers can jump
	}
	if (ent->client->NPC_class == CLASS_DESANN
		|| ent->client->NPC_class == CLASS_VADER
		|| ent->client->NPC_class == CLASS_SITHLORD
		|| ent->client->NPC_class == CLASS_ALORA
		|| ent->client->NPC_class == CLASS_TAVION
		|| ent->client->NPC_class == CLASS_LUKE
		|| ent->client->NPC_class == CLASS_KYLE
		|| ent->client->NPC_class == CLASS_REBORN
		|| Q_stricmp("tavion_scepter", ent->NPC_type) == 0
		|| Q_stricmp("alora_dual", ent->NPC_type) == 0)
	{
		//boss character flag set
		ent->NPC->aiFlags |= NPCAI_BOSS_CHARACTER;
	}
	else if (Q_stricmp("alora", ent->NPC_type) == 0
		|| Q_stricmp("rosh_dark", ent->NPC_type) == 0)
	{
		//sub-boss character flag set
		ent->NPC->aiFlags |= NPCAI_SUBBOSS_CHARACTER;
	}
	if (ent->client->NPC_class == CLASS_SABER_DROID)
	{
		ent->flags |= FL_NO_KNOCKBACK;
	}
	else if (ent->client->NPC_class == CLASS_SABOTEUR)
	{
		//can cloak
		ent->NPC->aiFlags |= NPCAI_SHIELDS; //give them the ability to cloak
		if (ent->spawnflags & 16)
		{
			//start cloaked
			Saboteur_Cloak(ent);
		}
	}
	else if (ent->client->NPC_class == CLASS_ASSASSIN_DROID)
	{
		ent->client->ps.stats[STAT_ARMOR] = 250; // start with full armor
		if (ent->s.weapon == WP_BLASTER)
		{
			ent->NPC->scriptFlags |= SCF_ALT_FIRE;
		}
		ent->flags |= FL_NO_KNOCKBACK;
	}
	else if (ent->client->NPC_class == CLASS_SBD)
	{
		ent->client->ps.stats[STAT_ARMOR] = 250; // start with full armor
		ent->flags |= FL_NO_KNOCKBACK;
		ent->NPC->scriptFlags = SCF_CHASE_ENEMIES | SCF_LOOK_FOR_ENEMIES;
		ent->NPC->scriptFlags |= SCF_DONT_FLEE;
	}
	else if (ent->client->NPC_class == CLASS_DROIDEKA)
	{
		ent->client->ps.stats[STAT_ARMOR] = 100; // start with full armor (as SP: the shield recharges to 100, Droideka_ShieldUpdate)
		ent->flags |= FL_NO_KNOCKBACK;
		ent->NPC->scriptFlags = SCF_CHASE_ENEMIES | SCF_LOOK_FOR_ENEMIES;
		ent->NPC->scriptFlags |= SCF_DONT_FLEE;
	}
	//rosh boss special flags
	if (Q_stricmp("DKothos", ent->NPC_type) == 0
		|| Q_stricmp("VKothos", ent->NPC_type) == 0)
	{
		//flags for the force healer dudes.
		ent->NPC->scriptFlags |= SCF_DONT_FIRE;
		ent->NPC->aiFlags |= NPCAI_HEAL_ROSH;
		ent->count = 100;
	}
	else if (Q_stricmp("rosh_dark", ent->NPC_type) == 0)
	{
		//boss flag for Rosh
		ent->NPC->aiFlags |= NPCAI_ROSH;
	}
	if (ent->client->NPC_class == CLASS_SEEKER)
	{
		ent->NPC->defaultBehavior = BS_DEFAULT;
		ent->client->ps.gravity = 0;
		ent->NPC->aiFlags |= NPCAI_CUSTOM_GRAVITY;
		ent->client->ps.eFlags2 |= EF2_FLYING;
		if (ent->client->leader //has leader
			&& ent->client->leader->client //leader is a client
			&& ent->client->leader->client->remote == ent) //has us as their remote.
		{
			//player's seeker item
			ent->count = -1;
		}
		else
		{
			//normal NPC seeker
			ent->count = 30; // SEEKER shot ammo count
		}
	}
	if (ent->client->NPC_class == CLASS_GONK)
	{
		// I guess we generically make them player usable
		ent->r.svFlags |= SVF_PLAYER_USABLE;
	}

	//***I'm not sure whether I should leave this as a TEAM_ switch, I think NPC_class may be more appropriate - dmv
	switch ((team_t)ent->client->playerTeam)
	{
	case (team_t)NPCTEAM_PLAYER:
	{
		if (ent->client->NPC_class == CLASS_SEEKER)
		{
			ent->NPC->defaultBehavior = BS_DEFAULT;
			ent->client->ps.gravity = 0;
			ent->client->ps.eFlags2 |= EF2_FLYING;

			ent->client->moveType = MT_FLYSWIM;
			ent->count = 30; // SEEKER shot ammo count
			return;
		}
		if (ent->client->NPC_class == CLASS_JEDI
			|| ent->client->NPC_class == CLASS_KYLE
			|| ent->client->NPC_class == CLASS_LUKE)
		{
			//good jedi
			ent->client->enemyTeam = NPCTEAM_ENEMY;
			if (ent->spawnflags & JSF_AMBUSH)
			{
				//ambusher
				ent->NPC->scriptFlags |= SCF_IGNORE_ALERTS;
				ent->client->noclip = qtrue; //hang
			}
		}
		else
		{
			switch (ent->client->ps.weapon)
			{
			case WP_BRYAR_PISTOL: //FIXME: new weapon: imp blaster pistol
			case WP_DISRUPTOR:
			case WP_BOWCASTER:
			case WP_REPEATER:
			case WP_DEMP2:
			case WP_FLECHETTE:
			case WP_ROCKET_LAUNCHER:
			default:
				break;
			case WP_THERMAL:
			case WP_BLASTER:
				ST_ClearTimers(ent);
				if (ent->NPC->rank >= RANK_LT || ent->client->ps.weapon == WP_THERMAL)
				{
					//officers, grenade-throwers use alt-fire
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			}
			if (!Q_stricmp("galak_mech", ent->NPC_type))
			{
				//starts with armor
				NPC_GalakMech_Init(ent);
				bubble_shield_update();
			}
		}
	}
	if (ent->client->NPC_class == CLASS_KYLE || ent->client->NPC_class == CLASS_VEHICLE || ent->spawnflags &
		SFB_CINEMATIC)
	{
		ent->NPC->defaultBehavior = BS_CINEMATIC;
	}
	else
	{
		ent->NPC->defaultBehavior = BS_FOLLOW_LEADER;
		ent->client->leader = &g_entities[0]; //player
	}
	break;

	case (team_t)NPCTEAM_NEUTRAL:

		if (Q_stricmp(ent->NPC_type, "gonk") == 0)
		{
			// I guess we generically make them player usable
			ent->r.svFlags |= SVF_PLAYER_USABLE;

			// Not even sure if we want to give different levels of batteries?  ...Or even that these are the values we'd want to use.
			switch (g_npcspskill.integer)
			{
			case 0: //	EASY
				ent->client->ps.batteryCharge = MAX_BATTERIES * 0.8f;
				break;
			case 1: //	MEDIUM
				ent->client->ps.batteryCharge = MAX_BATTERIES * 0.75f;
				break;
			default:
			case 2: //	HARD
				ent->client->ps.batteryCharge = MAX_BATTERIES * 0.5f;
				break;
			}
		}
		break;

	case (team_t)NPCTEAM_ENEMY:
	{
		ent->NPC->defaultBehavior = BS_DEFAULT;

		if (ent->client->NPC_class == CLASS_SHADOWTROOPER && Q_stricmpn("shadowtrooper", ent->NPC_type, 13) == 0)
		{
			if (!PM_SaberInAttack(ent->client->ps.saberMove))
			{
				Jedi_Cloak(ent);
			}
		}
		if (ent->client->NPC_class == CLASS_TAVION ||
			ent->client->NPC_class == CLASS_ALORA ||
			ent->client->NPC_class == CLASS_SBD ||
			ent->client->NPC_class == CLASS_REBORN && ent->client->ps.weapon == WP_SABER ||
			ent->client->NPC_class == CLASS_DESANN ||
			ent->client->NPC_class == CLASS_SITHLORD ||
			ent->client->NPC_class == CLASS_VADER ||
			ent->client->NPC_class == CLASS_SHADOWTROOPER)
		{
			ent->client->enemyTeam = NPCTEAM_PLAYER;

			if (ent->spawnflags & JSF_AMBUSH)
			{
				//ambusher
				ent->NPC->scriptFlags |= SCF_IGNORE_ALERTS;
				ent->client->noclip = qtrue; //hang
			}
		}
		else if (ent->client->NPC_class == CLASS_PROBE ||
			ent->client->NPC_class == CLASS_REMOTE ||
			ent->client->NPC_class == CLASS_GLIDER ||
			ent->client->NPC_class == CLASS_INTERROGATOR ||
			ent->client->NPC_class == CLASS_SENTRY)
		{
			ent->NPC->defaultBehavior = BS_DEFAULT;
			ent->client->ps.gravity = 0;
			ent->NPC->aiFlags |= NPCAI_CUSTOM_GRAVITY;
			ent->client->ps.eFlags2 |= EF2_FLYING;
		}
		else
		{
			switch (ent->client->ps.weapon)
			{
			case WP_BRYAR_PISTOL:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				if (!Q_stricmp("Imperial", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("StormPilot", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			case WP_DISRUPTOR:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				if (!Q_stricmp("saboteursniper", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			case WP_BOWCASTER:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				if (!Q_stricmp("human_merc_bow", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			case WP_REPEATER:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				if (!Q_stricmp("human_merc_rep", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("StormTrooper_red", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			case WP_DEMP2:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				break;
			case WP_FLECHETTE:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				//shotgunner
				if (!Q_stricmp("stofficeralt", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("STOfficer", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("STOfficer2", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("human_merc_flc", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("StormTrooper_blue", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			case WP_ROCKET_LAUNCHER:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				break;
			case WP_CONCUSSION:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				if (!Q_stricmp("human_merc_cnc", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			case WP_THERMAL:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				break;
			case WP_STUN_BATON:
				break;
			default:
			case WP_BLASTER:
				NPCS.NPCInfo->scriptFlags |= SCF_PILOT;
				ST_ClearTimers(ent);
				if (ent->NPC->rank >= RANK_COMMANDER)
				{
					//commanders use alt-fire
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (ent->client->NPC_class == CLASS_IMPERIAL)
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (ent->client->NPC_class == CLASS_RODIAN)
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("human_merc", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				if (!Q_stricmp("rodian2", ent->NPC_type))
				{
					ent->NPC->scriptFlags |= SCF_ALT_FIRE;
				}
				break;
			}
			if (!Q_stricmp("galak_mech", ent->NPC_type))
			{
				//starts with armor
				NPC_GalakMech_Init(ent);
				bubble_shield_update();
			}
		}
	}
	break;

	default:
		ent->NPC->defaultBehavior = BS_DEFAULT;
		break;
	}

	if (ent->client->NPC_class == CLASS_ATST || ent->client->NPC_class == CLASS_MARK1)
		// chris/steve/kevin requested that the mark1 be shielded also
	{
		ent->flags |= FL_SHIELDED | FL_NO_KNOCKBACK;
	}

	// (DH 2026-11-03: SP NPC_spawn.cpp NPC_SetMiscDefaultData - the per-NPC flags MP's combat code reads)
	if (ent->client->NPC_class == CLASS_BOBAFETT)
	{
		if (level.gametype == GT_SINGLE_PLAYER)
		{
			ent->flags |= FL_UNDYING; // Missions: he can't be killed - he flees and comes back, his scripts end the fight (SP)
		}
		if (!Q_stricmp("boba_fett", ent->NPC_type))
		{
			ent->flags |= FL_BOBAFETT; //low-level shots bounce off, no knockback
		}
	}
	else if (ent->client->NPC_class == CLASS_JANGO || ent->client->NPC_class == CLASS_JANGODUAL
		|| ent->client->NPC_class == CLASS_MANDO)
	{
		ent->behaviorSet[BSET_FLEE] = NULL;
		ent->flags |= FL_SABERDAMAGE_RESIST; //Partially resistant to sabers
	}
	if (!Q_stricmp("Ep7_Snoke", ent->NPC_type) || !Q_stricmp("Ep8_Snoke", ent->NPC_type)
		|| !Q_stricmp("Emperor_Palpatine", ent->NPC_type))
	{
		ent->NPC->scriptFlags |= SCF_DONT_FIRE | SCF_NO_FORCE;
		ent->flags |= FL_SHIELDED;
	}
	if (!Q_stricmp("Captain_Phasma", ent->NPC_type))
	{
		ent->flags |= FL_SHIELDED;
	}
	if (ent->client->NPC_class == CLASS_TUSKEN && g_npcspskill.integer > 1)
	{
		ent->NPC->stats.runSpeed = 280;
		ent->NPC->stats.walkSpeed = 65;
	}
	if (ent->client->ps.weapon == WP_CLONECARBINE || ent->client->ps.weapon == WP_CLONERIFLE
		|| ent->client->ps.weapon == WP_CLONECOMMANDO || ent->client->ps.weapon == WP_REBELRIFLE)
	{
		ent->NPC->scriptFlags |= SCF_CHASE_ENEMIES; // (SP)
	}
	// Set CAN FLY Flag for Navigation On The Following Classes
	//----------------------------------------------------------
	if (ent->client->NPC_class == CLASS_PROBE ||
		ent->client->NPC_class == CLASS_REMOTE ||
		ent->client->NPC_class == CLASS_SEEKER ||
		ent->client->NPC_class == CLASS_SENTRY ||
		ent->client->NPC_class == CLASS_GLIDER ||
		ent->client->NPC_class == CLASS_IMPWORKER ||
		ent->client->NPC_class == CLASS_BOBAFETT ||
		ent->client->NPC_class == CLASS_MANDO ||
		ent->client->NPC_class == CLASS_ROCKETTROOPER)
	{
		ent->NPC->scriptFlags |= SCF_NAV_CAN_FLY;
	}

	if (ent->client->NPC_class == CLASS_SEEKER && ent->activator)
	{
		//assume my teams are already set correctly
	}
	else
	{
		//for siege, want "bad" npc's allied with the "bad" team
		if (G_SiegeMissionRules() && ent->s.NPC_class != CLASS_VEHICLE) // SJE (2026-10-16): also Team FFA missions
		{
			if (ent->client->enemyTeam == NPCTEAM_PLAYER)
			{
				ent->client->sess.sessionTeam = SIEGETEAM_TEAM1;
			}
			else if (ent->client->enemyTeam == NPCTEAM_ENEMY)
			{
				ent->client->sess.sessionTeam = SIEGETEAM_TEAM2;
			}
			else
			{
				ent->client->sess.sessionTeam = TEAM_FREE;
			}
		}
	}
	G_ClassSetDontFlee(ent);
}

/*
-------------------------
NPC_WeaponsForTeam
-------------------------
*/

int NPC_WeaponsForTeam(const npcteam_t team, const int spawnflags, const char* NPC_type)
{
	//*** not sure how to handle this, should I pass in class instead of team and go from there? - dmv
	switch (team)
	{
	case (npcteam_t)NPCTEAM_ENEMY:
		if (Q_stricmp("tavion", NPC_type) == 0 ||
			Q_strncmp("reborn", NPC_type, 6) == 0 ||
			Q_stricmp("desann", NPC_type) == 0 ||
			Q_strncmp("shadowtrooper", NPC_type, 13) == 0)
			return 1 << WP_SABER;

		if (Q_strncmp("stofficer", NPC_type, 9) == 0)
		{
			return 1 << WP_FLECHETTE;
		}
		if (Q_strncmp("stofficer2", NPC_type, 9) == 0)
		{
			return 1 << WP_FLECHETTE;
		}
		if (Q_stricmp("stcommander", NPC_type) == 0)
		{
			return 1 << WP_REPEATER;
		}
		if (Q_stricmp("swamptrooper", NPC_type) == 0)
		{
			return 1 << WP_FLECHETTE;
		}
		if (Q_stricmp("swamptrooper2", NPC_type) == 0)
		{
			return 1 << WP_REPEATER;
		}
		if (Q_stricmp("rockettrooper", NPC_type) == 0)
		{
			return 1 << WP_ROCKET_LAUNCHER;
		}
		if (Q_strncmp("shadowtrooper", NPC_type, 13) == 0)
		{
			return 1 << WP_SABER;
		}
		if (Q_stricmp("imperial", NPC_type) == 0)
		{
			return 1 << WP_BRYAR_PISTOL;
		}
		if (Q_strncmp("impworker", NPC_type, 9) == 0)
		{
			return 1 << WP_BRYAR_PISTOL;
		}
		if (Q_stricmp("stormpilot", NPC_type) == 0)
		{
			return 1 << WP_BRYAR_PISTOL;
		}
		if (Q_stricmp("galak", NPC_type) == 0)
		{
			return 1 << WP_BLASTER;
		}
		if (Q_stricmp("galak_mech", NPC_type) == 0)
		{
			return 1 << WP_REPEATER;
		}
		if (Q_strncmp("ugnaught", NPC_type, 8) == 0)
		{
			return WP_NONE;
		}
		if (Q_stricmp("granshooter", NPC_type) == 0)
		{
			return 1 << WP_BLASTER;
		}
		if (Q_stricmp("granboxer", NPC_type) == 0)
		{
			return 1 << WP_MELEE;
		}
		if (Q_strncmp("gran", NPC_type, 4) == 0)
		{
			return 1 << WP_THERMAL | 1 << WP_MELEE;
		}
		if (Q_stricmp("rodian", NPC_type) == 0)
		{
			return 1 << WP_DISRUPTOR;
		}
		if (Q_stricmp("rodian2", NPC_type) == 0)
		{
			return 1 << WP_BLASTER;
		}

		if (Q_stricmp("interrogator", NPC_type) == 0 || Q_stricmp("sentry", NPC_type) == 0 || Q_strncmp(
			"protocol", NPC_type, 8) == 0)
		{
			return WP_NONE;
		}

		if (Q_strncmp("weequay", NPC_type, 7) == 0)
		{
			return 1 << WP_BOWCASTER; //|( 1 << WP_STAFF )(FIXME: new weap?)
		}
		if (Q_stricmp("impofficer", NPC_type) == 0)
		{
			return 1 << WP_BLASTER;
		}
		if (Q_stricmp("impcommander", NPC_type) == 0)
		{
			return 1 << WP_BLASTER;
		}
		if (Q_stricmp("probe", NPC_type) == 0 || Q_stricmp("seeker", NPC_type) == 0)
		{
			//return ( 1 << WP_BOT_LASER);
			return 0;
		}
		if (Q_stricmp("remote", NPC_type) == 0)
		{
			//return ( 1 << WP_BOT_LASER );
			return 0;
		}
		if (Q_stricmp("trandoshan", NPC_type) == 0)
		{
			return 1 << WP_REPEATER;
		}
		if (Q_stricmp("atst", NPC_type) == 0)
		{
			//return (( 1 << WP_ATST_MAIN)|( 1 << WP_ATST_SIDE));
			return 0;
		}
		if (Q_stricmp("mark1", NPC_type) == 0)
		{
			//return ( 1 << WP_BOT_LASER);
			return 0;
		}
		if (Q_stricmp("mark2", NPC_type) == 0)
		{
			//return ( 1 << WP_BOT_LASER);
			return 0;
		}
		if (Q_stricmp("minemonster", NPC_type) == 0)
		{
			return 1 << WP_MELEE;
		}
		if (Q_stricmp("howler", NPC_type) == 0)
		{
			return 1 << WP_MELEE;
		}
		//Stormtroopers, etc.
		return 1 << WP_BLASTER;

	case  (npcteam_t)NPCTEAM_PLAYER:

		if (spawnflags & SFB_RIFLEMAN)
			return 1 << WP_REPEATER;

		if (spawnflags & SFB_PHASER)
			return 1 << WP_BRYAR_PISTOL;

		if (Q_strncmp("jedi", NPC_type, 4) == 0 || Q_stricmp("luke", NPC_type) == 0)
			return 1 << WP_SABER;

		if (Q_strncmp("prisoner", NPC_type, 8) == 0
			|| Q_strncmp("elder", NPC_type, 5) == 0)
		{
			return WP_NONE;
		}
		if (Q_strncmp("bespincop", NPC_type, 9) == 0)
		{
			return 1 << WP_BRYAR_PISTOL;
		}

		if (Q_stricmp("MonMothma", NPC_type) == 0)
		{
			return WP_NONE;
		}

		//rebel
		return 1 << WP_BLASTER;

	case  (npcteam_t)NPCTEAM_NEUTRAL:

		if (Q_stricmp("mark1", NPC_type) == 0)
		{
			return WP_NONE;
		}
		if (Q_stricmp("mark2", NPC_type) == 0)
		{
			return WP_NONE;
		}
		if (Q_strncmp("ugnaught", NPC_type, 8) == 0)
		{
			return WP_NONE;
		}
		if (Q_stricmp("bartender", NPC_type) == 0)
		{
			return WP_NONE;
		}
		if (Q_stricmp("morgankatarn", NPC_type) == 0)
		{
			return WP_NONE;
		}

		break;

	default:
		break;
	}

	return WP_NONE;
}

extern void ChangeWeapon(const gentity_t* ent, int new_weapon);

/*
-------------------------
NPC_SetWeapons
-------------------------
*/

static void NPC_SetWeapons(gentity_t* ent)
{
	int bestWeap = WP_NONE;
	const int weapons = NPC_WeaponsForTeam(ent->client->playerTeam, ent->spawnflags, ent->NPC_type);

	BG_ClearWeapons(ent->client->ps.stats);
	for (int curWeap = WP_SABER; curWeap < WP_NUM_OWNABLE_WEAPONS; curWeap++)
	{
		if (WP_MASK_HAS(weapons, curWeap))
		{
			BG_GiveWeapon(ent->client->ps.stats, curWeap);
			RegisterItem(BG_FindItemForWeapon(curWeap)); //precache the weapon
			//rwwFIXMEFIXME: Precache
			ent->NPC->currentAmmo = ent->client->ps.ammo[weaponData[curWeap].ammoIndex] = 100; //FIXME: max ammo

			if (bestWeap == WP_SABER)
			{
				// still want to register other weapons -- force saber to be best weap
				continue;
			}

			if (curWeap == WP_MELEE)
			{
				if (bestWeap == WP_NONE)
				{
					// We'll only consider giving Melee since we haven't found anything better yet.
					bestWeap = curWeap;
				}
			}
			else if (curWeap > bestWeap || bestWeap == WP_MELEE)
			{
				// This will never override saber as best weap.  Also will override WP_STUN_BATON if something better comes later in the list
				bestWeap = curWeap;
			}
		}
	}

	ent->client->ps.weapon = bestWeap;
	G_AddEvent(ent, EV_WEAPINVCHANGE, ent->client->ps.stats[STAT_WEAPONS]);
}

/*
-------------------------
NPC_SpawnEffect

  NOTE:  Make sure any effects called here have their models, tga's and sounds precached in
			CG_RegisterNPCEffects in cg_player.cpp
-------------------------
*/

static void NPC_SpawnEffect(gentity_t* ent)
{}

//--------------------------------------------------------------
// NPC_SetFX_SpawnStates
//
// Set up any special parms for spawn effects
//--------------------------------------------------------------
static void NPC_SetFX_SpawnStates(const gentity_t* ent)
{
	if (!(ent->NPC->aiFlags & NPCAI_CUSTOM_GRAVITY))
	{
		ent->client->ps.gravity = g_gravity.value;
	}
}

/*
================
NPC_SpotWouldTelefrag

================
*/
// Check whether placing an NPC at its current origin would telefrag another entity.
// Behaviour preserved; only stack usage and safety improved.
static qboolean NPC_SpotWouldTelefrag(const gentity_t* npc)
{
	// SAFETY FIX: npc must exist
	if (npc == NULL)
	{
		return qfalse;
	}

	// static: too big for the stack, and BG_Alloc is never freed: asked for on every call, this ran the pool dry
	static int touch[MAX_GENTITIES];

	vec3_t mins = { 0 }, maxs = { 0 };

	// Compute bounding box for telefrag check
	VectorAdd(npc->r.currentOrigin, npc->r.mins, mins);
	VectorAdd(npc->r.currentOrigin, npc->r.maxs, maxs);

	// Query entities inside bounding box
	const int num = trap->EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

	for (int i = 0; i < num; i++)
	{
		const int entNum = touch[i];

		// Validate index
		if (entNum < 0 || entNum >= MAX_GENTITIES)
		{
			continue;
		}

		const gentity_t* hit = &g_entities[entNum];

		// Telefrag conditions:
		// - entity is in use
		// - entity has a client
		// - entity is not the NPC itself
		// - entity is solid to NPCs
		// - entity is not owned by NPC and NPC is not owned by entity
		if (hit->inuse &&
			hit->client &&
			hit->s.number != npc->s.number &&
			(hit->r.contents & MASK_NPCSOLID) &&
			hit->s.number != npc->r.ownerNum &&
			hit->r.ownerNum != npc->s.number)
		{
			return qtrue;
		}
	}

	return qfalse;
}

extern qboolean G_ValidSaberStyle(const gentity_t* ent, int saber_style);
extern qboolean WP_SaberCanTurnOffSomeBlades(const saberInfo_t* saber);
//--------------------------------------------------------------
void NPC_Begin(gentity_t* ent)
{
	vec3_t spawn_origin = { 0 }, spawn_angles = { 0 };
	usercmd_t ucmd = { 0 };
	gentity_t* spawnPoint = NULL;

	memset(&ucmd, 0, sizeof ucmd);

	if (!(ent->spawnflags & SFB_NOTSOLID))
	{
		//No NPCs should telefrag
		if (NPC_SpotWouldTelefrag(ent))
		{
			if (ent->wait < 0)
			{
				//remove yourself
				G_DebugPrint(WL_DEBUG, "NPC %s could not spawn, firing target3 (%s) and removing self\n",
					ent->targetname, ent->target3);
				//Fire off our target3
				G_UseTargets2(ent, ent, ent->target3);

				//Kill us
				ent->think = G_FreeEntity;
				ent->nextthink = level.time + 100;
			}
			else
			{
				G_DebugPrint(WL_DEBUG, "NPC %s could not spawn, waiting %4.2 secs to try again\n", ent->targetname,
					ent->wait / 1000.0f);
				ent->think = NPC_Begin;
				ent->nextthink = level.time + ent->wait; //try again in half a second
			}
			return;
		}
	}
	//Spawn effect
	NPC_SpawnEffect(ent);

	VectorCopy(ent->client->ps.origin, spawn_origin);
	VectorCopy(ent->s.angles, spawn_angles);
	spawn_angles[YAW] = ent->NPC->desiredYaw;

	gclient_t* client = ent->client;

	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;

	client->airOutTime = level.time + 12000;

	client->ps.clientNum = ent->s.number;
	// clear entity values

	if (ent->health) // Was health supplied in map
	{
		client->pers.maxHealth = client->ps.stats[STAT_MAX_HEALTH] = ent->health;
	}
	else if (ent->NPC->stats.health) // Was health supplied in NPC.cfg?
	{
		if (ent->client->NPC_class != CLASS_REBORN
			&& ent->client->NPC_class != CLASS_SHADOWTROOPER
			&& ent->client->NPC_class != CLASS_JEDI)
		{
			// up everyone except jedi
			if (!Q_stricmp("tavion_sith_sword", ent->NPC_type)
				|| !Q_stricmp("tavion_scepter", ent->NPC_type)
				|| !Q_stricmp("kyle_boss", ent->NPC_type)
				|| !Q_stricmp("alora_dual", ent->NPC_type)
				|| !Q_stricmp("alora_boss", ent->NPC_type))
			{
				//bosses are a bit different
				ent->NPC->stats.health = ceil(
					(float)ent->NPC->stats.health * 0.75f + (float)ent->NPC->stats.health / 4.0f * g_npcspskill.
					integer); // 75% on easy, 100% on medium, 125% on hard
			}
			else
			{
				ent->NPC->stats.health += ent->NPC->stats.health / 4 * g_npcspskill.integer;
				// 100% on easy, 125% on medium, 150% on hard
			}
		}

		client->pers.maxHealth = client->ps.stats[STAT_MAX_HEALTH] = ent->NPC->stats.health;
	}
	else
	{
		client->pers.maxHealth = client->ps.stats[STAT_MAX_HEALTH] = 100;
	}

	if (!Q_stricmp("rodian", ent->NPC_type))
	{
		//sniper
		//NOTE: this will get overridden by any aim settings in their spawnscripts
		switch (g_npcspskill.integer)
		{
		case 0:
			ent->NPC->stats.aim = 1;
			break;
		case 1:
			ent->NPC->stats.aim = Q_irand(2, 3);
			break;
		case 2:
			ent->NPC->stats.aim = Q_irand(3, 4);
			break;
		default:;
		}
	}
	else if (ent->client->NPC_class == CLASS_STORMTROOPER
		|| ent->client->NPC_class == CLASS_CLONETROOPER
		|| ent->client->NPC_class == CLASS_STORMCOMMANDO
		|| ent->client->NPC_class == CLASS_SBD
		|| ent->client->NPC_class == CLASS_DROIDEKA
		|| ent->client->NPC_class == CLASS_SWAMPTROOPER
		|| ent->client->NPC_class == CLASS_IMPWORKER
		|| !Q_stricmp("rodian2", ent->NPC_type))
	{
		//tweak yawspeed for these NPCs based on difficulty
		switch (g_npcspskill.integer)
		{
		case 0:
			ent->NPC->stats.yawSpeed *= 0.75f;
			if (ent->client->NPC_class == CLASS_IMPWORKER)
			{
				ent->NPC->stats.aim -= Q_irand(3, 6);
			}
			break;
		case 1:
			if (ent->client->NPC_class == CLASS_IMPWORKER)
			{
				ent->NPC->stats.aim -= Q_irand(2, 4);
			}
			break;
		case 2:
			ent->NPC->stats.yawSpeed *= 1.5f;
			if (ent->client->NPC_class == CLASS_IMPWORKER)
			{
				ent->NPC->stats.aim -= Q_irand(0, 2);
			}
			break;
		default:;
		}
	}
	else if (ent->client->NPC_class == CLASS_REBORN
		|| ent->client->NPC_class == CLASS_SHADOWTROOPER)
	{
		switch (g_npcspskill.integer)
		{
		case 1:
			ent->NPC->stats.yawSpeed *= 1.25f;
			break;
		case 2:
			ent->NPC->stats.yawSpeed *= 1.5f;
			break;
		default:;
		}
	}

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->mass = 10;
	ent->takedamage = qtrue;

	ent->inuse = qtrue;

	if (!ent->classname || Q_stricmp(ent->classname, "noclass") == 0)
		ent->classname = "NPC";

	if (!(ent->spawnflags & SFB_NOTSOLID))
	{
		ent->r.contents = CONTENTS_BODY;
		ent->clipmask = MASK_NPCSOLID;
	}
	else
	{
		ent->r.contents = 0;
		ent->clipmask = MASK_NPCSOLID & ~CONTENTS_BODY;
	}

	if (!ent->client->moveType) //Static?
	{
		ent->client->moveType = MT_RUNJUMP;
	}
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->client->ps.rocketLockIndex = ENTITYNUM_NONE;
	ent->client->ps.rocketLockTime = 0;

	//visible to player and NPCs
	if (ent->client->NPC_class != CLASS_R2D2 &&
		ent->client->NPC_class != CLASS_R5D2 &&
		ent->client->NPC_class != CLASS_MOUSE &&
		ent->client->NPC_class != CLASS_GONK &&
		ent->client->NPC_class != CLASS_PROTOCOL)
	{
		ent->flags &= ~FL_NOTARGET;
	}
	ent->s.eFlags &= ~EF_NODRAW;

	NPC_SetFX_SpawnStates(ent);

	if (ent->client->ps.weapon == WP_NONE)
	{
		//not set by the NPCs.cfg
		NPC_SetWeapons(ent);
	}
	//select the weapon
	ent->NPC->currentAmmo = ent->client->ps.ammo[weaponData[ent->client->ps.weapon].ammoIndex];
	ent->client->ps.weaponstate = WEAPON_IDLE;
	ChangeWeapon(ent, ent->client->ps.weapon);

	VectorCopy(spawn_origin, client->ps.origin);

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	// clear entity state values
	ent->s.eType = ET_NPC;

	VectorCopy(spawn_origin, ent->s.origin);

	SetClientViewAngle(ent, spawn_angles);
	client->renderInfo.lookTarget = ENTITYNUM_NONE;
	//clear IK grabbing stuff
	client->ps.heldClient = client->ps.heldByClient = ENTITYNUM_NONE;

	if (!(ent->spawnflags & 64))
	{
		if (Q_stricmp(ent->NPC_type, "nullDriver") == 0)
		{
			//FIXME: should be a better way to check this
		}
		else
		{
			g_kill_box(ent);
		}
		trap->LinkEntity((sharedEntity_t*)ent);
	}

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.value * 1000;
	client->latched_buttons = 0;

	if (ent->s.m_iVehicleNum)
	{
		//I'm an NPC in a vehicle (or a vehicle), I already have owner set
	}
	else if (client->NPC_class == CLASS_SEEKER && ent->activator != NULL)
	{
		//somebody else "owns" me
		ent->s.owner = ent->r.ownerNum = ent->activator->s.number;
	}
	else
	{
		ent->s.owner = ENTITYNUM_NONE;
	}

	// set default animations
	if (ent->client->NPC_class != CLASS_VEHICLE)
	{
		NPC_SetAnim(ent, SETANIM_BOTH, BOTH_STAND1, SETANIM_FLAG_NORMAL);
	}

	if (spawnPoint)
	{
		// fire the targets of the spawn point
		G_UseTargets(spawnPoint, ent);
	}

	//ICARUS include
	trap->ICARUS_InitEnt((sharedEntity_t*)ent);

	//==NPC initialization
	SetNPCGlobals(ent);

	ent->enemy = NULL;
	NPCS.NPCInfo->timeOfDeath = 0;
	NPCS.NPCInfo->shotTime = 0;
	NPC_ClearGoal();
	NPC_ChangeWeapon(ent->client->ps.weapon);

	//==Final NPC initialization
	ent->pain = NPC_PainFunc(ent); //painF_NPC_Pain;
	ent->touch = NPC_TouchFunc(ent); //touchF_NPC_Touch;

	ent->client->ps.ping = ent->NPC->stats.reactions * 50;

	//MCG - Begin: NPC hacks
	//FIXME: Set the team correctly
	if (ent->s.NPC_class != CLASS_VEHICLE || level.gametype != GT_SIEGE)
	{
		ent->client->ps.persistant[PERS_TEAM] = ent->client->playerTeam;
	}

	ent->use = NPC_Use;
	ent->think = NPC_Think;
	ent->nextthink = level.time + FRAMETIME + Q_irand(0, 100);

	NPC_SetMiscDefaultData(ent);
	if (ent->health <= 0)
	{
		//ORIGINAL ID: health will count down towards max_health
		ent->health = client->ps.stats[STAT_HEALTH] = ent->client->pers.maxHealth;
	}
	else
	{
		client->ps.stats[STAT_HEALTH] = ent->health;
	}

	if (ent->s.shouldtarget)
	{
		ent->maxHealth = ent->health;
		G_ScaleNetHealth(ent);
	}

	ChangeWeapon(ent, ent->client->ps.weapon); //yes, again... sigh

	//set saber_anim_levelBase
	if (ent->client->saber[0].model[0] && ent->client->saber[1].model[0])
	{
		ent->client->ps.fd.saber_anim_levelBase = ent->client->ps.fd.saberAnimLevel = SS_DUAL;
	}
	else if (ent->client->saber[0].numBlades > 1
		&& WP_SaberCanTurnOffSomeBlades(&ent->client->saber[0]))
	{
		ent->client->ps.fd.saber_anim_levelBase = ent->client->ps.fd.saberAnimLevel = SS_STAFF;
	}
	else if (ent->client->saberStylesKnown & 1 << SS_BLASTER)
	{
		// SS_BLASTER: the NPC file gave it its styles: start in the one it set last (NPC_ParseParms), else the first known
		int newLevel = ent->client->ps.fd.saberAnimLevel;

		if (!G_ValidSaberStyle(ent, newLevel))
		{
			for (newLevel = SS_FAST; newLevel < SS_NUM_SABER_STYLES; newLevel++)
			{
				if (G_ValidSaberStyle(ent, newLevel))
				{
					break;
				}
			}
			if (newLevel >= SS_NUM_SABER_STYLES)
			{
				newLevel = SS_MEDIUM;
			}
		}
		ent->client->ps.fd.saber_anim_levelBase = ent->client->ps.fd.saberAnimLevel = newLevel;
	}
	else
	{
		int newLevel = Q_irand(SS_FAST, SS_STAFF);
		ent->client->ps.fd.saber_anim_levelBase = SS_MEDIUM;

		//new validation technique.
		if (!G_ValidSaberStyle(ent, newLevel))
		{
			//had an illegal style, revert to a valid one
			for (int count = SS_FAST; count < SS_STAFF; count++)
			{
				newLevel++;
				if (newLevel > SS_STAFF)
				{
					if (ent->client->saber[0].type == SABER_SINGLE_BACKHAND
						|| ent->client->saber[0].type == SABER_SINGLE_ASBACKHAND)
					{
						newLevel = SS_STAFF;
					}
					else if (ent->client->saber[0].type == SABER_STAFF
						|| ent->client->saber[0].type == SABER_STAFF_UNSTABLE
						|| ent->client->saber[0].type == SABER_STAFF_THIN)
					{
						newLevel = SS_TAVION;
					}
					else
					{
						newLevel = SS_FAST;
					}
				}

				if (G_ValidSaberStyle(ent, newLevel))
				{
					break;
				}
			}
		}

		ent->client->ps.fd.saberAnimLevel = newLevel;
	}

	if (!(ent->spawnflags & SFB_STARTINSOLID))
	{
		//Not okay to start in solid
		G_CheckInSolid(ent, qtrue);
	}
	VectorClear(ent->NPC->lastClearOrigin);

	//Run a script if you have one assigned to you
	if (G_ActivateBehavior(ent, BSET_SPAWN))
	{
		trap->ICARUS_MaintainTaskManager(ent->s.number);
	}

	VectorCopy(ent->r.currentOrigin, ent->client->renderInfo.eyePoint);

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	memset(&ucmd, 0, sizeof ucmd);
	//_VectorCopy( client->pers.cmd_angles, ucmd.angles );
	VectorCopyM(client->pers.cmd.angles, ucmd.angles);

	ent->client->ps.groundEntityNum = ENTITYNUM_NONE;

	if (ent->NPC->aiFlags & NPCAI_MATCHPLAYERWEAPON)
	{
		//G_MatchPlayerWeapon( ent );
		//rwwFIXMEFIXME: Use this? Probably doesn't really matter for MP.
	}

	ClientThink(ent->s.number, &ucmd);

	trap->LinkEntity((sharedEntity_t*)ent);

	if (ent->client->playerTeam == NPCTEAM_ENEMY)
	{
		//valid enemy spawned
		if (!(ent->spawnflags & SFB_CINEMATIC) && ent->NPC->behaviorState != BS_CINEMATIC)
		{
			//not a cinematic enemy
			if (g_entities[0].client)
			{
				//g_entities[0].client->sess.missionStats.enemiesSpawned++;
				//rwwFIXMEFIXME: Do something here? And this is a rather strange place to be storing
				//this sort of data.
			}
		}
	}
	ent->waypoint = ent->NPC->homeWp = WAYPOINT_NONE;

	if (ent->m_pVehicle)
	{
		//a vehicle
		//check for droidunit
		if (ent->m_pVehicle->m_iDroidUnitTag != -1)
		{
			char* droidNPCType = NULL;
			if (ent->model2
				&& ent->model2[0])
			{
				//specified on the NPC_Vehicle spawner ent
				droidNPCType = ent->model2;
			}
			else if (ent->m_pVehicle->m_pVehicleInfo->droidNPC
				&& ent->m_pVehicle->m_pVehicleInfo->droidNPC[0])
			{
				//specified in the vehicle's .veh file
				droidNPCType = ent->m_pVehicle->m_pVehicleInfo->droidNPC;
			}

			if (droidNPCType != NULL)
			{
				if (Q_stricmp("random", droidNPCType) == 0
					|| Q_stricmp("default", droidNPCType) == 0)
				{
					//use default - R2D2 or R5D2
					if (Q_irand(0, 1))
					{
						droidNPCType = "r2d2";
					}
					else
					{
						droidNPCType = "r5d2";
					}
				}
				gentity_t* droidEnt = NPC_SpawnType(ent, droidNPCType, NULL, qfalse);
				if (droidEnt != NULL)
				{
					if (droidEnt->client)
					{
						droidEnt->client->ps.m_iVehicleNum =
							droidEnt->s.m_iVehicleNum =
							//droidEnt->s.otherentityNum2 =
							droidEnt->s.owner =
							droidEnt->r.ownerNum = ent->s.number;
						ent->m_pVehicle->m_pDroidUnit = (bgEntity_t*)droidEnt;
						//set team
						droidEnt->alliedTeam = ent->alliedTeam;
						droidEnt->teamnodmg = ent->teamnodmg;
						droidEnt->client->sess.sessionTeam = ent->client->sess.sessionTeam;
						droidEnt->client->ps.persistant[PERS_TEAM] = ent->client->ps.persistant[PERS_TEAM];
						//position
						VectorCopy(ent->r.currentOrigin, droidEnt->s.origin);
						VectorCopy(ent->r.currentOrigin, droidEnt->client->ps.origin);
						G_SetOrigin(droidEnt, droidEnt->s.origin);
						trap->LinkEntity((sharedEntity_t*)droidEnt);
						VectorCopy(ent->r.currentAngles, droidEnt->s.angles);
						G_SetAngles(droidEnt, droidEnt->s.angles);
						if (droidEnt->NPC)
						{
							droidEnt->NPC->desiredYaw = droidEnt->s.angles[YAW];
							droidEnt->NPC->desiredPitch = droidEnt->s.angles[PITCH];
						}
						droidEnt->flags |= FL_UNDYING;
					}
					else
					{
						//wtf?
						G_FreeEntity(droidEnt);
					}
				}
			}
		}
	}
}

gNPC_t* gNPCPtrs[MAX_GENTITIES];

static gNPC_t* New_NPC_t(const int entNum)
{
	if (!gNPCPtrs[entNum])
	{
		gNPCPtrs[entNum] = (gNPC_t*)BG_Alloc(sizeof(gNPC_t));
	}

	gNPC_t* ptr = gNPCPtrs[entNum];

	if (ptr)
	{
		// clear it...
		//
		memset(ptr, 0, sizeof * ptr);
	}

	return ptr;
}

static void NPC_DefaultScriptFlags(const gentity_t* ent)
{
	if (!ent || !ent->NPC)
	{
		return;
	}
	//Set up default script flags
	ent->NPC->scriptFlags = SCF_CHASE_ENEMIES | SCF_LOOK_FOR_ENEMIES;
}

/*
-------------------------
NPC_Spawn_Go
-------------------------
*/
extern void G_CreateAnimalNPC(Vehicle_t** p_veh, const char* strAnimalType);
extern void G_CreateSpeederNPC(Vehicle_t** p_veh, const char* strType);
extern void G_CreateWalkerNPC(Vehicle_t** p_veh, const char* strAnimalType);
extern void G_CreateFighterNPC(Vehicle_t** p_veh, const char* strType);

#define TURN_ON				0x00000000
#define TURN_OFF			0x00000100
#define MAX_SAFESPAWN_ENTS 4

static qboolean NPC_SafeSpawn(const gentity_t* ent, const float safeRadius)
{
	int radius_ents[MAX_SAFESPAWN_ENTS] = { 0 };
	vec3_t safeMins = { 0 }, safeMaxs = { 0 };
	int i = 0;
	const float safeRadiusSquared = safeRadius * safeRadius;

	if (!ent)
	{
		return qfalse;
	}

	//Setup the bbox to search in
	for (i = 0; i < 3; i++)
	{
		safeMins[i] = ent->r.currentOrigin[i] - safeRadius;
		safeMaxs[i] = ent->r.currentOrigin[i] + safeRadius;
	}

	//Get a number of entities in a given space
	const int num_ents = trap->EntitiesInBox(safeMins, safeMaxs, radius_ents, MAX_SAFESPAWN_ENTS);

	for (i = 0; i < num_ents; i++)
	{
		const gentity_t* FoundEnt = &g_entities[radius_ents[i]];

		//Don't consider self
		if (FoundEnt == ent)
			continue;

		if (FoundEnt->NPC && FoundEnt->health <= 0)
		{
			// ignore dead guys
			continue;
		}

		const float distance = DistanceSquared(ent->r.currentOrigin, FoundEnt->r.currentOrigin);

		//Found one too close to us
		if (distance < safeRadiusSquared)
		{
			return qfalse;
		}
	}
	return qtrue;
}

gentity_t* NPC_Spawn_Do(gentity_t* ent)
{
	vec3_t saveOrg = { 0 };

	if (ent->spawnflags & 4096)
	{
		if (!NPC_SafeSpawn(ent, 64))
		{
			return NULL;
		}
	}

	//Test for drop to floor
	if (ent->spawnflags & NSF_DROP_TO_FLOOR)
	{
		trace_t tr = { 0 };
		vec3_t bottom = { 0 };

		VectorCopy(ent->r.currentOrigin, saveOrg);
		VectorCopy(ent->r.currentOrigin, bottom);
		bottom[2] = MIN_WORLD_COORD;
		trap->Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, bottom, ent->s.number, MASK_NPCSOLID, qfalse,
			0, 0);
		if (!tr.allsolid && !tr.startsolid && tr.fraction < 1.0)
		{
			G_SetOrigin(ent, tr.endpos);
		}
	}

	//Check the spawner's count
	if (ent->count != -1)
	{
		ent->count--;

		if (ent->count <= 0)
		{
			ent->use = 0; //never again
			//FIXME: why not remove me...?  Because of all the string pointers?  Just do G_NewStrings?
		}
	}

	gentity_t* newent = G_Spawn();

	if (newent == NULL)
	{
		Com_Printf(S_COLOR_RED "ERROR: NPC G_Spawn failed\n");
		return NULL;
	}

	newent->fullName = ent->fullName;

	newent->NPC = New_NPC_t(newent->s.number);
	if (newent->NPC == NULL)
	{
		Com_Printf(S_COLOR_RED "ERROR: NPC G_Alloc NPC failed\n");
		goto finish;
		//	return NULL;
	}

	G_CreateFakeClient(newent->s.number, &newent->client);

	newent->NPC->tempGoal = G_Spawn();

	if (newent->NPC->tempGoal == NULL)
	{
		newent->NPC = NULL;
		goto finish;
		//	return NULL;
	}

	newent->NPC->tempGoal->classname = "NPC_goal";
	newent->NPC->tempGoal->parent = newent;
	newent->NPC->tempGoal->r.svFlags |= SVF_NOCLIENT;

	if (newent->client == NULL)
	{
		Com_Printf(S_COLOR_RED "ERROR: NPC BG_Alloc client failed\n");
		goto finish;
		//	return NULL;
	}

	memset(newent->client, 0, sizeof * newent->client);

	//Assign the pointer for bg entity access
	newent->playerState = &newent->client->ps;

	//==NPC_Connect( newent, net_name );===================================

	if (ent->NPC_type == NULL)
	{
		ent->NPC_type = "random";
	}
	else
	{
		ent->NPC_type = Q_strlwr(G_NewString(ent->NPC_type));
	}

	if (ent->r.svFlags & SVF_NO_BASIC_SOUNDS)
	{
		newent->r.svFlags |= SVF_NO_BASIC_SOUNDS;
	}
	if (ent->r.svFlags & SVF_NO_COMBAT_SOUNDS)
	{
		newent->r.svFlags |= SVF_NO_COMBAT_SOUNDS;
	}
	if (ent->r.svFlags & SVF_NO_EXTRA_SOUNDS)
	{
		newent->r.svFlags |= SVF_NO_EXTRA_SOUNDS;
	}

	if (ent->message)
	{
		//has a key
		newent->message = ent->message; //transfer the key name
		newent->flags |= FL_NO_KNOCKBACK; //don't fall off ledges
		newent->client->ps.generic1 = 100;
	}

	// If this is a vehicle we need to see what kind it is so we properlly allocate it.
	if (Q_stricmp(ent->classname, "NPC_Vehicle") == 0)
	{
		// Get the vehicle entry index.
		const int iVehIndex = BG_VehicleGetIndex(ent->NPC_type);

		if (iVehIndex == VEHICLE_NONE)
		{
			G_FreeEntity(newent);
			//get rid of the spawner, too, I guess
			G_FreeEntity(ent);
			return NULL;
		}
		// NOTE: If you change/add any of these, update NPC_Spawn_f for the new vehicle you
		// want to be able to spawn in manually.

		// See what kind of vehicle this is and allocate it properly.
		switch (g_vehicleInfo[iVehIndex].type)
		{
		case VH_ANIMAL:
			// Create the animal (making sure all it's data is initialized).
			G_CreateAnimalNPC(&newent->m_pVehicle, ent->NPC_type);
			break;
		case VH_SPEEDER:
			// Create the speeder (making sure all it's data is initialized).
			G_CreateSpeederNPC(&newent->m_pVehicle, ent->NPC_type);
			break;
		case VH_FIGHTER:
			// Create the fighter (making sure all it's data is initialized).
			G_CreateFighterNPC(&newent->m_pVehicle, ent->NPC_type);
			break;
		case VH_WALKER:
			// Create the walker (making sure all it's data is initialized).
			G_CreateWalkerNPC(&newent->m_pVehicle, ent->NPC_type);
			break;

		default:
			Com_Printf(S_COLOR_RED "ERROR: Couldn't spawn NPC %s\n", ent->NPC_type);
			G_FreeEntity(newent);
			//get rid of the spawner, too, I guess
			G_FreeEntity(ent);
			return NULL;
		}

		assert(newent->m_pVehicle &&
			newent->m_pVehicle->m_pVehicleInfo &&
			newent->m_pVehicle->m_pVehicleInfo->Initialize);

		//set up my happy prediction hack
		newent->m_pVehicle->m_vOrientation = &newent->client->ps.vehOrientation[0];

		// Setup the vehicle.
		newent->m_pVehicle->m_pParentEntity = (bgEntity_t*)newent;
		newent->m_pVehicle->m_pVehicleInfo->Initialize(newent->m_pVehicle);

		//cache all the assets
		newent->m_pVehicle->m_pVehicleInfo->RegisterAssets(newent->m_pVehicle);
		//set the class
		newent->client->NPC_class = CLASS_VEHICLE;
		if (g_vehicleInfo[iVehIndex].type == VH_FIGHTER)
		{
			//FIXME: EXTERN!!!
			newent->flags |= FL_NO_KNOCKBACK | FL_SHIELDED | FL_DMG_BY_HEAVY_WEAP_ONLY;
			//don't get pushed around, blasters bounce off, only damage from heavy weaps
		}
		//WTF?!!! Ships spawning in pointing straight down!
		//set them up to start landed
		newent->m_pVehicle->m_vOrientation[YAW] = ent->s.angles[YAW];
		newent->m_pVehicle->m_vOrientation[PITCH] = newent->m_pVehicle->m_vOrientation[ROLL] = 0.0f;
		G_SetAngles(newent, newent->m_pVehicle->m_vOrientation);
		SetClientViewAngle(newent, newent->m_pVehicle->m_vOrientation);

		//newent->m_pVehicle->m_ulFlags |= VEH_GEARSOPEN;
		//why? this would just make it so the initial anim never got played... -rww
		//There was no initial anim, it would just open the gear even though it's already on the ground (fixed now, made an initial anim)

		//For SUSPEND spawnflag, the amount of time to drop like a rock after SUSPEND turns off
		newent->fly_sound_debounce_time = ent->fly_sound_debounce_time;

		//for no-pilot-death delay
		newent->damage = ent->damage;

		//no-pilot-death distance
		newent->speed = ent->speed;

		//for veh transfer all healy stuff
		newent->healingclass = ent->healingclass;
		newent->healingsound = ent->healingsound;
		newent->healingrate = ent->healingrate;
		newent->model2 = ent->model2; //for droidNPC
	}
	else
	{
		newent->client->ps.weapon = WP_NONE; //init for later check in NPC_Begin
	}

	VectorCopy(ent->s.origin, newent->s.origin);
	VectorCopy(ent->s.origin, newent->client->ps.origin);
	VectorCopy(ent->s.origin, newent->r.currentOrigin);
	G_SetOrigin(newent, ent->s.origin); //just to be sure!
	//NOTE: on vehicles, anything in the .npc file will STOMP data on the NPC that's set by the vehicle
	if (!NPC_ParseParms(ent->NPC_type, newent))
	{
		Com_Printf(S_COLOR_RED "ERROR: Couldn't spawn NPC %s\n", ent->NPC_type);
		G_FreeEntity(newent);
		//get rid of the spawner, too, I guess
		G_FreeEntity(ent);
		return NULL;
	}

	if (ent->NPC_type)
	{
		if (!Q_stricmp(ent->NPC_type, "player"))
		{
			//FIXME: "player", not Kyle?  Or check NPC_type against player's NPC_type?
			newent->NPC->aiFlags |= NPCAI_MATCHPLAYERWEAPON;
		}
		else if (!Q_stricmp(ent->NPC_type, "test"))
		{
			for (int n = 0; n < 1; n++)
			{
				if (g_entities[n].s.eType != ET_NPC && g_entities[n].client)
				{
					VectorCopy(g_entities[n].s.origin, newent->s.origin);
					newent->client->playerTeam = newent->s.teamowner = g_entities[n].client->playerTeam;
					break;
				}
			}
			newent->NPC->defaultBehavior = newent->NPC->behaviorState = BS_WAIT;
			newent->classname = "NPC";
		}
	}
	//=====================================================================
	//set the info we want
	if (!newent->health)
	{
		newent->health = ent->health;
	}
	newent->script_targetname = ent->NPC_targetname;
	newent->targetname = ent->NPC_targetname;
	newent->target = ent->NPC_target; //death
	newent->target2 = ent->target2; //knocked out death
	newent->target3 = ent->target3; //???
	newent->target4 = ent->target4; //ffire death
	newent->wait = ent->wait;

	for (int index = BSET_FIRST; index < NUM_BSETS; index++)
	{
		if (ent->behaviorSet[index])
		{
			newent->behaviorSet[index] = ent->behaviorSet[index];
		}
	}

	newent->classname = "NPC";
	newent->NPC_type = ent->NPC_type;
	trap->UnlinkEntity((sharedEntity_t*)newent);

	VectorCopy(ent->s.angles, newent->s.angles);
	VectorCopy(ent->s.angles, newent->r.currentAngles);
	VectorCopy(ent->s.angles, newent->client->ps.viewangles);
	newent->NPC->desiredYaw = ent->s.angles[YAW];

	trap->LinkEntity((sharedEntity_t*)newent);
	newent->spawnflags = ent->spawnflags;

	if (ent->paintarget)
	{
		//safe to point at owner's string since memory is never freed during game
		newent->paintarget = ent->paintarget;
	}
	if (ent->opentarget)
	{
		newent->opentarget = ent->opentarget;
	}

	//==New stuff=====================================================================
	newent->s.eType = ET_NPC; //ET_PLAYER;

	//FIXME: Call CopyParms
	if (ent->parms)
	{
		for (int parmNum = 0; parmNum < MAX_PARMS; parmNum++)
		{
			if (ent->parms->parm[parmNum][0])
			{
				Q3_SetParm(newent->s.number, parmNum, ent->parms->parm[parmNum]);
			}
		}
	}
	//FIXME: copy cameraGroup, store mine in message or other string field

	//set origin
	newent->s.pos.trType = TR_INTERPOLATE;
	newent->s.pos.trTime = level.time;
	VectorCopy(newent->r.currentOrigin, newent->s.pos.trBase);
	VectorClear(newent->s.pos.trDelta);
	newent->s.pos.trDuration = 0;
	//set angles
	newent->s.apos.trType = TR_INTERPOLATE;
	newent->s.apos.trTime = level.time;
	//VectorCopy( newent->r.currentOrigin, newent->s.apos.trBase );
	//Why was the origin being used as angles? Typo I'm assuming -rww
	VectorCopy(newent->s.angles, newent->s.apos.trBase);

	VectorClear(newent->s.apos.trDelta);
	newent->s.apos.trDuration = 0;

	newent->NPC->combatPoint = -1;

	newent->flags |= FL_NOTARGET; //So he's ignored until he's fully spawned
	newent->s.eFlags |= EF_NODRAW; //So he's ignored until he's fully spawned

	newent->think = NPC_Begin;
	newent->nextthink = level.time + FRAMETIME;
	NPC_DefaultScriptFlags(newent);

	//copy over team variables, too
	newent->s.shouldtarget = ent->s.shouldtarget;
	newent->s.teamowner = ent->s.teamowner;
	newent->alliedTeam = ent->alliedTeam;
	newent->teamnodmg = ent->teamnodmg;

	//allow the NPC's default team to override if the team isn't set and you're in CoOp mode
	if (ent->s.teamowner != TEAM_FREE || level.gametype != GT_SINGLE_PLAYER)
	{
		newent->s.teamowner = ent->s.teamowner;
	}

	if (!newent->message && newent->client->NPC_class == CLASS_IMPERIAL)
	{
		//the imperial model has a key by default, remove it since we don't have a key
		NPC_SetSurfaceOnOff(newent, "l_arm_key", TURN_OFF);
	}

	if (ent->team && ent->team[0])
	{
		//specified team directly?
		newent->client->sess.sessionTeam = atoi(ent->team);
	}
	else if (newent->s.teamowner != TEAM_FREE)
	{
		newent->client->sess.sessionTeam = newent->s.teamowner;
	}
	else if (newent->alliedTeam != TEAM_FREE)
	{
		newent->client->sess.sessionTeam = newent->alliedTeam;
	}
	else if (newent->teamnodmg != TEAM_FREE)
	{
		newent->client->sess.sessionTeam = newent->teamnodmg;
	}
	else
	{
		newent->client->sess.sessionTeam = TEAM_FREE;
	}
	newent->client->ps.persistant[PERS_TEAM] = newent->client->sess.sessionTeam;

	// the client sees the NPC's own team (s.teamowner: the crosshair's green ally / red enemy, the radar) when the map
	// gave it none; NPC_Think keeps it up as scripts or the force change it (2026-11-18). Not siege, vehicles or
	// shouldtarget ones, whose teamowner means a side (and sessionTeam is left as it was).
	if (newent->s.teamowner == TEAM_FREE && level.gametype != GT_SIEGE && !newent->s.shouldtarget
		&& newent->client->NPC_class != CLASS_VEHICLE)
	{
		newent->NPC->teamownerIsTeam = qtrue;
		newent->s.teamowner = newent->client->playerTeam;
	}

	trap->LinkEntity((sharedEntity_t*)newent);

	if (!ent->use)
	{
		if (ent->target)
		{
			//use any target we're pointed at
			G_UseTargets(ent, ent);
		}
		if (ent->closetarget)
		{
			//last guy should fire this target when he dies
			newent->target = ent->closetarget;
		}
		ent->targetname = NULL;
		//why not remove me...?  Because of all the string pointers?  Just do G_NewStrings?
		G_FreeEntity(ent); //bye!
	}

finish:
	if (ent->spawnflags & NSF_DROP_TO_FLOOR)
	{
		G_SetOrigin(ent, saveOrg);
	}

	// added some attributes initialization
	if (newent && newent->client)
	{
		newent->client->cloakDebReduce = 0;

		// saboteur npcs start with cloak
		if (Q_stristr(newent->NPC_type, "saboteur"))
		{
			Saboteur_Cloak(newent);
		}
	}

	return newent;
}

void NPC_Spawn_Go(gentity_t* ent)
{
	NPC_Spawn_Do(ent);
}

/*
-------------------------
NPC_ShySpawn
-------------------------
*/

#define SHY_THINK_TIME			1000
#define SHY_SPAWN_DISTANCE		128
#define SHY_SPAWN_DISTANCE_SQR	( SHY_SPAWN_DISTANCE * SHY_SPAWN_DISTANCE )

void NPC_ShySpawn(gentity_t* ent)
{
	ent->nextthink = level.time + SHY_THINK_TIME;
	ent->think = NPC_ShySpawn;

	//rwwFIXMEFIXME: Care about other clients not just 0?
	if (DistanceSquared(g_entities[0].r.currentOrigin, ent->r.currentOrigin) <= SHY_SPAWN_DISTANCE_SQR)
		return;

	if (InFOV(ent, &g_entities[0], 80, 64)) // FIXME: hardcoded fov
		if (NPC_ClearLOS2(&g_entities[0], ent->r.currentOrigin))
			return;

	ent->think = 0;
	ent->nextthink = 0;

	NPC_Spawn_Go(ent);
}

/*
-------------------------
NPC_Spawn
-------------------------
*/

void NPC_Spawn(gentity_t* ent, const gentity_t* other, gentity_t* activator)
{
	//delay before spawning NPC
	if (ent->delay)
	{
		if (ent->spawnflags & 2048) // SHY
		{
			ent->think = NPC_ShySpawn;
		}
		else
		{
			ent->think = NPC_Spawn_Go;
		}

		ent->nextthink = level.time + ent->delay;
	}
	else
	{
		if (ent->spawnflags & 2048) // SHY
		{
			NPC_ShySpawn(ent);
		}
		else
		{
			NPC_Spawn_Do(ent);
		}
	}
}

/*QUAKED NPC_spawner (1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

NPC_type - name of NPC (in npcs.cfg) to spawn in

targetname - name this NPC goes by for targetting
target - NPC will fire this when it spawns it's last NPC (should this be when the last NPC it spawned dies?)
target2 - Fired by stasiss spawners when they try to spawn while their spawner model is broken
target3 - Fired by spawner if they try to spawn and are blocked and have a wait < 0 (removes them)

If targeted, will only spawn a NPC when triggered
count - how many NPCs to spawn (only if targetted) default = 1
delay - how long to wait to spawn after used
wait - if trying to spawn and blocked, how many seconds to wait before trying again (default = 0.5, < 0 = never try again and fire target2)

NPC_targetname - NPC's targetname AND script_targetname
NPC_target - NPC's target to fire when killed
NPC_target2 - NPC's target to fire when knocked out
NPC_target4 - NPC's target to fire when killed by friendly fire
NPC_type - type of NPC ("Borg" (default), "Xian", etc)
health - starting health (default = 100)

spawnscript - default script to run once spawned (none by default)
usescript - default script to run when used (none by default)
awakescript - default script to run once awoken (none by default)
angerscript - default script to run once angered (none by default)
painscript - default script to run when hit (none by default)
fleescript - default script to run when hit and below 50% health (none by default)
deathscript - default script to run when killed (none by default)
These strings can be used to activate behaviors instead of scripts - these are checked
first and so no scripts should be names with these names:
	default - 0: whatever
	idle - 1: Stand around, do abolutely nothing
	roam - 2: Roam around, collect stuff
	walk - 3: Crouch-Walk toward their goals
	run - 4: Run toward their goals
	standshoot - 5: Stay in one spot and shoot- duck when neccesary
	standguard - 6: Wait around for an enemy
	patrol - 7: Follow a path, looking for enemies
	huntkill - 8: Track down enemies and kill them
	evade - 9: Run from enemies
	evadeshoot - 10: Run from enemies, shoot them if they hit you
	runshoot - 11: Run to your goal and shoot enemy when possible
	defend - 12: Defend an entity or spot?
	snipe - 13: Stay hidden, shoot enemy only when have perfect shot and back turned
	combat - 14: Attack, evade, use cover, move about, etc.  Full combat AI - id NPC code
	medic - 15: Go for lowest health buddy, hide and heal him.
	takecover - 16: Find nearest cover from enemies
	getammo - 17: Go get some ammo
	advancefight - 18: Go somewhere and fight along the way
	face - 19: turn until facing desired angles
	wait - 20: do nothing
	formation - 21: Maintain a formation
	crouch - 22: Crouch-walk toward their goals

delay - after spawned or triggered, how many seconds to wait to spawn the NPC

showhealth - set to 1 to show health bar on this entity when crosshair is over it

teamowner - crosshair shows green for this team, red for opposite team
	0 - none
	1 - red
	2 - blue

teamuser - only this team can use this NPC
	0 - none
	1 - red
	2 - blue

teamnodmg - team that NPC does not take damage from (turrets and other auto-defenses that have "alliedTeam" set to this team won't target this NPC)
	0 - none
	1 - red
	2 - blue

"noBasicSounds" - set to 1 to prevent loading and usage of basic sounds (pain, death, etc)
"noCombatSounds" - set to 1 to prevent loading and usage of combat sounds (anger, victory, etc.)
"noExtraSounds" - set to 1 to prevent loading and usage of "extra" sounds (chasing the enemy - detecting them, flanking them... also jedi combat sounds)
*/
extern void NPC_PrecacheAnimationCFG(const char* npc_type);
void NPC_Precache(gentity_t* spawner);

static void NPC_PrecacheType(char* NPC_type)
{
	gentity_t* fakespawner = G_Spawn();
	if (fakespawner)
	{
		fakespawner->NPC_type = NPC_type;
		NPC_Precache(fakespawner);
		//NOTE: does the spawner have to stay around to send any precached info to the clients...?
		G_FreeEntity(fakespawner);
	}
}

extern void NPC_PrecacheByClassName(const char* type);
extern void Precashe_Key(void);

/*
-------------------------
Missions (GT_SINGLE_PLAYER) and g_allowNPC (DH 2026-11-02)

g_allowNPC 0: only the NPCs the level needs spawn. Needed: a spawner whose killing fires something (deathtarget /
target / target2: kill objectives, door counters, the mission waves of ext_data/mission_ents), or whose name is listed
for the map in ext_data/mission_npcs.dat (its NPC_targetname, or "@" + its targetname when it has none: the map's scripts
affect, kill, remove, use it or make it an enemy / leader / view target; built from the campaign scripts, DH may add
names). The old list of NPC types stays allowed too. g_allowNPC 1: every NPC of the level (the old per-map crash bans
were removed 2026-11-25).
-------------------------
*/
#define MISSION_NPCS_FILE		"ext_data/mission_npcs.dat"
#define MISSION_NPCS_MAXSIZE	65536

static qboolean NPC_MissionNeeded(const gentity_t* self)
{
	static char buf[MISSION_NPCS_MAXSIZE];
	static int loadedLevel = -1;
	char mapname[MAX_QPATH] = { 0 };
	char* s = NULL;

	if (level.gametype != GT_SINGLE_PLAYER)
	{
		return qfalse;
	}
	if (G_SpawnString("deathtarget", "", &s) && s[0] || self->target && self->target[0]
		|| self->target2 && self->target2[0])
	{
		return qtrue;
	}

	if (loadedLevel != level.startTime)
	{
		fileHandle_t f = 0;
		const int len = trap->FS_Open(MISSION_NPCS_FILE, &f, FS_READ);
		loadedLevel = level.startTime;
		buf[0] = 0;
		if (f)
		{
			if (len > 0 && len < (int)sizeof buf)
			{
				trap->FS_Read(buf, len, f);
				buf[len] = 0;
			}
			trap->FS_Close(f);
		}
	}
	if (!buf[0])
	{
		return qfalse;
	}

	trap->Cvar_VariableStringBuffer("mapname", mapname, sizeof mapname);
	const char* p = buf;
	while (1)
	{
		const char* token = COM_ParseExt(&p, qtrue);
		if (!token[0])
		{
			return qfalse;
		}
		if (Q_stricmp(token, mapname))
		{
			SkipRestOfLine(&p);
			continue;
		}
		while ((token = COM_ParseExt(&p, qfalse))[0])
		{
			if (token[0] == '@')
			{
				if (self->targetname && !Q_stricmp(token + 1, self->targetname))
				{
					return qtrue;
				}
			}
			else if (self->NPC_targetname && !Q_stricmp(token, self->NPC_targetname))
			{
				return qtrue;
			}
		}
		return qfalse;
	}
}

/*
-------------------------
Missions: cutscene-only actors never spawn (DH 2026-11-03)

A cutscene in a mission is not shown (g_ICARUScb.c), so the NPCs that only play in a map's start cutscene (Kyle and the
masters at academy1, the fake players...) would just stand there. ext_data/mission_cutscene_actors.dat lists them per
map: <map> NPC_targetname ... (built from the campaign scripts: named by the start cutscene's scripts only, targeted by
no entity, their killing fires nothing; only maps whose start cutscene has a skip script). A listed spawner whose killing
fires something is kept all the same.
-------------------------
*/
#define MISSION_ACTORS_FILE		"ext_data/mission_cutscene_actors.dat"
#define MISSION_ACTORS_MAXSIZE	16384

static qboolean NPC_MissionCutsceneActor(const gentity_t* self)
{
	static char buf[MISSION_ACTORS_MAXSIZE];
	static int loadedLevel = -1;
	char mapname[MAX_QPATH] = { 0 };
	char* s = NULL;

	if (level.gametype != GT_SINGLE_PLAYER || !self->NPC_targetname || !self->NPC_targetname[0])
	{
		return qfalse;
	}
	if (G_SpawnString("deathtarget", "", &s) && s[0] || self->target && self->target[0]
		|| self->target2 && self->target2[0])
	{
		return qfalse;
	}

	if (loadedLevel != level.startTime)
	{
		fileHandle_t f = 0;
		const int len = trap->FS_Open(MISSION_ACTORS_FILE, &f, FS_READ);
		loadedLevel = level.startTime;
		buf[0] = 0;
		if (f)
		{
			if (len > 0 && len < (int)sizeof buf)
			{
				trap->FS_Read(buf, len, f);
				buf[len] = 0;
			}
			trap->FS_Close(f);
		}
	}
	if (!buf[0])
	{
		return qfalse;
	}

	trap->Cvar_VariableStringBuffer("mapname", mapname, sizeof mapname);
	const char* p = buf;
	while (1)
	{
		const char* token = COM_ParseExt(&p, qtrue);
		if (!token[0])
		{
			return qfalse;
		}
		if (Q_stricmp(token, mapname))
		{
			SkipRestOfLine(&p);
			continue;
		}
		while ((token = COM_ParseExt(&p, qfalse))[0])
		{
			if (!Q_stricmp(token, self->NPC_targetname))
			{
				return qtrue;
			}
		}
		return qfalse;
	}
}

void SP_NPC_spawner(gentity_t* self)
{
	int t = 0;
	const qboolean needed = NPC_MissionNeeded(self); // (Missions: the level needs this NPC, above)

	if (NPC_MissionCutsceneActor(self))
	{
		// (Missions: an NPC only the start cutscene uses, above)
		self->think = G_FreeEntity;
		self->nextthink = level.time;
		return;
	}

	if (level.gametype == GT_SINGLE_PLAYER && !g_allowNPC.integer) //singleplayer npcs turned off
	{
		// exceptions so scripts can run
		if (!needed && !(Q_stricmp(self->NPC_type, "ImpOfficer") == 0
			|| Q_stricmp(self->NPC_type, "ImpOfficer2") == 0
			|| Q_stricmp(self->NPC_type, "ImpCommander") == 0
			|| Q_stricmp(self->NPC_type, "Imperial") == 0
			|| Q_stricmp(self->NPC_type, "Imperial2") == 0
			|| Q_stricmp(self->NPC_type, "ImperialJK2") == 0
			|| Q_stricmp(self->NPC_type, "kyle") == 0
			|| Q_stricmp(self->NPC_type, "Kyle2") == 0
			|| Q_stricmp(self->NPC_type, "Kyle_boss") == 0
			|| Q_stricmp(self->NPC_type, "KyleJK2") == 0
			|| Q_stricmp(self->NPC_type, "KyleJK2noforce") == 0
			|| Q_stricmp(self->NPC_type, "Kyle_JKA") == 0
			|| Q_stricmp(self->NPC_type, "outcast_jedi") == 0
			|| Q_stricmp(self->NPC_type, "Player") == 0
			|| Q_stricmp(self->NPC_type, "Player1") == 0
			|| Q_stricmp(self->NPC_type, "Player2") == 0
			|| Q_stricmp(self->NPC_type, "Player3") == 0
			|| Q_stricmp(self->NPC_type, "Jan") == 0
			|| Q_stricmp(self->NPC_type, "Jan2") == 0
			|| Q_stricmp(self->NPC_type, "JanJK2") == 0
			|| Q_stricmp(self->NPC_type, "sand_creature") == 0
			|| Q_stricmp(self->NPC_type, "sand_creature_fast") == 0
			|| Q_stricmp(self->NPC_type, "mutant_rancor") == 0
			|| Q_stricmp(self->NPC_type, "wampa") == 0
			|| Q_stricmp(self->NPC_type, "Prisoner") == 0
			|| Q_stricmp(self->NPC_type, "Prisoner2") == 0
			|| Q_stricmp(self->NPC_type, "Prisoner3") == 0
			|| Q_stricmp(self->NPC_type, "rancor") == 0
			|| Q_stricmp(self->NPC_type, "rancor2") == 0
			|| Q_stricmp(self->NPC_type, "probe") == 0
			|| Q_stricmp(self->NPC_type, "mark1") == 0
			|| Q_stricmp(self->NPC_type, "mark2") == 0
			|| Q_stricmp(self->NPC_type, "Luke") == 0
			|| Q_stricmp(self->NPC_type, "LukeJK2") == 0
			|| Q_stricmp(self->NPC_type, "lannik_racto") == 0
			|| Q_stricmp(self->NPC_type, "interrogator") == 0
			|| Q_stricmp(self->NPC_type, "gonk") == 0
			|| Q_stricmp(self->NPC_type, "gonk2") == 0
			|| Q_stricmp(self->NPC_type, "wampa") == 0))
		{
			self->think = G_FreeEntity;
			self->nextthink = level.time;
			return;
		}
	}
	else
	{
		if (level.gametype != GT_SINGLE_PLAYER) //not single player so no npcs
		{
			self->think = G_FreeEntity;
			self->nextthink = level.time;
			return;
		}
		// (2026-11-25, Jace: the per-map NPC bans here - t1_fatal, t3_hevil, vjun3, t3_rift, t1_rail, t1_sour, t2_rouge,
		// t3_byss - were old crash workarounds for problems that no longer exist: gone, every NPC of the map spawns)
	}

	if (!self->fullName || !self->fullName[0])
	{
		//FIXME: make an index into an external string table for localization
		self->fullName = "Humanoid Lifeform";
	}

	if (!self->count)
	{
		self->count = 1;
	}

	{
		//Stop loading of certain extra sounds
		static int garbage;

		if (G_SpawnInt("noBasicSounds", "0", &garbage))
		{
			self->r.svFlags |= SVF_NO_BASIC_SOUNDS;
		}
		if (G_SpawnInt("noCombatSounds", "0", &garbage))
		{
			self->r.svFlags |= SVF_NO_COMBAT_SOUNDS;
		}
		if (G_SpawnInt("noExtraSounds", "0", &garbage))
		{
			self->r.svFlags |= SVF_NO_EXTRA_SOUNDS;
		}
	}

	if (!self->wait)
	{
		self->wait = 500;
	}
	else
	{
		if (!(self->spawnflags & 65536)) // dont multiply if has this flag
		{
			self->wait *= 1000; //1 = 1 msec, 1000 = 1 sec
		}
	}

	if (!(self->spawnflags & 65536)) // dont multiply if has this flag
	{
		self->wait *= 1000; //1 = 1 msec, 1000 = 1 sec
	}

	self->spawnflags |= 65536;

	G_SpawnInt("showhealth", "0", &t);
	if (t)
	{
		self->s.shouldtarget = qtrue;
	}

	//We have to load the animation.cfg now because spawnscripts are going to want to set anims and we need to know their length and if they're valid
	NPC_PrecacheAnimationCFG(self->NPC_type);

	//rww - can't cheat and do this on the client like in SP, so I'm doing this.
	NPC_Precache(self);

	if (self->targetname)
	{
		//Wait for triggering
		self->use = NPC_Spawn;
		self->r.svFlags |= SVF_NPC_PRECACHE;
	}
	else
	{
		//NOTE: auto-spawners never check for shy spawning

		if (1) //just gonna always do this I suppose.
		{
			//in entity spawn stage - map starting up
			self->think = NPC_Spawn_Go;
			self->nextthink = level.time + START_TIME_REMOVE_ENTS + 50;
		}
	}

	NPC_PrecacheByClassName(self->NPC_type); //precashe additional fx/sounds/models/etc
	//for special NPC classes
	if (self->message)
	{
		//precashe key stuff if we're set to carry a key.
		Precashe_Key();
	}
}

extern void G_VehicleSpawn(gentity_t* self);
/*QUAKED NPC_Vehicle (1 0 0) (-16 -16 -24) (16 16 32) NO_PILOT_DIE SUSPENDED x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
NO_PILOT_DIE - die after certain amount of time of not having a pilot
SUSPENDED - Fighters: Don't drop until someone gets in it (this only works as long as no-nw has *ever* ridden the vehicle, to simulate ships that are suspended-docked) - note: ships inside trigger_spaces do not drop when unoccupied
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

set NPC_type to vehicle name in vehicles.dat

"dropTime" use with SUSPENDED - if set, the vehicle will drop straight down for this number of seconds before flying forward
"dmg" use with NO_PILOT_DIE - delay in milliseconds for ship to explode if no pilot (default 10000)
"speed" use with NO_PILOT_DIE - distance for pilot to get away from ship after dismounting before it starts counting down the death timer
"model2" - if the vehicle can have a droid (has "*droidunit" tag), this NPC will be spawned and placed there - note: game will automatically use the one specified in the .veh file (if any) or, absent that, it will use an R2D2 or R5D2 NPC)

showhealth - set to 1 to show health bar on this entity when crosshair is over it

teamowner - crosshair shows green for this team, red for opposite team
	0 - none
	1 - red
	2 - blue

teamuser - only this team can use this NPC
	0 - none
	1 - red
	2 - blue

teamnodmg - team that NPC does not take damage from (turrets and other auto-defenses that have "alliedTeam" set to this team won't target this NPC)
	0 - none
	1 - red
	2 - blue
*/
static qboolean NPC_VehiclePrecache(const gentity_t* spawner)
{
	char* droidNPCType = NULL;
	const int iVehIndex = BG_VehicleGetIndex(spawner->NPC_type);
	if (iVehIndex == VEHICLE_NONE)
	{
		//fixme: error msg?
		return qfalse;
	}

	G_model_index(va("$%s", spawner->NPC_type)); //make sure the thing is frickin precached
	//now cache his model/skin/anim config
	const vehicleInfo_t* pVehInfo = &g_vehicleInfo[iVehIndex];
	if (pVehInfo->model && pVehInfo->model[0])
	{
		void* tempG2 = NULL;
		int skin = 0;
		if (pVehInfo->skin && pVehInfo->skin[0])
		{
			skin = trap->R_RegisterSkin(va("models/players/%s/model_%s.skin", pVehInfo->model, pVehInfo->skin));
		}
		trap->G2API_InitGhoul2Model(&tempG2, va("models/players/%s/model.glm", pVehInfo->model), 0, skin, 0, 0, 0);
		if (tempG2)
		{
			//now, cache the anim config.
			char GLAName[1024] = { 0 };

			GLAName[0] = 0;
			trap->G2API_GetGLAName(tempG2, 0, GLAName);

			if (GLAName[0])
			{
				char* slash = Q_strrchr(GLAName, '/');
				if (slash)
				{
					strcpy(slash, "/animation.cfg");

					bg_parse_animation_file(GLAName, NULL, qfalse);
				}
			}
			trap->G2API_CleanGhoul2Models(&tempG2);
		}
	}

	//also precache the droid NPC if there is one
	if (spawner->model2
		&& spawner->model2[0])
	{
		droidNPCType = spawner->model2;
	}
	else if (g_vehicleInfo[iVehIndex].droidNPC
		&& g_vehicleInfo[iVehIndex].droidNPC[0])
	{
		droidNPCType = g_vehicleInfo[iVehIndex].droidNPC;
	}

	if (droidNPCType)
	{
		if (Q_stricmp("random", droidNPCType) == 0
			|| Q_stricmp("default", droidNPCType) == 0)
		{
			//precache both r2 and r5, as defaults
			NPC_PrecacheType("r2d2");
			NPC_PrecacheType("r5d2");
		}
		else
		{
			NPC_PrecacheType(droidNPCType);
		}
	}
	return qtrue;
}

void NPC_VehicleSpawnUse(gentity_t* self, gentity_t* other, gentity_t* activator)
{
	if (self->delay)
	{
		self->think = G_VehicleSpawn;
		self->nextthink = level.time + self->delay;
	}
	else
	{
		G_VehicleSpawn(self);
	}
}

void SP_NPC_Vehicle(gentity_t* self)
{
	float dropTime = 0.0f;
	int t = 0;
	if (!self->NPC_type)
	{
		self->NPC_type = "swoop";
	}

	if (!self->classname)
	{
		self->classname = "NPC_Vehicle";
	}

	if (!self->wait)
	{
		self->wait = 500;
	}
	else
	{
		self->wait *= 1000; //1 = 1 msec, 1000 = 1 sec
	}
	self->delay *= 1000; //1 = 1 msec, 1000 = 1 sec

	G_SetOrigin(self, self->s.origin);
	G_SetAngles(self, self->s.angles);
	G_SpawnFloat("dropTime", "0", &dropTime);
	if (dropTime)
	{
		self->fly_sound_debounce_time = ceil(dropTime * 1000.0);
	}

	G_SpawnInt("showhealth", "0", &t);
	if (t)
	{
		self->s.shouldtarget = qtrue;
	}

	if (self->targetname)
	{
		if (!NPC_VehiclePrecache(self))
		{
			//FIXME: err msg?
			G_FreeEntity(self);
			return;
		}
		self->use = NPC_VehicleSpawnUse;
	}
	else
	{
		if (self->delay)
		{
			if (!NPC_VehiclePrecache(self))
			{
				//FIXME: err msg?
				G_FreeEntity(self);
				return;
			}
			self->think = G_VehicleSpawn;
			self->nextthink = level.time + self->delay;
		}
		else
		{
			G_VehicleSpawn(self);
		}
	}
}

//Characters

//STAR WARS NPCs============================================================================

//=============================================================================================
//CHARACTERS
//=============================================================================================

/*QUAKED NPC_Kyle (1 0 0) (-16 -16 -24) (16 16 32) x RIFLEMAN PHASER TRICORDER DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Kyle(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "Kyle_boss";
	}
	else
	{
		self->NPC_type = "Kyle";
	}

	WP_SetSaberModel(NULL, CLASS_KYLE);

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Lando(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Lando(gentity_t* self)
{
	self->NPC_type = "Lando";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Jan(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Jan(gentity_t* self)
{
	self->NPC_type = "Jan";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Luke(1 0 0) (-16 -16 -24) (16 16 40) x x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Luke(gentity_t* self)
{
	self->NPC_type = "Luke";

	WP_SetSaberModel(NULL, CLASS_LUKE);

	SP_NPC_spawner(self);
}

void SP_NPC_Merchant(gentity_t* self)
{
	self->NPC_type = "merchant";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_MonMothma(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_MonMothma(gentity_t* self)
{
	self->NPC_type = "MonMothma";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Tavion (1 0 0) (-16 -16 -24) (16 16 32) x x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Tavion(gentity_t* self)
{
	self->NPC_type = "Tavion";

	WP_SetSaberModel(NULL, CLASS_TAVION);

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Tavion_New (1 0 0) (-16 -16 -24) (16 16 32) SCEPTER SITH_SWORD x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
Has a red lightsaber and force powers, uses her saber style from JK2

SCEPTER - Has a red lightsaber and force powers, Ragnos' Scepter in left hand, uses dual saber style and occasionally attacks with Scepter
SITH_SWORD - Has Ragnos' Sith Sword in right hand and force powers, uses strong style
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Tavion_New(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "tavion_scepter";
	}
	else if (self->spawnflags & 2)
	{
		self->NPC_type = "tavion_sith_sword";
	}
	else
	{
		self->NPC_type = "tavion_new";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Alora (1 0 0) (-16 -16 -24) (16 16 32) DUAL x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
Lightsaber and level 2 force powers, 300 health

DUAL - Dual sabers and level 3 force powers, 500 health
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Alora(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "alora_dual";
	}
	else
	{
		self->NPC_type = "alora";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Reborn_New(1 0 0) (-16 -16 -24) (16 16 40) DUAL STAFF WEAK x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY

Reborn is an excellent lightsaber fighter, acrobatic and uses force powers.  Full-length red saber, 200 health.

DUAL - Use 2 shorter sabers
STAFF - Uses a saber staff
WEAK - Is a bit less tough
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Reborn_New(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 4)
		{
			//weaker guys
			if (self->spawnflags & 1)
			{
				self->NPC_type = "reborn_dual2";
			}
			else if (self->spawnflags & 2)
			{
				self->NPC_type = "reborn_staff2";
			}
			else
			{
				self->NPC_type = "reborn_new2";
			}
		}
		else
		{
			if (self->spawnflags & 1)
			{
				self->NPC_type = "reborn_dual";
			}
			else if (self->spawnflags & 2)
			{
				self->NPC_type = "reborn_staff";
			}
			else
			{
				self->NPC_type = "reborn_new";
			}
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Cultist_Saber(1 0 0) (-16 -16 -24) (16 16 40) MED STRONG ALL THROW CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
Uses a saber and no force powers.  100 health.

default fencer uses fast style - weak, but can attack rapidly.  Good defense.

MED - Uses medium style - average speed and attack strength, average defense.
STRONG - Uses strong style, slower than others, but can do a lot of damage with one blow.  Weak defense.
ALL - Knows all 3 styles, switches between them, good defense.
THROW - can throw their saber (level 2) - reduces their defense some (can use this spawnflag alone or in combination with any *one* of the previous 3 spawnflags)

CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Cultist_Saber(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_med_throw";
			}
			else
			{
				self->NPC_type = "cultist_saber_med";
			}
		}
		else if (self->spawnflags & 2)
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_strong_throw";
			}
			else
			{
				self->NPC_type = "cultist_saber_strong";
			}
		}
		else if (self->spawnflags & 4)
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_all_throw";
			}
			else
			{
				self->NPC_type = "cultist_saber_all";
			}
		}
		else
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_throw";
			}
			else
			{
				self->NPC_type = "cultist_saber";
			}
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Cultist_Saber_Powers(1 0 0) (-16 -16 -24) (16 16 40) MED STRONG ALL THROW CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
Uses a saber and has a couple low-level powers.  150 health.

default fencer uses fast style - weak, but can attack rapidly.  Good defense.

MED - Uses medium style - average speed and attack strength, average defense.
STRONG - Uses strong style, slower than others, but can do a lot of damage with one blow.  Weak defense.
ALL - Knows all 3 styles, switches between them, good defense.
THROW - can throw their saber (level 2) - reduces their defense some (can use this spawnflag alone or in combination with any *one* of the previous 3 spawnflags)

CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Cultist_Saber_Powers(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_med_throw2";
			}
			else
			{
				self->NPC_type = "cultist_saber_med2";
			}
		}
		else if (self->spawnflags & 2)
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_strong_throw2";
			}
			else
			{
				self->NPC_type = "cultist_saber_strong2";
			}
		}
		else if (self->spawnflags & 4)
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_all_throw2";
			}
			else
			{
				self->NPC_type = "cultist_saber_all2";
			}
		}
		else
		{
			if (self->spawnflags & 8)
			{
				self->NPC_type = "cultist_saber_throw";
			}
			else
			{
				self->NPC_type = "cultist_saber2";
			}
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Cultist(1 0 0) (-16 -16 -24) (16 16 40) SABER GRIP LIGHTNING DRAIN CEILING CINEMATIC NOTSOLID STARTINSOLID SHY

Cultist uses a blaster and force powers.  40 health.

SABER - Uses a saber and no force powers
GRIP - Uses no weapon and grip, push and pull
LIGHTNING - Uses no weapon and lightning and push
DRAIN - Uses no weapons and drain and push

CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Cultist(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = NULL;
			self->spawnflags = 0; //fast, no throw
			switch (Q_irand(0, 2))
			{
			case 0: //medium
				self->spawnflags |= 1;
				break;
			case 1: //strong
				self->spawnflags |= 2;
				break;
			case 2: //all
				self->spawnflags |= 4;
				break;
			default:;
			}
			if (Q_irand(0, 1))
			{
				//throw
				self->spawnflags |= 8;
			}
			SP_NPC_Cultist_Saber(self);
			return;
		}
		if (self->spawnflags & 2)
		{
			self->NPC_type = "cultist_grip";
		}
		else if (self->spawnflags & 4)
		{
			self->NPC_type = "cultist_lightning";
		}
		else if (self->spawnflags & 8)
		{
			if (!Q_irand(0, 1))
			{
				self->NPC_type = "cultist_drain";
			}
			else
			{
				self->NPC_type = "cultist_destroyer";
			}
		}
		else
		{
			self->NPC_type = "cultist";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Cultist_Commando(1 0 0) (-16 -16 -24) (16 16 40) x x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY

Cultist uses dual blaster pistols and force powers.  40 health.

CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Cultist_Commando(gentity_t* self)
{
	if (!self->NPC_type)
	{
		self->NPC_type = "cultistcommando";
	}
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Cultist_Destroyer(1 0 0) (-16 -16 -24) (16 16 40) x x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY

Cultist has no weapons, runs up to you chanting & building up a Force Destruction blast - when gets to you, screams & explodes

CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Cultist_Destroyer(gentity_t* self)
{
	self->NPC_type = "cultist_destroyer"; //"cultist_explode";
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Reelo(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Reelo(gentity_t* self)
{
	self->NPC_type = "Reelo";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Galak(1 0 0) (-16 -16 -24) (16 16 40) MECH x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
MECH - will be the armored Galak

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Galak(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "Galak_Mech";
		NPC_GalakMech_Precache();
	}
	else
	{
		self->NPC_type = "Galak";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Desann(1 0 0) (-16 -16 -24) (16 16 40) x x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Desann(gentity_t* self)
{
	self->NPC_type = "Desann";

	WP_SetSaberModel(NULL, CLASS_DESANN);

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Bartender(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Bartender(gentity_t* self)
{
	self->NPC_type = "Bartender";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_MorganKatarn(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_MorganKatarn(gentity_t* self)
{
	self->NPC_type = "MorganKatarn";

	SP_NPC_spawner(self);
}

//=============================================================================================
//ALLIES
//=============================================================================================

/*QUAKED NPC_Jedi(1 0 0) (-16 -16 -24) (16 16 40) TRAINER MASTER RANDOM x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
TRAINER - Special Jedi- instructor
MASTER - Special Jedi- master
RANDOM - creates a random Jedi student using the available player models/skins
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Ally Jedi NPC Buddy - tags along with player
*/
void SP_NPC_Jedi(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 4)
		{
			//random!
			int sanityCheck = 20; //just in case
			while (sanityCheck--)
			{
				switch (Q_irand(0, 11))
				{
				case 0:
					self->NPC_type = "jedi_hf1";
					break;
				case 1:
					self->NPC_type = "jedi_hf2";
					break;
				case 2:
					self->NPC_type = "jedi_hm1";
					break;
				case 3:
					self->NPC_type = "jedi_hm2";
					break;
				case 4:
					self->NPC_type = "jedi_kdm1";
					break;
				case 5:
					self->NPC_type = "jedi_kdm2";
					break;
				case 6:
					self->NPC_type = "jedi_rm1";
					break;
				case 7:
					self->NPC_type = "jedi_rm2";
					break;
				case 8:
					self->NPC_type = "jedi_tf1";
					break;
				case 9:
					self->NPC_type = "jedi_tf2";
					break;
				case 10:
					self->NPC_type = "jedi_zf1";
					break;
				case 11:
				default: //just in case
					self->NPC_type = "jedi_zf2";
					break;
				}
			}
		}
		else if (self->spawnflags & 2)
		{
			switch (Q_irand(0, 2))
			{
			case 0:
				self->NPC_type = "jedimaster";
				break;
			case 1:
				self->NPC_type = "jedimaster2";
				break;
			case 2:
				self->NPC_type = "jedimaster3";
				break;
			default:;
			}
		}
		else if (self->spawnflags & 1)
		{
			switch (Q_irand(0, 3))
			{
			case 0:
				self->NPC_type = "jeditrainer";
				break;
			case 1:
				self->NPC_type = "jeditrainer2";
				break;
			case 2:
				self->NPC_type = "jeditrainer3";
				break;
			case 3:
				self->NPC_type = "jeditrainer4";
				break;
			default:;
			}
		}
		else
		{
			switch (Q_irand(0, 3))
			{
			case 0:
				self->NPC_type = "JediF";
				break;
			case 1:
				self->NPC_type = "Jedi";
				break;
			case 2:
				self->NPC_type = "Jedi2";
				break;
			case 3:
				self->NPC_type = "Jedi3";
				break;
			default:;
			}
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Prisoner(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Prisoner(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "elder";
			}
			else
			{
				self->NPC_type = "elder2";
			}
		}
		else
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "Prisoner";
			}
			else
			{
				self->NPC_type = "Prisoner2";
			}
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Rebel(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Rebel(gentity_t* self)
{
	if (!self->NPC_type)
	{
		switch (Q_irand(0, 3))
		{
		case 0:
			self->NPC_type = "Rebel";
			break;
		case 1:
			self->NPC_type = "Rebel2";
			break;
		case 2:
			self->NPC_type = "Rebel_Pilot";
			break;
		case 3:
			self->NPC_type = "Rebel3";
			break;
		default:;
		}
	}

	SP_NPC_spawner(self);
}

//=============================================================================================
//ENEMIES
//=============================================================================================

/*QUAKED NPC_Human_Merc(1 0 0) (-16 -16 -24) (16 16 40) BOWCASTER REPEATER FLECHETTE CONCUSSION DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
100 health, blaster rifle

BOWCASTER - Starts with a Bowcaster
REPEATER - Starts with a Repeater
FLECHETTE - Starts with a Flechette gun
CONCUSSION - Starts with a Concussion Rifle

If you want them to start with any other kind of weapon, make a spawnscript for them that sets their weapon.

"message" - turns on his key surface.  This is the name of the key you get when you walk over his body.  This must match the "message" field of the func_security_panel you want this key to open.  Set to "goodie" to have him carrying a goodie key that player can use to operate doors with "GOODIE" spawnflag.  NOTE: this overrides all the weapon spawnflags

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Human_Merc(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->message)
		{
			self->NPC_type = "human_merc_key";
		}
		else if (self->spawnflags & 1)
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "human_merc_bow";
			}
			else
			{
				self->NPC_type = "human_merc_bow2";
			}
		}
		else if (self->spawnflags & 2)
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "human_merc_rep";
			}
			else
			{
				self->NPC_type = "human_merc_rep2";
			}
		}
		else if (self->spawnflags & 4)
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "human_merc_flc";
			}
			else
			{
				self->NPC_type = "human_merc_flc2";
			}
		}
		else if (self->spawnflags & 8)
		{
			self->NPC_type = "human_merc_cnc";
		}
		else
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "human_merc";
			}
			else
			{
				self->NPC_type = "human_merc2";
			}
		}
	}
	RegisterItem(BG_FindItemForWeapon(WP_ROCKET_LAUNCHER));
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Stormtrooper(1 0 0) (-16 -16 -24) (16 16 40) OFFICER COMMANDER ALTOFFICER ROCKET DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
30 health, blaster

OFFICER - 60 health, flechette
COMMANDER - 60 health, heavy repeater
ALTOFFICER - 60 health, alt-fire flechette (grenades)
ROCKET - 60 health, rocket launcher

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
// MD SP plays Jedi Outcast with ext_data/npcs' _jk2 stormtroopers, pilots and imperials (its NPC_spawn.cpp, com_outcast
// 1), so a Jedi Outcast mission in MP does too (g_missionSPNPCs, 2026-11-18). NPC_AddMissingParms finds their files.
static char* NPC_OutcastType(char* type)
{
	static const char* jk2[][2] = {
		{ "rockettrooper", "rockettrooper_jk2" }, { "stofficeralt", "stofficeralt_jk2" },
		{ "stcommander", "stcommander_jk2" }, { "stofficer", "stofficer_jk2" }, { "StormTrooper", "StormTrooper_jk2" },
		{ "StormTrooper2", "StormTrooper2_jk2" }, { "stormpilot", "stormpilot_jk2" }, { "stormpilot2", "stormpilot_jk2" },
		{ "ImpOfficer", "ImpOfficer_jk2" }, { "ImpCommander", "ImpCommander_jk2" }, { "Imperial", "Imperial_jk2" }
	};
	char mapname[MAX_QPATH] = { 0 };
	int i = 0;

	if (!type || g_missionSPNPCs.integer <= 0 || (g_missionSPNPCs.integer < 2 && level.gametype != GT_SINGLE_PLAYER))
	{
		return type;
	}
	trap->Cvar_VariableStringBuffer("mapname", mapname, sizeof mapname);
	if (!G_IsOutcastSPMap(mapname))
	{
		return type;
	}
	for (i = 0; i < (int)ARRAY_LEN(jk2); i++)
	{
		if (!Q_stricmp(type, jk2[i][0]))
		{
			return (char*)jk2[i][1];
		}
	}
	return type;
}

void SP_NPC_Stormtrooper(gentity_t* self)
{
	if (self->spawnflags & 8)
	{
		//rocketer
		self->NPC_type = "rockettrooper";
	}
	else if (self->spawnflags & 4)
	{
		//alt-officer
		self->NPC_type = "stofficeralt";
	}
	else if (self->spawnflags & 2)
	{
		//commander
		self->NPC_type = "stcommander";
	}
	else if (self->spawnflags & 1)
	{
		//officer
		self->NPC_type = "stofficer";
	}
	else
	{
		//regular trooper
		if (Q_irand(0, 1))
		{
			self->NPC_type = "StormTrooper";
		}
		else
		{
			self->NPC_type = "StormTrooper2";
		}
	}
	self->NPC_type = NPC_OutcastType(self->NPC_type);

	SP_NPC_spawner(self);
}

void SP_NPC_StormtrooperOfficer(gentity_t* self)
{
	self->spawnflags |= 1;
	SP_NPC_Stormtrooper(self);
}

/*QUAKED NPC_Rosh_Penin (1 0 0) (-16 -16 -24) (16 16 32) DARKSIDE NOFORCE x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
Good Rosh
DARKSIDE (1) - Evil Rosh
NOFORCE (2) - Can't jump, starts with no saber
DROPTOFLOOR (16) - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC (32) - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID (64) - Starts not solid
STARTINSOLID (128) - Don't try to fix if spawn in solid
SHY (256) - Spawner is shy
*/
extern void npc_rosh_dark_precache(void);

void SP_NPC_Rosh_Penin(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "rosh_dark";
		npc_rosh_dark_precache();
	}
	else if (self->spawnflags & 2)
	{
		self->NPC_type = "rosh_penin_noforce";
	}
	else
	{
		self->NPC_type = "rosh_penin";
	}
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Saboteur(1 0 0) (-16 -16 -24) (16 16 40) SNIPER PISTOL x x CLOAKED CINEMATIC NOTSOLID STARTINSOLID SHY
Has a blaster rifle, can cloak and roll

SNIPER - Has a sniper rifle, no acrobatics, but can dodge
PISTOL - Just has a pistol, can roll
COMMANDO - Has 2 pistols and can roll & dodge

CLOAKED - Starts cloaked
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Saboteur(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "saboteursniper";
	}
	else if (self->spawnflags & 2)
	{
		self->NPC_type = "saboteurpistol";
	}
	else if (self->spawnflags & 4)
	{
		self->NPC_type = "saboteurcommando";
	}
	else
	{
		self->NPC_type = "saboteur";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Chewbacca
DROPTOFLOOR (16) - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC (32) - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID (64) - Starts not solid
STARTINSOLID (128) - Don't try to fix if spawn in solid
SHY (256) - Spawner is shy
*/
void SP_NPC_Chewbacca(gentity_t* self)
{
	self->NPC_type = "chewie";
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Player
Creates a clone of the player

DROPTOFLOOR (16) - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC (32) - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID (64) - Starts not solid
STARTINSOLID (128) - Don't try to fix if spawn in solid
SHY (256) - Spawner is shy
*/
void SP_NPC_Player(gentity_t* self)
{
	self->NPC_type = "player";
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Snowtrooper(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
30 health, blaster

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Snowtrooper(gentity_t* self)
{
	self->NPC_type = "snowtrooper";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Tie_Pilot(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
30 health, blaster

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Tie_Pilot(gentity_t* self)
{
	if (Q_irand(0, 1))
	{
		self->NPC_type = "stormpilot";
	}
	else
	{
		self->NPC_type = "stormpilot2";
	}
	self->NPC_type = NPC_OutcastType(self->NPC_type);

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Ugnaught(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Ugnaught(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (Q_irand(0, 1))
		{
			self->NPC_type = "Ugnaught";
		}
		else
		{
			self->NPC_type = "Ugnaught2";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Jawa(1 0 0) (-16 -16 -24) (16 16 40) ARMED x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
ARMED - starts with the Jawa gun in-hand

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Jawa(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "jawa_armed";
		}
		else
		{
			self->NPC_type = "jawa";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Gran(1 0 0) (-16 -16 -24) (16 16 40) SHOOTER BOXER x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
Uses grenade

SHOOTER - uses blaster instead of
BOXER - uses fists only
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Gran(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "granshooter";
		}
		else if (self->spawnflags & 2)
		{
			self->NPC_type = "granboxer";
		}
		else
		{
			if (Q_irand(0, 1))
			{
				self->NPC_type = "gran";
			}
			else
			{
				self->NPC_type = "gran2";
			}
		}
	}

	SP_NPC_spawner(self);
}

void SP_NPC_RocketTrooper(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "rockettrooper2Officer";
		}
		else
		{
			self->NPC_type = "rockettrooper2";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Rodian(1 0 0) (-16 -16 -24) (16 16 40) BLASTER NO_HIDE x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
BLASTER uses a blaster instead of sniper rifle, different skin
NO_HIDE (only applicable with snipers) does not duck and hide between shots
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Rodian(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "rodian2";
		}
		else
		{
			self->NPC_type = "rodian";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Weequay(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Weequay(gentity_t* self)
{
	if (!self->NPC_type)
	{
		switch (Q_irand(0, 3))
		{
		case 0:
			self->NPC_type = "Weequay";
			break;
		case 1:
			self->NPC_type = "Weequay2";
			break;
		case 2:
			self->NPC_type = "Weequay3";
			break;
		case 3:
			self->NPC_type = "Weequay4";
			break;
		default:;
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Trandoshan(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Trandoshan(gentity_t* self)
{
	if (!self->NPC_type)
	{
		self->NPC_type = "Trandoshan";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Tusken(1 0 0) (-16 -16 -24) (16 16 40) SNIPER x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Tusken(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "tuskensniper";
		}
		else
		{
			self->NPC_type = "tusken";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Noghri(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Noghri(gentity_t* self)
{
	if (!self->NPC_type)
	{
		self->NPC_type = "noghri";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_SwampTrooper(1 0 0) (-16 -16 -24) (16 16 40) REPEATER x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
REPEATER - Swaptrooper who uses the repeater
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_SwampTrooper(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "SwampTrooper2";
		}
		else
		{
			self->NPC_type = "SwampTrooper";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Imperial(1 0 0) (-16 -16 -24) (16 16 40) OFFICER COMMANDER x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY

Greyshirt grunt, uses blaster pistol, 20 health.

OFFICER - Brownshirt Officer, uses blaster rifle, 40 health
COMMANDER - Blackshirt Commander, uses rapid-fire blaster rifle, 80 healt

"message" - if a COMMANDER, turns on his key surface.  This is the name of the key you get when you walk over his body.  This must match the "message" field of the func_security_panel you want this key to open.  Set to "goodie" to have him carrying a goodie key that player can use to operate doors with "GOODIE" spawnflag.

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Imperial(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "ImpOfficer";
		}
		else if (self->spawnflags & 2)
		{
			self->NPC_type = "ImpCommander";
		}
		else
		{
			self->NPC_type = "Imperial";
		}
		self->NPC_type = NPC_OutcastType(self->NPC_type);
	}
	//rwwFIXMEFIXME: Allow goodie keys
	SP_NPC_spawner(self);
}

/*QUAKED NPC_ImpWorker(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_ImpWorker(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (!Q_irand(0, 2))
		{
			self->NPC_type = "ImpWorker";
		}
		else if (Q_irand(0, 1))
		{
			self->NPC_type = "ImpWorker2";
		}
		else
		{
			self->NPC_type = "ImpWorker3";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_BespinCop(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_BespinCop(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (!Q_irand(0, 1))
		{
			self->NPC_type = "BespinCop";
		}
		else
		{
			self->NPC_type = "BespinCop2";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_BobaFett(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/

void SP_NPC_BobaFett(gentity_t* self)
{
	self->NPC_type = "Boba_Fett";
	SP_NPC_spawner(self);
}

void SP_NPC_BobaFett2(gentity_t* self)
{
	self->NPC_type = "Boba_Fett2";
	SP_NPC_spawner(self);
}

void SP_NPC_BobaFett3(gentity_t* self)
{
	self->NPC_type = "Boba_Fett3";
	SP_NPC_spawner(self);
}

void SP_NPC_BobaFett4(gentity_t* self)
{
	self->NPC_type = "Boba_Fett4";
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Reborn(1 0 0) (-16 -16 -24) (16 16 40) FORCE FENCER ACROBAT BOSS CEILING CINEMATIC NOTSOLID STARTINSOLID SHY

Default Reborn is A poor lightsaber fighter, acrobatic and uses no force powers.  Yellow saber, 40 health.

FORCE - Uses force powers but is not the best lightsaber fighter and not acrobatic.  Purple saber, 75 health.
FENCER - A good lightsaber fighter, but not acrobatic and uses no force powers.  Yellow saber, 100 health.
ACROBAT - quite acrobatic, but not the best lightsaber fighter and uses no force powers.  Red saber, 100 health.
BOSS - quite acrobatic, good lightsaber fighter and uses force powers.  Orange saber, 150 health.

NOTE: Saber colors are temporary until they have different by skins to tell them apart

CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Reborn(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "rebornforceuser";
		}
		else if (self->spawnflags & 2)
		{
			self->NPC_type = "rebornfencer";
		}
		else if (self->spawnflags & 4)
		{
			self->NPC_type = "rebornacrobat";
		}
		else if (self->spawnflags & 8)
		{
			self->NPC_type = "rebornboss";
		}
		else
		{
			self->NPC_type = "reborn";
		}
	}

	WP_SetSaberModel(NULL, CLASS_REBORN);
	SP_NPC_spawner(self);
}

/*QUAKED NPC_ShadowTrooper(1 0 0) (-16 -16 -24) (16 16 40) x x x x CEILING CINEMATIC NOTSOLID STARTINSOLID SHY
CEILING - Sticks to the ceiling until he sees an enemy or takes pain
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_ShadowTrooper(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (!Q_irand(0, 1))
		{
			self->NPC_type = "ShadowTrooper";
		}
		else
		{
			self->NPC_type = "ShadowTrooper2";
		}
	}

	npc_shadow_trooper_precache();
	WP_SetSaberModel(NULL, CLASS_SHADOWTROOPER);

	SP_NPC_spawner(self);
}

//=============================================================================================
//MONSTERS
//=============================================================================================

/*QUAKED NPC_Monster_Murjj (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Murjj(gentity_t* self)
{
	self->NPC_type = "Murjj";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Swamp (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Swamp(gentity_t* self)
{
	self->NPC_type = "Swamp";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Howler (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Howler(gentity_t* self)
{
	self->NPC_type = "howler";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_MineMonster (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_MineMonster(gentity_t* self)
{
	self->NPC_type = "minemonster";

	SP_NPC_spawner(self);
	NPC_MineMonster_Precache();
}

/*QUAKED NPC_Monster_Claw (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Claw(gentity_t* self)
{
	self->NPC_type = "Claw";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Glider (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Glider(gentity_t* self)
{
	self->NPC_type = "Glider";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Flier2 (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Flier2(gentity_t* self)
{
	self->NPC_type = "Flier2";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Lizard (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Lizard(gentity_t* self)
{
	self->NPC_type = "Lizard";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Fish (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Fish(gentity_t* self)
{
	self->NPC_type = "Fish";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Wampa (1 0 0) (-12 -12 -24) (12 12 40) WANDER SEARCH x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
WANDER - When I don't have an enemy, I'll just wander the waypoint network aimlessly
SEARCH - When I don't have an enemy, I'll go back and forth between the nearest waypoints, looking for enemies
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Wampa(gentity_t* self)
{
	self->NPC_type = "wampa";

	NPC_Wampa_Precache();

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Rancor (1 0 0) (-30 -30 -24) (30 30 104) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Rancor(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "mutant_rancor"; // (SP: MUTANT spawnflag)
	}
	else
	{
		self->NPC_type = "rancor";
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Mutant_Rancor (1 0 0) (-60 -60 -24) (60 60 360) x FASTKILL x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
Bigger, meaner, nastier, Frencher.  Breath attack, pound attack. (SP, DH 2026-11-03)

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Monster_Mutant_Rancor(gentity_t* self)
{
	self->NPC_type = "mutant_rancor";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Monster_Sand_Creature (1 0 0) (-24 -24 -24) (24 24 0) FAST x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

turfrange - if set, they will not go beyond this dist from their spawn position
*/
void SP_NPC_Monster_Sand_Creature(gentity_t* self) //wahoo sand creature
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "sand_creature_fast";
	}
	else
	{
		self->NPC_type = "sand_creature";
	}

	SP_NPC_spawner(self);
}

//=============================================================================================
//DROIDS
//=============================================================================================

/*QUAKED NPC_Droid_Interrogator (1 0 0) (-12 -12 -24) (12 12 0) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Droid_Interrogator(gentity_t* self)
{
	self->NPC_type = "interrogator";

	SP_NPC_spawner(self);

	NPC_Interrogator_Precache();
}

/*QUAKED NPC_Droid_Probe (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Imperial Probe Droid - the multilegged floating droid that Han and Chewie shot on the ice planet Hoth
*/
void SP_NPC_Droid_Probe(gentity_t* self)
{
	self->NPC_type = "probe";

	SP_NPC_spawner(self);

	NPC_Probe_Precache();
}

/*QUAKED NPC_Droid_Mark1 (1 0 0) (-36 -36 -24) (36 36 80) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Big walking droid

*/
void SP_NPC_Droid_Mark1(gentity_t* self)
{
	self->NPC_type = "mark1";

	SP_NPC_spawner(self);

	NPC_Mark1_Precache();
}

/*QUAKED NPC_Droid_Mark2 (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Small rolling droid with one gun.

*/
void SP_NPC_Droid_Mark2(gentity_t* self)
{
	self->NPC_type = "mark2";

	SP_NPC_spawner(self);

	NPC_Mark2_Precache();
}

/*QUAKED NPC_Droid_ATST (1 0 0) (-40 -40 -24) (40 40 248) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Droid_ATST(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "atst_vehicle";
	}
	else
	{
		self->NPC_type = "atst";
	}

	SP_NPC_spawner(self);

	NPC_ATST_Precache();
}

/*QUAKED NPC_Droid_Remote (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Remote Droid - the floating round droid used by Obi Wan to train Luke about the force while on the Millenium Falcon.
*/
void SP_NPC_Droid_Remote(gentity_t* self)
{
	self->NPC_type = "remote_sp";

	SP_NPC_spawner(self);

	NPC_Remote_Precache();
}

/*QUAKED NPC_Droid_Seeker (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Seeker Droid - floating round droids that shadow troopers spawn
*/
void SP_NPC_Droid_Seeker(gentity_t* self)
{
	self->NPC_type = "seeker";

	SP_NPC_spawner(self);

	NPC_Seeker_Precache();
}

/*QUAKED NPC_Droid_Sentry (1 0 0) (-24 -24 -24) (24 24 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Sentry Droid - Large, armored floating Imperial droids with 3 forward-facing gun turrets
*/
void SP_NPC_Droid_Sentry(gentity_t* self)
{
	self->NPC_type = "sentry";

	SP_NPC_spawner(self);

	NPC_Sentry_Precache();
}

/*QUAKED NPC_Droid_Gonk (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Gonk Droid - the droid that looks like a walking ice machine. Was in the Jawa land crawler, walking around talking to itself.

NOTARGET by default
*/
void SP_NPC_Droid_Gonk(gentity_t* self)
{
	self->NPC_type = "gonk";

	SP_NPC_spawner(self);

	//precache the Gonk sounds
	NPC_Gonk_Precache();
}

/*QUAKED NPC_Droid_Mouse (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

Mouse Droid - small, box shaped droid, first seen on the Death Star. Chewie yelled at it and it backed up and ran away.

NOTARGET by default
*/
void SP_NPC_Droid_Mouse(gentity_t* self)
{
	self->NPC_type = "mouse";

	SP_NPC_spawner(self);

	//precache the Mouse sounds
	NPC_Mouse_Precache();
}

/*QUAKED NPC_Droid_R2D2 (1 0 0) (-12 -12 -24) (12 12 40) IMPERIAL x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

R2D2 Droid - you probably know this one already.

NOTARGET by default
*/
void SP_NPC_Droid_R2D2(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		//imperial skin
		self->NPC_type = "r2d2_imp";
	}
	else
	{
		self->NPC_type = "r2d2";
	}

	SP_NPC_spawner(self);

	NPC_R2D2_Precache();
}

/*QUAKED NPC_Droid_R5D2 (1 0 0) (-12 -12 -24) (12 12 40) IMPERIAL ALWAYSDIE x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
ALWAYSDIE - won't go into spinning zombie AI when at low health.
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

R5D2 Droid - the droid originally chosen by Uncle Owen until it blew a bad motivator, and they took R2D2 instead.

NOTARGET by default
*/
void SP_NPC_Droid_R5D2(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		//imperial skin
		self->NPC_type = "r5d2_imp";
	}
	else
	{
		self->NPC_type = "r5d2";
	}

	SP_NPC_spawner(self);

	NPC_R5D2_Precache();
}

/*QUAKED NPC_Droid_Protocol (1 0 0) (-12 -12 -24) (12 12 40) IMPERIAL x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy

NOTARGET by default
*/
void SP_NPC_Droid_Protocol(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		//imperial skin
		self->NPC_type = "protocol_imp";
	}
	else
	{
		self->NPC_type = "protocol";
	}

	SP_NPC_spawner(self);
	NPC_Protocol_Precache();
}

//NPC console commands
/*
NPC_Spawn_f
*/

gentity_t* NPC_SpawnType(const gentity_t* ent, const char* npc_type, const char* targetname, const qboolean isVehicle)
{
	gentity_t* NPCspawner = G_Spawn();
	vec3_t forward = { 0 }, end = { 0 };
	trace_t trace = { 0 };

	if (!NPCspawner)
	{
		Com_Printf(S_COLOR_RED"NPC_Spawn Error: Out of entities!\n");
		return NULL;
	}

	NPCspawner->think = G_FreeEntity;
	NPCspawner->nextthink = level.time + FRAMETIME;

	if (!npc_type)
	{
		return NULL;
	}

	if (!npc_type[0])
	{
		Com_Printf(
			S_COLOR_RED"Error, expected one of:\n"S_COLOR_WHITE
			" NPC spawn [NPC type (from ext_data/mpnpcs)]\n NPC spawn vehicle [VEH type (from ext_data/vehicles)]\n");
		return NULL;
	}

	if (!ent || !ent->client)
	{
		//screw you, go away
		return NULL;
	}

	//rwwFIXMEFIXME: Care about who is issuing this command/other clients besides 0?
	//Spawn it at spot of first player
	//FIXME: will gib them!
	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorMA(ent->r.currentOrigin, 128, forward, end);
	trap->Trace(&trace, ent->r.currentOrigin, NULL, NULL, end, 0, MASK_SOLID, qfalse, 0, 0);
	VectorCopy(trace.endpos, end);
	end[2] -= 24;
	trap->Trace(&trace, trace.endpos, NULL, NULL, end, 0, MASK_SOLID, qfalse, 0, 0);
	VectorCopy(trace.endpos, end);
	end[2] += 24;
	G_SetOrigin(NPCspawner, end);
	VectorCopy(NPCspawner->r.currentOrigin, NPCspawner->s.origin);
	//set the yaw so that they face away from player
	NPCspawner->s.angles[1] = ent->client->ps.viewangles[1];

	trap->LinkEntity((sharedEntity_t*)NPCspawner);

	NPCspawner->NPC_type = G_NewString(npc_type);

	if (targetname)
	{
		NPCspawner->NPC_targetname = G_NewString(targetname);
	}

	NPCspawner->count = 1;

	NPCspawner->delay = 0;

	if (isVehicle)
	{
		NPCspawner->classname = "NPC_Vehicle";
	}

	//call precache funcs for James' builds
	if (!Q_stricmp("gonk", NPCspawner->NPC_type))
	{
		NPC_Gonk_Precache();
	}
	else if (!Q_stricmp("mouse", NPCspawner->NPC_type))
	{
		NPC_Mouse_Precache();
	}
	else if (!Q_strncmp("r2d2", NPCspawner->NPC_type, 4))
	{
		NPC_R2D2_Precache();
	}
	else if (!Q_stricmp("atst", NPCspawner->NPC_type))
	{
		NPC_ATST_Precache();
	}
	else if (!Q_strncmp("r5d2", NPCspawner->NPC_type, 4))
	{
		NPC_R5D2_Precache();
	}
	else if (!Q_stricmp("mark1", NPCspawner->NPC_type))
	{
		NPC_Mark1_Precache();
	}
	else if (!Q_stricmp("mark2", NPCspawner->NPC_type))
	{
		NPC_Mark2_Precache();
	}
	else if (!Q_stricmp("interrogator", NPCspawner->NPC_type))
	{
		NPC_Interrogator_Precache();
	}
	else if (!Q_stricmp("rosh_dark", NPCspawner->NPC_type))
	{
		npc_rosh_dark_precache();
	}
	else if (!Q_stricmp("probe", NPCspawner->NPC_type))
	{
		NPC_Probe_Precache();
	}
	else if (!Q_stricmp("seeker", NPCspawner->NPC_type))
	{
		NPC_Seeker_Precache();
	}
	else if (!Q_stricmp("remote", NPCspawner->NPC_type))
	{
		NPC_Remote_Precache();
	}
	else if (!Q_strncmp("shadowtrooper", NPCspawner->NPC_type, 13))
	{
		npc_shadow_trooper_precache();
	}
	else if (!Q_stricmp("minemonster", NPCspawner->NPC_type))
	{
		NPC_MineMonster_Precache();
	}
	else if (!Q_stricmp("howler", NPCspawner->NPC_type))
	{
		NPC_Howler_Precache();
	}
	else if (!Q_stricmp("sentry", NPCspawner->NPC_type))
	{
		NPC_Sentry_Precache();
	}
	else if (!Q_stricmp("protocol", NPCspawner->NPC_type))
	{
		NPC_Protocol_Precache();
	}
	else if (!Q_stricmp("galak_mech", NPCspawner->NPC_type))
	{
		NPC_GalakMech_Precache();
	}
	else if (!Q_stricmp("wampa", NPCspawner->NPC_type))
	{
		NPC_Wampa_Precache();
	}
	else
	{
		NPC_PrecacheByClassName(NPCspawner->NPC_type);
	}

	return NPC_Spawn_Do(NPCspawner);
}

static void NPC_Spawn_f(gentity_t* ent)
{
	char npc_type[1024] = { 0 };
	char targetname[1024] = { 0 };
	qboolean isVehicle = qfalse;

	trap->Argv(2, npc_type, 1024);
	if (Q_stricmp("vehicle", npc_type) == 0)
	{
		isVehicle = qtrue;
		trap->Argv(3, npc_type, 1024);
		trap->Argv(4, targetname, 1024);
	}
	else
	{
		trap->Argv(3, targetname, 1024);
	}

	NPC_SpawnType(ent, npc_type, targetname, isVehicle);
}

/*
NPC_Kill_f
*/
extern stringID_table_t TeamTable[];

static void NPC_Kill_f(void)
{
	int n = 0;
	char name[1024] = { 0 };
	npcteam_t killTeam = NPCTEAM_FREE;
	qboolean killNonSF = qfalse;

	trap->Argv(2, name, 1024);

	if (!name[0])
	{
		Com_Printf(S_COLOR_RED"Error, Expected:\n");
		Com_Printf(S_COLOR_RED"NPC kill '[NPC targetname]' - kills NPCs with certain targetname\n");
		Com_Printf(S_COLOR_RED"or\n");
		Com_Printf(S_COLOR_RED"NPC kill 'all' - kills all NPCs\n");
		Com_Printf(S_COLOR_RED"or\n");
		Com_Printf(
			S_COLOR_RED"NPC team '[teamname]' - kills all NPCs of a certain team ('nonally' is all but your allies)\n");
		return;
	}

	if (Q_stricmp("team", name) == 0)
	{
		trap->Argv(3, name, 1024);

		if (!name[0])
		{
			Com_Printf(S_COLOR_RED"NPC_Kill Error: 'npc kill team' requires a team name!\n");
			Com_Printf(S_COLOR_RED"Valid team names are:\n");
			for (n = TEAM_FREE + 1; n < TEAM_NUM_TEAMS; n++)
			{
				Com_Printf(S_COLOR_RED"%s\n", TeamNames[n]);
			}
			Com_Printf(S_COLOR_RED"nonally - kills all but your teammates\n");
			return;
		}

		if (Q_stricmp("nonally", name) == 0)
		{
			killNonSF = qtrue;
		}
		else
		{
			killTeam = GetIDForString(TeamTable, name);

			if (killTeam == NPCTEAM_FREE)
			{
				Com_Printf(S_COLOR_RED"NPC_Kill Error: team '%s' not recognized\n", name);
				Com_Printf(S_COLOR_RED"Valid team names are:\n");
				for (n = TEAM_FREE + 1; n < TEAM_NUM_TEAMS; n++)
				{
					Com_Printf(S_COLOR_RED"%s\n", TeamNames[n]);
				}
				Com_Printf(S_COLOR_RED"nonally - kills all but your teammates\n");
				return;
			}
		}
	}

	for (n = 1; n < ENTITYNUM_MAX_NORMAL; n++)
	{
		gentity_t* player = &g_entities[n];
		if (!player->inuse)
		{
			continue;
		}
		if (killNonSF)
		{
			if (player)
			{
				if (player->client)
				{
					if (player->client->playerTeam != NPCTEAM_PLAYER)
					{
						Com_Printf(S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname);
						player->health = 0;

						if (player->die && player->client)
						{
							player->die(player, player, player, player->client->pers.maxHealth, MOD_UNKNOWN);
						}
					}
				}
				else if (player->NPC_type && player->classname && player->classname[0] && Q_stricmp(
					"NPC_starfleet", player->classname) != 0)
				{
					//A spawner, remove it
					Com_Printf(S_COLOR_GREEN"Removing NPC spawner %s with NPC named %s\n", player->NPC_type,
						player->NPC_targetname);
					G_FreeEntity(player);
				}
			}
		}
		else if (player && player->NPC && player->client)
		{
			if (killTeam != NPCTEAM_FREE)
			{
				if (player->client->playerTeam == killTeam)
				{
					Com_Printf(S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname);
					player->health = 0;
					if (player->die)
					{
						player->die(player, player, player, player->client->pers.maxHealth, MOD_UNKNOWN);
					}
				}
			}
			else if (player->targetname && Q_stricmp(name, player->targetname) == 0
				|| Q_stricmp(name, "all") == 0)
			{
				Com_Printf(S_COLOR_GREEN"Killing NPC %s named %s\n", player->NPC_type, player->targetname);
				player->health = 0;
				player->client->ps.stats[STAT_HEALTH] = 0;
				if (player->die)
				{
					player->die(player, player, player, 100, MOD_UNKNOWN);
				}
			}
		}
		else if (player && player->r.svFlags & SVF_NPC_PRECACHE)
		{
			//a spawner
			if (player->targetname && Q_stricmp(name, player->targetname) == 0
				|| Q_stricmp(name, "all") == 0)
			{
				Com_Printf(S_COLOR_GREEN"Removing NPC spawner %s named %s\n", player->NPC_type, player->targetname);
				G_FreeEntity(player);
			}
		}
	}
}

static void NPC_PrintScore(const gentity_t* ent)
{
	Com_Printf("%s: %d\n", ent->targetname, ent->client->ps.persistant[PERS_SCORE]);
}

/*
Svcmd_NPC_f

parse and dispatch bot commands
*/
qboolean showBBoxes = qfalse;

void Cmd_NPC_f(gentity_t* ent)
{
	char cmd[1024] = { 0 };

	trap->Argv(1, cmd, 1024);

	if (!cmd[0])
	{
		Com_Printf("Valid NPC commands are:\n");
		Com_Printf(" spawn [NPC type (from NPCs.cfg)]\n");
		Com_Printf(" kill [NPC targetname] or [all(kills all NPCs)] or 'team [teamname]'\n");
		Com_Printf(" showbounds (draws exact bounding boxes of NPCs)\n");
		Com_Printf(" score [NPC targetname] (prints number of kills per NPC)\n");
	}
	else if (Q_stricmp(cmd, "spawn") == 0)
	{
		NPC_Spawn_f(ent);
	}
	else if (Q_stricmp(cmd, "kill") == 0)
	{
		NPC_Kill_f();
	}
	else if (Q_stricmp(cmd, "showbounds") == 0)
	{
		//Toggle on and off
		showBBoxes = showBBoxes ? qfalse : qtrue;
	}
	else if (Q_stricmp(cmd, "score") == 0)
	{
		char cmd2[1024] = { 0 };
		const gentity_t* thisent = NULL;

		trap->Argv(2, cmd2, sizeof cmd2);

		if (!cmd2[0])
		{
			//Show the score for all NPCs

			Com_Printf("SCORE LIST:\n");
			for (int i = 0; i < ENTITYNUM_WORLD; i++)
			{
				thisent = &g_entities[i];
				if (!thisent || !thisent->client)
				{
					continue;
				}
				NPC_PrintScore(thisent);
			}
		}
		else
		{
			if ((thisent = G_Find(NULL, FOFS(targetname), cmd2)) != NULL && thisent->client)
			{
				NPC_PrintScore(thisent);
			}
			else
			{
				Com_Printf("ERROR: NPC score - no such NPC %s\n", cmd2);
			}
		}
	}
}

void Cmd_AdminNPC_f(gentity_t* ent)
{
	char cmd[1024] = { 0 };

	if (!(ent->r.svFlags & SVF_ADMIN))
	{
		trap->SendServerCommand(ent - g_entities, "print \"Must login with /adminlogin (password)\n\"");
		return;
	}

	trap->Argv(1, cmd, 1024);

	if (!cmd[0])
	{
		Com_Printf("Valid NPC commands are:\n");
		Com_Printf(" spawn [NPC type (from NPCs.cfg)]\n");
		Com_Printf(" kill [NPC targetname] or [all(kills all NPCs)] or 'team [teamname]'\n");
	}
	else if (Q_stricmp(cmd, "spawn") == 0)
	{
		NPC_Spawn_f(ent);
	}
	else if (Q_stricmp(cmd, "kill") == 0)
	{
		NPC_Kill_f();
	}
}

/*QUAKED NPC_Rax(1 0 0) (-16 -16 -24) (16 16 40) FUN x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
FUN - Makes him magically the funnest thing ever in any game ever made. (actually does nothing, it'll just be fun by the power of suggestion).
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Rax(gentity_t* self)
{
	self->NPC_type = "Rax";
	SP_NPC_spawner(self);
}

//The reborn boss twins
/*QUAKED NPC_Kothos(1 0 0) (-16 -16 -24) (16 16 40) VIL x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
VIL - spawns Vil Kothos instead of Dasariah (can anyone tell them apart anyway...?)

Force only... will (eventually) be set up to re-inforce their leader (use SET_LEADER) by healing them, recharging them, keeping the player away, etc.

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Kothos(gentity_t* self)
{
	if (self->spawnflags & 1)
	{
		self->NPC_type = "VKothos";
	}
	else
	{
		self->NPC_type = "DKothos";
	}
	SP_NPC_spawner(self);
}

/*QUAKED NPC_Droid_Saber (1 0 0) (-12 -12 -24) (12 12 40) TRAINING x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
TRAINING - much weaker version 50 health vs the normal 300 health

DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Droid_Saber(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "saber_droid_training";
		}
		else
		{
			self->NPC_type = "saber_droid";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_HazardTrooper(1 0 0) (-16 -16 -24) (16 16 40) OFFICER CONCUSSION x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
250 health, repeater

OFFICER - 400 health, flechette
CONCUSSION - 400 health, concussion rifle
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_HazardTrooper(gentity_t* self)
{
	if (!self->NPC_type)
	{
		if (self->spawnflags & 1)
		{
			self->NPC_type = "hazardtrooperofficer";
		}
		else if (self->spawnflags & 2)
		{
			self->NPC_type = "hazardtrooperconcussion";
		}
		else
		{
			self->NPC_type = "hazardtrooper";
		}
	}

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Lannik_Racto(1 0 0) (-16 -16 -24) (16 16 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Lannik_Racto(gentity_t* self)
{
	self->NPC_type = "lannik_racto";

	SP_NPC_spawner(self);
}

/*QUAKED NPC_Droid_Assassin (1 0 0) (-12 -12 -24) (12 12 40) x x x x DROPTOFLOOR CINEMATIC NOTSOLID STARTINSOLID SHY
DROPTOFLOOR - NPC can be in air, but will spawn on the closest floor surface below it
CINEMATIC - Will spawn with no default AI (BS_CINEMATIC)
NOTSOLID - Starts not solid
STARTINSOLID - Don't try to fix if spawn in solid
SHY - Spawner is shy
*/
void SP_NPC_Droid_Assassin(gentity_t* self)
{
	if (!self->NPC_type)
	{
		self->NPC_type = "assassin_droid";
	}

	SP_NPC_spawner(self);
}

// The rancor that lives on mp/duel7 (2026-10-26): when the map starts a rancor NPC spawns in the pit, and
// g_rancorPitRespawn seconds after it dies a new one spawns, over and over. g_rancorPit 0 = off,
// g_rancorPitMap = the map (default mp/duel7). Called every frame from G_RunFrame.
// 2026-11-26: the snow maps get a wampa the same way: g_wampaPitMaps = the maps (default "mp/ctf2 mp/duel9",
// a list split by spaces; g_rancorPitMap may be a list too). g_rancorPit and g_rancorPitRespawn cover both.
// 2026-11-26: more animals, a few of each, at spawn points round the map (not one in the middle): howlers
// (g_howlerMaps "mp/duel6 mp/ctf3"), mine monsters (g_mineMonsterMaps "mp/ctf_bonus1 mp/ctf4 mp/ffa_bonus
// mp/ffa_bonus4 mp/ffa3") and mouse droids (g_mouseMaps "mp/ctf1 mp/ctf5 mp/ffa1 mp/ffa4"), g_howlerCount /
// g_mineMonsterCount / g_mouseCount of them (3, at most 8). Each kind is looked after on its own (pitCreatures).
static int rancorPitNextCheck = 0;

// the animals (rancor and wampa: one, in the middle of the spawn points; the others: a few, round the map)
typedef struct
{
	const char* npcType;
	int npcClass;
	const char* mapsCvar;		// maps it lives on (a list)
	const char* mapsDefault;
	const char* countCvar;		// how many (NULL = one)
	float halfWidth;			// room it needs to spawn
	float height;
	qboolean middle;			// in the middle of the spawn points (else at one of them)
} pitCreature_t;

static const pitCreature_t pitCreatures[] = {
	{ "rancor", CLASS_RANCOR, "g_rancorPitMap", "mp/duel7", NULL, 72.0f, 150.0f, qtrue },
	{ "wampa", CLASS_WAMPA, "g_wampaPitMaps", "mp/ctf2 mp/duel9", NULL, 40.0f, 110.0f, qtrue },
	{ "howler", CLASS_HOWLER, "g_howlerMaps", "mp/duel6 mp/ctf3", "g_howlerCount", 24.0f, 56.0f, qfalse },
	{ "minemonster", CLASS_MINEMONSTER, "g_mineMonsterMaps", "mp/ctf_bonus1 mp/ctf4 mp/ffa_bonus mp/ffa_bonus4 mp/ffa3",
		"g_mineMonsterCount", 24.0f, 48.0f, qfalse },
	{ "mouse", CLASS_MOUSE, "g_mouseMaps", "mp/ctf1 mp/ctf5 mp/ffa1 mp/ffa4", "g_mouseCount", 16.0f, 32.0f, qfalse },
};
#define NUM_PIT_CREATURES ((int)(sizeof pitCreatures / sizeof pitCreatures[0]))
static vmCvar_t pitMapsCvar[NUM_PIT_CREATURES];
static vmCvar_t pitCountCvar[NUM_PIT_CREATURES];
static vmCvar_t ambientMaxCvar;
static vmCvar_t ambientLifeCvar;
static qboolean pitCvarsRegistered = qfalse;
#define PIT_MAPS_STRING(t) (trap->Cvar_Update(&pitMapsCvar[t]), pitMapsCvar[t].string)
#define PIT_COUNT_INT(t) (trap->Cvar_Update(&pitCountCvar[t]), pitCountCvar[t].integer)

// is the map in list ("mp/duel7" or "mp/ctf2 mp/duel9"; maps/ and .bsp are optional both sides)
static qboolean G_RancorPitMapListed(const char* list)
{
	const char* name = level.mapname;
	const char* p = list;
	size_t nameLen = 0;

	if (!p || !p[0])
	{
		return qfalse;
	}
	if (!Q_stricmpn(name, "maps/", 5))
	{
		name += 5;
	}
	nameLen = strlen(name);
	if (nameLen > 4 && !Q_stricmp(name + nameLen - 4, ".bsp"))
	{
		nameLen -= 4;
	}
	while (*p)
	{
		const char* word = NULL;
		size_t len = 0;

		while (*p == ' ' || *p == ',' || *p == ';' || *p == '\t')
		{
			p++;
		}
		word = p;
		while (*p && *p != ' ' && *p != ',' && *p != ';' && *p != '\t')
		{
			p++;
		}
		len = (size_t)(p - word);
		if (len > 5 && !Q_stricmpn(word, "maps/", 5))
		{
			word += 5;
			len -= 5;
		}
		if (len > 4 && !Q_stricmpn(word + len - 4, ".bsp", 4))
		{
			len -= 4;
		}
		if (len && len == nameLen && !Q_stricmpn(name, word, (int)len))
		{
			return qtrue;
		}
	}
	return qfalse;
}

// 2026-11-26: map life from a data file. ext_data/sje_ambient.dat says what lives on each map: a block per map (or
// "mapA mapB" in quotes for several) with "npc" lines ("<NPC type> <how many> [middle] [calm] [respawn <s>]") and
// "civilians" / "civilianTypes" for the wandering civilians. Only when the file is missing do the cvar lists above
// (g_rancorPitMap, g_wampaPitMaps, g_howlerMaps ...) still decide. Everything spawned here is named sje_ambient:
// killing it scores nothing (MP), and "calm" ones have no side and no gun and wander, like the civilians.
// g_ambientMax caps how many of them are about at once (24).
#define AMBIENT_FILE		"ext_data/sje_ambient.dat"
#define AMBIENT_NAME		"sje_ambient"
#define MAX_AMBIENT_SLOTS	16

typedef struct
{
	char npcType[64];
	int count;				// how many (0-8)
	float halfWidth;		// room it needs to spawn
	float height;
	qboolean middle;		// one spot in the middle of the spawn points (else round one of them)
	qboolean calm;			// no side, no weapon, wanders
	int respawn;			// seconds before a dead one comes back (0 = g_rancorPitRespawn)
	int fails;				// spawns that came back with no NPC (a bad type: given up after 5)
	qboolean neutral;		// keeps its weapon, leaves everyone alone until hurt (or called by an alarm)
	qboolean spectate;		// stands round the middle and watches the fight
	qboolean scavenge;		// picks up dropped weapons and items (calm as well)
	qboolean alarm;			// when it has an enemy, calls the "neutral" ones near it
} ambientSlot_t;

static ambientSlot_t ambientSlots[MAX_AMBIENT_SLOTS];
static int numAmbientSlots = 0;
static int ambientCivilians = -1;			// civilians the map wants (-1 = it says nothing: g_civilians)
static char ambientCivilianTypes[512];	// "" = g_civilianTypes
static char ambientMapName[MAX_QPATH];
static int ambientLastTime = 0;
static int pitRespawnTime[MAX_AMBIENT_SLOTS];	// when the next missing one spawns (0 = not set)
static qboolean pitSpawnedAll[MAX_AMBIENT_SLOTS];	// all of them have been there once (respawns wait)
static qboolean pitPrecached[MAX_AMBIENT_SLOTS];
static int rancorPitCreature = 0; // the slot being looked after (index into ambientSlots)
static void G_CivilianCalm(gentity_t* civ);
static void G_AmbientRegisterCvars(void);
static qboolean G_AmbientReadFile(void);

// the room an NPC type needs (the big ones by name, else a person, or a big unknown creature in the middle)
static void G_AmbientSize(ambientSlot_t* slot)
{
	static const struct { const char* type; float half, height; } sizes[] = {
		{ "rancor", 72.0f, 150.0f }, { "mutant_rancor", 72.0f, 150.0f }, { "wampa", 40.0f, 110.0f },
		{ "howler", 24.0f, 56.0f }, { "minemonster", 24.0f, 48.0f }, { "mouse", 16.0f, 32.0f },
		{ "gonk", 16.0f, 40.0f }, { "r2d2", 16.0f, 48.0f }, { "r5d2", 16.0f, 48.0f },
		{ "probe", 24.0f, 48.0f }, { "interrogator", 16.0f, 32.0f }, { "sentry", 24.0f, 48.0f },
	};
	int i = 0;

	for (i = 0; i < (int)(sizeof sizes / sizeof sizes[0]); i++)
	{
		if (!Q_stricmp(slot->npcType, sizes[i].type))
		{
			slot->halfWidth = sizes[i].half;
			slot->height = sizes[i].height;
			return;
		}
	}
	slot->halfWidth = slot->middle && !slot->spectate ? 40.0f : 20.0f;
	slot->height = slot->middle && !slot->spectate ? 110.0f : 64.0f;
}

// an "npc" line: "<NPC type> <how many> [middle] [calm] [respawn <seconds>]"
static void G_AmbientAddSlot(const char* line)
{
	char buf[256] = { 0 };
	char* words[12] = { 0 };
	char* s = NULL;
	int n = 0, i = 0;
	ambientSlot_t* slot = NULL;

	if (numAmbientSlots >= MAX_AMBIENT_SLOTS)
	{
		Com_Printf(S_COLOR_YELLOW "%s: more than %d npc lines for %s, the rest are left out\n", AMBIENT_FILE, MAX_AMBIENT_SLOTS, level.mapname);
		return;
	}
	Q_strncpyz(buf, line, sizeof buf);
	for (s = strtok(buf, " \t,"); s && n < 12; s = strtok(NULL, " \t,"))
	{
		words[n++] = s;
	}
	if (!n)
	{
		return;
	}
	slot = &ambientSlots[numAmbientSlots];
	memset(slot, 0, sizeof *slot);
	Q_strncpyz(slot->npcType, words[0], sizeof slot->npcType);
	slot->count = 1;
	for (i = 1; i < n; i++)
	{
		if (words[i][0] >= '0' && words[i][0] <= '9')
		{
			slot->count = atoi(words[i]);
		}
		else if (!Q_stricmp(words[i], "middle"))
		{
			slot->middle = qtrue;
		}
		else if (!Q_stricmp(words[i], "calm"))
		{
			slot->calm = qtrue;
		}
		else if (!Q_stricmp(words[i], "neutral"))
		{
			slot->neutral = qtrue;
		}
		else if (!Q_stricmp(words[i], "spectate"))
		{
			slot->spectate = qtrue;
			slot->middle = qtrue; // round the middle of the map
		}
		else if (!Q_stricmp(words[i], "scavenge"))
		{
			slot->scavenge = qtrue;
			slot->calm = qtrue;
		}
		else if (!Q_stricmp(words[i], "alarm"))
		{
			slot->alarm = qtrue;
		}
		else if (!Q_stricmp(words[i], "respawn") && i + 1 < n)
		{
			slot->respawn = atoi(words[++i]);
		}
		else
		{
			Com_Printf(S_COLOR_YELLOW "%s: \"%s\" in npc \"%s\" is not known (count, middle, calm, neutral, spectate, scavenge, alarm, respawn <s>)\n", AMBIENT_FILE, words[i], line);
		}
	}
	slot->count = slot->count < 0 ? 0 : slot->count > 8 ? 8 : slot->count;
	G_AmbientSize(slot);
	numAmbientSlots++;
}

// "sound" and "effect" lines (see G_AmbientExtrasThink)
#define MAX_AMBIENT_SOUNDS			8
#define MAX_AMBIENT_EFFECTS			8
#define MAX_AMBIENT_EFFECT_SPOTS	8
typedef struct
{
	int index;
	int minMs, maxMs;
	int next;
} ambientSound_t;
typedef struct
{
	int index;
	int count;
	int minMs, maxMs;
	int numSpots;
	vec3_t spots[MAX_AMBIENT_EFFECT_SPOTS];
	int next[MAX_AMBIENT_EFFECT_SPOTS];
} ambientEffect_t;
static ambientSound_t ambientSounds[MAX_AMBIENT_SOUNDS];
static int numAmbientSounds = 0;
static ambientEffect_t ambientEffects[MAX_AMBIENT_EFFECTS];
static int numAmbientEffects = 0;
static void G_AmbientResetLife(void);
static void G_AmbientTag(const gentity_t* ent, int slot);

// splits a value into words (at most max), returns how many
static int G_AmbientWords(const char* line, char* buf, const int bufSize, char** words, const int max)
{
	char* s = NULL;
	int n = 0;

	Q_strncpyz(buf, line, bufSize);
	for (s = strtok(buf, " \t,"); s && n < max; s = strtok(NULL, " \t,"))
	{
		words[n++] = s;
	}
	return n;
}

// a "sound" line: "<sound file> [<min seconds> <max seconds>]" (30 - 90 when left out)
static void G_AmbientAddSound(const char* line)
{
	char buf[256] = { 0 };
	char* words[4] = { 0 };
	const int n = G_AmbientWords(line, buf, sizeof buf, words, 4);
	ambientSound_t* snd = NULL;

	if (!n)
	{
		return;
	}
	if (numAmbientSounds >= MAX_AMBIENT_SOUNDS)
	{
		Com_Printf(S_COLOR_YELLOW "%s: more than %d sound lines for %s, the rest are left out\n", AMBIENT_FILE, MAX_AMBIENT_SOUNDS, level.mapname);
		return;
	}
	snd = &ambientSounds[numAmbientSounds];
	memset(snd, 0, sizeof *snd);
	snd->minMs = (n > 1 ? atoi(words[1]) : 30) * 1000;
	snd->maxMs = (n > 2 ? atoi(words[2]) : 90) * 1000;
	snd->minMs = snd->minMs < 2000 ? 2000 : snd->minMs;
	snd->maxMs = snd->maxMs < snd->minMs ? snd->minMs : snd->maxMs;
	snd->index = G_SoundIndex(words[0]);
	numAmbientSounds++;
}

// an "effect" line: "<effect file> [<how many spots> [<min seconds> <max seconds>]]" (3 spots, 4 - 10 s when left out)
static void G_AmbientAddEffect(const char* line)
{
	char buf[256] = { 0 };
	char* words[5] = { 0 };
	const int n = G_AmbientWords(line, buf, sizeof buf, words, 5);
	ambientEffect_t* fx = NULL;

	if (!n)
	{
		return;
	}
	if (numAmbientEffects >= MAX_AMBIENT_EFFECTS)
	{
		Com_Printf(S_COLOR_YELLOW "%s: more than %d effect lines for %s, the rest are left out\n", AMBIENT_FILE, MAX_AMBIENT_EFFECTS, level.mapname);
		return;
	}
	fx = &ambientEffects[numAmbientEffects];
	memset(fx, 0, sizeof *fx);
	fx->count = n > 1 ? atoi(words[1]) : 3;
	fx->count = fx->count < 1 ? 1 : fx->count > MAX_AMBIENT_EFFECT_SPOTS ? MAX_AMBIENT_EFFECT_SPOTS : fx->count;
	fx->minMs = (n > 2 ? atoi(words[2]) : 4) * 1000;
	fx->maxMs = (n > 3 ? atoi(words[3]) : 10) * 1000;
	fx->minMs = fx->minMs < 500 ? 500 : fx->minMs;
	fx->maxMs = fx->maxMs < fx->minMs ? fx->minMs : fx->maxMs;
	fx->index = G_EffectIndex(words[0]);
	numAmbientEffects++;
}

#define AMB_MODEL_INDEX(m)	G_model_index(m)
// "flyby" lines: a ship model crossing the sky now and then (see G_AmbientFlybyThink)
#define MAX_AMBIENT_FLYBYS		6
#define MAX_ACTIVE_FLYBYS		4
typedef struct
{
	int model;
	int loopSound;			// engine loop on the ship
	int passSound;			// played as it goes over
	float speed;
	int minMs, maxMs;
	int next;
} ambientFlyby_t;
typedef struct
{
	int ent;				// entity number + 1
	int passTime;			// when it is over the middle (the pass sound)
	int endTime;
} ambientFlyer_t;
static ambientFlyby_t ambientFlybys[MAX_AMBIENT_FLYBYS];
static int numAmbientFlybys = 0;
static ambientFlyer_t ambientFlyers[MAX_ACTIVE_FLYBYS];
static int ambientSkyChecked = 0;	// 0 = not yet, 1 = the map has open sky, -1 = none (indoors)
static vec3_t ambientSkyCenter;
static float ambientSkyHeight;
static void G_AmbientFlybyThink(void);
static void G_AmbientFreeStrayFlyers(void);
static char ambientFlybyClass[] = "sje_ambient_flyby";

// a "flyby" line: "<ship> [<min seconds> <max seconds>]" (60 - 150 when left out). Ships: tie, xwing, falcon,
// shuttle; or your own: "<model.md3> <sound folder (with loop.wav and flyby.mp3)> [<min> <max>]"
static void G_AmbientAddFlyby(const char* line)
{
	static const struct { const char* name; const char* model; const char* sounds; float speed; } ships[] = {
		{ "tie", "models/map_objects/ships/tie_vader.md3", "sound/vehicles/tie-advanced", 2200.0f },
		{ "xwing", "models/map_objects/ships/x_wing.md3", "sound/vehicles/a-wing", 2000.0f },
		{ "falcon", "models/map_objects/ships/falcon.md3", "sound/vehicles/yt-1300", 1500.0f },
		{ "shuttle", "models/map_objects/cinematics/shuttle.md3", "sound/vehicles/Y-wing", 900.0f },
	};
	char buf[256] = { 0 };
	char* words[5] = { 0 };
	const int n = G_AmbientWords(line, buf, sizeof buf, words, 5);
	const char* model = NULL;
	const char* sounds = NULL;
	float speed = 1800.0f;
	int timeWord = 1, k = 0;
	ambientFlyby_t* fb = NULL;

	if (!n)
	{
		return;
	}
	if (numAmbientFlybys >= MAX_AMBIENT_FLYBYS)
	{
		Com_Printf(S_COLOR_YELLOW "%s: more than %d flyby lines for %s, the rest are left out\n", AMBIENT_FILE, MAX_AMBIENT_FLYBYS, level.mapname);
		return;
	}
	for (k = 0; k < (int)(sizeof ships / sizeof ships[0]); k++)
	{
		if (!Q_stricmp(words[0], ships[k].name))
		{
			model = ships[k].model;
			sounds = ships[k].sounds;
			speed = ships[k].speed;
		}
	}
	if (!model)
	{
		if (!strchr(words[0], '/') || n < 2)
		{
			Com_Printf(S_COLOR_YELLOW "%s: flyby \"%s\": ships are tie, xwing, falcon, shuttle, or \"<model.md3> <sound folder>\"\n", AMBIENT_FILE, line);
			return;
		}
		model = words[0];
		sounds = words[1];
		timeWord = 2;
	}
	fb = &ambientFlybys[numAmbientFlybys];
	memset(fb, 0, sizeof *fb);
	fb->model = AMB_MODEL_INDEX(model);
	fb->loopSound = G_SoundIndex(va("%s/loop.wav", sounds));
	fb->passSound = G_SoundIndex(va("%s/flyby.mp3", sounds));
	fb->speed = speed;
	fb->minMs = (n > timeWord ? atoi(words[timeWord]) : 60) * 1000;
	fb->maxMs = (n > timeWord + 1 ? atoi(words[timeWord + 1]) : 150) * 1000;
	fb->minMs = fb->minMs < 5000 ? 5000 : fb->minMs;
	fb->maxMs = fb->maxMs < fb->minMs ? fb->minMs : fb->maxMs;
	numAmbientFlybys++;
}

// reads the blocks for this map out of the file (qfalse = no file)
static qboolean G_AmbientParse(const char* buf)
{
	const char* p = buf;

	COM_BeginParseSession(AMBIENT_FILE);
	while (p)
	{
		char maps[256] = { 0 };
		qboolean ours = qfalse;
		const char* token = COM_ParseExt(&p, qtrue);

		if (!token[0])
		{
			break;
		}
		Q_strncpyz(maps, token, sizeof maps);
		token = COM_ParseExt(&p, qtrue);
		if (Q_stricmp(token, "{"))
		{
			Com_Printf(S_COLOR_YELLOW "%s: no { after %s\n", AMBIENT_FILE, maps);
			break;
		}
		ours = G_RancorPitMapListed(maps);
		while (1)
		{
			char key[32] = { 0 };

			token = COM_ParseExt(&p, qtrue);
			if (!token[0] || !Q_stricmp(token, "}"))
			{
				break;
			}
			Q_strncpyz(key, token, sizeof key);
			token = COM_ParseExt(&p, qfalse);
			if (!Q_stricmp(key, "npc"))
			{
				if (ours)
				{
					G_AmbientAddSlot(token);
				}
			}
			else if (!Q_stricmp(key, "sound"))
			{
				if (ours)
				{
					G_AmbientAddSound(token);
				}
			}
			else if (!Q_stricmp(key, "flyby"))
			{
				if (ours)
				{
					G_AmbientAddFlyby(token);
				}
			}
			else if (!Q_stricmp(key, "effect"))
			{
				if (ours)
				{
					G_AmbientAddEffect(token);
				}
			}
			else if (!Q_stricmp(key, "civilians"))
			{
				if (ours)
				{
					ambientCivilians = atoi(token);
				}
			}
			else if (!Q_stricmp(key, "civilianTypes"))
			{
				if (ours)
				{
					Q_strncpyz(ambientCivilianTypes, token, sizeof ambientCivilianTypes);
				}
			}
			else
			{
				Com_Printf(S_COLOR_YELLOW "%s: unknown key \"%s\" in %s (npc, sound, effect, flyby, civilians, civilianTypes)\n", AMBIENT_FILE, key, maps);
			}
		}
	}
	return qtrue;
}

// no file: the old cvar lists (rancor, wampa, howlers, mine monsters, mice)
static void G_AmbientFromCvars(void)
{
	int t = 0;

	for (t = 0; t < NUM_PIT_CREATURES && numAmbientSlots < MAX_AMBIENT_SLOTS; t++)
	{
		ambientSlot_t* slot = &ambientSlots[numAmbientSlots];

		if (!G_RancorPitMapListed(PIT_MAPS_STRING(t)))
		{
			continue;
		}
		memset(slot, 0, sizeof *slot);
		Q_strncpyz(slot->npcType, pitCreatures[t].npcType, sizeof slot->npcType);
		slot->count = 1;
		if (pitCreatures[t].countCvar)
		{
			const int c = PIT_COUNT_INT(t);
			slot->count = c < 0 ? 0 : c > 8 ? 8 : c;
		}
		slot->halfWidth = pitCreatures[t].halfWidth;
		slot->height = pitCreatures[t].height;
		slot->middle = pitCreatures[t].middle;
		numAmbientSlots++;
	}
}

// a new map (or map_restart / loaded game): read what lives here
static void G_AmbientMapCheck(void)
{
	if (level.time >= ambientLastTime && !Q_stricmp(level.mapname, ambientMapName))
	{
		ambientLastTime = level.time;
		return;
	}
	ambientLastTime = level.time;
	Q_strncpyz(ambientMapName, level.mapname, sizeof ambientMapName);
	numAmbientSlots = 0;
	ambientCivilians = -1;
	ambientCivilianTypes[0] = 0;
	memset(ambientSlots, 0, sizeof ambientSlots);
	numAmbientSounds = 0;
	numAmbientEffects = 0;
	numAmbientFlybys = 0;
	G_AmbientFreeStrayFlyers(); // map_restart / loaded game: ships nobody looks after
	ambientSkyChecked = 0;
	G_AmbientResetLife();
	memset(pitRespawnTime, 0, sizeof pitRespawnTime);
	memset(pitSpawnedAll, 0, sizeof pitSpawnedAll);
	memset(pitPrecached, 0, sizeof pitPrecached);
	rancorPitNextCheck = 0;
	G_AmbientRegisterCvars();
	if (!G_AmbientReadFile())
	{
		G_AmbientFromCvars();
	}
}

// MP: killing these scores nothing (g_combat.c)
qboolean G_IsAmbientNPC(const gentity_t* ent)
{
	return ent && ent->s.number >= MAX_CLIENTS && ent->NPC && ent->targetname
		&& (!Q_stricmp(ent->targetname, AMBIENT_NAME) || !Q_stricmp(ent->targetname, "sje_civilian")) ? qtrue : qfalse;
}

static void G_AmbientRegisterCvars(void)
{
	int t = 0;

	if (pitCvarsRegistered)
	{
		return;
	}
	for (t = 0; t < NUM_PIT_CREATURES; t++)
	{
		trap->Cvar_Register(&pitMapsCvar[t], pitCreatures[t].mapsCvar, pitCreatures[t].mapsDefault, CVAR_ARCHIVE);
		if (pitCreatures[t].countCvar)
		{
			trap->Cvar_Register(&pitCountCvar[t], pitCreatures[t].countCvar, "3", CVAR_ARCHIVE);
		}
	}
	trap->Cvar_Register(&ambientMaxCvar, "g_ambientMax", "24", CVAR_ARCHIVE);
	trap->Cvar_Register(&ambientLifeCvar, "g_ambientLife", "1", CVAR_ARCHIVE); // the setup menu's "Ambient Life"
	pitCvarsRegistered = qtrue;
}

static qboolean G_AmbientReadFile(void)
{
	static char ambient_buf[65536];
	fileHandle_t f = 0;
	const int len = trap->FS_Open(AMBIENT_FILE, &f, FS_READ);

	if (len <= 0 || len >= (int)sizeof ambient_buf)
	{
		if (f)
		{
			trap->FS_Close(f);
		}
		if (len >= (int)sizeof ambient_buf)
		{
			Com_Printf(S_COLOR_YELLOW "%s is too big (%d bytes, %d at most)\n", AMBIENT_FILE, len, (int)sizeof ambient_buf - 1);
		}
		return qfalse;
	}
	trap->FS_Read(ambient_buf, len, f);
	trap->FS_Close(f);
	ambient_buf[len] = 0;
	return G_AmbientParse(ambient_buf);
}

static const char* G_RancorPitNPCType(void)
{
	return ambientSlots[rancorPitCreature].npcType;
}

static int G_RancorPitAlive(void)
{
	int i = 0, alive = 0;

	for (i = MAX_CLIENTS; i < level.num_entities; i++)
	{
		gentity_t* ent = &g_entities[i];
		if (ent->inuse && ent->NPC && ent->client && ent->health > 0 && ent->targetname && !Q_stricmp(ent->targetname, AMBIENT_NAME)
			&& ent->NPC_type && !Q_stricmp(ent->NPC_type, ambientSlots[rancorPitCreature].npcType))
		{
			alive++;
			G_AmbientTag(ent, rancorPitCreature);
		}
	}
	return alive;
}

// Rancor-sized room (2026-10-26 fix: the first version checked a box smaller than the rancor,
// so it could spawn half inside a wall or a doorway). Solid + monster clip, not players.
#define RANCORPIT_MASK (MASK_SOLID | CONTENTS_MONSTERCLIP)
#define RANCORPIT_HALFWIDTH 72.0f
#define RANCORPIT_HEIGHT 150.0f

// drops spot onto the floor below it; qfalse when there is no floor within 512 units
static qboolean G_RancorPitFloor(vec3_t spot)
{
	vec3_t start = { 0 }, end = { 0 };
	trace_t tr = { 0 };

	VectorCopy(spot, start);
	start[2] += 32;
	VectorCopy(spot, end);
	end[2] -= 512;
	trap->Trace(&tr, start, NULL, NULL, end, ENTITYNUM_NONE, RANCORPIT_MASK, qfalse, 0, 0);
	if (tr.startsolid || tr.allsolid || tr.fraction >= 1.0f)
	{
		return qfalse;
	}
	VectorCopy(tr.endpos, spot);
	return qtrue;
}

// pos = a floor point: room for the whole rancor standing there
static qboolean G_RancorPitRoom(const vec3_t pos)
{
	const float half = ambientSlots[rancorPitCreature].halfWidth;
	const float height = ambientSlots[rancorPitCreature].height;
	vec3_t mins = { 0 }, maxs = { 0 };
	vec3_t start = { 0 };
	trace_t tr = { 0 };

	VectorSet(mins, -half, -half, 0);
	VectorSet(maxs, half, half, height);
	VectorCopy(pos, start);
	start[2] += 18; // step height, so a bumpy floor does not count as a wall
	trap->Trace(&tr, start, mins, maxs, start, ENTITYNUM_NONE, RANCORPIT_MASK, qfalse, 0, 0);
	return (qboolean)!(tr.startsolid || tr.allsolid);
}

static qboolean G_RancorPitNobodyNear(const vec3_t spot)
{
	int i = 0;

	for (i = 0; i < level.num_entities; i++)
	{
		const gentity_t* ent = &g_entities[i];
		if (ent->inuse && ent->client && ent->health > 0)
		{
			const float dx = ent->r.currentOrigin[0] - spot[0];
			const float dy = ent->r.currentOrigin[1] - spot[1];
			if (dx * dx + dy * dy < 192.0f * 192.0f && fabs(ent->r.currentOrigin[2] - spot[2]) < 160.0f)
			{
				return qfalse;
			}
		}
	}
	return qtrue;
}

// a straight, open line between the two floor points (so the spot is in the same pit, not behind a wall)
static qboolean G_RancorPitClearLine(const vec3_t a, const vec3_t b)
{
	vec3_t start = { 0 }, end = { 0 };
	trace_t tr = { 0 };

	VectorCopy(a, start);
	start[2] += 64;
	VectorCopy(b, end);
	end[2] += 64;
	trap->Trace(&tr, start, NULL, NULL, end, ENTITYNUM_NONE, RANCORPIT_MASK, qfalse, 0, 0);
	return (qboolean)(tr.fraction >= 1.0f && !tr.startsolid && !tr.allsolid);
}

// SP / MP differences for the map life code below (the SP copy has its own)
#define AMB_ORG(e)			((e)->r.currentOrigin)
#define AMB_MINS(e)			((e)->r.mins)
#define AMB_MAXS(e)			((e)->r.maxs)
#define AMB_NUM_ENTS		(level.num_entities)
#define AMB_FIRST_NPC		MAX_CLIENTS
#define AMB_TRACE(tr, s, mn, mx, en, pass, mask) trap->Trace((tr), (s), (mn), (mx), (en), (pass), (mask), qfalse, 0, 0)
#define AMB_NEUTRAL			NPCTEAM_NEUTRAL
#define AMB_SABER_ON(e)		((e)->client->ps.weapon == WP_SABER && (e)->client->ps.saberHolstered < 2)
#define AMB_HAS_ANIM(e, a)	BG_HasAnimation((e)->localAnimIndex, (a))
#define AMB_IS_PLAYER(e)	((e)->s.number < MAX_CLIENTS && (e)->client && (e)->client->pers.connected == CON_CONNECTED \
							&& (e)->client->sess.sessionTeam != TEAM_SPECTATOR)
#define AMB_LIFE_ON			(ambientLifeCvar.integer)
#define AMB_SOUND_AT(org, idx) G_SoundAtLoc((org), CHAN_AUTO, (idx))
#define AMB_EFFECT_AT(fx, org) do { vec3_t amb_up = { -90, 0, 0 }; G_PlayEffectID((fx), (org), amb_up); } while (0)
#define AMB_LAUNCH(item, org, vel) LaunchItem((item), (org), (vel))
#define AMB_LAST_ATTACKER(e)	((e)->client->lasthurt_client)
#define AMB_SVFLAGS(e)		((e)->r.svFlags)
#define AMB_LINK(e)			trap->LinkEntity((sharedEntity_t*)(e))
#define AMB_SOUND_ON(e, idx)	G_Sound((e), CHAN_AUTO, (idx))
#define AMB_EVAL_TRAJ(tr, t, out)	BG_EvaluateTrajectory((tr), (t), (out))
extern void G_SoundAtLoc(vec3_t loc, int channel, int soundIndex);
extern gentity_t* G_PlayEffectID(int fxID, vec3_t org, vec3_t ang);
extern gentity_t* LaunchItem(gitem_t* item, vec3_t origin, vec3_t velocity);
extern void NPC_SetMoveGoal(const gentity_t* ent, vec3_t point, int radius, qboolean isNavGoal, int combatPoint, gentity_t* targetEnt);
extern void NPC_SetAnim(gentity_t* ent, int setAnimParts, int anim, int setAnimFlags);

//---------------------------------------------------------------------------------------------------------------------
// Map life that does things (2026-11-26). Flags on an "npc" line in ext_data/sje_ambient.dat:
//   calm      no side, no weapon, wanders
//   neutral   keeps its weapon and walks about, leaves everyone alone until somebody hurts it (or an alarm calls it),
//             then fights back for a while
//   spectate  stands round the middle of the map and watches the fight (duel maps), now and then a gesture
//   scavenge  (calm as well) walks to dropped weapons and items and picks them up; killing it drops them again
//   alarm     when it has an enemy, every "neutral" one within 1500 joins in (a probe droid calling the troopers)
// And for all of them: the calm ones, spectators, scavengers, wandering civilians and mice run from a lit saber,
// gunfire, a big creature or an explosion (people with nowhere to run cower), rancors and wampas hunt the smaller
// animals, and "sound" / "effect" lines give the map random sounds and effects.
// g_ambientLife 0 turns all of it off and takes away what is there (the setup menu has it as "Ambient Life").
//---------------------------------------------------------------------------------------------------------------------
#define AMBIENT_FLEE_SABER		220.0f	// a lit saber this close
#define AMBIENT_FLEE_FIRE		400.0f	// somebody shooting this close
#define AMBIENT_FLEE_BEAST		420.0f	// a rancor, wampa, howler ... this close
#define AMBIENT_FLEE_DIST		400.0f	// how far they try to run
#define AMBIENT_HUNT_RANGE		700.0f
#define AMBIENT_ALARM_RANGE		1500.0f
#define AMBIENT_SCAVENGE_RANGE	900.0f
#define AMBIENT_MAX_CARRY		3
#define AMBIENT_ANGRY_TIME		20000	// ms a hurt "neutral" one stays angry

static int ambientSlotOf[MAX_GENTITIES];			// slot + 1 of an NPC spawned here (0 = not ours)
static const char* ambientTag[MAX_GENTITIES];		// its NPC_type pointer then (a reused entity has another)
static int ambientFleeUntil[MAX_GENTITIES];		// no new flee check before this
static int ambientAngryUntil[MAX_GENTITIES];
static int ambientLastHealth[MAX_GENTITIES];
static int ambientAlarmNext[MAX_GENTITIES];
static gitem_t* ambientCarry[MAX_GENTITIES][AMBIENT_MAX_CARRY];
static int ambientCarryNum[MAX_GENTITIES];
static int ambientFleeNext = 0;
static int ambientExtrasNext = 0;
static qboolean ambientEffectsPlaced = qfalse;
static qboolean ambientLifeWas = qtrue;
static vec3_t ambientSpawnPts[64];
static int numAmbientSpawnPts = -1;			// -1 = not looked up yet
static int ambientStrollNext[MAX_GENTITIES];
static int ambientStillSince[MAX_GENTITIES];
static vec3_t ambientLastPos[MAX_GENTITIES];
static int ambientTalkNext[MAX_GENTITIES];
static qboolean ambientStroll[MAX_GENTITIES];	// stands still in BS_WANDER (no nav): strolls to spots

static void G_AmbientResetLife(void)
{
	memset(ambientSlotOf, 0, sizeof ambientSlotOf);
	memset((void*)ambientTag, 0, sizeof ambientTag);
	memset(ambientFleeUntil, 0, sizeof ambientFleeUntil);
	memset(ambientAngryUntil, 0, sizeof ambientAngryUntil);
	memset(ambientLastHealth, 0, sizeof ambientLastHealth);
	memset(ambientAlarmNext, 0, sizeof ambientAlarmNext);
	memset(ambientCarry, 0, sizeof ambientCarry);
	memset(ambientCarryNum, 0, sizeof ambientCarryNum);
	ambientFleeNext = 0;
	ambientExtrasNext = 0;
	ambientEffectsPlaced = qfalse;
	numAmbientSpawnPts = -1;
	memset(ambientStrollNext, 0, sizeof ambientStrollNext);
	memset(ambientStillSince, 0, sizeof ambientStillSince);
	memset(ambientLastPos, 0, sizeof ambientLastPos);
	memset(ambientStroll, 0, sizeof ambientStroll);
	memset(ambientTalkNext, 0, sizeof ambientTalkNext);
}

static ambientSlot_t* G_AmbientSlotOf(const gentity_t* ent)
{
	const int n = ent->s.number;

	if (n < 0 || n >= MAX_GENTITIES || !ambientSlotOf[n] || ambientTag[n] != ent->NPC_type || ambientSlotOf[n] > numAmbientSlots)
	{
		return NULL;
	}
	return &ambientSlots[ambientSlotOf[n] - 1];
}

static void G_AmbientTag(const gentity_t* ent, const int slot)
{
	const int n = ent->s.number;

	if (n >= 0 && n < MAX_GENTITIES && (ambientSlotOf[n] != slot + 1 || ambientTag[n] != ent->NPC_type))
	{
		ambientSlotOf[n] = slot + 1;
		ambientTag[n] = ent->NPC_type;
		ambientAngryUntil[n] = 0;
		ambientLastHealth[n] = ent->health;
		ambientCarryNum[n] = 0;
	}
}

static qboolean G_AmbientIsBeast(const gentity_t* ent)
{
	switch (ent->client->NPC_class)
	{
	case CLASS_RANCOR:
	case CLASS_WAMPA:
	case CLASS_HOWLER:
	case CLASS_MINEMONSTER:
	case CLASS_SAND_CREATURE:
		return qtrue;
	default:
		return qfalse;
	}
}

static qboolean G_AmbientIsDroid(const gentity_t* ent)
{
	switch (ent->client->NPC_class)
	{
	case CLASS_MOUSE:
	case CLASS_GONK:
	case CLASS_R2D2:
	case CLASS_R5D2:
	case CLASS_PROTOCOL:
		return qtrue;
	default:
		return qfalse;
	}
}

// does this one run from danger (the calm ones, spectators, scavengers, civilians, little droids)
static qboolean G_AmbientFlees(const gentity_t* ent)
{
	const ambientSlot_t* slot = NULL;

	if (!G_IsAmbientNPC(ent))
	{
		return qfalse;
	}
	if (ent->targetname && !Q_stricmp(ent->targetname, "sje_civilian"))
	{
		return qtrue;
	}
	slot = G_AmbientSlotOf(ent);
	if (slot && slot->neutral)
	{
		return qfalse; // armed, fights back
	}
	if (slot && (slot->calm || slot->spectate || slot->scavenge))
	{
		return qtrue;
	}
	return G_AmbientIsDroid(ent);
}

// any danger near self? danger = where it is
static qboolean G_AmbientDanger(const gentity_t* self, vec3_t danger)
{
	const float* org = AMB_ORG(self);
	int i = 0;

	for (i = 0; i < AMB_NUM_ENTS; i++)
	{
		const gentity_t* ent = &g_entities[i];
		float range = 0.0f;

		if (ent == self || !ent->inuse || !ent->client || ent->health <= 0)
		{
			continue;
		}
		if (!ent->NPC && !AMB_IS_PLAYER(ent))
		{
			continue;
		}
		if (ent->NPC && G_AmbientFlees(ent))
		{
			continue; // another frightened one is no danger
		}
		if (G_AmbientIsBeast(ent) && ent->client->NPC_class != self->client->NPC_class)
		{
			range = AMBIENT_FLEE_BEAST;
		}
		if (AMB_SABER_ON(ent) && range < AMBIENT_FLEE_SABER)
		{
			range = AMBIENT_FLEE_SABER;
		}
		if (ent->client->ps.weaponstate == WEAPON_FIRING && ent->client->ps.weapon != WP_SABER
			&& ent->client->ps.weapon != WP_NONE && ent->client->ps.weapon != WP_MELEE && range < AMBIENT_FLEE_FIRE)
		{
			range = AMBIENT_FLEE_FIRE;
		}
		if (range <= 0.0f || fabs(AMB_ORG(ent)[2] - org[2]) > 192.0f)
		{
			continue;
		}
		if (DistanceSquared(AMB_ORG(ent), org) < range * range)
		{
			VectorCopy(AMB_ORG(ent), danger);
			return qtrue;
		}
	}
	// shots, explosions, things breaking (the NPC alert events)
	for (i = 0; i < level.numAlertEvents; i++)
	{
		const float r = level.alertEvents[i].radius < 600.0f ? level.alertEvents[i].radius : 600.0f;

		if (level.alertEvents[i].level < AEL_DANGER || level.time - level.alertEvents[i].timestamp > 600
			|| level.alertEvents[i].owner == self)
		{
			continue;
		}
		if (DistanceSquared(level.alertEvents[i].position, org) < r * r)
		{
			VectorCopy(level.alertEvents[i].position, danger);
			return qtrue;
		}
	}
	return qfalse;
}

// an open spot on the floor away from the danger (straight away first, then up to 90 degrees off it)
static qboolean G_AmbientFleeSpot(const gentity_t* self, const vec3_t danger, vec3_t out)
{
	static const float offs[7] = { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f };
	vec3_t away = { 0 }, start = { 0 }, end = { 0 }, mins = { 0 }, maxs = { 0 };
	float best = 0.0f, baseYaw = 0.0f;
	int k = 0;

	VectorSubtract(AMB_ORG(self), danger, away);
	away[2] = 0;
	if (VectorNormalize(away) < 1.0f)
	{
		away[0] = Q_flrand(-1.0f, 1.0f);
		away[1] = Q_flrand(-1.0f, 1.0f);
	}
	baseYaw = atan2f(away[1], away[0]);
	VectorCopy(AMB_MINS(self), mins);
	VectorCopy(AMB_MAXS(self), maxs);
	mins[2] += 18.0f; // steps
	if (mins[2] > maxs[2] - 1.0f)
	{
		mins[2] = maxs[2] - 1.0f;
	}
	VectorCopy(AMB_ORG(self), start);
	for (k = 0; k < 7; k++)
	{
		const float yaw = baseYaw + offs[k] * ((float)M_PI / 180.0f);
		trace_t tr;
		trace_t down;
		vec3_t spot = { 0 }, below = { 0 };
		float dist = 0.0f, score = 0.0f;

		VectorCopy(start, end);
		end[0] += cosf(yaw) * AMBIENT_FLEE_DIST;
		end[1] += sinf(yaw) * AMBIENT_FLEE_DIST;
		AMB_TRACE(&tr, start, mins, maxs, end, self->s.number, MASK_NPCSOLID);
		if (tr.allsolid)
		{
			continue;
		}
		dist = tr.fraction * AMBIENT_FLEE_DIST - 16.0f;
		if (dist < 96.0f)
		{
			continue;
		}
		VectorCopy(start, spot);
		spot[0] += cosf(yaw) * dist;
		spot[1] += sinf(yaw) * dist;
		VectorCopy(spot, below);
		below[2] -= 96.0f;
		AMB_TRACE(&down, spot, NULL, NULL, below, self->s.number, MASK_NPCSOLID);
		if (down.fraction >= 1.0f || down.allsolid)
		{
			continue; // a ledge
		}
		score = dist - k * 12.0f;
		if (score > best)
		{
			best = score;
			VectorCopy(spot, out);
		}
	}
	return best > 0.0f ? qtrue : qfalse;
}

static void G_AmbientStartFlee(gentity_t* self, const vec3_t danger)
{
	vec3_t spot = { 0 };
	const int n = self->s.number;

	if (G_AmbientFleeSpot(self, danger, spot))
	{
		NPC_SetMoveGoal(self, spot, 24, qfalse, -1, NULL);
		self->NPC->tempBehavior = BS_FLEE;
		self->NPC->scriptFlags &= ~SCF_WALKING;
		TIMER_Set(self, "flee", Q_irand(3000, 5000));
		TIMER_Set(self, "panic", 6000);
		ambientFleeUntil[n] = level.time + 1500;
		return;
	}
	// nowhere to run: people cower, droids just stay put
	if (!G_AmbientIsDroid(self) && AMB_HAS_ANIM(self, BOTH_COWER1))
	{
		NPC_SetAnim(self, SETANIM_BOTH, BOTH_COWER1, SETANIM_FLAG_OVERRIDE | SETANIM_FLAG_HOLD);
	}
	ambientFleeUntil[n] = level.time + 3000;
}

// four times a second: everything that runs from danger looks about
static void G_AmbientFleeThink(void)
{
	int i = 0;

	if (level.time < ambientFleeNext)
	{
		return;
	}
	ambientFleeNext = level.time + 250;
	for (i = AMB_FIRST_NPC; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		gentity_t* ent = &g_entities[i];
		vec3_t danger = { 0 };

		if (!ent->inuse || !ent->client || !ent->NPC || ent->health <= 0 || level.time < ambientFleeUntil[i])
		{
			continue;
		}
		if (!G_AmbientFlees(ent))
		{
			continue;
		}
		if (G_AmbientDanger(ent, danger))
		{
			G_AmbientStartFlee(ent, danger);
		}
	}
}

// a random player (or bot) to put a sound near; NULL = none
static const gentity_t* G_AmbientRandomPlayer(void)
{
	const gentity_t* pick = NULL;
	int i = 0, seen = 0;

	for (i = 0; i < MAX_CLIENTS && i < AMB_NUM_ENTS; i++)
	{
		const gentity_t* ent = &g_entities[i];
		if (ent->inuse && ent->client && ent->health > 0 && AMB_IS_PLAYER(ent) && Q_irand(0, seen++) == 0)
		{
			pick = ent;
		}
	}
	return pick;
}

// the map's spawn points (deathmatch, single player, team), at most max
static int G_AmbientSpawnPoints(vec3_t* spots, const int max)
{
	static const char* names[] = { "info_player_deathmatch", "info_player_start", "team_CTF_redplayer",
		"team_CTF_blueplayer", "team_CTF_redspawn", "team_CTF_bluespawn" };
	int num = 0, k = 0;

	for (k = 0; k < (int)(sizeof names / sizeof names[0]) && num < max; k++)
	{
		gentity_t* found = NULL;
		while ((found = G_Find(found, FOFS(classname), names[k])) != NULL && num < max)
		{
			VectorCopy(found->s.origin, spots[num]);
			num++;
		}
	}
	return num;
}

// the "effect" lines get their spots once: round the spawn points, on the floor, in the same room
static void G_AmbientPlaceEffects(void)
{
	vec3_t spawns[64];
	const int numSpawns = G_AmbientSpawnPoints(spawns, 64);
	int e = 0;

	ambientEffectsPlaced = qtrue;
	if (!numSpawns)
	{
		return;
	}
	for (e = 0; e < numAmbientEffects; e++)
	{
		ambientEffect_t* fx = &ambientEffects[e];
		int k = 0, tries = 0;

		fx->numSpots = 0;
		for (k = 0; k < fx->count && fx->numSpots < MAX_AMBIENT_EFFECT_SPOTS; k++)
		{
			for (tries = 0; tries < 8; tries++)
			{
				vec3_t from = { 0 }, spot = { 0 };
				const float yaw = Q_flrand(0.0f, 2.0f * (float)M_PI), dist = Q_flrand(64.0f, 384.0f);

				VectorCopy(spawns[Q_irand(0, numSpawns - 1)], from);
				if (!G_RancorPitFloor(from))
				{
					continue;
				}
				VectorCopy(from, spot);
				spot[0] += cosf(yaw) * dist;
				spot[1] += sinf(yaw) * dist;
				spot[2] += 48.0f;
				if (!G_RancorPitFloor(spot) || !G_RancorPitClearLine(from, spot))
				{
					continue;
				}
				spot[2] += 4.0f;
				VectorCopy(spot, fx->spots[fx->numSpots]);
				fx->next[fx->numSpots] = level.time + Q_irand(500, fx->maxMs);
				fx->numSpots++;
				break;
			}
		}
	}
}

// sounds and effects (every frame, cheap)
static void G_AmbientExtrasThink(void)
{
	int i = 0, k = 0;

	if (level.time < ambientExtrasNext)
	{
		return;
	}
	ambientExtrasNext = level.time + 200;
	G_AmbientFlybyThink();
	if (!ambientEffectsPlaced)
	{
		G_AmbientPlaceEffects();
	}
	for (i = 0; i < numAmbientSounds; i++)
	{
		ambientSound_t* snd = &ambientSounds[i];

		if (!snd->next)
		{
			snd->next = level.time + Q_irand(snd->minMs / 2, snd->maxMs / 2 + 1);
			continue;
		}
		if (level.time < snd->next)
		{
			continue;
		}
		snd->next = level.time + Q_irand(snd->minMs, snd->maxMs);
		{
			const gentity_t* near_ent = G_AmbientRandomPlayer();
			vec3_t spot = { 0 };
			const float yaw = Q_flrand(0.0f, 2.0f * (float)M_PI), dist = Q_flrand(300.0f, 700.0f);

			if (!near_ent)
			{
				continue;
			}
			VectorCopy(AMB_ORG(near_ent), spot);
			spot[0] += cosf(yaw) * dist;
			spot[1] += sinf(yaw) * dist;
			spot[2] += 64.0f;
			AMB_SOUND_AT(spot, snd->index);
		}
	}
	for (i = 0; i < numAmbientEffects; i++)
	{
		ambientEffect_t* fx = &ambientEffects[i];

		for (k = 0; k < fx->numSpots; k++)
		{
			if (level.time >= fx->next[k])
			{
				fx->next[k] = level.time + Q_irand(fx->minMs, fx->maxMs);
				AMB_EFFECT_AT(fx->index, fx->spots[k]);
			}
		}
	}
}

// "neutral": peaceful until hurt (or called), then it fights whoever it has for a while
static void G_AmbientNeutral(gentity_t* ent, const int n)
{
	if (ent->health < ambientLastHealth[n])
	{
		ambientAngryUntil[n] = level.time + AMBIENT_ANGRY_TIME; // hurt
	}
	ambientLastHealth[n] = ent->health;
	if (level.time < ambientAngryUntil[n] && (!ent->enemy || !ent->enemy->inuse || ent->enemy->health <= 0))
	{
		// go for whoever hurt it last (its side was neutral, so the NPC AI may have dropped him as an enemy)
		const int att = AMB_LAST_ATTACKER(ent);
		gentity_t* who = att >= 0 && att < ENTITYNUM_WORLD ? &g_entities[att] : NULL;

		if (who && who != ent && who->inuse && who->client && who->health > 0)
		{
			ent->client->enemyTeam = who->client->playerTeam;
			G_SetEnemy(ent, who);
		}
	}
	if (level.time < ambientAngryUntil[n] && ent->enemy && ent->enemy->inuse && ent->enemy->client && ent->enemy->health > 0)
	{
		// fighting back: the enemy's side is its enemy now, and the default AI does the fighting
		ent->client->enemyTeam = ent->enemy->client->playerTeam;
		ent->NPC->defaultBehavior = BS_DEFAULT;
		if (ent->NPC->behaviorState == BS_WANDER || ent->NPC->behaviorState == BS_CINEMATIC)
		{
			ent->NPC->behaviorState = BS_DEFAULT;
		}
		ent->NPC->scriptFlags &= ~SCF_WALKING;
		return;
	}
	ambientAngryUntil[n] = 0;
	ent->client->playerTeam = AMB_NEUTRAL;
	ent->client->enemyTeam = AMB_NEUTRAL;
	ent->enemy = NULL;
	ent->NPC->defaultBehavior = BS_WANDER;
	if (ent->NPC->behaviorState == BS_DEFAULT)
	{
		ent->NPC->behaviorState = BS_WANDER;
	}
}

// "alarm": has an enemy, so the neutral ones near it join in
static void G_AmbientAlarm(const gentity_t* ent, const int n)
{
	int i = 0;

	if (!ent->enemy || !ent->enemy->inuse || !ent->enemy->client || ent->enemy->health <= 0 || level.time < ambientAlarmNext[n])
	{
		return;
	}
	ambientAlarmNext[n] = level.time + 3000;
	for (i = AMB_FIRST_NPC; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		gentity_t* other = &g_entities[i];
		const ambientSlot_t* slot = NULL;

		if (other == ent || !other->inuse || !other->client || !other->NPC || other->health <= 0)
		{
			continue;
		}
		slot = G_AmbientSlotOf(other);
		if (!slot || !slot->neutral || other->enemy || DistanceSquared(AMB_ORG(other), AMB_ORG(ent)) > AMBIENT_ALARM_RANGE * AMBIENT_ALARM_RANGE)
		{
			continue;
		}
		other->client->enemyTeam = ent->enemy->client->playerTeam;
		G_SetEnemy(other, ent->enemy);
		ambientAngryUntil[i] = level.time + AMBIENT_ANGRY_TIME;
	}
}

// "spectate": stand and watch the nearest fight, the odd gesture
static void G_AmbientSpectate(gentity_t* ent)
{
	const gentity_t* watch = NULL;
	float best = 2000.0f * 2000.0f;
	int i = 0;

	ent->NPC->defaultBehavior = BS_DEFAULT;
	if (ent->NPC->behaviorState == BS_WANDER)
	{
		ent->NPC->behaviorState = BS_DEFAULT;
	}
	if (ent->NPC->tempBehavior == BS_FLEE)
	{
		return; // running off
	}
	for (i = 0; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		const gentity_t* other = &g_entities[i];
		float d = 0.0f;

		if (other == ent || !other->inuse || !other->client || other->health <= 0 || (!AMB_IS_PLAYER(other) && !other->NPC))
		{
			continue;
		}
		if (other->NPC && G_IsAmbientNPC(other))
		{
			continue;
		}
		d = DistanceSquared(AMB_ORG(other), AMB_ORG(ent));
		if (d < best && G_RancorPitClearLine(AMB_ORG(ent), AMB_ORG(other)))
		{
			best = d;
			watch = other;
		}
	}
	if (watch)
	{
		vec3_t dir = { 0 };
		VectorSubtract(AMB_ORG(watch), AMB_ORG(ent), dir);
		ent->NPC->desiredYaw = vectoyaw(dir);
		if (!Q_irand(0, 7))
		{
			const int anim = Q_irand(0, 1) ? BOTH_TALKGESTURE1 : BOTH_GESTURE1;
			if (AMB_HAS_ANIM(ent, anim))
			{
				NPC_SetAnim(ent, SETANIM_TORSO, anim, SETANIM_FLAG_NORMAL);
			}
		}
	}
}

// "scavenge": go to the nearest dropped weapon / item and pick it up
static void G_AmbientScavenge(gentity_t* ent, const int n)
{
	gentity_t* best = NULL;
	float bestDist = AMBIENT_SCAVENGE_RANGE * AMBIENT_SCAVENGE_RANGE;
	int i = 0;

	if (ambientCarryNum[n] >= AMBIENT_MAX_CARRY || level.time < ambientFleeUntil[n])
	{
		// full, or just ran from danger
		return;
	}
	for (i = MAX_CLIENTS; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		gentity_t* it = &g_entities[i];
		float d = 0.0f;

		if (!it->inuse || it->s.eType != ET_ITEM || !it->item || !(it->flags & FL_DROPPED_ITEM))
		{
			continue;
		}
		if (it->item->giType != IT_WEAPON && it->item->giType != IT_AMMO && it->item->giType != IT_HEALTH
			&& it->item->giType != IT_ARMOR && it->item->giType != IT_HOLDABLE)
		{
			continue; // not flags, holocrons ...
		}
		if (fabs(AMB_ORG(it)[2] - AMB_ORG(ent)[2]) > 128.0f)
		{
			continue;
		}
		d = DistanceSquared(AMB_ORG(it), AMB_ORG(ent));
		if (d < bestDist && G_RancorPitClearLine(AMB_ORG(ent), AMB_ORG(it)))
		{
			bestDist = d;
			best = it;
		}
	}
	if (!best)
	{
		return;
	}
	if (bestDist < 48.0f * 48.0f)
	{
		ambientCarry[n][ambientCarryNum[n]++] = best->item;
		G_FreeEntity(best);
		return;
	}
	// walk (scurry) over to it
	NPC_SetMoveGoal(ent, AMB_ORG(best), 16, qfalse, -1, NULL);
	ent->NPC->tempBehavior = BS_FLEE;
	TIMER_Set(ent, "flee", 2500);
	TIMER_Set(ent, "panic", 3000);
}

// a scavenger that died drops what it picked up
static void G_AmbientDropCarried(void)
{
	int n = 0, k = 0;

	for (n = 0; n < MAX_GENTITIES; n++)
	{
		gentity_t* ent = &g_entities[n];

		if (!ambientCarryNum[n])
		{
			continue;
		}
		if (n >= AMB_NUM_ENTS || !ent->inuse || ambientTag[n] != ent->NPC_type)
		{
			ambientCarryNum[n] = 0; // gone without dying (taken away)
			continue;
		}
		if (ent->health > 0)
		{
			continue;
		}
		for (k = 0; k < ambientCarryNum[n] && k < AMBIENT_MAX_CARRY; k++)
		{
			vec3_t org = { 0 }, vel = { 0 };

			VectorCopy(AMB_ORG(ent), org);
			org[2] += 16.0f;
			vel[0] = Q_flrand(-80.0f, 80.0f);
			vel[1] = Q_flrand(-80.0f, 80.0f);
			vel[2] = Q_flrand(120.0f, 200.0f);
			if (ambientCarry[n][k])
			{
				AMB_LAUNCH(ambientCarry[n][k], org, vel);
			}
			ambientCarry[n][k] = NULL;
		}
		ambientCarryNum[n] = 0;
	}
}

// rancors and wampas go after the smaller animals near them, howlers and mine monsters after mice
static void G_AmbientHunt(gentity_t* ent)
{
	const int cls = ent->client->NPC_class;
	gentity_t* prey = NULL;
	float best = AMBIENT_HUNT_RANGE * AMBIENT_HUNT_RANGE;
	int i = 0;

	if (cls != CLASS_RANCOR && cls != CLASS_WAMPA && cls != CLASS_HOWLER && cls != CLASS_MINEMONSTER)
	{
		return;
	}
	if (ent->enemy && ent->enemy->inuse && ent->enemy->health > 0)
	{
		return; // busy
	}
	for (i = AMB_FIRST_NPC; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		gentity_t* other = &g_entities[i];
		int ocls = 0;
		float d = 0.0f;

		if (other == ent || !other->inuse || !other->client || !other->NPC || other->health <= 0 || !G_IsAmbientNPC(other))
		{
			continue;
		}
		ocls = other->client->NPC_class;
		if (cls == CLASS_RANCOR || cls == CLASS_WAMPA)
		{
			if (ocls != CLASS_HOWLER && ocls != CLASS_MINEMONSTER && ocls != CLASS_MOUSE)
			{
				continue;
			}
		}
		else if (ocls != CLASS_MOUSE)
		{
			continue;
		}
		d = DistanceSquared(AMB_ORG(other), AMB_ORG(ent));
		if (d < best && G_RancorPitClearLine(AMB_ORG(ent), AMB_ORG(other)))
		{
			best = d;
			prey = other;
		}
	}
	if (prey)
	{
		G_SetEnemy(ent, prey);
	}
}

// the map's spawn points, once a map (players must never spawn into one of ours)
static qboolean G_AmbientNearSpawn(const vec3_t spot)
{
	int i = 0;

	if (numAmbientSpawnPts < 0)
	{
		numAmbientSpawnPts = G_AmbientSpawnPoints(ambientSpawnPts, 64);
	}
	for (i = 0; i < numAmbientSpawnPts; i++)
	{
		const float dx = ambientSpawnPts[i][0] - spot[0];
		const float dy = ambientSpawnPts[i][1] - spot[1];
		if (dx * dx + dy * dy < 112.0f * 112.0f && fabs(ambientSpawnPts[i][2] - spot[2]) < 128.0f)
		{
			return qtrue;
		}
	}
	return qfalse;
}

// Ships over the map (2026-11-26): every so often a ship from a "flyby" line crosses the sky in a straight line,
// from wall to wall at a good height under the sky, engine loop on it and its flyby sound as it goes over. Just
// a model moving on its own (TR_LINEAR, sent to everyone): nothing hits it, nothing thinks for it. A map with no
// open sky over its middle / spawn points gets none.
static qboolean G_AmbientFindSky(void)
{
	vec3_t spots[64];
	const int numSpots = G_AmbientSpawnPoints(spots, 64);
	int i = 0;

	ambientSkyChecked = -1;
	for (i = 0; i < numSpots && i < 16; i++)
	{
		trace_t tr;
		vec3_t start = { 0 }, end = { 0 };

		VectorCopy(spots[(i * 7) % numSpots], start);
		start[2] += 64.0f;
		VectorCopy(start, end);
		end[2] += 8192.0f;
		AMB_TRACE(&tr, start, NULL, NULL, end, ENTITYNUM_NONE, MASK_SOLID);
		if (tr.fraction < 1.0f && tr.surfaceFlags & SURF_SKY && tr.endpos[2] - start[2] > 384.0f)
		{
			VectorCopy(start, ambientSkyCenter);
			// high enough to be in the sky, under the sky brush
			ambientSkyHeight = tr.endpos[2] - 160.0f;
			if (ambientSkyHeight > start[2] + 2400.0f)
			{
				ambientSkyHeight = start[2] + 2400.0f;
			}
			ambientSkyChecked = 1;
			return qtrue;
		}
	}
	return qfalse;
}

static void G_AmbientLaunchFlyby(const ambientFlyby_t* fb)
{
	vec3_t mid = { 0 }, dir = { 0 }, start = { 0 }, end = { 0 }, angles = { 0 };
	trace_t tr;
	float back = 0.0f, ahead = 0.0f;
	const float yaw = Q_flrand(0.0f, 2.0f * (float)M_PI);
	int slot = -1, i = 0;
	gentity_t* ship = NULL;

	for (i = 0; i < MAX_ACTIVE_FLYBYS; i++)
	{
		if (!ambientFlyers[i].ent)
		{
			slot = i;
			break;
		}
	}
	if (slot < 0)
	{
		return;
	}
	VectorCopy(ambientSkyCenter, mid);
	mid[0] += Q_flrand(-512.0f, 512.0f);
	mid[1] += Q_flrand(-512.0f, 512.0f);
	mid[2] = ambientSkyHeight - Q_flrand(0.0f, 256.0f);
	dir[0] = cosf(yaw);
	dir[1] = sinf(yaw);
	// from wall to wall (or 6000 each way in open sky)
	VectorMA(mid, -6000.0f, dir, end);
	AMB_TRACE(&tr, mid, NULL, NULL, end, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.startsolid || tr.allsolid)
	{
		return;
	}
	back = tr.fraction * 6000.0f;
	VectorMA(mid, 6000.0f, dir, end);
	AMB_TRACE(&tr, mid, NULL, NULL, end, ENTITYNUM_NONE, MASK_SOLID);
	ahead = tr.fraction * 6000.0f;
	if (back + ahead < 1500.0f)
	{
		return; // too small a sky
	}
	VectorMA(mid, -(back - 64.0f), dir, start);
	VectorMA(mid, ahead - 64.0f, dir, end);

	ship = G_Spawn();
	if (!ship)
	{
		return;
	}
	ship->classname = ambientFlybyClass;
	ship->s.eType = ET_GENERAL;
	ship->s.modelindex = fb->model;
	ship->s.loopSound = fb->loopSound;
	vectoangles(dir, angles);
	VectorCopy(angles, ship->s.angles);
	VectorCopy(angles, ship->s.apos.trBase);
	ship->s.apos.trType = TR_STATIONARY;
	ship->s.pos.trType = TR_LINEAR;
	ship->s.pos.trTime = level.time;
	VectorCopy(start, ship->s.pos.trBase);
	VectorScale(dir, fb->speed, ship->s.pos.trDelta);
	VectorCopy(start, AMB_ORG(ship));
	VectorCopy(start, ship->s.origin);
	AMB_SVFLAGS(ship) |= SVF_BROADCAST; // seen and heard from everywhere, whatever its origin says
	AMB_LINK(ship);
	ambientFlyers[slot].ent = ship->s.number + 1;
	ambientFlyers[slot].passTime = level.time + (int)(back / fb->speed * 1000.0f);
	ambientFlyers[slot].endTime = level.time + (int)((back + ahead) / fb->speed * 1000.0f);
	ambientFlyers[slot].passTime = -ambientFlyers[slot].passTime; // negative = pass sound still to play
}

static void G_AmbientFreeFlyers(void)
{
	int i = 0;

	for (i = 0; i < MAX_ACTIVE_FLYBYS; i++)
	{
		if (ambientFlyers[i].ent)
		{
			gentity_t* ship = &g_entities[ambientFlyers[i].ent - 1];
			if (ship->inuse && ship->classname && !Q_stricmp(ship->classname, "sje_ambient_flyby"))
			{
				G_FreeEntity(ship);
			}
		}
	}
	memset(ambientFlyers, 0, sizeof ambientFlyers);
}

// ships left from before (a loaded game, map_restart) that nothing looks after any more
static void G_AmbientFreeStrayFlyers(void)
{
	int i = 0;

	for (i = 0; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		gentity_t* ship = &g_entities[i];
		if (ship->inuse && ship->classname && !Q_stricmp(ship->classname, "sje_ambient_flyby"))
		{
			G_FreeEntity(ship);
		}
	}
	memset(ambientFlyers, 0, sizeof ambientFlyers);
}

static void G_AmbientFlybyThink(void)
{
	int i = 0;

	if (!numAmbientFlybys)
	{
		return;
	}
	if (!ambientSkyChecked && !G_AmbientFindSky())
	{
		return;
	}
	if (ambientSkyChecked < 0)
	{
		return; // no sky here
	}
	// the ones in the air
	for (i = 0; i < MAX_ACTIVE_FLYBYS; i++)
	{
		ambientFlyer_t* fl = &ambientFlyers[i];
		gentity_t* ship = NULL;

		if (!fl->ent)
		{
			continue;
		}
		ship = &g_entities[fl->ent - 1];
		if (!ship->inuse || !ship->classname || Q_stricmp(ship->classname, "sje_ambient_flyby"))
		{
			fl->ent = 0;
			continue;
		}
		if (fl->passTime < 0 && level.time >= -fl->passTime)
		{
			int k = 0;
			fl->passTime = -fl->passTime;
			for (k = 0; k < numAmbientFlybys; k++)
			{
				if (ambientFlybys[k].model == ship->s.modelindex)
				{
					AMB_SOUND_ON(ship, ambientFlybys[k].passSound);
					break;
				}
			}
		}
		if (level.time >= fl->endTime)
		{
			G_FreeEntity(ship);
			fl->ent = 0;
		}
		else
		{
			AMB_EVAL_TRAJ(&ship->s.pos, level.time, AMB_ORG(ship));
			AMB_LINK(ship);
		}
	}
	// time for the next one of each kind
	for (i = 0; i < numAmbientFlybys; i++)
	{
		ambientFlyby_t* fb = &ambientFlybys[i];

		if (!fb->next)
		{
			fb->next = level.time + Q_irand(fb->minMs / 4, fb->minMs);
			continue;
		}
		if (level.time >= fb->next)
		{
			fb->next = level.time + Q_irand(fb->minMs, fb->maxMs);
			G_AmbientLaunchFlyby(fb);
		}
	}
}

// droids say something now and then as they go (the sounds their patrol AI plays)
static void G_AmbientDroidTalk(gentity_t* ent, const int n)
{
	const char* snd = NULL;

	if (level.time < ambientTalkNext[n])
	{
		return;
	}
	ambientTalkNext[n] = level.time + Q_irand(3000, 9000);
	switch (ent->client->NPC_class)
	{
	case CLASS_MOUSE:
		snd = va("sound/chars/mouse/misc/mousego%d.wav", Q_irand(1, 3));
		break;
	case CLASS_R2D2:
		snd = va("sound/chars/r2d2/misc/r2d2talk0%d.wav", Q_irand(1, 3));
		break;
	case CLASS_R5D2:
		snd = va("sound/chars/r5d2/misc/r5talk%d.wav", Q_irand(1, 4));
		break;
	case CLASS_GONK:
		snd = va("sound/chars/gonk/misc/gonktalk%d.wav", Q_irand(1, 2));
		break;
	default:
		return; // protocol droids talk in the NPC chatter
	}
	AMB_SOUND_ON(ent, G_SoundIndex(snd));
}

// gonks are power droids: half the time they go to a charger on the wall (or a floor unit) near them
static qboolean G_AmbientGonkCharger(const gentity_t* ent, vec3_t spot)
{
	static const char* chargers[] = { "misc_model_shield_power_converter", "misc_model_ammo_power_converter",
		"misc_model_health_power_converter", "misc_shield_floor_unit", "misc_ammo_floor_unit" };
	float best = 1500.0f * 1500.0f;
	qboolean found = qfalse;
	int k = 0;

	for (k = 0; k < (int)(sizeof chargers / sizeof chargers[0]); k++)
	{
		gentity_t* ch = NULL;
		while ((ch = G_Find(ch, FOFS(classname), chargers[k])) != NULL)
		{
			const float d = DistanceSquared(AMB_ORG(ch), AMB_ORG(ent));
			if (d > 96.0f * 96.0f && d < best && fabs(AMB_ORG(ch)[2] - AMB_ORG(ent)[2]) < 128.0f
				&& G_RancorPitClearLine(AMB_ORG(ent), AMB_ORG(ch)))
			{
				best = d;
				VectorCopy(AMB_ORG(ch), spot);
				found = qtrue;
			}
		}
	}
	return found;
}

// Ours walk about (2026-11-26): droids and creatures only move to a goal, and on a map with no NPC nav (an MP map
// in SP) people stand still in BS_WANDER. So every few seconds each one of ours that is not busy (fighting, running,
// spectating) gets an open spot nearby to go to: droids in their own AI (BS_DEFAULT), creatures in their patrol
// (looking for enemies as they go), people (once they have stood still 5 s) walking in BS_CINEMATIC.
static void G_AmbientStroll(gentity_t* ent, const ambientSlot_t* slot, const int n)
{
	const qboolean droid = G_AmbientIsDroid(ent);
	const qboolean beast = G_AmbientIsBeast(ent);
	vec3_t dir = { 0 }, from = { 0 }, spot = { 0 };

	if (slot->spectate || (ent->enemy && ent->enemy->inuse && ent->enemy->health > 0) || ent->NPC->tempBehavior == BS_FLEE
		|| (slot->neutral && level.time < ambientAngryUntil[n]))
	{
		ambientStillSince[n] = 0;
		return;
	}
	if (DistanceSquared(AMB_ORG(ent), ambientLastPos[n]) > 16.0f * 16.0f)
	{
		VectorCopy(AMB_ORG(ent), ambientLastPos[n]);
		ambientStillSince[n] = level.time;
	}
	else if (!ambientStillSince[n])
	{
		ambientStillSince[n] = level.time;
	}
	if (droid)
	{
		G_AmbientDroidTalk(ent, n);
		// the droid AI (Droid_Run) goes to its goal in BS_DEFAULT; BS_WANDER would leave it standing
		ent->NPC->defaultBehavior = BS_DEFAULT;
		if (ent->NPC->behaviorState == BS_WANDER)
		{
			ent->NPC->behaviorState = BS_DEFAULT;
		}
	}
	else if (beast)
	{
		ent->NPC->scriptFlags |= SCF_LOOK_FOR_ENEMIES; // its patrol: walks to the goal, looking for prey
	}
	else if (!slot->calm && !slot->neutral)
	{
		return; // a fighter with no flags: left to its own AI
	}
	else
	{
		if (!ambientStroll[n])
		{
			if (level.time - ambientStillSince[n] < 5000)
			{
				return; // its own wandering works here
			}
			ambientStroll[n] = qtrue;
		}
		ent->NPC->defaultBehavior = BS_CINEMATIC; // stands, and walks to its goal when it has one
		ent->NPC->behaviorState = BS_CINEMATIC;
		ent->NPC->scriptFlags |= SCF_WALKING;
	}
	// a new spot every few seconds, sooner when it got there (stood still 2 s)
	if (level.time < ambientStrollNext[n] && level.time - ambientStillSince[n] < 2000)
	{
		return;
	}
	ambientStrollNext[n] = level.time + (droid ? Q_irand(2500, 6000) : Q_irand(4000, 9000));
	if (!Q_irand(0, droid ? 6 : 4))
	{
		return; // stand a while
	}
	if (ent->client->NPC_class == CLASS_GONK && Q_irand(0, 1) && G_AmbientGonkCharger(ent, spot))
	{
		NPC_SetMoveGoal(ent, spot, 56, qfalse, -1, NULL); // off to a charger
		ambientStillSince[n] = level.time;
		ambientStrollNext[n] = level.time + Q_irand(8000, 14000); // and stays a while
		return;
	}
	dir[0] = Q_flrand(-1.0f, 1.0f);
	dir[1] = Q_flrand(-1.0f, 1.0f);
	VectorNormalize(dir);
	VectorMA(AMB_ORG(ent), -64.0f, dir, from); // "away from" a point behind it
	if (G_AmbientFleeSpot(ent, from, spot))
	{
		NPC_SetMoveGoal(ent, spot, 24, qfalse, -1, NULL);
		ambientStillSince[n] = level.time;
	}
}

// once a second: what each one of ours does
static void G_AmbientBehaveAll(void)
{
	int i = 0;

	for (i = AMB_FIRST_NPC; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
	{
		gentity_t* ent = &g_entities[i];
		const ambientSlot_t* slot = NULL;

		if (!ent->inuse || !ent->client || !ent->NPC || ent->health <= 0)
		{
			continue;
		}
		slot = G_AmbientSlotOf(ent);
		if (!slot)
		{
			continue;
		}
		if (slot->neutral)
		{
			G_AmbientNeutral(ent, i);
		}
		else if (slot->calm || slot->spectate)
		{
			G_CivilianCalm(ent);
			if (slot->spectate)
			{
				G_AmbientSpectate(ent);
			}
		}
		if (slot->scavenge)
		{
			G_AmbientScavenge(ent, i);
		}
		if (slot->alarm)
		{
			G_AmbientAlarm(ent, i);
		}
		G_AmbientHunt(ent);
		G_AmbientStroll(ent, slot, i);
	}
	G_AmbientDropCarried();
}

// a new one of ours: tag it and set it up at once, before its first think
static void G_AmbientSpawned(gentity_t* npc)
{
	const ambientSlot_t* slot = &ambientSlots[rancorPitCreature];

	G_AmbientTag(npc, rancorPitCreature);
	if (slot->neutral)
	{
		G_AmbientNeutral(npc, npc->s.number);
	}
	else if (slot->calm || slot->spectate)
	{
		G_CivilianCalm(npc);
		if (slot->spectate)
		{
			npc->NPC->defaultBehavior = BS_DEFAULT;
			npc->NPC->behaviorState = BS_DEFAULT;
		}
	}
}

// g_ambientLife 0: take away everything this file spawned (once), and do nothing more
static qboolean G_AmbientLifeCheck(void)
{
	int i = 0;

	if (AMB_LIFE_ON)
	{
		ambientLifeWas = qtrue;
		return qtrue;
	}
	if (ambientLifeWas)
	{
		ambientLifeWas = qfalse;
		for (i = AMB_FIRST_NPC; i < AMB_NUM_ENTS && i < MAX_GENTITIES; i++)
		{
			gentity_t* ent = &g_entities[i];
			if (ent->inuse && ent->NPC && ent->targetname && !Q_stricmp(ent->targetname, AMBIENT_NAME))
			{
				G_FreeEntity(ent);
			}
		}
		G_AmbientFreeFlyers();
		G_AmbientResetLife();
		memset(pitRespawnTime, 0, sizeof pitRespawnTime);
		memset(pitSpawnedAll, 0, sizeof pitSpawnedAll);
	}
	return qfalse;
}

extern wpobject_t* gWPArray[];
extern int gWPNum;
// MP: up to 32 random bot waypoints as more places to put map life
#define AMB_MORE_ANCHORS(cand, num, max) do { int amb_k; \
	for (amb_k = 0; amb_k < 32 && gWPNum > 0 && (num) < (max); amb_k++) { \
		const wpobject_t* amb_wp = gWPArray[Q_irand(0, gWPNum - 1)]; \
		if (amb_wp && amb_wp->inuse) { VectorCopy(amb_wp->origin, (cand)[(num)]); (num)++; } } } while (0)

// Where the next small one goes (2026-11-26): spread over the map and random looking, never all in one place.
// CTF: first the red flag, the blue flag and the middle between them (one each, in random order); after that,
// of the spawn points, the flags, the middle and (MP) some random bot waypoints, the spot farthest from all of
// ours already there - most of the time the farthest, now and then the second or third, so it is not laid out.
#define AMBIENT_MAX_ANCHORS		128
#define AMBIENT_ZONE_TAKEN		600.0f	// one of ours this close = that flag / middle has its one

static float G_AmbientNearestOurs(const vec3_t spot, vec3_t* ours, const int numOurs)
{
	float best = 999999.0f * 999999.0f;
	int i = 0;

	for (i = 0; i < numOurs; i++)
	{
		const float d = DistanceSquared(spot, ours[i]);
		if (d < best)
		{
			best = d;
		}
	}
	return best;
}

static void G_AmbientPickAnchor(vec3_t* spots, const int numSpots, vec3_t out)
{
	static vec3_t cand[AMBIENT_MAX_ANCHORS];
	static vec3_t ours[MAX_GENTITIES / 4];
	vec3_t flags[3] = { { 0 } };
	int numCand = 0, numOurs = 0, numFlags = 0, i = 0, best[3] = { -1, -1, -1 };
	float bestScore[3] = { -1.0f, -1.0f, -1.0f };
	gentity_t* red = G_Find(NULL, FOFS(classname), "team_CTF_redflag");
	gentity_t* blue = G_Find(NULL, FOFS(classname), "team_CTF_blueflag");

	// where ours are now (any kind)
	for (i = AMB_FIRST_NPC; i < AMB_NUM_ENTS && i < MAX_GENTITIES && numOurs < MAX_GENTITIES / 4; i++)
	{
		const gentity_t* ent = &g_entities[i];
		if (ent->inuse && ent->NPC && ent->health > 0 && ent->targetname && !Q_stricmp(ent->targetname, AMBIENT_NAME))
		{
			VectorCopy(AMB_ORG(ent), ours[numOurs]);
			numOurs++;
		}
	}

	// CTF: one at each flag and one in the middle first
	if (red && blue)
	{
		VectorCopy(red->s.origin, flags[0]);
		VectorCopy(blue->s.origin, flags[1]);
		VectorAdd(flags[0], flags[1], flags[2]);
		VectorScale(flags[2], 0.5f, flags[2]);
		numFlags = 3;
		{
			const int first = Q_irand(0, 2);
			for (i = 0; i < 3; i++)
			{
				const int k = (first + i) % 3;
				if (G_AmbientNearestOurs(flags[k], ours, numOurs) > AMBIENT_ZONE_TAKEN * AMBIENT_ZONE_TAKEN)
				{
					VectorCopy(flags[k], out);
					return;
				}
			}
		}
	}

	// the candidates
	for (i = 0; i < numFlags && numCand < AMBIENT_MAX_ANCHORS; i++)
	{
		VectorCopy(flags[i], cand[numCand]);
		numCand++;
	}
	for (i = 0; i < numSpots && numCand < AMBIENT_MAX_ANCHORS; i++)
	{
		VectorCopy(spots[i], cand[numCand]);
		numCand++;
	}
	AMB_MORE_ANCHORS(cand, numCand, AMBIENT_MAX_ANCHORS);
	if (!numCand)
	{
		VectorCopy(spots[Q_irand(0, numSpots - 1)], out);
		return;
	}
	if (!numOurs)
	{
		VectorCopy(cand[Q_irand(0, numCand - 1)], out); // the first one: anywhere
		return;
	}

	// the three farthest from ours (a little random in the scores too)
	for (i = 0; i < numCand; i++)
	{
		const float score = G_AmbientNearestOurs(cand[i], ours, numOurs) * Q_flrand(0.8f, 1.2f);
		if (score > bestScore[0])
		{
			bestScore[2] = bestScore[1]; best[2] = best[1];
			bestScore[1] = bestScore[0]; best[1] = best[0];
			bestScore[0] = score; best[0] = i;
		}
		else if (score > bestScore[1])
		{
			bestScore[2] = bestScore[1]; best[2] = best[1];
			bestScore[1] = score; best[1] = i;
		}
		else if (score > bestScore[2])
		{
			bestScore[2] = score; best[2] = i;
		}
	}
	{
		const int roll = Q_irand(0, 99);
		int pick = roll < 60 ? 0 : roll < 85 ? 1 : 2;
		while (pick > 0 && best[pick] < 0)
		{
			pick--;
		}
		VectorCopy(cand[best[pick]], out);
	}
}

static qboolean G_RancorPitSpawn(void)
{
	vec3_t spots[64] = { 0 }, center = { 0 }, spot = { 0 }, face = { 0 };
	int numSpots = 0, i = 0, ring = 0, step = 0;
	gentity_t* found = NULL;
	gentity_t* spawner = NULL;
	qboolean ok = qfalse;

	// the middle of the deathmatch spawn points is the middle of the pit
	while ((found = G_Find(found, FOFS(classname), "info_player_deathmatch")) != NULL && numSpots < 64)
	{
		VectorCopy(found->s.origin, spots[numSpots]);
		numSpots++;
	}
	if (!numSpots)
	{
		while ((found = G_Find(found, FOFS(classname), "info_player_start")) != NULL && numSpots < 64)
		{
			VectorCopy(found->s.origin, spots[numSpots]);
			numSpots++;
		}
	}
	if (!numSpots)
	{
		// a team map with no deathmatch spawns (ctf): the middle of all the team spawns is mid field
		static const char* teamSpawns[] = { "team_CTF_redplayer", "team_CTF_blueplayer", "team_CTF_redspawn", "team_CTF_bluespawn" };
		int t = 0;
		for (t = 0; t < 4; t++)
		{
			found = NULL;
			while ((found = G_Find(found, FOFS(classname), teamSpawns[t])) != NULL && numSpots < 64)
			{
				VectorCopy(found->s.origin, spots[numSpots]);
				numSpots++;
			}
		}
	}
	if (!numSpots)
	{
		return qfalse;
	}

	VectorClear(center);
	if (!ambientSlots[rancorPitCreature].middle)
	{
		// the smaller ones: spread over the map (flags / middle / spawn points / waypoints), random looking
		G_AmbientPickAnchor(spots, numSpots, center);
	}
	else
	{
		for (i = 0; i < numSpots; i++)
		{
			VectorAdd(center, spots[i], center);
		}
		VectorScale(center, 1.0f / numSpots, center);
	}
	if (!G_RancorPitFloor(center))
	{
		// no floor under the middle: start from the spawn point nearest to it
		int best = 0;
		float bestDist = -1.0f;
		for (i = 0; i < numSpots; i++)
		{
			const float d = DistanceSquared(spots[i], center);
			if (bestDist < 0.0f || d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}
		VectorCopy(spots[best], center);
		if (!G_RancorPitFloor(center))
		{
			return qfalse;
		}
	}

	// open floor nearest the middle: the middle itself, then rings further out (16 directions each),
	// with room for the whole creature, nobody close by and a clear line back to the middle
	for (ring = ambientSlots[rancorPitCreature].spectate ? 6 : ambientSlots[rancorPitCreature].middle ? 0 : 3; ring <= (ambientSlots[rancorPitCreature].spectate ? 16 : 10) && !ok; ring++)
	{
		const float radius = ring * 40.0f;
		const int dirs = ring ? 16 : 1;
		const int first = Q_irand(0, 15);
		for (step = 0; step < dirs; step++)
		{
			const float ang = (float)((first + step) % 16) * (2.0f * (float)M_PI / 16.0f);
			VectorCopy(center, spot);
			spot[0] += cosf(ang) * radius;
			spot[1] += sinf(ang) * radius;
			spot[2] += 48;
			if (!G_RancorPitFloor(spot) || fabs(spot[2] - center[2]) > 96.0f)
			{
				continue;
			}
			if (G_RancorPitRoom(spot) && G_RancorPitNobodyNear(spot) && !G_AmbientNearSpawn(spot)
				&& (!ring || G_RancorPitClearLine(center, spot)))
			{
				ok = qtrue;
				break;
			}
		}
	}

	// (2026-11-26: no longer falls back to a spawn point itself - a player spawning there landed inside the
	// animal and was thrown up; it tries again next second round another spawn point)
	if (!ok)
	{
		return qfalse; // try again next second
	}

	spot[2] += 40; // NPC origin above the floor; the spawn code settles it down
	VectorSubtract(center, spot, face);
	if (VectorLengthSquared(face) < 1.0f)
	{
		face[0] = Q_flrand(-1.0f, 1.0f);
		face[1] = Q_flrand(-1.0f, 1.0f);
	}
	face[2] = 0;

	spawner = G_Spawn();
	if (!spawner)
	{
		return qfalse;
	}
	spawner->think = G_FreeEntity;
	spawner->nextthink = level.time + FRAMETIME;
	G_SetOrigin(spawner, spot);
	VectorCopy(spawner->r.currentOrigin, spawner->s.origin);
	spawner->s.angles[YAW] = vectoyaw(face);
	spawner->NPC_type = G_NewString(G_RancorPitNPCType());
	spawner->NPC_targetname = G_NewString(AMBIENT_NAME);
	spawner->count = 1;
	spawner->delay = 0;

	{
		gentity_t* npc = NPC_Spawn_Do(spawner);
		if (!npc)
		{
			ambientSlot_t* slot = &ambientSlots[rancorPitCreature];
			if (++slot->fails == 5)
			{
				Com_Printf(S_COLOR_YELLOW "%s: NPC \"%s\" on %s would not spawn, left out\n", AMBIENT_FILE, slot->npcType, level.mapname);
			}
			return qfalse;
		}
		G_AmbientSpawned(npc);
	}
	return qtrue;
}

void G_RancorPitThink(void)
{
	int t = 0, total = 0;

	G_AmbientMapCheck();
	trap->Cvar_Update(&ambientLifeCvar);
	if (!G_AmbientLifeCheck() || level.intermissiontime)
	{
		return; // g_ambientLife 0
	}
	G_AmbientFleeThink();
	G_AmbientExtrasThink();
	if (level.time < rancorPitNextCheck)
	{
		return;
	}
	rancorPitNextCheck = level.time + 1000;
	G_AmbientBehaveAll();
	if (!g_rancorPit.integer)
	{
		return;
	}
	trap->Cvar_Update(&ambientMaxCvar);

	for (t = 0; t < numAmbientSlots; t++)
	{
		rancorPitCreature = t;
		total += G_RancorPitAlive();
	}
	for (t = 0; t < numAmbientSlots; t++)
	{
		ambientSlot_t* slot = &ambientSlots[t];
		const int want = slot->fails >= 5 ? 0 : slot->count;

		rancorPitCreature = t;
		if (!want)
		{
			continue;
		}
		if (!pitPrecached[t])
		{
			NPC_PrecacheByClassName(G_RancorPitNPCType());
			pitPrecached[t] = qtrue;
		}
		if (G_RancorPitAlive() >= want)
		{
			pitSpawnedAll[t] = qtrue;
			pitRespawnTime[t] = 0;
			continue;
		}
		if (total >= ambientMaxCvar.integer)
		{
			continue; // g_ambientMax about already
		}
		if (!pitRespawnTime[t])
		{
			pitRespawnTime[t] = level.time + (pitSpawnedAll[t] ? (slot->respawn > 0 ? slot->respawn : g_rancorPitRespawn.integer) * 1000 : 2000);
		}
		if (level.time >= pitRespawnTime[t] && G_RancorPitSpawn())
		{
			pitRespawnTime[t] = 0; // the next missing one gets its own time
			total++;
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
// Wandering civilians (2026-11-16, DH; after Jedi Knight Galaxies' civilian spawner, g_bot.cpp G_CheckCivilianNPCs):
// g_civilians N keeps N civilian NPCs (types from g_civilianTypes) around the players. They appear out of sight
// 500-1400 units away, walk the map's NPC waypoints (BS_WANDER; on a map without them they stand about), never pick a
// fight, and the NPC chatter makes them talk to each other. One that drifts over 3000 units from every player while
// out of sight is taken away again. 0 = off (default): campaign maps keep their own NPCs only.
//---------------------------------------------------------------------------------------------------------------------
#define MAX_CIVILIANS		32
#define CIVILIAN_NEAR		500.0f
#define CIVILIAN_FAR		1400.0f
#define CIVILIAN_DESPAWN	3000.0f

static int civilianEnts[MAX_CIVILIANS]; // entity number + 1
static int civilianNextSpawn;
static unsigned civilianBadTypes; // types that failed to spawn (bit per position in g_civilianTypes)

// can any player see this spot (or is one closer than near)
static qboolean G_CivilianSeen(const vec3_t pos, const float near, float* closest)
{
	int p = 0;
	trace_t tr = { 0 };

	*closest = 999999.0f;
	for (p = 0; p < level.maxclients; p++)
	{
		const gentity_t* pl = &g_entities[p];
		if (!pl->inuse || !pl->client || pl->client->pers.connected != CON_CONNECTED || pl->client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			continue;
		}
		{
			vec3_t eye = { 0 }, top = { 0 };
			const float d = Distance(pl->r.currentOrigin, pos);

			if (d < *closest)
			{
				*closest = d;
			}
			if (d < near)
			{
				return qtrue;
			}
			VectorCopy(pl->r.currentOrigin, eye);
			eye[2] += pl->client->ps.viewheight;
			VectorCopy(pos, top);
			top[2] += 32.0f;
			trap->Trace(&tr, eye, NULL, NULL, top, pl->s.number, MASK_OPAQUE, qfalse, 0, 0);
			if (tr.fraction >= 1.0f)
			{
				return qtrue;
			}
		}
	}
	return qfalse;
}

// a free spot on the ground out of sight near a random player, qfalse = none found this time
static qboolean G_CivilianSpot(vec3_t out)
{
	int tries = 0, p = 0, count = 0, pick = 0;
	const gentity_t* around = NULL;
	vec3_t mins = { -16, -16, -24 }, maxs = { 16, 16, 40 };

	for (p = 0; p < level.maxclients; p++)
	{
		const gentity_t* pl = &g_entities[p];
		if (!pl->inuse || !pl->client || pl->client->pers.connected != CON_CONNECTED || pl->client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			continue;
		}
		count++;
	}
	if (!count)
	{
		return qfalse;
	}
	pick = Q_irand(1, count);
	for (p = 0; p < level.maxclients; p++)
	{
		const gentity_t* pl = &g_entities[p];
		if (!pl->inuse || !pl->client || pl->client->pers.connected != CON_CONNECTED || pl->client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			continue;
		}
		if (--pick == 0)
		{
			around = pl;
			break;
		}
	}
	if (!around)
	{
		return qfalse;
	}
	for (tries = 0; tries < 8; tries++)
	{
		trace_t tr = { 0 };
		vec3_t start = { 0 }, end = { 0 };
		float closest = 0;
		const float yaw = Q_flrand(0.0f, 2.0f * M_PI), dist = Q_flrand(CIVILIAN_NEAR, CIVILIAN_FAR);

		VectorCopy(around->r.currentOrigin, start);
		start[0] += cosf(yaw) * dist;
		start[1] += sinf(yaw) * dist;
		start[2] += 64.0f;
		VectorCopy(start, end);
		end[2] -= 512.0f;
		trap->Trace(&tr, start, mins, maxs, end, ENTITYNUM_NONE, MASK_NPCSOLID, qfalse, 0, 0);
		if (tr.startsolid || tr.allsolid || tr.fraction >= 1.0f || tr.plane.normal[2] < 0.7f)
		{
			continue;
		}
		if (G_CivilianSeen(tr.endpos, CIVILIAN_NEAR, &closest))
		{
			continue;
		}
		VectorCopy(tr.endpos, out);
		return qtrue;
	}
	return qfalse;
}

// a civilian: no side, no gun, wanders (the NPC file may have given it a team and a weapon)
static void G_CivilianCalm(gentity_t* civ)
{
	if (!civ->client || !civ->NPC)
	{
		return;
	}
	civ->client->playerTeam = NPCTEAM_NEUTRAL;
	civ->client->enemyTeam = NPCTEAM_NEUTRAL;
	civ->enemy = NULL;
	if (civ->client->ps.weapon != WP_NONE)
	{
		civ->client->ps.weapon = WP_NONE;
		civ->client->ps.stats[STAT_WEAPONS] = 0;
		civ->s.weapon = WP_NONE;
	}
	civ->NPC->defaultBehavior = BS_WANDER;
	if (civ->NPC->behaviorState == BS_DEFAULT)
	{
		civ->NPC->behaviorState = BS_WANDER;
	}
}

static void G_CivilianSpawn(void)
{
	char list[512] = { 0 };
	char* types[32] = { 0 };
	int num_types = 0, i = 0, pick = 0;
	char* s = NULL;
	gentity_t* spawner = NULL;
	gentity_t* civ = NULL;
	vec3_t pos = { 0 };

	Q_strncpyz(list, ambientCivilianTypes[0] ? ambientCivilianTypes : g_civilianTypes.string, sizeof list);
	for (s = strtok(list, " ,;"); s && num_types < 32; s = strtok(NULL, " ,;"))
	{
		types[num_types++] = s;
	}
	if (!num_types || !G_CivilianSpot(pos))
	{
		return;
	}
	pick = Q_irand(0, num_types - 1);
	for (i = 0; i < num_types && civilianBadTypes & 1u << pick; i++)
	{
		pick = (pick + 1) % num_types;
	}
	if (civilianBadTypes & 1u << pick)
	{
		return; // none of them spawns
	}

	spawner = G_Spawn();
	if (!spawner)
	{
		return;
	}
	spawner->think = G_FreeEntity;
	spawner->nextthink = level.time + 100;
	G_SetOrigin(spawner, pos);
	VectorCopy(pos, spawner->s.origin);
	spawner->s.angles[YAW] = Q_flrand(0.0f, 360.0f);
	trap->LinkEntity((sharedEntity_t*)spawner);
	spawner->NPC_type = G_NewString(types[pick]);
	Q_strlwr(spawner->NPC_type);
	spawner->NPC_targetname = G_NewString("sje_civilian");
	spawner->count = 1;
	spawner->delay = 0;
	civ = NPC_Spawn_Do(spawner);
	if (!civ)
	{
		civilianBadTypes |= 1u << pick;
		return;
	}
	G_CivilianCalm(civ); // at once, before its first think
	for (i = 0; i < MAX_CIVILIANS; i++)
	{
		if (!civilianEnts[i])
		{
			civilianEnts[i] = civ->s.number + 1;
			break;
		}
	}
}

// every server frame: keep g_civilians civilians about, make them peaceful wanderers, take far ones away
void G_CivilianThink(void)
{
	int want = 0, i = 0, have = 0;

	G_AmbientMapCheck(); // the map may say how many (ext_data/sje_ambient.dat)
	want = ambientCivilians >= 0 && AMB_LIFE_ON ? ambientCivilians : g_civilians.integer;
	want = want < 0 ? 0 : want > MAX_CIVILIANS ? MAX_CIVILIANS : want;

	if (level.time < civilianNextSpawn)
	{
		return;
	}
	civilianNextSpawn = level.time + 1000;

	for (i = 0; i < MAX_CIVILIANS; i++)
	{
		gentity_t* civ = NULL;
		float closest = 0;

		if (!civilianEnts[i])
		{
			continue;
		}
		civ = &g_entities[civilianEnts[i] - 1];
		if (!civ->inuse || !civ->client || !civ->NPC || civ->health <= 0 || !civ->targetname
			|| Q_stricmp(civ->targetname, "sje_civilian"))
		{
			civilianEnts[i] = 0; // gone (killed, removed)
			continue;
		}
		// over the limit, or wandered far off, and nobody sees it: take it away
		if (!G_CivilianSeen(civ->r.currentOrigin, CIVILIAN_NEAR, &closest) && (have >= want || closest > CIVILIAN_DESPAWN))
		{
			civilianEnts[i] = 0;
			G_FreeEntity(civ);
			continue;
		}
		have++;
		G_CivilianCalm(civ);
	}
	if (have < want)
	{
		G_CivilianSpawn();
	}
}

void G_CivilianReset(void)
{
	memset(civilianEnts, 0, sizeof civilianEnts);
	civilianNextSpawn = 0;
	civilianBadTypes = 0;
}
