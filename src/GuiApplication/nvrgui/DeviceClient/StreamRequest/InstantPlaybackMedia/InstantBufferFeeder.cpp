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
//   File         : BufferFeeder.cpp
//   Description  : This module provides buffer to store media frames and their
//                  headers. It has APIs to write frame into and read frame from
//                  the buffer. It also notifies a signal when buffer size
//                  crosses minimum and maximum threshold.
//                  It provides a feeder thread which is useful to feed frames
//                  to decoder module, maintaining time delay between
//                  consecutive frames.
//
/////////////////////////////////////////////////////////////////////////////

#include "InstantBufferFeeder.h"
#include "../MediaRequest.h"
#include <sys/prctl.h>
#include <QDateTime>
#include "../DecoderLib/include/DecDispLib.h"

#define PAUSE_POLL_TIME         100                 // in mSec
#define AVERAGE_FRAME_SIZE      (50 * ONE_KILOBYTE)
#define DEFAULT_FRAME_DELAY     100                 // in mSec
#define MIN_FRAME_DELAY         16                  // in mSec
#define FEEDFLAG_FALSE_DELAY    50                  // in mSec

InstantBufferFeeder::InstantBufferFeeder(quint8 decoderId,
                                         quint64 bufferSize,
                                         quint8 upperThreshold,
                                         quint8 lowerThreshold)
    : buffSize(bufferSize)
{
    frameBuff = new QByteArray(buffSize, 0);
    runFlag = true;
    feedFrameFlag = true;
    playbackOverFlag = false;
    decId = decoderId;

    bufferLock.lockForWrite();
    headerBuff.clear();
    locationBuff.clear();
    headerBuff.reserve(buffSize / AVERAGE_FRAME_SIZE);
    locationBuff.reserve(buffSize / AVERAGE_FRAME_SIZE);
    currBuffSize = 0;
    wrLocation = 0;
    bufferLock.unlock();

    maxThreshold = (buffSize * upperThreshold) / 100;
    minThreshold = (buffSize * lowerThreshold) / 100;
    throttleFlag = false;

    pbSpeed = MAX_PB_SPEED;
}

InstantBufferFeeder::~InstantBufferFeeder()
{
    delete frameBuff;
}

bool InstantBufferFeeder::writeFrame(QByteArray frame,
                                     FRAME_HEADER_t header)
{
    bool    status = false;
    quint64 frameLen;

    bufferLock.lockForWrite();

    frameLen = (quint64)header.frameSize - sizeof (header);

    if ((wrLocation + frameLen) > buffSize)
    {
        wrLocation = 0;
    }

    if ((wrLocation + frameLen) <= buffSize)
    {
        frameBuff->replace (wrLocation, frameLen, frame);
        headerBuff.append (header);
        locationBuff.append (wrLocation);

        wrLocation += frameLen;
        currBuffSize += frameLen;

        if ((currBuffSize >= maxThreshold) && (throttleFlag == false))
        {
            throttleFlag = true;
            emit sigBufferThreshold(CROSSED_MAXIMUM);
        }

        status = true;
    }

    bufferLock.unlock ();

    return status;
}

void InstantBufferFeeder::resetBufData()
{
    bufferLock.lockForWrite();

    headerBuff.clear();
    locationBuff.clear();
    headerBuff.reserve(buffSize / AVERAGE_FRAME_SIZE);
    locationBuff.reserve(buffSize / AVERAGE_FRAME_SIZE);
    currBuffSize = 0;
    wrLocation = 0;
    throttleFlag = false;

    bufferLock.unlock();

    setPlaybackOverFlag(true);
}

bool InstantBufferFeeder::readFrame(QByteArray &frame,
                                    FRAME_HEADER_t &header,
                                    quint64 &currFrameTime,
                                    quint64 &nextFrameTime)
{
    bool            status = false;
    quint64         location, frameLen;
    FRAME_HEADER_t  nextHeader;

    bufferLock.lockForRead ();

    if (locationBuff.count () > 0)
    {
        location = locationBuff.takeFirst ();
        header = headerBuff.takeFirst ();
        frameLen = (quint64)header.frameSize - sizeof (header);
        frame = frameBuff->mid (location, frameLen);

        currFrameTime = ((quint64)header.seconds * ONE_MILISEC) + header.mSec;

        if (locationBuff.count () > 0)
        {
            nextHeader = headerBuff.first ();

            if(nextHeader.streamStatus == STREAM_NORMAL)
            {
                nextFrameTime = ((quint64)nextHeader.seconds * ONE_MILISEC) + nextHeader.mSec;
            }
            else
            {
                nextFrameTime = currFrameTime;
            }
        }
        else
        {
            nextFrameTime = currFrameTime;
        }

        currBuffSize -= frameLen;

        if ((currBuffSize <= minThreshold) && (throttleFlag == true))
        {
            throttleFlag = false;
            emit sigBufferThreshold(CROSSED_MINIMUM);
        }

        status = true;
    }

    bufferLock.unlock ();

    return status;
}

