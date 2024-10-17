#include "MatrixDnsClient.h"
#include "ValidationMessage.h"

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(194)

#define CNFG_TO_INDEX               1

// List of control
typedef enum
{
    MTRXDNS_ENABLE,
    MTRXDNS_HOST_NAME,
    MTRXDNS_PORT,
    MTRXDNS_REG_BUTTON,

    MAX_MTRXDNS_ELEMETS
}MTRXDNS_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_MTRX_DNS_ENABLE,
    FIELD_HOST_NAME,
    FIELD_FWD_PORT,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_MTRXDNS_ELEMETS] =
{
    "Enable Matrix DNS Service",
    "Host Name",
    "Forwarded Port",
    "Register"
};

MatrixDnsClient::MatrixDnsClient(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_MTRXDNS_ELEMETS)
{
    createDefaultComponent();
    MatrixDnsClient::getConfig();
}

MatrixDnsClient::~MatrixDnsClient()
{
    disconnect (enableCheckbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (enableCheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete enableCheckbox;

    disconnect (hostNameTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (hostNameTextbox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    delete hostNameTextbox;
    delete hostNameParam;

    disconnect (portTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete portTextbox;
    delete portParam;

    disconnect (regButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (regButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotRegBtnButtonClick(int)));
    delete regButton;
}

void MatrixDnsClient::createDefaultComponent()
{
    enableCheckbox = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                            FIRST_ELE_YOFFSET,
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            labelStr[MTRXDNS_ENABLE],
                                            this,
                                            TOP_LAYER,
                                            SCALE_HEIGHT(10),
                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                            NORMAL_FONT_SIZE,
                                            MTRXDNS_ENABLE, true, NORMAL_FONT_COLOR, true);
    m_elementList[MTRXDNS_ENABLE] = enableCheckbox;
    connect (enableCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect (enableCheckbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    hostNameParam = new TextboxParam();
    hostNameParam->labelStr = labelStr[MTRXDNS_HOST_NAME];
    hostNameParam->minChar = 3;
    hostNameParam->maxChar = 30;
    hostNameParam->isTotalBlankStrAllow = true;
    hostNameParam->validation = QRegExp(QString("[a-zA-Z0-9_ ]"));
    hostNameParam->startCharVal = QRegExp(QString("[a-zA-Z]"));

    hostNameTextbox = new TextBox(enableCheckbox->x (),
                                  enableCheckbox->y () + enableCheckbox->height (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  MTRXDNS_HOST_NAME,
                                  TEXTBOX_ULTRALARGE, this,
                                  hostNameParam, MIDDLE_TABLE_LAYER,
                                  true, false, false, SCALE_WIDTH(60));
    m_elementList[MTRXDNS_HOST_NAME] = hostNameTextbox;
    connect (hostNameTextbox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (hostNameTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    portParam = new TextboxParam();
    portParam->labelStr = labelStr[MTRXDNS_PORT];
    portParam->suffixStr = "(1-65535)";
    portParam->textStr = "80";

    portParam->isNumEntry = true;
    portParam->maxChar = 5;
    portParam->minNumValue = 1;
    portParam->maxNumValue = 65535;
    portParam->validation = QRegExp(QString("[0-9]"));

    portTextbox = new TextBox(hostNameTextbox->x (),
                              hostNameTextbox->y () + hostNameTextbox->height (),
                              BGTILE_MEDIUM_SIZE_WIDTH,
                              BGTILE_HEIGHT,
                              MTRXDNS_PORT, TEXTBOX_SMALL,
                              this, portParam,
                              MIDDLE_TABLE_LAYER, true, false, false,
                              SCALE_WIDTH(60));
    m_elementList[MTRXDNS_PORT] = portTextbox;
    connect (portTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    regButton = new ControlButton(REGISTER_BUTTON_INDEX,
                                  portTextbox->x (),
                                  portTextbox->y () + portTextbox->height (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  this, DOWN_LAYER,
                                  SCALE_WIDTH(360), labelStr[MTRXDNS_REG_BUTTON],
                                  true, MTRXDNS_REG_BUTTON);
    m_elementList[MTRXDNS_REG_BUTTON] = regButton;
    connect (regButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotRegBtnButtonClick(int)));
    connect (regButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
}
void MatrixDnsClient::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             MATRIX_DNS_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void MatrixDnsClient::defaultConfig()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             MATRIX_DNS_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void MatrixDnsClient::saveConfig()
{
    QString tempHostName = hostNameTextbox->getInputText ();
    QString tempPort = portTextbox->getInputText ();

    if((tempPort == "") && (enableCheckbox->getCurrentState () == ON_STATE))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT));
    }
    else if((saveHostName != tempHostName) || (savePort != tempPort))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MATRIX_DNS_REG_HOST_NAME));
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (FIELD_MTRX_DNS_ENABLE,
                                         enableCheckbox->getCurrentState ());
        payloadLib->setCnfgArrayAtIndex (FIELD_HOST_NAME, tempHostName);
        payloadLib->setCnfgArrayAtIndex (FIELD_FWD_PORT, tempPort);

        //create the payload for Get Cnfg
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 MATRIX_DNS_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_TO_INDEX,
                                                                 CNFG_FRM_FIELD,
                                                                 MAX_FIELD_NO,
                                                                 MAX_FIELD_NO);

        DevCommParam* param = new DevCommParam();

        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;

        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void MatrixDnsClient::processDeviceResponse(DevCommParam *param, QString deviceName)
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
                if(payloadLib->getcnfgTableIndex () == MATRIX_DNS_TABLE_INDEX)
                {
                    state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_MTRX_DNS_ENABLE).toUInt ());
                    enableCheckbox->changeState (state);

                    saveHostName = payloadLib->getCnfgArrayAtIndex (FIELD_HOST_NAME).toString ();
                    savePort = payloadLib->getCnfgArrayAtIndex (FIELD_FWD_PORT).toString ();

                    hostNameTextbox->setInputText (saveHostName);
                    portTextbox->setInputText (savePort);

                    hostNameTextbox->setIsEnabled (state);
                    portTextbox->setIsEnabled (state);
                    regButton->setIsEnabled (state);
                }
                break;

            case MSG_SET_CFG:
                saveHostName = hostNameTextbox->getInputText ();
                savePort = portTextbox->getInputText ();
                //load info page with msg
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            case MSG_SET_CMD:
                if(param->cmdType == UPDT_HOST_NM)
                {
                    saveHostName = hostNameTextbox->getInputText ();
                    savePort = portTextbox->getInputText ();
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MATRIX_DNS_REG_SUCCESS));
                }
                break;

            default:
                break;
            }
            break;

        default:
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
        update();
    }
}

