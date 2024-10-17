#include "AlaramOutput.h"

static const QString alarmStatusStrings[] = { "DeActivate",
                                              "Activate",
                                              "Unknown" };

AlaramOutput::AlaramOutput(QString devName,
                           QWidget *parent)
    :ManageMenuOptions(devName, parent, MAX_DEV_ALARM)
{

    QStringList CnfgButtonName;
    CnfgButtonName.clear();
    ApplController* applcontroller = ApplController::getInstance();
    m_maxAlarmOutput = 0;
    applcontroller->GetMaxAlarmOutputSupport(devName, m_maxAlarmOutput);
    m_maxElements = m_maxAlarmOutput;

    for(quint8 index = 0 ; index < m_maxAlarmOutput ; index++)
    {
        CnfgButtonName.append( "Alarm " + QString("%1").arg(index+1));
    }

    for(quint8 index = 0; index < m_maxAlarmOutput; index++)
    {
        m_elementList[index] = NULL;
    }

    for(quint8 index = 0; index < m_maxAlarmOutput ; index++)
    {
        m_bgTile[index] = new BgTile( ((this->width() - BGTILE_SMALL_SIZE_WIDTH) / 2) ,
                                      (index== 0?
                                           ((this->height()) -
                                            (m_maxAlarmOutput*BGTILE_HEIGHT))/2
                                         : (m_bgTile[index-1]->y()+
                                            m_bgTile[index-1]->height())),
                                      BGTILE_SMALL_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      COMMON_LAYER,
                                      this);

        m_cnfgheading[index] = new Heading ((m_bgTile[index]->x () + SCALE_WIDTH(140)),
                                            (m_bgTile[index]->y () + (BGTILE_HEIGHT / 2)),
                                            CnfgButtonName.at(index),
                                            this,
                                            HEADING_TYPE_2);

        m_cnfgButton[index] = new CnfgButton(CNFGBUTTON_MEDIAM,
                                             m_bgTile[index]->x () + (BGTILE_SMALL_SIZE_WIDTH / 2) + SCALE_WIDTH(60),
                                             m_bgTile[index]->y () + (BGTILE_HEIGHT / 2),
                                             alarmStatusStrings[ALARM_ACTIVATE],
                                             this,
                                             index);

        m_elementList[index] = m_cnfgButton[index];
        connect (m_cnfgButton[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotAlaramActivate(int)) );
        connect(m_cnfgButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    }

    getAlarmStatus(m_currentDeviceName);
    this->show();
}

AlaramOutput :: ~AlaramOutput()
{
    for(quint8 index = 0; index < m_maxAlarmOutput ; index++)
    {
        disconnect(m_cnfgButton[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_cnfgButton[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotAlaramActivate(int)) );
        delete m_cnfgButton[index];

        delete m_cnfgheading[index];
        delete m_bgTile[index];
    }
}

void AlaramOutput :: triggerAlaram(QString devName,quint8 index)
{
    m_payloadLib->setCnfgArrayAtIndex (0,index + 1);
    m_payloadLib->setCnfgArrayAtIndex (1,alarmStatus[index]);
    QString payloadString = m_payloadLib->createDevCmdPayload(2);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = ALRM_OUT;
    param->payload = payloadString;

    processBar->loadProcessBar ();

    if(m_applController->processActivity(devName, DEVICE_COMM, param) == true)
    {
        m_currentDeviceName = devName;
    }
}

void AlaramOutput:: updateStatus (QString deviceName, qint8 status, qint8 index,quint8 eventType,
                                  quint8 eventSubType)
{

    if((m_currentDeviceName == deviceName) &&
            (eventType == LOG_ALARM_EVENT) &&
            (eventSubType == LOG_ALARM_OUTPUT) )
    {
        alarmStatus[index] =  status;
        changeText(index);
    }
}

void AlaramOutput:: changeText(quint8 index)
{
    switch( alarmStatus[index])
    {
    case ALARM_ACTIVATE:
    {
        m_cnfgButton[index]->changeText (alarmStatusStrings[ALARM_DEACTIVATE]);
    }
        break;
    case ALARM_DEACTIVATE:
    {
        m_cnfgButton[index]->changeText (alarmStatusStrings[ALARM_ACTIVATE]);
    }
        break;

    default:
        m_cnfgButton[index]->changeText (alarmStatusStrings[ALARM_UNKNOWN]);
    }
    m_cnfgButton[index]->update ();
}

void AlaramOutput :: getAlarmStatus(QString deviceName)
{
    if(m_applController->GetHlthStatusSingleParam(deviceName, alarmStatus,
                                                  ALARM_STS))
    {
        for(quint8 index = 0; index < m_maxAlarmOutput ; index++)
        {
            changeText(index);
        }
    }
}

void AlaramOutput::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName == m_currentDeviceName)
            && (param->msgType == MSG_SET_CMD))
    {
        m_payloadLib->parseDevCmdReply(true, param->payload);
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            break;

        default:
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
    processBar->unloadProcessBar ();
}

void AlaramOutput :: slotAlaramActivate(int index)
{
    if (alarmStatus[index] == ALARM_DEACTIVATE)
    {
        alarmStatus[index] =  ALARM_ACTIVATE;
    }
    else
    {
        alarmStatus[index] = ALARM_DEACTIVATE;
    }

    triggerAlaram(m_currentDeviceName,index);
}
