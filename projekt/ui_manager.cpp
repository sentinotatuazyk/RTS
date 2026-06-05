#include "ui_manager.h"
#include "ui_manager.h"

#include "player.h"
#include "unit.h"
#include "map.h"
#include "enemy.h"
#include "utils.hpp"  // toString(UnitType)

#include <sstream>
#include <algorithm>
#include <cmath>

static std::size_t hashCombine(std::size_t h, std::size_t v) {
    return h ^ (v * 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2));
}


static bool anySelected(const Player& player, int arg) {
    if (arg == 1) {
        for (const auto& u : player.getUnits())
            if (u.isSelected()) return true;
        return false;
    }
    else {
        for (const auto& b : player.getBuildings())
			if (b.get()->isSelected()) return true;
        return false;
    }
}

bool UIManager::init(const std::string& fontPath) {
    if (!m_font.openFromFile(fontPath)) return false;
    m_ready = true;
    return true;
}

void UIManager::setMap(const Map* map) {
    m_mapRef = map;
}

std::size_t UIManager::computeSelectionHash(const Player& player) const {
    std::size_t h = 0;
    const auto& units = player.getUnits();
    for (std::size_t i = 0; i < units.size(); ++i) {
        if (units[i].isSelected())
            h = hashCombine(h, i + 1);
    }

    const auto& buildings = player.getBuildings();
    for (std::size_t i = 0; i < buildings.size(); ++i) {
        if (buildings[i] && buildings[i]->isSelected())
            h = hashCombine(h, (i + 1) * 0x9e3779b9);  // inny mnożnik = inna "przestrzeń" hashy
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
	m_resourcesPanel.bounds = sf::FloatRect({ colW + 10.f, top }, { colW / 2, m_panelHeight });
    m_actionsPanel.bounds = sf::FloatRect({ colW * 2.f + 1.f, top }, { colW, m_panelHeight });
	m_buildPanel.bounds = sf::FloatRect({ colW * 2.f + 1.f, top - 130.f }, { colW, 120.f});
    m_mapPanel.bounds = sf::FloatRect({ W - colW/1.5f, H - colW/1.5f }, { colW / 1.5f , colW/ 1.5f});
}

void UIManager::rebuildInfoPanel(const sf::RenderWindow&, Player&) {
    m_infoPanel.clear();
    // przyciski info nie są potrzebne teraz (overlay robimy w drawInfoPanelOverlay)
}

void UIManager::rebuildActionsPanel(const sf::RenderWindow&, Player& player) {
    m_actionsPanel.clear();
    m_showBuildPanel = false;  // resetuj panel budowy

    bool anyUnitSelected = anySelected(player, 1);
    bool anyBuildingSelected = anySelected(player, 2);

    // Nic nie zaznaczone — pusty panel
    if (!anyUnitSelected && !anyBuildingSelected) {
        return;
    }

    std::vector<Action> actions;

    if (anyUnitSelected) {
        // AKCJE DLA JEDNOSTEK
        bool hasSelectedWorker = false;
        for (const auto& u : player.getUnits()) {
            if (u.isSelected() && u.getType() == UnitType::Worker) {
                hasSelectedWorker = true;
                break;
            }
        }

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

        if (hasSelectedWorker) {
            actions.push_back({ "Build", [this]() {
                m_showBuildPanel = !m_showBuildPanel;
            } });
        }
    }
    else if (anyBuildingSelected) {
        // AKCJE DLA BUDYNKÓW
        actions.push_back({ "Deselect", [&player]() {
            for (auto& b : player.getBuildings())
                if (b && b->isSelected()) b->setSelected(false);
        } });

        actions.push_back({ "Demolish", [&player]() {
            // TODO: zaimplementuj niszczenie budynku
            player.addGold(5);
        } });

        // Sprawdź czy zaznaczono Barracks
        bool hasSelectedBarracks = false;
        for (const auto& b : player.getBuildings()) {
            if (b && b->getType() == BuildingType::Barracks && b->isSelected()) {
                hasSelectedBarracks = true;
                break;
            }
        }

        if (hasSelectedBarracks) {
            actions.push_back({ "Train Warrior", [&player]() {
                // TODO: trenuj wojownika
                player.addGold(5);
            } });
            actions.push_back({ "Train Archer", [&player]() {
                // TODO: trenuj łucznika
                player.addGold(5);
            } });
            actions.push_back({ "Train Hero", [&player]() {
                // TODO: trenuj bohatera
                player.addGold(5);
            } });
        }
    }

    fillPanelGrid(m_actionsPanel, actions, 2, 12.f, 42.f);
}

void UIManager::rebuildMapPanel(const sf::RenderWindow&, Player&) {
    m_mapPanel.clear();
    // placeholder; minimapa później
}

void UIManager::rebuildResourcesPanel(const sf::RenderWindow& window, Player& player)
{
	m_resourcesPanel.clear();
}



void UIManager::rebuildAll(const sf::RenderWindow& window, Player& player) {
    layoutPanels(window);
    rebuildInfoPanel(window, player);
    rebuildActionsPanel(window, player);
    rebuildMapPanel(window, player);
}

void UIManager::drawPausedOverlay(sf::RenderWindow& window)
{

	sf::RectangleShape overlay({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
	overlay.setFillColor(sf::Color(128, 128, 128, 100));
	overlay.setPosition({ 0.f, 0.f });

    sf::Text pausedText(m_font);
    pausedText.setString("PAUSED - Press SPACE to continue");
    pausedText.setCharacterSize(50);
    auto b = pausedText.getLocalBounds();
    pausedText.setFillColor(sf::Color::White);
    pausedText.setOrigin({ b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f });
    pausedText.setPosition({ window.getSize().x * 0.5f, window.getSize().y * 0.5f });

    window.draw(overlay);
    window.draw(pausedText);
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

void UIManager::drawRecourcesBarUI(sf::RenderWindow& window, const Player& player)
{
	const float x = m_resourcesPanel.bounds.position.x + 12.f;
    const float y = m_resourcesPanel.bounds.position.y + 12.f;

    std::ostringstream ss;
    ss << "Resources: "<<std::endl;
    ss << "Wood: " << player.getResource(ResourceType::Wood) << std::endl;
    ss << "Rock: " << player.getResource(ResourceType::Rock) << std::endl;
	ss << "Gold: " << player.getResource(ResourceType::Gold) << std::endl;
    sf::Text t(m_font, ss.str(), 18);
	t.setFillColor(sf::Color::White);
	t.setPosition({ x, y });
    window.draw(t);
    

}

void UIManager::drawBuildTypesUI(sf::RenderWindow& window, Player& player)
{
	m_buildPanel.clear();

    if (!m_showBuildPanel) return;


    std::vector<Action> actions;

	bool hasSelectedWorker = false;
    for (const auto& u : player.getUnits()) {
        if (u.isSelected() && u.getType() == UnitType::Worker) {
            hasSelectedWorker = true;
            break;
        }
    }
    
    if (!hasSelectedWorker) {
        m_showBuildPanel = false;
        return;
	}

    actions.push_back({ "Build Quarry", [&player]() {
            player.beginPlaceBuilding(BuildingType::Quarry);
    } });
    actions.push_back({ "Build Forester's Lodge", [&player]() {
            player.beginPlaceBuilding(BuildingType::Foresters);
    } });
    actions.push_back({ "Build GoldMine", [&player]() {
            player.beginPlaceBuilding(BuildingType::GoldMine);
    } });
    actions.push_back({ "Build Barracks", [&player]() {
            player.beginPlaceBuilding(BuildingType::Barracks);
	} });


	fillPanelGrid(m_buildPanel, actions, /*columns=*/1, /*padding=*/8.f, /*rowHeight=*/36.f);
}

void UIManager::drawInfoPanelOverlay(sf::RenderWindow& window, const Player& player) {
    const float x = m_infoPanel.bounds.position.x + 12.f;
    const float y = m_infoPanel.bounds.position.y + 12.f;

    const Unit* selectedUnit = nullptr;
	const Building* selectedBuilding = nullptr;
    int selectedCount = 0;

    for (const auto& u : player.getUnits()) {
        if (u.isSelected()) {
            selectedUnit = &u;
            ++selectedCount;
        }
    }

    for (const auto& b : player.getBuildings()) {
        if (b && b->isSelected()) {
            selectedBuilding = b.get();
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

    if (selectedCount == 1 && selectedUnit) {
        auto [hp, hpMax] = selectedUnit->getHealth();
        std::string type = toString(selectedUnit->getType());

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
    else if (selectedBuilding) {
        auto [hp, hpMax] = selectedBuilding->getHealth();
        std::string type = toString(selectedBuilding->getType());

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

void UIManager::drawMiniMap(sf::RenderWindow& window, const Player& player, const sf::View& gameView) {
    if (!m_mapRef) return;

    // Obszar minimapy (wewnątrz panelu z marginesem)
    float margin = 8.f;
    float mapX = m_mapPanel.bounds.position.x + margin;
    float mapY = m_mapPanel.bounds.position.y + margin;
    float mapW = m_mapPanel.bounds.size.x - margin * 2.f;
    float mapH = m_mapPanel.bounds.size.y - margin * 2.f;

    // Tło minimapy
    sf::RectangleShape minimapBg({ mapW, mapH });
    minimapBg.setPosition({ mapX, mapY });
    minimapBg.setFillColor(sf::Color(20, 20, 20, 255));
    minimapBg.setOutlineThickness(1.f);
    minimapBg.setOutlineColor(sf::Color(80, 80, 80, 255));
    window.draw(minimapBg);

    auto [mapWidth, mapHeight] = m_mapRef->getSize();
    float tileSize = m_mapRef->getTileSize();
    float worldW = static_cast<float>(mapWidth) * tileSize;
    float worldH = static_cast<float>(mapHeight) * tileSize;

    // Skala: ile pikseli minimapy na jednostkę świata
    float scaleX = mapW / worldW;
    float scaleY = mapH / worldH;
    float scale = std::min(scaleX, scaleY);

    // Wyśrodkuj minimapę w panelu
    float offsetX = mapX + (mapW - worldW * scale) * 0.5f;
    float offsetY = mapY + (mapH - worldH * scale) * 0.5f;

    // Rysuj teren (uproszczony — co N-ty tile dla wydajności)
    const int skip = 4; // Pomijaj co 4 tile'y
    sf::VertexArray terrainDots(sf::PrimitiveType::Triangles);

    for (unsigned int y = 0; y < mapHeight; y += skip) {
        for (unsigned int x = 0; x < mapWidth; x += skip) {
            TileType type = m_mapRef->getTile(x, y);
            sf::Color color;

            switch (type) {
            case TileType::Water:  color = sf::Color(64, 164, 223); break;
            case TileType::Sand:   color = sf::Color(194, 178, 128); break;
            case TileType::Grass:  color = sf::Color(34, 139, 34); break;
            case TileType::Mountain:   color = sf::Color(139, 137, 137); break;
            case TileType::Snow:   color = sf::Color(255, 250, 250); break;
            default:               color = sf::Color::Magenta;
            }

            float px = offsetX + static_cast<float>(x) * tileSize * scale;
            float py = offsetY + static_cast<float>(y) * tileSize * scale;

            terrainDots.append({ {px, py}, color });
            terrainDots.append({ {px + 9, py}, color });
            terrainDots.append({ {px, py + 9}, color });

            terrainDots.append({ {px + 9, py}, color });
            terrainDots.append({ {px + 9, py + 9}, color });
            terrainDots.append({ {px, py + 9}, color });
        }
    }
    window.draw(terrainDots);

    // Rysuj jednostki gracza (niebieskie kropki)
    for (const auto& unit : player.getUnits()) {
        sf::Vector2f pos = unit.getPosition();
        float px = offsetX + pos.x * scale;
        float py = offsetY + pos.y * scale;

        sf::CircleShape dot(3.f);
        dot.setPosition({ px - 3.f, py - 3.f });
        dot.setFillColor(unit.isSelected() ? sf::Color::Yellow : sf::Color::Cyan);
        window.draw(dot);
    }

    for (const auto& building : player.getBuildings()) {
		auto b = building.get();
		sf::Vector2f pos = b->getPosition();
		float px = offsetX + pos.x * scale;
		float py = offsetY + pos.y * scale;

		sf::RectangleShape rect({ 4.f, 4.f});
		rect.setPosition({ px - 2.f, py - 2.f });
        rect.setFillColor(b->getColor());
		window.draw(rect);
    }

    // Rysuj prostokąt widoku kamery (biała ramka)
    sf::Vector2f viewCenter = gameView.getCenter();
    sf::Vector2f viewSize = gameView.getSize();

    float viewX = offsetX + (viewCenter.x - viewSize.x / 2.f) * scale;
    float viewY = offsetY + (viewCenter.y - viewSize.y / 2.f) * scale;
    float viewW = viewSize.x * scale;
    float viewH = viewSize.y * scale;

    sf::RectangleShape viewRect({ viewW, viewH });
    viewRect.setPosition({ viewX, viewY });
    viewRect.setFillColor(sf::Color::Transparent);
    viewRect.setOutlineThickness(1.5f);
    viewRect.setOutlineColor(sf::Color::White);
    window.draw(viewRect);
}

void UIManager::draw(sf::RenderWindow& window, const Player& player, const sf::View& view) {
    if (!m_ready) return;

    m_infoPanel.draw(window);
	m_resourcesPanel.draw(window);
    m_actionsPanel.draw(window);
    if (m_showBuildPanel) {
        m_buildPanel.draw(window);
		drawBuildTypesUI(window, const_cast<Player&>(player)); // const_cast bo drawBuildTypesUI modyfikuje stan UI (pokazuje/ukrywa panel), ale to jest tylko implementacja detali, więc powinno być ok
    }
    m_mapPanel.draw(window);

    drawInfoPanelOverlay(window, player);
    drawRecourcesBarUI(window, player);
	drawMiniMap(window, player, view);

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

            if (m_showBuildPanel && m_buildPanel.handleClick(mousePx)) {
				m_showBuildPanel = false; // klik w panel budowy też go ukrywa
                return true;
            }

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