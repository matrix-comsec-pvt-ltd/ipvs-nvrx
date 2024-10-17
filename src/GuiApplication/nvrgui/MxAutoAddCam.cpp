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
//   Project      : SATATYA DEVICES
//   Owner        : Kaushal Patel
//   File         : MxAutoAddCam.cpp
//   Description  : This file is used to give pop up of auto add cam on boot.
/////////////////////////////////////////////////////////////////////////////

#include "MxAutoAddCam.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include "MxCommandFields.h"
#include "ValidationMessage.h"
#include "ConfigPages/BasicSettings/AutoConfigureCamera.h"

#define AUTO_ADD_PAGE_WIDTH         SCALE_WIDTH(450)
#define AUTO_ADD_PAGE_HEIGHT        SCALE_HEIGHT(250)

#define SEARCH_TIME                 300000  // 5min = 1000 * 60 * 5 = 300000
#define RETRY_TIME                  10000   // 10sec = 1000 * 10 = 10000
#define RETRY_COUNT                 6       // 10sec*6 = 60sec(1Min).we are retrying auto search for 1Min.
#define CNFG_TO_INDEX               1

#define AUTO_ADD_CAM_DISABLE_IMG_PATH ":/Images_Nvrx/InfoPageImages/AutoAddCam.png"

typedef enum
{
    UP_ARROW,
    DOWN_ARROW,
    MAX_SOURCE_TYPE
}SOURCE_TYPE_e;

typedef enum
{
    MX_LABLE_CONFORMATION,
    MX_LABLE_CAM_FOUND,
    MX_LABLE_CONFIG_CAM,
    MX_LABLE_CAM_CREDENTIAL,
    MAX_MX_LABLE_AUTO_ADD_CAM
}MX_LABLE_AUTO_ADD_CAM_e;

typedef enum
{
    MX_USER_NAME,
    MX_PASSWORD,
    MAX_AUTO_ADD_CAM_ELEMETS
}MX_AUTO_ADD_CAM_ELELIST_e;

static QString changeCameraCrdntialImagePath[MAX_SOURCE_TYPE] =
{
    ":/Images_Nvrx/UpArrow/",
    ":/Images_Nvrx/DownArrow/"
};

static const QString autoAddCamLableStr[MAX_MX_LABLE_AUTO_ADD_CAM] =
{
    "Would you like to configure all?",
    "new camera(s) found.",
    "Customize camera configuration...",
    "Change Camera Credentials"
};

static const QString autoAddCamElementString[MAX_AUTO_ADD_CAM_ELEMETS] =
{
    "Username",
    "Password"
};

MxAutoAddCam::MxAutoAddCam(QWidget *parent):BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - AUTO_ADD_PAGE_WIDTH)/2) - 2),
                                                       (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - AUTO_ADD_PAGE_HEIGHT)/2)),
                                                       AUTO_ADD_PAGE_WIDTH  + SCALE_WIDTH(5),
                                                       AUTO_ADD_PAGE_HEIGHT + SCALE_HEIGHT(40),
                                                       BACKGROUND_TYPE_4,
                                                       MAX_TOOLBAR_BUTTON,
                                                       parent,true), m_isShowElement(0)
{
    setObjectName("MX_AUTO_ADD");
    INIT_OBJ(m_disabledImage);
    m_applController = ApplController::getInstance();
    createDefaultComponent();
    showHideElements(false);
    getConfig();
    m_currElement = AUTO_ADD_YES_BTN;
    m_elementList[m_currElement]->forceActiveFocus();
    unloadAutoAddCam ();
}

