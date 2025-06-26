#include "mainwindow.h"
#include "python_processor.h"

#include <QApplication>

#include <loguru/loguru.hpp>
#include <nlohmann/json.hpp>


QPalette createDarkPalette()
{
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::PlaceholderText, Qt::gray);
    darkPalette.setColor(QPalette::Mid, QColor(40,40,40));
    darkPalette.setColor(QPalette::Shadow, QColor(20,20,20));
    darkPalette.setColor(QPalette::Dark, QColor(20,20,20));
    darkPalette.setColor(QPalette::Light, QColor(70,70,70));
    return darkPalette;
}

int main(int argc, char *argv[])
{
    // Initialize loguru
    loguru::init(argc, argv);
    
    LOG_F(INFO, "Starting main function...");
    
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    app.setPalette(createDarkPalette());
    LOG_F(INFO, "QApplication created successfully");
    
    LOG_F(INFO, "Creating main window...");
    CppIdeasMainWindow window;
    
    LOG_F(INFO, "Showing window...");
    window.show();
    
    LOG_F(INFO, "Starting event loop...");
    return app.exec();
}
