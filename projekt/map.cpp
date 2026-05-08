#include "map.h"

Map::Map(unsigned int width, unsigned int height, float tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize)
{
    // Wypełniamy całą mapę trawą. 
    // width * height to całkowita liczba kafelków.
    m_grid.resize(m_width * m_height, TileType::Grass);

    // Dodajmy trochę wody dla testu (np. małe jeziorko na środku)
    setTile(5, 5, TileType::Water);
    setTile(6, 5, TileType::Water);
    setTile(5, 6, TileType::Water);

    // I może jakiś murek z prawej strony
    for (unsigned int y = 2; y < 8; ++y) {
        setTile(10, y, TileType::Wall);
    }
}

Map::~Map() {}

void Map::update(float deltaTime) {
    // Na razie mapa jest statyczna, nic tu nie robimy,
    // ale może tu być np. animacja falowania wody w przyszłości.
}

void Map::draw(sf::RenderWindow& window) {
    // Tworzymy jeden kwadrat, którym będziemy rysować wszystkie kafelki
    sf::RectangleShape tileShape({ m_tileSize, m_tileSize });

    // Opcjonalnie: lekki zarys siatki, żeby było widać kafelki
    tileShape.setOutlineThickness(-1.f);
    tileShape.setOutlineColor(sf::Color(0, 0, 0, 50)); // Półprzezroczysty czarny

    // Rysujemy kafelek po kafelku
    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {

            // Pobieramy typ terenu
            TileType type = getTile(x, y);

            // Wybieramy kolor w zależności od typu terenu
            switch (type) {
            case TileType::Grass: tileShape.setFillColor(sf::Color(34, 139, 34)); break; // Ciemnozielony
            case TileType::Water: tileShape.setFillColor(sf::Color(65, 105, 225)); break; // Niebieski
            case TileType::Wall:  tileShape.setFillColor(sf::Color(105, 105, 105)); break; // Szary
            }

            // Ustawiamy pozycję na ekranie (x * rozmiar, y * rozmiar)
            tileShape.setPosition({ static_cast<float>(x) * m_tileSize, static_cast<float>(y) * m_tileSize });

            // Rysujemy
            window.draw(tileShape);
        }
    }
}

TileType Map::getTile(unsigned int x, unsigned int y) const {
    // Zabezpieczenie przed wyjściem poza tablicę
    if (x >= m_width || y >= m_height) {
        return TileType::Wall; // Jeśli poza mapą, udajemy, że to ściana
    }
    // Wzór na indeksowanie tablicy 1D jak 2D to: (y * szerokość + x)
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
