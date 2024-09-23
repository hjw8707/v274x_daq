#include <iomanip>
#include <iostream>

#include "SharedMemory.hxx"

void eventParser(uint64_t* datas, int nWords) {
    int channel = (datas[0] >> 56) & 0x7F;
    bool special_event = (datas[0] >> 55) & 0x1;
    uint64_t timestamp = datas[0] & 0xFFFFFFFFFFFF;
    bool with_waveform = (datas[1] >> 62) & 0x1;
    uint16_t flags_low_priority = (datas[1] >> 50) & 0x3FF;
    uint16_t flags_high_priority = (datas[1] >> 42) & 0xFF;
    uint16_t energy_short = (datas[1] >> 26) & 0xFFFF;
    uint16_t fine_timestamp = (datas[1] >> 16) & 0xFFFF;
    uint16_t energy = datas[1] & 0xFFFF;

    // std::cout << "Event Parser" << std::endl;
    // std::cout << "channel: " << channel << std::endl;
    // std::cout << "special_event: " << special_event << std::endl;
    // std::cout << "timestamp: " << timestamp << std::endl;
    // std::cout << "with_waveform: " << with_waveform << std::endl;
    // std::cout << "flags_high_priority: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(3)
    //           << flags_high_priority << std::endl;
    // std::cout << "flags_low_priority: 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
    //           << flags_low_priority << std::endl;
    // std::cout << std::dec;
    // std::cout << "energy_short: " << energy_short << std::endl;
    // std::cout << "fine_timestamp: " << fine_timestamp << std::endl;
    // std::cout << "energy: " << energy << std::endl;
}

int main() {
    SharedMemory shm("v2740", true);
    uint64_t* buffer = new uint64_t[2621440 / 8];

    while (true) {
        size_t size = shm.readNewData(buffer);

        int j = 1;
        for (int i = 1; i < size; i++) {
            buffer[i] = __builtin_bswap64(buffer[i]);
            std::cout << "buffer[i]: " << std::hex << buffer[i] << std::endl;
            if ((buffer[i] >> 63) & 0x1) {
                std::cout << "Event Detected" << std::endl;
                std::cout << "i: " << i << std::endl;
                std::cout << "j: " << j << std::endl;
                eventParser(&buffer[j], i - j + 1);
                j = i + 1;
            }  // 이벤트 수 증가
        }
        getchar();
    }

    delete[] buffer;
    return 0;
}
