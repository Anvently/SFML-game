#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>
#include <ctime>
#include <random>
#include <math.h>
#include <vector>
#include <algorithm>

struct GameSettings {
    int window_width;
    int window_height;
    int platform_width;
    int platform_height;
    int tick_interval; //Tick interval in milliseconds
    int grid_x_size;
    int grid_y_size;
    sf::Vector2f ball_size;
    int ball_speed; // Interval in milliseconds for the ball to make a distance equivalent to its size.
    int ball_weigth;
};

enum BALL_EFFECT {unstoppable, heavy, explosive};

//Utility class to calculate intersection points
class Inter2f : public sf::Vector2f {
    public: 
        using sf::Vector2f::Vector2f;
        Inter2f():x(-0.f),y(-0.f),distance(0.f),corner(-1),edge(-1){};
        inline Inter2f(const float xx, const float yy, const float dis):x(xx),y(yy),distance(dis) {};
        bool operator<(Inter2f const &b);
        bool operator>(Inter2f const &b);   
        const static Inter2f NULL_INTER; 
        float x;
        float y;
        float distance;
    private:
        
        int corner;
        int edge;
        friend class Game;
};



class BrickHit : public sf::Vector2i {
    public : 
        using sf::Vector2i::Vector2i;
        BrickHit(int xx, int yy):x(xx),y(yy){};
        BrickHit():x(-1),y(-1),edge(-1){};
        BrickHit(int xx,int yy, int Edge):x(xx),y(yy),edge(Edge){};
        friend bool operator==(BrickHit const& l,BrickHit const& r);
    private :
        int x;
        int y;
        int edge;
        friend class Game;
};


class Ball : public sf::RectangleShape {
    public:
        using sf::RectangleShape::RectangleShape;
    private: 
        sf::Vector2f m_direction;
        sf::Vector2f m_ballSize;
        int m_ballWeigth;
        float m_ballSpeed;
        sf::Vector2f m_position; //Current position of the ball during movements calculation 
        sf::Vector2f m_distance; //Flat distance remaining to be traveled in a ball move
        std::vector<BrickHit> m_bricksHit; //Vector of BrickHit coordinates of the brick hitted at t time
        bool m_edgeHits[4] = {0};
        friend class Game;
};

class Platform : public sf::RectangleShape {
    public:
        using sf::RectangleShape::RectangleShape;
    private: 
        sf::Vector2f m_platformSize;
        friend class Game;
};

class Brick {
    protected : 
        int m_strength; //Number of remaining hit before the brick is destroyed 
        bool m_ballInside; //Switch to true when the ball enter the brick. Switch to false when the ball leave the brick
        friend class Game;
};

class Game {
    public:
        Game();
        Game(GameSettings settings);
        virtual ~Game();
        void update();
        void render();
        void pollEvent();
        bool isRunning();
        
    private:

        //Init
        void initWindow();
        void initVariables();
        sf::RenderWindow* m_window;
        int m_frameCount;
        sf::Event m_event;
        sf::VideoMode m_videoMode;
        sf::Vector2f m_brickSize;
        int m_grid_x_size;
        int m_grid_y_size;

        //Game object
        Platform m_platform; 
        Brick** m_grid; //The grid is designed as m_grid[y][x] 
        Ball m_ball;

        //Player interaction
        void movePlatform(int x);

        //Game logic
        GameSettings m_settings;
        std::clock_t m_timerFPS;
        std::clock_t m_timerTick;
        std::clock_t m_timerBall;
        void moveBall();
        void tick();
        void checkCellCollision(); 
        void destroyBricks();
        bool ballBounce();
        bool checkPlatformCollision();
        void calculatePlatformHit();
        void triggerBallEffect(enum BALL_EFFECT);
        void changeBallSize(float coeff);
        void increaseBallSpeed();

        //Utility class
        sf::Vector2i findGridCoord(sf::Vector2f coords);
        void updateBallDistance(float t);
        void updateBallPosition(float t);
        Inter2f findInter(sf::Vector2f A,sf::Vector2f B,sf::Vector2f C, sf::Vector2f D);
        Inter2f findInterBis(sf::Vector2f A,sf::Vector2f B,sf::Vector2f C);
        float findTinter(float A,float B,float C);
}; 

#endif