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

#include "BufferFeeder.h"
#include "../MediaRequest.h"
#include <sys/prctl.h>
#include <QDateTime>
#include "PlaybackMedia.h"

//******** Defines and Data Types ****
#define AVERAGE_FRAME_SIZE      (50 * ONE_KILOBYTE)
#define DEFAULT_FRAME_DELAY     100         // in mSec
#define MIN_FRAME_DELAY         16          // in mSec


//******** Function Prototypes *******



//******** Global Variables **********



//******** Static Variables **********



//******** Function Definitions ******

//*****************************************************************************
//  BufferFeeder ()
//      Param:
//          IN : quint64 bufferSize
//               quint8 upperThreshold
//               quint8 lowerThreshold
//          OUT: NONE
//
//	Returns:
//          Not Applicable
//
//	Description:
//          This is constructor of class BufferFeeder. It initializes the object
//          with buffer size and threshold levels.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
BufferFeeder::BufferFeeder (quint8 decoderId,
                            PlaybackMedia *iPlaybackMedia,
                            quint64 bufferSize,
                            quint8 upperThreshold,
                            quint8 lowerThreshold)
    : buffSize(bufferSize)
{
    // create buffer to store frames
    frameBuff = new QByteArray (buffSize, 0);
    // initialize buffers to store frmae location and header
    headerBuff.reserve (buffSize / AVERAGE_FRAME_SIZE);
    locationBuff.reserve (buffSize / AVERAGE_FRAME_SIZE);
    // initialize current write location and current buffer size
    pbSpeed = MAX_PB_SPEED;
    wrLocation = 0;
    currBuffSize = 0;
    runFlag = true;
    decId = decoderId;

    // calculate threshold value
    maxThreshold = (buffSize * upperThreshold) / 100;
    minThreshold = (buffSize * lowerThreshold) / 100;
    // set throttle flag to false
    throttleFlag = false;
    mPlaybackMedia = iPlaybackMedia;
}

//*****************************************************************************
//  ~BufferFeeder ()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//
//	Returns:
//          Not Applicable
//	Description:
//          This API is destructor of class BufferFeeder.
//          It deletes the frame buffer.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
BufferFeeder::~BufferFeeder ()
{
    // delete frame buffer
    delete frameBuff;
}

//*****************************************************************************
//  writeFrame ()
//      Param:
//          IN : QByteArray frame
//               FRAME_HEADER_t header
//          OUT: NONE
//
//	Returns:
//          bool [true / false]
//	Description:
//          This API write a frame and header into the buffer.
//          If current write cause the buffer size to cross maximum threshold,
//          it emits maximum threshold cross signal.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool BufferFeeder::writeFrame (QByteArray frame,
                               FRAME_HEADER_t header)
{
    bool status = false;
    quint64 frameLen;
    quint64 tempFrameLen;
    // lock write access to parameters
    bufferLock.lockForWrite ();
    // calculate frame size
    frameLen = (quint64)header.frameSize - sizeof (header);
    // Verify size for store data into buffer
    if((currBuffSize + frameLen) < buffSize)
    {
        // store frame header to header buffer
        headerBuff.append (header);
        // store current location in location buffer
        locationBuff.append (wrLocation);
        // if current frame write can go beyond buffer size
        if ((wrLocation + frameLen) > buffSize)
        {
            tempFrameLen = (buffSize - wrLocation);
            // write frame to frame buffer
            frameBuff->replace (wrLocation, tempFrameLen, frame);
            // set write location to beginning of buffer
            wrLocation = 0;
            // write frame to frame buffer
            frameBuff->replace (wrLocation, (frameLen - tempFrameLen), frame.mid(tempFrameLen));
            wrLocation += (frameLen - tempFrameLen);
        }
        else
        {
            // write frame to frame buffer
            frameBuff->replace (wrLocation, frameLen, frame);
            wrLocation += frameLen;
        }
        // update current buffer size
        currBuffSize += frameLen;

        // if buffer size crosses upper threshold limit AND
        // throttle signal has not been emitted
        if ( (currBuffSize >= maxThreshold) && (throttleFlag == false) )
        {
            // set threshold flag to true
            throttleFlag = true;
            // emit upper threshold cross signal
            if(mPlaybackMedia)
            {
                mPlaybackMedia->setPauseFlag(true);
            }
            emit sigBufferThreshold (CROSSED_MAXIMUM);
        } // endif

        // set write operation to true
        status = true;

    }//endif
    // unlock access to buffers
    bufferLock.unlock ();

    // return status
    return status;
}

