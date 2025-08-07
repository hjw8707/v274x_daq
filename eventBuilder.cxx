#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

// 시간 윈도우(ns) 설정 (예: 1000ns = 1us)
const uint64_t TIME_WINDOW_NS = 1000; // 필요에 따라 조정

// fine_timestamp 단위: 7.8125 ps
const double FINE_TIMESTAMP_UNIT_PS = 7.8125;
const double COARSE_TIMESTAMP_UNIT_NS = 8.0;

struct MergedEvent {
    std::vector<UChar_t> channel;
    std::vector<ULong64_t> timestamp;
    std::vector<UShort_t> fine_timestamp;
    std::vector<UShort_t> energy;
    std::vector<UShort_t> energy_short;
    std::vector<UShort_t> flags_low_priority;
    std::vector<UShort_t> flags_high_priority;
    std::vector<UInt_t> event_size;
    // 대표 timestamp(첫번째 entry의 timestamp)
    ULong64_t merged_timestamp;
    UShort_t merged_fine_timestamp;
};

double get_abs_time_ps(ULong64_t timestamp, UShort_t fine_timestamp) {
    // timestamp: 8ns 단위, fine_timestamp: 7.8125ps 단위
    return timestamp * COARSE_TIMESTAMP_UNIT_NS * 1000.0 + fine_timestamp * FINE_TIMESTAMP_UNIT_PS;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "사용법: ./eventBuilder <input.root> <output.root> [윈도우(ns)]" << std::endl;
        return 1;
    }
    std::string inputFileName = argv[1];
    std::string outputFileName = argv[2];
    uint64_t time_window_ns = TIME_WINDOW_NS;
    if (argc >= 4) {
        time_window_ns = std::stoull(argv[3]);
    }

    // 입력 파일 열기
    TFile* inFile = TFile::Open(inputFileName.c_str(), "READ");
    if (!inFile || inFile->IsZombie()) {
        std::cerr << "입력 파일을 열 수 없습니다: " << inputFileName << std::endl;
        return 1;
    }
    TTree* inTree = (TTree*)inFile->Get("EventTree");
    if (!inTree) {
        std::cerr << "EventTree를 찾을 수 없습니다." << std::endl;
        inFile->Close();
        return 1;
    }

    // 입력 브랜치 변수 선언
    UChar_t channel;
    ULong64_t timestamp;
    UShort_t fine_timestamp;
    UShort_t energy;
    UShort_t energy_short;
    UShort_t flags_low_priority;
    UShort_t flags_high_priority;
    UInt_t event_size;

    inTree->SetBranchAddress("channel", &channel);
    inTree->SetBranchAddress("timestamp", &timestamp);
    inTree->SetBranchAddress("fine_timestamp", &fine_timestamp);
    inTree->SetBranchAddress("energy", &energy);
    inTree->SetBranchAddress("energy_short", &energy_short);
    inTree->SetBranchAddress("flags_low_priority", &flags_low_priority);
    inTree->SetBranchAddress("flags_high_priority", &flags_high_priority);
    inTree->SetBranchAddress("event_size", &event_size);

    // 출력 파일 및 트리 생성
    TFile* outFile = new TFile(outputFileName.c_str(), "RECREATE");
    TTree* outTree = new TTree("MergedEventTree", "Merged Event Tree");

    // 출력 브랜치 변수 (vector)
    std::vector<UChar_t>* v_channel = nullptr;
    std::vector<ULong64_t>* v_timestamp = nullptr;
    std::vector<UShort_t>* v_fine_timestamp = nullptr;
    std::vector<UShort_t>* v_energy = nullptr;
    std::vector<UShort_t>* v_energy_short = nullptr;
    std::vector<UShort_t>* v_flags_low_priority = nullptr;
    std::vector<UShort_t>* v_flags_high_priority = nullptr;
    std::vector<UInt_t>* v_event_size = nullptr;
    ULong64_t merged_timestamp = 0;
    UShort_t merged_fine_timestamp = 0;

    outTree->Branch("channel", &v_channel);
    outTree->Branch("timestamp", &v_timestamp);
    outTree->Branch("fine_timestamp", &v_fine_timestamp);
    outTree->Branch("energy", &v_energy);
    outTree->Branch("energy_short", &v_energy_short);
    outTree->Branch("flags_low_priority", &v_flags_low_priority);
    outTree->Branch("flags_high_priority", &v_flags_high_priority);
    outTree->Branch("event_size", &v_event_size);
    outTree->Branch("merged_timestamp", &merged_timestamp, "merged_timestamp/l");
    outTree->Branch("merged_fine_timestamp", &merged_fine_timestamp, "merged_fine_timestamp/s");

    // 모든 이벤트를 메모리로 읽어서 정렬 (timestamp, fine_timestamp 기준)
    struct Entry {
        UChar_t channel;
        ULong64_t timestamp;
        UShort_t fine_timestamp;
        UShort_t energy;
        UShort_t energy_short;
        UShort_t flags_low_priority;
        UShort_t flags_high_priority;
        UInt_t event_size;
    };
    std::vector<Entry> entries;
    Long64_t nEntries = inTree->GetEntries();
    entries.reserve(nEntries);

    for (Long64_t i = 0; i < nEntries; ++i) {
        inTree->GetEntry(i);
        entries.push_back({channel, timestamp, fine_timestamp, energy, energy_short,
                           flags_low_priority, flags_high_priority, event_size});
    }

    // timestamp, fine_timestamp 기준 정렬
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.timestamp != b.timestamp)
            return a.timestamp < b.timestamp;
        return a.fine_timestamp < b.fine_timestamp;
    });

    // 이벤트 병합
    size_t idx = 0;
    while (idx < entries.size()) {
        // 새로운 병합 이벤트 시작
        v_channel = new std::vector<UChar_t>();
        v_timestamp = new std::vector<ULong64_t>();
        v_fine_timestamp = new std::vector<UShort_t>();
        v_energy = new std::vector<UShort_t>();
        v_energy_short = new std::vector<UShort_t>();
        v_flags_low_priority = new std::vector<UShort_t>();
        v_flags_high_priority = new std::vector<UShort_t>();
        v_event_size = new std::vector<UInt_t>();

        const Entry& first = entries[idx];
        double ref_time_ps = get_abs_time_ps(first.timestamp, first.fine_timestamp);
        merged_timestamp = first.timestamp;
        merged_fine_timestamp = first.fine_timestamp;

        // 윈도우 내에 들어오는 entry를 모두 병합
        size_t j = idx;
        for (; j < entries.size(); ++j) {
            double cur_time_ps = get_abs_time_ps(entries[j].timestamp, entries[j].fine_timestamp);
            if ((cur_time_ps - ref_time_ps) / 1000.0 > time_window_ns) break; // ns 단위로 비교
            v_channel->push_back(entries[j].channel);
            v_timestamp->push_back(entries[j].timestamp);
            v_fine_timestamp->push_back(entries[j].fine_timestamp);
            v_energy->push_back(entries[j].energy);
            v_energy_short->push_back(entries[j].energy_short);
            v_flags_low_priority->push_back(entries[j].flags_low_priority);
            v_flags_high_priority->push_back(entries[j].flags_high_priority);
            v_event_size->push_back(entries[j].event_size);
        }
        outTree->Fill();
        // 메모리 해제
        delete v_channel;
        delete v_timestamp;
        delete v_fine_timestamp;
        delete v_energy;
        delete v_energy_short;
        delete v_flags_low_priority;
        delete v_flags_high_priority;
        delete v_event_size;
        idx = j;
    }

    outFile->cd();
    outTree->Write();
    outFile->Close();
    inFile->Close();

    std::cout << "병합 완료! 결과 파일: " << outputFileName << std::endl;
    return 0;
}




