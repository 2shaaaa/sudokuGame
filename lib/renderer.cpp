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
SDL_Texture *Renderer::iconTexture = nullptr;

Renderer::Renderer() : window(nullptr), renderer(nullptr), font(nullptr), icon(nullptr) {}

Renderer::~Renderer() {
    if (cachedBackground)
    {
        SDL_DestroyTexture(cachedBackground);
        cachedBackground = nullptr;
    }
    if (iconTexture)
    {
        SDL_DestroyTexture(iconTexture);
        iconTexture = nullptr;
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

    icon = IMG_Load("/Users/tusha/Documents/GitHub/sudokuGame/image/menu_icon.png");
    if (icon) {
        // create texture once and free the surface
        iconTexture = SDL_CreateTextureFromSurface(renderer, icon);
        SDL_FreeSurface(icon);
        icon = nullptr;
        if (!iconTexture) {
            std::cerr << "Failed to create icon texture: " << SDL_GetError() << std::endl;
        }
    } else {
        // image missing - log but continue without failing
        std::cerr << "Failed to load menu icon: " << IMG_GetError() << std::endl;
        icon = nullptr;
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

void Renderer::getGridPosition(int x, int y, int &row, int &col) {
    const int GRID_START_Y = 50;
    const int GRID_PIXELS = Sudoku::GRID_SIZE * CELL_SIZE;
    const int GRID_START_X = (WINDOW_WIDTH - GRID_PIXELS) / 2;
    row = (y - GRID_START_Y) / CELL_SIZE;
    col = (x - GRID_START_X) / CELL_SIZE;

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

    // Render buttons with hover effect
    SDL_Color btnColor = {99, 108, 203, 255}; // Base blue button
    SDL_Color hoverColor = {79, 129, 255, 255}; // Lighter blue on hover
    const int margin = 200;
    const int buttonWidth = WINDOW_WIDTH - margin * 2;
    const int buttonHeight = 60;
    const int gap = 20;
    const int xPos = margin;
    const int yStart = WINDOW_HEIGHT / 2 - 40;

    // Layout three buttons: Play Again, Main Menu, Exit
    SDL_Rect newGameBtn = { xPos, yStart, buttonWidth, buttonHeight };
    SDL_Rect mainMenuBtn = { xPos, yStart + (buttonHeight + gap), buttonWidth, buttonHeight };
    SDL_Rect exitBtn = { xPos, yStart + 2 * (buttonHeight + gap), buttonWidth, buttonHeight };

    // Get mouse position to determine hover
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    bool hoverNew = (mouseX >= newGameBtn.x && mouseX <= newGameBtn.x + newGameBtn.w && mouseY >= newGameBtn.y && mouseY <= newGameBtn.y + newGameBtn.h);
    bool hoverMain = (mouseX >= mainMenuBtn.x && mouseX <= mainMenuBtn.x + mainMenuBtn.w && mouseY >= mainMenuBtn.y && mouseY <= mainMenuBtn.y + mainMenuBtn.h);
    bool hoverExit = (mouseX >= exitBtn.x && mouseX <= exitBtn.x + exitBtn.w && mouseY >= exitBtn.y && mouseY <= exitBtn.y + exitBtn.h);

    // Draw shadows for buttons
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
    SDL_Rect newShadow = { newGameBtn.x + 3, newGameBtn.y + 3, newGameBtn.w, newGameBtn.h };
    SDL_Rect mainShadow = { mainMenuBtn.x + 3, mainMenuBtn.y + 3, mainMenuBtn.w, mainMenuBtn.h };
    SDL_Rect exitShadow = { exitBtn.x + 3, exitBtn.y + 3, exitBtn.w, exitBtn.h };
    SDL_RenderFillRect(renderer, &newShadow);
    SDL_RenderFillRect(renderer, &mainShadow);
    SDL_RenderFillRect(renderer, &exitShadow);

    // Draw button bodies (slightly lift hovered button)
    SDL_Rect newBody = newGameBtn;
    SDL_Rect mainBody = mainMenuBtn;
    SDL_Rect exitBody = exitBtn;
    if (hoverNew) newBody.y -= 2; // lift up on hover
    if (hoverMain) mainBody.y -= 2;
    if (hoverExit) exitBody.y -= 2;

    SDL_Color fillNew = hoverNew ? hoverColor : btnColor;
    SDL_Color fillMain = hoverMain ? hoverColor : btnColor;
    SDL_Color fillExit = hoverExit ? hoverColor : btnColor;

    SDL_SetRenderDrawColor(renderer, fillNew.r, fillNew.g, fillNew.b, fillNew.a);
    SDL_RenderFillRect(renderer, &newBody);
    SDL_SetRenderDrawColor(renderer, fillMain.r, fillMain.g, fillMain.b, fillMain.a);
    SDL_RenderFillRect(renderer, &mainBody);
    SDL_SetRenderDrawColor(renderer, fillExit.r, fillExit.g, fillExit.b, fillExit.a);
    SDL_RenderFillRect(renderer, &exitBody);

    // Draw borders (highlight when hovered)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120);
    if (hoverNew) SDL_RenderDrawRect(renderer, &newBody);
    if (hoverMain) SDL_RenderDrawRect(renderer, &mainBody);
    if (hoverExit) SDL_RenderDrawRect(renderer, &exitBody);

    // Center text inside buttons and adjust style on hover
    int tw, th;
    std::string newStr = "Play Again";
    TTF_SizeText(font, newStr.c_str(), &tw, &th);
    if (hoverNew) TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText(newStr, newBody.x + (newBody.w - tw) / 2, newBody.y + (newBody.h - th) / 2, {255, 255, 255, 255});
    if (hoverNew) TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    std::string mainStr = "Main Menu";
    TTF_SizeText(font, mainStr.c_str(), &tw, &th);
    if (hoverMain) TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText(mainStr, mainBody.x + (mainBody.w - tw) / 2, mainBody.y + (mainBody.h - th) / 2, {255, 255, 255, 255});
    if (hoverMain) TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    std::string exitStr = "Exit";
    TTF_SizeText(font, exitStr.c_str(), &tw, &th);
    if (hoverExit) TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText(exitStr, exitBody.x + (exitBody.w - tw) / 2, exitBody.y + (exitBody.h - th) / 2, {255, 255, 255, 255});
    if (hoverExit) TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

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
    const int yStart = WINDOW_HEIGHT / 2 - 40;
    SDL_Rect newGameBtn = { xPos, yStart, buttonWidth, buttonHeight };
    SDL_Rect mainMenuBtn = { xPos, yStart + (buttonHeight + gap), buttonWidth, buttonHeight };
    SDL_Rect exitBtn = { xPos, yStart + 2 * (buttonHeight + gap), buttonWidth, buttonHeight };

    // New Game button
    if (x >= newGameBtn.x && x <= newGameBtn.x + newGameBtn.w &&
        y >= newGameBtn.y && y <= newGameBtn.y + newGameBtn.h) {
        return 1;  // New Game clicked
    }

    // Main Menu button
    if (x >= mainMenuBtn.x && x <= mainMenuBtn.x + mainMenuBtn.w &&
        y >= mainMenuBtn.y && y <= mainMenuBtn.y + mainMenuBtn.h) {
        return 2;  // Main Menu clicked
    }

    // Exit button
    if (x >= exitBtn.x && x <= exitBtn.x + exitBtn.w &&
        y >= exitBtn.y && y <= exitBtn.y + exitBtn.h) {
        return 3;  // Exit clicked
    }

    return 0; // No action
}

void Renderer::renderMenuScreen() {
    // Clear the screen with white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Draw a semi-transparent overlay for the menu screen
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 158, 198, 243, 100);
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);

    // Title at top
    TTF_Font* titleFont = TTF_OpenFont("/System/Library/Fonts/Supplemental/Comic Sans MS.ttf", 72);
    TTF_SetFontStyle(titleFont, TTF_STYLE_BOLD);
    // Use the same title color as the victory screen for visual consistency
    SDL_Color titleColor = {99, 108, 203, 255}; // Victory-screen purple-blue
    
    // Render SUDOKU title with larger font
    SDL_Surface* surface = TTF_RenderText_Blended(titleFont, "sUdOkU", titleColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect destRect;
            TTF_SizeText(titleFont, "sUdOkU", &destRect.w, &destRect.h);
            destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
            destRect.y = WINDOW_HEIGHT / 4 - destRect.h;
            SDL_RenderCopy(renderer, texture, NULL, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
        }
        TTF_CloseFont(titleFont);

            // Set font style and size for the subtitle
    TTF_Font* subtitleFont = TTF_OpenFont("/System/Library/Fonts/Supplemental/Comic Sans MS.ttf", 16);
    TTF_SetFontStyle(subtitleFont, TTF_STYLE_ITALIC);
    SDL_Color subtitleColor = {128, 128, 128, 255}; // Gray color for subtitle
    
    // Render subtitle with smaller font
    surface = TTF_RenderText_Blended(subtitleFont, "Made by TuSha", subtitleColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect destRect;
            TTF_SizeText(subtitleFont, "Made by TuSha", &destRect.w, &destRect.h);
            destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
            destRect.y = WINDOW_HEIGHT / 4 - destRect.h / 2;
            SDL_RenderCopy(renderer, texture, NULL, &destRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
    TTF_CloseFont(subtitleFont);

    // Reset font style for the button
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    // Game icon in center
    const int IMG_SIZE = 350;
    int imgX = WINDOW_WIDTH/2 - IMG_SIZE/2;
    int imgY = WINDOW_HEIGHT/2 - IMG_SIZE/2 - 20;
    if (iconTexture) {
        SDL_Rect dst = { imgX, imgY, IMG_SIZE, IMG_SIZE };
        SDL_RenderCopy(renderer, iconTexture, NULL, &dst);
    }

    // Render start button with the same style as victory screen buttons
    SDL_Rect startBtn = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 150, 200, 40};

    // Get mouse state for hover effect
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    bool isHovered = (mouseX >= startBtn.x && mouseX <= startBtn.x + startBtn.w &&
                     mouseY >= startBtn.y && mouseY <= startBtn.y + startBtn.h);
    bool isClicked = isHovered && (mouseState & SDL_BUTTON_LMASK);

    // Button shadow (same as victory screen)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
    SDL_Rect btnShadow = {startBtn.x + 3, startBtn.y + 3, startBtn.w, startBtn.h};
    SDL_RenderFillRect(renderer, &btnShadow);

    // Button body with interaction effects (lift on hover)
    SDL_Rect buttonRect = startBtn;
    if (isHovered) buttonRect.y -= 2; // same gentle lift as victory

    // Colors matching victory screen
    SDL_Color btnColor = {99, 108, 203, 255}; // base
    SDL_Color hoverColor = {79, 129, 255, 255}; // hover
    SDL_Color fill = isHovered ? hoverColor : btnColor;

    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &buttonRect);

    // Button border: show a light highlight when hovered
    if (isHovered) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 120);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 60);
    }
    SDL_RenderDrawRect(renderer, &buttonRect);

    // Button text (bold when hovered)
    SDL_Color white = {255, 255, 255, 255};
    if (isHovered) TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText("Start Game",
              buttonRect.x + (buttonRect.w - 100) / 2,
              buttonRect.y + (buttonRect.h - 24) / 2,
              white);
    if (isHovered) TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    SDL_RenderPresent(renderer);
}

