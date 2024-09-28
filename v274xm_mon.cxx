// main.c
#include <stdio.h>

#include "QOnlineMonitorWindow.hxx"  // QCAENV2740 헤더 파일 포함
#include "QtCore/QCommandLineOption.h"
#include "QtCore/QCommandLineParser.h"
#include "QtCore/QString.h"
#include "QtWidgets/QApplication.h"
#include "TApplication.h"
#include "TROOT.h"
int main(int argc, char *argv[]) {
    gROOT->SetBatch(true);
    QApplication app(argc, argv);  // QApplication 객체 생성
    TApplication tApp("v274xm_mon", &argc, argv);

    // QCommandLineParser를 사용하여 IP 주소를 받는 -a 옵션 추가
    QCommandLineParser parser;  // QCommandLineParser 객체 생성
    parser.setApplicationDescription(
        "This program uses THttpServer to monitor the current data in SHM.");  // Set application description
    parser.addHelpOption();                                                    // 도움말 옵션 추가
    parser.addVersionOption();                                                 // 버전 옵션 추가

    QCommandLineOption shmOption(QStringList() << "s"
                                               << "shm",
                                 "Names of shared memories to monitor, separated by commas.", "shm");
    parser.addOption(shmOption);
    parser.process(app);

    QStringList shmNames;
    if (parser.isSet(shmOption)) {
        shmNames = parser.value(shmOption).split(',', Qt::SkipEmptyParts);
    }
    for (const QString &shm : shmNames) {
        qDebug() << "Shared Memory Name:" << shm;
    }

    QOnlineMonitorWindow qom;
    std::thread appThread([&tApp]() { tApp.Run(kTRUE); });
    for (const QString &shm : shmNames) qom.getOnlineMonitor()->attachSharedMemory(shm);
    qom.show();
    int ret = app.exec();

    tApp.Terminate(0);
    appThread.join();
    return ret;
}