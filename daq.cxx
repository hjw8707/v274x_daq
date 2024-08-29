
#include <iostream>

#include "CAENV2740.hxx"
#include "CAENV2740Par.hxx"

void usage() {
    std::cerr << "사용법: ./daq [-p] <V2740_IP>\n";
    std::cerr << "  -p: 파라미터를 로딩할 때 사용하는 옵션입니다.\n";
    std::cerr << "  <V2740_IP>: V2740 모듈의 IP 주소를 지정합니다.\n";
}

static void read_data_loop(FILE* f_evt, struct acq_data* acq_data, uint64_t ep_handle, struct event* evt,
                           struct histograms* h) {
    struct counters total;
    struct counters interval;

    counters_reset(&total, time(NULL));
    counters_reset(&interval, total.t_begin);

    for (;;) {
        const time_t current_time = time(NULL);
        const int ret = CAEN_FELib_ReadData(
            ep_handle, 100, &evt->channel, &evt->timestamp, &evt->fine_timestamp, &evt->energy, &evt->energy_short,
            evt->analog_probes[0], evt->analog_probes[1], evt->digital_probes[0], evt->digital_probes[1],
            evt->digital_probes[2], evt->digital_probes[3], &evt->analog_probes_type[0], &evt->analog_probes_type[1],
            &evt->digital_probes_type[0], &evt->digital_probes_type[1], &evt->digital_probes_type[2],
            &evt->digital_probes_type[3], &evt->n_samples, &evt->flags_low_priority, &evt->flags_high_priority,
            &evt->event_size);
        switch (ret) {
            case CAEN_FELib_Success: {
                evt->timestamp_us = evt->timestamp * .008;

                counters_increment(&total, evt->event_size);
                counters_increment(&interval, evt->event_size);

                fill_histogram(evt, h);
                save_event(f_evt, evt);

                const bool has_waveform = evt->n_samples > 0;

                if (has_waveform) {
                    mtx_lock(&acq_data->mtx);
                    if (acq_data->plot_next_wave) {
                        acq_data->plot_next_wave = false;
                        plot_waveform(plotters->gnuplot_w, evt);
                    }
                    mtx_unlock(&acq_data->mtx);
                }

                break;
            }
            case CAEN_FELib_Timeout:
                break;
            case CAEN_FELib_Stop:
                printf("\nStop received.\n");
                return;
            default:
                print_last_error();
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    bool flagPar = false;
    std::string parFile;
    std::string ip;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-p") {
            if (i + 1 < argc) {
                flagPar = true;
                parFile = argv[i + 1];
                i++;  // 다음 인자로 넘어가기 위해
            }
        } else {
            ip = argv[i];  // IP 주소 저장
        }
    }
    if (ip.empty()) {
        usage();
        return 1;
    }

#ifdef DEBUG
    std::cerr << "사용된 IP 주소: " << ip << std::endl;
    if (flagPar) {
        std::cerr << "파라미터 파일명: " << parFile << std::endl;
    }
#endif

    // CAEN V2740 모듈 초기화
    CAENV2740 v2740("dig2://" + std::string(argv[1]));
    v2740.connect();

    // 파라미터 파일 로딩
    if (flagPar) {
        CAENV2740Par par(parFile);
        v2740.loadParameter(par);
    }

    // 데이터 수집 설정
    v2740.setDataFormatDPPPSD();
    v2740.writeActiveEndPoint("dpppsd");

    // 데이터 수집 시작
    v2740.startAcquisition();

    // 데이터 수집 중...
    printf("데이터 수집 중...\n");

    // 데이터 수집 종료
    v2740.stopAcquisition();

    // 데이터 수집 종료 확인
    printf("데이터 수집 종료.\n");

    return 0;
}
