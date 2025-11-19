/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "IsMovingValue.h"

#include "Playerbots.h"

bool IsMovingValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    if (!target)
        return false;

    return target->isMoving();
}

bool IsSwimmingValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    if (!target)
        return false;

    int8 targetInLiquidState = target->GetLiquidData().Status;
    bool result = targetInLiquidState == LIQUID_MAP_UNDER_WATER || (targetInLiquidState == LIQUID_MAP_IN_WATER && target->CanSwim());

    // DEBUG
    if (botAI->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "IsSwimming: liquidState=" << (int)targetInLiquidState
            << " canSwim=" << target->CanSwim()
            << " result=" << result;
        botAI->TellMaster(out.str());
    }

    return result;
}
