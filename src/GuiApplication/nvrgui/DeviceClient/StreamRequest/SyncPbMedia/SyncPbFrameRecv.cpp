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
#include <sys/prctl.h>
#include "SyncPbFrameRecv.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/
#define PAUSE_STATE_SLEEP_TIME              100 // in mSec
#define STEP_CURRFRAMETIME_READRETRY_TIME   10 // in mSec
#define RECVPAUSE_FLAG_TRUE_TIME            30
#define MAX_RECV_FAIL_COUNT                 15
#define PAUSE_MAR_SIZE                      (16 * ONE_MEGABYTE)
#define RESUME_MAR_SIZE                     (16 * ONE_MEGABYTE)

/***********************************************************************************************
* @FUNCTION DEFINATION
***********************************************************************************************/
/**
 * @brief SyncPbFrameRecv::SyncPbFrameRecv
 * @param serverInfo
 * @param requestInfo
 * @param commandId
 * @param decoderId
 * @param pDecIdSet
 */
SyncPbFrameRecv::SyncPbFrameRecv(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId, quint8 decoderId, const quint8 *pDecIdSet)
    : MediaRequest (serverInfo, requestInfo, commandId, decoderId, pDecIdSet)
{
    for (quint8 index=0; index < MAX_SYNC_PB_SESSION; index++)
    {
        // initialize buffer and feeder request to NULL
        INIT_OBJ(feeder[index]);
		chIdSet[index] = INVALID_CAMERA_INDEX;
        m_currentFrameFeedTime[index] = 0;
    }
    pbSpeed = MAX_PB_SPEED;
    syncFrameNo = 0;
    // store type of command to send
    cmdId = commandId;
    prevFrameTime = 0;
    frameStoreFlag = true;
	direction = MAX_PLAY_DIRECTION;

    recvPauseFlag = false;
    maxCh = 0;
    throttle = false;
    INIT_OBJ(startBufPtr);
    INIT_OBJ(endBufPtr);
}

/**
 * @brief SyncPbFrameRecv::~SyncPbFrameRecv
 */
SyncPbFrameRecv::~SyncPbFrameRecv(void)
{
}

/**
 * @brief SyncPbFrameRecv::setThrottleFlag
 * @param flag
 */
void SyncPbFrameRecv::setThrottleFlag(bool flag)
{
    throttleAccess.lock ();
    throttle = flag;
    throttleAccess.unlock ();
}

/**
 * @brief SyncPbFrameRecv::getThrottleFlag
 * @param flag
 */
bool SyncPbFrameRecv::getThrottleFlag(void)
{
    bool flag;

    throttleAccess.lock();
    flag = throttle;
    throttleAccess.unlock();

    return flag;
}

/**
 * @brief   This API is called from the system thread. It serves as independant instance of execution,
 *          which does the communication with the server. It requests for the playback stream and
 *          starts receiving frames upon success. It stores the frames into the buffer.
 */
