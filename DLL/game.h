#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "player.h"
#include "API.h"
#include "menu.h"
#include "map.h"
#include "enemy.h"
#include "ui_manager.h"
#include "settings.h"

class Game {
public:
    GAME_API Game();
    GAME_API ~Game();
    GAME_API void run();
private:

    float m_prepTimer = 180.f;
	float m_waveTimer = 0.f;
	int m_currentWave = 0;
	bool m_isPrepPhase = true;

	void spawnWave(int waveNumber);

    bool m_gameOver = false;
    std::string m_gameOverReason;

    AppSettings m_settings;
    SettingsScreen m_settingsScreen;
    enum class State { Menu, Settings, Playing, Paused, Exiting };

    void procesEvents();
    void update(float deltaTime);
    void render();
	void checkGameOver();

	void checkFoodPenalty(float dt);
	float m_foodPenaltyTimer = 0.f;

    sf::RenderWindow m_window;
    sf::Clock m_clock;
    State m_state = State::Menu;
    Menu m_menu;

    Player m_player;
    Map m_map;
    sf::Vector2f findValidSpawnPosition(float pX, float pY);
    std::vector<Enemy> m_enemies;
    void addEnemy(sf::Vector2f position, EnemyType type);
    void handleEnemyCollisions();
    void handleCombat(float deltaTime);
    void removeDeadEntities();
    void handleAi();

    sf::View m_view;
    float m_viewSpeed = 600.f;
    float m_zoomLevel = 1.f;
    bool m_draggingCamera = false;
    sf::Vector2i m_lastMousePx{};

    void handleCameraInput(float deltaTime);
    void clampCameraToMap(float mapW, float mapH);

    UIManager m_ui;

    sf::Texture m_fenceAtlas;

	void applySettingsToWindows();

    void drawFpsCounter();           
    sf::Text m_fpsText;              
    sf::Font m_fpsFont;              
    float m_fpsUpdateTimer = 0.f;    
    int m_frameCount = 0;            
    float m_currentFps = 0.f;        








    template <typename Attacker, typename Target>
    void basicAI(std::vector<Attacker*>& attackers,
        std::vector<Target>& targets,
        float aggroRange,
        bool skipIfMoving,
        const Map& map,
        bool escape = false)
    {
        for (auto* attacker : attackers) {
            if (!attacker) continue;

            if (escape) {
                float closestDist = aggroRange;
                Target* closestTarget = nullptr;
                for (auto& target : targets) {
                    sf::Vector2f dir = attacker->getPosition() - target.getPosition();
                    float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestTarget = &target;
                    }
                }
                if (closestTarget) {
                    sf::Vector2f dir = attacker->getPosition() - closestTarget->getPosition();
                    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    if (length > 0.f) {
                        dir /= length;
                        dir *= aggroRange;
                        attacker->moveTo(attacker->getPosition() + dir, map);
                    }
                }
                continue;
            }

            if (skipIfMoving && attacker->m_isMoving) continue;

            float closestDist = aggroRange;
            Target* targetEntity = nullptr;

            for (auto& target : targets) {
                sf::Vector2f dir = attacker->getPosition() - target.getPosition();
                float dist = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (dist < closestDist) {
                    closestDist = dist;
                    targetEntity = &target;
                }
            }

            if (targetEntity) {
                if (closestDist > attacker->m_attackRange) attacker->moveTo(targetEntity->getPosition(), map);
                else attacker->stop();
            }
        }
    }
};
