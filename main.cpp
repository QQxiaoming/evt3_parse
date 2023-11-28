#include <QWidget>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QResizeEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QMainWindow>
#include <QApplication>
#include <QDebug>

#define USE_UDP  1
#define USE_TCP  0

#define IMG_WIDTH  (1280)
#define IMG_HEIGHT (720)
#define IMG_FPS    (30)
#define SEVER_PORT (13456)
#define CLIENT_PORT (15678)

class EventSensorRenderWidget : public QWidget
{
    Q_OBJECT

public:
    EventSensorRenderWidget(int port, bool drawFrame = true,
        bool drawFps = true, bool drawTrigger = false, QWidget *parent = nullptr);
    ~EventSensorRenderWidget();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void processPendingDatagrams();

private:
    void init_param();
    void drawPixel(int x, int y, int pol);
    void process_event(uint16_t evt_data);
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

public:
    void setDrawFrame(bool value) { m_drawFrame = value;}
    bool getDrawFrame(void) {return m_drawFrame;}
    void setDrawFps(bool value) { m_drawFps = value;}
    bool getDrawFps(void) {return m_drawFps;}
    void setDrawTrigger(bool value) { m_drawTrigger = value;}
    bool getDrawTrigger(void) {return m_drawTrigger;}
    void setRec(bool value) { m_rec = value;}
    bool getRec(void) {return m_rec;}
    void setDiff(uint32_t diff, uint32_t diff_on, uint32_t diff_off);

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

    // parse structure
    uint64_t timestamp = 0; // 24bit width
    uint32_t addr_x_base;
    uint32_t addr_y_base;
    uint8_t evt_pol;
    bool find_start;
    uint32_t otherEvent[2];
    uint32_t currOtherNum;
    uint32_t index_12;

    // stream structure
#if USE_UDP
    QUdpSocket *m_udpSocket;
#endif
#if USE_TCP
    QTcpServer *m_tcpServer;
    QTcpSocket *m_tcpSocket;
#endif
    QUdpSocket *m_udpSocketSetDiff;
    QHostAddress m_host;
    int m_port;
    uint32_t lastUdpPackIndex = UINT_LEAST32_MAX;

    // render structure
    uchar *m_buff0;
    uchar *m_buff1;
    uchar *m_wbuff;
    uchar *m_lastbuff;
    QImage m_image0;
    QImage m_image1;
    uint32_t m_calFps = 0;
    bool m_first_get_timestamp = true;
    uint64_t m_first_timestamp = 0;
    uint64_t m_last_switch_timestamp = 0;
    uint64_t m_base_timestamp = QDateTime::currentMSecsSinceEpoch();
    QList<int32_t> m_trigEventH;
    QList<int32_t> m_trigEventL;
    QPen m_pen;
    QFont m_font;

    // save
    bool m_rec = false;
    QFile *m_saveFile = nullptr;

    bool m_drawFrame;
    bool m_drawFps;
    bool m_drawTrigger;
};

#include "main.moc"

EventSensorRenderWidget::EventSensorRenderWidget(int port, bool drawFrame, 
        bool drawFps, bool drawTrigger, QWidget *parent)
    :  QWidget(parent), m_drawFrame(drawFrame), m_drawFps(drawFps),
      m_drawTrigger(drawTrigger)
{
    m_pen.setColor(Qt::white);
    m_pen.setWidth(1);
    m_font = QApplication::font();
    m_font.setPointSize(20);

    m_host = QHostAddress::LocalHost;
    m_udpSocketSetDiff = new QUdpSocket(this);
    
#if USE_UDP
    m_udpSocket = new QUdpSocket(this);
    m_udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption,32*1024*1024);
    bool ret = m_udpSocket->bind(QHostAddress::Any,port);
    if(!ret) {
        QMessageBox::critical(this, tr("Error"), tr("bind %1").arg(m_udpSocket->errorString()));
        abort();
    }
    connect(m_udpSocket, &QUdpSocket::readyRead,
            this, &EventSensorRenderWidget::processPendingDatagrams);
    connect(m_udpSocket, &QUdpSocket::errorOccurred, [&](QAbstractSocket::SocketError socketError){
        QMessageBox::critical(this, tr("Error"), tr("errorOccurred %1").arg(socketError));
    });
