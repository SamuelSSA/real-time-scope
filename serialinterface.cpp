#include "serialinterface.h"

SerialInterface::SerialInterface()
{

}

SerialInterface::~SerialInterface()
{
    this->closeSerialPort();
}

void SerialInterface::closeSerialPort()
{
    if (this->isOpen())
    {
        disconnect(&this->timer, &QTimer::timeout, this, &SerialInterface::dispatchData);
        this->close();
    }
}

void SerialInterface::getInputData()
{
    if (this->bytesAvailable())
    {
        QByteArray data = this->readAll();
        this->addDataToBuffer(data);
    }
}

bool SerialInterface::openSerialPort(QString name)
{
    this->setPortName(name);
    this->setBaudRate(QSerialPort::Baud115200);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::StopBits::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);

    bool ret = this->open(QIODevice::ReadWrite);
    if (ret)
    {
        connect(this, &SerialInterface::readyRead, this, &SerialInterface::getInputData);
        connect(&this->timer, &QTimer::timeout, this, &SerialInterface::dispatchData);
        timer.start(100);
    }
    else
    {
        qDebug() << "Error: " <<  this->errorString();
    }

    return ret;
}

void SerialInterface::addDataToBuffer(const QByteArray &ba)
{
    this->mutex.lock();
    buffer.append(ba);
    this->mutex.unlock();
}

void SerialInterface::dispatchData()
{
    this->mutex.lock();
    if (this->buffer.size() > 0)
    {
        std::vector<uint8_t> data;
        for(auto i: qAsConst(this->buffer))
        {
            data.push_back(static_cast<uint8_t>(i));
        }

        emit newData(data);
        this->buffer.clear();
    }
    this->mutex.unlock();
}
