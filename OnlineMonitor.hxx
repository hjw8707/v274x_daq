#include <mutex>

#include "SharedMemory.hxx"
#include "TCanvas.h"
#include "TH1F.h"
#include "THttpServer.h"
class OnlineMonitor {
   public:
    OnlineMonitor(const char* shmName);
    ~OnlineMonitor();
    inline void setVerbose(bool verbose) { this->verbose = verbose; }
    inline bool getVerbose() { return verbose; }
    void start();
    void parseData();
    void createHistogram();
    void serveHistogram();

   private:
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
    const int nCh = 64;

    SharedMemory* shm;              // 공유 메모리
    sem_t* semRead;                 // 읽기 세마포어
    std::vector<TH1F*> histograms;  // ROOT 히스토그램
    std::mutex mtx;                 // 뮤텍스
    THttpServer* httpServer;        // HTTP 서버
    uint64_t* datas;                // 데이터

    /////////////////////////
    // Event Header Parsing
    int nWords;
    int aggregate_counter;
    int flush;
    int board_good;
    int eventType;
    /////////////////////////

    bool verbose;
};