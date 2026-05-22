#pragma once
#include "API.h"
#include <SFML/Graphics.hpp>
#include <vector>

enum class TileType {Grass, Water, Wall};

class EXPORT_API Map {
public:
	Map(unsigned int m_width, unsigned int m_height, float tileSize);
	~Map();

	void update(float deltaTime);
	void draw(sf::RenderWindow& window);

	void drawVisible(sf::RenderWindow& window, const sf::View& view) const;

	TileType getTile(unsigned int x, unsigned int y) const;
	void setTile(unsigned int x, unsigned int y, TileType type);

	float getTileSize() const;
	std::pair<unsigned int, unsigned int> getSize() const;
private:
	unsigned int m_width;
	unsigned int m_height;
	float m_tileSize;

	std::vector<TileType> m_grid;
};

