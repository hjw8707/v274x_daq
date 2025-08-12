#include "CAENV2740Event.hxx"
#define MAX_NUMBER_OF_SAMPLES (4095 * 2)

CAENV2740Event::CAENV2740Event()
    : channel(0),
      timestamp(0),
      fine_timestamp(0),
      energy(0),
      energy_short(0),
      flags_low_priority(0),
      flags_high_priority(0),
      event_size(0),
      n_samples(0) {
    for (size_t i = 0; i < 2; i++) {
        analog_probes_type[i] = 0;
        analog_probes[i] = new int32_t[MAX_NUMBER_OF_SAMPLES];
    }
    for (size_t i = 0; i < 4; i++) {
        digital_probes_type[i] = 0;
        digital_probes[i] = new uint8_t[MAX_NUMBER_OF_SAMPLES];
    }
    n_allocated_samples = MAX_NUMBER_OF_SAMPLES;
}
CAENV2740Event::~CAENV2740Event() {
    for (size_t i = 0; i < 2; i++) delete[] analog_probes[i];
    for (size_t i = 0; i < 4; i++) delete[] digital_probes[i];
}

void CAENV2740Event::Print(std::ostream& os) const {
    os << "===============================================================\n";
    os << "채널 번호: " << static_cast<unsigned int>(channel) << std::endl;
    os << "타임스탬프: " << timestamp << std::endl;
    os << "세밀한 타임스탬프: " << fine_timestamp << std::endl;
    os << "에너지: " << energy << std::endl;
    os << "단기 에너지: " << energy_short << std::endl;
    os << "낮은 우선순위 플래그: 0x" << std::hex << flags_low_priority << std::endl;
    os << "높은 우선순위 플래그: 0x" << std::hex << flags_high_priority << std::endl;
    os << std::dec;
    os << "이벤트 크기: " << event_size << std::endl;
    os << "샘플 수: " << n_samples << std::endl;
    os << "===============================================================\n";
}

void CAENV2740Event::Serialize(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&channel), sizeof(channel));
    os.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
    os.write(reinterpret_cast<const char*>(&fine_timestamp), sizeof(fine_timestamp));
    os.write(reinterpret_cast<const char*>(&energy), sizeof(energy));
    os.write(reinterpret_cast<const char*>(&energy_short), sizeof(energy_short));
    os.write(reinterpret_cast<const char*>(&flags_low_priority), sizeof(flags_low_priority));
    os.write(reinterpret_cast<const char*>(&flags_high_priority), sizeof(flags_high_priority));
    os.write(reinterpret_cast<const char*>(&event_size), sizeof(event_size));
}

// 복사 생성자
CAENV2740Event::CAENV2740Event(const CAENV2740Event& other)
    : channel(other.channel),
      timestamp(other.timestamp),
      fine_timestamp(other.fine_timestamp),
      energy(other.energy),
      energy_short(other.energy_short),
      flags_low_priority(other.flags_low_priority),
      flags_high_priority(other.flags_high_priority),
      event_size(other.event_size),
      n_samples(other.n_samples),
      n_allocated_samples(other.n_allocated_samples) {
    // 동적 할당된 배열들을 깊은 복사
    for (size_t i = 0; i < 2; i++) {
        analog_probes_type[i] = other.analog_probes_type[i];
        analog_probes[i] = new int32_t[MAX_NUMBER_OF_SAMPLES];
        std::copy(other.analog_probes[i], other.analog_probes[i] + MAX_NUMBER_OF_SAMPLES, analog_probes[i]);
    }
    for (size_t i = 0; i < 4; i++) {
        digital_probes_type[i] = other.digital_probes_type[i];
        digital_probes[i] = new uint8_t[MAX_NUMBER_OF_SAMPLES];
        std::copy(other.digital_probes[i], other.digital_probes[i] + MAX_NUMBER_OF_SAMPLES, digital_probes[i]);
    }
}

// 대입 연산자
CAENV2740Event& CAENV2740Event::operator=(const CAENV2740Event& other) {
    if (this != &other) {  // 자기 자신에 대한 대입 검사
        // 기존 메모리 해제
        for (size_t i = 0; i < 2; i++) {
            delete[] analog_probes[i];
        }
        for (size_t i = 0; i < 4; i++) {
            delete[] digital_probes[i];
        }

        // 기본 멤버 변수들 복사
        channel = other.channel;
        timestamp = other.timestamp;
        fine_timestamp = other.fine_timestamp;
        energy = other.energy;
        energy_short = other.energy_short;
        flags_low_priority = other.flags_low_priority;
        flags_high_priority = other.flags_high_priority;
        event_size = other.event_size;
        n_samples = other.n_samples;
        n_allocated_samples = other.n_allocated_samples;

        // 동적 할당된 배열들을 깊은 복사
        for (size_t i = 0; i < 2; i++) {
            analog_probes_type[i] = other.analog_probes_type[i];
            analog_probes[i] = new int32_t[MAX_NUMBER_OF_SAMPLES];
            std::copy(other.analog_probes[i], other.analog_probes[i] + MAX_NUMBER_OF_SAMPLES, analog_probes[i]);
        }
        for (size_t i = 0; i < 4; i++) {
            digital_probes_type[i] = other.digital_probes_type[i];
            digital_probes[i] = new uint8_t[MAX_NUMBER_OF_SAMPLES];
            std::copy(other.digital_probes[i], other.digital_probes[i] + MAX_NUMBER_OF_SAMPLES, digital_probes[i]);
        }
    }
    return *this;
}

void CAENV2740Event::SerializeWithWave(std::ostream& os) const {
    os.write(reinterpret_cast<const char*>(&channel), sizeof(channel));
    os.write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp));
    os.write(reinterpret_cast<const char*>(&fine_timestamp), sizeof(fine_timestamp));
    os.write(reinterpret_cast<const char*>(&energy), sizeof(energy));
    os.write(reinterpret_cast<const char*>(&energy_short), sizeof(energy_short));
    os.write(reinterpret_cast<const char*>(&flags_low_priority), sizeof(flags_low_priority));
    os.write(reinterpret_cast<const char*>(&flags_high_priority), sizeof(flags_high_priority));
    os.write(reinterpret_cast<const char*>(&event_size), sizeof(event_size));
    os.write(reinterpret_cast<const char*>(analog_probes), sizeof(analog_probes));
    os.write(reinterpret_cast<const char*>(digital_probes), sizeof(digital_probes));
    os.write(reinterpret_cast<const char*>(analog_probes_type), sizeof(analog_probes_type));
    os.write(reinterpret_cast<const char*>(digital_probes_type), sizeof(digital_probes_type));
    os.write(reinterpret_cast<const char*>(&n_allocated_samples), sizeof(n_allocated_samples));
    os.write(reinterpret_cast<const char*>(&n_samples), sizeof(n_samples));
}