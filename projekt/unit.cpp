#include "unit.h"

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

void Unit::spawn(sf::Vector2f position,UnitType type) {
    m_position = position;
    m_type = type;
    m_shape.setPosition(m_position);
    m_isMoving = false;

    if (type == UnitType::Worker) {
        m_speed = 150.f;
        m_health = 100;
        m_shape.setRadius(10.f);
        m_shape.setFillColor(sf::Color::Red);
        m_shape.setOrigin({ 10.f, 10.f });
    }
    else if (type == UnitType::Warrior) {
        m_speed = 110.f;
        m_health = 300;
        m_shape.setRadius(15.f);
        m_shape.setFillColor(sf::Color::Blue);
        m_shape.setOrigin({ 15.f, 15.f });
    }
    else if (type == UnitType::Archer) {
        m_speed = 160.f;
        m_health = 80;
        m_shape.setRadius(12.f);
        m_shape.setFillColor(sf::Color::Green);
        m_shape.setOrigin({ 12.f, 12.f });
    }
    else if (type == UnitType::Hero) {
        m_speed = 180.f;
        m_health = 500;
        m_shape.setRadius(18.f);
        m_shape.setFillColor(sf::Color::Yellow);
        m_shape.setOutlineThickness(-1.f);
        m_shape.setOutlineColor(sf::Color::Magenta);
        m_shape.setOrigin({ 18.f, 18.f });
    }
    m_shape.setPosition(m_position);
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

void Unit::stop()
{
    m_isMoving = false;
    m_targetPosition = m_position;
}

sf::FloatRect Unit::getBounds() const
{
    return m_shape.getGlobalBounds();
}

void Unit::update(float dt) {
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
}

void Unit::moveTo(sf::Vector2f target) {
    m_targetPosition = target;
    m_isMoving = true;
}




