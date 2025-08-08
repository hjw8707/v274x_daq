#include "RawDataHeader.hxx"

#include <QtCore/QDataStream>
#include <QtCore/QDebug>
#include <QtCore/QIODevice>

RawDataHeader::RawDataHeader() : runName(""), runNumber(0), startTime(0), comment("") {}
RawDataHeader::RawDataHeader(const char *runName, uint64_t runNumber, const QDateTime &startTime, const char *comment)
    : runName(runName), runNumber(runNumber), startTime(startTime.toSecsSinceEpoch()), comment(comment) {}
RawDataHeader::RawDataHeader(const QString &runName, uint64_t runNumber, const QDateTime &startTime,
                             const QString &comment)
    : runName(runName), runNumber(runNumber), startTime(startTime.toSecsSinceEpoch()), comment(comment) {}
RawDataHeader::RawDataHeader(const uint64_t *data) {
    runName = QString::fromLatin1(reinterpret_cast<const char *>(data), 8).trimmed();
    runNumber = data[1];
    startTime = data[2];
    // comment는 data[3]부터 data[7]까지 5워드(40바이트)에서 읽어야 함
    comment = QString::fromLatin1(reinterpret_cast<const char *>(data + 3), 40).trimmed();
}
RawDataHeader::~RawDataHeader() {}

void RawDataHeader::setRunName(const QString &runName) { this->runName = runName; }
void RawDataHeader::setRunName(const char *runName) { this->runName = QString(runName); }
void RawDataHeader::setRunName(const std::string &runName) { this->runName = QString(runName.c_str()); }
void RawDataHeader::setRunNumber(uint64_t runNumber) { this->runNumber = runNumber; }
void RawDataHeader::setStartTime(uint64_t startTime) { this->startTime = startTime; }
void RawDataHeader::setStartTime(const QDateTime &startTime) { this->startTime = startTime.toSecsSinceEpoch(); }
void RawDataHeader::setComment(const QString &comment) { this->comment = comment; }
void RawDataHeader::setComment(const char *comment) { this->comment = QString(comment); }
void RawDataHeader::setComment(const std::string &comment) { this->comment = QString(comment.c_str()); }

QString RawDataHeader::getRunName() const { return runName; }
uint64_t RawDataHeader::getRunNumber() const { return runNumber; }
uint64_t RawDataHeader::getStartTime() const { return startTime; }
QString RawDataHeader::getComment() const { return comment; }

QByteArray RawDataHeader::toByteArray() const {
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << quint64(header); // endian change (automatic)
    ////////////////////////////////////////////////////////////////////////////////
    // No endian change below this line
    QByteArray nameBytes = runName.toLatin1().leftJustified(8, '\0', true);
    stream.writeRawData(nameBytes.constData(), 8); // 정확히 8바이트만 쓰기
    stream.writeRawData(reinterpret_cast<const char *>(&runNumber), 8);
    stream.writeRawData(reinterpret_cast<const char *>(&startTime), 8);
    QByteArray commentBytes = comment.toLatin1().leftJustified(40, '\0', true);
    stream.writeRawData(commentBytes.constData(), 40); // 정확히 40바이트만 쓰기
    ////////////////////////////////////////////////////////////////////////////////
    qDebug() << "RawDataHeader::toByteArray():" << bytes.size();
    QByteArray hex = bytes.toHex();
    qDebug() << "RawDataHeader::toByteArray():";
    for (int i = 0; i < hex.size(); i += 16) { // 8바이트 = 16 hex 문자
        qDebug() << hex.mid(i, 16);
    }
    return bytes;
}

void RawDataHeader::getHeaderInfo(QTextStream &stream) const {
    stream << "================================================" << "\n";
    stream << "RawDataHeader" << "\n";
    stream << "================================================" << "\n";
    stream << "Run Name: " << runName << "\n";
    stream << "Run Number: " << runNumber << "\n";
    stream << "Start Time: " << QDateTime::fromSecsSinceEpoch(startTime).toString("yyyy-MM-dd hh:mm:ss") << "\n";
    stream << "Comment: " << comment << "\n";
    stream << "================================================" << "\n";
}