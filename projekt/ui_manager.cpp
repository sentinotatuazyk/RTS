#include "ui_manager.h"
#include "player.h"
#include "unit.h"
#include "utils.hpp"
#include <sstream>
#include <algorithm>


static std::size_t hashCombine(std::size_t h, std::size_t v) {
	return h ^ (v * 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2));
}


bool UIManager::init(const std::string& fontPath) {
	if (!m_font.openFromFile(fontPath)) {
		return false;
	}
	m_ready = true;
	return true;
}

std::size_t UIManager::computeSelectionHash(const Player& player) const {
	std::size_t h = 0;
	const auto& units = player.getUnits();

	for (std::size_t i = 0; i < units.size(); ++i) {
		if (units[i].isSelected())
			h = hashCombine(h, i + 1);
	}
	return h;
}

void UIManager::rebuildButtons(const sf::RenderWindow& window, Player& player) {
    m_buttons.clear();

    const sf::Vector2u win = window.getSize();
    const float panelTop = static_cast<float>(win.y) - m_panelHeight;

    sf::Vector2f start(20.f, panelTop + 20.f);
    sf::Vector2f btnSize(150.f, 42.f);
    float gap = 12.f;

    auto makeButton = [&](const std::string& text, sf::Vector2f pos, std::function<void()> cb) {
        UIButton b(m_font);
        b.shape.setPosition(pos);
        b.shape.setSize(btnSize);
        b.shape.setFillColor(sf::Color(30, 30, 30, 230));
        b.shape.setOutlineThickness(1.f);
        b.shape.setOutlineColor(sf::Color(90, 90, 90));

        b.label = sf::Text(m_font, text, 18);
        b.label.setFillColor(sf::Color::White);
        b.label.setPosition({ pos.x + 10.f, pos.y + 8.f });

        b.onClick = std::move(cb);
        m_buttons.push_back(std::move(b));
        };

    bool anySelected = false;
    for (const auto& u : player.getUnits()) {
        if (u.isSelected()) { anySelected = true; break; }
    }

    if (!anySelected) return;

    // 1) Stop
    makeButton("Stop", start, [&player]() {
        for (auto& u : player.getUnits()) {
            if (u.isSelected()) u.stop();
        }
        });
}

void UIManager::update(sf::RenderWindow& window,Player& player) {
    if (!m_ready) return;

    std::size_t h = computeSelectionHash(player);
    if (h != m_lastSelectionHash) {
    }

    (void)window;
}

void UIManager::drawHealthBarUI(sf::RenderWindow& window, sf::Vector2f pos, sf::Vector2f size, float hp01) {
    hp01 = std::clamp(hp01, 0.f, 1.f);

    sf::RectangleShape back(size);
    back.setPosition(pos);
    back.setFillColor(sf::Color(20, 20, 20, 255));
    back.setOutlineThickness(1.f);
    back.setOutlineColor(sf::Color(70, 70, 70, 255));

    sf::RectangleShape fill({ size.x * hp01, size.y });
    fill.setPosition(pos);

    if (hp01 > 0.6f) fill.setFillColor(sf::Color(0, 200, 0, 255));
    else if (hp01 > 0.3f) fill.setFillColor(sf::Color(220, 160, 0, 255));
    else fill.setFillColor(sf::Color(220, 40, 40, 255));

    window.draw(back);
    window.draw(fill);
}

void UIManager::drawSelectedUnitInfo(sf::RenderWindow& window, const Player& player, float panelTop) {
    const Unit* selected = nullptr;
    int selectedCount = 0;

    for (const auto& u : player.getUnits()) {
        if (u.isSelected()) {
            selected = &u;
            ++selectedCount;
        }
    }

    {
        std::ostringstream ss;
        ss << "Selected units: " << selectedCount;

        sf::Text t(m_font, ss.str(), 18);
        t.setFillColor(sf::Color::White);
        t.setPosition({ 20.f, panelTop + 78.f });
        window.draw(t);
    }

    if (selectedCount == 1 && selected) {
        auto [hp, hpMax] = selected->getHealth();
        std::string type = toString(selected->getType());

        float hp01 = 0.f;
        if (hpMax > 0) hp01 = static_cast<float>(hp) / static_cast<float>(hpMax);

        std::ostringstream ss;
        ss << type <<" - HP: " << hp << " / " << hpMax;

        sf::Text t(m_font, ss.str(), 18);
        t.setFillColor(sf::Color::White);
        t.setPosition({ 20.f, panelTop + 105.f });
        window.draw(t);

        drawHealthBarUI(window, { 20.f, panelTop + 130.f }, { 240.f, 16.f }, hp01);
    }
}

void UIManager::drawBottomPanel(sf::RenderWindow& window, const Player& player) {
    const sf::Vector2u win = window.getSize();
    const float panelTop = static_cast<float>(win.y) - m_panelHeight;

    sf::RectangleShape panel;
    panel.setPosition({ 0.f, panelTop });
    panel.setSize({ static_cast<float>(win.x), m_panelHeight });
    panel.setFillColor(sf::Color(10, 10, 10, 210));
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(80, 80, 80, 255));
    window.draw(panel);

    drawSelectedUnitInfo(window, player, panelTop);

    for (const auto& b : m_buttons)
        b.draw(window);
}

void UIManager::draw(sf::RenderWindow& window, const Player& player) {
    if (!m_ready) return;
    drawBottomPanel(window, player);
}
void UIManager::forceRebuild(const sf::RenderWindow& window, Player& player) {
    m_lastSelectionHash = computeSelectionHash(player);
    rebuildButtons(window, player);
}


bool UIManager::handleEvent(const sf::Event& event, sf::RenderWindow& window, Player& player) {
    if (!m_ready) return false;

    std::size_t h = computeSelectionHash(player);
    if (h != m_lastSelectionHash) {
        m_lastSelectionHash = h;
        rebuildButtons(window, player);
    }

    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePx(
                static_cast<float>(mb->position.x),
                static_cast<float>(mb->position.y)
            );

            for (auto& b : m_buttons) {
                if (b.hitTest(mousePx)) {
                    if (b.onClick) b.onClick();
                    return true; 
                }
            }
        }
    }

    return false;
}
