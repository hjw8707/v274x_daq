#include "QCAENV2740Single.hxx"

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
// DataAcquisitionThread
//
// @brief 데이터 수집 스레드
// @details 데이터 수집 스레드는 데이터를 수집하고 처리하는 스레드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief This function is the run method of the DataAcquisitionThread class.
 * @details It is responsible for running the data acquisition thread.
 */
void DataAcquisitionThread::run() {
    size_t size = 0;
    uint8_t *data = new uint8_t[2621440];
    uint32_t nevent_raw = 0;
    uint64_t nbunch = 0;
    uint32_t prev_aggregate_counter = 0;
    size_t totalBytes = 0;  // 총 전송된 바이트 수
    qint64 totalTime = 0;   // 총 측정 시간
    QElapsedTimer timer;    // 타이머 추가
    timer.start();          // 타이머 시작
    while (!isInterruptionRequested()) {
        int ret = daq->readDataRaw(1000, data, &size, &nevent_raw);

        switch (ret) {
            case CAEN_FELib_Success:
                // emit dataAcquired(data, size);
                if (fout) fout->write(reinterpret_cast<char *>(data), size);
                if (shm) shm->writeData(data, size);
                nbunch++;
                break;
            case CAEN_FELib_Timeout:
                std::cout << "CAEN_FELib_Timeout" << std::endl;
                break;
            case CAEN_FELib_Stop:
                std::cout << "CAEN_FELib_Stop" << std::endl;
                break;
            default:
                std::cout << "CAEN_FELib_Error" << std::endl;
                break;
        }

        totalBytes += size;  // 전송된 바이트 수 누적
        // 초당 전송된 바이트 수 업데이트
        if (timer.elapsed() >= 1000) {
            totalTime += timer.elapsed();                                     // 1초마다
            emit updateBytesPerSecond(totalBytes * 1000. / timer.elapsed());  // 시그널 전송
            emit updateMeasurementTime(totalTime);                            // 측정 시간 업데이트
            totalBytes = 0;   // 카운터 초기화                                  // 측정 시간 업데이트
            timer.restart();  // 타이머 재시작
        }
    }
    totalTime += timer.elapsed();
    emit updateMeasurementTime(totalTime);
    emit updateBytesPerSecond(0.0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// QCAENV2740
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QCAENV2740Single::QCAENV2740Single(QWidget *parent) : verbose(false), currentStatus(-1), QMainWindow(parent) {
    setWindowTitle("V274X DAQ");
    setWindowIcon(QIcon("icons/dig_v2740.png"));

    timer = new QTimer(this);
    verbose = true;

    // shm = nullptr;
    shm = new SharedMemory("/v2740");
    //  CAENV2740 초기화
    initDAQ();
    // GUI 초기화
    initUI();
}

QCAENV2740Single::~QCAENV2740Single() {
    // 자원 해제 코드
    if (daq) {
        delete daq;
        daq = nullptr;
    }
    if (par) {
        delete par;
        par = nullptr;
    }
    if (timer) {
        delete timer;
        timer = nullptr;
    }
    if (shm) {
        delete shm;
        shm = nullptr;
    }
}

void QCAENV2740Single::initUI() {
    // GUI 구성 요소 설정
    resize(800, 600);
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    //////////////////////////////////////////////////////////////
    // Connect Layout
    QHBoxLayout *connectLayout = new QHBoxLayout();
    layout->addLayout(connectLayout);

    QLabel *ipLabel = new QLabel("IP Address:");
    ipLineEdit = new QLineEdit();
    ipLineEdit->setPlaceholderText("Enter IP Address");
    connectButton = new QPushButton("&Connect");
    clearButton = new QPushButton("C&lear");
    resetButton = new QPushButton("\u21bb &Reset");
    rebootButton = new QPushButton("\u21b6 Re&boot");
    disconnectButton = new QPushButton("&Disconnect");
    exitButton = new QPushButton("\u23fb E&xit");

    connectLayout->addWidget(ipLabel);
    connectLayout->addWidget(ipLineEdit);
    connectLayout->addWidget(connectButton);
    connectLayout->addWidget(clearButton);
    connectLayout->addWidget(resetButton);
    connectLayout->addWidget(rebootButton);
    connectLayout->addWidget(disconnectButton);
    connectLayout->addWidget(exitButton);
    //  ... 추가 GUI 구성 ...

    connect(connectButton, &QPushButton::clicked, this, &QCAENV2740Single::connectDAQ);
    connect(clearButton, &QPushButton::clicked, this, &QCAENV2740Single::clearDAQ);
    connect(resetButton, &QPushButton::clicked, this, &QCAENV2740Single::resetDAQ);
    connect(rebootButton, &QPushButton::clicked, this, &QCAENV2740Single::rebootDAQ);
    connect(disconnectButton, &QPushButton::clicked, this, &QCAENV2740Single::disconnectDAQ);
    connect(exitButton, &QPushButton::clicked, this, &QCAENV2740Single::exitDAQ);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Parameter Layout
    QHBoxLayout *parameterLayout = new QHBoxLayout();
    layout->addLayout(parameterLayout);

    QLabel *parameterLabel = new QLabel("Parameter:");
    parameterLineEdit = new QLineEdit();
    parameterLineEdit->setPlaceholderText("Enter Parameter File Path");
    loadButton = new QPushButton("&Load");
    viewButton = new QPushButton("&View");
    applyButton = new QPushButton("&Apply");

    parameterLayout->addWidget(parameterLabel);
    parameterLayout->addWidget(parameterLineEdit);
    parameterLayout->addWidget(loadButton);
    parameterLayout->addWidget(viewButton);
    parameterLayout->addWidget(applyButton);

    connect(loadButton, &QPushButton::clicked, this, &QCAENV2740Single::loadParameter);
    connect(viewButton, &QPushButton::clicked, this, &QCAENV2740Single::viewParameter);
    connect(applyButton, &QPushButton::clicked, this, &QCAENV2740Single::applyParameter);
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

    connect(runButton, &QPushButton::clicked, this, &QCAENV2740Single::runDAQ);
    connect(runNSButton, &QPushButton::clicked, this, &QCAENV2740Single::runNSDAQ);
    connect(stopButton, &QPushButton::clicked, this, &QCAENV2740Single::stopDAQ);
    //////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    // Digitizer Layout
    QFrame *digitizerFrame = new QFrame();
    digitizerFrame->setFrameShape(QFrame::Box);
    digitizerFrame->setFrameShadow(QFrame::Sunken);
    digitizerFrame->setLineWidth(1);
    QVBoxLayout *digitizerLayout = new QVBoxLayout(digitizerFrame);
    layout->addWidget(digitizerFrame);

    QHBoxLayout *digitizerSettingsLayout = new QHBoxLayout();
    QLabel *digitizerLabel = new QLabel("Digitizer Settings");
    applySettingsCheckBox = new QCheckBox("Apply Settings");
    digitizerSettingsLayout->addWidget(digitizerLabel);
    digitizerSettingsLayout->addWidget(applySettingsCheckBox);
    digitizerLayout->addLayout(digitizerSettingsLayout);

    digitizerCHEnableGroupBox = new QGroupBox("CH Enable");
    QVBoxLayout *digitizerCHEnableLayout = new QVBoxLayout(digitizerCHEnableGroupBox);
    digitizerLayout->addWidget(digitizerCHEnableGroupBox);

    for (int i = 0; i < 4; i++) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        for (int j = 0; j < 16; j++) {
            QCheckBox *checkBox = new QCheckBox();
            checkBox->setText(QString("%1").arg(16 * i + j, 2, 10, QChar('0')));
            rowLayout->addWidget(checkBox);
            checkBoxes.append(checkBox);
        }
        digitizerCHEnableLayout->addLayout(rowLayout);
    }

    triggerSettingsGroupBox = new QGroupBox("Trigger Settings");
    QHBoxLayout *triggerSettingsLayout = new QHBoxLayout(triggerSettingsGroupBox);
    digitizerLayout->addWidget(triggerSettingsGroupBox);

    QLabel *startSourceLabel = new QLabel("Start Source:");
    QComboBox *startSourceComboBox = new QComboBox();
    startSourceComboBox->addItem("EncodedClkIn");
    startSourceComboBox->addItem("SINlevel");
    startSourceComboBox->addItem("SINedge");
    startSourceComboBox->addItem("SWcmd");
    startSourceComboBox->addItem("LVDS");
    startSourceComboBox->addItem("P0");
    startSourceComboBox->setCurrentText("SWcmd");
    triggerSettingsLayout->addWidget(startSourceLabel);
    triggerSettingsLayout->addWidget(startSourceComboBox);

    QLabel *globalTrigSourceLabel = new QLabel("Global Trigger Source:");
    QComboBox *globalTrigSourceComboBox = new QComboBox();
    globalTrigSourceComboBox->addItem("TrgIn");
    globalTrigSourceComboBox->addItem("P0");
    globalTrigSourceComboBox->addItem("SwTrg");
    globalTrigSourceComboBox->addItem("LVDS");
    globalTrigSourceComboBox->addItem("ITLA");
    globalTrigSourceComboBox->addItem("ITLB");
    globalTrigSourceComboBox->addItem("ITLA_AND_ITLB");
    globalTrigSourceComboBox->addItem("ITLA_OR_ITLB");
    globalTrigSourceComboBox->addItem("EncodedClkIn");
    globalTrigSourceComboBox->addItem("GPIO");
    globalTrigSourceComboBox->addItem("TestPulse");
    globalTrigSourceComboBox->addItem("UserTrg");
    globalTrigSourceComboBox->setCurrentText("SwTrg");

    triggerSettingsLayout->addWidget(globalTrigSourceLabel);
    triggerSettingsLayout->addWidget(globalTrigSourceComboBox);

    connect(applySettingsCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        bool isChecked = state == Qt::Checked;
        digitizerCHEnableGroupBox->setEnabled(isChecked);
        triggerSettingsGroupBox->setEnabled(isChecked);
    });

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
    connect(this, &QCAENV2740Single::statusUpdated, statusLabel, &QLabel::setText);
    connect(thread, &DataAcquisitionThread::updateMeasurementTime, this, [this](qint64 time) {
        elapsedTimeLabel->setText(QString("Elapsed Time: %1 s").arg(time / 1000., 0, 'f', 1));
    });
    connect(thread, &DataAcquisitionThread::updateBytesPerSecond, this, [this](float bps) {
        bytesPerSecondLabel->setText(QString("%1 kB/s").arg(bps / 1024, 0, 'f', 1));
        bpsProgressBar->setValue(static_cast<int>(bps / 1024));  // 게이지 업데이트
    });

    QHBoxLayout *statusLayout2 = new QHBoxLayout();
    layout->addLayout(statusLayout2);
    filenameLabel = new QLabel("Saving to file: ");
    fileSizeLabel = new QLabel("File Size: 0 kBytes");
    statusLayout2->addWidget(filenameLabel);
    statusLayout2->addWidget(fileSizeLabel);
    // 초기 상태 설정
    setStatus(0);
    //////////////////////////////////////////////////////////////
}

