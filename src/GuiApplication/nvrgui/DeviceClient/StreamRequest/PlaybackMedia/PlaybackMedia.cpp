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
//   Project      : NVR ( Network Video Recorder)
//   Owner        : Aekam Parmar
//   File         : PlaybackMedia.cpp
//   Description  : This module provides APIs for playback media streaming.
//                  It has a thread function which sends request to server
//                  and if the response is success, receives stream and stores
//                  in the buffer.
//                  It also sends throttle requests [pause / resume] in response
//                  to buffer threshold cross, [maximum / minimum].
//
/////////////////////////////////////////////////////////////////////////////

#include <QFile>
#include <QStringList>
#include <sys/prctl.h>
#include "PlaybackMedia.h"


//******** Extern Variables **********


//******** Defines and Data Types ****
#define PAUSE_POLL_TIME         100     // in mSec


//******** Function Prototypes *******


//******** Global Variables **********


//******** Static Variables **********


//******** Function Definitions ******
//*****************************************************************************
//  PlaybackMedia ()
//      Param:
//          IN : SERVER_INFO_t serverInfo       // server information [ip / port]
//               REQ_INFO_t requestInfo         // request information
//               SET_COMMAND_e commandId        // type of command to send
//               QString playbackId             // playback id
//          OUT: NONE
//
//	Returns: Not Applicable
//
//	Description:
//          This API is constructor of class PlaybackMedia. It creates and
//          initializes the object with the parameters like, server infomration,
//          request infomrmation, window index, playback index.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
PlaybackMedia::PlaybackMedia (SERVER_INFO_t serverInfo,
                              REQ_INFO_t &requestInfo,
                              SET_COMMAND_e commandId,
                              QString playbackId,
                              quint8 decoderId)
    : MediaRequest(serverInfo,
                   requestInfo,
                   commandId,
                   decoderId), timeout(requestInfo.timeout), srvrInfo(serverInfo), sessionId(requestInfo.sessionId),
       cmdId(commandId), pbId(playbackId)
{
    // store information needed for throttle request [pause / resume]

    // set pause flag to false
    setPauseFlag(false);
    // set throttle request to NULL
    throttle = NULL;

    // initialize buffer and feeder request to NULL
    buffer = NULL;
    feeder = NULL;
}

//*****************************************************************************
//  ~PlaybackMedia ()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//
//	Returns: Not Applicable
//
//	Description:
//          This API is destructor of class PlaybackMedia.
//          As of now it doesn't serve any functionality. Kept for future use.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
PlaybackMedia::~PlaybackMedia()
{

}

