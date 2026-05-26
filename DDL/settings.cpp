#include "settings.h"
#include <sstream>
#include <algorithm>

SettingsScreen::SettingsScreen() :
    m_title(m_font),
    m_resLabel(m_font),
    m_fpsLabel(m_font),
    m_resValue(m_font),
    m_resPrevText(m_font),
    m_resNextText(m_font),
    m_fsText(m_font),
    m_fpsValue(m_font),
    m_fpsMinusText(m_font),
    m_fpsPlusText(m_font),
    m_applyText(m_font),
    m_backText(m_font)
{
}

const std::vector<sf::Vector2u>& AppSettings::presets() {
    static std::vector<sf::Vector2u> p = {
        {1280, 720},
        {1366, 768},
        {1600, 900},
        {1920, 1080},
        {2560, 1440}
    };
    return p;
}

std::string AppSettings::presetToString(sf::Vector2u r) {
    return std::to_string(r.x) + " x " + std::to_string(r.y);
}

sf::Vector2u AppSettings::chosenResolutionOrDesktop() const {
    if (fullscreen) {
        return sf::VideoMode::getDesktopMode().size;
    }
    const auto& p = presets();
    int idx = std::clamp(resolutionIndex, 0, static_cast<int>(p.size()) - 1);
    return p[idx];
}

bool SettingsScreen::init(const std::string& fontPath) {
    if (!m_font.openFromFile(fontPath)) return false;
    m_ready = true;

    m_title = sf::Text(m_font, "Settings", 32);
    m_title.setFillColor(sf::Color::White);

    m_resLabel = sf::Text(m_font, "Resolution", 20);
    m_resLabel.setFillColor(sf::Color::White);

    m_fpsLabel = sf::Text(m_font, "FPS limit", 20);
    m_fpsLabel.setFillColor(sf::Color::White);

    // Panel styl
    m_panel.setFillColor(sf::Color(10, 10, 10, 220));
    m_panel.setOutlineThickness(1.f);
    m_panel.setOutlineColor(sf::Color(80, 80, 80, 255));

    auto initBtn = [&](sf::RectangleShape& r, sf::Text& t, const std::string& s) {
        r.setFillColor(sf::Color(30, 30, 30, 230));
        r.setOutlineThickness(1.f);
        r.setOutlineColor(sf::Color(90, 90, 90));
        t = sf::Text(m_font, s, 20);
        t.setFillColor(sf::Color::White);
        };

    // Resolution controls
    initBtn(m_resPrev, m_resPrevText, "<");
    initBtn(m_resNext, m_resNextText, ">");

    m_resBox.setFillColor(sf::Color(20, 20, 20, 230));
    m_resBox.setOutlineThickness(1.f);
    m_resBox.setOutlineColor(sf::Color(90, 90, 90));
    m_resValue = sf::Text(m_font, "", 20);
    m_resValue.setFillColor(sf::Color::White);

    // Fullscreen
    m_fsBox.setFillColor(sf::Color(20, 20, 20, 230));
    m_fsBox.setOutlineThickness(1.f);
    m_fsBox.setOutlineColor(sf::Color(90, 90, 90));
    m_fsCheck.setFillColor(sf::Color(0, 200, 0, 255)); // rysujemy tylko gdy zaznaczone
    m_fsText = sf::Text(m_font, "Fullscreen (exclusive)", 20);
    m_fsText.setFillColor(sf::Color::White);

    // FPS controls
    initBtn(m_fpsMinus, m_fpsMinusText, "-");
    initBtn(m_fpsPlus, m_fpsPlusText, "+");

    m_fpsBox.setFillColor(sf::Color(20, 20, 20, 230));
    m_fpsBox.setOutlineThickness(1.f);
    m_fpsBox.setOutlineColor(sf::Color(90, 90, 90));
    m_fpsValue = sf::Text(m_font, "", 20);
    m_fpsValue.setFillColor(sf::Color::White);

    // Apply / Back
    initBtn(m_applyBtn, m_applyText, "Apply");
    initBtn(m_backBtn, m_backText, "Back");

    return true;
}

void SettingsScreen::open(const AppSettings& current) {
    m_edit = current;
}

const AppSettings& SettingsScreen::edited() const {
    return m_edit;
}

bool SettingsScreen::hit(const sf::RectangleShape& r, sf::Vector2f p) const {
    return r.getGlobalBounds().contains(p);
}

void SettingsScreen::centerTextInRect(sf::Text& t, const sf::RectangleShape& r) {
    // proste centrowanie w rect
    auto b = t.getLocalBounds();
    sf::Vector2f pos = r.getPosition();
    sf::Vector2f size = r.getSize();

    t.setPosition({
        pos.x + (size.x - b.size.x) * 0.5f - b.position.x,
        pos.y + (size.y - b.size.y) * 0.5f - b.position.y
        });
}

