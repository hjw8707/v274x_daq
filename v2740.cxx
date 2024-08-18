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
void CAENV2740::writeEnAutoDisarmAcq(bool enable) { writeRegister("/par/EnAutoDisarmAcq", enable ? "True" : "False"); }

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

std::string CAENV2740::readLVDSMode(int channel) {
    return readRegisterInString("/lvds/" + std::to_string(channel) + "/par/LVDSMode");
}
void CAENV2740::writeLVDSMode(int channel, std::string mode) {
    writeRegister("/lvds/" + std::to_string(channel) + "/par/LVDSMode", mode);
}
std::string CAENV2740::readLVDSDirection(int channel) {
    return readRegisterInString("/lvds/" + std::to_string(channel) + "/par/LVDSDirection");
}
void CAENV2740::writeLVDSDirection(int channel, std::string direction) {
    writeRegister("/lvds/" + std::to_string(channel) + "/par/LVDSDirection", direction);
}
std::string CAENV2740::readLVDSIOReg() { return readRegisterInString("/par/LVDSIOReg"); }
void CAENV2740::writeLVDSIOReg(std::string reg) { writeRegister("/par/LVDSIOReg", reg); }
std::string CAENV2740::readLVDSTrgMask() { return readRegisterInString("/par/LVDSTrgMask"); }
void CAENV2740::writeLVDSTrgMask(uint32_t mask) { writeRegister("/par/LVDSTrgMask", mask); }

std::string CAENV2740::readDACoutMode() { return readRegisterInString("/par/DACoutMode"); }
void CAENV2740::writeDACoutMode(std::string mode) { writeRegister("/par/DACoutMode", mode); }
uint32_t CAENV2740::readDACoutStaticLevel() { return readRegister("/par/DACoutStaticLevel"); }
void CAENV2740::writeDACoutStaticLevel(uint32_t level) { writeRegister("/par/DACoutStaticLevel", level); }
uint32_t CAENV2740::readDACoutChSelect() { return readRegister("/par/DACoutChSelect"); }
void CAENV2740::writeDACoutChSelect(uint32_t channel) { writeRegister("/par/DACoutChSelect", channel); }

uint32_t CAENV2740::readSelfTriggerWidth(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/SelfTriggerWidth");
}
void CAENV2740::writeSelfTriggerWidth(int channel, uint32_t width) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/SelfTriggerWidth", width);
}

uint32_t CAENV2740::readInputDelay(int group) {
    return readRegister("/group/" + std::to_string(group) + "/par/InputDelay");
}
void CAENV2740::writeInputDelay(int group, uint32_t delay) {
    writeRegister("/group/" + std::to_string(group) + "/par/InputDelay", delay);
}

std::string CAENV2740::readEnOffsetCalibration() { return readRegisterInString("/par/EnOffsetCalibration"); }
void CAENV2740::writeEnOffsetCalibration(bool enable) {
    writeRegister("/par/EnOffsetCalibration", enable ? "True" : "False");
}

std::string CAENV2740::readChEnable(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/ChEnable");
}
void CAENV2740::writeChEnable(int channel, bool enable) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChEnable", enable ? "True" : "False");
}

float CAENV2740::readSelfTrgRate(int channel) {
    return readRegisterInFloat("/ch/" + std::to_string(channel) + "/par/SelfTrgRate");
}
uint32_t CAENV2740::readChStatus(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ChStatus");
}

float CAENV2740::readDCOffset(int channel) {
    return readRegisterInFloat("/ch/" + std::to_string(channel) + "/par/DCOffset");
}
void CAENV2740::writeDCOffset(int channel, float offset) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/DCOffset", offset);
}

uint32_t CAENV2740::readSignalOffset(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/SignalOffset");
}
void CAENV2740::writeSignalOffset(int channel, uint32_t offset) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/SignalOffset", offset);
}

float CAENV2740::readGainFactor(int channel) {
    return readRegisterInFloat("/ch/" + std::to_string(channel) + "/par/GainFactor");
}
float CAENV2740::readADCToVolts(int channel) {
    return readRegisterInFloat("/ch/" + std::to_string(channel) + "/par/ADCToVolts");
}

uint32_t CAENV2740::readTriggerThr(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/TriggerThr");
}
void CAENV2740::writeTriggerThr(int channel, uint32_t threshold) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TriggerThr", threshold);
}

std::string CAENV2740::readPulsePolarity(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/PulsePolarity");
}
void CAENV2740::writePulsePolarity(int channel, std::string polarity) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/PulsePolarity", polarity);
}

uint32_t CAENV2740::readEnergySkimLowDiscriminator(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergySkimLowDiscriminator");
}
void CAENV2740::writeEnergySkimLowDiscriminator(int channel, uint32_t threshold) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergySkimLowDiscriminator", threshold);
}
uint32_t CAENV2740::readEnergySkimHighDiscriminator(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergySkimHighDiscriminator");
}
void CAENV2740::writeEnergySkimHighDiscriminator(int channel, uint32_t threshold) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergySkimHighDiscriminator", threshold);
}

