#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "player.h"
#include "API.h"
#include "menu.h"
#include "map.h"
#include "enemy.h"
#include "ui_manager.h"


class EXPORT_API Game {
public:
	Game();
	~Game();
	void run();
private:
	enum class State { Menu, Playing, Paused, Exiting };

	void procesEvents();
	void update(float deltaTime);
	void render();

	sf::RenderWindow m_window;
	sf::Clock m_clock;
	State m_state = State::Menu;
	Menu m_menu;

	Player m_player;
	Map m_map;
	std::vector<Enemy> m_enemies;
	void addEnemy(sf::Vector2f position, EnemyType type);
	void handleEnemyCollisions();
	void handleCombat(float deltaTime);
	void removeDeadEntities();
	void handleAI();

	sf::View m_view;
	float m_viewSpeed = 600.f;
	float m_zoomLevel = 1.f;
	bool m_draggingCamera = false;
	sf::Vector2i m_lastMousePx{};

	void handleCameraInput(float deltaTime);
	void clampCameraToMap(float mapW, float mapH);
	
	UIManager m_ui;
	void drawUI();





	template <typename T1, typename T2>
	void basicAI(std::vector<T1>& attackers, std::vector<T2>& targets, float aggroRange, bool skipIfMoving) {
		for (auto& attacker : attackers) {

			if (skipIfMoving && attacker.m_isMoving) continue;

			float closestDist = aggroRange;
			T2* targetEntity = nullptr;

			for (auto& target : targets) {
				sf::Vector2f dir = attacker.getPosition() - target.getPosition();
				float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);

				if (dist < closestDist) {
					closestDist = dist;
					targetEntity = &target;
				}
			}

			if (targetEntity != nullptr) {
				if (closestDist > attacker.m_attackRange) {
					attacker.moveTo(targetEntity->getPosition());
				}
				else {
					attacker.stop();
				}
			}
		}
	}
};