MxAutoAddCam::~MxAutoAddCam()
{
    if(IS_VALID_OBJ(m_noButton))
    {
        disconnect (m_noButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotCnfgBtnClicked(int)));
        disconnect (m_noButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_noButton);
    }

    if(IS_VALID_OBJ(m_yesButton))
    {
        disconnect (m_yesButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotCnfgBtnClicked(int)));

        disconnect (m_yesButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_yesButton);
    }

    DELETE_OBJ (m_conformationStr);
    DELETE_OBJ (m_camNoStr);
    DELETE_OBJ (m_devResponseLabel);

    if(IS_VALID_OBJ(m_customizeConfigLable))
    {
        disconnect(m_customizeConfigLable,
                   SIGNAL(sigTextClick(int)),
                   this,
                   SLOT(slotTextClicked(int)));
        disconnect(m_customizeConfigLable,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ (m_customizeConfigLable);
    }

    if(IS_VALID_OBJ(m_mainCloseButton))
    {
        disconnect(m_mainCloseButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClicked(int)));
        disconnect(m_mainCloseButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
    }

    if(IS_VALID_OBJ(m_getAqrListTimer))
    {
        if(m_getAqrListTimer->isActive())
        {
            m_getAqrListTimer->stop();
        }

        disconnect (m_getAqrListTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotAqrListTimeOut()));
        DELETE_OBJ (m_getAqrListTimer);
    }

    if(IS_VALID_OBJ(m_searchDurationTimer))
    {
        if(m_searchDurationTimer->isActive())
        {
            m_searchDurationTimer->stop();
        }

        disconnect (m_searchDurationTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotSearchDurationTimeOut()));
        DELETE_OBJ (m_searchDurationTimer);
    }

    DELETE_OBJ (m_payloadLib);
    DELETE_OBJ(m_inVisibleWidget);

    if(IS_VALID_OBJ(m_saveButton))
    {
        disconnect (m_saveButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_saveButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_saveButton);

    }

    if(IS_VALID_OBJ(m_passwordTextBox))
    {
       disconnect (m_passwordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
       DELETE_OBJ(m_passwordTextBox);
    }
    DELETE_OBJ(m_passwordTextBoxParam);

    if(IS_VALID_OBJ(m_userNameTextBox))
    {
        disconnect (m_userNameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_userNameTextBox);
    }
    DELETE_OBJ(m_userNameTextBoxParam);

    if(IS_VALID_OBJ(m_changeCameraCrdntialTile))
    {
        disconnect(m_changeCameraCrdntialTile,
            SIGNAL(sigTileWithTextClick(int)),
            this,
            SLOT(slotTileWithTextClick(int)));
        DELETE_OBJ(m_changeCameraCrdntialTile);
    }

    if(IS_VALID_OBJ(m_changeCameraCrdntialImage))
    {
        disconnect(m_changeCameraCrdntialImage,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotImageClicked(int)));
        DELETE_OBJ(m_changeCameraCrdntialImage);
    }

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect (m_infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
        delete m_infoPage;
    }

    DELETE_OBJ(m_insideTile);
    DELETE_OBJ(m_complTile);
}

