#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <memory>
#include "unit.h"
#include "buildings.h"
#include "API.h"
#include "map.h"


enum class ResourceType { Gold, Wood, Rock };

class EXPORT_API Player {
public:
	Player();
    ~Player();
	void update(float deltaTime, const Map& map);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

	void addGold(int amount);
	unsigned int getResource(ResourceType type) const;
	void addResource(ResourceType type, int amount);

    void addUnit(sf::Vector2f position, UnitType type);
    std::vector<Unit>& getUnits();
    const std::vector<Unit>& getUnits() const;
    void addBuilding(sf::Vector2f position, BuildingType type);
    const std::vector < std::unique_ptr < Building >> &getBuildings() const;
    bool hasTownHall() const;

    void beginPlaceBuilding(BuildingType type);
    bool isPlacingBuilding() const;

    void setLastMouseWorldPos(sf::Vector2f pos);
    
private:
    unsigned int m_gold;
    unsigned int m_rocks;
	unsigned int m_wood;

    std::vector<Unit> m_units;
    std::vector<std::unique_ptr<Building>> m_buildings;

    sf::RectangleShape m_selectionBox;
    sf::Vector2f m_startClick;
    bool m_isDragging = false;

	bool m_TownHallBuilt = false;
    bool m_TownHallDestroyed = false;

    bool m_placingBuilding = false;
    BuildingType m_placeType = BuildingType::Quarry;

    sf::Vector2f m_lastMouseWorldPos{ 0.f, 0.f };

    sf::RectangleShape m_buildGhost;

	sf::Vector2f m_ghostPos{ 0.f, 0.f };
	sf::Vector2f m_ghostSize{ 0.f, 0.f };
	bool m_ghostValid = true;
};