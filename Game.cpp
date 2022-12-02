#include "Game.hpp"
#include <iostream>


//CONSTRUCTOR METHODS

Game::Game() {
    m_settings={800, //window_width
                600, //window_height
                80, //platform_width
                10, //platform_height
                1500, //tick_interval
                40, //grid_x_size
                30, //grid_y_size
                sf::Vector2f(40.f,20.f), //ball_size
                100, //ball_speed
                1 //ball_weigth
    };
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
    sf::RectangleShape rect;
    for (int y = 0; y < m_grid_y_size; y++) {
        for (int x = 0; x < m_grid_x_size; x++) {
            if (!m_grid[y][x].m_strength) continue;
            
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
    m_window->draw(m_platform); //Draw player platform
    m_window->draw(m_ball); //Draw ball
    

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
    m_ball.m_direction=sf::Vector2f(0.5,-1.f);
    m_ball.m_ballWeigth=m_settings.ball_weigth;
    m_ball.m_ballSpeed=(float)m_settings.ball_speed;
    m_ball.setPosition((float)m_settings.window_width/2.f+m_ball.m_ballSize.x/2.f,m_platform.getPosition().y-m_ball.m_ballSize.y);
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

//GAME LOGIC

//Calculate the ball position and set a new one. Check collision and change the direction accordingly.
void Game::moveBall() {

    //Calculate ball position
    long time = std::clock();
    long interval = time-m_timerBall;
    interval = interval > 500 ? 500 : interval; //Prevent the ball from going to far and leave the window when restarting after debugging or freeze
    sf::Vector2f distanceFlat(
        (float)interval*m_ball.m_ballSize.x/m_ball.m_ballSpeed,
        (float)interval*m_ball.m_ballSize.y/m_ball.m_ballSpeed
    );
    sf::Vector2f distance(
        distanceFlat.x*m_ball.m_direction.x,
        distanceFlat.y*m_ball.m_direction.y
    );
    sf::Vector2f newPosition(m_ball.getPosition().x + distance.x,m_ball.getPosition().y + distance.y);
    
    /*
        Check collision on bricks
        1 : Find every brick overlapping ball position
        2 : Destroy bricks
        3 : Take one remaining brick and change direction from which border was hitted first
    */
    
    //1 : Find every brick overlapping ball position that have not been entered yet

    std::vector<sf::Vector2i> bricksHit = checkBricksCollision(newPosition);


    // 2 : Destroy every hitted brick

    destroyBricks(bricksHit);

    // 3 : If an hitted brick remains. Then we have to change direction

    newPosition = handleBrickBounce(newPosition, bricksHit);

    
    //Check Wall collision
    newPosition = checkWallCollision(newPosition,distance);

    /* 
        Check platform collision 
        1 : Check if the ball hit the platform on its trajectory
        2 : Move the ball to the collision coordinate with old direction
        3 : Calculate new direction and move the ball with the remaining distance 
    */

    if ((newPosition.y + m_ball.m_ballSize.y)>m_platform.getPosition().y) { //If hit the platform row
        
        // 1 : Check platform hit 
        float minX = m_platform.getPosition().x - m_ball.m_ballSize.x; //Minimum x hit
        float maxX = m_platform.getPosition().x + m_platform.m_platformSize.x; //Maximum x hit
        float distanceYToPlatform = m_platform.getPosition().y-(m_ball.getPosition().y+m_ball.m_ballSize.y);
        float distanceXToPlatfrom = distanceYToPlatform * distance.x / distance.y; //horizontal distance to hit the platform.
        float xPositionHit = m_ball.getPosition().x+distanceXToPlatfrom; 

        if (xPositionHit >= minX && xPositionHit <= maxX) { //If hit the platform
            
            //Increase speed every hit
            increaseBallSpeed();
            
            //Calculate new position and change direction
            newPosition = calculatePlatformHit(newPosition, distance);
        }
    }

    m_timerBall=time;
    m_ball.setPosition(newPosition);


}

/*
        Check collision on walls
        1: Start with side wall 
        2: Check upper wall
        Return the computed new position and change the direction of the ball 
*/
sf::Vector2f Game::checkWallCollision (sf::Vector2f newPosition, sf::Vector2f distance) {
    

    //1 : Check side wall collision
    if (newPosition.x < 0) { //If going through left wall
        newPosition.x = -distance.x - m_ball.getPosition().x; 
        m_ball.m_direction.x=-m_ball.m_direction.x;
    } else if (newPosition.x > (float) m_settings.window_width - m_ball.m_ballSize.x) {//If going through right wall
        float distanceToWall = (float) m_settings.window_width - (m_ball.getPosition().x + m_ball.m_ballSize.x);
        newPosition.x += -distance.x + distanceToWall;
        m_ball.m_direction.x=-m_ball.m_direction.x;
    }

    //2 : Check upper wall collision
    if (newPosition.y < 0) { //If going through upper wall
        newPosition.y = -distance.y - m_ball.getPosition().y; 
        m_ball.m_direction.y=-m_ball.m_direction.y;
    }

    return newPosition;
}

 /* 
        Calculate angle of collision and the new direction
        1: Virtually move the ball to the collision coordinate with old direction
        2: Calculate new direction and move the ball with the remaining distance 
*/
sf::Vector2f Game::calculatePlatformHit(sf::Vector2f newPosition, sf::Vector2f distance){
    
    float minX = m_platform.getPosition().x - m_ball.m_ballSize.x; //Minimum x hit
    float maxX = m_platform.getPosition().x + m_platform.m_platformSize.x; //Maximum x hit
    float distanceYToPlatform = m_platform.getPosition().y-(m_ball.getPosition().y+m_ball.m_ballSize.y);
    float distanceXToPlatfrom = distanceYToPlatform * distance.x / distance.y; //horizontal distance to hit the platform.
    float xPositionHit = m_ball.getPosition().x+distanceXToPlatfrom; 
    
    //1 : Moving the ball the required distance to hit the platform
    newPosition.y = m_ball.getPosition().y + distanceYToPlatform;
    newPosition.x = xPositionHit;
    distance.x -= distanceXToPlatfrom;
    distance.y -=distanceYToPlatform;

    //2 : Changing direction
    float midX = (minX + maxX)/2; //x position of a middle hit
    float xDistanceMid = xPositionHit - midX; //distance to middle hit 
    float cosDir = xDistanceMid/((maxX-minX)/2); //Position of the hit compared with the middle of the platform. From -1 to 1. 
    float sinDir = sin(acos(cosDir));

    m_ball.m_direction.x = cosDir;
    m_ball.m_direction.y = -sinDir;

    //Calculate true position from remaining distance after changed direction
    newPosition.x += abs(distance.x) * m_ball.m_direction.x;
    newPosition.y += abs(distance.y) * m_ball.m_direction.y;

    return newPosition;
}

std::vector<sf::Vector2i> Game::checkBricksCollision(sf::Vector2f newPosition) {
    /*
        Check collision on bricks
        Find every brick overlapping ball position
        Return a vector of coordinates of bricks.
       
    */
    
    //For every edge, check if a new brick was hitted along the trajectory
    //Find the first edge to hit some brick
    //Transform newPosition to stop at the hit position. 
    //Find any non empty brick colliding with the ball
    //Add them to the bricksHit
    
    //Then destroy the bricksHit, if at least one remaining change direction according to the edge colliding
    //Check case where 2 or even 3 edge are colliding brick at the same time. 
    //Than would mean t are equals. 
    //Then we would have to change the direction for every colliding edge
    
    std::vector<sf::Vector2i> bricksHit;

    sf::Vector2f corners[4];
    sf::Vector2f cornersN[4];
    corners[0] = m_ball.getPosition();
    corners[1] = sf::Vector2f(m_ball.getPosition().x+m_ball.m_ballSize.x,m_ball.getPosition().y);
    corners[2] = sf::Vector2f(m_ball.getPosition().x,m_ball.getPosition().y+m_ball.m_ballSize.y);
    corners[3] = sf::Vector2f(m_ball.getPosition().x+m_ball.m_ballSize.x,m_ball.getPosition().y+m_ball.m_ballSize.y);
    cornersN[0] = newPosition;
    cornersN[1] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y);
    cornersN[2] = sf::Vector2f(newPosition.x,newPosition.y+m_ball.m_ballSize.y);
    cornersN[3] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y+m_ball.m_ballSize.y);
    sf::Vector2i v = findGridCoord(cornersN[0])-findGridCoord(corners[0]); //Director vector in grid coordinates
    sf::Vector2f u = cornersN[0] - corners[0];
    std::vector<Inter2f> intersections;

    //If the ball went up a brick, then we want to see the distance between the top edge and the upper row 
    if (v.y < 0) {
        float yCD = (float)(findGridCoord(corners[0]).y-1)*m_brickSize.y;
        if (yCD>0.f) { //If we are not looking outisde the grid
            Inter2f inter;
            inter.distance = findTinter(corners[0].y,cornersN[0].y,yCD);
            intersections.push_back(inter);
        }
    } 
    
    //If the ball went down a brick, ... 
    else if (v.y>0) {
        float yCD = (float)(findGridCoord(corners[2]).y+1)*m_brickSize.y;
        if (yCD<(float)m_videoMode.height) { //If we are not looking outisde the grid
            Inter2f inter;
            inter.distance = findTinter(corners[2].y,cornersN[2].y,yCD);
            inter.edge = 2;
            intersections.push_back(inter);
        }
    }

    //If the ball went right, we want to see the distance between the right edge and the right col 
    else if (v.x>0) {
        float xCD = (float)(findGridCoord(corners[1]).x+1)*m_brickSize.x;
        if (xCD<(float)m_videoMode.width) { //If we are not looking outisde the grid
            Inter2f inter;
            inter.distance = findTinter(corners[1].x,cornersN[1].x,xCD);
            inter.edge = 1;
            intersections.push_back(inter);
        }
    }

    //If the ball went left, ...
    else if (v.x<0) {
        float xCD = (float)(findGridCoord(corners[3]).x-1)*m_brickSize.x;
        if (xCD>0.f) { //If we are not looking outisde the grid
            Inter2f inter;
            inter.distance = findTinter(corners[3].x,cornersN[3].x,xCD);
            inter.edge = 3;
            intersections.push_back(inter);
        }
    }

    //Calculate exact colliding position with the shortest intersection distance
    Inter2f sInter = *(std::min_element(intersections.begin(),intersections.end()));
    newPosition = sf::Vector2f(corners[0].x+u.x*sInter.distance,corners[0].y+u.y*sInter.distance);

    //Find every brick overlapping ball position
    sf::Vector2f corners[4];
    std::vector<sf::Vector2i> bricksHit;
    corners[0] = sf::Vector2f(newPosition.x,newPosition.y);
    corners[1] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y);
    corners[2] = sf::Vector2f(newPosition.x,newPosition.y+m_ball.m_ballSize.y);
    corners[3] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y+m_ball.m_ballSize.y);

    //Looping through every bricks overlapping ball
    for (int y = findGridCoord(corners[0]).y; y <= findGridCoord(corners[2]).y; y+=1) { 
        for (int x = findGridCoord(corners[0]).x; x <= findGridCoord(corners[1]).x; x+=1) {
            if (x >= 0 && x < m_grid_x_size && y >= 0 && y < m_grid_y_size && m_grid[y][x].m_strength) {
                bricksHit.push_back(sf::Vector2i(x,y));
            }
        }
    }

    return bricksHit;
}