void MxAutoAddCam::createDefaultComponent ()
{
    m_isAutoSearchStarted = false;
    m_isInfoPageLoaded = false;
    m_isCamSearchPageLoaded = false;
    m_isCancelSend = false;
    m_currentButton = MAX_AUTO_ADD_ELEMENTS;
    m_autoSrchRetryCnt = 0;
    m_payloadLib = new PayloadLib();
    m_isShowCameraCrdntialSettings = false;
    m_isYesbtnclick = false;
    m_retainIpAddrInAutoConfig = true;
    m_autoConfigIpAddrFamily = 0;
    this->setVisible(false);

    m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
    m_inVisibleWidget->setVisible(false);

    m_devResponseLabel = new TextLabel((this->width()/2 - SCALE_WIDTH(10)),
                                       SCALE_HEIGHT(22),
                                       SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                       "",
                                       this,
                                       POPUP_STATUS_LINE_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_START_Y);
    m_devResponseLabel->setVisible(false);

    m_insideTile = new BgTile((this->width()/2)-((AUTO_ADD_PAGE_WIDTH-SCALE_WIDTH(70))/2),
                              (this->height()/2)-((AUTO_ADD_PAGE_HEIGHT-SCALE_HEIGHT(50))/2 ),
                              AUTO_ADD_PAGE_WIDTH-SCALE_WIDTH(70),
                              AUTO_ADD_PAGE_HEIGHT-SCALE_HEIGHT(100),
                              COMMON_LAYER,
                              this);
    m_changeCameraCrdntialTile = new TileWithText(m_insideTile->x(),
                                                  (m_insideTile->y() + m_insideTile->height() + SCALE_HEIGHT(40)),
                                                  AUTO_ADD_PAGE_WIDTH-SCALE_WIDTH(70),
                                                  AUTO_ADD_PAGE_HEIGHT - SCALE_HEIGHT(210),
                                                  SCALE_WIDTH(10),
                                                  autoAddCamLableStr[MX_LABLE_CAM_CREDENTIAL],
                                                  COMMON_LAYER,
                                                  this,
                                                  true,
                                                  0,
                                                  "#c8c8c8",
                                                  MX_LABLE_CAM_CREDENTIAL);
    connect(m_changeCameraCrdntialTile,
            SIGNAL(sigTileWithTextClick(int)),
            this,
            SLOT(slotTileWithTextClick(int)));

    m_complTile = new BgTile(m_changeCameraCrdntialTile->x(),
                             m_changeCameraCrdntialTile->y() + m_changeCameraCrdntialTile->height(),
                             AUTO_ADD_PAGE_WIDTH-SCALE_WIDTH(70),
                             SCALE_HEIGHT(140),
                             COMMON_LAYER,
                             this);

    m_changeCameraCrdntialImage = new Image(m_changeCameraCrdntialTile->x()+m_changeCameraCrdntialTile->width() - SCALE_WIDTH(10),
                                            m_changeCameraCrdntialTile->y()+m_changeCameraCrdntialTile->height()/2,
                                            changeCameraCrdntialImagePath[UP_ARROW],
                                            this,
                                            END_X_CENTER_Y,
                                            MX_AUTO_ADD_CAM_CAMERA_CRDENTIAL_IMAGE,
                                            true,
                                            false,
                                            true,
                                            false);
    m_elementList[MX_AUTO_ADD_CAM_CAMERA_CRDENTIAL_IMAGE] = m_changeCameraCrdntialImage;
    if(m_isShowCameraCrdntialSettings == false)
    {
        m_changeCameraCrdntialImage->updateImageSource(changeCameraCrdntialImagePath[DOWN_ARROW],true);
    }

    connect(m_changeCameraCrdntialImage,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotImageClicked(int)));

    m_userNameTextBoxParam = new TextboxParam();
    m_userNameTextBoxParam->labelStr = autoAddCamElementString[MX_USER_NAME];
    m_userNameTextBoxParam->isCentre = true;
    m_userNameTextBoxParam->isTotalBlankStrAllow = true;
    m_userNameTextBoxParam->maxChar = 24;
    m_userNameTextBoxParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_userNameTextBox = new TextBox(m_changeCameraCrdntialTile->x() + SCALE_WIDTH(20),
                                    m_changeCameraCrdntialTile->y() + SCALE_HEIGHT(50),
                                    BGTILE_SMALL_SIZE_WIDTH,
                                    BGTILE_HEIGHT - SCALE_HEIGHT(5),
                                    AUTO_ADD_USERNAME,
                                    TEXTBOX_MEDIAM,
                                    this,
                                    m_userNameTextBoxParam,
                                    NO_LAYER);
    m_elementList[AUTO_ADD_USERNAME] = m_userNameTextBox;
    m_userNameTextBox->setVisible(false);
    m_userNameTextBox->setIsEnabled(false);

    connect(m_userNameTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_passwordTextBoxParam = new TextboxParam();
    m_passwordTextBoxParam->labelStr = autoAddCamElementString[MX_PASSWORD];
    m_passwordTextBoxParam->suffixStr = "(Max 20 chars)";
    m_passwordTextBoxParam->isCentre =  true;
    m_passwordTextBoxParam->isTotalBlankStrAllow = true;
    m_passwordTextBoxParam->maxChar = 20;
    m_passwordTextBoxParam->minChar = 4;
    m_passwordTextBoxParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_passwordTextBox = new PasswordTextbox(m_userNameTextBox->x() + SCALE_WIDTH(5),
                                            m_userNameTextBox->y() + m_userNameTextBox->height() + SCALE_HEIGHT(5),
                                            BGTILE_SMALL_SIZE_WIDTH,
                                            BGTILE_HEIGHT - SCALE_HEIGHT(5),
                                            AUTO_ADD_PASSWORD,
                                            TEXTBOX_MEDIAM,
                                            this,
                                            m_passwordTextBoxParam,
                                            NO_LAYER);
    m_elementList[AUTO_ADD_PASSWORD] = m_passwordTextBox;
    m_passwordTextBox->setIsEnabled(false);

    connect(m_passwordTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_customizeConfigLable = new TextLabel((this->width() - ((this->width()-m_insideTile->width())/2)),
                                           (this->height()-SCALE_HEIGHT(70)),
                                           SCALE_FONT(SUFFIX_FONT_SIZE),
                                           autoAddCamLableStr[MX_LABLE_CONFIG_CAM],
                                           this,
                                           HIGHLITED_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_END_X_END_Y,
                                           0,
                                           true,
                                           0,
                                           AUTO_ADD_CHANGE_CONFIG_LABEL);
    m_elementList[AUTO_ADD_CHANGE_CONFIG_LABEL] = (NavigationControl*)(m_customizeConfigLable);
    connect(m_customizeConfigLable,
            SIGNAL(sigTextClick(int)),
            this,
            SLOT(slotTextClicked(int)));
    connect(m_customizeConfigLable,
            SIGNAL(sigMouseHover(int,bool)),
            this,
            SLOT(slotTextLableHover(int,bool)));

    m_infoPage = new InfoPage (0,0,
                               AUTO_ADD_PAGE_WIDTH + SCALE_WIDTH(5) ,
                               AUTO_ADD_PAGE_HEIGHT,
                               MAX_INFO_PAGE_TYPE,
                               this);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageBtnclick(int)));

    m_saveButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (this->width()/2),
                                (this->height()/2 + SCALE_HEIGHT(245)) ,
                                AUTO_ADD_CONFORMATION_BTN_SAVE,
                                this,
                                AUTO_ADD_SAVE_BTN);
    m_elementList[AUTO_ADD_SAVE_BTN] = m_saveButton;
    connect(m_saveButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_saveButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    m_saveButton->setIsEnabled(false);

    m_noButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (m_insideTile->width () - SCALE_WIDTH(25)),
                                (m_insideTile->height () + SCALE_HEIGHT(15)),
                                AUTO_ADD_CONFORMATION_BTN_NO,
                                this,
                                AUTO_ADD_NO_BTN);
    m_elementList[AUTO_ADD_NO_BTN] = m_noButton;
    connect(m_noButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_noButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgBtnClicked(int)));

    m_yesButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 (m_insideTile->width () - SCALE_WIDTH(135)),
                                 (m_insideTile->height () + SCALE_HEIGHT(15)),
                                 AUTO_ADD_CONFORMATION_BTN_YES,
                                 this,
                                 AUTO_ADD_YES_BTN);
    m_elementList[AUTO_ADD_YES_BTN] = m_yesButton;
    connect(m_yesButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_yesButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgBtnClicked(int)));

    m_conformationStr = new TextLabel((m_insideTile->x() + m_insideTile->width()/2 - SCALE_WIDTH(20)),
                                      (m_insideTile->y() + m_insideTile->height()/2 - SCALE_HEIGHT(20)),
                                      NORMAL_FONT_SIZE,
                                      autoAddCamLableStr[MX_LABLE_CONFORMATION],
                                      this,
                                      NORMAL_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_CENTRE_X_CENTER_Y);

    m_camNoStr = new TextLabel((m_conformationStr->x()),
                               (m_conformationStr->y() - SCALE_HEIGHT(20)),
                               NORMAL_FONT_SIZE,
                               INT_TO_QSTRING(0) + " " + Multilang(autoAddCamLableStr[MX_LABLE_CAM_FOUND].toUtf8().constData()),
                               this,
                               NORMAL_FONT_COLOR,
                               NORMAL_FONT_FAMILY,
                               ALIGN_START_X_START_Y);

    if(IS_VALID_OBJ(m_mainCloseButton))
    {
        m_mainCloseButton->setIndexInPage(AUTO_ADD_CLS_BTN);
        m_elementList[AUTO_ADD_CLS_BTN] = m_mainCloseButton;
        connect(m_mainCloseButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCloseButtonClicked(int)));
        connect(m_mainCloseButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    }

    m_searchDurationTimer = new QTimer(this);
    connect(m_searchDurationTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotSearchDurationTimeOut()));
    m_searchDurationTimer->setInterval(SEARCH_TIME);
    m_searchDurationTimer->setSingleShot(true);
    m_searchDurationTimer->start();

    /* Start onboot camera search */
    m_payloadLib->setCnfgArrayAtIndex(0, 1);
    sendCommand(AUTO_SEARCH, 1);

    m_getAqrListTimer = new QTimer();
    connect(m_getAqrListTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotAqrListTimeOut()));
    m_getAqrListTimer->setInterval(RETRY_TIME);
    m_getAqrListTimer->start();
}

