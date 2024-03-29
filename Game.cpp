#include "Game.hpp"
#include <iostream>

const Inter2f Inter2f::NULL_INTER = Inter2f(-1.f,-1.f,-1.f);

//CONSTRUCTOR METHODS

Game::Game() {
    m_settings={800, //window_width
                600, //window_height
                100, //platform_width
                10, //platform_height
                1500, //tick_interval
                40, //grid_x_size
                30, //grid_y_size
                sf::Vector2f(20.f,20.f), //ball_size
                75, //ball_speed
                1, //ball_weight
                10, //explosion weight
                200, //explosion_speed
                200.f, //explosion_radius
                normal_hit, //hit_mode
                normal_bounce, //bounce_mod
                false //practice_mod
    };
    initVariables();
    initWindow();

}

Game::Game(GameSettings settings) {
    m_settings = settings;
    initVariables();
    initWindow();
}

Game::~Game() {
    delete m_window;
    //Free the memory allocated to the grid
    for (int i = 0; i < m_grid_y_size; i++) {
        delete m_grid[i];
    }
    delete m_grid;

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
            else if (m_event.key.code == sf::Keyboard::T) {
                switchBallEffect();
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

    if (!m_gameOver) {
        
        movePlatform(sf::Mouse::getPosition(*m_window).x);  

        for (Explosion &explosion : m_explosions) {
            explosion.update();
            checkExplosionHits(explosion);
            destroyExplosionHits(explosion);
        }

        if (m_explosions.size()) m_explosions.erase(std::remove_if(m_explosions.begin(),m_explosions.end(),
            [](Explosion const& explosion) {       
                return explosion.getRadius() >= explosion.m_explosionRadius;
            }
        ),m_explosions.end());


        moveBall();

        if ((std::clock() - m_timerTick) >= m_settings.tick_interval) { //Tick counter
            //TICK INSTRUCTION
            tick();

            m_timerTick = std::clock();
        }
    }

}

void Game::render() {

    //CLEAR

    m_window->clear(sf::Color::Black);

    //DRAW
    
    

    //Draw grid 
    sf::RectangleShape rect;
    for (int y = 0; y < m_grid_y_size; y++) {
        for (int x = 0; x < m_grid_x_size; x++) {
            if (m_grid[y][x].m_strength <= 0) continue;
            
            rect.setSize(m_brickSize);
            rect.setPosition(sf::Vector2f(m_brickSize.x*(float)x,m_brickSize.y*(float)y));
            if (m_grid[y][x].m_strength == 1) rect.setFillColor(sf::Color::Yellow);
            else if (m_grid[y][x].m_strength == 2) rect.setFillColor(sf::Color::Green);
            else if (m_grid[y][x].m_strength == 3) rect.setFillColor(sf::Color::Cyan);
            else if (m_grid[y][x].m_strength == 4) rect.setFillColor(sf::Color::Blue);
            rect.setOutlineColor(sf::Color::Green);
            rect.setOutlineThickness(-1.f);
            m_window->draw(rect);
        }
    }

    //Draw objects

    for (Explosion const& explosion : m_explosions) {
        m_window->draw(explosion);
    }
    m_window->draw(m_platform); //Draw player platform
    m_window->draw(m_ball); //Draw ball


    //Draw texts

    sf::Font arial;
    arial.loadFromFile("arial.ttf");

    sf::Text score(std::to_string(m_score),arial,30U);
    score.setPosition(10.f,10.f);
    score.setFillColor(sf::Color::Red);
    m_window->draw(score);

    std::string effectText[] = {
        "0 - no effect",
        "1 - unstopabble",
        "2 - heavy",
        "3 - explosive",
        "4 - transparent",
        "5 - inertia"
    };
    sf::Text effect(effectText[m_ball.m_ballEffect],arial,30U);
    effect.setPosition((float)m_videoMode.width-(effect.getGlobalBounds().width+10.f),10.f);
    effect.setFillColor(sf::Color::Red);
    m_window->draw(effect);

    if (m_gameOver) {   
        sf::Text game_over ("GAME OVER",arial, 60U);
        sf::FloatRect textBounds = game_over.getGlobalBounds();
        game_over.setPosition(((float)m_videoMode.width-textBounds.width)/2.f,((float)m_videoMode.height-textBounds.height)/2.f);
        game_over.setFillColor(sf::Color::Red);
        m_window->draw(game_over);
    }
    

    //DISPLAY

    m_window->display();

}

//PRIVATE METHOD

//INIT

void Game::initVariables() {
    m_window = nullptr;

    m_timerFPS = std::clock();
    m_timerTick = std::clock();
    m_timerBall = std::clock();
    m_frameCount = 0;
    m_score = 0;
    m_gameOver = false;
    m_explosionWeight = m_settings.explosion_weight;

    //Set window size
    m_videoMode.width = m_settings.window_width;
    m_videoMode.height = m_settings.window_height;
    m_grid_x_size = m_settings.grid_x_size;
    m_grid_y_size = m_settings.grid_y_size;
    m_brickSize = sf::Vector2f((float)(m_videoMode.width/m_grid_x_size),(float)(m_videoMode.height/m_grid_y_size));

    //Initialize grid and fill it with 0.
    m_grid = new Brick*[m_grid_y_size];
    for (int i=0; i<m_grid_y_size; i++) {
        m_grid[i] = new Brick[m_grid_x_size];
        for (int j = 0; j<m_grid_x_size; j++) {
            m_grid[i][j].m_strength=0;
        }
    }

    //Set player platform parameters
    m_platform.setFillColor(sf::Color::Red);
    m_platform.m_platformSize = sf::Vector2f((float) m_settings.platform_width, (float) m_settings.platform_height);
    m_platform.setSize(m_platform.m_platformSize);
    m_platform.setPosition(sf::Vector2f(0.f,(float)m_videoMode.height-m_platform.m_platformSize.y));

    //Set ball parameters
    m_ball.setFillColor(sf::Color::Magenta);
    m_ball.setSize(m_settings.ball_size);
    m_ball.m_ballSize = m_settings.ball_size;
    m_ball.m_direction=sf::Vector2f((float)-0.5,-1.f);
    m_ball.m_ballWeight=m_settings.ball_weight;
    m_ball.m_ballSpeed=(float)m_settings.ball_speed;
    m_ball.setPosition((float)m_settings.window_width/2.f+m_ball.m_ballSize.x/2.f,m_platform.getPosition().y-m_ball.m_ballSize.y);
    m_ball.m_hittingMode = m_settings.ball_hitting_mode;
    m_ball.m_bouncingMode = m_settings.ball_bouncing_mode;
    m_ball.m_ballEffect = no_effect;
}

void Game::initWindow() {
   
    m_window = new sf::RenderWindow(m_videoMode,"Game",sf::Style::Titlebar | sf::Style::Close);
    //m_window->setFramerateLimit(60);
}

//PLAYER INTERACTION

//Move platform to the x pixel
void Game::movePlatform(int x) {
    if (x>(int)m_videoMode.width-(int)m_platform.m_platformSize.x) x = m_videoMode.width-(int)m_platform.m_platformSize.x; //Prevent platform from leaving window
    else if (x<0) x=0;
    m_platform.setPosition(sf::Vector2f ((float)x,(float)(m_videoMode.height-(int)m_platform.m_platformSize.y)));
}

//Switch the ball to the next ball effect
void Game::switchBallEffect() {
    
    if (m_ball.m_ballEffect+1>5) m_ball.m_ballEffect = no_effect; //Reset effect if at the end of the effect range
    else m_ball.m_ballEffect = (BALL_EFFECT)((int)m_ball.m_ballEffect + 1); //Increment ball effect

    switch (m_ball.m_ballEffect) {
        case no_effect:
            m_ball.m_bouncingMode = normal_bounce;
            m_ball.m_hittingMode = normal_hit;
            break;
        case unstoppable_effect:
            m_ball.m_bouncingMode = unstoppable_bounce;
            m_ball.m_hittingMode = normal_hit;
            break;
        case heavy_effect:
            m_ball.m_bouncingMode = normal_bounce;
            m_ball.m_hittingMode = heavy_hit;
            break;
        case explosive_effect:
            m_ball.m_bouncingMode = inertia_bounce;
            m_ball.m_hittingMode = explosive_hit;
            break;
        case transparent_effect:
            m_ball.m_bouncingMode = unstoppable_bounce;
            m_ball.m_hittingMode = lightweight_hit;
            break;
        case inertia_effect:
            m_ball.m_bouncingMode = inertia_bounce;
            m_ball.m_hittingMode = normal_hit;
            break;
    }


}

//GAME LOGIC

/*
1: Calculate the next position of the ball. 
2 : check collision on cells.
- if collision on cell, move the ball at the colliding position and substract it from the remaining distance
3 : Check brick collision & destroy bricks + bounce
4 : check platform hit and make the ball bounce
5 : if there was a bounce, then we loop again from the start to check for new collision dur to the change of direction
6 : If no collision remaining, move the ball with the remaining distance 
*/
void Game::moveBall() {

    //Calculate ball position
    long time = std::clock();
    long interval = time-m_timerBall;
    interval = interval > 500 ? 500 : interval; //Prevent the ball from going to far and leave the window when restarting after debugging or freeze
    sf::Vector2f distanceFlat(
        (float)interval*m_ball.m_ballSize.x/m_ball.m_ballSpeed,
        (float)interval*m_ball.m_ballSize.y/m_ball.m_ballSpeed
    );
    m_ball.m_position = m_ball.getPosition();
    m_ball.m_distance = distanceFlat;

    //If there is a move.
    if (abs(distanceFlat.x*m_ball.m_direction.x) > 1.f || abs(distanceFlat.y*m_ball.m_direction.y) > 1.f) {
        //While the ball has moved and that distance remains we check collision. 
        //If there is distance to cover left but the ball is not moving, then there is no intersection and we can escape the loop. 
        sf::Vector2f oldPosition(-1.f,-1.f);
        while ((m_ball.m_distance.x > 0.f || m_ball.m_distance.y > 0.f) && m_ball.m_position !=  oldPosition) { //Continue while collision happen
            m_ball.m_bricksHit.clear();           
            oldPosition = m_ball.m_position;

            

            /*
                Check collision on cells
                1 : Check cells collision & update brick hit 
                2 : Destroy bricks
                3 : Handle bounce on walls or bricks
                
            */
            
            //1 : Check cell intersection & update brick hit 

            checkCellCollision();


            // 2 : Destroy every hitted brick

            if (m_ball.m_hittingMode != lightweight_hit) destroyBricks(); 

            // 3 : Change direction from which edge intersected a cell.

            if(ballBounce()) {
                //If the ball bounced and changed direction, then we recheck for any other collision/bounce before checking platform hit
                continue; 
            }
            

            /* 
                Check platform collision 
                1 : Check if the ball hit the platform on its trajectory
                2 : Move the ball to the collision coordinate with old direction
                3 : Change direction of the ball using the angle of collision
            */

            if (checkPlatformCollision()) {
                //Increase speed every hit
                increaseBallSpeed(); 
                    
                //Calculate new position and change direction
                calculatePlatformHit();
            }

            
        }

        //If distance remains (and there is no more collision)
        //Move the ball to the new position
        updateBallPosition(1.f);

        m_timerBall=time;

        m_ball.setPosition(m_ball.m_position);

    }
}


/*
Check if there is a collision with platform. 
Move the ball to the intersection point and update the intersect coordinates in m_platform object.
*/
bool Game::checkPlatformCollision() {

    sf::Vector2f distance(m_ball.m_distance.x*m_ball.m_direction.x,
        m_ball.m_distance.y*m_ball.m_direction.y
    ); 
    sf::Vector2f newPosition(m_ball.m_position.x + distance.x,
        m_ball.m_position.y + distance.y
    );

    sf::Vector2f bottomCorner(m_ball.m_position.x,m_ball.m_position.y+m_ball.m_ballSize.y); //A point
    sf::Vector2f bottomCornerN(newPosition.x,newPosition.y+m_ball.m_ballSize.y); //B point
    sf::Vector2f platform(m_platform.getPosition()); //C point

    if (bottomCornerN.y > platform.y) { //If hit the platform row
        
        float t = findTinter(bottomCorner.y,bottomCornerN.y,platform.y);
        //Coordinates of intersect point 
        Inter2f intersect;
        intersect.x = bottomCorner.x + distance.x * t; 
        intersect.y = bottomCorner.y + distance.y * t;
        
        float minX = platform.x - m_ball.m_ballSize.x; //Minimum x hit
        float maxX = platform.x + m_platform.m_platformSize.x; //Maximum x hit

        if (intersect.x >= minX && intersect.x <= maxX) { //If hit the platform
            
            updateBallPosition(t);
            //Update the distance travelled and switch it to flat distance
            updateBallDistance(t);

            return true;
            
        }
    }
    return false;
}

 /* 
        Calculate angle of collision and the new direction
        xdirection = cosinus of angle
        ydirection = -sin of angle

*/
void Game::calculatePlatformHit(){

    sf::Vector2f platform(m_platform.getPosition()); 
    float minX = platform.x - m_ball.m_ballSize.x; //Minimum x hit
    float maxX = platform.x + m_platform.m_platformSize.x; //Maximum x hit

    //2 : Changing direction
    float midX = (minX + maxX)/2; //x position of a middle hit
    float xDistanceMid = m_ball.m_position.x - midX; //distance to middle hit 
    float cosDir = xDistanceMid/((maxX-minX)/2); //Position of the hit compared with the middle of the platform. From -1 to 1. 
    float sinDir = sin(acos(cosDir));

    m_ball.m_direction.x = roundf(cosDir*100.f)/100.f;
    m_ball.m_direction.y = roundf(-sinDir*100.f)/100.f;

}

/*        
Check collision on cells
//For every edge, check if a new cell was hitted along the trajectory
//Find the first edge(s) to hit some cell. Handle the case where multiple edge are colliding at the exact same time.
//Store the colliding edge in m_ball parameters
//Transform the new position to stop at the hit position. 
//Find any non empty cell colliding with the ball
//Add them to the bricksHit
*/
void Game::checkCellCollision() {
    
    //std::cout<<"Nouvelle position avant check : x = "<<m_ball.m_position.x<<", y = "<<m_ball.m_position.y<<std::endl;

    sf::Vector2f distance(m_ball.m_distance.x*m_ball.m_direction.x,
        m_ball.m_distance.y*m_ball.m_direction.y
    ); 
    sf::Vector2f newPosition(m_ball.m_position.x + distance.x,
        m_ball.m_position.y + distance.y
    );

    //std::cout<<"Position prévue : x = "<<newPosition.x<<", y = "<<newPosition.y<<std::endl;
    
    sf::Vector2f corners[4];
    sf::Vector2f cornersN[4];
    corners[0] = m_ball.m_position;
    corners[1] = sf::Vector2f(m_ball.m_position.x+m_ball.m_ballSize.x,m_ball.m_position.y);
    corners[2] = sf::Vector2f(m_ball.m_position.x+m_ball.m_ballSize.x,m_ball.m_position.y+m_ball.m_ballSize.y);
    corners[3] = sf::Vector2f(m_ball.m_position.x,m_ball.m_position.y+m_ball.m_ballSize.y);
    cornersN[0] = newPosition;
    cornersN[1] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y);
    cornersN[2] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y+m_ball.m_ballSize.y);
    cornersN[3] = sf::Vector2f(newPosition.x,newPosition.y+m_ball.m_ballSize.y);
    std::vector<Inter2f> intersections; //List of intersections between ball and grid cells along the trajectory

    
    int goingDown = m_ball.m_direction.y > 0.f;
    int goingRight = m_ball.m_direction.x > 0.f;
    for (int i = 0; i < 4; i++) {
        if ((goingDown && i == 0) || (!goingDown && i == 2)) continue; //We don't want to check each opposi<te side...
        if ((goingRight && i == 3) || (!goingRight && i == 1)) continue;  //...only the one consistent with the direction 
        float yCD = goingDown || ((corners[i].y - (float)(findGridCoord(corners[i]).y)*m_brickSize.y) != 0.f) ? //If going up and already at the row, then we look for the previous row
            (float)(findGridCoord(corners[i]).y+goingDown)*m_brickSize.y //yCD = current row
            : corners[i].y - m_brickSize.y; //yCD = next row
        float xCD = goingRight || (corners[i].x - (float)findGridCoord(corners[i]).x*m_brickSize.x != 0.f) ? //If going left and already at the col, then we look for the previous col
            (float)(findGridCoord(corners[i]).x+goingRight)*m_brickSize.x  //xCD = current col
            : corners[i].x - m_brickSize.x; //xCD = next col
        //if (x,yCD == 0.f) || (x,yCD > window_size) => Wall collision
        Inter2f inter;
        if (i == 1 || i == 3 ) inter.distance = findTinter(corners[i].x,cornersN[i].x,xCD);
        else if (i == 0 || i == 2 ) inter.distance = findTinter(corners[i].y,cornersN[i].y,yCD);
        inter.edge = i;
        if (inter.distance <= 1.f) intersections.push_back(inter);
        
    }

    bool intersect = false;
    if (intersections.size() > 0) { //If intersection was found
        intersect = true;
        //Calculate exact colliding position with the shortest intersection distance
        std::sort(intersections.begin(),intersections.end());
        
        //This check for multiple collision with cell/wall and switch the colliding edge in the ball object.
        std::vector<Inter2f>::iterator i = intersections.begin();
        bool b;
        do  {
            m_ball.m_edgeHits[i->edge]=true;
            b = i->distance == (i+1)->distance;
            i++;  
                  
        } while (i != intersections.end() && b);


        //Virtually move the ball to the colliding position and return the new position
        Inter2f sInter = *(std::min_element(intersections.begin(),intersections.end()));
        updateBallPosition(sInter.distance);
        
        /*
        std::cout<<"Nouvelle position après intersect : x = "<<m_ball.m_position.x<<", y = "<<m_ball.m_position.y<<std::endl;
        std::cout<<"Distance : x = "<<distance.x<<", "<<distance.y<<std::endl;
        std::cout<<"Direction : x = "<<m_ball.m_direction.x<<", y = "<<m_ball.m_direction.y<<std::endl;
        std::cout<<"t = "<<sInter.distance<<std::endl;
        */

        //Update the distance travelled
        updateBallDistance(sInter.distance);

        if ((m_ball.m_distance.y == 0.f && m_ball.m_distance.x != 0.f) 
            || (m_ball.m_position.x < 0.f) 
            || ((m_ball.m_position.x + m_ball.m_ballSize.x)  > (float)m_videoMode.width )
            || (m_ball.m_position.y < 0.f) 
            || ((m_ball.m_position.y + m_ball.m_ballSize.y) > (float)m_videoMode.height)
            ) {
            std::cout<<"passage"<<std::endl;
        }

    }

    //Find every brick colliding ball

    //Adjacent cell coordinates
    int xbegin = findGridCoord(m_ball.m_position).x; //Grid coordinates of ball 
    int xend = findGridCoord(sf::Vector2f(m_ball.m_position.x+m_ball.m_ballSize.x,newPosition.y)).x;
    int ybegin = findGridCoord(m_ball.m_position).y;
    int yend = findGridCoord(sf::Vector2f(m_ball.m_position.x,m_ball.m_position.y+m_ball.m_ballSize.y)).y;
    
    if (m_ball.m_position.x-((float)findGridCoord(m_ball.m_position).x*m_brickSize.x) == 0.f) xbegin -= 1; //If left edge intersect grid, then we want to look at the previous col
    if (m_ball.m_position.y-((float)findGridCoord(m_ball.m_position).y*m_brickSize.y) == 0.f) ybegin -= 1; //Same for bottom edge

    

    if (intersect) {
        for (int y = ybegin; y <= yend; y++) {
            if (y < 0 || y > m_grid_y_size-1) continue; //If outside col
            int edge = -1; //will remain -1 for bricks inside the ball but not in collision with the edges 
            if (y == ybegin && m_ball.m_edgeHits[0]) edge = 0; 
            else if (y == yend && m_ball.m_edgeHits[2]) edge = 2;

            for (int  x = xbegin; x <= xend; x++) {
                if (x < 0 || x > m_grid_x_size-1) continue; //If outside row
                if (x == xbegin && m_ball.m_edgeHits[3]) edge = 3;
                else if (x == xend && m_ball.m_edgeHits[1]) edge = 1; 
                if (m_grid[y][x].m_strength > 0) {
                        if (std::find_if(m_ball.m_bricksHit.begin(),m_ball.m_bricksHit.end(),[&](BrickHit const& l){return l == BrickHit(x,y);}) == m_ball.m_bricksHit.end()) {//If brick is not already in the vector
                            m_ball.m_bricksHit.push_back(BrickHit(x,y,edge));
                            m_grid[y][x].m_ballInside = true; //Trigger brick_enter event
                        } 
                    }
            }
        }
        std::fill(m_ball.m_edgeHits,m_ball.m_edgeHits+4,false); //Reset the edge hits.
    }

}



