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

    QThread *thread1 = QThread::create([&]() { test(true); });
    QThread *thread2 = QThread::create([&]() { test(false); });
    thread1->start();
    thread2->start();

    thread1->wait();
    thread2->wait();
    return 0;
}
