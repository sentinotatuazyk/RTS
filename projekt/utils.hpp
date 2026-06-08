#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#include "map.h"
#include "API.h"
#include "enemy.h"
#include "unit.h"
#include "buildings.h"

static const sf::Color DarkRed = sf::Color(139, 0, 0);

EXPORT_API void applyCircleCollision(sf::Vector2f& pos1, sf::Vector2f& pos2, float radius1, float radius2);
EXPORT_API bool applyMapCollision(sf::Vector2f& pos, float radius, const Map& map);
EXPORT_API void drawHealthBar(sf::RenderWindow& window, sf::Vector2f position, float healthPercent);
bool applyBuildingCollision(sf::Vector2f& pos, float radius, const std::vector<std::unique_ptr<Building>>& buildings);

inline sf::Vector2f snapToTile(sf::Vector2f p, float tileSize) {
    p.x = std::floor(p.x / tileSize) * tileSize;
    p.y = std::floor(p.y / tileSize) * tileSize;
    return p;
}

inline sf::Vector2f buildingSize(BuildingType t, float tileSize = 40.f) {
    switch (t) {
        case BuildingType::TownHall: return {tileSize * 3, tileSize * 3};
        case BuildingType::Quarry:   return {tileSize * 2, tileSize * 2};
        case BuildingType::Barracks: return {tileSize * 2, tileSize * 3};
		case BuildingType::GoldMine: return { tileSize * 2, tileSize * 2 };
		case BuildingType::Foresters: return { tileSize * 2, tileSize * 2 };
        default:                     return {tileSize, tileSize};
    }
}

inline std::string toString(UnitType type) {
    switch (type) {
    case UnitType::Worker:  return "Worker";
    case UnitType::Warrior: return "Warrior";
    case UnitType::Archer:  return "Archer";
    case UnitType::Hero:    return "Hero";
    default:                return "Unknown";
    } 
}

inline std::string toString(BuildingType type) {
    switch (type) {
    case BuildingType::TownHall:  return "Town Hall";
    case BuildingType::Quarry:    return "Quarry";
    case BuildingType::Barracks:  return "Barracks";
    case BuildingType::GoldMine:  return "Gold Mine";
    case BuildingType::Foresters: return "Forester's Lodge";
    default:                      return "Unknown";
    }
}


inline std::string toString(EnemyType type) {
    switch (type) {
    case EnemyType::Goblin: return "Goblin";
    case EnemyType::Orc:    return "Orc";
    case EnemyType::Troll:  return "Troll";
    default:                return "Unknown";
    }
}



