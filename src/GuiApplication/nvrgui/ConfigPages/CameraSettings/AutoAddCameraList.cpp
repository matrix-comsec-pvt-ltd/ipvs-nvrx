#include "AutoAddCameraList.h"
#include "ValidationMessage.h"
#include "MxCommandFields.h"
#include "QPainter"

#define CAM_SERCH_FEILD_CELL_HEIGHT     SCALE_HEIGHT(60)
#define CAM_SERCH_BGTILE_WIDTH          SCALE_WIDTH(876)
#define CAM_ADD_NO                      0
#define MAX_NAVIGATE_CONTROL_IN_ROW     1

#define CAMERA_SETTING_FROM_FIELD                   1
#define CAMERA_SETTING_TO_FIELD_IP_TABLE            16
#define CNFG_TO_INDEX                   			1
#define CAM_SEARCH_AUTO_CNFG_FIELD                  20


typedef enum {
    CAM_SRCH_SEL_ALL_CAM,
    CAM_SRCH_SEL_CAM,    
    CAM_SRCH_PREVIOUS_BUTTON = (CAM_SRCH_SEL_ALL_CAM + 3*MAX_RECORD_DATA_AUTO_ADD),
    CAM_SRCH_ADD_BUTTON,
    CAM_SRCH_REJECT_BUTTON,
    CAM_SRCH_NEXT_BUTTON,
    CAM_SRCH_CLOSE_BUTTON,

    MAX_CAM_SRCH_ELEMENT
}CAM_SRCH_ELEMENT_e;

typedef enum {
    MAC_ADDRESS_STR,
    IP_ADDR_STR,
    MODEL_STR,
    ACTIVATION_STR,
    ADD_STR,
    REJECT_STR,
    NEXT_STR,
    PREVIOUS_STR,
    MAX_AUTO_ADD_CAM_STR

}CAM_SEARCH_STR;

static const QString cameraSearchStr[MAX_AUTO_ADD_CAM_STR] ={
    "MAC Address",
    "IP Address",
    "Model",
    "Activation",
    "Add",
    "Reject",
    "Next",
    "Previous"
};

AutoAddCameraList::AutoAddCameraList(QString m_currDevName, QWidget* parent, PayloadLib* payloadLib)
    : KeyBoard(parent), NavigationControl(0,true), maximumPages(0), maxSearchListCount(0)
{
    this->setGeometry (0, 0, parent->width(), parent->height());

    for(quint8 loop = 0; loop < 100; loop++)
    {
        m_elementList[loop] = NULL;
    }

    applController = ApplController::getInstance();

    currDevName = m_currDevName;
    m_payloadLib = payloadLib;

    createDefaultComponent();

    m_currentElement = CAM_SRCH_ADD_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();

    this->show ();

    sendCommand(GET_CAM_INITIATED_LIST);
}

