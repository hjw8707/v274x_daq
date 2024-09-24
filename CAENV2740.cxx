// CAENV2740.cpp
#include "CAENV2740.hxx"

#include <iostream>
#include <stdexcept>

constexpr unsigned int HashCode(const char* str) {
    return str[0] ? static_cast<unsigned int>(str[0]) + 0xEDB8832Full * HashCode(str + 1) : 8603;
}

CAENV2740::CAENV2740() : connectionString(""), handle(0), verbose(false) {}

CAENV2740::~CAENV2740() {
    if (handle != 0) {
        close();
    }
}

bool CAENV2740::available(const std::string& str) {
    uint64_t handle = 0;
    std::cout << "V2740 연결 확인: " << str << std::endl;
    int ret = CAEN_FELib_Open(str.c_str(), &handle);
    if (ret != CAEN_FELib_Success) {
        std::cout << "V2740 연결 실패" << std::endl;
        return false;
    }
    std::cout << "V2740 연결 성공" << std::endl;
    CAEN_FELib_Close(handle);
    return true;
}

void CAENV2740::connect(const std::string& str) {
    connectionString = str;
    int ret = CAEN_FELib_Open(connectionString.c_str(), &handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 연결 실패");
    }
    if (verbose) {
        std::cout << "V2740 연결 성공" << std::endl;
        std::cout << "연결 문자열: " << connectionString << std::endl;
    }
}

void CAENV2740::reset() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/Reset");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 리셋 실패");
    }
    if (verbose) std::cout << "V2740 리셋 성공" << std::endl;
}

void CAENV2740::reboot() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/Reboot");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 리부트 실패");
    }
    if (verbose) std::cout << "V2740 리부트 성공" << std::endl;
}

void CAENV2740::clear() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/ClearData");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 클리어 실패");
    }
    if (verbose) std::cout << "V2740 클리어 성공" << std::endl;
}

void CAENV2740::configure() {
    // 여기에 V2740 설정 코드를 추가하세요
    // 예: 샘플링 레이트, 트리거 설정 등
}
void CAENV2740::close() {
    int ret = CAEN_FELib_Close(handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("V2740 연결 종료 실패");
    }
    handle = 0;
    if (verbose) std::cout << "V2740 연결 종료 성공" << std::endl;
}
void CAENV2740::armAcquisition() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/ArmAcquisition");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 수집 시작 실패");
    }
}

void CAENV2740::disarmAcquisition() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/DisarmAcquisition");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 수집 중지 실패");
    }
}

void CAENV2740::startAcquisition() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/SwStartAcquisition");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 수집 시작 실패");
    }
}

void CAENV2740::stopAcquisition() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/SwStopAcquisition");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 수집 중지 실패");
    }
}

