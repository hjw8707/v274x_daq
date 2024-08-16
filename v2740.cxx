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

float CAENV2740::readRegisterInFloat(const std::string& registerPath) {
    char value[32];  // 충분한 크기의 문자 배열 선언
    int ret = CAEN_FELib_GetValue(handle, registerPath.c_str(), value);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 읽기 실패: " + registerPath);
    }
    return std::stof(value);  // 문자열을 float로 변환
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

void CAENV2740::writeRegister(const std::string& registerPath, float value) {
    std::string valueStr = std::to_string(value);
    int ret = CAEN_FELib_SetValue(handle, registerPath.c_str(), valueStr.c_str());
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 쓰기 실패: " + registerPath);
    }
}

void CAENV2740::writeRegister(const std::string& registerPath, std::string value) {
    int ret = CAEN_FELib_SetValue(handle, registerPath.c_str(), value.c_str());
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

std::string CAENV2740::readClockSource() { return readRegisterInString("/par/ClockSource"); }
void CAENV2740::writeClockSource(std::string source) { writeRegister("/par/ClockSource", source); }
std::string CAENV2740::readEnClockOutFP() { return readRegisterInString("/par/EnClockOutFP"); }
void CAENV2740::writeEnClockOutFP(bool enable) { writeRegister("/par/EnClockOutFP", enable ? "True" : "False"); }

std::string CAENV2740::readSFPLinkPresence() { return readRegisterInString("/par/SFPLinkPresence"); }
std::string CAENV2740::readSFPLinkActive() { return readRegisterInString("/par/SFPLinkActive"); }
std::string CAENV2740::readSFPLinkProtocol() { return readRegisterInString("/par/SFPLinkProtocol"); }

std::string CAENV2740::readIPAddress() { return readRegisterInString("/par/IPAddress"); }
std::string CAENV2740::readSubnetMask() { return readRegisterInString("/par/Netmask"); }
std::string CAENV2740::readGateway() { return readRegisterInString("/par/Gateway"); }

std::string CAENV2740::readStartSource() { return readRegisterInString("/par/StartSource"); }
void CAENV2740::writeStartSource(std::string source) { writeRegister("/par/StartSource", source); }

std::string CAENV2740::readGlobalTriggerSource() { return readRegisterInString("/par/GlobalTriggerSource"); }
void CAENV2740::writeGlobalTriggerSource(std::string source) { writeRegister("/par/GlobalTriggerSource", source); }

std::string CAENV2740::readWaveTriggerSource(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveTriggerSource");
}
void CAENV2740::writeWaveTriggerSource(int channel, std::string source) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveTriggerSource", source);
}

std::string CAENV2740::readEventTriggerSource(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/EventTriggerSource");
}
void CAENV2740::writeEventTriggerSource(int channel, std::string source) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EventTriggerSource", source);
}

std::string CAENV2740::readChannelTriggerMask(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/ChannelsTriggerMask");
}
void CAENV2740::writeChannelTriggerMask(int channel, uint32_t mask) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChannelsTriggerMask", mask);
}

std::string CAENV2740::readWaveSaving(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveSaving");
}
void CAENV2740::writeWaveSaving(int channel, std::string mode) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveSaving", mode);
}

std::string CAENV2740::readTrgOutMode() { return readRegisterInString("/par/TrgOutMode"); }
void CAENV2740::writeTrgOutMode(std::string mode) { writeRegister("/par/TrgOutMode", mode); }

std::string CAENV2740::readGPIOMode() { return readRegisterInString("/par/GPIOMode"); }
void CAENV2740::writeGPIOMode(std::string mode) { writeRegister("/par/GPIOMode", mode); }

std::string CAENV2740::readBusyInSource() { return readRegisterInString("/par/BusyInSource"); }
void CAENV2740::writeBusyInSource(std::string source) { writeRegister("/par/BusyInSource", source); }

std::string CAENV2740::readSyncOutMode() { return readRegisterInString("/par/SyncOutMode"); }
void CAENV2740::writeSyncOutMode(std::string mode) { writeRegister("/par/SyncOutMode", mode); }

