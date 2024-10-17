#include "NetworkDrive.h"
#include "ValidationMessage.h"

#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(50)

typedef enum {

    NTK_DRV_ENBL,
    NTK_DRV_NAME,
    NTK_DRV_IP_ADDR,
    NTK_DRV_USERNAME,
    NTK_DRV_PASSWORD,
    NTK_DRV_FILE_SYS,
    NTK_DRV_DEFAULT_FOLDER,

    MAX_NTWRK_DRV_FIELDS
}NTWRK_DRV_FIELDS_e;

typedef enum {

    NTWRK_DRV_SPINBOX,
    NTWRK_DRV_ENBL,
    NTWRK_DRV_NAME_TXTBX,
    NTWRK_DRV_IP_ADDR,
    NTWRK_DRV_USERNAME_TXTBX,
    NTWRK_DRV_PASSWORD_TXTBX,
    NTWRK_DRV_FILE_SYS,
    NTWRK_DRV_DEFAULT_FOLDER_TXTBX,
    NTWRK_DRV_CONN_BTN,

    MAX_NTWRK_DRV_CTRL
}NTWRK_DRV_CTRL_e;

static const QString networkDriveStrings[MAX_NTWRK_DRV_CTRL]={
    "Network Drive",
    "Enable",
    "Name",
    "IP Address",
    "Username",
    "Password",
    "File System",
    "Default Folder",
    "Test Connection"
};

static const QStringList networkDriveList = QStringList()
        << "1" << "2";

static const QStringList netrkFileSysList = QStringList()
        << "CIFS"
        << "NFS";

NetworkDrive::NetworkDrive(QString devName,QWidget *parent)
    : ConfigPageControl(devName, parent,MAX_NTWRK_DRV_CTRL)
{
    createDefaultComponents ();
    cnfgIndex = 1;
    NetworkDrive::getConfig();
}

