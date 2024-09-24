// CAENV2740.h
#ifndef CAENV2740_H
#define CAENV2740_H

#include <CAEN_FELib.h>

#include <string>

#include "CAENV2740Event.hxx"
#include "CAENV2740Par.hxx"
class CAENV2740 {
   private:
    uint64_t handle;  // int에서 uint64_t로 변경
    std::string connectionString;
    CAENV2740Event evt;

    bool verbose;

   public:
    CAENV2740();
    ~CAENV2740();

    static bool available(const std::string& str);

    inline void setVerbose(bool verbose) { this->verbose = verbose; }
    inline bool getVerbose() { return verbose; }

    inline uint64_t getHandle() { return handle; }
    inline const CAENV2740Event& getEvent() { return evt; }

    void connect(const std::string& str);
    void configure();
    void close();

    void reset();
    void reboot();
    void clear();

    void armAcquisition();
    void disarmAcquisition();

    void startAcquisition();
    void stopAcquisition();

    void sendSWTrigger();
    void sendChSWTrigger(int channel);

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Parameter treatment
    void loadParameter(const CAENV2740Par& par);
    void parameterParsing(const std::string& key, ryml::ConstNodeRef node, int channel = -1);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Print Information
    void printDigitizerInfo();
    void printLicenseInfo();
    void printSpecInfo();
    void printNetworkInfo();

    void printClockInfo();
    void printTriggerSourceInfo();
    void printPanelIOInfo();

    void printWaveParamInfo();
    void printWaveProbeInfo();
    void printRawDataInfo();

    void printCounterInfo();
    void printITLInfo(int option = 3);  // option = 1: ITLA, 2: ITLB, 3: ITLA + ITLB
    void printLVDSInfo();
    void printPanelDACInfo();

    void printInputInfo();
    void printEventInfo();

    void printDPPPSDInfo();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // About Data Access
    std::string readActiveEndPoint();
    void writeActiveEndPoint(std::string endPoint);  // endPoint = "RAW", "DPPPSD", "DPPPHA"

    void setDataFormatRaw();
    void setDataFormatDPPPSD();
    void setDataFormatDPPPHA();
    void setDataFormatDPPPSDStats();
    void setDataFormatDPPPHAStats();

