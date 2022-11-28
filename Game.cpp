#include "Game.hpp"

//Constructors

Game::Game() {
   initVariables();
   initWindow();
}

Game::~Game() {
    delete m_window;

}

//Private

void Game::initVariables() {
    m_window = nullptr;
}

void Game::initWindow() {
    m_videoMode.width = 800;
    m_videoMode.height = 600;
    m_window = new sf::RenderWindow(m_videoMode,"Game",sf::Style::Titlebar | sf::Style::Close);
}

//Accessors
bool Game::isRunning() {
    return m_window->isOpen();
}

//Public

void Game::pollEvent() {
    while (m_window->pollEvent(m_event))
    {
        switch (m_event.type)
        {
        case sf::Event::Closed:
            m_window->close();
            break;
        case sf::Event::KeyPressed:
            if (m_event.key.code == sf::Keyboard::Escape) {
                m_window->close();
            }
            break;
        default:
            break;
        }
           
        
    }
}

void Game::render() {

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Blue);

    m_window->clear(sf::Color::Black);
    m_window->draw(shape);
    m_window->display();
}

void Game::update() {
    pollEvent();
}



