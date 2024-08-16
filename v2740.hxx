// CAENV2740.h
#ifndef CAENV2740_H
#define CAENV2740_H

#include <CAEN_FELib.h>

#include <string>

class CAENV2740 {
   private:
    uint64_t handle;  // int에서 uint64_t로 변경
    std::string connectionString;

   public:
    CAENV2740(const std::string& connString);
    ~CAENV2740();

    void connect();
    void configure();

    void reset();
    void reboot();
    void clear();

    void startAcquisition();
    void stopAcquisition();

    uint32_t readRegister(const std::string& registerPath);
    float readRegisterInFloat(const std::string& registerPath);
    std::string readRegisterInString(const std::string& registerPath);
    void writeRegister(const std::string& registerPath, uint32_t value);
    void writeRegister(const std::string& registerPath, float value);
    void writeRegister(const std::string& registerPath, std::string value);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // V2740 관련 일반 파라미터
    std::string readCUPVersion();
    std::string readFPGAFWVersion();
    std::string readFWType();

    std::string readModelCode();
    std::string readPBCode();
    std::string readModelName();
    std::string readFormFactor();
    std::string readFamilyCode();
    std::string readSerialNumber();

    std::string readPCBRevMB();
    std::string readPCBRevPB();

    std::string readLicense();
    std::string readLicenseStatus();
    std::string readLicenseRemainingTime();

    int readNumberOfChannels();
    int readADCResolution();
    int readADCSamplingRate();
    int readInputRange();
    std::string readInputType();
    int readInputImpedance();

    std::string readClockSource();
    void writeClockSource(std::string source);  /// source = "Internal" or "FPClkIn"
    std::string readEnClockOutFP();
    void writeEnClockOutFP(bool enable);

    std::string readSFPLinkPresence();
    std::string readSFPLinkActive();
    std::string readSFPLinkProtocol();

    std::string readIPAddress();
    std::string readSubnetMask();
    std::string readGateway();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // acquisition, trigger, and veto 관련
    std::string readStartSource();
    void writeStartSource(std::string source);  // source = "EncodedClkIn", "SINlevel", "SINedge", "SWcmd", "LVDS", "P0"
    std::string readGlobalTriggerSource();
    void writeGlobalTriggerSource(
        std::string source);  // source = "TrgIn", "P0", "SwTrg", "LVDS", "ITLA", "ITLB", "ITLA_AND_ITLB",
                              // "ITLA_OR_ITLB", "EncodedClkIn", "GPIO", "TestPulse", "UserTrg"
    std::string readWaveTriggerSource(int channel);
    void writeWaveTriggerSource(int channel,
                                std::string source);  // source =  "Disabled", "Ch64Trigger", "ChSelfTrigger", "SwTrg",
                                                      // "ADCOverSaturation", "ADCUnderSaturation", "ExternalInhibit",
                                                      // "TRGIN", "GlobalTriggerSource", "LVDS", "ITLA", "ITLB"
    std::string readEventTriggerSource(int channel);
    void writeEventTriggerSource(int channel,
                                 std::string source);  // source = "Disabled", "Ch64Trigger", "ChSelfTrigger", "SwTrg",
                                                       // "TRGIN", "GlobalTriggerSource", "LVDS", "ITLA", "ITLB"
    std::string readChannelTriggerMask(int channel);
    void writeChannelTriggerMask(int channel, uint32_t mask);

    std::string readWaveSaving(int channel);
    void writeWaveSaving(int channel, std::string mode);  // mode = "Always", "OnRequest"

    std::string readTrgOutMode();
    void writeTrgOutMode(
        std::string mode);  // mode = "Disabled", "TrgIn", "P0", "SwTrg", "LVDS", "ITLA", "ITLB", "ITLA_AND_ITLB",
                            // "ITLA_OR_ITLB", "EncodedClkIn", "RUN", "RefClk", "TestPulse", "Busy", "UserTrgout",
                            // "Fixed0", "Fixed1", "SyncIn", "SIN", "GPIO", "LBinClk", "AcceptTrg", "TrgClk"
    std::string readGPIOMode();
    void writeGPIOMode(std::string mode);  // mode = "Disabled", "TrgIn", "P0", "SIN", "LVDS", "ITLA", "ITLB",
                                           // "ITLA_AND_ITLB", "ITLA_OR_ITLB", "EncodedClkIn", "SWTrg", "Run", "RefClk",
                                           // "TestPulse", "Busy", "UserGPO", "Fixed0", "Fixed1"
    std::string readBusyInSource();
    void writeBusyInSource(std::string source);  // source = "Disabled", "SIN", "GPIO", "LVDS"
    std::string readSyncOutMode();
    void writeSyncOutMode(std::string mode);  // mode = "Disabled", "SyncIn", "TestPulse", "IntClk", "Run", "User"

