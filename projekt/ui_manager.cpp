#include "ui_manager.h"
#include "ui_manager.h"

#include "player.h"
#include "unit.h"
#include "map.h"
#include "enemy.h"
#include "utils.hpp"  // toString(UnitType)
#include "buildings.h"

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

std::size_t UIManager::computeSelectionHashUnit(const Player& player) const {
    std::size_t h = 0;
    const auto& units = player.getUnits();
    for (std::size_t i = 0; i < units.size(); ++i) {
        if (units[i].isSelected())
            h = hashCombine(h, i + 1);
    }
    return h;
}
std::size_t UIManager::computeSelectionHashBuilding(const Player& player) const {
    std::size_t h = 0;
    const auto& b = player.getBuildings();
    for (std::size_t i = 0; i < b.size(); ++i) {
        if (b[i]->isSelected())
            h = hashCombine(h, (i + 1) * 0x9e3779b9);
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
	m_buildPanel.bounds = sf::FloatRect({ colW * 2.f + 1.f, top - 190.f }, { colW, 1900.f});
    m_mapPanel.bounds = sf::FloatRect({ W - colW/1.5f, H - colW/1.5f }, { colW / 1.5f , colW/ 1.5f });
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

	unsigned short barackLvl = 0;

    if (anyUnitSelected) {
        // AKCJE DLA JEDNOSTEK
        bool hasSelectedWorker = false;
        for (const auto& u : player.getUnits()) {
            if (u.isSelected() && u.getType() == UnitType::Worker) {
                hasSelectedWorker = true;
                break;
            }
        }

        actions.push_back({ "Stop", [&player, this]() {
            for (auto& u : player.getUnits()) if (u.isSelected()) u.stop();
            addNotification("Units stopped!", sf::Color::White, 2.f);
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
                clearTooltip();
            } });
        }
    }
    else if (anyBuildingSelected) {
        // AKCJE DLA BUDYNKÓW

        actions.push_back({ "Demolish", [&player, this]() {
            bool anyDemolished = false;
            for (auto& b : player.getBuildings()) {
                if (b && b->isSelected()) {
                    anyDemolished = true;
                    break;
                }
            }

            player.demolishSelectedBuildings();
            if (anyDemolished) addNotification("Building demolished!", sf::Color::Red, 2.f);
        } });

        
        // Sprawdź czy zaznaczono Barracks
        Barracks* selectedBarracks = nullptr;
        for (const auto& b : player.getBuildings()) {
            if (b && b->getType() == BuildingType::Barracks && b->isSelected()) {
                selectedBarracks = dynamic_cast<Barracks*>(b.get());
                break;
            }
        }

        


        if (selectedBarracks) {
            if (selectedBarracks->isTraining()) {
                actions.push_back({ "Training...", []() {} });
            }
            else {
                unsigned short lvl = selectedBarracks->getLvl();
				barackLvl = lvl;
                
                if ( lvl < 3 ){
                    actions.push_back({ "Upgrade", [&player, selectedBarracks, this, lvl]() {
                        Cost cost = getUpgradeCost(BuildingType::Barracks, lvl + 1);
                        switch (lvl) {
                        case 3:
							addNotification("Barracks are already at max level!", sf::Color::Yellow, 5.f);
                            break;
                        default:
                            if (player.canAfford(cost)) {
                                player.spendResource(ResourceType::Gold, cost.gold);
                                player.spendResource(ResourceType::Wood, cost.wood);
                                player.spendResource(ResourceType::Rock, cost.rock);
                                selectedBarracks->upgrade(player);
                                addNotification("Barracks are being upgraded...", sf::Color::Yellow, 5.f);
                            }
                            else {
                                addNotification("Not enough recources for upgrade!", sf::Color::Red, 5.f);
                            }
                            break;
}
                        } });
                }
                
                if (lvl >= 1) {
                    actions.push_back({ "Train Warrior", [&player, selectedBarracks, this]() {
                        Cost cost = getUnitCost(UnitType::Warrior);
                        if (player.canAfford(cost)) {
                            player.spendResource(ResourceType::Gold, cost.gold);
                            player.spendResource(ResourceType::Wood, cost.wood);
                            player.spendResource(ResourceType::Rock, cost.rock);
                            selectedBarracks->startTraining(UnitType::Warrior);
                            addNotification("Training Warrior...", sf::Color::Yellow, 5.f);
                        }
                        else {
                            addNotification("Not enough recources!", sf::Color::Red, 5.f);
                        }

                    } });
                }

                if (lvl >= 2) {
                    actions.push_back({ "Train Archer", [&player, selectedBarracks,this]() {
                        Cost cost = getUnitCost(UnitType::Archer);
                        if (player.canAfford(cost)) {
                            player.spendResource(ResourceType::Gold, cost.gold);
                            player.spendResource(ResourceType::Wood, cost.wood);
                            player.spendResource(ResourceType::Rock, cost.rock);
                            selectedBarracks->startTraining(UnitType::Archer);
                            addNotification("Training Archer...", sf::Color::Yellow, 5.f);
                        }
                        else {
                            addNotification("Not enough recources!", sf::Color::Red, 5.f);
                        }
                    } });
                }

                if (lvl == 3) {
                    actions.push_back({ "Train Hero", [&player, selectedBarracks,this]() {
                        Cost cost = getUnitCost(UnitType::Hero);
                        if (player.canAfford(cost)) {
                            player.spendResource(ResourceType::Gold, cost.gold);
                            player.spendResource(ResourceType::Wood, cost.wood);
                            player.spendResource(ResourceType::Rock, cost.rock);
                            selectedBarracks->startTraining(UnitType::Hero);
                            addNotification("Training Hero...", sf::Color::Yellow, 5.f);
                        }
                        else {
                            addNotification("Not enough recources!", sf::Color::Red, 5.f);
                        }
                    } });
                }
            }
        }
    }

    auto& buttons = fillPanelGrid(m_actionsPanel, actions, 2, 12.f, 42.f);

    // ========== USTAW TOOLTIPY ==========
    for (auto& btn : buttons) {
        std::string label = btn.label.getString().toAnsiString();

        if (label == "Upgrade") {
			auto c = getUpgradeCost(BuildingType::Barracks, barackLvl + 1);
            btn.tooltip = "Upgrade to level " + std::to_string(barackLvl + 1) + "\nCost: " + std::to_string(c.gold) + "G "
                + std::to_string(c.wood) + "W "
				+ std::to_string(c.rock) + "R";
        }
        else if (label == "Train Warrior") {
            auto c = getUnitCost(UnitType::Warrior);
            float t = getTrainingTime(UnitType::Warrior);
            btn.tooltip = "Cost: " + std::to_string(c.gold) + "G "
                + std::to_string(c.wood) + "W "
                + std::to_string(c.rock) + "R\nTime: "
                + std::to_string(static_cast<int>(t)) + "s\nFood spend: "
                + std::to_string(c.food) + "/s";
        }
        else if (label == "Train Archer") {
            auto c = getUnitCost(UnitType::Archer);
            float t = getTrainingTime(UnitType::Archer);
            btn.tooltip = "Cost: " + std::to_string(c.gold) + "G "
                + std::to_string(c.wood) + "W "
                + std::to_string(c.rock) + "R\nTime: "
                + std::to_string(static_cast<int>(t)) + "s\nFood spend: "
                + std::to_string(c.food) + "/s";
        }
        else if (label == "Train Hero") {
            auto c = getUnitCost(UnitType::Hero);
            float t = getTrainingTime(UnitType::Hero);
            btn.tooltip = "Cost: " + std::to_string(c.gold) + "G "
                + std::to_string(c.wood) + "W "
                + std::to_string(c.rock) + "R\nTime: "
                + std::to_string(static_cast<int>(t)) + "s\nFood spend: "
                + std::to_string(c.food) + "/s";
        }
        else if (label == "Demolish") {
            btn.tooltip = "Destroy building";
        }
        else if (label == "Build") {
            btn.tooltip = "Open build menu";
        }
        else if (label == "Stop") {
            btn.tooltip = "Stop all selected units";
        }
        else if (label == "Aggressive") {
            btn.tooltip = "Attack enemies on sight";
        }
        else if (label == "Passive") {
            btn.tooltip = "Flee from enemies";
        }
        else if (label == "Neutral") {
            btn.tooltip = "Defend only when attacked";
        }
    }


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

void UIManager::addNotification(const std::string& text, sf::Color color, float duration)
{
    UINotification n;
    n.text = text;
    n.color = color;
    n.lifetime = duration;
    n.timeLeft = duration;
    n.slideIn = 0.3f;

    m_notifications.push_back(n);

    while (m_notifications.size() > 8) {
        m_notifications.pop_front();
    }
}

void UIManager::addNotification(const std::string& text, float duration) {
    addNotification(text, sf::Color::White, duration);
}

void UIManager::setTooltip(const std::string& text, sf::Vector2f mousePos)
{
    if (!m_tooltip || m_tooltip->text != text) {
        m_tooltip = UIToolTip{ text,mousePos,0.f };
    }
    m_tooltip->position = mousePos;
}

void UIManager::clearTooltip()
{
    m_tooltip = std::nullopt;
}

void UIManager::forceRebuild(const sf::RenderWindow& window, Player& player) {
    if (!m_ready) return;
    m_lastSelectionHashUnit = computeSelectionHashUnit(player);
	m_lastSelectionHashBuilding = computeSelectionHashBuilding(player);
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
	ss << "Food: " << player.getResource(ResourceType::Food) << std::endl;
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

    actions.push_back({ "Fence", [&player,this]() {
        auto cost = getBuildingCost(BuildingType::Fence);
        if (player.canAfford(cost)) {
            player.beginPlaceBuilding(BuildingType::Fence);
            //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
        }
        else {
            addNotification("Not enough recources!", sf::Color::Red, 2.f);

        }
    } });

    if (!player.hasTownHall()) {
        actions.push_back({ "Build Townhall", [&player,this]() {
                auto cost = getBuildingCost(BuildingType::TownHall);
                if (player.canAfford(cost)) {
                    player.beginPlaceBuilding(BuildingType::TownHall);
                    //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
                }
                else {
                    addNotification("Not enough recources!", sf::Color::Red, 2.f);
                }
            } });
    }

    actions.push_back({ "Build Quarry", [&player,this]() {
        auto cost = getBuildingCost(BuildingType::Quarry);
        if (player.canAfford(cost)) {
            player.beginPlaceBuilding(BuildingType::Quarry);
            //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
        }
        else {
            addNotification("Not enough recources!", sf::Color::Red, 2.f);
        }
            
    } });
    actions.push_back({ "Build Forester's Lodge", [&player,this]() {
        auto cost = getBuildingCost(BuildingType::Foresters);
        if (player.canAfford(cost)) {
            player.beginPlaceBuilding(BuildingType::Foresters);
            //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
        }
        else {
            addNotification("Not enough recources!", sf::Color::Red, 2.f);
        }
    } });
    actions.push_back({ "Build GoldMine", [&player,this]() {
            auto cost = getBuildingCost(BuildingType::GoldMine);
        if (player.canAfford(cost)) {
            player.beginPlaceBuilding(BuildingType::GoldMine);
            //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
        }
        else {
            addNotification("Not enough recources!", sf::Color::Red, 2.f);
        }
    } });
    actions.push_back({ "Build Barracks", [&player,this]() {
            auto cost = getBuildingCost(BuildingType::Barracks);
        if (player.canAfford(cost)) {
            player.beginPlaceBuilding(BuildingType::Barracks);
            //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
        }
        else {
            addNotification("Not enough recources!", sf::Color::Red, 2.f);
        }
	} });
    actions.push_back({"Build Farm", [&player,this]() {
            auto cost = getBuildingCost(BuildingType::Farm);
            if (player.canAfford(cost)) {
                player.beginPlaceBuilding(BuildingType::Farm);
                //BEZ NOTYFIKACJI BO WYSWIETLA SIE PRZY KLIKU, NIE PRZY ROZPOCZECIU BUDOWY
            }
            else {
                addNotification("Not enough recources!", sf::Color::Red, 2.f);
            }
        } });


    auto& buttons = fillPanelGrid(m_buildPanel, actions, 1, 8.f, 38.f);

    // ========== USTAW TOOLTIPY DLA BUDYNKÓW ==========
    for (auto& btn : buttons) {
        std::string label = btn.label.getString().toAnsiString();

        if (label == "Build TownHall") {
            auto cost = getBuildingCost(BuildingType::TownHall);
            std::stringstream ss;
            ss << "Cost: " << cost.gold << "G, " << cost.wood << "W, " << cost.rock << "R\nMain building - required for others ";
            btn.tooltip = ss.str();
        }
        else if (label == "Build Quarry") {
            auto cost = getBuildingCost(BuildingType::Quarry); 
            std::stringstream ss;
            ss << "Cost: " << cost.gold << "G, " << cost.wood << "W, " << cost.rock << "R\nGenerates: Rock\nRequires: Mountain tile nearby";
            btn.tooltip = ss.str();
        }
        else if (label == "Build Forester's Lodge") {
            auto cost = getBuildingCost(BuildingType::Foresters);
            std::stringstream ss;
            ss << "Cost: " << cost.gold << "G, " << cost.wood << "W, " << cost.rock << "R\nGenerates: Wood";
            btn.tooltip = ss.str();
        }
        else if (label == "Build GoldMine") {
            auto cost = getBuildingCost(BuildingType::GoldMine);
            std::stringstream ss;
            ss << "Cost: " << cost.gold << "G, " << cost.wood << "W, " << cost.rock << "R\nGenerates: Gold\nRequires: Sand tile";
            btn.tooltip = ss.str();
        }
        else if (label == "Build Barracks") {
            auto cost = getBuildingCost(BuildingType::Barracks);
            std::stringstream ss;
            ss << "Cost: " << cost.gold << "G, " << cost.wood << "W, " << cost.rock << "R\nTrains: Warrior, Archer, Hero";
            btn.tooltip = ss.str();
        }
        else if (label == "Build Farm") {
            auto cost = getBuildingCost(BuildingType::Farm);
            std::stringstream ss;
            ss << "Cost: " << cost.gold << "G, " << cost.wood << "W, " << cost.rock << "R\nGenerates: Food\nRequired for training units";
			btn.tooltip = ss.str();
        }

    }
}

void UIManager::drawInfoPanelOverlay(sf::RenderWindow& window, const Player& player) {
    const float x = m_infoPanel.bounds.position.x + 12.f;
    const float y = m_infoPanel.bounds.position.y + 12.f;

    const Unit* selectedUnit = nullptr;
	Building* selectedBuilding = nullptr;
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

        if (selectedBuilding->getType() == BuildingType::Barracks) {
            auto* barracks = dynamic_cast<Barracks*>(selectedBuilding);
            if (barracks && barracks->isTraining()) {
                float progress = barracks->getTrainProgress();
                drawHealthBarUI(window, { x,y + 110.f }, { m_infoPanel.bounds.size.x - 24.f,12.f }, progress);
                sf::Text tt(m_font, "Training...", 16);
                tt.setFillColor(sf::Color::Yellow);
                tt.setPosition({ x, y + 130.f });
                window.draw(tt);
            }
        }
        else if (selectedBuilding->getType() == BuildingType::GoldMine) {
            auto* goldMine = dynamic_cast<GoldMine*>(selectedBuilding);
            if (goldMine) {
				sf::Text tt(m_font, "+1G/s", 16);
                tt.setFillColor(sf::Color::Yellow);
                tt.setPosition({x, y + 130.f});
                window.draw(tt);
			}
        }
        else if (selectedBuilding->getType() == BuildingType::Quarry) {
            auto* quarry = dynamic_cast<Quarry*>(selectedBuilding);
            if (quarry) {
                sf::Text tt(m_font, "+5R/s", 16);
                tt.setFillColor(sf::Color::Yellow);
                tt.setPosition({ x, y + 130.f });
                window.draw(tt);
            }
        }
        else if (selectedBuilding->getType() == BuildingType::Foresters) {
            auto* foresters= dynamic_cast<Foresters*>(selectedBuilding);
            if (foresters) {
                sf::Text tt(m_font, "+5W/s", 16);
                tt.setFillColor(sf::Color::Yellow);
                tt.setPosition({ x, y + 130.f });
                window.draw(tt);
            }
        }
        else if (selectedBuilding->getType() == BuildingType::Farm) {
            auto* farm = dynamic_cast<Farm*>(selectedBuilding);
            if (farm) {
                int output = player.calculateFoodIncome();
				std::ostringstream ss;
				ss << output << "F/s";
                sf::Text tt(m_font, ss.str(), 16);
                tt.setFillColor(sf::Color::Yellow);
                tt.setPosition({ x, y + 130.f });
                window.draw(tt);
            }
        }

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

void UIManager::drawNotification(sf::RenderWindow& window)
{
    if (m_notifications.empty()) return;

    const float winW = static_cast<float>(window.getSize().x);
    const float startX = winW - NOTIFICATION_MAX_WIDTH - NOTIFICATION_MARGIN;

    float currentY = NOTIFICATION_MARGIN;

    for (const auto& n : m_notifications) {
        float alpha = 255.f;
        if (n.timeLeft < 1.f) {
            alpha = n.timeLeft * 255.f;
        }

        float slideOffset = 0.f;
        float elapsed = n.lifetime - n.timeLeft;
        if (elapsed < n.slideIn) {
            float t = elapsed / n.slideIn;
            t = 1.f - std::pow(1.f - t, 3.f);
            slideOffset = (1.f - t) * NOTIFICATION_MAX_WIDTH;
        }

        float x = startX + slideOffset;
        float y = currentY;

        sf::RectangleShape bg({ NOTIFICATION_MAX_WIDTH, NOTIFICATION_HEIGHT });
        bg.setPosition({ x, y });
        sf::Color bgColor(20, 20, 20, static_cast<std::uint8_t>(alpha * 0.85f));
        bg.setFillColor(bgColor);
        bg.setOutlineThickness(1.f);
        sf::Color outlineColor = n.color;
        outlineColor.a = static_cast<std::uint8_t>(alpha * 0.6f);
        bg.setOutlineColor(outlineColor);
        window.draw(bg);

        // Pasek postępu (malejący)
        float progress = n.timeLeft / n.lifetime;
        sf::RectangleShape progressBar({ NOTIFICATION_MAX_WIDTH * progress, 3.f });
        progressBar.setPosition({ x, y + NOTIFICATION_HEIGHT - 3.f });
        sf::Color progressColor = n.color;
        progressColor.a = static_cast<std::uint8_t>(alpha);
        progressBar.setFillColor(progressColor);
        window.draw(progressBar);

        // Tekst
        sf::Text text(m_font, n.text, 14);
        text.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));

        // Przytnij tekst jeśli za długi
        auto bounds = text.getLocalBounds();
        if (bounds.size.x > NOTIFICATION_MAX_WIDTH - 20.f) {
            std::string truncated = n.text;
            while (!truncated.empty()) {
                text.setString(truncated + "...");
                bounds = text.getLocalBounds();
                if (bounds.size.x <= NOTIFICATION_MAX_WIDTH - 20.f) break;
                truncated.pop_back();
            }
        }

        text.setPosition({ x + 10.f, y + (NOTIFICATION_HEIGHT - text.getLocalBounds().size.y) / 2.f - 2.f });
        window.draw(text);

        currentY += NOTIFICATION_HEIGHT + NOTIFICATION_SPACING;
    }
}

