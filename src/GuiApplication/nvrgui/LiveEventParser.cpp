#include "LiveEventParser.h"

LiveEventParser :: LiveEventParser():m_eventTypeIndex(LOG_MAX_EVENT_TYPE),
    m_eventSubTypeIndex((LOG_EVENT_SUBTYPE_e)0), m_eventStateIndex((LOG_EVENT_STATE_e)0),
    m_cameraNumber(0)
{
    m_payloadLib = new PayloadLib();
}

LiveEventParser::~LiveEventParser()
{
    delete m_payloadLib;
}

void LiveEventParser::setEventString(QString eventString, bool isRecordIndexIncluded)
{
    m_eventString = eventString;
    m_payloadLib->parseDevCmdReply(isRecordIndexIncluded, m_eventString);
    parseEventList();
}

QString LiveEventParser::getDateAndTime()
{
    return m_dateTime;
}

QString LiveEventParser::getEventType()
{
    return m_eventType;
}

QString LiveEventParser::getEventSubType()
{
    return m_eventSubType;
}

QString LiveEventParser::getEventState()
{
    return m_eventState;
}

QString LiveEventParser::getEventSource()
{
    return m_eventSource;
}

QString LiveEventParser::getEventAdvanceDetail()
{
    return m_eventAdvanceDetail;
}

QString LiveEventParser::getActualDateAndTime () const
{
    return m_actualEventDateTime;
}

QString LiveEventParser::getExpectedDateAndTime () const
{
    return m_expectedDateAndTime;
}

QString LiveEventParser::getActualDateAndTimeWithSec () const
{
    return m_actualEventDateTimeWithSec;
}

QString LiveEventParser::setDateTime (QString dateTimeString,quint8 secs)
{
    quint32 date = dateTimeString.mid(0, 2).toInt();
    quint32 month = dateTimeString.mid(2, 2).toInt();
    quint32 year = dateTimeString.mid(4, 4).toInt();

    quint32 hour = dateTimeString.mid(8, 2).toInt() ;
    quint32 minute = dateTimeString.mid(10, 2).toInt();

    QDate Date;
    QTime Time;
    QDateTime startDateTime;

    Date.setDate (year, month, date);
    Time.setHMS (hour, minute, 0);

    if(secs > 0)
    {
        Time = Time.addSecs (secs);
    }

    startDateTime.setDate (Date);
    startDateTime.setTime (Time);

    return startDateTime.toString ("ddMMyyyyHHmm");
}

QString LiveEventParser::setDateTimeWithSec (QString dateTimeString)
{
    quint32 date = dateTimeString.mid(0, 2).toInt();
    quint32 month = dateTimeString.mid(2, 2).toInt();
    quint32 year = dateTimeString.mid(4, 4).toInt();

    quint32 hour = dateTimeString.mid(8, 2).toInt() ;
    quint32 minute = dateTimeString.mid(10, 2).toInt();
    quint32 sec = dateTimeString.mid(12, 2).toInt();

    QDate Date;
    QTime Time;
    QDateTime startDateTime;

    Date.setDate (year, month, date);
    Time.setHMS (hour, minute, sec);

    startDateTime.setDate (Date);
    startDateTime.setTime (Time);

    return startDateTime.toString ("ddMMyyyyHHmmss");
}

LOG_EVENT_TYPE_e LiveEventParser::getEventTypeIndex() const
{
    return m_eventTypeIndex;
}

LOG_EVENT_SUBTYPE_e LiveEventParser::getEventSubTypeIndex() const
{
    return m_eventSubTypeIndex;
}

LOG_EVENT_STATE_e LiveEventParser::getEventStateIndex() const
{
    return m_eventStateIndex;
}

quint8 LiveEventParser::getCameraNumber () const
{
    return m_cameraNumber;
}

