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
#include "SyncPbMedia.h"

/***********************************************************************************************
* @DEFINES
***********************************************************************************************/
#define RESET_PREVFRAMETIME_DELAY           25

/***********************************************************************************************
* @FUNCTION DEFINATION
***********************************************************************************************/
/**
 * @brief creates and initializes the object with the parameters like, server infomration, request infomrmation.
 * @param serverInfo
 * @param requestInfo
 * @param commandId
 * @param decoderId
 * @param pDecIdSet
 * @param pChnlIdSet
 */
SyncPbMedia::SyncPbMedia(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId,
                         quint8 decoderId, const quint8 *pDecIdSet, const quint8 *pChnlIdSet)
    : timeout(requestInfo.timeout), msgQueIndex(0), srvrInfo(serverInfo), sessionId(requestInfo.sessionId)

{
    pbState = MAX_PB_STATE;
    runFlag = true;

    syncPbFrameRecv = NULL;
    syncCommand = NULL;
    m_isRecvPauseSigSend = false;

    //Intilized mesg queue
	for(quint8 index = 0; index < MAX_SYNC_PB_MSGQUE_LENGTH; index++)
    {
        msgIdQue[index] = 0;
    }

    //Create thread for receive frames from server
    createSyncFrameRecv(serverInfo, requestInfo, commandId, decoderId, pDecIdSet, pChnlIdSet);
}

/**
 * @brief SyncPbMedia::~SyncPbMedia
 */
SyncPbMedia::~SyncPbMedia ()
{
}

/**
 * @brief It serves as independant instance of execution, which does the communication with the server.
 * It requests for the playback stream related control command and take action accordingly.
 * It also handle framereceive thread by flag. It also handle bufferFeeder thread by some flags.
 */
