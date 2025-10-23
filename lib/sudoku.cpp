#include "sudoku.h"
#include <iostream>

Sudoku::Sudoku() : grid(GRID_SIZE, std::vector<int>(GRID_SIZE, 0)),
                   fixed(GRID_SIZE, std::vector<bool>(GRID_SIZE, false)) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    generatePuzzle();
}

void Sudoku::generatePuzzle() {
    // Start with an empty grid
    for(auto& row : grid) {
        std::fill(row.begin(), row.end(), 0);
    }
    for(auto& row : fixed) {
        std::fill(row.begin(), row.end(), false);
    }

    // Fill diagonal subgrids
    for (int box = 0; box < GRID_SIZE; box += SUBGRID_SIZE) {
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
        std::fill(row.begin(), row.end(), false);
    }
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
    // Temporarily remove the current number to check validity
    int temp = grid[row][col];
    grid[row][col] = 0;

    if (num == 0 || isValid(row, col, num)) {
        grid[row][col] = num;
        return true;
    }

    grid[row][col] = temp;
    return false;
}

int Sudoku::getNumber(int row, int col) const {
    return grid[row][col];
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



