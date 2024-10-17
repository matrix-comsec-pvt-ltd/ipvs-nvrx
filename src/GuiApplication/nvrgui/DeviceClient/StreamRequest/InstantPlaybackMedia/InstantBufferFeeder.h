#ifndef INSTANTBUFFERFEEDER_H
#define INSTANTBUFFERFEEDER_H

#include <QObject>
#include <QByteArray>
#include <QThread>
#include <QReadWriteLock>
#include "EnumFile.h"
#include "DeviceDefine.h"
#include "DeviceClient/StreamRequest/FrameHeader.h"
#include "../VideoStreamParser.h"

class InstantBufferFeeder : public QThread
{
    Q_OBJECT
public:
    // This API is constructor of class BufferFeeder
    explicit InstantBufferFeeder(quint8 decoderId,
                        quint64 bufferSize = 6 * ONE_MEGABYTE,
                        quint8 upperThreshold = 80,
                        quint8 lowerThreshold = 20);
    // This API is destructor of class BufferFeeder
    ~InstantBufferFeeder();

    // write function to store frame to buffer
    bool writeFrame(QByteArray frame,
                    FRAME_HEADER_t header);
    void resetBufData();

    // feeder thread which feeds frame to decoder module,
    // maintaining delay between consecutive frames.
    void run();
    // sets run flag status, stops feeder thread when set to false
    void setRunFlag(bool flag);
    void setFeedFrameFlag(bool flag);
    bool getFeedFrameFlag();
    void setPlaybackOverFlag(bool flag);
    bool getPlaybackOverFlag();

    bool StreamPauseStatus(void);
    bool getRunFlag();

protected:
    // throttle request flag
    bool throttleFlag;
    // run flag for feeder thread
    bool runFlag;
    bool feedFrameFlag;
    bool playbackOverFlag;
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

    // read function to retrieve frame from buffer
    bool readFrame(QByteArray &frame,
                   FRAME_HEADER_t &header,
                   quint64 &currFrameTime,
                   quint64 &nextFrameTime);

    // read write access lock for feeder run flag
    QReadWriteLock runFlagLock;
    QReadWriteLock feedFrameFlagLock;
    QReadWriteLock playbackOverFlagLock;

signals:
    // signal to convey buffer size threshold cross
    void sigBufferThreshold(BUFFER_THRESHOLD_e threshold);
    // signal to convey the threshold command response
    void sigFeederResponse(DEVICE_REPLY_TYPE_e statusId,
                           QString responsePayload);
};


#endif // INSTANTBUFFERFEEDER_H
