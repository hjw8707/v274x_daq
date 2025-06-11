// main.c
#include <stdio.h>

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtWidgets/QApplication>

#include "QCAENV2740Single.hxx"  // QCAENV2740 헤더 파일 포함

int main(int argc, char* argv[]) {
    QLoggingCategory::setFilterRules("*.debug=false");

    QApplication app(argc, argv);  // QApplication 객체 생성
    app.setApplicationName("V274X_DAQ");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;  // QCommandLineParser 객체 생성
    parser.setApplicationDescription(
        "이 프로그램은 QCAENV2740 장치와의 연결 및 데이터 수집을 관리하는 GUI 애플리케이션입니다.");  // 프로그램 설명
                                                                                                      // 설정
    parser.addHelpOption();     // 도움말 옵션 추가
    parser.addVersionOption();  // 버전 옵션 추가

    QCommandLineOption ipOption(QStringList() << "c" << "connect", "Set the IP address", "ip");
    QCommandLineOption parOption(QStringList() << "p" << "parameter", "Set the parameter file", "par");
    QCommandLineOption dataDirectoryOption(QStringList() << "d" << "data", "Set the data directory", "data");
    parser.addOption(ipOption);
    parser.addOption(parOption);
    parser.addOption(dataDirectoryOption);

    parser.process(app);  // 인자 처리

    // 명령줄 인자가 있으면 그것을 사용하고, 없으면 설정 파일의 값을 사용
    QString ipAddress = parser.isSet(ipOption) ? parser.value(ipOption) : "";
    QString parFile = parser.isSet(parOption) ? parser.value(parOption) : "";
    QString dataDirectory = parser.isSet(dataDirectoryOption) ? parser.value(dataDirectoryOption) : "";

    QCAENV2740Single qcaenv2740;
    // 화면 표시
    qcaenv2740.show();
    return app.exec();
}