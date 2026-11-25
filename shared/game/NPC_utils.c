/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
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

// NPC_utils for both games. SP's version is built on C++ classes that the MP game rewrote in C, so this file
// still holds both complete versions; was code/game/NPC_utils.cpp and codemp/game/NPC_utils.c.

#include "game/g_npcshared.h"

#ifdef SP_GAME

//NPC_utils.cpp

#include "game/b_local.h"
#include "game/Q3_Interface.h"
#include "game/g_navigator.h"
#include "cgame/cg_local.h"
#include "game/g_nav.h"
#include "game/g_vehicles.h"
#include "game/teams.h"
#include "game/bg_public.h"
#include <qcommon/q_math.h>
#include <qcommon/q_platform.h>
#include <qcommon/q_shared.h>
#include "game/b_public.h"
#include <cmath>
#include "game/bstate.h"
#include "game/ghoul2_shared.h"
#include "game/g_local.h"
#include "game/g_public.h"
#include "game/g_shared.h"
#include "game/weapons.h"
#include <icarus/IcarusInterface.h>
#include <rd-common/mdx_format.h>
#include <qcommon/q_string.h>

extern Vehicle_t* G_IsRidingVehicle(const gentity_t* pEnt);

int teamNumbers[TEAM_NUM_TEAMS];
int teamStrength[TEAM_NUM_TEAMS];
int teamCounter[TEAM_NUM_TEAMS];

constexpr auto VALID_ATTACK_CONE = 2.0f;	//Degrees
void GetAnglesForDirection(const vec3_t p1, const vec3_t p2, vec3_t out);
extern void WP_DeactivateSaber(const gentity_t* self, qboolean clear_length = qfalse);
extern void G_AddVoiceEvent(const gentity_t* self, int event, int speak_debounce_time);
extern qboolean InFront(vec3_t spot, vec3_t from, vec3_t fromAngles, float threshHold = 0.0f);
extern qboolean PM_SaberInAttack(int move);
extern qboolean PM_SaberInStart(int move);
extern qboolean PM_SaberInSpecialAttack(int anim);
extern qboolean PM_SpinningSaberAnim(int anim);
extern void ViewHeightFix(const gentity_t* ent);
extern void AddLeanOfs(const gentity_t* ent, vec3_t point);
extern void subtract_lean_ofs(const gentity_t* ent, vec3_t point);

void CalcEntitySpot(const gentity_t* ent, const spot_t spot, vec3_t point)
{
	vec3_t forward = { 0 }, up = { 0 }, right = { 0 };
	vec3_t start = { 0 }, end = { 0 };
	trace_t tr = { qfalse };

	if (!ent)
	{
		return;
	}
	ViewHeightFix(ent);
	switch (spot)
	{
	case SPOT_ORIGIN:
		if (VectorCompare(ent->currentOrigin, vec3_origin))
		{
			//brush
			VectorSubtract(ent->absmax, ent->absmin, point); //size
			VectorMA(ent->absmin, 0.5, point, point);
		}
		else
		{
			VectorCopy(ent->currentOrigin, point);
		}
		break;

	case SPOT_CHEST:
	case SPOT_HEAD:
		if (ent->client && VectorLengthSquared(ent->client->renderInfo.eyePoint) && (ent->client->ps.viewEntity <= 0 ||
			ent->client->ps.viewEntity >= ENTITYNUM_WORLD))
		{
			//Actual tag_head eyespot!
			//FIXME: Stasis aliens may have a problem here...
			VectorCopy(ent->client->renderInfo.eyePoint, point);
			if (ent->client->NPC_class == CLASS_ATST)
			{
				//adjust up some
				point[2] += 28; //magic number :)
			}
			if (ent->NPC)
			{
				//always aim from the center of my bbox, so we don't wiggle when we lean forward or backwards
				point[0] = ent->currentOrigin[0];
				point[1] = ent->currentOrigin[1];
			}
			else if (!ent->s.number)
			{
				subtract_lean_ofs(ent, point);
			}
		}
		else
		{
			VectorCopy(ent->currentOrigin, point);
			if (ent->client)
			{
				point[2] += ent->client->ps.viewheight;
			}
		}
		if (spot == SPOT_CHEST && ent->client)
		{
			if (ent->client->NPC_class != CLASS_ATST)
			{
				//adjust up some
				point[2] -= ent->maxs[2] * 0.2f;
			}
		}
		break;

	case SPOT_HEAD_LEAN:
		if (ent->client && VectorLengthSquared(ent->client->renderInfo.eyePoint) && (ent->client->ps.viewEntity <= 0 ||
			ent->client->ps.viewEntity >= ENTITYNUM_WORLD))
		{
			//Actual tag_head eyespot!
			//FIXME: Stasis aliens may have a problem here...
			VectorCopy(ent->client->renderInfo.eyePoint, point);
			if (ent->client->NPC_class == CLASS_ATST)
			{
				//adjust up some
				point[2] += 28; //magic number :)
			}
			if (ent->NPC)
			{
				//always aim from the center of my bbox, so we don't wiggle when we lean forward or backwards
				point[0] = ent->currentOrigin[0];
				point[1] = ent->currentOrigin[1];
			}
			else if (!ent->s.number)
			{
				subtract_lean_ofs(ent, point);
			}
			//NOTE: automatically takes leaning into account!
		}
		else
		{
			VectorCopy(ent->currentOrigin, point);
			if (ent->client)
			{
				point[2] += ent->client->ps.viewheight;
			}
			//AddLeanOfs ( ent, point );
		}
		break;

	case SPOT_LEGS:
		VectorCopy(ent->currentOrigin, point);
		point[2] += ent->mins[2] * 0.5;
		break;

	case SPOT_WEAPON:
		if (ent->NPC && !VectorCompare(ent->NPC->shootAngles, vec3_origin) && !VectorCompare(
			ent->NPC->shootAngles, ent->client->ps.viewangles))
		{
			AngleVectors(ent->NPC->shootAngles, forward, right, up);
		}
		else
		{
			AngleVectors(ent->client->ps.viewangles, forward, right, up);
		}
		CalcMuzzlePoint(const_cast<gentity_t*>(ent), forward, point, 0);
		//NOTE: automatically takes leaning into account!
		break;

	case SPOT_GROUND:
		// if entity is on the ground, just use it's absmin
		if (ent->s.groundEntityNum != -1)
		{
			VectorCopy(ent->currentOrigin, point);
			point[2] = ent->absmin[2];
			break;
		}

		// if it is reasonably close to the ground, give the point underneath of it
		VectorCopy(ent->currentOrigin, start);
		start[2] = ent->absmin[2];
		VectorCopy(start, end);
		end[2] -= 64;
		gi.trace(&tr, start, ent->mins, ent->maxs, end, ent->s.number, MASK_PLAYERSOLID, static_cast<EG2_Collision>(0),
			0);
		if (tr.fraction < 1.0)
		{
			VectorCopy(tr.endpos, point);
			break;
		}

		// otherwise just use the origin
		VectorCopy(ent->currentOrigin, point);
		break;

	default:
		VectorCopy(ent->currentOrigin, point);
		break;
	}
}

//===================================================================================

/*
qboolean NPC_UpdateAngles ( qboolean doPitch, qboolean doYaw )

Added: option to do just pitch or just yaw

Does not include "aim" in it's calculations

FIXME: stop compressing angles into shorts!!!!
*/
extern cvar_t* g_timescale;
extern bool NPC_IsTrooper(const gentity_t* ent);

qboolean NPC_UpdateAngles(qboolean doPitch, qboolean doYaw)
{
#if 1

	float error = 0.0f;
	float decay = 0.0f;
	float targetPitch = 0;
	float targetYaw = 0;
	float yawSpeed = 0.0f;
	qboolean exact = qtrue;

	// if angle changes are locked; just keep the current angles
	// aimTime isn't even set anymore... so this code was never reached, but I need a way to lock NPC's yaw, so instead of making a new SCF_ flag, just use the existing render flag... - dmv
	if (!NPC->enemy && (level.time < NPCInfo->aimTime || NPC->client->renderInfo.renderFlags & RF_LOCKEDANGLE))
	{
		if (doPitch)
			targetPitch = NPCInfo->lockedDesiredPitch;

		if (doYaw)
			targetYaw = NPCInfo->lockedDesiredYaw;
	}
	else
	{
		// we're changing the lockedDesired Pitch/Yaw below so it's lost it's original meaning, get rid of the lock flag
		NPC->client->renderInfo.renderFlags &= ~RF_LOCKEDANGLE;

		if (doPitch)
		{
			targetPitch = NPCInfo->desiredPitch;
			NPCInfo->lockedDesiredPitch = NPCInfo->desiredPitch;
		}

		if (doYaw)
		{
			targetYaw = NPCInfo->desiredYaw;
			NPCInfo->lockedDesiredYaw = NPCInfo->desiredYaw;
		}
	}

	if (NPC->s.weapon == WP_EMPLACED_GUN)
	{
		// FIXME: this seems to do nothing, actually...
		yawSpeed = 20;
	}
	else
	{
		if (NPC->client->NPC_class == CLASS_ROCKETTROOPER
			&& !NPC->enemy)
		{
			//just slowly lookin' around
			yawSpeed = 1;
		}
		else
		{
			yawSpeed = NPCInfo->stats.yawSpeed;
		}
	}

	if (NPC->s.weapon == WP_SABER && NPC->client->ps.forcePowersActive & 1 << FP_SPEED)
	{
		yawSpeed *= 1.0f / g_timescale->value;
	}

	if (!NPC_IsTrooper(NPC)
		&& NPC->enemy
		&& !G_IsRidingVehicle(NPC)
		&& NPC->client->NPC_class != CLASS_VEHICLE)
	{
		if (NPC->s.weapon == WP_BLASTER_PISTOL ||
			NPC->s.weapon == WP_BLASTER ||
			NPC->s.weapon == WP_BOWCASTER ||
			NPC->s.weapon == WP_REPEATER ||
			NPC->s.weapon == WP_FLECHETTE ||
			NPC->s.weapon == WP_BRYAR_PISTOL ||
			NPC->s.weapon == WP_SBD_BLASTER ||
			NPC->s.weapon == WP_NOGHRI_STICK ||
			NPC->s.weapon == WP_BATTLEDROID ||
			NPC->s.weapon == WP_THEFIRSTORDER ||
			NPC->s.weapon == WP_CLONECARBINE ||
			NPC->s.weapon == WP_REBELBLASTER ||
			NPC->s.weapon == WP_CLONERIFLE ||
			NPC->s.weapon == WP_CLONECOMMANDO ||
			NPC->s.weapon == WP_Z6_ROTARY_CANNON ||
			NPC->s.weapon == WP_WRIST_BLASTER ||
			NPC->s.weapon == WP_REBELRIFLE ||
			NPC->s.weapon == WP_REY ||
			NPC->s.weapon == WP_JANGO ||
			NPC->s.weapon == WP_DUAL_PISTOL ||
			NPC->s.weapon == WP_DUAL_CLONEPISTOL ||
			NPC->s.weapon == WP_DROIDEKA ||
			NPC->s.weapon == WP_BOBA ||
			NPC->s.weapon == WP_CLONEPISTOL ||
			NPC->s.weapon == WP_JAWA)
		{
			yawSpeed *= 10.0f;
		}
	}

	if (doYaw)
	{
		// decay yaw error
		error = AngleDelta(NPC->client->ps.viewangles[YAW], targetYaw);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0f / 1000.0f; //msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}

		ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + error) - client->ps.delta_angles[YAW];
	}

	//FIXME: have a pitchSpeed?
	if (doPitch)
	{
		// decay pitch error
		error = AngleDelta(NPC->client->ps.viewangles[PITCH], targetPitch);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0f / 1000.0f; //msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}

		ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + error) - client->ps.delta_angles[PITCH];
	}

	ucmd.angles[ROLL] = ANGLE2SHORT(NPC->client->ps.viewangles[ROLL]) - client->ps.delta_angles[ROLL];

	if (exact && Q3_TaskIDPending(NPC, TID_ANGLE_FACE))
	{
		Q3_TaskIDComplete(NPC, TID_ANGLE_FACE);
	}
	return exact;

