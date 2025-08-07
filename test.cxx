#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QIODevice>

int main() {
    QDateTime start = QDateTime::currentDateTime();
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << start;

    qDebug() << "시작 시간:" << start;
    qDebug() << "바이트 크기:" << bytes.size();

    QDateTime restoredDateTime;
    QDataStream readStream(&bytes, QIODevice::ReadOnly);
    readStream >> restoredDateTime;

    qDebug() << "복원된 시작 시간:" << restoredDateTime;
    return 0;
}