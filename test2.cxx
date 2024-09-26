#include <QtCore/QBuffer>
#include <QtCore/QDataStream>
#include <QtCore/QDebug>
#include <QtCore/QSharedMemory>
#include <QtCore/QThread>

// Usage example
int main() {
    QSharedMemory sharedMemory("shm_buffer1");
    if (!sharedMemory.attach()) {
        qCritical() << "Unable to attach to shared memory segment: " << sharedMemory.errorString();
        return -1;
    }

    uint64_t size = 0;
    memcpy(&size, sharedMemory.data(), sizeof(uint64_t));
    qDebug() << "size:" << size;

    QByteArray dataRead(reinterpret_cast<char*>(sharedMemory.data()) + sizeof(uint64_t), size);
    qDebug() << "dataRead:" << dataRead;

    sharedMemory.detach();
    return 0;
}