AutoAddCameraList::~AutoAddCameraList ()
{
    if ( IS_VALID_OBJ(selectAllCam))
    {
        disconnect (selectAllCam,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

        disconnect (selectAllCam,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(selectAllCam);
    }

    for ( quint8 index = 0 ; index < MAX_RECORD_FEILDS_AUTO_ADD ; index++)
    {
        if(IS_VALID_OBJ(fieldsHeading[index]))
            DELETE_OBJ(fieldsHeading[index]);

        DELETE_OBJ(fieldsHeadingStr[index]);
    }

    for ( quint8 index = 0 ; index < MAX_RECORD_DATA_AUTO_ADD; index++)
    {
        DELETE_OBJ(srNumber[index]);
        DELETE_OBJ(macAddress[index]);

        DELETE_OBJ(macAddressStr[index]);
        DELETE_OBJ(ipAddress[index]);

        DELETE_OBJ(ipAddrStr[index]);
        DELETE_OBJ(model[index]);
        DELETE_OBJ(modelStr[index]);

        if ( IS_VALID_OBJ(selectCam[index]))
        {
            disconnect (selectCam[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

            disconnect (selectCam[index],
                        SIGNAL(sigUpdateCurrentElement(int)),
                        this,
                        SLOT(slotUpdateCurrentElement(int)));

            DELETE_OBJ(selectCam[index]);
        }
    }

    if ( IS_VALID_OBJ(m_closeButton))
    {
        disconnect (m_closeButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        disconnect (m_closeButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));


        DELETE_OBJ(m_closeButton);
    }

    if ( IS_VALID_OBJ(previousButton))
    {
        disconnect (previousButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));

        disconnect (previousButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(previousButton);
    }

    if ( IS_VALID_OBJ(addButton))
    {
        disconnect (addButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));

        disconnect (addButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(addButton);
    }

    if ( IS_VALID_OBJ(rejectButton))
    {
        disconnect (rejectButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));

        disconnect (rejectButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ (rejectButton);
    }

    if ( IS_VALID_OBJ(nextButton))
    {
        disconnect (nextButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));

        disconnect (nextButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ (nextButton);
    }

    DELETE_OBJ(processBar);

    if ( IS_VALID_OBJ(nextButton))
    {
        disconnect(infoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(infoPage);
    }

    DELETE_OBJ(topBgtile);
    DELETE_OBJ(bottomBgtile);

    DELETE_OBJ(m_backGround);
    DELETE_OBJ(m_heading);
}

void AutoAddCameraList::createDefaultComponent()
{
    //initalization of the variables
    INIT_OBJ(topBgtile);
    INIT_OBJ(bottomBgtile);

    INIT_OBJ(m_backGround);
    INIT_OBJ(m_closeButton);

    INIT_OBJ(m_heading);
    INIT_OBJ(selectAllCam);

    quint16 headerWidthArray[] = {35, 170, 350, 200, 100};

    currentPageNo = 0;
    isCancelSend = false;

    for ( quint8 index = 0 ; index < MAX_RECORD_FEILDS_AUTO_ADD ; index++)
    {
        INIT_OBJ(fieldsHeading[index]);
        INIT_OBJ(fieldsHeadingStr[index]);
    }

    //backgound of the page
    m_backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CAM_SERCH_BGTILE_WIDTH) / 2))-SCALE_WIDTH(15),
                                 (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - (6*CAM_SERCH_FEILD_CELL_HEIGHT)) / 2)) - SCALE_HEIGHT(60),
                                 CAM_SERCH_BGTILE_WIDTH + SCALE_WIDTH(30),
                                 (6*CAM_SERCH_FEILD_CELL_HEIGHT) + SCALE_HEIGHT(160),
                                 0,
                                 NORMAL_BKG_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this);
    //heading of the page
    m_heading = new Heading((m_backGround->x () + (m_backGround->width () / 2)),
                            (m_backGround->y () + SCALE_HEIGHT(30)),
                            "Auto Add Camera",
                            this,
                            HEADING_TYPE_2);

    //close button
    m_closeButton = new CloseButtton ((m_backGround->x () + m_backGround->width () - SCALE_WIDTH(20)),
                                      (m_backGround->y () + SCALE_HEIGHT(30)),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      CAM_SRCH_CLOSE_BUTTON);

    if ( IS_VALID_OBJ(m_closeButton))
    {
        m_elementList[CAM_SRCH_CLOSE_BUTTON] = m_closeButton;

        connect (m_closeButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (m_closeButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    //top background tile
    topBgtile = new BgTile((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CAM_SERCH_BGTILE_WIDTH) / 2)),
                           (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- (6*CAM_SERCH_FEILD_CELL_HEIGHT)) / 2)),
                           CAM_SERCH_BGTILE_WIDTH,
                           (6*CAM_SERCH_FEILD_CELL_HEIGHT),
                           TOP_LAYER,
                           this);

    //bottom background tile
    bottomBgtile = new BgTile(topBgtile->x (),
                              topBgtile->y () + topBgtile->height (),
                              topBgtile->width (),
                              CAM_SERCH_FEILD_CELL_HEIGHT,
                              BOTTOM_LAYER,
                              this);

    fieldsHeading[0] = new TableCell(topBgtile->x () + SCALE_WIDTH(10),
                                     topBgtile->y () + SCALE_HEIGHT(10),
                                     (SCALE_WIDTH(headerWidthArray[0]) - 1),
                                     SCALE_HEIGHT(50),
                                     this,
                                     true);

    selectAllCam = new OptionSelectButton(fieldsHeading[0]->x () + SCALE_WIDTH(5),
                                          fieldsHeading[0]->y (),
                                          fieldsHeading[0]->width (),
                                          fieldsHeading[0]->height (),
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "","", -1,
                                          CAM_SRCH_SEL_ALL_CAM,
                                          true);

    if ( IS_VALID_OBJ(selectAllCam))
    {
        m_elementList[CAM_SRCH_SEL_ALL_CAM] = selectAllCam;

        connect (selectAllCam,
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

        connect (selectAllCam,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    for ( quint8 index = 1 ; index < MAX_RECORD_FEILDS_AUTO_ADD ; index++)
    {
        fieldsHeading[index] = new TableCell(fieldsHeading[index -1]->x () +
                                             fieldsHeading[index -1]->width (),
                                             fieldsHeading[index -1]->y (),
                                             (SCALE_WIDTH(headerWidthArray[index]) - 1),
                                             SCALE_HEIGHT(50),
                                             this,
                                             true);

        fieldsHeadingStr[index-1] = new TextLabel(fieldsHeading[(index)]->x () + SCALE_WIDTH(10),
                                                  fieldsHeading[(index)]->y () +
                                                  (fieldsHeading[index]->height ())/2,
                                                  NORMAL_FONT_SIZE,
                                                  cameraSearchStr[index-1],
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, 0, (SCALE_WIDTH(headerWidthArray[index]) - 1));
    }

    srNumber[0] = new TableCell(fieldsHeading[0]->x (),
                                fieldsHeading[0]->y () +
                                fieldsHeading[0]->height (),
                                fieldsHeading[0]->width (),
                                BGTILE_HEIGHT,
                                this);

    selectCam[0] = new OptionSelectButton(srNumber[0]->x () + SCALE_WIDTH(5),
                                          srNumber[0]->y (),
                                          srNumber[0]->width (),
                                          srNumber[0]->height (),
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "","", -1,
                                          CAM_SRCH_SEL_CAM,
                                          true);

    if ( IS_VALID_OBJ(selectCam[0]))
    {
        m_elementList[CAM_SRCH_SEL_CAM] = selectCam[0];

        selectCam[0]->setVisible (false);

        connect (selectCam[0],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

        connect (selectCam[0],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    macAddress[0] = new TableCell(fieldsHeading[1]->x (),
                                 fieldsHeading[1]->y () +
                                 fieldsHeading[1]->height (),
                                 fieldsHeading[1]->width (),
                                 BGTILE_HEIGHT,
                                 this);

    macAddressStr[0] = new TextLabel(macAddress[0]->x () + SCALE_WIDTH(10),
                                    macAddress[0]->y () +
                                    (macAddress[0]->height ())/2,
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y, 0, 0, fieldsHeading[1]->width ());

    ipAddress[0] = new TableCell(fieldsHeading[2]->x (),
                              fieldsHeading[2]->y () +
                              fieldsHeading[2]->height (),
                              fieldsHeading[2]->width (),
                              BGTILE_HEIGHT,
                              this);

    ipAddrStr[0] = new TextLabel(ipAddress[0]->x () + SCALE_WIDTH(10),
                                 ipAddress[0]->y () +
                                 (ipAddress[0]->height ())/2,
                                 NORMAL_FONT_SIZE,
                                 "",
                                 this,
                                 NORMAL_FONT_COLOR,
                                 NORMAL_FONT_FAMILY,
                                 ALIGN_START_X_CENTRE_Y, 0, 0, fieldsHeading[2]->width ());

    model[0] = new TableCell(fieldsHeading[3]->x (),
                             fieldsHeading[3]->y () +
                             fieldsHeading[3]->height (),
                             fieldsHeading[3]->width (),
                             BGTILE_HEIGHT,
                             this);

    modelStr[0] = new TextLabel(model[0]->x () + SCALE_WIDTH(10),
            model[0]->y () +
            (model[0]->height ())/2,
            NORMAL_FONT_SIZE,
            "",
            this,
            NORMAL_FONT_COLOR,
            NORMAL_FONT_FAMILY,
            ALIGN_START_X_CENTRE_Y, 0, 0, fieldsHeading[3]->width ());

    status[0] = new TableCell(fieldsHeading[4]->x (),
                             fieldsHeading[4]->y () +
                             fieldsHeading[4]->height (),
                             fieldsHeading[4]->width (),
                             BGTILE_HEIGHT,
                             this);

    statusStr[0] = new TextLabel(status[0]->x () + SCALE_WIDTH(10),
            status[0]->y () +
            (status[0]->height ())/2,
            NORMAL_FONT_SIZE,
            "",
            this,
            NORMAL_FONT_COLOR,
            NORMAL_FONT_FAMILY,
            ALIGN_START_X_CENTRE_Y, 0, 0, fieldsHeading[4]->width ());

    for(quint8 index = 1 ; index < MAX_RECORD_DATA_AUTO_ADD; index++)
    {

        srNumber[index] = new TableCell(srNumber[(index -1)]->x (),
                                        srNumber[(index -1)]->y () +
                                        srNumber[(index -1)]->height (),
                                        srNumber[(index -1)]->width () - 1,
                                        BGTILE_HEIGHT,
                                        this);

        selectCam[index] = new OptionSelectButton(srNumber[index]->x () + SCALE_WIDTH(5),
                                                  srNumber[index]->y (),
                                                  srNumber[index]->width (),
                                                  srNumber[index]->height (),
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  NO_LAYER,
                                                  "","", -1,
                                                  (CAM_SRCH_SEL_CAM + (index*MAX_NAVIGATE_CONTROL_IN_ROW)),
                                                  true);

        if ( IS_VALID_OBJ(selectCam[index]))
        {
            m_elementList[CAM_SRCH_SEL_CAM + (index*MAX_NAVIGATE_CONTROL_IN_ROW)] = selectCam[index];

            selectCam[index]->setVisible (false);

            connect (selectCam[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

            connect (selectCam[index],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));
        }

        macAddress[index] = new TableCell(macAddress[(index -1)]->x (),
                                         macAddress[(index -1)]->y () +
                                         macAddress[(index -1)]->height (),
                                         macAddress[(index -1)]->width () - 1,
                                         BGTILE_HEIGHT,
                                         this);

        macAddressStr[index] = new TextLabel(macAddress[index]->x () + SCALE_WIDTH(10),
                                            macAddress[index]->y () +
                                            (macAddress[index]->height ())/2,
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, macAddress[(index -1)]->width () - 1);

        ipAddress[index] = new TableCell(ipAddress[(index -1)]->x (),
                                      ipAddress[(index -1)]->y () +
                                      ipAddress[(index -1)]->height (),
                                      ipAddress[(index -1)]->width () - 1,
                                      BGTILE_HEIGHT,
                                      this);

        ipAddrStr[index] = new TextLabel(ipAddress[index]->x () + SCALE_WIDTH(10),
                                         ipAddress[index]->y () +
                                         (ipAddress[index]->height ())/2,
                                         NORMAL_FONT_SIZE,
                                         "",
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_CENTRE_Y, 0, 0, ipAddress[(index -1)]->width () - 1);

        model[index] = new TableCell(model[(index -1)]->x (),
                                     model[(index -1)]->y () +
                                     model[(index -1)]->height (),
                                     model[(index -1)]->width () - 1,
                                     BGTILE_HEIGHT,
                                     this);

        modelStr[index] = new TextLabel(model[index]->x () + SCALE_WIDTH(10),
                                        model[index]->y () +
                                        (model[index]->height ())/2,
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, model[(index -1)]->width () - 1);

        status[index] = new TableCell(status[(index -1)]->x (),
                                     status[(index -1)]->y () +
                                     status[(index -1)]->height (),
                                     status[(index -1)]->width () - 1,
                                     BGTILE_HEIGHT,
                                     this);

        statusStr[index] = new TextLabel(status[index]->x () + SCALE_WIDTH(10),
                                        status[index]->y () +
                                        (status[index]->height ())/2,
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, status[(index -1)]->width () - 1);
    }

    previousButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                       topBgtile->x () + SCALE_WIDTH(15),
                                       bottomBgtile->y () + SCALE_HEIGHT(20),
                                       topBgtile->width (),
                                       BGTILE_HEIGHT,
                                       this,
                                       NO_LAYER,
                                       -1,
                                       cameraSearchStr[PREVIOUS_STR],
                                       false,
                                       CAM_SRCH_PREVIOUS_BUTTON,
                                       false);

    if ( IS_VALID_OBJ(previousButton))
    {
        m_elementList[CAM_SRCH_PREVIOUS_BUTTON] = previousButton;

        connect (previousButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (previousButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    addButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  topBgtile->x () + SCALE_WIDTH(380),
                                  previousButton->y () + SCALE_HEIGHT(20) ,
                                  cameraSearchStr[ADD_STR],
                                  this,
                                  CAM_SRCH_ADD_BUTTON);

    if ( IS_VALID_OBJ(addButton))
    {
        m_elementList[CAM_SRCH_ADD_BUTTON] = addButton;

        connect (addButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (addButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    rejectButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  topBgtile->x () + SCALE_WIDTH(530),
                                  previousButton->y () + SCALE_HEIGHT(20) ,
                                  cameraSearchStr[REJECT_STR],
                                  this,
                                  CAM_SRCH_REJECT_BUTTON,
                                  true);

    if ( IS_VALID_OBJ(rejectButton))
    {
        m_elementList[CAM_SRCH_REJECT_BUTTON] = rejectButton;

        connect (rejectButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (rejectButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   topBgtile->x () + topBgtile->width () - SCALE_WIDTH(90),
                                   bottomBgtile->y () + SCALE_HEIGHT(20),
                                   topBgtile->width (),
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   -1,
                                   cameraSearchStr[NEXT_STR],
                                   false,
                                   CAM_SRCH_NEXT_BUTTON);

    if ( IS_VALID_OBJ(nextButton))
    {
        m_elementList[CAM_SRCH_NEXT_BUTTON] = nextButton;

        connect (nextButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (nextButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    infoPage = new InfoPage (0, 0,
                             (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH)),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget());
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));


    processBar = new ProcessBar(m_backGround->x(), m_backGround->y(),
                                m_backGround->width(),
                                m_backGround->height(),
                                SCALE_WIDTH(0), this);
}

void AutoAddCameraList::sendCommand(SET_COMMAND_e cmdType, quint8 totalfeilds)
{
    processBar->loadProcessBar();

    QString payloadString = m_payloadLib->createDevCmdPayload(totalfeilds);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void AutoAddCameraList::handleInfoPageMessage (int index)
{
    Q_UNUSED(index);
}

void AutoAddCameraList::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    quint8 configCam = 0;
    quint8 maxPossibleCam = 0;

    processBar->unloadProcessBar();

    switch(param->cmdType)
    {
    case GET_CAM_INITIATED_LIST:
        updateList(param);
        break;

    case MAX_ADD_CAM:
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            m_payloadLib->parseDevCmdReply(true, param->payload);

            configCam = m_payloadLib->getCnfgArrayAtIndex(0).toUInt();

            applController->GetDeviceInfo(deviceName, devTableInfo);

            maxPossibleCam = (devTableInfo.ipCams - configCam);

            if(maxPossibleCam == 0)
            {
                infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_MAX_CAM_CONFIGED));
            }
            else
            {
                if(AddCamList.length () <= maxPossibleCam)
                {
                    for(quint8 index = 0, fieldIndex = 0; index < AddCamList.length(); index++)
                    {
                        m_payloadLib->setCnfgArrayAtIndex(((index * 3) + fieldIndex++),AddCamList.at(index));
                        m_payloadLib->setCnfgArrayAtIndex(((index * 3) + fieldIndex++),AddCamIpAddrList.at(index));
                        m_payloadLib->setCnfgArrayAtIndex(((index * 3) + fieldIndex), AddCamModelList.at(index));
                        fieldIndex = 0;
                    }

                    sendCommand(ADD_CAM_INITIATED, (AddCamList.length() * 3));
                }
                else
                {
                    infoPage->loadInfoPage (QString("%1").arg (configCam) + " " + Multilang("camera(s) are already configured Only") + " "
                                     + QString("%1").arg (devTableInfo.ipCams - configCam)
                                     + " " + Multilang("camera(s) can be configured"));
                }
            }
        }
        else
        {
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }
    }
        break;

    case ADD_CAM_INITIATED:
    case RJCT_CAM_INITIATED:

        AddCamList.clear();
        AddCamIpAddrList.clear();
        AddCamModelList.clear();

        sendCommand(GET_CAM_INITIATED_LIST);
        break;

    default:
        break;
    }
}

void AutoAddCameraList::clearAllList ()
{
    macAddressList.clear();
    camIpAddrList.clear();
    modelList.clear();
    camStatusList.clear();
}

void AutoAddCameraList::updateList(DevCommParam *param)
{
    quint8 index = 0, totalResult = 0;

    clearAllList();

    if(param->deviceStatus == CMD_SUCCESS)
    {
        m_payloadLib->parseDevCmdReply (true,param->payload);

        totalResult = (m_payloadLib->getTotalCmdFields () / MAX_MX_CMD_CAM_LIST_FIELDS);

        for(index = 0; index < totalResult; index++)
        {
            macAddressList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_CAM_LIST_MAC_ADDRESS +
                                                                  (index * MAX_MX_CMD_CAM_LIST_FIELDS)).toString ());
            camIpAddrList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_CAM_LIST_IP_ADDRESS +
                                                               (index * MAX_MX_CMD_CAM_LIST_FIELDS)).toString ());
            modelList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_CAM_LIST_MODEL_NAME +
                                                              (index * MAX_MX_CMD_CAM_LIST_FIELDS)).toString ());
            camStatusList.append ("0");
        }
    }

    maxSearchListCount = macAddressList.length ();

    maximumPages = (maxSearchListCount % MAX_RECORD_DATA_AUTO_ADD == 0 )?
                (maxSearchListCount / MAX_RECORD_DATA_AUTO_ADD) :
                ((maxSearchListCount / MAX_RECORD_DATA_AUTO_ADD) + 1);

    showCameraSearchList ();
}

void AutoAddCameraList::showCameraSearchList()
{
    quint8 recordOnPage = 0;
    quint8 eleIndex = 0;

    selectAllCam->setVisible (true);
    selectAllCam->changeState(OFF_STATE);

    maximumPages = (maxSearchListCount % MAX_RECORD_DATA_AUTO_ADD == 0 )?
                (maxSearchListCount / MAX_RECORD_DATA_AUTO_ADD) :
                ((maxSearchListCount / MAX_RECORD_DATA_AUTO_ADD) + 1);

    if(maxSearchListCount < (MAX_RECORD_DATA_AUTO_ADD*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA_AUTO_ADD*(currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA_AUTO_ADD;
    }

    if(((recordOnPage == 0) && (maxSearchListCount != 0)) ||
            (maximumPages <= currentPageNo))
    {
        currentPageNo = 0;
        if(maxSearchListCount < (MAX_RECORD_DATA_AUTO_ADD*(currentPageNo + 1)))
        {
            recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA_AUTO_ADD*(currentPageNo)) );
        }
        else
        {
            recordOnPage = MAX_RECORD_DATA_AUTO_ADD;
        }
    }

    if(recordOnPage != 0)
    {
        updateNavigationControlStatus();
    }
    else
    {
        previousButton->setIsEnabled (false);
        nextButton->setIsEnabled (false);
        currentPageNo = 0;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        eleIndex = ((index + (currentPageNo*MAX_RECORD_DATA_AUTO_ADD)));

        selectCam[index]->setVisible (true);
        selectCam[index]->changeState(OFF_STATE);

        macAddressStr[index]->changeText (macAddressList.at (eleIndex));
        macAddressStr[index]->update ();

        ipAddrStr[index]->changeText (camIpAddrList.at (eleIndex));
        ipAddrStr[index]->update ();

        modelStr[index]->changeText (modelList.at (eleIndex));
        modelStr[index]->update ();

        statusStr[index]->changeText ((((camStatusList.at (eleIndex)).toInt()) == 0) ? "Pending" : "");
        statusStr[index]->update ();
    }

    if(recordOnPage != MAX_RECORD_DATA_AUTO_ADD)
        clearSerachDisplayList(recordOnPage);
}

void AutoAddCameraList::clearSerachDisplayList (quint8 recordOnPage)
{
    for(quint8 index = recordOnPage; index < MAX_RECORD_DATA_AUTO_ADD; index++)
    {
        macAddressStr[index]->changeText ("");
        ipAddrStr[index]->changeText ("");
        modelStr[index]->changeText ("");
        statusStr[index]->changeText ("");
        selectCam[index]->setVisible (false);
        selectCam[index]->setIsEnabled (false);
    }
}

void AutoAddCameraList::updateNavigationControlStatus ()
{
    previousButton->setIsEnabled ((currentPageNo != 0 ? true : false ));

    if( currentPageNo < (maximumPages - 1 ) )
    {
        nextButton->setIsEnabled (true);
        if((m_currentElement != CAM_SRCH_PREVIOUS_BUTTON) || (currentPageNo == 0))
        {
            m_currentElement = CAM_SRCH_NEXT_BUTTON;
        }
    }
    else if( currentPageNo == (maximumPages - 1 ))
    {
        nextButton->setIsEnabled (false);
        m_currentElement = CAM_SRCH_PREVIOUS_BUTTON;
    }

    m_elementList[m_currentElement]->forceActiveFocus ();
}

void AutoAddCameraList::fillAddCamList ()
{
    QString macAddr = "";
    quint8 recordOnPage;

    if(maxSearchListCount < (MAX_RECORD_DATA_AUTO_ADD*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA_AUTO_ADD*(currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA_AUTO_ADD;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        macAddr = "";

        if((selectCam[index]->getIsEnabled ()) &&
                (selectCam[index]->getCurrentState () == ON_STATE))
        {
            quint8 eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA_AUTO_ADD)));

            macAddr = macAddressList.at(eleIndex);

            if(!AddCamList.contains (macAddr))
            {
                AddCamList.append (macAddr);
                AddCamIpAddrList.append(camIpAddrList.at(eleIndex));
                AddCamModelList.append(modelList.at(eleIndex));
            }
        }
    }

    if(AddCamList.length() > macAddressList.length())
    {
        quint8 temp = 0;

        while (AddCamList.length() > macAddressList.length())
        {
            if(macAddressList.contains(AddCamList.at(temp)) == false)
            {
                AddCamList.removeAt(temp);
            }
            else
            {
                temp++;
            }
        }
    }
}

