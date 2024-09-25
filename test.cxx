#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QThread>

#include "QBufferedFileWriter.hxx"

QBuffer *buffer1 = new QBuffer();
QBuffer *buffer2 = new QBuffer();

void test(bool isBuffer1) {
    if (isBuffer1) {
        buffer1->open(QIODevice::ReadWrite);
        for (int i = 0; i < 1000; i++) {
            buffer1->write(QByteArray("test1"));
            qDebug() << buffer1->data();
            QThread::msleep(100);
        }
        buffer1->close();
    } else {
        buffer2->open(QIODevice::ReadWrite);
        for (int i = 0; i < 1000; i++) {
            buffer2->write(QByteArray("test2"));
            qDebug() << buffer2->data();
            QThread::msleep(100);
        }
        buffer2->close();
    }
}
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QBufferedFileWriter *writer = QBufferedFileWriter::getInstance();
    writer->addBuffer("buffer1");
    writer->addBuffer("buffer2");
    writer->setSingleFileMode(true, "test.txt");
    writer->start();

    for (int i = 0; i < 100; i++) {
        writer->write("buffer1", QString("b1 %1\n").arg(i).toUtf8());
        writer->write("buffer2", QString("b2 %1\n").arg(i).toUtf8());
        qDebug() << "write" << i;
        QThread::msleep(10);
    }
    writer->stop();
    return 0;
}
