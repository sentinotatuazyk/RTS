#include "game.h"
#include "game.h"
#include "utils.hpp"
#include <vector>
#include <algorithm>
#include <random>

constexpr unsigned int mX = 150; //  sf::VideoMode::getDesktopMode().size.x / 40 + 1
constexpr unsigned int mY = 100; //  sf::VideoMode::getDesktopMode().size.y / 40 + 1
constexpr float tS = 32.f;

Game::Game() :
	m_window(sf::VideoMode({ 1280, 720 }), "RTS Game", sf::Style::Titlebar | sf::Style::Close),
	m_menu(m_window),
	m_map(mX, mY, tS, std::random_device{}()), 
	m_fpsText(m_fpsFont){
	m_window.setFramerateLimit(144);
	if (!m_settings.loadFromFile("settings.bin")) {
		m_settings.fpsLimit = 144;
		m_settings.fullscreen = false;
		m_settings.showFps = false;
		m_settings.resolutionIndex = 0;
		m_settings.saveToFile("settings.bin");
	}

	if (m_fpsFont.openFromFile("geistmono_light.ttf")) {
		m_fpsText.setFont(m_fpsFont);
		m_fpsText.setCharacterSize(16);
		m_fpsText.setFillColor(sf::Color::Green);
		m_fpsText.setPosition({ 10.f, 10.f });  // lewy górny róg
	}

	applySettingsToWindows();
	
	m_settingsScreen.init("geistmono_light.ttf");
	float centerX = mX * tS / 2.f;
	float centerY = mX * tS / 2.f;

	m_view = m_window.getDefaultView();
	m_view.setCenter({centerX, centerY});

	m_fenceAtlas.loadFromFile("fence_atlas.png");

	Fence::setAtlas(&m_fenceAtlas);

	m_ui.init("geistmono_light.ttf");
	m_ui.setMap(&m_map);
	m_ui.forceRebuild(m_window, m_player);

	m_player.addBuilding(findValidSpawnPosition(centerX, centerY), BuildingType::TownHall);

	m_player.addUnit(findValidSpawnPosition(centerX + 80.f, centerY - 50.f), UnitType::Worker);
	m_player.addUnit(findValidSpawnPosition(centerX + 80.f, centerY + 50.f), UnitType::Worker);

	m_player.addUnit(findValidSpawnPosition(centerX + 150.f, centerY), UnitType::Warrior);
	m_player.addUnit(findValidSpawnPosition(centerX + 200.f, centerY), UnitType::Archer);
	m_player.addUnit(findValidSpawnPosition(centerX + 250.f, centerY), UnitType::Hero);

	// Wrogowie — daleko od gracza, na lądzie
	addEnemy(findValidSpawnPosition(1000.f, 200.f), EnemyType::Goblin);
	addEnemy(findValidSpawnPosition(1050.f, 200.f), EnemyType::Orc);
	addEnemy(findValidSpawnPosition(1000.f, 250.f), EnemyType::Troll);
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

sf::Vector2f Game::findValidSpawnPosition(float pX, float pY) {
	unsigned int tileX = static_cast<unsigned int>(pX / tS);
	unsigned int tileY = static_cast<unsigned int>(pY / tS);

	// Sprawdź czy startowa pozycja jest valid
	if (tileX < mX && tileY < mY) {
		TileType startTile = m_map.getTile(tileX, tileY);
		if (startTile != TileType::Water && startTile != TileType::Mountain) {
			// Zwróć środek tile'a (snap do siatki)
			return { tileX * tS + tS / 2.f, tileY * tS + tS / 2.f };
		}
	}

	// Szukaj w okolicy (spirala)
	for (int r = 1; r < 50; ++r) {
		for (int dy = -r; dy <= r; ++dy) {
			for (int dx = -r; dx <= r; ++dx) {
				// Tylko obramowanie kwadratu (optymalizacja)
				if (std::abs(dx) != r && std::abs(dy) != r) continue;

				int checkX = static_cast<int>(tileX) + dx;
				int checkY = static_cast<int>(tileY) + dy;

				// POPRAWKA: granice mapy
				if (checkX < 0 || checkX >= static_cast<int>(mX) ||
					checkY < 0 || checkY >= static_cast<int>(mY)) continue;

				// POPRAWKA: AND zamiast OR
				TileType checkTile = m_map.getTile(checkX, checkY);
				if (checkTile != TileType::Water && checkTile != TileType::Mountain) {
					return { checkX * tS + tS / 2.f, checkY * tS + tS / 2.f };
				}
			}
		}
	}

	// Fallback: środek mapy
	return { mX * tS / 2.f, mY * tS / 2.f };
}

void Game::addEnemy(sf::Vector2f position, EnemyType type)
{
	Enemy newEnemy;
	newEnemy.spawn(position, type);
	m_enemies.push_back(newEnemy);
}

void Game::handleEnemyCollisions()
{
	// enemy vs mapa
	for (auto& enemy : m_enemies) {
		sf::Vector2f pos = enemy.getPosition();
		applyMapCollision(pos, enemy.getRadius(), m_map);
		enemy.setPosition(pos);
	}

	//enemy vs budynki
	for (auto& enemy : m_enemies) {
		sf::Vector2f pos = enemy.getPosition();
		applyBuildingCollision(pos, enemy.getRadius(), m_player.getBuildings());
		enemy.setPosition(pos);
	}

	// enemy vs enemy
	for (size_t i = 0; i < m_enemies.size(); ++i) {
		for (size_t j = i + 1; j < m_enemies.size(); ++j) {
			sf::Vector2f pos1 = m_enemies[i].getPosition();
			sf::Vector2f pos2 = m_enemies[j].getPosition();

			applyCircleCollision(pos1, pos2, m_enemies[i].getRadius(), m_enemies[j].getRadius());

			m_enemies[i].setPosition(pos1);
			m_enemies[j].setPosition(pos2);
		}
	}

	// enemy vs unit
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
	m_fpsUpdateTimer += deltaTime;
	m_frameCount++;

	if (m_fpsUpdateTimer >= 0.25f) {  // aktualizuj co 0.25 sekundy
		m_currentFps = static_cast<float>(m_frameCount) / m_fpsUpdateTimer;
		m_frameCount = 0;
		m_fpsUpdateTimer = 0.f;

		// Aktualizuj tekst tylko jeśli showFps jest włączone
		if (m_settings.showFps) {
			m_fpsText.setString(std::to_string(static_cast<int>(m_currentFps)) + " FPS");
		}
	}

	if (m_state == State::Menu) {
		m_menu.update();
	}
	else if (m_state == State::Settings) {
		(void)deltaTime; // na razie nic tu nie ma, ale może być przydatne
	}
	else if (m_state == State::Paused) {
		(void)deltaTime;

	}
	else if (m_state == State::Playing) {
		handleCameraInput(deltaTime);

		// Ogranicz kamerę do granic mapy
		sf::FloatRect mapBounds = m_map.getMapBounds();
		clampCameraToMap(mapBounds.size.x, mapBounds.size.y);

		m_map.update(deltaTime);
		m_player.update(deltaTime,m_map);
		m_ui.update(deltaTime);
		for (auto& enemy : m_enemies) {
			enemy.update(deltaTime);
		}
		handleEnemyCollisions();
		handleAi();
		handleCombat(deltaTime);
		removeDeadEntities();
	}
	
}


void Game::render() {
	m_window.clear(sf::Color(20,20,40));
	if (m_state == State::Menu) {
		m_window.setView(m_window.getDefaultView());
		m_menu.draw();
	}
	else if (m_state == State::Settings) {
		m_window.setView(m_window.getDefaultView());
		m_settingsScreen.draw(m_window);
	}
	else if (m_state == State::Playing || m_state == State::Paused) {
		sf::FloatRect mapBounds = m_map.getMapBounds();
		clampCameraToMap(mapBounds.size.x, mapBounds.size.y);

		m_window.setView(m_view);
		m_map.drawVisible(m_window,m_view);  
		m_player.draw(m_window); 
		for (auto& enemy : m_enemies) {
			enemy.draw(m_window);
		}
		m_window.setView(m_window.getDefaultView());
		m_ui.draw(m_window,m_player, m_view);

		if (m_settings.showFps) {
			m_window.draw(m_fpsText);
		}
		if (m_state == State::Paused) {
			m_ui.drawPausedOverlay(m_window);
		}


	}
	m_window.display();
}

void Game::procesEvents() {
	while (std::optional<sf::Event> event = m_window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			m_window.close();
			return;
		}

		if (const auto* r = event->getIf<sf::Event::Resized>()) {
			sf::View dv;
			dv.setSize({ static_cast<float>(r->size.x), static_cast<float>(r->size.y) });
			dv.setCenter(dv.getSize() * 0.5f);
			m_window.setView(dv);

			m_ui.forceRebuild(m_window, m_player);
		}
		if (m_state == State::Menu) {
			MenuAction action = m_menu.handleEvent(*event);
			if (action == MenuAction::ExitGame) m_window.close();
			else if (action == MenuAction::Settings) { m_settingsScreen.open(m_settings); m_state = State::Settings; }
			else if (action == MenuAction::StartGame) m_state = State::Playing;
		}
		else if (m_state == State::Playing || m_state==State::Paused) {
			m_window.setView(m_window.getDefaultView());
			if (m_ui.handleEvent(*event, m_window, m_player)) {
				continue;
			}
			m_window.setView(m_view);
			m_player.handleEvent(*event, m_window);

			// KLAWIATURA
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
					m_state = (m_state == State::Playing) ? State::Paused : State::Playing;
					continue;
				}
				if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
					m_state = State::Menu;
					continue;
				}
				else if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
					for (auto& unit : m_player.getUnits()) {
						if (unit.isSelected()) {
							unit.stop();
						}
					}
				}
				
				else if (keyPressed->scancode == sf::Keyboard::Scancode::Num1) {
					for (auto& u : m_player.getUnits()) if (u.isSelected()) u.setState(UnitState::Aggressive);
				}
				else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2) {
					for (auto& u : m_player.getUnits()) if (u.isSelected()) u.setState(UnitState::Passive);
				}
				else if (keyPressed->scancode == sf::Keyboard::Scancode::Num3) {
					for (auto& u : m_player.getUnits()) if (u.isSelected()) u.setState(UnitState::Neutral);
				}			
			}

			//MYSZKA
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
		else if (m_state == State::Settings) {
			m_window.setView(m_window.getDefaultView());

			SettingsAction sa = m_settingsScreen.handleEvent(*event, m_window);

			if (sa == SettingsAction::Back) {
				m_state = State::Menu;

			}
			else if (sa == SettingsAction::Apply) {
				m_settings = m_settingsScreen.edited();
				m_settings.saveToFile("settings.bin");
				applySettingsToWindows();
				m_state = State::Menu;
			}
		}
	}
}