void AutoAddCameraList::slotButtonClick (int indexInPage)
{
    switch(indexInPage)
    {
    case CAM_SRCH_ADD_BUTTON:

        fillAddCamList ();

        if(AddCamList.empty ())
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(AUTO_ADD_CAM_NO_CAM_SEL));
        }
        else
        {
            sendCommand (MAX_ADD_CAM);
        }
        break;

    case CAM_SRCH_REJECT_BUTTON:

        fillAddCamList ();

        if(AddCamList.empty ())
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(AUTO_ADD_CAM_NO_CAM_SEL));
        }
        else
        {
            for(quint8 index = 0, fieldIndex = 0; index < AddCamList.length(); index++)
            {
                m_payloadLib->setCnfgArrayAtIndex(((index * 3) + fieldIndex++),AddCamList.at(index));
                m_payloadLib->setCnfgArrayAtIndex(((index * 3) + fieldIndex++),AddCamIpAddrList.at(index));
                m_payloadLib->setCnfgArrayAtIndex(((index * 3) + fieldIndex), AddCamModelList.at(index));
                fieldIndex = 0;
            }

            sendCommand(RJCT_CAM_INITIATED, (AddCamList.length() * 3));
        }
        break;

    case CAM_SRCH_PREVIOUS_BUTTON:
    {
        fillAddCamList();
        if(currentPageNo > 0)
        {
            currentPageNo --;
        }
        showCameraSearchList ();
        updateSelCamState();
    }
        break;

    case CAM_SRCH_NEXT_BUTTON:
    {
        fillAddCamList ();
        if (currentPageNo != (maximumPages - 1))
        {
            currentPageNo ++;
        }
        showCameraSearchList ();
        updateSelCamState();
    }
        break;

    default:
        emit sigObjectDelete (false);
        break;
    }
}