void NetworkDrive::createDefaultComponents ()
{
    QMap<quint8, QString>  networkDriveMapList;

    for(quint8 index = 0; index <  networkDriveList.length (); index++)
    {
        networkDriveMapList.insert (index,networkDriveList.at (index));
    }

    networkDriveDropDownBox = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2 + SCALE_WIDTH(10) ,
                                           (SCALE_WIDTH(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) - 8*BGTILE_HEIGHT)/2,
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           NTWRK_DRV_SPINBOX,
                                           DROPDOWNBOX_SIZE_200,
                                           networkDriveStrings[NTWRK_DRV_SPINBOX],
                                           networkDriveMapList,
                                           this, "", true, 0, COMMON_LAYER,
                                           true, 8, false, false, 5,
                                           LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_SPINBOX] = networkDriveDropDownBox;

    connect (networkDriveDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (networkDriveDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    enableDrive = new OptionSelectButton(networkDriveDropDownBox->x (),
                                         networkDriveDropDownBox->y ()  + networkDriveDropDownBox->height (),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CHECK_BUTTON_INDEX,
                                         this,
                                         COMMON_LAYER,
                                         networkDriveStrings[NTWRK_DRV_ENBL],
                                         "",
                                         -1,
                                         NTWRK_DRV_ENBL, true, -1,
                                         SUFFIX_FONT_COLOR, false,
                                         LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_ENBL] = enableDrive;

    connect (enableDrive,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (enableDrive,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    nameTextBoxParam = new TextboxParam();
    nameTextBoxParam->isCentre = true;
    nameTextBoxParam->labelStr = networkDriveStrings[NTWRK_DRV_NAME_TXTBX];
    nameTextBoxParam->maxChar = 40;
    nameTextBoxParam->isTotalBlankStrAllow = true;

    nameTextBox = new TextBox(enableDrive->x (),
                              enableDrive->y () + enableDrive->height (),
                              BGTILE_MEDIUM_SIZE_WIDTH,
                              BGTILE_HEIGHT,
                              NTWRK_DRV_NAME_TXTBX,
                              TEXTBOX_LARGE,
                              this,
                              nameTextBoxParam,
                              COMMON_LAYER, true,
                              false, false,
                              LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_NAME_TXTBX] = nameTextBox;

    connect (nameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    ipTextBox =  new IpTextBox(nameTextBox->x (),
                               nameTextBox->y () + BGTILE_HEIGHT,
                               BGTILE_MEDIUM_SIZE_WIDTH,
                               BGTILE_HEIGHT,
                               NTWRK_DRV_IP_ADDR,
                               networkDriveStrings[NTWRK_DRV_IP_ADDR],
                               IP_ADDR_TYPE_IPV4_AND_IPV6,
                               this, COMMON_LAYER, true, 0,
                               true, IP_FIELD_TYPE_IPV6_ADDR,
                               IP_TEXTBOX_ULTRALARGE,
                               LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_IP_ADDR] = ipTextBox;

    connect (ipTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    usernameTextBoxParam = new TextboxParam();
    usernameTextBoxParam->isCentre = true;
    usernameTextBoxParam->labelStr = networkDriveStrings[NTWRK_DRV_USERNAME_TXTBX];
    usernameTextBoxParam->maxChar = 40;
    usernameTextBoxParam->isTotalBlankStrAllow = true;

    usernameTextBox = new TextBox(ipTextBox->x (),
                                  ipTextBox->y () + ipTextBox->height (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  NTWRK_DRV_USERNAME_TXTBX,
                                  TEXTBOX_LARGE,
                                  this,
                                  usernameTextBoxParam,
                                  COMMON_LAYER, true, false,
                                  false, LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_USERNAME_TXTBX] = usernameTextBox;

    connect (usernameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    passwordTextBoxParam = new TextboxParam();
    passwordTextBoxParam->isCentre = true;
    passwordTextBoxParam->labelStr = networkDriveStrings[NTWRK_DRV_PASSWORD_TXTBX];
    passwordTextBoxParam->maxChar = 24;
    passwordTextBoxParam->isTotalBlankStrAllow = true;

    passwordTextBox = new PasswordTextbox(usernameTextBox->x (),
                                          usernameTextBox->y () + usernameTextBox->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          NTWRK_DRV_PASSWORD_TXTBX,
                                          TEXTBOX_LARGE,
                                          this,
                                          passwordTextBoxParam,
                                          COMMON_LAYER, true,
                                          LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_PASSWORD_TXTBX] = passwordTextBox;

    connect (passwordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString>  netrkFileSysMapList;

    for(quint8 index = 0; index <  netrkFileSysList.length (); index++)
    {
        netrkFileSysMapList.insert (index,netrkFileSysList.at (index));
    }

    netwrkFileSysDriveDropDownBox = new DropDown(passwordTextBox->x (),
                                                 passwordTextBox->y () + passwordTextBox->height (),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 NTWRK_DRV_FILE_SYS,
                                                 DROPDOWNBOX_SIZE_90,
                                                 networkDriveStrings[NTWRK_DRV_FILE_SYS],
                                                 netrkFileSysMapList,
                                                 this, "", true, 0,
                                                 COMMON_LAYER, true, 8,
                                                 false, false, 5,
                                                 LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_FILE_SYS] = netwrkFileSysDriveDropDownBox;

    connect (netwrkFileSysDriveDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    defaultFolderTextBoxParam = new TextboxParam();
    defaultFolderTextBoxParam->isCentre = true;
    defaultFolderTextBoxParam->labelStr = networkDriveStrings[NTWRK_DRV_DEFAULT_FOLDER_TXTBX];
    defaultFolderTextBoxParam->validation = QRegExp(QString("[^\\ ]"));
    defaultFolderTextBoxParam->maxChar = 255;

    defaultFolderTextBox = new TextBox(netwrkFileSysDriveDropDownBox->x (),
                                       netwrkFileSysDriveDropDownBox->y () + netwrkFileSysDriveDropDownBox->height (),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       NTWRK_DRV_DEFAULT_FOLDER_TXTBX,
                                       TEXTBOX_ULTRALARGE,
                                       this,
                                       defaultFolderTextBoxParam,
                                       COMMON_LAYER, true, false,
                                       false, LEFT_MARGIN_FROM_CENTER);

    m_elementList[NTWRK_DRV_DEFAULT_FOLDER_TXTBX] = defaultFolderTextBox;

    connect (defaultFolderTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    testConnButton = new ControlButton(TEST_CONNECTION_BUTTON_INDEX,
                                       defaultFolderTextBox->x (),
                                       defaultFolderTextBox->y () + defaultFolderTextBox->height (),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT, this, DOWN_LAYER,
                                       SCALE_WIDTH(290), networkDriveStrings[NTWRK_DRV_CONN_BTN],
                                       true, NTWRK_DRV_CONN_BTN);

    m_elementList[NTWRK_DRV_CONN_BTN] = testConnButton;

    connect (testConnButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (testConnButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestBtnClick(int)));

}

NetworkDrive::~NetworkDrive()
{
    disconnect (testConnButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (testConnButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotTestBtnClick(int)));
    delete testConnButton;

    disconnect (networkDriveDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (networkDriveDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    delete networkDriveDropDownBox;

    disconnect (enableDrive,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (enableDrive,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    delete enableDrive;

    disconnect (nameTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete nameTextBox;
    delete nameTextBoxParam;

    disconnect (ipTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete ipTextBox;

    disconnect (usernameTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete usernameTextBox;
    delete usernameTextBoxParam;

    disconnect (passwordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete passwordTextBox;
    delete passwordTextBoxParam;

    disconnect (netwrkFileSysDriveDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete netwrkFileSysDriveDropDownBox;

    disconnect (defaultFolderTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete defaultFolderTextBox;
    delete defaultFolderTextBoxParam;
}

void NetworkDrive::enableControls (bool state)
{
    nameTextBox->setIsEnabled (state);
    ipTextBox->setIsEnabled (state);
    usernameTextBox->setIsEnabled (state);
    passwordTextBox->setIsEnabled (state);
    netwrkFileSysDriveDropDownBox->setIsEnabled (state);
    defaultFolderTextBox->setIsEnabled (state);
    testConnButton->setIsEnabled (state);
}

void NetworkDrive::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString =
            payloadLib->createDevCnfgPayload(msgType,
                                             NETWORK_DRIVE_MANAGMENT_TABLE_INDEX,
                                             cnfgIndex,
                                             cnfgIndex,
                                             CNFG_FRM_INDEX,
                                             MAX_NTWRK_DRV_FIELDS,
                                             MAX_NTWRK_DRV_FIELDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void NetworkDrive::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

void NetworkDrive::getConfig ()
{
    createPayload(MSG_GET_CFG);
}

void NetworkDrive::saveConfig ()
{
    QString tempStr;

    if(enableDrive->getCurrentState () == ON_STATE)
    {
        if(nameTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(NW_DRIVE_ENT_NAME));
            networkDriveDropDownBox->setIndexofCurrElement (cnfgIndex -1);
            return;
        }

        ipTextBox->getIpaddress (tempStr);
        if(tempStr == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR));
            networkDriveDropDownBox->setIndexofCurrElement (cnfgIndex -1);
            getConfig();
            return;
        }

        if(defaultFolderTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(NW_DRIVE_ENT_DFL_FOLDER));
            networkDriveDropDownBox->setIndexofCurrElement (cnfgIndex -1);
            return;
        }
    }

    payloadLib->setCnfgArrayAtIndex (NTK_DRV_ENBL,enableDrive->getCurrentState ());

    payloadLib->setCnfgArrayAtIndex (NTK_DRV_NAME,nameTextBox->getInputText ());

    ipTextBox->getIpaddress(tempStr);
    payloadLib->setCnfgArrayAtIndex (NTK_DRV_IP_ADDR,tempStr);

    payloadLib->setCnfgArrayAtIndex (NTK_DRV_USERNAME,usernameTextBox->getInputText ());

    payloadLib->setCnfgArrayAtIndex (NTK_DRV_PASSWORD,passwordTextBox->getInputText ());

    payloadLib->setCnfgArrayAtIndex (NTK_DRV_FILE_SYS,netwrkFileSysDriveDropDownBox->getIndexofCurrElement ());

    payloadLib->setCnfgArrayAtIndex (NTK_DRV_DEFAULT_FOLDER,defaultFolderTextBox->getInputText ());


    createPayload (MSG_SET_CFG);
}

void NetworkDrive::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload (param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == NETWORK_DRIVE_MANAGMENT_TABLE_INDEX)
                {
                    quint8 temp;

                    m_configResponse.clear();

                    temp = payloadLib->getCnfgArrayAtIndex (NTK_DRV_ENBL).toUInt ();
                    enableDrive->changeState (temp == 1 ? ON_STATE: OFF_STATE);
                    enableControls (temp == 1 ? true : false);

                    m_configResponse[NTK_DRV_ENBL]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_ENBL);

                    nameTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (NTK_DRV_NAME).toString ());

                    m_configResponse[NTK_DRV_NAME]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_NAME).toString ();

                    ipTextBox->setIpaddress(
                                payloadLib->getCnfgArrayAtIndex (NTK_DRV_IP_ADDR).toString ());

                    m_configResponse[NTK_DRV_IP_ADDR]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_IP_ADDR).toString ();

                    usernameTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (NTK_DRV_USERNAME).toString ());

                    m_configResponse[NTK_DRV_USERNAME]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_USERNAME).toString ();

                    passwordTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (NTK_DRV_PASSWORD).toString ());

                    m_configResponse[NTK_DRV_PASSWORD]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_PASSWORD).toString ();

                    netwrkFileSysDriveDropDownBox->setIndexofCurrElement (
                                payloadLib->getCnfgArrayAtIndex (NTK_DRV_FILE_SYS).toUInt ());

                    m_configResponse[NTK_DRV_FILE_SYS]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_FILE_SYS).toUInt();

                    defaultFolderTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex (NTK_DRV_DEFAULT_FOLDER).toString ());

                    m_configResponse[NTK_DRV_DEFAULT_FOLDER]=payloadLib->getCnfgArrayAtIndex (NTK_DRV_DEFAULT_FOLDER).toString ();

                    processBar->unloadProcessBar ();
                }
                break;

            case MSG_SET_CFG:
                processBar->unloadProcessBar ();
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                cnfgIndex = networkDriveDropDownBox->getIndexofCurrElement ()+1;
                getConfig ();
                break;

            case MSG_DEF_CFG:
                processBar->unloadProcessBar ();
                getConfig ();
                break;

            case MSG_SET_CMD:
                if(param->cmdType == TST_ND_CON)
                {
                    processBar->unloadProcessBar ();
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(NW_DRIVE_TEST_CONN_SUCCESS));
                }
                break;

            default:
                break;
            }
            break;

        default:
            processBar->unloadProcessBar ();

            if((param->msgType == MSG_SET_CMD) &&
                    (param->cmdType == TST_ND_CON) &&
                    (param->deviceStatus != CMD_SERVER_NOT_RESPONDING))
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(NW_DRIVE_TEST_CONN_FAILED));
            }
            else
            {
                infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                networkDriveDropDownBox->setIndexofCurrElement (cnfgIndex -1);
            }
            break;
        }
    }
}


