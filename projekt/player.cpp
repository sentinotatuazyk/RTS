#include "player.h"
#include <cmath>
#include <algorithm>
#include "utils.hpp"
#include <iostream>

// W konstruktorze ustawiamy już tylko złoto na 0 (lub inną wartość startową)
Player::Player() : m_gold(0),m_wood(0), m_rocks(0) {
    m_selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    m_selectionBox.setOutlineColor(sf::Color::Green);
    m_selectionBox.setOutlineThickness(1.f);

    m_buildGhost.setFillColor(sf::Color(0, 255, 0, 80));
    m_buildGhost.setOutlineThickness(2.f);
	m_buildGhost.setOutlineColor(sf::Color(0, 255, 0, 180));
}

Player::~Player() {
}

void Player::addGold(int amount) {
    m_gold += amount;
}

unsigned int Player::getResource(ResourceType type) const
{
    switch (type) {
        case ResourceType::Gold:
			return m_gold;
		case ResourceType::Wood:
            return m_wood;
        case ResourceType::Rock:
			return m_rocks;
    }
	return 0; 
}

void Player::addResource(ResourceType type, int amount)
{
    switch (type) {
        case ResourceType::Gold:
            m_gold += amount;
            break;
        case ResourceType::Wood:
            m_wood += amount;
            break;
        case ResourceType::Rock:
            m_rocks += amount;
			break;
    }
}

void Player::spendResource(ResourceType type, int amount)
{
    switch (type) {
    case ResourceType::Gold:
		m_gold -= amount;
        break;
	case ResourceType::Wood:
		m_wood -= amount;
        break;
    case ResourceType::Rock:
        m_rocks -= amount;
        break;
    }
}


void Player::addUnit(sf::Vector2f position,UnitType type) {
    Unit newUnit;
    newUnit.spawn(position, type);
    m_units.push_back(newUnit);
}

bool Player::canAfford(Cost cost) const
{
    return m_gold >= cost.gold && m_wood >= cost.wood && m_rocks >= cost.rock;
}


std::vector<Unit>& Player::getUnits()
{
    return m_units;
}

const std::vector<Unit>& Player::getUnits() const
{
    return m_units;
}

void Player::addBuilding(sf::Vector2f position, BuildingType type)
{
    switch (type) {
        case BuildingType::TownHall:
            if (m_TownHallBuilt) {
				std::cout << "Town Hall already built!" << std::endl;
                return;
            }
			m_TownHallBuilt = true;
            m_buildings.push_back(std::make_unique<TownHall>(position));
            break;
        case BuildingType::Quarry:
            m_buildings.push_back(std::make_unique<Quarry>(position));
            break;
        case BuildingType::Barracks:
            m_buildings.push_back(std::make_unique<Barracks>(position));
			break;
        case BuildingType::GoldMine:
			m_buildings.push_back(std::make_unique<GoldMine>(position));
            break;
        case BuildingType::Foresters:
			m_buildings.push_back(std::make_unique<Foresters>(position));
            break;
        default:
			break;
    }
}

const std::vector <std::unique_ptr<Building>> &Player::getBuildings() const
{
    return m_buildings;
}

bool Player::hasTownHall() const
{
    return m_TownHallBuilt && !m_TownHallDestroyed; 
}

void Player::update(float deltaTime, const Map& map) {
    float tileSize = map.getTileSize();
    float unitRadius = 15.f;

    for (auto& building : m_buildings) {
        building->update(deltaTime, *this);
    }

    for (auto& unit : m_units) {
        unit.update(deltaTime);
        if (unit.consumeBuildFinishedFlag()) {
			addBuilding(unit.buildJobPos(), unit.buildJobType());
        }

        sf::Vector2f pos = unit.getPosition();

        applyMapCollision(pos, unit.getRadius(), map);
        unit.setPosition(pos);
    }


	for (size_t i = 0; i < m_units.size(); ++i) {
		for (size_t j = i + 1; j < m_units.size(); ++j ) {

			sf::Vector2f pos1 = m_units[i].getPosition();
			sf::Vector2f pos2 = m_units[j].getPosition();
			float r1 = m_units[i].getRadius();
			float r2 = m_units[j].getRadius();

			applyCircleCollision(pos1, pos2, r1, r2);
			m_units[i].setPosition(pos1);
			m_units[j].setPosition(pos2);
		}
	}

    // WALIDACJA GHOSTA BUDYNKU
    if (m_placingBuilding) {
        const float tileSize = map.getTileSize();

        m_ghostSize = Building::defaultSize(m_placeType);
        m_ghostPos = snapToTile(m_lastMouseWorldPos, tileSize);


        sf::FloatRect ghostRect(m_ghostPos, m_ghostSize);

        bool ok = true;

        // sprawdzamy kafelki pod budynkiem
        const int startX = static_cast<int>(std::floor(m_ghostPos.x / tileSize));
        const int startY = static_cast<int>(std::floor(m_ghostPos.y / tileSize));
        const int endX = static_cast<int>(std::floor((m_ghostPos.x + m_ghostSize.x - 1.f) / tileSize));
        const int endY = static_cast<int>(std::floor((m_ghostPos.y + m_ghostSize.y - 1.f) / tileSize));

        for (int y = startY; y <= endY && ok; ++y) {
            for (int x = startX; x <= endX && ok; ++x) {
                TileType tt = map.getTile(x, y);
                if (tt == TileType::Mountain || tt == TileType::Water) {
                    ok = false;
                }

                if (m_placeType == BuildingType::Quarry) {
                    bool nearMountain = false;

					const int sx = startX - 1;
                    const int sy = startY - 1;
                    const int ex = endX + 1;
                    const int ey = endY + 1;

                    for (int ny = sy; ny <= ey && !nearMountain; ++ny) {
                        for (int nx = sx; nx <= ex && !nearMountain; ++nx) {
                            if (nx < 0 || ny < 0) continue;
                            if (map.getTile(nx, ny) == TileType::Mountain) {
                                nearMountain = true;
                            }
                        }
					}
                    if (!nearMountain) ok = false;
                }

                if (m_placeType == BuildingType::GoldMine) {
                    bool onSand = false;

                    if (map.getTile(x, y) == TileType::Sand) {
						onSand = true;
                    }

                    if (!onSand) ok = false;
                }
            }
        }
        if (ok) {
            for (const auto& b : m_buildings) {
                // musimy mieć bounds budynku -> dodaj getBounds() w Building (sekcja 6)
                if (ghostRect.findIntersection(b->getBounds())) {
                    ok = false;
                    break;
                }
            }
        }

        // (opcjonalnie) kolizja z jednostkami (żeby nie stawiać na nich)
        if (ok) {
            for (const auto& u : m_units) {
                if (ghostRect.findIntersection(u.getBounds())) {
                    ok = false;
                    break;
                }
            }
        }

        m_ghostValid = ok;

        m_buildGhost.setPosition(m_ghostPos);
        m_buildGhost.setSize(m_ghostSize);

        if (m_ghostValid) {
            m_buildGhost.setFillColor(sf::Color(0, 255, 0, 80));
            m_buildGhost.setOutlineColor(sf::Color(0, 255, 0, 180));
        }
        else {
            m_buildGhost.setFillColor(sf::Color(255, 0, 0, 80));
            m_buildGhost.setOutlineColor(sf::Color(255, 0, 0, 180));
        }
    }
}

