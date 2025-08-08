#ifndef RAWDATAHEADER_HXX
#define RAWDATAHEADER_HXX

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <string>

////////////////////////////////////////////////////////////
// RawDataHeader
////////////////////////////////////////////////////////////
class RawDataHeader {
   public:
    RawDataHeader();
    RawDataHeader(const char *runName, uint64_t runNumber, const QDateTime &startTime, const char *comment);
    RawDataHeader(const QString &runName, uint64_t runNumber, const QDateTime &startTime, const QString &comment);
    RawDataHeader(const uint64_t *data);
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

    void getHeaderInfo(QTextStream &stream) const;

   private:
    uint64_t header =
        0x4000000000000008;  // 8 bytes (0x4 for format, 0x8 for header size in words)
    QString runName;         // within 8 bytes -> 8 bytes when serialized (1 word)
    uint64_t runNumber;      // 8 bytes (1 word)
    uint64_t startTime;      // unix timestamp, 8 bytes (1 word)
    QString comment;         // within 40 bytes -> 40 bytes when serialized (5 words)
};

#endif