void Game::destroyBricks(std::vector<sf::Vector2i> bricks) {
    for (sf::Vector2i& brick : bricks) {
        m_grid[brick.y][brick.x].m_strength -= m_ball.m_ballWeigth;
    }
}

//Take one remaining brick and change direction from which border was hitted first
sf::Vector2f Game::handleBrickBounce(sf::Vector2f newPosition, std::vector<sf::Vector2i> bricks) {

    //Find the first brick remaining. !!!Cons : Always start from the upper-left corner of the ball!!!
    sf::Vector2i brick(-1,-1);
    for(auto cell : bricks) {
        if (m_grid[cell.y][cell.x].m_strength) { //If cell non empty
            brick = cell;
            break;
        } 
    }

    if (brick != sf::Vector2i(-1,-1)) { //If a brick was find

        //The shortest distance between an origin corner and an intersection point determines which edge is touched first.

        std::vector<Inter2f> intersections; //Vector that will contains every intersection point
        sf::Vector2f cornersOrigin[4];
        sf::Vector2f cornersDest[4];
        sf::Vector2f edgeOrigin[4]; //1 : Top - 2 : Right - 3 : Bottom - 4 : Left
        sf::Vector2f edgeDest[4]; 
        cornersOrigin[0] = m_ball.getPosition();
        cornersOrigin[1] = sf::Vector2f(m_ball.getPosition().x+m_ball.m_ballSize.x,m_ball.getPosition().y);
        cornersOrigin[2] = sf::Vector2f(m_ball.getPosition().x,m_ball.getPosition().y+m_ball.m_ballSize.y);
        cornersOrigin[3] = sf::Vector2f(m_ball.getPosition().x+m_ball.m_ballSize.x,m_ball.getPosition().y+m_ball.m_ballSize.y);
        cornersDest[0] = newPosition;
        cornersDest[1] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y);
        cornersDest[2] = sf::Vector2f(newPosition.x,newPosition.y+m_ball.m_ballSize.y);
        cornersDest[3] = sf::Vector2f(newPosition.x+m_ball.m_ballSize.x,newPosition.y+m_ball.m_ballSize.y);
        edgeOrigin[0] = sf::Vector2f((float)brick.x * m_brickSize.x,(float)brick.y * m_brickSize.y);
        edgeDest[0] = sf::Vector2f(((float)brick.x+1.f)* m_brickSize.x,(float)brick.y * m_brickSize.y);
        edgeOrigin[1] = edgeDest[0];
        edgeDest[1] = sf::Vector2f(((float)brick.x+1.f)* m_brickSize.x,((float)brick.y+1.f) * m_brickSize.y);
        edgeOrigin[2] = edgeDest[1];
        edgeDest[2] = sf::Vector2f((float)brick.x* m_brickSize.x,((float)brick.y+1.f) * m_brickSize.y);
        edgeOrigin[3] = edgeDest[2];
        edgeDest[3] = edgeOrigin[1];
        
        //Check every edge
        for (int j = 0; j < 4; j++) { //j for edge
            for (int i = 0; i < 4; i++) { //i for corner
                
                //Algorithm taken from https://nguyen.univ-tln.fr/share/GeomAlgo/trans_inter.pdf

                //Cramer determinant
                float det = (cornersDest[i].x-cornersOrigin[i].x)*(edgeOrigin[j].y-edgeDest[j].y)-(edgeOrigin[j].x-edgeDest[j].x)*(cornersDest[i].y-cornersOrigin[i].y);
                if (det ==0) continue; //If parallel
                
                //Find equation parameters
                float t = ((edgeOrigin[j].x - cornersOrigin[i].x)*(edgeOrigin[j].y - edgeDest[j].y ) - (edgeOrigin[j].x - edgeDest[j].x )*(edgeOrigin[j].y - cornersOrigin[i].y))/det;
                float k = ((cornersDest[i].x - cornersOrigin[i].x)*(edgeOrigin[j].y - cornersOrigin[i].y) - (edgeOrigin[j].x - cornersOrigin[i].x)*(cornersDest[i].y - cornersOrigin[i].y))/det;

                if (t > 1 || t < 0 || k > 1 || k < 0) continue; //If no intersection on segments

                //Intersection coordinates
                Inter2f inter(cornersOrigin[i].x+t*(cornersDest[i].x-cornersOrigin[i].x),cornersOrigin[i].y+t*(cornersDest[i].y-cornersOrigin[i].y));
                inter.corner = i;
                inter.edge = j;
                inter.distance = sqrt(pow(cornersOrigin[i].x-inter.x,2.f) + pow(cornersOrigin[i].y-inter.y,2.f));
                intersections.push_back(inter); //Add intersection to the list of intersections
            }
        }

        //Find shortest distance intersection
        
        //Inter2f sInter; //Shortest intersection
        
        auto sInterIt = std::min_element(intersections.begin(),intersections.end());
        
        //
        if (sInterIt != intersections.end()) {
            Inter2f sInter(*sInterIt);
            //Change direction of the ball
            if (sInter.edge == 0 || sInter.edge == 2) { //If touch top or bottom edge
                m_ball.m_direction.y = -m_ball.m_direction.y;
            }
            else m_ball.m_direction.x = -m_ball.m_direction.x;
        }
    }

    return newPosition;
}

