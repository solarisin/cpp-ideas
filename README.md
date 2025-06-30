# Neural Network Tic Tac Toe (C++)

This project implements a simple feedforward neural network that learns to play Tic Tac Toe. The neural network is trained by playing against a random player and can then play interactively against a human.

## Features
- Complete Tic Tac Toe game logic
- Simple neural network (configurable layers, backpropagation)
- Training loop (self-play vs random)
- Play against the trained neural net in the terminal

## How to Build

This project uses CMake. To build the neural network Tic Tac Toe executable:

```bash
cmake -S . -B build
cmake --build build --target tictactoe_nn
```

Or use your IDE's build system to build the `tictactoe_nn` target.

## How to Run

From the build directory:

```bash
./build/bin/tictactoe_nn
```

- The program will first train the neural network by playing 10,000 games against a random player.
- After training, you can play as O against the neural network (which plays as X).
- Enter your move as two numbers: `row col` (e.g., `1 2` for row 1, column 2; rows and columns are 0-indexed).

## GUI Version (ImGui)

To build the GUI version (requires ImGui, GLFW, OpenGL):

```
cmake --build build --target tictactoe_nn_imgui
```

To run:

```
./build/bin/tictactoe_nn_imgui
```

- Play as O by clicking on the board.
- The neural network (X) will play automatically after your move.
- Click "Reset Game" to start over.

> ImGui, GLFW, and OpenGL must be available (e.g., via vcpkg: `vcpkg install imgui[glfw,opengl3] glfw3`)

## File Overview
- `src/tictactoe.h` / `tictactoe.cpp`: Game logic
- `src/simple_nn.h` / `simple_nn.cpp`: Simple feedforward neural network
- `src/nn_tictactoe_player.h` / `nn_tictactoe_player.cpp`: Neural network player
- `src/main.cpp`: Training and play loop

## Customization
- You can adjust the number of training games or learning rate in `main.cpp`.
- The neural network architecture can be changed in `nn_tictactoe_player.cpp`.

## Requirements
- C++23 compiler
- CMake 3.30+
- [fmt](https://github.com/fmtlib/fmt), [nlohmann_json](https://github.com/nlohmann/json), [loguru](https://github.com/emilk/loguru) (installed via vcpkg or system packages)

## Notes
- The neural network is simple and may not reach perfect play, but demonstrates basic reinforcement learning and neural network integration in C++.
- For best results, experiment with more training games, different architectures, or more advanced training strategies.

---

Feel free to open issues or contribute improvements!
