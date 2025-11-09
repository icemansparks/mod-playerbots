/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CheckLiquidStateAction.h"

#include "Playerbots.h"

bool CheckLiquidStateAction::isUseful()
{
    // Only run when debug is enabled
    return botAI->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT);
}

bool CheckLiquidStateAction::Execute(Event event)
{
    int8 liquidState = bot->GetLiquidData().Status;
    uint32 breathTimer = bot->GetUInt32Value(PLAYER_BYTES_3) & 0xFF;
    bool canSwim = bot->CanSwim();
    bool isSwimming = AI_VALUE2(bool, "swimming", "self target");

    std::ostringstream out;
    out << "LIQUID DEBUG: "
        << "state=" << (int)liquidState
        << " (0=none,1=above,2=in,3=under)"
        << " breath=" << breathTimer
        << " canSwim=" << canSwim
        << " IsSwimmingValue=" << isSwimming;

    botAI->TellMaster(out.str());

    return true;
}
