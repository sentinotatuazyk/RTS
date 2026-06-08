#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <memory>
#include "buildings.h"
#include "API.h"
#include "map.h"
#include "enums.h"




class EXPORT_API Player {
public:
	Player();
    ~Player();
	void update(float deltaTime, const Map& map);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

	int getResource(ResourceType type) const;
	void addResource(ResourceType type, int amount);
	void spendResource(ResourceType type, int amount);

    void addUnit(sf::Vector2f position, UnitType type);
	int getUnitCount() const;
	int getUnitCount(UnitType type) const;
	void changeUnitCount(UnitType type, int amount);
    bool canAfford(Cost cost) const;
    std::vector<Unit>& getUnits();
    const std::vector<Unit>& getUnits() const;
    void addBuilding(sf::Vector2f position, BuildingType type);
	int getBuildingCount() const;
	int getBuildingCount(BuildingType type) const;
	void changeBuildingCount(BuildingType type, int amount);

    void updateAllFenceSegments();

	void removeBuilding(Building* toremove);
    void demolishSelectedBuildings();
    const std::vector < std::unique_ptr < Building >> &getBuildings() const;

    int getFoodIncome() const;
	void changeFoodIncome(int amount) { m_foodIncome += amount; }
    int calculateFoodIncome() const;
    bool hasTownHall() const;

    void beginPlaceBuilding(BuildingType type);
    bool isPlacingBuilding() const;

    void setLastMouseWorldPos(sf::Vector2f pos);
    
private:
    unsigned int m_gold;
    unsigned int m_rocks;
	unsigned int m_wood;
	int m_food;

	int m_foodIncome = 0;

    std::vector<Unit> m_units;
	int m_unitCount = 0;
    std::unordered_map<UnitType, int> m_unitTable = {
        {UnitType::Worker,  0}, 
		{UnitType::Warrior, 0},
		{UnitType::Archer,  0},
        {UnitType::Hero,    0} 
    };
    

    std::vector<std::unique_ptr<Building>> m_buildings;
	int m_buildingCount = 0;
    std::unordered_map<BuildingType, int> m_buildingTable = {
        {BuildingType::TownHall,  0},
        {BuildingType::Quarry,    0},
        {BuildingType::Barracks,  0},
        {BuildingType::GoldMine,  0},
        {BuildingType::Foresters, 0},
        {BuildingType::Farm,      0},
        {BuildingType::Fence,     0}
    };

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