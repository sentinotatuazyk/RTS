#pragma once

#include "API.h"
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include "enums.h"
#include <cmath>
#include <utility>

class Game;

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

	void setState(UnitState state);

	sf::FloatRect getBounds() const;
	std::pair<int, int> getHealth() const;
	UnitType getType() const;
	UnitState getState() const;

	void takeDamage(int dmg);
	bool isDead() const;

	void startBuildJob(BuildingType type, sf::Vector2f pos);
	bool hasBuildJob() const;
	bool isBuildingNow() const;
	BuildingType buildJobType() const;
	sf::Vector2f buildJobPos() const;
	bool consumeBuildFinishedFlag();

	float getBuildProgress01() const;
	sf::Vector2f getBuildSitePos() const; 

private:
	sf::Vector2f m_position;
	sf::CircleShape m_shape;
	

	UnitType m_type;
	UnitState m_state = UnitState::Neutral;

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

	//Worker
	bool m_buildJobActive = false;
	bool m_buildingNow = false;
	BuildingType m_buildType = BuildingType::Quarry;
	sf::Vector2f m_buildPos{ 0.f,0.f };

	float m_buildTimer = 0.f;
	float m_buildDuration = 3.f;

	bool m_buildFinished = false;




};