#ifndef QCAENV2740SINGLE_H
#define QCAENV2740SINGLE_H

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
 * @brief This class is the DataAcquisitionThread class.
 * @details It is responsible for acquiring and processing data in a separate thread.
 */
class DataAcquisitionThread : public QThread {
    Q_OBJECT
   public:
    /**
     * @brief This is the constructor of the DataAcquisitionThread class.
     * @param daq The CAENV2740 object.
     */
    DataAcquisitionThread(CAENV2740 *daq) : daq(daq), fout(nullptr), shm(nullptr) {}
    /**
     * @brief This function sets the output file stream.
     * @param fout The output file stream.
     */
    void setFout(std::ofstream *fout) { this->fout = fout; }
    void setShm(SharedMemory *shm) { this->shm = shm; }
    void run() override;

   signals:
    void dataAcquired(uint8_t *data, size_t size);
    void updateBytesPerSecond(float bps);     // 바이트 수 업데이트 시그널
    void updateMeasurementTime(qint64 time);  // 측정 시간 업데이트 시그널

   private:
    CAENV2740 *daq;
    std::ofstream *fout;
    SharedMemory *shm;
};

/**
 * @brief This class is the QCAENV2740 class.
 * @details It is responsible for the main window of the application.
 */
class QCAENV2740Single : public QMainWindow {
    Q_OBJECT

   signals:
    void statusUpdated(const QString &status);  // 상태 업데이트 시그널

   public:
    QCAENV2740Single(QWidget *parent = nullptr);
    virtual ~QCAENV2740Single();  // 가상 소멸자 추가

    void setIPAddress(const QString &ipAddress) { ipLineEdit->setText(ipAddress); }
    QString getIPAddress() const { return ipLineEdit->text(); }

    void connectDAQ();

   private:
    bool verbose;
    bool nosave;
    // QString ipAddress;
    int currentStatus;
    uint32_t prev_aggregate_counter;

    CAENV2740 *daq;     // CAENV2740 객체
    CAENV2740Par *par;  // CAENV2740 Parameter 객체

    DataAcquisitionThread *thread;

    SharedMemory *shm;
    std::ofstream fout;

    void initUI();
    void initDAQ();

    void clearDAQ();
    void resetDAQ();
    void rebootDAQ();
    void disconnectDAQ();
    void exitDAQ();

    void runDAQ();
    void runNSDAQ();
    void stopDAQ();

    // void rawDataAcquisition();

    void loadParameter();
    void viewParameter();
    void applyParameter();

    void loadYamlToTreeWidget(ryml::ConstNodeRef rootNode, QTreeWidget *treeWidget);

    void handleData(uint8_t *data, size_t size);

    void setStatus(int status);

    void applySettings();

    void startMonitoring();

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