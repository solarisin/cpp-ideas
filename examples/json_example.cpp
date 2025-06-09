#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include <iostream>
#include <fstream>

using json = nlohmann::json;

int main()
{
    fmt::print("📋 JSON Library Examples\n");
    fmt::print("=========================\n\n");

    // 1. Creating JSON objects
    fmt::print("1. Creating JSON objects:\n");
    json person = {
        {"name", "John Doe"},
        {"age", 30},
        {"city", "New York"},
        {"married", true},
        {"children", {"Alice", "Bob"}},
        {"address", {
            {"street", "123 Main St"},
            {"zipcode", "10001"}
        }}
    };
    
    fmt::print("   Person: {}\n\n", person.dump(2));

    // 2. Accessing JSON data
    fmt::print("2. Accessing JSON data:\n");
    fmt::print("   Name: {}\n", person["name"].get<std::string>());
    fmt::print("   Age: {}\n", person["age"].get<int>());
    fmt::print("   First child: {}\n", person["children"][0].get<std::string>());
    fmt::print("   Street: {}\n", person["address"]["street"].get<std::string>());
    fmt::print("\n");

    // 3. Modifying JSON
    fmt::print("3. Modifying JSON:\n");
    person["age"] = 31;
    person["children"].push_back("Charlie");
    person["job"] = "Software Engineer";
    
    fmt::print("   Updated person: {}\n\n", person.dump(2));

    // 4. JSON arrays
    fmt::print("4. Working with arrays:\n");
    json numbers = json::array({1, 2, 3, 4, 5});
    json mixed = json::array({"hello", 42, true, nullptr});
    
    fmt::print("   Numbers: {}\n", numbers.dump());
    fmt::print("   Mixed array: {}\n", mixed.dump());
    fmt::print("   Array size: {}\n\n", numbers.size());

    // 5. JSON parsing from string
    fmt::print("5. Parsing JSON from string:\n");
    std::string json_string = R"({
        "project": "C++ Ideas",
        "version": "1.0.0",
        "dependencies": ["fmt", "nlohmann/json", "Qt5"],
        "settings": {
            "debug": true,
            "optimization": "O2"
        }
    })";
    
    try {
        json config = json::parse(json_string);
        fmt::print("   Parsed successfully!\n");
        fmt::print("   Project: {}\n", config["project"].get<std::string>());
        fmt::print("   Dependencies count: {}\n", config["dependencies"].size());
        
        fmt::print("   Dependencies:\n");
        for (const auto& dep : config["dependencies"]) {
            fmt::print("     - {}\n", dep.get<std::string>());
        }
    } catch (const json::exception& e) {
        fmt::print("   JSON parsing error: {}\n", e.what());
    }
    fmt::print("\n");

    // 6. Type checking and safety
    fmt::print("6. Type checking:\n");
    json test_value = 42;
    
    if (test_value.is_number_integer()) {
        fmt::print("   Value is an integer: {}\n", test_value.get<int>());
    }
    
    test_value = "hello";
    if (test_value.is_string()) {
        fmt::print("   Value is a string: {}\n", test_value.get<std::string>());
    }
    
    test_value = json::array({1, 2, 3});
    if (test_value.is_array()) {
        fmt::print("   Value is an array with {} elements\n", test_value.size());
    }
    fmt::print("\n");

    // 7. Custom objects
    fmt::print("7. Working with custom structures:\n");
    struct Config {
        std::string name;
        int version;
        bool enabled;
        std::vector<std::string> features;
    };
    
    // Convert to JSON
    json config_json;
    config_json["name"] = "MyApp";
    config_json["version"] = 2;
    config_json["enabled"] = true;
    config_json["features"] = {"auth", "logging", "metrics"};
    
    fmt::print("   Config as JSON: {}\n", config_json.dump(2));

    return 0;
}