void SyncPbFrameRecv::run(void)
{
    QByteArray * frameBuffer = NULL;
    FRAME_HEADER_t *header = NULL;
    QTcpSocket tcpSocket;
    QByteArray frame;
    quint8 chIndex = 0;
    QStringList payloadFields;
    QString tempFsp(FSP);
    quint8 decInstanceAvail = 0;
	PB_SPEED_e tPbSpeed;

    char *readMostPos = NULL;
    char *wrLocInBuffer = NULL, *checkStartPos=NULL;
    quint32 diff = 0;
    quint64 frameLen = 0;

    /* set thread name */
    prctl(PR_SET_NAME, "SYNC_PB_FR_RCV", 0, 0, 0);

    /* allocate common global buffer to store frames of all the sync playback streams */
    frameBuffer = new QByteArray(TOTAL_BUFFER_SIZE, 0);

    /* Intilized Start and end pointer */
    if (!IS_VALID_OBJ(frameBuffer))
    {
        /* EMIT signal with no instance of decoder */
        emit sigMediaResponse(request.requestId, PLYBCK_RCD, CMD_INTERNAL_RESOURCE_LIMIT, "");
        return;
    }

    /* buffer start & end position */
    startBufPtr = frameBuffer->data();
    endBufPtr = startBufPtr + TOTAL_BUFFER_SIZE;

    /* set current write location to start of buffer */
    wrLocInBuffer = startBufPtr;

	/* Create buffer feeder threads */
	if (false == createAllSyncBufFeeder())
	{
        emit sigMediaResponse (request.requestId, PLYBCK_RCD, CMD_DECODER_ERROR, "");
		DELETE_OBJ(frameBuffer);
		return;
	}

    payloadFields = request.payload.split (FSP);

    /* parse & store speed, direction,change camera field */
    direction = payloadFields.at (2).toUInt();

    SetPbSpeed((PB_SPEED_e)payloadFields.value(4).toInt(), direction);

    /* for server speed should be of range [0-4] */
    if (pbSpeed <= PB_SPEED_NORMAL)
    {
        tPbSpeed = (PB_SPEED_e)0;
    }
    else
    {
        tPbSpeed = (PB_SPEED_e)(pbSpeed - PB_SPEED_NORMAL);
    }

    payloadFields.replace (4, QString ("%1").arg (tPbSpeed));
    request.payload = payloadFields.join (tempFsp);

    /* connect to server */
    if (false == connectToServer(tcpSocket))
    {
        emit sigMediaResponse (request.requestId, PLYBCK_RCD, statusId, "");
        DELETE_OBJ(frameBuffer);
        return;
    }

    /* send request to server */
    if (false == sendRequest(tcpSocket))
    {
        emit sigMediaResponse(request.requestId, PLYBCK_RCD, statusId, "");
        DELETE_OBJ(frameBuffer);
        return;
    }

    /* receive response from server */
    if ((false == receiveResponse(tcpSocket, STREAM_RESPONSE_SIZE) || (statusId != CMD_SUCCESS)))
    {
        emit sigMediaResponse(request.requestId, PLYBCK_RCD, statusId, "");
        DELETE_OBJ(frameBuffer);
        return;
    }

    /* emit signal with appropriate status id */
    emit sigMediaResponse(request.requestId, PLYBCK_RCD, statusId, QString ("%1").arg (MAX_SYNC_PB_SESSION - decInstanceAvail));
	/* emit signal to update playback speed on UI */
	QString respPayLoad = QString ("%1").arg (pbSpeed);
	emit sigMediaResponse (request.requestId,
							PLYBCK_RCD,
							CMD_STREAM_PB_SPEED,
							respPayLoad);

    while (true == getRunFlag())
    {
        /* check current loction exit endbuffer */
        if (static_cast<unsigned int>(endBufPtr - wrLocInBuffer) <= sizeof(FRAME_HEADER_t))
        {
            wrLocInBuffer = startBufPtr;
        }

        /* receive frame header */
        if (receiveHeaderToDirLocSyncPb(wrLocInBuffer, tcpSocket) == false)
        {
            /* if playback is paused don't calculate timeout */
            if (false == getRecvPauseFlag())
            {
				/* Failed to get frames then wait for data */
                msleep(500);
            }
            else
            {
                /* check for need to send resume */
                if (true == getThrottleFlag())
                {
                    /* get read & write loaction at current time */
                    SyncBufferFeeder::getMostReadPos(&readMostPos);

                    if (IS_VALID_OBJ(readMostPos))
                    {
                        /* get the occupied space in the buffer */
                        if (wrLocInBuffer > readMostPos)
                        {
                            diff = (wrLocInBuffer - readMostPos);
                        }
                        else
                        {
                            diff = (TOTAL_BUFFER_SIZE - (readMostPos - wrLocInBuffer));
                        }

                        /* if filled data is in buffer is less than RESUME_MAR_SIZE, start receiving new frames */
                        if (diff <= RESUME_MAR_SIZE)
                        {
                            /* send resume signal */
                            DPRINT(GUI_SYNC_PB_MEDIA, "send resume signal: get more data: [buffer_data=%d bytes]", diff);
                            emit sigPbFrameRecvThreshold(CROSSED_MINIMUM);
                            setThrottleFlag(false);
                        }
                    }
                }
            }
        }
        else
        {
            header = (FRAME_HEADER_t *)wrLocInBuffer;

            if (getFrameStoreFlag() == true)
            {
                frameLen = ((quint64)header->frameSize - sizeof(FRAME_HEADER_t));

                /* check current frame size exit endBuffer */
                checkStartPos = wrLocInBuffer + (frameLen + sizeof(FRAME_HEADER_t));

                /* if no enough space at end of the buffer to store full frame, write it to start of the buffer */
                if (checkStartPos > endBufPtr)
                {
                    /* if no space in buffer, reset write location */
                    wrLocInBuffer = startBufPtr;

                    /* copy this header to start of buffer */
                    memcpy(wrLocInBuffer, header, sizeof(FRAME_HEADER_t));
                }

                /* receive frame */
                if (receiveFrameToDirLocSyncPb((wrLocInBuffer + sizeof(FRAME_HEADER_t)), frameLen, tcpSocket) == true)
                {
                    for (chIndex = 0; chIndex < maxCh; chIndex++)
                    {
                        if (chIdSet[chIndex] == header->channel)
                        {
							if (IS_VALID_OBJ(feeder[chIndex]))
							{
								/* store write location to feeder object read list */
								feeder[chIndex]->storeLocationBuf (wrLocInBuffer);

								/* shift write location pointer */
								wrLocInBuffer += (frameLen + sizeof(FRAME_HEADER_t));
							}
                            break;
                        }
                    }

                    /* check wether need to send pause on buffer data threshold */
                    if (false == getThrottleFlag())
                    {
                        SyncBufferFeeder::getMostReadPos(&readMostPos);

                        /* calculate free space in video frame buffer */
                        if (IS_VALID_OBJ(readMostPos))
                        {
                            if (wrLocInBuffer >= readMostPos)
                            {
                                diff = (TOTAL_BUFFER_SIZE - (wrLocInBuffer - readMostPos));
                            }
                            else
                            {
                                diff = (readMostPos - wrLocInBuffer);
                            }
                        }
                        else
                        {
                            /* Verify that buffer will not be overwrite if readPos is invalid */
                            diff = (endBufPtr - wrLocInBuffer);
                        }

                        /* if free space is less than PAUSE_MAR_SIZE, then pause the reception of frames */
                        if (diff <= (PAUSE_MAR_SIZE))
                        {
                            /* send pause signal */
                            DPRINT(GUI_SYNC_PB_MEDIA, "send pause signal: [buffer_data=%d bytes]", (TOTAL_BUFFER_SIZE - diff));

                            emit sigPbFrameRecvThreshold(CROSSED_MAXIMUM);
                            setThrottleFlag(true);
                        }
                    }
				}
                else
                {
                    wrLocInBuffer += sizeof(FRAME_HEADER_t);
                }
            }
            else
            {
                // read frame & descard
                receiveFrame (frame, header->frameSize - sizeof(FRAME_HEADER_t), tcpSocket);

                syncFrameNoAccess.lock ();

                if ((header->streamStatus == STREAM_SYNC_START_INDICATOR) && (header->syncFrameNum == syncFrameNo))
                {
                    DPRINT(GUI_SYNC_PB_MEDIA, "sync indicator: reset data buffer");

                    /* Reset write and read position */
                    wrLocInBuffer = startBufPtr;
                    SyncBufferFeeder::resetMostReadPos();

                    setFrameStoreFlag(true);
                    setRecvPauseFlag(false);
                }
                syncFrameNoAccess.unlock ();
            }
        }
    }

    emit sigMediaResponse(request.requestId, PLYBCK_RCD, statusId, "");

    deleteAllSyncBufFeeder();
    DELETE_OBJ(frameBuffer);
}

