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
    for (size_t i = 0; i < 2; i++) delete analog_probes[i];
    for (size_t i = 0; i < 4; i++) delete digital_probes[i];
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