std::string CAENV2740::readEventSelector(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/EventSelector");
}
void CAENV2740::writeEventSelector(int channel, std::string selector) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EventSelector", selector);
}
std::string CAENV2740::readWaveSelector(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveSelector");
}
void CAENV2740::writeWaveSelector(int channel, std::string selector) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveSelector", selector);
}

std::string CAENV2740::readEventNeutronReject(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/EventNeutronReject");
}
void CAENV2740::writeEventNeutronReject(int channel, std::string reject) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EventNeutronReject", reject);
}
std::string CAENV2740::readWaveNeutronReject(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveNeutronReject");
}
void CAENV2740::writeWaveNeutronReject(int channel, std::string reject) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveNeutronReject", reject);
}

std::string CAENV2740::readCoincidenceMask(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/CoincidenceMask");
}
void CAENV2740::writeCoincidenceMask(int channel, uint32_t mask) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CoincidenceMask", mask);
}
std::string CAENV2740::readAntiCoincidenceMask(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/AntiCoincidenceMask");
}
void CAENV2740::writeAntiCoincidenceMask(int channel, uint32_t mask) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/AntiCoincidenceMask", mask);
}

uint32_t CAENV2740::readCoincidenceLength(int channel, bool isSample) {
    return isSample ? readCoincidenceLengthS(channel) : readCoincidenceLengthT(channel);
}
void CAENV2740::writeCoincidenceLength(int channel, uint32_t length, bool isSample) {
    isSample ? writeCoincidenceLengthS(channel, length) : writeCoincidenceLengthT(channel, length);
}
uint32_t CAENV2740::readCoincidenceLengthS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/CoincidenceLengthS");
}
void CAENV2740::writeCoincidenceLengthS(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CoincidenceLengthS", length);
}
uint32_t CAENV2740::readCoincidenceLengthT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/CoincidenceLengthT");
}
void CAENV2740::writeCoincidenceLengthT(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CoincidenceLengthT", length);
}

uint32_t CAENV2740::readSmoothingFactor(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/SmoothingFactor");
}
void CAENV2740::writeSmoothingFactor(int channel, uint32_t factor) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/SmoothingFactor", factor);
}

std::string CAENV2740::readChargeSmoothing(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/ChargeSmoothing");
}

void CAENV2740::writeChargeSmoothing(int channel, std::string smoothing) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChargeSmoothing", smoothing);
}

std::string CAENV2740::readTimeFilterSmoothing(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/TimeFilterSmoothing");
}
void CAENV2740::writeTimeFilterSmoothing(int channel, std::string smoothing) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterSmoothing", smoothing);
}
uint32_t CAENV2740::readTimeFilterRetriggerGuard(int channel, bool isSample) {
    return isSample ? readTimeFilterRetriggerGuardS(channel) : readTimeFilterRetriggerGuardT(channel);
}
void CAENV2740::writeTimeFilterRetriggerGuard(int channel, uint32_t guard, bool isSample) {
    isSample ? writeTimeFilterRetriggerGuardS(channel, guard) : writeTimeFilterRetriggerGuardT(channel, guard);
}
uint32_t CAENV2740::readTimeFilterRetriggerGuardS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRetriggerGuardS");
}
void CAENV2740::writeTimeFilterRetriggerGuardS(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRetriggerGuardS", guard);
}
uint32_t CAENV2740::readTimeFilterRetriggerGuardT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRetriggerGuardT");
}
void CAENV2740::writeTimeFilterRetriggerGuardT(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRetriggerGuardT", guard);
}
std::string CAENV2740::readTriggerHysteresis(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/TriggerHysteresis");
}
void CAENV2740::writeTriggerHysteresis(int channel, std::string hysteresis) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TriggerHysteresis", hysteresis);
}

uint32_t CAENV2740::readCFDDelay(int channel, bool isSample) {
    return isSample ? readCFDDelayS(channel) : readCFDDelayT(channel);
}

void CAENV2740::writeCFDDelay(int channel, uint32_t delay, bool isSample) {
    isSample ? writeCFDDelayS(channel, delay) : writeCFDDelayT(channel, delay);
}

uint32_t CAENV2740::readCFDDelayS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/CFDDelayS");
}

void CAENV2740::writeCFDDelayS(int channel, uint32_t delay) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CFDDelayS", delay);
}

uint32_t CAENV2740::readCFDDelayT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/CFDDelayT");
}

void CAENV2740::writeCFDDelayT(int channel, uint32_t delay) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CFDDelayT", delay);
}

uint32_t CAENV2740::readCFDFraction(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/CFDFraction");
}

void CAENV2740::writeCFDFraction(int channel, uint32_t fraction) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CFDFraction", fraction);
}