//Hit and damage every brick inside bricksHits vector. 
void Game::destroyBricks() {
    
    //Damage bricks
    for (BrickHit& hit : m_ball.m_bricksHit) {
        //If the ball is travelling accros the brick without bouncing, we have to prevent multiple hit.
        if (m_ball.m_bouncingMode == unstoppable_bounce && hit.edge == -1) continue; //If unstoppable do not damage bricks inside the ball
        if (m_ball.m_hittingMode == normal_hit) m_grid[hit.y][hit.x].m_strength -= m_ball.m_ballWeight; //Bricks at the intersection will be the only ones damaged
        else if (m_ball.m_hittingMode == heavy_hit) m_grid[hit.y][hit.x].m_strength = 0;
        else if (m_ball.m_hittingMode == explosive_hit) m_grid[hit.y][hit.x].m_strength -= m_explosionWeight;
        m_score++; //Increment player score
    }

    if (m_ball.m_hittingMode == explosive_hit && m_ball.m_bricksHit.size()) { //If we need to trigger an explosion
        Explosion explosion(m_settings.explosion_radius,m_settings.explosion_speed);
        //We add the bricks colliding the ball in the explosion radius
        std::for_each(m_ball.m_bricksHit.begin(),m_ball.m_bricksHit.end(),
            [&explosion](BrickHit brick) {
                brick.edge=-2; //edge = -2 means that the brick was already hit by the explosion radius 
                explosion.m_bricksHit.push_back(brick);
            }
        );
        explosion.start();
        explosion.setPosition(m_ball.getBallCenter());
        explosion.m_origin = explosion.getPosition();
        m_explosions.push_back(explosion); //Add the explosion to the list of explosions.
        switchBallEffect(); //Only one explosion 
    }
    
    if (m_ball.m_bouncingMode == inertia_bounce) { //If ball has inertia, then the bounce will happen only if a brick has at least 2 strength left
        //Remove every destroyed brick from bricksHit
        
        m_ball.m_bricksHit.erase(
        std::remove_if(m_ball.m_bricksHit.begin(),m_ball.m_bricksHit.end(),
            [&](BrickHit& brick) {
                return (m_grid[brick.y][brick.x].m_strength <= 0);
            }
        
        ),m_ball.m_bricksHit.end());
    } 
    
}

