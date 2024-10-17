#include "WizardAddCamera.h"
#include <QPainter>
#include <QKeyEvent>
#include "ValidationMessage.h"
#include "../ConfigPages/CameraSettings/CameraSettings.h"

#define ADD_CAMERA_WIDTH            SCALE_WIDTH(570)
#define ADD_CAMERA_HEIGHT           SCALE_HEIGHT(520)
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(85)

static const QString addCameraStr[] = {"Add Camera",
                                       "Camera Index",
                                       "IP Address",
                                       "HTTP Port",
                                       "ONVIF Support",
                                       "ONVIF Port",
                                       "Brand",
                                       "Model",
                                       "Camera Name",
                                       "Username",
                                       "Password",
                                       "Save"};

WizardAddCamera::WizardAddCamera(QString currDevName,
                                 QMap<quint8, QString> ipAddrMapList,
                                 QString httpPortStr,
                                 QString onvifPortStr,
                                 bool onvifSupport,
                                 QString brandlistStr,
                                 QString modellistStr,
                                 QString camName,
                                 QString usrName,
                                 QString paswrd,
                                 QWidget *parent,
                                 quint8 idxInPage,
                                 MX_CAM_STATUS_e camStatus,
                                 quint8 camIndex,
                                 DEV_TABLE_INFO_t *devTabInfo) : KeyBoard(parent)
{
    processbar = NULL;
    payloadLib =  new PayloadLib();
    applController = ApplController::getInstance();
    currentDeviceName = currDevName;
    m_devTabInfo = devTabInfo;

    this->setGeometry (0, 0, parent->width(), parent->height());

    if((onvifSupport == false) && (camStatus == MX_CAM_UNIDENTIFY))
    {
        brandlistStr = "";
        modellistStr = "";
    }

    if(camName.size() > MAX_CAMERA_NAME_LENGTH)
    {
        camName.truncate(MAX_CAMERA_NAME_LENGTH);
    }

    ipAddrMap = ipAddrMapList;
    httpPortAddress = httpPortStr;
    onvifPort = onvifPortStr;
    cameraName = camName;
    userName = usrName;
    password = paswrd;
    isOnvifSupport = onvifSupport;
    indexInPage = idxInPage;
    selIndex = camIndex;
    currentIndex = camIndex;
    brandNameValue = brandlistStr;
    modelNameValue = modellistStr;
    cameraStatus = camStatus;
    isCameraAdded = (cameraStatus == MX_CAM_ADDED) ? true : false;
    m_cameraInformation = NULL;
    createDefaultElements();
    this->show ();
}

