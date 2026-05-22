#include "game.h"
#include "utils.hpp"
#include <vector>
#include <algorithm>

constexpr unsigned int mX = 200; //  sf::VideoMode::getDesktopMode().size.x / 40 + 1
constexpr unsigned int mY = 200; //  sf::VideoMode::getDesktopMode().size.y / 40 + 1

Game::Game() :
	m_window(sf::VideoMode::getDesktopMode(), "RTS Game"),
	m_menu(m_window),
	m_map(mX, mY, 40.f) {
	m_window.setFramerateLimit(60);
	float centerX = m_window.getSize().x / 2.f;
	float centerY = m_window.getSize().y / 2.f;

	m_view = m_window.getDefaultView();

	// Dodajemy główny Ratusz na środku:
	m_player.addBuilding({ centerX, centerY }, BuildingType::TownHall);

	// Dodajemy różne jednostki obok ratusza:
	m_player.addUnit({ centerX + 80.f, centerY - 50.f }, UnitType::Worker);
	m_player.addUnit({ centerX + 80.f, centerY + 50.f }, UnitType::Worker);

	m_player.addUnit({ centerX + 150.f, centerY }, UnitType::Warrior);
	m_player.addUnit({ centerX + 200.f, centerY }, UnitType::Archer);

	m_player.addUnit({ centerX + 250.f, centerY }, UnitType::Hero);

	addEnemy({ 1000.f, 200.f }, EnemyType::Goblin);
	addEnemy({ 1050.f, 200.f }, EnemyType::Orc);
	addEnemy({ 1000.f, 250.f }, EnemyType::Troll);
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

void Game::addEnemy(sf::Vector2f position, EnemyType type)
{
	Enemy newEnemy;
	newEnemy.spawn(position, type);
	m_enemies.push_back(newEnemy);
}

void Game::handleEnemyCollisions()
{
	// 1. Wróg vs Mapa
	for (auto& enemy : m_enemies) {
		sf::Vector2f pos = enemy.getPosition();
		applyMapCollision(pos, enemy.getRadius(), m_map);
		enemy.setPosition(pos);
	}

	// 2. Wróg vs Wróg
	for (size_t i = 0; i < m_enemies.size(); ++i) {
		for (size_t j = i + 1; j < m_enemies.size(); ++j) {
			sf::Vector2f pos1 = m_enemies[i].getPosition();
			sf::Vector2f pos2 = m_enemies[j].getPosition();

			applyCircleCollision(pos1, pos2, m_enemies[i].getRadius(), m_enemies[j].getRadius());

			m_enemies[i].setPosition(pos1);
			m_enemies[j].setPosition(pos2);
		}
	}

	// 3. Wróg vs Gracz
	for (auto& enemy : m_enemies) {
		for (auto& unit : m_player.getUnits()) {
			sf::Vector2f posE = enemy.getPosition();
			sf::Vector2f posU = unit.getPosition();

			applyCircleCollision(posE, posU, enemy.getRadius(), unit.getRadius());

			enemy.setPosition(posE);
			unit.setPosition(posU);
		}
	}
}

void Game::update(float deltaTime) {
	if (m_state == State::Menu) {
		m_menu.update();
	}
	else if (m_state == State::Playing) {
		handleCameraInput(deltaTime);
		//clampCameraToMap(m_map.getSize().first, m_map.getSize().second);
		m_map.update(deltaTime);
		m_player.update(deltaTime,m_map);
		for (auto& enemy : m_enemies) {
			enemy.update(deltaTime);
		}
		handleEnemyCollisions();
		handleAI();
		handleCombat(deltaTime);
		removeDeadEntities();
	}
	
}


void Game::render() {
	m_window.clear();
	if (m_state == State::Menu) {
		m_window.setView(m_window.getDefaultView());
		m_menu.draw();
	}
	else if (m_state == State::Playing) {
		m_window.setView(m_view);
		m_map.drawVisible(m_window,m_view);  
		m_player.draw(m_window); 
		for (auto& enemy : m_enemies) {
			enemy.draw(m_window);
		}
		m_window.setView(m_window.getDefaultView());
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
			m_window.setView(m_view);
			m_player.handleEvent(*event, m_window);
			m_window.setView(m_window.getDefaultView());
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
			if (const auto* mouseInteracted = event->getIf<sf::Event::MouseWheelScrolled>()) {
				float factor = (mouseInteracted->delta > 0) ? 0.9f : 1.1f;
				m_view.zoom(factor);
			}

			if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mb->button == sf::Mouse::Button::Middle) {
					m_draggingCamera = true;
					m_lastMousePx = { mb->position.x, mb->position.y };
				}
			}

			if (const auto* mb = event->getIf<sf::Event::MouseButtonReleased>()) {
				if (mb->button == sf::Mouse::Button::Middle) {
					m_draggingCamera = false;
				}
			}


		}
	}
}

