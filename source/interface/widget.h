#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void handlePushButtomSendClicked();
    void readPendingDatagrams();
    void checkPluginStatusTimerTimeout();
    void handleBytesWritten(qint64 bytes);
private:
    Ui::Widget *ui;
    QUdpSocket *UDPSocketSender;
    QUdpSocket *UDPSocketReceiver;
    QDateTime lastUdpDatagramReceivedTime, appStartTime;
    bool pluginConnected;
    std::string pluginSignature = "coter.TeleportAircraft";
};
#endif // WIDGET_H
