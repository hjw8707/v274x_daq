#include "OnlineMonitor.hxx"

#include <iomanip>
#include <iostream>

#include "TDirectory.h"
OnlineMonitor::OnlineMonitor(const char* shmName)
    : nWords(0), aggregate_counter(0), flush(0), board_good(0), eventType(0), verbose(false) {
    shm = new SharedMemory(shmName, true);
    semRead = sem_open(shmName, O_CREAT, 0666, 0);
    httpServer = new THttpServer("http:8080");
    datas = new uint64_t[SHM_SIZE / sizeof(uint64_t)];

    createHistogram();
}

OnlineMonitor::~OnlineMonitor() {
    if (shm) delete shm;
    if (httpServer) delete httpServer;
    if (datas) delete[] datas;
    for (auto& histogram : histograms) {
        if (histogram) delete histogram;
    }
}

void OnlineMonitor::start() {
    while (true) {
        // parseData();
        if (!shm->IsOpen()) {
            std::cerr << "Shared memory is not open" << std::endl;
            shm->Initialize();
        }
        if (shm->IsOpen()) {
            parseData();
        }
        usleep(1000);
    }
}

void OnlineMonitor::parseData() {
    size_t size = shm->readNewData(datas) / sizeof(uint64_t);  // 읽은 데이터의 크기 (byte -> word)
    if (size <= 0) return;
    // endian 변환
    for (int i = 0; i < size; i++) datas[i] = __builtin_bswap64(datas[i]);

    // 헤더 파싱
    int eventType = parseHeader(datas[0]);
    int startIndex;
    // eventType에 따라 적절한 처리를 수행합니다.
    switch (eventType) {
        case 1:  // Common Trigger 이벤트 처리
            break;
        case 2:  // Individual Trigger 이벤트 처리
            startIndex = 1;
            for (int i = 1; i < size; i++) {
                if ((datas[i] >> 63) & 0x1) {  // 이벤트 끝 탐지
                    eventParser(&datas[startIndex], i - startIndex + 1);
                    startIndex = i + 1;
                }  // 이벤트 수 증가
            }
            break;
        case 3:                                     // Special 이벤트 처리
            if ((aggregate_counter & 0xFF) == 3) {  // runEvent
                startRunParser(&datas[1]);
            } else if ((aggregate_counter & 0xFF) == 2) {  // runEvent
                stopRunParser(&datas[1]);
            } else {
                std::cout << "Unknown event type: " << (aggregate_counter & 0xFF) << std::endl;
            }
            break;
        default:  // 알 수 없는 이벤트 타입 처리
            break;
    }
}

void OnlineMonitor::createHistogram() {
    TCanvas* canvas = new TCanvas("canvas", "DAQ Histogram");
    canvas->Divide(8, 8);
    httpServer->Register("/", canvas);
    TDirectory* tdir = new TDirectory("raw", "raw");
    tdir->cd();
    for (int i = 0; i < nCh; i++) {
        TH1F* histogram = new TH1F(Form("h%d", i), Form("Channel %d Histogram", i), 1000, 0, 10000);
        histograms.push_back(histogram);
        canvas->cd(i + 1);
        histogram->Draw();
    }
}

int OnlineMonitor::parseHeader(uint64_t header) {
    nWords = header & 0xFFFFFFFF;                   // 0 ~ 31 비트 추출
    aggregate_counter = (header >> 32) & 0xFFFFFF;  // 32 ~ 47 비트 추출
    flush = (header >> 56) & 0x1;                   // 56 비트 추출
    board_good = (header >> 59) & 0x1;              // 59 비트 추출
    eventType = (header >> 60) & 0xF;               // 60 ~ 63 비트 추출

    if (verbose) {
        std::cout << "nWords: " << nWords << std::endl;
        std::cout << "aggregate_counter: " << aggregate_counter << std::endl;
        std::cout << "flush: " << flush << std::endl;
        std::cout << "board_good: " << board_good << std::endl;
        std::cout << "eventType: " << eventType << std::endl;
    }

    return eventType;
}

void OnlineMonitor::startRunParser(uint64_t* datas) {
    uint32_t acquisition_width = datas[0] & 0x1FFFFFF;
    uint16_t n_traces = (datas[0] >> 25) & 0x3;
    uint16_t dec_factor_log2 = (datas[0] >> 27) & 0x1F;
    uint64_t channel_mask = datas[1] & 0xFFFFFFFF + ((datas[2] & 0xFFFFFFFF) << 32);

    if (verbose) {
        std::cout << "Start Run 이벤트 파싱" << std::endl;
        std::cout << "acquisition_width: " << acquisition_width << std::endl;
        std::cout << "n_traces: " << n_traces << std::endl;
        std::cout << "dec_factor_log2: " << dec_factor_log2 << std::endl;
        std::cout << "channel_mask: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(16)
                  << channel_mask << std::endl;
        std::cout << std::dec;
    }
}

void OnlineMonitor::stopRunParser(uint64_t* datas) {
    uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
    uint32_t deadtime = datas[1] & 0xFFFFFFFF;
    if (verbose) {
        std::cout << "Stop Run 이벤트 파싱" << std::endl;
        std::cout << "timestamp: " << timestamp << std::endl;
        std::cout << "deadtime: " << deadtime << std::endl;
    }
}

void OnlineMonitor::eventParser(uint64_t* datas, int nWords) {
    int channel = (datas[0] >> 56) & 0x7F;
    bool special_event = (datas[0] >> 55) & 0x1;
    uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
    bool with_waveform = (datas[1] >> 62) & 0x1;
    uint16_t flags_low_priority = (datas[1] >> 50) & 0x3FF;
    uint16_t flags_high_priority = (datas[1] >> 42) & 0xFF;
    uint16_t energy_short = (datas[1] >> 26) & 0xFFFF;
    uint16_t fine_timestamp = (datas[1] >> 16) & 0xFFFF;
    uint16_t energy = datas[1] & 0xFFFF;

    histograms[channel]->Fill(energy);

    if (verbose) {
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