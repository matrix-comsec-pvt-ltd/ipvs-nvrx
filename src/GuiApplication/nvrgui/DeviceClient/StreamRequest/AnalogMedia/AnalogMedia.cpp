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
//   Project      : DVR ( Hybrid Video Recorder)
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////
#include <QStringList>
#include <sys/prctl.h>

#include "AnalogMedia.h"

//******** Extern Variables **********

//******** Defines and Data Types ****

//******** Function Prototypes *******

//******** Global Variables **********

//******** Static Variables **********

//******** Function Definitions ******

//*****************************************************************************
//  AnalogMedia ()
//      Param:
//          IN : REQ_MSG_ID_e requestId     // request index
//               SET_COMMAND_e commandId    // command index
//               quint8 channelId           // channel Index
//          OUT: NONE
//	Returns:
//		Not Applicable
//      Description:
//          This API is constructor of a class AnalogMedia.
//          It initializes the newly created object with requestId and
//          channel Id.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
AnalogMedia:: AnalogMedia(SERVER_INFO_t serverInfo,
                          REQ_INFO_t &requestInfo,
                          SET_COMMAND_e commandId,
                          quint8 channelId)
    : chId(channelId), cmdId(commandId), statusId(CMD_MAX_DEVICE_REPLY)
{
    srvrInfo.ipAddress = serverInfo.ipAddress;
    srvrInfo.tcpPort = serverInfo.tcpPort;

    reqInfo.payload = requestInfo.payload;
    reqInfo.requestId = requestInfo.requestId;
    reqInfo.sessionId = requestInfo.sessionId;
    reqInfo.timeout = requestInfo.timeout;
    reqInfo.bytePayload = NULL;

    runFlag = true;
    stopFlag = false;
    commandReq = NULL;
}

AnalogMedia::~AnalogMedia()
{
    if(commandReq != NULL)
    {
        delete commandReq;
    }
}

//*****************************************************************************
//  run()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//          NONE
//      Description:
//          The starting point for the thread. After calling start(), the newly
//          created thread calls this function.
//          This API check the status of camera whether it is enable or disable
//
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void AnalogMedia::run()
{
    //store camera satus enable/disable
    bool status = false;
    bool videoPriv = false;

    char threadName[16];

    QString     replyStr;
    QStringList payloadField;

    snprintf(threadName,sizeof(threadName), "ANA_MED_%d", chId);
    prctl(PR_SET_NAME, threadName, 0, 0, 0);

    DEVICE_REPLY_TYPE_e devResponse = CMD_SUCCESS;

    // for checking privilage
    reqInfo.payload.clear ();
    reqInfo.payload = QString("%1").arg (chId);
    reqInfo.payload.append (FSP);
    reqInfo.bytePayload = NULL;

    status = createCmdReq(srvrInfo,
                          reqInfo,
                          CHK_PRIV);
    if(status == true)
    {
        commandReq->getBlockingRes(replyStr, devResponse);
        deleteCmdReq ();
    }

    payloadField = replyStr.split(FSP);
    videoPriv = (bool)payloadField.at(1).toUInt();

    if(videoPriv == false)
    {
        emit sigAnalogMediaResponse(reqInfo.requestId,
                                    cmdId,
                                    CMD_NO_PRIVILEGE,
                                    "");
    }
    else
    {
        emit sigAnalogMediaResponse(reqInfo.requestId,
                                    cmdId,
                                    CMD_CHANNEL_DISABLED,
                                    "");
    }
}

//*****************************************************************************
//  setRunFlag()
//      Param:
//          IN : bool flag
//          OUT: NONE
//	Returns:
//		 NONE
//      Description:
//          This API sets the status of run flag to true or false.
//          Which in turn breaks the media loop, if set to false.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void AnalogMedia::setRunFlag(bool flag)
{
    // lock write access for run flag
    runFlagLock.lock();
    // set run flag to the value of flag
    runFlag = flag;
    // unlock access to run flag
    runFlagLock.unlock ();
}

void AnalogMedia::setStopFlag(bool flag)
{
    stopFlagAccess.lock ();
    stopFlag = flag;
    stopFlagAccess.unlock ();
}

bool AnalogMedia::getStopFlag(void)
{
    bool flag = false;
    stopFlagAccess.lock ();
    flag = stopFlag;
    stopFlagAccess.unlock ();

    return flag;
}

//*****************************************************************************
//  createCmdReq ()
//      Param:
//          IN : SERVER_INFO_t serverInfo
//               REQ_INFO_t requestInfo
//               SET_COMMAND_e cmd
//          OUT: NONE
//	Returns:
//		 bool
//      Description:
//          This API sets the status of run flag to true or false.
//          Which in turn breaks the media loop, if set to false.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool AnalogMedia::createCmdReq(SERVER_INFO_t serverInfo,
                               REQ_INFO_t &requestInfo,
                               SET_COMMAND_e cmd)
{
    bool status = false;

    if(commandReq == NULL)
    {
        commandReq = new CommandRequest(serverInfo,
                                        requestInfo,
                                        cmd,
                                        STRM_RELAT_CMD_SES_ID);

        if(commandReq != NULL)
        {
            status = true;
        }
    }
    return status;
}

bool AnalogMedia::deleteCmdReq()
{
    bool status = false;

    if(commandReq != NULL)
    {
        delete commandReq;
        commandReq = NULL;
        status = true;
    }
    return status;
}

void AnalogMedia::setServerSessionInfo(SERVER_INFO_t serverInfo,
                                       REQ_INFO_t &requestInfo,
                                       quint8 channelId)
{
    // store server parameter parameter
    srvrInfo.ipAddress = serverInfo.ipAddress;
    srvrInfo.tcpPort = serverInfo.tcpPort;

    // store request parameter
    reqInfo.sessionId = requestInfo.sessionId;
    reqInfo.requestId = requestInfo.requestId;
    reqInfo.payload = requestInfo.payload;
    reqInfo.timeout = requestInfo.timeout;
    reqInfo.bytePayload = NULL;

    chId = channelId;
}

