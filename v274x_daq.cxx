// main.c
#include <stdio.h>

#include "QCAENV2740.hxx"  // QCAENV2740 헤더 파일 포함
#include "QtWidgets/QApplication.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);  // QApplication 객체 생성

    QCAENV2740 qcaenv2740;
    qcaenv2740.show();
    return app.exec();
}