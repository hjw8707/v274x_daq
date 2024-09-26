#include "QCAENV274XMulti.hxx"

#include "QCloseEvent"
#include "QtCore/QObject"
#include "QtCore/QThread"
#include "QtCore/QTimer"
#include "QtGui/QIcon"
#include "QtWidgets/QApplication"
#include "QtWidgets/QCheckBox"
#include "QtWidgets/QFileDialog"
#include "QtWidgets/QGroupBox"
#include "QtWidgets/QHBoxLayout"
#include "QtWidgets/QLabel"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QMessageBox"
#include "QtWidgets/QProgressBar"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QSpinBox"
#include "QtWidgets/QStyle"
#include "QtWidgets/QTreeWidget"
#include "QtWidgets/QTreeWidgetItem"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QWidget"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// QCAENV274XMulti
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QCAENV274XMulti::QCAENV274XMulti(QWidget *parent)
    : isRunning(false), numOfDigitizers(0), digitizers(), QMainWindow(parent) {
    qDebug() << "QCAENV274XMulti constructor";

    setWindowTitle("V274X Multi DAQ");
    setWindowIcon(QIcon("icons/dig_v2740.png"));

    updateTimer = new QTimer(this);
    elapsedTimer = new QElapsedTimer();

    writer = QBufferedFileWriter::getInstance();
    nosave = false;

    // GUI 초기화
    initUI();
    updateStatus(false);  // 초기 상태는 멈춤
}

QCAENV274XMulti::~QCAENV274XMulti() {
    qDebug() << "QCAENV274XMulti destructor";

    // 자원 해제 코드
    if (updateTimer) {
        delete updateTimer;
        updateTimer = nullptr;
    }
    if (elapsedTimer) {
        delete elapsedTimer;
        elapsedTimer = nullptr;
    }
}

void QCAENV274XMulti::initUI() {
    qDebug() << "QCAENV274XMulti::initUI()";

    // GUI 구성 요소 설정
    resize(800, 600);
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    //////////////////////////////////////////////////////////////
    // Connect Layout
    QHBoxLayout *connectLayout = new QHBoxLayout();
    layout->addLayout(connectLayout);

    QLabel *boardLabel = new QLabel("Board Name:");
    boardLineEdit = new QLineEdit();
    boardLineEdit->setPlaceholderText("Enter Board Name");
    QLabel *ipLabel = new QLabel("IP Address:");
    ipLineEdit = new QLineEdit();
    ipLineEdit->setPlaceholderText("Enter IP Address");
    connectButton = new QPushButton("&Connect");
    exitButton = new QPushButton("\u23fb E&xit");

    connectLayout->addWidget(boardLabel);
    connectLayout->addWidget(boardLineEdit);
    connectLayout->addWidget(ipLabel);
    connectLayout->addWidget(ipLineEdit);
    connectLayout->addWidget(connectButton);
    connectLayout->addWidget(exitButton);

    connect(connectButton, &QPushButton::clicked, this, &QCAENV274XMulti::addDigitizer);
    connect(exitButton, &QPushButton::clicked, this, &QCAENV274XMulti::close);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Run Layout
    QHBoxLayout *runLayout = new QHBoxLayout();
    layout->addLayout(runLayout);

    QLabel *runNameLabel = new QLabel("Run Name");
    runNameLineEdit = new QLineEdit();
    runNameLineEdit->setPlaceholderText("Enter Run Name");
    QLabel *runNumberLabel = new QLabel("Run Number");
    runNumberSpinBox = new QSpinBox();
    runNumberSpinBox->setRange(0, 999999);
    runNumberSpinBox->setSingleStep(1);
    runNumberSpinBox->setValue(0);
    autoIncCheckBox = new QCheckBox("Auto Inc.");
    autoIncCheckBox->setChecked(true);
    QLabel *measurementTimeLabel = new QLabel("Set Time (s):");
    measurementTimeSpinBox = new QSpinBox();
    measurementTimeSpinBox->setRange(0, 999999);
    measurementTimeSpinBox->setSingleStep(1);
    measurementTimeSpinBox->setValue(0);

    runLayout->addWidget(measurementTimeLabel);
    runLayout->addWidget(measurementTimeSpinBox);
    runNSButton = new QPushButton("&Run(NS)");
    runButton = new QPushButton("&Run");
    stopButton = new QPushButton("&Stop");

    runLayout->addWidget(runNameLabel);
    runLayout->addWidget(runNameLineEdit);
    runLayout->addWidget(runNumberLabel);
    runLayout->addWidget(runNumberSpinBox);
    runLayout->addWidget(autoIncCheckBox);
    runLayout->addWidget(measurementTimeLabel);
    runLayout->addWidget(measurementTimeSpinBox);
    runLayout->addWidget(runNSButton);
    runLayout->addWidget(runButton);
    runLayout->addWidget(stopButton);

    connect(runNSButton, &QPushButton::clicked, this, &QCAENV274XMulti::runNS);
    connect(runButton, &QPushButton::clicked, this, &QCAENV274XMulti::run);
    connect(stopButton, &QPushButton::clicked, this, &QCAENV274XMulti::stop);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Digitizer Layout
    QFrame *digitizerFrame = new QFrame();
    digitizerFrame->setFrameShape(QFrame::Box);
    digitizerFrame->setFrameShadow(QFrame::Sunken);
    digitizerFrame->setLineWidth(1);
    QVBoxLayout *digitizerLayout = new QVBoxLayout(digitizerFrame);
    layout->addWidget(digitizerFrame);

    digitizerLabel = new QLabel(QString("Digitizer Settings: # = %1").arg(numOfDigitizers));
    digitizerLayout->addWidget(digitizerLabel);

    digitizerTabWidget = new QTabWidget();
    digitizerLayout->addWidget(digitizerTabWidget);

    digitizerTabWidget->setTabPosition(QTabWidget::North);
    digitizerTabWidget->setTabShape(QTabWidget::Rounded);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Status Layout
    QHBoxLayout *statusLayout = new QHBoxLayout();
    layout->addLayout(statusLayout);

    statusLabel = new QLabel("Status: Stopped");
    statusIconLabel = new QLabel();
    statusIconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MediaStop));
    elapsedTimeLabel = new QLabel("Elapsed Time: ");
    elapsedTimeLabel->setAlignment(Qt::AlignCenter);
    elapsedTimeLabel->setFixedWidth(200);
    QLabel *bpsNameLabel = new QLabel("kB/s : ");
    bytesPerSecondLabel = new QLabel("0 kB/s");
    // 게이지 추가
    bpsProgressBar = new QProgressBar(this);
    bpsProgressBar->setRange(0, 1000);  // 최대값 설정 (예: 1000 kB/s)
    bpsProgressBar->setValue(0);        // 초기값 설정

    statusLayout->addWidget(statusLabel);
    // statusLayout->addWidget(statusIconLabel);
    statusLayout->addWidget(elapsedTimeLabel);
    statusLayout->addWidget(bpsNameLabel);
    statusLayout->addWidget(bytesPerSecondLabel);
    statusLayout->addWidget(bpsProgressBar);

    QHBoxLayout *statusLayout2 = new QHBoxLayout();
    layout->addLayout(statusLayout2);
    filenameLabel = new QLabel("Saving to file: ");
    fileSizeLabel = new QLabel("File Size: 0 kBytes");
    statusLayout2->addWidget(filenameLabel);
    statusLayout2->addWidget(fileSizeLabel);

    connect(updateTimer, &QTimer::timeout, this, [this]() {
        qint64 elapsed = elapsedTimer->elapsed();
        elapsedTimeLabel->setText(QString("Elapsed Time: %1 s").arg(elapsed / 1000., 0, 'f', 1));

        float totalBps = 0;
        for (int i = 0; i < numOfDigitizers; i++) totalBps += boardBps[i];
        bytesPerSecondLabel->setText(QString("%1 kB/s").arg(totalBps / 1024, 0, 'f', 1));
        bpsProgressBar->setValue(static_cast<int>(totalBps / 1024));  // 게이지 업데이트
    });
}

