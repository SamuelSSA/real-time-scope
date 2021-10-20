#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "plotwidget.h"
#include "udpinterface.h"

#include "encoderdecoder.h"
#include "inputManager.h"
#include "serialinterface.h"
#include "constants.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
      
    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
      
    private:
        void setupWidgets();
        void readConfig();
        void saveConfig();
        void plotExample();

        std::vector<uint32_t *> uintData;
        QVector<double> x0;
        InputManager inputManager;

        UDPInterface *udpIface;
        SerialInterface *serialIface;
        EncoderDecoder encoderDecoder;
        QTimer *timer;
        QVector<double> xdata;
        QVector<PlotWidget*> plotWidgets;
        QLineEdit *ipLineEdit;
        QLineEdit *portLineEdit;
        QTextEdit *textEdit;
        QDialogButtonBox *buttonBox;

        QComboBox *serialPortBox;
        QDialogButtonBox *buttonBoxSerial;

    public slots:
        void connectSerial();
        void connectUDP();
        void disconnectUDP();
        void disconnectSerial();
        void selectUDPSource();
        void selectDataType();
        void plotUdpData();
        void plotSerialData();
        void handleError(QSerialPort::SerialPortError error);
};

#endif // MAINWINDOW_H