std::string CAENV2740::readTriggerFilterSelection(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/TriggerFilterSelection");
}
void CAENV2740::writeTriggerFilterSelection(int channel, std::string selection) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TriggerFilterSelection", selection);
}
std::string CAENV2740::readADCInputBaselineAvg(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/ADCInputBaselineAvg");
}
void CAENV2740::writeADCInputBaselineAvg(int channel, std::string avg) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ADCInputBaselineAvg", avg);
}
uint32_t CAENV2740::readADCInputBaselineGuard(int channel, bool isSample) {
    return isSample ? readADCInputBaselineGuardS(channel) : readADCInputBaselineGuardT(channel);
}
void CAENV2740::writeADCInputBaselineGuard(int channel, uint32_t guard, bool isSample) {
    isSample ? writeADCInputBaselineGuardS(channel, guard) : writeADCInputBaselineGuardT(channel, guard);
}
uint32_t CAENV2740::readADCInputBaselineGuardS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ADCInputBaselineGuardS");
}
void CAENV2740::writeADCInputBaselineGuardS(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ADCInputBaselineGuardS", guard);
}
uint32_t CAENV2740::readADCInputBaselineGuardT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ADCInputBaselineGuardT");
}
void CAENV2740::writeADCInputBaselineGuardT(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ADCInputBaselineGuardT", guard);
}
uint32_t CAENV2740::readPileupGap(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/PileupGap");
}
void CAENV2740::writePileupGap(int channel, uint32_t gap) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/PileupGap", gap);
}
uint32_t CAENV2740::readAbsoluteBaseline(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/AbsoluteBaseline");
}
void CAENV2740::writeAbsoluteBaseline(int channel, uint32_t baseline) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/AbsoluteBaseline", baseline);
}
uint32_t CAENV2740::readGateOffset(int channel, bool isSample) {
    return isSample ? readGateOffsetS(channel) : readGateOffsetT(channel);
}
void CAENV2740::writeGateOffset(int channel, uint32_t offset, bool isSample) {
    isSample ? writeGateOffsetS(channel, offset) : writeGateOffsetT(channel, offset);
}
uint32_t CAENV2740::readGateOffsetS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/GateOffsetS");
}
void CAENV2740::writeGateOffsetS(int channel, uint32_t offset) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/GateOffsetS", offset);
}
uint32_t CAENV2740::readGateOffsetT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/GateOffsetT");
}
void CAENV2740::writeGateOffsetT(int channel, uint32_t offset) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/GateOffsetT", offset);
}
uint32_t CAENV2740::readGateLongLength(int channel, bool isSample) {
    return isSample ? readGateLongLengthS(channel) : readGateLongLengthT(channel);
}
void CAENV2740::writeGateLongLength(int channel, uint32_t length, bool isSample) {
    isSample ? writeGateLongLengthS(channel, length) : writeGateLongLengthT(channel, length);
}
uint32_t CAENV2740::readGateLongLengthS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/GateLongLengthS");
}
void CAENV2740::writeGateLongLengthS(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/GateLongLengthS", length);
}
uint32_t CAENV2740::readGateLongLengthT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/GateLongLengthT");
}
void CAENV2740::writeGateLongLengthT(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/GateLongLengthT", length);
}
uint32_t CAENV2740::readGateShortLength(int channel, bool isSample) {
    return isSample ? readGateShortLengthS(channel) : readGateShortLengthT(channel);
}
void CAENV2740::writeGateShortLength(int channel, uint32_t length, bool isSample) {
    isSample ? writeGateShortLengthS(channel, length) : writeGateShortLengthT(channel, length);
}
uint32_t CAENV2740::readGateShortLengthS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/GateShortLengthS");
}
void CAENV2740::writeGateShortLengthS(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/GateShortLengthS", length);
}
uint32_t CAENV2740::readGateShortLengthT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/GateShortLengthT");
}
void CAENV2740::writeGateShortLengthT(int channel, uint32_t length) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/GateShortLengthT", length);
}
uint32_t CAENV2740::readLongChargeIntegratorPedestal(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/LongChargeIntegratorPedestal");
}
void CAENV2740::writeLongChargeIntegratorPedestal(int channel, uint32_t pedestal) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/LongChargeIntegratorPedestal", pedestal);
}
uint32_t CAENV2740::readShortChargeIntegratorPedestal(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/ShortChargeIntegratorPedestal");
}
void CAENV2740::writeShortChargeIntegratorPedestal(int channel, uint32_t pedestal) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ShortChargeIntegratorPedestal", pedestal);
}
std::string CAENV2740::readEnergyGain(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/EnergyGain");
}
void CAENV2740::writeEnergyGain(int channel, std::string gain) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyGain", gain);
}
uint32_t CAENV2740::readNeutronThreshold(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/NeutronThreshold");
}
void CAENV2740::writeNeutronThreshold(int channel, uint32_t threshold) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/NeutronThreshold", threshold);
}
std::string CAENV2740::readEnDataReduction() { return readRegisterInString("/par/EnDataReduction"); }
void CAENV2740::writeEnDataReduction(bool reduction) {
    writeRegister("/par/EnDataReduction", reduction ? "True" : "False");
}
std::string CAENV2740::readEnStatEvents() { return readRegisterInString("/par/EnStatEvents"); }
void CAENV2740::writeEnStatEvents(bool events) { writeRegister("/par/EnStatEvents", events ? "True" : "False"); }