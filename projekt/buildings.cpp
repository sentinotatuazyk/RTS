#include "buildings.h"
#include "utils.hpp"
#include <iostream>

Building::Building(sf::Vector2f pos, int hp) : m_position(pos), m_health(hp), m_maxHealth(hp) {
    m_shape.setPosition(m_position);
}

// Niezbędne dla wektora i DLL-ki
Building::Building(const Building& other) = default;
Building::~Building() = default;

void Building::draw(sf::RenderWindow& window) {
    window.draw(m_shape);
	drawHealthBar(window, m_position, static_cast<float>(m_health) / m_maxHealth);
}

Quarry::Quarry(sf::Vector2f pos) : Building(pos, 500) {
    m_shape.setSize({ 40.f, 40.f });
	m_shape.setFillColor(sf::Color(139, 69, 19)); // Brązowy 
	m_shape.setOrigin({ 20.f, 20.f });
	m_miningTimer = 1.f;
}

Quarry::~Quarry() = default;

void Quarry::update(float dt) {
    m_miningTimer -= dt;
    if (m_miningTimer <= 0.f) {
		std::cout << "+5 kamienia zebrane przez Quarry!" << std::endl;
        m_miningTimer = 1.f; // Resetujemy timer
    }
}

Barracks::Barracks(sf::Vector2f pos) : Building(pos, 800) {
    m_shape.setSize({ 50.f, 50.f });
    m_shape.setFillColor(sf::Color(128, 128, 128)); // Szary 
    m_shape.setOrigin({ 25.f, 25.f });

	m_trainingTimer = 0.f;
	m_isTraining = false;
}

Barracks::~Barracks() = default;

void Barracks::update(float dt) {
    if (m_isTraining) {
        m_trainingTimer -= dt;
        if (m_trainingTimer <= 0.f) {
            std::cout << "Jednostka wytrenowana w Barracks!" << std::endl;
            m_isTraining = false; 
        }
    }
}

void Barracks::startTraining() {
    if (!m_isTraining) {
        m_trainingTimer = 3.f; // Czas treningu jednostki
        m_isTraining = true;
        std::cout << "Rozpoczęto trening jednostki w Barracks!" << std::endl;
    }
}

TownHall::TownHall(sf::Vector2f pos) : Building(pos, 2000) {
    m_shape.setSize({ 100.f, 100.f });
    m_shape.setFillColor(sf::Color(0, 51, 102)); //Granatowy 
    m_shape.setOrigin({ 50.f, 50.f });
}

TownHall::~TownHall() = default;

void TownHall::update(float dt) {
}