//Check the current ball position for any wall bounce. Check for any colliding edge and change direction. 
bool Game::ballBounce() {

    sf::Vector2f oldDirection = m_ball.m_direction;

    //Check wall collision

    if (m_ball.m_position.y == 0.f && m_ball.m_direction.y < 0.f) m_ball.m_direction.y = -m_ball.m_direction.y; //Top wall collision
    else if (m_ball.m_position.y + m_ball.m_ballSize.y == (float) m_videoMode.height && m_ball.m_direction.y > 0)  {//Bottom wall collision
        if (m_settings.practice_mode) m_ball.m_direction.y = -m_ball.m_direction.y; 
        //GAMEOVER
        else m_gameOver = true;
    }
    if (
        (m_ball.m_position.x == 0.f && m_ball.m_direction.x < 0.f)
        || (m_ball.m_position.x + m_ball.m_ballSize.x == (float) m_videoMode.width && m_ball.m_direction.x > 0)
        ) m_ball.m_direction.x = -m_ball.m_direction.x;

    //Check brick collision
    if (m_ball.m_bouncingMode != unstoppable_bounce) {
        
        bool edgeHit[4] = {0}; 

        //Find edges that are colliding bricks
        for(auto hit : m_ball.m_bricksHit) {
            if (hit.edge != -1) edgeHit[hit.edge]=true; //Then we have to change direction for this edge.
        }

        if (edgeHit[0] || edgeHit[2])  m_ball.m_direction.y = -m_ball.m_direction.y; //TOP
        if (edgeHit[1]|| edgeHit[3] ) m_ball.m_direction.x = -m_ball.m_direction.x; //RIGHT
    }
    //if (edgeHit[2]) m_ball.m_direction.y = -m_ball.m_direction.y; //BOTTOM
    //if (edgeHit[3] ) m_ball.m_direction.x = -m_ball.m_direction.x; //LEFT
    
    if (oldDirection != m_ball.m_direction) return true;
    return false;

}

