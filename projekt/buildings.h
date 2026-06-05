#pragma once

#include "API.h"
#include <SFML/Graphics.hpp>
#include <utility>
#include "../../SFML-3.1.0/include/SFML/Graphics/Color.hpp"

class Player;


enum class EXPORT_API BuildingType { TownHall, Quarry, Barracks, GoldMine, Foresters };

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
	static sf::Vector2f defaultSize(BuildingType type);
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
};

class Quarry : public Building {
public:
    Quarry(sf::Vector2f pos);
    virtual ~Quarry();
    Quarry(const Quarry& other) = default;
	void update(float dt, Player& p) override;
private:
	float m_miningTimer;

};

class Barracks : public Building {
public:
    Barracks(sf::Vector2f pos);
    virtual ~Barracks();
    Barracks(const Barracks& other) = default;
	void update(float dt,Player& p) override;
	void startTraining();

private:
	float m_trainingTimer;
	bool m_isTraining;
};

class GoldMine : public Building {
public:
    GoldMine(sf::Vector2f pos);
    virtual ~GoldMine();
    GoldMine(const GoldMine& other) = default;
    void update(float dt, Player& p) override;
private:
	float m_miningTimer;
};

class Foresters : public Building {
public:
    Foresters(sf::Vector2f pos);
    virtual ~Foresters();
    Foresters(const Foresters& other) = default;
    void update(float dt, Player& p) override;
private:
    float m_miningTimer;
};