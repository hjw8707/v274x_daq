#ifndef QCAENV2740_H
#define QCAENV2740_H

#include <fstream>

#include "CAENV2740.hxx"  // CAENV2740 클래스 포함
#include "CAENV2740Par.hxx"
#include "QBufferedFileWriter.hxx"
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
class DataAcquisitionThreadSingle : public QThread {
    Q_OBJECT
   public:
    /**
     * @brief This is the constructor of the DataAcquisitionThread class.
     * @param daq The CAENV2740 object.
     */
    DataAcquisitionThreadSingle(CAENV2740 *daq, int _boardNumber, QString _boardName)
        : daq(daq), boardNumber(_boardNumber), boardName(_boardName) {
        writer = QBufferedFileWriter::getInstance();
    }
    void run() override;

   signals:
    void dataAcquired(uint8_t *data, size_t size);
    void updateBoardBps(int board, float bps);  // 바이트 수 업데이트 시그널
    void updateBoardTotalBytes(int board, uint64_t bytes);
    void updateMeasurementTime(qint64 time);  // 측정 시간 업데이트 시그널

   private:
    CAENV2740 *daq;
    int boardNumber;
    QString boardName;
    QBufferedFileWriter *writer;
};

/**
 * @brief This class is the QCAENV2740 class.
 * @details It is responsible for the main window of the application.
 */
class QCAENV2740 : public QWidget {
    Q_OBJECT

   signals:
    void statusUpdated(const QString &status);  // 상태 업데이트 시그널
    void removeDigitizer(QCAENV2740 *digitizer);

   public:
    QCAENV2740(const char *ip, int _boardNumber = 0, QString _boardName = "", QWidget *parent = nullptr);
    virtual ~QCAENV2740();  // 가상 소멸자 추가

    const std::string getIPAddress() const { return ipAddress.toStdString(); }
    const QString getBoardName() const { return boardName; }

    DataAcquisitionThreadSingle *getThread() const { return thread; }

    void readyDAQ(QString fileName, bool nosave = false);
    void runDAQ();
    void stopDAQ();

   protected:
    void closeEvent(QCloseEvent *event) override;

   private:
    int boardNumber;
    QString boardName;

    QString ipAddress;
    QString model;

    int currentStatus;
    uint32_t prev_aggregate_counter;

    CAENV2740 *daq;     // CAENV2740 객체
    CAENV2740Par *par;  // CAENV2740 Parameter 객체

    DataAcquisitionThreadSingle *thread;

    void initUI();
    void initDAQ();

    void connectDAQ();
    void clearDAQ();
    void resetDAQ();
    void rebootDAQ();
    void disconnectDAQ();

    void loadParameter();
    void viewParameter();
    void applyParameter();

    void loadYamlToTreeWidget(ryml::ConstNodeRef rootNode, QTreeWidget *treeWidget);

    void handleData(uint8_t *data, size_t size);

    void setStatus(int status);

    void applySettings();

    // 위젯
    QLineEdit *parameterLineEdit;

    QCheckBox *applySettingsCheckBox;
    QGroupBox *digitizerCHEnableGroupBox;
    QGroupBox *triggerSettingsGroupBox;
    // QCheckBox를 QList로 관리하여 동적으로 생성
    QList<QCheckBox *> checkBoxes;

    QPushButton *clearButton;
    QPushButton *resetButton;
    QPushButton *rebootButton;
    QPushButton *disconnectButton;
    QPushButton *loadButton;
    QPushButton *viewButton;
    QPushButton *applyButton;
};

#endif  // MAINWINDOW_H