#else

	float		error;
	float		decay;
	float		targetPitch = 0;
	float		targetYaw = 0;
	float		yawSpeed;
	//float		runningMod = NPCInfo->currentSpeed/100.0f;
	qboolean	exact = qtrue;
	qboolean	doSound = qfalse;

	// if angle changes are locked; just keep the current angles
	if (level.time < NPCInfo->aimTime)
	{
		if (doPitch)
			targetPitch = NPCInfo->lockedDesiredPitch;
		if (doYaw)
			targetYaw = NPCInfo->lockedDesiredYaw;
	}
	else
	{
		if (doPitch)
			targetPitch = NPCInfo->desiredPitch;
		if (doYaw)
			targetYaw = NPCInfo->desiredYaw;

		//		NPCInfo->aimTime = level.time + 250;
		if (doPitch)
			NPCInfo->lockedDesiredPitch = NPCInfo->desiredPitch;
		if (doYaw)
			NPCInfo->lockedDesiredYaw = NPCInfo->desiredYaw;
	}

	yawSpeed = NPCInfo->stats.yawSpeed;

	if (doYaw)
	{
		// decay yaw error
		error = AngleDelta(NPC->client->ps.viewangles[YAW], targetYaw);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0 / 1000.0;//msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}
		ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + error) - client->ps.delta_angles[YAW];
	}

	//FIXME: have a pitchSpeed?
	if (doPitch)
	{
		// decay pitch error
		error = AngleDelta(NPC->client->ps.viewangles[PITCH], targetPitch);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0 / 1000.0;//msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}
		ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + error) - client->ps.delta_angles[PITCH];
	}

	ucmd.angles[ROLL] = ANGLE2SHORT(NPC->client->ps.viewangles[ROLL]) - client->ps.delta_angles[ROLL];

	return exact;

#endif
}

void NPC_AimWiggle(vec3_t enemy_org)
{
	//shoot for somewhere between the head and torso
	//NOTE: yes, I know this looks weird, but it works
	if (NPCInfo->aimErrorDebounceTime < level.time)
	{
		NPCInfo->aimOfs[0] = 0.3 * Q_flrand(NPC->enemy->mins[0], NPC->enemy->maxs[0]);
		NPCInfo->aimOfs[1] = 0.3 * Q_flrand(NPC->enemy->mins[1], NPC->enemy->maxs[1]);
		if (NPC->enemy->maxs[2] > 0)
		{
			NPCInfo->aimOfs[2] = NPC->enemy->maxs[2] * Q_flrand(0.0f, -1.0f);
		}
	}
	VectorAdd(enemy_org, NPCInfo->aimOfs, enemy_org);
}

/*
qboolean NPC_UpdateFiringAngles ( qboolean doPitch, qboolean doYaw )

  Includes aim when determining angles - so they don't always hit...
  */
qboolean NPC_UpdateFiringAngles(qboolean doPitch, qboolean doYaw)
{
#if 0

	float		diff;
	float		error;
	float		targetPitch = 0;
	float		targetYaw = 0;
	qboolean	exact = qtrue;

	if (level.time < NPCInfo->aimTime)
	{
		if (doPitch)
			targetPitch = NPCInfo->lockedDesiredPitch;

		if (doYaw)
			targetYaw = NPCInfo->lockedDesiredYaw;
	}
	else
	{
		if (doPitch)
		{
			targetPitch = NPCInfo->desiredPitch;
			NPCInfo->lockedDesiredPitch = NPCInfo->desiredPitch;
		}

		if (doYaw)
		{
			targetYaw = NPCInfo->desiredYaw;
			NPCInfo->lockedDesiredYaw = NPCInfo->desiredYaw;
		}
	}

	if (doYaw)
	{
		// add yaw error based on NPCInfo->aim value
		error = ((float)(6 - NPCInfo->stats.aim)) * Q_flrand(-1, 1);

		if (Q_irand(0, 1))
			error *= -1;

		diff = AngleDelta(NPC->client->ps.viewangles[YAW], targetYaw);

		if (diff)
			exact = qfalse;

		ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + diff + error) - client->ps.delta_angles[YAW];
	}

	if (doPitch)
	{
		// add pitch error based on NPCInfo->aim value
		error = ((float)(6 - NPCInfo->stats.aim)) * Q_flrand(-1, 1);

		diff = AngleDelta(NPC->client->ps.viewangles[PITCH], targetPitch);

		if (diff)
			exact = qfalse;

		ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + diff + error) - client->ps.delta_angles[PITCH];
	}

	ucmd.angles[ROLL] = ANGLE2SHORT(NPC->client->ps.viewangles[ROLL]) - client->ps.delta_angles[ROLL];

	return exact;

#else

	float error = 0.0f, diff = 0.0f;
	float decay = 0.0f;
	float targetPitch = 0;
	float targetYaw = 0;
	qboolean exact = qtrue;

	// if angle changes are locked; just keep the current angles
	if (level.time < NPCInfo->aimTime)
	{
		if (doPitch)
			targetPitch = NPCInfo->lockedDesiredPitch;
		if (doYaw)
			targetYaw = NPCInfo->lockedDesiredYaw;
	}
	else
	{
		if (doPitch)
			targetPitch = NPCInfo->desiredPitch;
		if (doYaw)
			targetYaw = NPCInfo->desiredYaw;

		//		NPCInfo->aimTime = level.time + 250;
		if (doPitch)
			NPCInfo->lockedDesiredPitch = NPCInfo->desiredPitch;
		if (doYaw)
			NPCInfo->lockedDesiredYaw = NPCInfo->desiredYaw;
	}

	if (NPCInfo->aimErrorDebounceTime < level.time)
	{
		if (Q_irand(0, 1))
		{
			NPCInfo->lastAimErrorYaw = static_cast<float>(6 - NPCInfo->stats.aim) * Q_flrand(-1, 1);
		}
		if (Q_irand(0, 1))
		{
			NPCInfo->lastAimErrorPitch = static_cast<float>(6 - NPCInfo->stats.aim) * Q_flrand(-1, 1);
		}
		NPCInfo->aimErrorDebounceTime = level.time + Q_irand(250, 2000);
	}

	if (doYaw)
	{
		// decay yaw diff
		diff = AngleDelta(NPC->client->ps.viewangles[YAW], targetYaw);

		if (diff)
		{
			exact = qfalse;

			decay = 60.0 + 80.0;
			decay *= 50.0f / 1000.0f; //msec
			if (diff < 0.0)
			{
				diff += decay;
				if (diff > 0.0)
				{
					diff = 0.0;
				}
			}
			else
			{
				diff -= decay;
				if (diff < 0.0)
				{
					diff = 0.0;
				}
			}
		}

		// add yaw error based on NPCInfo->aim value
		error = NPCInfo->lastAimErrorYaw;

		ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + diff + error) - client->ps.delta_angles[YAW];
	}

	if (doPitch)
	{
		// decay pitch diff
		diff = AngleDelta(NPC->client->ps.viewangles[PITCH], targetPitch);
		if (diff)
		{
			exact = qfalse;

			decay = 60.0 + 80.0;
			decay *= 50.0f / 1000.0f; //msec
			if (diff < 0.0)
			{
				diff += decay;
				if (diff > 0.0)
				{
					diff = 0.0;
				}
			}
			else
			{
				diff -= decay;
				if (diff < 0.0)
				{
					diff = 0.0;
				}
			}
		}

		error = NPCInfo->lastAimErrorPitch;

		ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + diff + error) - client->ps.delta_angles[PITCH];
	}

	ucmd.angles[ROLL] = ANGLE2SHORT(NPC->client->ps.viewangles[ROLL]) - client->ps.delta_angles[ROLL];

	return exact;

#endif
}

//===================================================================================

/*
static void NPC_UpdateShootAngles (vec3_t angles, qboolean doPitch, qboolean doYaw )

Does update angles on shootAngles
*/

void NPC_UpdateShootAngles(vec3_t angles, const qboolean doPitch, const qboolean doYaw)
{
	//FIXME: shoot angles either not set right or not used!
	float error = 0.0f;
	float decay = 0.0f;
	float targetPitch = 0;
	float targetYaw = 0;

	if (doPitch)
		targetPitch = angles[PITCH];
	if (doYaw)
		targetYaw = angles[YAW];

	if (doYaw)
	{
		// decay yaw error
		error = AngleDelta(NPCInfo->shootAngles[YAW], targetYaw);
		if (error)
		{
			decay = 60.0 + 80.0 * NPCInfo->stats.aim;
			decay *= 100.0f / 1000.0f; //msec
			if (error < 0.0)
			{
				error += decay;
				if (error > 0.0)
				{
					error = 0.0;
				}
			}
			else
			{
				error -= decay;
				if (error < 0.0)
				{
					error = 0.0;
				}
			}
		}
		NPCInfo->shootAngles[YAW] = targetYaw + error;
	}

	if (doPitch)
	{
		// decay pitch error
		error = AngleDelta(NPCInfo->shootAngles[PITCH], targetPitch);
		if (error)
		{
			decay = 60.0 + 80.0 * NPCInfo->stats.aim;
			decay *= 100.0f / 1000.0f; //msec
			if (error < 0.0)
			{
				error += decay;
				if (error > 0.0)
				{
					error = 0.0;
				}
			}
			else
			{
				error -= decay;
				if (error < 0.0)
				{
					error = 0.0;
				}
			}
		}
		NPCInfo->shootAngles[PITCH] = targetPitch + error;
	}
}

/*
void SetTeamNumbers ()

Sets the number of living clients on each team

FIXME: Does not account for non-respawned players!
FIXME: Don't include medics?
*/
void SetTeamNumbers()
{
	int i = 0;

	for (i = 0; i < TEAM_NUM_TEAMS; i++)
	{
		teamNumbers[i] = 0;
		teamStrength[i] = 0;
	}

	for (i = 0; i < 1; i++)
	{
		const gentity_t* found = &g_entities[i];

		if (found->client)
		{
			if (found->health > 0) //FIXME: or if a player!
			{
				teamNumbers[found->client->playerTeam]++;
				teamStrength[found->client->playerTeam] += found->health;
			}
		}
	}

	for (i = 0; i < TEAM_NUM_TEAMS; i++)
	{
		//Get the average health
		teamStrength[i] = floor(static_cast<float>(teamStrength[i]) / static_cast<float>(teamNumbers[i]));
	}
}

extern stringID_table_t BSTable[];
extern stringID_table_t BSETTable[];

qboolean G_ActivateBehavior(gentity_t* self, const int bset)
{
	auto bSID = static_cast<bState_t>(-1);

	if (!self)
	{
		return qfalse;
	}

	char* bs_name = self->behaviorSet[bset];

	if (!(VALIDSTRING(bs_name)))
	{
		return qfalse;
	}

	if (self->NPC)
	{
		bSID = static_cast<bState_t>(GetIDForString(BSTable, bs_name));
	}

	if (bSID != static_cast<bState_t>(-1))
	{
		self->NPC->tempBehavior = BS_DEFAULT;
		self->NPC->behaviorState = bSID;
		if (bSID == BS_SEARCH || bSID == BS_WANDER)
		{
			//FIXME: Re implement?
			if (self->waypoint != WAYPOINT_NONE)
			{
				NPC_BSSearchStart(self->waypoint, bSID);
			}
			else
			{
				self->waypoint = NAV::GetNearestNode(self);
				if (self->waypoint != WAYPOINT_NONE)
				{
					NPC_BSSearchStart(self->waypoint, bSID);
				}
			}
		}
	}
	else
	{
		Quake3Game()->DebugPrint(IGameInterface::WL_VERBOSE, "%s attempting to run bSet %s (%s)\n", self->targetname,
			GetStringForID(BSETTable, bset), bs_name);
		Quake3Game()->RunScript(self, bs_name);
	}
	return qtrue;
}

