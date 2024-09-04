#include "QCAENV2740.hxx"

#include "QtCore/QObject"
#include "QtCore/QThread"
#include "QtCore/QTimer"
#include "QtGui/QIcon"
#include "QtWidgets/QApplication"
#include "QtWidgets/QCheckBox"
#include "QtWidgets/QFileDialog"
#include "QtWidgets/QHBoxLayout"
#include "QtWidgets/QLabel"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QMessageBox"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QSpinBox"
#include "QtWidgets/QStyle"
#include "QtWidgets/QTreeWidget"
#include "QtWidgets/QTreeWidgetItem"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QWidget"

QCAENV2740::QCAENV2740(QWidget *parent) : currentStatus(-1), QMainWindow(parent) {
    setWindowTitle("V274X DAQ");
    setWindowIcon(QIcon("icons/dig_v2740.png"));

    timer = new QTimer(this);

    // CAENV2740 초기화
    initDAQ();
    // GUI 초기화
    initUI();
}

QCAENV2740::~QCAENV2740() {
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
}

void QCAENV2740::initUI() {
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

    connect(connectButton, &QPushButton::clicked, this, &QCAENV2740::connectDAQ);
    connect(clearButton, &QPushButton::clicked, this, &QCAENV2740::clearDAQ);
    connect(resetButton, &QPushButton::clicked, this, &QCAENV2740::resetDAQ);
    connect(rebootButton, &QPushButton::clicked, this, &QCAENV2740::rebootDAQ);
    connect(disconnectButton, &QPushButton::clicked, this, &QCAENV2740::disconnectDAQ);
    connect(exitButton, &QPushButton::clicked, this, &QCAENV2740::exitDAQ);
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

    connect(loadButton, &QPushButton::clicked, this, &QCAENV2740::loadParameter);
    connect(viewButton, &QPushButton::clicked, this, &QCAENV2740::viewParameter);
    connect(applyButton, &QPushButton::clicked, this, &QCAENV2740::applyParameter);
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
    QLabel *measurementTimeLabel = new QLabel("Measurement Time (s):");
    measurementTimeSpinBox = new QSpinBox();
    measurementTimeSpinBox->setRange(0, 999999);
    measurementTimeSpinBox->setSingleStep(1);
    measurementTimeSpinBox->setValue(0);

    runLayout->addWidget(measurementTimeLabel);
    runLayout->addWidget(measurementTimeSpinBox);
    runButton = new QPushButton("&Run");
    stopButton = new QPushButton("&Stop");

    runLayout->addWidget(runNameLabel);
    runLayout->addWidget(runNameLineEdit);
    runLayout->addWidget(runNumberLabel);
    runLayout->addWidget(runNumberSpinBox);
    runLayout->addWidget(autoIncCheckBox);
    runLayout->addWidget(measurementTimeLabel);
    runLayout->addWidget(measurementTimeSpinBox);
    runLayout->addWidget(runButton);
    runLayout->addWidget(stopButton);

    connect(runButton, &QPushButton::clicked, this, &QCAENV2740::runDAQ);
    connect(stopButton, &QPushButton::clicked, this, &QCAENV2740::stopDAQ);
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Status Layout
    QHBoxLayout *statusLayout = new QHBoxLayout();
    layout->addLayout(statusLayout);

    statusLabel = new QLabel("Status: Stopped");
    statusIconLabel = new QLabel();
    statusIconLabel->setPixmap(style()->standardPixmap(QStyle::SP_MediaStop));

    QLabel *bpsNameLabel = new QLabel("Bytes Per Second : ");
    bytesPerSecondLabel = new QLabel("0 Bytes/s");

    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(statusIconLabel);
    statusLayout->addWidget(bpsNameLabel);
    statusLayout->addWidget(bytesPerSecondLabel);

    connect(this, &QCAENV2740::statusUpdated, statusLabel, &QLabel::setText);
    connect(thread, &DataAcquisitionThread::updateBytesPerSecond, this,
            [this](size_t bytes) { bytesPerSecondLabel->setText(QString("%1 Bytes/s").arg(bytes)); });

    // 초기 상태 설정
    setStatus(0);
    //////////////////////////////////////////////////////////////
}

void QCAENV2740::initDAQ() {
    daq = new CAENV2740();
    daq->setVerbose(true);

    par = new CAENV2740Par();

    thread = new DataAcquisitionThread();
}

void QCAENV2740::connectDAQ() {
    if (currentStatus != 0) return;  // 현재 상태가 0(Disconnected)이 아니면 실행하지 않음
    // daq->connect("dig2://" + ipLineEdit->text().toStdString());
    setStatus(1);
}

void QCAENV2740::clearDAQ() { daq->clear(); }

void QCAENV2740::resetDAQ() { daq->reset(); }

void QCAENV2740::rebootDAQ() { daq->reboot(); }

