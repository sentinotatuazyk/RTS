#include "buildings.h"
#include "player.h"
#include "utils.h"
#include <iostream>


Building::Building(sf::Vector2f pos, int hp) : m_position(pos), m_health(hp), m_maxHealth(hp), m_type(BuildingType::TownHall) {
	m_shape.setSize(buildingSize(m_type));
    m_shape.setPosition(m_position);
}

Building::Building(const Building& other) = default;
Building::~Building() = default;

void Building::draw(sf::RenderWindow& window) {
    if (!isDestroyed()) {
        window.draw(m_shape);
        drawHealthBar(window, m_position, static_cast<float>(m_health) / m_maxHealth);
    }
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
    if (m_health <= 0) p.changeTownHallStatus(false);
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

Farm::Farm(sf::Vector2f pos) : Building(pos, 500) {
    m_type = BuildingType::Farm;
    m_shape.setFillColor(sf::Color(255, 0, 255)); // Magenta
    m_shape.setOutlineThickness(1.f);
    m_shape.setOutlineColor(sf::Color(255, 102, 255)); // Różowy
    m_shape.setOrigin({ 20.f, 20.f });
    m_miningTimer = 1.f;
}
Farm::~Farm() = default;

void Farm::update(float dt, Player& p) {
    m_miningTimer -= dt;
    if (m_miningTimer <= 0.f) {
        p.addResource(ResourceType::Food, p.calculateFoodIncome() / p.getBuildingCount(BuildingType::Farm));
        m_miningTimer = 1.f;
    }

    if (m_isSelected) {
        m_shape.setOutlineColor(sf::Color::White);
    }
    else {
        m_shape.setOutlineColor(sf::Color(255, 102, 255));
    }
}

sf::Texture* Fence::fenceAtlas = nullptr;

Fence::Fence(sf::Vector2f pos) : Building(pos, 300) {
    m_type = BuildingType::Fence;
    m_shape.setSize({ 32.f, 32.f });
    m_shape.setOrigin({10.f,10.f });

    if (fenceAtlas) {
        m_shape.setTexture(fenceAtlas);
    }
}

Fence::~Fence()
{
}

void Fence::update(float dt, Player& p)
{
}

void Fence::draw(sf::RenderWindow& window)
{
    window.draw(m_shape);
}

void Fence::updateSegment(const std::vector<std::unique_ptr<Building>>& allBuildings)
{
    sf::Vector2f pos = getPosition();
    float size = 32.f; // odległość między środkami sąsiednich płotów

    bool left = hasFenceNeighbor({ pos.x - size, pos.y }, allBuildings);
    bool right = hasFenceNeighbor({ pos.x + size, pos.y }, allBuildings);
    bool up = hasFenceNeighbor({ pos.x, pos.y - size }, allBuildings);
    bool down = hasFenceNeighbor({ pos.x, pos.y + size }, allBuildings);

    bool upup = false, upleft = false, upright = false, downdown = false, downleft = false, downright = false, leftleft = false, leftup = false, leftdown = false, rightright = false, rightup = false, rightdown = false;

    bool isUpOnRight = false, isDownOnRight = false;
    if (up) {
        upup = hasFenceNeighbor({ pos.x, pos.y - size * 2 }, allBuildings);
        upleft = hasFenceNeighbor({ pos.x - size, pos.y - size }, allBuildings);
        upright = hasFenceNeighbor({ pos.x + size, pos.y - size }, allBuildings);
		if (getFenceStateOnPos({ pos.x, pos.y - size }, allBuildings) == FenceStates::RightSide) isUpOnRight = true;
    }

    if (down) {
        downdown = hasFenceNeighbor({ pos.x, pos.y + size * 2 }, allBuildings);
        downleft = hasFenceNeighbor({ pos.x - size, pos.y + size }, allBuildings);
        downright = hasFenceNeighbor({ pos.x + size, pos.y + size }, allBuildings);
        if (getFenceStateOnPos({ pos.x, pos.y + size }, allBuildings) == FenceStates::RightSide) isDownOnRight = true;
    }

    if (left) {
        leftleft = hasFenceNeighbor({ pos.x - size * 2, pos.y }, allBuildings);
        leftup = hasFenceNeighbor({ pos.x - size, pos.y - size }, allBuildings);
        leftdown = hasFenceNeighbor({ pos.x - size, pos.y + size }, allBuildings);
    }

    if (right) {
        rightright = hasFenceNeighbor({ pos.x + size * 2, pos.y }, allBuildings);
        rightup = hasFenceNeighbor({ pos.x + size, pos.y - size }, allBuildings);
        rightdown = hasFenceNeighbor({ pos.x + size, pos.y + size }, allBuildings);
    }

    // Atlas 2x4, sprite 32x32:
    //       x=0        x=32
    // y=0:  [0]Front   [1]LeftBendDown
    // y=32: [2]RightBD [3]LeftBendUp
    // y=64: [4]LeftSide [5]RightBendUp
    // y=96: [6]RightSide [pusty]

    int spriteIndex = 0;

    if ((left || right) && !up && !down) {        // ✅ NAWIASY
        m_state = FenceStates::Front;             // ─
        spriteIndex = 0;
    }
    else if (up && left && !right && !down) {
        m_state = FenceStates::LeftBendDown;      // ┐ ✅ POPRAWIONE
        spriteIndex = 3;
    }
    else if (up && right && !left && !down) {
        m_state = FenceStates::RightBendDown;     // ┌
        spriteIndex = 5;
    }
    else if (down && left && !right && !up) {
        m_state = FenceStates::LeftBendUp;        // ┘ ✅ POPRAWIONE
        spriteIndex = 1;
    }
    else if (up && down && left && !right) {
        m_state = FenceStates::LeftSide;          // │ (sąsiad z lewej)
        spriteIndex = 4;
    }
    else if (down && right && !left && !up) {
        m_state = FenceStates::RightBendUp;       // └
        spriteIndex = 2;
    }
    else if (((up && upleft) || (down && downleft)) && !left && !right || (isUpOnRight || isDownOnRight)) {
        m_state = FenceStates::RightSide;         // │ (sąsiad z prawej)
        spriteIndex = 6;
    }
    else if ((up || down) && !left && !right) {
        m_state = FenceStates::LeftSide;            // │ domyślnie
        spriteIndex = 4;
    }
    else {
        m_state = FenceStates::Front;               // ─ domyślnie
        spriteIndex = 0;
    }

    int col = spriteIndex % 2;
    int row = spriteIndex / 2;
    int atlasX = col * 32;
    int atlasY = row * 32;

    m_shape.setTextureRect(sf::IntRect({ atlasX, atlasY }, { 32, 32 }));

    std::cout << "Fence at (" << pos.x << "," << pos.y << ") "
        << "L:" << left << " R:" << right << " U:" << up << " D:" << down
        << " -> sprite:" << spriteIndex
        << " rect:(" << atlasX << "," << atlasY << ")"
		<< std::endl << "===========================" << std::endl;
}

FenceStates Fence::getFenceState() const
{
    return m_state;
}

bool Fence::hasFenceNeighbor(const sf::Vector2f& pos, const std::vector<std::unique_ptr<Building>>& buildings)
{
    for (const auto& b : buildings) {
        if (b.get() == this) continue; // pomiń samego siebie
        if (b->getType() != BuildingType::Fence) continue;

        // Sprawdź czy pozycja się pokrywa (z małą tolerancją)
        float dx = std::abs(b->getPosition().x - pos.x);
        float dy = std::abs(b->getPosition().y - pos.y);
        if (dx < 1.f && dy < 1.f) return true;
    }
    return false;
}



