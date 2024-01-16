#include <QDebug>
#include <QPainter>
#include "eventsensorrender.h"

#define IMG_WIDTH  (1280)
#define IMG_HEIGHT (720)

EventSensorRender::EventSensorRender(QObject *parent)
    : QThread(parent) 
    , m_exit(false) {
    buff = new uchar[IMG_WIDTH * IMG_HEIGHT * 3];
    qImg = new QImage(buff, IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB888);
    memset(buff, 0x0, IMG_WIDTH * IMG_HEIGHT * 3);
}

EventSensorRender::~EventSensorRender() {
    m_exit = true;
    wait();
    delete qImg;
    delete[] buff;
}

void EventSensorRender::process(void) {
    QMutexLocker locker(&inMutex);
    if(in.size() > 0) {
        QByteArray datagram = in.first();
        in.pop_front();
        uint16_t *p = (uint16_t *)datagram.data();
        for(int i = 0; i < datagram.size()/2; i++)
        {
            process_event(p[i]);
        }
    }
}
void EventSensorRender::run() {
    while(!m_exit) {
        process();
    }
}

void EventSensorRender::pushData(const char *data, uint64_t size) {
    QMutexLocker locker(&inMutex);
    QByteArray ba(data,size);
    in.push_back(ba);
}

void EventSensorRender::pushData(QByteArray data) {
    QMutexLocker locker(&inMutex);
    in.push_back(data);
}

QImage EventSensorRender::getImg(void) {
    QMutexLocker locker(&imgMutex);
    if(img.size() > 0) {
        QImage rimg = img.first();
        img.pop_front();
        return rimg;
    }
    return QImage();
}

