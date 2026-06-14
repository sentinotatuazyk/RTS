#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <queue>
#include <unordered_set>
#include "PerlinNoise.h"
#include "enums.h"



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

	std::vector<sf::Vector2f> findPath(sf::Vector2f start, sf::Vector2f end) const;
    bool isWalkable(unsigned int x, unsigned int y) const;

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


    struct ANode {
        int x, y;
        float g, f;
        bool operator>(const ANode& other) const {
            return f > other.f;
		}
    };
	float heuristic(int x1, int y1, int x2, int y2) const;
	std::vector<std::pair<int,int>> getNeighbors(int x, int y) const;


};