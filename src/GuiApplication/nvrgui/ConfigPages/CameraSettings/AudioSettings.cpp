#include "AudioSettings.h"
#include "ValidationMessage.h"

#define AUDIO_OUT_HEADING_STR   "Audio Out"
#define AUDIO_OUT_PRIORITY_STR  "Priority"

#define MAX_CLIENT      2
#define CNFG_TO_INDEX   1

// cnfg field no According to CMS comm. module table no-57
typedef enum
{
    AUDIO_OUT_PRIORITY,
    MAX_AUDIO_OUT_SETTING
}AUDIO_OUT_SETTING_e;

static const QString audioOutClient[MAX_CLIENT] = {"Client", "Camera"};

AudioSettings::AudioSettings(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent, MAX_AUDIO_OUT_SETTING)
{
    QMap<quint8, QString> audioOutClientList;

    audioOutClientList.clear();
    for(quint8 index = 0; index < MAX_CLIENT ; index++)
    {
        audioOutClientList.insert(index, audioOutClient[index]);
    }

    audioOutSettingTile = new ElementHeading(SCALE_WIDTH(8),
                                             SCALE_HEIGHT(20),
                                             BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(10),
                                             BGTILE_HEIGHT,
                                             AUDIO_OUT_HEADING_STR,
                                             TOP_LAYER, this,
                                             false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    audioOutPlay = new DropDown(audioOutSettingTile->x(),
                                audioOutSettingTile->y() + audioOutSettingTile->height(),
                                audioOutSettingTile->width(),
                                BGTILE_HEIGHT,
                                AUDIO_OUT_PRIORITY,
                                DROPDOWNBOX_SIZE_114,
                                AUDIO_OUT_PRIORITY_STR,
                                audioOutClientList,
                                this,
                                "",
                                true,
                                0,
                                BOTTOM_TABLE_LAYER,
                                true, 4);
    m_elementList[AUDIO_OUT_PRIORITY] = audioOutPlay;
    connect(audioOutPlay,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    AudioSettings::getConfig();
    Q_UNUSED(devTabInfo);
}

AudioSettings::~AudioSettings()
{
    disconnect(audioOutPlay,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(audioOutPlay);
    DELETE_OBJ(audioOutSettingTile);
}

void AudioSettings::createPayload(REQ_MSG_ID_e msgType )
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             AUDIO_OUT_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_AUDIO_OUT_SETTING,
                                                             MAX_AUDIO_OUT_SETTING);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void AudioSettings::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void AudioSettings::getConfig()
{
    createPayload(MSG_GET_CFG);
}

void AudioSettings::saveConfig()
{
    payloadLib->setCnfgArrayAtIndex(0, audioOutPlay->getIndexofCurrElement() + 1);
    createPayload(MSG_SET_CFG);
}

void AudioSettings::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex(0) == AUDIO_OUT_TABLE_INDEX)
            {
                audioOutPlay->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex(0).toUInt() - 1);
            }
            processBar->unloadProcessBar();
        }
        break;

        case MSG_SET_CFG:
        {
            processBar->unloadProcessBar();
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            getConfig();
        }
        break;

        case MSG_DEF_CFG:
        {
            processBar->unloadProcessBar();
            getConfig();
        }
        break;

        default:
        {
            // Nothing to do
        }
        break;
    }
}