/*
=============================================================================

	Extended Functions

=============================================================================
*/
extern qboolean G_ControlledByPlayer(const gentity_t* self);
/*
-------------------------
NPC_ValidEnemy
-------------------------
*/
qboolean G_ValidEnemy(const gentity_t* self, const gentity_t* enemy)
{
	//Must be a valid pointer
	if (enemy == nullptr)
		return qfalse;

	//Must not be me
	if (enemy == self)
		return qfalse;

	//Must not be deleted
	if (enemy->inuse == qfalse)
		return qfalse;

	//Must be alive
	if (enemy->health <= 0)
		return qfalse;

	//In case they're in notarget mode
	if (enemy->flags & FL_NOTARGET)
		return qfalse;

	//Must be an NPC
	if (enemy->client == nullptr)
	{
		if (enemy->svFlags & SVF_NONNPC_ENEMY)
		{
			//still potentially valid
			if (self->client)
			{
				if (enemy->noDamageTeam == self->client->playerTeam)
				{
					return qfalse;
				}
				return qtrue;
			}
			if (enemy->noDamageTeam == self->noDamageTeam)
			{
				return qfalse;
			}
			return qtrue;
		}
		return qfalse;
	}

	// a Force Projection copy (CLASS_PROJECTION, put on the player's team in g_active.cpp) fights everyone who is not
	// on the player's team, whatever side he plays (DH 2026-11-09): enemies, other teams, rampaging creatures; not the
	// neutrals (civilians, droids), not the player, not another copy
	if (!self->client)
	{
		return qfalse; // the team checks below need my client (C6011)
	}
	if (self->client->NPC_class == CLASS_PROJECTION && g_entities[0].client)
	{
		return enemy->s.number != 0 && enemy->client->playerTeam != g_entities[0].client->playerTeam
			&& enemy->client->playerTeam != TEAM_NEUTRAL && enemy->client->NPC_class != CLASS_PROJECTION ? qtrue : qfalse;
	}

	if (enemy->client->playerTeam == TEAM_FREE && enemy->s.number < MAX_CLIENTS)
	{
		//An evil player, everyone attacks him
		return qtrue;
	}

	if (self->client->playerTeam == TEAM_SOLO || enemy->client->playerTeam == TEAM_SOLO)
	{
		return qtrue;
	}

	if (self->client->playerTeam == TEAM_PROJECTION
		/*&& (enemy->client->playerTeam != TEAM_PROJECTION && G_ControlledByPlayer(enemy))*/)
	{
		return qfalse;
	}

	//Can't be on the same team
	if (enemy->client->playerTeam == self->client->playerTeam)
	{
		return qfalse;
	}

	if (enemy->client->playerTeam == self->client->enemyTeam //simplest case: they're on my enemy team
		|| self->client->enemyTeam == TEAM_FREE && enemy->client->NPC_class != self->client->NPC_class
		//I get mad at anyone and this guy isn't the same class as me
		|| enemy->client->NPC_class == CLASS_WAMPA && enemy->enemy //a rampaging wampa
		|| enemy->client->NPC_class == CLASS_RANCOR && enemy->enemy //a rampaging rancor
		|| enemy->client->playerTeam == TEAM_FREE && enemy->client->enemyTeam == TEAM_FREE && enemy->enemy && enemy->
		enemy->client && (enemy->enemy->client->playerTeam == self->client->playerTeam || enemy->enemy->client->
			playerTeam != TEAM_ENEMY && self->client->playerTeam == TEAM_PLAYER)
		//enemy is a rampaging non-aligned creature who is attacking someone on our team or a non-enemy (this last condition is used only if we're a good guy - in effect, we protect the innocent)
		|| self->client->enemyTeam == TEAM_FREE && enemy->client->NPC_class == self->client->NPC_class)
	{
		return qtrue;
	}
	//all other cases = false?
	return qfalse;
}

qboolean NPC_ValidEnemy(const gentity_t* ent)
{
	return G_ValidEnemy(NPC, ent);
}

/*
-------------------------
NPC_TargetVisible
-------------------------
*/

static qboolean NPC_TargetVisible(const gentity_t* ent)
{
	//Make sure we're in a valid range
	if (DistanceSquared(ent->currentOrigin, NPC->currentOrigin) > NPCInfo->stats.visrange * NPCInfo->stats.visrange)
		return qfalse;

	//Check our FOV
	if (InFOV(ent, NPC, NPCInfo->stats.hfov, NPCInfo->stats.vfov) == qfalse)
		return qfalse;

	//Check for sight (Force Sight 3 senses through walls)
	if (!NPC_SeesThroughWalls(NPC) && NPC_ClearLOS(ent) == qfalse)
		return qfalse;

	return qtrue;
}

/*
-------------------------
NPC_FindNearestEnemy
-------------------------
*/

constexpr auto MAX_RADIUS_ENTS = 256; //NOTE: This can cause entities to be lost;
constexpr auto NEAR_DEFAULT_RADIUS = 256;
extern gentity_t* G_CheckControlledTurretEnemy(const gentity_t* self, gentity_t* enemy, qboolean validate);

static int NPC_FindNearestEnemy(const gentity_t* ent)
{
	// JA:R fix: the old 256-entry list dropped entities in busy scenes; NPC think is
	// serial, so a static full-size list is safe and keeps it off the stack
	static gentity_t* radius_ents[MAX_GENTITIES];
	vec3_t mins{}, maxs{};
	int nearestEntID = -1;
	float nearestDist = static_cast<float>(WORLD_SIZE) * static_cast<float>(WORLD_SIZE);
	int numChecks = 0;
	int i = 0;

	//Setup the bbox to search in
	for (i = 0; i < 3; i++)
	{
		mins[i] = ent->currentOrigin[i] - NPCInfo->stats.visrange;
		maxs[i] = ent->currentOrigin[i] + NPCInfo->stats.visrange;
	}

	//Get a number of entities in a given space
	const int num_ents = gi.EntitiesInBox(mins, maxs, radius_ents, MAX_GENTITIES);

	for (i = 0; i < num_ents; i++)
	{
		const gentity_t* nearest = G_CheckControlledTurretEnemy(ent, radius_ents[i], qtrue);

		//Don't consider self
		if (nearest == ent || nearest == nullptr)
			continue;

		//JA:R: skip the visibility trace for anything farther than the best so far
		if (DistanceSquared(ent->currentOrigin, nearest->currentOrigin) >= nearestDist)
			continue;

		//Must be valid
		if (NPC_ValidEnemy(nearest) == qfalse)
			continue;

		numChecks++;
		//Must be visible
		if (NPC_TargetVisible(nearest) == qfalse)
			continue;

		const float distance = DistanceSquared(ent->currentOrigin, nearest->currentOrigin);

		//Found one closer to us
		if (distance < nearestDist)
		{
			nearestEntID = nearest->s.number;
			nearestDist = distance;
		}
	}

	return nearestEntID;
}

/*
-------------------------
NPC_PickEnemyExt
-------------------------
*/

static gentity_t* NPC_PickEnemyExt(const qboolean checkAlerts = qfalse)
{
	//If we've asked for the closest enemy
	const int entID = NPC_FindNearestEnemy(NPC);

	//If we have a valid enemy, use it
	if (entID >= 0)
		return &g_entities[entID];

	if (checkAlerts)
	{
		const int alert_event = NPC_CheckAlertEvents(qtrue, qtrue, -1, qtrue, AEL_DISCOVERED);

		//There is an event to look at
		if (alert_event >= 0)
		{
			const alertEvent_t* event = &level.alertEvents[alert_event];

			//Don't pay attention to our own alerts
			if (event->owner == NPC)
				return nullptr;

			if (event->level >= AEL_DISCOVERED)
			{
				//If it's the player, attack him
				if (event->owner == &g_entities[0])
					return event->owner;

				//If it's on our team, then take its enemy as well
				if (event->owner && event->owner->client && event->owner->client->playerTeam == NPC->client->playerTeam)
					return event->owner->enemy;
			}
		}
	}

	return nullptr;
}

/*
-------------------------
NPC_FindPlayer
-------------------------
*/

qboolean NPC_FindPlayer()
{
	return NPC_TargetVisible(&g_entities[0]);
}

/*
-------------------------
NPC_CheckPlayerDistance
-------------------------
*/

static qboolean NPC_CheckPlayerDistance()
{
	//Make sure we have an enemy
	if (NPC->enemy == nullptr)
		return qfalse;

	//Only do this for non-players
	if (NPC->enemy->s.number == 0)
		return qfalse;

	//must be set up to get mad at player
	if (!NPC->client || NPC->client->enemyTeam != TEAM_PLAYER)
		return qfalse;

	//the player must be a valid enemy (notarget, dead): else G_SetEnemy refuses him but we still returned qtrue
	if (NPC_ValidEnemy(&g_entities[0]) == qfalse)
		return qfalse;

	//Must be within our FOV and in sight, not behind a wall (DH 2026-10-24)
	if (NPC_CheckVisibility(&g_entities[0], CHECK_360 | CHECK_FOV) < VIS_FOV)
		return qfalse;

	const float distance = DistanceSquared(NPC->currentOrigin, NPC->enemy->currentOrigin);

	if (distance > DistanceSquared(NPC->currentOrigin, g_entities[0].currentOrigin))
	{
		G_SetEnemy(NPC, &g_entities[0]);
		return qtrue;
	}

	return qfalse;
}

/*
-------------------------
NPC_FindEnemy
-------------------------
*/

extern cvar_t* com_outcast;
static qboolean NPC_FindEnemy(const qboolean checkAlerts)
{
	// 0. Exclusion: STOfficer + STOfficerAlt with flechette must NOT auto-acquire player
	qboolean isOfficerType = qfalse;
	qboolean isOfficerWithFlechette = qfalse;

	if (Q_stricmp(NPC->NPC_type, "STOfficer") == 0 ||
		Q_stricmp(NPC->NPC_type, "STOfficerAlt") == 0)
	{
		isOfficerType = qtrue;
	}

	if (isOfficerType && NPC->client->ps.weapon == WP_FLECHETTE)
	{
		isOfficerWithFlechette = qtrue;
	}

	// 2. Ignore enemies if flagged
	if (NPC->svFlags & SVF_IGNORE_ENEMIES)
	{
		G_ClearEnemy(NPC);
		return qfalse;
	}

	// 3. Confusion disables enemy acquisition
	if (NPCInfo->confusionTime > level.time)
	{
		G_ClearEnemy(NPC);
		return qfalse;
	}

	// 4. Keep locked enemy if valid
	if ((NPC->svFlags & SVF_LOCKEDENEMY) && NPC_ValidEnemy(NPC->enemy))
		return qtrue;

	// 4b. JKA campaign: take the player as soon as he is in sight (except the excluded officers). Only when he is a
	// valid enemy (not notarget, alive, not on our team). This used to run first, before the ignore-enemies, mind
	// trick and locked-enemy checks, and returned qtrue even when G_SetEnemy refused him (notarget), so a notarget
	// player kept being targeted and the old enemy was never dropped.
	if (com_outcast && com_outcast->integer == 0 && !isOfficerWithFlechette)
	{
		gentity_t* player = &g_entities[0];
		if (NPC_ValidEnemy(player) && NPC_CheckVisibility(player, CHECK_PVS | CHECK_360 | CHECK_FOV | CHECK_VISRANGE) >= VIS_FOV)
		{
			G_SetEnemy(NPC, player);
			if (NPC->enemy == player)
			{
				return qtrue;
			}
		}
	}

	// 5. Player-distance priority (except monsters)
	if (NPC->client->NPC_class != CLASS_RANCOR &&
		NPC->client->NPC_class != CLASS_WAMPA &&
		NPC->client->NPC_class != CLASS_SAND_CREATURE &&
		NPC_CheckPlayerDistance())
	{
		return qtrue;
	}

	// 6. Unlock enemy if we reach this point
	NPC->svFlags &= ~SVF_LOCKEDENEMY;

	// 7. Keep current enemy if still valid
	if (NPC_ValidEnemy(NPC->enemy))
		return qtrue;

	// 8. Try to pick a new enemy
	gentity_t* newEnemy = NPC_PickEnemyExt(checkAlerts);

	if (NPC_ValidEnemy(newEnemy))
	{
		G_SetEnemy(NPC, newEnemy);
		return qtrue;
	}

	// 9. No valid enemy found
	G_ClearEnemy(NPC);
	return qfalse;
}

/*
-------------------------
NPC_CheckEnemyExt
-------------------------
*/

qboolean NPC_CheckEnemyExt(const qboolean checkAlerts)
{
	return NPC_FindEnemy(checkAlerts);
}

