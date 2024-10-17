#include "CosecVideoPopupUser.h"

#define COSEC_VIDEO_POPUP_HEIGHT    SCALE_HEIGHT(300)
#define COSEC_VIDEO_POPUP_WIDTH     SCALE_WIDTH(760)

#define COSEC_VIDEO_POPUP_HEADING  "COSEC Event Details"

static const QString userDetailLabels[] = {"User ID:",
                                           "Username" + QString(":"),
                                           "Door Name:",
                                           "Date" + QString(":"),
                                           "Time" + QString(":")};

static const QString monthArray[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                     "Jul","Aug", "Sep", "Oct", "Nov", "Dec"};

static const QString evtCodeMsgArray[] = {
    "User Allowed",
    "User Denied",
    "User Denied – User Not Identified",
    "Aux In-1 Status Changed",
    "Duress Detection",
    "Dead Man timer Expired Alarm",
    "Panic Alarm",
    "Door abnormal",
    "Door force open",
    "Tamper alarm",
    "Intercom Panic",
    "Aux In-2 Status Changed",
    "Aux In-3 Status Changed",
    "Aux In-4 Status Changed",
    "Aux In-5 Status Changed",
    "Aux In-6 Status Changed",
    "Aux In-7 Status Changed",
    "Aux In-8 Status Changed"};

CosecVideoPopupUser::CosecVideoPopupUser(QWidget *parent) :
    QWidget(parent)
{
    this->setGeometry (ApplController::getXPosOfScreen(),
                       ApplController::getYPosOfScreen(),
                       ApplController::getWidthOfScreen(),
                       ApplController::getHeightOfScreen());

    applController = ApplController::getInstance ();
    payloadLib = new PayloadLib();

    backGround = new Rectangle((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - COSEC_VIDEO_POPUP_WIDTH) / 2)) ,
                               (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - COSEC_VIDEO_POPUP_HEIGHT) / 2)),
                               COSEC_VIDEO_POPUP_WIDTH,
                               COSEC_VIDEO_POPUP_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x () + backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    0);

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(20),
                          COSEC_VIDEO_POPUP_HEADING,
                          this,
                          HEADING_TYPE_1);

    eventCodeLabel = new Heading(backGround->x () + backGround->width ()/2,
                                 backGround->y () + SCALE_HEIGHT(55),
                                 "",
                                 this,
                                 HEADING_TYPE_2);

    for(quint8 index = 0; index < MAX_USER_DETAILS_FIELD; index++)
    {
        userDetails[index] = new ReadOnlyElement (backGround->x () + (backGround->width () - SCALE_WIDTH(720))/2,
                                                  backGround->y () + SCALE_HEIGHT(75) + index*BGTILE_HEIGHT,
                                                  SCALE_WIDTH(720),
                                                  BGTILE_HEIGHT,
                                                  SCALE_WIDTH(350),
                                                  READONLY_HEIGHT,
                                                  "",
                                                  this,
                                                  COMMON_LAYER,
                                                  -1,
                                                  SCALE_WIDTH(10),
                                                  userDetailLabels[index]);
    }

    closeButton->forceActiveFocus ();

    requestCosecPopUp ();

    this->show();
}

CosecVideoPopupUser :: ~CosecVideoPopupUser()
{
    delete payloadLib;
    delete backGround;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete closeButton;

    delete heading;
    delete eventCodeLabel;

    for(quint8 index = 0; index < MAX_USER_DETAILS_FIELD; index++)
    {
        delete userDetails[index];
    }
}

void CosecVideoPopupUser::requestCosecPopUp()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = COSEC_VIDEO_POPUP;

    applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void CosecVideoPopupUser::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if( (deviceName == LOCAL_DEVICE_NAME) &&
            (param->cmdType == COSEC_VIDEO_POPUP) &&
            (param->deviceStatus == CMD_SUCCESS) )
    {
        payloadLib->parseDevCmdReply(true, param->payload);
        quint8 evtCode;

        evtCode = (payloadLib->getCnfgArrayAtIndex (2).toUInt ());

        if((evtCode == 0) || (evtCode == 1) || (evtCode == 5))
        {
            userDetails[0]->changeValue ((payloadLib->getCnfgArrayAtIndex (0).toString ()));
            userDetails[1]->changeValue ((payloadLib->getCnfgArrayAtIndex (1).toString ()));
        }
        else
        {
            userDetails[0]->changeValue("---");
            userDetails[1]->changeValue("---");
        }

        eventCodeLabel->changeHeadingText (evtCodeMsgArray[evtCode]);
        userDetails[2]->changeValue ((payloadLib->getCnfgArrayAtIndex (3).toString ()));
        userDetails[3]->changeValue ((payloadLib->getCnfgArrayAtIndex (5).toString ()));
        userDetails[4]->changeValue ((payloadLib->getCnfgArrayAtIndex (6).toString ()));
    }
}

void CosecVideoPopupUser:: slotButtonClick(int)
{
    delete this;
}