void Game::handleCombat(float dt) {
	// 1. Wrogowie atakują gracza
	for (auto& enemy : m_enemies) {
		if (enemy.m_attackTimer > 0.f) continue; // Wróg musi odpocząć

		for (auto& unit : m_player.getUnits()) {
			sf::Vector2f dir = enemy.getPosition() - unit.getPosition();
			float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

			// Jeśli jednostka jest w zasięgu wroga
			if (dist <= enemy.m_attackRange) {
				unit.takeDamage(enemy.m_damage); // Zadajemy cios
				enemy.m_attackTimer = enemy.m_attackCooldown; // Resetujemy timer wroga
				break; // Uderzył jednego, nie sprawdza reszty w tej klatce
			}
		}
	}

	// 2. Gracz atakuje wrogów
	for (auto& unit : m_player.getUnits()) {
		if (unit.m_attackTimer > 0.f) continue; // Jednostka odpoczywa

		for (auto& enemy : m_enemies) {
			sf::Vector2f dir = unit.getPosition() - enemy.getPosition();
			float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

			if (dist <= unit.m_attackRange) {
				enemy.takeDamage(unit.m_damage);
				unit.m_attackTimer = unit.m_attackCooldown;

				// Super bajer: jeśli jednostka atakuje, niech przestanie iść
				unit.stop();
				break;
			}
		}
	}
}

// Czyszczenie mapy z trupów
void Game::removeDeadEntities() {
	// Usuwamy martwych wrogów
	m_enemies.erase(
		std::remove_if(m_enemies.begin(), m_enemies.end(), [](const Enemy& e) { return e.isDead(); }),
		m_enemies.end()
	);

	// Usuwamy martwe jednostki gracza
	auto& units = m_player.getUnits();
	units.erase(
		std::remove_if(units.begin(), units.end(), [](const Unit& u) { return u.isDead(); }),
		units.end()
	);
}

void Game::handleAI() {
	basicAI(m_enemies, m_player.getUnits(), 300.f, false);

	basicAI(m_player.getUnits(), m_enemies, 300.f, true);
}


void Game::handleCameraInput(float deltaTime) {
	sf::Vector2f movement(0.f, 0.f);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    movement.y -= m_viewSpeed * deltaTime;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  movement.y += m_viewSpeed * deltaTime;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  movement.x -= m_viewSpeed * deltaTime;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) movement.x += m_viewSpeed * deltaTime;

	m_view.move(movement);

	if (m_draggingCamera) {
		sf::Vector2i now = sf::Mouse::getPosition(m_window);
		sf::Vector2i deltaPx = now - m_lastMousePx;
		m_lastMousePx = now;

		sf::Vector2f size = m_view.getSize();
		sf::Vector2u win = m_window.getSize();

		float scaleX = size.x / static_cast<float>(win.x);
		float scaleY = size.y / static_cast<float>(win.y);

		m_view.move({-deltaPx.x * scaleX, -deltaPx.y * scaleY});
	}
}

void Game::clampCameraToMap(float mapW, float mapH) {
	sf::Vector2f center = m_view.getCenter();
	sf::Vector2f half = m_view.getSize() * 0.5f;

	float minX = half.x;
	float minY = half.y;
	float maxX = mapW - half.x;
	float maxY = mapH - half.y;

	center.x = std::max(minX, std::min(center.x, maxX));
	center.y = std::max(minY, std::min(center.y, maxY));

	m_view.setCenter(center);
}

void Game::drawUI() {
	// Na razie pusto, ale tu będzie rysowane złoto, minimapa, itp.
}