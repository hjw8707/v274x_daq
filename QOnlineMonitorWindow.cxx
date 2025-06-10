#include "QOnlineMonitorWindow.hxx"

#include <QtCore/QDebug>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QOnlineMonitorWindow::QOnlineMonitorWindow(QWidget* parent)
    : QMainWindow(parent) {
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

    startButton = new QPushButton("Start");
    stopButton = new QPushButton("Stop");
    exitButton = new QPushButton("Exit");

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
    addShmButton = new QPushButton("Add");
    shmListComboBox = new QComboBox();
    shmListComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    removeShmButton = new QPushButton("Remove");

    shmLayout->addWidget(shmNameLabel);
    shmLayout->addWidget(shmNameLineEdit);
    shmLayout->addWidget(addShmButton);
    shmLayout->addWidget(shmListComboBox);
    shmLayout->addWidget(removeShmButton);
    //////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    // Connect
    ////////////////////////////////////////////////////////////
    connect(startButton, &QPushButton::clicked, this,
            &QOnlineMonitorWindow::start);
    connect(stopButton, &QPushButton::clicked, this,
            &QOnlineMonitorWindow::stop);
    connect(exitButton, &QPushButton::clicked, this, &QMainWindow::close);
    connect(clearHistogramButton, &QPushButton::clicked, this,
            &QOnlineMonitorWindow::clearHistogram);
    connect(addShmButton, &QPushButton::clicked, this,
            &QOnlineMonitorWindow::addShm);
    connect(removeShmButton, &QPushButton::clicked, this,
            &QOnlineMonitorWindow::removeShm);
    //////////////////////////////////////////////////////////////
    // Button Enable/Disable
    //////////////////////////////////////////////////////////////
    stopButton->setEnabled(false);
}

void QOnlineMonitorWindow::start() {
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    exitButton->setEnabled(false);
    addShmButton->setEnabled(false);
    shmListComboBox->setEnabled(false);
    removeShmButton->setEnabled(false);
    onlineMonitor->start();
}

void QOnlineMonitorWindow::stop() {
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    exitButton->setEnabled(true);
    addShmButton->setEnabled(true);
    shmListComboBox->setEnabled(true);
    removeShmButton->setEnabled(true);
    onlineMonitor->stop();
}

void QOnlineMonitorWindow::clearHistogram() { onlineMonitor->clearHistogram(); }

void QOnlineMonitorWindow::addShmFromName(const QString& name) {
    QString shmName = "shm_" + name;  // name of the shared memory created by
                                      // DAQ program = shm_(buffer_name)
    shmListComboBox->addItem(shmName);
    onlineMonitor->attachSharedMemory(shmName);
}

void QOnlineMonitorWindow::addShm() {
    QString shmName =
        "shm_" + shmNameLineEdit->text();  // name of the shared memory created
                                           // by DAQ program = shm_(buffer_name)
    if (!shmName.isEmpty()) {
        shmListComboBox->addItem(shmName);
        onlineMonitor->attachSharedMemory(shmName);
        shmNameLineEdit->clear();
    }
}

void QOnlineMonitorWindow::removeShm() {
    int currentIndex = shmListComboBox->currentIndex();
    if (currentIndex != -1) {
        QString shmName = shmListComboBox->itemText(currentIndex);
        onlineMonitor->detachSharedMemory(shmName);
        shmListComboBox->removeItem(currentIndex);
    }
}