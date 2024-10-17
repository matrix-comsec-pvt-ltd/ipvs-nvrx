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
//   Project      : NVR ( Network Video Recorder)
//   Owner        : Aekam Parmar
//   File         : ConnectRequest.cpp
//   Description  : This module provides APIs to connect to NVR server.
//                  It does the login and polls the server periodically
//                  to keep the connectivity.
//                  It also makes event request, if any avalable.
//
/////////////////////////////////////////////////////////////////////////////
#include "ConnectRequest.h"
#include <sys/prctl.h>
#include <QLocalSocket>
#include <QStringList>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QEventLoop>

//******** Defines and Data Types ****
#define MAX_LOGIN_TIMEOUT       20   // in seconds
#define LOCAL_SOCKET_FILE       "/tmp/IntSocket"
#define GET_PORT_REQUEST        "CGI_REQ_GETPORT"
#define GET_MAC_SERVER_CFG      "GET_MAC_SERVER_CFG"
#define LOCAL_IP_ADDRESS        "127.0.0.1"

//*****************************************************************************
//  ConnectRequest()
//      Param:
//          IN : SERVER_INFO_t serverInfo   // server informations
//               REQ_INFO_t requestInfo     // request information
//               BOOL autoLogin             // auto login state
//          OUT: NONE
//	Returns:
//		Not Applicable
//      Description:
//          This API is constructor of class ConnectRequest. It initializes the
//          newly created object with server, request and auto login state.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
ConnectRequest::ConnectRequest(SERVER_INFO_t deviceInfo, REQ_INFO_t &requestInfo, quint16 forwardedTcpPort,
                               bool autoLogin, CONNECTION_TYPE_e connectionType)
    : GenericRequest (deviceInfo, requestInfo)
{
    // set poll start flag to false
    setPollFlag(false);
    // set thread run flag to true
    runFlag = true;
    // set auto login status
    autoLog = autoLogin;
    // keep a copy of username-password
    usrPwd = requestInfo.payload;
    // keep a copy of smart code
    smartCode = requestInfo.sessionId;

    m_tcpPort[0] = deviceInfo.tcpPort;
    m_tcpPort[1] = forwardedTcpPort;

    //keep a copy of connection type
    connType = connectionType;
    isDevDesconnSend = false;

    m_portLock.lockForWrite();
    tryForwardedPort = false;
    m_portLock.unlock();
    keepAliveTime = 0;
    m_portIndex= 0;
}

//*****************************************************************************
//  ~ConnectRequest()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//	Returns:
//		Not Applicable
//      Description:
//          This API is distructor of class CommandRequest.
//          As of now it does no functionality, kept for future use..
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
ConnectRequest::~ConnectRequest()
{

}