void QCAENV2740Single::initDAQ() {
    daq = new CAENV2740();
    daq->setVerbose(verbose);

    par = new CAENV2740Par();

    thread = new DataAcquisitionThread(daq);
}

void QCAENV2740Single::connectDAQ() {
    if (verbose) std::cout << "connectDAQ" << std::endl;
    if (verbose) std::cout << "currentStatus: " << currentStatus << std::endl;
    if (currentStatus != 0) return;  // 현재 상태가 0(Disconnected)이 아니면 실행하지 않음
    try {
        daq->connect("dig2://" + ipLineEdit->text().toStdString());
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", "연결 실패");
        return;
    }
    setStatus(1);
}

void QCAENV2740Single::clearDAQ() { daq->clear(); }

void QCAENV2740Single::resetDAQ() { daq->reset(); }

void QCAENV2740Single::rebootDAQ() { daq->reboot(); }

void QCAENV2740Single::disconnectDAQ() {
    if (verbose) std::cout << "disconnectDAQ" << std::endl;
    if (verbose) std::cout << "currentStatus: " << currentStatus << std::endl;
    daq->close();
    setStatus(0);
}

void QCAENV2740Single::exitDAQ() {
    if (verbose) std::cout << "exitDAQ" << std::endl;
    if (verbose) std::cout << "currentStatus: " << currentStatus << std::endl;
    if (currentStatus != 0) daq->close();
    this->close();
}

