#pragma once

enum class UnitType { Worker, Warrior, Archer, Hero };
enum class UnitState { Aggressive, Passive, Neutral };
enum class TileType { Grass, Water, Mountain, Sand, Snow };
enum class BuildingType { TownHall, Quarry, Barracks, GoldMine, Foresters };
enum class EnemyType { Goblin, Orc, Troll };
enum class EnemyState { Aggressive, Passive, Neutral };
enum class SettingsAction {
    None,
    Back,   // wróć do menu bez apply
    Apply   // zastosuj ustawienia
};

enum class ResourceType { Gold, Wood, Rock };
enum class MenuAction { None, StartGame, Settings, ExitGame };