/*
-------------------------
NPC_FacePosition
-------------------------
*/

qboolean NPC_FacePosition(vec3_t position, const qboolean doPitch)
{
	vec3_t muzzle = { 0 };
	qboolean facing = qtrue;

	//Get the positions
	if (NPC->client && (NPC->client->NPC_class == CLASS_RANCOR || NPC->client->NPC_class == CLASS_WAMPA || NPC->client->
		NPC_class == CLASS_SAND_CREATURE))
	{
		CalcEntitySpot(NPC, SPOT_ORIGIN, muzzle);
		muzzle[2] += NPC->maxs[2] * 0.75f;
	}
	else if (NPC->client && NPC->client->NPC_class == CLASS_GALAKMECH)
	{
		CalcEntitySpot(NPC, SPOT_WEAPON, muzzle);
	}
	else
	{
		CalcEntitySpot(NPC, SPOT_HEAD_LEAN, muzzle); //SPOT_HEAD
		if (NPC->client && NPC->client->NPC_class == CLASS_ROCKETTROOPER)
		{
			//*sigh*, look down more
			position[2] -= 32;
		}
	}

	//Find the desired angles
	vec3_t angles = { 0 };

	GetAnglesForDirection(muzzle, position, angles);

	NPCInfo->desiredYaw = AngleNormalize360(angles[YAW]);
	NPCInfo->desiredPitch = AngleNormalize360(angles[PITCH]);

	if (NPC->enemy && NPC->enemy->client && NPC->enemy->client->NPC_class == CLASS_ATST)
	{
		// FIXME: this is kind of dumb, but it was the easiest way to get it to look sort of ok
		NPCInfo->desiredYaw += Q_flrand(-5, 5) + sin(level.time * 0.004f) * 7;
		NPCInfo->desiredPitch += Q_flrand(-2, 2);
	}
	//Face that yaw
	NPC_UpdateAngles(qtrue, qtrue);

	//Find the delta between our goal and our current facing
	const float yawDelta = AngleNormalize360(
		NPCInfo->desiredYaw - SHORT2ANGLE(ucmd.angles[YAW] + client->ps.delta_angles[YAW]));

	//See if we are facing properly
	if (fabs(yawDelta) > VALID_ATTACK_CONE)
		facing = qfalse;

	if (doPitch)
	{
		//Find the delta between our goal and our current facing
		const float currentAngles = SHORT2ANGLE(ucmd.angles[PITCH] + client->ps.delta_angles[PITCH]);
		const float pitchDelta = NPCInfo->desiredPitch - currentAngles;

		//See if we are facing properly
		if (fabs(pitchDelta) > VALID_ATTACK_CONE)
			facing = qfalse;
	}

	return facing;
}

/*
-------------------------
NPC_FaceEntity
-------------------------
*/

qboolean NPC_FaceEntity(const gentity_t* ent, const qboolean doPitch)
{
	vec3_t ent_pos = { 0 };

	//Get the positions
	CalcEntitySpot(ent, SPOT_HEAD_LEAN, ent_pos);

	return NPC_FacePosition(ent_pos, doPitch);
}

/*
-------------------------
NPC_FaceEnemy
-------------------------
*/

qboolean NPC_FaceEnemy(const qboolean doPitch)
{
	if (NPC == nullptr)
		return qfalse;

	if (NPC->enemy == nullptr)
		return qfalse;

	return NPC_FaceEntity(NPC->enemy, doPitch);
}

/*
-------------------------
NPC_CheckCanAttackExt
-------------------------
*/

qboolean NPC_CheckCanAttackExt()
{
	//We don't want them to shoot
	if (NPCInfo->scriptFlags & SCF_DONT_FIRE)
		return qfalse;

	//Turn to face
	if (NPC_FaceEnemy(qtrue) == qfalse)
		return qfalse;

	//Must have a clear line of sight to the target
	if (NPC_ClearShot(NPC->enemy) == qfalse)
		return qfalse;

	return qtrue;
}

/*
-------------------------
NPC_ClearLookTarget
-------------------------
*/

void NPC_ClearLookTarget(const gentity_t* self)
{
	if (!self->client)
	{
		return;
	}

	self->client->renderInfo.lookTarget = ENTITYNUM_NONE; //ENTITYNUM_WORLD;
	self->client->renderInfo.lookTargetClearTime = 0;
}

/*
-------------------------
NPC_SetLookTarget
-------------------------
*/
void NPC_SetLookTarget(const gentity_t* self, const int entNum, const int clearTime)
{
	if (!self->client)
	{
		return;
	}

	self->client->renderInfo.lookTarget = entNum;
	self->client->renderInfo.lookTargetClearTime = clearTime;
}

/*
-------------------------
NPC_CheckLookTarget
-------------------------
*/
qboolean NPC_CheckLookTarget(const gentity_t* self)
{
	if (!self || !self->client)
		return qfalse;

	const int lt = self->client->renderInfo.lookTarget;

	// Valid index?
	if (lt < 0 || lt >= ENTITYNUM_WORLD)
		return qfalse;

	gentity_t* target = &g_entities[lt];

	// Target must exist and be in use
	if (!target || !target->inuse)
	{
		NPC_ClearLookTarget(self);
		return qfalse;
	}

	// Timed look target expired
	if (self->client->renderInfo.lookTargetClearTime &&
		self->client->renderInfo.lookTargetClearTime < level.time)
	{
		NPC_ClearLookTarget(self);
		return qfalse;
	}

	// If we have an enemy, always look at them instead of a scripted target
	if (target->client && self->enemy && target != self->enemy)
	{
		NPC_ClearLookTarget(self);
		return qfalse;
	}

	return qtrue;
}

/*
-------------------------
NPC_CheckCharmed
-------------------------
*/
extern void G_AddVoiceEvent(const gentity_t* self, int event, int speak_debounce_time);

void G_CheckCharmed(gentity_t* self)
{
	if (self
		&& self->client
		&& self->client->playerTeam == TEAM_PLAYER
		&& self->NPC
		&& self->NPC->charmedTime
		&& (self->NPC->charmedTime < level.time || self->health <= 0))
	{
		//we were charmed, set us back!
		//NOTE: presumptions here...
		const team_t savTeam = self->client->enemyTeam;
		self->client->enemyTeam = self->client->playerTeam;
		self->client->playerTeam = savTeam;
		self->client->leader = nullptr;
		self->NPC->charmedTime = 0;
		if (self->health > 0)
		{
			if (self->NPC->tempBehavior == BS_FOLLOW_LEADER)
			{
				self->NPC->tempBehavior = BS_DEFAULT;
			}
			G_ClearEnemy(self);
			//say something to let player know you've snapped out of it
			G_AddVoiceEvent(self, Q_irand(EV_CONFUSE1, EV_CONFUSE3), 2000);
		}
	}
}

void G_GetBoltPosition(gentity_t* self, const int bolt_index, vec3_t pos, const int modelIndex = 0)
{
	if (!self || !self->ghoul2.size())
	{
		return;
	}
	mdxaBone_t bolt_matrix = { 0 };
	const vec3_t angles = { 0, self->currentAngles[YAW], 0 };

	gi.G2API_GetBoltMatrix(self->ghoul2, modelIndex,
		bolt_index,
		&bolt_matrix, angles, self->currentOrigin, cg.time ? cg.time : level.time,
		nullptr, self->s.modelScale);
	if (pos)
	{
		vec3_t result = { 0 };
		gi.G2API_GiveMeVectorFromMatrix(bolt_matrix, ORIGIN, result);
		VectorCopy(result, pos);
	}
}

float NPC_EntRangeFromBolt(const gentity_t* targEnt, const int bolt_index)
{
	vec3_t org = { 0.0f };

	if (!targEnt)
	{
		return Q3_INFINITE;
	}

	G_GetBoltPosition(NPC, bolt_index, org);

	return Distance(targEnt->currentOrigin, org);
}

float NPC_EnemyRangeFromBolt(const int bolt_index)
{
	return NPC_EntRangeFromBolt(NPC->enemy, bolt_index);
}

int G_GetEntsNearBolt(gentity_t* self, gentity_t** radius_ents, const float radius, const int bolt_index, vec3_t bolt_org)
{
	vec3_t mins{}, maxs{};

	//get my handRBolt's position
	vec3_t org = { 0.0f };

	G_GetBoltPosition(self, bolt_index, org);

	VectorCopy(org, bolt_org);

	//Setup the bbox to search in
	for (int i = 0; i < 3; i++)
	{
		mins[i] = bolt_org[i] - radius;
		maxs[i] = bolt_org[i] + radius;
	}

	//Get the number of entities in a given space
	return gi.EntitiesInBox(mins, maxs, radius_ents, 128);
}

int NPC_GetEntsNearBolt(gentity_t** radius_ents, const float radius, const int bolt_index, vec3_t bolt_org)
{
	return G_GetEntsNearBolt(NPC, radius_ents, radius, bolt_index, bolt_org);
}

extern qboolean RT_Flying(const gentity_t* self);
extern void RT_FlyStart(gentity_t* self);
extern void RT_FlyStop(gentity_t* self);
extern qboolean Boba_Flying(const gentity_t* self);
extern void Boba_FlyStart(gentity_t* self);
extern void Boba_FlyStop(gentity_t* self);

qboolean rocket_trooper_player(const gentity_t* self)
{
	if (self->s.number < MAX_CLIENTS || G_ControlledByPlayer(self))
	{
		if (self->client->NPC_class == CLASS_ROCKETTROOPER)
		{
			return qtrue;
		}
	}
	return qfalse;
}

qboolean JET_Flying(const gentity_t* self)
{
	if (!self || !self->client)
	{
		return qfalse;
	}
	if (self->client->NPC_class == CLASS_BOBAFETT || self->client->NPC_class == CLASS_MANDALORIAN || self->client->
		NPC_class == CLASS_JANGO || self->client->NPC_class == CLASS_JANGODUAL || self->client->NPC_class ==
		CLASS_ROCKETTROOPER && rocket_trooper_player(self))
	{
		return Boba_Flying(self);
	}

	if (self->client->NPC_class == CLASS_ROCKETTROOPER && !rocket_trooper_player(self))
	{
		return RT_Flying(self);
	}
	return qfalse;
}

void JET_FlyStart(gentity_t* self)
{
	if (!self || !self->client)
	{
		return;
	}
	self->lastInAirTime = level.time;

	if (self->client->NPC_class == CLASS_BOBAFETT || self->client->NPC_class == CLASS_MANDALORIAN || self->client->
		NPC_class == CLASS_JANGO || self->client->NPC_class == CLASS_JANGODUAL || self->client->NPC_class ==
		CLASS_ROCKETTROOPER && rocket_trooper_player(self))
	{
		Boba_FlyStart(self);
	}
	else if (self->client->NPC_class == CLASS_ROCKETTROOPER && !rocket_trooper_player(self))
	{
		RT_FlyStart(self);
	}
	else
	{
		//
	}
}

void jet_fly_stop(gentity_t* self)
{
	if (!self || !self->client)
	{
		return;
	}

	if (self->client->NPC_class == CLASS_BOBAFETT ||
		self->client->NPC_class == CLASS_MANDALORIAN ||
		self->client->NPC_class == CLASS_JANGO ||
		self->client->NPC_class == CLASS_JANGODUAL ||
		self->client->NPC_class == CLASS_ROCKETTROOPER && rocket_trooper_player(self))
	{
		Boba_FlyStop(self);
	}
	else if (self->client->NPC_class == CLASS_ROCKETTROOPER && !rocket_trooper_player(self))
	{
		RT_FlyStop(self);
	}
	else
	{
		Boba_FlyStop(self);
	}
}

void NPC_SetSurfaceOnOff(gentity_t* ent, const char* surfaceName, qboolean turnOn)
{
	if (!ent || ent->ghoul2.size() == 0)
		return;

	CGhoul2Info* g2 = &ent->ghoul2[ent->playerModel];
	if (!g2)
		return;

	// Validate surface exists (3 args: g2, name, flags)
	// flags = 0 (no special behaviour)
	if (!gi.G2API_GetSurfaceIndex(g2, surfaceName))
		return;

	// Toggle surface (3 args: g2, name, flags)
	if (turnOn)
	{
		gi.G2API_SetSurfaceOnOff(g2, surfaceName, 0);  // ON = flags = 0
	}
	else
	{
		gi.G2API_SetSurfaceOnOff(g2, surfaceName, G2SURFACEFLAG_OFF);  // OFF flag
	}

	// Update model index for renderer
	g2->mModelindex = ent->s.modelindex;
}

