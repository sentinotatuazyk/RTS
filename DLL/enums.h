#pragma once

enum class UnitType { Worker, Warrior, Archer, Hero };
enum class UnitState { Aggressive, Passive, Neutral };
enum class TileType { Grass, Water, Mountain, Sand, Snow };
enum class BuildingType { TownHall, Quarry, Barracks, GoldMine, Foresters, Farm, Fence };
enum class EnemyType { Goblin, Orc, Troll };
enum class EnemyState { Aggressive, Passive, Neutral };
enum class SettingsAction {
    None,
    Back,   
    Apply   
};

enum class ResourceType { Gold, Wood, Rock, Food };
enum class MenuAction { None, StartGame, Settings, ExitGame };
enum class FenceStates { Front, LeftBendDown,RightBendDown, LeftBendUp, RightBendUp, LeftSide, RightSide};