// nn_tictactoe_player.cpp
// Implements a neural network-based Tic Tac Toe player
#include "nn_tictactoe_player.h"
#include "random_utils.h"
#include <algorithm>
#include <limits>
#include <vector>
#include <string>
#include <ranges>

extern std::mt19937& global_rng(); // In nn_tictactoe_player.cpp, include main.cpp's global_rng declaration

// Constructor: Initializes the neural network with 9 input nodes (for the board),
// 18 hidden nodes, and 9 output nodes (one for each possible move on the board).
NNTicTacToePlayer::NNTicTacToePlayer() : nn({9, 36, 9}) {}

// chooseMove: Given the current game state and player, selects the best move
// using the neural network's output. Returns a (row, col) pair.
// The neural network takes the board as input and outputs a score for each cell.
// The move with the highest score among valid moves is chosen.
std::pair<int, int> NNTicTacToePlayer::chooseMove(const TicTacToe& game, TicTacToe::Player player) {
    std::vector<int> flat = game.getFlatBoard();
    std::vector<double> input(flat.size());
    for (size_t i = 0; i < flat.size(); ++i) {
        input[i] = static_cast<double>(flat[i]) * (player == TicTacToe::X ? 1.0 : -1.0);
    }
    std::vector<double> output = nn.forward(input);
    double bestScore = -std::numeric_limits<double>::infinity();
    for (int i = 0; i < 9; ++i) {
        int row = i / 3, col = i % 3;
        if (game.isValidMove(row, col) && output[i] > bestScore) {
            bestScore = output[i];
        }
    }
    std::vector<int> bestMoves;
    for (int i = 0; i < 9; ++i) {
        int row = i / 3, col = i % 3;
        if (game.isValidMove(row, col) && output[i] == bestScore) {
            bestMoves.push_back(i);
        }
    }
    if (bestMoves.empty()) return {-1, -1};
    int chosen = bestMoves[nn::random::getInt(0, bestMoves.size() - 1)];
    return {chosen / 3, chosen % 3};
}

// getNN: Returns a reference to the underlying neural network (for training, etc.)
SimpleNN& NNTicTacToePlayer::getNN() {
    return nn;
}

// Remove the train function definition from this file to avoid multiple definition errors.
// The train function should be implemented in one place only, e.g., in main.cpp
