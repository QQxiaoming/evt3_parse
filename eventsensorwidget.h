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
    explicit EventSensorWidget(int port,int trig_time, int lut_time, QWidget *parent = nullptr);
    ~EventSensorWidget();

public:
    void setRec(bool value);
    bool getRec(void);
    void setDiff(uint32_t diff, uint32_t diff_on, uint32_t diff_off, uint32_t bias_fo, uint32_t bias_hpf, uint32_t bias_refr);

protected:
    void paintEvent(QPaintEvent *event);

private:
    EventSensorRender *render;
    EventSensorDataInput *dataIn;
    QTimer *timer;
};

#endif // EVENTSENSORWIDGET_H
