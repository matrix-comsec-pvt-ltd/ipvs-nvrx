/***********************************************************************************************
 *	Copyright (c) 2018, Matrix ComSec Pvt. Ltd.
 *
 * 	All right reserved. Matrix's source code is an unpublished work and the use
 *	of copyright notice does not imply otherwise.
 *
 *	This source code contains confidential, trade secret material of Matrix Telecom.
 *	Any attempt or participation in deciphering, decoding, reverse engineering or
 *	in any way altering the source code is strictly prohibited, unless the prior written
 *	consent of Matrix is obtained.
 *
 ***********************************************************************************************/

/***********************************************************************************************
* @INCLUDES
***********************************************************************************************/
#include "SyncBufferFeeder.h"
#include "../MediaRequest.h"

#include <QDateTime>
#include <sys/prctl.h>

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/
/* default delay when no frames are available */
#define DEFAULT_FRAME_DELAY_MS      50

/* sync window time */
#define SYNC_PLAYBACK_TIME_WINDOW   (1 * MSEC_IN_ONE_SEC)

/* Consider playback of single camer with D1 resolution & 512 Kbps bitrate.
 * We have buffer of 48 MB to store frames. We send pause after 32 MB of buffer is filled.
 * So this buffer should accomodate address of total frames of 32 MB storage.
 */
#define MAX_SYNC_BUFFER             (100 * ONE_KILOBYTE)

/***********************************************************************************************
* @VARIABLES
***********************************************************************************************/
char * SyncBufferFeeder::mostReadPos = NULL;
QMutex SyncBufferFeeder::mostReadPosAccess;
quint64 SyncBufferFeeder::playbackReferenceTime = 0;
QMutex  SyncBufferFeeder::referenceTimeMutexLock;

/***********************************************************************************************
* @FUNCTION DEFINATION
***********************************************************************************************/
/**
 * @brief initializes the object with buffer size and threshold levels.
 * @param decoderId
 * @param sesId
 */
SyncBufferFeeder::SyncBufferFeeder (quint8 decoderId, quint8 sesId): syncPbSpeed(0)
{
    locationBuff.reserve (MAX_SYNC_BUFFER);
    runFlag = true;
    decId = decoderId;
    pbSpeed = PB_SPEED_NORMAL;
    bufFeedId = sesId;
    stepFlag = STEP_STATE_FALSE;
    feedFrame = true;
    isForward = true;
    direction = 0;
    currFrameTime = 0;
    isFirstIFrameReceived = false;
    fwidth = fheight = fCurrFPS = 0;
    isFirstFramePlayed = false;
	restartWithIFrame = false;
}

/**
 * @brief deletes the frame buffer
 */
SyncBufferFeeder::~SyncBufferFeeder(void)
{
    resetBufData();
}

/**
 * @brief SyncBufferFeeder::storeLocationBuf
 * @param location
 */
void SyncBufferFeeder::storeLocationBuf(char *location)
{
    bool addFrameInBuff = false;

    bufferRdWr.lockForWrite();

    if (locationBuff.isEmpty())
    {
        FRAME_HEADER_t *header = (FRAME_HEADER_t *)location;
        if (header->frameType == I_FRAME)
        {
            isFirstIFrameReceived = true;
        }
        else if (header->streamStatus != STREAM_NORMAL)
        {
            addFrameInBuff = true;
        }
    }

    if ((isFirstIFrameReceived == true) || (addFrameInBuff == true))
    {
        if (locationBuff.count() < MAX_SYNC_BUFFER)
        {
            /* store current location in location buffer */
            FRAME_HEADER_t *header = (FRAME_HEADER_t *)location;
            if (header->magicCode == 0x000001FF)
            {
                locationBuff.append(location);
            }
        }
    }

    bufferRdWr.unlock();
}

/**
 * @brief SyncBufferFeeder::setMostReadPos
 * @param setPos
 */
void SyncBufferFeeder::setMostReadPos(char *setPos)
{
    mostReadPosAccess.lock();
    mostReadPos = setPos;
    mostReadPosAccess.unlock();
}

/**
 * @brief SyncBufferFeeder::getMostReadPos
 * @param readPos
 */
void SyncBufferFeeder::getMostReadPos(char **readPos)
{
    mostReadPosAccess.lock ();
    *readPos = mostReadPos;
    mostReadPosAccess.unlock ();
}