//*****************************************************************************
//  run()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          The starting point for the thread. After calling start(), the newly
//          created thread calls this function.
//          This API sends login request, if successfull, starts polling
//          periodically.
//          It also makes event request, if any live event available.
//          It emits signal whenever there is a change in connectivity state.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::run()
{
    bool        status;
    bool        tcpPortChangeReLoginFlag = false;
    QTcpSocket  tcpSocket;
    qint8       localDevTimeOut = 0;
    QString     macSerIp = "";
    QString     macService = "";
    quint16     macPort = 80;
    QString     reqUrl = "";
    QString     ipTemp1 = server.ipAddress;
    QString     ipTemp = "";
    QStringList macTempList;
    qint32      startIndex, endIndex;
    QByteArray  tempArray;
    QByteArray  trueUrlFormat = "<Request Successful";
    quint8      portIndex;
    quint8      requestCount;

    //create networkAccessManager instance to url req
    QNetworkAccessManager   manager;
    QNetworkReply           *reply;

    DEVICE_CONN_STATE_e currDevState = MAX_DEV_CONN_STATE;
    DEVICE_CONN_STATE_e prevDevState = MAX_DEV_CONN_STATE;

    m_portLock.lockForWrite();
    tryForwardedPort = false;
    m_portLock.unlock();

    // Take index for Port value taken for communication
    setPortIndex(0);

    /* Add server address last six characters to identify thread */
    QString threadName = (server.ipAddress == LOCAL_IP_ADDRESS) ? "CONN_REQ_LOCAL" : "CONN_REQ_" + server.ipAddress.right(6);
    prctl(PR_SET_NAME, threadName.toUtf8().constData(), 0, 0, 0);

    DPRINT(STREAM_REQ, "connect request: [ip=%s]", server.ipAddress.toUtf8().constData());

    // loop till auto login is true and response status is other than timeout or connectivity
    while(true)
    {
        autoLogLock.lock();
        if (autoLog == false)
        {
            autoLogLock.unlock();
            break;
        }
        autoLogLock.unlock();

        m_portLock.lockForWrite();
        tryForwardedPort = false;
        m_portLock.unlock();

        //check for connection type
        if((connType == BY_MATRIX_MAC) || (connType == BY_MATRIX_HOSTNAME))
        {
            // remove : field from ip address
            macTempList = ipTemp1.split (':');
            ipTemp = macTempList.join ("");

            //get MAC server ip & port
            status = getMacServerInfo(&macSerIp, &macService, macPort);
            if(status != false)
            {
                reqUrl.append("http://");
                reqUrl.append(macSerIp);
                reqUrl.append(QString(":%1/").arg (macPort));
                reqUrl.append(macService);
                reqUrl.append((connType == BY_MATRIX_MAC) ? "/get_addr" : "/get_addr_h");
                reqUrl.append("?smart_code=28574;action=get;");
                reqUrl.append((connType == BY_MATRIX_MAC) ? "address=" : "host_name=");
                reqUrl.append(ipTemp);

                //connect finished req signal to slot
                connect(&manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(quit()));

                //send http req
                reply = manager.get(QNetworkRequest(QUrl(reqUrl)));

                //start local event loop because we have to wait for mac
                //server response
                exec();
                reqUrl.clear();

                //disconnect signsl and slot
                disconnect (&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(quit()));

                //read response from server
                tempArray = reply->readAll();

                if(tempArray.startsWith (trueUrlFormat) == true)
                {
                    //for fetch ip address from response
                    startIndex = tempArray.indexOf ('=');
                    endIndex = tempArray.indexOf ('>');
                    ipTemp = tempArray.mid ((startIndex + 1) , (endIndex - startIndex -1));
                    server.ipAddress = ipTemp;
                }

                delete reply;
            }
        }

        isDevDesconnSend = false;
        if (server.ipAddress == LOCAL_IP_ADDRESS)
        {
            getTcpPort (server.tcpPort);
        }

        // loop till the thread run flag is true
        while (true)
        {
            // lock access to thread run flag
            runFlagLock.lockForRead();
            if (runFlag == false)
            {
                // unlock access to thread run flag
                runFlagLock.unlock();
                break;
            }

            if (server.ipAddress != LOCAL_IP_ADDRESS)
            {
                m_portLock.lockForRead();
                server.tcpPort = (tryForwardedPort == false) ? m_tcpPort[0] : m_tcpPort[1];
                m_portLock.unlock();
            }

            // unlock access to thread run flag
            runFlagLock.unlock();

            // connect to the server
            status = connectToServer (tcpSocket);
            if ((status == false) && (runFlag == true) && (tryForwardedPort == false))
            {
                m_portLock.lockForWrite();
                tryForwardedPort = true;
                m_portLock.unlock();
                tcpSocket.close();
                continue;
            }
            else if((status == true) && (server.ipAddress != LOCAL_IP_ADDRESS))
            {
                m_portLock.lockForRead();
                portIndex = (server.tcpPort == m_tcpPort[0]) ? (0) : (1);
                m_portLock.unlock();

                setPortIndex(portIndex);
            }
            else if((status == false) && (tryForwardedPort == true) && (runFlag == true))
            {
                m_portLock.lockForWrite();
                tryForwardedPort = false;
                m_portLock.unlock();
            }            

            // if connection established successfully
            if (status == true)
            {
                // send request to server
                status = sendRequest (tcpSocket);

                // if request send to server successfully
                if (status == true)
                {
                    // receive response from server
                    status = receiveResponse (tcpSocket);
                }
            }

            runFlagLock.lockForRead();
            if ((status == false) && (tcpSocket.state() == QAbstractSocket::UnconnectedState) && (runFlag == true))
            {
                requestCount = request.timeout;
                runFlagLock.unlock();
                while((requestCount > 0) && (getRunflag() == true))
                {
                    // no need to sleep() whole timeout sec. so check every 1 sec interval check.helping in shutdown time.
                    sleep(1);
                    requestCount--;
                }
            }
            else
            {
                runFlagLock.unlock();
                // disconnect from host
                tcpSocket.disconnectFromHost();
            }
            tcpSocket.close();

            // Here, we are checking runflag once more,
            // because in case of disconnected device, connectReq will take 7 sec,
            // and in between, if device config is change,then it is expected that
            // this run() ends instantly.

            // If we not do this, then this run() will block at pollSignal.wait()
            // because deviceclient is not sending pollsignal due to its block in
            // deleteconnectreq-wait()........

            runFlagLock.lockForRead();
            if (runFlag == false)
            {
                runFlagLock.unlock();
                // This is dummy response state... just to delete Connect req at device client
                statusId = CMD_MAX_DEVICE_REPLY;
                continue;
            }
            runFlagLock.unlock();

            // check the request Id that was sent to server
            switch (request.requestId)
            {
                case MSG_REQ_LOG:
                {
                    // lock access to polling signal variable
                    pollSigLock.lock();

                    // emit signal to notify status of login request
                    emit sigConnectResponse (MSG_REQ_LOG, statusId, request.payload, server.ipAddress, server.tcpPort);

                    // wait for polling signal
                    pollSignal.wait (&pollSigLock);

                    // unlock access to polling signal variable
                    pollSigLock.unlock();

                    // if polling start flag is true
                    if (getPollFlag() == true)
                    {
                        // set polling start flag to false
                        setPollFlag(false);
                        // set request Id to polling request
                        request.requestId = MSG_REQ_POL;
                        // load kep alive time to local variable
                        localDevTimeOut = keepAliveTime;
                        prevDevState = DEV_CONNECTED;
                    }
                    // else
                    else
                    {
                        setRunflag (false);
                    }

                    // clear payload
                    request.payload.clear();
                }
                break;

                case MSG_REQ_POL:
                {
                    // if connection not established or request timeout occured
                    if ((statusId == CMD_SERVER_NOT_RESPONDING) || (statusId == CMD_DEV_DISCONNECTED))
                    {
                        statusId = CMD_DEV_DISCONNECTED;
                        // deduct response timeout from keep alive time
                        localDevTimeOut -= request.timeout;

                        if (server.ipAddress == LOCAL_IP_ADDRESS)
                        {
                            quint16 tmpTcpPort = 0;
                            if ((true == getTcpPort(tmpTcpPort)) && (tmpTcpPort != server.tcpPort))
                            {
                                tcpPortChangeReLoginFlag = true;
                                WPRINT(STREAM_REQ, "local tcp port changed. Try to re-login: [oldPort=%d], [newPort=%d]", server.tcpPort, tmpTcpPort);
                                currDevState = DEV_DISCONNECTED;
                                setRunflag (false);
                            }
                        }

                        // polling failed till keep alive time
                        if (localDevTimeOut <= 0)
                        {
                            DPRINT(STREAM_REQ, "Keep alive timer expire: [ip=%s], [port=%d]", server.ipAddress.toUtf8().constData(), server.tcpPort);
                            // set device state to disconnected
                            currDevState = DEV_DISCONNECTED;
                            setRunflag (false);
                        }
                    }
                    else
                    {
                        // reload keep alive time
                        localDevTimeOut = keepAliveTime;

                        // set device state to connected
                        currDevState = DEV_CONNECTED;

                        // if request status is event available
                        if (statusId == CMD_EVENT_AVAILABLE)
                        {
                            // set request Id to event request
                            request.requestId = MSG_REQ_EVT;
                        }

                        statusId = CMD_SUCCESS;
                    }

                    // if current device state is not same as previous device state
                    if (currDevState != prevDevState)
                    {
                        // set current device state to previous device state
                        prevDevState = currDevState;

                        // emit signal to notify status of the operation
                        emit sigConnectResponse (MSG_REQ_POL, statusId, request.payload, server.ipAddress, server.tcpPort);

                        if(statusId == CMD_DEV_DISCONNECTED)
                        {
                            isDevDesconnSend = true;
                        }
                    }

                    // clear payload
                    request.payload.clear();
                }
                break;

                case MSG_REQ_EVT:
                {
                    // emit signal to notify status of the operation
                    emit sigConnectResponse (MSG_REQ_EVT, statusId, request.payload, server.ipAddress, server.tcpPort);

                    // clear payload
                    request.payload.clear();

                    // set request Id to polling request
                    request.requestId = MSG_REQ_POL;
                }
                break;

                default:
                {
                    setRunflag (false);
                }
                break;
            }
        }

        currDevState = DEV_DISCONNECTED;
        if(isDevDesconnSend == false)
        {
            emit sigConnectResponse (MSG_REQ_POL, statusId, request.payload, server.ipAddress, server.tcpPort);
            isDevDesconnSend = true;
        }

        // reinitialization of login parameters, in case if auto login is enabled.
        request.requestId = MSG_REQ_LOG;
        request.payload = usrPwd;
        request.sessionId = smartCode;

        autoLogLock.lock();
        if (autoLog == false)
        {
            autoLogLock.unlock();
            break;
        }
        autoLogLock.unlock();

        /* immediately try for login when tcp port is changed */
        if (tcpPortChangeReLoginFlag == true)
        {
            // set run flag to true for next run in case if auto login is enabled
            setRunflag(true);
            tcpPortChangeReLoginFlag = false;
            continue;
        }

        for (quint8 secCnt = 0; secCnt < MAX_LOGIN_TIMEOUT; secCnt++)
        {
            sleep(1);
            autoLogLock.lock();
            if (autoLog == false)
            {
                autoLogLock.unlock();
                break;
            }
            autoLogLock.unlock();
        }

        // set run flag to true for next run in case if auto login is enabled
        setRunflag (true);
    }

    // close tcp socket
    tcpSocket.close();

    emit sigConnectResponse (MSG_REQ_POL, statusId, request.payload, server.ipAddress, server.tcpPort);
}

