#pragma once
#include <SFML/Graphics.hpp>
#include "API.h"

enum class MenuAction { None, StartGame, Settings, ExitGame };

class EXPORT_API Menu {
public:
	explicit Menu(sf::RenderWindow& window);
	~Menu();

	MenuAction handleEvent(const sf::Event& event);
	void update();
	void draw();
	void rebuildLayout();

private:
	void updateSelection();

	sf::RenderWindow& m_window;
	sf::Font m_font;
	sf::Text m_title;
	sf::Text m_start;
	sf::Text m_settings;
	sf::Text m_exit;

	int m_selected = 0;

};