void Game::increaseBallSpeed() {
    //CHANGE THE REMAINING DISTANCE
    m_ball.m_ballSpeed=m_ball.m_ballSpeed/1; 
}

//Add every non empty cell within the current radius of the exlosion in bricksHits
void Game::checkExplosionHits(Explosion &explosion) {
    int xmin = findGridCoord(explosion.getPosition()).x;
    int xmax = findGridCoord(sf::Vector2f(explosion.getPosition().x+explosion.getRadius()*2.f,explosion.getPosition().y)).x;
    int ymin = findGridCoord(explosion.getPosition()).y;
    int ymax = findGridCoord(explosion.getPosition()+sf::Vector2f(explosion.getRadius()*2,explosion.getRadius()*2)).y;
    //Loop every cell contained in the circle square and check if intersect the circle itself
    for (int x = xmin; x < xmax; x++) {
        if (x < 0 || x >= m_grid_x_size) continue; //Prevent looking outside grid
        for (int y = ymin; y < ymax; y++) {
            if (y < 0 || y >= m_grid_y_size) continue;
            if (std::find_if(explosion.m_bricksHit.begin(),explosion.m_bricksHit.end(),[&x,&y](BrickHit const& l){return l == BrickHit(x,y);}) != explosion.m_bricksHit.end()) 
                continue; //If the cell is already in the hits, we skip it
            if (m_grid[y][x].m_strength<=0) continue;//If cell empty
            //Check the distance between every of the cell and the origin of the circle
            sf::Vector2f corners[4];
            corners[0] = sf::Vector2f(explosion.m_origin-sf::Vector2f((float)x*m_brickSize.x,(float)y*m_brickSize.y));
            corners[1] = sf::Vector2f(explosion.m_origin-sf::Vector2f((float)(x+1)*m_brickSize.x,(float)y*m_brickSize.y));
            corners[2] = sf::Vector2f(explosion.m_origin-sf::Vector2f((float)x*m_brickSize.x,(float)(y+1)*m_brickSize.y));
            corners[3] = sf::Vector2f(explosion.m_origin-sf::Vector2f((float)(x+1)*m_brickSize.x,(float)(y+1)*m_brickSize.y));
            for (auto const& vCorner : corners) {
                if (getFlatDistance(vCorner) <= explosion.getRadius()) { //If distance <= radius then the cell is contained within the circle
                    explosion.m_bricksHit.push_back(BrickHit(x,y,-1));
                    break; 
                }
            }
        }
    }
}