#endif
#if USE_TCP
    m_tcpServer = new QTcpServer(this);
    bool ret = m_tcpServer->listen(QHostAddress::Any,port);
    if(!ret) {
        QMessageBox::critical(this, tr("Error"), tr("listen %1").arg(m_tcpServer->errorString()));
        abort();
    }
    connect(m_tcpServer, &QTcpServer::newConnection, [&](){
        m_tcpSocket = m_tcpServer->nextPendingConnection();
        m_host = m_tcpSocket->peerAddress();
        connect(m_tcpSocket, &QTcpSocket::readyRead,
                this, &EventSensorRenderWidget::processPendingDatagrams);
        connect(m_tcpSocket, &QTcpSocket::disconnected, [&](){
            m_tcpSocket->close();
            delete m_tcpSocket;
        });
    });
    connect(m_tcpServer, &QTcpServer::acceptError, [&](QAbstractSocket::SocketError socketError){
        QMessageBox::critical(this, tr("Error"), tr("acceptError %1").arg(socketError));
    });
#endif

    resize(IMG_WIDTH, IMG_HEIGHT);
    m_image0 = QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB888);
    m_image0.fill(Qt::black);
    m_image1 = QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB888);
    m_image1.fill(Qt::black);
    m_buff0 = m_image0.bits();
    m_buff1 = m_image1.bits();
    m_wbuff = m_buff0;
    m_lastbuff = m_buff0;
    m_port = port;

    QTimer *m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [&](void){
        update();
    });
    m_timer->start((int)(1000/IMG_FPS));
}

EventSensorRenderWidget::~EventSensorRenderWidget()
{
#if USE_UDP
    m_udpSocket->close();
    delete m_udpSocket;
#endif
#if USE_TCP
    m_tcpServer->close();
    delete m_tcpServer;
#endif
    delete m_udpSocketSetDiff;
}

void EventSensorRenderWidget::setDiff(uint32_t diff, uint32_t diff_on, uint32_t diff_off)
{
    uint32_t data[3] = {diff,diff_on,diff_off};
    m_udpSocketSetDiff->writeDatagram((char *)data,sizeof(data),m_host,m_port-SEVER_PORT+CLIENT_PORT);
}

void EventSensorRenderWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(m_pen);
    painter.setFont(m_font);

    // 1. draw frame
    if(!m_drawFrame) painter.fillRect(0,0,size().width(),size().height(),Qt::black);
    if(m_wbuff == m_buff1) {
        if(m_lastbuff == m_buff1) {
            m_calFps++;
        } else {
            m_calFps = 0;
        }
        if(m_drawFrame) {
            painter.drawImage(0, 0, m_image1.scaled(this->size()));
        }
    } else {
        if(m_lastbuff == m_buff0) {
            m_calFps++;
        } else {
            m_calFps = 0;
        }
        if(m_drawFrame) {
            painter.drawImage(0, 0, m_image0.scaled(this->size()));
        }
    }

    // 2. draw fps
    if(m_drawFps) {
        int fps = m_calFps==0?IMG_FPS:IMG_FPS/m_calFps;
        painter.drawText(QPoint(10,30),fps?QString::number(fps):"<1");
        painter.drawText(QPoint(10,60),
            QDateTime::fromMSecsSinceEpoch(m_base_timestamp+timestamp/1000).toString("hh:mm:ss.zzz"));
        painter.drawText(QPoint(10,90),
                         QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    }

    // 3. draw trigger
    if(m_drawTrigger) {
        painter.drawLine(QLine(0,this->size().height()-30,this->size().width(),this->size().height()-30));
        for(int i=0;i<m_trigEventH.count();i++) {
            if(m_trigEventH.at(i) > 0) {
                int trig = qMax(0,this->size().width() - m_trigEventH.at(i));
                painter.drawLine(QLine(trig,this->size().height()-30,trig,this->size().height()-30-30));
                m_trigEventH[i] -= 5;
                if(m_trigEventH.at(i) < 0) m_trigEventH[i] = 0;
            }
        }
        m_trigEventH.removeAll(0);

        for(int i=0;i<m_trigEventL.count();i++) {
            if(m_trigEventL.at(i) > 0) {
                int trig = qMax(0,this->size().width() - m_trigEventL.at(i));
                painter.drawLine(QLine(trig,this->size().height()-30,trig,this->size().height()));
                m_trigEventL[i] -= 5;
                if(m_trigEventL.at(i) < 0) m_trigEventL[i] = 0;
            }
        }
        m_trigEventL.removeAll(0);
    }

    Q_UNUSED(event);
}

