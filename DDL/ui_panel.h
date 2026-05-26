#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

struct UIButton {
    sf::RectangleShape shape;
    sf::Text label;
    std::function<void()> onClick;

    UIButton(const sf::Font& font) : label(font) {}

    void setText(const std::string& text, unsigned int charSize = 18) {
        label.setString(text);
        label.setCharacterSize(charSize);
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
    sf::FloatRect bounds{ 0.f, 0.f, 0.f, 0.f };

    sf::Color bg = sf::Color(10, 10, 10, 210);
    sf::Color outline = sf::Color(80, 80, 80, 255);
    float outlineThickness = 1.f;

    std::vector<UIButton> buttons;

    void clear() { buttons.clear(); }

    void draw(sf::RenderWindow& window) const {
        sf::RectangleShape panel;
        panel.setPosition({ bounds.left, bounds.top });
        panel.setSize({ bounds.width, bounds.height });
        panel.setFillColor(bg);
        panel.setOutlineThickness(outlineThickness);
        panel.setOutlineColor(outline);
        window.draw(panel);

        for (const auto& b : buttons) b.draw(window);
    }

    // true jeśli panel zjadł klik (klik w panel lub w przycisk)
    bool handleClick(sf::Vector2f mousePx) {
        if (!bounds.contains(mousePx)) return false;

        for (auto& b : buttons) {
            if (b.hitTest(mousePx)) {
                if (b.onClick) b.onClick();
                return true;
            }
        }
        return true; // klik w panel, ale nie w przycisk -> też zjedz, żeby nie klikało świata
    }
};