    std::string readBoardVetoSource();
    void writeBoardVetoSource(std::string source);  // source = "Disabled", "SIN", "GPIO", "LVDS", "P0", "EncodedClkIn"
    uint32_t readBoardVetoWidth();
    void writeBoardVetoWidth(uint32_t width);  // increament = 8 [ns]
    std::string readBoardVetoPolarity();
    void writeBoardVetoPolarity(std::string polarity);  // polarity = "ActiveLow", "ActiveHigh"
    std::string readChannelVetoSource(int channel);
    void writeChannelVetoSource(
        int channel, std::string source);  // source = "Disabled", "SIN", "GPIO", "LVDS", "P0", "EncodedClkIn"
    uint32_t readChannelVetoWidth(int channel);
    void writeChannelVetoWidth(int channel, uint32_t width);  // increament = 8 [ns]

    uint32_t readRunDelay();
    void writeRunDelay(uint32_t delay);  // delay = 0 ~ 1023 [ns]

    std::string readEnAutoDisarmAcq();
    void writeEnAutoDisarmAcq(std::string enable);  // enable = "True", "False"

    uint32_t readAcquisitionStatus();
    uint32_t readLEDStatus();

    uint32_t readVolatileClockOutDelay();
    void writeVolatileClockOutDelay(uint32_t delay);  // increment = 74.07407407407408 [ps]
    uint32_t readPermanentClockOutDelay();
    void writePermanentClockOutDelay(uint32_t delay);  // increment = 74.07407407407408 [ps]
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Waveform 관련
    uint32_t readChRecordLength(int channel, bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readChRecordLengthS(int channel);
    uint32_t readChRecordLengthT(int channel);
    void writeChRecordLength(int channel, uint32_t length,
                             bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeChRecordLengthS(int channel, uint32_t length);
    void writeChRecordLengthT(int channel, uint32_t length);

    std::string readWaveDownSamplingFactor(int channel);
    void writeWaveDownSamplingFactor(int channel, std::string factor);  // factor = "1", "2", "4", "8"
    std::string readWaveAnalogProbe(int num, int channel);              // num = 0, 1
    void writeWaveAnalogProbe(int num, int channel,
                              std::string probe);            // probe = "ADCInput", "ADCInputBaseline", "CFDFilter"
    std::string readWaveDigitalProbe(int num, int channel);  // num = 0, 1, 2, 3
    void writeWaveDigitalProbe(
        int num, int channel,
        std::string probe);  // probe = "Trigger", "CFDFilterArmed", "ReTriggerGuard", "ADCInputBaselineFreeze",
                             // "ADCinputOverthreshold", "ChargeReady", "LongGate" "PileUpTrigger", "ShortGate",
                             // "ChargeOverRange", "ADCSaturation", "ADCInputNegativeOverThreshold"

    uint32_t readMaxRawDataSize();
    uint32_t readChPreTrigger(int channel, bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readChPreTriggerS(int channel);
    uint32_t readChPreTriggerT(int channel);
    void writeChPreTrigger(int channel, uint32_t preTrigger,
                           bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeChPreTriggerS(int channel, uint32_t preTrigger);
    void writeChPreTriggerT(int channel, uint32_t preTrigger);

    std::string readWaveDataSource(int channel);
    void writeWaveDataSource(int channel,
                             std::string source);  // source = "ADC_DATA", "ADC_TEST_TOGGLE", "ADC_TEST_RAMP",
                                                   // "ADC_TEST_SIN", "ADC_TEST_PRBS", "Ramp", "IPE", "SquareWave"
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Service 관련
    uint32_t readTestPulsePeriod();
    void writeTestPulsePeriod(uint32_t period);  // increment = 8 [ns]
    uint32_t readTestPulseWidth();
    void writeTestPulseWidth(uint32_t width);  // increment = 8 [ns]
    uint32_t readTestPulseLowLevel();
    void writeTestPulseLowLevel(uint32_t level);  // increment = 1 [ADC]
    uint32_t readTestPulseHighLevel();
    void writeTestPulseHighLevel(uint32_t level);  // increment = 1 [ADC]
    std::string readIOlevel();
    void writeIOlevel(std::string level);  // level = "NIM", "TTL"

    float readTempSens(std::string sensor);  // sensor = "AirIn", "AirOut", "Core", "FirstADC", "LastADC", "HottestADC",
                                             // "ADC0", "ADC1", "ADC2", "ADC3", "ADC4", "ADC5", "ADC6", "ADC7", "DCDC"
    float readSensDCDC(std::string type);    // type = "Temp", "VIn", "VOut", "IOut"
    float readFreqSensCore();
    float readDutyCycleSensDCDC();
    float readSpeedSensFan(int num);  // num = 0, 1

    uint32_t readErrorFlagMask();
    void writeErrorFlagMask(uint32_t mask);
    uint32_t readErrorFlagDataMask();
    void writeErrorFlagDataMask(uint32_t mask);
    uint32_t readErrorFlags();

    std::string readBoardReady();  // "True", "False"
    float readChRealTimeMonitor(int channel);
    float readChDeadTimeMonitor(int channel);
    uint32_t readChTriggerCnt(int channel);
    uint32_t readChSavedEventCnt(int channel);
    uint32_t readChWaveCnt(int channel);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

#endif  // CAENV2740_H