void MxAutoAddCam::changeText(quint8 camNo)
{
    if(IS_VALID_OBJ(m_camNoStr))
    {
        m_camNoStr->changeText(INT_TO_QSTRING(camNo) + " " + Multilang(autoAddCamLableStr[MX_LABLE_CAM_FOUND].toUtf8().constData()));
        m_camNoStr->repaint();
    }
}

void MxAutoAddCam::loadAutoAddCam(bool isShowElement)
{
    if(IS_VALID_OBJ(m_inVisibleWidget))
    {
        m_inVisibleWidget->raise();
        m_inVisibleWidget->setVisible(true);
    }

    showHideElements(isShowElement);
    this->raise();
    this->setVisible(true);
    m_currElement = AUTO_ADD_YES_BTN;
    m_elementList[m_currElement]->forceActiveFocus();
    emit sigAutoAddCamPopUpAction (MX_AUTO_ADD_CAM_PAUSE_PLAYBACK);
}

void MxAutoAddCam::unloadAutoAddCam()
{
    if(IS_VALID_OBJ(m_devResponseLabel))
    {
        m_devResponseLabel->setVisible(false);
    }

    if(IS_VALID_OBJ(m_inVisibleWidget))
    {
        m_inVisibleWidget->setVisible(false);
    }

    this->setVisible(false);
}