QString LiveEventParser::parseDateTime(QString dateTimeString)
{
    int date = dateTimeString.mid(0, 2).toInt();
    int month = dateTimeString.mid(2, 2).toInt();
    int year = dateTimeString.mid(4, 4).toInt();

    int hour = dateTimeString.mid(8, 2).toInt() ;
    int minute = dateTimeString.mid(10, 2).toInt();
    int second = dateTimeString.mid(12, 2).toInt();

    QString dateTime = (date >= 10 ? QString("%1").arg(date) : "0" + QString("%1").arg(date))
            + "/"
            + (month >= 10 ? QString("%1").arg(month) : "0" + QString("%1").arg(month))
            + "/"
            + QString("%1").arg(year)
            + " "
            + (hour >= 10 ? QString("%1").arg(hour) : "0" + QString("%1").arg(hour))
            + ":"
            + (minute >= 10 ? QString("%1").arg(minute) : "0" + QString("%1").arg(minute))
            + ":"
            + (second >= 10 ? QString("%1").arg(second) : "0" + QString("%1").arg(second));

    return dateTime;
}

void LiveEventParser::parseEventStateAndSource(LOG_EVENT_TYPE_e eventType,
                                               LOG_EVENT_SUBTYPE_e eventSubType,
                                               LOG_EVENT_STATE_e eventState,
                                               QVariant eventSource)
{
    QString eventStateString = "";
    QString eventSourceString = "";
    m_cameraNumber = (MAX_CAMERAS + 1);

    switch(eventType)
    {
        case LOG_CAMERA_EVENT:
        {
            eventSourceString = "Camera" + QString(" ") + eventSource.toString();
            m_cameraNumber = eventSource.toUInt ();

            switch (eventSubType)
            {
                case LOG_CONNECTIVITY:
                {
                    eventStateString = cameraConnectivityStateString[eventState];
                }
                break;

                case LOG_MANUAL_RECORDING:
                case LOG_ALARM_RECORDING:
                case LOG_SCHEDULE_RECORDING:
                case LOG_SNAPSHOT_SCHEDULE:
                {
                    eventStateString = cameraRecordingStateString[eventState];
                }
                break;

                case LOG_LINE_CROSSING:
                case LOG_INTRUSION_DETECTION:
                case LOG_AUDIO_EXCEPTION_DETECTION:
                {
                    eventStateString = defaultStateString[eventState];
                }
                break;

                case LOG_PRESET_TOUR:
                {
                    eventStateString = cameraPresetTourStateString[eventState];
                }
                break;

                case LOG_IMAGE_SETTING:
                case LOG_STREAM_PARAM_COPY_TO_CAM:
                case LOG_MOTION_DETECTION_COPY_TO_CAM:
                {
                    eventStateString = servicesStateString[eventState];
                }
                break;

                default:
                {
                    eventStateString = defaultStateString[eventState];
                }
                break;
            }
        }
        break;

        case LOG_SENSOR_EVENT:
        {
            eventSourceString = "Sensor" + QString(" ") + eventSource.toString();
            eventStateString = defaultStateString[eventState];
        }
        break;

        case LOG_ALARM_EVENT:
        {
            eventSourceString = "Alarm" + QString(" ") + eventSource.toString();
            eventStateString = defaultStateString[eventState];
        }
        break;

        case LOG_SYSTEM_EVENT:
        {
            eventSourceString = eventSource.toString();
            switch(eventSubType)
            {
                case LOG_POWER_ON:
                case LOG_UNAUTH_IP_ACCESS:
                case LOG_SYSTEM_RESET:
                case LOG_LOGGER_ROLLOVER:
                case LOG_RECORDING_RESTART:
                {
                    eventStateString = systemAlertStateString[eventState];
                }
                break;

                case LOG_RTC_UPDATE:
                {
                     eventStateString = systemAlertStateString[eventState];
                     eventSourceString = rtcUpdateSourceString[eventSource.toInt()];
                }
                break;

                case LOG_MAINS_EVENT:
                {
                    eventStateString = systemMainsStateString[eventState];
                }
                break;

                case LOG_DST_EVENT:
                {
                    eventStateString = defaultStateString[eventState];
                }
                break;

                case LOG_SCHEDULE_BACKUP:
                case LOG_MANUAL_BACKUP:
                {
                    eventStateString = systemBackupStateString[eventState];
                }
                break;

                case LOG_SHUTDOWN:
                case LOG_RESTART:
                {
                    eventStateString = systemAlertStateString[eventState];
                    eventSourceString = systemLogInStateString[eventSource.toUInt ()];
                }
                break;

                case LOG_FIRMWARE_UPGRADE:
                {
                    eventStateString = systemFirmwareUpgradeStateString[eventState];
                }
                break;
                default:
                    break;
            }
        }
        break;

        case LOG_STORAGE_EVENT:
        {
            switch(eventSubType)
            {
                case LOG_HDD_STATUS:
                {
                    if(eventSource != "")
                    {
                        eventSourceString = hddStatusEventSourceString[eventSource.toInt()];
                    }
                    eventStateString = hddStatusStateString[eventState];
                }
                break;

                case LOG_HDD_VOLUME_AT_INIT:
                {
                    eventStateString = hddVolumeAtInitStateString[eventState];
                }
                break;

                case LOG_HDD_VOLUME:
                {
                    eventSourceString = hddVolumeEventSourceString[eventSource.toInt()];
                    eventStateString = hddVolumeStateString[eventState];
                }
                break;

                case LOG_HDD_CLEAN_UP:
                {
                    eventStateString = hddCleanupStateString[eventState];
                }
                break;

                case LOG_HDD_VOL_CLEAN_UP:
                {
                    eventStateString = hddVolCleanupStateString[eventState];
                }
                break;

                default:
                    break;
            }
        }
        break;

        case LOG_NETWORK_EVENT:
        {
            switch(eventSubType)
            {
                case LOG_ETHERNET_LINK:
                {
                    eventSourceString = "Port" + QString(" ") + eventSource.toString();
                    eventStateString = ethernetStateString[eventState];
                }
                break;

                case LOG_IP_ASSIGN:
                {
                    eventSourceString = eventSource.toString();
                    eventStateString = ipAssignStateString[eventState];
                }
                break;

                case LOG_DDNS_IP_UPDATE:
                case LOG_MAC_SERVER_UPDATE:
                {
                    eventSourceString = eventSource.toString();
                    eventStateString = systemAlertStateString[eventState];
                }
                break;

                case LOG_UPLOAD_IMAGE:
                {
                    if(eventSource != "")
                    {
                        eventSourceString = "Camera" + QString(" ") + eventSource.toString();
                        m_cameraNumber = eventSource.toUInt ();
                    }
                }
                /* FALLS THROUGH */
                case LOG_EMAIL_NOTIFICATION:
                case LOG_TCP_NOTIFICATION:
                case LOG_SMS_NOTIFICATION:
                {
                    eventStateString = servicesStateString[eventState];
                }
                break;

                case LOG_MODEM_STATUS:
                case LOG_P2P_STATUS:
                {
                    eventStateString = modemStateString[eventState];
                }
                break;

                case LOG_DHCP_SERVER_IP_ASSIGN:
                case LOG_DHCP_SERVER_IP_EXPIRE:
                {
                    eventSourceString = "LAN " + eventSource.toString();
                    eventStateString = systemAlertStateString[eventState];
                }
                break;

                default:
                    break;
            }
        }
        break;

        case LOG_OTHER_EVENT:
        {
            switch(eventSubType)
            {
                case LOG_UPGRADE_START:
                {
                    eventStateString = systemAlertStateString[eventState];
                }
                break;

                case LOG_UPGRADE_RESULT:
                {
                    eventStateString = servicesStateString[eventState];
                }
                break;

                case LOG_BUZZER_STATUS:
                {
                    if(eventSource != "")
                    {
                        eventSourceString = "Camera" + QString(" ") + eventSource.toString();
                    }
                    eventStateString = buzzerStateString[eventState];
                }
                break;

                case LOG_RESTORE_CONFIG_STRAT:
                {
                    eventStateString = systemAlertStateString[eventState];
                }
                break;

                case LOG_RESTORE_CONFIG_RESULT:
                {
                    eventStateString = servicesStateString[eventState];
                }
                break;

                default:
                    break;
            }
        }
        break;

        case LOG_USER_EVENT:
        {
            eventSourceString = eventSource.toString();
            switch(eventSubType)
            {
                case LOG_USER_SESSION:
                {
                    eventStateString = sessionStateString[eventState];
                }
                break;

                case LOG_MANUAL_TRIGGER:
                {
                    eventStateString = defaultStateString[eventState];
                }
                break;

                case LOG_CONFIG_CHANGE:
                {
                    eventSourceString = configChangeEventSourceString[eventSource.toInt()];
                    eventStateString = configChangeStateString[eventState];
                }
                break;

                case LOG_SYS_CONFIGURATION:
                {
                    eventSourceString = systemConfigChangeEventSourceDetail[eventSource.toInt()];
                    eventStateString = systemConfigurationStateString[eventState];
                }
                break;

                case LOG_FIRMWARE:
                {
                    eventStateString = firmwareStateString[eventState];
                }
                break;

                case LOG_LOGIN_REQUEST:
                case LOG_PASSWORD_RESET:
                {
                    eventStateString = servicesStateString[eventState];
                }
                break;

                default:
                    break;
            }
        }
        break;

        case LOG_COSEC_EVENT:
        {
            eventSourceString = "Camera" + QString(" ") + eventSource.toString();
            switch(eventSubType)
            {
                case LOG_COSEC_RECORDING:
                {
                    eventStateString = cameraRecordingStateString[eventState];
                }
                break;

                case LOG_COSEC_VIDEO_POP_UP:
                {
                    eventStateString = cosecStateString[eventState];
                }
                break;

                default:
                    break;
            }
        }
        break;

        default:
            break;
    }

    m_eventSource = eventSourceString;
    m_eventState = eventStateString;
}

