#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>

class Game {
    public:
        Game();
        virtual ~Game();
        void update();
        void render();
        void pollEvent();
        bool isRunning();

    private:
        sf::RenderWindow* m_window;
        sf::Event m_event;
        sf::VideoMode m_videoMode;
        void initWindow();
        void initVariables();
}; 

#endif