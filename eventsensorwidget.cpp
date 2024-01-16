#include <QPainter>
#include "eventsensorwidget.h"

EventSensorWidget::EventSensorWidget(QWidget *parent)
    : QWidget{parent} {
    render = new EventSensorRender(this);
    dataIn = new EventSensorDataInput(13456+1,this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&](){
        this->repaint();
    });
    timer->start(33*1000/300);

    connect(dataIn,&EventSensorDataInput::push_data, this, [&](QByteArray data){
        render->pushData(data);
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
