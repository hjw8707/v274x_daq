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
QBufferedFileWriter::QBufferedFileWriter(QObject *parent)
    : QObject(parent), singleFileMode(false), singleFile(nullptr), shmSave(false), fileSave(true) {
    writeThread = new WriteThread(this);

    qDebug() << "QBufferedFileWriter created";
}

QBufferedFileWriter::~QBufferedFileWriter() {
    qDebug() << "QBufferedFileWriter destroyed";

    for (const auto &buffer : buffers) {
        buffer->close();
        delete buffer;
    }
    for (const auto &file : files) {
        file->close();
        delete file;
    }
    for (const auto &lock : locks) delete lock;

    for (const auto &sharedMemory : sharedMemory) {
        if (sharedMemory->isAttached()) sharedMemory->detach();
        delete sharedMemory;
    }
    for (const auto &stream : streams) delete stream;

    if (singleFile && singleFile->isOpen()) singleFile->close();
    delete singleFile;
}
void QBufferedFileWriter::addBuffer(const QString &bufferName) { addBuffer(bufferName, bufferName + ".dat"); }
void QBufferedFileWriter::addBuffer(const QString &bufferName, const QString &fileName) {
    qDebug() << "QBufferedFileWriter::addBuffer():" << bufferName << fileName;
    if (bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;
    bufferNames.push_back(bufferName);
    files[bufferName] = new QFile(fileName);
    buffers[bufferName] = new QBuffer();
    streams[bufferName] = new QDataStream(buffers[bufferName]);
    sharedMemory[bufferName] = new QSharedMemory("shm_" + bufferName);
    if (shmSave) attachShm(bufferName);
    locks[bufferName] = new QReadWriteLock();
    files[bufferName]->open(QIODevice::WriteOnly);
    buffers[bufferName]->open(QIODevice::ReadWrite);
}

void QBufferedFileWriter::removeBuffer(const QString &bufferName) {
    qDebug() << "QBufferedFileWriter::removeBuffer():" << bufferName;
    if (!bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;
    files[bufferName]->close();
    buffers[bufferName]->close();
    delete files[bufferName];
    delete buffers[bufferName];
    if (sharedMemory[bufferName]->isAttached()) sharedMemory[bufferName]->detach();
    delete sharedMemory[bufferName];
    delete streams[bufferName];
    delete locks[bufferName];
    files.remove(bufferName);
    buffers.remove(bufferName);
    sharedMemory.remove(bufferName);
    streams.remove(bufferName);
    locks.remove(bufferName);
    bufferNames.removeAll(bufferName);
}

void QBufferedFileWriter::openFile(const QString &bufferName) {
    qDebug() << "QBufferedFileWriter::openFile():" << bufferName;
    if (!bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;
    files[bufferName]->open(QIODevice::WriteOnly);
}

void QBufferedFileWriter::openFile() {
    qDebug() << "QBufferedFileWriter::openFile()";
    if (singleFileMode)
        singleFile->open(QIODevice::WriteOnly);
    else
        for (const auto &bufferName : bufferNames) openFile(bufferName);
}

void QBufferedFileWriter::closeFile(const QString &bufferName) {
    qDebug() << "QBufferedFileWriter::closeFile():" << bufferName;
    if (!bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;
    files[bufferName]->close();
}

void QBufferedFileWriter::closeFile() {
    qDebug() << "QBufferedFileWriter::closeFile()";
    if (singleFileMode)
        singleFile->close();
    else
        for (const auto &bufferName : bufferNames) closeFile(bufferName);
}

void QBufferedFileWriter::setFileName(const QString &bufferName, const QString &fileName) {
    qDebug() << "QBufferedFileWriter::setFileName():" << bufferName << "->" << fileName;
    if (!bufferNames.contains(bufferName)) return;
    if (writeThread->isRunning()) return;

    if (files[bufferName]->isOpen()) files[bufferName]->close();
    if (files[bufferName]->size() == 0) files[bufferName]->remove();  // 파일 크기가 0이면 파일 삭제
    files[bufferName]->setFileName(fileName);
    files[bufferName]->open(QIODevice::WriteOnly);
    clear(bufferName);
}

void QBufferedFileWriter::setSingleFileMode(bool flag, const QString &fileName) {
    qDebug() << "QBufferedFileWriter::setSingleFileMode():" << (flag ? "ON" : "OFF") << "->" << fileName;
    if (flag) {
        if (fileName.isEmpty()) return;
        singleFileName = fileName;
        singleFile = new QFile(singleFileName);
    } else {
        if (singleFile->isOpen()) singleFile->close();
        delete singleFile;
        singleFile = nullptr;
        singleFileName.clear();
    }
    singleFileMode = flag;
}

uint64_t QBufferedFileWriter::getFileSize(const QString &fileName) const {
    if (files.contains(fileName)) return files[fileName]->size();
    return 0;
}

void QBufferedFileWriter::attachShm(const QString &bufferName) {
    qDebug() << "QBufferedFileWriter::attachShm():" << bufferName;
    if (sharedMemory[bufferName]->isAttached()) return;
    if (!sharedMemory[bufferName]->create(QSHM_SIZE)) {
        if (sharedMemory[bufferName]->error() == QSharedMemory::AlreadyExists) {
            qDebug() << "QBufferedFileWriter::attachShm(): already exists";
            sharedMemory[bufferName]->attach();
        } else {
            qCritical() << "Unable to create or attach to shared memory segment: "
                        << sharedMemory[bufferName]->errorString();
            throw std::runtime_error("Unable to create or attach to shared memory segment: " +
                                     sharedMemory[bufferName]->errorString().toStdString());
        }
    }  // SHM created
    qDebug() << "QBufferedFileWriter::attachShm(): created with SHM_SIZE" << QSHM_SIZE;
    uint64_t size = 0;
    memcpy(sharedMemory[bufferName]->data(), &size, sizeof(uint64_t));  // write size = 0
}

void QBufferedFileWriter::detachShm(const QString &bufferName) {
    qDebug() << "QBufferedFileWriter::detachShm():" << bufferName;
    if (sharedMemory[bufferName]->isAttached()) sharedMemory[bufferName]->detach();
}
void QBufferedFileWriter::setShmSave(bool flag) {
    qDebug() << "QBufferedFileWriter::setShmSave():" << (flag ? "ON" : "OFF");
    qDebug() << "QBufferedFileWriter::setShmSave(): SHM_SIZE" << QSHM_SIZE;
    for (const auto &bufferName : bufferNames) {
        flag ? attachShm(bufferName) : detachShm(bufferName);
    }
    shmSave = flag;
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
    QWriteLocker locker(locks[bufferName]);
    buffers[bufferName]->reset();
    buffers[bufferName]->buffer().clear();
}

void QBufferedFileWriter::write(const QString &bufferName, const QByteArray &data) {
    QWriteLocker locker(locks[bufferName]);
    buffers[bufferName]->write(data);
}

void QBufferedFileWriter::write(const QString &bufferName, const char *data, size_t size) {
    QWriteLocker locker(locks[bufferName]);
    buffers[bufferName]->write(data, size);
}

void QBufferedFileWriter::flush() {
    for (const auto &bufferName : bufferNames) flush(bufferName);
}

void QBufferedFileWriter::flush(const QString &bufferName) {
    {  // write to file
        QReadLocker locker(locks[bufferName]);
        if (buffers[bufferName]->buffer().isEmpty()) return;
        if (fileSave) {
            if (singleFileMode)
                singleFile->write(buffers[bufferName]->data());
            else
                files[bufferName]->write(buffers[bufferName]->data());
        }
        // write to shared memory
        if (shmSave) {
            sharedMemory[bufferName]->lock();
            uint64_t shmDataSize;
            memcpy(&shmDataSize, reinterpret_cast<uint64_t *>(sharedMemory[bufferName]->data()),
                   sizeof(uint64_t));  // 먼저 SHM 크기를 읽어옴
            uint64_t newDataSize = shmDataSize + buffers[bufferName]->buffer().size();
            // newDataSize > SHM_SIZE: Clear SHM
            if (newDataSize > QSHM_SIZE) newDataSize = buffers[bufferName]->buffer().size();
            // Write size first to the SHM (first 8 bytes)
            memcpy(reinterpret_cast<uint64_t *>(sharedMemory[bufferName]->data()), &newDataSize, sizeof(uint64_t));
            // Write data to the SHM
            memcpy(reinterpret_cast<char *>(sharedMemory[bufferName]->data()) + sizeof(uint64_t) + shmDataSize,
                   buffers[bufferName]->data().data(), buffers[bufferName]->buffer().size());
            sharedMemory[bufferName]->unlock();
        }
    }
    clear(bufferName);  // due to the lock, clear() should be called here
}

void QBufferedFileWriter::start(bool flagClear) {
    qDebug() << "QBufferedFileWriter::start()";
    if (fileSave) openFile();
    if (flagClear) clear();
    writeThread->start();
}

void QBufferedFileWriter::stop() {
    qDebug() << "QBufferedFileWriter::stop()";
    writeThread->stop();
    writeThread->wait();
    if (fileSave) closeFile();
}