void Game::handleCombat(float dt) {
	for (auto& enemy : m_enemies) {
		if (enemy.m_attackTimer > 0.f) continue; 

		for (auto& unit : m_player.getUnits()) {
			sf::Vector2f dir = enemy.getPosition() - unit.getPosition();
			float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

			if (dist <= enemy.m_attackRange) {
				unit.takeDamage(enemy.m_damage); 
				enemy.m_attackTimer = enemy.m_attackCooldown; 
				break; 
			}
		}
	}


	for (auto& unit : m_player.getUnits()) {
		if (unit.m_attackTimer > 0.f) continue; 

		for (auto& enemy : m_enemies) {
			sf::Vector2f dir = unit.getPosition() - enemy.getPosition();
			float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

			if (dist <= unit.m_attackRange) {
				enemy.takeDamage(unit.m_damage);
				unit.m_attackTimer = unit.m_attackCooldown;

				unit.stop();
				break;
			}
		}
	}
}

void Game::removeDeadEntities() {
	for (size_t i = 0; i < m_enemies.size(); ) {
		if (m_enemies[i].isDead()) {
			m_enemies[i] = std::move(m_enemies.back());
			m_enemies.pop_back();
		}
		else {
			++i;
		}
	}

	auto& units = m_player.getUnits();
	for (size_t i = 0; i < units.size(); ) {
		if (units[i].isDead()) {
			m_player.changeUnitCount(units[i].getType(), -1);
			units[i] = std::move(units.back());
			units.pop_back();

		}
		else {
			++i;
		}
	}
}

