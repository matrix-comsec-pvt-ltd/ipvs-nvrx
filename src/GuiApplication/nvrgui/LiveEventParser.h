#ifndef LIVEEVENTPRASER_H
#define LIVEEVENTPRASER_H

#include <QObject>
#include "EnumFile.h"
#include "PayloadLib.h"

typedef struct
{
    LOG_EVENT_SUBTYPE_e maxSubEventType;
    const QString eventString;
    const QString* eventSubtypeString;
}EVENT_TYPE_t;

static const QString eventTypeString[LOG_MAX_EVENT_TYPE] =
{
    "",
    "Camera",
    "Sensor",
    "Alarm",
    "System",
    "Storage",
    "Network",
    "Other",
    "User",
    "COSEC"
};

static const QString anyEventSubTypeString[1] =
{
    ""
};

static const QString cameraEventSubTypeString[LOG_MAX_CAMERA_EVENT] =
{
    "",
    "Motion Detection",
    "View Tampering",
    "Camera Sensor 1",
    "Camera Sensor 2",
    "Camera Sensor 3",
    "Camera Alarm 1",
    "Camera Alarm 2",
    "Camera Alarm 3",
    "Camera Status",
    "Manual Recording",
    "Alarm Recording",
    "Schedule Recording",
    "Preset Tour",
    "Snapshot Schedule",
    "Trip Wire",
    "Object Intrusion",
    "Audio Exception",
    "",
    "",
    "Missing Object",
    "Suspicious Object",
    "Loitering",
    "Camera Status",
    "Object Counting",
    "Copy Image Settings",
    "Copy Stream Settings",
    "Copy Motion Detection",
    "No Motion Detection"
};

static const QString sensorEventSubTypeString[LOG_MAX_SENSOR_EVENT] =
{
    "",
    "Sensor Input"
};

static const QString alarmEventSubTypeString[LOG_MAX_ALARM_EVENT] =
{
    "",
    "Alarm Output"
};

static const QString systemEventSubTypeString[LOG_MAX_SYSTEM_EVENT] =
{
    "",
    "Power On",
    "Mains",
    "Unauthorized IP Access",
    "RTC Update",
    "DST",
    "Schedule Backup",
    "Manual Backup",
    "System Reset",
    "Shutdown",
    "Log Rollover",
    "Restart",
    "",
    "",
    "",
    "",
    "Recording Restart",
    "Firmware Upgrade",
};

static const QString storageEventSubTypeString[LOG_MAX_STORAGE_EVENT] =
{
    "",
    "Storage Status",
    "Logical Volume Init.",
    "Logical Volume",
    "Storage Clean-up",
    "Disk Cleanup"
};

static const QString networkEventSubTypeString[LOG_MAX_NETWORK_EVENT] =
{
    "",
    "Ethernet Link",
    "IP Assign",
    "DDNS IP Update",
    "Upload Image",
    "Email Notification",
    "TCP Notification",
    "SMS Notification",
    "Matrix DNS IP Update",
    "Modem",
    "P2P Connection",
    "DHCP Server - IP Assign",
    "DHCP Server - IP Expire",
};

static const QString otherEventSubTypeString[LOG_MAX_OTHER_EVENT] =
{
    "",
    "Upgrade Process To Start",
    "",
    "",
    "",
    "Upgrade Result",
    "Restore Process To Start",
    "",
    "",
    "",
    "Restore Result",
    "Buzzer",
    "",
    ""
};

static const QString userEventSubTypeString[LOG_MAX_USER_EVENT] =
{
    "",
    "User",
    "Manual Trigger",
    "Configuration",
    "Default",
    "Firmware",
    "Login Request",
    "",
    "",
    "Password Reset",
};

static const QString cosecEventSubTypeString[LOG_MAX_COSEC_EVENT] =
{
    "",
    "COSEC Recording",
    "COSEC Video Pop-up"
};

//event state string
//default event
static const QString defaultStateString[] = {"Normal",
                                             "Active"};
//related to camera event
static const QString cameraConnectivityStateString[] = {"Camera Offline",
                                                        "Camera Online"};

static const QString cameraRecordingStateString[] = {"Stop",
                                                     "Start",
                                                     "Fail"};

static const QString cameraPresetTourStateString[] = {"Stop",
                                                      "Auto",
                                                      "Manual",
                                                      "Pause",
                                                      "Resume"};

//related to system event
static const QString systemAlertStateString[] = {"",
                                                 "Alert"};

static const QString systemMainsStateString[] = {"Fail",
                                                 "Active",
                                                 "Unknown"};

static const QString systemBackupStateString[] = {"Stop",
                                                  "Start",
                                                  "Complete",
                                                  "Fail "};

static const QString systemLogInStateString[] = {"Remote",
                                                 "Auto",
                                                 "Local"};

static const QString systemFirmwareUpgradeStateString[] = {"",
                                                           "Available"};

