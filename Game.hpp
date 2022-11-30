#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Graphics.hpp>

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
};

class Ball : public sf::RectangleShape {
    public:
        using sf::RectangleShape::RectangleShape;
    private: 
        sf::Vector2f m_direction;
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
        sf::RenderWindow* m_window;
        sf::Event m_event;
        sf::VideoMode m_videoMode;
        sf::RectangleShape m_platform; 
        int** m_grid; //The grid is designed as m_grid[y][x]
        Ball m_ball;
        GameSettings m_settings;
        std::clock_t m_timerFPS;
        std::clock_t m_timerTick;
        std::clock_t m_timerBall;
        int m_frameCount;
        void initWindow();
        void initVariables();
        void movePlatform(int x);
        void moveBall();
        void tick();
}; 

#endif