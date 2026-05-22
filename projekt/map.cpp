#include "map.h"
#include <algorithm>
#include <cmath>

Map::Map(unsigned int width, unsigned int height, float tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize)
{

    m_grid.resize(m_width * m_height, TileType::Grass);

    setTile(5, 5, TileType::Water);
    setTile(6, 5, TileType::Water);
    setTile(5, 6, TileType::Water);


    for (unsigned int y = 2; y < 8; ++y) {
        setTile(10, y, TileType::Wall);
    }
}

Map::~Map() {}

void Map::update(float deltaTime) {

}

void Map::draw(sf::RenderWindow& window) {

    sf::RectangleShape tileShape({ m_tileSize, m_tileSize });


    tileShape.setOutlineThickness(-1.f);
    tileShape.setOutlineColor(sf::Color(0, 0, 0, 50)); // Półprzezroczysty czarny


    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {

   
            TileType type = getTile(x, y);


            switch (type) {
            case TileType::Grass: tileShape.setFillColor(sf::Color(34, 139, 34)); break; // Ciemnozielony
            case TileType::Water: tileShape.setFillColor(sf::Color(65, 105, 225)); break; // Niebieski
            case TileType::Wall:  tileShape.setFillColor(sf::Color(105, 105, 105)); break; // Szary
            }

            tileShape.setPosition({ static_cast<float>(x) * m_tileSize, static_cast<float>(y) * m_tileSize });

            window.draw(tileShape);
        }
    }
}

void Map::drawVisible(sf::RenderWindow& window, const sf::View& view) const {
    sf::RectangleShape tileShape({ m_tileSize, m_tileSize });
    tileShape.setOutlineThickness(-1.f);
    tileShape.setOutlineColor(sf::Color(0, 0, 0, 50));

    const sf::Vector2f center = view.getCenter();
    const sf::Vector2f size = view.getSize();

    const float left = center.x - size.x * 0.5f;
    const float right = center.x + size.x * 0.5f;
    const float top = center.y - size.y * 0.5f;
    const float bottom = center.y + size.y * 0.5f;

    int x0 = static_cast<int>(std::floor(left / m_tileSize)) - 1;
    int x1 = static_cast<int>(std::floor(right / m_tileSize)) + 1;
    int y0 = static_cast<int>(std::floor(top / m_tileSize)) - 1;
    int y1 = static_cast<int>(std::floor(bottom / m_tileSize)) + 1;


    x0 = std::max(0, x0);
    y0 = std::max(0, y0);
    x1 = std::min(static_cast<int>(m_width) - 1, x1);
    y1 = std::min(static_cast<int>(m_height) - 1, y1);

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            TileType type = getTile(static_cast<unsigned>(x), static_cast<unsigned>(y));

            switch (type) {
            case TileType::Grass: tileShape.setFillColor(sf::Color(34, 139, 34)); break;
            case TileType::Water: tileShape.setFillColor(sf::Color(65, 105, 225)); break;
            case TileType::Wall:  tileShape.setFillColor(sf::Color(105, 105, 105)); break;
            }

            tileShape.setPosition({ static_cast<float>(x) * m_tileSize,
                                    static_cast<float>(y) * m_tileSize });

            window.draw(tileShape);
        }
    }
}

TileType Map::getTile(unsigned int x, unsigned int y) const {
    if (x >= m_width || y >= m_height) {
        return TileType::Wall; 
    }
    return m_grid[y * m_width + x];
}

void Map::setTile(unsigned int x, unsigned int y, TileType type) {
    if (x < m_width && y < m_height) {
        m_grid[y * m_width + x] = type;
    }
}

float Map::getTileSize() const
{
    return m_tileSize;
}

std::pair<unsigned int, unsigned int> Map::getSize() const {
	return { m_width, m_height };
}
