#include "RawDataEnder.hxx"

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

RawDataEnder::RawDataEnder() : endTime(0), comment("") {}
RawDataEnder::RawDataEnder(const QDateTime &endTime, const char *comment)
    : endTime(endTime.toSecsSinceEpoch()), comment(comment) {}
RawDataEnder::RawDataEnder(const QDateTime &endTime, const QString &comment)
    : endTime(endTime.toSecsSinceEpoch()), comment(comment) {}
RawDataEnder::RawDataEnder(const uint64_t *data) {
    endTime = data[0];
    comment = QString::fromLatin1(reinterpret_cast<const char *>(data + 1), 40).trimmed();
}
RawDataEnder::~RawDataEnder() {}

void RawDataEnder::setEndTime(uint64_t endTime) { this->endTime = endTime; }
void RawDataEnder::setEndTime(const QDateTime &endTime) { this->endTime = endTime.toSecsSinceEpoch(); }
void RawDataEnder::setComment(const QString &comment) { this->comment = comment; }
void RawDataEnder::setComment(const std::string &comment) { this->comment = QString(comment.c_str()); }
void RawDataEnder::setComment(const char *comment) { this->comment = QString(comment); }

uint64_t RawDataEnder::getEndTime() const { return endTime; }
QString RawDataEnder::getComment() const { return comment; }
QByteArray RawDataEnder::toByteArray() const {
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << quint64(header); // endian change (automatic)
    ////////////////////////////////////////////////////////////////////////////////
    // No endian change below this line
    stream.writeRawData(reinterpret_cast<const char *>(&endTime), 8);
    QByteArray commentBytes = comment.toLatin1().leftJustified(40, '\0', true);
    stream.writeRawData(commentBytes.constData(), 40);
    ////////////////////////////////////////////////////////////////////////////////
    qDebug() << "RawDataEnder::toByteArray():" << bytes.size();
    QByteArray hex = bytes.toHex();
    qDebug() << "RawDataHeader::toByteArray():";
    for (int i = 0; i < hex.size(); i += 16) { // 8바이트 = 16 hex 문자
        qDebug() << hex.mid(i, 16);
    }
    return bytes;
}

void RawDataEnder::getEnderInfo(QTextStream &stream) const {    
    stream << "================================================" << "\n";
    stream << "RawDataEnder" << "\n";
    stream << "================================================" << "\n";
    stream << "End Time: " << QDateTime::fromSecsSinceEpoch(endTime).toString("yyyy-MM-dd hh:mm:ss") << "\n";
    stream << "Comment: " << comment << "\n";
    stream << "================================================" << "\n";
}