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
    void setTime(int32_t trig_time, int32_t lut_time);
    
protected:
    void paintEvent(QPaintEvent *event);

private:
    EventSensorRender *render;
    EventSensorDataInput *dataIn;
    QTimer *timer;
    QTimer *state_timer;

    uint64_t data_num = 0;
    uint64_t img_num = 0;
    uint64_t lost_data_num = 0;
    uint64_t lost_img_num = 0;

    int m_port;
    int32_t m_lut_time;
    int32_t m_trig_time;
};

#endif // EVENTSENSORWIDGET_H
