#include "MediaFileAccess.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(259)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(219) //(558 - 120)/2

// List of control
typedef enum
{    
    FILE_ACCESS_FTP_ENABLE,
    FILE_ACCESS_PORT,
    MAX_FILE_ACCESS_ELEMETS

}FILE_ACCESS_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{    
    FIELD_FTP_ENABLE,
    FIELD_PORT,
    MAX_FIELD_NO

}CNFG_FIELD_NO_e;

static const QString labelStr[MAX_FILE_ACCESS_ELEMETS] =
{    
    "Enable FTP Access",
    "FTP Port"
};

MediaFileAccess::MediaFileAccess(QString devName, QWidget *parent):ConfigPageControl(devName, parent, MAX_FILE_ACCESS_ELEMETS)
{
    createDefaultComponent();
    MediaFileAccess::getConfig();
}

MediaFileAccess::~MediaFileAccess ()
{
    disconnect (enableFtpOpt,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect (enableFtpOpt,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete enableFtpOpt;

    disconnect (ftpportTextbox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (ftpportTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete ftpportTextbox;
    delete ftpPortParam;
}

void MediaFileAccess::createDefaultComponent()
{
    enableFtpOpt = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                          FIRST_ELE_YOFFSET,
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          COMMON_LAYER,
                                          labelStr[FILE_ACCESS_FTP_ENABLE],
                                          "", -1,
                                          FILE_ACCESS_FTP_ENABLE);
    m_elementList[FILE_ACCESS_FTP_ENABLE] = enableFtpOpt;
    connect (enableFtpOpt,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (enableFtpOpt,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableButtonClicked(OPTION_STATE_TYPE_e,int)));

    ftpPortParam = new TextboxParam();
    ftpPortParam->labelStr = labelStr[FILE_ACCESS_PORT];
    ftpPortParam->suffixStr = " (21, 1024-65535)";
    ftpPortParam->isNumEntry = true;
    ftpPortParam->minNumValue = 1024;
    ftpPortParam->maxNumValue = 65535;
    ftpPortParam->extraNumValue = 21;
    ftpPortParam->maxChar = 5;
    ftpPortParam->validation = QRegExp(QString("[0-9]"));

    ftpportTextbox = new TextBox(enableFtpOpt->x (),
                                 enableFtpOpt->y ()+ enableFtpOpt->height (),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 FILE_ACCESS_PORT,
                                 TEXTBOX_SMALL, this, ftpPortParam);
    m_elementList[FILE_ACCESS_PORT] = ftpportTextbox;
    connect (ftpportTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (ftpportTextbox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextboxLoadInfopage(int,INFO_MSG_TYPE_e)));
}

void MediaFileAccess::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             FILE_ACCESS_SERICE_TABLE_INDEX,
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

void MediaFileAccess::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             FILE_ACCESS_SERICE_TABLE_INDEX,
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

void MediaFileAccess::saveConfig()
{
    if(IS_VALID_OBJ(ftpportTextbox))
    {
        if(!ftpportTextbox->doneKeyValidation())
        {
            return;
        }
    }

    payloadLib->setCnfgArrayAtIndex (FIELD_FTP_ENABLE, enableFtpOpt->getCurrentState ());
    payloadLib->setCnfgArrayAtIndex (FIELD_PORT, ftpportTextbox->getInputText ());

    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                             FILE_ACCESS_SERICE_TABLE_INDEX,
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

void MediaFileAccess::processDeviceResponse(DevCommParam *param, QString deviceName)
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
                if(payloadLib->getcnfgTableIndex () == FILE_ACCESS_SERICE_TABLE_INDEX)
                {
                    state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_FTP_ENABLE).toUInt ());
                    enableFtpOpt->changeState (state);

                    ftpportTextbox->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_PORT).toString ());
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

void MediaFileAccess::slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int)
{
    ftpportTextbox->setIsEnabled (state);
}

void MediaFileAccess::slotTextboxLoadInfopage(int ,INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(MEDIA_FILE_ENT_FTP_PORT));
    }
}
