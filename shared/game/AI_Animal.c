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

// AI_Animal for both games. SP's version is built on C++ classes that the MP game rewrote in C, so this file
// still holds both complete versions; was code/game/AI_Animal.cpp and codemp/game/NPC_AI_Animal.c.

#include "game/g_npcshared.h"

#ifdef SP_GAME

#include "game/b_local.h"
#include "game/g_navigator.h"

#if !defined(RAVL_VEC_INC)
#include "Ravl/CVec.h"
#endif
#include "Ravl/CVec.h"
#include "Ratl/vector_vs.h"
#include "game/bg_public.h"
#include <qcommon/q_platform.h>
#include <qcommon/q_math.h>
#include "game/g_local.h"
#include <cmath>
#include "game/b_public.h"

constexpr auto MAX_PACKS = 10;

constexpr auto LEAVE_PACK_DISTANCE = 1000;
constexpr auto JOIN_PACK_DISTANCE = 800;
constexpr auto WANDER_RANGE = 1000;
constexpr auto FRIGHTEN_DISTANCE = 300;

extern qboolean G_PlayerSpawned();

ratl::vector_vs<gentity_t*, MAX_PACKS> mPacks;

////////////////////////////////////////////////////////////////////////////////////////
// Update The Packs, Delete Dead Leaders, Join / Split Packs, Find MY Leader
////////////////////////////////////////////////////////////////////////////////////////
static gentity_t* NPC_AnimalUpdateLeader()
{
	// NPC must exist and must have a client
	if (!NPC || !NPC->client)
	{
		return NULL;
	}

	gentity_t* closest_leader = NULL;
	float closest_dist = 0.0f;
	int my_leader_num = -1;

	for (int i = 0; i < mPacks.size(); )
	{
		gentity_t* ent = mPacks[i];

		// Remove dead or invalid leaders
		if (!ent || ent->health <= 0)
		{
			if (NPC->client->leader == ent)
			{
				NPC->client->leader = NULL;
			}

			mPacks.erase_swap(i);
			continue; // do NOT increment i after erase_swap
		}

		// Skip self
		if (ent == NPC)
		{
			my_leader_num = i;
			i++;
			continue;
		}

		// Compute distance
		float dist = Distance(ent->currentOrigin, NPC->currentOrigin);

		if (!closest_leader || dist < closest_dist)
		{
			closest_leader = ent;
			closest_dist = dist;
		}

		i++;
	}

	// Join a nearby leader
	if (closest_leader && closest_dist < JOIN_PACK_DISTANCE)
	{
		// If I was a leader, remove myself from the pack list
		if (NPC->client->leader == NPC && my_leader_num >= 0 && my_leader_num < mPacks.size())
		{
			mPacks.erase_swap(my_leader_num);
		}

		NPC->client->leader = closest_leader;
	}

	// If I have a leader, validate it
	if (NPC->client->leader)
	{
		gentity_t* L = NPC->client->leader;

		// Leader dead or removed?
		if (!L->inuse || L->health <= 0)
		{
			NPC->client->leader = NULL;
		}
		// Leader has a leader? Follow the chain
		else if (L->client && L->client->leader != L)
		{
			NPC->client->leader = L->client->leader;
		}
		// Leader too far away?
		else if (Distance(L->currentOrigin, NPC->currentOrigin) > LEAVE_PACK_DISTANCE)
		{
			NPC->client->leader = NULL;
		}
	}
	else
	{
		// No leader → become one
		if (!mPacks.full())
		{
			NPC->client->leader = NPC;
			mPacks.push_back(NPC);
		}
	}

	return NPC->client->leader;
}