/**
 * @brief creates a buffer upto maxCh to store playback stream.
 * @return
 */
bool SyncPbFrameRecv::createAllSyncBufFeeder(void)
{
	quint8	index = 0;
	quint8	tDecCntAvail = 0;

	/* verify decoder instance */
	for (index = 0; index < MAX_SYNC_PB_SESSION; index++)
	{
		if (SyncDecIdSet[index] == INVALID_DEC_DISP_PLAY_ID)
		{
			break;
		}
		else
		{
			tDecCntAvail++;
		}
	}

	/* if enough decoder instance available, then start play */
	if (tDecCntAvail < maxCh)
	{
		EPRINT(GUI_SYNC_PB_MEDIA, "Enough Decoders[%d] are not available", tDecCntAvail);
		return (false);
	}

	/* Create buffer feeder threads */
	for(index = 0; index < maxCh; index++)
	{
		createSyncBufferFeeder(index);
	}

    /* reset writer pointer */
    SyncBufferFeeder::resetMostReadPos();

	return (true);
}

/**
 * @brief creates a buffer to store playback stream. It also starts feeder thread which feedes frames to decoder module.
 * @param sesId
 * @return
 */
bool SyncPbFrameRecv::createSyncBufferFeeder(quint8 sesId)
{
    bool status = false;

    // if buffer is not created
    if (feeder[sesId] == NULL)
    {
        // create a buffer
        feeder[sesId] = new SyncBufferFeeder (SyncDecIdSet[sesId], sesId);

        // if buffer created
        if (feeder[sesId] != NULL)
        {
            /* set stack size of thread */
            feeder[sesId]->setStackSize(SYNC_PB_DECODER_THREAD_STACK_SIZE);

            connect (feeder[sesId],
                     SIGNAL(sigFeederResponse(DEVICE_REPLY_TYPE_e,
                                              quint32,
                                              quint8)),
                     this,
                     SLOT(slotPbMediaResponse(DEVICE_REPLY_TYPE_e,
                                              quint32,
                                              quint8)));

            connect(feeder[sesId],
                    SIGNAL(sigSetLowestTime(quint8,bool,quint64)),
                    this,
                    SLOT(slotSetLowestTime(quint8,bool,quint64)),
                    Qt::DirectConnection);

            // start feeder thread
            feeder[sesId]->start ();

            // set status to true
            status = true;
        }
    }
    // return status
    return status;
}

