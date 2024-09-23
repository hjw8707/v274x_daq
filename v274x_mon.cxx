#include <iostream>
#include <thread>

#include "OnlineMonitor.hxx"
#include "TApplication.h"
#include "TROOT.h"

int main(int argc, char** argv) {
    gROOT->SetBatch(true);
    TApplication app("app", &argc, argv);
    OnlineMonitor monitor("/v2740");
    // monitor.setVerbose(true);
    // monitor.start();
    // 모니터를 별도의 스레드에서 시작
    std::thread monitorThread([&monitor]() {
        std::cout << "Monitor Start" << std::endl;
        monitor.start();
    });

    app.Run();
    //  스레드 종료 대기
    if (monitorThread.joinable()) {
        monitorThread.join();
    }

    return 0;
}