void MxAutoAddCam::loadAutoAddCamAfterLogin()
{
    USRS_GROUP_e currentUserType = MAX_USER_GROUP;
    m_applController->GetUserGroupType(LOCAL_DEVICE_NAME, currentUserType);
    if(m_isYesbtnclick)
    {
        m_isYesbtnclick = false;
        if(currentUserType == ADMIN)
        {
            sendAutoAddCmd();
        }
    }
    else
    {
        if(currentUserType != ADMIN)
        {
            getConfig();
        }
        else
        {
            loadAutoAddCam(true);
        }
    }
}

void MxAutoAddCam::sendCommand(SET_COMMAND_e cmdType, quint32 totalfeilds)
{
    if(cmdType == CNCL_AUTO_SEARCH)
    {
        if(m_isCancelSend == false)
        {
            m_isCancelSend = true;
            isOnbootAuoCamSearchRunning = false;
        }
        else
        {
            return;
        }
    }

    QString payloadString = m_payloadLib->createDevCmdPayload(totalfeilds);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;

    if(cmdType == GET_ACQ_LIST)
    {
        param->windowId = WIN_ID_MXAUTO_ADD_CAM;
    }

    m_applController = ApplController::getInstance();
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MxAutoAddCam::processDeviceResponse (DevCommParam *param, QString)
{
    if(param->msgType == MSG_SET_CMD)
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            switch(param->cmdType)
            {
                case GET_ACQ_LIST:
                {
                    if(m_isCamSearchPageLoaded)
                    {
                        break;
                    }

                    updateList(param);
                    if(m_isShowCameraCrdntialSettings == false)
                    {
                        getConfig();
                    }
                }
                break;

                case AUTO_SEARCH:
                {
                    m_isAutoSearchStarted = true;
                    m_autoSrchRetryCnt = 0;
                    sendCommand(GET_ACQ_LIST);
                }
                break;

                case AUTO_CONFIGURE_CAMERA:
                {
                    unloadAutoAddCam();
                    if (m_isCancelSend == true)
                    {
                        emit sigCloseAlert ();
                    }
                }
                break;

                case CNCL_AUTO_SEARCH:
                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        else
        {
            if(param->cmdType == AUTO_SEARCH)
            {
                m_isAutoSearchStarted = false;
                m_autoSrchRetryCnt++;
            }
            else if(IS_VALID_OBJ(m_devResponseLabel))
            {
                m_devResponseLabel->changeText(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                m_devResponseLabel->setVisible(true);
            }
        }
    }
    else if(param->msgType == MSG_SET_CFG)
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
    }
    else if(param->msgType == MSG_GET_CFG)
    {
        m_payloadLib->parsePayload(param->msgType, param->payload);
        if(m_payloadLib->getcnfgTableIndex() == GENERAL_TABLE_INDEX)
        {
            m_retainIpAddrInAutoConfig = m_payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG + FIELD_AUTO_CONFIG_IP_RETAIN + 1).toInt() ? true : false;
            m_autoConfigIpAddrFamily = QHostAddress(m_payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG + FIELD_AUTO_CONFIG_START_IP + 1).toString()).protocol();
            m_userNameTextBox->setInputText(m_payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG_USERNAME).toString());
            m_passwordTextBox->setInputText(m_payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG_PASSWORD).toString());
        }
    }

    if((m_currentButton == AUTO_ADD_NO_BTN) || ((m_searchDurationTimer->isActive() == false) && (this->isVisible() == false)))
    {
        emit sigCloseAlert();
    }
}

void MxAutoAddCam::clearAllList ()
{
    m_searchList.clear ();
    m_camStatusList.clear ();
    m_camNumList.clear ();
    m_camNameList.clear ();
    m_ipv4AddressList.clear ();
    m_ipv6AddressList.clear();
    m_brandList.clear ();
    m_modelList.clear ();
    m_onvifSupportStatusList.clear ();
    m_onvifPortList.clear ();
    m_httpPortList.clear ();
}

