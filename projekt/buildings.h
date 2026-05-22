#pragma once

#include "API.h"
#include <SFML/Graphics.hpp>

enum class BuildingType { TownHall, Quarry, Barracks };

class Building {
public:
    Building(sf::Vector2f position, int hp);
    virtual ~Building();
    Building(const Building& other); 

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window);



    sf::Vector2f getPosition() const { return m_position; }
    int getHealth() const { return m_health; }
    void takeDamage(int amount) { m_health -= amount; }
    bool isDestroyed() const { return m_health <= 0; }

protected:
    sf::Vector2f m_position;
    sf::RectangleShape m_shape;
    BuildingType m_type;
    int m_health;
	int m_maxHealth;
};

class TownHall : public Building {
public:
    TownHall(sf::Vector2f pos);
    virtual ~TownHall();
    TownHall(const TownHall& other) = default;
    void update(float dt) override;
};

class  Quarry : public Building {
public:
    Quarry(sf::Vector2f pos);
    virtual ~Quarry();
    Quarry(const Quarry& other) = default;
	void update(float dt) override;
private:
	float m_miningTimer;

};

class Barracks : public Building {
public:
    Barracks(sf::Vector2f pos);
    virtual ~Barracks();
    Barracks(const Barracks& other) = default;
	void update(float dt) override;
	void startTraining();

private:
	float m_trainingTimer;
	bool m_isTraining;
};