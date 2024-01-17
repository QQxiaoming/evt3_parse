#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QEventLoop>
#include "eventsensordatainput.h"
#include "config.h"

EventSensorDataInput::EventSensorDataInput(int port, QObject *parent)
    : QThread(parent)
    , m_exit(false) {
    moveToThread(this);
    m_port = port;
    m_host = QHostAddress::LocalHost;
}

EventSensorDataInput::~EventSensorDataInput() {
    m_exit = true;
    wait();
    delete m_udpSocketSetDiff;
}

void EventSensorDataInput::run() {
    m_udpSocketSetDiff = new QUdpSocket(this);
    m_tcpServer = new QTcpServer;
    bool ret = m_tcpServer->listen(QHostAddress::Any,m_port);
    if(!ret) {
        emit error(tr("listen %1").arg(m_tcpServer->errorString()));
        return;
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
        emit error(tr("acceptError %1").arg(socketError));
    });

    while(!m_exit) {
        // we shuoild process events here
        QEventLoop loop;
        loop.processEvents(QEventLoop::AllEvents, 100);
    }

    if(m_saveFile != nullptr) {
        m_saveFile->close();
        delete m_saveFile;
        m_saveFile = nullptr;
    }
    if(m_tcpSocket != nullptr) {
        m_tcpSocket->disconnectFromHost();
    }
    delete m_tcpServer;
}

void EventSensorDataInput::processPendingDatagrams()
{
    while (m_tcpSocket->bytesAvailable() > 100*1024) {
        uint64_t size = m_tcpSocket->bytesAvailable();
        if(size > 1*1024*1024)
            size = 1*1024*1024;
        QByteArray* datagram = new QByteArray;
        datagram->resize(size-size%2);
        qint64 ret = m_tcpSocket->read(datagram->data(),datagram->size());
        datagram->resize(ret);
        if(m_rec) {
            if(m_saveFile == nullptr) {
                m_saveFile = new QFile(QString("evt3_%1_").arg(m_port)+QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")+".bin");
                if(!m_saveFile->open(QIODevice::WriteOnly)) {
                    qDebug()  << m_saveFile->error() << m_saveFile->errorString();
                    abort();
                }
            }
            m_saveFile->write(*datagram);
        } else {
            if(m_saveFile != nullptr) {
                m_saveFile->close();
                delete m_saveFile;
                m_saveFile = nullptr;
            }
        }
        emit push_data(datagram);
    }
}

void EventSensorDataInput::setDiff(uint32_t diff, uint32_t diff_on, uint32_t diff_off, uint32_t bias_fo, uint32_t bias_hpf, uint32_t bias_refr)
{
    uint32_t data[6] = {diff,diff_on,diff_off,bias_fo,bias_hpf,bias_refr};
    m_udpSocketSetDiff->writeDatagram((char *)data,sizeof(data),m_host,m_port-SEVER_PORT+CLIENT_PORT);
}
