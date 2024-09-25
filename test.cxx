#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#include "QBufferedFileWriter.hxx"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QBufferedFileWriter writer;

    // 버퍼 추가
    writer.addBuffer("buffer1", "file1.dat");
    writer.addBuffer("buffer2", "file2.dat");

    writer.clear();

    // 스트림에 데이터 쓰기
    QDataStream &stream1 = writer.stream("buffer1");
    stream1 << static_cast<char>(1);

    QDataStream &stream2 = writer.stream("buffer2");
    stream2 << static_cast<bool>(1);
    stream2 << static_cast<bool>(0);
    //  stream2 << QByteArray::fromHex("05");

    // 버퍼 내용 확인
    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    stream2 << static_cast<char>(2);

    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();
    // 버퍼 크기 확인
    qDebug() << "Buffer1 size:" << writer.buffer("buffer1").size();
    qDebug() << "Buffer2 size:" << writer.buffer("buffer2").size();

    // 버퍼 지우기
    writer.clear("buffer1");
    writer.clear("buffer2");

    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    // 버퍼 크기 확인
    qDebug() << "Buffer1 size after clear:" << writer.buffer("buffer1").size();
    qDebug() << "Buffer2 size after clear:" << writer.buffer("buffer2").size();

    stream1 << static_cast<char>(3);
    stream2 << static_cast<char>(4);

    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    // writer.flush("buffer1");
    // writer.flush("buffer2");

    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    writer.write("buffer1", "data1");
    writer.write("buffer2", "data2");
    writer.write("buffer2", "data3");
    writer.write("buffer2", "data4");

    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    // writer.flush();

    qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    // 버퍼 제거
    //  writer.removeBuffer("buffer1");

    // qDebug() << "Buffer1 content:" << writer.buffer("buffer1").toHex();
    qDebug() << "Buffer2 content:" << writer.buffer("buffer2").toHex();

    writer.start();
    for (int i = 0; i < 1000; i++) {
        writer.write("buffer2", QString("data%1").arg(i).toUtf8());
        QThread::msleep(1);
        writer.write("buffer1", QString("data%1").arg(i).toUtf8());
        QThread::msleep(1);
    }
    writer.stop();
    return 0;
}