void EventSensorRenderWidget::processPendingDatagrams()
{
#if USE_UDP
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &m_host);

        uint16_t *p = (uint16_t *)datagram.data();
        uint32_t udpPackIndex = *(uint32_t *)p;
        if((udpPackIndex <= lastUdpPackIndex) && (lastUdpPackIndex != UINT_LEAST32_MAX)) {
            qDebug() << "udpPackIndex err, will drop" << udpPackIndex;
            continue;
        }
        if(udpPackIndex - lastUdpPackIndex != 1)
        //qDebug() << udpPackIndex << lastUdpPackIndex;
        lastUdpPackIndex = udpPackIndex;
        for(int i = 2; i < datagram.size()/2; i++)
        {
            process_event(p[i]);
        }
    }
#endif
#if USE_TCP
    while (m_tcpSocket->bytesAvailable()) {
        QByteArray datagram;
        datagram.resize(m_tcpSocket->bytesAvailable());
        m_tcpSocket->read(datagram.data(), datagram.size());

        uint16_t *p = (uint16_t *)datagram.data();
        for(int i = 0; i < datagram.size()/2; i++)
        {
            process_event(p[i]);
        }
    }
#endif
}

void EventSensorRenderWidget::init_param()
{
    timestamp = 0;
    addr_x_base = 0;
    addr_y_base = 0;
    evt_pol = 0;
    find_start = false;

    otherEvent[0] = 0;
    otherEvent[1] = 0;
    currOtherNum = 0;
    index_12 = 0;
}

void EventSensorRenderWidget::drawPixel(int x, int y, int pol)
{
    if(timestamp == 0) {
        return;
    } else {
        if(m_first_get_timestamp) {
            m_first_timestamp = timestamp;
            m_base_timestamp = QDateTime::currentMSecsSinceEpoch();
            m_last_switch_timestamp = QDateTime::currentMSecsSinceEpoch();
            m_first_get_timestamp = false;
        }
        if(((timestamp - m_first_timestamp)/1000) >= (uint64_t)(1000/IMG_FPS)) {
            while(1) {
                if(QDateTime::currentMSecsSinceEpoch() - m_last_switch_timestamp > (int)(1000/IMG_FPS)) {
                    m_last_switch_timestamp = QDateTime::currentMSecsSinceEpoch();
                    m_first_timestamp = timestamp;
                    if(m_wbuff == m_buff1) {
                        m_image0.fill(Qt::black);
                        m_wbuff = m_buff0;
                    } else {
                        m_image1.fill(Qt::black);
                        m_wbuff = m_buff1;
                    }
                    break;
                }
                qApp->processEvents();
            }
        }
    }

#if 1
    m_wbuff[y*IMG_WIDTH*3 + x*3 + 0] = 0;
    m_wbuff[y*IMG_WIDTH*3 + x*3 + 1] = 0;
    m_wbuff[y*IMG_WIDTH*3 + x*3 + 2] = 0;
    m_wbuff[y*IMG_WIDTH*3 + x*3 + pol] = 255;
#else
    if(pol) {
        m_wbuff[y*IMG_WIDTH*3 + x*3 + 0] += 10;
    } else {
        m_wbuff[y*IMG_WIDTH*3 + x*3 + 0] -= 10;
    }
#endif
}

