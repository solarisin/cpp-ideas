// main_imgui.cpp
// ImGui-based GUI for neural network Tic Tac Toe
#include "tictactoe.h"
#include "nn_tictactoe_player.h"
#include "random_utils.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <algorithm>
#include <thread>
#include <chrono>
#include <loguru.hpp>
#include <mutex>
#include <numeric>
#include <future>

void train(NNTicTacToePlayer& nnPlayer, int games, double lr);

// Helper to initialize ImGui and GLFW
static bool InitImGui(GLFWwindow*& window) {
    if (!glfwInit()) return false;
    window = glfwCreateWindow(600, 600, "Tic Tac Toe NN", nullptr, nullptr);
    if (!window) return false;
    glfwMakeContextCurrent(window);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    return true;
}

static void ShutdownImGui(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, char** argv) {
    loguru::init(argc, argv);
    GLFWwindow* window = nullptr;
    if (!InitImGui(window)) {
        LOG_F(ERROR, "Failed to initialize ImGui/GLFW");
        return 1;
    }
    NNTicTacToePlayer nnPlayer;
    train(nnPlayer, 1000000, 0.01); // Use digit separator for clarity
    TicTacToe game;
    TicTacToe::Player current = TicTacToe::X;
    bool gameOver = false;
    std::string status = "Your turn (O)";
    std::vector<double> nn_scores(9, 0.0); // Cache of last NN scores used for move
    bool nn_scores_valid = false;
    bool humanIsX = false; // Default: human is O
    bool sideChanged = false;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Tic Tac Toe Neural Net", nullptr, ImGuiWindowFlags_NoScrollbar);
        ImGui::Text("Choose your side:");
        int side = humanIsX ? 0 : 1;
        if (ImGui::RadioButton("Human is X (goes first)", side == 0)) {
            if (!humanIsX) { sideChanged = true; }
            humanIsX = true;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Human is O", side == 1)) {
            if (humanIsX) { sideChanged = true; }
            humanIsX = false;
        }
        if (sideChanged) {
            game.reset();
            current = TicTacToe::X;
            gameOver = false;
            status = humanIsX ? "Your turn (X)" : "Your turn (O)";
            nn_scores_valid = false;
            sideChanged = false;
        }
        ImGui::Text("Click a cell to play as %c. NN is %c.", humanIsX ? 'X' : 'O', humanIsX ? 'O' : 'X');
        if (ImGui::Button("Reset Game")) {
            game.reset();
            current = TicTacToe::X;
            gameOver = false;
            status = humanIsX ? "Your turn (X)" : "Your turn (O)";
            nn_scores_valid = false;
        }
        ImGui::Text("%s", status.c_str());
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Spacing();
        // Calculate dynamic cell size based on window size
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float cell_size = std::min(avail.x, avail.y) / 3.2f; // 3 cells + padding
        if (cell_size < 30.0f) cell_size = 30.0f; // minimum size
        // Draw board using modern C++23 features
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (c > 0) ImGui::SameLine();
                auto cell = game.getBoard()[r][c];
                ImGui::PushID(r * 3 + c);
                bool disabled = (cell != TicTacToe::EMPTY) || gameOver || ((current == TicTacToe::X) != humanIsX);
                if (ImGui::Button(" ", ImVec2(cell_size, cell_size)) && !disabled) {
                    game.makeMove(r, c, current);
                    current = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
                }
                // Draw vector icon for X or O
                ImVec2 p_min = ImGui::GetItemRectMin();
                ImVec2 p_max = ImGui::GetItemRectMax();
                ImVec2 center = ImVec2((p_min.x + p_max.x) * 0.5f, (p_min.y + p_max.y) * 0.5f);
                float radius = cell_size * 0.37f;
                ImU32 color = IM_COL32(220, 220, 220, 255); // Light gray for X and O
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                if (cell == TicTacToe::X) {
                    float pad = cell_size * 0.23f;
                    draw_list->AddLine(ImVec2(p_min.x + pad, p_min.y + pad), ImVec2(p_max.x - pad, p_max.y - pad), color, 4.0f);
                    draw_list->AddLine(ImVec2(p_min.x + pad, p_max.y - pad), ImVec2(p_max.x - pad, p_min.y + pad), color, 4.0f);
                } else if (cell == TicTacToe::O) {
                    draw_list->AddCircle(center, radius, color, 0, 4.0f);
                }
                // Overlay NN score if available
                if (nn_scores_valid) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.3f", nn_scores[r * 3 + c]);
                    ImVec2 text_size = ImGui::CalcTextSize(buf);
                    ImVec2 text_pos = ImVec2(center.x - text_size.x * 0.5f, p_max.y - text_size.y - 4.0f);
                    ImU32 text_color = IM_COL32(180, 180, 255, 255);
                    draw_list->AddText(text_pos, text_color, buf);
                }
                ImGui::PopID();
            }
        }
        ImGui::NewLine();
        // Check for win/draw before NN move
        auto winner = game.checkWin();
        if (!gameOver && (winner != TicTacToe::EMPTY || game.isDraw())) {
            gameOver = true;
            if (winner == TicTacToe::X) status = "NN wins!";
            else if (winner == TicTacToe::O) status = "You win!";
            else status = "Draw!";
        }
        // NN move (only if not gameOver)
        if (!gameOver && ((current == TicTacToe::X) == !humanIsX)) {
            // Cache the NN scores used for this move
            std::vector<int> flat = game.getFlatBoard();
            std::vector<double> input(flat.size());
            for (size_t i = 0; i < flat.size(); ++i) {
                input[i] = static_cast<double>(flat[i]) * (current == TicTacToe::X ? 1.0 : -1.0);
            }
            nn_scores = nnPlayer.getNN().forward(input);
            nn_scores_valid = true;
            auto move = nnPlayer.chooseMove(game, current);
            if (move.first != -1) {
                game.makeMove(move.first, move.second, current);
            }
            current = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
        }
        ImGui::End();
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        // Improved frame limiter: sleep only for the remaining time to reach ~16.67ms (60 FPS)
        static auto last_frame = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_frame).count();
        constexpr int target_ms = 17; // ~16.67ms
        if (frame_time < target_ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(target_ms - frame_time));
        }
        last_frame = std::chrono::steady_clock::now();
    }
    ShutdownImGui(window);
    return 0;
}

