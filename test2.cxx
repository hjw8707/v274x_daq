#include <chrono>
#include <iostream>
#include <thread>

#include "QOnlineMonitor.hxx"
#include "QtWidgets/QApplication.h"
#include "QtWidgets/QMessageBox.h"
#include "TApplication.h"
#include "TROOT.h"
#include "TSystem.h"
int main(int argc, char** argv) {
    gROOT->SetBatch(true);
    TApplication app("app", &argc, argv);
    QApplication app2(argc, argv);
    QOnlineMonitor monitor;
    monitor.attachSharedMemory("online_monitor");
    monitor.attachSharedMemory("online_monitor_2");
    //  monitor.setVerbose(true);
    //  monitor.setVerbose(true);
    monitor.start();
    // 모니터를 별도의 스레드에서 시작

    std::thread appThread([&app]() { app.Run(kTRUE); });
    //  QTimer를 사용하여 TApplication의 이벤트를 주기적으로 처리
    // QTimer timer;
    // QObject::connect(&timer, &QTimer::timeout, []() { gSystem->ProcessEvents(); });
    // timer.start(10);  // 10ms마다 이벤트 처리
    std::cout << "Main thread started" << std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(100));
    auto result = QMessageBox::information(nullptr, "Information", "Hello, World!");
    std::cout << "Result: " << result << std::endl;
    if (result == QMessageBox::Ok) {
        std::cout << "Ok button clicked" << std::endl;
        app.Terminate(0);
        appThread.join();
        std::cout << "AppThread joined" << std::endl;
        monitor.stop();
        QTimer::singleShot(5000, &app2, &QApplication::quit);  // 5초 후에 애플리케이션 종료
    }
    return app2.exec();
}
