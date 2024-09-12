#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
class DataAcquisitionThread : public QThread {
    Q_OBJECT
   public:
    DataAcquisitionThread(CAENV2740 *daq) : daq(daq), fout(nullptr) {}
    void setFout(std::ofstream *fout) { this->fout = fout; }

    void run() override {
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
                    nbunch++;
                    break;
                case CAEN_FELib_Timeout:
                    return;  // break;
                case CAEN_FELib_Stop:
                    return;
                default:
                    // 데이터 읽기 실패
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

   signals:
    void dataAcquired(uint8_t *data, size_t size);
    void updateBytesPerSecond(float bps);     // 바이트 수 업데이트 시그널
    void updateMeasurementTime(qint64 time);  // 측정 시간 업데이트 시그널

   private:
    CAENV2740 *daq;
    std::ofstream *fout;
};
class QCAENV2740 : public QMainWindow {
    Q_OBJECT

   signals:
    void statusUpdated(const QString &status);

   public:
    QCAENV2740(QWidget *parent = nullptr);
    virtual ~QCAENV2740();  // 가상 소멸자 추가

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
};

#endif  // MAINWINDOW_H