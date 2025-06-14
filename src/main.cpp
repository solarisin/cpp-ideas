
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QTabWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFont>
#include <QScrollArea>
#include <QGridLayout>
#include <QFileDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QTimer>
#include <loguru/loguru.hpp>

#include "python_processor.h"
#include <nlohmann/json.hpp>
#include <memory>

class JsonPythonApp : public QMainWindow {
    Q_OBJECT

public:    JsonPythonApp(QWidget* parent = nullptr) : QMainWindow(parent) {
        LOG_F(INFO, "Starting JsonPythonApp constructor...");
        
        setupUI();
        setupMenuBar();
        setupStatusBar();
        
        LOG_F(INFO, "UI setup completed, initializing Python processor...");
        
        // Initialize Python processor
        pythonProcessor = std::make_unique<PythonProcessor>();
        
        LOG_F(INFO, "Python processor created, checking initialization...");
        
        // Check if Python processor is initialized
        if (!pythonProcessor->isInitialized()) {
            LOG_F(ERROR, "Python processor initialization failed: %s", pythonProcessor->getLastError().c_str());
            statusLabel->setText("Python processor initialization failed");
            statusLabel->setStyleSheet("color: red;");
            QMessageBox::warning(this, "Warning", 
                "Python processor failed to initialize:\n" + 
                QString::fromStdString(pythonProcessor->getLastError()));
        } else {
            LOG_F(INFO, "Python processor initialized successfully");
            statusLabel->setText("Python processor ready");
            statusLabel->setStyleSheet("color: green;");
        }
        
        LOG_F(INFO, "JsonPythonApp constructor completed");
    }

private slots:
    void processJson() {
        if (!pythonProcessor || !pythonProcessor->isInitialized()) {
            resultText->setPlainText("Error: Python processor not initialized");
            return;
        }
        
        QString jsonInput = inputText->toPlainText().trimmed();
        if (jsonInput.isEmpty()) {
            resultText->setPlainText("Error: Please enter JSON input");
            return;
        }
        
        // Show progress
        progressBar->setVisible(true);
        progressBar->setRange(0, 0); // Indeterminate progress
        statusLabel->setText("Processing...");
        
        // Process in a timer to avoid blocking UI
        QTimer::singleShot(10, [this, jsonInput]() {
            try {
                std::string result = pythonProcessor->processJson(jsonInput.toStdString());
                
                // Format the JSON result for better readability
                auto jsonDoc = QJsonDocument::fromJson(QByteArray::fromStdString(result));
                if (!jsonDoc.isNull()) {
                    QString formattedResult = jsonDoc.toJson(QJsonDocument::Indented);
                    resultText->setPlainText(formattedResult);
                } else {
                    resultText->setPlainText(QString::fromStdString(result));
                }
                
                statusLabel->setText("Processing completed");
                statusLabel->setStyleSheet("color: green;");
                
            } catch (const std::exception& e) {
                resultText->setPlainText("Error: " + QString::fromStdString(e.what()));
                statusLabel->setText("Processing failed");
                statusLabel->setStyleSheet("color: red;");
            }
            
            progressBar->setVisible(false);
        });
    }
    
    void clearAll() {
        inputText->clear();
        resultText->clear();
        statusLabel->setText("Ready");
        statusLabel->setStyleSheet("color: black;");
    }
    
    void generateSampleJson() {
        QString sampleType = sampleCombo->currentText();
        nlohmann::json sample;
        
        if (sampleType == "Math Operation") {
            sample = {
                {"type", "math"},
                {"operation", "add"},
                {"numbers", {1, 2, 3, 4, 5}}
            };
        } else if (sampleType == "Text Processing") {
            sample = {
                {"type", "text"},
                {"operation", "uppercase"},
                {"text", "hello world from qt!"}
            };
        } else if (sampleType == "Data Analysis") {
            sample = {
                {"type", "data"},
                {"operation", "stats"},
                {"dataset", {10, 20, 30, 40, 50, 25, 35, 45}}
            };
        } else if (sampleType == "Echo Test") {
            sample = {
                {"type", "echo"},
                {"message", "This is a test from Qt application"},
                {"timestamp", "user_generated"}
            };
        }
        
        QString jsonStr = QString::fromStdString(sample.dump(2));
        inputText->setPlainText(jsonStr);
    }
    
    void loadJsonFile() {
        QString fileName = QFileDialog::getOpenFileName(this, 
            "Load JSON File", "", "JSON Files (*.json);;All Files (*)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString content = file.readAll();
                inputText->setPlainText(content);
                statusLabel->setText("File loaded: " + QFileInfo(fileName).fileName());
            } else {
                QMessageBox::warning(this, "Error", "Could not open file: " + fileName);
            }
        }
    }
    
    void saveJsonFile() {
        QString content = resultText->toPlainText();
        if (content.isEmpty()) {
            QMessageBox::information(this, "Info", "No result to save");
            return;
        }
        
        QString fileName = QFileDialog::getSaveFileName(this, 
            "Save Result", "", "JSON Files (*.json);;Text Files (*.txt);;All Files (*)");
        
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << content;
                statusLabel->setText("Result saved: " + QFileInfo(fileName).fileName());
            } else {
                QMessageBox::warning(this, "Error", "Could not save file: " + fileName);
            }
        }
    }

