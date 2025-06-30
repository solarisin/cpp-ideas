// nn_tictactoe_player.h
// Declares a neural network-based Tic Tac Toe player class
#ifndef NN_TICTACTOE_PLAYER_H
#define NN_TICTACTOE_PLAYER_H
#include "tictactoe.h"
#include "simple_nn.h"

// NNTicTacToePlayer uses a neural network to select moves in Tic Tac Toe
class NNTicTacToePlayer {
public:
    NNTicTacToePlayer(); // Constructor: initializes the neural network
    // Returns (row, col) for the move chosen by the neural network
    std::pair<int, int> chooseMove(const TicTacToe& game, TicTacToe::Player player);
    // Returns a reference to the underlying neural network (for training, etc.)
    SimpleNN& getNN();
private:
    SimpleNN nn; // The neural network instance
};

#endif // NN_TICTACTOE_PLAYER_H
