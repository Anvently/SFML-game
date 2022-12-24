#include "Game.hpp"
#include <iostream>
#include <ctime>


using namespace std;

int main()
{
    //Init Game
    
    
    GameSettings settings={800, //window_width
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

    Game game(settings);

    cout<<"Hello world"<<endl;

    //Game loop

    while (game.isRunning())
    {
        
        //Update
        game.update();

        //Render
        game.render();
        
    }

    return 0;
}