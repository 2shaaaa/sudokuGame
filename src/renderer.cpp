#include "renderer.h"
#include <stdexcept>
#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <SDL_image.h>


SDL_Texture* Renderer::cachedBackground = nullptr;  // Define static member
SDL_Texture *Renderer::resetTexture = nullptr;

Renderer::Renderer() : window(nullptr), renderer(nullptr), font(nullptr) {}

Renderer::~Renderer() {
    if (cachedBackground)
    {
        SDL_DestroyTexture(cachedBackground);
        cachedBackground = nullptr;
    }
    if (resetTexture)
    {
        SDL_DestroyTexture(resetTexture);
        resetTexture = nullptr;
    }
    close();
}

bool Renderer::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    window = SDL_CreateWindow("sUdOkU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    if(!IMG_Init(IMG_INIT_PNG)) {
        std::cerr << "IMG_Init failed: " << IMG_GetError() << std::endl;
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}

void Renderer::close() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

void Renderer::render(const Sudoku& sudoku, int selectedRow, int selectedCol) {
    // Clear screen with white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Draw top padding
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect topPadding = {0, 0, WINDOW_WIDTH, 50};
    SDL_RenderFillRect(renderer, &topPadding);

    // Set color for grid
    SDL_SetRenderDrawColor(renderer, 110, 140, 251, 255);
    renderGrid();
    renderNumbers(sudoku);

    // Present the final render
    SDL_RenderPresent(renderer);
}

void Renderer::renderGrid() {
    SDL_SetRenderDrawColor(renderer, 56, 87, 246, 255);

	// Move grid down by 50 pixels to accommodate score display
	const int GRID_START_Y = 50;

	// Use gridSize defined at top; ensure 9 cells fit exactly
	const int GRID_PIXELS = 9 * CELL_SIZE;

	// Center grid horizontally
	const int GRID_START_X = (WINDOW_WIDTH - GRID_PIXELS) / 2;

	// Draw horizontal lines
	for (int i = 0; i <= 9; i++) {
		int lineWidth = (i % 3 == 0) ? 3 : 1;
		int y = GRID_START_Y + i * CELL_SIZE;
		SDL_Rect rect = {GRID_START_X, y - lineWidth/2, GRID_PIXELS, lineWidth};
		SDL_RenderFillRect(renderer, &rect);
	}

	// Draw vertical lines
	for (int i = 0; i <= 9; i++) {
		int lineWidth = (i % 3 == 0) ? 3 : 1;
		int x = GRID_START_X + i * CELL_SIZE;
		SDL_Rect rect = {x - lineWidth/2, GRID_START_Y, lineWidth, GRID_PIXELS};
		SDL_RenderFillRect(renderer, &rect);
	}
}

void Renderer::renderNumbers(const Sudoku& sudoku) {
    SDL_Color fixedColor = {0, 0, 0, 255};
    SDL_Color userColor = {0, 0, 255, 255};

    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            int number = sudoku.getNumber(row, col);
            if (number != 0) {
                bool isFixed = !sudoku.isCellEditable(row, col);
                renderNumber(number, row, col, isFixed);
            }
        }
    }
}

void Renderer::renderNumber(int number, int row, int col, bool isFixed) {
    SDL_Color color = isFixed ? SDL_Color{0, 0, 0, 255} : SDL_Color{0, 0, 255, 255};

    std::string text = std::to_string(number);
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);

    // Calculate the grid offset for proper positioning
    const int GRID_START_Y = 50;
    const int GRID_START_X = (WINDOW_WIDTH - (9 * CELL_SIZE)) / 2;

    SDL_Rect dstRect = {
        GRID_START_X + col * CELL_SIZE + (CELL_SIZE - textW) / 2,
        GRID_START_Y + row * CELL_SIZE + (CELL_SIZE - textH) / 2,
        textW,
        textH
    };

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}