std::string CAENV2740::readBoardVetoSource() { return readRegisterInString("/par/BoardVetoSource"); }
void CAENV2740::writeBoardVetoSource(std::string source) { writeRegister("/par/BoardVetoSource", source); }

uint32_t CAENV2740::readBoardVetoWidth() { return readRegister("/par/BoardVetoWidth"); }
void CAENV2740::writeBoardVetoWidth(uint32_t width) { writeRegister("/par/BoardVetoWidth", width); }

std::string CAENV2740::readBoardVetoPolarity() { return readRegisterInString("/par/BoardVetoPolarity"); }
void CAENV2740::writeBoardVetoPolarity(std::string polarity) { writeRegister("/par/BoardVetoPolarity", polarity); }

std::string CAENV2740::readChannelVetoSource(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/ChannelVetoSource");
}
void CAENV2740::writeChannelVetoSource(int channel, std::string source) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChannelVetoSource", source);
}

uint32_t CAENV2740::readChannelVetoWidth(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ADCVetoWidth");
}
void CAENV2740::writeChannelVetoWidth(int channel, uint32_t width) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ADCVetoWidth", width);
}

uint32_t CAENV2740::readRunDelay() { return readRegister("/par/RunDelay"); }
void CAENV2740::writeRunDelay(uint32_t delay) { writeRegister("/par/RunDelay", delay); }

std::string CAENV2740::readEnAutoDisarmAcq() { return readRegisterInString("/par/EnAutoDisarmAcq"); }
void CAENV2740::writeEnAutoDisarmAcq(std::string enable) { writeRegister("/par/EnAutoDisarmAcq", enable); }

uint32_t CAENV2740::readAcquisitionStatus() { return readRegister("/par/AcquisitionStatus"); }
uint32_t CAENV2740::readLEDStatus() { return readRegister("/par/LEDStatus"); }

uint32_t CAENV2740::readVolatileClockOutDelay() { return readRegister("/par/VolatileClockOutDelay"); }
void CAENV2740::writeVolatileClockOutDelay(uint32_t delay) { writeRegister("/par/VolatileClockOutDelay", delay); }

uint32_t CAENV2740::readPermanentClockOutDelay() { return readRegister("/par/PermanentClockOutDelay"); }
void CAENV2740::writePermanentClockOutDelay(uint32_t delay) { writeRegister("/par/PermanentClockOutDelay", delay); }

uint32_t CAENV2740::readChRecordLength(int channel, bool isSample) {
    return isSample ? readChRecordLengthS(channel) : readChRecordLengthT(channel);
}

uint32_t CAENV2740::readChRecordLengthS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChRecordLengthS");
}

uint32_t CAENV2740::readChRecordLengthT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChRecordLengthT");
}

void CAENV2740::writeChRecordLength(int channel, uint32_t length, bool isSample) {
    isSample ? writeChRecordLengthS(channel, length) : writeChRecordLengthT(channel, length);
}

void CAENV2740::writeChRecordLengthS(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChRecordLengthS", length);
}

void CAENV2740::writeChRecordLengthT(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChRecordLengthT", length);
}

std::string CAENV2740::readWaveDownSamplingFactor(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveDownSamplingFactor");
}

void CAENV2740::writeWaveDownSamplingFactor(int channel, std::string factor) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveDownSamplingFactor", factor);
}

std::string CAENV2740::readWaveAnalogProbe(int num, int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveAnalogProbe" + std::to_string(num));
}

void CAENV2740::writeWaveAnalogProbe(int num, int channel, std::string probe) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveAnalogProbe" + std::to_string(num), probe);
}

std::string CAENV2740::readWaveDigitalProbe(int num, int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveDigitalProbe" + std::to_string(num));
}

void CAENV2740::writeWaveDigitalProbe(int num, int channel, std::string probe) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveDigitalProbe" + std::to_string(num), probe);
}

uint32_t CAENV2740::readMaxRawDataSize() { return readRegister("/par/MaxRawDataSize"); }

