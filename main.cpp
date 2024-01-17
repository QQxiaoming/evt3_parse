#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QMessageBox>

#include "QGoodWindow"
#include "QGoodCentralWidget"
#include "mainwindow.h"

//./test_event_camif_core -idx 1 -ip 169.254.34.189 -trig_time 100 -lut_time 1000 -tcp
//./test_event_camif_core -idx 1 -ip 169.254.34.189 -tcp -allways_on

int main(int argc, char *argv[])
{
    QGoodWindow::setup();
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);

    QGoodWindow::setAppDarkTheme();
 
    MainWindow window;
    window.show();

    return app.exec();
}
