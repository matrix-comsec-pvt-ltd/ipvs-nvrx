#ifndef CONNECTREQUEST_H
#define CONNECTREQUEST_H
///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : NVR (Network Video Recorder)
//   Owner        : Aekam Parmar
//   File         : ConnectRequest.h
//   Description  : This module provides APIs to connect to NVR server.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QReadWriteLock>

#include "EnumFile.h"
#include "../GenericRequest/GenericRequest.h"

//******** Function Prototypes *******
class ConnectRequest : public GenericRequest
{
    Q_OBJECT

public:
    // constructor function, which creates and initializes object with server info, request info and auto login state
    ConnectRequest(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, quint16 forwardedTcpPort,
                   bool autoLogin = false, CONNECTION_TYPE_e connectionType = BY_IP_ADDRESS);
    // destructor function
    ~ConnectRequest();

    // run function, which is executed in system created thread.
    // it actually does the communication [login, polling, event request] with NVR device.
    void run();
    // stops the execution of the connectivity thread
    void setRunflag (bool flag);
    bool getRunflag();
    // sets the keep slive time
    void setKeepAliveTime (quint8 deviceTimeout);
    // sets the response time
    void setResponseTime (quint8 responseTime);
    // sets the session Id
    void setSessionId (QString sessionIdTemp);
    // sends signal to thread to proceed execution after login response.
    void sendPollSignal (void);
    // sets a flag which decides, to continue polling or not
    void setPollFlag (bool pollflag);
     bool getPollFlag();
    // set auto login state to false
    void UpdateAutoLogin (bool autologflag);

    // This is for local device autologin ...at this time username and password is local
    // but at deviceClient it is admin admin....so we need to update it at DeviceClient
    void getUsrnamePassword(QString &usrName, QString &password);

    void changeDeviceconfigForPortUpdate(quint16 port, quint16 forwardedTcpPort);
    quint8 getPortIndex();
    void setPortIndex(quint8 portIndex);

private:
    quint16 m_tcpPort[2];
    QReadWriteLock m_portLock;
    quint8 m_portIndex;
    QMutex pollLock;
    QMutex portIndexLock;

protected:
    // device connectivity state
    typedef enum
    {
        DEV_CONNECTED = 0,
        DEV_DISCONNECTED,
        MAX_DEV_CONN_STATE

    }DEVICE_CONN_STATE_e;

    // auto login flag
    bool autoLog;

    // flag to store thread run status
    bool runFlag;
    // indicater to switch to polling state
    bool pollDevFlag;

    // flag to not to send dev_disconnect second time
    bool isDevDesconnSend;

    // keep alive time, after which device is declared disconnected, if consecutive polling fails.
    quint8 keepAliveTime;

    QMutex autoLogLock;

    //store connection type
    CONNECTION_TYPE_e connType;

    // string to store username password
    QString usrPwd;
    // string to store smart code
    QString smartCode;

    // access lock for run flag
    QReadWriteLock runFlagLock;

    // condition signal to wake up the thread after login response
    QWaitCondition pollSignal;
    // condition signal lock
    QMutex pollSigLock;

    // api to get tcp port through local socket communication
    bool getTcpPort (quint16 &port);

    //api to get mac server info through local socket
    bool getMacServerInfo(QString *macServerIp, QString *macService, quint16 &macServerPort);

private:
    bool tryForwardedPort;

signals:
    // signal to convey the connectivity response
    void sigConnectResponse(REQ_MSG_ID_e requestId, DEVICE_REPLY_TYPE_e statusId, QString payload, QString ipAddr, quint16 tcpPort);
};

#endif // CONNECTREQUEST_H
