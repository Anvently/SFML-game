#include "Game.hpp"
#include <iostream>
#include <ctime>
#include <random>

//CONSTRUCTOR METHODS

Game::Game() {
    m_settings={800,600,80,20,1500,40,30,sf::Vector2f(200.f,200.f),1000};
    initVariables();
    initWindow();

}

Game::Game(GameSettings settings) {
    Game();
    m_settings = settings;
}

Game::~Game() {
    delete m_window;
    //Free the memory allocated to the grid
    for (int i = 0; i < m_settings.grid_y_size; i++) {
        delete m_grid[i];
    }
    delete m_grid;

}

//PRIVATE METHOD

void Game::initVariables() {
    m_window = nullptr;

    m_timerFPS = std::clock();
    m_timerTick = std::clock();
    m_timerBall = std::clock();
    m_frameCount = 0;

    m_videoMode.width = m_settings.window_width;
    m_videoMode.height = m_settings.window_height;

    //Initialize grid and fill it with 0.
    m_grid = new int*[m_settings.grid_y_size];
    for (int i=0; i<m_settings.grid_y_size; i++) {
        m_grid[i] = new int[m_settings.grid_x_size];
        for (int j = 0; j<m_settings.grid_x_size; j++) {
            m_grid[i][j]=0;
        }
    }

    //Set player platform parameters
    m_platform.setFillColor(sf::Color::Red);
    m_platform.setSize(sf::Vector2f ((float) m_settings.platform_width, (float) m_settings.platform_height));
    m_platform.setPosition(sf::Vector2f(0.f,(float)(m_videoMode.height-m_settings.platform_height)));

    //Set ball parameters
    m_ball.setFillColor(sf::Color::Yellow);
    m_ball.setSize(m_settings.ball_size);
    m_ball.m_direction=sf::Vector2f(0.5,-1.f);
    
    m_ball.setPosition((float)m_settings.window_width/2.f+m_settings.ball_size.x/2.f,m_platform.getPosition().y-m_settings.ball_size.y);
}

void Game::initWindow() {
   
    m_window = new sf::RenderWindow(m_videoMode,"Game",sf::Style::Titlebar | sf::Style::Close);
    //m_window->setFramerateLimit(144);
}

//Move platform to the x pixel
void Game::movePlatform(int x) {
    if (x>(int)m_videoMode.width-m_settings.platform_width) x = m_videoMode.width-m_settings.platform_width; //Prevent platform from leaving window
    else if (x<0) x=0;
    m_platform.setPosition(sf::Vector2f ((float)x,(float)(m_videoMode.height-m_settings.platform_height)));
}

//Calculate the ball position and set a new one. Check collision and change the direction accordingly.
void Game::moveBall() {
    //Calculate ball position
    long time = std::clock();
    long interval = time-m_timerBall;
    sf::Vector2f distanceFlat(
        (float)interval*m_settings.ball_size.x/(float)m_settings.ball_speed,
        (float)interval*m_settings.ball_size.y/(float)m_settings.ball_speed
    );
    sf::Vector2f distance(
        distanceFlat.x*m_ball.m_direction.x,
        distanceFlat.y*m_ball.m_direction.y
    );
    sf::Vector2f newPosition(m_ball.getPosition().x + distance.x,m_ball.getPosition().y + distance.y);
    
    //Check collision on walls

    if (newPosition.x < 0) { //If going through left wall
        newPosition.x = -distance.x - m_ball.getPosition().x; 
        m_ball.m_direction.x=-m_ball.m_direction.x;
    } else if (newPosition.x > (float) m_settings.window_width - m_settings.ball_size.x) {//If going through right wall
        float distanceToWall = (float) m_settings.window_width - (m_ball.getPosition().x + m_settings.ball_size.x);
        newPosition.x += -distance.x + distanceToWall;
        m_ball.m_direction.x=-m_ball.m_direction.x;
    }

    if (newPosition.y < 0) { //If going through upper wall
        newPosition.y = -distance.y - m_ball.getPosition().y; 
        m_ball.m_direction.y=-m_ball.m_direction.y;
    } else if ((newPosition.y + m_settings.ball_size.y)>m_platform.getPosition().y) { //If hit the platform
        float distanceToPlatform = m_platform.getPosition().y-(m_ball.getPosition().y+m_settings.ball_size.y);
        newPosition.y += -distance.y + distanceToPlatform;
        m_ball.m_direction.y=-m_ball.m_direction.y;
    }

    m_timerBall=time;
    m_ball.setPosition(newPosition);


}

//Make each row fall of 1. Remove the last row and add a new one at the top.  
void Game::tick() {

    //Creating random new row
    int* newRow = (int*) calloc(m_settings.grid_x_size,sizeof(int));
    std::mt19937 mt{ static_cast<unsigned int>(std::clock()) };
    for (int x = 0; x < m_settings.grid_x_size; x++) {
        newRow[x] = mt()%2;
    }

    //Swaping row
    for (int y = 0; y < m_settings.grid_y_size; y++) {
        int* tempY=m_grid[y];
        m_grid[y] = newRow;
        newRow = tempY;
    }

    delete newRow; //Free the last line from memory

}

//ACCESSORS METHODS
bool Game::isRunning() {
    return m_window->isOpen();
}

//PUBLIC METHODS

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


void Game::update() {
    
    pollEvent();

    m_frameCount++;
    if ((std::clock() - m_timerFPS) / CLOCKS_PER_SEC >= 1) { //FPS Counter
        std::cout<<m_frameCount<<" FPS"<<std::endl; 
        m_frameCount=0, m_timerFPS = std::clock();
    }

    movePlatform(sf::Mouse::getPosition(*m_window).x);  
    moveBall();

    if ((std::clock() - m_timerTick) >= m_settings.tick_interval) { //Tick counter
        //TICK INSTRUCTION
        tick();

        m_timerTick = std::clock();
    }


}

void Game::render() {

    //CLEAR

    m_window->clear(sf::Color::Black);

    //DRAW
    
    

    //Draw grid 
    float size_x = (float) (m_settings.window_width/m_settings.grid_x_size);
    float size_y = (float) (m_settings.window_height/m_settings.grid_y_size);
    sf::RectangleShape rect;
    for (int y = 0; y < m_settings.grid_y_size; y++) {
        for (int x = 0; x < m_settings.grid_x_size; x++) {
            if (!m_grid[y][x]) continue;
            
            rect.setSize(sf::Vector2f(size_x,size_y));
            rect.setPosition(sf::Vector2f(size_x*(float)x,size_y*(float)y));
            rect.setFillColor(sf::Color::Cyan);
            rect.setOutlineColor(sf::Color::Green);
            rect.setOutlineThickness(-1.f);
            m_window->draw(rect);
        }
    }
    m_window->draw(m_platform); //Draw player platform
    m_window->draw(m_ball); //Draw ball
    

    //DISPLAY

    m_window->display();

}



