#include "buildings.h"

Building::Building(sf::Vector2f position, BuildingType type)
    : m_position(position), m_type(type)
{
    m_shape.setPosition(m_position);

    // Konfigurujemy budynek w zależności od jego typu
    if (m_type == BuildingType::TownHall) {
        m_health = 1000;
        m_shape.setSize({ 80.f, 80.f }); // Ratusz jest duży
        m_shape.setFillColor(sf::Color::Blue);
        m_shape.setOrigin({ 40.f, 40.f }); // Środek
    }
    else if (m_type == BuildingType::Quarry) {
        m_health = 500;
        m_shape.setSize({ 50.f, 50.f }); // Kamieniołom jest mniejszy
        m_shape.setFillColor(sf::Color(128, 128, 128)); // Szary
        m_shape.setOrigin({ 25.f, 25.f });
    }
}

Building::~Building() {}

// Niezbędne dla wektora i DLL-ki
Building::Building(const Building& other) = default;

void Building::update(float dt) {
    // Budynki zazwyczaj stoją w miejscu, więc update może na razie być pusty.
    // W przyszłości tu będziemy np. generować surowce (Quarry) co kilka sekund.
}

void Building::draw(sf::RenderWindow& window) {
    window.draw(m_shape);
}