//*****************************************************************************
//  readFrame ()
//      Param:
//          IN : NONE
//          OUT: QByteArray &frame
//               FRAME_HEADER_t &header
//               quint64 &frameDelay
//	Returns:
//          bool [true / false]
//	Description:
//          This API read a frame and header from the buffer. It also outputs
//          frame delay for next frame read.
//          If current frame read cause buffer size to cross minimum threshold,
//          it emits minimum threshold cross signal.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool BufferFeeder::readFrame (QByteArray &frame,
                              FRAME_HEADER_t &header,
                              quint64 &currFrameTime,
                              quint64 &nextFrameTime)
{
    bool status = false;
    quint64 location, frameLen;
    FRAME_HEADER_t nextHeader;

    // lock read access to parameters
    bufferLock.lockForRead ();

    // if there is frame to read in buffer
    if (locationBuff.count () > 0)
    {
        // read location of frame
        location = locationBuff.takeFirst ();
        // read frame header
        header = headerBuff.takeFirst ();
        // calculate frame length to read
        frameLen = (quint64)header.frameSize - sizeof (header);
        // update current buffer size
        currBuffSize -= frameLen;
        // if current frame read can go beyond buffer size
        if((location + frameLen) > buffSize)
        {
            // read frame from frame Buffer
            frame = frameBuff->mid (location, (buffSize - location));
            // calculate remaining size
            frameLen -= (buffSize - location);
            // set read location to beginning of buffer
            location = 0;
            // append frame from starting location
            frame.append(frameBuff->mid (location, frameLen));
        }
        else
        {
            // read frame from frame Buffer
            frame = frameBuff->mid (location, frameLen);
        }
        currFrameTime = ((quint64)header.seconds * ONE_MILISEC) + header.mSec;

        if (locationBuff.count () > 0)
        {
            // read header of next frame
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

        // if buffer size crossed lower threshold limit
        // AND stream is paused
        if ((currBuffSize <= minThreshold) && (throttleFlag == true))
        {
            throttleFlag = false;

            if(mPlaybackMedia)
            {
                mPlaybackMedia->setPauseFlag(false);
            }

            // emit lower threshold cross signal
            emit sigBufferThreshold (CROSSED_MINIMUM);
        }

        // set status to true
        status = true;
    }

    // unlock access to buffers
    bufferLock.unlock ();

    // return status
    return status;
}

//*****************************************************************************
//  run ()
//      Param:
//          IN : NONE
//          OUT: NONE
//
//	Returns:
//          NONE
//	Description:
//          This is a feeder function, which reads frames and headers from
//          buffer, one at a time, and feeds it to decoder. It also maintains
//          delay between consecutive frames.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
void BufferFeeder::run ()
{
    bool status=true;
    bool feedFrame = true;

    DEVICE_REPLY_TYPE_e statusId = CMD_PLAYBACK_TIME;
    quint8 videoLoss = MAX_VIDEO_LOSS_STATUS;

    FRAME_INFO_t    frameInfo;
    QByteArray      frame;
    QDateTime       dt;
    FRAME_HEADER_t  header;

    quint64 frameDelay = 0;
    quint64 emitTime = 0;
    quint64 currFrameTime;

    quint64 nextFrameTime;
    quint64 prevFrameTime = 0;

    qint64 time1 = 0, time2 = 0, diff = 0;

    prctl(PR_SET_NAME, "BUF_FEDER", 0, 0, 0);

    frameInfo.frameWidth = 0;
    frameInfo.frameHeight = 0;
    DECODER_ERROR_e decError = MAX_DEC_ERROR;

    // LOOP till run flag is true
    runFlagLock.lockForRead ();

    while (runFlag == true)
    {
        runFlagLock.unlock ();
        // sleep for frame fetch delay time
        msleep (frameDelay);

        time1 = dt.currentMSecsSinceEpoch ();

        // read frame from buffer
        status = readFrame (frame, header, currFrameTime, nextFrameTime);
        // IF failed to read frame from buffer
        if (status == false)
        {
            // set next frame fetch delay to default
            frameDelay = DEFAULT_FRAME_DELAY;
        }
        // ELSE
        else
        {
            // IF stream status is not STREAM_NORMAL
            if (header.streamStatus != STREAM_NORMAL)
            {
                // set appropriate status id
                switch (header.streamStatus)
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

                // set run flag to false
                setRunFlag (false);
            }
            // ELSE
            else
            {
                //Cal delay for read next frame
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

                // IF previous frame second is not equal to current frame second
                if (emitTime > ONE_MILISEC)
                {
                    // EMIT signal with current frame time
                    emit sigFeederResponse (CMD_PLAYBACK_TIME,
                                            QString ("%1").arg (currFrameTime));
                    prevFrameTime = currFrameTime;
                }

                switch(header.streamType)
                {
                case STREAM_TYPE_AUDIO:
                    feedFrame = true;
                    break;

                case STREAM_TYPE_VIDEO:
                    if(header.videoLoss != videoLoss)
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

                // ENDIF
                // feed frame to decoder
                if(feedFrame == true)
                {
                    //Intilized frame info and send it to decoder
                    frameInfo.framePayload = frame.data ();
                    frameInfo.frameSize = header.frameSize - sizeof (header);
                    frameInfo.mediaType = (STREAM_TYPE_e)header.streamType;
                    frameInfo.codecType = (STREAM_CODEC_TYPE_e)header.codecType;
                    frameInfo.frameType = (FRAME_TYPE_e)header.frameType;
                    if(pbSpeed != PB_SPEED_NORMAL)
                    {
                        frameInfo.frameTimeSec = 0;
                        frameInfo.frameTimeMSec = 0;
                    }
                    else
                    {
                        frameInfo.frameTimeSec = header.seconds;
                        frameInfo.frameTimeMSec = header.mSec;
                    }

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
                            frameInfo.frameWidth
                                    = frameResolution[header.resolution][MediaRequest::FRAME_WIDTH];
                            frameInfo.frameHeight
                                    = frameResolution[header.resolution][MediaRequest::FRAME_HEIGHT];
                        }
                    }
                    else
                    {
                        if((header.resolution != 0) && (header.resolution < MAX_RESOLUTION))
                        {
                            frameInfo.frameWidth
                                    = frameResolution[header.resolution][MediaRequest::FRAME_WIDTH];
                            frameInfo.frameHeight
                                    = frameResolution[header.resolution][MediaRequest::FRAME_HEIGHT];
                        }
                    }

                    frameInfo.noOfReferanceFrame = header.noOfRefFrame;

                    if((header.version == 2)  && (header.frameRate != 0) && (header.frameRate < 100))
                        frameInfo.frameRate = header.frameRate;
                    else
                        frameInfo.frameRate = DEFAULT_FRAME_RATE;

                    //Update Fps according to playback speed
                    getFrameRate(frameInfo.frameRate);
                    if((frameInfo.frameWidth != 0)
                            && (frameInfo.frameHeight != 0))
                    {
                        status = DecodeDispFrame(decId, &frameInfo,&decError);
                    }
                    else
                    {
                        status = true;
                    }
                    // IF decoder returned error
                    if (status == false)
                    {
                        if (DEC_ERROR_NO_CAPACITY == decError)
                        {
                            EPRINT(GUI_PB_MEDIA, "no more decoding capacity: [decId=%d]", decId);
                            statusId = CMD_DECODER_CAPACITY_ERROR;
                        }
                        else
                        {
                            EPRINT(GUI_PB_MEDIA, "decoder error: [decId=%d], [decError=%d]", decId, decError);
                            statusId = CMD_DECODER_ERROR;
                        }

                        // set run flag to false
                        setRunFlag (false);
                    }
                    else
                    {
                        // get next frame fetch delay
                        nextFrameDelay (frameDelay);
                    }
                }
                else
                {
                    // get next frame fetch delay
                    nextFrameDelay (frameDelay);
                }

                time2 = dt.currentMSecsSinceEpoch ();

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
            // ENDIF
        }
        // ENDIF
        runFlagLock.lockForRead ();
    }

    runFlagLock.unlock ();
    // ENDLOOP

    if (!((statusId == CMD_PLAYBACK_TIME) ||
          (statusId == CMD_STREAM_VIDEO_LOSS) ||
          (statusId == CMD_STREAM_NO_VIDEO_LOSS)))
    {
        // EMIT signal with status id
        emit sigFeederResponse (statusId, "");
    }
}

//*****************************************************************************
//  setRunFlag ()
//      Param:
//          IN : bool flag
//          OUT: NONE
//
//	Returns:
//          NONE
//	Description:
//          This API sets run flag status to input flag status.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
void BufferFeeder::setRunFlag (bool flag)
{
    // lock write access to run flag
    runFlagLock.lockForWrite ();
    // set run flag to flag
    runFlag = flag;
    // unlock access to run flag
    runFlagLock.unlock ();
}

PB_SPEED_e BufferFeeder::SetPbSpeed (PB_SPEED_e speed)
{
    pbSpeed = speed;

    if (speed <= PB_SPEED_NORMAL)
    {
        speed = (PB_SPEED_e)0;
    }
    else
    {
        speed = (PB_SPEED_e)(speed - PB_SPEED_NORMAL);
    }

   return speed;
}

//*****************************************************************************
//  setRunFlag ()
//      Param:
//          IN : bool flag
//          OUT: NONE
//
//	Returns:
//          NONE
//	Description:
//          This API sets run flag status to input flag status.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool BufferFeeder::nextFrameDelay (quint64 &delay)
{
    bool status = true;

    switch (pbSpeed) {

    case PB_SPEED_16S:
        delay *= 16;
        break;

    case PB_SPEED_8S:
        delay *= 8;
        break;

    case PB_SPEED_4S:
        delay *= 4;
        break;

    case PB_SPEED_2S:
        delay *= 2;
        break;

    case PB_SPEED_2F:
        delay /= 2;
        break;

    case PB_SPEED_4F:
        delay /= 4;
        break;

    case PB_SPEED_8F:
        delay /= 8;
        break;

    case PB_SPEED_16F:
        delay /= 16;
        break;

    default:
        if(delay > ONE_MILISEC)
        {
            delay = ONE_MILISEC;
        }
        break;
    }
    return status;
}

/**
 * @brief BufferFeeder::getFrameRate
 * @param fps
 * @return
 */
bool BufferFeeder::getFrameRate(UINT16 &fps)
{

    bool status = true;
    switch (pbSpeed) {
    case PB_SPEED_2F:
        fps *= 2;
        break;
    case PB_SPEED_4F:
        fps *= 4;
        break;
    case PB_SPEED_8F:
    case PB_SPEED_16F:
        fps = DEFAULT_FRAME_RATE;
        break;
    default:
        break;
    }
    return status;
}