void EventSensorRenderWidget::process_event(uint16_t evt_data)
{
    EventType event = static_cast<EventType>(evt_data >> 12);

    if(!find_start) {
        if(event == TIME_HIGH) {
            find_start = true;
        } else {
            return;
        }
    }

    if(m_rec) {
        if(m_saveFile == nullptr) {
            m_saveFile = new QFile(QString("evt3_%1_").arg(m_port)+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".bin");
            if(!m_saveFile->open(QIODevice::WriteOnly)) {
                qDebug()  << m_saveFile->error() << m_saveFile->errorString();
                abort();
            }
        }
        m_saveFile->write((char *)&evt_data,2);
    } else {
        if(m_saveFile != nullptr) {
            m_saveFile->close();
            delete m_saveFile;
            m_saveFile = nullptr;
        }
    }
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

void EventSensorRenderWidget::HandleEVT_ADDR_Y(uint16_t event)
{
    addr_y_base = event & 0x7ff;
}

void EventSensorRenderWidget::HandleEVT_ADDR_X(uint16_t event)
{
    int x = event & 0x7ff;
    int pol = (event >> 11) & 0x1;
    evt_pol = pol; // does this needed??
    // plot the event on the buffer
    drawPixel(x, addr_y_base, pol);
}

void EventSensorRenderWidget::HandleVECT_BASE_X(uint16_t event)
{
    addr_x_base = event & 0x7ff;
    evt_pol = (event >> 11) & 0x1;
}

void EventSensorRenderWidget::HandleVECT_12(uint16_t event)
{
    uint16_t bits = event & 0x7ff;

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

void EventSensorRenderWidget::HandleVECT_8(uint16_t event)
{
    uint16_t bits = event & 0x7ff;

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

void EventSensorRenderWidget::HandleTIME_LOW(uint16_t event)
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

void EventSensorRenderWidget::HandleTIME_HIGH(uint16_t event)
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

void EventSensorRenderWidget::HandleEXT_TRIGGER(uint16_t event)
{
    uint8_t value = event & 0x1;
    uint8_t id = (event>>8) & 0xf;
    if(value) {
        m_trigEventH.push_back(this->size().width());
    } else {
        m_trigEventL.push_back(this->size().width());
    }
    qDebug("TRIGGER %d: %d %lu",id,value,timestamp);
}

void EventSensorRenderWidget::HandleOTHERS(uint16_t event)
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

void EventSensorRenderWidget::HandleCONTINUED_4(uint16_t event)
{
    otherEvent[currOtherNum] |= (((uint32_t)(event&0xf))<<(24));
    //qDebug("%d %s: %d",m_port-SEVER_PORT,currOtherNum?"MASTER_IN_TD_EVENT_COUNT":"MASTER_RATE_CONTROL_TD_EVENT_COUNT", otherEvent[currOtherNum]);
}

void EventSensorRenderWidget::HandleCONTINUED_12(uint16_t event)
{
    otherEvent[currOtherNum] |= (((uint32_t)(event&0xfff))<<(12*index_12));
    index_12++;
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);

#if 0 //independent window
    EventSensorRenderWidget widet_l(SEVER_PORT+0);
    widet_l.setWindowTitle("evt3_parse left");
    EventSensorRenderWidget widet_r(SEVER_PORT+1);
    widet_r.setWindowTitle("evt3_parse right");
    widet_r.show();
    widet_l.show();
#else
    QMainWindow window;
    QWidget widget(&window);
    QVBoxLayout layout(&widget);
    EventSensorRenderWidget widet_l(SEVER_PORT+0,true,true,false,&widget);
    EventSensorRenderWidget widet_r(SEVER_PORT+1,true,true,false,&widget);
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
    QAction * actionDF = menuOpt->addAction("drawFrame");
    actionDF->setCheckable(true);
    actionDF->setChecked(widet_l.getDrawFrame());
    QObject::connect(actionDF, &QAction::triggered, [&](){
        widet_l.setDrawFrame(actionDF->isChecked());
        widet_r.setDrawFrame(actionDF->isChecked());
    });
    QAction * actionDFS = menuOpt->addAction("drawFps");
    actionDFS->setCheckable(true);
    actionDFS->setChecked(widet_l.getDrawFps());
    QObject::connect(actionDFS, &QAction::triggered, [&](){
        widet_l.setDrawFps(actionDFS->isChecked());
        widet_r.setDrawFps(actionDFS->isChecked());
    });
    QAction * actionDT = menuOpt->addAction("drawTrigger");
    actionDT->setCheckable(true);
    actionDT->setChecked(widet_l.getDrawTrigger());
    QObject::connect(actionDT, &QAction::triggered, [&](){
        widet_l.setDrawTrigger(actionDT->isChecked());
        widet_r.setDrawTrigger(actionDT->isChecked());
    });
    QAction * actionDiff = menuOpt->addAction("setDiff");
    QObject::connect(actionDiff, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setDiff", "diff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(val|0x01A16300,0xffffffff,0xffffffff);
            widet_r.setDiff(val|0x01A16300,0xffffffff,0xffffffff);
        }
    });
    QAction * actionDiffOn = menuOpt->addAction("setDiffOn");
    QObject::connect(actionDiffOn, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setDiffOn", "diff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,val|0x01A15000,0xffffffff);
            widet_r.setDiff(0xffffffff,val|0x01A15000,0xffffffff);
        }
    });
    QAction * actionDiffOff = menuOpt->addAction("setDiffOff");
    QObject::connect(actionDiffOff, &QAction::triggered, [&](){
        static int val = 0;
        bool ok;
        val = QInputDialog::getInt(&window, "setDiffOff", "diff", val, 0, 255, 1, &ok);
        if(ok) {
            widet_l.setDiff(0xffffffff,0xffffffff,val|0x01A13700);
            widet_r.setDiff(0xffffffff,0xffffffff,val|0x01A13700);
        }
    });
    window.show();
#endif

    return app.exec();
}