void QCAENV274XMulti::runNS() {
    nosave = true;
    run();
}

void QCAENV274XMulti::run() {
    qDebug() << "QCAENV274XMulti::run()";
    if (numOfDigitizers == 0) {
        QMessageBox::warning(this, "No Digitizer", "No digitizer connected.");
        nosave = false;
        return;
    }
    // board bps 초기화
    for (int i = 0; i < numOfDigitizers; i++) boardBps[i] = 0;

    QString runName = runNameLineEdit->text().isEmpty() ? "run" : runNameLineEdit->text();
    int runNumber = runNumberSpinBox->value();
    QString fileName = runName + QString("%1").arg(runNumber, 4, 10, QChar('0')) + ".dat";
    std::ifstream fileCheck(fileName.toStdString());
    if (!nosave && fileCheck.is_open()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "파일 존재 확인",
                                      QString("파일 %1이 이미 존재합니다. 덮어씌우시겠습니까?").arg(fileName),
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }

    if (nosave) {
        filenameLabel->setText("Not saving to file");
    } else {
        filenameLabel->setText("Saving to file: " + QFileInfo(fileName).absoluteFilePath());
    }
    fileSizeLabel->setText("File Size: - kBytes");

    for (QCAENV2740 *digitizer : digitizers) {
        writer->setFileName(digitizer->getBoardName(), fileName + QString("_%1").arg(digitizer->getBoardName()));
        digitizer->readyDAQ(fileName, nosave);
    }
    writer->start();
    for (QCAENV2740 *digitizer : digitizers) digitizer->runDAQ();

    // 측정 시간이 0보다 크면 측정 시간 타이머 시작
    if (measurementTimeSpinBox->value() > 0)
        QTimer::singleShot(measurementTimeSpinBox->value() * 1000, this, &QCAENV274XMulti::stop);

    elapsedTimer->start();
    updateTimer->start(100);  // 100ms 마다 업데이트
    updateStatus(true);
}

