#ifndef EVENTSENSORRENDER_H
#define EVENTSENSORRENDER_H

#include <QThread>
#include <QImage>
#include <QVector>
#include <QByteArray>
#include <QMutex>

class EventSensorRender : public QThread
{
    Q_OBJECT
public:
    explicit EventSensorRender(QObject *parent = nullptr);
    ~EventSensorRender();
    QImage getImg(void);
    void pushData(const char *data, uint64_t size);
    void pushData(QByteArray data);

protected:
    void run();

private:
    enum EventType
    {
        EVT_ADDR_Y = 0b0000,
        EVT_ADDR_X = 0b0010,
        VECT_BASE_X = 0b0011,
        VECT_12 = 0b0100,
        VECT_8 = 0b0101,
        TIME_LOW = 0b0110,
        CONTINUED_4 = 0b0111,
        TIME_HIGH = 0b1000,
        PAD9999 = 0b1001,
        EXT_TRIGGER = 0b1010,
        OTHERS = 0b1110,
        CONTINUED_12 = 0b1111,
    };

    enum EventSubType {
        MASTER_IN_TD_EVENT_COUNT = 0x14,
        MASTER_RATE_CONTROL_TD_EVENT_COUNT = 0x16,
        DUMMY = 0xff,
    };
    void process_event(uint16_t evt_data);
    void drawPixel(int x, int y, int pol);
    void HandleEVT_ADDR_Y(uint16_t event);
    void HandleEVT_ADDR_X(uint16_t event);
    void HandleVECT_BASE_X(uint16_t event);
    void HandleVECT_12(uint16_t event);
    void HandleVECT_8(uint16_t event);
    void HandleTIME_LOW(uint16_t event);
    void HandleTIME_HIGH(uint16_t event);
    void HandleEXT_TRIGGER(uint16_t event);
    void HandleOTHERS(uint16_t event);
    void HandleCONTINUED_12(uint16_t event);
    void HandleCONTINUED_4(uint16_t event);
    void process(void);

private:
    QImage *qImg;
    uchar *buff;
    QMutex inMutex;
    QMutex imgMutex;
    QVector<QByteArray> in;
    QVector<QImage> img;
    // parse structure
    uint64_t timestamp = 0; // 24bit width
    uint32_t addr_x_base;
    uint32_t addr_y_base;
    uint8_t evt_pol;
    bool find_start;
    uint32_t otherEvent[2];
    uint32_t currOtherNum;
    uint32_t index_12;
    QList<int32_t> m_trigEventH;
    QList<int32_t> m_trigEventL;
};

#endif // EVENTSENSORRENDER_H
