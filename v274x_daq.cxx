// main.c
#include <stdio.h>

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtWidgets/QApplication>

#include "QCAENV2740Single.hxx"  // QCAENV2740 헤더 파일 포함
#include "SettingsManager.hxx"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);  // QApplication 객체 생성

    // 설정 로드
    SettingsManager& settings = SettingsManager::getInstance();
    if (!settings.loadSettings()) {
        qDebug() << "설정 파일을 로드할 수 없습니다. 기본값을 사용합니다.";
    }

    QCommandLineParser parser;  // QCommandLineParser 객체 생성
    parser.setApplicationDescription("Application Description");  // 설명 설정
    parser.addHelpOption();     // 도움말 옵션 추가
    parser.addVersionOption();  // 버전 옵션 추가

    // -a 옵션 추가 (IP 주소를 받음)
    QCommandLineOption ipOption("c", "Set the IP address", "ip");
    QCommandLineOption parOption("p", "Set the parameter file", "par");
    parser.addOption(ipOption);
    parser.addOption(parOption);

    parser.process(app);  // 인자 처리

    // 명령줄 인자가 있으면 그것을 사용하고, 없으면 설정 파일의 값을 사용
    QString ipAddress = parser.isSet(ipOption) ? parser.value(ipOption)
                                               : settings.getIPAddress();

    QString parFile = parser.isSet(parOption) ? parser.value(parOption)
                                              : settings.getParameterFile();

    QString dataDirectory = settings.getDataDirectory();

    // 새로운 값이 있으면 설정 파일 업데이트
    if (parser.isSet(ipOption)) {
        settings.setIPAddress(ipAddress);
    }
    if (parser.isSet(parOption)) {
        settings.setParameterFile(parFile);
    }

    // 설정 저장
    settings.saveSettings();

    QCAENV2740Single qcaenv2740;
    if (!ipAddress.isEmpty()) {
        qcaenv2740.setIPAddress(ipAddress);
        qcaenv2740.connectDAQ();
        if (!parFile.isEmpty()) qcaenv2740.loadParameterFromFile(parFile);
        qcaenv2740.setDataDirectory(dataDirectory);
    }
    qcaenv2740.show();
    return app.exec();
}