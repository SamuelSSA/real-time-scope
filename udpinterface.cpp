#include "udpinterface.h"
#include <QTime>
#include <QDebug>

UDPInterface::UDPInterface(QHostAddress ip, quint16 port, QObject *parent) :
    QObject(parent), ip(ip), port(port)
{
    udpSocket = new QUdpSocket(this);
    isConnectedFlag = false;
}

UDPInterface::~UDPInterface()
{
    disconnect();
    delete(udpSocket);
}

bool UDPInterface::isConnected()
{
    return this->isConnectedFlag;
}

bool UDPInterface::bind()
{
    bool ret = false;

    this->isConnectedFlag = udpSocket->bind(ip, port);

    if( this->isConnectedFlag == true )
    {
        qDebug() << QTime::currentTime() << udpSocket->localAddress() << " :" << udpSocket->localPort();
        qDebug() << "SocketState: " << udpSocket->state();
        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(receivedData()));
        ret = true;
    }
    else
    {
        qDebug() << "ERROR: Socket bind failure.";
        qDebug() << udpSocket->error();
        ret = false;
    }

    return ret;
}

void UDPInterface::unbind()
{
    if(this->isConnected())
    {
        disconnect(udpSocket, SIGNAL(readyRead()), this, SLOT(receivedData()));
        udpSocket->close();
        this->isConnectedFlag = false;
    }
}

void UDPInterface::receivedData()
{
    // aij = A[i * m + j];
    uint32_t  dataLen = udpSocket->pendingDatagramSize();
    dataLen = (dataLen > BUFF_SIZE) ? BUFF_SIZE : dataLen;

    //qDebug() << "data len: " << dataLen;

    std::vector<float> buffer(dataLen/sizeof(float));
    udpSocket->readDatagram((char *)buffer.data(), dataLen);
    qDebug() << "NEW DATA! Size: " << dataLen;
    emit newData(buffer);
}
