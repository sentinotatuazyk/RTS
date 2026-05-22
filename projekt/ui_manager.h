#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>
#include "API.h"
class Player;

struct UIButton {
	sf::RectangleShape shape;
	sf::Text label;
	std::function<void()> onClick;

	UIButton(const sf::Font& font) : label(font) {}

	bool hitTest(sf::Vector2f mousePx) const {
		return shape.getGlobalBounds().contains(mousePx);
	}

	void draw(sf::RenderWindow& w) const {
		w.draw(shape);
		w.draw(label);
	}
};

class UIManager {
public:
	bool init(const std::string& fontPath);
	void update( sf::RenderWindow& window, Player& player);
	void draw(sf::RenderWindow& window, const Player& player);
	void forceRebuild(const sf::RenderWindow& window, Player& player);

	bool handleEvent(const sf::Event& event, sf::RenderWindow& window, Player& player);

private:
	void rebuildButtons(const sf::RenderWindow& window, Player& player);
	void drawBottomPanel(sf::RenderWindow& window, const Player& player);
	void drawSelectedUnitInfo(sf::RenderWindow& window, const Player& player, float panelTop);
	void drawHealthBarUI(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, float hp01);

	

	std::size_t computeSelectionHash(const Player& player) const;
	
	sf::Font m_font;

	bool m_ready = false;

	float m_panelHeight = 170.f;

	mutable std::size_t m_lastSelectionHash = 0;

	std::vector<UIButton> m_buttons;
};

