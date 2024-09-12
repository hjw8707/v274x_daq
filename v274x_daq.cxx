// main.c
#include <stdio.h>

#include "QCAENV2740.hxx"  // QCAENV2740 헤더 파일 포함
#include "QtCore/QCommandLineOption.h"
#include "QtCore/QCommandLineParser.h"
#include "QtCore/QString.h"
#include "QtWidgets/QApplication.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);  // QApplication 객체 생성
                                   // QCommandLineParser를 사용하여 IP 주소를 받는 -a 옵션 추가
    QCommandLineParser parser;     // QCommandLineParser 객체 생성
    parser.setApplicationDescription("Application Description");  // 설명 설정
    parser.addHelpOption();                                       // 도움말 옵션 추가
    parser.addVersionOption();                                    // 버전 옵션 추가

    // -a 옵션 추가 (IP 주소를 받음)
    QCommandLineOption ipOption("a", "Set the IP address", "ip");
    parser.addOption(ipOption);

    parser.process(app);  // 인자 처리
    QString ipAddress;
    if (parser.isSet(ipOption)) ipAddress = parser.value(ipOption);

    QCAENV2740 qcaenv2740;
    if (!ipAddress.isEmpty()) {
        qcaenv2740.setIPAddress(ipAddress);
        qcaenv2740.connectDAQ();
    }
    qcaenv2740.show();
    return app.exec();
}