/*
-------------------------
NPC_BSAnimal_Default
-------------------------
*/
void NPC_BSAnimal_Default()
{
	if (!NPC || !NPC->client)
	{
		return;
	}

	// Update Some Positions
	//-----------------------
	const CVec3 current_location(NPC->currentOrigin);

	// Update The Leader
	//-------------------
	gentity_t* leader = NPC_AnimalUpdateLeader();

	// Select Closest Threat Location
	//--------------------------------
	CVec3 threat_location{};
	const qboolean player_spawned = G_PlayerSpawned();
	if (player_spawned)
	{
		//player is actually in the level now
		threat_location = player->currentOrigin;
	}
	const int alert_event = NPC_CheckAlertEvents(qtrue, qtrue, -1, qfalse, AEL_MINOR, qfalse);
	if (alert_event >= 0)
	{
		const alertEvent_t* event = &level.alertEvents[alert_event];
		if (event->owner != NPC && Distance(event->position, current_location.v) < event->radius)
		{
			threat_location = event->position;
		}
	}

	const bool evade_threat = level.time < NPCInfo->investigateSoundDebounceTime;
	const bool charmed_docile = level.time < NPCInfo->confusionTime;
	const bool charmed_approach = level.time < NPCInfo->charmedTime;

	STEER::Activate(NPC);
	{
		// Charmed Approach - Walk TOWARD The Threat Location
		//----------------------------------------------------
		if (charmed_approach)
		{
			NAV::GoTo(NPC, NPCInfo->investigateGoal);
		}

		// Charmed Docile - Stay Put
		//---------------------------
		else if (charmed_docile)
		{
			NAV::ClearPath(NPC);
			STEER::Stop(NPC);
		}

		// Run Away From This Threat
		//---------------------------
		else if (evade_threat)
		{
			NAV::ClearPath(NPC);
			STEER::Flee(NPC, NPCInfo->investigateGoal);
		}

		// Normal Behavior
		//-----------------
		else
		{
			// Follow Our Pack Leader!
			//-------------------------
			if (leader && leader != NPC)
			{
				constexpr float follow_dist = 100.0f;
				const float cur_dist = Distance(NPC->currentOrigin, leader->followPos);

				// Update The Leader's Follow Position
				//-------------------------------------
				STEER::FollowLeader(NPC, leader, follow_dist);

				const bool in_seek_range = cur_dist < follow_dist * 10.0f;
				const bool on_nbr_points =
					NAV::OnNeighboringPoints(NAV::GetNearestNode(NPC), leader->followPosWaypoint);
				const bool leader_stop = level.time - leader->lastMoveTime > 500;

				// If Close Enough, Dump Any Existing Path
				//-----------------------------------------
				if (in_seek_range || on_nbr_points)
				{
					NAV::ClearPath(NPC);

					// If The Leader Isn't Moving, Stop
					//----------------------------------
					if (leader_stop)
					{
						STEER::Stop(NPC);
					}

					// Otherwise, Try To Get To The Follow Position
					//----------------------------------------------
					else
					{
						STEER::Seek(NPC, leader->followPos, fabsf(follow_dist) / 2.0f/*slowing distance*/,
							1.0f/*wight*/,
							leader->resultspeed);
					}
				}

				// Otherwise, Get A Path To The Follow Position
				//----------------------------------------------
				else
				{
					NAV::GoTo(NPC, leader->followPosWaypoint);
				}
				STEER::Separation(NPC, 4.0f);
				STEER::AvoidCollisions(NPC, leader);
			}

			// Leader AI - Basically Wander
			//------------------------------
			else
			{
				// Are We Doing A Path?
				//----------------------
				bool has_path = NAV::HasPath(NPC);
				if (has_path)
				{
					has_path = NAV::UpdatePath(NPC);
					if (has_path)
					{
						STEER::Path(NPC); // Follow The Path
						STEER::AvoidCollisions(NPC);
					}
				}

				if (!has_path)
				{
					// If Debounce Time Has Expired, Choose A New Sub State
					//------------------------------------------------------
					if (NPCInfo->investigateDebounceTime < level.time)
					{
						// Clear Out Flags From The Previous Substate
						//--------------------------------------------
						NPCInfo->aiFlags &= ~NPCAI_OFF_PATH;
						NPCInfo->aiFlags &= ~NPCAI_WALKING;

						// Pick Another Spot
						//-------------------
						const int NEXTSUBSTATE = Q_irand(0, 10);

						const bool random_path_node = NEXTSUBSTATE < 8; //(NEXTSUBSTATE<9);
						const bool pathless_wander = NEXTSUBSTATE < 9; //false;

						// Random Path Node
						//------------------
						if (random_path_node)
						{
							// Sometimes, Walk
							//-----------------
							if (Q_irand(0, 1) == 0)
							{
								NPCInfo->aiFlags |= NPCAI_WALKING;
							}

							NPCInfo->investigateDebounceTime = level.time + Q_irand(3000, 10000);
							NAV::FindPath(NPC, NAV::ChooseRandomNeighbor(NAV::GetNearestNode(NPC)));
							//, mHome.v, WANDER_RANGE));
						}

						// Pathless Wandering
						//--------------------
						else if (pathless_wander)
						{
							// Sometimes, Walk
							//-----------------
							if (Q_irand(0, 1) == 0)
							{
								NPCInfo->aiFlags |= NPCAI_WALKING;
							}

							NPCInfo->investigateDebounceTime = level.time + Q_irand(3000, 10000);
							NPCInfo->aiFlags |= NPCAI_OFF_PATH;
						}

						// Just Stand Here
						//-----------------
						else
						{
							NPCInfo->investigateDebounceTime = level.time + Q_irand(2000, 6000);
							//NPC_SetAnim(NPC, SETANIM_BOTH, ((Q_irand(0, 1)==0)?(BOTH_GUARD_LOOKAROUND1):(BOTH_GUARD_IDLE1)), SETANIM_FLAG_NORMAL);
						}
					}

					// Ok, So We Don't Have A Path, And Debounce Time Is Still Active, So We Are Either Wandering Or Looking Around
					//--------------------------------------------------------------------------------------------------------------
					else
					{
						//	if (DistFromHome>(WANDER_RANGE))
						//	{
						//		STEER::Seek(NPC, mHome);
						//	}
						//	else
						{
							if (NPCInfo->aiFlags & NPCAI_OFF_PATH)
							{
								STEER::Wander(NPC);
								STEER::AvoidCollisions(NPC);
							}
							else
							{
								STEER::Stop(NPC);
							}
						}
					}
				}
			}
		}
	}
	STEER::DeActivate(NPC, &ucmd);

	NPC_UpdateAngles(qtrue, qtrue);
}