void Player::draw(sf::RenderWindow& window) {
    for (auto& unit : m_units) {
        unit.draw(window);
        if ( unit.getType() == UnitType::Worker && unit.isBuildingNow()) {
            float p = unit.getBuildProgress01();

            sf::Vector2f pos = unit.getBuildSitePos();
            sf::Vector2f size = { 80.f, 10.f }; // możesz dopasować do Building::defaultSize

            sf::RectangleShape back(size);
            back.setPosition({ pos.x, pos.y - 14.f });
            back.setFillColor(sf::Color(20, 20, 20, 220));
            back.setOutlineThickness(1.f);
            back.setOutlineColor(sf::Color(70, 70, 70, 255));

            sf::RectangleShape fill({ size.x * p, size.y });
            fill.setPosition(back.getPosition());
            fill.setFillColor(sf::Color(0, 200, 0, 230));

            window.draw(back);
            window.draw(fill);
        }
    }
    for (auto& building : m_buildings) {
        building->draw(window);
    }

    if (m_isDragging) {
        window.draw(m_selectionBox);
    }

    if (m_placingBuilding) {
        window.draw(m_buildGhost);
    }
}

void Player::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
	
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {

            if (m_placingBuilding) {
                if (!m_ghostValid) {
                    return;
                }

                sf::Vector2f buildPos = {m_ghostPos.x + 10.f, m_ghostPos.y + 10.f};

                for (auto& unit : m_units) {
                    if (unit.isSelected() && unit.getType() == UnitType::Worker) {
                        unit.startBuildJob(m_placeType, buildPos );
                        break;
                    }
				}
				m_placingBuilding = false;
                return;
            }

            m_isDragging = true;
            m_startClick = window.mapPixelToCoords(mousePressed->position);
            m_selectionBox.setPosition(m_startClick);
            m_selectionBox.setSize({ 0.f, 0.f });

            for (auto& unit : m_units) {
                unit.setSelected(false);
            }
            for (auto& building : m_buildings) {
                building->setSelected(false);
			}
        }
        else if (mousePressed->button == sf::Mouse::Button::Right) {
            sf::Vector2f targetPos = window.mapPixelToCoords(mousePressed->position);

            for (auto& unit : m_units) {
                if (unit.isSelected()) {
                    unit.moveTo(targetPos);
                }
            }
        }
    }
    else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left) {
            m_isDragging = false;
			bool unitSelected = false;

            sf::FloatRect selectionBounds = m_selectionBox.getGlobalBounds();

            if (selectionBounds.size.x < 2.f && selectionBounds.size.y < 2.f) {
                selectionBounds = sf::FloatRect({ m_startClick.x - 1.f, m_startClick.y - 1.f }, { 2.f, 2.f });
            }

            for (auto& unit : m_units) {
                if (selectionBounds.findIntersection(unit.getBounds())) {
                    unit.setSelected(true);
					unitSelected = true;
                }
            }

            if (!unitSelected) {
                for (auto& building : m_buildings) {
                    if (selectionBounds.findIntersection(building->getBounds())) {
                        building->setSelected(true);
                        break;
                    }
				}
            }

            
        }
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
		m_lastMouseWorldPos = window.mapPixelToCoords(mouseMoved->position);
        if (m_isDragging) {
            sf::Vector2f currentMouse = window.mapPixelToCoords(mouseMoved->position);
            m_selectionBox.setSize(currentMouse - m_startClick);
        }
    }
}

void Player::beginPlaceBuilding(BuildingType type) {
    m_placingBuilding = true;
    m_placeType = type;
}

bool Player::isPlacingBuilding() const
{
    return m_placingBuilding;
}

void Player::setLastMouseWorldPos(sf::Vector2f pos) {
    m_lastMouseWorldPos = pos;
}
