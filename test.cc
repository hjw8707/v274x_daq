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

        // 추가된 함수들 테스트
        std::cout << "CUP 버전: " << v2740.readCUPVersion() << '\n';
        std::cout << "펌웨어 타입: " << v2740.readFWType() << '\n';
        std::cout << "모델 코드: " << v2740.readModelCode() << '\n';
        std::cout << "PB 코드: " << v2740.readPBCode() << '\n';
        std::cout << "폼 팩터: " << v2740.readFormFactor() << '\n';
        std::cout << "패밀리 코드: " << v2740.readFamilyCode() << '\n';
        std::cout << "메인보드 PCB 리비전: " << v2740.readPCBRevMB() << '\n';
        std::cout << "피기백 PCB 리비전: " << v2740.readPCBRevPB() << '\n';
        std::cout << "라이선스: " << v2740.readLicense() << '\n';
        std::cout << "라이선스 상태: " << v2740.readLicenseStatus() << '\n';
        std::cout << "라이선스 남은 시간: " << v2740.readLicenseRemainingTime() << '\n';

        std::cout << "클럭 소스: " << v2740.readClockSource() << '\n';
        std::cout << "FP 클럭 출력 활성화 여부: " << v2740.readEnClockOutFP() << '\n';

        std::cout << "시작 소스: " << v2740.readStartSource() << '\n';
        std::cout << "글로벌 트리거 소스: " << v2740.readGlobalTriggerSource() << '\n';
        std::cout << "채널 0의 파형 트리거 소스: " << v2740.readWaveTriggerSource(0) << '\n';
        std::cout << "채널 0의 이벤트 트리거 소스: " << v2740.readEventTriggerSource(0) << '\n';
        std::cout << "채널 0의 트리거 마스크: " << v2740.readChannelTriggerMask(0) << '\n';
        std::cout << "채널 0의 파형 저장 모드: " << v2740.readWaveSaving(0) << '\n';
        v2740.writeTrgOutMode("Fixed0");
        std::cout << "트리거 출력 모드: " << v2740.readTrgOutMode() << '\n';
        std::cout << "GPIO 모드: " << v2740.readGPIOMode() << '\n';
        std::cout << "Busy 입력 소스: " << v2740.readBusyInSource() << '\n';
        std::cout << "동기화 출력 모드: " << v2740.readSyncOutMode() << '\n';
        std::cout << "보드 베토 소스: " << v2740.readBoardVetoSource() << '\n';
        std::cout << "보드 베토 폭: " << v2740.readBoardVetoWidth() << '\n';
        std::cout << "보드 베토 극성: " << v2740.readBoardVetoPolarity() << '\n';
        std::cout << "채널 0의 베토 소스: " << v2740.readChannelVetoSource(0) << '\n';
        std::cout << "채널 0의 베토 폭: " << v2740.readChannelVetoWidth(0) << '\n';
        v2740.writeRunDelay(0);
        std::cout << "실행 지연: " << v2740.readRunDelay() << '\n';
        std::cout << "자동 획득 해제 활성화: " << v2740.readEnAutoDisarmAcq() << '\n';
        std::cout << "획득 상태: 0x" << std::hex << v2740.readAcquisitionStatus() << std::dec << '\n';
        std::cout << "LED 상태: 0x" << std::hex << v2740.readLEDStatus() << std::dec << '\n';
        std::cout << "휘발성 클럭 출력 지연: " << v2740.readVolatileClockOutDelay() << '\n';
        std::cout << "영구 클럭 출력 지연: " << v2740.readPermanentClockOutDelay() << '\n';

        std::cout << "채널 0의 레코드 길이 (샘플): " << v2740.readChRecordLengthS(0) << '\n';
        std::cout << "채널 0의 레코드 길이 (시간): " << v2740.readChRecordLengthT(0) << '\n';
        v2740.writeChRecordLengthS(0, 1000);
        v2740.writeChRecordLengthT(0, 1000);
        std::cout << "채널 0의 파형 다운샘플링 팩터: " << v2740.readWaveDownSamplingFactor(0) << '\n';
        v2740.writeWaveDownSamplingFactor(0, "1");
        std::cout << "채널 0의 파형 아날로그 프로브 1: " << v2740.readWaveAnalogProbe(1, 0) << '\n';
        v2740.writeWaveAnalogProbe(1, 0, "ADC_DATA");
        std::cout << "트리거 히스테리시스 (채널 0): " << v2740.readTriggerHysteresis(0) << '\n';
        v2740.writeTriggerHysteresis(0, "1");
        std::cout << "CFD 지연 (채널 0, 샘플): " << v2740.readCFDDelayS(0) << '\n';
        v2740.writeCFDDelayS(0, 10);
        std::cout << "CFD 지연 (채널 0, 시간): " << v2740.readCFDDelayT(0) << '\n';
        v2740.writeCFDDelayT(0, 10);
        std::cout << "CFD 분율 (채널 0): " << v2740.readCFDFraction(0) << '\n';
        v2740.writeCFDFraction(0, 50);
        std::cout << "트리거 필터 선택 (채널 0): " << v2740.readTriggerFilterSelection(0) << '\n';
        v2740.writeTriggerFilterSelection(0, "Trapezoid");
        std::cout << "ADC 입력 기준선 평균 (채널 0): " << v2740.readADCInputBaselineAvg(0) << '\n';
        v2740.writeADCInputBaselineAvg(0, "16");
        std::cout << "ADC 입력 기준선 가드 (채널 0, 샘플): " << v2740.readADCInputBaselineGuardS(0) << '\n';
        v2740.writeADCInputBaselineGuardS(0, 100);
        std::cout << "ADC 입력 기준선 가드 (채널 0, 시간): " << v2740.readADCInputBaselineGuardT(0) << '\n';
        v2740.writeADCInputBaselineGuardT(0, 100);
        std::cout << "파일업 갭 (채널 0): " << v2740.readPileupGap(0) << '\n';
        v2740.writePileupGap(0, 50);
        std::cout << "절대 기준선 (채널 0): " << v2740.readAbsoluteBaseline(0) << '\n';
        v2740.writeAbsoluteBaseline(0, 1000);
        std::cout << "게이트 오프셋 (채널 0, 샘플): " << v2740.readGateOffsetS(0) << '\n';
        v2740.writeGateOffsetS(0, 10);
        std::cout << "게이트 오프셋 (채널 0, 시간): " << v2740.readGateOffsetT(0) << '\n';
        v2740.writeGateOffsetT(0, 10);
        std::cout << "게이트 긴 길이 (채널 0, 샘플): " << v2740.readGateLongLengthS(0) << '\n';
        v2740.writeGateLongLengthS(0, 100);
        std::cout << "게이트 긴 길이 (채널 0, 시간): " << v2740.readGateLongLengthT(0) << '\n';
        v2740.writeGateLongLengthT(0, 100);
        std::cout << "게이트 짧은 길이 (채널 0, 샘플): " << v2740.readGateShortLengthS(0) << '\n';
        v2740.writeGateShortLengthS(0, 50);
        std::cout << "게이트 짧은 길이 (채널 0, 시간): " << v2740.readGateShortLengthT(0) << '\n';
        v2740.writeGateShortLengthT(0, 50);
        std::cout << "긴 전하 적분기 페데스탈 (채널 0): " << v2740.readLongChargeIntegratorPedestal(0) << '\n';
        v2740.writeLongChargeIntegratorPedestal(0, 100);
        std::cout << "짧은 전하 적분기 페데스탈 (채널 0): " << v2740.readShortChargeIntegratorPedestal(0) << '\n';
        v2740.writeShortChargeIntegratorPedestal(0, 50);
        std::cout << "에너지 이득 (채널 0): " << v2740.readEnergyGain(0) << '\n';
        v2740.writeEnergyGain(0, "1");
        std::cout << "중성자 임계값 (채널 0): " << v2740.readNeutronThreshold(0) << '\n';
        v2740.writeNeutronThreshold(0, 1000);
        std::cout << "데이터 감소 활성화: " << v2740.readEnDataReduction() << '\n';
        v2740.writeEnDataReduction(true);
        std::cout << "통계 이벤트 활성화: " << v2740.readEnStatEvents() << '\n';
        v2740.writeEnStatEvents(true);

    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << '\n';
    }
    return 0;
}