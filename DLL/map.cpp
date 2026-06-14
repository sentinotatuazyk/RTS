#include "map.h"
#include <algorithm>
#include <cmath>

Map::Map(unsigned int width, unsigned int height, float tileSize, unsigned int seed)
    : m_width(width), m_height(height), m_tileSize(tileSize),
    m_grid(width* height), m_heightmap(width* height),
    m_vertices(sf::PrimitiveType::Triangles) {

    m_noise = std::make_unique<PerlinNoise>(seed);
    generateTerrain();
    rebuildVertices();
}



void Map::generateTerrain() {
    // Najpierw wygeneruj surowe wysokości
    std::vector<float> rawHeights(m_width * m_height);

    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {
            float nx = static_cast<float>(x) * m_noiseScale;
            float ny = static_cast<float>(y) * m_noiseScale;

            float height = m_noise->fbm(nx, ny, m_octaves, m_persistence);

            // Domain warping
            float warpX = m_noise->noise(nx * 2.0f + 5.2f, ny * 2.0f + 1.3f) * 2.0f;
            float warpY = m_noise->noise(nx * 2.0f + 9.2f, ny * 2.0f + 2.3f) * 2.0f;
            float warpedHeight = m_noise->fbm(nx + warpX, ny + warpY, m_octaves, m_persistence);

            height = height * 0.7f + warpedHeight * 0.3f;
            rawHeights[y * m_width + x] = height;
        }
    }

    // Pierwszy przebieg: woda, trawa, góry, śnieg (bez piasku!)
    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {
            float h = rawHeights[y * m_width + x];
            m_heightmap[y * m_width + x] = h;

            // Tymczasowo: woda lub ląd (bez piasku jeszcze)
			m_grid[y * m_width + x] = heightToTile(h);
        }
    }

    // Drugi przebieg: piasek TYLKO obok wody
    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {
            // Jeśli to nie jest trawa — pomijamy (piasek tylko z trawy)
            if (m_grid[y * m_width + x] != TileType::Grass) continue;

            // Sprawdź 8 sąsiadów
            bool nearWater = false;
            for (int dy = -1; dy <= 1 && !nearWater; ++dy) {
                for (int dx = -1; dx <= 1 && !nearWater; ++dx) {
                    if (dx == 0 && dy == 0) continue;

                    int nx = static_cast<int>(x) + dx;
                    int ny = static_cast<int>(y) + dy;

                    // Sprawdź granice mapy
                    if (nx < 0 || nx >= static_cast<int>(m_width) ||
                        ny < 0 || ny >= static_cast<int>(m_height)) continue;

                    if (m_grid[ny * m_width + nx] == TileType::Water) {
                        nearWater = true;
                    }
                }
            }

            // Jeśli obok jest woda i wysokość jest niska — piasek
            if (nearWater && rawHeights[y * m_width + x] < 0.42f) {
                m_grid[y * m_width + x] = TileType::Sand;
            }
        }
    }

    m_verticesNeedUpdate = true;
}

TileType Map::heightToTile(float height) const {
    // Progi wysokości dla biomów
    if (height < 0.3f) return TileType::Water;
    if (height < 0.55f) return TileType::Grass;
    if (height < 0.90f) return TileType::Mountain;
    return TileType::Snow;
}

sf::Color Map::tileToColor(TileType type) const {
    switch (type) {
    case TileType::Water:  return sf::Color(64, 164, 223);
    case TileType::Sand:   return sf::Color(194, 178, 128);
    case TileType::Grass:  return sf::Color(34, 139, 34);
    case TileType::Mountain:   return sf::Color(139, 137, 137);
    case TileType::Snow:   return sf::Color(255, 250, 250);
    default:               return sf::Color::Magenta;
    }
}

float Map::heuristic(int x1, int y1, int x2, int y2) const
{
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    return static_cast<float>(std::max(dx, dy) + (std::sqrt(2.0f) - 1.0f) * std::min(dx, dy));
}

std::vector<std::pair<int, int>> Map::getNeighbors(int x, int y) const
{
    std::vector<std::pair<int, int>> neighbors;
    neighbors.reserve(8);

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (!isWalkable(nx, ny)) continue;

            // Sprawdź "przycinanie" rogów — nie chodź na ukos przez narożnik
            if (dx != 0 && dy != 0) {
                if (!isWalkable(x + dx, y) || !isWalkable(x, y + dy))
                    continue; // blokowany róg
            }

            neighbors.push_back({ nx, ny });
        }
    }
    return neighbors;
}

