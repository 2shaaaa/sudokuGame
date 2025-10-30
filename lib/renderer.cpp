#include "renderer.h"
#include "game.h"
#include <stdexcept>
#include <array>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
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

    font = TTF_OpenFont("/System/Library/Fonts/Supplemental/Chalkboard.ttc", 24); //Comic Sans MS
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

    // Draw padding
    SDL_SetRenderDrawColor(renderer, 203, 220, 235, 255);
    SDL_Rect topPadding = {0, 0, WINDOW_WIDTH, 50};
    SDL_RenderFillRect(renderer, &topPadding);

    SDL_SetRenderDrawColor(renderer, 203, 220, 235, 255);
    SDL_Rect leftPadding = {0, 0, 50, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &leftPadding);

    SDL_SetRenderDrawColor(renderer, 203, 220, 235, 255);
    SDL_Rect bottomPadding = {0, WINDOW_HEIGHT - 50, WINDOW_WIDTH, 50};
    SDL_RenderFillRect(renderer, &bottomPadding);

    SDL_SetRenderDrawColor(renderer, 203, 220, 235, 255);
    SDL_Rect rightPadding = {WINDOW_WIDTH - 50, 0, 50, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &rightPadding);

    renderTimer(Game::getElapsedSeconds());

    if (selectedRow >= 0 && selectedCol >= 0) {
        renderSelectedCell(selectedRow, selectedCol);
    }

    renderGrid();
    renderNumbers(sudoku);
    renderNumberCounts(sudoku);

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
    for (int row = 0; row < Sudoku::GRID_SIZE; row++) {
        for (int col = 0; col < Sudoku::GRID_SIZE; col++) {
            int number = sudoku.getNumber(row, col);
            if (number != 0) {
                bool isFixed = !sudoku.isCellEditable(row, col);
                bool hasConflict = sudoku.hasConflict(row, col);
                renderNumber(number, row, col, isFixed, hasConflict);
            }
        }
    }
}

void Renderer::renderNumber(int number, int row, int col, bool isFixed, bool hasConflict) {
    SDL_Color color;
    if (hasConflict) {
        color = {255, 0, 0, 255}; // Red for conflicting numbers
    } else {
        color = isFixed ? SDL_Color{0, 0, 0, 255} : SDL_Color{0, 0, 255, 255}; // Black for fixed, Blue for user
    }

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

void Renderer::getGridPosition(int mouseX, int mouseY, int &row, int &col) {
    const int GRID_START_Y = 50;
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;
    const int GRID_START_X = (WINDOW_WIDTH - GRID_PIXELS) / 2;
    row = (mouseY - GRID_START_Y) / CELL_SIZE;
    col = (mouseX - GRID_START_X) / CELL_SIZE;

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

    SDL_Rect rowRect = {GRID_START_X, GRID_START_Y + row * CELL_SIZE, GRID_PIXELS, CELL_SIZE};    
    SDL_Rect colRect = {GRID_START_X +col * CELL_SIZE, GRID_START_Y, CELL_SIZE, GRID_PIXELS};

    // Highlight the 3x3 subgrid with yet another faint blue
    SDL_SetRenderDrawColor(renderer, 210, 233, 253, 255); // Third very light blue
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
        SDL_Color color = (counts[i] == 9) ? SDL_Color{56, 87, 246, 255} : SDL_Color{0, 0, 0, 255};
        
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

void Renderer::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    SDL_Rect dstRect;
    TTF_SizeText(font, text.c_str(), &dstRect.w, &dstRect.h);
    dstRect.x = x;
    dstRect.y = y;
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

void Renderer::renderTimer(int elapsedSeconds) {
    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    std::ostringstream ss;
    ss << "Time: " << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;

    SDL_Color color = {0, 0, 0, 255}; // Black color for timer
    renderText(ss.str(), 20, 10, color);
}

void Renderer::renderVictoryScreen(int elapsedSeconds) {
    // Clear screen with white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Draw a semi-transparent overlay for the victory screen
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 158, 198, 243, 100);
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);

    // Render victory message (centered) and elapsed time just below it
    std::string msg = "Congratulations! You solved the puzzle!";
    int msgW, msgH;
    TTF_SizeText(font, msg.c_str(), &msgW, &msgH);
    renderText(msg, (WINDOW_WIDTH - msgW) / 2, (WINDOW_HEIGHT - msgH) / 3, {99, 108, 203, 255});

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;
    std::ostringstream ss;
    ss << "Your time: " << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds;
    std::string timeStr = ss.str();
    int timeW, timeH;
    TTF_SizeText(font, timeStr.c_str(), &timeW, &timeH);
    renderText(timeStr, (WINDOW_WIDTH - timeW) / 2, (WINDOW_HEIGHT + timeH) / 3 + 5, {255, 255, 255, 255});

    //Render buttons
    SDL_Color btnColor = {99, 108, 203, 255}; // BLue button
    const int margin = 200;
    const int buttonWidth = WINDOW_WIDTH - margin * 2;
    const int buttonHeight = 60;
    const int gap = 20;
    const int xPos = margin;
    const int yStart = WINDOW_HEIGHT / 2 + 20;

    SDL_Rect newGameBtn = { xPos, yStart, buttonWidth, buttonHeight };
    SDL_Rect exitBtn = { xPos, yStart + buttonHeight + gap, buttonWidth, buttonHeight };

    SDL_SetRenderDrawColor(renderer, btnColor.r, btnColor.g, btnColor.b, btnColor.a);
    SDL_RenderFillRect(renderer, &newGameBtn);
    SDL_RenderFillRect(renderer, &exitBtn);

    // Center text inside buttons
    int tw, th;
    std::string newStr = "Play Again";
    TTF_SizeText(font, newStr.c_str(), &tw, &th);
    renderText(newStr, newGameBtn.x + (newGameBtn.w - tw) / 2, newGameBtn.y + (newGameBtn.h - th) / 2, {255, 255, 255, 255});

    std::string exitStr = "Exit";
    TTF_SizeText(font, exitStr.c_str(), &tw, &th);
    renderText(exitStr, exitBtn.x + (exitBtn.w - tw) / 2, exitBtn.y + (exitBtn.h - th) / 2, {255, 255, 255, 255});

    // Present once
    SDL_RenderPresent(renderer);
}