void NetworkDrive::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == ValidationMessage::getValidationMessage(SAVE_CHANGES))
        {
            saveConfig();
        }
    }
    else
    {
        if(infoPage->getText() == ValidationMessage::getValidationMessage(SAVE_CHANGES))
        {
            cnfgIndex = networkDriveDropDownBox->getIndexofCurrElement () + 1;
            getConfig();
        }
    }
}

bool NetworkDrive::isUserChangeConfig()
{
    bool isChange = false;

    do
    {
        if(m_configResponse.empty ())
       {
           isChange = true;
           break;
        }

        if((IS_VALID_OBJ(enableDrive)) && (m_configResponse[NTK_DRV_ENBL] != enableDrive->getCurrentState ()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(nameTextBox)) && (m_configResponse[NTK_DRV_NAME] != nameTextBox->getInputText ()))
        {
            isChange = true;
            break;
        }

        if(IS_VALID_OBJ(ipTextBox))
        {
            QString tempStr;
            ipTextBox->getIpaddress(tempStr);
            if(m_configResponse[NTK_DRV_IP_ADDR] != tempStr)
            {
                isChange = true;
                break;
            }
        }

        if((IS_VALID_OBJ(usernameTextBox)) && (m_configResponse[NTK_DRV_USERNAME] != usernameTextBox->getInputText ()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(passwordTextBox)) && (m_configResponse[NTK_DRV_PASSWORD] != passwordTextBox->getInputText ()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(netwrkFileSysDriveDropDownBox)) && (m_configResponse[NTK_DRV_FILE_SYS] != netwrkFileSysDriveDropDownBox->getIndexofCurrElement ()))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(defaultFolderTextBox)) && (m_configResponse[NTK_DRV_DEFAULT_FOLDER] != defaultFolderTextBox->getInputText ()))
        {
            isChange = true;
            break;
        }

    }while(0);

    return isChange;
}