void QCAENV2740Single::loadParameter() {
    // 파일 다이얼로그를 열어 파일 경로 선택
    QString filePath = QFileDialog::getOpenFileName(this, "Select Parameter File", "",
                                                    "YAML Files (*.yml *.yaml);;JSON Files (*.json)");
    if (!filePath.isEmpty()) {
        try {
            par->loadConfigFile(filePath.toStdString());
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", "파일이 적절한 yaml 형식이 아닙니다.");
            filePath = "";
        }
        parameterLineEdit->setText(filePath);
    }
}

void QCAENV2740Single::viewParameter() {
    if (par->getConfig().empty()) {
        QMessageBox::warning(this, "Warning", "No parameter file loaded.");
        return;
    }
    QWidget *window = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    QTreeWidget *treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(2);
    treeWidget->setHeaderLabels(QStringList() << "key" << "value");
    treeWidget->setAlternatingRowColors(true);
    loadYamlToTreeWidget(par->getConfig(), treeWidget);
    layout->addWidget(treeWidget);
    window->setLayout(layout);
    window->setWindowTitle("파라미터 보기");
    window->show();
}

void QCAENV2740Single::loadYamlToTreeWidget(ryml::ConstNodeRef rootNode, QTreeWidget *treeWidget) {
    for (const auto &node : rootNode) {
        QTreeWidgetItem *item = new QTreeWidgetItem(
            treeWidget,
            QStringList() << QString::fromLocal8Bit(node.key().data(), node.key().size())
                          << (node.has_val() ? QString::fromLocal8Bit(node.val().data(), node.val().size()) : ""));
        if (node.is_map()) {
            for (const auto &subNode : node) {
                QString value;
                if (subNode.has_val()) {
                    value = QString::fromLocal8Bit(subNode.val().data(), subNode.val().size());
                } else if (subNode.is_seq()) {
                    value = "[";
                    for (const auto &subSubNode : subNode) {
                        value += QString::fromLocal8Bit(subSubNode.val().data(), subSubNode.val().size()) + ",";
                    }
                    value = value.left(value.length() - 1) + "]";
                } else {
                    value = "";
                }
                QTreeWidgetItem *subItem = new QTreeWidgetItem(
                    item, QStringList() << QString::fromLocal8Bit(subNode.key().data(), subNode.key().size()) << value);
            }
        }
        if (node.is_seq()) {
            for (const auto &subNode : node) {
                std::string number;
                subNode["number"] >> number;
                QTreeWidgetItem *subItem = new QTreeWidgetItem(item, QStringList() << QString::fromStdString(number));
                for (const auto &subSubNode : subNode) {
                    QString value;
                    if (subNode.has_val()) {
                        value = QString::fromLocal8Bit(subNode.val().data(), subNode.val().size());
                    } else if (subNode.is_seq()) {
                        value = "[";
                        for (const auto &subSubNode : subNode) {
                            value += QString::fromLocal8Bit(subSubNode.val().data(), subSubNode.val().size()) + ",";
                        }
                        value = value.left(value.length() - 1) + "]";
                    } else {
                        value = "";
                    }
                    QTreeWidgetItem *subSubItem = new QTreeWidgetItem(
                        subItem, QStringList()
                                     << QString::fromLocal8Bit(subSubNode.key().data(), subSubNode.key().size())
                                     << value);
                }
            }
        }
    }
}