// Use modern C++23 features in train function
void train(NNTicTacToePlayer& nnPlayer, int games, double lr) {
    LOG_F(WARNING, "Training %d games with learning rate %.4f (shared NN, mutex)", games, lr);
    int num_threads = 3;
    int games_per_thread = games / num_threads;
    int leftover = games % num_threads;
    std::mutex nn_mutex;
    auto train_worker = [&](int n_games) {
        std::vector<std::vector<double>> states;
        std::vector<std::vector<double>> targets;
        for (int i = 0; i < n_games; ++i) {
            TicTacToe game;
            states.clear();
            targets.clear();
            TicTacToe::Player current = TicTacToe::X;
            while (true) {
                auto flat = game.getFlatBoard();
                std::vector<double> input(flat.size());
                std::ranges::transform(flat, input.begin(), [current](int v) {
                    return static_cast<double>(v) * (current == TicTacToe::X ? 1.0 : -1.0);
                });
                std::pair<int, int> move;
                if (current == TicTacToe::X) {
                    // Lock for NN move selection
                    std::scoped_lock lock(nn_mutex);
                    move = nnPlayer.chooseMove(game, current);
                } else {
                    std::vector<std::pair<int, int>> moves;
                    for (int r = 0; r < 3; ++r)
                        for (int c = 0; c < 3; ++c)
                            if (game.isValidMove(r, c)) moves.emplace_back(r, c);
                    if (moves.empty()) break;
                    int idx = nn::random::getInt(0, moves.size() - 1);
                    move = moves[idx];
                }
                if (move.first == -1) break;
                // Check if this move blocks an immediate win for the opponent
                bool blocked = false;
                if (current == TicTacToe::X || current == TicTacToe::O) {
                    TicTacToe::Player opp = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
                    TicTacToe tmp = game;
                    tmp.makeMove(move.first, move.second, current);
                    // For each possible move for the opponent, check if it would win
                    for (int r = 0; r < 3; ++r) {
                        for (int c = 0; c < 3; ++c) {
                            if (tmp.isValidMove(r, c)) {
                                TicTacToe tmp2 = tmp;
                                tmp2.makeMove(r, c, opp);
                                if (tmp2.checkWin() == opp) {
                                    blocked = true;
                                    break;
                                }
                            }
                        }
                        if (blocked) break;
                    }
                }
                game.makeMove(move.first, move.second, current);
                states.push_back(input);
                std::vector<double> target(9, 0.0);
                target[move.first * 3 + move.second] = 1.0;
                // Reward blocking move
                if (blocked) target[move.first * 3 + move.second] += 0.5; // or another value
                targets.push_back(target);
                auto winner = game.checkWin();
                if (winner != TicTacToe::EMPTY || game.isDraw()) break;
                current = (current == TicTacToe::X) ? TicTacToe::O : TicTacToe::X;
            }
            double reward = 0.0;
            auto winner = game.checkWin();
            if (winner == TicTacToe::X) reward = 1.0;
            else if (winner == TicTacToe::O) reward = -1.0;
            for (auto& t : targets)
                for (auto& v : t) v *= reward;
            // Lock for NN weight update
            {
                std::scoped_lock lock(nn_mutex);
                nnPlayer.getNN().train(states, targets, 1, lr);
            }
        }
    };
    std::vector<std::jthread> threads;
    for (int t = 0; t < num_threads; ++t) {
        int n_games = games_per_thread + (t < leftover ? 1 : 0);
        threads.emplace_back(train_worker, n_games);
    }
    for (auto& th : threads) th.join();
    LOG_F(WARNING, "Training complete (shared NN, %d threads)", num_threads);
}
