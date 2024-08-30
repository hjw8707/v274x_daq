
#include <unistd.h>

#include <csignal>
#include <fstream>
#include <iostream>

#include "CAENV2740.hxx"
#include "CAENV2740Par.hxx"

// 전역 변수로 루프 제어 플래그 선언
volatile sig_atomic_t running = 1;

// 신호 핸들러 함수 정의
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Exiting loop...\n";
    running = 0;  // 루프를 종료하기 위한 플래그 설정
}

void usage() {
    std::cerr << "사용법: ./daq [-p] <V2740_IP>\n";
    std::cerr << "  -p: 파라미터를 로딩할 때 사용하는 옵션입니다.\n";
    std::cerr << "  <V2740_IP>: V2740 모듈의 IP 주소를 지정합니다.\n";
}

int main(int argc, char* argv[]) {
    // 루프 제어 변수 추가
    std::signal(SIGINT, signalHandler);

    bool flagPar = false;
    std::string parFile;
    std::string ip;

    int nSec = -1;                        // DAQ 시간 설정
    std::string outputFile = "data.out";  // 기본 출력 파일 이름

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-p") {
            if (i + 1 < argc) {
                flagPar = true;
                parFile = argv[i + 1];
                i++;  // 다음 인자로 넘어가기 위해
            }
        } else if (std::string(argv[i]) == "-t") {
            if (i + 1 < argc) {
                nSec = std::stoi(argv[i + 1]);
                i++;  // 다음 인자로 넘어가기 위해
            }
        } else if (std::string(argv[i]) == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[i + 1];  // 출력 파일 이름 지정
                i++;                       // 다음 인자로 넘어가기 위해
            }
        } else {
            ip = argv[i];  // IP 주소 저장
        }
    }
    if (ip.empty()) {
        usage();
        return 1;
    }

#ifdef DEBUG
    std::cerr << "사용된 IP 주소: " << ip << std::endl;
    if (flagPar) {
        std::cerr << "파라미터 파일명: " << parFile << std::endl;
    }
#endif

    // CAEN V2740 모듈 초기화
    CAENV2740 v2740("dig2://" + std::string(argv[1]));
    v2740.connect();

    // 파라미터 파일 로딩
    if (flagPar) {
        CAENV2740Par par(parFile);
        v2740.loadParameter(par);
        par.printConfig();
    }

    // 파일 열기
    std::ofstream fout(outputFile);

    // 데이터 수집 설정
    v2740.setDataFormatDPPPSD();
    v2740.writeActiveEndPoint("dpppsd");
    // v2740.setDataFormatDPPPSDStats();
    //  v2740.writeActiveEndPoint("dpppsdstat");

    // 데이터 수집 시작
    v2740.clear();
    v2740.armAcquisition();
    v2740.startAcquisition();
    v2740.sendSWTrigger();

    // 데이터 수집 중...
    printf("데이터 수집 중...\n");
    uint64_t nevent = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    while (running && (nSec <= 0 || std::chrono::duration_cast<std::chrono::seconds>(
                                        std::chrono::high_resolution_clock::now() - start_time)
                                            .count() < nSec)) {
        v2740.readData();
        v2740.getEvent().Serialize(fout);
        nevent++;
        if (!(nevent % 100)) {
            std::cout << "Event: " << nevent << "\r";
            std::cout.flush();
        }
    }
    std::cout << "Event: " << nevent << "\n";

    std::cout << "채널\t실시간\t죽은 시간\t트리거 카운트\t저장된 이벤트 카운트\n";
    for (int channel = 0; channel < 8; channel++) {
        float realTime = v2740.readChRealTimeMonitor(channel);
        float deadTime = v2740.readChDeadTimeMonitor(channel);
        uint32_t triggerCnt = v2740.readChTriggerCnt(channel);
        uint32_t savedEventCnt = v2740.readChSavedEventCnt(channel);
        std::cout << channel << "\t" << realTime << "\t" << deadTime << "\t" << triggerCnt << "\t" << savedEventCnt
                  << std::endl;
    }
    // 데이터 수집 종료
    v2740.stopAcquisition();
    // v2740.disarmAcquisition();
    // 데이터 수집 종료 확인
    printf("데이터 수집 종료.\n");
    fout.close();

    return 0;
}
