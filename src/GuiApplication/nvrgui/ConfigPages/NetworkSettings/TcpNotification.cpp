#include "TcpNotification.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(194) //(558 - 170)/2

// List of control
typedef enum
{
    TCP_ENABLE,
    TCP_SERVER,
    TCP_PORT,
    TCP_TEST_CONN_BTN,

    MAX_TCP_STG_ELEMETS
}TCP_STG_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_ENABLE,
    FIELD_TCP_SERVER,
    FIELD_PORT,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_TCP_STG_ELEMETS] =
{
    "Enable TCP Service",
    "TCP Server",
    "Port",
    "Test Connection"
};

TcpNotification::TcpNotification(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_TCP_STG_ELEMETS)
{
    createDefaultComponent();
    TcpNotification::getConfig();
}

TcpNotification::~TcpNotification()
{
    disconnect (enableTcpOpt,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect (enableTcpOpt,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete enableTcpOpt;

    disconnect (textboxes[TCP_TEXTBOX_PORT],
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    for(quint8 index = TCP_TEXTBOX_SERVER; index <=TCP_TEXTBOX_PORT; index++)
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
    disconnect (testConnButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete testConnButton;
}

void TcpNotification::createDefaultComponent()
{
    enableTcpOpt = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                          FIRST_ELE_YOFFSET,
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          labelStr[TCP_ENABLE],
                                          this,
                                          UP_LAYER,
                                          SCALE_WIDTH(15),
                                          MX_OPTION_TEXT_TYPE_SUFFIX,
                                          NORMAL_FONT_SIZE,
                                          TCP_ENABLE, true, NORMAL_FONT_COLOR, true);
    m_elementList[TCP_ENABLE] = enableTcpOpt;
    connect (enableTcpOpt,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (enableTcpOpt,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    textboxParams[TCP_TEXTBOX_SERVER] = new TextboxParam();
    textboxParams[TCP_TEXTBOX_SERVER]->labelStr = labelStr[TCP_SERVER];
    textboxParams[TCP_TEXTBOX_SERVER]->maxChar = 40;
    textboxParams[TCP_TEXTBOX_SERVER]->validation = QRegExp(QString("[^\" ]"));

    textboxParams[TCP_TEXTBOX_PORT] = new TextboxParam();
    textboxParams[TCP_TEXTBOX_PORT]->labelStr = labelStr[TCP_PORT];
    textboxParams[TCP_TEXTBOX_PORT]->suffixStr = " (1-65535)";
    textboxParams[TCP_TEXTBOX_PORT]->isNumEntry = true;
    textboxParams[TCP_TEXTBOX_PORT]->minNumValue = 1;
    textboxParams[TCP_TEXTBOX_PORT]->maxNumValue = 65535;
    textboxParams[TCP_TEXTBOX_PORT]->maxChar = 5;
    textboxParams[TCP_TEXTBOX_PORT]->validation = QRegExp(QString("[0-9]"));

    textboxes[TCP_TEXTBOX_SERVER] = new TextBox(enableTcpOpt->x (),
                                                enableTcpOpt->y () + enableTcpOpt->height (),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                TCP_SERVER, TEXTBOX_ULTRALARGE,
                                                this, textboxParams[TCP_TEXTBOX_SERVER],
                                                MIDDLE_TABLE_LAYER, true, false, false,
                                                SCALE_WIDTH(60));
    m_elementList[TCP_SERVER] = textboxes[TCP_TEXTBOX_SERVER];
    connect (textboxes[TCP_TEXTBOX_SERVER],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    textboxes[TCP_TEXTBOX_PORT] = new TextBox(textboxes[TCP_TEXTBOX_SERVER]->x (),
                                              textboxes[TCP_TEXTBOX_SERVER]->y () + textboxes[TCP_TEXTBOX_SERVER]->height (),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              TCP_PORT, TEXTBOX_SMALL,
                                              this, textboxParams[TCP_TEXTBOX_PORT],
                                              MIDDLE_TABLE_LAYER, true, false, false,
                                              SCALE_WIDTH(60));
    m_elementList[TCP_PORT] = textboxes[TCP_TEXTBOX_PORT];
    connect (textboxes[TCP_TEXTBOX_PORT],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (textboxes[TCP_TEXTBOX_PORT],
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));

    testConnButton = new ControlButton(TEST_CONNECTION_BUTTON_INDEX,
                                       textboxes[TCP_TEXTBOX_PORT]->x (),
                                       textboxes[TCP_TEXTBOX_PORT]->y () + textboxes[TCP_TEXTBOX_PORT]->height (),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT, this, DOWN_LAYER,
                                       SCALE_WIDTH(290), labelStr[TCP_TEST_CONN_BTN],
                                       true, TCP_TEST_CONN_BTN);
    m_elementList[TCP_TEST_CONN_BTN] = testConnButton;
    connect (testConnButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (testConnButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestBtnClick(int)));

}
void TcpNotification::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             TCP_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void TcpNotification::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             TCP_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void TcpNotification::saveConfig()
{
    if(checkDataIsSufficient () == false)
    {
        return;
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (FIELD_ENABLE,
                                         enableTcpOpt->getCurrentState ());
        payloadLib->setCnfgArrayAtIndex (FIELD_TCP_SERVER,
                                         textboxes[TCP_TEXTBOX_SERVER]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (FIELD_PORT,
                                         textboxes[TCP_TEXTBOX_PORT]->getInputText ());

        //create the payload for Get Cnfg
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 TCP_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_TO_INDEX,
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

void TcpNotification::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    OPTION_STATE_TYPE_e state;
    processBar->unloadProcessBar();
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == TCP_TABLE_INDEX)
                {
                    state = (OPTION_STATE_TYPE_e)
                            (payloadLib->getCnfgArrayAtIndex (FIELD_ENABLE).toUInt ());
                    enableTcpOpt->changeState (state);

                    for(quint8 index = TCP_TEXTBOX_SERVER; index <= TCP_TEXTBOX_PORT; index++)
                    {
                        textboxes[index]->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_TCP_SERVER + index).toString ());
                    }                    
                    slotEnableButtonClicked(state, 0);
                }
                break;

            case MSG_SET_CFG:
                //load info page with msg
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            case MSG_SET_CMD:
                switch(param->cmdType)
                {
                case TST_TCP_CON:
                    //load info page with msg
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(TCP_CLIENT_NOTI_SENT_SUCCESS));
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
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
        update();
    }
}

bool TcpNotification::checkDataIsSufficient()
{
    if(enableTcpOpt->getCurrentState ()== ON_STATE)
    {
        if(textboxes[TCP_TEXTBOX_SERVER]->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(TCP_CLIENT_ENT_SERVER_VALUE));
            return false;
        }
    }
    return true;
}

void TcpNotification::slotTestBtnClick(int)
{
    if(checkDataIsSufficient () == false)
    {
        return;
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (0,
                                         textboxes[TCP_TEXTBOX_SERVER]->getInputText ());
        payloadLib->setCnfgArrayAtIndex (1,
                                         textboxes[TCP_TEXTBOX_PORT]->getInputText ());

        QString payloadString = payloadLib->createDevCmdPayload(2);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = TST_TCP_CON;
        param->payload = payloadString;

        processBar->loadProcessBar ();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void TcpNotification::slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int)
{
    for(quint8 index = TCP_TEXTBOX_SERVER; index <= TCP_TEXTBOX_PORT; index++)
    {
        if(textboxes[index]->isEnabled () != state)
        {
            textboxes[index]->setIsEnabled (state);
        }
    }

    if(testConnButton->isEnabled () != state)
    {
        testConnButton->setIsEnabled (state);
    }
}

void TcpNotification::slotTextboxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT));
    }
}
