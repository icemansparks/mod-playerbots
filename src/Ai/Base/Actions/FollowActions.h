/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_FOLLOWACTIONS_H
#define PLAYERBOTS_FOLLOWACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;

class FollowAction : public MovementAction
{
public:
    FollowAction(PlayerbotAI* botAI, std::string const name = "follow") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool CanDeadFollow(Unit* target);
};

class FleeToGroupLeaderAction : public FollowAction
{
public:
    FleeToGroupLeaderAction(PlayerbotAI* botAI) : FollowAction(botAI, "flee to group leader") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Runs the bot back to its OWN master (GetMaster(), not the party/group leader - so it works when
// several party members each brought bots). Used by the combat "follow master" trigger when the
// admin enabled AiPlayerbot.CombatPrioritizeMaster. Stands down when the master is fighting the
// bot's current target, so shared fights still assist instead of running away.
class FollowMasterCombatAction : public FollowAction
{
public:
    FollowMasterCombatAction(PlayerbotAI* botAI) : FollowAction(botAI, "follow master combat") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
