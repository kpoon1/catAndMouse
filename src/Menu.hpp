
#pragma once
#include "SFML/Graphics.hpp"
#include <SFML/Audio.hpp>

static const int Number = 3;

class Menu

{
public:
    Menu(float width, float height);
    ~Menu();
    
    void draw(sf::RenderWindow &window);
    void MoveUp();
    void MoveDown();
    void PlayMusic();
    void background();
    void Title(float width, float height);
    int selectedindex(){return selectedItemIndex;}
    
    
    
private:
    int selectedItemIndex = 0;
    sf::Font font;
    sf::Text menu[Number];
    sf::Text title;
    sf::Texture texture;

};


