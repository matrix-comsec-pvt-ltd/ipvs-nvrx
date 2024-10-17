#include "FtpClient.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1
#define MAX_FTP_PORT                2

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(114) //(558 - 330)/2
#define LEFT_MARGIN_FROM_CENTER     SCALE_WIDTH(60)

// List of control
typedef enum
{
    FTP_SERVER_SEL,
    FTP_ENABLE,
    FTP_SERVER_ADDRESS,
    FTP_PORT,
    FTP_USERNAME,
    FTP_PASSWORD,
    FTP_UPLOAD_PATH,
    FTP_TEST_CONN_BTN,

    MAX_FTP_STG_ELEMETS
}FTP_STG_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_ENABLE,
    FIELD_FTP_SERVER,
    FIELD_PORT,
    FIELD_USERNAME,
    FIELD_PASSWORD,
    FIELD_UPLOAD_PATH,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_FTP_STG_ELEMETS] =
{
    "FTP Server",
    "Enable FTP Service",
    "Address",
    "Port",
    "Username",
    "Password",
    "Upload Path",
    "Test Connection"
};

static const QStringList ftpSerOptList = QStringList()
        <<"1" <<"2";

FtpClient::FtpClient(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_FTP_STG_ELEMETS)
{
    createDefaultComponent();
    frmIndex = toIndex = 1;
    FtpClient::getConfig();
}