/**
 * @brief deletes all buffers.
 * @return
 */
bool SyncPbFrameRecv::deleteAllSyncBufFeeder(void)
{
    quint8 index = 0;
    bool status = true;

    for(index = 0; index < maxCh; index++)
    {
        deleteSyncBufferFeeder (index);
    }
    return status;
}

/**
 * @brief deletes the buffer & stops the feeder thread.
 * @param sesId
 * @return
 */
bool SyncPbFrameRecv::deleteSyncBufferFeeder(quint8 sesId)
{
    bool status = false;

    /* if buffer is already created */
    if (IS_VALID_OBJ(feeder[sesId]))
    {
        DPRINT(GUI_SYNC_PB_MEDIA, "delete buffer feeder: [sesId=%d]", sesId);

        /* stop feeder thread */
        feeder[sesId]->setRunFlag(false);
        feeder[sesId]->setStepFlag(MAX_STEP_STATE);
        feeder[sesId]->SendStepSig();

        /* wait for feeder thread to return */
        feeder[sesId]->wait();

        disconnect (feeder[sesId],
                    SIGNAL(sigFeederResponse(DEVICE_REPLY_TYPE_e,
                                             quint32,
                                             quint8)),
                    this,
                    SLOT(slotPbMediaResponse(DEVICE_REPLY_TYPE_e,
                                             quint32,
                                             quint8)));

        disconnect(feeder[sesId],
                   SIGNAL(sigSetLowestTime(quint8,bool,quint64)),
                   this,
                   SLOT(slotSetLowestTime(quint8,bool,quint64)));

        // delete buffer        
        DELETE_OBJ(feeder[sesId]);
    }

    // return status
    return status;
}