uint32_t CAENV2740::readChPreTrigger(int channel, bool isSample) {
    return isSample ? readChPreTriggerS(channel) : readChPreTriggerT(channel);
}

uint32_t CAENV2740::readChPreTriggerS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChPreTriggerS");
}

uint32_t CAENV2740::readChPreTriggerT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChPreTriggerT");
}

void CAENV2740::writeChPreTrigger(int channel, uint32_t preTrigger, bool isSample) {
    isSample ? writeChPreTriggerS(channel, preTrigger) : writeChPreTriggerT(channel, preTrigger);
}

void CAENV2740::writeChPreTriggerS(int channel, uint32_t preTrigger) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChPreTriggerS", preTrigger);
}

void CAENV2740::writeChPreTriggerT(int channel, uint32_t preTrigger) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChPreTriggerT", preTrigger);
}

std::string CAENV2740::readWaveDataSource(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveDataSource");
}

void CAENV2740::writeWaveDataSource(int channel, std::string source) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveDataSource", source);
}

uint32_t CAENV2740::readTestPulsePeriod() { return readRegister("/par/TestPulsePeriod"); }
void CAENV2740::writeTestPulsePeriod(uint32_t period) { writeRegister("/par/TestPulsePeriod", period); }
uint32_t CAENV2740::readTestPulseWidth() { return readRegister("/par/TestPulseWidth"); }
void CAENV2740::writeTestPulseWidth(uint32_t width) { writeRegister("/par/TestPulseWidth", width); }
uint32_t CAENV2740::readTestPulseLowLevel() { return readRegister("/par/TestPulseLowLevel"); }
void CAENV2740::writeTestPulseLowLevel(uint32_t level) { writeRegister("/par/TestPulseLowLevel", level); }
uint32_t CAENV2740::readTestPulseHighLevel() { return readRegister("/par/TestPulseHighLevel"); }
void CAENV2740::writeTestPulseHighLevel(uint32_t level) { writeRegister("/par/TestPulseHighLevel", level); }
std::string CAENV2740::readIOlevel() { return readRegisterInString("/par/IOlevel"); }
void CAENV2740::writeIOlevel(std::string level) { writeRegister("/par/IOlevel", level); }

float CAENV2740::readTempSens(std::string sensor) {
    return readRegisterInFloat("/par/TempSens" + sensor);
}  // sensor = "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5", "ADC6", "ADC7", "DCDC"
float CAENV2740::readSensDCDC(std::string type) { return readRegisterInFloat("/par/" + type + "SensDCDC"); }
float CAENV2740::readFreqSensCore() { return readRegisterInFloat("/par/FreqSensCore"); }
float CAENV2740::readDutyCycleSensDCDC() { return readRegisterInFloat("/par/DutyCycleSensDCDC"); }
float CAENV2740::readSpeedSensFan(int num) { return readRegisterInFloat("/par/SpeedSensFan" + std::to_string(num)); }

uint32_t CAENV2740::readErrorFlagMask() { return readRegister("/par/ErrorFlagMask"); }
void CAENV2740::writeErrorFlagMask(uint32_t mask) { writeRegister("/par/ErrorFlagMask", mask); }
uint32_t CAENV2740::readErrorFlagDataMask() { return readRegister("/par/ErrorFlagDataMask"); }
void CAENV2740::writeErrorFlagDataMask(uint32_t mask) { writeRegister("/par/ErrorFlagDataMask", mask); }
uint32_t CAENV2740::readErrorFlags() { return readRegister("/par/ErrorFlags"); }

std::string CAENV2740::readBoardReady() { return readRegisterInString("/par/BoardReady"); }
float CAENV2740::readChRealTimeMonitor(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChRealTimeMonitor");
}
float CAENV2740::readChDeadTimeMonitor(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChDeadTimeMonitor");
}
uint32_t CAENV2740::readChTriggerCnt(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChTriggerCnt");
}
uint32_t CAENV2740::readChSavedEventCnt(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChSavedEventCnt");
}

uint32_t CAENV2740::readChWaveCnt(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChWaveCnt");
}