#else // MP game

// leave this line at the top of all AI_xxxx.cpp files for PCH reasons...
#include "game/b_local.h"
#include "game/NPC_steer.h"

/*
-------------------------
Animals: SP's AI_Animal.cpp (DH 2026-11-03, NPC parity batch C)

As SP, a riderless animal vehicle (NPC.c NPC_BehaviorSet_Animal) runs this. Animals keep to packs: up to MAX_PACKS
leaders, an animal within JOIN_PACK_DISTANCE of a leader follows him (and a leader that finds another one joins his
pack), one more than LEAVE_PACK_DISTANCE away leaves; without a pack it leads its own. A follower keeps to its leader's
follow position (STEER_FollowLeader) and stops when he stops, a leader wanders: to a random neighbouring waypoint, about
without a path, or stands still a while. A charmed animal walks to what charmed it (charmedTime), a docile one
(confusionTime) stays put, a frightened one (investigateSoundDebounceTime) runs from investigateGoal.
MP has no navigator paths: the "path" is the chosen waypoint, followed with NAV_GoToPos until reached (NPC_steer.c).
-------------------------
*/

#define MAX_PACKS 10

#define LEAVE_PACK_DISTANCE 1000
#define JOIN_PACK_DISTANCE 800
#define ANIMAL_PATH_REACHED 40.0f

static gentity_t* mPacks[MAX_PACKS];
static int mNumPacks;
static int mPacksLevelStart = -1; // (level.startTime the packs belong to: emptied on a new map)

// the waypoint a leader is walking to, + 1 (0: none) (SP NAV::FindPath / HasPath / UpdatePath)
static int animalPathNode[MAX_GENTITIES];
static int animalPathFreeTime[MAX_GENTITIES];

static void NPC_AnimalPackRemove(const int i)
{
	// (ratl erase_swap)
	mPacks[i] = mPacks[mNumPacks - 1];
	mPacks[--mNumPacks] = NULL;
}

static qboolean NPC_AnimalHasPath(const gentity_t* ent)
{
	return animalPathFreeTime[ent->s.number] == ent->freetime && animalPathNode[ent->s.number] > 0
		? qtrue
		: qfalse;
}

static void NPC_AnimalClearPath(const gentity_t* ent)
{
	animalPathNode[ent->s.number] = 0;
	animalPathFreeTime[ent->s.number] = ent->freetime;
}

static void NPC_AnimalFindPath(const gentity_t* ent, const int node)
{
	animalPathNode[ent->s.number] = node >= 0 ? node + 1 : 0;
	animalPathFreeTime[ent->s.number] = ent->freetime;
}

// follows the path: qfalse when there is none (any more)
static qboolean NPC_AnimalUpdatePath(gentity_t* ent)
{
	vec3_t pos;
	if (!NAV_NodePosition(animalPathNode[ent->s.number] - 1, pos)
		|| STEER_Reached(ent, pos, ANIMAL_PATH_REACHED)
		|| !NAV_GoToPos(ent, pos, ANIMAL_PATH_REACHED))
	{
		NPC_AnimalClearPath(ent);
		return qfalse;
	}
	return qtrue;
}

