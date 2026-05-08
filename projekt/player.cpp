#include "player.h"
#include <cmath>
#include <algorithm>

// W konstruktorze ustawiamy już tylko złoto na 0 (lub inną wartość startową)
Player::Player() : m_gold(0) {
    m_selectionBox.setFillColor(sf::Color(0, 255, 0, 50));
    m_selectionBox.setOutlineColor(sf::Color::Green);
    m_selectionBox.setOutlineThickness(1.f);
}

Player::~Player() {
}

void Player::addGold(int amount) {
    m_gold += amount;
}

int Player::getGold() const {
    return m_gold;
}

void Player::addUnit(sf::Vector2f position,UnitType type) {
    Unit newUnit;
    newUnit.spawn(position, type);
    m_units.push_back(newUnit);
}

std::vector<Unit>& Player::getUnits()
{
    return m_units;
}

void Player::addBuilding(sf::Vector2f position, BuildingType type)
{
    Building newBuilding(position, type);
    m_buildings.push_back(newBuilding);
}

void Player::update(float deltaTime, const Map& map) {
    float tileSize = map.getTileSize();
    float unitRadius = 15.f;

    for (auto& building : m_buildings) {
        building.update(deltaTime);
    }

    for (auto& unit : m_units) {
        unit.update(deltaTime);

        sf::Vector2f pos = unit.getPosition();

        int gridX = static_cast<int>(pos.x / tileSize);
        int gridY = static_cast<int>(pos.y / tileSize);

        for (int y = gridY - 1; y <= gridY + 1; ++y) {
            for (int x = gridX - 1; x <= gridX + 1; ++x) {
                if (x < 0 || y < 0) continue;
                TileType type = map.getTile(x, y);

                if (type == TileType::Wall || type == TileType::Water) {
                    float tileLeft = x * tileSize;
                    float tileTop = y * tileSize;
                    float tileRight = tileLeft + tileSize;
                    float tileBottom = tileTop + tileSize;

                    float closestX = std::max(tileLeft, std::min(pos.x, tileRight));
                    float closestY = std::max(tileTop, std::min(pos.y, tileBottom));

                    float distX = pos.x - closestX;
                    float distY = pos.y - closestY;

                    float distanceSquared = distX * distX + distY * distY;

                    if (distanceSquared < unitRadius * unitRadius) {
                        float distance = std::sqrt(distanceSquared);

                        if (distance > 0.0001f) {
                            float overlap = unitRadius - distance;
                            pos.x += (distX / distance) * overlap;
                            pos.y += (distY / distance) * overlap;
                        }
                        unit.setPosition(pos);
                        //unit.stop();
                    }
                }
            }
        }
        unit.setPosition(pos);
    }


    for (size_t i = 0; i < m_units.size(); ++i) {
        for (size_t j = i + 1; j < m_units.size(); ++j ) {

            sf::Vector2f pos1 = m_units[i].getPosition();
            sf::Vector2f pos2 = m_units[j].getPosition();

            sf::Vector2f direction = pos1 - pos2;

            float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            float minDist = 31.f;

            if (distance < minDist && distance > 0.0001f) {
                float overlap = minDist - distance;

                sf::Vector2f pushDir = direction / distance;
                m_units[i].setPosition(pos1 + pushDir * (overlap / 2.f));
                m_units[j].setPosition(pos2 - pushDir * (overlap / 2.f));
            }
        }
    }
}

void Player::draw(sf::RenderWindow& window) {
    for (auto& unit : m_units) {
        unit.draw(window);
    }
    for (auto& building : m_buildings) {
        building.draw(window);
    }

    if (m_isDragging) {
        window.draw(m_selectionBox);
    }
}

void Player::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    // 1. Wciśnięcie lewego przycisku (Start zaznaczania)
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            m_isDragging = true;
            // Pobieramy pozycję myszki i zamieniamy na współrzędne świata
            m_startClick = window.mapPixelToCoords(mousePressed->position);
            m_selectionBox.setPosition(m_startClick);
            m_selectionBox.setSize({ 0.f, 0.f });

            // Odznaczamy wszystkie jednostki na początku nowego kliknięcia
            for (auto& unit : m_units) {
                unit.setSelected(false);
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
    // 2. Puszczenie lewego przycisku (Koniec zaznaczania)
    else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left) {
            m_isDragging = false;

            // Prostokąt zaznaczenia
            sf::FloatRect selectionBounds = m_selectionBox.getGlobalBounds();

            // Jeśli kliknięto w miejscu (zaznaczenie punktowe)
            if (selectionBounds.size.x < 2.f && selectionBounds.size.y < 2.f) {
                // Rozszerzamy punkt do małego kwadracika, by łatwiej było trafić myszką
                selectionBounds = sf::FloatRect({ m_startClick.x - 1.f, m_startClick.y - 1.f }, { 2.f, 2.f });
            }

            // Sprawdzamy, które jednostki łapią się w prostokąt
            for (auto& unit : m_units) {
                if (selectionBounds.findIntersection(unit.getBounds())) {
                    unit.setSelected(true);
                }
            }
        }
    }
    // 3. Ruch myszką (Aktualizacja wielkości prostokąta)
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        if (m_isDragging) {
            sf::Vector2f currentMouse = window.mapPixelToCoords(mouseMoved->position);
            // Wielkość to po prostu różnica między obecną pozycją a startową
            m_selectionBox.setSize(currentMouse - m_startClick);
        }
    }
}
