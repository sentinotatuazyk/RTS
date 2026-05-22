#include "player.h"
#include <cmath>
#include <algorithm>
#include "utils.hpp"

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
    switch (type) {
        case BuildingType::TownHall:
            m_buildings.push_back(std::make_unique<TownHall>(position));
            break;
        case BuildingType::Quarry:
            m_buildings.push_back(std::make_unique<Quarry>(position));
            break;
        case BuildingType::Barracks:
            m_buildings.push_back(std::make_unique<Barracks>(position));
			break;
    }
}

void Player::update(float deltaTime, const Map& map) {
    float tileSize = map.getTileSize();
    float unitRadius = 15.f;

    for (auto& building : m_buildings) {
        building->update(deltaTime);
    }

    for (auto& unit : m_units) {
        unit.update(deltaTime);

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
}

void Player::draw(sf::RenderWindow& window) {
    for (auto& unit : m_units) {
        unit.draw(window);
    }
    for (auto& building : m_buildings) {
        building->draw(window);
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