//****************************************************************************
//  stopConnThread()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          This API sets flag to stop the connectivity thread.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::setRunflag (bool flag)
{
    // lock write access to thread run flag
    runFlagLock.lockForWrite();
    // set thread run flag
    runFlag = flag;
    // unlock access to thread run flag
    runFlagLock.unlock();

    setStopFlag(!flag);
}

bool ConnectRequest::getRunflag()
{
    runFlagLock.lockForRead();
    bool flag = runFlag;
    runFlagLock.unlock();
    return flag;
}


//*****************************************************************************
//  setKeepAliveTime()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          This API sets keep alive time.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::setKeepAliveTime (quint8 deviceTimeout)
{
    // store keep alive time
    keepAliveTime = deviceTimeout;
}

//*****************************************************************************
//  setResponseTime()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          This API sets response time.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::setResponseTime (quint8 responseTime)
{
    // store response time
    request.timeout = responseTime;
}

//*****************************************************************************
//  setSessionId()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          This API sets sessionId.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::setSessionId (QString sessionIdTemp)
{
    // store session id
    request.sessionId = sessionIdTemp;
}
//*****************************************************************************
//  setPollFlag()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          This API sets polling flag.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::setPollFlag (bool pollFlag)
{
    // store poll start flag
    pollLock.lock();
    pollDevFlag = pollFlag;
    pollLock.unlock();
}

