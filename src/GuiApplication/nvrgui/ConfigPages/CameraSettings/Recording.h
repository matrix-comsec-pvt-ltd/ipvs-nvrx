#ifndef RECORDING_H
#define RECORDING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/ElementHeading.h"
#include "Controls/TextBox.h"
#include "DataStructure.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/PageOpenButton.h"
#include "Controls/ScheduleBar.h"
#include "SetSchedule.h"
#include "CopyToCamera.h"
#include "Controls/DropDown.h"


#define MAX_TIME_SLOTS   6
#define MAX_WEEKDAYS     7
#define MAX_SCHDBAR      8


typedef enum
{
    REC_CAMERANAME_SPINBOX_CTRL,

    REC_PRERECORD_TEXTBOX_CTRL,
    REC_POSTRECORD_TEXTBOX_CTRL,
    REC_ENB_PER_REC_CHCKBOX_CTRL,
    REC_COSEC_PRERECORD_TEXTBOX_CTRL,
    REC_MANUAL_RECORDING_CTRL,
    REC_SCHD_RECORDING_CTRL,
    REC_SCHD_WEEKLY_SET,
     REC_CPY_CAMERA_BTN_CTRL,
    MAX_RECORDING_CONTROL = MAX_WEEKDAYS +  REC_CPY_CAMERA_BTN_CTRL
}RECORDING_CONTROL_e;

class Recording :  public ConfigPageControl
{
    Q_OBJECT

public:
    explicit Recording(QString deviceName, QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~Recording();

    void createDefaultComponents();
    void createPayload(REQ_MSG_ID_e msgType );

    void getConfig();
    void defaultConfig();
    void saveConfig();
    void fillRecords();
    void fillRecordsForAlarmSchedule();
    void fillCameraList();
    void updateData();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);

signals:

public slots:
    void slotLoadInfopage(int,INFO_MSG_TYPE_e);
    void slotCheckboxClicked(OPTION_STATE_TYPE_e,int);
    void slotPageOpenBtnClick(int index);
    void slotSubObjectDelete(quint8);
    void slotSubObjectDelete();
    void slotSpinboxValueChanged(QString,quint32);

private:
    QMap<quint8, QString>   cameraList;
    quint8                  isScheduleSetForEntireDay[MAX_WEEKDAYS];
    quint8                  currentDaySelect;
    quint16                 currentCameraIndex;
    quint8                  copyToWeekdaysFields;

    CAMERA_BIT_MASK_t       copyToCameraFields;
    SCHEDULE_TIMING_t       scheduleTimeing[MAX_WEEKDAYS];

    DropDown*               cameraNameSpinbox;
    PageOpenButton*         copyToCameraBtn;
    ElementHeading*         alarmHeading;
    TextboxParam*           preRecordParam;
    TextBox*                preRecordTextBox;
    TextboxParam*           postRecordParam;
    TextBox*                postRecordTextBox;
    ElementHeading*         cosecHeading;
    OptionSelectButton*     enablePerRecordCheckBox;
    TextboxParam*           cosecPreRecordParam;
    TextBox*                cosecPreRecordTextBox;
    CopyToCamera*           copytoCamera;
    OptionSelectButton*     manualRecording;
    OptionSelectButton*     scheduleRecording;
    ScheduleBar*            scheduleWeekly[MAX_WEEKDAYS + 1];
    PageOpenButton*         scheduleWeeklySet[MAX_WEEKDAYS];
    SetSchedule*            setSchedule;
};

#endif // RECORDING_H
