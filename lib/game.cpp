#include "game.h"
#include "renderer.h"
#include <SDL2/SDL.h>

int Game::currentElapsedSeconds = 0;

Game::Game() : running(false), state(GameState::MENU), selectedRow(-1), selectedCol(-1), elapsedSeconds(0), startTime(0) {
}

Game::~Game() {}

bool Game::init() {
    if (!renderer.init()) {
        return false;
    }
    running = true;
    startTime = SDL_GetTicks();
    return true;
}

void Game::run() {
    while (running) {
        handleEvents();
        if (state == GameState::PLAYING) {
            updateTimer();
        }
        // If a win was detected during event handling, stop before drawing
        if (running) {
            if (state == GameState::MENU) {
                renderer.renderMenuScreen();
            } else if (state == GameState::PLAYING) {
                renderer.render(sudoku, selectedRow, selectedCol);
            }
            SDL_Delay(16); // Cap at ~60 FPS
        }
    }
    renderer.close();
}

bool Game::handleMenuClick(int x, int y) {
    if (renderer.handleMenuClick(x, y)) {
        state = GameState::PLAYING;
        startTime = SDL_GetTicks();
        return true;
    }
    return false;
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
    if (state == GameState::MENU) {
        handleMenuClick(x, y);
        return;
    }
    
    int newRow, newCol;
    renderer.getGridPosition(x, y, newRow, newCol);
    // Toggle selection if clicking the same cell
    if (newRow == selectedRow && newCol == selectedCol) {
        selectedRow = selectedCol = -1;
        return;
    }
    if(sudoku.isCellEditable(newRow, newCol)) {
        selectedRow = newRow;
        selectedCol = newCol;
    } else {
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

void Game::checkWinCondition() {
    if (sudoku.isSolved()) {
        if (selectedRow >= 0 && selectedCol >= 0) {
            renderer.completeEffect(sudoku, selectedRow, selectedCol, 1500);
        } else {
            // fallback center ripple
            renderer.completeEffect(sudoku, 4, 4, 1000);
        }

        int action = 0;
        SDL_Event event;
        bool shouldClose = false;

        while(action == 0 && !shouldClose) {
            while(SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        shouldClose = true;
                        running = false;
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            action = renderer.handleVictoryScreenClick(event.button.x, event.button.y);
                        }
                        break;
                    default:
                        break;
                }
            }
            if (!shouldClose) {
                renderer.renderVictoryScreen(elapsedSeconds);
                SDL_Delay(16);
            }
        }
        if (action == 1) {  // New Game
            sudoku = Sudoku();
            selectedRow = selectedCol = -1;
            startTime = SDL_GetTicks();
            elapsedSeconds = 0;
            currentElapsedSeconds = 0;
        } else if (action == 2) {  // Main Menu
            state = GameState::MENU;
            sudoku = Sudoku();
            selectedRow = selectedCol = -1;
            startTime = SDL_GetTicks();
            elapsedSeconds = 0;
            currentElapsedSeconds = 0;
        } else if (action == 3) {
            shouldClose = true;
            running = false;
        }
    }
}

void Game::updateTimer() {
    if (running) {
        Uint32 currentTime = SDL_GetTicks();
        elapsedSeconds = (currentTime - startTime) / 1000;
        currentElapsedSeconds = elapsedSeconds;
    }
}