/**
 * @brief SyncBufferFeeder::resetMostReadPos
 */
void SyncBufferFeeder::resetMostReadPos(void)
{
    mostReadPosAccess.lock();
    mostReadPos = NULL;
    mostReadPosAccess.unlock();
}

/**
 * @brief SyncBufferFeeder::setPlaybackReferenceTime
 * @param frameTime
 */
void SyncBufferFeeder::setPlaybackReferenceTime(quint64 frameTime)
{
    referenceTimeMutexLock.lock();
    playbackReferenceTime = frameTime;
    referenceTimeMutexLock.unlock();
}

/**
 * @brief SyncBufferFeeder::getPlaybackReferenceTime
 * @return
 */
quint64 SyncBufferFeeder::getPlaybackReferenceTime(void)
{
    quint64 tempFrameTime;

    referenceTimeMutexLock.lock();
    tempFrameTime = playbackReferenceTime;
    referenceTimeMutexLock.unlock();

    return tempFrameTime;
}

/**
 * @brief   read a frame and header from the buffer. It also outputs frame delay for next frame read.
 *          If current frame read cause buffer size to cross minimum threshold,
 *          it emits minimum threshold cross signal.
 * @param   header
 * @param   frameDelay
 * @return
 */
bool SyncBufferFeeder::readFrame(char **header, quint64 &frameDelay)
{
    bool playFrame = true;
    quint8 frameStreamStatus = STREAM_NORMAL;
    quint64 referenceTime = 0;
    quint64 frameTime = 0;
    FRAME_HEADER_t *frameHeader = nullptr;

    /* read time of next available frame */
    if (readNextFrameTime(frameTime, frameStreamStatus) == false)
    {
        /* frame not available */
        emit sigSetLowestTime(bufFeedId, false, 0);
        return false;
    }

    do
    {
        /* in case frame is with error, just pass frame header with no frame delay */
        if (STREAM_NORMAL != frameStreamStatus)
        {
            break;
        }

		bufferRdWr.lockForRead();

		/* wait for minimum two frames to calculate the sleep interval */
		if (locationBuff.count() < 2)
		{
			bufferRdWr.unlock();
			/* enough frame not available */
			emit sigSetLowestTime(bufFeedId, false, 0);
			return false;
		}
		bufferRdWr.unlock();

        /* in case of step frame don't check/apply delay as frames are played on user demand */
        if (getStepFlag() == STEP_STATE_TRUE)
        {
            break;
        }

        /* get the reference time */
        referenceTime = SyncBufferFeeder::getPlaybackReferenceTime();

        /* compare this frame time with reference time which will be lowest of all cameras in sync playback */
        if (direction == FORWARD_PLAY)
        {
            /* if current frame time is greater than reference time & difference is more than sync time window */
            if ((frameTime > referenceTime) && ((frameTime - referenceTime) > SYNC_PLAYBACK_TIME_WINDOW))
            {
                playFrame = false;
            }
        }
        else
        {
            /* if current frame time is smaller than reference time & difference is less than sync time window */
            if ((frameTime < referenceTime) && ((referenceTime - frameTime) > SYNC_PLAYBACK_TIME_WINDOW))
            {
                playFrame = false;
            }
        }

    }while (0);

    /* if frame is not going to be played, update frame time only */
    if (false == playFrame)
    {
        /* set whatever time is received */
        emit sigSetLowestTime(bufFeedId, false, frameTime);
        return (false);
    }

    /* get the time difference between frames to sleep */
    frameDelay = GetNextFrameDelay();

    bufferRdWr.lockForWrite();

    /* read the (header + frame) start location */
    if (true == locationBuff.isEmpty())
    {
        bufferRdWr.unlock();
        emit sigSetLowestTime(bufFeedId, false, 0);
        return false;
    }

    *header = locationBuff.takeFirst();

    /* set read position */
    setMostReadPos(*header);

    /* save current frame time */
    frameHeader = (FRAME_HEADER_t *)*header;
    currFrameTime = ((quint64)frameHeader->seconds * 1000) + frameHeader->mSec;

    bufferRdWr.unlock();

    /* send the current frame time to calculate lowest sync time */
    emit sigSetLowestTime(bufFeedId, getFeedFrameFlag(), frameTime);

    return true;
}

