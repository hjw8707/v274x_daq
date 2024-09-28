// main.c
#include <stdio.h>

#include "QCAENV274XMulti.hxx"  // QCAENV2740 헤더 파일 포함
#include "QtCore/QCommandLineOption.h"
#include "QtCore/QCommandLineParser.h"
#include "QtCore/QString.h"
#include "QtWidgets/QApplication.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);  // QApplication 객체 생성
    // QCommandLineParser를 사용하여 IP 주소를 받는 -a 옵션 추가
    QCommandLineParser parser;     // QCommandLineParser 객체 생성
    parser.setApplicationDescription("This program is used to connect to multiple V2740 digitizers via their IP addresses and manage data acquisition.");  // Set application description
    parser.addHelpOption();                                       // 도움말 옵션 추가
    parser.addVersionOption();                                    // 버전 옵션 추가
    QCommandLineOption ipOption(QStringList() << "c"
                                              << "connect",
                                "IP addresses to connect to, separated by commas.", "ip");
    parser.addOption(ipOption);

    parser.process(app);

    QStringList ipAddresses = parser.value(ipOption).split(',', Qt::SkipEmptyParts);
    for (const QString &ip : ipAddresses) {
        qDebug() << "IP Address:" << ip;
    }

    QCAENV274XMulti qcaenv274xm;
    for (const QString &ip : ipAddresses) {
        qcaenv274xm.addDigitizer(QString(), ip);
    }
    qcaenv274xm.show();
    return app.exec();
}