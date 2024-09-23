#ifndef QCAENV274XMULTI_H
#define QCAENV274XMULTI_H

#include <fstream>

#include "CAENV2740.hxx"  // CAENV2740 클래스 포함
#include "CAENV2740Par.hxx"
#include "QtCore/QElapsedTimer"
#include "QtCore/QThread"
#include "QtCore/QTimer"
#include "QtGui/QStandardItem"
#include "QtGui/QStandardItemModel"
#include "QtWidgets/QCheckBox"
#include "QtWidgets/QComboBox"
#include "QtWidgets/QGroupBox"
#include "QtWidgets/QLabel"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QListView"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QProgressBar"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QSpinBox"
#include "QtWidgets/QTreeWidget"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QWidget"
#include "SharedMemory.hxx"  // SharedMemory 클래스 포함

/**
 * @brief This class is the QCAENV274XMulti class.
 * @details It is responsible for the main window of the application.
 */
class QCAENV274XMulti : public QMainWindow {
    Q_OBJECT

    // signals:
    //  void statusUpdated(const QString &status);  // 상태 업데이트 시그널

   public:
    QCAENV274XMulti(QWidget *parent = nullptr);
    virtual ~QCAENV274XMulti();  // 가상 소멸자 추가

   private:
    bool verbose;
    int currentStatus;
    int numOfDigitizers;

    bool nosave;

    std::ofstream fout;

    void initUI();

    // 위젯
    QLineEdit *ipLineEdit;
    QLineEdit *parameterLineEdit;
    QLineEdit *runNameLineEdit;
    QSpinBox *runNumberSpinBox;
    QCheckBox *autoIncCheckBox;
    QSpinBox *measurementTimeSpinBox;
    QLabel *bytesPerSecondLabel;
    QProgressBar *bpsProgressBar;
    QLabel *statusLabel;
    QLabel *elapsedTimeLabel;
    QLabel *statusIconLabel;
    QLabel *filenameLabel;
    QLabel *fileSizeLabel;

    QTimer *timer;

    QCheckBox *applySettingsCheckBox;
    QGroupBox *digitizerCHEnableGroupBox;
    QGroupBox *triggerSettingsGroupBox;
    // QCheckBox를 QList로 관리하여 동적으로 생성
    QList<QCheckBox *> checkBoxes;

    QPushButton *connectButton;
    QPushButton *clearButton;
    QPushButton *resetButton;
    QPushButton *rebootButton;
    QPushButton *disconnectButton;
    QPushButton *exitButton;
    QPushButton *loadButton;
    QPushButton *viewButton;
    QPushButton *applyButton;
    QPushButton *runButton;
    QPushButton *runNSButton;
    QPushButton *stopButton;
    QPushButton *monitoringButton;
};

#endif  // MAINWINDOW_H