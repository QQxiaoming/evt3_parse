#include <QPainter>
#include <QMessageBox>
#include "eventsensorwidget.h"

EventSensorWidget::EventSensorWidget(int port,int trig_time, int lut_time, QWidget *parent)
    : QWidget{parent} {
    render = new EventSensorRender(this);
    dataIn = new EventSensorDataInput(port,this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&](){
        this->repaint();
    });
    timer->start(33*lut_time/trig_time);

    connect(dataIn,&EventSensorDataInput::push_data, this, [&](QByteArray data){
        render->pushData(data);
    });
    connect(dataIn,&EventSensorDataInput::error, this, [&](const QString &s){
        QMessageBox::critical(this, tr("EventSensorWidget"), s);
    });
    render->start();
    dataIn->start();
}

EventSensorWidget::~EventSensorWidget() {
    delete dataIn;
    delete render;
}

void EventSensorWidget::paintEvent(QPaintEvent *event) {
    QImage qImg = render->getImg();
    if(!qImg.isNull()) {
        QPainter painter;
        painter.begin(this);
        painter.drawPixmap(QPoint(0, 0),
          QPixmap::fromImage(qImg.scaled(this->size() - QSize(0, 0))));
        painter.end();
    } else {
        QPainter painter;
        painter.begin(this);
        painter.fillRect(QRect(0, 0, this->width(), this->height()), Qt::black);
        painter.end();
    }
    Q_UNUSED(event);
}

void EventSensorWidget::setRec(bool value) {
    dataIn->setRec(value);
}

bool EventSensorWidget::getRec(void) {
    return dataIn->getRec();
}

void EventSensorWidget::setDiff(uint32_t diff, uint32_t diff_on, uint32_t diff_off, uint32_t bias_fo, uint32_t bias_hpf, uint32_t bias_refr)
{
    dataIn->setDiff(diff,diff_on,diff_off,bias_fo,bias_hpf,bias_refr);
}
