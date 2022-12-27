// author: Leonardo Seiji Oyama
// contact: leonardooyama@gmail.com

// old definition of Pi constant
#define PI 3.1415926535897932384626433832795

//XPLM libs
#include "XPLMCamera.h"
#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMNavigation.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMScenery.h"
#include "XPLMUtilities.h"

//Widgets libs
#include "XPStandardWidgets.h"
#include "XPUIGraphics.h"
#include "XPWidgetDefs.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"

//Wrappers libs
#include "XPCBroadcaster.h"
#include "XPCDisplay.h"
#include "XPCListener.h"
#include "XPCProcessing.h"
#include "XPCWidget.h"
#include "XPCWidgetAttachments.h"

#if IBM
#include <windows.h>
#endif

// Qt libs
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QDataStream>
#include <QDateTime>
#include <QString>
#include <QVector3D>

QUdpSocket *UDPSocketSender;
QUdpSocket *UDPSocketReceiver;


double lat = 0;
double lon = 0;
double elev = 0;
double startProbeAlt = 10000;
double X = 0;
double Y = 0;
double Z = 0;
XPLMProbeRef ProbeRef = NULL;

// variables to read data from X-Plane
XPLMDataRef DataRefFlightModelLocalX;
XPLMDataRef DataRefFlightModelLocalY;
XPLMDataRef DataRefFlightModelLocalZ;
XPLMDataRef DataRefFlightModelLat;
XPLMDataRef DataRefFlightModelLon;
XPLMDataRef DataRefFlightModelElev;

void DebugToXPlaneLog(QString debugString);
void ReadDataFromSocket();

std::string pluginName = "TeleportAircraft";
std::string pluginSignature = "coter.TeleportAircraft";
std::string pluginDescription = "X-Plane plugin to teleport the aircraft according to a command received via udp socket.";

// flight loop callback to do something in the future
float FlightLoopListenUDPSocket(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon);

// window id to interface inside X-Plane, not used yet
XPLMWindowID	gWindow = NULL;

// function to handle window interface inside X-Plane, not used yet
void MyDrawWindowCallback(XPLMWindowID inWindowID, void *inRefcon);
// function to handle keybord interactions inside X-Plane, not used yet
void MyHandleKeyCallback(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void * inRefcon, int losingFocus);
// function to handle mouse interactions inside X-Plane, not used yet
int MyHandleMouseClickCallback(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void * inRefcon);

PLUGIN_API int XPluginStart(char *	outName, char *	outSig, char *	outDesc)
{
    strcpy_s(outName, pluginName.size() + 1 , pluginName.c_str());
    strcpy_s(outSig, pluginSignature.size() + 1, pluginSignature.c_str());
    strcpy_s(outDesc, pluginDescription.size() + 1, pluginDescription.c_str());

    UDPSocketSender = new QUdpSocket();
    UDPSocketReceiver = new QUdpSocket();

    XPLMRegisterFlightLoopCallback(FlightLoopListenUDPSocket, xplm_FlightLoop_Phase_AfterFlightModel, NULL);

    DataRefFlightModelLocalX = XPLMFindDataRef("sim/flightmodel/position/local_x");
    DataRefFlightModelLocalY = XPLMFindDataRef("sim/flightmodel/position/local_y");
    DataRefFlightModelLocalZ = XPLMFindDataRef("sim/flightmodel/position/local_z");
    DataRefFlightModelLat = XPLMFindDataRef("sim/flightmodel/position/latitude");
    DataRefFlightModelLon = XPLMFindDataRef("sim/flightmodel/position/longitude");
    DataRefFlightModelElev = XPLMFindDataRef("sim/flightmodel/position/elevation");

    bool bindSuccess = UDPSocketReceiver->bind(QHostAddress::LocalHost, 15857, QAbstractSocket::ReuseAddressHint);
    QString debugString;
    if (bindSuccess)
    {
        debugString = "Success binding UDP port " + QString::number(15857) + ".";
        DebugToXPlaneLog(debugString);
    }
    else
    {
        debugString = "Failed binding UDP port " + QString::number(15857) + ".";
        DebugToXPlaneLog(debugString);
    }
    return 1;
}


PLUGIN_API void	XPluginStop(void)
{

}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(
        XPLMPluginID	inFromWho,
        long			inMessage,
        void *			inParam)
{
}

