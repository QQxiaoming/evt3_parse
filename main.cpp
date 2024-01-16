#include <QApplication>

#include "eventsensorwidget.h"

//./test_event_camif_core -idx 1 -ip 169.254.34.189 -trig_time 300 -lut_time 1000 -tcp

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);
    EventSensorWidget widget;
    widget.show();

    return app.exec();
}
