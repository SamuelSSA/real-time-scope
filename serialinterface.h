#ifndef SERIALINTERFACE_H
#define SERIALINTERFACE_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QDebug>
#include <QMutex>

class SerialInterface : public QSerialPort
{
    Q_OBJECT

public:
    explicit SerialInterface();
    ~SerialInterface();

    void closeSerialPort();
    void getInputData();
    bool openSerialPort(QString name);
    uint32_t sendData(std::string data);

signals:
    void newData(std::vector<uint8_t>);

private:
    QTimer timer;
    QByteArray buffer;
    QMutex mutex;

    void addDataToBuffer(const QByteArray &ba);
    void dispatchData();
};

#endif // SERIALINTERFACE_H