void QCAENV274XMulti::stop() {
    qDebug() << "QCAENV274XMulti::stop()";

    for (QCAENV2740 *digitizer : digitizers) digitizer->stopDAQ();

    updateTimer->stop();
    elapsedTimeLabel->setText(QString("Elapsed Time: %1 s").arg(elapsedTimer->elapsed() / 1000., 0, 'f', 1));
    elapsedTimer->invalidate();
    updateStatus(false);

    uint64_t totalBytes = 0;
    for (int i = 0; i < numOfDigitizers; i++) totalBytes += boardTotalBytes[i];
    fileSizeLabel->setText(QString("File Size: %1 kBytes").arg(totalBytes / 1024.0, 0, 'f', 1));

    if (!nosave && autoIncCheckBox->isChecked()) runNumberSpinBox->setValue(runNumberSpinBox->value() + 1);
    nosave = false;
    writer->stop();
}

void QCAENV274XMulti::addDigitizer() {
    qDebug() << "QCAENV274XMulti::addDigitizer()";
    QString newBoardName = boardLineEdit->text();

    QString newIp = ipLineEdit->text();
    for (QCAENV2740 *digitizer : digitizers) {
        if (digitizer->getIPAddress() == newIp.toStdString()) {
            QMessageBox::information(this, "Duplicate IP", "이미 같은 IP를 가진 디지타이저가 있습니다.");
            return;
        }
    }
    qDebug() << "Add Digitizer with newIp: " << newIp;

    QString connectStr = "dig2://" + newIp;
    if (CAENV2740::available(connectStr.toStdString())) {
        digitizers.append(
            new QCAENV2740(ipLineEdit->text().toStdString().c_str(), numOfDigitizers, newBoardName, this));
        digitizerTabWidget->addTab(digitizers.last(), QString("Digitizer %1").arg(numOfDigitizers));
        numOfDigitizers++;
        connect(digitizers.last(), &QCAENV2740::removeDigitizer, this, &QCAENV274XMulti::removeDigitizer);
        connect(digitizers.last()->getThread(), &DataAcquisitionThreadSingle::updateBoardBps, this,
                &QCAENV274XMulti::updateBoardBps);
        connect(digitizers.last()->getThread(), &DataAcquisitionThreadSingle::updateBoardTotalBytes, this,
                &QCAENV274XMulti::updateBoardTotalBytes);
        writer->addBuffer(newBoardName);
        qDebug() << "Digitizer Added";
    } else {
        qDebug() << "Digitizer Not Added";
    }
    updateDigitizerLabel();
}

void QCAENV274XMulti::removeDigitizer(QCAENV2740 *digitizer) {
    qDebug() << "QCAENV274XMulti::removeDigitizer()";
    digitizerTabWidget->removeTab(digitizerTabWidget->indexOf(digitizer));
    digitizers.removeOne(digitizer);
    writer->removeBuffer(digitizer->getBoardName());
    delete digitizer;
    numOfDigitizers--;
    updateDigitizerLabel();
}

void QCAENV274XMulti::updateDigitizerLabel() {
    digitizerLabel->setText(QString("Digitizer Settings: # = %1").arg(numOfDigitizers));
}

void QCAENV274XMulti::closeEvent(QCloseEvent *event) {
    for (QCAENV2740 *digitizer : digitizers) {
        digitizer->close();
    }
    event->accept();
}

void QCAENV274XMulti::updateStatus(bool running) {
    qDebug() << "QCAENV274XMulti::updateStatus(): " << (running ? "Running" : "Stopped");
    isRunning = running;
    if (running) {
        ipLineEdit->setEnabled(false);
        runNameLineEdit->setEnabled(false);
        runNumberSpinBox->setEnabled(false);
        autoIncCheckBox->setEnabled(false);
        measurementTimeSpinBox->setEnabled(false);
        connectButton->setEnabled(false);
        runButton->setEnabled(false);
        runNSButton->setEnabled(false);
        stopButton->setEnabled(true);
        exitButton->setEnabled(false);
        statusLabel->setText("Status: Running" + QString(nosave ? " (No Save)" : ""));
        // statusIconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MediaPlay));
    } else {
        ipLineEdit->setEnabled(true);
        runNameLineEdit->setEnabled(true);
        runNumberSpinBox->setEnabled(true);
        autoIncCheckBox->setEnabled(true);
        measurementTimeSpinBox->setEnabled(true);
        connectButton->setEnabled(true);
        runButton->setEnabled(true);
        runNSButton->setEnabled(true);
        stopButton->setEnabled(false);
        exitButton->setEnabled(true);
        statusLabel->setText("Status: Stopped");
        // statusIconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MediaStop));
    }
}

void QCAENV274XMulti::updateBoardBps(int board, float bps) { boardBps[board] = bps; }

void QCAENV274XMulti::updateBoardTotalBytes(int board, uint64_t bytes) { boardTotalBytes[board] = bytes; }