void MxAutoAddCam::updateList(DevCommParam *param)
{
    quint8 index = 0, totalResult = 0;

    if(IS_VALID_OBJ(m_devResponseLabel))
    {
        m_devResponseLabel->setVisible(false);
    }

    m_payloadLib->parseDevCmdReply (true,param->payload);
	m_payload = param->payload;
    totalResult = (m_payloadLib->getTotalCmdFields () / MAX_MX_CMD_CAM_SEARCH_FIELDS);

    clearAllList();

    for(index = 0; index < totalResult; index++)
    {
        quint8  camState = m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAMERA_STATUS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toUInt() ;

        if((camState == MX_CAM_IDENTIFY) ||
                ((camState == MX_CAM_UNIDENTIFY)
                 && (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_SUPPORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toUInt() != 0)))
        {
            m_camStatusList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAMERA_STATUS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_camNumList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NUMBER + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_ipv4AddressList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV4_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_ipv6AddressList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV6_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_httpPortList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_HTTP_PORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_brandList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_modelList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_MODEL_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_onvifSupportStatusList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_SUPPORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            m_onvifPortList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_PORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());

            QString camName = m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ();
            if(camName == "")
            {
                if(m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString () != "")
                {
                    m_camNameList.append (m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
                }
                else
                {
                    m_camNameList.append(m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV4_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
                }
            }
            else
            {
                m_camNameList.append(m_payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            }
        }
    }

    if((m_ipv4AddressList.length() > 0) && (m_isInfoPageLoaded == false) && (m_isCamSearchPageLoaded == false))
    {
        changeText(m_ipv4AddressList.length());
        loadAutoAddCam(m_isShowCameraCrdntialSettings);
    }

    if (m_ipv4AddressList.length() == 0)
    {
        slotCloseButtonClicked(0);
    }
}

void MxAutoAddCam::setIsInfoPageLoaded(bool state)
{
    m_isInfoPageLoaded = state;
}

void MxAutoAddCam::setIsCamSearchPageLoaded(bool state)
{
    m_isCamSearchPageLoaded = state;
}

QString MxAutoAddCam::getPayloadList()
{
    QString str = "";

    if (m_isCancelSend == true)
    {
        str = m_payload;
    }

    return str;
}

void MxAutoAddCam::sendAutoAddCmd()
{
    quint8 totalCam = m_ipv4AddressList.length();
    if (totalCam == 0)
    {
        return;
    }

    for(quint8 index = 0; index < totalCam; index++)
    {
        QString tempCamName = (m_ipv4AddressList.at(index).isEmpty() == false) ? m_ipv4AddressList.at(index) : m_ipv6AddressList.at(index);

        if (tempCamName.size() > MAX_CAMERA_NAME_LENGTH)
        {
            tempCamName.truncate(MAX_CAMERA_NAME_LENGTH);
        }

        //AUTO_CONFIGURE_CAMERA
        m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_CAM_STATUS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), m_camStatusList.at(index));
        m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_CAM_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), tempCamName);
        if ((m_retainIpAddrInAutoConfig == true) || (m_autoConfigIpAddrFamily == QAbstractSocket::IPv4Protocol))
        {
            m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)),
                                              ((m_ipv4AddressList.at(index).isEmpty() == false) ? m_ipv4AddressList.at(index) : m_ipv6AddressList.at(index)));
        }
        else
        {
            m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)),
                                              ((m_ipv6AddressList.at(index).isEmpty() == false) ? m_ipv6AddressList.at(index) : m_ipv4AddressList.at(index)));
        }
        m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_HTTP_PORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), m_httpPortList.at(index));
        m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_BRAND_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), m_brandList.at(index));
        m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_MODEL_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), m_modelList.at(index));
        m_payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_ONVIF_SUPPORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), m_onvifSupportStatusList.at(index));
        m_payloadLib->setCnfgArrayAtIndex((AUTO_ADD_FEILDS_ONVIF_PORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), m_onvifPortList.at(index));
    }

    sendCommand(AUTO_CONFIGURE_CAMERA, (MAX_MX_CMD_AUTO_ADD_FIELDS*totalCam));
}

