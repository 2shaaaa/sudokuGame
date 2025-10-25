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
            case SDL_KEYDOWN:
                handleKeyPress(event.key.keysym.sym);
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

void Game::handleKeyPress(SDL_Keycode key) {
    if (selectedRow == -1 || selectedCol == -1) return;

    if (key >= SDLK_1 && key <= SDLK_9) {
        int num = key - SDLK_0;
        if (sudoku.setNumber(selectedRow, selectedCol, num)) {
            checkWinCondition();
        }
    } else if (key == SDLK_BACKSPACE || key == SDLK_DELETE || key == SDLK_0) {
        sudoku.setNumber(selectedRow, selectedCol, 0);
    } else if (key == SDLK_r) {
        sudoku.generatePuzzle();
        selectedRow = selectedCol = -1;
    }
}