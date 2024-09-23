#include "QCAENV274XMulti.hxx"

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
QCAENV274XMulti::QCAENV274XMulti(QWidget *parent) : verbose(false), currentStatus(-1), QMainWindow(parent) {
    setWindowTitle("V274X Multi DAQ");
    setWindowIcon(QIcon("icons/dig_v2740.png"));

    timer = new QTimer(this);
    verbose = true;

    // GUI 초기화
    initUI();
}

QCAENV274XMulti::~QCAENV274XMulti() {
    // 자원 해제 코드
    if (timer) {
        delete timer;
        timer = nullptr;
    }
}

void QCAENV274XMulti::initUI() {
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
    exitButton = new QPushButton("\u23fb E&xit");

    connectLayout->addWidget(ipLabel);
    connectLayout->addWidget(ipLineEdit);
    connectLayout->addWidget(connectButton);
    connectLayout->addWidget(exitButton);
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

    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Digitizer Layout
    QFrame *digitizerFrame = new QFrame();
    digitizerFrame->setFrameShape(QFrame::Box);
    digitizerFrame->setFrameShadow(QFrame::Sunken);
    digitizerFrame->setLineWidth(1);
    QVBoxLayout *digitizerLayout = new QVBoxLayout(digitizerFrame);
    layout->addWidget(digitizerFrame);

    QLabel *digitizerLabel = new QLabel("Digitizer Settings");
    digitizerLayout->addWidget(digitizerLabel);

    QTabWidget *digitizerTabWidget = new QTabWidget();
    digitizerLayout->addWidget(digitizerTabWidget);

    digitizerTabWidget->setTabPosition(QTabWidget::North);
    digitizerTabWidget->setTabShape(QTabWidget::Rounded);
    digitizerTabWidget->addTab(new QWidget(), "Digitizer 1");
    digitizerTabWidget->addTab(new QWidget(), "Digitizer 2");
    digitizerTabWidget->addTab(new QWidget(), "Digitizer 3");
    digitizerTabWidget->addTab(new QWidget(), "Digitizer 4");
    //////////////////////////////////////////////////////////////

    // Connect Layout
    //////////////////////////////////////////////////////////////
    // Connect Layout
    // QHBoxLayout *connectLayout = new QHBoxLayout();
    // layout->addLayout(connectLayout);

    // QLabel *ipLabel = new QLabel("IP Address:");
    // ipLineEdit = new QLineEdit();
    // ipLineEdit->setPlaceholderText("Enter IP Address");
    // connectButton = new QPushButton("&Connect");
    // clearButton = new QPushButton("C&lear");
    // resetButton = new QPushButton("\u21bb &Reset");
    // rebootButton = new QPushButton("\u21b6 Re&boot");
    // disconnectButton = new QPushButton("&Disconnect");
    // exitButton = new QPushButton("\u23fb E&xit");

    // connectLayout->addWidget(ipLabel);
    // connectLayout->addWidget(ipLineEdit);
    // connectLayout->addWidget(connectButton);
    // connectLayout->addWidget(clearButton);
    // connectLayout->addWidget(resetButton);
    // connectLayout->addWidget(rebootButton);
    // connectLayout->addWidget(disconnectButton);
    // connectLayout->addWidget(exitButton);
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

    //////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////
    // Digitizer Layout
    // QFrame *digitizerFrame = new QFrame();
    // digitizerFrame->setFrameShape(QFrame::Box);
    // digitizerFrame->setFrameShadow(QFrame::Sunken);
    // digitizerFrame->setLineWidth(1);
    // QVBoxLayout *digitizerLayout = new QVBoxLayout(digitizerFrame);
    // layout->addWidget(digitizerFrame);

    // QHBoxLayout *digitizerSettingsLayout = new QHBoxLayout();
    // QLabel *digitizerLabel = new QLabel("Digitizer Settings");
    // applySettingsCheckBox = new QCheckBox("Apply Settings");
    // digitizerSettingsLayout->addWidget(digitizerLabel);
    // digitizerSettingsLayout->addWidget(applySettingsCheckBox);
    // digitizerLayout->addLayout(digitizerSettingsLayout);

    // digitizerCHEnableGroupBox = new QGroupBox("CH Enable");
    // QVBoxLayout *digitizerCHEnableLayout = new QVBoxLayout(digitizerCHEnableGroupBox);
    // digitizerLayout->addWidget(digitizerCHEnableGroupBox);

    // for (int i = 0; i < 4; i++) {
    //     QHBoxLayout *rowLayout = new QHBoxLayout();
    //     for (int j = 0; j < 16; j++) {
    //         QCheckBox *checkBox = new QCheckBox();
    //         checkBox->setText(QString("%1").arg(16 * i + j, 2, 10, QChar('0')));
    //         rowLayout->addWidget(checkBox);
    //         checkBoxes.append(checkBox);
    //     }
    //     digitizerCHEnableLayout->addLayout(rowLayout);
    // }

    // triggerSettingsGroupBox = new QGroupBox("Trigger Settings");
    // QHBoxLayout *triggerSettingsLayout = new QHBoxLayout(triggerSettingsGroupBox);
    // digitizerLayout->addWidget(triggerSettingsGroupBox);

    // QLabel *startSourceLabel = new QLabel("Start Source:");
    // QComboBox *startSourceComboBox = new QComboBox();
    // startSourceComboBox->addItem("EncodedClkIn");
    // startSourceComboBox->addItem("SINlevel");
    // startSourceComboBox->addItem("SINedge");
    // startSourceComboBox->addItem("SWcmd");
    // startSourceComboBox->addItem("LVDS");
    // startSourceComboBox->addItem("P0");
    // startSourceComboBox->setCurrentText("SWcmd");
    // triggerSettingsLayout->addWidget(startSourceLabel);
    // triggerSettingsLayout->addWidget(startSourceComboBox);

    // QLabel *globalTrigSourceLabel = new QLabel("Global Trigger Source:");
    // QComboBox *globalTrigSourceComboBox = new QComboBox();
    // globalTrigSourceComboBox->addItem("TrgIn");
    // globalTrigSourceComboBox->addItem("P0");
    // globalTrigSourceComboBox->addItem("SwTrg");
    // globalTrigSourceComboBox->addItem("LVDS");
    // globalTrigSourceComboBox->addItem("ITLA");
    // globalTrigSourceComboBox->addItem("ITLB");
    // globalTrigSourceComboBox->addItem("ITLA_AND_ITLB");
    // globalTrigSourceComboBox->addItem("ITLA_OR_ITLB");
    // globalTrigSourceComboBox->addItem("EncodedClkIn");
    // globalTrigSourceComboBox->addItem("GPIO");
    // globalTrigSourceComboBox->addItem("TestPulse");
    // globalTrigSourceComboBox->addItem("UserTrg");
    // globalTrigSourceComboBox->setCurrentText("SwTrg");

    // triggerSettingsLayout->addWidget(globalTrigSourceLabel);
    // triggerSettingsLayout->addWidget(globalTrigSourceComboBox);

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
    //////////////////////////////////////////////////////////////
}
