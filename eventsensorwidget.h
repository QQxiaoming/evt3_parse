#ifndef EVENTSENSORWIDGET_H
#define EVENTSENSORWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPaintEvent>
#include "eventsensorrender.h"
#include "eventsensordatainput.h"

class EventSensorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EventSensorWidget(QWidget *parent = nullptr);
    ~EventSensorWidget();

protected:
    void paintEvent(QPaintEvent *event);

private:
    EventSensorRender *render;
    EventSensorDataInput *dataIn;
    QTimer *timer;
};

#endif // EVENTSENSORWIDGET_H