////////////////////////////////////////////////////////////////////////////////////////
// Update The Packs, Delete Dead Leaders, Join / Split Packs, Find MY Leader
////////////////////////////////////////////////////////////////////////////////////////
static gentity_t* NPC_AnimalUpdateLeader(void)
{
	// NPC must exist and must have a client
	if (!NPCS.NPC || !NPCS.NPC->client)
	{
		return NULL;
	}
	if (mPacksLevelStart != level.startTime)
	{
		memset(mPacks, 0, sizeof mPacks);
		mNumPacks = 0;
		mPacksLevelStart = level.startTime;
	}

	gentity_t* closestLeader = NULL;
	float closestDist = 0.0f;
	int myLeaderNum = -1;

	for (int i = 0; i < mNumPacks;)
	{
		gentity_t* ent = mPacks[i];

		// Remove dead or invalid leaders
		if (!ent || !ent->inuse || !ent->client || ent->health <= 0)
		{
			if (NPCS.NPC->client->leader == ent)
			{
				NPCS.NPC->client->leader = NULL;
			}

			NPC_AnimalPackRemove(i);
			continue; // do NOT increment i after erase_swap
		}

		// Skip self
		if (ent == NPCS.NPC)
		{
			myLeaderNum = i;
			i++;
			continue;
		}

		// Compute distance
		const float dist = Distance(ent->r.currentOrigin, NPCS.NPC->r.currentOrigin);

		if (!closestLeader || dist < closestDist)
		{
			closestLeader = ent;
			closestDist = dist;
		}

		i++;
	}

	// Join a nearby leader
	if (closestLeader && closestDist < JOIN_PACK_DISTANCE)
	{
		// If I was a leader, remove myself from the pack list
		if (NPCS.NPC->client->leader == NPCS.NPC && myLeaderNum >= 0 && myLeaderNum < mNumPacks)
		{
			NPC_AnimalPackRemove(myLeaderNum);
		}

		NPCS.NPC->client->leader = closestLeader;
	}

	// If I have a leader, validate it
	if (NPCS.NPC->client->leader)
	{
		gentity_t* L = NPCS.NPC->client->leader;

		// Leader dead or removed?
		if (!L->inuse || L->health <= 0)
		{
			NPCS.NPC->client->leader = NULL;
		}
		// Leader has a leader? Follow the chain
		else if (L->client && L->client->leader && L->client->leader != L)
		{
			NPCS.NPC->client->leader = L->client->leader;
		}
		// Leader too far away?
		else if (Distance(L->r.currentOrigin, NPCS.NPC->r.currentOrigin) > LEAVE_PACK_DISTANCE)
		{
			NPCS.NPC->client->leader = NULL;
		}
	}
	else
	{
		// No leader - become one
		if (mNumPacks < MAX_PACKS)
		{
			NPCS.NPC->client->leader = NPCS.NPC;
			mPacks[mNumPacks++] = NPCS.NPC;
		}
	}

	return NPCS.NPC->client->leader;
}