void InstantBufferFeeder::run()
{
    bool status;
    bool feedFrame = true;

    DEVICE_REPLY_TYPE_e statusId = CMD_PLAYBACK_TIME;
    quint8 videoLoss = MAX_VIDEO_LOSS_STATUS;

    QByteArray      frame;
    FRAME_HEADER_t  header;
    FRAME_INFO_t    frameInfo;
    QDateTime       dt;

    quint64 frameDelay = 0;
    quint64 emitTime = 0;
    quint64 currFrameTime;
    quint64 nextFrameTime;
    quint64 prevFrameTime = 0;
    DECODER_ERROR_e decError = MAX_DEC_ERROR;
    qint64 time1 = 0, time2 = 0,diff = 0;

    prctl(PR_SET_NAME, "INSTANT_BUF_FEDER", 0, 0, 0);

    frameInfo.frameWidth = 0;
    frameInfo.frameHeight = 0;

    while(getRunFlag () == true)
    {
        msleep(frameDelay);

        if(getFeedFrameFlag() == false)
        {
            frameDelay = FEEDFLAG_FALSE_DELAY;
        }
        else
        {
            time1 = dt.currentMSecsSinceEpoch();

            status = readFrame(frame, header, currFrameTime, nextFrameTime);

            if(status == false)
            {
                frameDelay = DEFAULT_FRAME_DELAY;
            }
            else
            {
                if(header.streamStatus != STREAM_NORMAL)
                {
                    switch(header.streamStatus)
                    {
                    case STREAM_FILE_ERROR:
                        statusId = CMD_STREAM_FILE_ERROR;
                        break;

                    case STREAM_HDD_FORMAT:
                        statusId = CMD_STREAM_HDD_FORMAT;
                        break;

                    case STREAM_CONFIG_CHANGE:
                        statusId = CMD_STREAM_CONFIG_CHANGE;
                        break;

                    case STREAM_PLAYBACK_OVER:
                        statusId = CMD_STREAM_PLAYBACK_OVER;
                        break;

                    default:
                        statusId = CMD_PROCESS_ERROR;
                        break;
                    }

                    setRunFlag(false);
                }
                else
                {
                    if(nextFrameTime > currFrameTime)
                    {
                        frameDelay = (nextFrameTime - currFrameTime);
                    }
                    else
                    {
                        frameDelay = (currFrameTime - nextFrameTime);
                    }

                    if(frameDelay == 0)
                    {
                        frameDelay = DEFAULT_FRAME_DELAY;
                    }

                    if(currFrameTime > prevFrameTime)
                    {
                        emitTime = (currFrameTime - prevFrameTime);
                    }
                    else
                    {
                        emitTime = (prevFrameTime - currFrameTime);
                    }

                    if (emitTime > ONE_MILISEC)
                    {
                        emit sigFeederResponse(CMD_PLAYBACK_TIME,
                                               QString ("%1").arg (currFrameTime));

                        prevFrameTime = currFrameTime;
                    }

                    switch(header.streamType)
                    {
                    case STREAM_TYPE_AUDIO:
                        feedFrame = true;
                        break;

                    case STREAM_TYPE_VIDEO:
                        if((header.videoLoss != videoLoss))
                        {
                            videoLoss = (quint8)header.videoLoss;

                            if(videoLoss == VIDEO_LOSS)
                            {
                                statusId = CMD_STREAM_VIDEO_LOSS;
                                feedFrame = false;
                            }
                            else
                            {
                                statusId = CMD_STREAM_NO_VIDEO_LOSS;
                                feedFrame = true;
                            }
                            emit sigFeederResponse(statusId, "");
                        }
                        else if(videoLoss == VIDEO_LOSS)
                        {
                            feedFrame = false;
                        }
                        break;

                    default:
                        break;
                    }

                    if(feedFrame == true)
                    {
                        frameInfo.framePayload = frame.data ();

                        if((STREAM_TYPE_VIDEO == header.streamType)
                                && (I_FRAME == (FRAME_TYPE_e)header.frameType))
                        {
                            if(header.resolution == 0)
                            {
                                if((VideoStreamParser::getHeightWidth(frameInfo.framePayload,
                                                                        &header,
                                                                        frameInfo.frameWidth,
                                                                        frameInfo.frameHeight)) == false)
                                {
                                    frameDelay = 0;
                                    continue;
                                }
                            }
                            else
                            {
                                if((header.resolution != 0) && (header.resolution < MAX_RESOLUTION))
                                {
                                    frameInfo.frameWidth  = frameResolution[header.resolution][MediaRequest::FRAME_WIDTH];
                                    frameInfo.frameHeight = frameResolution[header.resolution][MediaRequest::FRAME_HEIGHT];
                                }
                            }
                        }
                        else
                        {
                            if((header.resolution != 0) && (header.resolution < MAX_RESOLUTION))
                            {
                                frameInfo.frameWidth  = frameResolution[header.resolution][MediaRequest::FRAME_WIDTH];
                                frameInfo.frameHeight = frameResolution[header.resolution][MediaRequest::FRAME_HEIGHT];
                            }
                        }


                        frameInfo.frameSize = header.frameSize - sizeof (header);
                        frameInfo.mediaType = (STREAM_TYPE_e)header.streamType;
                        frameInfo.codecType = (STREAM_CODEC_TYPE_e)header.codecType;
                        frameInfo.frameType = (FRAME_TYPE_e)header.frameType;
                        frameInfo.frameTimeSec = header.seconds;
                        frameInfo.frameTimeMSec = header.mSec;

                        //consider configured fps from header version 2 onwards..
                        if((header.version >= 2) && (header.frameRate != 0) && (header.frameRate < 100))
                            frameInfo.frameRate = header.frameRate;
                        else
                            frameInfo.frameRate = DEFAULT_FRAME_RATE;

                        frameInfo.noOfReferanceFrame = header.noOfRefFrame;
                        if((frameInfo.frameWidth != 0)
                                && (frameInfo.frameHeight != 0))
                        {
                            status = DecodeDispFrame(decId, &frameInfo,&decError);
                            if(status == false)
                            {
								if(DEC_ERROR_NO_CAPACITY == decError)
								{
									statusId = CMD_DECODER_CAPACITY_ERROR;
								}
								else
								{
									statusId = CMD_DECODER_ERROR;
								}
                                setRunFlag(false);
                            }
                            else
                            {
                                if(frameDelay > ONE_MILISEC)
                                {
                                    frameDelay = ONE_MILISEC;
                                }
                            }
                        }
                        else
                        {
                            frameDelay = 0;
                        }
                    }
                    else
                    {
                        if(frameDelay > ONE_MILISEC)
                        {
                            frameDelay = ONE_MILISEC;
                        }
                    }

                    time2 = dt.currentMSecsSinceEpoch();

                    if(time2 > time1)
                    {
                        diff = (time2 - time1);
                    }
                    else
                    {
                        diff = 0;
                    }

                    if(frameDelay >= (quint64)diff)
                    {
                        frameDelay -= (quint64)diff;
                    }
                    else
                    {
                        frameDelay = 0;
                    }
                }
            }
        }
    }

    if (!((statusId == CMD_PLAYBACK_TIME)
          || (statusId == CMD_STREAM_VIDEO_LOSS)
          || (statusId == CMD_STREAM_NO_VIDEO_LOSS)))
    {
        emit sigFeederResponse(statusId, "");
    }
}

void InstantBufferFeeder::setRunFlag(bool flag)
{
    runFlagLock.lockForWrite();
    runFlag = flag;
    runFlagLock.unlock();
}

bool InstantBufferFeeder::getRunFlag()
{
    bool flag = false;
    runFlagLock.lockForWrite();
    flag = runFlag;
    runFlagLock.unlock ();

    return flag;
}

void InstantBufferFeeder::setFeedFrameFlag(bool flag)
{
    feedFrameFlagLock.lockForWrite();
    feedFrameFlag = flag;
    feedFrameFlagLock.unlock();
}

bool InstantBufferFeeder::getFeedFrameFlag()
{
    bool flag = false;

    feedFrameFlagLock.lockForWrite();
    flag = feedFrameFlag;
    feedFrameFlagLock.unlock();

    return flag;
}

void InstantBufferFeeder::setPlaybackOverFlag(bool flag)
{
    playbackOverFlagLock.lockForWrite();
    playbackOverFlag = flag;
    playbackOverFlagLock.unlock();
}

bool InstantBufferFeeder::getPlaybackOverFlag()
{
    bool flag = false;

    playbackOverFlagLock.lockForWrite();
    flag = playbackOverFlag;
    playbackOverFlagLock.unlock();

    return flag;
}

