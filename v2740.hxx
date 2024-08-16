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
    std::string readRegisterInString(const std::string& registerPath);
    void writeRegister(const std::string& registerPath, uint32_t value);

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

    std::string readSFPLinkPresence();
    std::string readSFPLinkActive();
    std::string readSFPLinkProtocol();

    std::string readIPAddress();
    std::string readSubnetMask();
    std::string readGateway();

    // V2740 관련 레지스터 함수들
    uint32_t readChannelThreshold(int channel);
    void writeChannelThreshold(int channel, uint32_t value);
    uint32_t readTriggerMask();
    void writeTriggerMask(uint32_t mask);
    uint32_t readSamplingRate();
    void writeSamplingRate(uint32_t rate);
    uint32_t readAcquisitionMode();
    void writeAcquisitionMode(uint32_t mode);
    uint32_t readChannelEnable(int channel);
    void writeChannelEnable(int channel, bool enable);
};

#endif  // CAENV2740_H