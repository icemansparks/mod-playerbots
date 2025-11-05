/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "VehicleActions.h"

#include "BattlegroundIC.h"
#include "BattleGroundTactics.h"
#include "ItemVisitors.h"
#include "ObjectDefines.h"
#include "Playerbots.h"
#include "QuestValues.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include "BattlefieldMgr.h"

// Wintergrasp key position (Gate) to detect fortress pressure
static constexpr float WG_GATE_X = 5162.991f;
static constexpr float WG_GATE_Y = 2841.232f;
static constexpr float WG_GATE_Z = 410.189f;

// IoC (Isle of Conquest) vehicle entries that bots should avoid
static constexpr uint32 NPC_KEEP_CANNON = 34929; // IoC Keep Cannon
static constexpr uint32 NPC_CATAPULT = 34935;    // IoC Catapult

static inline bool HasWGRankAtLeast(Player* p)
{
    // Any of the rank auras qualifies as ranked; specific vehicle types may require higher ranks (checked below)
    return p->HasAura(WG_SPELL_RECRUIT) || p->HasAura(WG_SPELL_CORPORAL) || p->HasAura(WG_SPELL_LIEUTENANT);
}

static inline bool HasWGRankForVehicle(Player* p, uint32 vehicleEntry)
{
    // Conservative default: Corporal for catapult/demolisher, Lieutenant for siege engine
    switch (vehicleEntry)
    {
        case WG_ENTRY_SIEGE_ENGINE_A:
        case WG_ENTRY_SIEGE_ENGINE_H:
            return p->HasAura(WG_SPELL_LIEUTENANT);
        case WG_ENTRY_DEMOLISHER:
        case WG_ENTRY_CATAPULT:
            return p->HasAura(WG_SPELL_CORPORAL) || p->HasAura(WG_SPELL_LIEUTENANT);
        default:
            return HasWGRankAtLeast(p);
    }
}

// TODO methods to enter/exit vehicle should be added to BGTactics or MovementAction (so that we can better control
// whether bot is in vehicle, eg: get out of vehicle to cap flag, if we're down to final boss, etc),
// right now they will enter vehicle based only what's available here, then they're stuck in vehicle until they die
// (LeaveVehicleAction doesnt do much seeing as they, or another bot, will get in immediately after exit)
bool EnterVehicleAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    // In Wintergrasp, require rank aura before entering vehicles
    bool isInWG = (bot->GetZoneId() == WINTERGRASP_ZONE_ID);
    if (isInWG && !HasWGRankAtLeast(bot))
        return false;

    Player* master = botAI->GetMaster();
    // Triggered by a chat command
    if (event.getOwner() && master && master->GetTarget())
    {
        Unit* vehicleBase = botAI->GetUnit(master->GetTarget());
        if (!vehicleBase)
            return false;
        Vehicle* veh = vehicleBase->GetVehicleKit();
        if (vehicleBase->IsVehicle() && veh && veh->GetAvailableSeatCount())
        {
            return EnterVehicle(vehicleBase, false);
        }
        return false;
    }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");

    // Prefer tower cannons for defenders when enemies are attacking near the fortress gate
    bool preferCannons = false;
    bool isWGDefender = false;
    if (isInWG)
    {
        if (Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(WINTERGRASP_ZONE_ID))
        {
            // Do not enter vehicles during WG preparation (no wartime)
            if (!bf->IsWarTime())
                return false;

            isWGDefender = (bf->GetDefenderTeam() == bot->GetTeamId());

            if (isWGDefender)
            {
                if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
                {
                    float dGateEnemy = enemy->GetDistance(WG_GATE_X, WG_GATE_Y, WG_GATE_Z);
                    preferCannons = (dGateEnemy < 250.0f);
                }
            }
        }
        else
        {
            return false; // no battlefield context, avoid vehicles in WG zone
        }
    }

    if (preferCannons)
    {
        Unit* bestCannon = nullptr;
        float bestDist = FLT_MAX;
        for (auto const& guid : npcs)
        {
            Unit* v = botAI->GetUnit(guid);
            if (!v)
                continue;
            if (v->GetEntry() != WG_TOWER_CANNON_ENTRY)
                continue;
            if (!v->IsFriendlyTo(bot))
                continue;
            Vehicle* veh = v->GetVehicleKit();
            if (!veh || !veh->GetAvailableSeatCount() || veh->IsVehicleInUse())
                continue;
            float d = sServerFacade->GetDistance2d(bot, v);
            if (d < bestDist)
            {
                bestDist = d;
                bestCannon = v;
            }
        }
        if (bestCannon)
        {
            if (EnterVehicle(bestCannon, true))
                return true;
        }
    }
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        // Don't let them get in IoC cannons (Keep Cannon & Catapult); allow WG tower cannons for defenders only
        uint32 entry = vehicleBase->GetEntry();
        if (NPC_KEEP_CANNON == entry || NPC_CATAPULT == entry || (!isWGDefender && entry == WG_TOWER_CANNON_ENTRY))
            continue;

        // Enforce WG per-vehicle rank requirements when in WG
        if (isInWG)
        {
            if (!HasWGRankForVehicle(bot, entry))
                continue;
        }

        if (!vehicleBase->IsFriendlyTo(bot))
            continue;

        if (!vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        // this will avoid adding passengers (which dont really do much for the IOC vehicles which is the only place
        // this code is used)
        if (vehicleBase->GetVehicleKit()->IsVehicleInUse())
            continue;

        if (EnterVehicle(vehicleBase, true))
            return true;
    }

    return false;
}

bool EnterVehicleAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = sServerFacade->GetDistance2d(bot, vehicleBase);
    if (dist > 40.0f)
        return false;

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);
    // Use HandleSpellClick instead of Unit::EnterVehicle to handle special vehicle script (ulduar)
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // dismount because bots can enter vehicle on mount
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool LeaveVehicleAction::Execute(Event event)
{
    Vehicle* myVehicle = bot->GetVehicle();
    if (!myVehicle)
        return false;

    VehicleSeatEntry const* seat = myVehicle->GetSeatForPassenger(bot);
    if (!seat || !seat->CanEnterOrExit())
        return false;

    WorldPacket p;
    bot->GetSession()->HandleRequestVehicleExit(p);

    return true;
}