/**
 * @brief set feed frame flag for feeding frame to decoder.
 * @param flag
 */
void SyncPbFrameRecv::setFeedFrameFlagToAll(bool flag)
{
    for (quint8 index = 0; index < maxCh; index++)
    {
        if (IS_VALID_OBJ(feeder[index]))
        {
            feeder[index]->setFeedFrameFlag(flag);
        }
    }
}

/**
 * @brief SyncPbFrameRecv::SetPbSpeed
 * @param pbkSpeed
 * @param dir
 * @return
 */
void SyncPbFrameRecv::SetPbSpeed(PB_SPEED_e pbkSpeed, quint8 dir)
{
	UINT32 maxCapacity;
    UINT32 reqCapacity = 0;

    /* get Max decoding capacity */
    maxCapacity = GetMaxDecodingCapacity();

    /* calculate required decoding capacity with new request param */
    for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
    {
        if (feeder[index] != NULL)
        {
            reqCapacity += feeder[index]->GetRequiredDecodingCapacity(pbkSpeed, dir);
        }
    }

    DPRINT(GUI_SYNC_PB_MEDIA, "set speed: [speed=%d], [max_capacity=%u], [required_capacity=%u]", pbkSpeed, maxCapacity, reqCapacity);

    /* check only in case of speed 2X 4X 8X & 16X in forward as well as reverse direction */
	if ((reqCapacity > maxCapacity) && (pbkSpeed > PB_SPEED_NORMAL))
    {
        EPRINT(GUI_SYNC_PB_MEDIA, "failed to change speed: not enough decoding capacity: [direction=%d], [speed=%d]", dir, pbkSpeed);

        /* It is expected only in increasing speed case. Decreasing speed case should not have decoding capacity issue.
         * Do not allow speed increase
         */
	}
    else
    {
		pbSpeed = pbkSpeed;
        direction = dir;

        /* set new speed and dir of playback to all playback sessions */
        for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
        {
            if (feeder[index] != NULL)
            {
                feeder[index]->SetPbSpeedBufFeeder(pbkSpeed, dir);
            }
        }
    }
}

/**
 * @brief gets current speed.
 * @param dir
 */
PB_SPEED_e SyncPbFrameRecv::getPbSpeed(void)
{
	return (pbSpeed);
}

/**
 * @brief reset previous frame time & store current direction.
 * @param dir
 */
void SyncPbFrameRecv::resetprevFrameTimeDirection(quint8 dir)
{
    prevTimeDirAccess.lock ();
    direction = dir;
    prevFrameTime = 0;
    prevTimeDirAccess.unlock ();
}

/**
 * @brief set frameStoreFlag , which is decide whether this received frame should be stored or not.
 * @param flag
 */
void SyncPbFrameRecv::setFrameStoreFlag(bool flag)
{
    frameStoreFlagAccess.lockForWrite ();
    frameStoreFlag = flag;
    frameStoreFlagAccess.unlock ();    
}

/**
 * @brief get frameStoreFlag , which is decide whether this received frame should be stored or not.
 * @param flag
 */
bool SyncPbFrameRecv::getFrameStoreFlag(void)
{
    bool flag;

    frameStoreFlagAccess.lockForRead();
    flag = frameStoreFlag;
    frameStoreFlagAccess.unlock();

    return flag;
}

/**
 * @brief update channel id set.
 * @param chnlIdSet
 */
void SyncPbFrameRecv::updateChIdSet(const quint8 *pChnlIdSet)
{
	maxCh = 0;
	for(quint8 index = 0; index < MAX_SYNC_PB_SESSION; index++)
	{
        chIdSet[index] = pChnlIdSet[index];
        if(chIdSet[index] != INVALID_CAMERA_INDEX)
		{
			maxCh++;
		}
	}
}

