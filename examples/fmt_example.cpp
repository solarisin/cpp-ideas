#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <vector>
#include <map>

int main()
{
    fmt::print("🎨 fmt Library Examples\n");
    fmt::print("========================\n\n");

    // Basic formatting
    fmt::print("1. Basic formatting:\n");
    fmt::print("   Hello, {}!\n", "fmt");
    fmt::print("   Number: {}, Float: {:.2f}\n", 42, 3.14159);
    fmt::print("\n");

    // Positional and named arguments
    fmt::print("2. Advanced formatting:\n");
    fmt::print("   {1} {0}\n", "World", "Hello");
    fmt::print("   {name} is {age} years old\n", 
               fmt::arg("name", "Alice"), 
               fmt::arg("age", 30));
    fmt::print("\n");

    // Containers (C++23 ranges support)
    fmt::print("3. Container formatting:\n");
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    fmt::print("   Vector: {}\n", numbers);
    
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Carol", 92}};
    fmt::print("   Map: {}\n", scores);
    fmt::print("\n");

    // Colors (if terminal supports it)
    fmt::print("4. Colored output:\n");
    fmt::print(fmt::fg(fmt::color::red), "   This is red text\n");
    fmt::print(fmt::fg(fmt::color::green), "   This is green text\n");
    fmt::print(fmt::fg(fmt::color::blue), "   This is blue text\n");
    fmt::print("\n");

    // Performance demonstration
    fmt::print("5. Performance test:\n");
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 10000; ++i) {
        std::string result = fmt::format("Iteration {}: value = {:.3f}", i, i * 0.1);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    fmt::print("   Formatted 10,000 strings in {} microseconds\n", duration.count());
    fmt::print("   That's {:.2f} strings per microsecond!\n", 10000.0 / duration.count());

    return 0;
}
