/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidTriggers.h"
#include "Player.h"
#include "Playerbots.h"

bool MarkOfTheWildOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("gift of the wild", GetTarget());
}

bool MarkOfTheWildTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("gift of the wild", GetTarget());
}

bool ThornsOnPartyTrigger::IsActive()
{
    return BuffOnPartyTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget());
}

bool EntanglingRootsKiteTrigger::IsActive()
{
    return DebuffTrigger::IsActive() && AI_VALUE(uint8, "attacker count") < 3 && !GetTarget()->GetPower(POWER_MANA);
}

bool ThornsTrigger::IsActive() { return BuffTrigger::IsActive() && !botAI->HasAura("thorns", GetTarget()); }

bool BearFormTrigger::IsActive() { return !botAI->HasAnyAuraOf(bot, "bear form", "dire bear form", nullptr); }

bool TreeFormTrigger::IsActive() { return !botAI->HasAura(33891, bot); }

bool CatFormTrigger::IsActive() { return !botAI->HasAura("cat form", bot); }

const std::set<uint32> HurricaneChannelCheckTrigger::HURRICANE_SPELL_IDS = {
    16914,  // Hurricane Rank 1
    17401,  // Hurricane Rank 2
    17402,  // Hurricane Rank 3
    27012,  // Hurricane Rank 4
    48467   // Hurricane Rank 5
};

bool HurricaneChannelCheckTrigger::IsActive()
{
    Player* bot = botAI->GetBot();

    // Check if the bot is channeling a spell
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
    {
        // Only trigger if the spell being channeled is Hurricane
        if (HURRICANE_SPELL_IDS.count(spell->m_spellInfo->Id))
        {
            uint8 attackerCount = AI_VALUE(uint8, "attacker count");
            return attackerCount < minEnemies;
        }
    }

    // Not channeling Hurricane
    return false;
}

bool AquaticFormToCasterTrigger::IsActive()
{
    // Only trigger if bot is in aquatic form
    if (!botAI->HasAura("aquatic form", bot))
    {
        return false;
    }

    // Only consider switching during combat in water
    if (!bot->IsInCombat())
    {
        return false;
    }

    // Check if bot is in water (where combat effectiveness matters)
    int8 liquidState = bot->GetLiquidData().Status;
    bool inWater = liquidState == LIQUID_MAP_IN_WATER || liquidState == LIQUID_MAP_UNDER_WATER;

    if (!inWater)
    {
        return false; // Game will handle form removal on land
    }

    // Self-preservation is more important - don't switch if drowning risk
    uint32 breathTimer = bot->GetUInt32Value(PLAYER_BYTES_3) & 0xFF;
    if (breathTimer <= 60)
    {
        return false; // Stay in aquatic form to prevent drowning
    }

    // Safe to switch for combat effectiveness if we have decent mana
    uint8 manaPercent = AI_VALUE2(uint8, "mana", "self target");
    return manaPercent > 50; // Switch to caster for combat, but only with good mana
}
