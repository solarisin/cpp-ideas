#include "random_utils.h"
#include <random>

namespace nn::random {
namespace {
   // Global RNG for all random choices
    std::mt19937& global_rng() {
        static std::mt19937 rng(std::random_device{}());
        return rng;
    }
}

// Generates a random integer in the range [min, max]
int getInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(global_rng());
}

} // namespace nn::random