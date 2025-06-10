#ifndef QONLINEMONITORWINDOW_HXX
#define QONLINEMONITORWINDOW_HXX

#include <QtCore/QDebug>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>

#include "QOnlineMonitor.hxx"

class QOnlineMonitorWindow : public QMainWindow {
    Q_OBJECT

   public:
    QOnlineMonitorWindow(QWidget* parent = nullptr);
    ~QOnlineMonitorWindow();

    QOnlineMonitor* getOnlineMonitor() { return onlineMonitor; }
    void addShm();
    void removeShm();
    void addShmFromName(const QString& shmName);

    void start();
    void stop();

    void clearHistogram();

   private:
    inline void initUI();

   private:
    QOnlineMonitor* onlineMonitor;
    QLineEdit* shmNameLineEdit;
    QComboBox* shmListComboBox;

    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* exitButton;

    QPushButton* addShmButton;
    QPushButton* removeShmButton;
};

#endif
