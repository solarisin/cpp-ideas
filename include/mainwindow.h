#ifndef CPP_IDEAS_MAINWINDOW_H
#define CPP_IDEAS_MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class PythonProcessor;
class QTextEdit;
class QComboBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QGroupBox;

class CppIdeasMainWindow : public QMainWindow {
    Q_OBJECT

public:
    CppIdeasMainWindow(QWidget* parent = nullptr);

private:
    Q_SLOT void processJson();
    Q_SLOT void clearAll();
    Q_SLOT void generateSampleJson();
    Q_SLOT void loadJsonFile();
    Q_SLOT void saveJsonFile();

    void setupUI();
    void setupMainTab(QWidget* parent);
    void setupHelpTab(QWidget* parent);
    void setupMenuBar();
    void setupStatusBar();

    std::unique_ptr<PythonProcessor> pythonProcessor;
    
    // UI components
    QTextEdit* inputText;
    QTextEdit* resultText;
    QComboBox* sampleCombo;
    QLabel* statusLabel;
    QProgressBar* progressBar;
};

#endif // CPP_IDEAS_MAINWINDOW_H