#else // MP game

//NPC_utils.cpp

#include "game/b_local.h"
#include "icarus/Q3_Interface.h"
#include "ghoul2/G2.h"

int teamNumbers[TEAM_NUM_TEAMS];
int teamStrength[TEAM_NUM_TEAMS];
int teamCounter[TEAM_NUM_TEAMS];

#define	VALID_ATTACK_CONE	2.0f	//Degrees
extern void G_DebugPrint(int level, const char* format, ...);

/*
void CalcEntitySpot ( gentity_t *ent, spot_t spot, vec3_t point )

Added: Uses shootAngles if a NPC has them

*/
void CalcEntitySpot(const gentity_t* ent, const spot_t spot, vec3_t point)
{
	vec3_t forward = { 0 }, up = { 0 }, right = { 0 };
	vec3_t start = { 0 }, end = { 0 };
	trace_t tr = { 0 };

	if (!ent)
	{
		return;
	}
	switch (spot)
	{
	case SPOT_ORIGIN:
		if (VectorCompare(ent->r.currentOrigin, vec3_origin))
		{
			//brush
			VectorSubtract(ent->r.absmax, ent->r.absmin, point); //size
			VectorMA(ent->r.absmin, 0.5, point, point);
		}
		else
		{
			VectorCopy(ent->r.currentOrigin, point);
		}
		break;

	case SPOT_CHEST:
	case SPOT_HEAD:
		if (ent->client && VectorLengthSquared(ent->client->renderInfo.eyePoint) && (ent->client->ps.viewEntity <= 0 ||
			ent->client->ps.viewEntity >= ENTITYNUM_WORLD))
		{
			//Actual tag_head eyespot!
			VectorCopy(ent->client->renderInfo.eyePoint, point);
			if (ent->client->NPC_class == CLASS_ATST)
			{
				//adjust up some
				point[2] += 28; //magic number :)
			}
			if (ent->NPC)
			{
				//always aim from the center of my bbox, so we don't wiggle when we lean forward or backwards
				point[0] = ent->r.currentOrigin[0];
				point[1] = ent->r.currentOrigin[1];
			}
		}
		else
		{
			VectorCopy(ent->r.currentOrigin, point);
			if (ent->client)
			{
				point[2] += ent->client->ps.viewheight;
			}
		}
		if (spot == SPOT_CHEST && ent->client)
		{
			if (ent->client->NPC_class != CLASS_ATST)
			{
				//adjust up some
				point[2] -= ent->r.maxs[2] * 0.2f;
			}
		}
		break;

	case SPOT_HEAD_LEAN:
		if (ent->client && VectorLengthSquared(ent->client->renderInfo.eyePoint) && (ent->client->ps.viewEntity <= 0 ||
			ent->client->ps.viewEntity >= ENTITYNUM_WORLD))
		{
			//Actual tag_head eyespot!
			VectorCopy(ent->client->renderInfo.eyePoint, point);
			if (ent->client->NPC_class == CLASS_ATST)
			{
				//adjust up some
				point[2] += 28; //magic number :)
			}
			if (ent->NPC)
			{
				//always aim from the center of my bbox, so we don't wiggle when we lean forward or backwards
				point[0] = ent->r.currentOrigin[0];
				point[1] = ent->r.currentOrigin[1];
			}
		}
		else
		{
			VectorCopy(ent->r.currentOrigin, point);
			if (ent->client)
			{
				point[2] += ent->client->ps.viewheight;
			}
			//AddLeanOfs ( ent, point );
		}
		break;

	case SPOT_LEGS:
		VectorCopy(ent->r.currentOrigin, point);
		point[2] += ent->r.mins[2] * 0.5;
		break;

	case SPOT_WEAPON:
		if (ent->NPC && !VectorCompare(ent->NPC->shootAngles, vec3_origin) && !VectorCompare(
			ent->NPC->shootAngles, ent->client->ps.viewangles))
		{
			AngleVectors(ent->NPC->shootAngles, forward, right, up);
		}
		else
		{
			AngleVectors(ent->client->ps.viewangles, forward, right, up);
		}
		calcmuzzlePoint((gentity_t*)ent, forward, right, point);
		//NOTE: automatically takes leaning into account!
		break;

	case SPOT_GROUND:
		// if entity is on the ground, just use it's absmin
		if (ent->s.groundEntityNum != ENTITYNUM_NONE)
		{
			VectorCopy(ent->r.currentOrigin, point);
			point[2] = ent->r.absmin[2];
			break;
		}

		// if it is reasonably close to the ground, give the point underneath of it
		VectorCopy(ent->r.currentOrigin, start);
		start[2] = ent->r.absmin[2];
		VectorCopy(start, end);
		end[2] -= 64;
		trap->Trace(&tr, start, ent->r.mins, ent->r.maxs, end, ent->s.number, MASK_PLAYERSOLID, qfalse, 0, 0);
		if (tr.fraction < 1.0)
		{
			VectorCopy(tr.endpos, point);
			break;
		}

		// otherwise just use the origin
		VectorCopy(ent->r.currentOrigin, point);
		break;

	default:
		VectorCopy(ent->r.currentOrigin, point);
		break;
	}
}

//===================================================================================

/*
qboolean NPC_UpdateAngles ( qboolean doPitch, qboolean doYaw )

Added: option to do just pitch or just yaw

Does not include "aim" in it's calculations

FIXME: stop compressing angles into shorts!!!!
*/
qboolean NPC_UpdateAngles(const qboolean do_pitch, const qboolean do_yaw)
{
#if 1

	float error = 0.0f;
	float decay = 0.0f;
	float targetPitch = 0;
	float targetYaw = 0;
	float yawSpeed = 0.0f;
	qboolean exact = qtrue;

	// if angle changes are locked; just keep the current angles
	// aimTime isn't even set anymore... so this code was never reached, but I need a way to lock NPC's yaw, so instead of making a new SCF_ flag, just use the existing render flag... - dmv
	if (!NPCS.NPC->enemy && (level.time < NPCS.NPCInfo->aimTime || NPCS.NPC->client->renderInfo.renderFlags &
		RF_LOCKEDANGLE))
	{
		if (do_pitch)
			targetPitch = NPCS.NPCInfo->lockedDesiredPitch;

		if (do_yaw)
			targetYaw = NPCS.NPCInfo->lockedDesiredYaw;
	}
	else
	{
		// we're changing the lockedDesired Pitch/Yaw below so it's lost it's original meaning, get rid of the lock flag
		NPCS.NPC->client->renderInfo.renderFlags &= ~RF_LOCKEDANGLE;

		if (do_pitch)
		{
			targetPitch = NPCS.NPCInfo->desiredPitch;
			NPCS.NPCInfo->lockedDesiredPitch = NPCS.NPCInfo->desiredPitch;
		}

		if (do_yaw)
		{
			targetYaw = NPCS.NPCInfo->desiredYaw;
			NPCS.NPCInfo->lockedDesiredYaw = NPCS.NPCInfo->desiredYaw;
		}
	}

	if (NPCS.NPC->s.weapon == WP_EMPLACED_GUN)
	{
		// FIXME: this seems to do nothing, actually...
		yawSpeed = 20;
	}
	else
	{
		if (NPCS.NPC->client->NPC_class == CLASS_ROCKETTROOPER
			&& !NPCS.NPC->enemy)
		{
			//just slowly lookin' around
			yawSpeed = 1;
		}
		else
		{
			yawSpeed = NPCS.NPCInfo->stats.yawSpeed;
		}
	}

	if (NPCS.NPC->s.weapon == WP_SABER && NPCS.NPC->client->ps.fd.forcePowersActive & 1 << FP_SPEED)
	{
		char buf[128] = { 0 };

		trap->Cvar_VariableStringBuffer("timescale", buf, sizeof buf);

		const float tFVal = atof(buf);

		yawSpeed *= 1.0f / tFVal;
	}

	if (NPCS.NPCInfo->lookMode)
	{
		//We have a specific kind of looking we are doing, lower the lookMod, slower the yawspeed
		yawSpeed *= (float)NPCS.NPCInfo->lookMode / (float)LT_FULLFACE;
	}

	if (do_yaw)
	{
		// decay yaw error
		error = AngleDelta(NPCS.NPC->client->ps.viewangles[YAW], targetYaw);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0f / 1000.0f; //msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}

		NPCS.ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + error) - NPCS.client->ps.delta_angles[YAW];
	}

	//FIXME: have a pitchSpeed?
	if (do_pitch)
	{
		// decay pitch error
		error = AngleDelta(NPCS.NPC->client->ps.viewangles[PITCH], targetPitch);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0f / 1000.0f; //msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}

		NPCS.ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + error) - NPCS.client->ps.delta_angles[PITCH];
	}

	NPCS.ucmd.angles[ROLL] = ANGLE2SHORT(NPCS.NPC->client->ps.viewangles[ROLL]) - NPCS.client->ps.delta_angles[ROLL];

	if (exact && trap->ICARUS_TaskIDPending((sharedEntity_t*)NPCS.NPC, TID_ANGLE_FACE))
	{
		trap->ICARUS_TaskIDComplete((sharedEntity_t*)NPCS.NPC, TID_ANGLE_FACE);
	}
	return exact;

#else

	float		error;
	float		decay;
	float		targetPitch = 0;
	float		targetYaw = 0;
	float		yawSpeed;
	qboolean	exact = qtrue;
	qboolean	doSound = qfalse;

	// if angle changes are locked; just keep the current angles
	if (level.time < NPCInfo->aimTime)
	{
		if (doPitch)
			targetPitch = NPCInfo->lockedDesiredPitch;
		if (doYaw)
			targetYaw = NPCInfo->lockedDesiredYaw;
	}
	else
	{
		if (doPitch)
			targetPitch = NPCInfo->desiredPitch;
		if (doYaw)
			targetYaw = NPCInfo->desiredYaw;

		if (doPitch)
			NPCInfo->lockedDesiredPitch = NPCInfo->desiredPitch;
		if (doYaw)
			NPCInfo->lockedDesiredYaw = NPCInfo->desiredYaw;
	}

	yawSpeed = NPCInfo->stats.yawSpeed;

	if (doYaw)
	{
		// decay yaw error
		error = AngleDelta(NPC->client->ps.viewangles[YAW], targetYaw);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0 / 1000.0;//msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}
		ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + error) - client->ps.delta_angles[YAW];
	}

	//FIXME: have a pitchSpeed?
	if (doPitch)
	{
		// decay pitch error
		error = AngleDelta(NPC->client->ps.viewangles[PITCH], targetPitch);
		if (fabs(error) > MIN_ANGLE_ERROR)
		{
			if (error)
			{
				exact = qfalse;

				decay = 60.0 + yawSpeed * 3;
				decay *= 50.0 / 1000.0;//msec

				if (error < 0.0)
				{
					error += decay;
					if (error > 0.0)
					{
						error = 0.0;
					}
				}
				else
				{
					error -= decay;
					if (error < 0.0)
					{
						error = 0.0;
					}
				}
			}
		}
		ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + error) - client->ps.delta_angles[PITCH];
	}

	ucmd.angles[ROLL] = ANGLE2SHORT(NPC->client->ps.viewangles[ROLL]) - client->ps.delta_angles[ROLL];

	return exact;

#endif
}

void NPC_AimWiggle(vec3_t enemy_org)
{
	//shoot for somewhere between the head and torso
	//NOTE: yes, I know this looks weird, but it works
	if (NPCS.NPCInfo->aimErrorDebounceTime < level.time)
	{
		NPCS.NPCInfo->aimOfs[0] = 0.3 * flrand(NPCS.NPC->enemy->r.mins[0], NPCS.NPC->enemy->r.maxs[0]);
		NPCS.NPCInfo->aimOfs[1] = 0.3 * flrand(NPCS.NPC->enemy->r.mins[1], NPCS.NPC->enemy->r.maxs[1]);
		if (NPCS.NPC->enemy->r.maxs[2] > 0)
		{
			NPCS.NPCInfo->aimOfs[2] = NPCS.NPC->enemy->r.maxs[2] * flrand(0.0f, -1.0f);
		}
	}
	VectorAdd(enemy_org, NPCS.NPCInfo->aimOfs, enemy_org);
}