void SyncPbMedia::run(void)
{
    bool status = true;
    QStringList payloadFields;
    QString tempFsp(FSP), tempPayload, retStr;
    SET_COMMAND_e command = MAX_COMMAND;
    DEVICE_REPLY_TYPE_e response = CMD_INVALID_MESSAGE;
    quint8 isSliderChangeInStep = 0;
    quint8 direction;
    PB_SPEED_e pbSpeedLocal;
    quint8 pbSpeedServer;
    QString payLoadRet;
	QString respPayLoad;

    pbState = PLAY_STATE;

    prctl(PR_SET_NAME, "SYNC_PB_MED", 0, 0, 0);

    while(true)
    {
        /* Loop till run flag is set */
        runFlagLock.lockForRead();
        if (false == runFlag)
        {
            runFlagLock.unlock();
            break;
        }
        runFlagLock.unlock();

        // check in message que, is any message in que
        msgQueLock.lock ();
        if (msgQueIndex <= 0)
        {
            // if no message in que
            msgQueLock.unlock ();

            recvPauseSigLock.lock ();
            if (m_isRecvPauseSigSend == false)
            {
                recvPauseSig.wait (&recvPauseSigLock);
            }
            m_isRecvPauseSigSend = false;
            recvPauseSigLock.unlock ();
            continue;
        }

        // fetch first cmd & its payload from que & related payload
        if (msgQueIndex < MAX_SYNC_PB_MSGQUE_LENGTH)
        {
            command = (SET_COMMAND_e)msgIdQue[msgQueIndex];
            tempPayload = msgQuePayload[msgQueIndex];
            msgQueIndex--;
        }
        msgQueLock.unlock ();

        // action upon msg & prev state
        switch (command)
        {
            case SYNC_PLYBCK_RCD:
            {
                syncPbFrameRecv->setRecvPauseFlag (true);
                syncPbFrameRecv->setThrottleFlag (false);

                switch (pbState)
                {
                    case STEP_STATE:
                    {
                        // set stepflag false & send wake signal to all feeder
                        syncPbFrameRecv->setStepFlagAll (false);
                        syncPbFrameRecv->sendStepWakeSigToAll ();
                    }
                    // FALL THROUGH
                    case PLAY_STATE:
                    case PAUSE_BY_BUFFER_STATE:
                    {
                        // stop feeding, stop storage of frame, reset buffer
                        syncPbFrameRecv->setFeedFrameFlagToAll(false);
                        syncPbFrameRecv->setFrameStoreFlag(false);
                        syncPbFrameRecv->resetAllBuffer();

                        // enable feeding flag
                        syncPbFrameRecv->setFeedFrameFlagToAll(true);
                        status = true;
                    }
                    break;

                    case STOP_STATE:
                    {
                        syncPbFrameRecv->deleteAllSyncBufFeeder ();
                        syncPbFrameRecv->setFrameStoreFlag (false);
                        /* Set updated channel ids on Play Command rcvd after Stop */
                        syncPbFrameRecv->createAllSyncBufFeeder ();
                        syncPbFrameRecv->setFeedFrameFlagToAll (true);
                        status = true;
                    }
                    break;

                    case PAUSE_STATE:
                    {
                        syncPbFrameRecv->setFeedFrameFlagToAll (true);
                        status = true;
                    }
                    break;

                    default:
                    {
                        /* Nothing to do */
                    }
                    break;
                }

                if (status == false)
                {
                    break;
                }

                payloadFields = tempPayload.split (FSP);
                syncPbFrameRecv->setSyncFrameNum((quint8)payloadFields.at(6).toInt());
                syncPbFrameRecv->SetPbSpeed((PB_SPEED_e)payloadFields.at(3).toInt(), payloadFields.at(1).toInt());
                pbSpeedLocal = syncPbFrameRecv->getPbSpeed();
                respPayLoad = QString("%1").arg(pbSpeedLocal);

                /* for server speed should be of range [0-4]: PLAY_1X = 0, PLAY_2X = 1, PLAY_4X = 2, PLAY_8X = 3, PLAY_16X = 4 */
                pbSpeedServer = (pbSpeedLocal <= PB_SPEED_NORMAL) ? 0 : (pbSpeedLocal - PB_SPEED_NORMAL);
                payloadFields.replace (3, QString ("%1").arg (pbSpeedServer));
                retStr = payloadFields.join(tempFsp);

                status = createSyncCommandReq(SYNC_PLYBCK_RCD, retStr);
                if (status == false)
                {
                    response = CMD_INTERNAL_RESOURCE_LIMIT;
                    break;
                }

                // send pause stream request
                syncCommand->getBlockingRes(payLoadRet, response);
                if (response != CMD_SUCCESS)
                {
                    status = false;
                    deleteSyncCommandReq();
                    break;
                }

                pbState = PLAY_STATE;
                // sleep before prevframe time reset, because if any feeder's time req is pending, it can be served
                // Its < so that this thread is not going to sleep long time, and give sync response to ui before video loss signal
                // Its > so that every time calculation req is served before resetting the prevframe time
                msleep (RESET_PREVFRAMETIME_DELAY);
                syncPbFrameRecv->resetprevFrameTimeDirection (payloadFields.at (1).toUInt ());

                /* Send Playback speed to update UI */
                emit sigSyncPbResponse (MSG_SET_CMD,
                                        command,
                                        CMD_STREAM_PB_SPEED,
                                        respPayLoad);
                deleteSyncCommandReq();
            }
            break;

            case PAUSE_RCD:
            {
                // stop feeding, stop storage of frame, reset buffer
                syncPbFrameRecv->setFeedFrameFlagToAll (false);
                syncPbFrameRecv->setFrameStoreFlag (false);
                syncPbFrameRecv->resetAllBuffer ();
                syncPbFrameRecv->setThrottleFlag (false);
                syncPbFrameRecv->setRecvPauseFlag (true);

                status = createSyncCommandReq (PAUSE_RCD ,"");
                if (status == false)
                {
                    response = CMD_INTERNAL_RESOURCE_LIMIT;
                    break;
                }

                // send pause stream request
                syncCommand->getBlockingRes (payLoadRet, response);
                if (response != CMD_SUCCESS)
                {
                    status = false;
                    deleteSyncCommandReq();
                    break;
                }

                pbState = PAUSE_STATE;
                deleteSyncCommandReq();
            }
            break;

            case SYNC_PAUSE_BUFFER:
            {
                syncPbFrameRecv->setRecvPauseFlag (true);
                if ((pbState != PLAY_STATE) && (pbState != STEP_STATE))
                {
                    break;
                }

                status = createSyncCommandReq (PAUSE_RCD ,"");
                if (status == false)
                {
                    break;
                }

                // send pause stream request
                syncCommand->getBlockingRes(payLoadRet, response);
                if (response == CMD_SUCCESS)
                {
                    if (pbState == PLAY_STATE)
                    {
                        pbState = PAUSE_BY_BUFFER_STATE;
                    }
                }
                deleteSyncCommandReq();
            }
            break;

            case SYNC_RSM_BUFFER:
            {
                if ((pbState != PAUSE_BY_BUFFER_STATE) && (pbState != STEP_STATE))
                {
                    break;
                }

                status = createSyncCommandReq (RSM_RCD ,"");
                if (status == false)
                {
                    break;
                }

                // send pause stream request
                syncCommand->getBlockingRes (payLoadRet, response);
                if (response == CMD_SUCCESS)
                {
                    syncPbFrameRecv->setRecvPauseFlag (false);
                    if (pbState == PAUSE_BY_BUFFER_STATE)
                    {
                        pbState = PLAY_STATE;
                    }
                }
                deleteSyncCommandReq();
            }
            break;

            case STP_RCD:
            {
                // stop feeding, stop storage of frame, delete all feeder
                syncPbFrameRecv->setFeedFrameFlagToAll (false);
                syncPbFrameRecv->setFrameStoreFlag (false);
                syncPbFrameRecv->setThrottleFlag (false);
                syncPbFrameRecv->setRecvPauseFlag (true);
                syncPbFrameRecv->deleteAllSyncBufFeeder ();

                status = createSyncCommandReq(STP_RCD, "");
                if (status == false)
                {
                    response = CMD_INTERNAL_RESOURCE_LIMIT;
                    break;
                }

                // send pause stream request
                syncCommand->getBlockingRes (payLoadRet, response);
                if (response != CMD_SUCCESS)
                {
                    status = false;
                    deleteSyncCommandReq();
                    break;
                }

                pbState = STOP_STATE;
                deleteSyncCommandReq();
            }
            break;

            case CLR_RCD:
            {
                syncPbFrameRecv->setRecvPauseFlag (true);
                syncPbFrameRecv->setThrottleFlag (false);
                // set run flag false
                setRunFlag(false);

                status = createSyncCommandReq(CLR_RCD, "");
                if (status == false)
                {
                    response = CMD_INTERNAL_RESOURCE_LIMIT;
                    break;
                }

                // send pause stream request
                syncCommand->getBlockingRes (payLoadRet, response);
                if (response != CMD_SUCCESS)
                {
                    status = false;
                    deleteSyncCommandReq();
                    break;
                }

                pbState = MAX_PB_STATE;
                deleteSyncCommandReq();
            }
            break;

            case SYNC_PLYBCK_SPEED:
            {
                payloadFields = tempPayload.split (FSP);
                /* Set Speed to buffer feeder */
                syncPbFrameRecv->SetPbSpeed((PB_SPEED_e)payloadFields.at(0).toInt(), payloadFields.at(1).toInt());

                /* Send Playback speed to update UI */
                pbSpeedLocal = syncPbFrameRecv->getPbSpeed();
                respPayLoad = QString ("%1").arg (pbSpeedLocal);
                response = CMD_SUCCESS;
            }
            break;

            case SYNC_STEP_FORWARD:
            case SYNC_STEP_REVERSE:
            {
                payloadFields = tempPayload.split (FSP);

                // if slider is change while doing stepping
                isSliderChangeInStep = payloadFields.at(9).toInt ();
                payloadFields.removeAt (9);
                retStr = payloadFields.join (tempFsp);

                syncPbFrameRecv->getDirection(direction);
                switch(pbState)
                {
                    case PLAY_STATE:
                    case PAUSE_BY_BUFFER_STATE:
                    case STEP_STATE:
                    {
                        if ((pbState == PLAY_STATE) || (pbState == PAUSE_BY_BUFFER_STATE) ||
                                (isSliderChangeInStep == 1) ||
                                (((direction == FORWARD_PLAY) && (command == SYNC_STEP_REVERSE)) ||
                                  ((direction == BACKWARD_PLAY) && (command == SYNC_STEP_FORWARD))))
                        {
                            // stop feeding, stop storage of frame, reset buffer
                            syncPbFrameRecv->setFeedFrameFlagToAll (false);
                            syncPbFrameRecv->setStepFlagAll (true);
                            syncPbFrameRecv->setFrameStoreFlag (false);
                            syncPbFrameRecv->setThrottleFlag (false);
                            syncPbFrameRecv->resetAllBuffer ();

                            /* enable feeding flag */
                            syncPbFrameRecv->setFeedFrameFlagToAll(true);
                            syncPbFrameRecv->setSyncFrameNum((quint8)payloadFields.at(6).toInt());
                            syncPbFrameRecv->SetPbSpeed((PB_SPEED_e)payloadFields.at(3).toInt(), payloadFields.at(1).toInt());
                            pbSpeedLocal = syncPbFrameRecv->getPbSpeed();
                            respPayLoad = QString ("%1").arg(pbSpeedLocal);

                            /* for server speed should be of range [0-4]:PLAY_1X = 0, PLAY_2X = 1, PLAY_4X = 2, PLAY_8X = 3, PLAY_16X = 4 */
                            pbSpeedServer = (command == SYNC_STEP_FORWARD) ? 3 : 0;
                            payloadFields.replace(3, QString("%1").arg(pbSpeedServer));
                            retStr = payloadFields.join (tempFsp);
                            status = createSyncCommandReq(SYNC_PLYBCK_RCD, retStr);
                            if (status == false)
                            {
                                response = CMD_INTERNAL_RESOURCE_LIMIT;
                                break;
                            }

                            // send pause stream request
                            pbState = STEP_STATE;
                            syncCommand->getBlockingRes (payLoadRet, response);
                            if (response != CMD_SUCCESS)
                            {
                                status = false;
                                deleteSyncCommandReq();
                                break;
                            }

                            /* Send Playback speed to update UI */
                            syncPbFrameRecv->setRecvPauseFlag (false);
                            emit sigSyncPbResponse (MSG_SET_CMD,
                                                    command,
                                                    CMD_STREAM_PB_SPEED,
                                                    respPayLoad);
                            deleteSyncCommandReq();
                        }
                    }
                    break;

                    case PAUSE_STATE:
                    {
                        syncPbFrameRecv->setStepFlagAll (true);

                        syncPbFrameRecv->setSyncFrameNum((quint8)payloadFields.at(6).toInt());
                        status = createSyncCommandReq(SYNC_PLYBCK_RCD, tempPayload);
                        if (status == false)
                        {
                            response = CMD_INTERNAL_RESOURCE_LIMIT;
                            break;
                        }

                        // send pause stream request
                        pbState = STEP_STATE;
                        syncPbFrameRecv->setFeedFrameFlagToAll (true);
                        syncCommand->getBlockingRes (payLoadRet, response);
                        if (response != CMD_SUCCESS)
                        {
                            status = false;
                            deleteSyncCommandReq();
                            break;
                        }

                        syncPbFrameRecv->setRecvPauseFlag (false);
                        deleteSyncCommandReq();
                    }
                    break;

                    default:
                    {
                        /* Nothing to do */
                    }
                    break;
                }

                /* send wakeup signal to all feeder thread, they will take care wether to play frame
                or not based on reference time */
                if (status == true)
                {
                    syncPbFrameRecv->sendStepWakeSigToAll();
                }
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
        }

        if (response != CMD_SUCCESS)
        {
            syncPbFrameRecv->setRecvPauseFlag (true);
            syncPbFrameRecv->setFeedFrameFlagToAll (false);
            syncPbFrameRecv->setFrameStoreFlag (false);
            syncPbFrameRecv->setThrottleFlag (false);
            syncPbFrameRecv->resetAllBuffer ();

            if ((command == SYNC_PAUSE_BUFFER) || (command == SYNC_RSM_BUFFER))
            {
                command = PLYBCK_RCD;
            }
        }

        if(command == SYNC_PLYBCK_SPEED)
        {
            emit sigSyncPbResponse (MSG_SET_CMD, command, response,	respPayLoad);
        }
        else if(!((command == SYNC_PAUSE_BUFFER) || (command == SYNC_RSM_BUFFER)))
        {
            emit sigSyncPbResponse (MSG_SET_CMD, command, response, "");
        }
    }

    deleteSyncFrameRecv();
}

/**
 * @brief creates a framereceive object. It also starts thread which receive frames and store in buffer.
 * @param serverInfo
 * @param requestInfo
 * @param commandId
 * @param decoderId
 * @param pDecIdSet
 * @param pChnlIdSet
 * @return
 */
bool SyncPbMedia::createSyncFrameRecv(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, SET_COMMAND_e commandId,
                                      quint8 decoderId, const quint8 *pDecIdSet, const quint8 *pChnlIdSet)
{
	// if buffer is not created
    if (IS_VALID_OBJ(syncPbFrameRecv))
    {
        return false;
    }

    //Create thread for receive frames from server
    /* PARASOFT: Memory Deallocated in delete syncFrame Recv */
    syncPbFrameRecv = new SyncPbFrameRecv(serverInfo, requestInfo, commandId, decoderId, pDecIdSet);

    /* set stack size of thread */
    syncPbFrameRecv->setStackSize(SYNC_PB_FRAME_RECV_THREAD_STACK_SIZE);

    connect (syncPbFrameRecv,
             SIGNAL(sigPbFrameRecvThreshold(BUFFER_THRESHOLD_e)),
             this,
             SLOT(slotPbFrameRecvThreshold(BUFFER_THRESHOLD_e)));

    connect (syncPbFrameRecv,
             SIGNAL(sigMediaResponse (REQ_MSG_ID_e, SET_COMMAND_e, DEVICE_REPLY_TYPE_e, QString)),
             this,
             SLOT(slotFrameRecvResponse (REQ_MSG_ID_e, SET_COMMAND_e, DEVICE_REPLY_TYPE_e, QString)));

    /* Update ChnlIdSet */
    syncPbFrameRecv->updateChIdSet(pChnlIdSet);

    // start feeder thread
    syncPbFrameRecv->start ();

    // return status
    return true;
}

/**
 * @brief creates a commandReq to send control command.
 * @param commandId
 * @param reqInfoPayload
 * @return
 */
bool SyncPbMedia::createSyncCommandReq(SET_COMMAND_e commandId, QString reqInfoPayload)
{
    REQ_INFO_t reqInfo;

    // initialize request information
    reqInfo.sessionId = sessionId;
    reqInfo.requestId = MSG_SET_CMD;
    reqInfo.timeout = timeout;
    reqInfo.payload = reqInfoPayload;
    reqInfo.bytePayload = NULL;
    reqInfo.windowId = MAX_WIN_ID;

    // throttle request is not active
    if (IS_VALID_OBJ(syncCommand))
    {
        return false;
    }

    // create throttle request [pause / resume]
    /* PARASOFT: Memory Deallocated in delete SyncCommand Req */
    syncCommand = new CommandRequest (srvrInfo, reqInfo, commandId, STRM_RELAT_CMD_SES_ID);

    // return status
    return true;
}

/**
 * @brief deletes syncCommand req.
 * @return
 */
bool SyncPbMedia::deleteSyncCommandReq(void)
{
    if (IS_VALID_OBJ(syncCommand))
    {
        // wait for thread to return
        syncCommand->wait ();

        // delete throttle request
        DELETE_OBJ(syncCommand);

        return true;
    }

    return false;
}

/**
 * @brief deletes syncFrameRecv object.
 * @return
 */
bool SyncPbMedia::deleteSyncFrameRecv(void)
{
    if (IS_VALID_OBJ(syncPbFrameRecv))
    {
        syncPbFrameRecv->setRunFlag(false);

        syncPbFrameRecv->wait();

        disconnect (syncPbFrameRecv,
                    SIGNAL(sigMediaResponse (REQ_MSG_ID_e, SET_COMMAND_e, DEVICE_REPLY_TYPE_e, QString)),
                    this,
                    SLOT(slotFrameRecvResponse (REQ_MSG_ID_e, SET_COMMAND_e, DEVICE_REPLY_TYPE_e, QString)));

        disconnect (syncPbFrameRecv,
                    SIGNAL(sigPbFrameRecvThreshold(BUFFER_THRESHOLD_e)),
                    this,
                    SLOT(slotPbFrameRecvThreshold(BUFFER_THRESHOLD_e)));

        DELETE_OBJ(syncPbFrameRecv);
    }

    return true;
}

/**
 * @brief puts command & related payload in que & Increment index of queue & also send condition wait signal.
 * @param commandId
 * @param payloadStr
 */
void SyncPbMedia::setActionOnControlCmd (SET_COMMAND_e commandId, QString payloadStr)
{
    msgQueLock.lock();

    msgQueIndex++;

	if (msgQueIndex < MAX_SYNC_PB_MSGQUE_LENGTH)
    {
        msgIdQue[msgQueIndex] = (quint8) commandId;
        msgQuePayload[msgQueIndex] = payloadStr;
        msgQueLock.unlock();
        sigToRecvPauseFalse();
    }
    else
    {
		EPRINT(GUI_SYNC_PB_MEDIA, "maximum requests exceeded");	
        msgQueLock.unlock();
    }
}

/**
 * @brief SyncPbMedia::updateDecInstances
 * @param idSet
 */
void SyncPbMedia::updateDecInstances(const quint8 *idSet)
{
    if(IS_VALID_OBJ(syncPbFrameRecv))
    {
        syncPbFrameRecv->updateDecoderInstances(idSet);
    }
}

/**
 * @brief returns decoder Id for Channel Id
 * @param chnlId
 * @param decoderId
 * @param indx
 */
void SyncPbMedia::GetDecIdForCh (quint8 chnlId, quint8 &decoderId, quint8 &indx)
{
    if(IS_VALID_OBJ(syncPbFrameRecv))
    {
        syncPbFrameRecv->getDecIdForChannel(chnlId, decoderId, indx);
    }
}

/**
 * @brief sets the playback pause flag to false.
 */
void SyncPbMedia::sigToRecvPauseFalse(void)
{
    recvPauseSigLock.lock();
    m_isRecvPauseSigSend = true;
    recvPauseSig.wakeAll();
    recvPauseSigLock.unlock();
}

/**
 * @brief SyncPbMedia::setRunFlag
 * @param flag
 */
void SyncPbMedia::setRunFlag(bool flag)
{
    runFlagLock.lockForWrite();
    runFlag = flag;
    runFlagLock.unlock();
}

/**
 * @brief slot to catch buffer threshold cross signal.
 * @param threshold
 */
void SyncPbMedia::slotPbFrameRecvThreshold(BUFFER_THRESHOLD_e threshold)
{
    msgQueLock.lock();

    msgQueIndex++;
    if (msgQueIndex >= MAX_SYNC_PB_MSGQUE_LENGTH)
    {
        msgQueLock.unlock();
        EPRINT(GUI_SYNC_PB_MEDIA, "maximum requests exceeded");
        return;
    }

    msgIdQue[msgQueIndex] = (quint8)((threshold == CROSSED_MINIMUM) ? SYNC_RSM_BUFFER : SYNC_PAUSE_BUFFER);
    msgQuePayload[msgQueIndex] = "";
    msgQueLock.unlock();
    sigToRecvPauseFalse();
}

/**
 * @brief sets the playback pause flag to false.
 * @param requestId
 * @param commandId
 * @param statusId
 * @param payload
 */
void SyncPbMedia::slotFrameRecvResponse (REQ_MSG_ID_e requestId, SET_COMMAND_e commandId, DEVICE_REPLY_TYPE_e statusId, QString payload)
{
    if (statusId == CMD_STREAM_PLAYBACK_OVER)
    {
        pbState = STOP_STATE;
    }

    emit sigSyncPbResponse (requestId, commandId, statusId, payload);
}
