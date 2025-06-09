#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <iostream>

using json = nlohmann::json;

int main(int argc, char *argv[])
{
    // Initialize Qt Application
    QApplication app(argc, argv);

    // Create main window
    QMainWindow window;
    window.setWindowTitle("C++ Ideas - fmt, JSON, Qt Demo");
    window.resize(400, 300);

    // Create central widget and layout
    QWidget *centralWidget = new QWidget(&window);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Demonstrate fmt library
    std::string formatted_message = fmt::format("Hello from fmt! The answer is {}", 42);
    QLabel *fmtLabel = new QLabel(QString::fromStdString(formatted_message));
    layout->addWidget(fmtLabel);

    // Demonstrate nlohmann/json
    json config = {
        {"name", "C++ Ideas Project"},
        {"version", "1.0.0"},
        {"dependencies", {"fmt", "Qt5", "nlohmann/json"}},
        {"features", {
            {"modern_cpp", true},
            {"cross_platform", true},
            {"performance", "high"}
        }}
    };
    
    std::string json_str = config.dump(2);
    fmt::print("Configuration JSON:\n{}\n", json_str);

    QLabel *jsonLabel = new QLabel("JSON configuration loaded successfully!");
    layout->addWidget(jsonLabel);

    // Add some info about the environment
    QLabel *infoLabel = new QLabel(
        QString("Qt Version: %1\nCompiler: %2")
        .arg(QT_VERSION_STR)
        .arg(__clang_version__)
    );
    layout->addWidget(infoLabel);

    window.setCentralWidget(centralWidget);
    window.show();

    fmt::print("🚀 Application started successfully!\n");
    fmt::print("📊 Using {} dependencies in modern C++23\n", config["dependencies"].size());

    return app.exec();
}
