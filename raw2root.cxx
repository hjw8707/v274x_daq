#include <fstream>
#include <iostream>

#include "CAENV2740Event.hxx"
#include "TFile.h"
#include "TTree.h"

class CAENV2740Reader {
   public:
    CAENV2740Reader() : flagVerbose(false), file(nullptr), tree(nullptr) {}
    ~CAENV2740Reader() {}

    void SetVerbose(bool flag) { flagVerbose = flag; }
    bool GetVerbose() { return flagVerbose; }

    void InitInput(const std::string &filename) {
        inputFile.open(filename, std::ios::binary);  // 이진 파일 열기

        // 파일 열기 실패 시 에러 메시지 출력
        if (!inputFile) {
            std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
            return;
        }
    }

    void InitOutput(std::string outputFilename) {
        file = new TFile(outputFilename.c_str(), "RECREATE");   // ROOT 파일 생성
        tree = new TTree("EventTree", "CAENV2740 Event Tree");  // ROOT 트리 생성

        // 트리에 저장할 데이터 브랜치 정의
        tree->Branch("channel", &event.channel, "channel/b");
        tree->Branch("timestamp", &event.timestamp, "timestamp/l");
        tree->Branch("fine_timestamp", &event.fine_timestamp, "fine_timestamp/s");
        tree->Branch("energy", &event.energy, "energy/s");
        tree->Branch("energy_short", &event.energy_short, "energy_short/s");
        tree->Branch("flags_low_priority", &event.flags_low_priority, "flags_low_priority/s");
        tree->Branch("flags_high_priority", &event.flags_high_priority, "flags_high_priority/s");
        tree->Branch("event_size", &event.event_size, "event_size/i");
    }

    void CloseInput() {
        if (inputFile) inputFile.close();
    }
    void CloseOutput() {
        if (tree) tree->Write();
        if (file) {
            file->Close();
            delete file;
        }
    }

   private:
    bool flagVerbose;

    std::ifstream inputFile;

    TFile *file;
    TTree *tree;

    CAENV2740Event event;

