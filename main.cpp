#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QMessageBox>

#include "eventsensorwidget.h"

//./test_event_camif_core -idx 1 -ip 169.254.34.189 -trig_time 100 -lut_time 1000 -tcp

#define IMG_WIDTH  (1280)
#define IMG_HEIGHT (720)
#define SEVER_PORT (13456)
#define CLIENT_PORT (15678)

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);

    QMainWindow window;
    QWidget widget(&window);
    QVBoxLayout layout(&widget);
    int trig_time = 100;
    bool ok;
    int val = QInputDialog::getInt(&window, "trig_time", "trig_time", trig_time, 0, 65536, 1, &ok);
    if(ok) {
        trig_time = val;
    }
    int lut_time = 1000;
    val = QInputDialog::getInt(&window, "lut_time", "lut_time", lut_time, 0, 65536, 1, &ok);
    if(ok) {
        lut_time = val;
    }
    EventSensorWidget widet_l(SEVER_PORT+0,trig_time,lut_time,&widget);
    EventSensorWidget widet_r(SEVER_PORT+1,trig_time,lut_time,&widget);
    layout.addWidget(&widet_l);
    layout.addWidget(&widet_r);
    layout.setSpacing(0);
    layout.setContentsMargins(0, 0, 0, 0);
    widget.setLayout(&layout);
    widget.setContentsMargins(0, 0, 0, 0);
    widget.setWindowTitle("evt3_parse");
    window.setCentralWidget(&widget);
    window.resize(IMG_WIDTH/2,IMG_HEIGHT);
    QMenuBar *menuBar = window.menuBar();
    QMenu * menuFile = menuBar->addMenu("File");
    QAction * rec = menuFile->addAction("rec");
    rec->setCheckable(true);
    rec->setChecked(widet_l.getRec());
    QObject::connect(rec, &QAction::triggered, [&](){
        widet_l.setRec(rec->isChecked());
        widet_r.setRec(rec->isChecked());
    });
    QMenu * menuOpt = menuBar->addMenu("Option");
    QAction * actionDiff = menuOpt->addAction("setDiff");
    QObject::connect(actionDiff, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setDiff", "setDiff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(val|0x01A15000,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
            widet_r.setDiff(val|0x01A15000,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
        }
    });
    QAction * actionDiffOn = menuOpt->addAction("setDiffOn");
    QObject::connect(actionDiffOn, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setDiffOn", "setDiffOn", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,val|0x01A16300,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
            widet_r.setDiff(0xffffffff,val|0x01A16300,0xffffffff,0xffffffff,0xffffffff,0xffffffff);
        }
    });
    QAction * actionDiffOff = menuOpt->addAction("setDiffOff");
    QObject::connect(actionDiffOff, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setDiffOff", "setDiffOff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,0xffffffff,val|0x01A13700,0xffffffff,0xffffffff,0xffffffff);
            widet_r.setDiff(0xffffffff,0xffffffff,val|0x01A13700,0xffffffff,0xffffffff,0xffffffff);
        }
    });
    QAction * actionBiasFo = menuOpt->addAction("setBiasFo");
    QObject::connect(actionBiasFo, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setBiasFo", "setBiasFo", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,0xffffffff,0xffffffff,val|0x03A1E800,0xffffffff,0xffffffff);
            widet_r.setDiff(0xffffffff,0xffffffff,0xffffffff,val|0x03A1E800,0xffffffff,0xffffffff);
        }
    });
    QAction * actionBiasHpf = menuOpt->addAction("setBiasHpf");
    QObject::connect(actionBiasHpf, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setBiasHpf", "setBiasHpf", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03A1FF00,0xffffffff);
            widet_r.setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03A1FF00,0xffffffff);
        }
    });
    QAction * actionBiasRefr = menuOpt->addAction("setBiasRefr");
    QObject::connect(actionBiasRefr, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setBiasRefr", "setBiasRefr", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03829600);
            widet_r.setDiff(0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,val|0x03829600);
        }
    });

    window.show();

    return app.exec();
}
