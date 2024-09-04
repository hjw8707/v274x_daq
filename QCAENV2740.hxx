#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <fstream>

#include "CAENV2740.hxx"  // CAENV2740 클래스 포함
#include "CAENV2740Par.hxx"
#include "QtCore/QElapsedTimer"
#include "QtCore/QThread"
#include "QtCore/QTimer"
#include "QtWidgets/QCheckBox"
#include "QtWidgets/QLabel"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QSpinBox"
#include "QtWidgets/QTreeWidget"

class DataAcquisitionThread : public QThread {
    Q_OBJECT
   public:
    void run() override {
        size_t size = 0;
        uint8_t *data = new uint8_t[2621440];
        uint64_t nbunch = 0;
        size_t totalBytes = 0;  // 총 전송된 바이트 수
        QElapsedTimer timer;    // 타이머 추가
        timer.start();          // 타이머 시작
        while (!isInterruptionRequested()) {
            // int ret = daq->readDataRaw(1000, data, &size, &nevent_raw);
            //////////////////////////////////////////////////////////////
            // 테스트용
            int ret = CAEN_FELib_Success;
            size = 1;
            data[0] = nbunch;

            QThread::msleep(100);  // sleep 함수 호출
            //////////////////////////////////////////////////////////////
            switch (ret) {
                case CAEN_FELib_Success:
                    emit dataAcquired(data, size);
                    nbunch++;
                    break;
                case CAEN_FELib_Timeout:
                    std::cout << "Bunch: " << nbunch << "\n";
                    return;  // break;
                case CAEN_FELib_Stop:
                    std::cout << "Bun ch: " << nbunch << "\n";
                    return;
                default:
                    // 데이터 읽기 실패
                    break;
            }

            if (!(nbunch % 100)) {
                std::cout << "Bunch: " << nbunch << "\r";
                std::cout.flush();
            }
            totalBytes += size;  // 전송된 바이트 수 누적
            // 초당 전송된 바이트 수 업데이트
            if (timer.elapsed() >= 1000) {              // 1초마다
                emit updateBytesPerSecond(totalBytes);  // 시그널 전송
                totalBytes = 0;                         // 카운터 초기화
                timer.restart();                        // 타이머 재시작
            }
        }
        std::cout << "Bunch: " << nbunch << "\n";
    }

   signals:
    void dataAcquired(uint8_t *data, size_t size);
    void updateBytesPerSecond(size_t bytes);  // 바이트 수 업데이트 시그널
};
class QCAENV2740 : public QMainWindow {
    Q_OBJECT

   signals:
    void statusUpdated(const QString &status);

   public:
    QCAENV2740(QWidget *parent = nullptr);
    virtual ~QCAENV2740();  // 가상 소멸자 추가

   private:
    int currentStatus;

    CAENV2740 *daq;     // CAENV2740 객체
    CAENV2740Par *par;  // CAENV2740 Parameter 객체

    DataAcquisitionThread *thread;
    // QThread *thread;  // DAQ를 위한 QThread 객체

    std::ofstream fout;

    void initUI();
    void initDAQ();

    void connectDAQ();
    void clearDAQ();
    void resetDAQ();
    void rebootDAQ();
    void disconnectDAQ();
    void exitDAQ();

    void runDAQ();
    void stopDAQ();

    void rawDataAcquisition();

    void loadParameter();
    void viewParameter();
    void applyParameter();

    void loadYamlToTreeWidget(ryml::ConstNodeRef rootNode, QTreeWidget *treeWidget);

    void handleData(uint8_t *data, size_t size);

    void setStatus(int status);

    // 위젯
    QLineEdit *ipLineEdit;
    QLineEdit *parameterLineEdit;
    QLineEdit *runNameLineEdit;
    QSpinBox *runNumberSpinBox;
    QCheckBox *autoIncCheckBox;
    QSpinBox *measurementTimeSpinBox;
    QLabel *bytesPerSecondLabel;
    QLabel *statusLabel;
    QLabel *statusIconLabel;
    QTimer *timer;
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
    QPushButton *stopButton;
};

#endif  // MAINWINDOW_H