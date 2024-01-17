#include <QPainter>
#include <QMessageBox>
#include "eventsensorwidget.h"

EventSensorWidget::EventSensorWidget(int port,int trig_time, int lut_time, QWidget *parent)
    : QWidget{parent} {
    render = new EventSensorRender(this);
    dataIn = new EventSensorDataInput(port);
    timer = new QTimer(this);
    state_timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [&](){
        this->repaint();
    });
    timer->start(33*lut_time/trig_time);
    connect(state_timer, &QTimer::timeout, this, [&](){
        data_num = render->getDataNum();
        img_num = render->getImgNum();
        lost_data_num = render->getLostDataNum();
        lost_img_num = render->getLostImgNum();
    });
    state_timer->start(1000*5);

    connect(dataIn,&EventSensorDataInput::push_data, this, [&](QByteArray *data){
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
    QPainter painter;
    painter.begin(this);
    if(!qImg.isNull()) {
        painter.drawPixmap(QPoint(0, 0),
          QPixmap::fromImage(qImg.scaled(this->size() - QSize(0, 0))));
    } else {
        painter.fillRect(QRect(0, 0, this->width(), this->height()), Qt::black);
    }
    painter.setPen(QColor(255,255,255));
    painter.setFont(QFont("Arial", 10));
    painter.drawText(QRect(0, 100, 200, 50), Qt::AlignLeft, QString("data_num: %1").arg(data_num));
    painter.drawText(QRect(0, 120, 200, 50), Qt::AlignLeft, QString("img_num: %1").arg(img_num));
    painter.drawText(QRect(0, 140, 200, 50), Qt::AlignLeft, QString("lost_data_num: %1").arg(lost_data_num));
    painter.drawText(QRect(0, 160, 200, 50), Qt::AlignLeft, QString("lost_img_num: %1").arg(lost_img_num));
    painter.end();
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
