#pragma once

#include <SFML/Graphics.hpp>
#include "player.h"
#include "API.h"
#include "menu.h"
#include "map.h"

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
};