    int readData(int timeout = 1000);  // timeout = 1000 [ms]
    int readDataRaw(int timeout = 1000, uint8_t* data = nullptr, size_t* size = nullptr, uint32_t* n_events = nullptr);
    int readStats();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        int channel,
        std::string source);  // source = "Disabled", "BoardVeto", "ADCOverSaturation", "ADCUnderSaturation"
    uint32_t readChannelVetoWidth(int channel);
    void writeChannelVetoWidth(int channel, uint32_t width);  // increament = 8 [ns]

    uint32_t readRunDelay();
    void writeRunDelay(uint32_t delay);  // delay = 0 ~ 1023 [ns]

    std::string readEnAutoDisarmAcq();
    void writeEnAutoDisarmAcq(bool enable);  // enable = "True", "False"

    uint32_t readAcquisitionStatus();
    uint32_t readLEDStatus();

    float readVolatileClockOutDelay();
    void writeVolatileClockOutDelay(float delay);  // increment = 74.07407407407408 [ps]
    float readPermanentClockOutDelay();
    void writePermanentClockOutDelay(float delay);  // increment = 74.07407407407408 [ps]
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
    void writeWaveDownSamplingFactor(int channel, int factor);  // factor = "1", "2", "4", "8"
    std::string readWaveAnalogProbe(int num, int channel);      // num = 0, 1
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Individual Trigger 관련
    std::string readITLXMainLogic(bool isA = true);          // isA = true: ITLA, false: ITLB
    void writeITLXMainLogic(bool isA, std::string logic);    // logic = "AND", "OR", "Majority"
    uint32_t readITLXMajorityLev(bool isA = true);           // isA = true: ITLA, false: ITLB
    void writeITLXMajorityLev(bool isA, uint32_t level);     // level = 0 ~ 15
    std::string readITLXPairLogic(bool isA = true);          // isA = true: ITLA, false: ITLB
    void writeITLXPairLogic(bool isA, std::string logic);    // logic = "AND", "OR", "NONE"
    std::string readITLXPolarity(bool isA = true);           // isA = true: ITLA, false: ITLB
    void writeITLXPolarity(bool isA, std::string polarity);  // polarity = "Direct", "Inverted"
    uint32_t readITLXMask(bool isA = true);                  // isA = true: ITLA, false: ITLB
    void writeITLXMask(bool isA, uint32_t mask);             // mask = 0 ~ 15
    std::string readITLConnect(int channel);
    void writeITLConnect(int channel, std::string connect);  // connect = "Disabled", "ITLA", "ITLB"
    uint32_t readITLXGateWidth(bool isA = true);             // isA = true: ITLA, false: ITLB
    void writeITLXGateWidth(bool isA, uint32_t width);       // increment: 8 [ns]
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LVDS 관련
    std::string readLVDSMode(int channel);
    void writeLVDSMode(int channel, std::string mode);  // mode = "SelfTriggers", "Sync", "IORegister", "User"
    std::string readLVDSDirection(int channel);
    void writeLVDSDirection(int channel, std::string direction);  // direction = "Input", "Output"
    std::string readLVDSIOReg();
    void writeLVDSIOReg(std::string reg);
    std::string readLVDSTrgMask();
    void writeLVDSTrgMask(uint32_t mask);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Front Panel LEMO DAC 관련
    std::string readDACoutMode();
    void writeDACoutMode(std::string mode);  // mode = "Static", "Ramp", "Sin5MHz", "Square", "IPE", "ChInput",
                                             // "MemOccupancy", "ChSum", "OverThrSum"
    uint32_t readDACoutStaticLevel();
    void writeDACoutStaticLevel(uint32_t level);
    uint32_t readDACoutChSelect();
    void writeDACoutChSelect(uint32_t channel);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Input Signal Conditioning 관련
    uint32_t readSelfTriggerWidth(int channel);
    void writeSelfTriggerWidth(int channel, uint32_t width);  // increment = 8 [ns]
    uint32_t readInputDelay(int group);
    void writeInputDelay(int group, uint32_t delay);  // increment = 1 [Sample]
    std::string readEnOffsetCalibration();
    void writeEnOffsetCalibration(bool enable);  // enable = "True", "False"
    std::string readChEnable(int channel);
    void writeChEnable(int channel, bool enable);  // enable = "True", "False"
    float readSelfTrgRate(int channel);            // in Hz
    uint32_t readChStatus(int channel);
    float readDCOffset(int channel);
    void writeDCOffset(int channel, float offset);  // increment = 0.0001 [%]
    uint32_t readSignalOffset(int channel);
    void writeSignalOffset(int channel, uint32_t offset);  // increment = 50 [uV]
    float readGainFactor(int channel);
    float readADCToVolts(int channel);  // [V]
    uint32_t readTriggerThr(int channel);
    void writeTriggerThr(int channel, uint32_t threshold);  // increment = 1 [ADC]
    std::string readPulsePolarity(int channel);
    void writePulsePolarity(int channel, std::string polarity);  // polarity = "Positive", "Negative"
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Event Selection and Coincidence parameters
    uint32_t readEnergySkimLowDiscriminator(int channel);
    void writeEnergySkimLowDiscriminator(int channel, uint32_t threshold);  // increment = 1 [bin]
    uint32_t readEnergySkimHighDiscriminator(int channel);
    void writeEnergySkimHighDiscriminator(int channel, uint32_t threshold);  // increment = 1 [bin]
    std::string readEventSelector(int channel);
    void writeEventSelector(int channel, std::string selector);  // selector = "All", "PileUp", "EnergySkim"
    std::string readWaveSelector(int channel);
    void writeWaveSelector(int channel, std::string selector);  // selector = "All", "PileUp", "EnergySkim"
    std::string readEventNeutronReject(int channel);
    void writeEventNeutronReject(int channel, bool reject);  // reject = "Disabled", "Enabled"
    std::string readWaveNeutronReject(int channel);
    void writeWaveNeutronReject(int channel, bool reject);  // reject = "Disabled", "Enabled"
    std::string readCoincidenceMask(int channel);
    void writeCoincidenceMask(
        int channel,
        std::string mask);  // mask = "Disabled", "Ch64Trigger", "TRGIN", "GlobalTriggerSource", "ITLA", "ITLB"
    std::string readAntiCoincidenceMask(int channel);
    void writeAntiCoincidenceMask(
        int channel,
        std::string mask);  // mask = "Disabled", "Ch64Trigger", "TRGIN", "GlobalTriggerSource", "ITLA", "ITLB"
    uint32_t readCoincidenceLength(int channel, bool isSample = true);
    void writeCoincidenceLength(int channel, uint32_t length, bool isSample = true);  // length = 1 [Sample]
    uint32_t readCoincidenceLengthS(int channel);
    void writeCoincidenceLengthS(int channel, uint32_t length);  // length = 1 [Sample]
    uint32_t readCoincidenceLengthT(int channel);
    void writeCoincidenceLengthT(int channel, uint32_t length);  // length = 8 [ns]
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DPP Algorithm 관련 (DPPPSD)
    uint32_t readSmoothingFactor(int channel);
    void writeSmoothingFactor(int channel, uint32_t factor);  // factor = 1, 2, 4, 8, 16
    std::string readChargeSmoothing(int channel);
    void writeChargeSmoothing(int channel, bool smoothing);  // smoothing = "Disabled", "Enabled"
    std::string readTimeFilterSmoothing(int channel);
    void writeTimeFilterSmoothing(int channel, bool smoothing);  // smoothing = "Disabled", "Enabled"
    uint32_t readTimeFilterRetriggerGuard(int channel,
                                          bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeTimeFilterRetriggerGuard(int channel, uint32_t guard,
                                       bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readTimeFilterRetriggerGuardS(int channel);
    void writeTimeFilterRetriggerGuardS(int channel, uint32_t guard);  // increment = 1 [Sample]
    uint32_t readTimeFilterRetriggerGuardT(int channel);
    void writeTimeFilterRetriggerGuardT(int channel, uint32_t guard);  // increment = 8 [ns]
    std::string readTriggerHysteresis(int channel);
    void writeTriggerHysteresis(int channel, bool hysteresis);  // hysteresis = "Disabled", "Enabled"
    uint32_t readCFDDelay(int channel, bool isSample = true);   // isSample = true: 샘플 수, false: 시간 [ns]
    void writeCFDDelay(int channel, uint32_t delay,
                       bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readCFDDelayS(int channel);
    void writeCFDDelayS(int channel, uint32_t delay);  // increment = 1 [Sample]
    uint32_t readCFDDelayT(int channel);
    void writeCFDDelayT(int channel, uint32_t delay);  // increment = 8 [ns]
    uint32_t readCFDFraction(int channel);
    void writeCFDFraction(int channel, uint32_t fraction);  // fraction = 25, 50, 75, 100 [%]
    std::string readTriggerFilterSelection(int channel);
    void writeTriggerFilterSelection(int channel, std::string selection);  // selection = "LeadingEdge", "CFD"
    std::string readADCInputBaselineAvg(int channel);
    void writeADCInputBaselineAvg(int channel,
                                  std::string avg);  // avg = "Fixed", "Low", "MediumLow", "MediumHigh", "High"
    uint32_t readADCInputBaselineGuard(int channel,
                                       bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeADCInputBaselineGuard(int channel, uint32_t guard,
                                    bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readADCInputBaselineGuardS(int channel);
    void writeADCInputBaselineGuardS(int channel, uint32_t guard);  // increment = 1 [Sample]
    uint32_t readADCInputBaselineGuardT(int channel);
    void writeADCInputBaselineGuardT(int channel, uint32_t guard);  // increment = 8 [ns]
    uint32_t readPileupGap(int channel);
    void writePileupGap(int channel, uint32_t gap);  // increment = 1 [ADC]
    uint32_t readAbsoluteBaseline(int channel);
    void writeAbsoluteBaseline(int channel, uint32_t baseline);  // increment = 1 [ADC]
    uint32_t readGateOffset(int channel, bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeGateOffset(int channel, uint32_t offset,
                         bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readGateOffsetS(int channel);
    void writeGateOffsetS(int channel, uint32_t offset);  // increment = 1 [Sample]
    uint32_t readGateOffsetT(int channel);
    void writeGateOffsetT(int channel, uint32_t offset);             // increment = 8 [ns]
    uint32_t readGateLongLength(int channel, bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeGateLongLength(int channel, uint32_t length,
                             bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readGateLongLengthS(int channel);
    void writeGateLongLengthS(int channel, uint32_t length);  // increment = 1 [Sample]
    uint32_t readGateLongLengthT(int channel);
    void writeGateLongLengthT(int channel, uint32_t length);          // increment = 8 [ns]
    uint32_t readGateShortLength(int channel, bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    void writeGateShortLength(int channel, uint32_t length,
                              bool isSample = true);  // isSample = true: 샘플 수, false: 시간 [ns]
    uint32_t readGateShortLengthS(int channel);
    void writeGateShortLengthS(int channel, uint32_t length);  // increment = 1 [Sample]
    uint32_t readGateShortLengthT(int channel);
    void writeGateShortLengthT(int channel, uint32_t length);  // increment = 8 [ns]
    uint32_t readLongChargeIntegratorPedestal(int channel);
    void writeLongChargeIntegratorPedestal(int channel, uint32_t pedestal);  // increment = 1 [ADC]
    uint32_t readShortChargeIntegratorPedestal(int channel);
    void writeShortChargeIntegratorPedestal(int channel, uint32_t pedestal);  // increment = 1 [ADC]
    std::string readEnergyGain(int channel);
    void writeEnergyGain(int channel, int gain);  // gain = 1, 4, 16, 64, 256
    uint32_t readNeutronThreshold(int channel);
    void writeNeutronThreshold(int channel, uint32_t threshold);  // increment = 1 [ADC]
    std::string readEnDataReduction();
    void writeEnDataReduction(bool reduction);  // reduction = "True", "False"
    std::string readEnStatEvents();
    void writeEnStatEvents(bool events);  // events = "True", "False"
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DPP Algorithm 관련 (additional for DPPPHA)
    uint32_t readTimeFilterRiseTime(int channel, bool isSample = true);
    void writeTimeFilterRiseTime(int channel, uint32_t riseTime, bool isSample = true);
    uint32_t readTimeFilterRiseTimeS(int channel);
    void writeTimeFilterRiseTimeS(int channel, uint32_t riseTime);
    uint32_t readTimeFilterRiseTimeT(int channel);
    void writeTimeFilterRiseTimeT(int channel, uint32_t riseTime);

    uint32_t readEnergyFilterRiseTime(int channel, bool isSample = true);
    void writeEnergyFilterRiseTime(int channel, uint32_t riseTime, bool isSample = true);
    uint32_t readEnergyFilterRiseTimeS(int channel);
    void writeEnergyFilterRiseTimeS(int channel, uint32_t riseTime);
    uint32_t readEnergyFilterRiseTimeT(int channel);
    void writeEnergyFilterRiseTimeT(int channel, uint32_t riseTime);

    uint32_t readEnergyFilterFlatTop(int channel, bool isSample = true);
    void writeEnergyFilterFlatTop(int channel, uint32_t flatTop, bool isSample = true);
    uint32_t readEnergyFilterFlatTopS(int channel);
    void writeEnergyFilterFlatTopS(int channel, uint32_t flatTop);
    uint32_t readEnergyFilterFlatTopT(int channel);
    void writeEnergyFilterFlatTopT(int channel, uint32_t flatTop);

    uint32_t readEnergyFilterPeakingPosition(int channel);
    void writeEnergyFilterPeakingPosition(int channel, uint32_t position);  // [%]
    std::string readEnergyFilterPeakingAvg(int channel);
    void writeEnergyFilterPeakingAvg(int channel,
                                     std::string avg);  // avg = "OneShot", "LowAVG", "MediumAVG", "HighAVG"

    uint32_t readEnergyFilterPoleZero(int channel, bool isSample = true);
    void writeEnergyFilterPoleZero(int channel, uint32_t poleZero, bool isSample = true);
    uint32_t readEnergyFilterPoleZeroS(int channel);
    void writeEnergyFilterPoleZeroS(int channel, uint32_t poleZero);
    uint32_t readEnergyFilterPoleZeroT(int channel);
    void writeEnergyFilterPoleZeroT(int channel, uint32_t poleZero);

    float readEnergyFilterFineGain(int channel);
    void writeEnergyFilterFineGain(int channel, float gain);  // increment = 0.001
    bool readEnergyFilterLFLimitation(int channel);
    void writeEnergyFilterLFLimitation(int channel, bool limitation);  // limitation = "On", "Off"
    std::string readEnergyFilterBaselineAvg(int channel);
    void writeEnergyFilterBaselineAvg(
        int channel, std::string avg);  // avg = "Fixed", "VeryLow", "Low", "MediumLow", "Medium", "MediumHigh", "High"

    uint32_t readEnergyFilterBaselineGuard(int channel, bool isSample = true);
    void writeEnergyFilterBaselineGuard(int channel, uint32_t guard, bool isSample = true);
    uint32_t readEnergyFilterBaselineGuardS(int channel);
    void writeEnergyFilterBaselineGuardS(int channel, uint32_t guard);
    uint32_t readEnergyFilterBaselineGuardT(int channel);
    void writeEnergyFilterBaselineGuardT(int channel, uint32_t guard);

    uint32_t readEnergyFilterPileupGuard(int channel, bool isSample = true);
    void writeEnergyFilterPileupGuard(int channel, uint32_t guard, bool isSample = true);
    uint32_t readEnergyFilterPileupGuardS(int channel);
    void writeEnergyFilterPileupGuardS(int channel, uint32_t guard);
    uint32_t readEnergyFilterPileupGuardT(int channel);
    void writeEnergyFilterPileupGuardT(int channel, uint32_t guard);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
};

#endif  // CAENV2740_H