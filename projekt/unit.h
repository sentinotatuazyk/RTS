#pragma once

#include "API.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

enum class UnitType {Worker, Warrior, Archer, Hero};

class EXPORT_API Unit {
public:
	Unit();
	~Unit();
	Unit(const Unit& other);
	void update(float dt);
	void draw(sf::RenderWindow& window);
	void moveTo(sf::Vector2f target);
	void spawn(sf::Vector2f position, UnitType type);
	void setSelected(bool selected);
	bool isSelected() const;
	sf::Vector2f getPosition() const;
	void setPosition(sf::Vector2f position);
	void stop();

	sf::FloatRect getBounds() const;
private:
	sf::Vector2f m_position;
	sf::CircleShape m_shape;
	float m_speed;
	int m_health;

	UnitType m_type;

	bool m_isMoving = false;
	bool m_isDead = false;
	bool m_isSelected = false;

	sf::Vector2f m_targetPosition;
};