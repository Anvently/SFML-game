# Brick breaker SFML-game

An introduction project to SFML and and more generally graphic program using C++.

## Objective

The challenge was to meet a few specification :
- not use any intersect function of SFML and manage our own  geometric calculation
- make every move frame independant (the movement distance are calculated from elllapsed time between two frame)
- have a realistic movement calculation, make sure the ball travel each frame the proper distance despite of the bounces.
- be able to easily modify the size of every object in the game and have it working on any size configuration : window size, cell-size and number of cell, platform size, ball size and ball shape (any rectangle)
- be able to trigger different ball effect changing the way the ball hit the brick and the way the ball bounce, including an explosion effect.

## Issues

- the biggest  issue is due to float rounding inprecision ending up with division by zero, causing the game to freeze. They specifically occure when using odd number dimension on game objects dimension.

## Usage

- You can set game settings using GameSettings object, see class declaration in game.hpp for how to use them
- Press T to switch effect and ESCAPE to close the game. 