//Hit every brick in Brickhits that have not already been hitted
void Game::destroyExplosionHits(Explosion &explosion) {
    for (BrickHit &hit : explosion.m_bricksHit) {
        if (hit.edge == -2) continue;
        m_grid[hit.y][hit.x].m_strength -= m_settings.explosion_weight;
        hit.edge = -2;
    }
}

//Make each row fall of 1. Remove the last row and add a new one at the top.  
void Game::tick() {

    //Creating random new row
    Brick* newRow = (Brick*) calloc(m_grid_x_size,sizeof(Brick));
    std::mt19937 mt{ static_cast<unsigned int>(std::clock()) };
    for (int x = 0; x < m_grid_x_size; x++) {
        int s = mt()%2; //Strength of the cell
        if (s && (mt()%2) && (mt()%2)) s=2; //0.25 chances of having a 2 strength.
        if (s==2 && (mt()%2) && (mt()%2)) s=4; // 1/16 chances of having a 4 strength.
        newRow[x].m_strength=s;
    }

    //Swaping row
    for (int y = 0; y < m_grid_y_size; y++) {
        Brick* tempY=m_grid[y];
        m_grid[y] = newRow;
        newRow = tempY;
    }

    delete[] newRow; //Free the last line from memory

}

Explosion::Explosion(float explosionRadius, int speedExplosion) : sf::CircleShape::CircleShape(0.f,30U) {
    m_explosionRadius = explosionRadius;
    m_speedExplosion = speedExplosion;
    this->setFillColor(sf::Color(255,128,0));
    
}

