// CAENV2740.cpp
#include "v2740.hxx"

#include <stdexcept>

CAENV2740::CAENV2740(const std::string& connString) : connectionString(connString), handle(0) {}

CAENV2740::~CAENV2740() {
    if (handle != 0) {
        CAEN_FELib_Close(handle);
    }
}

void CAENV2740::connect() {
    int ret = CAEN_FELib_Open(connectionString.c_str(), &handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 연결 실패");
    }
}

void CAENV2740::reset() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/reset");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 리셋 실패");
    }
}

void CAENV2740::reboot() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/reboot");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 리부트 실패");
    }
}

void CAENV2740::clear() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/clearData");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 클리어 실패");
    }
}

void CAENV2740::configure() {
    // 여기에 V2740 설정 코드를 추가하세요
    // 예: 샘플링 레이트, 트리거 설정 등
}

void CAENV2740::startAcquisition() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/swStartAcquisition");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 수집 시작 실패");
    }
}

void CAENV2740::stopAcquisition() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/swStopAcquisition");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 수집 중지 실패");
    }
}

uint32_t CAENV2740::readRegister(const std::string& registerPath) {
    char value[32];  // 충분한 크기의 문자 배열 선언
    int ret = CAEN_FELib_GetValue(handle, registerPath.c_str(), value);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 읽기 실패: " + registerPath);
    }
    return std::stoul(value);  // 문자열을 uint32_t로 변환
}

std::string CAENV2740::readRegisterInString(const std::string& registerPath) {
    char value[32];  // 충분한 크기의 문자 배열 선언
    int ret = CAEN_FELib_GetValue(handle, registerPath.c_str(), value);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 읽기 실패: " + registerPath);
    }
    return std::string(value);
}

void CAENV2740::writeRegister(const std::string& registerPath, uint32_t value) {
    std::string valueStr = std::to_string(value);
    int ret = CAEN_FELib_SetValue(handle, registerPath.c_str(), valueStr.c_str());
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 쓰기 실패: " + registerPath);
    }
}

std::string CAENV2740::readCUPVersion() { return readRegisterInString("/par/CupVer"); }
std::string CAENV2740::readFPGAFWVersion() { return readRegisterInString("/par/FPGA_FwVer"); }
std::string CAENV2740::readFWType() { return readRegisterInString("/par/FwType"); }
std::string CAENV2740::readModelCode() { return readRegisterInString("/par/ModelCode"); }
std::string CAENV2740::readPBCode() { return readRegisterInString("/par/PBCode"); }
std::string CAENV2740::readModelName() { return readRegisterInString("/par/ModelName"); }
std::string CAENV2740::readFormFactor() { return readRegisterInString("/par/FormFactor"); }
std::string CAENV2740::readFamilyCode() { return readRegisterInString("/par/FamilyCode"); }
std::string CAENV2740::readSerialNumber() { return readRegisterInString("/par/SerialNum"); }

std::string CAENV2740::readPCBRevMB() { return readRegisterInString("/par/PCBrev_MB"); }
std::string CAENV2740::readPCBRevPB() { return readRegisterInString("/par/PCBrev_PB"); }

std::string CAENV2740::readLicense() { return readRegisterInString("/par/License"); }
std::string CAENV2740::readLicenseStatus() { return readRegisterInString("/par/LicenseStatus"); }
std::string CAENV2740::readLicenseRemainingTime() { return readRegisterInString("/par/LicenseRemainingTime"); }

int CAENV2740::readNumberOfChannels() { return readRegister("/par/NumCh"); }
int CAENV2740::readADCResolution() { return readRegister("/par/ADC_Nbit"); }
int CAENV2740::readADCSamplingRate() { return readRegister("/par/ADC_SamplRate"); }
int CAENV2740::readInputRange() { return readRegister("/par/InputRange"); }
std::string CAENV2740::readInputType() { return readRegisterInString("/par/InputType"); }
int CAENV2740::readInputImpedance() { return readRegister("/par/Zin"); }

std::string CAENV2740::readSFPLinkPresence() { return readRegisterInString("/par/SFPLinkPresence"); }
std::string CAENV2740::readSFPLinkActive() { return readRegisterInString("/par/SFPLinkActive"); }
std::string CAENV2740::readSFPLinkProtocol() { return readRegisterInString("/par/SFPLinkProtocol"); }

std::string CAENV2740::readIPAddress() { return readRegisterInString("/par/IPAddress"); }
std::string CAENV2740::readSubnetMask() { return readRegisterInString("/par/Netmask"); }
std::string CAENV2740::readGateway() { return readRegisterInString("/par/Gateway"); }

uint32_t CAENV2740::readChannelThreshold(int channel) {
    std::string registerPath = "/ch/" + std::to_string(channel) + "/threshold";
    return readRegister(registerPath);
}

void CAENV2740::writeChannelThreshold(int channel, uint32_t value) {
    std::string registerPath = "/ch/" + std::to_string(channel) + "/threshold";
    writeRegister(registerPath, value);
}

uint32_t CAENV2740::readTriggerMask() { return readRegister("/board/trigger_mask"); }

void CAENV2740::writeTriggerMask(uint32_t mask) { writeRegister("/board/trigger_mask", mask); }

uint32_t CAENV2740::readSamplingRate() { return readRegister("/board/sampling_rate"); }

void CAENV2740::writeSamplingRate(uint32_t rate) { writeRegister("/board/sampling_rate", rate); }

uint32_t CAENV2740::readAcquisitionMode() { return readRegister("/board/acquisition_mode"); }

void CAENV2740::writeAcquisitionMode(uint32_t mode) { writeRegister("/board/acquisition_mode", mode); }

uint32_t CAENV2740::readChannelEnable(int channel) {
    std::string registerPath = "/ch/" + std::to_string(channel) + "/enable";
    return readRegister(registerPath);
}

void CAENV2740::writeChannelEnable(int channel, bool enable) {
    std::string registerPath = "/ch/" + std::to_string(channel) + "/enable";
    writeRegister(registerPath, enable ? 1 : 0);
}
