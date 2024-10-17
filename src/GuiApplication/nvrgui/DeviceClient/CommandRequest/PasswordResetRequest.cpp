#include "PasswordResetRequest.h"

//******** Static Variables **********
bool PasswordResetRequest::requestCmdFree[PWD_RST_CMD_SESSION_MAX] = {true};

// access lock for stream request status array
QMutex PasswordResetRequest::requestCmdLock;

PasswordResetRequest::PasswordResetRequest(SERVER_INFO_t serverInfo,
                                           REQ_INFO_t &requestInfo,
                                           PWD_RST_CMD_e commandId,
                                           quint8 cmdSesId)
    : GenericRequest(serverInfo, requestInfo), cmdSessionId(cmdSesId), cmdId(commandId)
{
    /* Prepend related command string to payload string */
    request.payload.prepend(FSP);
    request.payload.prepend(pwdRstCmdStr[cmdId]);

    /* Declare the timeout of the request */
    request.timeout = pwdRstCmdTimeout[cmdId];

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

PasswordResetRequest::~PasswordResetRequest()
{
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

void PasswordResetRequest::initializeCmdSes(void)
{
    requestCmdLock.lock();
    for(quint8 index = 0; index < PWD_RST_CMD_SESSION_MAX; index++)
    {
        requestCmdFree[index] = true;
    }
    requestCmdLock.unlock();
}

bool PasswordResetRequest::getFreeCmdSession(quint8 &sesId)
{
    bool status = false;

    /* lock access to stream session list */
    requestCmdLock.lock ();

    /* loop till maximum stream session */
    for (int index = 0; index < PWD_RST_CMD_SESSION_MAX; ++index)
    {
        /* if session free flag is true */
        status = requestCmdFree[index];
        if (status == true)
        {
            /* store index of session to session Id */
            sesId = index;

            /* set session free flag to false */
            requestCmdFree[sesId] = false;

            /* break the loop */
            break;
        }
    }

    /* unlock access to stream session list */
    requestCmdLock.unlock ();

    /* return status */
    return status;
}

bool PasswordResetRequest::setCmdSessionFree(quint8 sesId)
{
    /* if stream session id is valid */
    if (sesId >= PWD_RST_CMD_SESSION_MAX)
    {
        return false;
    }

    /* set session free flag to true */
    requestCmdLock.lock ();
    requestCmdFree[sesId] = true;
    requestCmdLock.unlock ();

    /* return status */
    return true;
}

void PasswordResetRequest::slotGenericResponse(REQ_MSG_ID_e requestId,
                                               DEVICE_REPLY_TYPE_e respStatusId,
                                               QString payload,
                                               quint8 genReqSesId)
{
    /* emit signal to notify status of operation */
    emit sigPwdRstCmdResponse(requestId, cmdId, respStatusId, payload, cmdSessionId);
    (void) (genReqSesId);
}