//*****************************************************************************
//  run ()
//      Param:
//          IN : NONE
//          OUT: NONE
//
//	Returns: NONE
//
//	Description:
//          This API is called from the system thread. It serves as independant
//          instance of execution, which does the communication with the server.
//          It requests for the playback stream and starts receiving frames upon
//          success. It stores the frames into the buffer and also issues
//          pause stream and resume stream commands in order to maintain the
//          frame storage under buffer limit.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
void PlaybackMedia::run()
{
    bool status = true;
    bool pauseTempFlag;
    QTcpSocket tcpSocket;
    FRAME_HEADER_t header;
    QByteArray frame;
    QStringList payloadFields;
    PB_SPEED_e pbSpeed;
    QString tempFsp(FSP);

    prctl(PR_SET_NAME, "PLY_MED", 0, 0, 0);

    // create bufferFeeder
    status = createBufferFeeder();
    // IF failed to create bufferFeeder
    if  (status == false)
    {
        // EMIT signal with resource limit error
        emit sigMediaResponse(request.requestId,
                              cmdId,
                              CMD_RESOURCE_LIMIT,
                              request.payload);
    }
    // ELSE
    else
    {
        payloadFields = request.payload.split(FSP);
        pbSpeed = feeder->SetPbSpeed((PB_SPEED_e)payloadFields.value(5).toInt());
        payloadFields.replace(5, QString ("%1").arg(pbSpeed));
        request.payload = payloadFields.join(tempFsp);

        // connect to server
        status = connectToServer(tcpSocket);

        // IF connection to server made successfully
        if (status == true)
        {
            // send request to server
            status = sendRequest(tcpSocket);

            // if request to server sent successfully
            if (status == true)
            {
                receiveResponse(tcpSocket, STREAM_RESPONSE_SIZE);
            }
            // IF response is not command success
            if ((status == true)
                    && (statusId == CMD_SUCCESS))
            {
                // EMIT signal with appropriate status id
                emit sigMediaResponse(request.requestId,
                                      cmdId,
                                      statusId,
                                      request.payload);

                runFlagLock.lockForRead ();
                // LOOP till run flag is set
                while (runFlag == true)
                {
                    runFlagLock.unlock();

                    getPauseFlag(pauseTempFlag);

                    if(pauseTempFlag == true)
                    {
                        // sleep for pause poll time
                        msleep(PAUSE_POLL_TIME);
                    }
                    // ELSE
                    else
                    {
                        // read frame header and read frame
                        // IF failed to read either of thing
                        if ((receiveHeader(header, tcpSocket) == false)
                                || (receiveFrame(frame,
                                                 (header.frameSize - sizeof(FRAME_HEADER_t)),
                                                 tcpSocket) == false))
                        {
                            // set run flag to false
                            getPauseFlag(pauseTempFlag);

                            if(pauseTempFlag == false)
                            {
                                statusId = CMD_SERVER_NOT_RESPONDING;
                                setRunFlag(false);
                            }
                        }
                        // ELSE
                        else
                        {
                            // write frame to buffer
                            buffer->writeFrame(frame, header);

                            if(cmdId == STEP_RCD_STRM)
                            {
                                setPauseFlag(false);
                            }
                            switch(header.streamStatus)
                            {
                            case STREAM_PLAYBACK_OVER:
                                setPauseFlag(true);
                                break;

                            case STREAM_CONFIG_CHANGE:
                                statusId = CMD_STREAM_CONFIG_CHANGE;
                                setRunFlag(false);
                                break;

                            case STREAM_FILE_ERROR:
                                statusId = CMD_STREAM_FILE_ERROR;
                                setRunFlag(false);
                                break;

                            case STREAM_HDD_FORMAT:
                                statusId = CMD_STREAM_HDD_FORMAT;
                                setRunFlag(false);
                                break;

                            default:
                                break;
                            }
                        }
                        // ENDIF
                    }
                    // ENDIF
                    runFlagLock.lockForRead();
                }
                runFlagLock.unlock();
                // ENDLOOP
            }
            // ENDIF
        }
        // ENDIF
        // delete bufferFeeder instance
        deleteBufferFeeder ();
    }

    if ((getStopFlag()== true) || (cmdId == STEP_RCD_STRM))
    {
        statusId = CMD_STREAM_STOPPED;
    }

    emit sigMediaResponse(request.requestId,
                          cmdId,
                          statusId,
                          request.payload);
}

void PlaybackMedia::setPauseFlag(bool flag)
{
    // set stream pause flag to false
    pauseFlagAccess.lockForWrite();
    pauseFlag = flag;
    pauseFlagAccess.unlock();
}

void PlaybackMedia::getPauseFlag(bool &flag)
{
    // set stream pause flag to false
    pauseFlagAccess.lockForRead();
    flag = pauseFlag;
    pauseFlagAccess.unlock();
}