void Map::rebuildVertices() {
    m_vertices.clear();

    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {
            float px = static_cast<float>(x) * m_tileSize;
            float py = static_cast<float>(y) * m_tileSize;

            TileType type = m_grid[y * m_width + x];
            sf::Color color = tileToColor(type);

            // Dodaj cieniowanie bazujące na wysokości
            float height = m_heightmap[y * m_width + x];
            float shade = 0.7f + height * 0.3f;  // Jaśniejsze na górze
            color.r = static_cast <std::uint8_t> (color.r * shade);
            color.g = static_cast <std::uint8_t> (color.g * shade);
            color.b = static_cast <std::uint8_t> (color.b * shade);

            // Dwa trójkąty na każdy tile (quad)
            // Lewy górny
            m_vertices.append({ {px, py}, color });
            m_vertices.append({ {px + m_tileSize, py}, color });
            m_vertices.append({ {px, py + m_tileSize}, color });

            // Prawy dolny
            m_vertices.append({ {px + m_tileSize, py}, color });
            m_vertices.append({ {px + m_tileSize, py + m_tileSize}, color });
            m_vertices.append({ {px, py + m_tileSize}, color });
        }
    }
    m_verticesNeedUpdate = false;
}

void Map::update(float deltaTime) {
    if (m_verticesNeedUpdate) {
        rebuildVertices();
    }
}

void Map::draw(sf::RenderWindow& window) {
    window.draw(m_vertices);
}

void Map::drawVisible(sf::RenderWindow& window, const sf::View& view) const {
    // Oblicz zakres widocznych tile'i
    sf::FloatRect viewBounds(
        { view.getCenter().x - view.getSize().x / 2.0f,
          view.getCenter().y - view.getSize().y / 2.0f },
        { view.getSize().x,
          view.getSize().y }
    );

    float margin = m_tileSize * 2.f;

    int startX = static_cast<int>(std::max(0.0f, (viewBounds.position.x - margin) / m_tileSize));
    int startY = static_cast<int>(std::max(0.0f, (viewBounds.position.y - margin) / m_tileSize));
    int endX = static_cast<int>(std::min(static_cast<float>(m_width),
        (viewBounds.position.x + viewBounds.size.x + margin) / m_tileSize + 1.0f));
    int endY = static_cast<int>(std::min(static_cast<float>(m_height),
        (viewBounds.position.y + viewBounds.size.y + margin) / m_tileSize + 1.0f));

    int tilesPerRow = static_cast<int>(m_width);
    int verticesPerTile = 6;

    std::vector<sf::Vertex> visibleVertices;
    visibleVertices.reserve((endY - startY) * (endX - startX) * verticesPerTile);

    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            int tileIndex = y * tilesPerRow + x;
            int vertexIndex = tileIndex * verticesPerTile;

            // Rysuj 6 wierzchołków danego tile'a
            for (int i = 0; i < verticesPerTile; ++i) {
                visibleVertices.push_back(m_vertices[vertexIndex + i]);
            }
        }
    }

    if (!visibleVertices.empty()) {
        window.draw(visibleVertices.data(), visibleVertices.size(), sf::PrimitiveType::Triangles);
    }
}

TileType Map::getTile(unsigned int x, unsigned int y) const {
    if (x >= m_width || y >= m_height) return TileType::Mountain;
    return m_grid[y * m_width + x];
}

void Map::setTile(unsigned int x, unsigned int y, TileType type) {
    if (x >= m_width || y >= m_height) return;
    m_grid[y * m_width + x] = type;
    m_verticesNeedUpdate = true;
}

float Map::getTileSize() const {
    return m_tileSize;
}

std::pair<unsigned int, unsigned int> Map::getSize() const {
    return { m_width, m_height };
}

sf::FloatRect Map::getMapBounds() const {
    return sf::FloatRect({ 0.0f, 0.0f }, { m_width * m_tileSize, m_height * m_tileSize });
}

void Map::regenerate(unsigned int seed) {
    m_noise = std::make_unique<PerlinNoise>(seed);
    generateTerrain();
    rebuildVertices();
}

