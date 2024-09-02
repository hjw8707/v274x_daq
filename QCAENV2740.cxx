#include "QCAENV2740.hxx"

#include "QtCore/QEvent"
#include "QtCore/QObject"
#include "QtGui/QKeyEvent"
#include "QtWidgets/QApplication"
#include "QtWidgets/QFileDialog"
#include "QtWidgets/QHBoxLayout"
#include "QtWidgets/QLabel"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QMessageBox"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QTreeWidget"
#include "QtWidgets/QTreeWidgetItem"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QWidget"

QCAENV2740::QCAENV2740(QWidget *parent) : QMainWindow(parent) {
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
}

void QCAENV2740::initUI() {
    // GUI 구성 요소 설정
    setWindowTitle("V274X DAQ");
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
    QPushButton *connectButton = new QPushButton("&Connect");
    QPushButton *clearButton = new QPushButton("C&lear");
    QPushButton *resetButton = new QPushButton("\u21bb &Reset");
    QPushButton *rebootButton = new QPushButton("\u21b6 Re&boot");
    QPushButton *disconnectButton = new QPushButton("&Disconnect");
    QPushButton *exitButton = new QPushButton("\u23fb E&xit");

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
    QPushButton *loadButton = new QPushButton("&Load");
    QPushButton *viewButton = new QPushButton("&View");
    QPushButton *applyButton = new QPushButton("&Apply");

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

    QLabel *runLabel = new QLabel("Run:");
    QLineEdit *runLineEdit = new QLineEdit();
    runLineEdit->setPlaceholderText("Enter Run Number");
    QPushButton *runButton = new QPushButton("&Run");
    QPushButton *stopButton = new QPushButton("&Stop");
    QPushButton *pauseButton = new QPushButton("&Pause");
    QPushButton *resumeButton = new QPushButton("&Resume");

    runLayout->addWidget(runButton);
    runLayout->addWidget(stopButton);
    runLayout->addWidget(pauseButton);
    runLayout->addWidget(resumeButton);
    //////////////////////////////////////////////////////////////
}

void QCAENV2740::initDAQ() {
    daq = new CAENV2740();
    daq->setVerbose(true);

    par = new CAENV2740Par();
}

void QCAENV2740::connectDAQ() { daq->connect("dig2://" + ipLineEdit->text().toStdString()); }

void QCAENV2740::clearDAQ() { daq->clear(); }

void QCAENV2740::resetDAQ() { daq->reset(); }

void QCAENV2740::rebootDAQ() { daq->reboot(); }

void QCAENV2740::disconnectDAQ() { daq->close(); }

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
