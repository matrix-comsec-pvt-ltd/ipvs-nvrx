#ifndef CAMERAEVENTANDEVENTACTION_H
#define CAMERAEVENTANDEVENTACTION_H

#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ScheduleBar.h"
#include "Controls/PageOpenButton.h"
#include "Controls/DropDown.h"

#include "ConfigPages/CameraSettings/CopyToCamera.h"
#include "ConfigPages/EventAndActionSettings/EventActionSchedule.h"
#include "ConfigPages/EventAndActionSettings/EventActionEmailNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionTCPNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionSmsNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionPtzPos.h"
#include "ConfigPages/EventAndActionSettings/EventActionDeviceAlarm.h"
#include "ConfigPages/EventAndActionSettings/EventCameraAlarmOutput.h"

#define MAX_CAM_EVNT_WEEKDAYS        MAX_WEEKDAYS
#define MAX_CAM_EVNT_EVENTS          MAX_EVNT_SCHD_EVNT
#define MAX_CAM_EVNT_SCHEDULE_BAR    (MAX_CAM_EVNT_EVENTS + 1)
#define MAX_CAM_EVNT_ACTIONS_STATUS  (MAX_CAM_EVNT_WEEKDAYS*MAX_CAM_EVNT_WEEKDAYS)

class CameraEventAndEventAction : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit CameraEventAndEventAction(QString deviceName, QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~CameraEventAndEventAction();

    void getConfig ();
    void saveConfig ();
    void defaultConfig ();

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
    QMap<quint8, QString>   cameraList;
    QMap<quint8, QString>   cameraEventTypeMapList;
    QMap<quint8, QString>   presetList;

    quint8                  totalCamera;
    quint32                 currentCameraIndex;
    quint32                 cnfg1FrmIndx;
    quint32                 cnfg2FrmIndx;
    quint32                 cnfg2ToIndx;
    quint8                  currentDayIndex;
    CAMERA_TYPE_e           currentcamtype;
    quint8                  currentEventType;
    QString                 currentEventStr;

    DropDown*               cameraListDropDownBox;
    DropDown*               eventListDropDownBox;
    DropDown*               weekdayDropDownBox;
    ElementHeading*         actionSchdHeading;
    ScheduleBar*            scheduleBar[MAX_CAM_EVNT_SCHEDULE_BAR];
    PageOpenButton*         setButtons[MAX_CAM_EVNT_SCHEDULE_BAR];

    OptionSelectButton*     actionEventCheckbox;

    CAMERA_BIT_MASK_t       alarmRecordingSelectedCamera;
    CAMERA_BIT_MASK_t       uploadImageSelectedCamera;
    CAMERA_BIT_MASK_t       eventSelectedCamera;

    QString                 emailAddress;
    QString                 emailSubject;
    QString                 emailMessage;

    QString                 tcpMessage;

    QString                 smsMobileNum1;
    QString                 smsMobileNum2;
    QString                 smsMessage;

    quint32                 presetCameraNum;
    quint32                 presetGotoPosition;

    bool                    deviceAlaram[MAX_DEV_ALARM];
    bool                    cameraAlarm[MAX_CAM_ALARM];
    quint8                  currentCamForAlarm;

    bool                    isScheduleSetForEntireDay[MAX_CAM_EVNT_WEEKDAYS];
    bool                    isDaySelectForSchedule[MAX_CAM_EVNT_WEEKDAYS];
    SCHEDULE_TIMING_t       scheduleTimeing[MAX_CAM_EVNT_EVENTS];

    quint32                 actionStatus[MAX_CAM_EVNT_ACTIONS_STATUS];

    bool                    isMotionSupport;
    bool                    isNoMotionSupport;
    bool                    isTemperSupport;
    bool                    isPTZSupport;
    bool                    isAudioSupport;
    bool                    isLineDetectionSupport;
    bool                    isIntrusionSupport;
    bool                    isAudioExceptionSupport;
    bool                    isMissingObjectSupport;
    bool                    isSuspiousObjectSupport;
    bool                    isLoteringSupport;
    bool                    isObjectCountingSupport;
    quint8                  maxAlarm;
    quint8                  maxSensor;

    CopyToCamera*           cameraSelectForAlarm;
    CopyToCamera*           cameraSelectForImageUpload;
    EventActionEmailNotify* eventActionEmailNotify;
    EventActionTCPNotify*   eventActionTCPNotify;
    EventActionSmsNotify*   eventActionSmsNotify;
    EventActionPtzPos*      eventActionPtzPos;
    EventActionDeviceAlarm* eventActionDeviceAlarm;
    EventCameraAlarmOutput* eventCameraAlarmOutput;
    EventActionSchedule*    eventActionSchedule;
    CopyToCamera*           eventCopyToCamera;
    PageOpenButton*         copyToCameraBtn;

    bool                    isMotionConfigured;
    bool                    isCameraEnable;
    bool                    isRequestSend;
    quint8                  suppAlarms;
    quint8                  suppSensors;

    // private Functions
    void initlizeVariables();
    void createDefaultComponents();
    void fillCameraList();
    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig1 ();
    void setConfigFields();
    void fillRecords();
    void updateScheduleBar();
    void enableControlsOnAction(bool status);
    void deleteSubObejects();
    void sendCommand(SET_COMMAND_e cmdType, quint8 totalfeilds = 0);
    void fillOtherSupportData();
};

#endif // CAMERAEVENTANDEVENTACTION_H