void EventSensorRender::drawPixel(int x, int y, int pol)
{
    static uint64_t last_timestamp = 0;
    const static uchar test[IMG_WIDTH * IMG_HEIGHT * 3] = {0};
    while(1) {
        if(timestamp - last_timestamp >= 33333) {
            #if 1
            if(memcmp(buff,test,IMG_WIDTH * IMG_HEIGHT * 3) != 0) {
            #else
            {
            #endif
                QMutexLocker locker(&imgMutex);
                img.push_back(qImg->copy());
                delete[] buff;
                delete qImg;
                buff = new uchar[IMG_WIDTH * IMG_HEIGHT * 3];
                qImg = new QImage(buff, IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB888);
                memset(buff, 0x0, IMG_WIDTH * IMG_HEIGHT * 3);
            }
            last_timestamp += 33333;
        } else {
            break;
        }
    }
    buff[y*IMG_WIDTH*3 + x*3 + 0] = 0;
    buff[y*IMG_WIDTH*3 + x*3 + 1] = 0;
    buff[y*IMG_WIDTH*3 + x*3 + 2] = 0;
    buff[y*IMG_WIDTH*3 + x*3 + pol] = 255;
}

void EventSensorRender::process_event(uint16_t evt_data)
{
    EventType event = static_cast<EventType>(evt_data >> 12);

    switch (event)
    {
    case EVT_ADDR_Y:
        HandleEVT_ADDR_Y(evt_data);
        break;
    case EVT_ADDR_X:
        HandleEVT_ADDR_X(evt_data);
        break;
    case VECT_BASE_X:
        HandleVECT_BASE_X(evt_data);
        break;
    case VECT_12:
        HandleVECT_12(evt_data);
        break;
    case VECT_8:
        HandleVECT_8(evt_data);
        break;
    case TIME_LOW:
        HandleTIME_LOW(evt_data);
        break;
    case TIME_HIGH:
        HandleTIME_HIGH(evt_data);
        break;
    case EXT_TRIGGER:
        HandleEXT_TRIGGER(evt_data);
        break;
    case OTHERS:
        HandleOTHERS(evt_data);
        break;
    case CONTINUED_4:
        HandleCONTINUED_4(evt_data);
        break;
    case CONTINUED_12:
        HandleCONTINUED_12(evt_data);
        break;
    case PAD9999:
        if(evt_data != 0x9999) {
            qDebug("Unknown event 0x%04x",evt_data);
        }
        break;
    default:
        qDebug("Unknown event 0x%04x",evt_data);
        break;
    }
}

void EventSensorRender::HandleEVT_ADDR_Y(uint16_t event)
{
    addr_y_base = event & 0x7ff;
}

void EventSensorRender::HandleEVT_ADDR_X(uint16_t event)
{
    int x = event & 0x7ff;
    int pol = (event >> 11) & 0x1;
    evt_pol = pol; // does this needed??
    // plot the event on the buffer
    drawPixel(x, addr_y_base, pol);
}

void EventSensorRender::HandleVECT_BASE_X(uint16_t event)
{
    addr_x_base = event & 0x7ff;
    evt_pol = (event >> 11) & 0x1;
}

void EventSensorRender::HandleVECT_12(uint16_t event)
{
    uint16_t bits = event & 0xfff;

    for (int i = 0; i < 12; i++)
    {
        if (bits & (1 << i))
        {
            if(addr_x_base + i > IMG_WIDTH) {
                if(addr_y_base + 1 > IMG_HEIGHT) {
                    drawPixel(addr_x_base + i - IMG_WIDTH, addr_y_base+1-IMG_HEIGHT, evt_pol);
                } else {
                    drawPixel(addr_x_base + i - IMG_WIDTH, addr_y_base+1, evt_pol);
                }
            } else {
                drawPixel(addr_x_base + i, addr_y_base, evt_pol);
            }
        }
    }
    addr_x_base += 12;
    if(addr_x_base >= IMG_WIDTH) {
        addr_x_base -= IMG_WIDTH;
        addr_y_base++;
        if(addr_y_base >= IMG_HEIGHT) {
            addr_y_base -= IMG_HEIGHT;
        }
    }
}

void EventSensorRender::HandleVECT_8(uint16_t event)
{
    uint16_t bits = event & 0xff;

    for (int i = 0; i < 8; i++)
    {
        if (bits & (1 << i))
        {
            if(addr_x_base + i > IMG_WIDTH) {
                if(addr_y_base + 1 > IMG_HEIGHT) {
                    drawPixel(addr_x_base + i - IMG_WIDTH, addr_y_base+1-IMG_HEIGHT, evt_pol);
                } else {
                    drawPixel(addr_x_base + i - IMG_WIDTH, addr_y_base+1, evt_pol);
                }            } else {
                drawPixel(addr_x_base + i, addr_y_base, evt_pol);
            }
        }
    }
    addr_x_base += 8;
    if(addr_x_base >= IMG_WIDTH) {
        addr_x_base -= IMG_WIDTH;
        addr_y_base++;
        if(addr_y_base >= IMG_HEIGHT) {
            addr_y_base -= IMG_HEIGHT;
        }
    }
}

void EventSensorRender::HandleTIME_LOW(uint16_t event)
{
    uint16_t time_low_12b = event & 0xfff;
    uint64_t old_timestamp = timestamp;
    uint16_t old_time_low_12b = (old_timestamp) & 0xfff;
    if(old_time_low_12b > time_low_12b) {
        timestamp = ((timestamp & 0xfffffffffffff000) | time_low_12b) + (1<<12);
    } else {
        timestamp = ((timestamp & 0xfffffffffffff000) | time_low_12b);
    }

    if(old_timestamp > timestamp) {
        qDebug("LOW timestamp overflow %lu -> %lu : %lu %u %u\n",old_timestamp,timestamp,old_timestamp-timestamp,time_low_12b,old_time_low_12b);
    }
}

void EventSensorRender::HandleTIME_HIGH(uint16_t event)
{
    uint32_t time_high_12b = event & 0xfff;
    uint64_t old_timestamp = timestamp;
    uint32_t old_time_high_12b = (old_timestamp>>12) & 0xfff;
    if(old_time_high_12b == time_high_12b) {
        timestamp = (timestamp & 0xffffffffff000fff) | (time_high_12b << 12);
    } else if(old_time_high_12b >= time_high_12b) {
        qDebug() << "up 1" << old_time_high_12b << time_high_12b;
        timestamp = (time_high_12b << 12) + (timestamp & 0xffffffffff000000) + (1<<24);
    } else {
        timestamp = (time_high_12b << 12) + (timestamp & 0xffffffffff000000);
    }

    if(old_timestamp > timestamp) {
        qDebug("HIGH timestamp overflow %lu -> %lu : %lu %u %u\n",old_timestamp,timestamp,old_timestamp-timestamp,time_high_12b,old_time_high_12b);
    }
}

void EventSensorRender::HandleEXT_TRIGGER(uint16_t event)
{
    uint8_t value = event & 0x1;
    uint8_t id = (event>>8) & 0xf;
    if(value) {
        // painer text on current qImg
        QPainter painter;
        painter.begin(qImg);
        painter.setPen(QColor(255,255,255));
        painter.setFont(QFont("Arial", 20));
        painter.drawText(0, 40, QString("TRIGGER %1: %2 %3").arg(id).arg(value).arg(timestamp));
        painter.end();
    }
    qDebug("TRIGGER %d: %d %lu",id,value,timestamp);
}

void EventSensorRender::HandleOTHERS(uint16_t event)
{
    uint16_t sub_type = (event) & 0xfff;
    switch (sub_type) {
    case MASTER_IN_TD_EVENT_COUNT:
        currOtherNum = 0;
        otherEvent[currOtherNum] = 0;
        index_12 = 0;
        break;
    case MASTER_RATE_CONTROL_TD_EVENT_COUNT:
        currOtherNum = 1;
        otherEvent[currOtherNum] = 0;
        index_12 = 0;
        break;
    case DUMMY:
    default:
        break;
    }
}

void EventSensorRender::HandleCONTINUED_4(uint16_t event)
{
    otherEvent[currOtherNum] |= (((uint32_t)(event&0xf))<<(24));
    //qDebug("%d %s: %d",m_port-SEVER_PORT,currOtherNum?"MASTER_IN_TD_EVENT_COUNT":"MASTER_RATE_CONTROL_TD_EVENT_COUNT", otherEvent[currOtherNum]);
}

void EventSensorRender::HandleCONTINUED_12(uint16_t event)
{
    otherEvent[currOtherNum] |= (((uint32_t)(event&0xfff))<<(12*index_12));
    index_12++;
}
