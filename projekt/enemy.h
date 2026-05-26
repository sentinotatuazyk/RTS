#pragma once
#include <SFML/Graphics.hpp>
#include "API.h"
class Game;

enum class EnemyType { Goblin, Orc, Troll };
enum class EnemyState { Aggressive, Passive, Neutral  };

class EXPORT_API Enemy
{
	friend class Game;
public:
	Enemy();
	~Enemy();		
	Enemy(const Enemy& other);
	Enemy& operator=(const Enemy& other);

	void update(float dt);
	void draw(sf::RenderWindow& window);
	void spawn(sf::Vector2f position, EnemyType type);
	sf::Vector2f getPosition() const;
	void setPosition(sf::Vector2f position);
	void moveTo(sf::Vector2f target);
	float getRadius() const;
	void stop();
	void setState(EnemyState state);

	EnemyState getState() const;

	void takeDamage(int dmg);
	bool isDead() const;
	
private:
	sf::Vector2f m_position;
	sf::CircleShape m_shape;
	sf::Texture m_texture;

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

