#include "enemy.h"
#include "utils.hpp"
Enemy::Enemy() : m_speed(150.f), m_health(100), m_isMoving(false), m_isDead(false)
{
    m_shape.setRadius(15.f);
 

    // Ustawiamy środek ciężkości (Origin) na sam środek kółka,
    // to bardzo ułatwi potem pozycjonowanie i obracanie!
    m_shape.setOrigin({ 15.f, 15.f });
}

Enemy::~Enemy()
{
}

Enemy::Enemy(const Enemy& other) = default;

Enemy& Enemy::operator=(const Enemy& other) = default;

void Enemy::update(float dt)
{
    if (m_attackTimer > 0.f) {
        m_attackTimer -= dt;
    }
    if (m_isMoving) {
        sf::Vector2f direction = m_targetPosition - m_position;
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (distance < 5.f) {
            m_isMoving = false;
            m_position = m_targetPosition;
        }
        else {
            sf::Vector2f normalizedDir = direction / distance;
            m_position += normalizedDir * m_speed * dt;
        }
    }
    m_shape.setPosition(m_position);
}

void Enemy::draw(sf::RenderWindow& window)
{
	window.draw(m_shape);
    if (m_health != m_maxHealth)    drawHealthBar(window, m_position, static_cast<float>(m_health) / m_maxHealth); 
}

void Enemy::spawn(sf::Vector2f position, EnemyType type)
{
    m_position = position;
    m_type = type;
    m_shape.setPosition(m_position);
    m_isMoving = false;
    m_attackTimer = 0.f;

    switch(type){
    case EnemyType::Goblin:
        m_speed = 150.f;
        m_health = 100;
        m_damage = 20;
        m_attackRange = 90.f;
        m_attackCooldown = 1.3f;
        m_shape.setRadius(15.f);
        m_shape.setFillColor(DarkRed);
        m_shape.setOutlineColor(sf::Color::Red);
        m_shape.setOutlineThickness(-2.f);
        m_shape.setOrigin({ 15.f, 15.f });
        break;
    case EnemyType::Orc:
        m_speed = 110.f;
        m_health = 300;
        m_damage = 21;
        m_attackRange = 35.f;
        m_attackCooldown = 1.f;
        m_shape.setRadius(20.f);
        m_shape.setFillColor(DarkRed);
        m_shape.setOutlineColor(sf::Color::Red);
        m_shape.setOrigin({ 20.f, 20.f });
        break;
    case EnemyType::Troll:
        m_speed = 80.f;
        m_health = 500;
        m_damage = 60;
        m_attackRange = 45.f;
        m_attackCooldown = 1.f;
        m_shape.setRadius(25.f);
        m_shape.setFillColor(DarkRed);
        m_shape.setOutlineColor(sf::Color::Red);
        m_shape.setOrigin({ 25.f, 25.f });
        break;  
    }
    m_shape.setPosition(m_position);
    m_maxHealth = m_health;
}

sf::Vector2f Enemy::getPosition() const
{
	return m_position;
}

void Enemy::setPosition(sf::Vector2f position)
{
	m_position = position;
	m_targetPosition = position;
}

void Enemy::moveTo(sf::Vector2f target)
{
    m_targetPosition = target;
    m_isMoving = true;
}

float Enemy::getRadius() const
{
    return m_shape.getRadius();
}

void Enemy::stop()
{
    m_isMoving = false;
    m_targetPosition = m_position;
}

void Enemy::takeDamage(int dmg) {
    m_health -= dmg;
    if (m_health < 0) m_health = 0;
}

bool Enemy::isDead() const {
    return m_health <= 0;
}
