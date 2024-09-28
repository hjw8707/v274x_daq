#ifndef QCAENV274XMULTI_H
#define QCAENV274XMULTI_H

#include <fstream>

#include "CAENV2740.hxx"  // CAENV2740 클래스 포함
#include "CAENV2740Par.hxx"
#include "QBufferedFileWriter.hxx"
#include "QCAENV2740.hxx"
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

/**
 * @brief This class is the QCAENV274XMulti class.
 * @details It is responsible for the main window of the application.
 */
class QCAENV274XMulti : public QMainWindow {
    Q_OBJECT

   public:
    QCAENV274XMulti(QWidget *parent = nullptr);
    virtual ~QCAENV274XMulti();  // 가상 소멸자 추가

   public slots:
    void addDigitizer(const QString &name, const QString &ip);  // default name = digXX
    void removeDigitizer(QCAENV2740 *digitizer);
    void updateDigitizerLabel();
    void updateBoardBps(int board, float bps);
    void updateBoardTotalBytes(int board, uint64_t bytes);

   protected:
    void closeEvent(QCloseEvent *event) override;

   private:
    void updateStatus(bool running);

    int numOfDigitizers;
    bool isRunning;

    bool nosave;

    float boardBps[100];
    uint64_t boardTotalBytes[100];

    void initUI();

    void runNS();
    void run();
    void stop();

    // 위젯
    QBufferedFileWriter *writer;
    QList<QCAENV2740 *> digitizers;
    QTabWidget *digitizerTabWidget;
    QLabel *digitizerLabel;

    QLineEdit *boardLineEdit;
    QLineEdit *ipLineEdit;
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

    QTimer *updateTimer;
    QTimer *measurementTimer;
    QElapsedTimer *elapsedTimer;

    QPushButton *connectButton;
    QPushButton *exitButton;

    QPushButton *runButton;
    QPushButton *runNSButton;
    QPushButton *stopButton;
};

#endif  // MAINWINDOW_H