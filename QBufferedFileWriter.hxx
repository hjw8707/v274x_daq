#ifndef QBUFFEREDFILEWRITER_HXX
#define QBUFFEREDFILEWRITER_HXX

#define QSHM_SIZE 1048576  // 1 MB

#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSharedMemory>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <iostream>
#include <vector>

class QBufferedFileWriter;

class WriteThread : public QThread {
    Q_OBJECT
   public:
    WriteThread(QBufferedFileWriter *writer);
    void run() override;
    void stop();

   private:
    QBufferedFileWriter *writer;
    bool running;
};

class QBufferedFileWriter : public QObject {
    Q_OBJECT

   public:
    static QBufferedFileWriter *getInstance() {
        static QBufferedFileWriter instance;
        return &instance;
    }

    QBufferedFileWriter(QBufferedFileWriter const &) = delete;
    void operator=(QBufferedFileWriter const &) = delete;

    void addBuffer(const QString &bufferName);
    void addBuffer(const QString &bufferName, const QString &fileName);
    void removeBuffer(const QString &bufferName);

    void setFileName(const QString &bufferName, const QString &fileName);
    void setSingleFileMode(bool flag, const QString &fileName = "");

    void setShmSave(bool flag);
    bool getShmSave() const { return shmSave; }

    uint64_t getFileSize(const QString &fileName) const;
    //////////////////////////////////////////////////////////
    // ! The functions below do not lock the buffer.
    // ! They can cause the problem in multi-thread environment.
    //////////////////////////////////////////////////////////
    QDataStream &stream(size_t index);
    QDataStream &stream(const QString &bufferName);

    const QByteArray &buffer(size_t index) const;
    const QByteArray &buffer(const QString &bufferName) const;
    //////////////////////////////////////////////////////////

   public slots:
    void clear();  // -1 means clear all
    void clear(const QString &bufferName);
    //////////////////////////////////////////////////////////////////////
    // ! The functions below lock the buffer.
    //////////////////////////////////////////////////////////////////////
    void write(const QString &bufferName, const QByteArray &data);
    void write(const QString &bufferName, const char *data, size_t size);
    void flush();
    void flush(const QString &bufferName);
    //////////////////////////////////////////////////////////////////////
    void start(bool flagClear = true);
    void stop();

   private:
    QBufferedFileWriter(QObject *parent = nullptr);
    ~QBufferedFileWriter();

    bool singleFileMode;
    QString singleFileName;
    QFile *singleFile;

    bool shmSave;

    QList<QString> bufferNames;
    QHash<QString, QBuffer *> buffers;
    QHash<QString, QDataStream *> streams;
    QHash<QString, QFile *> files;
    QHash<QString, QSharedMemory *> sharedMemory;
    QHash<QString, QReadWriteLock *> locks;
    WriteThread *writeThread;
};

#endif  // QBUFFEREDFILEWRITER_HXX
