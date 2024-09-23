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
    parser.setApplicationDescription("Application Description");  // 설명 설정
    parser.addHelpOption();                                       // 도움말 옵션 추가
    parser.addVersionOption();                                    // 버전 옵션 추가

    QCAENV274XMulti qcaenv274xm;
    qcaenv274xm.show();
    return app.exec();
}