float FlightLoopListenUDPSocket(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
    if (UDPSocketReceiver->hasPendingDatagrams())
    {
        ReadDataFromSocket();
    }
    lat = XPLMGetDatad(DataRefFlightModelLat);
    lon = XPLMGetDatad(DataRefFlightModelLon);
    elev = XPLMGetDatad(DataRefFlightModelElev);
    QByteArray dataToSend;
    QDataStream outStream(&dataToSend, QIODevice::WriteOnly);
    char* pluginSig = new char[pluginSignature.size()+1];
    strcpy_s(pluginSig, pluginSignature.size()+1, pluginSignature.c_str());
    char* dataType = new char[sizeof("current")+1];
    strcpy_s(dataType, sizeof("current")+1, "current");
    outStream << pluginSig << dataType << lat << lon << elev;
    QNetworkDatagram datagramToSend;
    datagramToSend.setDestination(QHostAddress::LocalHost, 15858);
    datagramToSend.setData(dataToSend);
    UDPSocketSender->writeDatagram(datagramToSend);
    delete[] dataType;
    delete[] pluginSig;
    return -10;
}

void DebugToXPlaneLog(QString debugString)
{
    QString dbg;
    dbg = QDateTime::currentDateTime().toString("dd-MM-yyyy, hh'h' mm'min' ss's': ");
    dbg+= "[" + QString::fromStdString(pluginSignature) + "]: ";
    dbg+= debugString + "\n";
    XPLMDebugString(dbg.toStdString().c_str());
}

void ReadDataFromSocket()
{
    DebugToXPlaneLog("Got some data from UDPSocketReceiver!");
    QByteArray dataReceived = UDPSocketReceiver->receiveDatagram().data();
    QDataStream inStream(&dataReceived, QIODevice::ReadOnly);
    double receivedLat, receivedLon;
    char* pluginSig = new char[pluginSignature.size()+1];
    strcpy_s(pluginSig, pluginSignature.size()+1, pluginSignature.c_str());
    inStream >> pluginSig;
    QString plgSig = QString::fromStdString(pluginSig);
    QString debugString;
    if (plgSig.toStdString() != pluginSignature)
    {
        debugString = "Plugin signature wrong: " + QString::fromStdString(pluginSig);
        DebugToXPlaneLog(debugString);
        return;
    }
    inStream >> receivedLat;
    inStream >> receivedLon;
    debugString = "Received Latitude = " + QString::number(receivedLat, 'f', 6) + "; ";
    debugString += "Received Longitude = " + QString::number(receivedLon, 'f', 6);
    DebugToXPlaneLog(debugString);

    // send a datagram to the interface to notify that the message was received
    QByteArray dataToSend;
    QDataStream outStream(&dataToSend, QIODevice::WriteOnly);
    char* dataType = new char[sizeof("updated")+1];
    strcpy_s(dataType, sizeof("updated")+1, "updated");
    outStream << pluginSig << dataType << receivedLat << receivedLon;
    QNetworkDatagram datagramToSend;
    datagramToSend.setDestination(QHostAddress::LocalHost, 15858);
    datagramToSend.setData(dataToSend);
    UDPSocketSender->writeDatagram(datagramToSend);
    delete[] dataType;

    // create probe
    ProbeRef = XPLMCreateProbe(xplm_ProbeY);
    XPLMWorldToLocal(receivedLat, receivedLon, startProbeAlt, &X, &Y, &Z);
    XPLMProbeInfo_t info;
    info.structSize = sizeof(info);
    // Probe the terrain
    XPLMProbeResult result = XPLMProbeTerrainXYZ( ProbeRef, X, Y, Z, &info);
    if (result == 0) // The probe hit terrain and returned valid values.
    {
        double tempLat, tempLon, tempElev;
        XPLMLocalToWorld(info.locationX, info.locationY, info.locationZ, &tempLat, &tempLon, &tempElev);
        tempElev += 1.0;
        XPLMWorldToLocal(receivedLat, receivedLon, tempElev, &X, &Y, &Z);
        XPLMSetDatad(DataRefFlightModelLocalX, X); // writes to aicraft position X
        XPLMSetDatad(DataRefFlightModelLocalY, Y); // writes to aicraft position Y
        XPLMSetDatad(DataRefFlightModelLocalZ, Z); // writes to aicraft position Z
        DebugToXPlaneLog("Aircraft teleported!");
    }
    delete[] pluginSig;
}