private:
    void setupUI() {
        setWindowTitle("Qt JSON Python Processor");
        setMinimumSize(1000, 700);
        
        auto* centralWidget = new QWidget;
        setCentralWidget(centralWidget);
        
        auto* mainLayout = new QVBoxLayout(centralWidget);
        
        // Create tab widget
        auto* tabWidget = new QTabWidget;
        mainLayout->addWidget(tabWidget);
        
        // Main processing tab
        auto* mainTab = new QWidget;
        tabWidget->addTab(mainTab, "JSON Processor");
        setupMainTab(mainTab);
        
        // Help tab
        auto* helpTab = new QWidget;
        tabWidget->addTab(helpTab, "Help & Examples");
        setupHelpTab(helpTab);
    }
    
    void setupMainTab(QWidget* parent) {
        auto* layout = new QVBoxLayout(parent);
        
        // Controls section
        auto* controlsGroup = new QGroupBox("Controls");
        auto* controlsLayout = new QHBoxLayout(controlsGroup);
        
        // Sample selection
        controlsLayout->addWidget(new QLabel("Sample JSON:"));
        sampleCombo = new QComboBox;
        sampleCombo->addItems({"Math Operation", "Text Processing", "Data Analysis", "Echo Test"});
        controlsLayout->addWidget(sampleCombo);
        
        auto* generateBtn = new QPushButton("Generate Sample");
        connect(generateBtn, &QPushButton::clicked, this, &JsonPythonApp::generateSampleJson);
        controlsLayout->addWidget(generateBtn);
        
        controlsLayout->addStretch();
        
        auto* loadBtn = new QPushButton("Load JSON File");
        connect(loadBtn, &QPushButton::clicked, this, &JsonPythonApp::loadJsonFile);
        controlsLayout->addWidget(loadBtn);
        
        auto* clearBtn = new QPushButton("Clear All");
        connect(clearBtn, &QPushButton::clicked, this, &JsonPythonApp::clearAll);
        controlsLayout->addWidget(clearBtn);
        
        layout->addWidget(controlsGroup);
        
        // Main content area
        auto* splitter = new QSplitter(Qt::Horizontal);
        layout->addWidget(splitter);
        
        // Input section
        auto* inputGroup = new QGroupBox("JSON Input");
        auto* inputLayout = new QVBoxLayout(inputGroup);
        
        inputText = new QTextEdit;
        inputText->setFont(QFont("Consolas", 10));
        inputText->setPlaceholderText("Enter your JSON request here...");
        inputLayout->addWidget(inputText);
        
        auto* processBtn = new QPushButton("Process JSON");
        processBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px; }");
        connect(processBtn, &QPushButton::clicked, this, &JsonPythonApp::processJson);
        inputLayout->addWidget(processBtn);
        
        splitter->addWidget(inputGroup);
        
        // Output section
        auto* outputGroup = new QGroupBox("Python Result");
        auto* outputLayout = new QVBoxLayout(outputGroup);
        
        resultText = new QTextEdit;
        resultText->setFont(QFont("Consolas", 10));
        resultText->setReadOnly(true);
        resultText->setPlaceholderText("Results will appear here...");
        outputLayout->addWidget(resultText);
        
        auto* saveBtn = new QPushButton("Save Result");
        connect(saveBtn, &QPushButton::clicked, this, &JsonPythonApp::saveJsonFile);
        outputLayout->addWidget(saveBtn);
        
        splitter->addWidget(outputGroup);
        
        // Set equal sizes for the splitter
        splitter->setSizes({500, 500});
    }
    
    void setupHelpTab(QWidget* parent) {
        auto* layout = new QVBoxLayout(parent);
        
        auto* scrollArea = new QScrollArea;
        auto* helpWidget = new QWidget;
        auto* helpLayout = new QVBoxLayout(helpWidget);
        
        // Title
        auto* titleLabel = new QLabel("JSON Python Processor Help");
        titleLabel->setFont(QFont("Arial", 16, QFont::Bold));
        titleLabel->setAlignment(Qt::AlignCenter);
        helpLayout->addWidget(titleLabel);
        
        // Description
        auto* descLabel = new QLabel(
            "This application allows you to send JSON requests to a Python script "
            "and receive processed results. The Python script can handle various "
            "types of operations including mathematical calculations, text processing, "
            "data analysis, and echo operations."
        );
        descLabel->setWordWrap(true);
        helpLayout->addWidget(descLabel);
        
        // Supported operations
        auto* opsGroup = new QGroupBox("Supported Operations");
        auto* opsLayout = new QVBoxLayout(opsGroup);
        
        auto* mathLabel = new QLabel(
            "<b>Math Operations:</b><br>"
            "• add, multiply, mean, sqrt, power<br>"
            "• Example: {\"type\": \"math\", \"operation\": \"add\", \"numbers\": [1,2,3]}"
        );
        mathLabel->setWordWrap(true);
        opsLayout->addWidget(mathLabel);
        
        auto* textLabel = new QLabel(
            "<b>Text Operations:</b><br>"
            "• uppercase, lowercase, reverse, word_count, char_count, capitalize<br>"
            "• Example: {\"type\": \"text\", \"operation\": \"uppercase\", \"text\": \"hello\"}"
        );
        textLabel->setWordWrap(true);
        opsLayout->addWidget(textLabel);
        
        auto* dataLabel = new QLabel(
            "<b>Data Operations:</b><br>"
            "• stats, sort, unique, filter_numbers<br>"
            "• Example: {\"type\": \"data\", \"operation\": \"stats\", \"dataset\": [1,2,3,4,5]}"
        );
        dataLabel->setWordWrap(true);
        opsLayout->addWidget(dataLabel);
        
        auto* echoLabel = new QLabel(
            "<b>Echo Operation:</b><br>"
            "• Returns the input data with timestamp<br>"
            "• Example: {\"type\": \"echo\", \"message\": \"test\"}"
        );
        echoLabel->setWordWrap(true);
        opsLayout->addWidget(echoLabel);
        
        helpLayout->addWidget(opsGroup);
        
        // Usage instructions
        auto* usageGroup = new QGroupBox("How to Use");
        auto* usageLayout = new QVBoxLayout(usageGroup);
        
        auto* instructions = new QLabel(
            "1. Select a sample JSON from the dropdown or create your own<br>"
            "2. Click 'Generate Sample' to populate the input area<br>"
            "3. Modify the JSON as needed<br>"
            "4. Click 'Process JSON' to send to Python script<br>"
            "5. View the result in the output area<br>"
            "6. Save the result to a file if needed"
        );
        instructions->setWordWrap(true);
        usageLayout->addWidget(instructions);
        
        helpLayout->addWidget(usageGroup);
        
        helpLayout->addStretch();
        
        scrollArea->setWidget(helpWidget);
        scrollArea->setWidgetResizable(true);
        layout->addWidget(scrollArea);
    }
    
    void setupMenuBar() {
        auto* fileMenu = menuBar()->addMenu("File");
        
        auto* loadAction = fileMenu->addAction("Load JSON File");
        connect(loadAction, &QAction::triggered, this, &JsonPythonApp::loadJsonFile);
        
        auto* saveAction = fileMenu->addAction("Save Result");
        connect(saveAction, &QAction::triggered, this, &JsonPythonApp::saveJsonFile);
        
        fileMenu->addSeparator();
        
        auto* exitAction = fileMenu->addAction("Exit");
        connect(exitAction, &QAction::triggered, this, &QWidget::close);
        
        auto* helpMenu = menuBar()->addMenu("Help");
        auto* aboutAction = helpMenu->addAction("About");
        connect(aboutAction, &QAction::triggered, [this]() {
            QMessageBox::about(this, "About", 
                "Qt JSON Python Processor\n\n"
                "A Qt application that processes JSON data through Python scripts using Boost.Python.\n\n"
                "Features:\n"
                "• Mathematical operations\n"
                "• Text processing\n"
                "• Data analysis\n"
                "• File I/O support\n"
                "• Real-time processing");
        });
    }
    
    void setupStatusBar() {
        progressBar = new QProgressBar;
        progressBar->setVisible(false);
        progressBar->setMaximumWidth(200);
        
        statusLabel = new QLabel("Ready");
        
        statusBar()->addWidget(statusLabel);
        statusBar()->addPermanentWidget(progressBar);
    }
    
private:
    std::unique_ptr<PythonProcessor> pythonProcessor;
    
    // UI components
    QTextEdit* inputText;
    QTextEdit* resultText;
    QComboBox* sampleCombo;
    QLabel* statusLabel;
    QProgressBar* progressBar;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    // Initialize loguru
    loguru::init(argc, argv);
    
    LOG_F(INFO, "Starting main function...");
    
    QApplication app(argc, argv);
    LOG_F(INFO, "QApplication created successfully");
    
    LOG_F(INFO, "Creating main window...");
    JsonPythonApp window;
    
    LOG_F(INFO, "Showing window...");
    window.show();
    
    LOG_F(INFO, "Starting event loop...");
    return app.exec();
}