//*****************************************************************************
//  createBufferFeeder ()
//      Param:
//          IN : NONE
//          OUT: NONE
//
//	Returns: bool [true / false]
//
//	Description:
//          This API creates a buffer to store playback stream.
//          It also starts feeder thread which feedes frames to decoder module.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool PlaybackMedia::createBufferFeeder(void)
{
    bool status = false;

    // if buffer is not created
    if (buffer == NULL)
    {
        // create a buffer
        /* PARASOFT: Memory Deallocated in delete Buffer Feeder */
        buffer = new BufferFeeder(decId, this);

        // if buffer created
        if (buffer != NULL)
        {
            // connect buffer threshold signal to the slot of playback media
            // object
            connect(buffer,
                    SIGNAL(sigBufferThreshold(BUFFER_THRESHOLD_e)),
                    this,
                    SLOT(slotBufferThreshold(BUFFER_THRESHOLD_e)));

            // duplicate buffer pointer to feeder
            feeder = buffer;

            connect (feeder,
                     SIGNAL(sigFeederResponse(DEVICE_REPLY_TYPE_e,
                                              QString)),
                     this,
                     SLOT(slotPbMediaResponse(DEVICE_REPLY_TYPE_e,
                                              QString)));

            // start feeder thread
            feeder->start();

            // set status to true
            status = true;
        }
    }

    // return status
    return status;
}

//*****************************************************************************
//  deleteBufferFeeder ()
//      Param:
//          IN : NONE
//          OUT: NONE
//
//	Returns: bool [true / false]
//
//	Description:
//          This API deletes the buffer.
//          It also stops the feeder thread.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool PlaybackMedia::deleteBufferFeeder(void)
{
    bool status = false;

    // if buffer is already created
    if (buffer != NULL)
    {
        // stop feeder thread
        feeder->setRunFlag(false);
        // wait for feeder thread to return
        feeder->wait();

        disconnect(feeder,
                   SIGNAL(sigFeederResponse(DEVICE_REPLY_TYPE_e,
                                            QString)),
                   this,
                   SLOT(slotPbMediaResponse(DEVICE_REPLY_TYPE_e,
                                            QString)));

        // disconect buffer threshold signal from the slot of playback media
        // object
        disconnect(buffer,
                   SIGNAL(sigBufferThreshold(BUFFER_THRESHOLD_e)),
                   this,
                   SLOT(slotBufferThreshold(BUFFER_THRESHOLD_e)));

        // delete buffer
        delete buffer;

        // mark buffer and feeder to 'not in use'
        buffer = NULL;
        feeder = NULL;
    }

    // return status
    return status;
}

//*****************************************************************************
//  createThrottleReq ()
//      Param:
//          IN : SET_COMMAND_e commandId
//          OUT: NONE
//
//	Returns: bool [true / false]
//
//	Description:
//          This API creates a throttle request [pause / resume] depending upon
//          the command id specified.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool PlaybackMedia::createThrottleReq(SET_COMMAND_e commandId)
{
    bool status = false;
    REQ_INFO_t reqInfo;

    // initialize request information
    reqInfo.sessionId = sessionId;
    reqInfo.requestId = MSG_SET_CMD;
    reqInfo.timeout = timeout;
    reqInfo.bytePayload = NULL;
    reqInfo.payload = pbId.append(FSP);
    reqInfo.windowId = MAX_WIN_ID;

    // throttle request is not active
    if (throttle == NULL)
    {
        // create throttle request [pause / resume]
        /* PARASOFT: Memory Deallocated in delete Throttle Req */
        throttle = new CommandRequest(srvrInfo, reqInfo, commandId, STRM_RELAT_CMD_SES_ID);

        // if request created
        if(throttle != NULL)
        {
            // connect signal of throttle request to slot of playback media
            connect(throttle,
                    SIGNAL(sigCommandResponse(REQ_MSG_ID_e,
                                              SET_COMMAND_e,
                                              DEVICE_REPLY_TYPE_e,
                                              QString,
                                              quint8)),
                    this,
                    SLOT(slotThrottleResponse(REQ_MSG_ID_e,
                                              SET_COMMAND_e,
                                              DEVICE_REPLY_TYPE_e,
                                              QString)));

            // set status to true
            status = true;
        }
    }

    // return status
    return status;
}