void NetworkDrive::slotCheckBoxClicked (OPTION_STATE_TYPE_e state, int)
{
    enableControls (state);
}

void NetworkDrive::slotSpinBoxValueChanged (QString, quint32)
{
    if(isUserChangeConfig())
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
    }
    else
    {
        cnfgIndex = networkDriveDropDownBox->getIndexofCurrElement () + 1;
        getConfig ();
    }
}

void NetworkDrive::slotTestBtnClick (int)
{
    if(enableDrive->getCurrentState () == ON_STATE)
    {
        if(ipTextBox->getIpaddress () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR));
            getConfig();
            return;
        }

        if(defaultFolderTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(NW_DRIVE_ENT_DFL_FOLDER));
            return;
        }

        payloadLib->setCnfgArrayAtIndex (0,ipTextBox->getIpaddress ());

        payloadLib->setCnfgArrayAtIndex (1,netwrkFileSysDriveDropDownBox->getIndexofCurrElement ());

        payloadLib->setCnfgArrayAtIndex (2,usernameTextBox->getInputText ());

        payloadLib->setCnfgArrayAtIndex (3,passwordTextBox->getInputText ());

        payloadLib->setCnfgArrayAtIndex (4,defaultFolderTextBox->getInputText ());

        payloadLib->setCnfgArrayAtIndex (5,networkDriveDropDownBox->getIndexofCurrElement ());

        QString payloadString = payloadLib->createDevCmdPayload(6);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = TST_ND_CON;
        param->payload = payloadString;

        processBar->loadProcessBar ();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}
