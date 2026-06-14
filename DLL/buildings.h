#pragma once

#include "API.h"
#include "unit.h"
#include <SFML/Graphics.hpp>
#include <utility>
#include "../../SFML-3.1.0/include/SFML/Graphics/Color.hpp"
#include "enums.h"
#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

class Player;

struct Cost {
    int gold;
    int wood;
    int rock;
    int food;
};

static Cost getUnitCost(UnitType type) { // 5 kamienia i drewna na sekunde, 1 gold na sekunde
    switch (type) {
    case UnitType::Warrior: return { 20, 15, 0, 1 };
    case UnitType::Archer:  return { 40, 50, 0, 2 };
    case UnitType::Hero:    return { 300, 100, 100, 5 };
    default:                return { 0, 0 , 0, 0 };
    }
}

static Cost getBuildingCost(BuildingType type) {
    switch (type) {
    case BuildingType::TownHall:  return { 0,0,0,0 };
    case BuildingType::Quarry:    return { 20,100,50,0 };
    case BuildingType::Barracks:  return { 50,200,150,0 };
    case BuildingType::Foresters: return { 20,100,50,0 };
    case BuildingType::GoldMine:  return { 20,100,50,0 };
	case BuildingType::Farm:      return { 20,100,50,0 };
	case BuildingType::Fence:	  return { 0,10,5,0 };
    default:                      return { 0,0,0,0 };
    }
}

static Cost getUpgradeCost(BuildingType type, unsigned short lvl) {
    switch (type) {
    case BuildingType::Barracks:
        if (lvl == 2)            return { 0,0,0,0 };
        else if (lvl == 3)       return { 0,0,0,0 };
		else                     return { 0,0,0,0 };
    default:                     return { 0,0,0,0 };
    }
}

static float getTrainingTime(UnitType type) {
    switch (type) {
    case UnitType::Warrior: return 10.f;
    case UnitType::Archer:  return 15.f;
    case UnitType::Hero:    return 30.f;
    default:                return 3.f;
    }
}



class Building {
public:
    Building(sf::Vector2f position, int hp);
    virtual ~Building();
    Building(const Building& other); 

    virtual void update(float dt, Player& p) = 0;
    virtual void draw(sf::RenderWindow& window);



    sf::Vector2f getPosition() const { return m_position; }
    std::pair<int, int> getHealth() const;
    void takeDamage(int amount) { m_health -= amount; }
    bool isDestroyed() const { return m_health <= 0; }
    sf::FloatRect getBounds() const;
    sf::Vector2f getSize() const;
    void setSelected(bool selected);
    bool isSelected() const;
    BuildingType getType() const;
    sf::Color getColor();

protected:
    sf::Vector2f m_position;
    sf::RectangleShape m_shape;
    BuildingType m_type;
    int m_health;
	int m_maxHealth;
	bool m_isSelected = false;
};

class TownHall : public Building {
public:
    TownHall(sf::Vector2f pos);
    virtual ~TownHall();
    TownHall(const TownHall& other) = default;
    void update(float dt, Player& p) override;
protected:
    int m_maxHealth = 2000;
};

class Quarry : public Building {
public:
    Quarry(sf::Vector2f pos);
    virtual ~Quarry();
    Quarry(const Quarry& other) = default;
	void update(float dt, Player& p) override;
protected:
    int m_maxHealth = 400;
private:
	float m_miningTimer;
};

class Barracks : public Building {
public:
    Barracks(sf::Vector2f pos);
    virtual ~Barracks();
    Barracks(const Barracks& other) = default;
	void update(float dt,Player& p) override;

	void startTraining(UnitType type);
	void upgrade(Player& p);
    bool isTraining() const;
    UnitType getTrainingType() const;
	unsigned short getLvl() const { return m_lvl; }

    float getTrainProgress();
protected:
    int m_maxHealth = 800;

private:
    UnitType m_currentTraining = UnitType::Worker;

    float m_trainingTimer = 0.f;
	bool m_isTraining = false;
    unsigned short m_lvl = 1;
};

class GoldMine : public Building {
public:
    GoldMine(sf::Vector2f pos);
    virtual ~GoldMine();
    GoldMine(const GoldMine& other) = default;
    void update(float dt, Player& p) override;
protected:
    int m_maxHealth = 400;
private:
	float m_miningTimer;
};

class Foresters : public Building {
public:
    Foresters(sf::Vector2f pos);
    virtual ~Foresters();
    Foresters(const Foresters& other) = default;
    void update(float dt, Player& p) override;
protected:
    int m_maxHealth = 400;
private:
    float m_miningTimer;
};

class Farm : public Building {
public:
    Farm(sf::Vector2f pos);
    virtual ~Farm();
    Farm(const Farm& other) = default;
    void update(float dt, Player& p) override;
protected:
    int m_maxHealth = 400;
private:

    float m_miningTimer;
};

class Fence : public Building {
public:
    Fence(sf::Vector2f pos);
    virtual ~Fence();
    Fence(const Fence& other) = default;
    void update(float dt, Player& p) override;
    void draw(sf::RenderWindow& window) override;

    void updateSegment(const std::vector<std::unique_ptr<Building>>& allBuildings);
    FenceStates getFenceState() const;
	static void setAtlas(sf::Texture* atlas) { fenceAtlas = atlas; }
protected:
    int m_maxHealth = 150;
private:
	FenceStates m_state = FenceStates::Front;
	static sf::Texture* fenceAtlas;

	bool hasFenceNeighbor(const sf::Vector2f& pos,const std::vector<std::unique_ptr<Building>>& buildings );
};

static FenceStates getFenceStateOnPos(sf::Vector2f pos, const std::vector<std::unique_ptr<Building>>& buildings) {
    for (auto& b : buildings) {
        if (b->getType() == BuildingType::Fence && b->getPosition() == pos) {
            Fence* fence = dynamic_cast<Fence*>(b.get());
            return fence->getFenceState();
        }
    }
    return FenceStates::Front;
}