bool Renderer::handleMenuClick(int x, int y) {
    SDL_Rect startBtn = {WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 150, 200, 40};
    return (x >= startBtn.x && x <= startBtn.x + startBtn.w &&
            y >= startBtn.y && y <= startBtn.y + startBtn.h);
}

void Renderer::renderDifficultyScreen() {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 158, 198, 243, 100);
    SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);

    // Title
    TTF_Font* titleFont = TTF_OpenFont("/System/Library/Fonts/Supplemental/Comic Sans MS.ttf", 48);
    SDL_Color titleColor = {0, 0, 0, 255};
    if (titleFont) {
        SDL_Surface* surface = TTF_RenderText_Blended(titleFont, "Select Difficulty", titleColor);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect destRect;
                TTF_SizeText(titleFont, "Select Difficulty", &destRect.w, &destRect.h);
                destRect.x = WINDOW_WIDTH / 2 - destRect.w / 2;
                destRect.y = WINDOW_HEIGHT / 4 - destRect.h / 2;
                SDL_RenderCopy(renderer, texture, NULL, &destRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
        TTF_CloseFont(titleFont);
    }

    // Three difficulty buttons
    const int btnW = 220;
    const int btnH = 50;
    const int gap = 20;
    const int totalH = btnH * 3 + gap * 2;
    const int startY = WINDOW_HEIGHT / 2 - totalH / 2;
    const int xPos = WINDOW_WIDTH / 2 - btnW / 2;

    SDL_Rect easyBtn = { xPos, startY, btnW, btnH };
    SDL_Rect medBtn = { xPos, startY + btnH + gap, btnW, btnH };
    SDL_Rect hardBtn = { xPos, startY + 2*(btnH + gap), btnW, btnH };

    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    bool hoverEasy = (mouseX >= easyBtn.x && mouseX <= easyBtn.x + easyBtn.w && mouseY >= easyBtn.y && mouseY <= easyBtn.y + easyBtn.h);
    bool hoverMed = (mouseX >= medBtn.x && mouseX <= medBtn.x + medBtn.w && mouseY >= medBtn.y && mouseY <= medBtn.y + medBtn.h);
    bool hoverHard = (mouseX >= hardBtn.x && mouseX <= hardBtn.x + hardBtn.w && mouseY >= hardBtn.y && mouseY <= hardBtn.y + hardBtn.h);

    // Shadows
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60);
    SDL_Rect s1 = { easyBtn.x + 3, easyBtn.y + 3, easyBtn.w, easyBtn.h };
    SDL_Rect s2 = { medBtn.x + 3, medBtn.y + 3, medBtn.w, medBtn.h };
    SDL_Rect s3 = { hardBtn.x + 3, hardBtn.y + 3, hardBtn.w, hardBtn.h };
    SDL_RenderFillRect(renderer, &s1);
    SDL_RenderFillRect(renderer, &s2);
    SDL_RenderFillRect(renderer, &s3);

    SDL_Color base = {99, 108, 203, 255};
    SDL_Color hover = {79, 129, 255, 255};

    SDL_Rect b1 = easyBtn, b2 = medBtn, b3 = hardBtn;
    if (hoverEasy) b1.y -= 2;
    if (hoverMed) b2.y -= 2;
    if (hoverHard) b3.y -= 2;

    SDL_SetRenderDrawColor(renderer, (hoverEasy?hover.r:base.r), (hoverEasy?hover.g:base.g), (hoverEasy?hover.b:base.b), 255);
    SDL_RenderFillRect(renderer, &b1);
    SDL_SetRenderDrawColor(renderer, (hoverMed?hover.r:base.r), (hoverMed?hover.g:base.g), (hoverMed?hover.b:base.b), 255);
    SDL_RenderFillRect(renderer, &b2);
    SDL_SetRenderDrawColor(renderer, (hoverHard?hover.r:base.r), (hoverHard?hover.g:base.g), (hoverHard?hover.b:base.b), 255);
    SDL_RenderFillRect(renderer, &b3);

    // Button text
    TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    renderText("Easy", b1.x + (b1.w/2) - 24, b1.y + (b1.h/2) - 12, {255,255,255,255});
    renderText("Medium", b2.x + (b2.w/2) - 36, b2.y + (b2.h/2) - 12, {255,255,255,255});
    renderText("Hard", b3.x + (b3.w/2) - 24, b3.y + (b3.h/2) - 12, {255,255,255,255});
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    SDL_RenderPresent(renderer);
}

int Renderer::handleDifficultyClick(int x, int y) {
    const int btnW = 220;
    const int btnH = 50;
    const int gap = 20;
    const int totalH = btnH * 3 + gap * 2;
    const int startY = WINDOW_HEIGHT / 2 - totalH / 2;
    const int xPos = WINDOW_WIDTH / 2 - btnW / 2;

    SDL_Rect easyBtn = { xPos, startY, btnW, btnH };
    SDL_Rect medBtn = { xPos, startY + btnH + gap, btnW, btnH };
    SDL_Rect hardBtn = { xPos, startY + 2*(btnH + gap), btnW, btnH };

    if (x >= easyBtn.x && x <= easyBtn.x + easyBtn.w && y >= easyBtn.y && y <= easyBtn.y + easyBtn.h) return 1;
    if (x >= medBtn.x && x <= medBtn.x + medBtn.w && y >= medBtn.y && y <= medBtn.y + medBtn.h) return 2;
    if (x >= hardBtn.x && x <= hardBtn.x + hardBtn.w && y >= hardBtn.y && y <= hardBtn.y + hardBtn.h) return 3;
    return 0;
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