/*
qboolean NPC_UpdateFiringAngles ( qboolean doPitch, qboolean doYaw )

  Includes aim when determining angles - so they don't always hit...
  */
qboolean NPC_UpdateFiringAngles(const qboolean do_pitch, const qboolean do_yaw)
{
#if 0

	float		diff;
	float		error;
	float		targetPitch = 0;
	float		targetYaw = 0;
	qboolean	exact = qtrue;

	if (level.time < NPCInfo->aimTime)
	{
		if (doPitch)
			targetPitch = NPCInfo->lockedDesiredPitch;

		if (doYaw)
			targetYaw = NPCInfo->lockedDesiredYaw;
	}
	else
	{
		if (doPitch)
		{
			targetPitch = NPCInfo->desiredPitch;
			NPCInfo->lockedDesiredPitch = NPCInfo->desiredPitch;
		}

		if (doYaw)
		{
			targetYaw = NPCInfo->desiredYaw;
			NPCInfo->lockedDesiredYaw = NPCInfo->desiredYaw;
		}
	}

	if (doYaw)
	{
		// add yaw error based on NPCInfo->aim value
		error = ((float)(6 - NPCInfo->stats.aim)) * flrand(-1, 1);

		if (Q_irand(0, 1))
			error *= -1;

		diff = AngleDelta(NPC->client->ps.viewangles[YAW], targetYaw);

		if (diff)
			exact = qfalse;

		ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + diff + error) - client->ps.delta_angles[YAW];
	}

	if (doPitch)
	{
		// add pitch error based on NPCInfo->aim value
		error = ((float)(6 - NPCInfo->stats.aim)) * flrand(-1, 1);

		diff = AngleDelta(NPC->client->ps.viewangles[PITCH], targetPitch);

		if (diff)
			exact = qfalse;

		ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + diff + error) - client->ps.delta_angles[PITCH];
	}

	ucmd.angles[ROLL] = ANGLE2SHORT(NPC->client->ps.viewangles[ROLL]) - client->ps.delta_angles[ROLL];

	return exact;

#else

	float error = 0.0f, diff = 0.0f;
	float decay = 0.0f;
	float targetPitch = 0;
	float targetYaw = 0;
	qboolean exact = qtrue;

	// if angle changes are locked; just keep the current angles
	if (level.time < NPCS.NPCInfo->aimTime)
	{
		if (do_pitch)
			targetPitch = NPCS.NPCInfo->lockedDesiredPitch;
		if (do_yaw)
			targetYaw = NPCS.NPCInfo->lockedDesiredYaw;
	}
	else
	{
		if (do_pitch)
			targetPitch = NPCS.NPCInfo->desiredPitch;
		if (do_yaw)
			targetYaw = NPCS.NPCInfo->desiredYaw;

		if (do_pitch)
			NPCS.NPCInfo->lockedDesiredPitch = NPCS.NPCInfo->desiredPitch;
		if (do_yaw)
			NPCS.NPCInfo->lockedDesiredYaw = NPCS.NPCInfo->desiredYaw;
	}

	if (NPCS.NPCInfo->aimErrorDebounceTime < level.time)
	{
		if (Q_irand(0, 1))
		{
			NPCS.NPCInfo->lastAimErrorYaw = (float)(6 - NPCS.NPCInfo->stats.aim) * flrand(-1, 1);
		}
		if (Q_irand(0, 1))
		{
			NPCS.NPCInfo->lastAimErrorPitch = (float)(6 - NPCS.NPCInfo->stats.aim) * flrand(-1, 1);
		}
		NPCS.NPCInfo->aimErrorDebounceTime = level.time + Q_irand(250, 2000);
	}

	if (do_yaw)
	{
		// decay yaw diff
		diff = AngleDelta(NPCS.NPC->client->ps.viewangles[YAW], targetYaw);

		if (diff)
		{
			exact = qfalse;

			decay = 60.0 + 80.0;
			decay *= 50.0f / 1000.0f; //msec
			if (diff < 0.0)
			{
				diff += decay;
				if (diff > 0.0)
				{
					diff = 0.0;
				}
			}
			else
			{
				diff -= decay;
				if (diff < 0.0)
				{
					diff = 0.0;
				}
			}
		}

		// add yaw error based on NPCInfo->aim value
		error = NPCS.NPCInfo->lastAimErrorYaw;

		NPCS.ucmd.angles[YAW] = ANGLE2SHORT(targetYaw + diff + error) - NPCS.client->ps.delta_angles[YAW];
	}

	if (do_pitch)
	{
		// decay pitch diff
		diff = AngleDelta(NPCS.NPC->client->ps.viewangles[PITCH], targetPitch);
		if (diff)
		{
			exact = qfalse;

			decay = 60.0 + 80.0;
			decay *= 50.0f / 1000.0f; //msec
			if (diff < 0.0)
			{
				diff += decay;
				if (diff > 0.0)
				{
					diff = 0.0;
				}
			}
			else
			{
				diff -= decay;
				if (diff < 0.0)
				{
					diff = 0.0;
				}
			}
		}

		error = NPCS.NPCInfo->lastAimErrorPitch;

		NPCS.ucmd.angles[PITCH] = ANGLE2SHORT(targetPitch + diff + error) - NPCS.client->ps.delta_angles[PITCH];
	}

	NPCS.ucmd.angles[ROLL] = ANGLE2SHORT(NPCS.NPC->client->ps.viewangles[ROLL]) - NPCS.client->ps.delta_angles[ROLL];

	return exact;

#endif
}

//===================================================================================

/*
static void NPC_UpdateShootAngles (vec3_t angles, qboolean doPitch, qboolean doYaw )

Does update angles on shootAngles
*/

void NPC_UpdateShootAngles(vec3_t angles, const qboolean do_pitch, const qboolean do_yaw)
{
	//FIXME: shoot angles either not set right or not used!
	float error = 0.0f;
	float decay = 0.0f;
	float targetPitch = 0;
	float targetYaw = 0;

	if (do_pitch)
		targetPitch = angles[PITCH];
	if (do_yaw)
		targetYaw = angles[YAW];

	if (do_yaw)
	{
		// decay yaw error
		error = AngleDelta(NPCS.NPCInfo->shootAngles[YAW], targetYaw);
		if (error)
		{
			decay = 60.0 + 80.0 * NPCS.NPCInfo->stats.aim;
			decay *= 100.0f / 1000.0f; //msec
			if (error < 0.0)
			{
				error += decay;
				if (error > 0.0)
				{
					error = 0.0;
				}
			}
			else
			{
				error -= decay;
				if (error < 0.0)
				{
					error = 0.0;
				}
			}
		}
		NPCS.NPCInfo->shootAngles[YAW] = targetYaw + error;
	}

	if (do_pitch)
	{
		// decay pitch error
		error = AngleDelta(NPCS.NPCInfo->shootAngles[PITCH], targetPitch);
		if (error)
		{
			decay = 60.0 + 80.0 * NPCS.NPCInfo->stats.aim;
			decay *= 100.0f / 1000.0f; //msec
			if (error < 0.0)
			{
				error += decay;
				if (error > 0.0)
				{
					error = 0.0;
				}
			}
			else
			{
				error -= decay;
				if (error < 0.0)
				{
					error = 0.0;
				}
			}
		}
		NPCS.NPCInfo->shootAngles[PITCH] = targetPitch + error;
	}
}

/*
void SetTeamNumbers (void)

Sets the number of living clients on each team

FIXME: Does not account for non-respawned players!
FIXME: Don't include medics?
*/
void SetTeamNumbers(void)
{
	int i = 0;

	for (i = 0; i < TEAM_NUM_TEAMS; i++)
	{
		teamNumbers[i] = 0;
		teamStrength[i] = 0;
	}

	//OJKFIXME: clientNum 0
	for (i = 0; i < 1; i++)
	{
		const gentity_t* found = &g_entities[i];

		if (found->client)
		{
			if (found->health > 0) //FIXME: or if a player!
			{
				teamNumbers[found->client->playerTeam]++;
				teamStrength[found->client->playerTeam] += found->health;
			}
		}
	}

	for (i = 0; i < TEAM_NUM_TEAMS; i++)
	{
		//Get the average health
		teamStrength[i] = floor((float)teamStrength[i] / (float)teamNumbers[i]);
	}
}

extern stringID_table_t BSTable[];
extern stringID_table_t BSETTable[];

qboolean G_ActivateBehavior(gentity_t* self, const int bset)
{
	bState_t b_sid = -1;

	if (!self)
	{
		return qfalse;
	}

	char* bs_name = self->behaviorSet[bset];

	if (!(VALIDSTRING(bs_name)))
	{
		return qfalse;
	}

	if (self->NPC)
	{
		b_sid = (bState_t)GetIDForString(BSTable, bs_name);
	}

	if (b_sid != (bState_t)-1)
	{
		self->NPC->tempBehavior = BS_DEFAULT;
		self->NPC->behaviorState = b_sid;
	}
	else
	{
		extern void G_MissionNoteScript(int entNum, const char* script);
		extern qboolean G_MissionReplayBlocksScript(const char* script);

		G_MissionNoteScript(self->s.number, bs_name); // (Missions: the script a cutscene comes from, g_target.c)
		if (G_MissionReplayBlocksScript(bs_name))
		{
			return qtrue; // (Missions: a replayed map's first cutscene does not run again, g_ICARUScb.c)
		}
		//make the code handle the case of the scripts directory already being given
		if (!Q_strncmp(bs_name, va("%s/", Q3_SCRIPT_DIR), 8))
		{
			//already has script directory specified.
			trap->ICARUS_RunScript((sharedEntity_t*)self, bs_name);
		}
		else
		{
			trap->ICARUS_RunScript((sharedEntity_t*)self, va("%s/%s", Q3_SCRIPT_DIR, bs_name));
		}
	}
	return qtrue;
}

/*
=============================================================================

	Extended Functions

=============================================================================
*/

//rww - special system for sync'ing bone angles between client and server.
void NPC_SetBoneAngles(gentity_t* ent, const char* bone, vec3_t angles)
{
	int* thebone = &ent->s.boneIndex1;
	int* first_free = NULL;
	int i = 0;
	const int boneIndex = G_BoneIndex(bone);
	vec3_t* bone_vector = &ent->s.boneAngles1;
	vec3_t* free_bone_vec = NULL;

	while (thebone)
	{
		if (!*thebone && !first_free)
		{
			//if the value is 0 then this index is clear, we can use it if we don't find the bone we want already existing.
			first_free = thebone;
			free_bone_vec = bone_vector;
		}
		else if (*thebone)
		{
			if (*thebone == boneIndex)
			{
				//this is it
				break;
			}
		}

		switch (i)
		{
		case 0:
			thebone = &ent->s.boneIndex2;
			bone_vector = &ent->s.boneAngles2;
			break;
		case 1:
			thebone = &ent->s.boneIndex3;
			bone_vector = &ent->s.boneAngles3;
			break;
		case 2:
			thebone = &ent->s.boneIndex4;
			bone_vector = &ent->s.boneAngles4;
			break;
		default:
			thebone = NULL;
			bone_vector = NULL;
			break;
		}

		i++;
	}

	if (!thebone)
	{
		//didn't find it, create it
		if (!first_free)
		{
			//no free bones.. can't do a thing then.
			Com_Printf("WARNING: NPC has no free bone indexes\n");
			return;
		}

		thebone = first_free;

		*thebone = boneIndex;
		bone_vector = free_bone_vec;
	}

	//If we got here then we have a vector and an index.

	//Copy the angles over the vector in the entitystate, so we can use the corresponding index
	//to set the bone angles on the client.
	VectorCopy(angles, *bone_vector);

	//Now set the angles on our server instance if we have one.

	if (!ent->ghoul2)
	{
		return;
	}

	const int flags = BONE_ANGLES_POSTMULT;
	const int up = POSITIVE_X;
	const int right = NEGATIVE_Y;
	const int forward = NEGATIVE_Z;

	//first 3 bits is forward, second 3 bits is right, third 3 bits is up
	ent->s.boneOrient = forward | right << 3 | up << 6;

	trap->G2API_SetBoneAngles(ent->ghoul2, 0, bone, angles, flags, up, right, forward, NULL, 100, level.time);
}