/**
 * @brief SyncBufferFeeder::readNextFrameTime
 * @param nextFrameTime
 * @param nextFrameStatus
 * @return
 */
bool SyncBufferFeeder::readNextFrameTime(quint64 &nextFrameTime, quint8 &nextFrameStatus)
{
    FRAME_HEADER_t *header;

    bufferRdWr.lockForRead();

    /* if there is frame to read in buffer */
    if (true == locationBuff.isEmpty())
    {
        bufferRdWr.unlock();
        return false;
    }

    /* don't remove frame from buffer, just check the time */
    header = (FRAME_HEADER_t *)locationBuff.first();
    bufferRdWr.unlock();

    /* time in ms */
    nextFrameTime = ((quint64)header->seconds * MSEC_IN_ONE_SEC) + header->mSec;

    /* header status */
    nextFrameStatus = header->streamStatus;

    return true;
}

/**
 * @brief SyncBufferFeeder::GetNextFrameDelay
 * @return
 */
quint64 SyncBufferFeeder::GetNextFrameDelay(void)
{
    quint64 firstFrameTimeMs = 0, secondFrameTimeMs = 0, delay = 0;
    FRAME_HEADER_t *pHeader = nullptr;

    bufferRdWr.lockForRead();

//    EPRINT(GUI_SYNC_PB_MEDIA, "frame count: [bufFeedId=%d], [count=%u]", bufFeedId, locationBuff.count());

    if (locationBuff.count() < 2)
    {
        bufferRdWr.unlock();
        return (DEFAULT_FRAME_DELAY_MS);
    }

    /* get first frame timestamp */
    pHeader = (FRAME_HEADER_t *)locationBuff.at(0);
    firstFrameTimeMs = (pHeader->seconds * 1000) + (quint64)(pHeader->mSec);

    /* get second frame timestamp */
    pHeader = (FRAME_HEADER_t *)locationBuff.at(1);
    secondFrameTimeMs = (pHeader->seconds * 1000) + (quint64)(pHeader->mSec);

    bufferRdWr.unlock();

    if (secondFrameTimeMs >= firstFrameTimeMs)
    {
        delay = (secondFrameTimeMs - firstFrameTimeMs);
    }
    else
    {
        delay = (firstFrameTimeMs - secondFrameTimeMs);
    }

    //EPRINT(GUI_SYNC_PB_MEDIA, "consecutive frame delay: [bufFeedId=%d], [delay=%llu ms]", bufFeedId, delay);

    /* now update delay based on playback speed */
    switch (pbSpeed)
    {
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

    /* when speed is 16x, server put alternate I-frame to buffer due to which
     * no change in terms of speed is visible to user compare to 8x speed */
    case PB_SPEED_16F:
        delay /= 32;
        break;

    case PB_SPEED_NORMAL:
    default:
        break;
    }

    /*
     * In case of any playback (e.g. Adaptive Recording), if frame difference is greater than 1 second then
     * sleep for maximum 1 second to view video playing properly.
     * e.g.
     * consider adaptive recording of video with 5 FPS & 50 GOP. So when we play syncronize playback,
     * each frame will play after 10 second which is not proper for practicaly useful.
     */
    if (delay > MSEC_IN_ONE_SEC)
    {
        delay = MSEC_IN_ONE_SEC;
    }

    return (delay);
}

/**
 * @brief   This is a feeder function, which reads frames and headers from
 *          buffer, one at a time, and feeds it to decoder. It also maintains delay between consecutive frames.
 */
