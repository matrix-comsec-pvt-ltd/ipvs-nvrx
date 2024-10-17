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
//   Project      : NVR [Network Video Recorder]
//   Owner        : Aekam Parmar
//   File         : GenericRequest.cpp
//   Description  : This module provides functionalities to communicate with
//                  the NVR server.
//                  It has APIs to create request, send request, receive request
//                  and parse request.
//
/////////////////////////////////////////////////////////////////////////////
#include <QMetaType>
#include <sys/prctl.h>
#include "GenericRequest.h"

//******** Static Variables **********
// request message string
static const char *reqStrPtr[MAX_REQ_MSG] =
{
    "REQ_LOG",
    "REQ_POL",
    "GET_CFG",
    "SET_CFG",
    "DEF_CFG",
    "SET_CMD",
    "REQ_EVT",
    "PWD_RST",
};

// response message string
static const char *resStrPtr[MAX_RES_MSG] =
{
    "ACK_LOG",
    "ACK_POL",
    "RPL_CFG",
    "RPL_CMD",
    "RCV_EVT",
    "RPL_PWD",
};

//******** Function Definitions ******
//*****************************************************************************
//  GenericRequest()
//      Param:
//          IN : SERVER_INFO_t serverInfo   // server informations, like
//                                          // IP address and TCP port
//               REQ_INFO_t requestInfo     // request information like,
//                                          // sessionId, requestId, payload,
//                                          // timeout.
//          OUT: NONE
//	Returns:
//		Not Applicable
//      Description:
//          This API is constructor of class GenericRequest. It initializes the
//          newly created object with server and request parameters.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
GenericRequest::GenericRequest(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, quint8 genReqSesId): payloadSize((qint64)0)
{
    setObjectName ("GEN_REQ");
	m_genReqSesId = genReqSesId;
    stopFlagAccess.lockForWrite();
    stopFlag = false;
    stopFlagAccess.unlock();

    // store server parameter parameter
    server.ipAddress = serverInfo.ipAddress;
    server.tcpPort = serverInfo.tcpPort;

    // store request parameter
    request.sessionId = requestInfo.sessionId;
    request.requestId = requestInfo.requestId;

    request.payload = requestInfo.payload;
    request.bytePayload = requestInfo.bytePayload;

    request.timeout = requestInfo.timeout;

    resId = MAX_RES_MSG;
    statusId = CMD_MAX_DEVICE_REPLY;
}

//*****************************************************************************
//  ~GenericRequest()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//	Returns:
//		Not Applicable
//      Description:
//          This API is distructor of class GenericRequest.
//          As of now it does no functionality, kept for future use.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
GenericRequest::~GenericRequest()
{

}

void GenericRequest::setStopFlag(bool flag)
{
    stopFlagAccess.lockForWrite();
    stopFlag = flag;
    stopFlagAccess.unlock();
}

