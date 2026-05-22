#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <memory>
#include "unit.h"
#include "buildings.h"
#include "API.h"
#include "map.h"


class EXPORT_API Player {
public:
	Player();
    ~Player();
	void update(float deltaTime, const Map& map);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

	void addGold(int amount);
    int getGold() const;

    void addUnit(sf::Vector2f position, UnitType type);
    std::vector<Unit>& getUnits();
    const std::vector<Unit>& getUnits() const;
    void addBuilding(sf::Vector2f position, BuildingType type);
    
private:
    int m_gold;
    std::vector<Unit> m_units;
    std::vector<std::unique_ptr<Building>> m_buildings;

    sf::RectangleShape m_selectionBox;
    sf::Vector2f m_startClick;
    bool m_isDragging = false;
};