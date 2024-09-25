#include "QBufferedFileWriter.hxx"

////////////////////////////////////////////////////////////
// WriteThread
////////////////////////////////////////////////////////////
WriteThread::WriteThread(QBufferedFileWriter *writer) : writer(writer), running(false) {}
void WriteThread::run() {
    running = true;
    while (running) {
        QThread::sleep(1);  // 주기적으로 실행
        writer->flush();
    }
}
void WriteThread::stop() { running = false; }
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// QBufferedFileWriter
////////////////////////////////////////////////////////////
QBufferedFileWriter::QBufferedFileWriter(QObject *parent) : QObject(parent) { writeThread = new WriteThread(this); }

QBufferedFileWriter::~QBufferedFileWriter() {
    for (const auto &buffer : buffers) {
        buffer->close();
        delete buffer;
    }
    for (const auto &file : files) {
        file->close();
        delete file;
    }
    for (const auto &lock : locks) {
        delete lock;
    }
    for (const auto &stream : streams) {
        delete stream;
    }
}
void QBufferedFileWriter::addBuffer(const QString &bufferName) { addBuffer(bufferName, bufferName + ".dat"); }
void QBufferedFileWriter::addBuffer(const QString &bufferName, const QString &fileName) {
    if (bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;
    bufferNames.push_back(bufferName);
    files[bufferName] = new QFile(fileName);
    buffers[bufferName] = new QBuffer();
    streams[bufferName] = new QDataStream(buffers[bufferName]);
    locks[bufferName] = new QReadWriteLock();
    files[bufferName]->open(QIODevice::WriteOnly);
    buffers[bufferName]->open(QIODevice::ReadWrite);
}

void QBufferedFileWriter::removeBuffer(const QString &bufferName) {
    if (!bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;
    files[bufferName]->close();
    buffers[bufferName]->close();
    delete files[bufferName];
    delete buffers[bufferName];
    delete streams[bufferName];
    delete locks[bufferName];
    files.remove(bufferName);
    buffers.remove(bufferName);
    streams.remove(bufferName);
    locks.remove(bufferName);
    bufferNames.removeAll(bufferName);
}

void QBufferedFileWriter::setFileName(const QString &bufferName, const QString &fileName) {
    if (!bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;

    if (files[bufferName]->isOpen()) files[bufferName]->close();
    if (files[bufferName]->size() == 0) files[bufferName]->remove();  // 파일 크기가 0이면 파일 삭제
    files[bufferName]->setFileName(fileName);
    files[bufferName]->open(QIODevice::WriteOnly);
    clear(bufferName);
}

QDataStream &QBufferedFileWriter::stream(size_t index) { return *streams[bufferNames[index]]; }

QDataStream &QBufferedFileWriter::stream(const QString &bufferName) { return *streams[bufferName]; }

const QByteArray &QBufferedFileWriter::buffer(size_t index) const { return buffers[bufferNames[index]]->data(); }

const QByteArray &QBufferedFileWriter::buffer(const QString &bufferName) const { return buffers[bufferName]->data(); }

void QBufferedFileWriter::clear() {
    for (const auto &bufferName : bufferNames) {
        QWriteLocker locker(locks[bufferName]);
        buffers[bufferName]->reset();
        buffers[bufferName]->buffer().clear();
    }
}

void QBufferedFileWriter::clear(const QString &bufferName) {
    if (buffers.contains(bufferName)) {
        QWriteLocker locker(locks[bufferName]);
        buffers[bufferName]->reset();
        buffers[bufferName]->buffer().clear();
    }
}

void QBufferedFileWriter::write(const QString &bufferName, const QByteArray &data) {
    if (buffers.contains(bufferName)) {
        QWriteLocker locker(locks[bufferName]);
        buffers[bufferName]->write(data);
    }
}

void QBufferedFileWriter::write(const QString &bufferName, const char *data, size_t size) {
    if (buffers.contains(bufferName)) {
        QWriteLocker locker(locks[bufferName]);
        buffers[bufferName]->write(data, size);
    }
}

void QBufferedFileWriter::flush() {
    for (const auto &bufferName : bufferNames) {
        QReadLocker locker(locks[bufferName]);
        if (buffers[bufferName]->buffer().isEmpty()) continue;
        files[bufferName]->write(buffers[bufferName]->data());
    }
    clear();  // due to the lock, clear() should be called here
}

void QBufferedFileWriter::flush(const QString &bufferName) {
    if (buffers.contains(bufferName)) {
        QReadLocker locker(locks[bufferName]);
        if (buffers[bufferName]->buffer().isEmpty()) return;
        files[bufferName]->write(buffers[bufferName]->data());
    }
    clear(bufferName);  // due to the lock, clear() should be called here
}

void QBufferedFileWriter::start(bool flagClear) {
    if (flagClear) clear();
    writeThread->start();
}

void QBufferedFileWriter::stop() {
    writeThread->stop();
    writeThread->wait();
}