void Map::generateChunk(int chunkX, int chunkY, int chunkSize) {
    // Do użycia z chunk managerem - generuje fragment mapy
    for (int y = 0; y < chunkSize; ++y) {
        for (int x = 0; x < chunkSize; ++x) {
            int worldX = chunkX * chunkSize + x;
            int worldY = chunkY * chunkSize + y;

            if (worldX < 0 || worldX >= static_cast<int>(m_width) ||
                worldY < 0 || worldY >= static_cast<int>(m_height)) continue;

            float nx = static_cast<float>(worldX) * m_noiseScale;
            float ny = static_cast<float>(worldY) * m_noiseScale;

            float height = m_noise->fbm(nx, ny, m_octaves, m_persistence);
            height = (height + 1.0f) * 0.5f;

            m_heightmap[worldY * m_width + worldX] = height;
            m_grid[worldY * m_width + worldX] = heightToTile(height);
        }
    }
    m_verticesNeedUpdate = true;
}

std::vector<sf::Vector2f> Map::findPath(sf::Vector2f start, sf::Vector2f end) const
{
    // Konwertuj world → tile
    int startX = static_cast<int>(std::floor(start.x / m_tileSize));
    int startY = static_cast<int>(std::floor(start.y / m_tileSize));
    int endX = static_cast<int>(std::floor(end.x / m_tileSize));
    int endY = static_cast<int>(std::floor(end.y / m_tileSize));

    // Granice
    if (!isWalkable(startX, startY) || !isWalkable(endX, endY)) {
        return {}; // start lub cel w wodzie/górach
    }

    // A*
    std::priority_queue<ANode, std::vector<ANode>, std::greater<ANode>> open;
    std::unordered_set<int> closed; // zakodowane: y * m_width + x

    std::unordered_map<int, int> cameFrom; // zakodowane: current → parent
    std::unordered_map<int, float> gScore;

    auto encode = [this](int x, int y) { return y * static_cast<int>(m_width) + x; };

    open.push({ startX, startY, 0.f, heuristic(startX, startY, endX, endY) });
    gScore[encode(startX, startY)] = 0.f;

    while (!open.empty()) {
        ANode current = open.top();
        open.pop();

        int currentKey = encode(current.x, current.y);

        if (closed.count(currentKey)) continue;
        closed.insert(currentKey);

        // Cel osiągnięty
        if (current.x == endX && current.y == endY) {
            // Rekonstrukcja ścieżki
            std::vector <sf::Vector2f> path;
            int key = currentKey;

            while (key != encode(startX, startY)) {
                int x = key % static_cast<int>(m_width);
                int y = key / static_cast<int>(m_width);
                path.push_back({
                    x * m_tileSize + m_tileSize / 2.f,
                    y * m_tileSize + m_tileSize / 2.f
                    });
                auto it = cameFrom.find(key);
                if (it == cameFrom.end()) break; // bezpieczeństwo
                key = it->second;
            }

            // Dodaj start (opcjonalnie)
            // path.push_back(startWorld);

            std::reverse(path.begin(), path.end());
            return path;
        }

        // Sąsiedzi
        for (auto [nx, ny] : getNeighbors(current.x, current.y)) {
            int neighborKey = encode(nx, ny);
            if (closed.count(neighborKey)) continue;

            // Koszt ruchu: 1.0 dla prosto, 1.414 dla ukos
            float moveCost = ((nx != current.x) && (ny != current.y)) ? 1.414f : 1.0f;
            float tentativeG = current.g + moveCost;

            auto it = gScore.find(neighborKey);
            if (it == gScore.end() || tentativeG < it->second) {
                cameFrom[neighborKey] = currentKey;
                gScore[neighborKey] = tentativeG;
                float f = tentativeG + heuristic(nx, ny, endX, endY);
                open.push({ nx, ny, tentativeG, f });
            }
        }
    }

    return {}; // brak ścieżki
}

bool Map::isWalkable(unsigned int x, unsigned int y) const
{
    if (x < 0 || x >= static_cast<int>(m_width) || y < 0 || y >= static_cast<int>(m_height)) return false;
	TileType type = getTile(x, y);
	return type == TileType::Grass || type == TileType::Sand;
}
