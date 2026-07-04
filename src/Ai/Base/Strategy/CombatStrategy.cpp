/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CombatStrategy.h"

#include "Strategy.h"

void CombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Opt-in (AiPlayerbot.CombatPrioritizeMaster): while in combat and beyond TooCloseDistance from
    // the master, run back to the master instead of chasing/holding on a mob the bot aggroed itself.
    // Relevance ACTION_HIGH+3 (23) outranks "reach melee" (21) / "reach spell" (20) so the return
    // wins each tick while far. "follow master combat" follows the bot's OWN master (not the party
    // leader) and stands down when the master is on the bot's target, so shared fights still assist.
    // The "combat follow master" trigger is inert unless the config is enabled.
    triggers.push_back(
        new TriggerNode(
            "combat follow master",
            {
                NextAction("follow master combat", ACTION_HIGH + 3)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of spell",
            {
                NextAction("reach spell", ACTION_HIGH)
            }
        )
    );
    // drop target relevance 99 (lower than Worldpacket triggers)
    triggers.push_back(
        new TriggerNode(
            "invalid target",
            {
                NextAction("drop target", 99)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mounted",
            {
                NextAction("check mount state", 54)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combat stuck",
            {
                NextAction("reset", 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not facing target",
            {
                NextAction("set facing", ACTION_MOVE + 7)
            }
        )
    );
    // The pet-attack trigger is commented out because it was forcing the bot's pet to attack, overriding stay and follow commands.
    // Pets will automatically attack the bot's enemy if they are in "defensive" or "aggressive"
    // stance, or if the master issues an attack command.
}

AvoidAoeStrategy::AvoidAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> AvoidAoeStrategy::getDefaultActions()
{
    return {
        NextAction("avoid aoe", ACTION_EMERGENCY)
    };
}

void AvoidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

void AvoidAoeStrategy::InitMultipliers(std::vector<Multiplier*>& /*multipliers*/)
{
}

TankFaceStrategy::TankFaceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> TankFaceStrategy::getDefaultActions()
{
    return {
        NextAction("tank face", ACTION_MOVE)
    };
}

void TankFaceStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

std::vector<NextAction> CombatFormationStrategy::getDefaultActions()
{
    return {
        NextAction("combat formation move", ACTION_NORMAL)
    };
}
