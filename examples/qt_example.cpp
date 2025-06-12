#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QListWidget>
#include <QtCore/QTimer>
#include <fmt/core.h>

class ExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    ExampleWindow(QWidget* parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        connectSignals();
        startTimer();
    }

private slots:
    void onButtonClicked()
    {
        QString text = lineEdit->text();
        if (!text.isEmpty()) {
            listWidget->addItem(fmt::format("Added: {}", text.toStdString()).c_str());
            lineEdit->clear();
            updateStatus();
        }
    }

    void onTimerUpdate()
    {
        static int counter = 0;
        counter++;
        
        QString timeText = fmt::format("Timer ticks: {} | Items: {}", 
                                     counter, 
                                     listWidget->count()).c_str();
        timerLabel->setText(timeText);
    }

private:
    void setupUI()
    {
        setWindowTitle("Qt5 + fmt Example");
        setMinimumSize(500, 400);

        // Central widget and main layout
        QWidget* central = new QWidget(this);
        setCentralWidget(central);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(central);

        // Title
        QLabel* title = new QLabel("Qt5 Integration Example", this);
        title->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        // Input section
        QHBoxLayout* inputLayout = new QHBoxLayout();
        
        lineEdit = new QLineEdit(this);
        lineEdit->setPlaceholderText("Enter some text...");
        
        QPushButton* addButton = new QPushButton("Add Item", this);
        
        inputLayout->addWidget(lineEdit);
        inputLayout->addWidget(addButton);
        mainLayout->addLayout(inputLayout);

        // List widget
        listWidget = new QListWidget(this);
        mainLayout->addWidget(listWidget);

        // Status area
        QVBoxLayout* statusLayout = new QVBoxLayout();
        
        statusLabel = new QLabel("Status: Ready", this);
        statusLabel->setStyleSheet("color: green; font-weight: bold;");
        
        timerLabel = new QLabel("Timer: Starting...", this);
        timerLabel->setStyleSheet("color: blue;");
        
        statusLayout->addWidget(statusLabel);
        statusLayout->addWidget(timerLabel);
        mainLayout->addLayout(statusLayout);

        // Info text
        QTextEdit* infoText = new QTextEdit(this);
        infoText->setMaximumHeight(100);
        infoText->setPlainText(
            fmt::format(
                "This example demonstrates Qt5 integration with fmt library.\n"
                "Qt Version: {}\n"
                "Compiler: {}\n"
                "Built with modern C++23 features!",
                QT_VERSION_STR,
                __clang_version__
            ).c_str()
        );
        infoText->setReadOnly(true);
        mainLayout->addWidget(infoText);

        // Connect button
        connect(addButton, &QPushButton::clicked, this, &ExampleWindow::onButtonClicked);
    }

    void connectSignals()
    {
        // Connect Enter key in line edit
        connect(lineEdit, &QLineEdit::returnPressed, this, &ExampleWindow::onButtonClicked);
    }

    void startTimer()
    {
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &ExampleWindow::onTimerUpdate);
        timer->start(1000); // Update every second
    }

    void updateStatus()
    {
        QString status = fmt::format("Status: {} items in list", 
                                   listWidget->count()).c_str();
        statusLabel->setText(status);
    }

    QLineEdit* lineEdit;
    QListWidget* listWidget;
    QLabel* statusLabel;
    QLabel* timerLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    fmt::print("🖥️  Starting Qt5 Example Application\n");
    fmt::print("Qt Version: {}\n", QT_VERSION_STR);
    fmt::print("Using modern C++23 with fmt formatting!\n\n");

    ExampleWindow window;
    window.show();

    fmt::print("✅ Window displayed successfully!\n");
    return app.exec();
}

#include "qt_example.moc"
