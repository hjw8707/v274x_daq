#include <unistd.h>

#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>  // 스레드 사용을 위한 헤더 추가

#include "CAENV2740.hxx"
#include "CAENV2740Par.hxx"

void usage() {
    std::cerr << "사용법: ./daq [-p] [-c] [-t <시간>] [-o <파일 이름>] <V2740_IP>\n";
    std::cerr << "  -p: 파라미터를 로딩할 때 사용하는 옵션입니다.\n";
    std::cerr << "  -c: 인코딩된 이벤트를 읽는 옵션입니다.\n";
    std::cerr << "  -t: 데이터 수집 시간을 지정합니다.\n";
    std::cerr << "  -o: 출력 파일 이름을 지정합니다.\n";
    std::cerr << "  <V2740_IP>: V2740 모듈의 IP 주소를 지정합니다.\n";
}

void startAcquisition(CAENV2740& v2740) {
    v2740.clear();
    v2740.armAcquisition();
    v2740.startAcquisition();
}

void stopAcquisition(CAENV2740& v2740) {
    // 데이터 수집 종료 시 채널별 실시간 모니터링 정보 출력
    std::cout << "채널\tRealTimeMonitor\tDeadTimeMonitor\tTriggerCnt\tSavedEvent\n";
    for (int i = 0; i < 8; i++) {
        std::cout << i << "\t" << v2740.readChRealTimeMonitor(i) << "\t" << v2740.readChDeadTimeMonitor(i) << "\t"
                  << v2740.readChTriggerCnt(i) << "\t" << v2740.readChSavedEventCnt(i) << std::endl;
    }
    v2740.stopAcquisition();
    v2740.disarmAcquisition();
}

void codedDataAcquisition(CAENV2740& v2740, std::ofstream& fout, int nSec) {
    uint64_t nevent = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    while ((nSec <= 0 ||
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time)
                    .count() < nSec)) {
        int ret = v2740.readData();
        switch (ret) {
            case CAEN_FELib_Success:
                v2740.getEvent().Serialize(fout);
                nevent++;
                break;
            case CAEN_FELib_Timeout:
                std::cerr << "Timeout\n";
                break;
            case CAEN_FELib_Stop:
                std::cout << "Event: " << nevent << "\n";
                return;
            default:
                // 데이터 읽기 실패
                break;
        }

        if (!(nevent % 100)) {
            std::cout << "Event: " << nevent << "\r";
            std::cout.flush();
        }
    }
    std::cout << "Event: " << nevent << "\n";
}

void rawDataAcquisition(CAENV2740& v2740, std::ofstream& fout, int nSec) {
    size_t size = 0;
    uint8_t* data = nullptr;
    data = new uint8_t[2621440];

    uint64_t nbunch = 0;
    uint32_t nevent_raw = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    while ((nSec <= 0 ||
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time)
                    .count() < nSec)) {
        int ret = v2740.readDataRaw(1000, data, &size, &nevent_raw);
        switch (ret) {
            case CAEN_FELib_Success:
                fout.write(reinterpret_cast<char*>(data), size);
                nbunch++;
                break;
            case CAEN_FELib_Timeout:
                std::cout << "Bunch: " << nbunch << "\n";
                return;  // break;
            case CAEN_FELib_Stop:
                std::cout << "Bunch: " << nbunch << "\n";
                return;
            default:
                // 데이터 읽기 실패
                break;
        }

        if (!(nbunch % 100)) {
            std::cout << "Bunch: " << nbunch << "\r";
            std::cout.flush();
        }
    }
    std::cout << "Bunch: " << nbunch << "\n";
}

int main(int argc, char* argv[]) {
    bool flagPar = false;
    bool flagCoded = false;
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
        } else if (std::string(argv[i]) == "-c") {
            flagCoded = true;  // 인코딩된 이벤트를 읽는 옵션
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
    CAENV2740 v2740;
    v2740.connect("dig2://" + std::string(argv[1]));

    // 파라미터 파일 로딩
    if (flagPar) {
        CAENV2740Par par(parFile);
        v2740.loadParameter(par);
        par.printConfig();
    }

    // 파일 열기
    std::ofstream fout(outputFile);

    // 데이터 수집 설정
    void (*acq_fun)(CAENV2740&, std::ofstream&, int) = rawDataAcquisition;
    if (flagCoded) {  // 인코딩된 이벤트를 읽는 옵션 (시간이 오래 걸림)
        acq_fun = codedDataAcquisition;
        v2740.setDataFormatDPPPSD();
        v2740.writeActiveEndPoint("dpppsd");
    } else {
        v2740.setDataFormatRaw();
        v2740.writeActiveEndPoint("raw");
    }

    std::thread acquisitionThread;  // 쓰레드 선언

    // 사용자 입력을 통한 데이터 수집 제어
    char command;
    std::cout << "데이터 수집을 시작하려면 's'를 입력하세요. 종료하려면 'e'를 입력하세요.\n";
    if (nSec > 0) {
        std::cout << "지정된 시간 동안 데이터 수집을 시작합니다.\n";
        // 데이터 수집 시작
        startAcquisition(v2740);

        // 데이터 수집을 위한 스레드 시작
        acquisitionThread = std::thread(acq_fun, std::ref(v2740), std::ref(fout), nSec);
        std::this_thread::sleep_for(std::chrono::seconds(nSec));

        // 데이터 수집 종료
        stopAcquisition(v2740);
        std::cout << "지정된 시간이 끝나 데이터 수집을 종료합니다.\n";
    } else {
        while (true) {
            std::cin.get(command);
            if (command == 's') {
                std::cout << "데이터 수집 시작.\n";
                // 데이터 수집 시작
                startAcquisition(v2740);

                // 데이터 수집을 위한 스레드 시작
                if (!acquisitionThread.joinable()) {  // 쓰레드가 실행 중이지 않을 때만 시작
                    acquisitionThread = std::thread(acq_fun, std::ref(v2740), std::ref(fout), nSec);
                }
            } else if (command == 'e') {
                stopAcquisition(v2740);
                std::cout << "데이터 수집 종료.\n";
                break;
            }
        }
    }
    acquisitionThread.join();

    fout.close();
    return 0;
}
