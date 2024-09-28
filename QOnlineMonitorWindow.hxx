#ifndef QONLINEMONITORWINDOW_HXX
#define QONLINEMONITORWINDOW_HXX

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>

#include "QOnlineMonitor.hxx"

class QOnlineMonitorWindow : public QMainWindow {
    Q_OBJECT

   public:
    QOnlineMonitorWindow(QWidget* parent = nullptr);
    ~QOnlineMonitorWindow();

    QOnlineMonitor* getOnlineMonitor() { return onlineMonitor; }

    void start();
    void stop();

    void clearHistogram();

   private:
    inline void initUI();

   private:
    QOnlineMonitor* onlineMonitor;
    QLineEdit* shmNameLineEdit;
};

#endif
