#include <QtCore/QBuffer>
#include <QtCore/QDataStream>
#include <QtCore/QDebug>
#include <QtCore/QSharedMemory>
#include <QtCore/QThread>

class SharedMemoryBuffer {
   public:
    SharedMemoryBuffer(const QString &key) : sharedMemory(key) {
        if (!sharedMemory.create(1024)) {
            qDebug() << "Unable to create shared memory segment.";
        }
        sharedMemory.attach();
    }
    ~SharedMemoryBuffer() { sharedMemory.detach(); }

    bool writeToBuffer(const QByteArray &data) {
        qDebug() << "Writing to shared memory";
        if (!sharedMemory.isAttached()) {
            if (!sharedMemory.attach()) {
                qDebug() << "Unable to attach to shared memory segment.";
                return false;
            }
        }
        int size = qMin(sharedMemory.size(), data.size());
        qDebug() << "Size:" << size;
        sharedMemory.lock();
        char *to = (char *)sharedMemory.data();
        const char *from = data.data();
        memcpy(to, from, size);
        sharedMemory.unlock();

        return true;
    }

    QByteArray readFromBuffer() {
        qDebug() << "Reading from shared memory";
        if (!sharedMemory.isAttached()) {
            if (!sharedMemory.attach()) {
                qDebug() << "Unable to attach to shared memory segment.";
                return QByteArray();
            }
        }
        QBuffer buffer;
        sharedMemory.lock();
        buffer.setData((char *)sharedMemory.constData(), sharedMemory.size());
        buffer.open(QBuffer::ReadOnly);

        sharedMemory.unlock();

        return QByteArray(buffer.data());
    }

   private:
    QSharedMemory sharedMemory;
};

// Usage example
int main() {
    SharedMemoryBuffer buffer("SharedMemoryKey");

    QByteArray dataRead = buffer.readFromBuffer();
    qDebug() << "Data read from shared memory:" << dataRead;

    return 0;
}