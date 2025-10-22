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
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        } 
    }
}
