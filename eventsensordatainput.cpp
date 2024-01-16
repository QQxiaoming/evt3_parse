#include <QMessageBox>
#include "eventsensordatainput.h"

EventSensorDataInput::EventSensorDataInput(int port, QObject *parent)
    : QThread(parent) {
    m_port = port;
    m_host = QHostAddress::LocalHost;
}

EventSensorDataInput::~EventSensorDataInput() {
}

void EventSensorDataInput::run() {
    m_tcpServer = new QTcpServer;
    bool ret = m_tcpServer->listen(QHostAddress::Any,m_port);
    if(!ret) {
        qDebug() << tr("listen %1").arg(m_tcpServer->errorString());
        //QMessageBox::critical(this, tr("Error"), tr("listen %1").arg(m_tcpServer->errorString()));
        //abort();
    }
    connect(m_tcpServer, &QTcpServer::newConnection, [&](){
        m_tcpSocket = m_tcpServer->nextPendingConnection();
        m_host = m_tcpSocket->peerAddress();
        connect(m_tcpSocket, &QTcpSocket::readyRead,
                this, &EventSensorDataInput::processPendingDatagrams);
        connect(m_tcpSocket, &QTcpSocket::disconnected, [&](){
            m_tcpSocket->close();
            delete m_tcpSocket;
        });
    });
    connect(m_tcpServer, &QTcpServer::acceptError, [&](QAbstractSocket::SocketError socketError){
        qDebug() << tr("acceptError %1").arg(socketError);
        //QMessageBox::critical(this, tr("Error"), tr("acceptError %1").arg(socketError));
    });

    while(1) {
        exec();
    }
}

void EventSensorDataInput::processPendingDatagrams()
{
    while (m_tcpSocket->bytesAvailable() > 100*1024) {
        QByteArray datagram;
        uint64_t size = m_tcpSocket->bytesAvailable();
        datagram.resize(size-size%2);
        m_tcpSocket->read(datagram.data(), datagram.size());
        emit push_data(datagram);
    }
}
