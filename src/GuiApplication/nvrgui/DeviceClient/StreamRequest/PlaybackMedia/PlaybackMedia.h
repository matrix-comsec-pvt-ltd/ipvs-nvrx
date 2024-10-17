#ifndef PLAYBACKMEDIA_H
#define PLAYBACKMEDIA_H
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
//   File         : PlaybackMedia.h
//   Description  : This module provides APIs for playback media streaming.
//                  It has a thread function which sends request to server
//                  and if the response is success, receives stream and stores
//                  in the buffer.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QString>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QMutex>
#include <QTimer>

#include "../MediaRequest.h"
#include "BufferFeeder.h"


//******** Defines and Data Types ****
#define MAX_PLAYBACK_SESSION    (MAX_DEC_DISP_CHN_PLAYBACK)


//******** Function Prototypes *******

class PlaybackMedia : public MediaRequest
{
    Q_OBJECT

public:
    // This API is constructor of class PlaybackMedia
    PlaybackMedia(SERVER_INFO_t serverInfo,
                  REQ_INFO_t &requestInfo,
                  SET_COMMAND_e commandId,
                  QString playbackId,
                  quint8 decoderId);

    // This API is destructor of class PlaybackMedia
    ~PlaybackMedia();

    // run function, which is executed in system created thread.
    // it requests for playback media, receives frame and stores it in buffer.
    void run();

    // This API sets pause flag to false
    void setPauseFlag(bool flag);
    void getPauseFlag(bool &flag);

protected:
    // stream pause status
    bool pauseFlag;
    // response timeout [used for pause and resume command]
    quint8 timeout;

    // server information, like ip address and tcp port
    SERVER_INFO_t srvrInfo;
    // active session id of NVR [used for pause and resume command]
    QString sessionId;
    // type of command [whether play or step]
    SET_COMMAND_e cmdId;

    // playback id [used for pause and resume command]
    QString pbId;

    // pointer to buffer object
    BufferFeeder *buffer;
    // feeder pointer, which duplicates the buffer pointer
    // [used just for sake of clarity in program]
    BufferFeeder *feeder;

    // read write access lock for stream pause status
    QReadWriteLock pauseFlagAccess;

    // throttle command pointer [used for pause and resume command]
    CommandRequest *throttle;
    // This API creates buffer to store playback stream
    // and feeder thread to feed frames to decoder
    bool createBufferFeeder(void);
    // This API deletes buffer feeder
    bool deleteBufferFeeder(void);
    // This API creates throttle request [pause / resume]
    bool createThrottleReq(SET_COMMAND_e commandId);
    // This API deletes throttle request
    bool deleteThrottleReq(void);

public slots:
    // slot to catch buffer threshold cross signal
    void slotBufferThreshold(BUFFER_THRESHOLD_e threshold);
    // slot to catch throttle request
    void slotThrottleResponse(REQ_MSG_ID_e requestId,
                              SET_COMMAND_e commandId,
                              DEVICE_REPLY_TYPE_e tStatusId,
                              QString payload);

    void slotPbMediaResponse(DEVICE_REPLY_TYPE_e tStatusId,
                             QString frameTime);
};

#endif // PLAYBACKMEDIA_H