void SettingsScreen::rebuildLayout(const sf::RenderWindow& window) {
    const auto win = window.getSize();
    const float W = static_cast<float>(win.x);
    const float H = static_cast<float>(win.y);

    const float panelW = std::min(700.f, W * 0.85f);
    const float panelH = std::min(520.f, H * 0.85f);

    const float px = (W - panelW) * 0.5f;
    const float py = (H - panelH) * 0.5f;

    m_panel.setPosition({ px, py });
    m_panel.setSize({ panelW, panelH });

    m_title.setPosition({ px + 24.f, py + 18.f });

    float rowY = py + 90.f;
    const float rowGap = 90.f;

    // Resolution row
    m_resLabel.setPosition({ px + 24.f, rowY });

    m_resPrev.setPosition({ px + 24.f, rowY + 32.f });
    m_resPrev.setSize({ 50.f, 44.f });

    m_resBox.setPosition({ px + 24.f + 60.f, rowY + 32.f });
    m_resBox.setSize({ 260.f, 44.f });

    m_resNext.setPosition({ px + 24.f + 60.f + 270.f, rowY + 32.f });
    m_resNext.setSize({ 50.f, 44.f });

    // Fullscreen row
    rowY += rowGap;

    m_fsBox.setPosition({ px + 24.f, rowY + 32.f });
    m_fsBox.setSize({ 28.f, 28.f });

    m_fsCheck.setPosition(m_fsBox.getPosition() + sf::Vector2f(4.f, 4.f));
    m_fsCheck.setSize({ 20.f, 20.f });

    m_fsText.setPosition({ px + 24.f + 40.f, rowY + 28.f });

    // FPS row
    rowY += rowGap;

    m_fpsLabel.setPosition({ px + 24.f, rowY });

    m_fpsMinus.setPosition({ px + 24.f, rowY + 32.f });
    m_fpsMinus.setSize({ 50.f, 44.f });

    m_fpsBox.setPosition({ px + 24.f + 60.f, rowY + 32.f });
    m_fpsBox.setSize({ 260.f, 44.f });

    m_fpsPlus.setPosition({ px + 24.f + 60.f + 270.f, rowY + 32.f });
    m_fpsPlus.setSize({ 50.f, 44.f });

    // Buttons
    m_applyBtn.setSize({ 160.f, 52.f });
    m_backBtn.setSize({ 160.f, 52.f });

    m_applyBtn.setPosition({ px + panelW - 24.f - 160.f, py + panelH - 24.f - 52.f });
    m_backBtn.setPosition({ px + 24.f,                py + panelH - 24.f - 52.f });

    // teksty przycisków i kontrolki
    centerTextInRect(m_resPrevText, m_resPrev);
    centerTextInRect(m_resNextText, m_resNext);
    centerTextInRect(m_fpsMinusText, m_fpsMinus);
    centerTextInRect(m_fpsPlusText, m_fpsPlus);
    centerTextInRect(m_applyText, m_applyBtn);
    centerTextInRect(m_backText, m_backBtn);
}

void SettingsScreen::draw(sf::RenderWindow& window) {
    if (!m_ready) return;

    rebuildLayout(window);

    // Wartości dynamiczne
    const auto res = m_edit.chosenResolutionOrDesktop();
    m_resValue.setString(AppSettings::presetToString(res));
    centerTextInRect(m_resValue, m_resBox);

    m_fpsValue.setString(std::to_string(m_edit.fpsLimit));
    centerTextInRect(m_fpsValue, m_fpsBox);

    // “disable” resolution UI gdy fullscreen
    const bool resEnabled = !m_edit.fullscreen;
    auto setEnabled = [&](sf::RectangleShape& r, bool enabled) {
        r.setFillColor(enabled ? sf::Color(30, 30, 30, 230) : sf::Color(25, 25, 25, 160));
        };
    setEnabled(m_resPrev, resEnabled);
    setEnabled(m_resNext, resEnabled);

    window.draw(m_panel);
    window.draw(m_title);

    window.draw(m_resLabel);
    window.draw(m_resPrev);
    window.draw(m_resNext);
    window.draw(m_resBox);
    window.draw(m_resPrevText);
    window.draw(m_resNextText);
    window.draw(m_resValue);

    window.draw(m_fsBox);
    if (m_edit.fullscreen) window.draw(m_fsCheck);
    window.draw(m_fsText);

    window.draw(m_fpsLabel);
    window.draw(m_fpsMinus);
    window.draw(m_fpsPlus);
    window.draw(m_fpsBox);
    window.draw(m_fpsMinusText);
    window.draw(m_fpsPlusText);
    window.draw(m_fpsValue);

    window.draw(m_applyBtn);
    window.draw(m_backBtn);
    window.draw(m_applyText);
    window.draw(m_backText);
}

SettingsAction SettingsScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (!m_ready) return SettingsAction::None;

    // klik
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mb->button == sf::Mouse::Button::Left) {
            const sf::Vector2f mousePx(
                static_cast<float>(mb->position.x),
                static_cast<float>(mb->position.y)
            );

            // Back / Apply
            if (hit(m_backBtn, mousePx))  return SettingsAction::Back;
            if (hit(m_applyBtn, mousePx)) return SettingsAction::Apply;

            // Fullscreen toggle
            if (hit(m_fsBox, mousePx)) {
                m_edit.fullscreen = !m_edit.fullscreen;
                return SettingsAction::None;
            }

            // Resolution prev/next (tylko gdy fullscreen off)
            if (!m_edit.fullscreen) {
                const int maxIdx = static_cast<int>(AppSettings::presets().size()) - 1;

                if (hit(m_resPrev, mousePx)) {
                    m_edit.resolutionIndex = std::max(0, m_edit.resolutionIndex - 1);
                }
                if (hit(m_resNext, mousePx)) {
                    m_edit.resolutionIndex = std::min(maxIdx, m_edit.resolutionIndex + 1);
                }
            }

            // FPS minus/plus
            if (hit(m_fpsMinus, mousePx)) {
                if (m_edit.fpsLimit > 30) m_edit.fpsLimit -= 10;
            }
            if (hit(m_fpsPlus, mousePx)) {
                if (m_edit.fpsLimit < 240) m_edit.fpsLimit += 10;
            }
        }
    }

    // skrót: Escape = Back
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->scancode == sf::Keyboard::Scancode::Escape)
            return SettingsAction::Back;
    }

    // żeby UI dobrze wyglądało po resize w menu
    if (event.is<sf::Event::Resized>()) {
        // nic nie musisz robić – draw() i tak liczy layout, ale zostawiamy hook
        (void)window;
    }

    return SettingsAction::None;
}