#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QSplitter>

#include "QGoodWindow"
#include "QGoodCentralWidget"
#include "eventsensorwidget.h"

class MainWindow : public QGoodWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    QGoodCentralWidget *m_good_central_widget;
    QMenuBar *m_menu_bar = nullptr;
    EventSensorWidget *widet_l;
    EventSensorWidget *widet_r;
    QWidget *m_central_widget;
    QVBoxLayout *layout;
    QSplitter *splitter;
};

#endif // MAINWINDOW_H