//*****************************************************************************
//  deleteThrottleReq ()
//      Param:
//          IN : NONE
//          OUT: NONE
//
//	Returns: bool [true / false]
//
//	Description:
//          This API deletes throttle request [pause / resume]
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
bool PlaybackMedia::deleteThrottleReq(void)
{
    bool status = false;

    // if throttle request is active
    if(throttle != NULL)
    {
        // wait for thread to return
        throttle->wait();

        // disconnect signal of throttle request from the slot of playback media
        disconnect(throttle,
                   SIGNAL(sigCommandResponse(REQ_MSG_ID_e,
                                             SET_COMMAND_e,
                                             DEVICE_REPLY_TYPE_e,
                                             QString,
                                             quint8)),
                   this,
                   SLOT(slotThrottleResponse(REQ_MSG_ID_e,
                                             SET_COMMAND_e,
                                             DEVICE_REPLY_TYPE_e,
                                             QString)));

        // delete throttle request
        delete throttle;
        // mark it inactive
        throttle = NULL;

        // set status to true
        status = true;
    }

    // reutrn status
    return status;
}

//*****************************************************************************
//  slotBufferThreshold ()
//      Param:
//          IN : BUFFER_THRESHOLD_e threshold   // buffer threshold [min / max]
//          OUT: NONE
//
//	Returns: NONE
//
//	Description:
//          This API is slot to catch buffer threshold cross signal.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
void PlaybackMedia::slotBufferThreshold(BUFFER_THRESHOLD_e threshold)
{
    switch (threshold)
    {
    // buffer size crossed minimum threshold
    case CROSSED_MINIMUM:
        // create resume playback stream
        if(createThrottleReq(RSM_PLY_RCD_STRM) == true)
        {
            // set stream pause flag to false
            setPauseFlag(false);

            // send resume stream request
            throttle->start();
        }
        break;

        // buffer size crossed maximum threshold
    case CROSSED_MAXIMUM:
        // create pause playback stream
        if(createThrottleReq(PAUSE_RCD_STRM) == true)
        {
            // set stream pause flag to true
            setPauseFlag(true);

            // send pause stream request
            throttle->start();
        }
        break;

    default:
        break;
    }
}

//*****************************************************************************
//  slotThrottleResponse ()
//      Param:
//          IN : REQ_MSG_ID_e requestId
//               SET_COMMAND_e commandId
//               DEVICE_REPLY_TYPE_e statusId
//               QString payload
//          OUT: NONE
//
//	Returns: NONE
//
//	Description:
//          This API is slot to catch throttle request response.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
void PlaybackMedia::slotThrottleResponse(REQ_MSG_ID_e,
                                         SET_COMMAND_e commandId,
                                         DEVICE_REPLY_TYPE_e tStatusId,
                                         QString)
{
    deleteThrottleReq();
    Q_UNUSED(commandId);
    Q_UNUSED(tStatusId);
}

//*****************************************************************************
//  slotPbMediaResponse ()
//      Param:
//          IN : DEVICE_REPLY_TYPE_e statusId
//               quint8 decoderId
//               quint8 pageId
//               QString frameTime
//          OUT: NONE
//
//	Returns: NONE
//
//	Description:
//          This API sets the playback pause flag to false.
//	[Pre-condition:] (optional)
//          NONE
//	[Constraints:] (optional)
//          NONE
//
//*****************************************************************************
void PlaybackMedia::slotPbMediaResponse(DEVICE_REPLY_TYPE_e tStatusId,
                                        QString payload)
{
    // EMIT signal to stream request class
    if (!((tStatusId == CMD_PLAYBACK_TIME) ||
          (tStatusId == CMD_STREAM_VIDEO_LOSS) ||
          (tStatusId == CMD_STREAM_NO_VIDEO_LOSS)))
    {
        this->statusId = tStatusId;
    }

    emit sigMediaResponse(request.requestId,
                          cmdId,
                          tStatusId,
                          payload);

    if ((cmdId == STEP_RCD_STRM)
            && (tStatusId == CMD_PLAYBACK_TIME))
    {
        setRunFlag(false);
    }
}

