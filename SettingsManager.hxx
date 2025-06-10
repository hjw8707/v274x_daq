// SettingsManager.hxx
#pragma once
#include <QtCore/QFile>
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
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            return false;
        }

        m_settings = doc.object();
        return true;
    }

    bool saveSettings(const QString& filename = "settings.json") {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        QJsonDocument doc(m_settings);
        file.write(doc.toJson());
        return true;
    }

    QString getIPAddress() const { return m_settings["ip_address"].toString(); }

    QString getParameterFile() const {
        return m_settings["parameter_file"].toString();
    }

    QString getDataDirectory() const {
        return m_settings["data_directory"].toString();
    }

    void setIPAddress(const QString& ip) { m_settings["ip_address"] = ip; }

    void setParameterFile(const QString& file) {
        m_settings["parameter_file"] = file;
    }

    void setDataDirectory(const QString& dataDirectory) {
        m_settings["data_directory"] = dataDirectory;
    }

   private:
    SettingsManager() {
        // 기본값 설정
        m_settings["ip_address"] = "192.168.1.100";
        m_settings["parameter_file"] = "default.par";
        m_settings["data_directory"] = "./";
    }
    QJsonObject m_settings;
};