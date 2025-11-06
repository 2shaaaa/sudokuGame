#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include "sudoku.h"
#include "renderer.h"

enum class GameState {
    MENU,
    DIFFICULTY,
    PLAYING,
    VICTORY
};

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
    int difficulty; 
    bool running;
    GameState state;
    int selectedRow;
    int selectedCol;
    
    Uint32 startTime;
    int elapsedSeconds;
    static int currentElapsedSeconds;

    bool handleMenuClick(int x, int y);
    void handleEvents();
    void handleMouseClick(int x, int y);
    void handleKeyPress(SDL_Keycode key);
    void checkWinCondition();
    void updateTimer();
};

#endif