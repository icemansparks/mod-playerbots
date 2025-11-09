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
    // Check if in a shapeshift form
    bool inShapeshift = botAI->HasAnyAuraOf(GetTarget(), "dire bear form", "bear form", "cat form", "travel form", "aquatic form",
                               "flight form", "swift flight form", "moonkin form", nullptr);

    if (!inShapeshift)
        return false;

    // If in aquatic form, check if we should stay in it for drowning prevention
    if (botAI->HasAura("aquatic form", bot))
    {
        int8 liquidState = bot->GetLiquidData().Status;

        // If underwater, check breath timer
        if (liquidState == LIQUID_MAP_UNDER_WATER)
        {
            uint32 breathTimer = bot->GetUInt32Value(PLAYER_BYTES_3) & 0xFF;
            // Keep aquatic form if breath is low (drowning prevention priority)
            if (breathTimer <= 60)
                return false;
        }

        // If breath is safe and in combat, shift out to attack
        Unit* target = AI_VALUE(Unit*, "current target");
        if (target)
            return true;
    }

    // For other forms, only shift if mana is decent
    return AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig->mediumHealth;
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

bool CastAquaticFormAction::isPossible()
{
    return CastBuffSpellAction::isPossible();
}

bool CastAquaticFormAction::isUseful()
{
    // Don't use if already in aquatic form
    if (botAI->HasAura("aquatic form", bot))
        return false;

    // Get liquid status
    int8 liquidState = bot->GetLiquidData().Status;

    // Not in water? Don't use aquatic form
    if (liquidState != LIQUID_MAP_IN_WATER && liquidState != LIQUID_MAP_UNDER_WATER)
        return false;

    // If underwater, check breath to prevent drowning (even during combat)
    if (liquidState == LIQUID_MAP_UNDER_WATER)
    {
        uint32 breathTimer = bot->GetUInt32Value(PLAYER_BYTES_3) & 0xFF;
        // If breath is getting low (below 60 seconds), prioritize aquatic form
        if (breathTimer <= 60)
            return true;
    }

    // Otherwise, only use aquatic form when not in combat
    Unit* target = AI_VALUE(Unit*, "current target");
    return !target;
}

NextAction** CastAquaticFormAction::getAlternatives()
{
    // If no mana for aquatic form but drowning, swim to surface instead
    return NextAction::array(0, new NextAction("swim to surface"), nullptr);
}

bool CastTreeFormAction::isUseful()
{
    return GetTarget() && CastSpellAction::isUseful() && !botAI->HasAura(33891, bot);
}
