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
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////

#include "CommandRequest.h"

//******** Extern Variables **********

//******** Defines and Data Types ****

//******** Function Prototypes *******


//******** Global Variables **********


//******** Static Variables **********
bool CommandRequest::requestCmdFree[MAX_CMD_SESSION] = {true};
// access lock for stream request status array
QMutex CommandRequest::requestCmdLock;

//******** Function Definitions ******

//*****************************************************************************
//  CommandRequest ()
//      Param:
//          IN : SERVER_INFO_t serverInfo   // server informations
//               REQ_INFO_t requestInfo     // request information
//               SET_COMMAND_e commandId // command index
//          OUT: NONE
//	Returns:
//		Not Applicable
//      Description:
//          This API is constructor of class CommandRequest. It initializes the
//          newly created object with server, request and commandId.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
CommandRequest::CommandRequest (SERVER_INFO_t serverInfo,
                                REQ_INFO_t &requestInfo,
                                SET_COMMAND_e commandId,
                                quint8 cmdSesId)

    :GenericRequest(serverInfo, requestInfo), cmdSessionId(cmdSesId), cmdId(commandId)
{
    // prepend related command string to payload string
    prependCommand();

    /* Declare the timeout of the request. For live view, we will apply pre video loss duration for timeout */
    if (commandId != SRT_LV_STRM)
    {
        request.timeout = cmdTimeout[commandId];
    }

    // connect signal of generic request object to the slot of command request object
    connect(this,
            SIGNAL(sigGenericResponse(REQ_MSG_ID_e,
                                      DEVICE_REPLY_TYPE_e,
                                      QString,
                                      quint8)),
            this,
            SLOT(slotGenericResponse(REQ_MSG_ID_e,
                                     DEVICE_REPLY_TYPE_e,
                                     QString,
                                     quint8)));
}

//*****************************************************************************
//  ~CommandRequest ()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//	Returns:
//		Not Applicable
//      Description:
//          This API is distructor of class CommandRequest.
//          As of now it does no functionality, kept for future use.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
CommandRequest::~CommandRequest ()
{
    // disconnect signal of generic request object
    // from the slot of command request object
	disconnect (this,
				SIGNAL(sigGenericResponse(REQ_MSG_ID_e,
										DEVICE_REPLY_TYPE_e,
										QString,
										quint8)),
				this,
				SLOT(slotGenericResponse(REQ_MSG_ID_e,
										DEVICE_REPLY_TYPE_e,
										QString,
										quint8)));
}

//*****************************************************************************
//  setChannelId ()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API set channel id.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void CommandRequest::initializeCmdSes()
{
    requestCmdLock.lock ();
    for(quint8 index = 0; index<MAX_CMD_SESSION; index++)
    {
        requestCmdFree[index] = true;
    }
    requestCmdLock.unlock ();
}

//*****************************************************************************
//  prependCommand ()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API prepends command string to the payload.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool CommandRequest::prependCommand ()
{
    bool status = true;

    // prepend FSP to payload
    request.payload.prepend (FSP);

    // prepend command string to payload
    request.payload.prepend (cmdString[cmdId]);

    // return status
    return status;
}

//*****************************************************************************
//  getFreeCmdSession ()
//      Param:
//          IN : quint8 &sessionId      // reference to stream session variable
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API checks the list for free session and occupies it,
//          if found any.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool CommandRequest::getFreeCmdSession (quint8 &sesId)
{
    bool status = false;

    // lock access to stream session list
    requestCmdLock.lock ();

    // loop till maximum stream session
    for (int index = 0; index < MAX_CMD_SESSION; ++index)
    {
        // if session free flag is true
        if ((status = requestCmdFree [index]) == true)
        {
            // store index of session to session Id
            sesId = index;
            // set session free flag to false
            requestCmdFree[sesId] = false;

            // break the loop
            break;
        }
    }

    // unlock access to stream session list
    requestCmdLock.unlock ();

    // return status
    return status;
}

//*****************************************************************************
//  setSessionFree ()
//      Param:
//          IN : quint8 sessionId
//          OUT: NONE
//	Returns:
//		bool [true / false]
//      Description:
//          This API frees the specified stream session.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
bool CommandRequest::setCmdSessionFree (quint8 sesId)
{
    bool status = false;

    // if stream session id is valid
    if (sesId < MAX_CMD_SESSION)
    {
        // lock access to stream session list
        requestCmdLock.lock ();
        // set session free flag to true
        requestCmdFree[sesId] = true;
        // unlock access to stream session list
        requestCmdLock.unlock ();

        // set status true
        status = true;
    }

    // return status
    return status;
}

SET_COMMAND_e CommandRequest::getCommandOfCommandReq()
{
    return cmdId;
}

void CommandRequest::setServerSessionInfo(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo)
{
    // store server parameter
    server.ipAddress = serverInfo.ipAddress;
    server.tcpPort = serverInfo.tcpPort;

    // store request parameter
    request.sessionId = requestInfo.sessionId;
    request.requestId = requestInfo.requestId;
    request.payload = requestInfo.payload;
    request.bytePayload = NULL;

    prependCommand();
    request.timeout = cmdTimeout[cmdId];
}

//*****************************************************************************
//  prependCommand ()
//      Param:
//          IN : NONE
//          OUT: NONE
//	Returns:
//		BOOL [true / false]
//      Description:
//          This API catches signal from GenericRequest module and then emits
//          its own signal with command signature.
//	[Pre-condition:]
//          NONE.
//	[Constraints:]
//          NONE
//
//*****************************************************************************
void CommandRequest::slotGenericResponse (REQ_MSG_ID_e requestId,
                                          DEVICE_REPLY_TYPE_e tstatusId,
                                          QString payload,
										  quint8 genReqSesId)
{
    (void) (genReqSesId);
    // emit signal to notify status of operation
    emit sigCommandResponse (requestId,
                             cmdId,
							 tstatusId,
                             payload,
                             cmdSessionId);
}