//rww - and another method of automatically managing surface status for the client and server at once
#define TURN_ON				0x00000000
#define TURN_OFF			0x00000100

void NPC_SetSurfaceOnOff(gentity_t* ent, const char* surfaceName, const int surfaceFlags)
{
	int i = 0;
	qboolean foundIt = qfalse;

	while (i < BG_NUM_TOGGLEABLE_SURFACES && bgToggleableSurfaces[i])
	{
		if (!Q_stricmp(surfaceName, bgToggleableSurfaces[i]))
		{
			//got it
			foundIt = qtrue;
			break;
		}
		i++;
	}

	if (!foundIt)
	{
		Com_Printf("WARNING: Tried to toggle NPC surface that isn't in toggleable surface list (%s)\n", surfaceName);
		return;
	}

	if (surfaceFlags == TURN_ON)
	{
		//Make sure the entitystate values reflect this surface as on now.
		ent->s.surfacesOn |= 1 << i;
		ent->s.surfacesOff &= ~(1 << i);
	}
	else
	{
		//Otherwise make sure they're off.
		ent->s.surfacesOn &= ~(1 << i);
		ent->s.surfacesOff |= 1 << i;
	}

	if (!ent->ghoul2)
	{
		return;
	}

	trap->G2API_SetSurfaceOnOff(ent->ghoul2, surfaceName, surfaceFlags);
}

//rww - cheap check to see if an armed client is looking in our general direction
qboolean NPC_SomeoneLookingAtMe(const gentity_t* ent)
{
	int i = 0;

	while (i < MAX_CLIENTS)
	{
		const gentity_t* pEnt = &g_entities[i];

		if (pEnt && pEnt->inuse && pEnt->client && pEnt->client->sess.sessionTeam != TEAM_SPECTATOR &&
			pEnt->client->tempSpectate < level.time && !(pEnt->client->ps.pm_flags & PMF_FOLLOW) && pEnt->s.weapon !=
			WP_NONE)
		{
			if (trap->InPVS(ent->r.currentOrigin, pEnt->r.currentOrigin))
			{
				if (InFOV(ent, pEnt, 30, 30))
				{
					//I'm in a 30 fov or so cone from this player.. that's enough I guess.
					return qtrue;
				}
			}
		}

		i++;
	}

	return qfalse;
}

qboolean NPC_ClearLOS(const vec3_t start, const vec3_t end)
{
	return G_ClearLOS(NPCS.NPC, start, end);
}

qboolean NPC_ClearLOS5(const vec3_t end)
{
	return G_ClearLOS5(NPCS.NPC, end);
}

qboolean NPC_ClearLOS4(gentity_t* ent)
{
	return G_ClearLOS4(NPCS.NPC, ent);
}

qboolean NPC_ClearLOS3(const vec3_t start, gentity_t* ent)
{
	return G_ClearLOS3(NPCS.NPC, start, ent);
}

qboolean NPC_ClearLOS2(gentity_t* ent, const vec3_t end)
{
	return G_ClearLOS2(NPCS.NPC, ent, end);
}

/*
-------------------------
NPC_TargetVisible
-------------------------
*/

static qboolean NPC_TargetVisible(gentity_t* ent)
{
	//Make sure we're in a valid range
	if (DistanceSquared(ent->r.currentOrigin, NPCS.NPC->r.currentOrigin) > NPCS.NPCInfo->stats.visrange * NPCS.NPCInfo->
		stats.visrange)
		return qfalse;

	//Check our FOV
	if (InFOV(ent, NPCS.NPC, NPCS.NPCInfo->stats.hfov, NPCS.NPCInfo->stats.vfov) == qfalse)
		return qfalse;

	//Check for sight (Force Sight 3 senses through walls)
	if (!NPC_SeesThroughWalls(NPCS.NPC) && NPC_ClearLOS4(ent) == qfalse)
		return qfalse;

	return qtrue;
}

/*
-------------------------
NPC_FindNearestEnemy
-------------------------
*/

#define	MAX_RADIUS_ENTS			256	//NOTE: This can cause entities to be lost
#define NEAR_DEFAULT_RADIUS		256
extern gentity_t* G_CheckControlledTurretEnemy(const gentity_t* self, gentity_t* enemy, qboolean validate);

static int NPC_FindNearestEnemy(const gentity_t* ent)
{
	// JA:R fix: the old 256-entry list dropped entities in busy scenes (props, NPCs).
	// NPC think runs serially, so a static full-size list is safe and keeps it off the stack.
	static int iradius_ents[MAX_GENTITIES];
	vec3_t mins = { 0 }, maxs = { 0 };
	int nearestEntID = -1;
	float nearestDist = (float)WORLD_SIZE * (float)WORLD_SIZE;
	int numChecks = 0;
	int i = 0;

	//Setup the bbox to search in
	for (i = 0; i < 3; i++)
	{
		mins[i] = ent->r.currentOrigin[i] - NPCS.NPCInfo->stats.visrange;
		maxs[i] = ent->r.currentOrigin[i] + NPCS.NPCInfo->stats.visrange;
	}

	//Get a number of entities in a given space
	const int num_ents = trap->EntitiesInBox(mins, maxs, iradius_ents, MAX_GENTITIES);

	for (i = 0; i < num_ents; i++)
	{
		//nearest = G_CheckControlledTurretEnemy(ent, iradius_ents[i], qtrue);
		gentity_t* radEnt = &g_entities[iradius_ents[i]];

		//Don't consider self
		if (radEnt == ent)
			continue;

		//JA:R: cheap rejects first - scenery that can't be hurt, or farther than the best so far
		if (!radEnt->client && !radEnt->takedamage)
			continue;
		if (DistanceSquared(ent->r.currentOrigin, radEnt->r.currentOrigin) >= nearestDist)
			continue;

		//Must be valid
		if (NPC_ValidEnemy(radEnt) == qfalse)
			continue;

		numChecks++;
		//Must be visible
		if (NPC_TargetVisible(radEnt) == qfalse)
			continue;

		const float distance = DistanceSquared(ent->r.currentOrigin, radEnt->r.currentOrigin);

		//Found one closer to us
		if (distance < nearestDist)
		{
			nearestEntID = radEnt->s.number;
			nearestDist = distance;
		}
	}

	return nearestEntID;
}

/*
-------------------------
NPC_PickEnemyExt
-------------------------
*/

static gentity_t* NPC_PickEnemyExt(const qboolean check_alerts)
{
	//If we've asked for the closest enemy
	const int entID = NPC_FindNearestEnemy(NPCS.NPC);

	//If we have a valid enemy, use it
	if (entID >= 0)
		return &g_entities[entID];

	if (check_alerts)
	{
		const int alert_event = NPC_CheckAlertEvents(qtrue, qtrue, -1, qtrue, AEL_DISCOVERED);

		//There is an event to look at
		if (alert_event >= 0)
		{
			const alertEvent_t* event = &level.alertEvents[alert_event];

			//Don't pay attention to our own alerts
			if (event->owner == NPCS.NPC)
				return NULL;

			if (event->level >= AEL_DISCOVERED)
			{
				//If it's a player, attack him (JA:R fix: any client, not just client 0, for co-op)
				if (event->owner && event->owner->s.number < MAX_CLIENTS && event->owner->client && NPC_ValidEnemy(event->owner))
				{
					VectorCopy(event->position, NPCS.NPCInfo->enemyLastSeenLocation);
					NPCS.NPCInfo->enemyLastSeenTime = level.time;
					return event->owner;
				}

				//If it's on our team, then take its enemy as well
				if (event->owner && event->owner->client && event->owner->client->playerTeam == NPCS.NPC->client->playerTeam &&
					NPC_ValidEnemy(event->owner))
					return event->owner->enemy;
			}
		}
	}

	return NULL;
}

/*
-------------------------
NPC_FindPlayer
-------------------------
*/

qboolean NPC_FindPlayer(void)
{
	return NPC_TargetVisible(&g_entities[0]);
}

/*
-------------------------
NPC_CheckPlayerDistance
-------------------------
*/

static qboolean NPC_CheckPlayerDistance(void)
{
	//racc - check for a closer player to the NPC than it's current enemy.
	int ClosestPlayer = -1; //current closest player

	//Make sure we have an enemy
	if (NPCS.NPC->enemy == NULL)
		return qfalse;

	float ClosestDistance = DistanceSquared(NPCS.NPC->r.currentOrigin, NPCS.NPC->enemy->r.currentOrigin);

	//must be set up to get mad at player
	if (!NPCS.NPC->client || NPCS.NPC->client->enemyTeam != NPCTEAM_PLAYER)
		return qfalse;

	//go into the scan loop.
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		const gentity_t* player = &g_entities[i];

		if (!player->inuse || !player->client
			|| player->client->pers.connected != CON_CONNECTED
			|| player->client->sess.sessionTeam == TEAM_SPECTATOR
			|| player->health < 0
			|| (player->flags & FL_NOTARGET)) // notarget: G_SetEnemy refuses him, but we returned qtrue and kept the old enemy
		{
			//not a good player
			continue;
		}

		//Must be within our FOV and in sight, not behind a wall (DH 2026-10-24)
		if (NPC_CheckVisibility(player, CHECK_360 | CHECK_FOV) < VIS_FOV)
			continue;

		const float distance = DistanceSquared(NPCS.NPC->r.currentOrigin, player->r.currentOrigin);

		if (distance < ClosestDistance)
		{
			//we're closer than the current closest
			ClosestDistance = distance;
			ClosestPlayer = player->s.number;
		}
	}

	if (ClosestPlayer != -1)
	{
		if (ClosestDistance + 128 * 128 < DistanceSquared(NPCS.NPC->r.currentOrigin, NPCS.NPC->enemy->r.currentOrigin))
		{
			//player has too be reasonably closer than the current enemy
			G_SetEnemy(NPCS.NPC, &g_entities[ClosestPlayer]);
			return qtrue;
		}
	}

	return qfalse;
}

/*
-------------------------
NPC_FindEnemy
-------------------------
*/

static qboolean NPC_FindEnemy(const qboolean check_alerts)
{
	//We're ignoring all enemies for now
	if (NPCS.NPC->NPC->scriptFlags & SCF_IGNORE_ENEMIES)
	{
		G_ClearEnemy(NPCS.NPC);
		return qfalse;
	}

	//we can't pick up any enemies for now
	if (NPCS.NPCInfo->confusionTime > level.time)
	{
		G_ClearEnemy(NPCS.NPC);
		return qfalse;
	}

	//Don't want a new enemy
	if (ValidEnemy(NPCS.NPC->enemy) && NPCS.NPC->NPC->aiFlags2 & NPCAI2_LOCKEDENEMY)
		return qtrue;

	//See if the player is closer than our current enemy
	if (NPCS.NPC->client->NPC_class != CLASS_RANCOR
		&& NPCS.NPC->client->NPC_class != CLASS_WAMPA
		&& NPCS.NPC->client->NPC_class != CLASS_SAND_CREATURE
		&& NPC_CheckPlayerDistance())
	{
		//rancors, wampas & sand creatures don't care if player is closer, they always go with closest
		return qtrue;
	}

	NPCS.NPC->NPC->aiFlags2 &= ~NPCAI2_LOCKEDENEMY;

	//If we've gotten here alright, then our target it still valid
	if (NPC_ValidEnemy(NPCS.NPC->enemy))
		return qtrue;

	gentity_t* newenemy = NPC_PickEnemyExt(check_alerts);

	//if we found one, take it as the enemy
	if (NPC_ValidEnemy(newenemy))
	{
		G_SetEnemy(NPCS.NPC, newenemy);
		return qtrue;
	}

	G_ClearEnemy(NPCS.NPC);
	return qfalse;
}

/*
-------------------------
NPC_CheckEnemyExt
-------------------------
*/

qboolean NPC_CheckEnemyExt(const qboolean check_alerts)
{
	return NPC_FindEnemy(check_alerts);
}

