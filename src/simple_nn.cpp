// simple_nn.cpp
// Implements a simple feedforward neural network with backpropagation
#include "simple_nn.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <vector>
#include <string>
#include <ranges>

// Constructor: initializes weights and biases randomly for each layer
SimpleNN::SimpleNN(const std::vector<int>& layers) : layers(layers), rng(std::random_device{}()) {
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    weights.resize(layers.size() - 1);
    biases.resize(layers.size() - 1);
    for (size_t i = 0; i < layers.size() - 1; ++i) {
        weights[i].resize(layers[i + 1], std::vector<double>(layers[i]));
        biases[i].resize(layers[i + 1]);
        for (int j = 0; j < layers[i + 1]; ++j) {
            for (int k = 0; k < layers[i]; ++k) {
                weights[i][j][k] = dist(rng);
            }
            biases[i][j] = dist(rng);
        }
    }
}

// Sigmoid activation function
double SimpleNN::sigmoid(double x) const {
    return 1.0 / (1.0 + std::exp(-x));
}

// Derivative of sigmoid
double SimpleNN::dsigmoid(double x) const {
    double s = sigmoid(x);
    return s * (1 - s);
}

// Forward pass: computes the output for a given input
std::vector<double> SimpleNN::forward(const std::vector<double>& input) {
    auto a = input;
    for (size_t l = 0; l < weights.size(); ++l) {
        std::vector<double> z(weights[l].size());
        for (size_t j = 0; j < weights[l].size(); ++j) {
            z[j] = biases[l][j];
            for (size_t k = 0; k < weights[l][j].size(); ++k) {
                z[j] += weights[l][j][k] * a[k];
            }
        }
        // Only apply sigmoid if not the output layer
        if (l < weights.size() - 1) {
            for (auto& v : z) v = sigmoid(v);
        }
        a = z;
    }
    return a;
}

// Trains the network using backpropagation on a batch of inputs/targets
void SimpleNN::train(const std::vector<std::vector<double>>& inputs,
                    const std::vector<std::vector<double>>& targets,
                    int epochs, double lr) {
    assert(inputs.size() == targets.size());
    for (int epoch = 0; epoch < epochs; ++epoch) {
        for (size_t n = 0; n < inputs.size(); ++n) {
            // Forward pass: store activations and pre-activations (zs)
            std::vector<std::vector<double>> activations = {inputs[n]};
            std::vector<std::vector<double>> zs;
            for (size_t l = 0; l < weights.size(); ++l) {
                std::vector<double> z(weights[l].size());
                std::vector<double> a(weights[l].size());
                for (size_t j = 0; j < weights[l].size(); ++j) {
                    z[j] = biases[l][j];
                    for (size_t k = 0; k < weights[l][j].size(); ++k) {
                        z[j] += weights[l][j][k] * activations.back()[k];
                    }
                    // Only apply sigmoid if not the output layer
                    if (l < weights.size() - 1)
                        a[j] = sigmoid(z[j]);
                    else
                        a[j] = z[j]; // Linear output
                }
                zs.push_back(z);
                activations.push_back(a);
            }
            // Backward pass: compute gradients
            std::vector<std::vector<double>> delta(weights.size());
            // Output layer error (linear, so derivative is 1)
            delta.back().resize(activations.back().size());
            for (size_t j = 0; j < activations.back().size(); ++j) {
                double error = activations.back()[j] - targets[n][j];
                delta.back()[j] = error; // *1 for linear
            }
            // Hidden layers
            for (int l = (int)weights.size() - 2; l >= 0; --l) {
                delta[l].resize(activations[l + 1].size(), 0.0);
                for (size_t j = 0; j < weights[l + 1].size(); ++j) {
                    for (size_t k = 0; k < weights[l + 1][j].size(); ++k) {
                        delta[l][k] += weights[l + 1][j][k] * delta[l + 1][j];
                    }
                }
                for (size_t k = 0; k < delta[l].size(); ++k) {
                    delta[l][k] *= dsigmoid(zs[l][k]);
                }
            }
            // Update weights and biases using gradients
            for (size_t l = 0; l < weights.size(); ++l) {
                for (size_t j = 0; j < weights[l].size(); ++j) {
                    for (size_t k = 0; k < weights[l][j].size(); ++k) {
                        weights[l][j][k] -= lr * delta[l][j] * activations[l][k];
                    }
                    biases[l][j] -= lr * delta[l][j];
                }
            }
        }
    }
}

// Mutates the weights and biases randomly (for evolutionary strategies)
void SimpleNN::mutate(double rate, double scale) {
    std::uniform_real_distribution<double> dist(-scale, scale);
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    for (auto& layer : weights)
        for (auto& neuron : layer)
            for (auto& w : neuron)
                if (prob(rng) < rate) w += dist(rng);
    for (auto& layer : biases)
        for (auto& b : layer)
            if (prob(rng) < rate) b += dist(rng);
}

// Set the weights and biases (for loading/saving or evolution)
void SimpleNN::setWeights(const std::vector<std::vector<std::vector<double>>>& w,
                         const std::vector<std::vector<double>>& b) {
    weights = w;
    biases = b;
}

// Get the weights
std::vector<std::vector<std::vector<double>>> SimpleNN::getWeights() const {
    return weights;
}

// Get the biases
std::vector<std::vector<double>> SimpleNN::getBiases() const {
    return biases;
}

// Use modern C++23 features and best practices throughout the file