void SyncBufferFeeder::run(void)
{
    bool tFeedFrame = true;
	bool ret = false;
    char *frameStartPos = NULL;

    quint64 frameDelay = 0;

    quint64 time1 = 0, time2 = 0, diff = 0;
    QDateTime dt;

    FRAME_HEADER_t *header = NULL;
    FRAME_INFO_t frameInfo;

    DEVICE_REPLY_TYPE_e statusId = CMD_PLAYBACK_TIME;
    quint8 videoLoss = NO_VIDEO_LOSS;
    DECODER_ERROR_e decError = MAX_DEC_ERROR;

    /* set thread name */
    SetThreadName();

    /* loop till run flag is true */
    while (true == getRunFlag())
    {
		if (getStepFlag() == STEP_STATE_TRUE)
        {
			stepSigLock.lock();
			ret = stepSig.wait(&stepSigLock, (5 * MSEC_IN_ONE_SEC));
			stepSigLock.unlock();

			/* wait() will return false if the wait timed out */
			if (false == ret)
			{
				continue;
			}

            /* reset default frame delay in case  user continue play after step frames */
			frameDelay = DEFAULT_FRAME_DELAY_MS;
        }
        else
        {
            if (frameDelay > 0)
            {
                /* sleep for frame fetch delay time */
                msleep(frameDelay);
            }
        }

        time1 = dt.currentMSecsSinceEpoch();

        /* read frame from buffer */
        if (false == readFrame(&frameStartPos, frameDelay))
        {
            /* reset default frame delay in case  user continue play after step frames */
            frameDelay = DEFAULT_FRAME_DELAY_MS;

            continue;
        }

        /* get header */
        header = (FRAME_HEADER_t *)frameStartPos;

        /* validate header */
        if (!IS_VALID_OBJ(header))
        {
            resetBufData();
            EPRINT(GUI_SYNC_PB_MEDIA, "invalid header pointer: [bufFeedId=%d]", bufFeedId);
            continue;
        }

        /* validate header magic code */
        if (header->magicCode != 0x000001FF)
        {
            resetBufData();
            EPRINT(GUI_SYNC_PB_MEDIA, "header magic number mismatch: [bufFeedId=%d]", bufFeedId);
            continue;
        }

        /* error in video stream */
        if ((header->streamStatus != STREAM_NORMAL) && (header->streamStatus != STREAM_SYNC_START_INDICATOR))
        {
            /* set appropriate status id */
            switch (header->streamStatus)
            {
            default:
                EPRINT(GUI_SYNC_PB_MEDIA, "header status not normal: [bufFeedId=%d], [streamStatus=%d]", bufFeedId, header->streamStatus);
                statusId = CMD_REC_MEDIA_ERR;
                break;

            case STREAM_FILE_ERROR:
                statusId = CMD_STREAM_FILE_ERROR;
                EPRINT(GUI_SYNC_PB_MEDIA,"file error: [bufFeedId=%d]", bufFeedId);
                break;

            case STREAM_HDD_FORMAT:
                statusId = CMD_STREAM_HDD_FORMAT;
                EPRINT(GUI_SYNC_PB_MEDIA, "hdd formatting: [bufFeedId=%d]", bufFeedId);
                break;

            case STREAM_CONFIG_CHANGE:
                EPRINT(GUI_SYNC_PB_MEDIA, "config change: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_STREAM_CONFIG_CHANGE;
                break;

            case STREAM_PLAYBACK_OVER:
                EPRINT(GUI_SYNC_PB_MEDIA, "playback over: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_STREAM_PLAYBACK_OVER;
                break;

            case STREAM_PLAYBACK_SESSION_NOT_AVAIL:
                EPRINT(GUI_SYNC_PB_MEDIA, "session not available: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_MAX_STREAM_LIMIT;
                break;

            case STREAM_PLAYBACK_CAM_PREVILAGE:
                EPRINT(GUI_SYNC_PB_MEDIA, "no user privilege: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_NO_PRIVILEGE;
                break;

            case MEDIA_REC_DRIVE_CONFIG_CHANGE:
                EPRINT(GUI_SYNC_PB_MEDIA, "drive config change: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_REC_DRIVE_CONFIG_CHANGES;
                break;

            case MEDIA_OTHER_ERROR:
                EPRINT(GUI_SYNC_PB_MEDIA, "media error: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_REC_MEDIA_ERR;
                break;

            case STREAM_PLAYBACK_PROCESS_ERROR:
                EPRINT(GUI_SYNC_PB_MEDIA, "playback processing error: [bufFeedId=%d]", bufFeedId);
                statusId = CMD_PLAYBACK_PROCESS_ERROR;
            break;
            }

            /* send status to UI */
            emit sigFeederResponse (statusId, 0, bufFeedId);

            /* continue to receive next frame */
            continue;
        }

        /* feed frame flag */
        tFeedFrame = (header->streamStatus == STREAM_SYNC_START_INDICATOR) ? false : true;

        /* check for video loss */
        switch (header->streamType)
        {
        case STREAM_TYPE_VIDEO:

            if (header->videoLoss != videoLoss)
            {
                videoLoss = (quint8)header->videoLoss;

                if (videoLoss == VIDEO_LOSS)
                {
                    statusId = CMD_STREAM_VIDEO_LOSS;
                    tFeedFrame = false;

                    /* send status to UI */
                    emit sigFeederResponse (statusId, 0, bufFeedId);

                    EPRINT(GUI_SYNC_PB_MEDIA, "video loss declared: [bufFeedId=%d]", bufFeedId);
                    continue;
                }

                statusId = CMD_STREAM_NO_VIDEO_LOSS;

                /* send status to UI */
                emit sigFeederResponse (statusId, 0, bufFeedId);

                EPRINT(GUI_SYNC_PB_MEDIA, "video loss recovered: [bufFeedId=%d]", bufFeedId);
            }
            else if(videoLoss == VIDEO_LOSS)
            {
                tFeedFrame = false;
            }
            break;

        default:
            break;
        }

        if (false == getFeedFrameFlag())
        {
            /* receive next frame */
            continue;
        }

		/* Do not feed frames to decoder for cameras which are not visible in current layout */
		if ((isFirstFramePlayed == true) && (false == ValidateDecoderSyncPb(decId)))
		{
			/* This flag purpose is to remove glitch when camera comeback in the current layout
				In such case restart playback with next I-frame */
			restartWithIFrame = true;
			continue;
		}

        /* skip audio frames in case playback speed is not 1x */
        if ((false == tFeedFrame) ||
            ((PB_SPEED_NORMAL != pbSpeed) && (STREAM_TYPE_AUDIO == (STREAM_TYPE_e)header->streamType)))
        {
            continue;
        }

        /* get video resolution */
        if (false == GetVideoResolution(header, (frameStartPos + sizeof(FRAME_HEADER_t)), frameInfo.frameWidth, frameInfo.frameHeight))
        {
            continue;
        }

		if ((restartWithIFrame == true) && (STREAM_TYPE_VIDEO == (STREAM_TYPE_e)header->streamType))
		{
			if (I_FRAME != (FRAME_TYPE_e)header->frameType)
			{
				continue;
			}
			restartWithIFrame = false;
		}

        /* feed frame to decoder */
        frameInfo.frameTimeSec = 0;
        frameInfo.frameTimeMSec = 0;
        frameInfo.framePayload = (frameStartPos + sizeof(FRAME_HEADER_t));
        frameInfo.frameSize = (header->frameSize - sizeof (FRAME_HEADER_t));
        frameInfo.mediaType = (STREAM_TYPE_e)header->streamType;
        frameInfo.codecType = (STREAM_CODEC_TYPE_e)header->codecType;
        frameInfo.frameType = (FRAME_TYPE_e)header->frameType;
        frameInfo.noOfReferanceFrame = header->noOfRefFrame;

        if (pbSpeed == PB_SPEED_NORMAL)
        {
            frameInfo.frameTimeSec = header->seconds;
            frameInfo.frameTimeMSec = header->mSec;
        }

        /* consider the fps written in metadata for header version=2 and onwards */
        if ((header->version >= 2) && (header->frameRate != 0))
        {
            frameInfo.frameRate = header->frameRate;
        }
        else
        {
            frameInfo.frameRate = DEFAULT_FRAME_RATE;
        }

        /* save latest fps, height & width to calculate decoding capacity when changing playback speed */
        if (fCurrFPS != frameInfo.frameRate)
        {
            fCurrFPS = frameInfo.frameRate;
        }

        if (fheight != frameInfo.frameHeight)
        {
            fheight = frameInfo.frameHeight;
        }

        if (fwidth != frameInfo.frameWidth)
        {
            fwidth  = frameInfo.frameWidth;
        }

        /* update FPS according to playback speed */
        getFrameRateForPlaySpeed(frameInfo.frameRate);

        /* if decoder returned error */
        if (false == DecodeDispFrame(decId, &frameInfo, &decError))
        {
            if (DEC_ERROR_NO_CAPACITY == decError)
            {
                EPRINT(GUI_SYNC_PB_MEDIA, "no more decoding capacity: [bufFeedId=%d], [decId=%d]", bufFeedId, decId);
                statusId = CMD_DECODER_CAPACITY_ERROR;
            }
            else
            {
                EPRINT(GUI_SYNC_PB_MEDIA, "decoder error: [bufFeedId=%d], [decId=%d], [decError=%d]", bufFeedId, decId, decError);
                statusId = CMD_DECODER_ERROR;
            }

            /* make flag false */
            setFeedFrameFlag(false);

            /* Not consider this in decoding capacity */
            fwidth = fheight = fCurrFPS = 0;

            /* send status to UI */
            emit sigFeederResponse(statusId, 0, bufFeedId);
		}
		else
		{
			if (false == isFirstFramePlayed)
			{
				isFirstFramePlayed = true;
			}
		}

//		DPRINT(GUI_SYNC_PB_MEDIA, "frame delay: [bufFeedId=%d], [frame_type=%d], [time=%u sec], [delay=%llu ms]",
//				 bufFeedId, frameInfo.frameType, header->seconds, frameDelay);

        /* send current time for playback UI */
        statusId = CMD_PLAYBACK_TIME;
        emit sigFeederResponse(statusId, header->seconds, bufFeedId);

        diff = 0;
        time2 = dt.currentMSecsSinceEpoch();

        /* derive execution time of above code & decoder */
        if (time2 > time1)
        {
            diff = (time2 - time1);
        }

        /* dynamic sleep based on execution time taken */
        if (frameDelay >= (quint64)diff)
        {
            frameDelay -= (quint64)diff;
        }
        else
        {
            frameDelay = 0;
        }
    }

    /* clear data buffered that was allocated */
    clearBufData();

    DPRINT(GUI_SYNC_PB_MEDIA, "buffer feeder thread exit: [bufFeedId=%d]", bufFeedId);
}

/**
 * @brief sets run flag status to input flag status
 * @param flag
 */
void SyncBufferFeeder::setRunFlag(bool flag)
{
    runFlagLock.lockForWrite();
    runFlag = flag;
    runFlagLock.unlock();
}

/**
 * @brief SyncBufferFeeder::getRunFlag
 * @return
 */
bool SyncBufferFeeder::getRunFlag(void)
{
    bool runF = false;

    runFlagLock.lockForRead();
    runF = runFlag;
    runFlagLock.unlock();
    return runF;
}

/**
 * @brief sets run flag status to input flag status.
 * @param speed
 * @param direction
 */
void SyncBufferFeeder::SetPbSpeedBufFeeder(PB_SPEED_e speed, quint8 dir)
{
    pbSpeed = speed;

    if (pbSpeed > PB_SPEED_NORMAL)
    {
        syncPbSpeed = (quint32)((pbSpeed - PB_SPEED_NORMAL) + 1);
    }
    else
    {
        syncPbSpeed = 1;
    }

    direction = dir;
}

/**
 * @brief sets run flag status to input flag status.
 */
void SyncBufferFeeder::SendStepSig(void)
{
    stepSigLock.lock ();
    stepSig.wakeAll ();
    stepSigLock.unlock ();
}

/**
 * @brief SyncBufferFeeder::setFeedFrameFlag
 * @param flag
 */
void SyncBufferFeeder::setFeedFrameFlag(bool flag)
{
    feedFrameLock.lock ();
    feedFrame = flag;
    feedFrameLock.unlock ();

    emit sigSetLowestTime(bufFeedId, flag, currFrameTime);
}

/**
 * @brief SyncBufferFeeder::getFeedFrameFlag
 * @return
 */
bool SyncBufferFeeder::getFeedFrameFlag(void)
{
    bool feedFlag =  false;

    feedFrameLock.lock ();
    feedFlag = feedFrame;
    feedFrameLock.unlock ();

    return feedFlag;
}

/**
 * @brief SyncBufferFeeder::resetBufData
 */
void SyncBufferFeeder::resetBufData(void)
{
    bufferRdWr.lockForWrite();
    locationBuff.clear();
    locationBuff.reserve(MAX_SYNC_BUFFER);
    isFirstIFrameReceived = false;
	isFirstFramePlayed = false;
	restartWithIFrame = false;
    bufferRdWr.unlock();
    resetMostReadPos();
}

/**
 * @brief SyncBufferFeeder::clearBufData
 */
void SyncBufferFeeder::clearBufData(void)
{
    bufferRdWr.lockForWrite();
    locationBuff.clear();
    bufferRdWr.unlock();
	isFirstFramePlayed = false;
}

/**
 * @brief SyncBufferFeeder::setStepFlag
 * @param flagState
 */
void SyncBufferFeeder::setStepFlag(STEP_STATE_e flagState)
{
    stepSigLock.lock ();
    if (stepFlag != MAX_STEP_STATE)
    {
        stepFlag = flagState;
    }
    stepSigLock.unlock ();
}

/**
 * @brief SyncBufferFeeder::getStepFlag
 * @return
 */
bool SyncBufferFeeder::getStepFlag(void)
{
	bool tStepFlag =  false;

	stepSigLock.lock ();
	tStepFlag = stepFlag;
	stepSigLock.unlock ();
	return tStepFlag;
}

/**
 * @brief SyncBufferFeeder::getFrameRateForPlaySpeed
 * @param fps
 */
void SyncBufferFeeder::getFrameRateForPlaySpeed(UINT16 &fps)
{
	/* for reverse playback fps is same which is received in frame header */
    if (direction != FORWARD_PLAY)
    {
        return;
    }

    switch (pbSpeed)
    {
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
}

/**
 * @brief calculate required decoding capacity
 * @param speed : new speed
 * @param direction : playback direction
 * @return
 */
quint64 SyncBufferFeeder::GetRequiredDecodingCapacity(PB_SPEED_e speed, quint8 playbackDirection)
{
    quint64 reqCapacity = 0;
    quint8  factor = 1;

    if (playbackDirection == FORWARD_PLAY)
    {
        /* multiplication factor for fps based on playback speed */
        if (speed == PB_SPEED_2F)
        {
            factor = 2;
        }
        else if (speed == PB_SPEED_4F)
        {
            factor = 4;
        }
    }

    /* calculate required capacity based on current FPS & resolution */
    reqCapacity = ((quint64)fheight * fwidth * fCurrFPS * factor);

    DPRINT(GUI_SYNC_PB_MEDIA, "required decoding capacity: [bufFeedId=%d], [direction=%d], [speed=%d], [res=%ux%u], [fps=%d], [factor=%d], [capacity=%llu]",
                              bufFeedId, playbackDirection, speed, fwidth, fheight, fCurrFPS, factor, reqCapacity);

    return (reqCapacity);
}

/**
 * @brief LiveMedia::SetThreadName
 */
void SyncBufferFeeder::SetThreadName(void)
{
    /* thread name */
	char threadName[17];

    /* set thread name */
    snprintf(threadName, sizeof(threadName), "SYNC_BUF_FED_%d", bufFeedId);
    prctl(PR_SET_NAME, threadName, 0, 0, 0);
}

/**
 * @brief SyncBufferFeeder::GetVideoResolution
 * @param header
 * @param frameData
 * @param frameWidth
 * @param frameHeight
 * @return
 */
bool SyncBufferFeeder::GetVideoResolution(FRAME_HEADER_t *header, char *frameData, UINT16 &frameWidth, UINT16 &frameHeight)
{
    /* verify audio parameters */
    if (header->streamType != STREAM_TYPE_VIDEO)
    {
        return (true);
    }

    /* video frame resolution */
    if (I_FRAME == (FRAME_TYPE_e)header->frameType)
    {
        /* parse frame header to extract frame resolution if actual resolution not provided */
        if (header->resolution == 0)
        {
            if (false == (VideoStreamParser::getHeightWidth(frameData, header, frameWidth, frameHeight)))
            {
                return (false);
            }

            /* validate resloution */
            if ((frameWidth == 0) || (frameHeight == 0))
            {
                return (false);
            }
        }
        /* if reslution is provided get width and height */
        else
        {
            if (header->resolution >= MAX_RESOLUTION)
            {
                EPRINT(GUI_SYNC_PB_MEDIA, "invalid video frame resolution: [bufFeedId=%d], [resolution=%dx%d]",
                        bufFeedId, frameWidth, frameHeight);

                return (false);
            }

            frameWidth = frameResolution[header->resolution][MediaRequest::FRAME_WIDTH];
            frameHeight = frameResolution[header->resolution][MediaRequest::FRAME_HEIGHT];
        }
    }

    return (true);
}
