#include <QTextStream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "CAENV2740Event.hxx"
#include "RawDataEnder.hxx"
#include "RawDataHeader.hxx"
#include "TFile.h"
#include "TTree.h"

class CAENV2740Reader {
   public:
    CAENV2740Reader()
        : flagVerbose(false),
          flagSort(false),
          flagMerge(false),
          timeWindowNs(1000),
          flagCalib(false),
          file(nullptr),
          tree(nullptr),
          totalEventCount(0) {}
    ~CAENV2740Reader() {}

    void SetVerbose(bool flag) { flagVerbose = flag; }
    bool GetVerbose() { return flagVerbose; }
    void SetSort(bool flag) { flagSort = flag; }
    void SetCalibrationFile(const std::string &filename) {
        if (filename.empty()) {
            flagCalib = false;
            return;
        }
        LoadCalibrationFile(filename);
    }
    void SetMerge(bool flag, uint64_t windowNs = 1000) {
        flagMerge = flag;
        timeWindowNs = windowNs;
        // 병합을 위해서는 정렬이 필수이므로 자동으로 정렬 활성화
        if (flag) {
            flagSort = true;
        }
    }

    void InitInput(const std::string &filename) {
        inputFile.open(filename, std::ios::binary);  // 이진 파일 열기

        // 파일 열기 실패 시 에러 메시지 출력
        if (!inputFile) {
            std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
            return;
        }
    }

