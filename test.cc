#include <fstream>
#include <iomanip>
#include <iostream>

#include "CAENV2740.hxx"
#include "CAENV2740Event.hxx"
#include "CAENV2740Par.hxx"

int main() {
    std::cout << "CAEN V2740 디지타이저 테스트 프로그램\n";
    std::cout << "이 프로그램은 CAEN V2740 디지타이저의 기본 정보를 읽고 출력하기 위한 것입니다.\n";

    try {
        CAENV2740 v2740("dig2://192.168.0.10");  // PCI 연결 문자열 예시
        v2740.connect();

        v2740.printDigitizerInfo();
        v2740.printLicenseInfo();
        v2740.printSpecInfo();
        v2740.printNetworkInfo();

        v2740.printClockInfo();
        v2740.printTriggerSourceInfo();
        v2740.printPanelIOInfo();

        v2740.printWaveParamInfo();
        v2740.printWaveProbeInfo();
        v2740.printRawDataInfo();

        v2740.printCounterInfo();
        v2740.printITLInfo();
        v2740.printLVDSInfo();
        v2740.printPanelDACInfo();

        v2740.printInputInfo();
        v2740.printEventInfo();
        v2740.printDPPPSDInfo();

    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << '\n';
    }

    return 0;
}