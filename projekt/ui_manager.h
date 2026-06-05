#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <vector>
#include <string>
#include <cstddef>
#include <utility>

class Player;
class Map;

struct UIButton {
    sf::RectangleShape shape;
    sf::Text label;
    std::function<void()> onClick;

    float flashTimeLeft = 0.f;
    float flashDuration = 0.12f;

	sf::Color baseColor = sf::Color(30, 30, 30, 230);
	sf::Color flashColor = sf::Color(80, 80, 80, 255);

    UIButton(const sf::Font& font) : label(font) {}

    void setText(const std::string& text, unsigned int charSize = 18) {
        label.setString(text);
        label.setCharacterSize(charSize);
    }

    void triggerFlash() {
        flashTimeLeft = flashDuration;
		shape.setFillColor(flashColor);
    }

    void updtate(float dt){
        if (flashTimeLeft <= 0.f) return;
        flashTimeLeft -= dt;
        if (flashTimeLeft <= 0.f) {
			flashTimeLeft = 0.f;
			shape.setFillColor(baseColor);
        }

    }

    bool hitTest(sf::Vector2f mousePx) const {
        return shape.getGlobalBounds().contains(mousePx);
    }

    void draw(sf::RenderWindow& w) const {
        w.draw(shape);
        w.draw(label);
    }
};

struct UIPanel {
    sf::FloatRect bounds{ {0.f, 0.f}, {0.f, 0.f} };
    sf::Color bg = sf::Color(10, 10, 10, 210);
    sf::Color outline = sf::Color(80, 80, 80, 255);
    float outlineThickness = 1.f;

    std::vector<UIButton> buttons;

    void clear() { buttons.clear(); }

    void draw(sf::RenderWindow& window) const {
        sf::RectangleShape panel;
        panel.setPosition(bounds.position);
        panel.setSize(bounds.size);
        panel.setFillColor(bg);
        panel.setOutlineThickness(outlineThickness);
        panel.setOutlineColor(outline);
        window.draw(panel);

        for (const auto& b : buttons) b.draw(window);
    }

    bool handleClick(sf::Vector2f mousePx) {
        if (!bounds.contains(mousePx)) return false;

        for (auto& b : buttons) {
            if (b.hitTest(mousePx)) {
                b.triggerFlash();
                if (b.onClick) b.onClick();
                return true;
            }
        }
        return true; // klik w panel tła też “zjada”
    }
};

class UIManager {
public:
    using Action = std::pair<std::string, std::function<void()>>;

    bool init(const std::string& fontPath);
    void forceRebuild(const sf::RenderWindow& window, Player& player);

    void setMap(const Map* map);
    void draw(sf::RenderWindow& window, const Player& player, const sf::View& view);
	void update(float dt);

	// true = UI zjadło event (np. klik w panel/przycisk)
	bool handleEvent(const sf::Event& event, sf::RenderWindow& window, Player& player);

	void drawPausedOverlay(sf::RenderWindow& window);


    
private:
	void layoutPanels(const sf::RenderWindow& window);
	void rebuildAll(const sf::RenderWindow& window, Player& player);

	void rebuildInfoPanel(const sf::RenderWindow& window, Player& player);
    void rebuildActionsPanel(const sf::RenderWindow& window, Player& player);
    void rebuildMapPanel(const sf::RenderWindow& window, Player& player);
	void rebuildResourcesPanel(const sf::RenderWindow& window, Player& player);

    void drawInfoPanelOverlay(sf::RenderWindow& window, const Player& player);
    void drawHealthBarUI(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, float hp01);
	void drawRecourcesBarUI(sf::RenderWindow& window, const Player& player);
	void drawBuildTypesUI(sf::RenderWindow& window, Player& player);

    void drawMiniMap(sf::RenderWindow& window, const Player& player, const sf::View& view);

    std::size_t computeSelectionHash(const Player& player) const;

    // Jeden wspólny “layout engine” dla przycisków (super przy rozbudowie UI)
    void fillPanelGrid(
        UIPanel& panel,
        const std::vector<Action>& actions,
        int columns,
        float padding,
        float rowHeight
    );
     
private:
    sf::Font m_font;
    bool m_ready = false;

    float m_panelHeight = 170.f;

    UIPanel m_infoPanel;
	UIPanel m_resourcesPanel;
    UIPanel m_actionsPanel;
	UIPanel m_buildPanel;
    UIPanel m_mapPanel;

	bool m_showBuildPanel = false;

	const Map* m_mapRef = nullptr; // do minimapy

    std::size_t m_lastSelectionHash = 0;

 

    
};