#include "Game.hpp"
#include <iostream>

using namespace std;

int main()
{
    
    //Init Game
    Game game;

    
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