#include <iostream>

#include "v2740.hxx"

int main() {
    std::cout << "CAEN V2740 디지타이저 테스트 프로그램\n";
    std::cout << "이 프로그램은 CAEN V2740 디지타이저의 기본 정보를 읽고 출력하기 위한 것입니다.\n";
    try {
        CAENV2740 v2740("dig2://192.168.0.10");  // PCI 연결 문자열 예시
        v2740.connect();

        std::cout << "모델 이름: " << v2740.readModelName() << '\n';
        std::cout << "시리얼 번호: " << v2740.readSerialNumber() << '\n';
        std::cout << "펌웨어 버전: " << v2740.readFPGAFWVersion() << '\n';

        int numChannels = v2740.readNumberOfChannels();
        std::cout << "채널 수: " << numChannels << '\n';

        // 다른 함수들 테스트
        std::cout << "ADC 해상도: " << v2740.readADCResolution() << " 비트\n";
        std::cout << "ADC 샘플링 레이트: " << v2740.readADCSamplingRate() << " Hz\n";
        std::cout << "입력 범위: " << v2740.readInputRange() << " mV\n";
        std::cout << "입력 타입: " << v2740.readInputType() << '\n';
        std::cout << "입력 임피던스: " << v2740.readInputImpedance() << " Ω\n";

        std::cout << "SFP 링크 존재 여부: " << v2740.readSFPLinkPresence() << '\n';
        std::cout << "SFP 링크 활성화 여부: " << v2740.readSFPLinkActive() << '\n';
        std::cout << "SFP 링크 프로토콜: " << v2740.readSFPLinkProtocol() << '\n';

        std::cout << "IP 주소: " << v2740.readIPAddress() << '\n';
        std::cout << "서브넷 마스크: " << v2740.readSubnetMask() << '\n';
        std::cout << "게이트웨이: " << v2740.readGateway() << '\n';

    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << '\n';
    }
    return 0;
}