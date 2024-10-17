#ifndef GENERICREQUEST_H
#define GENERICREQUEST_H
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
//   File         : GenericRequest.h
//   Description  : This module provides APIs to send requests to and receive
//                  response from NVR server.
//
/////////////////////////////////////////////////////////////////////////////
#include <QObject>
#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QByteArray>
#include <QReadWriteLock>

#include "EnumFile.h"
#include "DataStructure.h"

// maximum buffer size for sending request
#define MAX_SEND_BUFFER_SIZE            (16 * ONE_KILOBYTE)

// maximum buffer size for receiving response
#define MAX_RECEIVE_BUFFER_SIZE         (128* ONE_KILOBYTE)
#define MAX_RCV_BUFFER_SIZE_FOR_IMAGE   (8 * ONE_MEGABYTE)
#define MAX_GEN_REQ_SESSION             2

//******** Function Prototypes ********
class GenericRequest : public QThread
{
    Q_OBJECT

public:
    // constructor function, which creates and initializes object with server and request information
    GenericRequest(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, quint8 genReqSesId = MAX_GEN_REQ_SESSION);
    // destructor function
    ~GenericRequest();

    // run function, which is executed in system created thread. it actually does the communication with NVR device.
    void run();
    // blocking function of dev req
    void getBlockingRes(QString &payloadStr, DEVICE_REPLY_TYPE_e &repStatus);

    // this api is used, when bitwise data are transsfered,
    // in which EOM may be arrive in between,
    const char *getResWithoutChekEom(quint32 &size, DEVICE_REPLY_TYPE_e &repley, qint64 maxSize = MAX_RECEIVE_BUFFER_SIZE);

    void setStopFlag(bool flag);
    bool getStopFlag(void);    

protected:
    SERVER_INFO_t       server;         // keeps the server information
    REQ_INFO_t          request;        // keeps the request information
    RES_MSG_ID_e        resId;          // response index
    DEVICE_REPLY_TYPE_e statusId;       // status of the request
    QByteArray          ioBuffer;       // this buffer is used to store data to communicate over network.
    QByteArray          resString;      // keeps string of response message id
    QByteArray          statusString;   // keeps string of response status
    qint64              payloadSize;    // keeps size of payload

    // flag to indicate stream is stopped by user
    bool                stopFlag;
    // access lock for stop flag
    QReadWriteLock      stopFlagAccess;
    quint8              m_genReqSesId;

    // this API tries to connect to server.
    bool connectToServer(QTcpSocket &tcpSocket);

    // this API creates request string and sends it to the server.
    bool sendRequest(QTcpSocket &tcpSocket);
    // this API creates request string to be sent to server.
    bool createRequest();
    // this API sends the created string to the server.
    bool sendData(QTcpSocket &tcpSocket);

    // this API receives response string from server and parses it.
    bool receiveResponse(QTcpSocket &tcpSocket, quint64 size = MAX_RECEIVE_BUFFER_SIZE);
    // this API receives string from the server
    bool receiveData(QTcpSocket &tcpSocket, qint64 size);
    // this API parses the received response from the server.
    bool parseRequest();

    // this API splits the string by delimiter
    bool splitData(QByteArray &sourceString, quint8 delimiter, QByteArray &destString);

    // this API sets response timeout
    bool serResponseTime(quint8 responseTime);

    bool receiveDataWithoutEom(QTcpSocket &tcpSocket, qint64 size = MAX_RECEIVE_BUFFER_SIZE);
    bool parseReqWithoutEom();

signals:
    // signal to convey the generic response
    void sigGenericResponse(REQ_MSG_ID_e requestId, DEVICE_REPLY_TYPE_e statusId, QString payload, quint8 genReqSesId);
};

#endif // GENERICREQUEST_H
