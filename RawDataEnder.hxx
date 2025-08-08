#ifndef RAWDATAENDER_HXX
#define RAWDATAENDER_HXX

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <iostream>
#include <string>

////////////////////////////////////////////////////////////
// RawDataEnder
////////////////////////////////////////////////////////////
class RawDataEnder {
   public:
    RawDataEnder();
    RawDataEnder(const QDateTime &endTime, const char *comment);
    RawDataEnder(const QDateTime &endTime, const QString &comment);
    RawDataEnder(const uint64_t *data);
    ~RawDataEnder();

    void setEndTime(uint64_t endTime);
    void setEndTime(const QDateTime &endTime);
    void setComment(const QString &comment);
    void setComment(const std::string &comment);
    void setComment(const char *comment);

    uint64_t getEndTime() const;
    QString getComment() const;

    QByteArray toByteArray() const;
    void getEnderInfo(QTextStream &stream) const;

   private:
    uint64_t header =
        0x5000000000000006;  // 8 bytes (0x5 for format, 0x6 for header size in words)
    uint64_t endTime;        // unix timestamp, 8 bytes (1 word)
    QString comment;         // within 40 bytes -> 40 bytes when serialized (5 words)
};

#endif