bool GenericRequest::getStopFlag(void)
{
    stopFlagAccess.lockForRead();
    bool flag = stopFlag;
    stopFlagAccess.unlock();
    return flag;
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
//          This API connects to the server, creates a request,
//          sends the request to server, receives response from the server
//          and parses response.
//          After this it emits a signal to notify the response.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void GenericRequest::run()
{
    bool status;
    QTcpSocket tcpSocket;

    prctl(PR_SET_NAME, "GEN_REQ", 0, 0, 0);

    // connect to the server
    status = connectToServer(tcpSocket);

    // if connected to server
    if (status == true)
    {
        // then send request to the server
        status = sendRequest(tcpSocket);

        // if request sent successfully
        if (status == true)
        {
            // receive response from server
            receiveResponse(tcpSocket);
        }

        // get disconnect from host
        tcpSocket.disconnectFromHost();
    }

    // close tcp socket
    tcpSocket.close();

    // emit signal to notify the status of operation
    // for configuration request [slotConfigResponse]  will be invoked.
	emit sigGenericResponse(request.requestId, statusId, request.payload, m_genReqSesId);
}

//*****************************************************************************
//  connectToServer()
//      Param:
//          IN : QTcpSocket &tcpSocket  // tcp socket by which the connection
//                                      // has to be established
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API tries to connect with the server within timeout.
//          If connction is made, returns true, otherwise false.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::connectToServer(QTcpSocket &tcpSocket)
{
    quint8 index = 0;

    // connect to the server
    tcpSocket.connectToHost (server.ipAddress, server.tcpPort);

    do
    {
        if (getStopFlag() == true)
        {
            break;
        }

        // wait for socket to become readable
        if (true == tcpSocket.waitForConnected (1 * ONE_MILISEC))
        {
            return true;
        }

        index++;

    } while(index < request.timeout);

    // ser request status to device not connected
    statusId = CMD_SERVER_NOT_RESPONDING;
    return false;
}

//*****************************************************************************
//  sendRequest()
//      Param:
//          IN : QTcpSocket &tcpSocket  // tcp socket over which the request
//                                      // has to be sent
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API creates a request and sends it to server over tcpSocket.
//          If operation is failed at any point, false is returned else true.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::sendRequest(QTcpSocket &tcpSocket)
{
    // if failed to create request
    if (false == createRequest())
    {
        // set request status to maximum buffer reached
        statusId = CMD_MAX_BUFFER_LIMIT;
        return false;
    }

    // if sending data to server failed
    if (false == sendData(tcpSocket))
    {
        // set request status to server not responding
        statusId = CMD_SERVER_NOT_RESPONDING;
        return false;
    }

    // return status
    return true;
}

//*****************************************************************************
//  receiveResponse()
//      Param:
//          IN : QTcpSocket &tcpSocket  // tcp socket over which the response
//                                      // has to be received
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API receives a response from server and does the parsing.
//          If operation is failed at any point, false is returned else true.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::receiveResponse(QTcpSocket &tcpSocket, quint64 size)
{
    // if failed to received data
    if (false == receiveData(tcpSocket, size))
    {
        // set request status to server not responding
        statusId = CMD_SERVER_NOT_RESPONDING;
        return false;
    }

    // if parsing of request is successful
    if (false == parseRequest())
    {
        return false;
    }

    // convert request status to integer form
    statusId = (DEVICE_REPLY_TYPE_e)statusString.toInt();

    // return status
    return true;
}

//*****************************************************************************
//  createRequest()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API constructs the request.
//          request format is <SOM><HEADER><PAYLOAD><EOM>
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::createRequest()
{
    // compose message as, <SOM><HEADER><PAYLOAD><EOM>
    // where HEADER is expanded to <MESSAGE ID><FSP><SESSION ID><FSP>
    ioBuffer.clear();
    ioBuffer.append(SOM);
    ioBuffer.append(reqStrPtr[request.requestId]);
    ioBuffer.append(FSP);
    ioBuffer.append(request.sessionId);
    ioBuffer.append(FSP);
    ioBuffer.append(request.payload);

    if(request.bytePayload != NULL)
    {
        for(quint8 index = 0; index < MAX_MOTION_BYTE; index++)
        {
            ioBuffer.append (request.bytePayload[index]);
        }
    }
    ioBuffer.append (EOM);

    // calculate payload size
    payloadSize = ioBuffer.size();

    // if payload size exceeds maximum buffer limit
    if (payloadSize > MAX_SEND_BUFFER_SIZE)
    {
        ioBuffer.clear();
        return false;
    }

    return true;
}

//*****************************************************************************
//  parseRequest()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API parses the response.
//          response is broken into responseId, statusId and payload.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::parseRequest()
{
    qint64 index;

    // remove SOM
    index = ioBuffer.indexOf (SOM);
    ioBuffer.remove (index, 1);

    // remove EOM
    index = ioBuffer.indexOf (EOM);
    ioBuffer.remove (index, 1);

    // split data to get response string
    splitData (ioBuffer, FSP, resString);

    resId = MAX_RES_MSG;

    // match response string, to find response type
    for (index = 0; index < MAX_RES_MSG; ++index)
    {
        if (qstrcmp (resString, resStrPtr [index]) == 0)
        {
            resId = (RES_MSG_ID_e) index;
            break;
        }
    }

    splitData (ioBuffer, FSP, statusString);
    if(resId == MSG_ACK_LOG)
    {
        // split string to get status string
        request.payload = statusString;
        statusString.clear();
        statusString.append (request.payload.mid(0, 3));
    }
    else
    {
        // store payload
        request.payload = ioBuffer;
    }

    // return status
    return true;
}

//*****************************************************************************
//  sendData()
//      Param:
//          IN : QTcpSocket &tcpSocket
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API sends the request to server over tcpSocket.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::sendData(QTcpSocket &tcpSocket)
{
    qint64  transferredBytes = 0;
    qint64  chunkSize;

    // loop until whole data is not sent
    do
    {
        // write data to socket
        chunkSize = tcpSocket.write (ioBuffer.right (payloadSize - transferredBytes));

        // if writing data to socket failed
        if (chunkSize < 0)
        {
            return false;
        }

        transferredBytes += chunkSize;

    }while (transferredBytes < payloadSize);

    return true;
}

//*****************************************************************************
//  receiveData()
//      Param:
//          IN : QTcpSocket &tcpSocket
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API receives the response from server over tcpSocket.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::receiveData(QTcpSocket &tcpSocket, qint64 size)
{
    quint8  index = 0;
    qint64  transferredBytes = 0;

    while(true)
    {
        if (getStopFlag() == true)
        {
            break;
        }

        // wait till request timeout for socket to become readable
        if(true == tcpSocket.waitForReadyRead(1 * ONE_MILISEC))
        {
            break;
        }

        if(!((tcpSocket.error() == QAbstractSocket::ConnectionRefusedError)
             || (tcpSocket.error() == QAbstractSocket::SocketTimeoutError)))
        {
            return false;
        }

        index++;
        if (index >= request.timeout)
        {
            return false;
        }
    }

    index = 0;
    ioBuffer.clear();

    // read data and store to buffer
    ioBuffer.append(tcpSocket.read((size - transferredBytes)));

    // calculate total number of bytes received
    transferredBytes = ioBuffer.size();

    // if first byte received is SOM
    if(ioBuffer.indexOf(SOM) != 0)
    {
        return false;
    }

    // then loop till EOM is not found
    while(ioBuffer.indexOf(EOM) < 0)
    {
        // if received bytes is greater than maximum buffer size
        if(transferredBytes >= size)
        {
            return false;
        }

        while(true)
        {
            if(getStopFlag() == true)
            {
                return false;
            }

            // wait till request timeout for socket to become readable
            if(true == tcpSocket.waitForReadyRead(1 * ONE_MILISEC))
            {
                break;
            }

            if(!((tcpSocket.error() == QAbstractSocket::ConnectionRefusedError)
                 || (tcpSocket.error() == QAbstractSocket::SocketTimeoutError)))
            {
                return false;
            }

            index++;
            if (index >= request.timeout)
            {
                return false;
            }

        }

        // read data and store to buffer
        ioBuffer.append(tcpSocket.read((size - transferredBytes)));

        // calculate total number of bytes received
        transferredBytes = ioBuffer.size();
    }

    return true;
}

//*****************************************************************************
//  splitData()
//      Param:
//          IN : QByteArray &sourceString   // contains original string
//               quint8 delimiter           // delimiter character
//          OUT: QByteArray &destString     // contains splitted string
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API splits the source string at first occurance of delimiter
//          character and stores the splitted string to destination string
//          buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::splitData(QByteArray &sourceString, quint8 delimiter, QByteArray &destString)
{
    quint64 index;

    // find index of delimiter
    index = sourceString.indexOf (delimiter);

    // copy string left to delimiter to destination string
    destString = sourceString.left (index);

    // remove string till delimiter from source string
    sourceString.remove (0, index + 1);

    // return status
    return true;
}

//*****************************************************************************
//  getBlockingRes()
//      Param:
//          IN : NONE
//          OUT: quint16 &size          // give size of payload receive
//               DEVICE_REPLY_TYPE_e &repley     // contains device reply
//	Returns:
//		BOOL const char * ;
//      Description:
//          This API receives data from tcp buffer, until EOM.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
void GenericRequest::getBlockingRes(QString &payloadStr, DEVICE_REPLY_TYPE_e &repStatus)
{
    QTcpSocket tcpSocket;

    // if connected to server
    if (connectToServer(tcpSocket) == true)
    {
        // if request sent successfully
        if (sendRequest(tcpSocket) == true)
        {
            // receive response from server
            receiveResponse (tcpSocket);
        }

        // get disconnect from host
        tcpSocket.disconnectFromHost();
    }

    // close tcp socket
    tcpSocket.close();
    payloadStr = request.payload;
    repStatus = statusId;
}

//*****************************************************************************
//  getResWithoutChekEom()
//      Param:
//          IN : NONE
//          OUT: quint16 &size          // give size of payload receive
//               DEVICE_REPLY_TYPE_e &repley     // contains device reply
//	Returns:
//		BOOL const char * ;
//      Description:
//          This API receives data from tcp buffer, until data is unavailable
//          in 2 sec timeout. Stop receiving doesn't depend on EOM.
//          buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
const char* GenericRequest::getResWithoutChekEom(quint32 &size, DEVICE_REPLY_TYPE_e &repley, qint64 maxSize)
{
    QTcpSocket tcpSocket;

    // if connected to server
    if (connectToServer(tcpSocket) == true)
    {
        // if request sent successfully
        if (sendRequest(tcpSocket) == true)
        {
            // receive response from server
            if(receiveDataWithoutEom (tcpSocket, maxSize) == true)
            {
                if (parseReqWithoutEom() == true)
                {
                    // convert request status to integer form
                    statusId = (DEVICE_REPLY_TYPE_e)statusString.toInt();
                }
            }
            else
            {
                statusId = CMD_SERVER_NOT_RESPONDING;
            }
        }

        // get disconnect from host
        tcpSocket.disconnectFromHost();
    }

    // close tcp socket
    tcpSocket.close();

    size = ioBuffer.size();
    repley = statusId;
    return ioBuffer.constData();
}

//*****************************************************************************
//  receiveDataWithoutEom()
//      Param:
//          IN : NONE
//          OUT: QTcpSocket &tcpSocket
//               qint64 size
//	Returns:
//		BOOL    true/ false
//      Description:
//          This API receives data from tcp buffer, until data is unavailable
//          in 2 sec timeout. Stop receiving doesn't depend on EOM.
//          buffer.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::receiveDataWithoutEom(QTcpSocket &tcpSocket, qint64 size)
{
    qint64  bytesAvail = 0;
    qint64  transferredBytes = 0;
    qint64  prevTransferBytes = 0;

    // wait till request timeout for socket to become readable. is socket become readable?
    if (false == tcpSocket.waitForReadyRead(request.timeout * ONE_MILISEC))
    {
        return false;
    }

    ioBuffer.clear();
    // read data and store to buffer
    ioBuffer.append (tcpSocket.read (size - transferredBytes));

    // calculate total number of bytes received
    transferredBytes = ioBuffer.size();

    // if first byte received is SOM
    if (ioBuffer.indexOf (SOM) != 0)
    {
        return false;
    }

    do
    {
        prevTransferBytes = transferredBytes;
        bytesAvail = tcpSocket.bytesAvailable();
        if(bytesAvail == 0)
        {
            if (false == tcpSocket.waitForReadyRead(2 * ONE_MILISEC))
            {
                break;
            }
        }

        ioBuffer.append(tcpSocket.read (size - transferredBytes));
        transferredBytes = ioBuffer.size();

    }while(prevTransferBytes != transferredBytes);

    return true;
}

//*****************************************************************************
//  parseReqWithoutEom()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API parses the response.
//          response is broken into responseId, statusId and payload.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE.
//
//*****************************************************************************
bool GenericRequest::parseReqWithoutEom()
{
    qint64 index;

    // remove SOM
    index = ioBuffer.indexOf (SOM);
    ioBuffer.remove (index, 1);

    // remove EOM
    index = ioBuffer.lastIndexOf (EOM);
    ioBuffer.remove (index, 1);

    // split data to get response string
    splitData (ioBuffer, FSP, resString);

    resId = MAX_RES_MSG;

    // match response string, to find response type
    for (index = 0; index < MAX_RES_MSG; ++index)
    {
        if (qstrcmp (resString, resStrPtr [index]) == 0)
        {
            resId = (RES_MSG_ID_e) index;
            break;
        }
    }
    // split string to get status string
    splitData (ioBuffer, FSP, statusString);

    // return status
    return true;
}
