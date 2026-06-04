#include "utils.hpp"
#include <cmath>
#include <algorithm>

void applyCircleCollision(sf::Vector2f& pos1, sf::Vector2f& pos2, float radius1, float radius2) {
	sf::Vector2f dir = pos1 - pos2;
	float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	float minDist = radius1 + radius2 + 1.f;

	if (dist < minDist && dist > 0.0001f) {
		float overlap = minDist - dist;
		sf::Vector2f pushDir = dir / dist;

		pos1 += pushDir * (overlap / 2.f);
		pos2 -= pushDir * (overlap / 2.f);

	}

}

bool applyMapCollision(sf::Vector2f& pos, float radius, const Map& map) {
	bool collided = false;
	float tileSize = map.getTileSize();

	int gridX = static_cast<int>(pos.x / tileSize);
	int gridY = static_cast<int>(pos.y / tileSize);
	for (int y = gridY - 1; y <= gridY + 1; ++y) {
		for (int x = gridX - 1; x <= gridX + 1; ++x) {
			if (x < 0 || y < 0) continue;
			TileType type = map.getTile(x, y);
			if (type == TileType::Mountain || type == TileType::Water) {
				float tileLeft = x * tileSize;
				float tileTop = y * tileSize;
				float tileRight = tileLeft + tileSize;
				float tileBottom = tileTop + tileSize;
				float closestX = std::max(tileLeft, std::min(pos.x, tileRight));
				float closestY = std::max(tileTop, std::min(pos.y, tileBottom));
				float distX = pos.x - closestX;
				float distY = pos.y - closestY;
				float distanceSquared = distX * distX + distY * distY;
				if (distanceSquared < radius * radius) {
					float distance = std::sqrt(distanceSquared);
					if (distance > 0.0001f) {
						float overlap = radius - distance;
						pos.x += (distX / distance) * overlap;
						pos.y += (distY / distance) * overlap;
					}
					collided = true;
				}
			}
		}
	}
	return collided;
}

void drawHealthBar(sf::RenderWindow& window, sf::Vector2f position, float healthPercent) {
	if (healthPercent <= 0.1f) return;

	float w = 30.f;
	float h = 4.f;
	float yOffset = 25.f;

	sf::RectangleShape bg({ w, h });
	bg.setFillColor(sf::Color::Red);
	bg.setPosition({ position.x - w / 2.f, position.y - yOffset });

	sf::RectangleShape fg({ w * healthPercent, h });
	fg.setFillColor(sf::Color::Green);
	fg.setPosition({ position.x - w / 2.f, position.y - yOffset });

	window.draw(bg);
	window.draw(fg);
}


