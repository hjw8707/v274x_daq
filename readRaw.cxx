#include <fstream>
#include <iostream>

#include "CAENV2740Event.hxx"
#include "TFile.h"
#include "TTree.h"

void ReadCAENV2740Event(const std::string &filename) {
    CAENV2740Event event;
    std::ifstream inputFile(filename, std::ios::binary);
    TFile *file = new TFile("output.root", "RECREATE");
    TTree *tree = new TTree("EventTree", "CAENV2740 Event Tree");

    tree->Branch("channel", &event.channel, "channel/b");
    tree->Branch("timestamp", &event.timestamp, "timestamp/l");
    tree->Branch("fine_timestamp", &event.fine_timestamp, "fine_timestamp/s");
    tree->Branch("energy", &event.energy, "energy/s");
    tree->Branch("energy_short", &event.energy_short, "energy_short/s");
    tree->Branch("flags_low_priority", &event.flags_low_priority, "flags_low_priority/s");
    tree->Branch("flags_high_priority", &event.flags_high_priority, "flags_high_priority/s");
    tree->Branch("event_size", &event.event_size, "event_size/i");

    if (!inputFile) {
        std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
        return;
    }
    static int eventCount = 0;
    while (inputFile) {
        inputFile.read(reinterpret_cast<char *>(&event.channel), sizeof(event.channel));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.timestamp), sizeof(event.timestamp));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.fine_timestamp), sizeof(event.fine_timestamp));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.energy), sizeof(event.energy));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.energy_short), sizeof(event.energy_short));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.flags_low_priority), sizeof(event.flags_low_priority));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.flags_high_priority), sizeof(event.flags_high_priority));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료
        inputFile.read(reinterpret_cast<char *>(&event.event_size), sizeof(event.event_size));
        if (!inputFile) break;  // 파일 읽기에 실패하면 루프를 종료

        // 읽은 데이터를 root tree에 저장
        tree->Fill();
        eventCount++;
        if (eventCount % 100 == 0) {
            std::cout << "\r현재까지 읽은 이벤트 수: " << eventCount << std::flush;
        }
    }

    inputFile.close();
    tree->Write();
    file->Close();
}

int main() {
    ReadCAENV2740Event("test.out");  // 파일 이름을 적절히 변경하세요
    return 0;
}