/**
 * @brief gets current direction.
 * @param dir
 */
void SyncPbFrameRecv::getDirection(quint8 &dir)
{
    prevTimeDirAccess.lock();
    dir = direction;
    prevTimeDirAccess.unlock();
}

/**
 * @brief SyncPbFrameRecv::setRecvPauseFlag
 * @param flag
 */
void SyncPbFrameRecv::setRecvPauseFlag(bool flag)
{
    recvPauseFlagAccess.lockForWrite();
    recvPauseFlag = flag;
    recvPauseFlagAccess.unlock();
}

/**
 * @brief SyncPbFrameRecv::getRecvPauseFlag
 * @param flag
 */
bool SyncPbFrameRecv::getRecvPauseFlag(void)
{
    bool flag;

    recvPauseFlagAccess.lockForRead();
    flag = recvPauseFlag;
    recvPauseFlagAccess.unlock();

    return flag;
}

/**
 * @brief give decoder Id for particular channel id.
 * @param chId
 * @param decodeId
 * @param indx
 */
void SyncPbFrameRecv::getDecIdForChannel(quint8 chId, quint8 &decodeId, quint8 &indx)
{
    quint8 index = 0;
    bool status = false;

    for (index = 0; index < MAX_SYNC_PB_SESSION; index++)
    {
        if (chId == chIdSet[index])
        {
            decodeId = SyncDecIdSet[index];
            indx = index;
            status = true;
            break;
        }
    }

    if (status == false)
    {
        decodeId = INVALID_DEC_DISP_PLAY_ID;
    }
}

/**
 * @brief SyncPbFrameRecv::updateDecoderInstances
 * @param decIdSet
 */
void SyncPbFrameRecv::updateDecoderInstances(const quint8 *decIdSet)
{
    if (decIdSet != NULL)
    {
        for (quint8 index=0; index < MAX_SYNC_PB_SESSION; index++)
        {
            SyncDecIdSet[index] = *(decIdSet + index);
        }
    }
}

/**
 * @brief reset all buffer data.
 */
void SyncPbFrameRecv::resetAllBuffer(void)
{
    for (quint8 index = 0; index < maxCh; index++)
    {
        if (IS_VALID_OBJ(feeder[index]))
        {
            feeder[index]->resetBufData();
            feeder[index]->setFeedFrameFlag (true);
        }
    }
}

/**
 * @brief SyncPbFrameRecv::setStepFlagAll
 * @param flag
 */
void SyncPbFrameRecv::setStepFlagAll(bool flag)
{
    for (quint8 index = 0; index < maxCh; index++)
    {
        if (IS_VALID_OBJ(feeder[index]))
        {
            feeder[index]->setStepFlag ((flag == true) ? STEP_STATE_TRUE : STEP_STATE_FALSE);
        }
    }
}

/**
 * @brief SyncPbFrameRecv::sendStepWakeSigToAll
 */
void SyncPbFrameRecv::sendStepWakeSigToAll(void)
{
    for (quint8 index = 0; index < maxCh; index++)
    {
        if (IS_VALID_OBJ(feeder[index]))
        {
            feeder[index]->SendStepSig ();
        }
    }
}

/**
 * @brief SyncPbFrameRecv::setSyncFrameNum
 * @param num
 */
void SyncPbFrameRecv::setSyncFrameNum(quint8 num)
{
    syncFrameNoAccess.lock();
    syncFrameNo = num;
    syncFrameNoAccess.unlock();
}

/**
 * @brief does current time comparison and send timing signal to pbMedia & send any error to pbMedia.
 * @param tStatusId
 * @param currentTime
 * @param feederId
 */
