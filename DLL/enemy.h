#pragma once
#include <SFML/Graphics.hpp>
#include "API.h"
#include "enums.h"
class Game;
class Map;

class Enemy
{
	friend class Game;
public:
	Enemy();
	~Enemy();		
	Enemy(const Enemy& other);
	Enemy& operator=(const Enemy& other);

	void update(float dt, const Map& map);
	void draw(sf::RenderWindow& window);
	void spawn(sf::Vector2f position, EnemyType type);
	sf::Vector2f getPosition() const;
	void setPosition(sf::Vector2f position);
	void moveTo(sf::Vector2f target,const Map& map);
	float getRadius() const;
	void stop();
	void setState(EnemyState state);
	EnemyType getType();

	EnemyState getState() const;

	void takeDamage(int dmg);
	bool isDead() const;
	
	void setPath(const std::vector<sf::Vector2f>& path);
	void clearPath();
	bool hasPath() const;
	void followPath(float dt, float speed, const Map& map);
private:
	sf::Vector2f m_position;
	sf::CircleShape m_shape;
	sf::Texture m_texture;

	std::vector<sf::Vector2f> m_path;
	std::size_t m_pathIndex = 0;
	static constexpr float PATH_REACHED_DIST = 5.f;

	EnemyType m_type;
	EnemyState m_state = EnemyState::Aggressive;

	bool m_isMoving = false;
	bool m_isDead = false;

	//STATYSTYKI
	float m_speed;
	int m_health;
	int m_maxHealth;
	int m_damage;
	float m_attackRange;
	float m_attackCooldown;
	float m_attackTimer;

	sf::Vector2f m_targetPosition;
};

