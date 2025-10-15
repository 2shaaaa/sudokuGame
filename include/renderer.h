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
    static const int WINDOW_WIDTH = CELL_SIZE * 9;
    static const int WINDOW_HEIGHT = CELL_SIZE * 9 + 50;

    Renderer();
    ~Renderer();

    bool init();
    void close();
    void renderGrid();
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    static SDL_Texture *cachedBackground;
    static SDL_Texture *resetTexture;


    // void renderNumbers(const Sudoku& sudoku);
    // void renderSelectedCell(int row, int col);
    // void renderNumber(int number, int row, int col, bool isFixed);
};

#endif