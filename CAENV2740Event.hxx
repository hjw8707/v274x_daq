#include <CAEN_FELib.h>

#include <iostream>

class CAENV2740Event {
   public:
    CAENV2740Event();
    ~CAENV2740Event();

    void Serialize(std::ostream& os) const;
    void SerializeWithWave(std::ostream& os) const;

   public:
    uint8_t channel;
    uint64_t timestamp;
    uint16_t fine_timestamp;
    uint16_t energy;
    uint16_t energy_short;
    uint16_t flags_low_priority;
    uint16_t flags_high_priority;
    size_t event_size;
    int32_t* analog_probes[2];
    uint8_t* digital_probes[4];
    uint8_t analog_probes_type[2];
    uint8_t digital_probes_type[4];
    size_t n_allocated_samples;
    size_t n_samples;
};