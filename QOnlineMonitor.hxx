#ifndef QONLINEMONITOR_HXX
#define QONLINEMONITOR_HXX

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QObject>  // Add this include for QObject
#include <QtCore/QReadWriteLock>
#include <QtCore/QSemaphore>
#include <QtCore/QSharedMemory>
#include <QtCore/QThread>
#include <QtCore/QVector>
#include <QtCore/QWaitCondition>

#include "QBufferedFileWriter.hxx"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TH1.h"
#include "THttpServer.h"

typedef std::vector<TH1*> HistVec;

class QOnlineMonitor : public QObject {  // Change class name and inherit from QObject
    Q_OBJECT

   public:
    QOnlineMonitor(QObject* parent = nullptr);
    ~QOnlineMonitor();

    void setVerbose(bool _verbose) { verbose = _verbose; }
    bool getVerbose() { return verbose; }

    void attachSharedMemory(QString name);
    void createHistograms(QString name);
    void detachSharedMemory(QString name);
    void removeHistograms(QString name);

    void clearHistogram();
    void clearHistogram(QString name);

    uint64_t getSharedMemorySize(QString name);
    uint64_t* getSharedMemoryData(QString name, uint64_t size);
    void clearSharedMemory(QString name);

   public slots:
    void start();
    void stop();
    void run();

   private:
    void parseData(uint64_t* datas, uint64_t size);
    //@brief 헤더를 파싱하여 이벤트 타입을 반환합니다.
    //@param header 헤더
    //@return 이벤트 타입
    int parseHeader(uint64_t header);

    //@brief 이벤트 타입이 3, runEvent가 3일 때 파싱합니다.
    //@param datas 데이터
    void startRunParser(uint64_t* datas);

    //@brief 이벤트 타입이 3, runEvent가 2일 때 파싱합니다.
    //@param datas 데이터
    void stopRunParser(uint64_t* datas);

    //@brief 이벤트 타입이 2일 때 파싱합니다.
    //@param datas 데이터
    void eventParser(uint64_t* datas, int nWords);

   private:
    const int t_sleep = 100;  // 100ms
    const int nCh = 64;

    bool verbose;
    bool isRunning;
    THttpServer* httpServer;  // HTTP 서버
    QThread* thread;

    QHash<QString, QSharedMemory*> sharedMemory;
    QHash<QString, QReadWriteLock*> locks;
    QHash<QString, TCanvas*> canvases;
    QHash<QString, TDirectory*> directories;
    QHash<QString, HistVec> histograms;  // ROOT 히스토그램

    QString currentShmName;  // 현재 사용하는 공유 메모리 이름
    uint64_t* buffer;        // 데이터 버퍼

    /////////////////////////
    // Event Header Parsing
    int nWords;
    int aggregate_counter;
    int flush;
    int board_good;
    int eventType;
    /////////////////////////
};

#endif
