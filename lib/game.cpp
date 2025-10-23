#include "game.h"
#include "renderer.h"
#include <SDL2/SDL.h>

Game::Game() : running(false), selectedRow(-1), selectedCol(-1) {
}

Game::~Game() {}

bool Game::init() {
    if (!renderer.init()) {
        return false;
    }
    running = true;
    return true;
}

void Game::run() {
    SDL_Event e;
    while (running) {
        handleEvents();
        renderer.render(sudoku, selectedRow, selectedCol);
        SDL_Delay(16); // Cap at ~60 FPS
    }
    renderer.close();
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseClick(event.button.x, event.button.y);
                }
                break;
            default:
                break;
        }
    }
}

void Game::handleMouseClick(int x, int y) {
    renderer.getGridPosition(x, y, selectedRow, selectedCol);
    if(!sudoku.isCellEditable(selectedRow, selectedCol)) {
        selectedRow = selectedCol = -1;
    }
}