//related to storage event
static const QString hddStatusStateString[] = {"Normal",
                                               "No Disk",
                                               "Full",
                                               "Low Memory",
                                               "Fault",
                                               "Busy",
                                               "Read Only"};

static const QString hddVolumeAtInitStateString[] = {"Incomplete Volume",
                                                     "Missing Disk",
                                                     "Missing Volume"};

static const QString hddVolumeStateString[] = {"Creating",
                                               "Format"};

static const QString hddCleanupStateString[] = {"Regular",
                                                "Backup File",
                                                "Full"};

static const QString hddVolCleanupStateString[] = {"Stop",
                                                   "Start"};
//related to network event
static const QString ethernetStateString[] = {"Up",
                                              "Down"};

static const QString ipAssignStateString[] = {"PPPoE",
                                              "DHCP",
                                              "SLAAC"};

static const QString servicesStateString[] = {"Fail",
                                              "Success"};

static const QString modemStateString[] = {"Up",
                                           "Down"};

// related to Other event
static const QString buzzerStateString[] = {"Normal",
                                            "Ringing"};

static const QString scheduleBackupString[] = {"No disk found."};

//related to user event
static const QString sessionStateString[] = {"Log-Out",
                                             "Log-In"};

static const QString configChangeStateString[] = {"Default",
                                                  "Change"};

static const QString systemConfigurationStateString[] = {"",
                                                         "Start",
                                                         "Fail",
                                                         "Success"};
static const QString firmwareStateString[] = {"",
                                              "Upgrade"};

//related to cosec event
static const QString cosecStateString[] = {"Active"};

//event source string
static const QString hddVolumeEventSourceString[] = {"",
                                                     "Single Disk Volume 1",
                                                     "Single Disk Volume 2",
                                                     "Single Disk Volume 3",
                                                     "Single Disk Volume 4",
                                                     "Single Disk Volume 5",
                                                     "Single Disk Volume 6",
                                                     "Single Disk Volume 7",
                                                     "Single Disk Volume 8",
                                                     "RAID 0 Disk 1 2",
                                                     "RAID 0 Disk 3 4",
                                                     "RAID 0 Disk 5 6",
                                                     "RAID 0 Disk 7 8",
                                                     "RAID 1 Disk 1 2",
                                                     "RAID 1 Disk 3 4",
                                                     "RAID 1 Disk 5 6",
                                                     "RAID 1 Disk 7 8",
                                                     "RAID 5 Disk 1 2 3 4",
                                                     "RAID 5 Disk 5 6 7 8",
                                                     "RAID 10 Disk 1 2 3 4",
                                                     "RAID 10 Disk 5 6 7 8",
                                                     "Network Drive 1",
                                                     "Network Drive 2"};

static const QString hddStatusEventSourceString[] = {"",
                                                     "Single Disk Volume",
                                                     "RAID 0",
                                                     "RAID 1",
                                                     "RAID 5",
                                                     "RAID 10",
                                                     "Network Drive 1",
                                                     "Network Drive 2"};

static const QString systemConfigChangeEventSourceDetail[] = {"",
                                                              "Network",
                                                              "User Account",
                                                              "Configuration"};

static const QString configChangeEventSourceString[] = {"",
                                                        "General",                      // table 01
                                                        "Date & Time",                  // table 02
                                                        "Daylight Saving Time",         // table 03
                                                        "COM Port",                     // table 04
                                                        "LAN 1",                        // table 05
                                                        "LAN 2",                        // table 06
                                                        "IP Address Filtering",         // table 07
                                                        "IP Address Filtering",         // table 08
                                                        "DDNS Client",                  // table 09
                                                        "Email Client",                 // table 10
                                                        "FTP Client",                   // table 11
                                                        "TCP Client",                   // table 12
                                                        "Media-File Access",            // table 13
                                                        "HDD",                          // table 14
                                                        "Matrix DNS Client",            // table 15
                                                        "User Account",                 // table 16
                                                        "Camera",                       // table 17
                                                        "Stream",                       // table 18
                                                        "Schedule Recording",           // table 19
                                                        "Schedule Recording",           // table 20
                                                        "Alarm Recording",              // table 21
                                                        "Preset Position",              // table 22
                                                        "Preset Tour Manual",           // table 23
                                                        "Preset Tour Auto",             // table 24
                                                        "Preset Tour Schedule",         // table 25
                                                        "Device Sensor Input",          // table 26
                                                        "Device Alarm Output",          // table 27
                                                        "Image Upload",                 // table 28
                                                        "Storage",                      // table 29
                                                        "Backup",                       // table 30
                                                        "Backup",                       // table 31
                                                        "Camera Event & Action",        // table 32
                                                        "Camera Event & Action",        // table 33
                                                        "Sensor Event & Action",        // table 34
                                                        "Sensor Event & Action",        // table 35
                                                        "System Event & Action",        // table 36
                                                        "COSEC Recording",              // table 37
                                                        "Camera Alarm Output",          // table 38
                                                        "Static Routing",               // table 39
                                                        "Static Routing",               // table 40
                                                        "Broadband",                    // table 41
                                                        "Broadband",                    // table 42
                                                        "SMS Setting",                  // table 43
                                                        "Manual Recording",             // table 44
                                                        "Network Drive",                // table 45
                                                        "IP Camera",                    // table 46
                                                        "Analog Camera",                // table 47
                                                        "PTZ Interface Settings",       // table 48
                                                        "Audio Settings",               // table 49
                                                        "Monitor Client Settings",      // table 50
                                                        "Network Devices",              // table 51
                                                        "Snapshot Schedule",            // table 52
                                                        "Snapshot Schedule",            // table 53
                                                        "Password Policy",              // table 54
                                                        "Audio Settings",               // table 55
                                                        "P2P",                          // table 56
                                                        "Image Settings",               // table 57
                                                        "DHCP Server",                  // table 58
                                                        "Firmware Management",          // table 59
                                                        "Push Notification Status",     // table 60
                                                        "Password Recovery",            // table 61
                                                        "HDD Group",                    // table 62
                                                       };

