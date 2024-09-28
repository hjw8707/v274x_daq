#include "QOnlineMonitorWindow.hxx"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QOnlineMonitorWindow::QOnlineMonitorWindow(QWidget* parent) : QMainWindow(parent) {
    qDebug() << "QOnlineMonitorWindow::QOnlineMonitorWindow()";
    setWindowTitle("V274XMOnline Monitor");

    onlineMonitor = new QOnlineMonitor();
    initUI();
}

QOnlineMonitorWindow::~QOnlineMonitorWindow() {
    qDebug() << "QOnlineMonitorWindow::~QOnlineMonitorWindow()";
    delete onlineMonitor;
}

void QOnlineMonitorWindow::initUI() {
    qDebug() << "QOnlineMonitorWindow::initUI()";
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    //////////////////////////////////////////////////////////////
    // Button Layout
    //////////////////////////////////////////////////////////////
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    layout->addLayout(buttonLayout);

    QPushButton* startButton = new QPushButton("Start");
    QPushButton* stopButton = new QPushButton("Stop");
    QPushButton* exitButton = new QPushButton("Exit");

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(exitButton);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Histogram Layout
    //////////////////////////////////////////////////////////////
    QHBoxLayout* histogramLayout = new QHBoxLayout();
    layout->addLayout(histogramLayout);

    QPushButton* clearHistogramButton = new QPushButton("Clear Histogram");
    histogramLayout->addWidget(clearHistogramButton);
    //////////////////////////////////////////////////////////////
    // SHM add layout
    //////////////////////////////////////////////////////////////
    QHBoxLayout* shmLayout = new QHBoxLayout();
    layout->addLayout(shmLayout);

    QLabel* shmNameLabel = new QLabel("SHM Name:");
    shmNameLineEdit = new QLineEdit();
    shmNameLineEdit->setMinimumWidth(200);
    QPushButton* addShmButton = new QPushButton("Add");
    QComboBox* shmListComboBox = new QComboBox();
    shmListComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QPushButton* removeShmButton = new QPushButton("Remove");

    shmLayout->addWidget(shmNameLabel);
    shmLayout->addWidget(shmNameLineEdit);
    shmLayout->addWidget(addShmButton);
    shmLayout->addWidget(shmListComboBox);
    shmLayout->addWidget(removeShmButton);
    //////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    // Connect
    ////////////////////////////////////////////////////////////
    connect(startButton, &QPushButton::clicked, this, &QOnlineMonitorWindow::start);
    connect(stopButton, &QPushButton::clicked, this, &QOnlineMonitorWindow::stop);
    connect(exitButton, &QPushButton::clicked, this, &QMainWindow::close);
    connect(clearHistogramButton, &QPushButton::clicked, this, &QOnlineMonitorWindow::clearHistogram);
    connect(addShmButton, &QPushButton::clicked, [this, shmListComboBox]() {
        QString shmName = shmNameLineEdit->text();
        if (!shmName.isEmpty()) {
            shmListComboBox->addItem(shmName);
            onlineMonitor->attachSharedMemory(shmName);
            shmNameLineEdit->clear();
        }
    });
    connect(removeShmButton, &QPushButton::clicked, [this, shmListComboBox]() {
        int currentIndex = shmListComboBox->currentIndex();
        if (currentIndex != -1) {
            QString shmName = shmListComboBox->itemText(currentIndex);
            onlineMonitor->detachSharedMemory(shmName);
            shmListComboBox->removeItem(currentIndex);
        }
    });
    //////////////////////////////////////////////////////////////
    // Button Enable/Disable
    //////////////////////////////////////////////////////////////
    stopButton->setEnabled(false);
    connect(startButton, &QPushButton::clicked,
            [startButton, stopButton, exitButton, addShmButton, shmListComboBox, removeShmButton]() {
                startButton->setEnabled(false);
                stopButton->setEnabled(true);
                exitButton->setEnabled(false);
                addShmButton->setEnabled(false);
                shmListComboBox->setEnabled(false);
                removeShmButton->setEnabled(false);
            });
    connect(stopButton, &QPushButton::clicked,
            [startButton, stopButton, exitButton, addShmButton, shmListComboBox, removeShmButton]() {
                startButton->setEnabled(true);
                stopButton->setEnabled(false);
                exitButton->setEnabled(true);
                addShmButton->setEnabled(true);
                shmListComboBox->setEnabled(true);
                removeShmButton->setEnabled(true);
            });
}

void QOnlineMonitorWindow::start() { onlineMonitor->start(); }

void QOnlineMonitorWindow::stop() { onlineMonitor->stop(); }

void QOnlineMonitorWindow::clearHistogram() { onlineMonitor->clearHistogram(); }