void UIManager::drawTooltip(sf::RenderWindow& window)
{
    if (!m_tooltip) return;

    const float padding = 8.f;
    const float maxWidth = 300.f;

    sf::Text text(m_font, m_tooltip->text, 14);
    text.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(m_tooltip->alpha)));

    auto bounds = text.getLocalBounds();
    float w = std::min(bounds.size.x + padding * 2.f, maxWidth);
    float h = bounds.size.y + (padding+2.f) * 2.f;

    // Pozycja: obok kursora, ale w granicach ekranu
    float x = m_tooltip->position.x + 15.f;
    float y = m_tooltip->position.y + 15.f;

    // Nie wychodź poza ekran
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    if (x + w > winW) x = m_tooltip->position.x - w - 10.f;
    if (y + h > winH) y = m_tooltip->position.y - h - 10.f;

    // Tło
    sf::RectangleShape bg({ w, h });
    bg.setPosition({ x, y });
    sf::Color bgColor(10, 10, 10, static_cast<std::uint8_t>(m_tooltip->alpha * 0.9f));
    bg.setFillColor(bgColor);
    bg.setOutlineThickness(1.f);
    bg.setOutlineColor(sf::Color(120, 120, 120, static_cast<std::uint8_t>(m_tooltip->alpha * 0.5f)));
    window.draw(bg);

    text.setPosition({ x + padding, y + padding });
    window.draw(text);
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
    drawNotification(window);
    drawTooltip(window);

}