void CAENV2740::sendSWTrigger() {
    int ret = CAEN_FELib_SendCommand(handle, "/cmd/SendSWTrigger");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("소프트웨어 트리거 전송 실패");
    }
}
void CAENV2740::sendChSWTrigger(int channel) {
    std::string command = "/ch/" + std::to_string(channel) + "/cmd/SendChSWTrigger";
    int ret = CAEN_FELib_SendCommand(handle, command.c_str());
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("채널 소프트웨어 트리거 전송 실패");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parameter treatment
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAENV2740::loadParameter(const CAENV2740Par& par) {
    if (par.getFilename().empty()) throw std::runtime_error("파라미터 파일이 없습니다.");

    std::cout << "================================================" << std::endl;
    std::cout << "V2740 파라미터 로딩 시작" << std::endl;
    std::cout << "파라미터 파일: " << par.getFilename() << std::endl;

    auto config = par.getConfig();
    // For global
    for (const auto& it : config["global"]) parameterParsing(std::string(it.key().data(), it.key().size()), it);

    // for channel
    int channel;
    for (const auto& it : config["channel"]) {
        it["number"] >> channel;
        for (const auto& it2 : it) parameterParsing(std::string(it2.key().data(), it2.key().size()), it2, channel);
    }

    std::cout << "V2740 파라미터 로딩 완료" << std::endl;
    std::cout << "================================================" << std::endl;
}

void CAENV2740::parameterParsing(const std::string& key, ryml::ConstNodeRef node, int channel) {
    std::string val_str;
    bool val_bool;
    uint32_t val_uint32;
    float val_float;

    int i = 0;
    switch (HashCode(key.c_str())) {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Global
        case HashCode("clock_source"):
            node >> val_str;
            writeClockSource(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("en_clockoutfp"):
            node >> val_bool;
            writeEnClockOutFP(val_bool);
            std::cout << "키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("start_source"):  // sequence
            for (const auto& it : node) {
                if (!val_str.empty()) val_str += "|";
                val_str += std::string(it.val().data(), it.val().size());
            }
            writeStartSource(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("global_trigger_source"):  // sequence
            for (const auto& it : node) {
                if (!val_str.empty()) val_str += "|";
                val_str += std::string(it.val().data(), it.val().size());
            }
            writeGlobalTriggerSource(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("trgout_mode"):
            node >> val_str;
            writeTrgOutMode(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("gpio_mode"):
            node >> val_str;
            writeGPIOMode(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("busyin_source"):
            node >> val_str;
            writeBusyInSource(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("syncout_mode"):
            node >> val_str;
            writeSyncOutMode(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("board_veto_source"):
            node >> val_str;
            writeBoardVetoSource(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("board_veto_width"):
            node >> val_uint32;
            writeBoardVetoWidth(val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("veto_polarity"):
            node >> val_str;
            writeBoardVetoPolarity(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("run_delay"):
            node >> val_uint32;
            writeRunDelay(val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("auto_disarm_acq"):
            node >> val_bool;
            writeEnAutoDisarmAcq(val_bool);
            std::cout << "키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("volatile_clkout_delay"):
            node >> val_float;
            writeVolatileClockOutDelay(val_float);
            std::cout << "키: " << key << ", 값: " << val_float << std::endl;
            break;
        case HashCode("permanent_clkout_delay"):
            node >> val_float;
            writePermanentClockOutDelay(val_float);
            std::cout << "키: " << key << ", 값: " << val_float << std::endl;
            break;
        case HashCode("tp_period"):
            node >> val_uint32;
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("tp_width"):
            node >> val_uint32;
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("tp_low_level"):
            node >> val_uint32;
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("tp_high_level"):
            node >> val_uint32;
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("io_level"):
            node >> val_str;
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("errorflag_mask"):
            node >> val_uint32;
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("errorflag_data_mask"):
            node >> val_uint32;
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("itla_main_logic"):
            node >> val_str;
            writeITLXMainLogic(true, val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itla_majority_level"):
            node >> val_uint32;
            writeITLXMajorityLev(true, val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("itla_pair_logic"):
            node >> val_str;
            writeITLXPairLogic(true, val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itla_polarity"):
            node >> val_str;
            writeITLXPolarity(true, val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itla_mask"):
            node >> val_uint32;
            writeITLXMask(true, val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("itla_gate_width"):
            node >> val_uint32;
            writeITLXGateWidth(true, val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("itlb_main_logic"):
            node >> val_str;
            writeITLXMainLogic(false, val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itlb_majority_level"):
            node >> val_uint32;
            writeITLXMajorityLev(false, val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("itlb_pair_logic"):
            node >> val_str;
            writeITLXPairLogic(false, val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itlb_polarity"):
            node >> val_str;
            writeITLXPolarity(false, val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itlb_mask"):
            node >> val_uint32;
            writeITLXMask(false, val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("itlb_gate_width"):
            node >> val_uint32;
            writeITLXGateWidth(false, val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("lvds_mode"):  // sequence
            std::cout << "키: " << key << ", 값: ";
            for (const auto& it : node) {
                it >> val_str;
                writeLVDSMode(i++, val_str);
                std::cout << val_str << " ";
            }
            std::cout << std::endl;
            break;
        case HashCode("lvds_direction"):  // sequence
            std::cout << "키: " << key << ", 값: ";
            for (const auto& it : node) {
                it >> val_str;
                writeLVDSDirection(i++, val_str);
                std::cout << val_str << " ";
            }
            std::cout << std::endl;
            break;
        case HashCode("lvds_trg_mask"):
            node >> val_uint32;
            writeLVDSTrgMask(val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("dacout_mode"):
            node >> val_str;
            writeDACoutMode(val_str);
            std::cout << "키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("dacout_static_level"):
            node >> val_uint32;
            writeDACoutStaticLevel(val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("dacout_ch_select"):
            node >> val_uint32;
            writeDACoutChSelect(val_uint32);
            std::cout << "키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("input_delay"):  // sequence
            std::cout << "키: " << key << ", 값: ";
            for (const auto& it : node) {
                it >> val_uint32;
                writeInputDelay(i++, val_uint32);
                std::cout << val_uint32 << " ";
            }
            std::cout << std::endl;
            break;
        case HashCode("offset_calibration"):
            node >> val_bool;
            writeEnOffsetCalibration(val_bool);
            std::cout << "키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("data_reduction"):
            node >> val_bool;
            writeEnDataReduction(val_bool);
            std::cout << "키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("stat_events"):
            node >> val_bool;
            writeEnStatEvents(val_bool);
            std::cout << "키: " << key << ", 값: " << val_bool << std::endl;
            break;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Channel
        case HashCode("wave_trigger_source"):
            node >> val_str;
            writeWaveTriggerSource(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("event_trigger_source"):
            node >> val_str;
            writeEventTriggerSource(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("channels_trigger_mask"):
            node >> val_uint32;
            writeChannelTriggerMask(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("wave_saving"):
            node >> val_str;
            writeWaveSaving(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("veto_source"):
            node >> val_str;
            writeChannelVetoSource(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("veto_width"):
            node >> val_uint32;
            writeChannelVetoWidth(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("record_length"):
            node >> val_uint32;
            writeChRecordLengthT(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("downsampling_factor"):
            node >> val_uint32;
            writeWaveDownSamplingFactor(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("analog_probe"):
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: ";
            for (const auto& it : node) {
                it >> val_str;
                writeWaveAnalogProbe(i++, channel, val_str);
                std::cout << val_str << " ";
            }
            std::cout << std::endl;
            break;
        case HashCode("digital_probe"):
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: ";
            for (const auto& it : node) {
                it >> val_str;
                writeWaveDigitalProbe(i++, channel, val_str);
                std::cout << val_str << " ";
            }
            std::cout << std::endl;
            break;
        case HashCode("pre_trigger"):
            node >> val_uint32;
            writeChPreTriggerT(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("wave_data_source"):
            node >> val_str;
            writeWaveDataSource(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("itl_connect"):
            node >> val_str;
            writeITLConnect(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("self_trig_width"):
            node >> val_uint32;
            writeSelfTriggerWidth(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("enable"):
            node >> val_bool;
            writeChEnable(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("dc_offset"):
            node >> val_uint32;
            writeDCOffset(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("signal_offset"):
            node >> val_uint32;
            writeSignalOffset(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("trig_threshold"):
            node >> val_uint32;
            writeTriggerThr(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("pulse_polarity"):
            node >> val_str;
            writePulsePolarity(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("energy_skim_low_discriminator"):
            node >> val_uint32;
            writeEnergySkimLowDiscriminator(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energy_skim_high_discriminator"):
            node >> val_uint32;
            writeEnergySkimHighDiscriminator(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("event_selector"):
            node >> val_str;
            writeEventSelector(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("wave_selector"):
            node >> val_str;
            writeWaveSelector(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("event_neutron_reject"):
            node >> val_bool;
            writeEventNeutronReject(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("wave_neutron_reject"):
            node >> val_bool;
            writeWaveNeutronReject(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("coin_mask"):
            node >> val_str;
            writeCoincidenceMask(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("anticoin_mask"):
            node >> val_str;
            writeAntiCoincidenceMask(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("coin_length"):
            node >> val_uint32;
            writeCoincidenceLength(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("smoothing_factor"):
            node >> val_uint32;
            writeSmoothingFactor(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("charge_smoothing"):
            node >> val_bool;
            writeChargeSmoothing(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("timefilter_smoothing"):
            node >> val_bool;
            writeTimeFilterSmoothing(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("timefilter_retrigger_guard"):
            node >> val_uint32;
            writeTimeFilterRetriggerGuard(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("trigger_hysteresis"):
            node >> val_bool;
            writeTriggerHysteresis(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("cfd_delay"):
            node >> val_uint32;
            writeCFDDelay(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("cfd_fraction"):
            node >> val_uint32;
            writeCFDFraction(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("trigger_filter"):
            node >> val_str;
            writeTriggerFilterSelection(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("adcinput_baseline_avg"):
            node >> val_str;
            writeADCInputBaselineAvg(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("adcinput_baseline_guard"):
            node >> val_uint32;
            writeADCInputBaselineGuard(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("pileup_gap"):
            node >> val_uint32;
            writePileupGap(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("abs_baseline"):
            node >> val_uint32;
            writeAbsoluteBaseline(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("gate_offset"):
            node >> val_uint32;
            writeGateOffsetT(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("gate_long_length"):
            node >> val_uint32;
            writeGateLongLengthT(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("gate_short_length"):
            node >> val_uint32;
            writeGateShortLengthT(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("long_charge_integ_ped"):
            node >> val_uint32;
            writeLongChargeIntegratorPedestal(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("short_charge_integ_ped"):
            node >> val_uint32;
            writeShortChargeIntegratorPedestal(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energy_gain"):
            node >> val_uint32;
            writeEnergyGain(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("neutron_threshold"):
            node >> val_uint32;
            writeNeutronThreshold(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("timefilter_rise_time"):
            node >> val_uint32;
            writeTimeFilterRiseTime(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_rise_time"):
            node >> val_uint32;
            writeEnergyFilterRiseTime(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_flat_top"):
            node >> val_uint32;
            writeEnergyFilterFlatTop(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_peaking_pos"):
            node >> val_uint32;
            writeEnergyFilterPeakingPosition(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_peaking_avg"):
            node >> val_str;
            writeEnergyFilterPeakingAvg(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("energyfilter_pole_zero"):
            node >> val_uint32;
            writeEnergyFilterPoleZero(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_fine_gain"):
            node >> val_uint32;
            writeEnergyFilterFineGain(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_lf_limit"):
            node >> val_bool;
            writeEnergyFilterLFLimitation(channel, val_bool);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_bool << std::endl;
            break;
        case HashCode("energyfilter_baseline_avg"):
            node >> val_str;
            writeEnergyFilterBaselineAvg(channel, val_str);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_str << std::endl;
            break;
        case HashCode("energyfilter_baseline_guard"):
            node >> val_uint32;
            writeEnergyFilterBaselineGuard(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        case HashCode("energyfilter_pileup_guard"):
            node >> val_uint32;
            writeEnergyFilterPileupGuard(channel, val_uint32);
            std::cout << "채널: " << channel << ", 키: " << key << ", 값: " << val_uint32 << std::endl;
            break;
        default:
            break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print Information
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CAENV2740::printDigitizerInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "V2740 디지털 형식 변환기 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "모델 이름: " << readModelName() << '\n';
    std::cout << "시리얼 번호: " << readSerialNumber() << '\n';
    std::cout << "CUP 버전: " << readCUPVersion() << '\n';
    std::cout << "FPGA 펌웨어 버전: " << readFPGAFWVersion() << '\n';
    std::cout << "FW 타입: " << readFWType() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printLicenseInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "라이센스 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "라이센스: " << readLicense() << '\n';
    std::cout << "라이센스 상태: " << readLicenseStatus() << '\n';
    std::cout << "라이센스 남은 시간: " << readLicenseRemainingTime() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printSpecInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "사양 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "채널 수: " << readNumberOfChannels() << '\n';
    std::cout << "ADC 해상도: " << readADCResolution() << " 비트\n";
    std::cout << "ADC 샘플링 레이트: " << readADCSamplingRate() << " Hz\n";
    std::cout << "입력 범위: " << readInputRange() << " mV\n";
    std::cout << "입력 타입: " << readInputType() << '\n';
    std::cout << "입력 임피던스: " << readInputImpedance() << " Ω\n";
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printNetworkInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "네트워크 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "SPF 링크 존재 여부: " << readSFPLinkPresence() << '\n';
    std::cout << "SPF 링크 활성화 여부: " << readSFPLinkActive() << '\n';
    std::cout << "SPF 링크 프로토콜: " << readSFPLinkProtocol() << '\n';
    std::cout << "IP 주소: " << readIPAddress() << '\n';
    std::cout << "서브넷 마스크: " << readSubnetMask() << '\n';
    std::cout << "게이트웨이: " << readGateway() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printClockInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "클럭 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "클럭 소스: " << readClockSource() << '\n';
    std::cout << "클럭 출력 활성화: " << readEnClockOutFP() << '\n';
    std::cout << "변경 가능한 클럭 출력 지연: " << readVolatileClockOutDelay() << '\n';
    std::cout << "영원한 클럭 출력 지연: " << readPermanentClockOutDelay() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printTriggerSourceInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "트리거 소스" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "시작 소스: " << readStartSource() << '\n';
    std::cout << "전역 트리거 소스: " << readGlobalTriggerSource() << '\n';
    std::cout << "파형 저장 모드: " << readWaveSaving(0) << '\n';
    std::cout << "파형 트리거 소스: " << readWaveTriggerSource(0) << '\n';
    std::cout << "이벤트 트리거 소스: " << readEventTriggerSource(0) << '\n';
    std::cout << "채널 트리거 마스크: " << readChannelTriggerMask(0) << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printPanelIOInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "패널 IO 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "트리거 출력 모드: " << readTrgOutMode() << '\n';
    std::cout << "GPIO 모드: " << readGPIOMode() << '\n';
    std::cout << "BusyIn 입력 소스: " << readBusyInSource() << '\n';
    std::cout << "SyncOut 출력 모드: " << readSyncOutMode() << '\n';
    std::cout << "BoardVeto 입력 소스: " << readBoardVetoSource() << '\n';
    std::cout << "BoardVeto 너비: " << readBoardVetoWidth() << '\n';
    std::cout << "BoardVeto 극성: " << readBoardVetoPolarity() << '\n';
    std::cout << "ChannelVeto 입력 소스: " << readChannelVetoSource(0) << '\n';
    std::cout << "ChannelVeto 출력 모드: " << readChannelVetoWidth(0) << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printWaveParamInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "파형 매개변수 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "채널 레코드 길이: " << readChRecordLength(0) << " Samples" << '\n';
    std::cout << "채널 PreTrigger 길이: " << readChPreTrigger(0) << " Samples" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printWaveProbeInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "파형 프로브 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "파형 아날로그 프로브 0: " << readWaveAnalogProbe(0, 0) << '\n';
    std::cout << "파형 아날로그 프로브 1: " << readWaveAnalogProbe(1, 0) << '\n';
    std::cout << "파형 디지털 프로브 0: " << readWaveDigitalProbe(0, 0) << '\n';
    std::cout << "파형 디지털 프로브 1: " << readWaveDigitalProbe(1, 0) << '\n';
    std::cout << "파형 디지털 프로브 2: " << readWaveDigitalProbe(2, 0) << '\n';
    std::cout << "파형 디지털 프로브 3: " << readWaveDigitalProbe(3, 0) << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}
void CAENV2740::printRawDataInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "Raw 데이터 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "최대 Raw 데이터 크기: " << readMaxRawDataSize() << '\n';
    std::cout << "파형 데이터 소스: " << readWaveDataSource(0) << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printCounterInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "카운터 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "채널 실시간 모니터: " << readChRealTimeMonitor(0) << '\n';
    std::cout << "채널 데드타임 모니터: " << readChDeadTimeMonitor(0) << '\n';
    std::cout << "채널 트리거 카운터: " << readChTriggerCnt(0) << '\n';
    std::cout << "채널 저장된 이벤트 카운터: " << readChSavedEventCnt(0) << '\n';
    std::cout << "채널 파형 카운터: " << readChWaveCnt(0) << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printITLInfo(int option) {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "ITLX 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "ITL 연결 정보: " << readITLConnect(0) << '\n';
    if (option == 1 || option == 3) {
        std::cout << "ITLA 정보" << '\n';
        std::cout << "ITLA 메인 로직: " << readITLXMainLogic(true) << '\n';
        std::cout << "ITLA Majority 레벨: " << readITLXMajorityLev(true) << '\n';
        std::cout << "ITLA Pair 로직: " << readITLXPairLogic(true) << '\n';
        std::cout << "ITLA 극성: " << readITLXPolarity(true) << '\n';
        std::cout << "ITLA Mask: " << readITLXMask(true) << '\n';
        std::cout << "ITLA Gate 너비: " << readITLXGateWidth(true) << '\n';
    }
    if (option == 2 || option == 3) {
        std::cout << "ITLB 정보" << '\n';
        std::cout << "ITLB 메인 로직: " << readITLXMainLogic(false) << '\n';
        std::cout << "ITLB Majority 레벨: " << readITLXMajorityLev(false) << '\n';
        std::cout << "ITLB Pair 로직: " << readITLXPairLogic(false) << '\n';
        std::cout << "ITLB 극성: " << readITLXPolarity(false) << '\n';
        std::cout << "ITLB Mask: " << readITLXMask(false) << '\n';
        std::cout << "ITLB Gate 너비: " << readITLXGateWidth(false) << '\n';
    }
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printLVDSInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "LVDS 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "LVDS 모드: " << readLVDSMode(0) << '\n';
    std::cout << "LVDS 방향: " << readLVDSDirection(0) << '\n';
    std::cout << "LVDS I/O 레지스터: " << readLVDSIOReg() << '\n';
    // std::cout << "LVDS 트리거 마스크: " << readLVDSTrgMask() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printPanelDACInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "패널 DAC 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "DAC 출력 모드: " << readDACoutMode() << '\n';
    std::cout << "DAC 출력 정적 레벨: " << readDACoutStaticLevel() << '\n';
    std::cout << "DAC 출력 채널 선택: " << readDACoutChSelect() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printInputInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "입력 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "SelfTrigger 너비: " << readSelfTriggerWidth(0) << '\n';
    std::cout << "입력 지연: " << readInputDelay(0) << '\n';
    std::cout << "오프셋 교정: " << readEnOffsetCalibration() << '\n';
    std::cout << "채널 가능: " << readChEnable(0) << '\n';
    std::cout << "트리거 속도: " << readSelfTrgRate(0) << '\n';
    std::cout << "채널 상태: " << readChStatus(0) << '\n';
    std::cout << "채널 DC 오프셋: " << readDCOffset(0) << '\n';
    std::cout << "채널 신호 오프셋: " << readSignalOffset(0) << '\n';
    std::cout << "채널 게인 팩터: " << readGainFactor(0) << '\n';
    std::cout << "ADC to Volts 팩터: " << readADCToVolts(0) << '\n';
    std::cout << "트리거 문턱값: " << readTriggerThr(0) << '\n';
    std::cout << "펄스 극성: " << readPulsePolarity(0) << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printEventInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "이벤트 선택 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "에너지 스킴 저감 판별치: " << readEnergySkimLowDiscriminator(0) << '\n';
    std::cout << "에너지 스킴 고감 판별치: " << readEnergySkimHighDiscriminator(0) << '\n';
    std::cout << "이벤트 선택자: " << readEventSelector(0) << '\n';
    std::cout << "파동 선택자: " << readWaveSelector(0) << '\n';
    std::cout << "이벤트 중성자 거절: " << readEventNeutronReject(0) << '\n';
    std::cout << "파동 중성자 거절: " << readWaveNeutronReject(0) << '\n';
    std::cout << "동시 발생 마스크: " << readCoincidenceMask(0) << '\n';
    std::cout << "반동시 발생 마스크: " << readAntiCoincidenceMask(0) << '\n';
    std::cout << "동시 발생 길이: " << readCoincidenceLength(0) << " Samples\n";
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}

void CAENV2740::printDPPPSDInfo() {
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "DPPPSD 설정 정보" << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
    std::cout << "스무딩 팩터: " << readSmoothingFactor(0) << '\n';
    std::cout << "충전 스무딩: " << readChargeSmoothing(0) << '\n';
    std::cout << "시간 필터 스무딩: " << readTimeFilterSmoothing(0) << '\n';
    std::cout << "시간 필터 재트리거 가드: " << readTimeFilterRetriggerGuard(0) << " 샘플 수\n";
    std::cout << "트리거 히스테리시스: " << readTriggerHysteresis(0) << '\n';
    std::cout << "CFD 지연: " << readCFDDelay(0) << " 샘플 수\n";
    std::cout << "CFD 분수: " << readCFDFraction(0) << " %\n";
    std::cout << "트리거 필터 선택: " << readTriggerFilterSelection(0) << '\n';
    std::cout << "ADC 입력 기준선 평균: " << readADCInputBaselineAvg(0) << '\n';
    std::cout << "ADC 입력 기준선 가드: " << readADCInputBaselineGuard(0) << " 샘플 수\n";
    std::cout << "파이업 간격: " << readPileupGap(0) << " ADC\n";
    std::cout << "절대 기준선: " << readAbsoluteBaseline(0) << " ADC\n";
    std::cout << "게이트 오프셋: " << readGateOffset(0) << " 샘플 수\n";
    std::cout << "게이트 긴 길이: " << readGateLongLength(0) << " 샘플 수\n";
    std::cout << "게이트 짧은 길이: " << readGateShortLength(0) << " 샘플 수\n";
    std::cout << "긴 충전積분기 피데스탈: " << readLongChargeIntegratorPedestal(0) << " ADC\n";
    std::cout << "짧은 충전積분기 피데스탈: " << readShortChargeIntegratorPedestal(0) << " ADC\n";
    std::cout << "에너지 이득: " << readEnergyGain(0) << '\n';
    std::cout << "중성자 임계값: " << readNeutronThreshold(0) << " ADC\n";
    std::cout << "데이터 축소 활성화: " << readEnDataReduction() << '\n';
    std::cout << "통계 이벤트 활성화: " << readEnStatEvents() << '\n';
    std::cout << "========================================================================================="
                 "==============="
              << '\n';
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string CAENV2740::readActiveEndPoint() { return readRegisterInString("/endpoint/par/ActiveEndPoint"); }
void CAENV2740::writeActiveEndPoint(std::string endPoint) { writeRegister("/endpoint/par/ActiveEndPoint", endPoint); }

void CAENV2740::setDataFormatRaw() {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/raw", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("Handle 얻기 실패");
    }

    ret = CAEN_FELib_SetReadDataFormat(ep_handle,
                                       " \
        [ \
            { \"name\" : \"DATA\", \"type\" : \"U8\", \"dim\": 1 }, \
            { \"name\" : \"SIZE\", \"type\" : \"SIZE_T\" }, \
            { \"name\" : \"N_EVENTS\", \"type\" : \"U32\" } \
        ]");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 포맷 설정 실패");
    }
}

void CAENV2740::setDataFormatDPPPSD() {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/dpppsd", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("Handle 얻기 실패");
    }

    ret = CAEN_FELib_SetReadDataFormat(ep_handle,
                                       " \
        [ \
                { \"name\" : \"CHANNEL\", \"type\" : \"U8\" }, \
                { \"name\" : \"TIMESTAMP\", \"type\" : \"U64\" }, \
                { \"name\" : \"FINE_TIMESTAMP\", \"type\" : \"U16\" }, \
                { \"name\" : \"ENERGY\", \"type\" : \"U16\" }, \
                { \"name\" : \"ENERGY_SHORT\", \"type\" : \"U16\" }, \
                { \"name\" : \"ANALOG_PROBE_1\", \"type\" : \"I32\", \"dim\" : 1 }, \
                { \"name\" : \"ANALOG_PROBE_2\", \"type\" : \"I32\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_1\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_2\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_3\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_4\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"ANALOG_PROBE_1_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"ANALOG_PROBE_2_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_1_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_2_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_3_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_4_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"WAVEFORM_SIZE\", \"type\" : \"SIZE_T\" }, \
                { \"name\" : \"FLAGS_LOW_PRIORITY\", \"type\" : \"U16\"}, \
                { \"name\" : \"FLAGS_HIGH_PRIORITY\", \"type\" : \"U16\" }, \
                { \"name\" : \"EVENT_SIZE\", \"type\" : \"SIZE_T\" } \
        ]");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 포맷 설정 실패");
    }
}

void CAENV2740::setDataFormatDPPPHA() {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/dpppha", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error(std::string(__func__) + "함수에서 Handle 얻기 실패");
    }

    ret = CAEN_FELib_SetReadDataFormat(ep_handle,
                                       " \
        [ \
                { \"name\" : \"CHANNEL\", \"type\" : \"U8\" }, \
                { \"name\" : \"TIMESTAMP\", \"type\" : \"U64\" }, \
                { \"name\" : \"FINE_TIMESTAMP\", \"type\" : \"U16\" }, \
                { \"name\" : \"ENERGY\", \"type\" : \"U16\" }, \
                { \"name\" : \"ENERGY_SHORT\", \"type\" : \"U16\" }, \
                { \"name\" : \"ANALOG_PROBE_1\", \"type\" : \"I32\", \"dim\" : 1 }, \
                { \"name\" : \"ANALOG_PROBE_2\", \"type\" : \"I32\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_1\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_2\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_3\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"DIGITAL_PROBE_4\", \"type\" : \"U8\", \"dim\" : 1 }, \
                { \"name\" : \"ANALOG_PROBE_1_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"ANALOG_PROBE_2_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_1_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_2_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_3_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"DIGITAL_PROBE_4_TYPE\", \"type\" : \"U8\" }, \
                { \"name\" : \"WAVEFORM_SIZE\", \"type\" : \"SIZE_T\" }, \
                { \"name\" : \"FLAGS_LOW_PRIORITY\", \"type\" : \"U16\"}, \
                { \"name\" : \"FLAGS_HIGH_PRIORITY\", \"type\" : \"U16\" }, \
                { \"name\" : \"EVENT_SIZE\", \"type\" : \"SIZE_T\" } \
        ]");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error(std::string(__func__) + "함수에서 데이터 포맷 설정 실패");
    }
}

void CAENV2740::setDataFormatDPPPSDStats() {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/dpppsd/stats", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error(std::string(__func__) + "함수에서 Handle 얻기 실패");
    }

    ret = CAEN_FELib_SetReadDataFormat(ep_handle,
                                       " \
        [ \
            { \"name\" : \"REAL_TIME_NS\", \"type\" : \"U64\", \"dim\": 1 }, \
            { \"name\" : \"DEAD_TIME_NS\", \"type\" : \"U64\", \"dim\": 1 }, \
            { \"name\" : \"LIVE_TIME_NS\", \"type\" : \"U64\", \"dim\": 1 }, \
            { \"name\" : \"TRIGGER_CNT\", \"type\" : \"U32\", \"dim\": 1  }, \
            { \"name\" : \"SAVED_EVENT_CNT\", \"type\" : \"U32\", \"dim\": 1  }, \
        ]");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error(std::string(__func__) + "함수에서 데이터 포맷 설정 실패");
    }
}

void CAENV2740::setDataFormatDPPPHAStats() {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/dpppha/stats", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("Handle 얻기 실패");
    }

    ret = CAEN_FELib_SetReadDataFormat(ep_handle,
                                       " \
        [ \
            { \"name\" : \"REAL_TIME_NS\", \"type\" : \"U64\", \"dim\": 1 }, \
            { \"name\" : \"DEAD_TIME_NS\", \"type\" : \"U64\", \"dim\": 1 }, \
            { \"name\" : \"LIVE_TIME_NS\", \"type\" : \"U64\", \"dim\": 1 }, \
            { \"name\" : \"TRIGGER_CNT\", \"type\" : \"U32\", \"dim\": 1  }, \
            { \"name\" : \"SAVED_EVENT_CNT\", \"type\" : \"U32\", \"dim\": 1  }, \
        ]");
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("데이터 포맷 설정 실패");
    }
}

int CAENV2740::readData(int timeout) {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/dpppsd", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("Handle 얻기 실패");
    }

    ret = CAEN_FELib_ReadData(ep_handle, timeout, &evt.channel, &evt.timestamp, &evt.fine_timestamp, &evt.energy,
                              &evt.energy_short, evt.analog_probes[0], evt.analog_probes[1], evt.digital_probes[0],
                              evt.digital_probes[1], evt.digital_probes[2], evt.digital_probes[3],
                              &evt.analog_probes_type[0], &evt.analog_probes_type[1], &evt.digital_probes_type[0],
                              &evt.digital_probes_type[1], &evt.digital_probes_type[2], &evt.digital_probes_type[3],
                              &evt.n_samples, &evt.flags_low_priority, &evt.flags_high_priority, &evt.event_size);

    return ret;
}

int CAENV2740::readDataRaw(int timeout, uint8_t* data, size_t* size, uint32_t* n_events) {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/raw", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("Handle 얻기 실패");
    }

    ret = CAEN_FELib_ReadData(ep_handle, timeout, data, size, n_events);

    return ret;
}

int CAENV2740::readStats() {
    uint64_t ep_handle;
    int ret = CAEN_FELib_GetHandle(handle, "/endpoint/dpppsd/stats", &ep_handle);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("Handle 얻기 실패");
    }

    ret = CAEN_FELib_ReadData(ep_handle, 100, &evt.channel, &evt.timestamp, &evt.fine_timestamp, &evt.energy,
                              &evt.energy_short, evt.analog_probes[0], evt.analog_probes[1], evt.digital_probes[0],
                              evt.digital_probes[1], evt.digital_probes[2], evt.digital_probes[3],
                              &evt.analog_probes_type[0], &evt.analog_probes_type[1], &evt.digital_probes_type[0],
                              &evt.digital_probes_type[1], &evt.digital_probes_type[2], &evt.digital_probes_type[3],
                              &evt.n_samples, &evt.flags_low_priority, &evt.flags_high_priority, &evt.event_size);

    return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t CAENV2740::readRegister(const std::string& registerPath) {
    char value[128];  // 충분한 크기의 문자 배열 선언
    int ret = CAEN_FELib_GetValue(handle, registerPath.c_str(), value);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 읽기 실패: " + registerPath);
    }
    return std::stoul(value);  // 문자열을 uint32_t로 변환
}

float CAENV2740::readRegisterInFloat(const std::string& registerPath) {
    char value[128];  // 충분한 크기의 문자 배열 선언
    int ret = CAEN_FELib_GetValue(handle, registerPath.c_str(), value);
    if (ret != CAEN_FELib_Success) {
        throw std::runtime_error("레지스터 읽기 실패: " + registerPath);
    }
    return std::stof(value);  // 문자열을 float로 변환
}

std::string CAENV2740::readRegisterInString(const std::string& registerPath) {
    char value[128];  // 충분한 크기의 문자 배열 선언
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

float CAENV2740::readVolatileClockOutDelay() { return readRegisterInFloat("/par/VolatileClockOutDelay"); }
void CAENV2740::writeVolatileClockOutDelay(float delay) { writeRegister("/par/VolatileClockOutDelay", delay); }

float CAENV2740::readPermanentClockOutDelay() { return readRegisterInFloat("/par/PermanentClockOutDelay"); }
void CAENV2740::writePermanentClockOutDelay(float delay) { writeRegister("/par/PermanentClockOutDelay", delay); }

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

void CAENV2740::writeWaveDownSamplingFactor(int channel, int factor) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveDownSamplingFactor", std::to_string(factor));
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
    return readRegisterInFloat("/ch/" + std::to_string(channel) + "/par/ChRealTimeMonitor");
}
float CAENV2740::readChDeadTimeMonitor(int channel) {
    return readRegisterInFloat("/ch/" + std::to_string(channel) + "/par/ChDeadTimeMonitor");
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

std::string CAENV2740::readITLXMainLogic(bool isA) {
    return readRegisterInString("/par/ITL" + std::string(isA ? "A" : "B") + "MainLogic");
}
void CAENV2740::writeITLXMainLogic(bool isA, std::string logic) {
    writeRegister("/par/ITL" + std::string(isA ? "A" : "B") + "MainLogic", logic);
}
uint32_t CAENV2740::readITLXMajorityLev(bool isA) {
    return readRegister("/par/ITL" + std::string(isA ? "A" : "B") + "MajorityLev");
}
void CAENV2740::writeITLXMajorityLev(bool isA, uint32_t level) {
    writeRegister("/par/ITL" + std::string(isA ? "A" : "B") + "MajorityLev", level);
}
std::string CAENV2740::readITLXPairLogic(bool isA) {
    return readRegisterInString("/par/ITL" + std::string(isA ? "A" : "B") + "PairLogic");
}
void CAENV2740::writeITLXPairLogic(bool isA, std::string logic) {
    writeRegister("/par/ITL" + std::string(isA ? "A" : "B") + "PairLogic", logic);
}
std::string CAENV2740::readITLXPolarity(bool isA) {
    return readRegisterInString("/par/ITL" + std::string(isA ? "A" : "B") + "Polarity");
}
void CAENV2740::writeITLXPolarity(bool isA, std::string polarity) {
    writeRegister("/par/ITL" + std::string(isA ? "A" : "B") + "Polarity", polarity);
}
uint32_t CAENV2740::readITLXMask(bool isA) { return readRegister("/par/ITL" + std::string(isA ? "A" : "B") + "Mask"); }
void CAENV2740::writeITLXMask(bool isA, uint32_t mask) {
    writeRegister("/par/ITL" + std::string(isA ? "A" : "B") + "Mask", mask);
}
std::string CAENV2740::readITLConnect(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/ITLConnect");
}
void CAENV2740::writeITLConnect(int channel, std::string connect) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ITLConnect", connect);
}
uint32_t CAENV2740::readITLXGateWidth(bool isA) {
    return readRegister("/par/ITL" + std::string(isA ? "A" : "B") + "GateWidth");
}
void CAENV2740::writeITLXGateWidth(bool isA, uint32_t width) {
    writeRegister("/par/ITL" + std::string(isA ? "A" : "B") + "GateWidth", width);
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
void CAENV2740::writeEventNeutronReject(int channel, bool reject) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EventNeutronReject", reject ? "Enabled" : "Disabled");
}
std::string CAENV2740::readWaveNeutronReject(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/WaveNeutronReject");
}
void CAENV2740::writeWaveNeutronReject(int channel, bool reject) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/WaveNeutronReject", reject ? "Enabled" : "Disabled");
}

std::string CAENV2740::readCoincidenceMask(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/CoincidenceMask");
}
void CAENV2740::writeCoincidenceMask(int channel, std::string mask) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/CoincidenceMask", mask);
}
std::string CAENV2740::readAntiCoincidenceMask(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/AntiCoincidenceMask");
}
void CAENV2740::writeAntiCoincidenceMask(int channel, std::string mask) {
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

void CAENV2740::writeChargeSmoothing(int channel, bool smoothing) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/ChargeSmoothing", smoothing ? "Enabled" : "Disabled");
}

std::string CAENV2740::readTimeFilterSmoothing(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/TimeFilterSmoothing");
}
void CAENV2740::writeTimeFilterSmoothing(int channel, bool smoothing) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterSmoothing", smoothing ? "Enabled" : "Disabled");
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
void CAENV2740::writeTriggerHysteresis(int channel, bool hysteresis) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TriggerHysteresis", hysteresis ? "Enabled" : "Disabled");
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
void CAENV2740::writeEnergyGain(int channel, int gain) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyGain", "x" + std::to_string(gain));
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DPP Algorithm 관련 (additional for DPPPHA)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t CAENV2740::readTimeFilterRiseTime(int channel, bool isSample) {
    return isSample ? readTimeFilterRiseTimeS(channel) : readTimeFilterRiseTimeT(channel);
}
void CAENV2740::writeTimeFilterRiseTime(int channel, uint32_t riseTime, bool isSample) {
    isSample ? writeTimeFilterRiseTimeS(channel, riseTime) : writeTimeFilterRiseTimeT(channel, riseTime);
}
uint32_t CAENV2740::readTimeFilterRiseTimeS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRiseTimeS");
}
void CAENV2740::writeTimeFilterRiseTimeS(int channel, uint32_t riseTime) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRiseTimeS", riseTime);
}
uint32_t CAENV2740::readTimeFilterRiseTimeT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRiseTimeT");
}
void CAENV2740::writeTimeFilterRiseTimeT(int channel, uint32_t riseTime) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/TimeFilterRiseTimeT", riseTime);
}

uint32_t CAENV2740::readEnergyFilterRiseTime(int channel, bool isSample) {
    return isSample ? readEnergyFilterRiseTimeS(channel) : readEnergyFilterRiseTimeT(channel);
}
void CAENV2740::writeEnergyFilterRiseTime(int channel, uint32_t riseTime, bool isSample) {
    isSample ? writeEnergyFilterRiseTimeS(channel, riseTime) : writeEnergyFilterRiseTimeT(channel, riseTime);
}

uint32_t CAENV2740::readEnergyFilterRiseTimeS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterRiseTimeS");
}

void CAENV2740::writeEnergyFilterRiseTimeS(int channel, uint32_t riseTime) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterRiseTimeS", riseTime);
}

uint32_t CAENV2740::readEnergyFilterRiseTimeT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterRiseTimeT");
}

void CAENV2740::writeEnergyFilterRiseTimeT(int channel, uint32_t riseTime) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterRiseTimeT", riseTime);
}
uint32_t CAENV2740::readEnergyFilterFlatTop(int channel, bool isSample) {
    return isSample ? readEnergyFilterFlatTopS(channel) : readEnergyFilterFlatTopT(channel);
}

void CAENV2740::writeEnergyFilterFlatTop(int channel, uint32_t flatTop, bool isSample) {
    isSample ? writeEnergyFilterFlatTopS(channel, flatTop) : writeEnergyFilterFlatTopT(channel, flatTop);
}

uint32_t CAENV2740::readEnergyFilterFlatTopS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterFlatTopS");
}

void CAENV2740::writeEnergyFilterFlatTopS(int channel, uint32_t flatTop) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterFlatTopS", flatTop);
}

uint32_t CAENV2740::readEnergyFilterFlatTopT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterFlatTopT");
}

void CAENV2740::writeEnergyFilterFlatTopT(int channel, uint32_t flatTop) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterFlatTopT", flatTop);
}
uint32_t CAENV2740::readEnergyFilterPeakingPosition(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPeakingPosition");
}

void CAENV2740::writeEnergyFilterPeakingPosition(int channel, uint32_t position) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPeakingPosition", position);
}

std::string CAENV2740::readEnergyFilterPeakingAvg(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/EnergyFilterPeakingAvg");
}

void CAENV2740::writeEnergyFilterPeakingAvg(int channel, std::string avg) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPeakingAvg", avg);
}

uint32_t CAENV2740::readEnergyFilterPoleZero(int channel, bool isSample) {
    return isSample ? readEnergyFilterPoleZeroS(channel) : readEnergyFilterPoleZeroT(channel);
}

void CAENV2740::writeEnergyFilterPoleZero(int channel, uint32_t poleZero, bool isSample) {
    isSample ? writeEnergyFilterPoleZeroS(channel, poleZero) : writeEnergyFilterPoleZeroT(channel, poleZero);
}

uint32_t CAENV2740::readEnergyFilterPoleZeroS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPoleZeroS");
}

void CAENV2740::writeEnergyFilterPoleZeroS(int channel, uint32_t poleZero) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPoleZeroS", poleZero);
}

uint32_t CAENV2740::readEnergyFilterPoleZeroT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPoleZeroT");
}

void CAENV2740::writeEnergyFilterPoleZeroT(int channel, uint32_t poleZero) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPoleZeroT", poleZero);
}

float CAENV2740::readEnergyFilterFineGain(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterFineGain");
}

void CAENV2740::writeEnergyFilterFineGain(int channel, float gain) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterFineGain", gain);
}

bool CAENV2740::readEnergyFilterLFLimitation(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterLFLimitation");
}

void CAENV2740::writeEnergyFilterLFLimitation(int channel, bool limitation) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterLFLimitation", limitation ? "On" : "Off");
}

std::string CAENV2740::readEnergyFilterBaselineAvg(int channel) {
    return readRegisterInString("/ch/" + std::to_string(channel) + "/par/EnergyFilterBaselineAvg");
}

void CAENV2740::writeEnergyFilterBaselineAvg(int channel, std::string avg) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterBaselineAvg", avg);
}

uint32_t CAENV2740::readEnergyFilterBaselineGuard(int channel, bool isSample) {
    return isSample ? readEnergyFilterBaselineGuardS(channel) : readEnergyFilterBaselineGuardT(channel);
}

void CAENV2740::writeEnergyFilterBaselineGuard(int channel, uint32_t guard, bool isSample) {
    isSample ? writeEnergyFilterBaselineGuardS(channel, guard) : writeEnergyFilterBaselineGuardT(channel, guard);
}

uint32_t CAENV2740::readEnergyFilterBaselineGuardS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterBaselineGuardS");
}

void CAENV2740::writeEnergyFilterBaselineGuardS(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterBaselineGuardS", guard);
}

uint32_t CAENV2740::readEnergyFilterBaselineGuardT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterBaselineGuardT");
}

void CAENV2740::writeEnergyFilterBaselineGuardT(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterBaselineGuardT", guard);
}

uint32_t CAENV2740::readEnergyFilterPileupGuard(int channel, bool isSample) {
    return isSample ? readEnergyFilterPileupGuardS(channel) : readEnergyFilterPileupGuardT(channel);
}

void CAENV2740::writeEnergyFilterPileupGuard(int channel, uint32_t guard, bool isSample) {
    isSample ? writeEnergyFilterPileupGuardS(channel, guard) : writeEnergyFilterPileupGuardT(channel, guard);
}

uint32_t CAENV2740::readEnergyFilterPileupGuardS(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPileupGuardS");
}

void CAENV2740::writeEnergyFilterPileupGuardS(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPileupGuardS", guard);
}

uint32_t CAENV2740::readEnergyFilterPileupGuardT(int channel) {
    return readRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPileupGuardT");
}

void CAENV2740::writeEnergyFilterPileupGuardT(int channel, uint32_t guard) {
    writeRegister("/ch/" + std::to_string(channel) + "/par/EnergyFilterPileupGuardT", guard);
}