void Game::increaseBallSpeed() {
    m_ball.m_ballSpeed=m_ball.m_ballSpeed/1; 
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

//UTILITY METHODS

//Return grid coordinates of given point in pixels.
sf::Vector2i Game::findGridCoord(sf::Vector2f coords) {
    int x, y;
    x = (int)(coords.x/m_brickSize.x);
    y = (int)(coords.y/m_brickSize.y);
    x = x <= m_grid_x_size ? x : -1;
    y = y <= m_grid_y_size ? y : -1;
    return sf::Vector2i(x,y);
}

Inter2f Game::findInterBis(sf::Vector2f A,sf::Vector2f B,sf::Vector2f C) {
    
    sf::Vector2f v(B-A);

    float tx = (C.x-A.x)/v.x;
    float ty = (C.y-A.y)/v.y;
    float t = std::min(tx,ty);
    Inter2f inter(A.x+t*v.x,A.y+t*v.y);
    inter.distance=t;
    
    return inter;
}

float Game::findTinter(float A,float B,float C) {
    
    float v = B-A;

    float t = (C-A)/v;
    
    return t;
}

/*
Return an Inter2f object with coordinates of the intersection if it exists.
Return NULL_INTER constant if it no intersection.
*/
Inter2f Game::findInter(sf::Vector2f A,sf::Vector2f B,sf::Vector2f C, sf::Vector2f D) {
    
    
    
    
    
    
    //Algorithm taken from https://nguyen.univ-tln.fr/share/GeomAlgo/trans_inter.pdf

    //Cramer determinant
    float det = (B.x-A.x)*(C.y-C.y)-(C.x-C.x)*(B.y-A.y);
    if (det ==0) return Inter2f::NULL_INTER; //If parallel
    
    //Find equation parameters
    float t = ((C.x - A.x)*(C.y - C.y ) - (C.x - C.x )*(C.y - A.y))/det;
    float k = ((B.x - A.x)*(C.y - A.y) - (C.x - A.x)*(B.y - A.y))/det;

    if (t > 1 || t < 0 || k > 1 || k < 0) return Inter2f::NULL_INTER; //If no intersection on segments

    //Intersection coordinates
    Inter2f inter(A.x+t*(B.x-A.x),A.y+t*(B.y-A.y));
    inter.distance = t;

    return inter;
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

    /*
    //If the ball went up a brick, then we want to see the distance between the top edge and the upper row 
    if (v.y < 0) {
        float yCD = (float)(findGridCoord(corners[0]).y-1)*m_brickSize.y;
        if (yCD>0.f) { //If we are not looking outisde the grid
            Inter2f inter = findInter(corners[0],cornersN[0],sf::Vector2f(0.f,yCD),sf::Vector2f((float) m_videoMode.width,yCD));
            inter.edge = 0;
            intersections.push_back(inter);
        }
    } 
    
    //If the ball went down a brick, ... 
    else if (v.y>0) {
        float yCD = (float)(findGridCoord(corners[2]).y+1)*m_brickSize.y;
        if (yCD<(float)m_videoMode.height) { //If we are not looking outisde the grid
            Inter2f inter = findInter(corners[2],cornersN[2],sf::Vector2f(0.f,yCD),sf::Vector2f((float) m_videoMode.width,yCD));
            inter.edge = 2;
            intersections.push_back(inter);
        }
    }

    //If the ball went right, we want to see the distance between the right edge and the right col 
    else if (v.x>0) {
        float xCD = (float)(findGridCoord(corners[1]).x+1)*m_brickSize.x;
        if (xCD<(float)m_videoMode.width) { //If we are not looking outisde the grid
            Inter2f inter = findInter(corners[1],cornersN[1],sf::Vector2f(xCD,0.f),sf::Vector2f(xCD,(float)m_videoMode.height));
            inter.edge = 1;
            intersections.push_back(inter);
        }
    }

    //If the ball went left, ...
    else if (v.x<0) {
        float xCD = (float)(findGridCoord(corners[3]).x-1)*m_brickSize.x;
        if (xCD>0.f) { //If we are not looking outisde the grid
            Inter2f inter = findInter(corners[3],cornersN[3],sf::Vector2f(xCD,0.f),sf::Vector2f(xCD,(float)m_videoMode.height));
            inter.edge = 3;
            intersections.push_back(inter);
        }
    }
    */