/*
-------------------------
NPC_FacePosition
-------------------------
*/

// Face a world-space position, optionally including pitch.
// Behaviour preserved; only safety and structure improved.
qboolean NPC_FacePosition(vec3_t position, const qboolean do_pitch)
{
	vec3_t muzzle = { 0 };
	vec3_t angles = { 0 };
	qboolean facing = qtrue;

	// SAFETY FIX: NPC, NPC->client, NPCS.client must exist
	if (NPCS.NPC == NULL || NPCS.NPC->client == NULL || NPCS.client == NULL)
	{
		return qfalse;
	}

	// Get the muzzle/head position depending on NPC class
	if (NPCS.NPC->client->NPC_class == CLASS_RANCOR ||
		NPCS.NPC->client->NPC_class == CLASS_WAMPA)
	{
		CalcEntitySpot(NPCS.NPC, SPOT_ORIGIN, muzzle);
		muzzle[2] += NPCS.NPC->r.maxs[2] * 0.75f;
	}
	else if (NPCS.NPC->client->NPC_class == CLASS_GALAKMECH)
	{
		CalcEntitySpot(NPCS.NPC, SPOT_WEAPON, muzzle);
	}
	else
	{
		CalcEntitySpot(NPCS.NPC, SPOT_HEAD_LEAN, muzzle);

		if (NPCS.NPC->client->NPC_class == CLASS_ROCKETTROOPER)
		{
			// Look down more
			position[2] -= 32.0f;
		}
	}

	// Compute desired facing angles
	GetAnglesForDirection(muzzle, position, angles);

	NPCS.NPCInfo->desiredYaw = AngleNormalize360(angles[YAW]);
	NPCS.NPCInfo->desiredPitch = AngleNormalize360(angles[PITCH]);

	// AT-ST enemy wobble behaviour
	if (NPCS.NPC->enemy &&
		NPCS.NPC->enemy->client &&
		NPCS.NPC->enemy->client->NPC_class == CLASS_ATST)
	{
		NPCS.NPCInfo->desiredYaw += flrand(-5.0f, 5.0f) + sin(level.time * 0.004f) * 7.0f;
		NPCS.NPCInfo->desiredPitch += flrand(-2.0f, 2.0f);
	}

	// Apply desired angles
	NPC_UpdateAngles(qtrue, qtrue);

	// Compute yaw delta between desired and current
	const float currentYaw =
		SHORT2ANGLE(NPCS.ucmd.angles[YAW] + NPCS.client->ps.delta_angles[YAW]);

	const float yawDelta =
		AngleNormalize360(NPCS.NPCInfo->desiredYaw - currentYaw);

	if (fabs(yawDelta) > VALID_ATTACK_CONE)
	{
		facing = qfalse;
	}

	// Pitch check if requested
	if (do_pitch == qtrue)
	{
		const float currentPitch =
			SHORT2ANGLE(NPCS.ucmd.angles[PITCH] + NPCS.client->ps.delta_angles[PITCH]);

		const float pitchDelta = NPCS.NPCInfo->desiredPitch - currentPitch;

		if (fabs(pitchDelta) > VALID_ATTACK_CONE)
		{
			facing = qfalse;
		}
	}

	return facing;
}

/*
-------------------------
NPC_FaceEntity
-------------------------
*/

qboolean NPC_FaceEntity(const gentity_t* ent, const qboolean do_pitch)
{
	vec3_t entPos = { 0 };

	//Get the positions
	CalcEntitySpot(ent, SPOT_HEAD_LEAN, entPos);

	return NPC_FacePosition(entPos, do_pitch);
}

/*
-------------------------
NPC_FaceEnemy
-------------------------
*/

qboolean NPC_FaceEnemy(const qboolean do_pitch)
{
	if (NPCS.NPC == NULL)
		return qfalse;

	if (NPCS.NPC->enemy == NULL)
		return qfalse;

	return NPC_FaceEntity(NPCS.NPC->enemy, do_pitch);
}

/*
-------------------------
NPC_CheckCanAttackExt
-------------------------
*/

qboolean NPC_CheckCanAttackExt(void)
{
	//We don't want them to shoot
	if (NPCS.NPCInfo->scriptFlags & SCF_DONT_FIRE)
		return qfalse;

	//Turn to face
	if (NPC_FaceEnemy(qtrue) == qfalse)
		return qfalse;

	//Must have a clear line of sight to the target
	if (NPC_ClearShot(NPCS.NPC->enemy) == qfalse)
		return qfalse;

	return qtrue;
}

/*
-------------------------
NPC_ClearLookTarget
-------------------------
*/

void NPC_ClearLookTarget(const gentity_t* self)
{
	if (!self->client)
	{
		return;
	}

	if (self->client->ps.eFlags2 & EF2_HELD_BY_MONSTER)
	{
		//lookTarget is set by and to the monster that's holding you, no other operations can change that
		return;
	}

	self->client->renderInfo.lookTarget = ENTITYNUM_NONE; //ENTITYNUM_WORLD;
	self->client->renderInfo.lookTargetClearTime = 0;
}

/*
-------------------------
NPC_SetLookTarget
-------------------------
*/
void NPC_SetLookTarget(const gentity_t* self, const int entNum, const int clear_time)
{
	if (!self->client)
	{
		return;
	}

	if (self->client->ps.eFlags2 & EF2_HELD_BY_MONSTER)
	{
		//lookTarget is set by and to the monster that's holding you, no other operations can change that
		return;
	}

	self->client->renderInfo.lookTarget = entNum;
	self->client->renderInfo.lookTargetClearTime = clear_time;
}

/*
-------------------------
NPC_CheckLookTarget
-------------------------
*/
qboolean NPC_CheckLookTarget(const gentity_t* self)
{
	if (self->client)
	{
		if (self->client->renderInfo.lookTarget >= 0 && self->client->renderInfo.lookTarget < ENTITYNUM_WORLD)
		{
			//within valid range
			int lookTargetIdx = self->client->renderInfo.lookTarget;
			if (lookTargetIdx < 0 ||
				lookTargetIdx >= level.num_entities ||
				!g_entities[lookTargetIdx].inuse ||
				!g_entities[lookTargetIdx].client /* or other checks as needed */)
			{
				// handle invalid look target
				NPC_ClearLookTarget(self);
			}
			else
			{
				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
-------------------------
NPC_CheckCharmed
-------------------------
*/
extern void G_AddVoiceEvent(const gentity_t* self, int event, int speak_debounce_time);

void G_CheckCharmed(gentity_t* self)
{
	if (self
		&& self->client
		&& self->client->playerTeam == NPCTEAM_PLAYER
		&& self->NPC
		&& self->NPC->charmedTime
		&& (self->NPC->charmedTime < level.time || self->health <= 0))
	{
		//we were charmed, set us back!
		//NOTE: presumptions here...
		const team_t savTeam = (team_t)self->client->enemyTeam;
		self->client->enemyTeam = self->client->playerTeam;
		self->client->playerTeam = (npcteam_t)savTeam;
		self->client->leader = NULL;
		self->NPC->charmedTime = 0;
		if (self->health > 0)
		{
			if (self->NPC->tempBehavior == BS_FOLLOW_LEADER)
			{
				self->NPC->tempBehavior = BS_DEFAULT;
			}
			G_ClearEnemy(self);
			//say something to let player know you've snapped out of it
			G_AddVoiceEvent(self, Q_irand(EV_CONFUSE1, EV_CONFUSE3), 2000);
		}
	}
}

void G_GetBoltPosition(gentity_t* self, const int boltIndex, vec3_t pos, const int modelIndex)
{
	mdxaBone_t boltMatrix = { 0 };
	vec3_t angles = { 0 };

	if (!self || !self->inuse)
	{
		return;
	}

	if (self->client)
	{
		//clients don't actually even keep r.currentAngles maintained
		VectorSet(angles, 0, self->client->ps.viewangles[YAW], 0);
	}
	else
	{
		VectorSet(angles, 0, self->r.currentAngles[YAW], 0);
	}

	if (/*!self || ...haha (sorry, i'm tired)*/ !self->ghoul2)
	{
		return;
	}

	trap->G2API_GetBoltMatrix(self->ghoul2, modelIndex,
		boltIndex,
		&boltMatrix, angles, self->r.currentOrigin, level.time,
		NULL, self->modelScale);
	if (pos)
	{
		vec3_t result = { 0 };
		BG_GiveMeVectorFromMatrix(&boltMatrix, ORIGIN, result);
		VectorCopy(result, pos);
	}
}

float NPC_EntRangeFromBolt(const gentity_t* targ_ent, const int boltIndex)
{
	vec3_t org = { 0 };

	if (!targ_ent)
	{
		return Q3_INFINITE;
	}

	G_GetBoltPosition(NPCS.NPC, boltIndex, org, 0);

	return Distance(targ_ent->r.currentOrigin, org);
}

float NPC_EnemyRangeFromBolt(const int boltIndex)
{
	return NPC_EntRangeFromBolt(NPCS.NPC->enemy, boltIndex);
}

int NPC_GetEntsNearBolt(int* radius_ents, const float radius, const int boltIndex, vec3_t bolt_org)
{
	vec3_t mins = { 0 }, maxs = { 0 };

	//get my handRBolt's position
	vec3_t org = { 0 };

	G_GetBoltPosition(NPCS.NPC, boltIndex, org, 0);

	VectorCopy(org, bolt_org);

	//Setup the bbox to search in
	for (int i = 0; i < 3; i++)
	{
		mins[i] = bolt_org[i] - radius;
		maxs[i] = bolt_org[i] + radius;
	}

	//Get the number of entities in a given space
	return trap->EntitiesInBox(mins, maxs, radius_ents, 128);
}

extern qboolean Boba_Flying(const gentity_t* self);
extern void Boba_FlyStart(gentity_t* self);
extern void Boba_FlyStop(gentity_t* self);

qboolean JET_Flying(const gentity_t* self)
{
	if (!self || !self->client)
	{
		return qfalse;
	}
	if (self->client->NPC_class == CLASS_BOBAFETT)
	{
		return Boba_Flying(self);
	}
	if (self->client->NPC_class == CLASS_MANDO)
	{
		return Boba_Flying(self);
	}
	if (self->client->NPC_class == CLASS_ROCKETTROOPER)
	{
		return Boba_Flying(self);
	}
	return qfalse;
}

void JET_FlyStart(gentity_t* self)
{
	if (!self || !self->client)
	{
		return;
	}
	self->lastInAirTime = level.time;
	if (self->client->NPC_class == CLASS_BOBAFETT)
	{
		Boba_FlyStart(self);
	}
	else if (self->client->NPC_class == CLASS_MANDO
		|| self->client->NPC_class == CLASS_JANGO || self->client->NPC_class == CLASS_JANGODUAL) // (MD SP)
	{
		Boba_FlyStart(self);
	}
	else if (self->client->NPC_class == CLASS_ROCKETTROOPER)
	{
		Boba_FlyStart(self);
	}
}

void jet_fly_stop(gentity_t* self)
{
	if (!self || !self->client)
	{
		return;
	}
	if (self->client->NPC_class == CLASS_BOBAFETT)
	{
		Boba_FlyStop(self);
	}
	else if (self->client->NPC_class == CLASS_MANDO
		|| self->client->NPC_class == CLASS_JANGO || self->client->NPC_class == CLASS_JANGODUAL) // (MD SP)
	{
		Boba_FlyStop(self);
	}
	else if (self->client->NPC_class == CLASS_ROCKETTROOPER)
	{
		Boba_FlyStop(self);
	}
}

qboolean InPlayersPVS(vec3_t point)
{
	//checks to see if this point is visible to all the players in the game.
	int Counter = 0;

	for (; Counter < level.maxclients; Counter++)
	{
		const gentity_t* checkEnt = &g_entities[Counter];
		if (!checkEnt->inuse || !checkEnt->client
			|| checkEnt->client->pers.connected == CON_DISCONNECTED
			|| checkEnt->client->sess.sessionTeam == TEAM_SPECTATOR)
		{
			//this entity isn't going to be seeing anything
			continue;
		}

		if (trap->InPVS(point, checkEnt->client->ps.origin))
		{
			//can be seen
			return qtrue;
		}
	}

	return qfalse;
}

#endif
