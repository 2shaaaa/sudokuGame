#ifndef GAME_H
#define GAME_H

#include "sudoku.h"
#include "renderer.h"

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();

private: 
    Renderer renderer;
    bool running;
    int selectedRow;
    int selectedCol;

};

#endif