   public:
    void ReadCAENV2740CodedEvent() {
        static int eventCount = 0;  // 이벤트 수 카운터 초기화
        while (inputFile) {
            // 이벤트 데이터 읽기
            inputFile.read(reinterpret_cast<char *>(&event.channel), sizeof(event.channel));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.timestamp), sizeof(event.timestamp));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.fine_timestamp), sizeof(event.fine_timestamp));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.energy), sizeof(event.energy));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.energy_short), sizeof(event.energy_short));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.flags_low_priority), sizeof(event.flags_low_priority));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.flags_high_priority), sizeof(event.flags_high_priority));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료
            inputFile.read(reinterpret_cast<char *>(&event.event_size), sizeof(event.event_size));
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료

            // 읽은 데이터를 ROOT 트리에 저장
            tree->Fill();
            eventCount++;  // 이벤트 수 증가
            if (eventCount % 100 == 0) {
                std::cout << "\r현재까지 읽은 이벤트 수: " << eventCount << std::flush;  // 진행 상황 출력
            }
        }
    }

    void startRunParser(uint64_t *datas) {
        uint32_t acquisition_width = datas[0] & 0x1FFFFFF;
        uint16_t n_traces = (datas[0] >> 25) & 0x3;
        uint16_t dec_factor_log2 = (datas[0] >> 27) & 0x1F;
        uint64_t channel_mask = datas[1] & 0xFFFFFFFF + ((datas[2] & 0xFFFFFFFF) << 32);

        if (flagVerbose) {
            std::cout << "Start Run 이벤트 파싱" << std::endl;
            std::cout << "acquisition_width: " << acquisition_width << std::endl;
            std::cout << "n_traces: " << n_traces << std::endl;
            std::cout << "dec_factor_log2: " << dec_factor_log2 << std::endl;
            std::cout << "channel_mask: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(16)
                      << channel_mask << std::endl;
            std::cout << std::dec;
            // getchar();
        }
    }

    void stopRunParser(uint64_t *datas) {
        uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
        uint32_t deadtime = datas[1] & 0xFFFFFFFF;
        if (flagVerbose) {
            std::cout << "Stop Run 이벤트 파싱" << std::endl;
            std::cout << "timestamp: " << timestamp << std::endl;
            std::cout << "deadtime: " << deadtime << std::endl;
        }
    }

    void eventParser(uint64_t *datas, int nWords) {
        int channel = (datas[0] >> 56) & 0x7F;
        bool special_event = (datas[0] >> 55) & 0x1;
        uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
        bool with_waveform = (datas[1] >> 62) & 0x1;
        uint16_t flags_low_priority = (datas[1] >> 50) & 0x3FF;
        uint16_t flags_high_priority = (datas[1] >> 42) & 0xFF;
        uint16_t energy_short = (datas[1] >> 26) & 0xFFFF;
        uint16_t fine_timestamp = (datas[1] >> 16) & 0xFFFF;
        uint16_t energy = datas[1] & 0xFFFF;

        event.channel = channel;
        event.timestamp = timestamp;
        event.flags_high_priority = flags_high_priority;
        event.flags_low_priority = flags_low_priority;
        event.energy_short = energy_short;
        event.fine_timestamp = fine_timestamp;
        event.energy = energy;

        tree->Fill();
        if (flagVerbose) {
            std::cout << "Event Parser" << std::endl;
            std::cout << "channel: " << channel << std::endl;
            std::cout << "special_event: " << special_event << std::endl;
            std::cout << "timestamp: " << timestamp << std::endl;
            std::cout << "with_waveform: " << with_waveform << std::endl;
            std::cout << "flags_high_priority: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(3)
                      << flags_high_priority << std::endl;
            std::cout << "flags_low_priority: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
                      << flags_low_priority << std::endl;
            std::cout << std::dec;
            std::cout << "energy_short: " << energy_short << std::endl;
            std::cout << "fine_timestamp: " << fine_timestamp << std::endl;
            std::cout << "energy: " << energy << std::endl;
            // getchar();
        }
    }

    void ReadCAENV2740RawEvent() {
        static int eventCount = 0;  // 이벤트 수 카운터 초기화
        int eventType = 0;          // 1 = Common Trigger, 2 = Individual Trigger, 3 = Special
        int nWords = 0;
        bool flush;
        bool board_good;
        int prev_aggregate_counter = 0;
        int aggregate_counter;
        uint64_t data[10];

        int runEvent = 0;
        int j;
        while (inputFile) {
            inputFile.read(reinterpret_cast<char *>(&data[0]), sizeof(data[0]));
            data[0] = __builtin_bswap64(data[0]);
            if (!inputFile) break;  // 파일 읽기에 실패하면 루프 종료

            nWords = data[0] & 0xFFFFFFFF;                   // 0 ~ 31 비트 추출
            aggregate_counter = (data[0] >> 32) & 0xFFFFFF;  // 32 ~ 47 비트 추출
            flush = (data[0] >> 56) & 0x1;                   // 56 비트 추출
            board_good = (data[0] >> 59) & 0x1;              // 59 비트 추출
            eventType = (data[0] >> 60) & 0xF;               // 60 ~ 63 비트 추출
            if (aggregate_counter != prev_aggregate_counter + 1) {
                std::cerr << "Aggregate Counter Error: " << prev_aggregate_counter << " -> " << aggregate_counter
                          << std::endl;
            }  // aggregate_counter validity check
            prev_aggregate_counter = aggregate_counter;
            if (flagVerbose) {
                std::cout << "nWords: " << nWords << std::endl;
                std::cout << "aggregate_counter: " << aggregate_counter << std::endl;
                std::cout << "flush: " << flush << std::endl;
                std::cout << "board_good: " << board_good << std::endl;
                std::cout << "eventType: " << eventType << std::endl;
                getchar();
            }

            switch (eventType) {
                case 1:
                    if (flagVerbose) std::cout << "Common Trigger" << std::endl;
                    break;
                case 2:
                    if (flagVerbose) std::cout << "Individual Trigger" << std::endl;
                    if (!board_good) break;
                    j = 0;
                    for (int i = 1; i < nWords; i++) {
                        inputFile.read(reinterpret_cast<char *>(&data[j]), sizeof(data[j]));
                        data[j] = __builtin_bswap64(data[j]);
                        //  std::cout << "\r현재까지 읽은 워드 수: " << i << ", " << std::hex << data[j]
                        //           << std::endl;  // 진행 상황 출력
                        // std::cout << std::dec;
                        if ((data[j++] >> 63) & 0x1) {
                            eventParser(data, j);
                            eventCount++;
                            j = 0;
                        }  // 이벤트 수 증가
                    }
                    break;
                case 3:
                    if (flagVerbose) std::cout << "Special" << std::endl;
                    runEvent = (data[0] >> 32) & 0xFF;  // 32 ~ 39 비트 추출
                    if (runEvent == 3) {
                        for (int i = 0; i < 3; i++) {
                            inputFile.read(reinterpret_cast<char *>(&data[i]), sizeof(data[i]));
                            data[i] = __builtin_bswap64(data[i]);
                        }
                        startRunParser(data);
                    } else if (runEvent == 2) {
                        for (int i = 0; i < 2; i++) {
                            inputFile.read(reinterpret_cast<char *>(&data[i]), sizeof(data[i]));
                            data[i] = __builtin_bswap64(data[i]);
                        }
                        stopRunParser(data);
                    } else {
                        std::cerr << "Special Event Code Error: " << runEvent << std::endl;
                        getchar();
                    }
                    break;
                default:
                    if (flagVerbose) {
                        std::cerr << "Unknown Event Code: " << eventType << std::endl;
                        std::cout << "nWords: " << nWords << std::endl;
                        std::cout << "aggregate_counter: " << aggregate_counter << std::endl;
                        std::cout << "flush: " << flush << std::endl;
                        std::cout << "board_good: " << board_good << std::endl;
                        std::cout << "eventType: " << eventType << std::endl;
                        getchar();
                    }
                    break;
            }
            if (eventCount % 100 == 0) {
                std::cout << "\rEvent Count: " << eventCount << std::flush;  // 진행 상황 출력
            }
        }
        std::cout << "Event Count: " << eventCount << std::endl;
    }
};

