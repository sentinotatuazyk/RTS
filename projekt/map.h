#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "PerlinNoise.h"

enum class TileType { Grass, Water, Mountain, Sand, Snow };

class Map {
public:
    Map(unsigned int width, unsigned int height, float tileSize, unsigned int seed = 12345);
    ~Map() = default;

    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void drawVisible(sf::RenderWindow& window, const sf::View& view) const;

    TileType getTile(unsigned int x, unsigned int y) const;
    void setTile(unsigned int x, unsigned int y, TileType type);

    float getTileSize() const;
    std::pair<unsigned int, unsigned int> getSize() const;

    sf::FloatRect getMapBounds() const;

    // Nowe: regeneruj mapę z nowym seedem
    void regenerate(unsigned int seed);
    // Nowe: generuj chunk proceduralnie
    void generateChunk(int chunkX, int chunkY, int chunkSize);

private:
    unsigned int m_width;
    unsigned int m_height;
    float m_tileSize;

    std::vector<TileType> m_grid;
    sf::VertexArray m_vertices;
    bool m_verticesNeedUpdate = true;

    std::unique_ptr<PerlinNoise> m_noise;
    float m_noiseScale = 0.05f;
    int m_octaves = 8;
    float m_persistence = 0.5f;

    // Wysokości dla każdego tile'a (opcjonalnie - do cieniowania)
    std::vector<float> m_heightmap;

    void generateTerrain();
    void rebuildVertices();
    TileType heightToTile(float height) const;
    sf::Color tileToColor(TileType type) const;


};