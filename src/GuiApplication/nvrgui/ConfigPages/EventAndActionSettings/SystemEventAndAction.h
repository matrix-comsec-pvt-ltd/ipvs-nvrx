#ifndef SYSTEMEVENTANDACTION_H
#define SYSTEMEVENTANDACTION_H

#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/PageOpenButton.h"
#include "Controls/TextLabel.h"

#include "ConfigPages/CameraSettings/CopyToCamera.h"
#include "ConfigPages/EventAndActionSettings/EventActionEmailNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionTCPNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionSmsNotify.h"
#include "ConfigPages/EventAndActionSettings/EventActionPtzPos.h"
#include "ConfigPages/EventAndActionSettings/EventActionDeviceAlarm.h"
#include "ConfigPages/EventAndActionSettings/EventCameraAlarmOutput.h"

/* System events without set button */
#define SYS_EVENT_WO_SET_BTN    2

/* System event offset (Alarm recording and Video popup events are not available in system events and action) */
#define SYS_EVENT_OFFSET        2

typedef enum
{
    SYS_EVENT_TYPE_BUZZER = 0,
    SYS_EVENT_TYPE_CAM_ALARM,
    SYS_EVENT_TYPE_SYS_DEVICE_ALARM,
    SYS_EVENT_TYPE_PTZ,
    SYS_EVENT_TYPE_SMS,
    SYS_EVENT_TYPE_TCP,
    SYS_EVENT_TYPE_EMAIL,
    SYS_EVENT_TYPE_IMAGE_UPLOAD,
    SYS_EVENT_TYPE_PUSH_NOTIFICATION,

    SYS_EVENT_TYPE_MAX
}SYS_EVENT_TYPE_e;

class SystemEventAndAction : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit SystemEventAndAction(QString deviceName, QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~SystemEventAndAction();

    void createDefaultComponents();
    void fillCameraList();
    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig ();
    void setConfigFields();
    void saveConfig ();
    void defaultConfig ();
    void deleteSubObejects ();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);

signals:
    
public slots:
    void slotButtonClick(int);
    void slotSubObjectDelete(quint8);
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);
    void slotSpinboxValueChanged(QString,quint32 );

private:

    QMap<quint8, QString> cameraList;
    QMap<quint8, QString> presetList;

    DropDown*           eventTypeDropDownBox;
    quint8              currentEventIndex;

    OptionSelectButton* actionSelection;
    OptionSelectButton* events[SYS_EVENT_TYPE_MAX];
    PageOpenButton*     eventsSetBtn[SYS_EVENT_TYPE_MAX - SYS_EVENT_WO_SET_BTN];

    CAMERA_BIT_MASK_t   alarmRecordingSelectedCamera;
    CAMERA_BIT_MASK_t   uploadImageSelectedCamera;

    QString         emailAddress;
    QString         emailSubject;
    QString         emailMessage;

    QString         tcpMessage;

    QString         smsMobileNum1;
    QString         smsMobileNum2;
    QString         smsMessage;

    quint32         presetCameraNum;
    quint32         presetGotoPosition;

    bool            deviceAlaram[MAX_DEV_ALARM];
    bool            cameraAlarm[MAX_CAM_ALARM];

    quint32         actionStatus;
    quint8          currentclickedIndex;
    quint8          currentCamforAlarm;
    quint8          suppAlarm;
    quint8          suppSensors;

    CopyToCamera*           cameraSelectForAlarm;
    CopyToCamera*           cameraSelectForImageUpload;
    EventActionEmailNotify* eventActionEmailNotify;
    EventActionTCPNotify*   eventActionTCPNotify;
    EventActionSmsNotify*   eventActionSmsNotify;
    EventActionPtzPos*      eventActionPtzPos;
    EventActionDeviceAlarm* eventActionDeviceAlarm;
    EventCameraAlarmOutput* eventCameraAlarmOutput;
    TextLabel*              footNoteLabel;

    void updateEventNumber();

};

#endif // SYSTEMEVENTANDACTION_H
