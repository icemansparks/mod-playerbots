/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BATTLEGROUNDTACTICSACTION_H
#define _PLAYERBOT_BATTLEGROUNDTACTICSACTION_H

#include "BattlegroundAV.h"
#include "MovementActions.h"

class ChatHandler;
class Battleground;
class PlayerbotAI;
struct Position;
class GameObject;

#define SPELL_CAPTURE_BANNER 21651

enum WSBotStrategy : uint8
{
    WS_STRATEGY_BALANCED      = 0,
    WS_STRATEGY_OFFENSIVE     = 1,
    WS_STRATEGY_DEFENSIVE     = 2,
    WS_STRATEGY_MAX           = 3,
};

enum ABBotStrategy : uint8
{
    AB_STRATEGY_BALANCED      = 0,
    AB_STRATEGY_OFFENSIVE     = 1,
    AB_STRATEGY_DEFENSIVE     = 2,
    AB_STRATEGY_MAX           = 3,
};

enum AVBotStrategy : uint8
{
    AV_STRATEGY_BALANCED      = 0,
    AV_STRATEGY_OFFENSIVE     = 1,
    AV_STRATEGY_DEFENSIVE     = 2,
    AV_STRATEGY_MAX           = 3,
};

enum EYBotStrategy : uint8
{
    EY_STRATEGY_BALANCED      = 0,
    EY_STRATEGY_FRONT_FOCUS   = 1,
    EY_STRATEGY_BACK_FOCUS    = 2,
    EY_STRATEGY_FLAG_FOCUS    = 3,
    EY_STRATEGY_MAX           = 4
};

// Wintergrasp Zone and Constants
constexpr uint32 WINTERGRASP_ZONE_ID = 4197;

// Wintergrasp Area IDs (from AzerothCore AreaDefines.h)
constexpr uint32 AREA_THE_SUNKEN_RING    = 4538; // NE Workshop
constexpr uint32 AREA_THE_BROKEN_TEMPLE  = 4539; // NW Workshop
constexpr uint32 AREA_WESTSPARK_WORKSHOP = 4611; // SW Workshop
constexpr uint32 AREA_EASTSPARK_WORKSHOP = 4612; // SE Workshop
constexpr uint32 AREA_WINTERGRASP_FORTRESS = 4575; // Keep area
constexpr uint32 AREA_THE_CHILLED_QUAGMIRE = 4589; // Horde staging area

// Wintergrasp Game Mechanics Constants
constexpr uint32 WG_VEHICLES_PER_WORKSHOP = 4; // Each workshop adds 4 vehicles to max capacity

// Wintergrasp Strategy Types
enum WGBotStrategy : uint8
{
    WG_STRATEGY_BALANCED  = 0,
    WG_STRATEGY_OFFENSIVE = 1,
    WG_STRATEGY_DEFENSIVE = 2,
    WG_STRATEGY_MAX       = 3
};

// Wintergrasp Battlefield Data constants (from AzerothCore BattlefieldWG.h)
enum WintergraspDataIds
{
    BATTLEFIELD_WG_DATA_INTACT_TOWER_ATT = 0,
    BATTLEFIELD_WG_DATA_DAMAGED_TOWER_ATT = 1,
    BATTLEFIELD_WG_DATA_BROKEN_TOWER_ATT = 2,
    BATTLEFIELD_WG_DATA_MAX_VEHICLE_A = 3,
    BATTLEFIELD_WG_DATA_MAX_VEHICLE_H = 4,
    BATTLEFIELD_WG_DATA_VEHICLE_A = 5,
    BATTLEFIELD_WG_DATA_VEHICLE_H = 6,
    BATTLEFIELD_WG_DATA_MAX = 7,
};

// Wintergrasp Workshop IDs
enum WintergraspWorkshopIds
{
    BATTLEFIELD_WG_WORKSHOP_NE = 0,
    BATTLEFIELD_WG_WORKSHOP_NW = 1,
    BATTLEFIELD_WG_WORKSHOP_SE = 2,
    BATTLEFIELD_WG_WORKSHOP_SW = 3,
    BATTLEFIELD_WG_WORKSHOP_KEEP_WEST = 4,
    BATTLEFIELD_WG_WORKSHOP_KEEP_EAST = 5,
};

// Wintergrasp Game Objects
enum WintergraspGameObjects
{
    GO_WINTERGRASP_FACTORY_BANNER_NE = 190475,
    GO_WINTERGRASP_FACTORY_BANNER_NW = 190487,
    GO_WINTERGRASP_FACTORY_BANNER_SE = 194959,
    GO_WINTERGRASP_FACTORY_BANNER_SW = 194962,
    GO_WINTERGRASP_TITAN_S_RELIC = 192829,
};

// Wintergrasp Vehicle Entries
constexpr uint32 WG_ENTRY_SIEGE_ENGINE_A = 28312;
constexpr uint32 WG_ENTRY_SIEGE_ENGINE_H = 32627;
constexpr uint32 WG_ENTRY_CATAPULT       = 27881;
constexpr uint32 WG_ENTRY_DEMOLISHER     = 28094;
constexpr uint32 WG_TOWER_CANNON_ENTRY   = 28366; // NPC_WINTERGRASP_TOWER_CANNON

// Wintergrasp Rank Auras
constexpr uint32 WG_SPELL_RECRUIT    = 37795;
constexpr uint32 WG_SPELL_CORPORAL   = 33280;
constexpr uint32 WG_SPELL_LIEUTENANT = 55629;

