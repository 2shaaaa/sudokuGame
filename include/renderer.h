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
    static const int WINDOW_WIDTH = CELL_SIZE * Sudoku::GRID_SIZE ;
    static const int WINDOW_HEIGHT = CELL_SIZE * Sudoku::GRID_SIZE + 100;

    Renderer();
    ~Renderer();

    // initialize SDL, window, renderer, and font
    bool init();
    // render the whole game
    void render(const Sudoku& sudoku, int selectedRow = -1, int selectedCol = -1);
    // display win game message
    void renderMessage(const std::string& message);
    // convert mouse coordinates to grid position / where mousee points to
    void getGridPosition(int mouseX, int mouseY, int& row, int& col);
    void close();
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;

    void renderGrid();
    void renderNumbers(const Sudoku& sudoku);
    void renderNumber(int number, int row, int col, bool isFixed, bool hasConflict);
    void renderSelectedCell(int row, int col);
    void renderNumberCounts(const Sudoku& sudoku);
    std::array<int, 9> calculateNumberCounts(const Sudoku& sudoku) const;
    static SDL_Texture *cachedBackground;
    static SDL_Texture *resetTexture;

};

#endif