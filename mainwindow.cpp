#include "mainwindow.h"
#include <QFile>
#include <QTime>

#include <iostream>
#include <QDebug>
#include <QSerialPortInfo>

//TODO: Give parent to widgets to use QTs memory management

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent)
{
    this->setupWidgets();
    this->readConfig();

    this->timer = new QTimer(this);
    this->timer->setTimerType(Qt::PreciseTimer);

    udpIface = new UDPInterface(QHostAddress(ipLineEdit->text()),
                    portLineEdit->text().toInt(), this);

    serialIface = new SerialInterface();

    this->connect(buttonBox, &QDialogButtonBox::accepted, this, &MainWindow::connectUDP);
    this->connect(buttonBox, &QDialogButtonBox::rejected, this, &MainWindow::disconnectUDP);
    this->connect(buttonBoxSerial, &QDialogButtonBox::accepted, this, &MainWindow::connectSerial);
    this->connect(buttonBoxSerial, &QDialogButtonBox::rejected, this, &MainWindow::disconnectSerial);


    constexpr float _bw_chirp = 500e6;
    constexpr float _t_chirp = 0.125;
    constexpr float _c = 3e8;


    const float dist_rb =  (16 * (_t_chirp/2) * _c)/ (2 * _bw_chirp);

    x0.append(QVector<double>());
    x0.append(QVector<double>());
    x0.append(QVector<double>());

    int l = 0;
    for(int i = 0; i < GRAPHS; i++)
    {
        x0.append(QVector<double>());
        int k = 0;
        for(uint32_t j =0; j < (SAMPLES); j++)
        {
            if((i == 2) || (i == 3))
            //if(1)
            {
                //x0[i].append((SAMPLES*i + j)*dist_rb);
                x0[i].append((SAMPLES*l + k)*dist_rb);
                k++;
            }
            else
            {
                x0[i].append(j);
            }
        }
        if((i == 2) || (i == 3))
            l++;
    }
}

MainWindow::~MainWindow()
{
    this->saveConfig();
}

void MainWindow::plotSerialData()
{
    QVector<double> yData[GRAPHS];
    static QTime last_time;

    std::vector<uint8_t> rawData = this->inputManager.requestRawData(sizeof(DATA_PACKET));

    QTime time_now = QTime::currentTime();

    DATA_PACKET data_packet;
    if(rawData.size() > 0)
    {
        bool decodeStatus = this->encoderDecoder.decodeData(rawData, data_packet);
        if (decodeStatus)
        {
            qDebug() << last_time.msecsTo(time_now);
            this->inputManager.clearBuffer();
            for(int i = 0; i < this->inputManager.getInputsNum(); i++)
            {
                yData[i].reserve(SAMPLES);
                std::copy(data_packet.data_output[i], data_packet.data_output[i] + SAMPLES, std::back_inserter(yData[i]));

                if((i == 0) || (i == 1))
                    plotWidgets[i]->graph(0)->setData(x0[i],yData[i],true);
                else
                    plotWidgets[i]->graph(0)->setData(x0[i],yData[i],true);

                plotWidgets[i]->replot();
            }
            last_time = QTime::currentTime();
        }
    }
}

void MainWindow::plotUdpData()
{
    QVector<double> yData[GRAPHS];
    std::vector<float> buffer;

    for(int i = 0; i < this->inputManager.getInputsNum(); i++)
    {
        buffer = this->inputManager.getData(i);

        if(!buffer.empty())
        {
            yData[i].reserve(buffer.size());
            std::copy(buffer.begin(), buffer.end(), std::back_inserter(yData[i]));

            plotWidgets[i]->graph(0)->setData(x0[i],yData[i],true);
            plotWidgets[i]->replot();
        }
    }
}