void SyncPbFrameRecv::slotPbMediaResponse(DEVICE_REPLY_TYPE_e tStatusId, quint32 currentTime, quint8 feederId)
{
    quint32 emitTime = 0;

    if (currentTime > 0)
    {
        prevTimeDirAccess.lock();

        if (prevFrameTime != 0)
        {
            if ((direction == FORWARD_PLAY) && (currentTime > prevFrameTime))
            {
                emitTime = (currentTime - prevFrameTime);
            }
            else if ((direction == BACKWARD_PLAY) && (currentTime < prevFrameTime))
            {
                emitTime = (prevFrameTime - currentTime);
            }

            if(emitTime >= 1)
            {
                prevFrameTime = currentTime;
            }
        }
        else
        {
            emitTime = 1;
            prevFrameTime = currentTime;
        }

        prevTimeDirAccess.unlock();
    }
    else
    {
        if ( (tStatusId == CMD_STREAM_PLAYBACK_OVER)
                || (tStatusId == CMD_STREAM_HDD_FORMAT)
                || (tStatusId == CMD_REC_MEDIA_ERR)
                || (tStatusId == CMD_MAX_STREAM_LIMIT)
                || (tStatusId == CMD_NO_PRIVILEGE)
                || (tStatusId == CMD_REC_DRIVE_CONFIG_CHANGES))
        {
			setRunFlag(false);
			setFeedFrameFlagToAll(false);
        }
    }

    if (emitTime >= 1)
    {
        emit sigMediaResponse (request.requestId,
                               PLYBCK_RCD,
                               CMD_PLAYBACK_TIME,
                               QString ("%1").arg (currentTime));
    }

    if (tStatusId != CMD_PLAYBACK_TIME)
    {
        emit sigMediaResponse (request.requestId,
                               PLYBCK_RCD,
                               tStatusId,
                               QString ("%1").arg (chIdSet[feederId]));
    }
}

/**
 * @brief SyncPbFrameRecv::slotSetLowestTime
 * @param feederId
 * @param feedFrameFlag
 * @param frameTime
 */
void SyncPbFrameRecv::slotSetLowestTime(quint8 feederId, bool feedFrameFlag, quint64 frameTime)
{
    quint64 refTime = 0;

    m_feedInfoLock.lockForWrite();

    if (feeder[feederId] == NULL)
    {
        m_feedInfoLock.unlock();
        m_currentFrameFeedTime[feederId] = 0;
        DPRINT(GUI_SYNC_PB_MEDIA, "set playback reference time: invalid feeder id: [feederId=%d]", feederId);
        return;
    }

    /* if time is already reset, no need to process */
    if ((0 == m_currentFrameFeedTime[feederId]) && (0 == frameTime))
    {
        m_feedInfoLock.unlock();
        return;
    }

    /* save/update frame time based on feed frame flag */
    m_currentFrameFeedTime[feederId] = (feedFrameFlag == false) ? 0 : frameTime;

    /* forward play: set maximum reference value */
    if (direction == FORWARD_PLAY)
    {
        refTime = (~0);
    }

    /* check which feeder has the lowest time */
    for (quint8 chIndex = 0; chIndex < maxCh; chIndex++)
    {
        /* skip channel whose frame time is not set */
        if (m_currentFrameFeedTime[chIndex] == 0)
        {
            continue;
        }

        /* forward play */
        if (direction == FORWARD_PLAY)
        {
            /* if given time is lowest than previous, set it as reference time */
            if (m_currentFrameFeedTime[chIndex] < refTime)
            {
                refTime = m_currentFrameFeedTime[chIndex];
            }
        }
        /* backward play */
        else
        {
            /* in backward play, bigger reference time will be lowest reference */
            if (m_currentFrameFeedTime[chIndex] > refTime)
            {
                refTime = m_currentFrameFeedTime[chIndex];
            }
        }
    }

    m_feedInfoLock.unlock();

    /* set reference time which can be used by all feeder thread when playing frames */
    SyncBufferFeeder::setPlaybackReferenceTime(refTime);
}
