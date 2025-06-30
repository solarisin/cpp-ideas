// tictactoe.cpp
// Implements the TicTacToe game logic
#include "tictactoe.h"
#include <vector>
#include <string>
#include <ranges>
#include <algorithm>
#include <iostream>

// Constructor: Initializes a 3x3 board with EMPTY cells
TicTacToe::TicTacToe() : board(3, std::vector<Player>(3, EMPTY)) {}

// Resets the board to all EMPTY cells
void TicTacToe::reset() {
    for (auto& row : board) {
        std::ranges::fill(row, EMPTY);
    }
}

// Attempts to make a move for the given player at (row, col)
// Returns true if the move is valid and made, false otherwise
bool TicTacToe::makeMove(int row, int col, Player player) {
    if (isValidMove(row, col)) {
        board[row][col] = player;
        return true;
    }
    return false;
}

// Checks if the move at (row, col) is valid (in bounds and empty)
bool TicTacToe::isValidMove(int row, int col) const {
    return row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == EMPTY;
}

// Checks for a win: returns the winning player, or EMPTY if no winner
TicTacToe::Player TicTacToe::checkWin() const {
    // Check rows and columns
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] != EMPTY && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
        if (board[0][i] != EMPTY && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return board[0][i];
    }
    // Check diagonals
    if (board[0][0] != EMPTY && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] != EMPTY && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];
    return EMPTY;
}

// Checks if the game is a draw (no empty cells and no winner)
bool TicTacToe::isDraw() const {
    if (checkWin() != EMPTY) return false;
    for (const auto& row : board)
        for (auto cell : row)
            if (cell == EMPTY) return false;
    return true;
}

// Prints the board to stdout
void TicTacToe::printBoard() const {
    for (const auto& row : board) {
        for (auto cell : row) {
            char c = (cell == X) ? 'X' : (cell == O) ? 'O' : '.';
            std::cout << c << ' ';
        }
        std::cout << '\n';
    }
}

// Returns the board as a 2D vector
std::vector<std::vector<TicTacToe::Player>> TicTacToe::getBoard() const {
    return board;
}

// Returns the board as a flat vector of ints
std::vector<int> TicTacToe::getFlatBoard() const {
    std::vector<int> flat;
    flat.reserve(9);
    for (const auto& row : board)
        std::ranges::copy(row | std::views::transform([](Player p) { return static_cast<int>(p); }), std::back_inserter(flat));
    return flat;
}

// Use modern C++23 features in tictactoe.cpp