void MainWindow::connectSerial()
{
    if(this->serialIface->openSerialPort(serialPortBox->currentText()))
    {
        this->connect(this->serialIface, &SerialInterface::newData, &this->inputManager, &InputManager::addRawDataToMemory);
        this->connect(this->serialIface, &SerialInterface::errorOccurred, this, &MainWindow::handleError);      
        this->connect(timer, &QTimer::timeout, this, &MainWindow::plotSerialData);

        this->disconnectUDP();

        qDebug() << "Serial opened";
        this->timer->setInterval(100);
        this->timer->start();
    }
    else
    {
        qDebug() << "Serial not opened";
    }
}

void MainWindow::connectUDP()
{
    if(udpIface->bind())
    {
        this->connect(udpIface, &UDPInterface::newData, &this->inputManager, &InputManager::addToMemory);
        this->connect(timer, &QTimer::timeout, this, &MainWindow::plotUdpData);

        this->disconnectSerial();

        qDebug() << "UDP connected";
        this->timer->setInterval(100);
        this->timer->start();
    }
    else
    {
        QMessageBox::critical(this,"Error!",
            "Could not connect to Host!",
            QMessageBox::Ok);
    }
}

void MainWindow::disconnectUDP()
{
    if( udpIface->isConnected() )
    {
        this->disconnect(udpIface, &UDPInterface::newData, this, &MainWindow::plotUdpData);
        this->disconnect(udpIface, &UDPInterface::newData, &this->inputManager, &InputManager::addToMemory);
        this->disconnect(timer, &QTimer::timeout, this, &MainWindow::plotUdpData);

        udpIface->unbind();
        qDebug() << "UDP disconnected";

        if(this->timer->isActive())
            this->timer->stop();
    }
    // Reanable widgets
    this->selectUDPSource();
}

void MainWindow::disconnectSerial()
{
    this->disconnect(this->serialIface, &SerialInterface::newData, &this->inputManager, &InputManager::addRawDataToMemory);
    this->disconnect(this->serialIface, &SerialInterface::errorOccurred, this, &MainWindow::handleError);

    this->serialIface->closeSerialPort();
    qDebug() << "Serial disconnected.";
}

void MainWindow::selectUDPSource()
{
    ipLineEdit->setEnabled(true);
    portLineEdit->setEnabled(true);
}

void MainWindow::selectDataType()
{

}

void MainWindow::readConfig()
{
    QFile xmlFile("gui-config.xml");

    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,"Load XML File Problem",
            "Couldn't open gui-config.xml to load settings",
            QMessageBox::Ok);
    }
    else
    {
        QXmlStreamReader xmlReader(&xmlFile);
        
        while(!xmlReader.atEnd() && !xmlReader.hasError())
        {
            QXmlStreamReader::TokenType token = xmlReader.readNext();

            if(token == QXmlStreamReader::StartElement)
            {
                if(xmlReader.name() == "ipaddr")
                {
                    ipLineEdit->setText(xmlReader.readElementText());
                }

                if(xmlReader.name() == "port")
                {
                    portLineEdit->setText(xmlReader.readElementText());
                }
            }
            if(xmlReader.hasError())
            {
                QMessageBox::critical(this,
                "gui-config.xml Parse Error",xmlReader.errorString(),
                QMessageBox::Ok);
            }
        }

        xmlFile.close();
    }
 
}

void MainWindow::saveConfig()
{
    QFile xmlFile("gui-config.xml");

    if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QXmlStreamWriter xmlWriter(&xmlFile);
        xmlWriter.setAutoFormatting(true);

        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement("n310-scope-config");
        xmlWriter.writeTextElement("ipaddr", ipLineEdit->text());
        xmlWriter.writeTextElement("port", portLineEdit->text());
        
        xmlWriter.writeEndElement();
        
        xmlFile.close();
    }
}

