#ifndef SensorEventAndAction_H
#define SensorEventAndAction_H

#include <QWidget>

#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ScheduleBar.h"
#include "Controls/PageOpenButton.h"

#include "ConfigPages/CameraSettings/CopyToCamera.h"
#include "ConfigPages/EventAndActionSettings/EventActionSchedule.h"
#include "ConfigPages/EventAndActionSettings/EventActionEmailNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionTCPNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionSmsNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionPtzPos.h"
#include "ConfigPages/EventAndActionSettings/EventActionDeviceAlarm.h"
#include "ConfigPages/EventAndActionSettings/EventCameraAlarmOutput.h"

#define MAX_SENS_SCHD_TIME_SLOTS      6
#define MAX_SENS_EVNT_WEEKDAYS        MAX_WEEKDAYS
#define MAX_SENS_EVNT_EVENTS          (MAX_EVNT_SCHD_EVNT - 1)	/* Sensor doesn't have video popup action */
#define MAX_SENS_EVNT_SCHEDULE_BAR    MAX_SENS_EVNT_EVENTS + 1
#define MAX_SENS_EVNT_ACTIONS_STATUS  (MAX_SENS_EVNT_WEEKDAYS*MAX_SENS_EVNT_WEEKDAYS)

class SensorEventAndAction : public ConfigPageControl
{
    Q_OBJECT

public:
    explicit SensorEventAndAction(QString deviceName, QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL );
    ~SensorEventAndAction();

    void initlizeVariables();
    void createDefaultComponents();
    void fillCameraList();
    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig ();
    void setConfigFields();
    void saveConfig ();
    void defaultConfig ();
    void fillRecords();
    void updateScheduleBar();
    void enableControlsOnAction(bool status);
    void getSensorList();
    void deleteSubObejects();

    void processDeviceResponse(DevCommParam *param, QString deviceName);   
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);

signals:

public slots:
    void slotSpinBoxValueChanged(QString,quint32);
    void slotCheckBoxValueChanged(OPTION_STATE_TYPE_e,int);
    void slotButtonClick(int);
    void slotSubObjectDelete(quint8);

private:
    QMap<quint8, QString> cameraList;
    QMap<quint8, QString> sensorList;
    QMap<quint8, QString> presetList;

    quint8  totalCamera;
    quint8  currentSensorIndex;
    quint32 cnfg1FrmIndx;
    quint32 cnfg1ToIndx;
    quint32 cnfg2FrmIndx;
    quint32 cnfg2ToIndx;
    quint32 m_currentDayIndex;

    DropDown*           sensorListDropDownBox;
    OptionSelectButton* actionEventCheckbox;
    ElementHeading*     actionSchdHeading;
    DropDown*           weekdayDropDownBox;
    ScheduleBar*        scheduleBar[MAX_SENS_EVNT_SCHEDULE_BAR];
    PageOpenButton*     setButtons[MAX_SENS_EVNT_SCHEDULE_BAR];

    CAMERA_BIT_MASK_t   alarmRecordingSelectedCamera;
    CAMERA_BIT_MASK_t   uploadImageSelectedCamera;

    QString emailAddress;
    QString emailSubject;
    QString emailMessage;

    QString tcpMessage;

    QString smsMobileNum1;
    QString smsMobileNum2;
    QString smsMessage;

    quint32 presetCameraNum;
    quint32 presetGotoPosition;

    bool    deviceAlaram[MAX_DEV_ALARM];
    bool    cameraAlarm[MAX_CAM_ALARM];
    quint8  currentCamForAlarm;

    bool                isScheduleSetForEntireDay[MAX_SENS_EVNT_WEEKDAYS];
    bool                isDaySelectForSchedule[MAX_SENS_EVNT_WEEKDAYS];
    SCHEDULE_TIMING_t   scheduleTimeing[MAX_SENS_EVNT_EVENTS];
    quint32             actionStatus[MAX_SENS_EVNT_ACTIONS_STATUS];

    CopyToCamera*           cameraSelectForAlarm;
    CopyToCamera*           cameraSelectForImageUpload;
    EventActionEmailNotify* eventActionEmailNotify;
    EventActionTCPNotify*   eventActionTCPNotify;
    EventActionSmsNotify*   eventActionSmsNotify;
    EventActionPtzPos*      eventActionPtzPos;
    EventActionDeviceAlarm* eventActionDeviceAlarm;
    EventCameraAlarmOutput* eventCameraAlarmOutput;
    EventActionSchedule*    eventActionSchedule;
};

#endif // SensorEventAndAction_H
