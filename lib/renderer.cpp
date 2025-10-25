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

    if (selectedRow >= 0 && selectedCol >= 0) {
        renderSelectedCell(selectedRow, selectedCol);
    }
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

void Renderer::renderMessage(const std::string& message) {
    SDL_Color color = {0, 128, 0, 255}; // Green color for success message
    SDL_Surface* surface = TTF_RenderText_Blended(font, message.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int textW, textH;
    SDL_QueryTexture(texture, nullptr, nullptr, &textW, &textH);

    SDL_Rect dstRect = {
        (WINDOW_WIDTH - textW) / 2,
        WINDOW_HEIGHT / 2 - textH / 2,
        textW,
        textH
    };

    // Draw semi-transparent background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_Rect bgRect = {0, WINDOW_WIDTH/2 - 30, WINDOW_HEIGHT, 60};
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_RenderPresent(renderer);
}

void Renderer::getGridPosition(int mouseX, int mouseY, int &row, int &col) {
    const int GRID_START_Y = 50;
    row = (mouseY - GRID_START_Y) / CELL_SIZE;
    col = mouseX / CELL_SIZE;

    // Clamp values to be within grid bounds
    if (row < 0) row = 0;
    if (row >= Sudoku::GRID_SIZE) row = Sudoku::GRID_SIZE - 1;
    if (col < 0) col = 0;
    if (col >= Sudoku::GRID_SIZE) col = Sudoku::GRID_SIZE - 1;
}

void Renderer::renderSelectedCell(int row, int col) {
    const int GRID_START_Y = 50;
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;
    const int GRID_START_X = (WINDOW_WIDTH - GRID_PIXELS) / 2;

    SDL_SetRenderDrawColor(renderer, 173, 216, 230, 100); // Light blue with some transparency
    SDL_Rect rowRect = {GRID_START_X, GRID_START_Y + row * CELL_SIZE, GRID_PIXELS, CELL_SIZE};    
    SDL_SetRenderDrawColor(renderer, 173, 216, 230, 100); // Light blue with some transparency
    SDL_Rect colRect = {GRID_START_X +col * CELL_SIZE, GRID_START_Y, CELL_SIZE, GRID_PIXELS};

    // Highlight the 3x3 subgrid with yet another faint blue
    SDL_SetRenderDrawColor(renderer, 225, 238, 255, 255); // Third very light blue
    int subgridStartRow = (row / 3) * 3;
    int subgridStartCol = (col / 3) * 3;
    SDL_Rect subgridRect = {GRID_START_X + subgridStartCol * CELL_SIZE,GRID_START_Y + subgridStartRow * CELL_SIZE, CELL_SIZE * 3, CELL_SIZE * 3};

    SDL_RenderFillRect(renderer, &rowRect);
    SDL_RenderFillRect(renderer, &colRect);
    SDL_RenderFillRect(renderer, &subgridRect);

    // Highlight the selected cell with the original light blue color
    SDL_SetRenderDrawColor(renderer, 173, 216, 230, 255); // Original light blue
    SDL_Rect selectedRect = {GRID_START_X + col * CELL_SIZE, GRID_START_Y + row * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &selectedRect);
}

void Renderer::renderNumberCounts(const Sudoku& sudoku) {
    auto counts = calculateNumberCounts(sudoku);
    
    // Use same grid constants for alignment
    const int GRID_START_Y = 50;
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;
    const int GRID_START_X = (WINDOW_WIDTH - GRID_PIXELS) / 2;
    
    // Position the counter row just below the grid
    const int COUNTER_Y = GRID_START_Y + GRID_PIXELS + 10;
    
    for (int i = 0; i < 9; i++) {
        SDL_Color color = (counts[i] == 9) ? SDL_Color{0, 255, 0, 255} : SDL_Color{0, 0, 0, 255};
        
        // Render main number (1-9) with bold style
        TTF_SetFontStyle(font, TTF_STYLE_BOLD);
        std::string numStr = std::to_string(i + 1);
        SDL_Surface* numSurface = TTF_RenderText_Blended(font, numStr.c_str(), color);
        if (!numSurface) continue;
        
        SDL_Texture* numTexture = SDL_CreateTextureFromSurface(renderer, numSurface);
        if (!numTexture) {
            SDL_FreeSurface(numSurface);
            continue;
        }
        
        // Center number in its cell
        SDL_Rect numRect = {
            GRID_START_X + i * CELL_SIZE + (CELL_SIZE - numSurface->w) / 2,
            COUNTER_Y,
            numSurface->w,
            numSurface->h
        };
        SDL_RenderCopy(renderer, numTexture, nullptr, &numRect);
        
        // Render frequency count as tiny superscript
        if (counts[i] < 9) {
            TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
            std::string countStr = std::to_string(counts[i]);
            SDL_Surface* countSurface = TTF_RenderText_Blended(font, countStr.c_str(), color);
            if (countSurface) {
                SDL_Texture* countTexture = SDL_CreateTextureFromSurface(renderer, countSurface);
                if (countTexture) {
                    // Position and size the superscript
                    SDL_Rect countRect = {
                        numRect.x + numRect.w - 2,        // Slightly overlapping with number
                        numRect.y - numRect.h/4,          // Raised above the baseline
                        static_cast<int>(countSurface->w * 0.5),  // Make it 40% of original size
                        static_cast<int>(countSurface->h * 0.5)
                    };
                    SDL_RenderCopy(renderer, countTexture, nullptr, &countRect);
                    SDL_DestroyTexture(countTexture);
                }
                SDL_FreeSurface(countSurface);
            }
        }
        
        SDL_FreeSurface(numSurface);
        SDL_DestroyTexture(numTexture);
    }
    
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
}

std::array<int, 9> Renderer::calculateNumberCounts(const Sudoku& sudoku) const {
    std::array<int, 9> counts = {0};
    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            int number = sudoku.getNumber(row, col);
            if (number != 0) {
                counts[number - 1]++;
            }
        }
    }
    return counts;
}