/*
-------------------------
NPC_BSAnimal_Default
-------------------------
*/
void NPC_BSAnimal_Default(void)
{
	if (!NPCS.NPC || !NPCS.NPC->client)
	{
		return;
	}

	// Update The Leader
	//-------------------
	gentity_t* leader = NPC_AnimalUpdateLeader();

	// (SP worked out the closest threat here too, but never used it)
	NPC_CheckAlertEvents(qtrue, qtrue, -1, qfalse, AEL_MINOR);

	const qboolean evadeThreat = level.time < NPCS.NPCInfo->investigateSoundDebounceTime ? qtrue : qfalse;
	const qboolean charmedDocile = level.time < NPCS.NPCInfo->confusionTime ? qtrue : qfalse;
	const qboolean charmedApproach = level.time < NPCS.NPCInfo->charmedTime ? qtrue : qfalse;

	STEER_Activate(NPCS.NPC);
	{
		// Charmed Approach - Walk TOWARD The Threat Location
		//----------------------------------------------------
		if (charmedApproach)
		{
			NAV_GoToPos(NPCS.NPC, NPCS.NPCInfo->investigateGoal, ANIMAL_PATH_REACHED);
		}

		// Charmed Docile - Stay Put
		//---------------------------
		else if (charmedDocile)
		{
			NPC_AnimalClearPath(NPCS.NPC);
			STEER_Stop(NPCS.NPC);
		}

		// Run Away From This Threat
		//---------------------------
		else if (evadeThreat)
		{
			NPC_AnimalClearPath(NPCS.NPC);
			STEER_Flee(NPCS.NPC, NPCS.NPCInfo->investigateGoal, 1.0f);
		}

		// Normal Behavior
		//-----------------
		else
		{
			// Follow Our Pack Leader!
			//-------------------------
			if (leader && leader != NPCS.NPC)
			{
				const float followDist = 100.0f;
				const float curDist = Distance(NPCS.NPC->r.currentOrigin, STEER_FollowPos(leader));

				// Update The Leader's Follow Position
				//-------------------------------------
				STEER_FollowLeader(NPCS.NPC, leader, followDist);

				const qboolean inSeekRange = curDist < followDist * 10.0f ? qtrue : qfalse;
				const qboolean leaderStop = level.time - STEER_LastMoveTime(leader) > 500 ? qtrue : qfalse;

				// If Close Enough, Dump Any Existing Path
				//-----------------------------------------
				if (inSeekRange)
				{
					NPC_AnimalClearPath(NPCS.NPC);

					// If The Leader Isn't Moving, Stop
					//----------------------------------
					if (leaderStop)
					{
						STEER_Stop(NPCS.NPC);
					}

					// Otherwise, Try To Get To The Follow Position
					//----------------------------------------------
					else
					{
						STEER_Seek(NPCS.NPC, STEER_FollowPos(leader), fabsf(followDist) / 2.0f/*slowing distance*/,
							1.0f/*wight*/,
							STEER_ResultSpeed(leader));
					}
				}

				// Otherwise, Get A Path To The Follow Position
				//----------------------------------------------
				else
				{
					NAV_GoToPos(NPCS.NPC, STEER_FollowPos(leader), ANIMAL_PATH_REACHED);
				}
				STEER_Separation(NPCS.NPC, 4.0f);
				STEER_AvoidCollisions(NPCS.NPC, leader);
			}

			// Leader AI - Basically Wander
			//------------------------------
			else
			{
				// Are We Doing A Path?
				//----------------------
				qboolean hasPath = NPC_AnimalHasPath(NPCS.NPC);
				if (hasPath)
				{
					hasPath = NPC_AnimalUpdatePath(NPCS.NPC); // Follow The Path
				}

				if (!hasPath)
				{
					// If Debounce Time Has Expired, Choose A New Sub State
					//------------------------------------------------------
					if (NPCS.NPCInfo->investigateDebounceTime < level.time)
					{
						// Clear Out Flags From The Previous Substate
						//--------------------------------------------
						NPCS.NPCInfo->aiFlags &= ~NPCAI_OFF_PATH;
						NPCS.NPCInfo->aiFlags &= ~NPCAI_WALKING;

						// Pick Another Spot
						//-------------------
						const int NEXTSUBSTATE = Q_irand(0, 10);

						const qboolean randomPathNode = NEXTSUBSTATE < 8 ? qtrue : qfalse; //(NEXTSUBSTATE<9);
						const qboolean pathlessWander = NEXTSUBSTATE < 9 ? qtrue : qfalse; //false;

						// Random Path Node
						//------------------
						if (randomPathNode)
						{
							// Sometimes, Walk
							//-----------------
							if (Q_irand(0, 1) == 0)
							{
								NPCS.NPCInfo->aiFlags |= NPCAI_WALKING;
							}

							NPCS.NPCInfo->investigateDebounceTime = level.time + Q_irand(3000, 10000);
							NPC_AnimalFindPath(NPCS.NPC, NAV_ChooseRandomNeighbor(NAV_NearestNode(NPCS.NPC)));
						}

						// Pathless Wandering
						//--------------------
						else if (pathlessWander)
						{
							// Sometimes, Walk
							//-----------------
							if (Q_irand(0, 1) == 0)
							{
								NPCS.NPCInfo->aiFlags |= NPCAI_WALKING;
							}

							NPCS.NPCInfo->investigateDebounceTime = level.time + Q_irand(3000, 10000);
							NPCS.NPCInfo->aiFlags |= NPCAI_OFF_PATH;
						}

						// Just Stand Here
						//-----------------
						else
						{
							NPCS.NPCInfo->investigateDebounceTime = level.time + Q_irand(2000, 6000);
						}
					}

					// Ok, So We Don't Have A Path, And Debounce Time Is Still Active, So We Are Either Wandering Or Looking Around
					//--------------------------------------------------------------------------------------------------------------
					else
					{
						if (NPCS.NPCInfo->aiFlags & NPCAI_OFF_PATH)
						{
							STEER_Wander(NPCS.NPC);
							STEER_AvoidCollisions(NPCS.NPC, NULL);
						}
						else
						{
							STEER_Stop(NPCS.NPC);
						}
					}
				}
			}
		}
	}
	STEER_DeActivate(NPCS.NPC, &NPCS.ucmd);

	NPC_UpdateAngles(qtrue, qtrue);
}

#endif
