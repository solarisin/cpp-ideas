// tictactoe.h
// Defines the TicTacToe game logic and board representation
#ifndef TICTACTOE_H
#define TICTACTOE_H
#include <vector>
#include <iostream>

// TicTacToe class manages the game board, moves, and win/draw detection
class TicTacToe {
public:
    // Player enum: EMPTY (0), X (1), O (-1)
    enum Player { EMPTY = 0, X = 1, O = -1 };
    TicTacToe(); // Constructor: initializes empty board
    void reset(); // Resets the board to empty
    bool makeMove(int row, int col, Player player); // Makes a move if valid
    bool isValidMove(int row, int col) const; // Checks if a move is valid
    Player checkWin() const; // Checks if a player has won
    bool isDraw() const; // Checks if the game is a draw
    void printBoard() const; // Prints the board to stdout
    std::vector<std::vector<Player>> getBoard() const; // Returns board state
    std::vector<int> getFlatBoard() const; // Returns board as flat vector
private:
    std::vector<std::vector<Player>> board; // 3x3 board
};

#endif // TICTACTOE_H