void Game::handleAi() {
	std::vector<Unit*> uAgg, uPas, uNeu;
	for (auto& unit : m_player.getUnits()) {
		switch (unit.getState()) {
		case UnitState::Aggressive:
			uAgg.push_back(&unit);
			break;
		case UnitState::Passive:
			uPas.push_back(&unit);
			break;
		case UnitState::Neutral:
			uNeu.push_back(&unit);
			break;
		}
	}
	std::vector<Enemy*> eAgg, ePas, eNeu;
	for (auto& enemy : m_enemies) {
		switch (enemy.getState()) {
		case EnemyState::Aggressive:
			eAgg.push_back(&enemy);
			break;
		case EnemyState::Passive:
			ePas.push_back(&enemy);
			break;
		case EnemyState::Neutral:
			eNeu.push_back(&enemy);
			break;
		}
	}

	//AI enemy
	basicAI(eAgg, m_player.getUnits(), 300.f, false);
	basicAI(ePas, m_player.getUnits(), 200.f, false, true);

	//AI player
	basicAI(uAgg, m_enemies, 350.f, true);   // skipIfMoving=true np. żeby nie przerywały marszu
	basicAI(uPas, m_enemies, 250.f, true);   // Passive: mniejszy zasięg aggro albo wcale
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

	// Nie pozwól wyjechać poza mapę
	float minX = half.x;
	float minY = half.y;
	float maxX = mapW - half.x;
	float maxY = mapH - half.y;

	// Jeśli widok jest większy niż mapa, wyśrodkuj
	if (maxX < minX) {
		center.x = mapW / 2.0f;
	}
	else {
		center.x = std::max(minX, std::min(center.x, maxX));
	}

	if (maxY < minY) {
		center.y = mapH / 2.0f;
	}
	else {
		center.y = std::max(minY, std::min(center.y, maxY));
	}

	m_view.setCenter(center);
}

void Game::applySettingsToWindows()
{
	sf::VideoMode vm;

	sf::State state;
	unsigned int style = sf::Style::Titlebar | sf::Style::Close;

	if (m_settings.fullscreen) {
		vm = sf::VideoMode::getDesktopMode();
		state = sf::State::Fullscreen;

		style = sf::Style::None;
	}
	else {
		sf::Vector2u res = m_settings.chosenResolutionOrDesktop();
		vm = sf::VideoMode(res);
		state = sf::State::Windowed;
	}

	m_window.create(vm, "RTS Game", style, state);
	if (m_settings.fpsLimit > 0) {
		m_window.setFramerateLimit(m_settings.fpsLimit);
	}
	m_view = m_window.getDefaultView();
	m_ui.forceRebuild(m_window, m_player);
}
