#include "sudoku.h"
#include <iostream>

Sudoku::Sudoku() : grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
                   fixed(GRID_SIZE, std::vector<bool>(GRID_SIZE, false)) {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed for randomness
    generatePuzzle(2); // default to Medium
}

void Sudoku::generatePuzzle(int difficulty) {
    // Start with an empty grid
    for(auto& row : grid) {
        std::fill(row.begin(), row.end(), 0);
    }
    for(auto& row : fixed) {
        std::fill(row.begin(), row.end(), false);
    }

    // Fill diagonal subgrids
    for (int box = 0; box < GRID_SIZE; box += SUBGRID_SIZE) {
        // Generate random permutation of numbers 1-9
        std::vector<int> nums(GRID_SIZE);
        std::iota(nums.begin(), nums.end(), 1);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(nums.begin(), nums.end(), gen);
        
        for (int i = 0; i < SUBGRID_SIZE; i++) {
            for (int j = 0; j < SUBGRID_SIZE; j++) {
                grid[box + i][box + j] = nums[i * SUBGRID_SIZE + j];
            }
        }
    }

    solveGrid();

    // Mark all cells as fixed
    for(auto& row : fixed) {
        std::fill(row.begin(), row.end(), true);
    }

    // Determine how many cells to remove based on difficulty
    int cellsToRemove = 0;
    if (difficulty <= 1) {
        // Easy: fewer removals (more clues)
        cellsToRemove = 1; //36 + (std::rand() % 9); // 36..44
    } else if (difficulty == 2) {
        // Medium: previous behavior
        cellsToRemove = 2; //45 + (std::rand() % 11); // 45..55
    } else {
        // Hard: more removals (fewer clues)
        cellsToRemove = 3; //56 + (std::rand() % 5); // 56..60
    }

    removeCells(cellsToRemove);
}

bool Sudoku::isValid(int row, int col, int num) const {
    return isValidInRow(row, num) && 
           isValidInCol(col, num) && 
           isValidInBox(row - row % SUBGRID_SIZE, col - col % SUBGRID_SIZE, num);
}

bool Sudoku::isCellEditable(int row, int col) const {
    return !fixed[row][col];
}

bool Sudoku::setNumber(int row, int col, int num) {
    if(!isCellEditable(row, col)) {
        return false;
    }
    
    // Allow any number (0-9) to be entered
    if (num >= 0 && num <= 9) {
        grid[row][col] = num;
        return true;
    }
    
    return false;
}

int Sudoku::getNumber(int row, int col) const {
    return grid[row][col];
}

bool Sudoku::isSolved() const {
    // Check if all cells are filled
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] == 0) {
                return false;
            }
        }
    }

    // Check each row
    for (int row = 0; row < GRID_SIZE; row++) {
        std::vector<bool> seen(GRID_SIZE + 1, false);
        for (int col = 0; col < GRID_SIZE; col++) {
            int num = grid[row][col];
            if (seen[num]) return false;
            seen[num] = true;
        }
    }

    // Check each column
    for (int col = 0; col < GRID_SIZE; col++) {
        std::vector<bool> seen(GRID_SIZE + 1, false);
        for (int row = 0; row < GRID_SIZE; row++) {
            int num = grid[row][col];
            if (seen[num]) return false;
            seen[num] = true;
        }
    }

    // Check each 3x3 box
    for (int box = 0; box < 9; ++box) {
        bool seen[10] = {false};
        int startRow = (box / 3) * 3;
        int startCol = (box % 3) * 3;
        for (int i = 0; i < 9; ++i) {
            int row = startRow + i / 3;
            int col = startCol + i % 3;
            int num = grid[row][col];
            if (num == 0) continue;
            if (seen[num]) return false;
            seen[num] = true;
        }
    }
    return true;
}

bool Sudoku::solveGrid() {
    int row, col;
    if (!findEmptyCell(row, col)) {
        return true; // Solved
    }

    std::vector<int> nums(GRID_SIZE);
    std::iota(nums.begin(), nums.end(), 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(nums.begin(), nums.end(), gen);

    for (int num : nums) {
        if (isValid(row, col, num)) {
            grid[row][col] = num;

            if (solveGrid()) {
                return true;
            }

            grid[row][col] = 0; // Backtrack
        }
    }
    return false;
}

void Sudoku::removeCells(int cellsToRemove) {
    while (cellsToRemove > 0) {
        int row = std::rand() % GRID_SIZE;
        int col = std::rand() % GRID_SIZE;

        if (grid[row][col] != 0) {
            grid[row][col] = 0;
            fixed[row][col] = false;
            cellsToRemove--;
        }
    }
}

bool Sudoku::findEmptyCell(int &row, int &col) const {
    for (row = 0; row < GRID_SIZE; row++) {
        for (col = 0; col < GRID_SIZE; col++) {
            if (grid[row][col] == 0) {
                return true;
            }
        }
    }
    return false;
}

bool Sudoku::isValidInRow(int row, int num) const {
    for (int col = 0; col < GRID_SIZE; col++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

bool Sudoku::isValidInCol(int col, int num) const {
    for (int row = 0; row < GRID_SIZE; row++) {
        if (grid[row][col] == num) {
            return false;
        }
    }
    return true;
}

bool Sudoku::isValidInBox(int startRow, int startCol, int num) const {
    for (int i = 0; i < SUBGRID_SIZE; i++) {
        for (int j = 0; j < SUBGRID_SIZE; j++) {
            if (grid[i + startRow][j + startCol] == num) {
                return false;
            }
        }
    }
    return true;
}

bool Sudoku::hasConflict(int row, int col) const {
    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE || grid[row][col] == 0) {
        return false;
    }

    int num = grid[row][col];
    int count = 0;

    // Check row
    for (int c = 0; c < GRID_SIZE; c++) {
        if (grid[row][c] == num) count++;
    }
    if (count > 1) return true;

    // Check column
    count = 0;
    for (int r = 0; r < GRID_SIZE; r++) {
        if (grid[r][col] == num) count++;
    }
    if (count > 1) return true;

    // Check box
    count = 0;
    int boxStartRow = row - (row % SUBGRID_SIZE);
    int boxStartCol = col - (col % SUBGRID_SIZE);
    for (int r = 0; r < SUBGRID_SIZE; r++) {
        for (int c = 0; c < SUBGRID_SIZE; c++) {
            if (grid[boxStartRow + r][boxStartCol + c] == num) count++;
        }
    }
    return count > 1;
}