void QCAENV2740::disconnectDAQ() {
    // daq->close();
    setStatus(0);
}

void QCAENV2740::exitDAQ() {
    daq->close();
    delete daq;
    exit(0);
}

void QCAENV2740::loadParameter() {
    // 파일 다이얼로그를 열어 파일 경로 선택
    QString filePath = QFileDialog::getOpenFileName(this, "Select Parameter File", "", "All Files (*)");
    if (!filePath.isEmpty()) {
        parameterLineEdit->setText(filePath);
        par->loadConfigFile(filePath.toStdString());
    }
}

void QCAENV2740::viewParameter() {
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

void QCAENV2740::loadYamlToTreeWidget(ryml::ConstNodeRef rootNode, QTreeWidget *treeWidget) {
    for (const auto &node : rootNode) {
        QTreeWidgetItem *item = new QTreeWidgetItem(
            treeWidget,
            QStringList() << QString::fromStdString(std::string(node.key().data(), node.key().size()))
                          << (node.has_val() ? QString::fromStdString(std::string(node.val().data(), node.val().size()))
                                             : ""));
        if (node.is_map()) {
            for (const auto &subNode : node) {
                QTreeWidgetItem *subItem = new QTreeWidgetItem(
                    item,
                    QStringList() << QString::fromStdString(std::string(subNode.key().data(), subNode.key().size()))
                                  << (subNode.has_val() ? QString::fromStdString(
                                                              std::string(subNode.val().data(), subNode.val().size()))
                                                        : ""));
                item->addChild(subItem);
            }
        }
        if (node.is_seq()) {
            for (const auto &subNode : node) {
                std::string number;
                subNode["number"] >> number;
                QTreeWidgetItem *subItem = new QTreeWidgetItem(item, QStringList() << QString::fromStdString(number));
                item->addChild(subItem);
                for (const auto &subSubNode : subNode) {
                    QTreeWidgetItem *subSubItem =
                        new QTreeWidgetItem(
                            subItem,
                            QStringList()
                                << QString::fromStdString(std::string(subSubNode.key().data(), subSubNode.key().size()))
                                << (subSubNode.has_val() ? QString::fromStdString(std::string(subSubNode.val().data(),
                                                                                              subSubNode.val().size()))
                                                         : ""));
                    subItem->addChild(subSubItem);
                }
            }
        }
    }
}

void QCAENV2740::applyParameter() { daq->loadParameter(*par); }

void QCAENV2740::runDAQ() {
    if (currentStatus != 1) return;  // 현재 상태가 1(Stopped)이 아니면 실행하지 않음

    std::string runName = runNameLineEdit->text().isEmpty() ? "run" : runNameLineEdit->text().toStdString();
    int runNumber = runNumberSpinBox->value();
    std::string fileName = runName + QString("%1").arg(runNumber, 4, 10, QChar('0')).toStdString() + ".dat";
    std::ifstream fileCheck(fileName);
    if (fileCheck.is_open()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
            this, "파일 존재 확인",
            QString("파일 %1이 이미 존재합니다. 덮어씌우시겠습니까?").arg(QString::fromStdString(fileName)),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    fout.open(fileName, std::ios::binary);

    // 측정 시간이 0보다 큰 경우 QTimer 설정
    if (measurementTimeSpinBox->value() > 0) {
        connect(timer, &QTimer::timeout, this, &QCAENV2740::stopDAQ);
        timer->start(measurementTimeSpinBox->value() * 1000);  // 밀리초로 변환
    }
    // daq->clear();
    // daq->armAcquisition();
    // daq->startAcquisition();
    std::cout << "runDAQ" << std::endl;
    connect(thread, &DataAcquisitionThread::dataAcquired, this, &QCAENV2740::handleData);

    thread->start();
    setStatus(2);
}

void QCAENV2740::stopDAQ() {
    if (currentStatus != 2) return;  // 현재 상태가 2(Running)이 아니면 실행하지 않음
    // daq->stopAcquisition();
    // daq->disarmAcquisition();
    std::cout << "stopDAQ" << std::endl;
    thread->requestInterruption();  // 종료 신호 전송
    thread->quit();
    thread->wait();

    fout.close();

    if (autoIncCheckBox->isChecked()) runNumberSpinBox->setValue(runNumberSpinBox->value() + 1);
    bytesPerSecondLabel->setText("0 Bytes/s");
    setStatus(1);

    if (timer) {  // 타이머가 존재할 경우 정지
        timer->stop();
    }
}

void QCAENV2740::handleData(uint8_t *data, size_t size) { fout.write(reinterpret_cast<char *>(data), size); }

void QCAENV2740::setStatus(int status) {
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
    stopButton->setEnabled(isRunning);

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
    std::cout << "statusText: " << statusText.toStdString() << std::endl;
    emit statusUpdated(statusText);  // 상태가 변경될 때 시그널 발송
}