// main 함수 수정
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "사용법: ./readRaw <파일 이름> [-c] [-o <출력 파일 이름>]" << std::endl;
        std::cerr << "  -c: 인코딩된 이벤트를 읽습니다." << std::endl;
        std::cerr << "  -o <출력 파일 이름>: 출력 파일 이름을 지정합니다." << std::endl;
        return 1;
    }
    bool isCodedEvent = false;
    std::string outputFile = "";
    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-c") {
            isCodedEvent = true;
        } else if (std::string(argv[i]) == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[i + 1];
                i++;  // 다음 인자로 넘어가기 위해
            } else {
                std::cerr << "출력 파일 이름이 지정되지 않았습니다." << std::endl;
                return 1;
            }
        }
    }
    if (outputFile.empty()) {
        std::string inputFileName = argv[1];
        size_t dotPos = inputFileName.find('.');
        if (dotPos != std::string::npos) {
            outputFile = inputFileName.substr(0, dotPos) + ".root";
        } else {
            outputFile = inputFileName + ".root";
        }
    }

    CAENV2740Reader reader;  // CAENV2740Reader 객체 생성
    reader.InitInput(argv[1]);
    reader.InitOutput(outputFile);
    // reader.SetVerbose(true);

    if (isCodedEvent)
        reader.ReadCAENV2740CodedEvent();
    else
        reader.ReadCAENV2740RawEvent();
    reader.CloseInput();
    reader.CloseOutput();
    return 0;
}
