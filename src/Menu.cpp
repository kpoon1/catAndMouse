

#include "Menu.hpp"
#include <SFML/Audio.hpp>

Menu::Menu(float width, float height)
{
    font.loadFromFile("img/GoodDog.otf");
    
    menu[0].setFont(font);
    menu[0].setString("Alien");
    menu[0].setCharacterSize(90);
    menu[0].setColor(sf::Color::Cyan);
    menu[0].setPosition(width/2.6, height/4);
    
    menu[1].setFont(font);
    menu[1].setString("Marine");
    menu[1].setCharacterSize(70);
    menu[1].setColor(sf::Color::Yellow);
    menu[1].setPosition(width/2.6, height/4*1.7 );
    
}

Menu::~Menu()
{
    
}

void Menu::draw(sf::RenderWindow &window)
{
    for (int i = 0; i < Number; i++)
    {
        window.draw(menu[i]);
    }
    window.draw(title);
}


void Menu::MoveUp()
{
        menu[0].setCharacterSize(90);
        selectedItemIndex = 0;
        menu[1].setCharacterSize(70);
}


void Menu::MoveDown()
{
        menu[0].setCharacterSize(70);
        selectedItemIndex = 1;
        menu[1].setCharacterSize(90);
}

void Menu::background(){
    sf::Texture texture;
    texture.loadFromFile("img/cute_image.jpg");
    sf::Sprite sprite(texture);
}