int Renderer::handleVictoryScreenClick(int x, int y) {
    // Match the button positions used in renderVictoryScreen (stacked)
    const int margin = 200;
    const int buttonWidth = WINDOW_WIDTH - margin * 2;
    const int buttonHeight = 60;
    const int gap = 20;
    const int xPos = margin;
    const int yStart = WINDOW_HEIGHT / 2 + 20;
    SDL_Rect newGameBtn = { xPos, yStart, buttonWidth, buttonHeight };
    SDL_Rect exitBtn = { xPos, yStart + buttonHeight + gap, buttonWidth, buttonHeight };

    // New Game button
    if (x >= newGameBtn.x && x <= newGameBtn.x + newGameBtn.w &&
        y >= newGameBtn.y && y <= newGameBtn.y + newGameBtn.h) {
        return 1;  // New Game clicked
    }

    // Exit button
    if (x >= exitBtn.x && x <= exitBtn.x + exitBtn.w &&
        y >= exitBtn.y && y <= exitBtn.y + exitBtn.h) {
        return 2;  // Exit clicked
    }

    return 0; // No action
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

void Renderer::completeEffect(const Sudoku& sudoku, int originRow, int originCol, int durationMs) {
    const int GRID_START_Y = 50;
    const int GRID_PIXELS = 9 * CELL_SIZE;
    const int GRID_START_X = (WINDOW_WIDTH - GRID_PIXELS) / 2;

    // origin center in pixels
    const double originCx = GRID_START_X + originCol * CELL_SIZE + CELL_SIZE / 2.0;
    const double originCy = GRID_START_Y + originRow * CELL_SIZE + CELL_SIZE / 2.0;

    // maximum radius to cover grid corners
    double maxDx = std::max(originCx - GRID_START_X, GRID_START_X + GRID_PIXELS - originCx);
    double maxDy = std::max(originCy - GRID_START_Y, GRID_START_Y + GRID_PIXELS - originCy);
    double maxRadius = std::sqrt(maxDx * maxDx + maxDy * maxDy) + 1.0;

    const int frameDelayMs = 16;
    Uint32 start = SDL_GetTicks();
    while (true) {
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - start;
        if (elapsed > (Uint32)durationMs) break;

        // Draw base frame (same as render without presenting)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw paddings
        SDL_SetRenderDrawColor(renderer, 203, 220, 235, 255);
        SDL_Rect topPadding = {0, 0, WINDOW_WIDTH, 50}; SDL_RenderFillRect(renderer, &topPadding);
        SDL_Rect leftPadding = {0, 0, 50, WINDOW_HEIGHT}; SDL_RenderFillRect(renderer, &leftPadding);
        SDL_Rect bottomPadding = {0, WINDOW_HEIGHT - 50, WINDOW_WIDTH, 50}; SDL_RenderFillRect(renderer, &bottomPadding);
        SDL_Rect rightPadding = {WINDOW_WIDTH - 50, 0, 50, WINDOW_HEIGHT}; SDL_RenderFillRect(renderer, &rightPadding);

        renderTimer(Game::getElapsedSeconds());
        renderGrid();
        renderNumbers(sudoku);
        renderNumberCounts(sudoku);

        // compute ripple radius (linear from 0..maxRadius)
        double progress = static_cast<double>(elapsed) / static_cast<double>(durationMs);
        double radius = progress * maxRadius;
        const double thickness = 40.0; // thickness of the ring in pixels

        // For each cell compute center distance and draw overlay if within ring
        for (int row = 0; row < Sudoku::GRID_SIZE; ++row) {
            for (int col = 0; col < Sudoku::GRID_SIZE; ++col) {
                double cx = GRID_START_X + col * CELL_SIZE + CELL_SIZE / 2.0;
                double cy = GRID_START_Y + row * CELL_SIZE + CELL_SIZE / 2.0;
                double dx = cx - originCx;
                double dy = cy - originCy;
                double d = std::sqrt(dx*dx + dy*dy);

                double diff = std::abs(d - radius);
                double intensity = 0.0;
                if (diff <= thickness) {
                    intensity = 1.0 - (diff / thickness);
                }
                if (intensity > 0.0) {
                    Uint8 alpha = static_cast<Uint8>(80 + intensity * 175); // 80..255
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 203, 220, 235, alpha);
                    SDL_Rect cellRect = { GRID_START_X + col * CELL_SIZE, GRID_START_Y + row * CELL_SIZE, CELL_SIZE, CELL_SIZE };
                    SDL_RenderFillRect(renderer, &cellRect);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(frameDelayMs);
    }
}