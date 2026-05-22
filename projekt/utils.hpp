#pragma once

#include <SFML/Graphics/Color.hpp>
#include <string>
#include "map.h"
#include "API.h"
#include "enemy.h"
#include "unit.h"

static const sf::Color DarkRed = sf::Color(139, 0, 0);

EXPORT_API void applyCircleCollision(sf::Vector2f& pos1, sf::Vector2f& pos2, float radius1, float radius2);
EXPORT_API bool applyMapCollision(sf::Vector2f& pos, float radius, const Map& map);
EXPORT_API void drawHealthBar(sf::RenderWindow& window, sf::Vector2f position, float healthPercent);

inline std::string toString(UnitType type) {
    switch (type) {
    case UnitType::Worker: return "Worker";
    case UnitType::Warrior: return "Warrior";
    case UnitType::Archer: return "Archer";
    case UnitType::Hero: return "Hero";
    default: return "Unknown";
    }
}

inline std::string toString(EnemyType type) {
    switch (type) {
    case EnemyType::Goblin: return "Goblin";
    case EnemyType::Orc: return "Orc";
    case EnemyType::Troll: return "Troll";
    default: return "Unknown";
    }
}



