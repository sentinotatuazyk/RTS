#include "game.h"

Game::Game() :
	m_window(sf::VideoMode::getDesktopMode(), "RTS Game"),
	m_menu(m_window),
	m_map(sf::VideoMode::getDesktopMode().size.x / 40 + 1, sf::VideoMode::getDesktopMode().size.y / 40 + 1, 40.f ) {
	m_window.setFramerateLimit(60);
	float centerX = m_window.getSize().x / 2.f;
	float centerY = m_window.getSize().y / 2.f;

	// Dodajemy główny Ratusz na środku:
	m_player.addBuilding({ centerX, centerY }, BuildingType::TownHall);

	// Dodajemy różne jednostki obok ratusza:
	m_player.addUnit({ centerX + 80.f, centerY - 50.f }, UnitType::Worker);
	m_player.addUnit({ centerX + 80.f, centerY + 50.f }, UnitType::Worker);

	m_player.addUnit({ centerX + 150.f, centerY }, UnitType::Warrior);
	m_player.addUnit({ centerX + 200.f, centerY }, UnitType::Archer);

	m_player.addUnit({ centerX + 250.f, centerY }, UnitType::Hero);;
}

Game::~Game()
{
}

void Game::run() {
	while (m_window.isOpen()) {
		float deltaTime = m_clock.restart().asSeconds();

		procesEvents();
		update(deltaTime);
		render();
	}
}

void Game::update(float deltaTime) {
	if (m_state == State::Menu) {
		m_menu.update();
	}
	else if (m_state == State::Playing) {
		m_map.update(deltaTime);
		m_player.update(deltaTime,m_map);
	}
}


void Game::render() {
	m_window.clear();
	if (m_state == State::Menu) {
		m_menu.draw();
	}
	else if (m_state == State::Playing) {
		m_map.draw(m_window);  
		m_player.draw(m_window); 
	}

	m_window.display();
}

void Game::procesEvents() {
	while (std::optional<sf::Event> event = m_window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			m_window.close();
			return;
		}
		if (m_state == State::Menu) {
			MenuAction action = m_menu.handleEvent(*event);
			if (action == MenuAction::ExitGame) m_window.close();
			else if (action == MenuAction::StartGame) m_state = State::Playing;
		}
		else if (m_state == State::Playing) {
			m_player.handleEvent(*event, m_window);
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
					m_state = State::Menu;
				}
				else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
					for (auto& unit : m_player.getUnits()) {
						if (unit.isSelected()) {
							unit.stop();
						}
					}
				}
			}
		}
	}
}