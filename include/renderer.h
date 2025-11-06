#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include "sudoku.h"

class Renderer {
public:
    static const int CELL_SIZE = 60;
    static const int WINDOW_WIDTH = CELL_SIZE * Sudoku::GRID_SIZE + 100;
    static const int WINDOW_HEIGHT = CELL_SIZE * Sudoku::GRID_SIZE + 100;

    Renderer();
    ~Renderer();

    bool init();
    void render(const Sudoku& sudoku, int selectedRow = -1, int selectedCol = -1);
    void getGridPosition(int x, int y, int& row, int& col);     // convert mouse coordinates to grid position / where mousee points to
    void close();
    void renderTimer(int elapsedSeconds);
    void renderVictoryScreen(int elapsedSeconds);
    void renderMenuScreen();
    void renderDifficultyScreen();
    int handleDifficultyClick(int x, int y);
    void completeEffect(const Sudoku& sudoku, int originRow, int originCol, int durationMs = 1200);
    int handleVictoryScreenClick(int x, int y);
    bool handleMenuClick(int x, int y);
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Surface* icon;

    void renderGrid();
    void renderNumbers(const Sudoku& sudoku);
    void renderNumber(int number, int row, int col, bool isFixed, bool hasConflict);
    void renderSelectedCell(int row, int col);
    void renderNumberCounts(const Sudoku& sudoku);
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    std::array<int, 9> calculateNumberCounts(const Sudoku& sudoku) const;

    static SDL_Texture *iconTexture;

};

#endif