WizardAddCamera::~WizardAddCamera ()
{
    delete backGround;
    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete closeButton;

    delete heading;
    disconnect (cameraListButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete cameraListButton;
    delete cameraIndexReadOnly;

    delete ipAddrDropdown;
    delete httpPortReadOnly;
    disconnect (onvifSupportCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionButtonClick(OPTION_STATE_TYPE_e,int)));
    delete onvifSupportCheckBox;

    delete onvifPortReadOnly;

    disconnect(brandNameDropdown,
            SIGNAL(sigValueChanged(QString, quint32)),
            this,
            SLOT(slotValueChanged(QString, quint32)));

    DELETE_OBJ(brandNameDropdown);
    DELETE_OBJ(brandNameListParam);

    disconnect(modelNameDropdown,
            SIGNAL(sigValueListEmpty(quint8)),
            this,
            SLOT(slotValueListEmpty(quint8)));

    DELETE_OBJ(modelNameDropdown);
    DELETE_OBJ(modelNameListParam);

    delete ipCameraNameTextBox;
    delete ipCameraNameTextboxParam;

    delete usernameTextBox;
    delete usernameTextboxParam;

    delete passwordTextBox;
    delete passwordTextboxParam;

    disconnect (saveButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete saveButton;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
    delete payloadLib;

    if(m_cameraInformation != NULL)
    {
        disconnect (m_cameraInformation,
                    SIGNAL(sigObjectDelete(quint8)),
                    this,
                    SLOT(slotObjectDelete(quint8)));
        delete m_cameraInformation;
    }
}

void WizardAddCamera::createDefaultElements()
{
    backGround = new Rectangle((SCALE_WIDTH(300)),
                               (SCALE_HEIGHT(100)),
                               ADD_CAMERA_WIDTH,
                               ADD_CAMERA_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton ((backGround->x ()+ backGround->width () - SCALE_WIDTH(20)),
                                    (backGround->y () + SCALE_HEIGHT(20)),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    WIZ_ADD_CAMERAS_CLOSE_BUTTON);
    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    heading = new Heading((backGround->x () + (backGround->width () / 2)),
                          (backGround->y () + SCALE_HEIGHT(20)),
                          addCameraStr[0],
                          this,
                          HEADING_TYPE_2);

    QString cameraNumberString = "" ;
    if(cameraStatus == MX_CAM_ADDED)
    {
        cameraNumberString = QString("%1").arg (selIndex);
    }

    cameraIndexReadOnly = new ReadOnlyElement((backGround->x () + ((ADD_CAMERA_WIDTH - BGTILE_MEDIUM_SIZE_WIDTH) / 2)),
                                              (heading->y () + BGTILE_HEIGHT),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              SCALE_WIDTH(READONLY_MEDIAM_WIDTH),
                                              READONLY_HEIGHT,
                                              cameraNumberString,
                                              this,
                                              COMMON_LAYER,
                                              -1,SCALE_WIDTH(10),
                                              addCameraStr[1], "", "", SCALE_FONT(10),
                                              true, NORMAL_FONT_COLOR,
                                              LEFT_MARGIN_FROM_CENTER);

    cameraListButton = new ControlButton(CAMERA_LIST_INDEX,
                                         (cameraIndexReadOnly->x () + cameraIndexReadOnly->width () - SCALE_WIDTH(100) - LEFT_MARGIN_FROM_CENTER),
                                         cameraIndexReadOnly->y (),
                                         cameraIndexReadOnly->width (),
                                         cameraIndexReadOnly->height (),
                                         this,
                                         NO_LAYER,
                                         -1,
                                         "",
                                         true,
                                         (WIZ_ADD_CAMERAS_CAMERA_INFO_BUTTON));
    connect (cameraListButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    ipAddrDropdown = new DropDown(cameraIndexReadOnly->x (),
                                  (cameraIndexReadOnly->y () + cameraIndexReadOnly->height ()),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  WIZ_ADD_CAMERAS_IP_ADDRESS_DROPDOWN,
                                  DROPDOWNBOX_SIZE_320,
                                  addCameraStr[2],
                                  ipAddrMap,
                                  this, "", true, 0,
                                  COMMON_LAYER, true, 8, false, false,
                                  5, LEFT_MARGIN_FROM_CENTER);

    if (ipAddrMap.size() <= 1)
    {
        ipAddrDropdown->setIsEnabled(false);
    }

    httpPortReadOnly = new ReadOnlyElement(ipAddrDropdown->x (),
                                           (ipAddrDropdown->y () + ipAddrDropdown->height ()),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(READONLY_MEDIAM_WIDTH),
                                           READONLY_HEIGHT,
                                           httpPortAddress,
                                           this,
                                           COMMON_LAYER,
                                           -1,SCALE_WIDTH(10),
                                           addCameraStr[3], "", "", SCALE_FONT(10),
                                           true, NORMAL_FONT_COLOR,
                                           LEFT_MARGIN_FROM_CENTER);

    onvifSupportCheckBox = new OptionSelectButton(httpPortReadOnly->x (),
                                                  (httpPortReadOnly->y () + httpPortReadOnly->height ()),
                                                  httpPortReadOnly->width (),
                                                  httpPortReadOnly->height (),
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  COMMON_LAYER,
                                                  addCameraStr[4],
                                                  "",
                                                  -1,
                                                  WIZ_ADD_CAMERAS_ONVIF_CHECKBOX,
                                                  (isOnvifSupport && (!isCameraAdded)),
                                                  -1, SUFFIX_FONT_COLOR, false,
                                                  LEFT_MARGIN_FROM_CENTER);

    connect (onvifSupportCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionButtonClick(OPTION_STATE_TYPE_e,int)));

    onvifPortReadOnly = new ReadOnlyElement(onvifSupportCheckBox->x (),
                                            (onvifSupportCheckBox->y () + onvifSupportCheckBox->height ()),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            SCALE_WIDTH(READONLY_MEDIAM_WIDTH),
                                            READONLY_HEIGHT,
                                            onvifPort,
                                            this,
                                            COMMON_LAYER,
                                            -1,SCALE_WIDTH(10),
                                            addCameraStr[5], "", "", SCALE_FONT(10),
                                            true, NORMAL_FONT_COLOR,
                                            LEFT_MARGIN_FROM_CENTER);

    brandNameList.clear ();
    brandNameList.insert (0, brandNameValue);

    brandNameListParam = new TextboxParam();
    brandNameListParam->labelStr = addCameraStr[6];
    brandNameListParam->textStr = brandNameList.value(0);
    brandNameListParam->isCentre = true;
    brandNameListParam->leftMargin = SCALE_WIDTH(20);
    brandNameListParam->maxChar = 30;
    brandNameListParam->isTotalBlankStrAllow = true;
    brandNameListParam->validation = QRegExp(QString("[a-zA-Z0-9]"));

    brandNameDropdown = new TextWithList(onvifPortReadOnly->x(),
                                         (onvifPortReadOnly->y() + onvifPortReadOnly->height()),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         WIZ_ADD_CAMERAS_BRAND_NAME_PICKLIST,
                                         brandNameList, this,
                                         brandNameListParam, COMMON_LAYER, true, 6,
                                         TEXTBOX_LARGE, "Search", false,
                                         LIST_FILTER_TYPE_ANY_CHAR,
                                         LEFT_MARGIN_FROM_CENTER);

    brandNameDropdown->setNewList(brandNameList, 0, (brandNameValue == ""), true);

    connect(brandNameDropdown,
            SIGNAL(sigValueChanged(QString, quint32)),
            this,
            SLOT(slotValueChanged(QString, quint32)));

    modelNameList.clear ();
    modelNameList.insert (0, modelNameValue);

    modelNameListParam = new TextboxParam();
    modelNameListParam->labelStr = addCameraStr[7];
    modelNameListParam->textStr = modelNameList.value(0);
    modelNameListParam->isCentre = true;
    modelNameListParam->leftMargin = SCALE_WIDTH(20);
    modelNameListParam->maxChar = 30;
    modelNameListParam->isTotalBlankStrAllow = true;
    modelNameListParam->validation = QRegExp(QString("[a-zA-Z0-9]"));

    modelNameDropdown = new TextWithList(brandNameDropdown->x(),
                                         brandNameDropdown->y() + brandNameDropdown->height(),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         WIZ_ADD_CAMERAS_MODEL_NAME_PICKLIST,
                                         modelNameList, this,
                                         modelNameListParam, COMMON_LAYER, true, 6,
                                         TEXTBOX_LARGE, "Search", false,
                                         LIST_FILTER_TYPE_ANY_CHAR,
                                         LEFT_MARGIN_FROM_CENTER);

    modelNameDropdown->setNewList(modelNameList, 0, (modelNameValue == ""), true);

    connect(modelNameDropdown,
            SIGNAL(sigValueListEmpty(quint8)),
            this,
            SLOT(slotValueListEmpty(quint8)));

    ipCameraNameTextboxParam = new TextboxParam();
    ipCameraNameTextboxParam->labelStr = addCameraStr[8];
    ipCameraNameTextboxParam->maxChar = 16;
    ipCameraNameTextboxParam->validation = QRegExp(QString("[^\\ ]"));
    ipCameraNameTextboxParam->textStr = cameraName;

    ipCameraNameTextBox = new TextBox(modelNameDropdown->x (),
                                      (modelNameDropdown->y () + BGTILE_HEIGHT),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      WIZ_ADD_CAMERAS_CAMERA_NAME_TEXTBOX,
                                      TEXTBOX_LARGE,
                                      this,
                                      ipCameraNameTextboxParam,
                                      COMMON_LAYER, true, false, false,
                                      LEFT_MARGIN_FROM_CENTER);

    usernameTextboxParam = new TextboxParam();
    usernameTextboxParam->maxChar = 24;
    usernameTextboxParam->labelStr = addCameraStr[9];

    usernameTextBox = new TextBox(ipCameraNameTextBox->x (),
                                  (ipCameraNameTextBox->y () + ipCameraNameTextBox->height ()),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  WIZ_ADD_CAMERAS_USERNAME_TEXTBOX,
                                  TEXTBOX_LARGE,
                                  this,
                                  usernameTextboxParam,
                                  COMMON_LAYER, true, false, false,
                                  LEFT_MARGIN_FROM_CENTER);

    passwordTextboxParam = new TextboxParam();
    passwordTextboxParam->maxChar = 20;
    passwordTextboxParam->labelStr = addCameraStr[10];
    passwordTextboxParam->suffixStr = "(Max 20 chars)";

    passwordTextBox = new PasswordTextbox(usernameTextBox->x (),
                                          (usernameTextBox->y () + usernameTextBox->height ()),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          WIZ_ADD_CAMERAS_PASSWORD_PASSWORDBOX,
                                          TEXTBOX_LARGE,
                                          this,
                                          passwordTextboxParam,
                                          COMMON_LAYER, true,
                                          LEFT_MARGIN_FROM_CENTER);

    saveButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (passwordTextBox->x () + (BGTILE_MEDIUM_SIZE_WIDTH / 2)),
                                (passwordTextBox->y () + passwordTextBox->height () + SCALE_HEIGHT(40)),
                                addCameraStr[11],
                                this,
                                WIZ_ADD_CAMERAS_SAVE_BUTTON);
    connect (saveButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    infoPage = new InfoPage (0, 0,
                             (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH)),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    if (!isCameraAdded)
    {
        if((cameraStatus == MX_CAM_UNIDENTIFY) && (isOnvifSupport))
        {
            brandNameDropdown->setIsEnabled (false);
            modelNameDropdown->setIsEnabled (false);
        }
        else
        {
            brandNameDropdown->setIsEnabled (true);
            modelNameDropdown->setIsEnabled (true);
        }
    }
    else
    {
        brandNameDropdown->setIsEnabled (false);
        modelNameDropdown->setIsEnabled (false);
    }

    if(cameraStatus == MX_CAM_IDENTIFY)
    {
        createCommandRequest(BRND_NAME, 0);
        payloadLib->setCnfgArrayAtIndex(0, brandNameValue);
        createCommandRequest(GET_USER_DETAIL, 1);
    }
    else if(isOnvifSupport)
    {
        onvifSupportCheckBox->changeState(ON_STATE);
        if((cameraStatus != MX_CAM_ADDED) && (cameraStatus != MAX_MX_CAM_STATUS))
        {
            if(cameraStatus == MX_CAM_IDENTIFY)
            {
                payloadLib->setCnfgArrayAtIndex(0, brandNameValue);
            }
            else if(cameraStatus == MX_CAM_UNIDENTIFY)
            {
                payloadLib->setCnfgArrayAtIndex(0, "ONVIF");
            }
            createCommandRequest(GET_USER_DETAIL, 1);
        }
    }
    else
    {
        if(cameraStatus == MX_CAM_IDENTIFY)
        {
            payloadLib->setCnfgArrayAtIndex(0, brandNameValue);
            createCommandRequest(GET_USER_DETAIL, 1);
        }
        else
        {
            createCommandRequest(BRND_NAME, 0);
        }
    }

    if(cameraStatus == MX_CAM_ADDED)
    {
        getConfig();
    }
}

void WizardAddCamera::processDeviceResponse(DevCommParam *param, QString)
{
    quint8 maxField = 4;
    QStringList cameraIndexList;
    QStringList cameraNameList;
    QStringList cameraIpList;
    QStringList cameraStateList;

    if(param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_SET_CMD:
        {
            payloadLib->parseDevCmdReply(true, param->payload);
            switch(param->cmdType)
            {
                case BRND_NAME:
                {
                    QString selectedBrand = brandNameDropdown->getCurrValue(true);
                    quint8 maxIndex = payloadLib->getTotalCmdFields();
                    quint8 brandIndex = maxIndex;
                    brandNameList.clear();
                    for (quint8 index = 0; index < maxIndex; index++)
                    {
                        brandNameList.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString ());
                        if ((brandIndex == maxIndex) && (selectedBrand != "") && (selectedBrand == brandNameList.value(index)))
                        {
                            brandIndex = index;
                        }
                    }

                    brandNameDropdown->setNewList(brandNameList, brandIndex, (selectedBrand == ""));

                    if ((onvifSupportCheckBox->getCurrentState() == OFF_STATE) && (selectedBrand != ""))
                    {
                        payloadLib->setCnfgArrayAtIndex(0, brandNameDropdown->getCurrValue(true));
                        createCommandRequest(MDL_NAME, 1);
                    }
                }
                break;

                case MDL_NAME:
                {
                    QString selectedModel = modelNameDropdown->getCurrValue(true);
                    quint8 maxIndex = payloadLib->getTotalCmdFields();
                    quint8 modelIndex = maxIndex;
                    modelNameList.clear();
                    for (quint8 index = 0; index < maxIndex; index++)
                    {
                        modelNameList.insert (index, payloadLib->getCnfgArrayAtIndex(index).toString ());
                        if ((modelIndex == maxIndex) && (selectedModel != "") && (selectedModel == modelNameList.value(index)))
                        {
                            modelIndex = index;
                        }
                    }

                    modelNameDropdown->setNewList(modelNameList, modelIndex, (selectedModel == ""));
                }
                break;

                case GET_USER_DETAIL:
                {
                    usernameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(0).toString());
                    passwordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(1).toString());
                }
                break;

                case GET_CAMERA_INFO:
                {
                    cameraIndexList.clear ();
                    cameraNameList.clear ();
                    cameraIpList.clear ();
                    cameraStateList.clear ();

                    for(quint8 index = 0; index < m_devTabInfo->ipCams; index++)
                    {
                        cameraIndexList.insert (index,payloadLib->getCnfgArrayAtIndex(index*maxField).toString ());
                        cameraIpList.insert (index,payloadLib->getCnfgArrayAtIndex((index*maxField) + 1).toString ());
                        cameraNameList.insert (index,payloadLib->getCnfgArrayAtIndex((index*maxField) + 2).toString ());
                        cameraStateList.insert (index,(payloadLib->getCnfgArrayAtIndex((index*maxField) + 3).toBool () ==  true) ? "Enabled" : "Disabled");
                    }

                    if(m_cameraInformation == NULL)
                    {
                        m_cameraInformation = new WizardCameraInformation(cameraIndexList,
                                                                  cameraIpList,
                                                                  cameraNameList,
                                                                  cameraStateList,
                                                                  this,
                                                                  selIndex);
                        connect (m_cameraInformation,
                                 SIGNAL(sigObjectDelete(quint8)),
                                 this,
                                 SLOT(slotObjectDelete(quint8)));
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        case MSG_GET_CFG:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                payloadLib->parsePayload(param->msgType, param->payload);

                if(payloadLib->getcnfgTableIndex() == IP_CAMERA_SETTING_TABLE_INDEX)
                {
                    usernameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(IP_CAMERA_USERNAME).toString());
                    passwordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(IP_CAMERA_PASSWORD).toString());
                }

                if (onvifSupportCheckBox->getCurrentState() == OFF_STATE)
                {
                    createCommandRequest(BRND_NAME, 0);
                }
            }
            else
            {
                infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void WizardAddCamera::createCommandRequest(SET_COMMAND_e cmdType, quint8 totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void WizardAddCamera::saveValues()
{
    if((onvifSupportCheckBox->getCurrentState () == OFF_STATE) && (brandNameDropdown->getCurrValue(true) == "") && (cameraStatus != MX_CAM_ADDED))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_VALID_BRAND_NM));
    }
    else if ((onvifSupportCheckBox->getCurrentState () == OFF_STATE) && (modelNameDropdown->getCurrValue(true) == "") && (cameraStatus != MX_CAM_ADDED))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_MODEL_NM));
    }
    else if((usernameTextBox->getInputText () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
    }
    else if ((passwordTextBox->getInputText () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
    }
    else
    {
        brandNameValue = brandNameDropdown->getCurrValue(true);
        modelNameValue = modelNameDropdown->getCurrValue(true);
        cameraName = ipCameraNameTextBox->getInputText();
        userName = usernameTextBox->getInputText();
        password = passwordTextBox->getInputText();
        isOnvifSupport = (onvifSupportCheckBox->getCurrentState () == ON_STATE ? true : false );

        emit sigDeleteObject(indexInPage, true,
                             ipAddrDropdown->getCurrValue(), httpPortAddress, onvifPort,
                             isOnvifSupport, brandNameValue,
                             modelNameValue, cameraName,
                             userName, password,selIndex,currentIndex);
    }
}

void WizardAddCamera::getConfig ()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             IP_CAMERA_SETTING_TABLE_INDEX,
                                                             selIndex,
                                                             selIndex,
                                                             (IP_CAMERA_BRAND + 1),
                                                             MAX_IP_CAMERA_SETTINGS_FIELDS,
                                                             MAX_IP_CAMERA_SETTINGS_FIELDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void WizardAddCamera::slotButtonClick(int index)
{
    switch(index)
    {
        case WIZ_ADD_CAMERAS_CLOSE_BUTTON:
            emit sigDeleteObject(indexInPage, false,
                                 ipAddrDropdown->getCurrValue(), httpPortAddress, onvifPort,
                                 isOnvifSupport, brandNameValue,
                                 modelNameValue, cameraName,
                                 userName, password,
                                 selIndex,currentIndex);
            break;

        case WIZ_ADD_CAMERAS_SAVE_BUTTON:
            saveValues();
            break;

        case WIZ_ADD_CAMERAS_CAMERA_INFO_BUTTON:
            createCommandRequest(GET_CAMERA_INFO,0);
            break;

        default:
            break;
    }
}

void WizardAddCamera::slotValueChanged(QString, quint32 index)
{
    if (WIZ_ADD_CAMERAS_BRAND_NAME_PICKLIST != index)
    {
        return;
    }

    modelNameList.clear();
    modelNameDropdown->setNewList(modelNameList, 0, true);

    payloadLib->setCnfgArrayAtIndex(0, brandNameDropdown->getCurrValue(true));
    createCommandRequest(MDL_NAME, 1);
    createCommandRequest(GET_USER_DETAIL, 1);
}

void WizardAddCamera::slotOptionButtonClick(OPTION_STATE_TYPE_e state,int)
{
    brandNameDropdown->setIsEnabled (state == ON_STATE ? false : true);
    modelNameDropdown->setIsEnabled (state == ON_STATE ? false : true);

    if(cameraStatus == MX_CAM_UNIDENTIFY)
    {
        brandNameList.clear ();
        brandNameDropdown->setNewList(brandNameList, 0, true);

        modelNameList.clear ();
        modelNameDropdown->setNewList(modelNameList, 0, true);
    }

    if(state == ON_STATE)
    {
        payloadLib->setCnfgArrayAtIndex(0, "ONVIF");
        createCommandRequest(GET_USER_DETAIL, 1);
    }
    else
    {
        createCommandRequest(BRND_NAME, 0);
    }
}

void WizardAddCamera::slotInfoPageBtnclick (int)
{
    infoPage->unloadInfoPage ();
}

void WizardAddCamera::slotObjectDelete (quint8 index)
{
    selIndex = index;
    if(index != 0)
    {
        cameraIndexReadOnly->changeValue (QString("%1").arg (selIndex));
    }

    disconnect (m_cameraInformation,
                SIGNAL(sigObjectDelete(quint8)),
                this,
                SLOT(slotObjectDelete(quint8)));
    delete m_cameraInformation;
    m_cameraInformation = NULL;
}

void WizardAddCamera::slotValueListEmpty(quint8 index)
{
    if (index == WIZ_ADD_CAMERAS_MODEL_NAME_PICKLIST)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_VALID_BRAND_NM));
    }
}