bool ConnectRequest::getPollFlag()
{
    // store poll start flag
    pollLock.lock();
    bool pollFlag = pollDevFlag;
    pollLock.unlock();
    return pollFlag;
}

void ConnectRequest::UpdateAutoLogin (bool autologflag)
{
    autoLogLock.lock();
    autoLog = autologflag;
    autoLogLock.unlock();
}

void ConnectRequest::getUsrnamePassword(QString &usrName, QString &password)
{
    QStringList list = usrPwd.split (FSP);
    usrName = list.at (0);
    password = list.at (1);
}

//*****************************************************************************
//  sendPollSignal()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		NONE
//      Description:
//          This API sends polling condition signal.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void ConnectRequest::sendPollSignal (void)
{
    // send polling signal
    pollSigLock.lock();
    pollSignal.wakeAll();
    pollSigLock.unlock();
}

//*****************************************************************************
//  getTcpPort()
//      Param:
//          IN : NONE
//          OUT: quint8& port
//	Returns:
//		NONE
//      Description:
//          This API sends polling condition signal.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool ConnectRequest::getTcpPort (quint16 &port)
{
    bool status;
    QLocalSocket localSocket;
    QByteArray getPortReq;
    QList<QByteArray> tempArray;
    qint64 numOfBytes;

    localSocket.connectToServer (LOCAL_SOCKET_FILE, QIODevice::ReadWrite);
    status = localSocket.waitForConnected (request.timeout);
    if (status == false)
    {
        return false;
    }

    getPortReq.append (SOM);
    getPortReq.append (GET_PORT_REQUEST);
    getPortReq.append (FSP);
    getPortReq.append (EOM);

    numOfBytes = localSocket.write (getPortReq);
    if (numOfBytes >= getPortReq.length())
    {
        getPortReq.clear();
        status = localSocket.waitForReadyRead (request.timeout);
        if (status == true)
        {
            getPortReq.append (localSocket.readAll());

            if ((getPortReq.startsWith(SOM) == true) && (getPortReq.endsWith(EOM) == true))
            {
                // split data to get response string
                tempArray = getPortReq.split (FSP);
                if (tempArray[0].contains (GET_PORT_REQUEST) == true)
                {
                    port = tempArray[1].toInt();
                }
            }
        }
    }

    localSocket.disconnectFromServer();
    return status;
}

