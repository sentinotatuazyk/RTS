#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include "enums.h"

struct AppSettings {
    int resolutionIndex = 0;

    bool fullscreen = false;
	bool showFps = false;

    unsigned int fpsLimit = 60;

    // Lista presetów
    static const std::vector<sf::Vector2u>& presets();
    static std::string presetToString(sf::Vector2u r);
    sf::Vector2u chosenResolutionOrDesktop() const;

	bool saveToFile(const std::string& path) const;
	bool loadFromFile(const std::string& path);
};


class SettingsScreen {
public:
    SettingsScreen();
    bool init(const std::string& fontPath);
    void open(const AppSettings& current);        
    const AppSettings& edited() const;

    void draw(sf::RenderWindow& window);
    SettingsAction handleEvent(const sf::Event& event, sf::RenderWindow& window);

private:
    void rebuildLayout(const sf::RenderWindow& window);

    bool hit(const sf::RectangleShape& r, sf::Vector2f p) const;
    void centerTextInRect(sf::Text& t, const sf::RectangleShape& r);

private:
    bool m_ready = false;
    sf::Font m_font;

    AppSettings m_edit;

    sf::RectangleShape m_panel;

    // Labels
    sf::Text m_title;
    sf::Text m_resLabel;
    sf::Text m_fpsLabel;

    // Resolution selector
    sf::RectangleShape m_resBox;
    sf::Text m_resValue;
    sf::RectangleShape m_resPrev;
    sf::Text m_resPrevText;
    sf::RectangleShape m_resNext;
    sf::Text m_resNextText;

    // Fullscreen checkbox
    sf::RectangleShape m_fsBox;
    sf::RectangleShape m_fsCheck;
    sf::Text m_fsText;

    // FPS shower
    sf::RectangleShape m_fpsShowerBox;
	sf::RectangleShape m_fpsShowerCheck;
	sf::Text m_fpsShowerText;

    // FPS selector
    sf::RectangleShape m_fpsBox;
    sf::Text m_fpsValue;
    sf::RectangleShape m_fpsMinus;
    sf::Text m_fpsMinusText;
    sf::RectangleShape m_fpsPlus;
    sf::Text m_fpsPlusText;

    // Buttons
    sf::RectangleShape m_applyBtn;
    sf::Text m_applyText;

    sf::RectangleShape m_backBtn;
    sf::Text m_backText;
};