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
        bool operator<(Inter2f const &b);
        bool operator>(Inter2f const &b);
    private:
        float distance;
        int corner;
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
        friend class Game;
};

class Platform : public sf::RectangleShape {
    public:
        using sf::RectangleShape::RectangleShape;
    private: 
        sf::Vector2f m_platformSize;
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

        //Game object
        Platform m_platform; 
        int** m_grid; //The grid is designed as m_grid[y][x]
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
        std::vector<sf::Vector2i> checkBricksCollision(sf::Vector2f newPosition); 
        void destroyBricks(std::vector<sf::Vector2i> bricks);
        sf::Vector2f handleBrickBounce(sf::Vector2f newPosition, std::vector<sf::Vector2i> bricks);
        sf::Vector2f checkWallCollision(sf::Vector2f newPosition, sf::Vector2f distance);
        sf::Vector2f calculatePlatformHit(sf::Vector2f newPosition, sf::Vector2f distance);
        void triggerBallEffect(enum BALL_EFFECT);
        void changeBallSize(float coeff);
        void increaseBallSpeed();

        //Utility class
        sf::Vector2i findGridCoord(sf::Vector2f coords);
 
}; 

#endif