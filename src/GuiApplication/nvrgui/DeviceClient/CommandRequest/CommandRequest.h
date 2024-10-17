#ifndef COMMANDREQUEST_H
#define COMMANDREQUEST_H
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
//   Project      : NVR (Network Video Recorder)
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>
#include <QMutex>

#include "../GenericRequest/GenericRequest.h"

//******** Defines and Data Types ****
#define MAX_CMD_SESSION             4
#define STRM_RELAT_CMD_SES_ID       255

//******** Function Prototypes *******
class CommandRequest : public GenericRequest
{
    Q_OBJECT

public:
    // constructor function, which creates and initializes object
    // with server info, request info and commandId
    CommandRequest (SERVER_INFO_t serverInfo,
                    REQ_INFO_t &requestInfo,
                    SET_COMMAND_e commandId,
                    quint8 cmdSesId);
    // destructor function
    ~CommandRequest ();

    // checks the pool for free session and occupies it, if found any.
    static bool getFreeCmdSession (quint8 &sesId);
    // sets a session to free
    static bool setCmdSessionFree (quint8 sesId);
    static void initializeCmdSes (void);

    // valid only for stream related commands
    SET_COMMAND_e getCommandOfCommandReq();

    void setServerSessionInfo(SERVER_INFO_t serverInfo, REQ_INFO_t &requestInfo);

protected:
    quint8 cmdSessionId;
    // variable to hold command Id
    SET_COMMAND_e cmdId;

    // holds status of session
    static bool requestCmdFree [MAX_CMD_SESSION];
    // access lock to session status
    static QMutex requestCmdLock;

    // this API prepends command string to payload
    bool prependCommand (void);

private:

signals:
    // signal to convey the command response
    void sigCommandResponse (REQ_MSG_ID_e requestId,
                             SET_COMMAND_e commandId,
                             DEVICE_REPLY_TYPE_e statusId,
                             QString payload,
                             quint8 cmdSesId);

public slots:
	// this slot is used to catch the generic response
	void slotGenericResponse (REQ_MSG_ID_e requestId,
								DEVICE_REPLY_TYPE_e tstatusId,
								QString payload,
								quint8 genReqSesId);
};

#endif // COMMANDREQUEST_H