static const QString rtcUpdateSourceString[] = {"Manual",
                                                "NTP",
                                                "Time Zone Change"};

static const EVENT_TYPE_t eventStruct[LOG_MAX_EVENT_TYPE] =
{
    {
        (LOG_EVENT_SUBTYPE_e)1,
        eventTypeString[LOG_ANY_EVENT],
        anyEventSubTypeString,
    },
    {
        LOG_MAX_CAMERA_EVENT,
        eventTypeString[LOG_CAMERA_EVENT],
        cameraEventSubTypeString,
    },
    {
        LOG_MAX_SENSOR_EVENT,
        eventTypeString[LOG_SENSOR_EVENT],
        sensorEventSubTypeString,
    },
    {
        LOG_MAX_ALARM_EVENT,
        eventTypeString[LOG_ALARM_EVENT],
        alarmEventSubTypeString,
    },
    {
        LOG_MAX_SYSTEM_EVENT,
        eventTypeString[LOG_SYSTEM_EVENT],
        systemEventSubTypeString,
    },
    {
        LOG_MAX_STORAGE_EVENT,
        eventTypeString[LOG_STORAGE_EVENT],
        storageEventSubTypeString,
    },
    {
        LOG_MAX_NETWORK_EVENT,
        eventTypeString[LOG_NETWORK_EVENT],
        networkEventSubTypeString,
    },
    {
        LOG_MAX_OTHER_EVENT,
        eventTypeString[LOG_OTHER_EVENT],
        otherEventSubTypeString,
    },
    {
        LOG_MAX_USER_EVENT,
        eventTypeString[LOG_USER_EVENT],
        userEventSubTypeString,
    },
    {
        LOG_MAX_COSEC_EVENT,
        eventTypeString[LOG_COSEC_EVENT],
        cosecEventSubTypeString,
    }
};

class LiveEventParser : public QObject
{
    Q_OBJECT
private:
    PayloadLib* m_payloadLib;

    QString m_dateTime;
    QString m_eventType;
    QString m_eventSubType;
    QString m_eventState;
    QString m_eventSource;
    QString m_eventAdvanceDetail;

    QString m_eventString;

    LOG_EVENT_TYPE_e     m_eventTypeIndex;
    LOG_EVENT_SUBTYPE_e  m_eventSubTypeIndex;
    LOG_EVENT_STATE_e    m_eventStateIndex;

    QString m_actualEventDateTime;
    QString m_actualEventDateTimeWithSec;
    QString m_expectedDateAndTime;
    quint8  m_cameraNumber;

public:
    LiveEventParser();
    ~LiveEventParser();

    void setEventString(QString eventString, bool isRecordIndexIncluded = false);

    QString getDateAndTime();
    QString getEventType();
    QString getEventSubType();
    QString getEventState();
    QString getEventSource();
    QString getEventAdvanceDetail();

    quint8 getCameraNumber() const;
    QString getActualDateAndTimeWithSec() const;
    QString getActualDateAndTime() const;
    QString getExpectedDateAndTime() const;
    LOG_EVENT_TYPE_e getEventTypeIndex() const;
    LOG_EVENT_SUBTYPE_e getEventSubTypeIndex() const;
    LOG_EVENT_STATE_e getEventStateIndex() const;

    void parseEventList();
    QString setDateTime(QString dateTimeString, quint8 addSecs = 0);
    QString setDateTimeWithSec(QString dateTimeString);
    QString parseDateTime(QString dateTimeString);
    void parseEventStateAndSource(LOG_EVENT_TYPE_e eventType,
                                  LOG_EVENT_SUBTYPE_e eventSubType,
                                  LOG_EVENT_STATE_e eventState,
                                  QVariant eventSource);
};

#endif //LIVEEVENTPRASER_H
