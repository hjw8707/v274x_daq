#include <TH1.h>

#include <fstream>
#include <iostream>

#include "CAENV2740Event.hxx"
#include "TFile.h"
#include "TTree.h"

void ReadCAENV2740Event(const std::string &filename) {
    CAENV2740Event event;                                          // CAEN 2740 이벤트 구조체 선언
    std::ifstream inputFile(filename, std::ios::binary);           // 이진 파일 열기
    TFile *file = new TFile("output.root", "RECREATE");            // ROOT 파일 생성
    TTree *tree = new TTree("EventTree", "CAENV2740 Event Tree");  // ROOT 트리 생성
    TH1F *energyHist = new TH1F("energyHist", "Energy Histogram", 100, 0, 1000);  // 에너지 히스토그램 생성

    // 트리에 저장할 데이터 브랜치 정의
    tree->Branch("channel", &event.channel, "channel/b");
    tree->Branch("timestamp", &event.timestamp, "timestamp/l");
    tree->Branch("fine_timestamp", &event.fine_timestamp, "fine_timestamp/s");
    tree->Branch("energy", &event.energy, "energy/s");
    tree->Branch("energy_short", &event.energy_short, "energy_short/s");
    tree->Branch("flags_low_priority", &event.flags_low_priority, "flags_low_priority/s");
    tree->Branch("flags_high_priority", &event.flags_high_priority, "flags_high_priority/s");
    tree->Branch("event_size", &event.event_size, "event_size/i");

    // 파일 열기 실패 시 에러 메시지 출력
    if (!inputFile) {
        std::cerr << "파일을 열 수 없습니다: " << filename << std::endl;
        return;
    }

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
        if (!inputFile) break;           // 파일 읽기에 실패하면 루프 종료
        energyHist->Fill(event.energy);  // 에너지 히스토그램에 데이터 추가
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

    inputFile.close();    // 파일 닫기
    tree->Write();        // 트리 저장
    energyHist->Write();  // 에너지 히스토그램 저장
}

int main() {
    ReadCAENV2740Event("test.out");  // 파일 이름을 적절히 변경하세요
    return 0;
}