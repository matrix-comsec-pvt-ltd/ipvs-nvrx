#ifndef CLIENTMEDIA_H
#define CLIENTMEDIA_H
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
//   Owner        : Kaushal Patel
//   File         : ClientMedia.h
//   Description  : This module provides APIs to make client audio request.
//                  It receives frame and frame header and feeds it to decoder
//                  module.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QReadWriteLock>
#include <QWaitCondition>
#include <QMutex>
#include "../MediaRequest.h"
#include "../DecoderLib/include/DecDispLib.h"

//******** Defines and Data Types ****


//******** Function Prototypes *******
class ClientMedia : public MediaRequest
{
    Q_OBJECT

public:
    // This API is constructor for the class LiveMedia. It initializes object
    // with server info, request info command id and windowId.
    ClientMedia(SERVER_INFO_t serverInfo,
              REQ_INFO_t &requestInfo,
              SET_COMMAND_e commandId,
              quint8 decoderId,
              quint8 streamId);
    // This API is destructor function for the class LiveMedia
    ~ClientMedia();

    // run function, which is executed in system created thread.
    // it requests for live media, receives frame and feeds it to
    // decoder module.
    void run();

private:
    quint8 m_streamId;
};

#endif // CLIENTMEDIA_H
