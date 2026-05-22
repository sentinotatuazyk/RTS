#pragma once

#include "API.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
class Game;

enum class UnitType {Worker, Warrior, Archer, Hero};

class EXPORT_API Unit 
{
	friend class Game;
public:
	Unit();
	~Unit();
	Unit(const Unit& other);
	Unit& operator=(const Unit& other);
	void update(float dt);
	void draw(sf::RenderWindow& window);
	void moveTo(sf::Vector2f target);
	void spawn(sf::Vector2f position, UnitType type);
	void setSelected(bool selected);
	bool isSelected() const;
	sf::Vector2f getPosition() const;
	void setPosition(sf::Vector2f position);
	float getRadius() const;
	void stop();

	sf::FloatRect getBounds() const;

	void takeDamage(int dmg);
	bool isDead() const;
	
private:
	sf::Vector2f m_position;
	sf::CircleShape m_shape;
	

	UnitType m_type;

	bool m_isMoving = false;
	bool m_isDead = false;
	bool m_isSelected = false;

	sf::Vector2f m_targetPosition;

	//STATYSTYKI
	float m_speed;
	int m_health;
	int m_maxHealth;
	int m_damage;
	float m_attackRange;
	float m_attackCooldown;
	float m_attackTimer;

};