//Start explosion. Trigger the clock and set position.
void Explosion::start() {
    m_timer = std::clock();
}

void Explosion::update() {
    
    //Update the clock
    long interval = std::clock() - m_timer;
    //Expand radius
    interval = (int)(interval) > m_speedExplosion ? (long)(m_speedExplosion) : (long)(interval);
    float radius = m_explosionRadius * ((float) interval / (float) m_speedExplosion);
    this->setRadius(radius);
    this->setPosition(m_origin.x-radius,m_origin.y-radius);
    
    



}

//UTILITY METHODS

//Return coordinates of the center of the ball.
sf::Vector2f Ball::getBallCenter() {
    return sf::Vector2f (
        m_position.x+(m_ballSize.x/2.f),
        m_position.y+(m_ballSize.y/2.f)
    );
}

//Return grid coordinates of given point in pixels.
sf::Vector2i Game::findGridCoord(sf::Vector2f coords) {
    int x =-1, y=-1;
    x = (int)(coords.x/m_brickSize.x);
    y = (int)(coords.y/m_brickSize.y);
    return sf::Vector2i(x,y);
}

//Update remaining m_ball flat distance with given t parameters.  
void Game::updateBallDistance(float t) {
    m_ball.m_distance.x= roundf((m_ball.m_distance.x - (m_ball.m_distance.x*t))*1000.f)/1000.f;
    m_ball.m_distance.y= roundf((m_ball.m_distance.y - (m_ball.m_distance.y*t))*1000.f)/1000.f;
}