    void InitOutput(std::string outputFilename) {
        file = new TFile(outputFilename.c_str(), "RECREATE");  // ROOT 파일 생성

        if (flagMerge) {
            // 병합된 이벤트를 위한 트리 생성
            tree = new TTree("MergedEventTree", "Merged CAENV2740 Event Tree");

            // 병합된 이벤트를 위한 vector 브랜치들
            tree->Branch("channel", &mergedEvent.channel);
            tree->Branch("timestamp", &mergedEvent.timestamp);
            tree->Branch("fine_timestamp", &mergedEvent.fine_timestamp);
            tree->Branch("energy", &mergedEvent.energy);
            tree->Branch("energy_short", &mergedEvent.energy_short);
            if (flagCalib) tree->Branch("cal_energy", &mergedEvent.cal_energy);
            tree->Branch("flags_low_priority", &mergedEvent.flags_low_priority);
            tree->Branch("flags_high_priority", &mergedEvent.flags_high_priority);
            tree->Branch("event_size", &mergedEvent.event_size);
            tree->Branch("merged_timestamp", &mergedEvent.merged_timestamp, "merged_timestamp/l");
            tree->Branch("merged_fine_timestamp", &mergedEvent.merged_fine_timestamp, "merged_fine_timestamp/s");
        } else {
            // 기존 단일 이벤트 트리 생성
            tree = new TTree("EventTree", "CAENV2740 Event Tree");

            // 트리에 저장할 데이터 브랜치 정의
            tree->Branch("channel", &event.channel, "channel/b");
            tree->Branch("timestamp", &event.timestamp, "timestamp/l");
            tree->Branch("fine_timestamp", &event.fine_timestamp, "fine_timestamp/s");
            tree->Branch("energy", &event.energy, "energy/s");
            tree->Branch("energy_short", &event.energy_short, "energy_short/s");
            if (flagCalib) tree->Branch("cal_energy", &cal_energy, "cal_energy/D");
            tree->Branch("flags_low_priority", &event.flags_low_priority, "flags_low_priority/s");
            tree->Branch("flags_high_priority", &event.flags_high_priority, "flags_high_priority/s");
            tree->Branch("event_size", &event.event_size, "event_size/i");
        }
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

    // 전체 이벤트 수 반환
    int GetTotalEventCount() const { return totalEventCount; }

    // 정렬된 이벤트들을 트리에 저장
    void FillSortedEvents() {
        if (flagSort && !eventBuffer.empty()) {
            if (flagVerbose) {
                std::cout << "이벤트 정렬 중... (" << eventBuffer.size() << "개 이벤트)" << std::endl;
            }

            // timestamp 기준으로 정렬
            std::sort(eventBuffer.begin(), eventBuffer.end(), [](const BufferedEvent &a, const BufferedEvent &b) {
                if (a.event.timestamp != b.event.timestamp) {
                    return a.event.timestamp < b.event.timestamp;
                }
                // timestamp가 같으면 fine_timestamp로 정렬
                return a.event.fine_timestamp < b.event.fine_timestamp;
            });

            if (flagVerbose) {
                std::cout << "정렬 완료. 트리에 저장 중..." << std::endl;
            }

            // 정렬된 이벤트들을 트리에 저장
            for (const auto &sortedEvent : eventBuffer) {
                event = sortedEvent.event;
                if (flagCalib) cal_energy = sortedEvent.cal_energy;
                tree->Fill();
            }

            if (flagVerbose) {
                std::cout << "정렬된 이벤트 " << eventBuffer.size() << "개를 트리에 저장했습니다." << std::endl;
            }

            // 버퍼 클리어
            eventBuffer.clear();
        }
    }

    // 병합된 이벤트들을 트리에 저장
    void FillMergedEvents() {
        if (flagMerge && !eventBuffer.empty()) {
            if (flagVerbose) {
                std::cout << "이벤트 병합 중... (" << eventBuffer.size() << "개 이벤트)" << std::endl;
            }

            // timestamp 기준으로 정렬
            std::sort(eventBuffer.begin(), eventBuffer.end(), [](const BufferedEvent &a, const BufferedEvent &b) {
                if (a.event.timestamp != b.event.timestamp) {
                    return a.event.timestamp < b.event.timestamp;
                }
                return a.event.fine_timestamp < b.event.fine_timestamp;
            });

            if (flagVerbose) {
                std::cout << "정렬 완료. 타임 윈도우 " << timeWindowNs << "ns로 병합 중..." << std::endl;
            }

            // 이벤트 병합 처리
            size_t idx = 0;
            while (idx < eventBuffer.size()) {
                // 새로운 병합 이벤트 시작
                mergedEvent.channel.clear();
                mergedEvent.timestamp.clear();
                mergedEvent.fine_timestamp.clear();
                mergedEvent.energy.clear();
                mergedEvent.energy_short.clear();
                if (flagCalib) mergedEvent.cal_energy.clear();
                mergedEvent.flags_low_priority.clear();
                mergedEvent.flags_high_priority.clear();
                mergedEvent.event_size.clear();

                const CAENV2740Event &first = eventBuffer[idx].event;
                double ref_time_ps = get_abs_time_ps(first.timestamp, first.fine_timestamp);
                mergedEvent.merged_timestamp = first.timestamp;
                mergedEvent.merged_fine_timestamp = first.fine_timestamp;

                // 윈도우 내에 들어오는 이벤트를 모두 병합
                size_t j = idx;
                for (; j < eventBuffer.size(); ++j) {
                    double cur_time_ps =
                        get_abs_time_ps(eventBuffer[j].event.timestamp, eventBuffer[j].event.fine_timestamp);
                    if ((cur_time_ps - ref_time_ps) / 1000.0 > timeWindowNs) break;  // ns 단위로 비교

                    mergedEvent.channel.push_back(eventBuffer[j].event.channel);
                    mergedEvent.timestamp.push_back(eventBuffer[j].event.timestamp);
                    mergedEvent.fine_timestamp.push_back(eventBuffer[j].event.fine_timestamp);
                    mergedEvent.energy.push_back(eventBuffer[j].event.energy);
                    mergedEvent.energy_short.push_back(eventBuffer[j].event.energy_short);
                    if (flagCalib) mergedEvent.cal_energy.push_back(eventBuffer[j].cal_energy);
                    mergedEvent.flags_low_priority.push_back(eventBuffer[j].event.flags_low_priority);
                    mergedEvent.flags_high_priority.push_back(eventBuffer[j].event.flags_high_priority);
                    mergedEvent.event_size.push_back(eventBuffer[j].event.event_size);
                }

                tree->Fill();
                idx = j;
            }

            if (flagVerbose) {
                std::cout << "병합 완료. 병합된 이벤트를 트리에 저장했습니다." << std::endl;
            }

            // 버퍼 클리어
            eventBuffer.clear();
        }
    }

   private:
    bool flagVerbose;
    bool flagSort;          // timestamp 정렬 플래그
    bool flagMerge;         // 이벤트 병합 플래그
    uint64_t timeWindowNs;  // 타임 윈도우 (ns)
    bool flagCalib;

    std::ifstream inputFile;

    TFile *file;
    TTree *tree;

    CAENV2740Event event;
    double cal_energy = 0.0;
    int totalEventCount;                      // 전체 파일에서 누적된 이벤트 수

    struct CalibCoeff {
        bool has = false;
        double c0 = 0.0;
        double c1 = 0.0;
        double c2 = 0.0;
    };
    std::vector<CalibCoeff> calib;  // channel index 기반

    struct BufferedEvent {
        CAENV2740Event event;
        double cal_energy = 0.0;
    };
    std::vector<BufferedEvent> eventBuffer;  // 정렬/병합을 위한 이벤트 버퍼

    // 병합된 이벤트를 위한 구조체
    struct MergedEvent {
        std::vector<UChar_t> channel;
        std::vector<ULong64_t> timestamp;
        std::vector<UShort_t> fine_timestamp;
        std::vector<UShort_t> energy;
        std::vector<UShort_t> energy_short;
        std::vector<Double_t> cal_energy;
        std::vector<UShort_t> flags_low_priority;
        std::vector<UShort_t> flags_high_priority;
        std::vector<UInt_t> event_size;
        ULong64_t merged_timestamp;
        UShort_t merged_fine_timestamp;
    } mergedEvent;

    // 시간 계산 함수
    double get_abs_time_ps(uint64_t timestamp, uint16_t fine_timestamp) {
        const double FINE_TIMESTAMP_UNIT_PS = 7.8125;
        const double COARSE_TIMESTAMP_UNIT_NS = 8.0;
        return timestamp * COARSE_TIMESTAMP_UNIT_NS * 1000.0 + fine_timestamp * FINE_TIMESTAMP_UNIT_PS;
    }

    void LoadCalibrationFile(const std::string &filename) {
        std::ifstream fin(filename);
        if (!fin) {
            std::cerr << "칼리브레이션 파일을 열 수 없습니다: " << filename << std::endl;
            flagCalib = false;
            return;
        }

        calib.assign(128, CalibCoeff{});
        std::string line;
        int loaded = 0;
        while (std::getline(fin, line)) {
            // trim left
            size_t pos = line.find_first_not_of(" \t\r\n");
            if (pos == std::string::npos) continue;
            if (line[pos] == '#') continue;

            std::istringstream iss(line);
            int ch = -1;
            double c0 = 0.0, c1 = 0.0, c2 = 0.0;
            if (!(iss >> ch >> c0 >> c1 >> c2)) continue;
            if (ch < 0 || ch >= static_cast<int>(calib.size())) continue;

            calib[ch].has = true;
            calib[ch].c0 = c0;
            calib[ch].c1 = c1;
            calib[ch].c2 = c2;
            loaded++;
        }

        flagCalib = true;
        if (flagVerbose) {
            std::cout << "칼리브레이션 로딩 완료: " << loaded << "개 채널 (" << filename << ")" << std::endl;
        }
    }

    double ComputeCalEnergy(uint8_t ch, uint16_t adc) const {
        if (!flagCalib) return 0.0;
        if (ch >= calib.size()) return 0.0;
        const auto &cc = calib[ch];
        if (!cc.has) return 0.0;
        const double x = static_cast<double>(adc);
        return cc.c0 + cc.c1 * x + cc.c2 * x * x;
    }

   public:
    void ReadCAENV2740CodedEvent() {
        int fileEventCount = 0;  // 현재 파일의 이벤트 수 카운터
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

            // 정렬 옵션이 활성화된 경우 버퍼에 저장, 아니면 바로 트리에 저장
            if (flagSort || flagMerge) {
                BufferedEvent be;
                be.event = event;
                be.cal_energy = ComputeCalEnergy(event.channel, event.energy);
                eventBuffer.push_back(be);
            } else {
                if (flagCalib) cal_energy = ComputeCalEnergy(event.channel, event.energy);
                tree->Fill();
            }

            fileEventCount++;   // 현재 파일의 이벤트 수 증가
            totalEventCount++;  // 전체 이벤트 수 증가

            if (fileEventCount % 100 == 0) {
                std::cout << "\r현재 파일 이벤트 수: " << fileEventCount << " (전체: " << totalEventCount << ")"
                          << std::flush;
            }
        }

        if (flagVerbose) {
            std::cout << "\n현재 파일 처리 완료 - 이벤트 수: " << fileEventCount << std::endl;
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

        if (flagCalib) cal_energy = ComputeCalEnergy(event.channel, event.energy);

        // 정렬 옵션이 비활성화된 경우에만 바로 트리에 저장
        if (!flagSort && !flagMerge) {
            tree->Fill();
        }

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
        int fileEventCount = 0;  // 현재 파일의 이벤트 수 카운터
        int eventType = 0;       // 1 = Common Trigger, 2 = Individual Trigger, 3 = Special
        int nWords = 0;
        bool flush;
        bool board_good;
        int prev_aggregate_counter = 0;
        int aggregate_counter;
        uint64_t data[10];

        RawDataHeader *header = nullptr;
        RawDataEnder *ender = nullptr;
        QTextStream stream(stdout);

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
            if (eventType < 3 && aggregate_counter != prev_aggregate_counter + 1) {
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

                            // 정렬 옵션이 활성화된 경우 버퍼에 저장, 아니면 바로 트리에 저장
                            if (flagSort || flagMerge) {
                                BufferedEvent be;
                                be.event = event;
                                be.cal_energy = ComputeCalEnergy(event.channel, event.energy);
                                eventBuffer.push_back(be);
                            }
                            // eventParser에서 이미 tree->Fill()을 호출하므로 정렬이 아닌 경우는 추가 호출 불필요

                            fileEventCount++;   // 현재 파일의 이벤트 수 증가
                            totalEventCount++;  // 전체 이벤트 수 증가
                            j = 0;
                        }
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
                case 4:  // RawDataHeader (nWords = 8)
                    if (flagVerbose) std::cout << "RawDataHeader" << std::endl;
                    for (int i = 0; i < nWords; i++) {  // no endian change
                        inputFile.read(reinterpret_cast<char *>(&data[i]), sizeof(data[i]));
                    }
                    header = new RawDataHeader(data);
                    header->getHeaderInfo(stream);
                    stream.flush();
                    break;
                case 5:  // RawDataEnder (nWords = 6)
                    if (flagVerbose) std::cout << "RawDataEnder" << std::endl;
                    for (int i = 0; i < nWords; i++) {  // no endian change
                        inputFile.read(reinterpret_cast<char *>(&data[i]), sizeof(data[i]));
                    }
                    ender = new RawDataEnder(data);
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
            if (fileEventCount % 100 == 0) {
                std::cout << "\r현재 파일 이벤트 수: " << fileEventCount << " (전체: " << totalEventCount << ")"
                          << std::flush;
            }
        }

        if (flagVerbose) {
            std::cout << "\n현재 파일 처리 완료 - 이벤트 수: " << fileEventCount << std::endl;
        }

        if (header) delete header;
        if (ender) {
            ender->getEnderInfo(stream);
            stream.flush();
            delete ender;
        }
    }
};