void LiveEventParser::parseEventList()
{
    m_dateTime = parseDateTime(m_payloadLib->getCnfgArrayAtIndex(LV_EVT_DATE_TIME).toString());

    m_actualEventDateTime =  setDateTime(m_payloadLib->getCnfgArrayAtIndex(LV_EVT_DATE_TIME).toString());
    m_expectedDateAndTime =  setDateTime(m_payloadLib->getCnfgArrayAtIndex(LV_EVT_DATE_TIME).toString(),60);
    m_actualEventDateTimeWithSec = setDateTimeWithSec (m_payloadLib->getCnfgArrayAtIndex(LV_EVT_DATE_TIME).toString());

    LOG_EVENT_TYPE_e eventType = (LOG_EVENT_TYPE_e)m_payloadLib->getCnfgArrayAtIndex(LV_EVT_TYPE).toInt();
    LOG_EVENT_SUBTYPE_e eventSubType =  (LOG_EVENT_SUBTYPE_e)m_payloadLib->getCnfgArrayAtIndex(LV_EVT_SUB_TYPE).toInt();
    QVariant eventSource = m_payloadLib->getCnfgArrayAtIndex(LV_EVT_DETAIL);
    LOG_EVENT_STATE_e eventState = (LOG_EVENT_STATE_e)m_payloadLib->getCnfgArrayAtIndex(LV_EVT_STATE).toInt();

    m_eventStateIndex = eventState;

    m_eventType = eventStruct[eventType].eventString;
    m_eventTypeIndex = eventType;

    m_eventSubType = eventStruct[eventType].eventSubtypeString[eventSubType];
    m_eventSubTypeIndex = eventSubType;

    parseEventStateAndSource(eventType, eventSubType, eventState, eventSource);

    m_eventAdvanceDetail = m_payloadLib->getCnfgArrayAtIndex(LV_EVT_ADV_DETAIL).toString();
}
