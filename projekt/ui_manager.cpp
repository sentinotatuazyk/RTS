#include "ui_manager.h"

#include "player.h"
#include "unit.h"
#include "utils.hpp"  // toString(UnitType)

#include <sstream>
#include <algorithm>
#include <cmath>

static std::size_t hashCombine(std::size_t h, std::size_t v) {
    return h ^ (v * 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2));
}

static bool anySelected(const Player& player) {
    for (const auto& u : player.getUnits())
        if (u.isSelected()) return true;
    return false;
}

bool UIManager::init(const std::string& fontPath) {
    if (!m_font.openFromFile(fontPath)) return false;
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

void UIManager::layoutPanels(const sf::RenderWindow& window) {
    const auto win = window.getSize();
    const float W = static_cast<float>(win.x);
    const float H = static_cast<float>(win.y);

    const float top = H - m_panelHeight;
    const float colW = W / 5.f;

    m_infoPanel.bounds = sf::FloatRect({ 0.f, top }, { colW, m_panelHeight });
    m_actionsPanel.bounds = sf::FloatRect({ colW * 2.f + 1.f, top }, { colW, m_panelHeight });
    m_mapPanel.bounds = sf::FloatRect({ W - colW, top }, { colW, m_panelHeight });
}

void UIManager::rebuildInfoPanel(const sf::RenderWindow&, Player&) {
    m_infoPanel.clear();
    // przyciski info nie są potrzebne teraz (overlay robimy w drawInfoPanelOverlay)
}

void UIManager::rebuildActionsPanel(const sf::RenderWindow&, Player& player) {
    m_actionsPanel.clear();
    if (!anySelected(player)) return;

    std::vector<Action> actions;

    actions.push_back({ "Stop", [&player]() {
        for (auto& u : player.getUnits()) if (u.isSelected()) u.stop();
    } });

    actions.push_back({ "Deselect", [&player]() {
        for (auto& u : player.getUnits()) if (u.isSelected()) u.setSelected(false);
    } });

    actions.push_back({ "Aggressive", [&player]() {
        for (auto& u : player.getUnits()) if (u.isSelected()) u.setState(UnitState::Aggressive);
    } });

    actions.push_back({ "Passive", [&player]() {
        for (auto& u : player.getUnits()) if (u.isSelected()) u.setState(UnitState::Passive);
    } });

    actions.push_back({ "Neutral", [&player]() {
        for (auto& u : player.getUnits()) if (u.isSelected()) u.setState(UnitState::Neutral);
    } });

    // W przyszłości tylko dopisujesz:
    // actions.push_back({"Attack-move", ...});
    // actions.push_back({"Build", ...});

    fillPanelGrid(m_actionsPanel, actions, /*columns=*/2, /*padding=*/12.f, /*rowHeight=*/42.f);
}

void UIManager::rebuildMapPanel(const sf::RenderWindow&, Player&) {
    m_mapPanel.clear();
    // placeholder; minimapa później
}

void UIManager::rebuildAll(const sf::RenderWindow& window, Player& player) {
    layoutPanels(window);
    rebuildInfoPanel(window, player);
    rebuildActionsPanel(window, player);
    rebuildMapPanel(window, player);
}

void UIManager::forceRebuild(const sf::RenderWindow& window, Player& player) {
    if (!m_ready) return;
    m_lastSelectionHash = computeSelectionHash(player);
    rebuildAll(window, player);
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

void UIManager::drawInfoPanelOverlay(sf::RenderWindow& window, const Player& player) {
    const float x = m_infoPanel.bounds.position.x + 12.f;
    const float y = m_infoPanel.bounds.position.y + 12.f;

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
        ss << "Selected: " << selectedCount;
        sf::Text t(m_font, ss.str(), 18);
        t.setFillColor(sf::Color::White);
        t.setPosition({ x, y });
        window.draw(t);
    }

    if (selectedCount == 1 && selected) {
        auto [hp, hpMax] = selected->getHealth();
        std::string type = toString(selected->getType());

        float hp01 = (hpMax > 0) ? (static_cast<float>(hp) / static_cast<float>(hpMax)) : 0.f;

        sf::Text t(m_font, type, 18);
        t.setFillColor(sf::Color::White);
        t.setPosition({ x, y + 28.f });
        window.draw(t);

        std::ostringstream ss;
        ss << "HP: " << hp << "/" << hpMax;
        sf::Text hpText(m_font, ss.str(), 18);
        hpText.setFillColor(sf::Color::White);
        hpText.setPosition({ x, y + 56.f });
        window.draw(hpText);

        drawHealthBarUI(window, { x, y + 84.f }, { m_infoPanel.bounds.size.x - 24.f, 16.f }, hp01);
    }
}

void UIManager::draw(sf::RenderWindow& window, const Player& player) {
    if (!m_ready) return;

    m_infoPanel.draw(window);
    m_actionsPanel.draw(window);
    m_mapPanel.draw(window);

    drawInfoPanelOverlay(window, player);
}

void UIManager::update(float dt) {
    if (!m_ready) return;

    for (auto& b : m_infoPanel.buttons)    b.updtate(dt);
	for (auto& b : m_actionsPanel.buttons) b.updtate(dt);
}

bool UIManager::handleEvent(const sf::Event& event, sf::RenderWindow& window, Player& player) {
    if (!m_ready) return false;

    if (event.is<sf::Event::Resized>()) {
        rebuildAll(window, player);
        return false;
    }

    // rebuild gdy zmienia się selekcja
    std::size_t h = computeSelectionHash(player);
    if (h != m_lastSelectionHash) {
        m_lastSelectionHash = h;
        rebuildAll(window, player);
    }

    // klik w UI
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePx(
                static_cast<float>(mb->position.x),
                static_cast<float>(mb->position.y)
            );

            // priorytet: actions -> info -> map
            if (m_actionsPanel.handleClick(mousePx)) return true;
            if (m_infoPanel.handleClick(mousePx)) return true;
            if (m_mapPanel.handleClick(mousePx)) return true;
        }
    }

    return false;
}

void UIManager::fillPanelGrid(
    UIPanel& panel,
    const std::vector<Action>& actions,
    int columns = 2,
    float padding = 12.f,
    float rowHeight = 42.f
) {
    panel.buttons.clear();
	if (actions.empty()) return;

	const float btnW = (panel.bounds.size.x - padding * (columns + 1)) / static_cast<float>(columns);
	const float btnH = rowHeight;

    for (std::size_t i = 0; i < actions.size(); ++i) {
		const int col = static_cast<int>(i) % columns;
		const int row = static_cast<int>(i) / columns;

		const float x = panel.bounds.position.x + padding + col * (btnW + padding);
		const float y = panel.bounds.position.y + padding + row * (btnH + padding);

		UIButton b(m_font);
		b.baseColor = sf::Color(30, 30, 30, 230);
		b.flashColor = sf::Color(80, 80, 80, 255);
		b.shape.setPosition({ x, y });
        b.shape.setSize({ btnW,btnH });
		b.shape.setFillColor(b.baseColor);
		b.shape.setOutlineColor(sf::Color(90, 90, 90));
		b.shape.setOutlineThickness(1.f);

		b.setText(actions[i].first, 18);
		b.label.setFillColor(sf::Color::White);
		b.label.setPosition({ x + 10.f, y + 8.f });

		b.onClick = actions[i].second;
		panel.buttons.push_back(std::move(b));
    }
}