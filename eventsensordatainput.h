#ifndef EVENTSENSORDATAINPUT_H
#define EVENTSENSORDATAINPUT_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QFile>

class EventSensorDataInput : public QThread
{
    Q_OBJECT
public:
    explicit EventSensorDataInput(int port, QObject *parent = nullptr);
    ~EventSensorDataInput();
    void setDiff(uint32_t diff, uint32_t diff_on, uint32_t diff_off, uint32_t bias_fo, uint32_t bias_hpf, uint32_t bias_refr);
    void setRec(bool value) { m_rec = value;}
    bool getRec(void) {return m_rec;}

protected:
    void run();

signals:
    void push_data(QByteArray data);
    void error(const QString &s);

private slots:
    void processPendingDatagrams();

private:
    QTcpServer *m_tcpServer = nullptr;
    QTcpSocket *m_tcpSocket = nullptr;
    QUdpSocket *m_udpSocketSetDiff = nullptr;
    QHostAddress m_host;
    int m_port;
    // save
    bool m_rec = false;
    QFile *m_saveFile = nullptr;
    bool m_exit;
};

#endif // EVENTSENSORDATAINPUT_H
