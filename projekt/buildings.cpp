#include "buildings.h"
#include "player.h"
#include "utils.hpp"
#include <iostream>


Building::Building(sf::Vector2f pos, int hp) : m_position(pos), m_health(hp), m_maxHealth(hp), m_type(BuildingType::TownHall) {
	m_shape.setSize(defaultSize(m_type));
    m_shape.setPosition(m_position);
}

Building::Building(const Building& other) = default;
Building::~Building() = default;

void Building::draw(sf::RenderWindow& window) {
    window.draw(m_shape);
	drawHealthBar(window, m_position, static_cast<float>(m_health) / m_maxHealth);
}

std::pair<int, int> Building::getHealth() const
{
    return {m_health, m_maxHealth};
}

sf::FloatRect Building::getBounds() const
{
    return m_shape.getGlobalBounds();
}

sf::Vector2f Building::getSize() const
{
    return m_shape.getSize();
}

sf::Vector2f Building::defaultSize(BuildingType type)
{
    switch (type) {
	case BuildingType::TownHall: return { 120.f, 120.f };
	case BuildingType::Quarry:   return { 80.f, 80.f };
	default:                     return { 40.f, 40.f };
    }
}

void Building::setSelected(bool selected)
{
	m_isSelected = selected;
}

bool Building::isSelected() const
{
    return m_isSelected;
}

BuildingType Building::getType() const
{
    return m_type;
}

sf::Color Building::getColor()
{
	return m_shape.getFillColor();
}

//QUARRY

Quarry::Quarry(sf::Vector2f pos) : Building(pos, 500) {
    m_type = BuildingType::Quarry;
	m_shape.setFillColor(sf::Color(139, 69, 19)); // Brązowy 
	m_shape.setOutlineThickness(1.f);
	m_shape.setOutlineColor(sf::Color(205, 133, 63)); // Jasnobrązowy
	m_shape.setOrigin({ 20.f, 20.f });
	m_miningTimer = 1.f;
}

Quarry::~Quarry() = default;

void Quarry::update(float dt, Player& p) {
    m_miningTimer -= dt;
    if (m_miningTimer <= 0.f) {
        p.addResource(ResourceType::Rock, 5);
		std::cout << "+5 kamienia zebrane przez Quarry!" << std::endl;
        m_miningTimer = 1.f; 
    }

    if (m_isSelected) {
        m_shape.setOutlineColor(sf::Color::White);
    }
    else {
        m_shape.setOutlineColor(sf::Color(205, 133, 63));
    }
}

//BARRACKS

Barracks::Barracks(sf::Vector2f pos) : Building(pos, 800) {
    m_type = BuildingType::Barracks;
    m_shape.setSize({ 50.f, 50.f });
    m_shape.setFillColor(sf::Color(128, 128, 128)); // Szary 
    m_shape.setOutlineThickness(1.f);
	m_shape.setOutlineColor(sf::Color(211, 211, 211));  //Jasnoszary
    m_shape.setOrigin({ 25.f, 25.f });

}

Barracks::~Barracks() = default;

void Barracks::update(float dt, Player& p) {
    if (m_isTraining) {
        m_trainingTimer -= dt;
        if (m_trainingTimer <= 0.f) {
            std::cout << toString(m_currentTraining) << "wytrenowany!" << std::endl;

            sf::Vector2f spawnPos = m_position + sf::Vector2f(60.f, 0.f);

            p.addUnit(spawnPos, m_currentTraining);

            m_isTraining = false;
            m_trainingTimer = 0.f;
        }
    }
    if (m_isSelected) {
        m_shape.setOutlineColor(sf::Color::White);
    }
    else {
        m_shape.setOutlineColor(sf::Color(211, 211, 211));
    }
    
}

void Barracks::startTraining(UnitType type) {
    if (m_isTraining) {
        std::cout << "Baraki są zajęte! Poczekaj na zakończenie treningu." << std::endl;
    }
    m_currentTraining = type;
    m_trainingTimer = getTrainingTime(type);
    m_isTraining = true;
    m_isTraining = true;
    std::cout << "Rozpoczęto trening" << toString(type) << std::endl;
}