// main 함수 수정
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "사용법: ./raw2root <파일 이름1> [<파일 이름2> ...] [-e] [-c <calib.txt>] [-o <출력 파일 이름>] [-v] "
                     "[-t] [-m <타임 윈도우(ns)>]"
                  << std::endl;
        std::cerr << "  <파일 이름1> [<파일 이름2> ...]: 처리할 raw 파일들 (와일드카드 지원)" << std::endl;
        std::cerr << "  -e: 인코딩된 이벤트를 읽습니다. (기존 -c는 더 이상 권장하지 않음)" << std::endl;
        std::cerr << "  -c <calib.txt>: 채널별 에너지 칼리브레이션을 적용하고 cal_energy 브랜치를 추가합니다." << std::endl;
        std::cerr << "  -o <출력 파일 이름>: 출력 파일 이름을 지정합니다." << std::endl;
        std::cerr << "  -v: 상세 출력을 활성화합니다." << std::endl;
        std::cerr << "  -t: timestamp 기준으로 이벤트를 정렬합니다." << std::endl;
        std::cerr << "  -m <타임 윈도우(ns)>: 지정된 시간 윈도우 내의 이벤트를 병합합니다. (정렬 자동 활성화)"
                  << std::endl;
        std::cerr << "예시: ./raw2root *.raw -c calib.txt -o combined.root -t" << std::endl;
        std::cerr << "예시: ./raw2root *.raw -e -o combined.root -t" << std::endl;
        std::cerr << "예시: ./raw2root *.raw -m 1000 -o merged.root (1us 윈도우로 병합)" << std::endl;
        std::cerr << "예시: ./raw2root *.raw -t -m 1000 -o sorted_merged.root (정렬 후 병합)" << std::endl;
        std::cerr << "참고: -m 옵션 사용 시 정렬이 자동으로 활성화됩니다." << std::endl;
        return 1;
    }

    bool isCodedEvent = false;
    bool flagVerbose = false;
    bool flagSort = false;
    bool flagMerge = false;
    uint64_t timeWindowNs = 1000;
    std::string outputFile = "";
    std::string calibFile = "";
    std::vector<std::string> inputFiles;

    // 명령행 인자 파싱
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-c") {
            // 새 의미: -c <calib.txt>
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                calibFile = argv[i + 1];
                i++;
            } else {
                // 하위호환: 과거 -c (coded event) 지원
                isCodedEvent = true;
                if (flagVerbose) {
                    std::cout << "경고: -c (coded event)는 더 이상 권장하지 않습니다. 대신 -e를 사용하세요." << std::endl;
                }
            }
        } else if (std::string(argv[i]) == "-e") {
            isCodedEvent = true;
        } else if (std::string(argv[i]) == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[i + 1];
                i++;  // 다음 인자로 넘어가기 위해
            } else {
                std::cerr << "출력 파일 이름이 지정되지 않았습니다." << std::endl;
                return 1;
            }
        } else if (std::string(argv[i]) == "-v") {
            flagVerbose = true;
        } else if (std::string(argv[i]) == "-t") {
            flagSort = true;
        } else if (std::string(argv[i]) == "-m") {
            if (i + 1 < argc) {
                flagMerge = true;
                timeWindowNs = std::stoull(argv[i + 1]);
                flagSort = true;  // 병합을 위해서는 정렬이 필수
                i++;              // 다음 인자로 넘어가기 위해
            } else {
                std::cerr << "이벤트 병합 타임 윈도우가 지정되지 않았습니다." << std::endl;
                return 1;
            }
        } else if (argv[i][0] != '-') {
            // 옵션이 아닌 경우 입력 파일로 간주
            inputFiles.push_back(argv[i]);
        }
    }

    if (inputFiles.empty()) {
        std::cerr << "입력 파일이 지정되지 않았습니다." << std::endl;
        return 1;
    }

    if (outputFile.empty()) {
        if (inputFiles.size() == 1) {
            // 단일 파일인 경우 기존 방식
            std::string inputFileName = inputFiles[0];
            size_t dotPos = inputFileName.find('.');
            if (dotPos != std::string::npos) {
                outputFile = inputFileName.substr(0, dotPos) + ".root";
            } else {
                outputFile = inputFileName + ".root";
            }
        } else {
            // 여러 파일인 경우 기본 이름 사용
            outputFile = "combined.root";
        }
    }

    if (flagVerbose) {
        std::cout << "처리할 파일 수: " << inputFiles.size() << std::endl;
        for (size_t i = 0; i < inputFiles.size(); i++) {
            std::cout << "  " << (i + 1) << ": " << inputFiles[i] << std::endl;
        }
        std::cout << "출력 파일: " << outputFile << std::endl;
        std::cout << "이벤트 타입: " << (isCodedEvent ? "인코딩된 이벤트" : "Raw 이벤트") << std::endl;
        if (flagSort && flagMerge) {
            std::cout << "정렬 옵션: 활성화 (timestamp 기준, 병합을 위해 자동 활성화)" << std::endl;
            std::cout << "병합 옵션: 활성화 (타임 윈도우 " << timeWindowNs << "ns)" << std::endl;
        } else if (flagSort) {
            std::cout << "정렬 옵션: 활성화 (timestamp 기준)" << std::endl;
        } else if (flagMerge) {
            std::cout << "병합 옵션: 활성화 (타임 윈도우 " << timeWindowNs << "ns)" << std::endl;
        } else {
            std::cout << "정렬/병합 옵션: 비활성화" << std::endl;
        }
    }

    CAENV2740Reader reader;  // CAENV2740Reader 객체 생성
    reader.SetVerbose(flagVerbose);
    reader.SetSort(flagSort);
    reader.SetMerge(flagMerge, timeWindowNs);
    if (!calibFile.empty()) reader.SetCalibrationFile(calibFile);
    reader.InitOutput(outputFile);

    // 모든 입력 파일 처리
    for (size_t i = 0; i < inputFiles.size(); i++) {
        if (flagVerbose) {
            std::cout << "\n[" << (i + 1) << "/" << inputFiles.size() << "] 처리 중: " << inputFiles[i] << std::endl;
        }

        reader.InitInput(inputFiles[i]);

        if (isCodedEvent) {
            reader.ReadCAENV2740CodedEvent();
        } else {
            reader.ReadCAENV2740RawEvent();
        }

        reader.CloseInput();
    }

    // 정렬 옵션이 활성화된 경우 정렬된 이벤트들을 트리에 저장
    if (flagSort && !flagMerge) {
        reader.FillSortedEvents();
    }

    // 병합 옵션이 활성화된 경우 병합된 이벤트들을 트리에 저장
    if (flagMerge) {
        reader.FillMergedEvents();
    }

    reader.CloseOutput();

    if (flagVerbose) {
        std::cout << "\n모든 파일 처리 완료. 출력 파일: " << outputFile << std::endl;
        std::cout << "총 처리된 이벤트 수: " << reader.GetTotalEventCount() << std::endl;
        if (flagSort && flagMerge) {
            std::cout << "이벤트가 timestamp 기준으로 정렬된 후, 타임 윈도우 " << timeWindowNs
                      << "ns로 병합되어 저장되었습니다." << std::endl;
        } else if (flagSort) {
            std::cout << "이벤트가 timestamp 기준으로 정렬되어 저장되었습니다." << std::endl;
        } else if (flagMerge) {
            std::cout << "이벤트가 timestamp 기준으로 정렬된 후, 타임 윈도우 " << timeWindowNs
                      << "ns로 병합되어 저장되었습니다." << std::endl;
        }
    } else {
        std::cout << "총 처리된 이벤트 수: " << reader.GetTotalEventCount() << std::endl;
        if (flagSort && flagMerge) {
            std::cout << "이벤트가 timestamp 기준으로 정렬된 후, 타임 윈도우 " << timeWindowNs
                      << "ns로 병합되어 저장되었습니다." << std::endl;
        } else if (flagSort) {
            std::cout << "이벤트가 timestamp 기준으로 정렬되어 저장되었습니다." << std::endl;
        } else if (flagMerge) {
            std::cout << "이벤트가 timestamp 기준으로 정렬된 후, 타임 윈도우 " << timeWindowNs
                      << "ns로 병합되어 저장되었습니다." << std::endl;
        }
    }

    return 0;
}
