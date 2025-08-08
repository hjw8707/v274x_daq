#ifndef QBUFFEREDFILEWRITER_HXX
#define QBUFFEREDFILEWRITER_HXX

// #define QSHM_SIZE 2621448  // 2.5 MB + 8 bytes (uint64_t)
#if defined(__APPLE__)
#define QSHM_SIZE (512 * 1024 + 8)  // 512 kB + 8 bytes (uint64_t)
#else
#define QSHM_SIZE (4 * 1024 * 1024 + 8)  // 4 MB + 8 bytes (uint64_t)
#endif

#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSharedMemory>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <iostream>
#include <string>
#include <vector>

class QBufferedFileWriter;

////////////////////////////////////////////////////////////
// RawDataHeader
////////////////////////////////////////////////////////////
class RawDataHeader {
   public:
    RawDataHeader();
    RawDataHeader(const char *runName, uint64_t runNumber, const QDateTime &startTime, const char *comment);
    RawDataHeader(const QString &runName, uint64_t runNumber, const QDateTime &startTime, const QString &comment);
    ~RawDataHeader();

    void setRunName(const QString &runName);
    void setRunName(const std::string &runName);
    void setRunName(const char *runName);
    void setRunNumber(uint64_t runNumber);
    void setStartTime(uint64_t startTime);
    void setStartTime(const QDateTime &startTime);
    void setComment(const QString &comment);
    void setComment(const std::string &comment);
    void setComment(const char *comment);

    QString getRunName() const;
    uint64_t getRunNumber() const;
    uint64_t getStartTime() const;
    QString getComment() const;

    QByteArray toByteArray() const;

   private:
    uint64_t header =
        0x4000000000000008;  // 8 bytes (0x4000000000000008) (0x4 for format, 0x8 for header size in words)
    QString runName;         // within 8 bytes -> 8 bytes when serialized (1 word)
    uint64_t runNumber;      // 8 bytes (1 word)
    uint64_t startTime;      // unix timestamp, 8 bytes (1 word)
    QString comment;         // within 40 bytes -> 40 bytes when serialized (5 words)
};

////////////////////////////////////////////////////////////
// RawDataEnder
////////////////////////////////////////////////////////////
class RawDataEnder {
   public:
    RawDataEnder();
    RawDataEnder(const QDateTime &endTime, const char *comment);
    RawDataEnder(const QDateTime &endTime, const QString &comment);
    ~RawDataEnder();

    void setEndTime(uint64_t endTime);
    void setEndTime(const QDateTime &endTime);
    void setComment(const QString &comment);
    void setComment(const std::string &comment);
    void setComment(const char *comment);

    uint64_t getEndTime() const;
    QString getComment() const;

    QByteArray toByteArray() const;

   private:
    uint64_t header =
        0x5000000000000006;  // 8 bytes (0x5000000000000006) (0x5 for format, 0x6 for header size in words)
    uint64_t endTime;        // unix timestamp, 8 bytes (1 word)
    QString comment;         // within 40 bytes -> 40 bytes when serialized (5 words)
};
////////////////////////////////////////////////////////////
// WriteThread
////////////////////////////////////////////////////////////

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

    void setFileSave(bool flag) { fileSave = flag; }
    bool getFileSave() const { return fileSave; }

    ///////////////////////////////////////////////////////////////
    //! Open & Close file manually unless using start & stop functions
    ///////////////////////////////////////////////////////////////
    void openFile(const QString &bufferName);
    void openFile();
    void closeFile(const QString &bufferName);
    void closeFile();
    ///////////////////////////////////////////////////////////////

    void setFileName(const QString &bufferName, const QString &fileName);
    void setSingleFileMode(bool flag, const QString &fileName = "");

    void setShmSave(bool flag);
    bool getShmSave() const { return shmSave; }

    void attachShm(const QString &bufferName);
    void detachShm(const QString &bufferName);
    void checkShm(const QString &bufferName);

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
    void writeToAllBuffers(const QByteArray &data);
    void writeToAllBuffers(const char *data, size_t size);
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

    bool fileSave;

    QList<QString> bufferNames;
    QHash<QString, QBuffer *> buffers;
    QHash<QString, QDataStream *> streams;
    QHash<QString, QFile *> files;
    QHash<QString, QSharedMemory *> sharedMemory;
    QHash<QString, QReadWriteLock *> locks;
    WriteThread *writeThread;
};

#endif  // QBUFFEREDFILEWRITER_HXX
