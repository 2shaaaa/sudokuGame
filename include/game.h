#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "sudoku.h"
#include "renderer.h"

class Game {
public:
    Game();
    ~Game();

    bool init();
    void run();
    static int getElapsedSeconds() { return currentElapsedSeconds; }

private: 
    Renderer renderer;
    Sudoku sudoku;
    bool running;
    int selectedRow;
    int selectedCol;

    Uint32 startTime;
    int elapsedSeconds;
    static int currentElapsedSeconds;

    void handleEvents();
    void handleMouseClick(int x, int y);
    void handleKeyPress(SDL_Keycode key);
    void checkWinCondition();
    void updateTimer();
};

#endif