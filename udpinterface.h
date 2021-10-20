#ifndef UDPINTERFACE_H
#define UDPINTERFACE_H

#include <QObject>
#include <QUdpSocket>
#include <memory>

#define BUFF_SIZE  65535

class UDPInterface : public QObject
{
Q_OBJECT

    public:
        explicit UDPInterface(QHostAddress ip, quint16 port, QObject *parent = NULL);
        ~UDPInterface();

         bool bind();
         void unbind();
         bool isConnected();
    
    signals:
        void newData(std::vector<float>);

    private:
         bool isConnectedFlag;
         QHostAddress ip;
         quint16 port;
         QUdpSocket *udpSocket;
    
    private slots:
        void receivedData();
};

#endif // UDPINTERFACE_H
