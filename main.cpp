#include "Game.hpp"
#include <iostream>
#include <ctime>


using namespace std;

int main()
{
    //Init Game
    Game game;

    float f1 = roundf((float)39.9999*100.f)/100.f;
    float f2 = roundf((float)51.0689*100000.f)/100000.f;
    cout<<f1<<" "<<f2<<endl;
    
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