// Wintergrasp Key Positions
extern Position const WG_GATE_POS;    // Fortress outer gate (siege focus for attackers)
extern Position const WG_RELIC_POS;   // Titan's Relic (final objective for attackers)
extern Position const WG_TOWER_W_POS; // Western tower
extern Position const WG_TOWER_S_POS; // Southern tower
extern Position const WG_TOWER_E_POS; // Eastern tower

typedef void (*BattleBotWaypointFunc)();

struct BGStrategyData
{
    uint8 allianceStrategy = 0;
    uint8 hordeStrategy = 0;
};

extern std::unordered_map<uint32, BGStrategyData> bgStrategies;

struct BattleBotWaypoint
{
    BattleBotWaypoint(float x_, float y_, float z_, BattleBotWaypointFunc func) : x(x_), y(y_), z(z_), pFunc(func){};

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    BattleBotWaypointFunc pFunc = nullptr;
};

struct AVNodePositionData
{
    Position pos;
    float maxRadius;
};

// Added to fix bot stuck at objectives
static std::unordered_map<uint8, AVNodePositionData> AVNodeMovementTargets = {
    {BG_AV_NODES_FIRSTAID_STATION, {Position(640.364f, -36.535f, 45.625f), 15.0f}},
    {BG_AV_NODES_STORMPIKE_GRAVE, {Position(665.598f, -292.976f, 30.291f), 15.0f}},
    {BG_AV_NODES_STONEHEART_GRAVE, {Position(76.108f, -399.602f, 45.730f), 15.0f}},
    {BG_AV_NODES_SNOWFALL_GRAVE, {Position(-201.298f, -119.661f, 78.291f), 15.0f}},
    {BG_AV_NODES_ICEBLOOD_GRAVE, {Position(-617.858f, -400.654f, 59.692f), 15.0f}},
    {BG_AV_NODES_FROSTWOLF_GRAVE, {Position(-1083.803f, -341.520f, 55.304f), 15.0f}},
    {BG_AV_NODES_FROSTWOLF_HUT, {Position(-1405.678f, -309.108f, 89.377f, 0.392f), 10.0f}},
    {BG_AV_NODES_DUNBALDAR_SOUTH, {Position(556.551f, -77.240f, 51.931f), 0.0f}},
    {BG_AV_NODES_DUNBALDAR_NORTH, {Position(670.664f, -142.031f, 63.666f), 0.0f}},
    {BG_AV_NODES_ICEWING_BUNKER, {Position(200.310f, -361.232f, 56.387f), 0.0f}},
    {BG_AV_NODES_STONEHEART_BUNKER, {Position(-156.302f, -440.032f, 40.403f), 0.0f}},
    {BG_AV_NODES_ICEBLOOD_TOWER, {Position(-569.702f, -265.362f, 75.009f), 0.0f}},
    {BG_AV_NODES_TOWER_POINT, {Position(-767.439f, -360.200f, 90.895f), 0.0f}},
    {BG_AV_NODES_FROSTWOLF_ETOWER, {Position(-1303.737f, -314.070f, 113.868f), 0.0f}},
    {BG_AV_NODES_FROSTWOLF_WTOWER, {Position(-1300.648f, -267.356f, 114.151f), 0.0f}},
};

typedef std::vector<BattleBotWaypoint> BattleBotPath;

extern std::vector<BattleBotPath*> const vPaths_WS;
extern std::vector<BattleBotPath*> const vPaths_AB;
extern std::vector<BattleBotPath*> const vPaths_AV;
extern std::vector<BattleBotPath*> const vPaths_EY;
extern std::vector<BattleBotPath*> const vPaths_IC;
extern std::vector<BattleBotPath*> const vPaths_WG;

class BGTactics : public MovementAction
{
public:
    static bool HandleConsoleCommand(ChatHandler* handler, char const* args);
    uint8 static GetBotStrategyForTeam(Battleground* bg, TeamId teamId);

    BGTactics(PlayerbotAI* botAI, std::string const name = "bg tactics") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    static std::string const HandleConsoleCommandPrivate(WorldSession* session, char const* args);
    bool moveToStart(bool force = false);
    bool selectObjective(bool reset = false);
    bool moveToObjective(bool ignoreDist);
    bool selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths);
    bool moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse = false);
    bool startNewPathBegin(std::vector<BattleBotPath*> const& vPaths);
    bool startNewPathFree(std::vector<BattleBotPath*> const& vPaths);
    bool resetObjective();
    bool wsJumpDown();
    bool eyJumpDown();
    bool atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds);
    bool flagTaken();
    bool teamFlagTaken();
    bool protectFC();
    bool useBuff();
    uint32 getPlayersInArea(TeamId teamId, Position point, float range, bool combat = true);
    bool IsLockedInsideKeep();
    bool handleWGTitansRelic(GameObject* go, float dist);
};

class ArenaTactics : public MovementAction
{
public:
    ArenaTactics(PlayerbotAI* botAI, std::string const name = "arena tactics") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

private:
    bool moveToCenter(Battleground* bg);
};

// Travel to Wintergrasp (Battlefield) when a battle is active
class WintergraspTravelAction : public Action
{
public:
    WintergraspTravelAction(PlayerbotAI* botAI, std::string const name = "wg travel") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Join the Wintergrasp queue anywhere (no NPC gossip), using battlefield API
class WintergraspQueueAction : public Action
{
public:
    WintergraspQueueAction(PlayerbotAI* botAI, std::string const name = "wg queue") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Enter the Wintergrasp war (teleport), using battlefield API
class WintergraspEnterWarAction : public Action
{
public:
    WintergraspEnterWarAction(PlayerbotAI* botAI, std::string const name = "wg enter war") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