void QCAENV2740Single::applyParameter() { daq->loadParameter(*par); }

void QCAENV2740Single::runNSDAQ() {
    if (currentStatus != 1) return;
    nosave = true;  // 현재 상태가 1(Stopped)이 아니면 실행하지 않음
    runDAQ();
}

void QCAENV2740Single::runDAQ() {
    if (currentStatus != 1) return;  // 현재 상태가 1(Stopped)이 아니면 실행하지 않음

    std::string runName = runNameLineEdit->text().isEmpty() ? "run" : runNameLineEdit->text().toStdString();
    int runNumber = runNumberSpinBox->value();
    std::string fileName = runName + QString("%1").arg(runNumber, 4, 10, QChar('0')).toStdString() + ".dat";
    std::ifstream fileCheck(fileName);
    if (!nosave && fileCheck.is_open()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this, "파일 존재 확인",
            QString("파일 %1이 이미 존재합니다. 덮어씌우시겠습니까?").arg(QString::fromStdString(fileName)),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    applySettings();

    if (nosave) {
        filenameLabel->setText("Not saving to file");
    } else {
        filenameLabel->setText("Saving to file: " + QFileInfo(QString::fromStdString(fileName)).absoluteFilePath());
        fout.open(fileName, std::ios::binary);
    }
    fileSizeLabel->setText("File Size: - kBytes");

    // 측정 시간이 0보다 큰 경우 QTimer 설정
    if (measurementTimeSpinBox->value() > 0) connect(timer, &QTimer::timeout, this, &QCAENV2740Single::stopDAQ);

    if (nosave)
        thread->setFout(nullptr);
    else
        thread->setFout(&fout);
    if (shm) thread->setShm(shm);

    daq->clear();
    daq->armAcquisition();
    daq->startAcquisition();
    if (measurementTimeSpinBox->value() > 0) timer->start(measurementTimeSpinBox->value() * 1000);  // 밀리초로 변환
    if (verbose) std::cout << "runDAQ" << std::endl;

    thread->start();
    setStatus(2);
}

void QCAENV2740Single::stopDAQ() {
    if (currentStatus != 2) return;  // 현재 상태가 2(Running)이 아니면 실행하지 않음
    daq->stopAcquisition();
    daq->disarmAcquisition();
    if (verbose) std::cout << "stopDAQ" << std::endl;
    thread->requestInterruption();  // 종료 신호 전송
    thread->quit();
    thread->wait();

    if (autoIncCheckBox->isChecked()) runNumberSpinBox->setValue(runNumberSpinBox->value() + 1);
    bytesPerSecondLabel->setText("0 Bytes/s");
    bpsProgressBar->setValue(0);

    fileSizeLabel->setText(QString("File Size: %1 kBytes").arg(fout.tellp() / 1024));
    fout.close();

    setStatus(1);

    if (timer) {  // 타이머가 존재할 경우 정지
        timer->stop();
    }

    nosave = false;
}

void QCAENV2740Single::setStatus(int status) {
    currentStatus = status;
    QString statusText;
    // 상태에 따라 입력 필드 및 버튼 활성화/비활성화
    bool isRunning = (currentStatus == 2);
    bool isDisconnected = (currentStatus == 0);
    bool isConnected = (currentStatus == 1);

    ipLineEdit->setEnabled(!isRunning);
    parameterLineEdit->setEnabled(!isRunning);
    runNameLineEdit->setEnabled(!isRunning);
    runNumberSpinBox->setEnabled(!isRunning);
    autoIncCheckBox->setEnabled(!isRunning);
    measurementTimeSpinBox->setEnabled(!isRunning);

    connectButton->setEnabled(isDisconnected);  // Disconnected일 때만 활성화
    clearButton->setEnabled(!isRunning && isConnected);
    resetButton->setEnabled(!isRunning && isConnected);
    rebootButton->setEnabled(!isRunning && isConnected);
    disconnectButton->setEnabled(!isRunning && isConnected);
    exitButton->setEnabled(!isRunning);  // Disconnected일 때만 활성화
    // loadButton->setEnabled(!isRunning && !isConnected);
    // viewButton->setEnabled(!isRunning && !isConnected);
    applyButton->setEnabled(!isRunning && isConnected);
    runButton->setEnabled(!isRunning && isConnected);
    runNSButton->setEnabled(!isRunning && isConnected);
    stopButton->setEnabled(isRunning);

    applySettingsCheckBox->setEnabled(!isRunning && isConnected);
    digitizerCHEnableGroupBox->setEnabled(applySettingsCheckBox->isChecked() && isConnected && !isRunning);
    triggerSettingsGroupBox->setEnabled(applySettingsCheckBox->isChecked() && isConnected && !isRunning);

    switch (currentStatus) {
        case 0:
            statusText = "Status: Disconnected";
            break;
        case 1:
            statusText = "Status: Stopped";
            break;
        case 2:
            statusText = "Status: Running";
            break;
        case 3:
            statusText = "Status: Error";
            break;
        default:
            statusText = "Status: Unknown";
            break;
    }
    if (verbose) std::cout << "statusText: " << statusText.toStdString() << std::endl;
    emit statusUpdated(statusText);  // 상태가 변경될 때 시그널 발송
}

void QCAENV2740Single::applySettings() {
    if (!applySettingsCheckBox->isChecked()) return;
    for (int i = 0; i < 64; i++) daq->writeChEnable(i, checkBoxes[i]->isChecked());
}