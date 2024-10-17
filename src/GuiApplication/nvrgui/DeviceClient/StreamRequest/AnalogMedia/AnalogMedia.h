#ifndef ANALOGMEDIA_H
#define ANALOGMEDIA_H
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
//   Project      : DVR ( Hybrid Video Recorder)
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMutex>
#include <QThread>

#include "EnumFile.h"
#include "../../CommandRequest/CommandRequest.h"

class AnalogMedia : public QThread
{
    Q_OBJECT

public:
    // constructor function, which creates and initializes object
    AnalogMedia(SERVER_INFO_t serverInfo,
                REQ_INFO_t &requestInfo,
                SET_COMMAND_e commandId,
                quint8 channelId);

    ~AnalogMedia();

    void run();

    // API to set run flag status
    void setRunFlag (bool flag);
    // API to set stop flag status
    void setStopFlag (bool flag);
    bool getStopFlag(void);

    bool createCmdReq(SERVER_INFO_t serverInfo,
                      REQ_INFO_t &requestInfo,
                      SET_COMMAND_e cmd);

    bool deleteCmdReq();

    void setServerSessionInfo(SERVER_INFO_t serverInfo,
                              REQ_INFO_t &requestInfo,
                              quint8 channelId);

protected:
    // run flag, which breaks the media loop if set to false
    bool runFlag;
    // flag to indicate stream is stopped by user
    bool stopFlag;

    quint8 chId;

    SERVER_INFO_t srvrInfo;
    REQ_INFO_t reqInfo;

    SET_COMMAND_e cmdId;
    DEVICE_REPLY_TYPE_e statusId;

    // read write access lock for run flag
    QMutex runFlagLock;

    // access lock for stop flag
    QMutex stopFlagAccess;
    CommandRequest *commandReq;


signals:
    // signal to convey the  response
    void sigAnalogMediaResponse (REQ_MSG_ID_e requestId,
                                 SET_COMMAND_e commandId,
                                 DEVICE_REPLY_TYPE_e statusId,
                                 QString payload);
};

#endif // ANALOGMEDIA_H
