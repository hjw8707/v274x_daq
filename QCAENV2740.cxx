#include "QCAENV2740.hxx"

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
// DataAcquisitionThread
//
// @brief 데이터 수집 스레드
// @details 데이터 수집 스레드는 데이터를 수집하고 처리하는 스레드입니다.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief This function is the run method of the DataAcquisitionThread class.
 * @details It is responsible for running the data acquisition thread.
 */
void DataAcquisitionThreadSingle::run() {
    size_t size = 0;
    uint8_t *data = new uint8_t[2621440];
    uint32_t nevent_raw = 0;
    uint64_t nbunch = 0;
    size_t totalBytes = 0;  // 총 전송된 바이트 수
    qint64 totalTime = 0;   // 총 측정 시간
    QElapsedTimer timer;
    timer.start();
    while (!isInterruptionRequested()) {
        int ret = daq->readDataRaw(1000, data, &size, &nevent_raw);

        switch (ret) {
            case CAEN_FELib_Success:
                writer->write(boardName, reinterpret_cast<char *>(data), size);
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
        if (timer.elapsed() >= 1000) {
            totalTime += timer.elapsed();  // 1초마다
            emit updateBoardBps(
                boardNumber,
                totalBytes * 1000. / timer.elapsed());  // 시그널 전송                      // 측정 시간 업데이트
            totalBytes = 0;   // 카운터 초기화                                  // 측정 시간 업데이트
            timer.restart();  // 타이머 재시작
        }
        totalBytes += size;  // 전송된 바이트 수 누적
    }
    emit updateBoardTotalBytes(boardNumber, writer->getFileSize(boardName));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// QCAENV2740
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
QCAENV2740::QCAENV2740(const char *ipAddress, int _boardNumber, QString _boardName, QWidget *parent)
    : currentStatus(-1), boardNumber(_boardNumber), boardName(_boardName), QWidget(parent) {
    qDebug() << "QCAENV2740 constructor";
    this->ipAddress = QString(ipAddress);

    if (boardName.isEmpty()) boardName = QString("%1").arg(boardNumber, 2, 10, QChar('0'));

    //  CAENV2740 초기화
    initDAQ();
    // GUI 초기화
    initUI();

    // shm = new SharedMemory("/" + model.toStdString() + "_" + QString::number(boardNumber).toStdString());
}

QCAENV2740::~QCAENV2740() {
    qDebug() << "QCAENV2740 destructor";
    // 자원 해제 코드
    if (daq) {
        delete daq;
        daq = nullptr;
    }
    if (par) {
        delete par;
        par = nullptr;
    }
}

void QCAENV2740::initUI() {
    qDebug() << "QCAENV2740::initUI()";
    // GUI 구성 요소 설정
    resize(800, 600);
    QVBoxLayout *layout = new QVBoxLayout(this);

    //////////////////////////////////////////////////////////////
    // Connect Layout
    QHBoxLayout *connectLayout = new QHBoxLayout();
    layout->addLayout(connectLayout);

    QLabel *ipLabel = new QLabel(QString("IP Address: %1").arg(ipAddress));
    QLabel *modelLabel = new QLabel(QString("Model: %1").arg(model));
    clearButton = new QPushButton("C&lear");
    resetButton = new QPushButton("\u21bb &Reset");
    rebootButton = new QPushButton("\u21b6 Re&boot");
    disconnectButton = new QPushButton("&Disconnect");

    connectLayout->addWidget(ipLabel);
    connectLayout->addWidget(modelLabel);

    connectLayout->addWidget(clearButton);
    connectLayout->addWidget(resetButton);
    connectLayout->addWidget(rebootButton);
    connectLayout->addWidget(disconnectButton);
    //  ... 추가 GUI 구성 ...

    connect(clearButton, &QPushButton::clicked, this, &QCAENV2740::clearDAQ);
    connect(resetButton, &QPushButton::clicked, this, &QCAENV2740::resetDAQ);
    connect(rebootButton, &QPushButton::clicked, this, &QCAENV2740::rebootDAQ);
    connect(disconnectButton, &QPushButton::clicked, this, &QCAENV2740::disconnectDAQ);
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

    // 초기 상태 설정
    // setStatus(0);
    setStatus(1);
    //////////////////////////////////////////////////////////////
}

void QCAENV2740::initDAQ() {
    qDebug() << "QCAENV2740::initDAQ()";

    daq = new CAENV2740();
    daq->setVerbose(true);

    connectDAQ();
    model = QString::fromStdString(daq->readModelName());

    par = new CAENV2740Par();

    thread = new DataAcquisitionThreadSingle(daq, boardNumber, boardName);
}

void QCAENV2740::connectDAQ() {
    qDebug() << "QCAENV2740::connectDAQ()";
    try {
        QString connectStr = "dig2://" + ipAddress;
        daq->connect(connectStr.toStdString());
        qDebug() << "Connected to DAQ";
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", "연결 실패");
        qDebug() << "Failed to connect to DAQ";
        return;
    }
}

void QCAENV2740::clearDAQ() {
    qDebug() << "QCAENV2740::clearDAQ()";
    daq->clear();
}

void QCAENV2740::resetDAQ() {
    qDebug() << "QCAENV2740::resetDAQ()";
    daq->reset();
}

void QCAENV2740::rebootDAQ() {
    qDebug() << "QCAENV2740::rebootDAQ()";
    daq->reboot();
}

void QCAENV2740::disconnectDAQ() {
    qDebug() << "QCAENV2740::disconnectDAQ()";
    daq->close();
    setStatus(0);

    emit removeDigitizer(this);
}

void QCAENV2740::closeEvent(QCloseEvent *event) {
    disconnectDAQ();
    event->accept();
}

void QCAENV2740::loadParameter() {
    qDebug() << "QCAENV2740::loadParameter()";
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

void QCAENV2740::viewParameter() {
    qDebug() << "QCAENV2740::viewParameter()";
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
    qDebug() << "QCAENV2740::loadYamlToTreeWidget()";
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

void QCAENV2740::applyParameter() {
    qDebug() << "QCAENV2740::applyParameter()";
    daq->loadParameter(*par);
}

void QCAENV2740::readyDAQ(QString fileName, bool nosave) {
    qDebug() << "QCAENV2740::readyDAQ()";
    applySettings();
    fileName += QString("_%1").arg(boardName);

    daq->clear();
    daq->armAcquisition();
}

void QCAENV2740::runDAQ() {
    qDebug() << "QCAENV2740::runDAQ()";
    daq->startAcquisition();
    thread->start();
    setStatus(2);
}

void QCAENV2740::stopDAQ() {
    qDebug() << "QCAENV2740::stopDAQ()";
    daq->stopAcquisition();
    daq->disarmAcquisition();
    thread->requestInterruption();  // 종료 신호 전송
    thread->quit();
    thread->wait();

    setStatus(1);
}

void QCAENV2740::setStatus(int status) {
    qDebug() << "QCAENV2740::setStatus()" << status;
    currentStatus = status;
    QString statusText;
    // 상태에 따라 입력 필드 및 버튼 활성화/비활성화
    bool isRunning = (currentStatus == 2);
    bool isDisconnected = (currentStatus == 0);
    bool isConnected = (currentStatus == 1);

    parameterLineEdit->setEnabled(!isRunning);

    // connectButton->setEnabled(isDisconnected);  // Disconnected일 때만 활성화
    clearButton->setEnabled(!isRunning && isConnected);
    resetButton->setEnabled(!isRunning && isConnected);
    rebootButton->setEnabled(!isRunning && isConnected);
    disconnectButton->setEnabled(!isRunning && isConnected);
    // loadButton->setEnabled(!isRunning && !isConnected);
    // viewButton->setEnabled(!isRunning && !isConnected);
    applyButton->setEnabled(!isRunning && isConnected);

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
    // emit statusUpdated(statusText);  // 상태가 변경될 때 시그널 발송
}

void QCAENV2740::applySettings() {
    qDebug() << "QCAENV2740::applySettings()";
    if (!applySettingsCheckBox->isChecked()) return;
    for (int i = 0; i < 64; i++) daq->writeChEnable(i, checkBoxes[i]->isChecked());
}