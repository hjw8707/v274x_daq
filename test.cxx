#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include "QBufferedFileWriter.hxx"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QBufferedFileWriter *writer = QBufferedFileWriter::getInstance();

    // 버퍼 추가
    writer->addBuffer("buffer1", "file1.dat");
    writer->addBuffer("buffer2", "file2.dat");

    // 파일 이름 설정
    writer->setFileName("buffer1", "new_file1.dat");
    writer->setFileName("buffer2", "new_file2.dat");

    // 단일 파일 모드 설정
    writer->setSingleFileMode(true, "single_file.dat");

    // 공유 메모리 저장 설정
    writer->setShmSave(true);

    // 데이터 쓰기
    writer->write("buffer1", "Hello, Buffer 1!", 16);
    writer->write("buffer2", "Hello, Buffer 2!", 16);

    // 플러시
    writer->flush();

    // 시작
    writer->start();

    // 잠시 대기
    // 데이터 쓰기
    writer->write("buffer1", "Hello, Buffer 3!");
    writer->write("buffer2", "Hello, Buffer 4!");
    QThread::sleep(2);

    // 정지
    writer->stop();

    QThread::sleep(10);

    // 버퍼 제거
    writer->removeBuffer("buffer1");
    writer->removeBuffer("buffer2");

    return 0;
}
