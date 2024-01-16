#ifndef EVENTSENSORDATAINPUT_H
#define EVENTSENSORDATAINPUT_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

class EventSensorDataInput : public QThread
{
    Q_OBJECT
public:
    explicit EventSensorDataInput(int port, QObject *parent = nullptr);
    ~EventSensorDataInput();

protected:
    void run();

signals:
    void push_data(QByteArray data);

private slots:
    void processPendingDatagrams();

private:
    QTcpServer *m_tcpServer;
    QTcpSocket *m_tcpSocket;
    QHostAddress m_host;
    int m_port;
};

#endif // EVENTSENSORDATAINPUT_H
