#pragma once

#include "API.h"
#include <SFML/Graphics.hpp>

// Typy budynków
enum class BuildingType { TownHall, Quarry };

class EXPORT_API Building {
public:
    // Konstruktor od razu przyjmuje pozycję i typ
    Building(sf::Vector2f position, BuildingType type);
    ~Building();
    Building(const Building& other); // Konstruktor kopiujący dla DLL

    void update(float dt);
    void draw(sf::RenderWindow& window);

private:
    sf::Vector2f m_position;
    sf::RectangleShape m_shape; // Budynki zróbmy kwadratowe, żeby odróżnić je od jednostek
    BuildingType m_type;
    int m_health;
};