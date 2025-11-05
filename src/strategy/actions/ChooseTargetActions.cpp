/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChooseTargetActions.h"

#include "ChooseRpgTargetAction.h"
#include "Event.h"
#include "LootObjectStack.h"
#include "NewRpgStrategy.h"
#include "Playerbots.h"
#include "PossibleRpgTargetsValue.h"
#include "PvpTriggers.h"
#include "ServerFacade.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "ThreatMgr.h"

static constexpr float kAssistRange = 30.0f;

// Determines whether engaging the target is acceptable despite level differences.
static bool IsAcceptableWorldPvpTarget(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    if (!bot || !botAI || !target)
        return false;

    if (!target->IsPlayer())
        return true;  // PvE targets keep original behavior; only player-vs-player gets gated.

    // Target is already fighting a friendly unit (pet, player, etc.).
    if (Unit* victim = target->GetVictim())
    {
        if (bot->IsFriendlyTo(victim))
            return true;
    }

    // Any current friendly attackers on the target also justify joining the fight.
    for (Unit* attacker : target->getAttackers())
    {
        if (!attacker || !attacker->IsAlive())
            continue;

        if (bot->IsFriendlyTo(attacker))
            return true;
    }

    // Count nearby players around the target to determine friendly advantage.
    uint32 friendlyNearby = bot->IsWithinDistInMap(target, kAssistRange) ? 1 : 0;
    uint32 enemyNearby = 0;

    GuidVector const& friendlyGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (ObjectGuid const& guid : friendlyGuids)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        if (!unit || !unit->IsAlive() || unit == bot)
            continue;

        if (!unit->IsWithinDistInMap(target, kAssistRange))
            continue;

        if (bot->IsFriendlyTo(unit))
        {
            ++friendlyNearby;

            // Friendlies actively tanking the target mean it is already an ongoing fight.
            if (unit->GetVictim() == target)
                return true;
        }
    }

    GuidVector const& enemyGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest enemy players")->Get();
    for (ObjectGuid const& guid : enemyGuids)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, guid);
        if (!unit || !unit->IsAlive() || unit == target)
            continue;

        if (!unit->IsWithinDistInMap(target, kAssistRange))
            continue;

        ++enemyNearby;
    }

    if (friendlyNearby > enemyNearby)
        return true;

    // Retaliate if the target has already generated threat on the bot or its party.
    ThreatMgr& threatMgr = target->GetThreatMgr();
    if (threatMgr.GetThreat(bot) > 0.0f)
        return true;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
        {
            Player* member = memberRef->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            if (threatMgr.GetThreat(member) > 0.0f)
                return true;
        }
    }

    // Enforce basic sanity checks so bots mirror typical player expectations in lopsided level fights.
    if (target->GetLevel() > bot->getLevel() + 10)
        return false;

    if (target->GetLevel() < bot->getLevel() - 10)
        return false;

    return true;
}

bool AttackEnemyPlayerAction::isUseful()
{
    if (PlayerHasFlag::IsCapturingFlag(bot))
        return false;

    return !sPlayerbotAIConfig->IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId());
}

bool AttackEnemyFlagCarrierAction::isUseful()
{
    Unit* target = context->GetValue<Unit*>("enemy flag carrier")->Get();
    return target && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target), 100.0f) &&
           PlayerHasFlag::IsCapturingFlag(bot);
}

bool AttackAnythingAction::isUseful()
{
    if (!bot || !botAI)  // Prevents invalid accesses
        return false;

    if (!botAI->AllowActivity(GRIND_ACTIVITY))  // Bot cannot be active
        return false;

    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT))
        return false;

    if (bot->IsInCombat())
        return false;

    Unit* target = GetTarget();
    if (!target || !target->IsInWorld())  // Checks if the target is valid and in the world
        return false;

    std::string const name = std::string(target->GetName());
    if (!name.empty() &&
        (name.find("Dummy") != std::string::npos ||
         name.find("Charge Target") != std::string::npos ||
         name.find("Melee Target") != std::string::npos ||
         name.find("Ranged Target") != std::string::npos))
    {
        return false;
    }

    if (!IsAcceptableWorldPvpTarget(bot, botAI, target))
        return false;

    return true;
}

bool DropTargetAction::Execute(Event event)
{
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    if (target && target->isDead())
    {
        ObjectGuid guid = target->GetGUID();
        if (guid)
            context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);
    }

    // ObjectGuid pullTarget = context->GetValue<ObjectGuid>("pull target")->Get();
    // GuidVector possible = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    // if (pullTarget && find(possible.begin(), possible.end(), pullTarget) == possible.end())
    // {
    //     context->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
    // }

    context->GetValue<Unit*>("current target")->Set(nullptr);

    bot->SetTarget(ObjectGuid::Empty);
    bot->SetSelection(ObjectGuid());
    botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
    if (bot->getClass() == CLASS_HUNTER) // Check for Hunter Class
    {
        Spell const* spell = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL); // Get the current spell being cast by the bot
        if (spell && spell->m_spellInfo->Id == 75) //Check spell is not nullptr before accessing m_spellInfo
        {
            bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL); // Interrupt Auto Shot
        }
    }
    bot->AttackStop();

    // if (Pet* pet = bot->GetPet())
    // {
    //     if (CreatureAI* creatureAI = ((Creature*)pet)->AI())
    //     {
    //         pet->SetReactState(REACT_PASSIVE);
    //         pet->GetCharmInfo()->SetCommandState(COMMAND_FOLLOW);
    //         pet->GetCharmInfo()->SetIsCommandFollow(true);
    //         pet->AttackStop();
    //         pet->GetCharmInfo()->IsReturning();
    //         pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST, pet->GetFollowAngle());
    //     }
    // }

    return true;
}

bool AttackAnythingAction::Execute(Event event)
{
    bool result = AttackAction::Execute(event);
    if (result)
    {
        if (Unit* grindTarget = GetTarget())
        {
            if (char const* grindName = grindTarget->GetName().c_str())
            {
                context->GetValue<ObjectGuid>("pull target")->Set(grindTarget->GetGUID());
                bot->GetMotionMaster()->Clear();
                // bot->StopMoving();
            }
        }
    }

    return result;
}

bool AttackAnythingAction::isPossible() { return AttackAction::isPossible() && GetTarget(); }

bool DpsAssistAction::isUseful()
{
    if (PlayerHasFlag::IsCapturingFlag(bot))
        return false;

    return true;
}

bool AttackRtiTargetAction::Execute(Event event)
{
    Unit* rtiTarget = AI_VALUE(Unit*, "rti target");

    if (rtiTarget && rtiTarget->IsInWorld() && rtiTarget->GetMapId() == bot->GetMapId())
    {
        botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({rtiTarget->GetGUID()});
        bool result = Attack(botAI->GetUnit(rtiTarget->GetGUID()));
        if (result)
        {
            context->GetValue<ObjectGuid>("pull target")->Set(rtiTarget->GetGUID());
            return true;
        }
    }
    else
    {
        botAI->TellError("I dont see my rti attack target");
    }

    return false;
}

bool AttackRtiTargetAction::isUseful()
{
    if (botAI->ContainsStrategy(STRATEGY_TYPE_HEAL))
        return false;

    return true;
}
