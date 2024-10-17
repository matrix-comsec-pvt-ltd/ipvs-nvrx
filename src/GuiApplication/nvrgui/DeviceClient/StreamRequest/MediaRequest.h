#ifndef MEDIAREQUEST_H
#define MEDIAREQUEST_H
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
//   File         : MediaRequest.h
//   Description  : This module provides common APIs to receive media for live
//                  and playback.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QMutex>
#include <QTcpSocket>

#include "EnumFile.h"
#include "FrameHeader.h"
#include "DeviceClient/CommandRequest/CommandRequest.h"


//******** Defines and Data Types ****
#define MAX_RESOLUTION          (107)
#define STREAM_RESPONSE_SIZE    (14)

//******** Function Prototypes *******
class MediaRequest : public CommandRequest
{
    Q_OBJECT

public:

    typedef enum {
        FRAME_WIDTH = 0,
        FRAME_HEIGHT,
        MAX_FRAME_DIMENSIONS

    }FRAME_DIMENSIONS_e;

    // constructor to initialize the object
    MediaRequest(SERVER_INFO_t serverInfo,
                 REQ_INFO_t &requestInfo,
                 SET_COMMAND_e commandId,
                 quint8 decoderId,
                 const quint8 *pDecIdSet = NULL);

    // destructor to de-initialize the object
    ~MediaRequest ();

    // pure virutal media thread function
    void run () = 0;
    // API to set run flag status
    void setRunFlag(bool flag);
    bool getRunFlag();

    // API to receive frame header
    bool receiveHeader (FRAME_HEADER_t &header,
                        QTcpSocket &tcpSocket);
    // API to receive frame
    bool receiveFrame (QByteArray &frame,
                       qint64 size,
                       QTcpSocket &tcpSocket);

    bool receiveHeaderToDirLoc (char * writePtr,
                                QTcpSocket &tcpSocket);

    bool receiveFrameToDirLoc (char *writePtr,
                               qint64 size,
                               QTcpSocket &tcpSocket);

    bool receiveHeaderToDirLocSyncPb (char * writePtr,
                                QTcpSocket &tcpSocket);

    bool receiveFrameToDirLocSyncPb (char *writePtr,
                               qint64 size,
                               QTcpSocket &tcpSocket);

    bool receiveHeader_v2(char* writePtr, qint64 size, QTcpSocket &tcpSocket, quint16 timeoutMs, quint64 *bytesRead);
    bool receiveFrame_v2(char *writePtr, qint64 size, QTcpSocket &tcpSocket, quint16 timeoutMs, quint64 *bytesRead);

protected:

    // run flag, which breaks the media loop if set to false
    bool runFlag;

    // decoder session id
    quint8 decId;
    quint8 SyncDecIdSet[MAX_SYNC_PB_SESSION];

    // 4 bytes of magic code, used as validation of FSH
    static quint32 magicCode;

    // read write access lock for run flag
    QReadWriteLock runFlagLock;

private:

signals:
    // signal to convey the command response
    void sigMediaResponse(REQ_MSG_ID_e requestId,
                          SET_COMMAND_e commandId,
                          DEVICE_REPLY_TYPE_e statusId,
                          QString payload);

};

//******** Extern Variables ****
extern const quint16 frameResolution[MAX_RESOLUTION][MediaRequest::MAX_FRAME_DIMENSIONS];

#endif // MEDIAREQUEST_H
