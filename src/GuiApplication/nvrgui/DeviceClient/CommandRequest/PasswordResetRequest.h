#ifndef PASSWORD_RESET_REQUEST_H
#define PASSWORD_RESET_REQUEST_H

#include <QObject>
#include <QMutex>

#include "../GenericRequest/GenericRequest.h"

#define PWD_RST_CMD_SESSION_MAX     2
#define CMD_NORMAL_TIMEOUT          7

class PasswordResetRequest : public GenericRequest
{
    Q_OBJECT

public:
    /* Constructor function, which creates and initializes object with server info, request info and commandId */
    PasswordResetRequest(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo, PWD_RST_CMD_e commandId, quint8 cmdSesId);

    /* Destructor function */
    ~PasswordResetRequest();

    /* Checks the pool for free session and occupies it, if found any */
    static bool getFreeCmdSession(quint8 &sesId);

    /* Sets a session to free */
    static bool setCmdSessionFree(quint8 sesId);
    static void initializeCmdSes(void);

    /* Valid only for stream related commands */
    PWD_RST_CMD_e getCommandOfCommandReq(void);

    void setServerSessionInfo(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo);

protected:
    quint8 cmdSessionId;

    /* Variable to hold command Id */
    PWD_RST_CMD_e cmdId;

    /* Holds status of session */
    static bool requestCmdFree [PWD_RST_CMD_SESSION_MAX];

    /* Access lock to session status */
    static QMutex requestCmdLock;

private:

signals:
    /* signal to convey the password reset command response */
    void sigPwdRstCmdResponse(REQ_MSG_ID_e requestId,
                              PWD_RST_CMD_e commandId,
                              DEVICE_REPLY_TYPE_e statusId,
                              QString payload,
                              quint8 cmdSesId);

public slots:
    /* this slot is used to catch the generic response */
    void slotGenericResponse(REQ_MSG_ID_e requestId,
                             DEVICE_REPLY_TYPE_e respStatusId,
                             QString payload,
                             quint8 genReqSesId);
};

#endif // PASSWORD_RESET_REQUEST_H
