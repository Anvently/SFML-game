#include "Game.hpp"
#include <iostream>
#include <ctime>
#include <random>
#include <math.h>
#include <vector>
#include <algorithm>

//CONSTRUCTOR METHODS

Game::Game() {
    m_settings={800,600,80,20,1500,40,30,sf::Vector2f(100.f,100.f),500};
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
    
    /*
        Check collision on bricks
        1 : Find every brick overlaying ball position
        2 : Destroy bricks
        3 : Take one brick and change direction from which border was hitted first
    */
    
    // 1 : Find every brick overlaying ball position
    sf::Vector2f corners[4];
    std::vector<sf::Vector2i> bricksHit;
    corners[0] = sf::Vector2f(newPosition.x,newPosition.y);
    corners[1] = sf::Vector2f(newPosition.x+m_settings.ball_size.x,newPosition.y);
    corners[2] = sf::Vector2f(newPosition.x,newPosition.y+m_settings.ball_size.y);
    corners[3] = sf::Vector2f(newPosition.x+m_settings.ball_size.x,newPosition.y+m_settings.ball_size.y);
    sf::Vector2i coord;
    for (int i = 0; i < 4; i++) {
        coord = findGridCoord(corners[i]);
        //Check if corner inside grid, if brick exist and if brick isn't already included 
        if (coord.x >= 0 && coord.y >= 0 && m_grid[coord.y][coord.x] && std::find(bricksHit.begin(),bricksHit.end(),coord) == bricksHit.end()) bricksHit.push_back(coord); 
    }

    // 2 : Destroy every brick overlaying the ball
    for (sf::Vector2i brick : bricksHit) {
        m_grid[brick.y][brick.x] = 0;
    }


    /*
        Check collision on walls
        1: Start with side wall 
        2: Check upper wall
    */

    //1 : Check side wall collision
    if (newPosition.x < 0) { //If going through left wall
        newPosition.x = -distance.x - m_ball.getPosition().x; 
        m_ball.m_direction.x=-m_ball.m_direction.x;
    } else if (newPosition.x > (float) m_settings.window_width - m_settings.ball_size.x) {//If going through right wall
        float distanceToWall = (float) m_settings.window_width - (m_ball.getPosition().x + m_settings.ball_size.x);
        newPosition.x += -distance.x + distanceToWall;
        m_ball.m_direction.x=-m_ball.m_direction.x;
    }

    //2 : Check upper wall collision
    if (newPosition.y < 0) { //If going through upper wall
        newPosition.y = -distance.y - m_ball.getPosition().y; 
        m_ball.m_direction.y=-m_ball.m_direction.y;

    /* 
        Check platform collision 
        1 : Check if the ball hit the platform on its trajectory
        2 : Move the ball to the collision coordinate with old direction
        3 : Calculate new direction and move the ball with the remaining distance 
    */

    } else if ((newPosition.y + m_settings.ball_size.y)>m_platform.getPosition().y) { //If hit the platform row
        
        // 1 : Check platform hit 
        float minX = m_platform.getPosition().x - m_settings.ball_size.x; //Minimum x hit
        float maxX = m_platform.getPosition().x + (float) m_settings.platform_width; //Maximum x hit
        float distanceYToPlatform = m_platform.getPosition().y-(m_ball.getPosition().y+m_settings.ball_size.y);
        float distanceXToPlatfrom = distanceYToPlatform * distance.x / distance.y; //horizontal distance to hit the platform.
        float xPositionHit = m_ball.getPosition().x+distanceXToPlatfrom; 

        if (xPositionHit >= minX && xPositionHit <= maxX) { //If hit the platform
            
            //Increase speed every hit
            m_settings.ball_speed=int((float)m_settings.ball_speed/1); 
            
            //2 : Moving the ball the required distance to hit the platform
            newPosition.y = m_ball.getPosition().y + distanceYToPlatform;
            newPosition.x = xPositionHit;
            distance.x -= distanceXToPlatfrom;
            distance.y -=distanceYToPlatform;

            //3 : Changing direction
            float midX = (minX + maxX)/2; //x position of a middle hit
            float xDistanceMid = xPositionHit - midX; //distance to middle hit 
            float cosDir = xDistanceMid/((maxX-minX)/2); //Position of the hit compared with the middle of the platform. From -1 to 1. 
            float sinDir = sin(acos(cosDir));

            m_ball.m_direction.x = cosDir;
            m_ball.m_direction.y = -sinDir;

            //Calculate true position from remaining distance after changed direction
            newPosition.x += abs(distance.x) * m_ball.m_direction.x;
            newPosition.y += abs(distance.y) * m_ball.m_direction.y;
        }
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

//UTILITY METHODS

//Return grid coordinates of given point in pixels.
sf::Vector2i Game::findGridCoord(sf::Vector2f coords) {
    int x, y;
    x = (int)(coords.x/(float)((float)m_settings.window_width/(float)m_settings.grid_x_size));
    y = (int)(coords.y/(float)((float)m_settings.window_height/(float)m_settings.grid_y_size));
    x = x <= m_settings.grid_x_size ? x : -1;
    y = y <= m_settings.grid_y_size ? y : -1;
    return sf::Vector2i(x,y);
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




