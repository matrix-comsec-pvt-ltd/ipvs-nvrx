#ifndef BUFFERFEEDER_H
#define BUFFERFEEDER_H
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
//   File         : BufferFeeder.h
//   Description  : This module provides buffer to store media frames and their
//                  headers. It has APIs to write frame into and read frame from
//                  the buffer. It also notifies a signal when buffer size
//                  crosses minimum and maximum threshold.
//                  It provides a feeder thread which is useful to feed frames
//                  to decoder module, maintaining time delay between
//                  consecutive frames.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QByteArray>
#include <QThread>
#include <QReadWriteLock>
#include "EnumFile.h"
#include "DeviceDefine.h"
#include "DeviceClient/StreamRequest/FrameHeader.h"
#include "../DecoderLib/include/DecDispLib.h"
#include "../VideoStreamParser.h"

//******** Defines and Data Types ****

class PlaybackMedia;

//******** Function Prototypes *******
class BufferFeeder : public QThread
{
    Q_OBJECT

public:
    // This API is constructor of class BufferFeeder
    explicit BufferFeeder(quint8 decoderId,
                 PlaybackMedia *iPlaybackMedia,
                 quint64 bufferSize = 6 * ONE_MEGABYTE,
                 quint8 upperThreshold = 80,
                 quint8 lowerThreshold = 20);

    // This API is destructor of class BufferFeeder
    ~BufferFeeder();

    // write function to store frame to buffer
    bool writeFrame(QByteArray frame,
                    FRAME_HEADER_t header);

    // feeder thread which feeds frame to decoder module,
    // maintaining delay between consecutive frames.
    void run();
    // sets run flag status, stops feeder thread when set to false
    void setRunFlag(bool flag);

    bool StreamPauseStatus(void);
    PB_SPEED_e SetPbSpeed(PB_SPEED_e speed);

protected:
    // throttle request flag
    bool throttleFlag;
    // run flag for feeder thread
    bool runFlag;
    // decoder id to feed frame to
    quint8 decId;

    // size of the buffer
    quint64 buffSize;
    // buffer to store the media frames
    QByteArray *frameBuff;
    // buffer to store frame location in buffer
    QList<quint64> locationBuff;
    // buffer to store frame headers
    QList<FRAME_HEADER_t> headerBuff;

    // current write location in frame buffer
    quint64 wrLocation;
    // current frame buffer size
    quint64 currBuffSize;
    // maximum threshold value
    quint64 maxThreshold;
    // minimum threshold value
    quint64 minThreshold;
    // read write access lock for buffer operations
    QReadWriteLock bufferLock;
    // playback speed value
    PB_SPEED_e pbSpeed;
    /** Play back media member vairable */
    PlaybackMedia *mPlaybackMedia;

    // read function to retrieve frame from buffer
    bool readFrame(QByteArray &frame,
                   FRAME_HEADER_t &header,
                   quint64 &currFrameTime,
                   quint64 &nextFrameTime);

    bool nextFrameDelay(quint64 &delay);
    bool getFrameRate(UINT16 &fps);

    // read write access lock for feeder run flag
    QReadWriteLock runFlagLock;

signals:
    // signal to convey buffer size threshold cross
    void sigBufferThreshold(BUFFER_THRESHOLD_e threshold);
    // signal to convey the threshold command response
    void sigFeederResponse(DEVICE_REPLY_TYPE_e statusId,
                           QString frameTime);
};

#endif // BUFFERFEEDER_H
