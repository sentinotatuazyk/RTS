#include "unit.h"
#include "utils.hpp"
// Inicjalizujemy podstawowe parametry
Unit::Unit() : m_speed(150.f), m_health(100), m_isMoving(false), m_isDead(false) {
    // Ustawiamy wygląd: czerwone kółko o promieniu 15 pikseli
    m_shape.setRadius(15.f);
    m_shape.setFillColor(sf::Color::Red);

    // Ustawiamy środek ciężkości (Origin) na sam środek kółka,
    // to bardzo ułatwi potem pozycjonowanie i obracanie!
    m_shape.setOrigin({ 15.f, 15.f });
}

Unit::~Unit() {}

Unit::Unit(const Unit& other) = default;

Unit& Unit::operator=(const Unit& other) = default;

void Unit::spawn(sf::Vector2f position,UnitType type) {
    m_position = position;
    m_type = type;
    m_shape.setPosition(m_position);
    m_isMoving = false;

    switch (type) {
    case UnitType::Worker:
        m_speed = 150.f;
        m_health = 100;
        m_damage = 5;
        m_attackRange = 35.f;
        m_attackCooldown = 1.f;
        m_shape.setRadius(10.f);
        m_shape.setFillColor(sf::Color::Red);
        m_shape.setOrigin({ 10.f, 10.f });
        break;
    case UnitType::Warrior:
        m_speed = 110.f;
        m_health = 300;
        m_damage = 20;
        m_attackRange = 40.f;
        m_attackCooldown = 1.2f;
        m_shape.setRadius(15.f);
        m_shape.setFillColor(sf::Color::Blue);
        m_shape.setOrigin({ 15.f, 15.f });
        break;
    case UnitType::Archer:
        m_speed = 160.f;
        m_health = 80;
        m_damage = 20;
        m_attackRange = 100.f;
        m_attackCooldown = 0.8f;
        m_shape.setRadius(12.f);
        m_shape.setFillColor(sf::Color::Green);
        m_shape.setOrigin({ 12.f, 12.f });
        break;
    case UnitType::Hero:
        m_speed = 180.f;
        m_health = 500;
        m_damage = 50;
        m_attackRange = 45.f;
        m_attackCooldown = 1.f;
        m_shape.setRadius(18.f);
        m_shape.setFillColor(sf::Color::Yellow);
        m_shape.setOutlineThickness(-1.f);
        m_shape.setOutlineColor(sf::Color::Magenta);
        m_shape.setOrigin({ 18.f, 18.f });
    }
    m_shape.setPosition(m_position);
	m_maxHealth = m_health;
}

void Unit::setSelected(bool selected)
{
    m_isSelected = selected;
    m_shape.setOutlineThickness(m_isSelected ? 2.f : 0.f);
    m_shape.setOutlineColor(sf::Color::White);
}

bool Unit::isSelected() const
{
    return m_isSelected;
}

sf::Vector2f Unit::getPosition() const
{
    return m_position;
}

void Unit::setPosition(sf::Vector2f position)
{
    m_position = position;
    m_shape.setPosition(m_position);
}

float Unit::getRadius() const {
    return m_shape.getRadius();
}

void Unit::stop()
{
    m_isMoving = false;
    m_targetPosition = m_position;
}

sf::FloatRect Unit::getBounds() const
{
    return m_shape.getGlobalBounds();
}

std::pair<int,int> Unit::getHealth() const
{
    return { m_health, m_maxHealth };
}

UnitType Unit::getType() const {
    return m_type;
}

void Unit::update(float dt) {
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

void Unit::draw(sf::RenderWindow& window) {
    window.draw(m_shape);
	drawHealthBar(window, m_position, static_cast<float>(m_health) / m_maxHealth);
}

void Unit::moveTo(sf::Vector2f target) {
    m_targetPosition = target;
    m_isMoving = true;
}

void Unit::takeDamage(int dmg) {
    m_health -= dmg;
	if (m_health < 0) m_health = 0;
}

bool Unit::isDead() const {
	return m_health <= 0;
}




