#include "menu.h"
#include "utils.h"

Menu::Menu(sf::RenderWindow& window) : m_window(window), m_title(m_font), m_start(m_font), m_settings(m_font), m_exit(m_font) {
	std::string path = assetPath("geistmono_light.ttf");
    if (!(m_font.openFromFile(path))) {
		throw std::runtime_error(std::string("Nieudano sie załadowac czcionki. Path: ")+ path);
	}

	const auto winSize = m_window.getSize();
	const float centerX = static_cast<float>(winSize.x) * 0.5f;
	const float topY = static_cast<float>(winSize.y) * 0.2f;
	const float spacing = 70.f;

    m_title.setString("Gra RTS / Konrad Toborek");
    m_title.setCharacterSize(50);

    m_start.setString("Start");
    m_start.setCharacterSize(50);

	m_settings.setString("Settings");
	m_settings.setCharacterSize(50);

    m_exit.setString("Exit");
    m_exit.setCharacterSize(50);

    auto centerTextX = [centerX](sf::Text& text, float y) {
        const auto b = text.getLocalBounds();
        text.setOrigin({ b.position.x + b.size.x * 0.5f, b.position.y });
        text.setPosition({ centerX,y });
        };

    centerTextX(m_title, topY);
    centerTextX(m_start, topY + spacing * 2.f);
	centerTextX(m_settings, topY + spacing * 3.f);
    centerTextX(m_exit, topY + spacing * 4.f);


    updateSelection();

}

Menu::~Menu()
{
}

MenuAction Menu::handleEvent(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        constexpr int numOption = 3;

        if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
            m_selected = (m_selected + numOption - 1) % numOption;
            updateSelection();
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
            m_selected = (m_selected + 1) % numOption;
            updateSelection();
        }
        else if (keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
            if (m_selected == 0) return MenuAction::StartGame;
            if (m_selected == 1) {  return MenuAction::Settings; }
			if (m_selected == 2) return MenuAction::ExitGame;
        }
    }
    return MenuAction::None;

}

void Menu::update() {

}

void Menu::draw() {
    if (m_font.getInfo().family.empty()) {
        return; 
    }
    rebuildLayout();

    m_window.draw(m_title);
    m_window.draw(m_start);
	m_window.draw(m_settings);
    m_window.draw(m_exit);
}

void Menu::updateSelection() {
    m_start.setFillColor(m_selected == 0 ? sf::Color::Yellow : sf::Color::White);
    m_settings.setFillColor(m_selected == 1 ? sf::Color::Yellow : sf::Color::White);
    m_exit.setFillColor(m_selected == 2 ? sf::Color::Yellow : sf::Color::White);
}

void Menu::rebuildLayout() {
    const auto winSize = m_window.getSize();
    const float centerX = static_cast<float>(winSize.x) * 0.5f;
    const float topY = static_cast<float>(winSize.y) * 0.2f;
    const float spacing = 70.f;

    auto centerText = [centerX](sf::Text& text, float y) {
        const auto b = text.getLocalBounds();
        text.setOrigin({ b.position.x + b.size.x * 0.5f, b.position.y });
        text.setPosition({ centerX, y });
        };
    centerText(m_title, topY);
    centerText(m_start, topY + spacing * 2.f );
    centerText(m_settings, topY + spacing * 3.f );
    centerText(m_exit, topY + spacing * 4.f );
}