void MainWindow::setupWidgets()
{
    setWindowTitle(tr("Visualizador Radar"));
    this->showMaximized();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    QWidget *centralWidget = new QWidget();

    // Scopes Box
    QGroupBox *plotsBox = new QGroupBox(tr("Scopes"));
    QVBoxLayout *plotsLayout = new QVBoxLayout();

    for(int i=0; i < N_SCOPES; i++)
    {
        if((i == 2) || (i == 3))
            plotWidgets.append(new PlotWidget("Distância[metros]","Magnitude[Volts]"));
        else
            plotWidgets.append(new PlotWidget("Amostras", "Magnitude[Volts]"));

        plotsLayout->addWidget(plotWidgets[i]);
        //plotWidgets[i]->setMargins(0, N_SAMPLES, 0, qPow(2,N_BITS-1)-1);
        plotWidgets[i]->setMargins(0, N_SAMPLES, -100, 5);
        plotWidgets[i]->yAxis->setRange(0, 6000);
    }

    plotsBox->setLayout(plotsLayout);

    // Footer Box
    QGroupBox *footerBox = new QGroupBox();
    QGroupBox *textEditBox = new QGroupBox(tr("Mensagens"));
    QGroupBox *netConfBox = new QGroupBox(tr("Configuração de Rede"));
    QGroupBox *serialConfBox = new QGroupBox(tr("Configuração Serial"));
    QGridLayout *footerLayout = new QGridLayout();
    
    footerLayout->addWidget(textEditBox, 0, 0);
    footerLayout->addWidget(netConfBox, 0, 1);
    footerLayout->addWidget(serialConfBox,0,2);
    footerBox->setLayout(footerLayout);
    footerBox->setMaximumHeight(250);

    // Text Edit Box
    QVBoxLayout *textEditLayout = new QVBoxLayout();
    textEdit = new QTextEdit();

    textEditLayout->addWidget(textEdit);
    textEditBox->setLayout(textEditLayout);

    // Networt Configuration Box
    QVBoxLayout *netLayout = new QVBoxLayout(this);
    QGroupBox *netBox = new QGroupBox(this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Conectar");
    buttonBox->button(QDialogButtonBox::Cancel)->setText("Disconectar");
    buttonBox->setCenterButtons(true);


    ipLineEdit = new QLineEdit(this);
    portLineEdit = new QLineEdit(this
                                 );
    ipLineEdit->setMaximumWidth(150);
    portLineEdit->setMaximumWidth(150);

    QGridLayout *gridlayout = new QGridLayout;
    gridlayout->addWidget(new QLabel(tr("Endereço IP")), 0, 0);
    gridlayout->addWidget(ipLineEdit, 0, 1);
    gridlayout->addWidget(new QLabel(tr("Porta")), 1, 0);
    gridlayout->addWidget(portLineEdit, 1, 1);
    gridlayout->setVerticalSpacing(5);

    netBox->setLayout(gridlayout);
    netLayout->addWidget(netBox);
    netLayout->addWidget(buttonBox);
    netConfBox->setLayout(netLayout);

    buttonBoxSerial = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
    buttonBoxSerial->button(QDialogButtonBox::Ok)->setText("Abrir");
    buttonBoxSerial->button(QDialogButtonBox::Cancel)->setText("Fechar");
    buttonBoxSerial->setCenterButtons(true);


    serialPortBox = new QComboBox(this);
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        serialPortBox->addItem(port.portName());
    }

    QGridLayout *serialGridLayout = new QGridLayout;
    serialGridLayout->addWidget(new QLabel(tr("Porta Serial")),0,0);
    serialGridLayout->addWidget(serialPortBox,0,1);
    serialGridLayout->setHorizontalSpacing(5);

    QVBoxLayout *serialVLayout = new QVBoxLayout(this);
    QGroupBox *serialBox = new QGroupBox(this);
    serialBox->setLayout(serialGridLayout);
    serialVLayout->addWidget(serialBox);
    serialVLayout->addWidget(buttonBoxSerial);
    serialConfBox->setLayout(serialVLayout);

    // Main Window Widgets
    mainLayout->addWidget(plotsBox);
    mainLayout->addWidget(footerBox);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), this->serialIface->errorString());
        this->serialIface->closeSerialPort();
    }
}