void AutoAddCameraList::slotOptionSelectionButton (OPTION_STATE_TYPE_e state, int indexInPage)
{
    MX_CAM_STATUS_e camState = MX_CAM_UNIDENTIFY;
    quint8 recordOnPage = 0;
    quint8 eleIndex = 0;
    quint8 numberOfDisable = 0;
    quint8 numberOfOnState = 0;
    QString macAddr;

    if(maxSearchListCount < (MAX_RECORD_DATA_AUTO_ADD*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA_AUTO_ADD*(currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA_AUTO_ADD;
    }

    if(indexInPage == CAM_SRCH_SEL_ALL_CAM)
    {
        if(state == ON_STATE)
        {
            updateSelCamState (true);
        }
        else
        {
            for(quint8 index = 0; index < recordOnPage; index++)
            {
                selectCam[index]->changeState (OFF_STATE);

                eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA_AUTO_ADD)));

                macAddr = macAddressList.at(eleIndex);
                AddCamList.removeOne (macAddr);
            }
        }
    }
    else
    {
        for(quint8 index = 0; index < recordOnPage; index++)
        {
            eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA_AUTO_ADD)));


            camState =(MX_CAM_STATUS_e)camStatusList.at(eleIndex).toUInt ();

            if(camState == MX_CAM_ADDED)
            {
                numberOfDisable++;
            }
            else
            {
                if(selectCam[index]->getCurrentState () == ON_STATE)
                {
                    numberOfOnState++;
                }
            }
        }

        if(((recordOnPage - numberOfOnState) == numberOfDisable) &&
                (numberOfDisable != recordOnPage))
        {
            selectAllCam->changeState (ON_STATE);
        }
        else
        {
            selectAllCam->changeState (OFF_STATE);
        }

        if ( state == OFF_STATE)
        {
            quint8 index = (indexInPage - CAM_SRCH_SEL_CAM);
            macAddr = macAddressStr[index]->getText ();

            if(AddCamList.contains (macAddr))
            {
                AddCamList.removeOne (macAddr);
            }
        }
    }
}

