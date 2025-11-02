#ifndef SUDOKU_H
#define SUDOKU_H

#include <vector>
#include <random>
#include <algorithm>

class Sudoku {
public:
    static const int GRID_SIZE = 9;
    static const int SUBGRID_SIZE = 3;

    Sudoku();
    // difficulty: 1 = Easy, 2 = Medium (default), 3 = Hard
    void generatePuzzle();
    void generatePuzzle(int difficulty);
    bool isValid(int row, int col, int num) const;
    bool isCellEditable(int row, int col) const;
    bool setNumber(int row, int col, int num);
    int getNumber(int row, int col) const;
    bool isSolved() const;
    bool hasConflict(int row, int col) const;

private:
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<bool>> fixed;
    
    bool solveGrid();
    bool findEmptyCell(int &row, int &col) const;
    void removeCells();
    void removeCells(int cellsToRemove);
    bool isValidInRow(int row, int num) const;
    bool isValidInCol(int col, int num) const;
    bool isValidInBox(int startRow, int startCol, int num) const;
};

#endif // SUDOKU_H