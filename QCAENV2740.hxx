#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "CAENV2740.hxx"  // CAENV2740 클래스 포함
#include "CAENV2740Par.hxx"
#include "QtWidgets/QLineEdit"
#include "QtWidgets/QMainWindow"
#include "QtWidgets/QPushButton"
#include "QtWidgets/QTreeWidget"

class QCAENV2740 : public QMainWindow {
    // Q_OBJECT

   public:
    QCAENV2740(QWidget *parent = nullptr);
    virtual ~QCAENV2740();  // 가상 소멸자 추가

   private:
    CAENV2740 *daq;     // CAENV2740 객체
    CAENV2740Par *par;  // CAENV2740 Parameter 객체

    void initUI();
    void initDAQ();

    void connectDAQ();
    void clearDAQ();
    void resetDAQ();
    void rebootDAQ();
    void disconnectDAQ();
    void exitDAQ();

    void loadParameter();
    void viewParameter();
    void applyParameter();

    void loadYamlToTreeWidget(ryml::ConstNodeRef rootNode, QTreeWidget *treeWidget);

    // 위젯
    QLineEdit *ipLineEdit;
    QLineEdit *parameterLineEdit;
};

#endif  // MAINWINDOW_H