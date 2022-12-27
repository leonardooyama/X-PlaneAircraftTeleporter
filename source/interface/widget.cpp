#include "widget.h"
#include "ui_widget.h"

#include <QNetworkDatagram>
#include <QTimer>
#include <QDataStream>
#include <QDoubleValidator>
#include <QLocale>
#include <QStyle>

#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    //-29.7403273288, -53.8378558848 // Campo de Instrução de Santa Maria
    ui->lineEdit_Latitude->setText("-29.7403273288");
    ui->lineEdit_Longitude->setText("-53.8378558848");
    QDoubleValidator *latValidator = new QDoubleValidator(-90, 90, 15, this);
    latValidator->setNotation(QDoubleValidator::StandardNotation);
    QDoubleValidator *lonValidator = new QDoubleValidator(-180, 180, 15, this);
    lonValidator->setNotation(QDoubleValidator::StandardNotation);
    QLocale UnitedStates(QLocale::English, QLocale::UnitedStates);
    latValidator->setLocale(UnitedStates);
    lonValidator->setLocale(UnitedStates);
    ui->lineEdit_Latitude->setValidator(latValidator);
    ui->lineEdit_Longitude->setValidator(lonValidator);

    UDPSocketSender = new QUdpSocket(this);
    UDPSocketReceiver = new QUdpSocket(this);
    bool bindSuccess = UDPSocketReceiver->bind(QHostAddress::LocalHost, 15858, QAbstractSocket::ReuseAddressHint);
    QString bindStatusString;

    if (bindSuccess)
    {
        bindStatusString = "Success binding UDP port " + QString::number(15858) + ".";
    }
    else
    {
        bindStatusString = "Failed binding UDP port " + QString::number(15858) + ".";
    }
    ui->label_SendStatus->setText(bindStatusString);
    ui->label_CurrentLatLongElev->setText("No data received.");
    connect(ui->pushButton_Send, &QPushButton::clicked, this, &Widget::handlePushButtomSendClicked);
    connect(UDPSocketReceiver, &QUdpSocket::readyRead, this, &Widget::readPendingDatagrams);
    connect(UDPSocketSender, &QUdpSocket::bytesWritten, this, &Widget::handleBytesWritten);
    QStyle *style = QApplication::style();
    ui->label_ConnectionStatusIcon->setPixmap(style->standardIcon(QStyle::SP_DialogNoButton, 0, this).pixmap(QSize(35,35)));
    ui->label_ConnectionStatusText->setText("X-Plane not connected.");
    pluginConnected = false;
    QTimer *checkPluginStatus = new QTimer(this);
    connect(checkPluginStatus, &QTimer::timeout, this, &Widget::checkPluginStatusTimerTimeout);
    checkPluginStatus->start(500);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::handlePushButtomSendClicked()
{
    ui->pushButton_Send->setEnabled(false);
    QByteArray dataToSend;
    QDataStream outStream(&dataToSend, QIODevice::WriteOnly);
    char* pluginSig = new char[pluginSignature.size()+1];
    strcpy_s(pluginSig, pluginSignature.size()+1, pluginSignature.c_str());
    outStream << pluginSig << ui->lineEdit_Latitude->text().toDouble() << ui->lineEdit_Longitude->text().toDouble();
    QNetworkDatagram datagramToSend;
    datagramToSend.setDestination(QHostAddress::LocalHost, 15857);
    datagramToSend.setData(dataToSend);
    UDPSocketSender->writeDatagram(datagramToSend);
    delete[] pluginSig;
}

void Widget::readPendingDatagrams()
{
    while (UDPSocketReceiver->hasPendingDatagrams())
    {
        QByteArray dataReceived = UDPSocketReceiver->receiveDatagram().data();
        QDataStream inStream(&dataReceived, QIODevice::ReadOnly);
        char* pluginSig = new char[pluginSignature.size()+1];
        strcpy_s(pluginSig, pluginSignature.size()+1, pluginSignature.c_str());
        char* dataType = new char[sizeof("updated")+1];
        double receivedLat, receivedLon, receivedElev;
        inStream >> pluginSig >> dataType >> receivedLat >> receivedLon;
        if (pluginSig == pluginSignature)
        {
            lastUdpDatagramReceivedTime = QDateTime::currentDateTime();
        }
        else
        {
            return;
        }
        QString dataTypeStr = QString::fromStdString(dataType);
        if (dataTypeStr == "current")
        {
            inStream >> receivedElev;
            QString currentLocation = "Current location: Lat = " + QString::number(receivedLat, 'f', 10);
            currentLocation += "; Lon = " +  QString::number(receivedLon, 'f', 10);
            currentLocation += "; Elev = " +  QString::number(receivedElev, 'f', 10);
            ui->label_CurrentLatLongElev->setText(currentLocation);
        }
        if (dataTypeStr == "updated")
        {
            QString updatedLocation = "Aircraft teleported to location: Lat = " + QString::number(receivedLat, 'f', 10);
            updatedLocation += "; Lon = " +  QString::number(receivedLon, 'f', 10);
            ui->label_SendStatus->setText(updatedLocation);
        }
        delete[] pluginSig;
    }
}

void Widget::handleBytesWritten(qint64 bytes)
{
    ui->pushButton_Send->setEnabled(true);
    ui->pushButton_Send->setFocus();
    qDebug() << "Bytes writen: " << bytes;
}

void Widget::checkPluginStatusTimerTimeout()
{
    qint64 timeSinceLastReceivedDatagram = lastUdpDatagramReceivedTime.msecsTo(QDateTime::currentDateTime());
    QStyle *style = QApplication::style();
    if (timeSinceLastReceivedDatagram < 2500 && timeSinceLastReceivedDatagram != 0)
    {
        ui->label_ConnectionStatusIcon->setPixmap(style->standardIcon(QStyle::SP_DialogYesButton, 0, this).pixmap(QSize(35,35)));
        ui->label_ConnectionStatusText->setText("X-Plane connected!");
    }
    else
    {
        ui->label_ConnectionStatusIcon->setPixmap(style->standardIcon(QStyle::SP_DialogNoButton, 0, this).pixmap(QSize(35,35)));
        ui->label_ConnectionStatusText->setText("X-Plane not connected.");
    }
}