void AutoAddCameraList::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect ( QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                                   SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect ( QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                                   SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void AutoAddCameraList::updateSelCamState (bool isStateOn)
{
    bool autoAddStateOn = false;
    bool selEnable = false;
    quint8 eleIndex = 0;
    quint8 recordOnPage = 0;
    quint8 numberOfDisable = 0;
    quint8 numberOfOnState = 0;
    MX_CAM_STATUS_e camState;
    OPTION_STATE_TYPE_e selState;

    if(!camStatusList.empty ())
    {
        if(maxSearchListCount < (MAX_RECORD_DATA_AUTO_ADD*(currentPageNo + 1)))
        {
            recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA_AUTO_ADD*(currentPageNo)) );
        }
        else
        {
            recordOnPage = MAX_RECORD_DATA_AUTO_ADD;
        }

        for(quint8 index = 0; index < recordOnPage; index++)
        {
            autoAddStateOn = false;
            selState = OFF_STATE;
            selEnable = false;

            eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA_AUTO_ADD)));

            camState = (MX_CAM_STATUS_e)camStatusList.at(eleIndex).toUInt ();

            if((camState == MX_CAM_ADDED))
            {
                numberOfDisable++;
            }
            else
            {
                selEnable = true;

                if(!AddCamList.isEmpty ())
                {
                    if(AddCamList.contains (macAddressStr[index]->getText ()))
                    {
                        autoAddStateOn = true;
                    }
                }

                if((isStateOn) || (autoAddStateOn))
                {
                    selState = ON_STATE;
                    numberOfOnState++;
                }
            }

            selectCam[index]->changeState (selState);
            selectCam[index]->setIsEnabled (selEnable);
        }

        if(((recordOnPage - numberOfOnState) == numberOfDisable) &&
                (numberOfDisable != recordOnPage))
        {
            selectAllCam->changeState (ON_STATE);
        }
        else
        {
            selectAllCam->changeState (OFF_STATE);
        }

        selectAllCam->setIsEnabled ((numberOfDisable == recordOnPage) ?
                                        false : true);
    }
}