void MatrixDnsClient::slotEnableButtonClicked(OPTION_STATE_TYPE_e state, int)
{
    hostNameTextbox->setIsEnabled (state);
    portTextbox->setIsEnabled (state);
    regButton->setIsEnabled (state);
    if(state == OFF_STATE)
    {
        hostNameTextbox->setInputText (saveHostName);
        portTextbox->setInputText (savePort);
    }
}

void MatrixDnsClient::slotTextboxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    QString tempStr = "";
    switch(msgType)
    {
    case INFO_MSG_STRAT_CHAR:
        tempStr = ValidationMessage::getValidationMessage(START_CHAR_ERROR_MSG);
        break;

    case INFO_MSG_ERROR:
        tempStr = ValidationMessage::getValidationMessage(MATRIX_DNS_HOST_NAME_ATLEAST_3_CHAR);
        break;

    default:
        break;
    }
    infoPage->loadInfoPage (tempStr);
}

void MatrixDnsClient::slotRegBtnButtonClick(int)
{
    QString tempPort = portTextbox->getInputText ();

    if((tempPort == "") && (enableCheckbox->getCurrentState () == ON_STATE))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MATRIX_DNS_ENT_VALID_PORT));
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (0, hostNameTextbox->getInputText ());
        payloadLib->setCnfgArrayAtIndex (1, portTextbox->getInputText ());

        QString payloadString = payloadLib->createDevCmdPayload(2);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = UPDT_HOST_NM;
        param->payload = payloadString;

        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}