//*****************************************************************************
//  getMacServerInfo()
//      Param:
//          IN : NONE
//          OUT: QString *macIp
//               QString *macServ,
//               quint16& port
//	Returns:
//		NONE
//      Description:
//          This API sends polling condition signal.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool ConnectRequest::getMacServerInfo (QString *macIp, QString *macServ, quint16 &macSerPort)
{
    bool status = false;
    QLocalSocket localSocket;
    QByteArray getDataReq;
    QList<QByteArray> tempArray;
    qint64 numOfBytes;

    localSocket.connectToServer (LOCAL_SOCKET_FILE, QIODevice::ReadWrite);
    status = localSocket.waitForConnected (request.timeout);
    if (status == false)
    {
        return false;
    }

    getDataReq.append(SOM);
    getDataReq.append(GET_MAC_SERVER_CFG);
    getDataReq.append(FSP);
    getDataReq.append(EOM);

    numOfBytes = localSocket.write (getDataReq);
    if (numOfBytes >= getDataReq.length())
    {
        getDataReq.clear();
        status = localSocket.waitForReadyRead (request.timeout);
        if (status == true)
        {
            getDataReq.append (localSocket.readAll());
            if ((getDataReq.startsWith (SOM) == true) && (getDataReq.endsWith (EOM) == true))
            {
                // split data to get response string
                tempArray = getDataReq.split (FSP);
                if (tempArray[0].contains (GET_MAC_SERVER_CFG) == true)
                {
                    *macIp = tempArray[1];
                    macSerPort = tempArray[2].toInt();
                    *macServ = tempArray[3];
                }
            }
        }
    }

    localSocket.disconnectFromServer();
    return status;
}

void ConnectRequest::changeDeviceconfigForPortUpdate(quint16 port, quint16 forwardedTcpPort)
{
    m_portLock.lockForWrite();
    if((port != m_tcpPort[0]) || (forwardedTcpPort != m_tcpPort[1]))
    {
        m_tcpPort[0] = port;
        m_tcpPort[1] = forwardedTcpPort;
        tryForwardedPort = false;
    }
    m_portLock.unlock();
}

void ConnectRequest::setPortIndex(quint8 portIndex)
{
    portIndexLock.lock();
    m_portIndex = portIndex;
    portIndexLock.unlock();
}

quint8 ConnectRequest::getPortIndex()
{
    portIndexLock.lock();
    quint8 portIndex = m_portIndex;
    portIndexLock.unlock();
    return portIndex;
}