FtpClient::~FtpClient()
{
    disconnect (ftpSerNoDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChanged(QString,quint32)));
    disconnect(ftpSerNoDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete ftpSerNoDropDownBox;

    disconnect (enableFtpOpt,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(enableFtpOpt,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete enableFtpOpt;


    disconnect(passwordTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete passwordTextbox;
    delete passwordParam;


    disconnect (textboxes[FTP_TEXTBOX_SER_ADDRESS],
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    for(quint8 index = FTP_TEXTBOX_SER_ADDRESS; index <= FTP_TEXTBOX_UPLOAD_PATH; index++)
    {
        disconnect (textboxes[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete textboxes[index];
        delete textboxParams[index];
    }

    disconnect (testConnButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotTestBtnClick(int)));
    disconnect(testConnButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete testConnButton;
}

void FtpClient::createDefaultComponent()
{
    QMap<quint8, QString>  ftpSerOptList;

    for(quint8 index = 0; index < MAX_FTP_PORT; index++)
    {
        ftpSerOptList.insert (index,QString("%1").arg (index+1));
    }

    ftpSerNoDropDownBox = new DropDown(FIRST_ELE_XOFFSET,
                                       FIRST_ELE_YOFFSET,
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       FTP_SERVER_SEL,
                                       DROPDOWNBOX_SIZE_90,
                                       labelStr[FTP_SERVER_SEL],
                                       ftpSerOptList, this, "",
                                       true, 0, TOP_LAYER);
    m_elementList[FTP_SERVER_SEL] = ftpSerNoDropDownBox;
    connect(ftpSerNoDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (ftpSerNoDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    enableFtpOpt = new OptionSelectButton(ftpSerNoDropDownBox->x (),
                                          ftpSerNoDropDownBox->y () + ftpSerNoDropDownBox->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          labelStr[FTP_ENABLE],
                                          this,
                                          MIDDLE_TABLE_LAYER,
                                          SCALE_WIDTH(10),
                                          MX_OPTION_TEXT_TYPE_SUFFIX,
                                          NORMAL_FONT_SIZE,
                                          FTP_ENABLE, true, NORMAL_FONT_COLOR, true);
    m_elementList[FTP_ENABLE] = enableFtpOpt;
    connect(enableFtpOpt,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (enableFtpOpt,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    textboxParams[FTP_TEXTBOX_SER_ADDRESS] = new TextboxParam();
    textboxParams[FTP_TEXTBOX_SER_ADDRESS]->labelStr = labelStr[FTP_SERVER_ADDRESS];
    textboxParams[FTP_TEXTBOX_SER_ADDRESS]->maxChar = 40;    
    textboxParams[FTP_TEXTBOX_SER_ADDRESS]->validation = QRegExp(QString("[^\\ ]"));

    textboxParams[FTP_TEXTBOX_PORT] = new TextboxParam();
    textboxParams[FTP_TEXTBOX_PORT]->labelStr = labelStr[FTP_PORT];
    textboxParams[FTP_TEXTBOX_PORT]->suffixStr = "(1 - 65535)";
    textboxParams[FTP_TEXTBOX_PORT]->isNumEntry = true;
    textboxParams[FTP_TEXTBOX_PORT]->minNumValue = 1;
    textboxParams[FTP_TEXTBOX_PORT]->maxNumValue = 65535;
    textboxParams[FTP_TEXTBOX_PORT]->maxChar = 5;
    textboxParams[FTP_TEXTBOX_PORT]->validation = QRegExp(QString("[0-9]"));

    textboxParams[FTP_TEXTBOX_USERNAME] = new TextboxParam();
    textboxParams[FTP_TEXTBOX_USERNAME]->labelStr = labelStr[FTP_USERNAME];
    textboxParams[FTP_TEXTBOX_USERNAME]->maxChar = 40;

    textboxParams[FTP_TEXTBOX_UPLOAD_PATH] = new TextboxParam();
    textboxParams[FTP_TEXTBOX_UPLOAD_PATH]->labelStr = labelStr[FTP_UPLOAD_PATH];
    textboxParams[FTP_TEXTBOX_UPLOAD_PATH]->maxChar = 255;
    textboxParams[FTP_TEXTBOX_UPLOAD_PATH]->isTotalBlankStrAllow = true;
    textboxParams[FTP_TEXTBOX_UPLOAD_PATH]->validation = QRegExp(QString("[a-zA-Z0-9\\-_;,'{}+\\[\\]]"));

    passwordParam = new TextboxParam();
    passwordParam->labelStr = labelStr[FTP_PASSWORD];
    passwordParam->maxChar = 24;

    for(quint8 index = FTP_TEXTBOX_SER_ADDRESS; index <= FTP_TEXTBOX_USERNAME; index++)
    {
        textboxes[index] = new TextBox(enableFtpOpt->x (),
                                       (enableFtpOpt->y ()+ enableFtpOpt->height () +
                                        (index * BGTILE_HEIGHT)),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       (FTP_SERVER_ADDRESS + index),
                                       (index == FTP_TEXTBOX_PORT)?TEXTBOX_SMALL:TEXTBOX_ULTRALARGE,
                                       this, textboxParams[index],
                                       MIDDLE_TABLE_LAYER, true, false, false,
                                       LEFT_MARGIN_FROM_CENTER);
        m_elementList[FTP_SERVER_ADDRESS + index] = textboxes[index];
    }
    passwordTextbox = new PasswordTextbox(textboxes[FTP_TEXTBOX_USERNAME]->x (),
                                          textboxes[FTP_TEXTBOX_USERNAME]->y () + textboxes[FTP_TEXTBOX_USERNAME]->height (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          FTP_PASSWORD,
                                          TEXTBOX_ULTRALARGE, this,
                                          passwordParam, MIDDLE_TABLE_LAYER, true,
                                          LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_PASSWORD] = passwordTextbox;
    connect(passwordTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));


    textboxes[FTP_TEXTBOX_UPLOAD_PATH] = new TextBox(passwordTextbox->x (),
                                                     passwordTextbox->y ()+ passwordTextbox->height (),
                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     FTP_UPLOAD_PATH,
                                                     TEXTBOX_ULTRALARGE, this,
                                                     textboxParams[FTP_TEXTBOX_UPLOAD_PATH],
                                                     MIDDLE_TABLE_LAYER, true, false, false,
                                                     LEFT_MARGIN_FROM_CENTER);
    m_elementList[FTP_UPLOAD_PATH] = textboxes[FTP_TEXTBOX_UPLOAD_PATH];

    for(quint8 index = FTP_TEXTBOX_SER_ADDRESS; index <= FTP_TEXTBOX_UPLOAD_PATH; index++)
    {
        connect (textboxes[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }
    connect (textboxes[FTP_TEXTBOX_SER_ADDRESS],
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    testConnButton = new ControlButton(TEST_CONNECTION_BUTTON_INDEX,
                                       textboxes[FTP_TEXTBOX_UPLOAD_PATH]->x (),
                                       textboxes[FTP_TEXTBOX_UPLOAD_PATH]->y () + textboxes[FTP_TEXTBOX_UPLOAD_PATH]->height (),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       this, DOWN_LAYER, SCALE_WIDTH(290),
                                       labelStr[FTP_TEST_CONN_BTN],
                                       true, FTP_TEST_CONN_BTN);
    m_elementList[FTP_TEST_CONN_BTN] = testConnButton;
    connect(testConnButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (testConnButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestBtnClick(int)));

}


void FtpClient::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             FTP_TABLE_INDEX,
                                                             frmIndex,
                                                             toIndex,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void FtpClient::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             FTP_TABLE_INDEX,
                                                             frmIndex,
                                                             toIndex,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void FtpClient::saveConfig()
{
    if(checkDataIsSufficient () == false)
    {
        return;
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (FIELD_ENABLE,
                                         enableFtpOpt->getCurrentState ());
        payloadLib->setCnfgArrayAtIndex (FIELD_FTP_SERVER,
                                         textboxes[FTP_TEXTBOX_SER_ADDRESS]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_PORT,
                                         textboxes[FTP_TEXTBOX_PORT]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_USERNAME,
                                         textboxes[FTP_TEXTBOX_USERNAME]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_PASSWORD,
                                         passwordTextbox->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_UPLOAD_PATH,
                                         textboxes[FTP_TEXTBOX_UPLOAD_PATH]->getInputText ());

        //create the payload for Get Cnfg
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 FTP_TABLE_INDEX,
                                                                 frmIndex,
                                                                 toIndex,
                                                                 CNFG_FRM_FIELD,
                                                                 MAX_FIELD_NO,
                                                                 MAX_FIELD_NO);

        DevCommParam* param = new DevCommParam();

        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;

        processBar->loadProcessBar ();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void FtpClient::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    OPTION_STATE_TYPE_e state;
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:

            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == FTP_TABLE_INDEX)
                {
                    state = (OPTION_STATE_TYPE_e)
                            (payloadLib->getCnfgArrayAtIndex (FIELD_ENABLE).toUInt ());
                    enableFtpOpt->changeState (state);

                    for(quint8 index = FTP_TEXTBOX_SER_ADDRESS; index <= FTP_TEXTBOX_USERNAME; index++)
                    {
                        textboxes[index]->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_FTP_SERVER + index).toString ());
                    }

                    passwordTextbox->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_PASSWORD).toString ());

                    textboxes[FTP_TEXTBOX_UPLOAD_PATH]->setInputText
                            (payloadLib->getCnfgArrayAtIndex (FIELD_UPLOAD_PATH).toString ());

                    slotEnableButtonClicked(state, 0);
                }
                processBar->unloadProcessBar ();

                break;

            case MSG_SET_CFG:
                // unload processing icon
                processBar->unloadProcessBar ();
                //load info page with msg
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            case MSG_SET_CMD:
                switch(param->cmdType)
                {
                case TST_FTP_CON:
                    // unload processing icon
                    processBar->unloadProcessBar ();
                    //load info page with msg
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(FTP_CLIENT_FILE_UPLOAD_SUCCESS));
                    break;

                default:
                    break;
                }

                break;

            default:
                break;
            }

            break;

        default:
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
        update ();
    }
}

bool FtpClient::checkDataIsSufficient()
{
    if(enableFtpOpt->getCurrentState ()== ON_STATE)
    {
        if(textboxes[FTP_TEXTBOX_SER_ADDRESS]->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(FTP_CLIENT_ENT_FTP_SERVER_ADDR));
            return false;
        }
        else if(textboxes[FTP_TEXTBOX_USERNAME]->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
            return false;
        }
        else if(passwordTextbox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
            return false;
        }
    }
    return true;
}

void FtpClient::slotTestBtnClick(int)
{
    if(checkDataIsSufficient () == false)
    {
        return;
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (0,
                                         textboxes[FTP_TEXTBOX_SER_ADDRESS]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (1,
                                         textboxes[FTP_TEXTBOX_PORT]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (2,
                                         textboxes[FTP_TEXTBOX_USERNAME]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (3,
                                         passwordTextbox->getInputText ());
        payloadLib->setCnfgArrayAtIndex (4,
                                         textboxes[FTP_TEXTBOX_UPLOAD_PATH]->getInputText ());

        QString payloadString = payloadLib->createDevCmdPayload(5);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = TST_FTP_CON;
        param->payload = payloadString;

        processBar->loadProcessBar ();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void FtpClient::slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int)
{
    for(quint8 index = FTP_TEXTBOX_SER_ADDRESS; index <= FTP_TEXTBOX_UPLOAD_PATH; index++)
    {
        textboxes[index]->setIsEnabled (state);
    }
    passwordTextbox->setIsEnabled (state);
    testConnButton->setIsEnabled (state);
}

void FtpClient::slotSpinboxValueChanged(QString,quint32)
{
    frmIndex = toIndex = (ftpSerNoDropDownBox->getIndexofCurrElement () + 1);
    getConfig ();
}

void FtpClient::slotTextboxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(FTP_CLIENT_ENT_CORRECT_ADDR));
    }
}