void Barracks::upgrade(Player& p)
{
    switch (m_lvl) {
    case 1:
        p.spendResource(ResourceType::Gold, 100);
        p.spendResource(ResourceType::Wood, 1000);
        p.spendResource(ResourceType::Gold, 1000);
        m_lvl++;
        break;
    case 2:
        p.spendResource(ResourceType::Gold, 300);
        p.spendResource(ResourceType::Wood, 2000);
        p.spendResource(ResourceType::Gold, 2000);
        m_lvl++;
        break;
    default:
        std::cout << "Maksymalny poziom barakow zostal osiagniety" << std::endl;
    }
    
}

bool Barracks::isTraining() const
{
    return m_isTraining;
}

UnitType Barracks::getTrainingType() const
{
    return m_currentTraining;
}

float Barracks::getTrainProgress()
{
    if (!m_isTraining) return 0.f;
    float total = getTrainingTime(m_currentTraining);
    return 1.f - (m_trainingTimer / total);
}



//TOWNHALL

TownHall::TownHall(sf::Vector2f pos) : Building(pos, 2000) {
	m_type = BuildingType::TownHall;
    m_shape.setSize({ 100.f, 100.f });
    m_shape.setFillColor(sf::Color(0, 51, 102)); //Granatowy
    m_shape.setOutlineThickness(1.f);
	m_shape.setOutlineColor(sf::Color(0, 102, 204)); //Jasny granatowy
    m_shape.setOrigin({ 50.f, 50.f });
}

TownHall::~TownHall() = default;

void TownHall::update(float dt, Player& p) {
    if (m_isSelected) {
        m_shape.setOutlineColor(sf::Color::White);
    }
    else {
        m_shape.setOutlineColor(sf::Color(0,102,204));
    }
}

//GOLDMINE

GoldMine::GoldMine(sf::Vector2f pos) : Building(pos, 500) {
    m_type = BuildingType::GoldMine;
    m_shape.setFillColor(sf::Color(255, 237, 0)); // Złoty 
    m_shape.setOutlineThickness(1.f);
    m_shape.setOutlineColor(sf::Color(254, 254, 51)); //Bananowy
    m_shape.setOrigin({ 20.f, 20.f });
    m_miningTimer = 1.f;
}
GoldMine::~GoldMine() = default;

void GoldMine::update(float dt, Player& p) {
    m_miningTimer -= dt;
    if (m_miningTimer <= 0.f) {
        p.addResource(ResourceType::Gold, 1);
        std::cout << "+1 złota zebrane przez GoldMine!" << std::endl;
        m_miningTimer = 1.f; 
    }

    if (m_isSelected) {
        m_shape.setOutlineColor(sf::Color::White);
    }
    else {
        m_shape.setOutlineColor(sf::Color(254, 237, 0));
    }

}

//FORESTERS

Foresters::Foresters(sf::Vector2f pos) : Building(pos, 500) {
    m_type = BuildingType::Foresters;
    m_shape.setFillColor(sf::Color(34, 139, 34)); // Zielony
    m_shape.setOutlineThickness(1.f);
	m_shape.setOutlineColor(sf::Color(83, 236, 83)); // limonkowy
    m_shape.setOrigin({ 20.f, 20.f });
    m_miningTimer = 1.f;
}

Foresters::~Foresters() = default;

void Foresters::update(float dt, Player& p) {
    m_miningTimer -= dt;
    if (m_miningTimer <= 0.f) {
        p.addResource(ResourceType::Wood, 5);
        std::cout << "+5 drewna zebrane przez Foresters!" << std::endl;
        m_miningTimer = 1.f; 
    }

    if (m_isSelected) {
        m_shape.setOutlineColor(sf::Color::White);
    }
    else {
        m_shape.setOutlineColor(sf::Color(83, 236, 83));
    }
}