bool MxAutoAddCam::validationOnSaveBtn()
{
    if(m_userNameTextBoxParam->textStr == "" )
    {
        if(IS_VALID_OBJ(m_inVisibleWidget))
        {
            m_inVisibleWidget->setVisible(false);
        }

        /* PARASOFT: Memory Deallocated in slot InfoPageBtnclick */
        m_disabledImage = new Image(SCALE_WIDTH(2),
                                   SCALE_HEIGHT(2),
                                   AUTO_ADD_CAM_DISABLE_IMG_PATH,
                                   this,
                                   START_X_START_Y,
                                   MAX_MX_LABLE_AUTO_ADD_CAM,
                                   false,
                                   true,
                                   false,
                                   false);
        m_infoPage->raise();
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_USER_NAME_ERROR));
        return false;
    }
    else if(m_passwordTextBoxParam->textStr == "" )
    {
        if(IS_VALID_OBJ(m_inVisibleWidget))
        {
            m_inVisibleWidget->setVisible(false);
        }

        /* PARASOFT: Memory Deallocated in slot InfoPageBtnclick */
        m_disabledImage = new Image(SCALE_WIDTH(2),
                                   SCALE_HEIGHT(2),
                                   AUTO_ADD_CAM_DISABLE_IMG_PATH,
                                   this,
                                   START_X_START_Y,
                                   MAX_MX_LABLE_AUTO_ADD_CAM,
                                   false,
                                   true,
                                   false,
                                   false);
        m_infoPage->raise();
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_PASSWORD_ERROR));
        return false;
    }

    return true;
}

void MxAutoAddCam::raiseVirtualKeypad()
{
    m_userNameTextBox->raiseVirtualKeypad();
    m_passwordTextBox->raiseVirtualKeypad();
}

void MxAutoAddCam::slotUpdateCurrentElement(int index)
{
    m_currElement = index;
}

void MxAutoAddCam::slotCnfgBtnClicked(int index)
{
    if(index == AUTO_ADD_YES_BTN)
    {
        showHideElements(false);
        m_isYesbtnclick = true;
        unloadAutoAddCam();
        emit sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_LOGIN);
    }
    else if(index == AUTO_ADD_NO_BTN)
    {
        emit sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_TIME_OUT);
        m_currentButton = AUTO_ADD_NO_BTN;

        if(IS_VALID_OBJ(m_getAqrListTimer))
        {
            if(m_getAqrListTimer->isActive())
            {
                m_getAqrListTimer->stop();
            }
        }

        emit sigCloseAlert();
        sendCommand(CNCL_AUTO_SEARCH);
    }
}
void MxAutoAddCam::slotButtonClick(int index)
{
    if(index == AUTO_ADD_SAVE_BTN)
    {
        if(validationOnSaveBtn() == true )
        {
            saveConfig();
        }
    }
}

void MxAutoAddCam::slotInfoPageBtnclick(int)
{
    m_elementList[m_currElement]->forceActiveFocus ();
    DELETE_OBJ(m_disabledImage);
}

void MxAutoAddCam::slotCloseButtonClicked(int)
{
    unloadAutoAddCam();
    showHideElements(false);
    if (m_isCancelSend == true)
    {
        emit sigCloseAlert ();
    }
}

void MxAutoAddCam::slotSearchDurationTimeOut()
{
    emit sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_TIME_OUT);
    if(IS_VALID_OBJ(m_getAqrListTimer))
    {
        if(m_getAqrListTimer->isActive())
        {
            m_getAqrListTimer->stop();
        }
    }
    emit sigCloseAlert();
    sendCommand(CNCL_AUTO_SEARCH);
}

void MxAutoAddCam::slotAqrListTimeOut()
{
    if(m_isAutoSearchStarted)
    {
        sendCommand(GET_ACQ_LIST);
        return;
    }

    if(m_autoSrchRetryCnt <= RETRY_COUNT)
    {
        m_payloadLib->setCnfgArrayAtIndex (0,1);
        sendCommand(AUTO_SEARCH, 1);
        return;
    }

    emit sigCloseAlert();
}

