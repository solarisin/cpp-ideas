// main.cpp
// Entry point for the neural network Tic Tac Toe program
#include "tictactoe.h"
#include "nn_tictactoe_player.h"
#include "random_utils.h"
#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <algorithm>

// randomMove: Selects a random valid move for the current board
std::pair<int, int> randomMove(const TicTacToe& game) {
    std::vector<std::pair<int, int>> moves;
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            if (game.isValidMove(r, c)) moves.emplace_back(r, c);
    if (moves.empty()) return {-1, -1};
    return moves[nn::random::getInt(0, moves.size() - 1)];
}

// Helper: Find a blocking move for the specified player and returns it
bool findBlockingMove(const TicTacToe& game, TicTacToe::Player player, std::pair<int, int>& move) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (!game.isValidMove(r, c)) continue;
            TicTacToe temp = game;
            temp.makeMove(r, c, player);
            if (temp.checkWin() == player) {
                move = {r, c};
                return true;
            }
        }
    }
    return false;
}

// playGame: Plays a game between the neural network and a random player
// Returns the winner (X, O, or EMPTY for draw)
TicTacToe::Player playGame(NNTicTacToePlayer& nnPlayer, bool nnIsX) {
    TicTacToe game;
    TicTacToe::Player current = TicTacToe::X;
    while (true) {
        std::pair<int, int> move;
        if ((current == TicTacToe::X) == nnIsX) {
            move = nnPlayer.chooseMove(game, current);
        } else {
            move = randomMove(game);
        }
        if (move.first == -1) break;
        game.makeMove(move.first, move.second, current);
        auto winner = game.checkWin();
        if (winner != TicTacToe::EMPTY) return winner;
        if (game.isDraw()) return TicTacToe::EMPTY;
        current = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
    }
    return TicTacToe::EMPTY;
}