//Update the ball position using the remaining distance and ball direction. 
//t parameter is between 0 and 1. It defines the proportion of remaining distance to cover.
void Game::updateBallPosition(float t) {
    sf::Vector2f distance = sf::Vector2f(m_ball.m_distance.x*m_ball.m_direction.x,m_ball.m_distance.y*m_ball.m_direction.y);
    m_ball.m_position.x = roundf((m_ball.m_position.x+distance.x*t)*100.f)/100.f;
    m_ball.m_position.y = roundf((m_ball.m_position.y+distance.y*t)*100.f)/100.f;
}

/*
Return the t parameters (distance) to intersection of (AB) with the given coordinates of (CD) 
*/
float Game::findTinter(float A,float B,float C) {
    
    float v = B-A;

    float t = (C-A)/v;
    
    return t;
}

/*
Return an Inter2f object with coordinates of the intersection if it exists.
Return NULL_INTER constant if it no intersection.
Not used in the 2nd approach of movement calculation
*/
Inter2f Game::findInter(sf::Vector2f A,sf::Vector2f B,sf::Vector2f C, sf::Vector2f D) {
    

    //Algorithm taken from https://nguyen.univ-tln.fr/share/GeomAlgo/trans_inter.pdf

    //Cramer determinant
    float det = (B.x-A.x)*(C.y-D.y)-(C.x-D.x)*(B.y-A.y);
    if (det ==0) return Inter2f::NULL_INTER; //If parallel
    
    //Find equation parameters
    float t = ((C.x - A.x)*(C.y - D.y ) - (C.x - D.x )*(C.y - A.y))/det;
    float k = ((B.x - A.x)*(C.y - A.y) - (C.x - A.x)*(B.y - A.y))/det;

    if (t > 1 || t < 0 || k > 1 || k < 0) return Inter2f::NULL_INTER; //If no intersection on segments

    //Intersection coordinates
    Inter2f inter(A.x+t*(B.x-A.x),A.y+t*(B.y-A.y));
    inter.distance = t;

    return inter;
}

//Return the hypothenuse distance of a x,y vector 
float Game::getFlatDistance(sf::Vector2f v) {
    return sqrt(pow(v.x,2.f)+pow(v.y,2.f));   
}

//ACCESSORS METHODS
bool Game::isRunning() {
    return m_window->isOpen();
}



//Overload to compare inter according to their distance 
bool Inter2f::operator<(Inter2f const &b) {
    return (this->distance < b.distance);
}

bool Inter2f::operator>(Inter2f const &b) {
    return (this->distance > b.distance);
}

bool operator==(BrickHit const& l,BrickHit const& r) {
    bool b = l.x == r.x && l.y == r.y;
    return b;
}