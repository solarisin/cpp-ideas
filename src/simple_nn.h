// simple_nn.h
// Defines a simple feedforward neural network class
#ifndef SIMPLE_NN_H
#define SIMPLE_NN_H
#include <vector>
#include <random>

// SimpleNN implements a configurable feedforward neural network
class SimpleNN {
public:
    // Constructor: takes a vector specifying the number of nodes in each layer
    SimpleNN(const std::vector<int>& layers);
    // Forward pass: computes the output for a given input
    std::vector<double> forward(const std::vector<double>& input);
    // Trains the network using backpropagation on a batch of inputs/targets
    void train(const std::vector<std::vector<double>>& inputs,
               const std::vector<std::vector<double>>& targets,
               int epochs, double lr);
    // Mutates the weights and biases randomly (for evolutionary strategies)
    void mutate(double rate, double scale);
    // Set/get weights and biases (for saving/loading or evolution)
    void setWeights(const std::vector<std::vector<std::vector<double>>>& w,
                   const std::vector<std::vector<double>>& b);
    std::vector<std::vector<std::vector<double>>> getWeights() const;
    std::vector<std::vector<double>> getBiases() const;
private:
    std::vector<int> layers; // Number of nodes in each layer
    std::vector<std::vector<std::vector<double>>> weights; // Weights for each layer
    std::vector<std::vector<double>> biases; // Biases for each layer
    std::mt19937 rng; // Random number generator
    double sigmoid(double x) const; // Sigmoid activation function
    double dsigmoid(double x) const; // Derivative of sigmoid
};

#endif // SIMPLE_NN_H