void UIManager::update(float dt) {
    if (!m_ready) return;

    for (auto& b : m_infoPanel.buttons)    b.updtate(dt);
	for (auto& b : m_actionsPanel.buttons) b.updtate(dt);

    for (auto& n : m_notifications) {
        n.timeLeft -= dt;
    }

    while (!m_notifications.empty() && m_notifications.front().timeLeft <= 0.f) {
        m_notifications.pop_front();
    }

    if (m_tooltip) {
        if (m_tooltip->alpha < m_tooltip->targetAlpha) {
            m_tooltip->alpha += TOOLTIP_FADE_SPEED * dt;
            if (m_tooltip->alpha > m_tooltip->targetAlpha)
                m_tooltip->alpha = m_tooltip->targetAlpha;
        }
    }
}

bool UIManager::handleEvent(const sf::Event& event, sf::RenderWindow& window, Player& player) {
    if (!m_ready) return false;

    if (event.is<sf::Event::Resized>()) {
        rebuildAll(window, player);
        return false;
    }

    // rebuild gdy zmienia się selekcja
    std::size_t h1 = computeSelectionHashUnit(player);
    std::size_t h2 = computeSelectionHashBuilding(player);
    if (h1 != m_lastSelectionHashUnit || h2 !=m_lastSelectionHashBuilding) {
        m_lastSelectionHashUnit = h1;
		m_lastSelectionHashBuilding = h2;
        rebuildAll(window, player);
    }

    sf::Vector2f mousePos(
        static_cast<float>(sf::Mouse::getPosition(window).x),
        static_cast<float>(sf::Mouse::getPosition(window).y)
    );


    // Sprawdź hover nad przyciskami
    bool hoveringButton = false;
    std::string hoveredTooltip;

    // 1. Build panel (najwyżej, tylko gdy widoczny)
    if (m_showBuildPanel && m_buildPanel.bounds.contains(mousePos)) {
        for (const auto& btn : m_buildPanel.buttons) {
            if (btn.hitTest(mousePos) && !btn.tooltip.empty()) {
                hoveredTooltip = btn.tooltip;
                hoveringButton = true;
                break;
            }
        }
    }

    // 2. Actions panel (tylko jeśli nie hover nad buildPanel)
    if (!hoveringButton && m_actionsPanel.bounds.contains(mousePos)) {
        for (const auto& btn : m_actionsPanel.buttons) {
            if (btn.hitTest(mousePos) && !btn.tooltip.empty()) {
                hoveredTooltip = btn.tooltip;
                hoveringButton = true;
                break;
            }
        }
    }

    // 3. Info panel (najniżej)
    if (!hoveringButton && m_infoPanel.bounds.contains(mousePos)) {
        for (const auto& btn : m_infoPanel.buttons) {
            if (btn.hitTest(mousePos) && !btn.tooltip.empty()) {
                hoveredTooltip = btn.tooltip;
                hoveringButton = true;
                break;
            }
        }
    }

    // Ustaw lub wyczyść tooltip
    if (hoveringButton) {
        setTooltip(hoveredTooltip, mousePos);
    }
    else {
        clearTooltip();
    }

    // klik w UI
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            sf::Vector2f mousePx(
                static_cast<float>(mb->position.x),
                static_cast<float>(mb->position.y)
            );

            // priorytet: build -> actions -> info -> map

            if (m_showBuildPanel && m_buildPanel.handleClick(mousePx)) {
				m_showBuildPanel = false; 
                return true;
            }

            if (m_actionsPanel.handleClick(mousePx)) return true;
            if (m_infoPanel.handleClick(mousePx)) return true;
            if (m_mapPanel.handleClick(mousePx)) return true;
        }
    }

    return false;
}

std::vector<UIButton>& UIManager::fillPanelGrid(
    UIPanel& panel,
    const std::vector<Action>& actions,
    int columns = 2,
    float padding = 12.f,
    float rowHeight = 42.f
) {
    panel.buttons.clear();
	if (actions.empty()) return panel.buttons;

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
    return panel.buttons;
}