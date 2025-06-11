// SettingsManager.hxx
#pragma once
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>

class SettingsManager {
   public:
    static SettingsManager& getInstance() {
        static SettingsManager instance;
        return instance;
    }

    bool loadSettings(const QString& filename = "settings.json") {
        qDebug() << "SettingsManager::loadSettings(" << filename << ")";
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "SettingsManager::loadSettings(" << filename << ") failed to open file";
            return false;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qDebug() << "SettingsManager::loadSettings(" << filename << ") failed to parse JSON";
            return false;
        }

        m_settings = doc.object();
        return true;
    }

    bool saveSettings(const QString& filename = "settings.json") {
        qDebug() << "SettingsManager::saveSettings(" << filename << ")";
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "SettingsManager::saveSettings(" << filename << ") failed to open file";
            return false;
        }

        QJsonDocument doc(m_settings);
        file.write(doc.toJson());
        return true;
    }

    QString getIPAddress() const { return m_settings["ip_address"].toString(); }

    QString getParameterFile() const { return m_settings["parameter_file"].toString(); }

    QString getDataDirectory() const { return m_settings["data_directory"].toString(); }

    QString getRunName() const { return m_settings["run_name"].toString(); }

    int getRunNumber() const {
        QJsonValue v = m_settings["run_number"];
        return v.isUndefined() ? -1 : v.toInt();
    }

    void setIPAddress(const QString& ip) { m_settings["ip_address"] = ip; }

    void setParameterFile(const QString& file) { m_settings["parameter_file"] = file; }

    void setDataDirectory(const QString& dataDirectory) { m_settings["data_directory"] = dataDirectory; }

    void setRunName(const QString& runName) { m_settings["run_name"] = runName; }

    void setRunNumber(const int& runNumber) { m_settings["run_number"] = runNumber; }

   private:
    SettingsManager() {
        // 기본값 설정
        m_settings["data_directory"] = "./";
        m_settings["run_name"] = "run";
        m_settings["run_number"] = 0;
        m_settings["ip_address"] = "192.168.1.100";
        m_settings["parameter_file"] = "default.par";
    }
    QJsonObject m_settings;
};