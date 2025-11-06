/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidShapeshiftActions.h"

#include "MovementActions.h"
#include "Playerbots.h"

bool CastBearFormAction::isPossible()
{
    return CastBuffSpellAction::isPossible() && !botAI->HasAura("dire bear form", GetTarget());
}

bool CastBearFormAction::isUseful()
{
    return CastBuffSpellAction::isUseful() && !botAI->HasAura("dire bear form", GetTarget());
}

NextAction** CastDireBearFormAction::getAlternatives()
{
    return NextAction::merge(NextAction::array(0, new NextAction("bear form"), nullptr),
                             CastSpellAction::getAlternatives());
}

bool CastTravelFormAction::isUseful()
{
    bool firstmount = bot->GetLevel() >= 20;

    // useful if no mount or with wsg flag
    return !bot->IsMounted() && (!firstmount || (bot->HasAura(23333) || bot->HasAura(23335) || bot->HasAura(34976))) &&
           !botAI->HasAura("dash", bot);
}

bool CastCasterFormAction::isUseful()
{
    return botAI->HasAnyAuraOf(GetTarget(), "dire bear form", "bear form", "cat form", "travel form", "aquatic form",
                               "flight form", "swift flight form", "moonkin form", nullptr) &&
           AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig->mediumHealth;
}

bool CastCasterFormAction::Execute(Event event)
{
    botAI->RemoveShapeshift();
    return true;
}

bool CastCancelTreeFormAction::isUseful()
{
    return botAI->HasAura(33891, bot);
}

bool CastCancelTreeFormAction::Execute(Event event)
{
    botAI->RemoveAura("tree of life");
    return true;
}

bool CastAquaticFormAction::isUseful()
{
    // Don't use if already in aquatic form
    if (botAI->HasAura("aquatic form", bot))
    {
        return false;
    }

    // Check mana requirements first - if we can't cast, allow fallback to swim to surface
    if (!CastSpellAction::isPossible())
    {
        return false; // Will fall back to generic swim to surface action
    }

    // Check if in water or underwater using bot's position directly
    int8 liquidState = bot->GetLiquidData().Status;
    bool inWater = liquidState == LIQUID_MAP_IN_WATER || liquidState == LIQUID_MAP_UNDER_WATER;

    if (!inWater)
    {
        return false;
    }

    // Always use aquatic form when in water, unless in combat and not drowning
    bool inCombat = bot->IsInCombat();

    if (!inCombat)
    {
        // Not in combat - always use aquatic form in water for convenience and safety
        return true;
    }
    else
    {
        // If in combat, only use for emergencies (breath-based only)
        uint32 breathTimer = bot->GetUInt32Value(PLAYER_BYTES_3) & 0xFF;

        // Emergency: Use only if breath is critically low
        return breathTimer <= DROWNING_EMERGENCY_THRESHOLD_SECONDS;
    }
}

bool CastTreeFormAction::isUseful()
{
    return GetTarget() && CastSpellAction::isUseful() && !botAI->HasAura(33891, bot);
}