// train: Trains the neural network by playing games against a random player
// Uses a simple reward system: +1 for win, -1 for loss, 0 for draw
void train(NNTicTacToePlayer& nnPlayer, int games, double lr) {
    for (int i = 0; i < games; ++i) {
        for (int nnIsX = 0; nnIsX < 2; ++nnIsX) { // Train as X and O
            TicTacToe game;
            std::vector<std::vector<double>> states;
            std::vector<std::vector<double>> targets;
            std::vector<int> moves_flat_idx;
            TicTacToe::Player current = TicTacToe::X;
            while (true) {
                auto flat = game.getFlatBoard();
                std::vector<double> input(flat.size());
                std::ranges::transform(flat, input.begin(), [current](int v) {
                    return static_cast<double>(v) * (current == TicTacToe::X ? 1.0 : -1.0);
                });
                std::pair<int, int> move;
                bool blocked = false;
                // Determine which player is the NN and which is the opponent
                bool nnTurn = ((current == TicTacToe::X) == (nnIsX == 1));
                TicTacToe::Player opp = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
                if (nnTurn) {
                    // Before NN moves, check if opponent could win next turn
                    bool opp_can_win = false;
                    for (int r = 0; r < 3; ++r) {
                        for (int c = 0; c < 3; ++c) {
                            if (!game.isValidMove(r, c)) continue;
                            TicTacToe temp = game;
                            temp.makeMove(r, c, opp);
                            if (temp.checkWin() == opp) {
                                opp_can_win = true;
                            }
                        }
                    }
                    move = nnPlayer.chooseMove(game, current);
                    // After NN moves, check if it blocked opponent's win
                    if (opp_can_win) {
                        TicTacToe temp = game;
                        temp.makeMove(move.first, move.second, current);
                        bool still_opp_can_win = false;
                        for (int r = 0; r < 3; ++r) {
                            for (int c = 0; c < 3; ++c) {
                                if (!temp.isValidMove(r, c)) continue;
                                TicTacToe temp2 = temp;
                                temp2.makeMove(r, c, opp);
                                if (temp2.checkWin() == opp) {
                                    still_opp_can_win = true;
                                }
                            }
                        }
                        if (!still_opp_can_win) blocked = true;
                    }
                } else {
                    int stratidx = 0;
                    while(move.first == -1) {
                        switch(stratidx++) {
                            case 0:
                                // Try to block the opponent's winning move
                                findBlockingMove(game, current, move);
                                break;
                            case 1:
                                // Take the center position if not occupied
                                if (game.isValidMove(1, 1))
                                    move = {1, 1};
                                break;
                            default:
                                move = randomMove(game);
                                break;
                        }
                    }
                }
                if (move.first == -1) break;
                game.makeMove(move.first, move.second, current);
                states.push_back(input);
                moves_flat_idx.push_back(move.first * 3 + move.second);
                // Track if this move was a block
                if (nnTurn && blocked) {
                    moves_flat_idx.back() |= (1 << 8); // use high bit as block flag
                }
                auto winner = game.checkWin();
                if (winner != TicTacToe::EMPTY || game.isDraw()) break;
                current = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
            }
            double reward = 0.0;
            auto winner = game.checkWin();
            // Reward is +1 for NN win, -1 for NN loss, 0 for draw
            if ((winner == TicTacToe::X && nnIsX == 1) || (winner == TicTacToe::O && nnIsX == 0)) reward = 1.0;
            else if ((winner == TicTacToe::X && nnIsX == 0) || (winner == TicTacToe::O && nnIsX == 1)) reward = -1.0;
            // Assign targets: only the move taken gets the reward, others are zero
            for (size_t j = 0; j < moves_flat_idx.size(); ++j) {
                std::vector<double> target(9, 0.0);
                // Penalize already-occupied cells
                const auto& board = states[j];
                for (int idx = 0; idx < 9; ++idx) {
                    if (std::abs(board[idx]) > 0.1 && (idx != (moves_flat_idx[j] & 0xFF))) {
                        target[idx] = -1.0; // Strong penalty for occupied cells
                    }
                }
                // Penalize moves that allow the opponent to win immediately
                bool allows_opp_win = false;
                TicTacToe temp = game;
                // Reconstruct the board up to this move
                temp.reset();
                TicTacToe::Player sim_current = TicTacToe::X;
                for (size_t k = 0; k <= j; ++k) {
                    int idx = moves_flat_idx[k] & 0xFF;
                    temp.makeMove(idx / 3, idx % 3, sim_current);
                    sim_current = (sim_current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
                }
                // After this move, check if opponent can win next
                TicTacToe::Player opp = (sim_current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
                for (int r = 0; r < 3; ++r) {
                    for (int c = 0; c < 3; ++c) {
                        if (!temp.isValidMove(r, c)) continue;
                        TicTacToe temp2 = temp;
                        temp2.makeMove(r, c, opp);
                        if (temp2.checkWin() == opp) {
                            allows_opp_win = true;
                        }
                    }
                }
                if (allows_opp_win) {
                    target[moves_flat_idx[j] & 0xFF] = -2.0; // Strong penalty for allowing forced loss
                } else if ((moves_flat_idx[j] & (1 << 8)) && reward == 0.0) {
                    target[moves_flat_idx[j] & 0xFF] = 1.0; // reward blocking move in draw games
                } else if (reward == 0.0) {
                    target[moves_flat_idx[j] & 0xFF] = -0.25; // draw
                } else {
                    target[moves_flat_idx[j] & 0xFF] = reward;
                }
                targets.push_back(target);
            }
            nnPlayer.getNN().train(states, targets, 1, lr);
        }
    }
}

int main() {
    // Create a neural network Tic Tac Toe player
    NNTicTacToePlayer nnPlayer;
    std::cout << "Training...\n";
    // Train the neural network by self-play against a random player, as both X and O
    train(nnPlayer, 50000000, 0.001);
    std::cout << "Done. Play against the neural net!\n";
    // Ask user for side
    char userSide;
    do {
        std::cout << "Do you want to be X or O? (X goes first): ";
        std::cin >> userSide;
        userSide = std::toupper(userSide);
    } while (userSide != 'X' && userSide != 'O');
    bool userIsX = (userSide == 'X');
    // Start a game: user vs NN
    TicTacToe game;
    TicTacToe::Player current = TicTacToe::X;
    while (true) {
        game.printBoard();
        if ((current == TicTacToe::X) == userIsX) {
            // Human player input
            int r, c;
            std::cout << "Your move (row col): ";
            std::cin >> r >> c;
            if (!game.makeMove(r, c, current)) {
                std::cout << "Invalid move!\n";
                continue;
            }
        } else {
            // NN chooses and plays its move
            std::pair<int, int> move = nnPlayer.chooseMove(game, current);
            std::cout << "NN plays: " << move.first << ", " << move.second << "\n";
            game.makeMove(move.first, move.second, current);
        }
        // Check for win or draw
        auto winner = game.checkWin();
        if (winner != TicTacToe::EMPTY) {
            game.printBoard();
            if ((winner == TicTacToe::X) == userIsX)
                std::cout << "You win!\n";
            else
                std::cout << "NN wins!\n";
            break;
        }
        if (game.isDraw()) {
            game.printBoard();
            std::cout << "Draw!\n";
            break;
        }
        // Switch player
        current = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
    }
    return 0;
}