void MxAutoAddCam::showHideElements(bool flag)
{
    if(flag == true)
    {
        m_changeCameraCrdntialImage->updateImageSource(changeCameraCrdntialImagePath[UP_ARROW],true);
        m_isShowCameraCrdntialSettings = flag;
        m_userNameTextBox->setVisible(true);
        m_userNameTextBox->setIsEnabled(true);
        m_passwordTextBox->setVisible(true);
        m_passwordTextBox->setIsEnabled(true);
        m_saveButton->setIsEnabled(true);
        this->resetGeometry(this->x(),
                            this->y(),
                            AUTO_ADD_PAGE_WIDTH + SCALE_WIDTH(5),
                            (AUTO_ADD_PAGE_HEIGHT + SCALE_HEIGHT(200)));
        m_complTile->setVisible(true);
    }
    else
    {
        m_changeCameraCrdntialImage->updateImageSource(changeCameraCrdntialImagePath[DOWN_ARROW],true);
        m_isShowCameraCrdntialSettings = flag;
        m_userNameTextBox->setVisible(false);
        m_userNameTextBox->setIsEnabled(false);
        m_passwordTextBox->setVisible(false);
        m_passwordTextBox->setIsEnabled(false);
        m_saveButton->setIsEnabled(false);
        this->resetGeometry(this->x(),
                            this->y(),
                            AUTO_ADD_PAGE_WIDTH + SCALE_WIDTH(5),
                            AUTO_ADD_PAGE_HEIGHT + SCALE_HEIGHT(40));
        m_complTile->setVisible(false);
    }
}

void MxAutoAddCam::slotTextClicked(int)
{
    showHideElements(false);
    unloadAutoAddCam();
    emit sigAutoAddCamPopUpAction (MX_AUTO_ADD_CAM_SEARCH_PAGE_LOAD);
}

void MxAutoAddCam::slotTextLableHover(int index, bool isHoverIn)
{
    if(index == AUTO_ADD_CHANGE_CONFIG_LABEL)
    {
        if(isHoverIn == true)
        {
            m_customizeConfigLable->changeColor(MOUSE_HOWER_COLOR);
            m_customizeConfigLable->repaint();
        }
        else
        {
            m_customizeConfigLable->changeColor(HIGHLITED_FONT_COLOR);
            m_customizeConfigLable->repaint();
        }
    }
}

void MxAutoAddCam::slotImageClicked(int indexInPage)
{
    if (indexInPage == MX_AUTO_ADD_CAM_CAMERA_CRDENTIAL_IMAGE)
    {
        showHideElements(m_isShowCameraCrdntialSettings ? false : true);
    }
}

void MxAutoAddCam::slotTileWithTextClick(int indexInPage)
{
    if (indexInPage == MX_LABLE_CAM_CREDENTIAL)
    {
        showHideElements(m_isShowCameraCrdntialSettings ? false : true);
    }
}

void MxAutoAddCam::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               CNFG_TO_INDEX,
                                                               CNFG_FRM_FIELD,
                                                               MAX_FIELD_NO,
                                                               0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    param->windowId = WIN_ID_MXAUTO_ADD_CAM;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MxAutoAddCam::saveConfig()
{
    m_isShowCameraCrdntialSettings = false;
    unloadAutoAddCam();
    emit sigAutoAddCamPopUpAction(MX_AUTO_ADD_CAM_LOGIN);
}

void MxAutoAddCam::saveConfiguration()
{
    if (m_isYesbtnclick)
    {
        return;
    }

    m_userName = m_userNameTextBoxParam->textStr;
    m_password = m_passwordTextBoxParam->textStr;

    m_payloadLib->setCnfgArrayAtIndex (0, m_userName);
    m_payloadLib->setCnfgArrayAtIndex (1, m_password);

    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               CNFG_TO_INDEX,
                                                               FIELD_AUTO_CONFIG_USERNAME + 1,
                                                               FIELD_AUTO_CONFIG_PASSWORD  + 1,
                                                               2);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;
    param->windowId = WIN_ID_MXAUTO_ADD_CAM;
    m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param);
}

void MxAutoAddCam::takeLeftKeyAction()
{
    bool status = true;

    do
    {
        if(m_currElement == 0)
        {
            m_currElement = (MAX_AUTO_ADD_ELEMENTS);
        }

        if(m_currElement)
        {
            m_currElement = (m_currElement - 1);
        }
        else
        {
            // pass key to parent
            status = false;
            break;
        }

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void MxAutoAddCam::takeRightKeyAction()
{
    bool status = true;

    do
    {
        if(m_currElement == (MAX_AUTO_ADD_ELEMENTS - 1))
        {
            m_currElement = -1;
        }

        if(m_currElement != (MAX_AUTO_ADD_ELEMENTS - 1))
        {
            m_currElement = (m_currElement + 1);
        }
        else
        {
            status = false;
            break;
        }

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    if( status == true)
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void MxAutoAddCam::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void MxAutoAddCam::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void MxAutoAddCam::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = AUTO_ADD_CLS_BTN;
    m_elementList[AUTO_ADD_CLS_BTN]->forceActiveFocus();
}

void MxAutoAddCam::navigationKeyPressed(QKeyEvent *event)
{
    if(this->isVisible())
    {
        event->accept();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}
