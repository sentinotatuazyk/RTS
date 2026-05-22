#pragma once

#include <SFML/Graphics/Color.hpp>
#include "map.h"
#include "API.h"

static const sf::Color DarkRed = sf::Color(139, 0, 0);

EXPORT_API void applyCircleCollision(sf::Vector2f& pos1, sf::Vector2f& pos2, float radius1, float radius2);
EXPORT_API bool applyMapCollision(sf::Vector2f& pos, float radius, const Map& map);
EXPORT_API void drawHealthBar(sf::RenderWindow& window, sf::Vector2f position, float healthPercent);