void AutoAddCameraList::slotInfoPageBtnclick(int index)
{
    m_elementList[m_currentElement]->forceActiveFocus ();
    Q_UNUSED(index);
}

void AutoAddCameraList::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void AutoAddCameraList::navigationKeyPressed(QKeyEvent *event)
{
   event->accept();
}

void AutoAddCameraList::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = CAM_SRCH_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void AutoAddCameraList::takeLeftKeyAction()
{
    bool tStatus = true;
    do
    {
        if(m_currentElement == 0)
        {
            m_currentElement = (MAX_CAM_SRCH_ELEMENT);
        }
        if(m_currentElement)
        {
            m_currentElement = (m_currentElement - 1);
        }
        else
        {
              tStatus = false;
              break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           ||(!m_elementList[m_currentElement]->getIsEnabled()));

    if(tStatus == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void AutoAddCameraList::takeRightKeyAction()
{
    bool tStatus = true;
    do
    {
        if(m_currentElement == (MAX_CAM_SRCH_ELEMENT - 1))
        {
            m_currentElement = -1;
        }
        if(m_currentElement != (MAX_CAM_SRCH_ELEMENT - 1))
        {
            m_currentElement = (m_currentElement + 1);
        }
        else
        {
              tStatus = false;
              break;
        }
    }while((m_elementList[m_currentElement] == NULL)
           ||(!m_elementList[m_currentElement]->getIsEnabled()));

    if(tStatus == true)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void AutoAddCameraList::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void AutoAddCameraList::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void AutoAddCameraList::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
