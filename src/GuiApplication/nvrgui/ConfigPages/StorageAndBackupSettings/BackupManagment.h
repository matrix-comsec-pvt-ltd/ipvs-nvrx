#ifndef BACKUPMANAGMENT_H
#define BACKUPMANAGMENT_H

#include <QWidget>
#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ControlButton.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"
#include "Controls/CalendarTile.h"
#include "Controls/Clockspinbox.h"
#include "ConfigPages/CameraSettings/CopyToCamera.h"

#define MAX_SCHD_MODE 3

typedef struct
{
    quint8              duration;
    quint8              backupLocation;
    CAMERA_BIT_MASK_t   camera;
    quint8              manBackupCheckBox;

}configCopy_t;

class BackupManagment : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit BackupManagment(QString devName,QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~BackupManagment();

    void createDefaultComponents ();
    void enableControls(quint32 state);

    void createPayload(REQ_MSG_ID_e msgType);
    void getCameraList();
    void getConfig();
    void saveConfig();
    void saveConfigFields();
    void defaultConfig();
    void getDevDateTime();
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);
    void startBackup();
    void processDeviceResponse (DevCommParam *param, QString deviceName);

signals:
    
public slots:
    void slotButtonClick(int);
    void slotSubObjectDelete(quint8);
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);
    void slotValueChanged(QString, quint32);
    void slotStartButtonClick(int);

private:

    OptionSelectButton*     enableSchdBackup;
    ElementHeading*         elementHeading;
    OptionSelectButton*     backupSelection[MAX_SCHD_MODE];

    BgTile                  *blank;
    DropDown*               timeIntervalDropDownBox;
    DropDown*               timeHourSelectDropDownBox;
    TextLabel*              timeHourSuffix;
    QMap<quint8, QString>   timeHourList;

    DropDown*               weeklyDayDropDownBox;
    TextLabel*              weekdayLabel;
    TextLabel*              weekdaySuffix;
    DropDown*               weeklyTimeDropDownBox;
    TextLabel*              weeklyTimeSuffix;
    DropDown*               backupLocationDropDownBox;
    ControlButton*          selectCamera;
    CAMERA_BIT_MASK_t       selectedCameraNum;

    QMap<quint8, QString>   cameraList;
    CopyToCamera*           copyToCamera;

    OptionSelectButton*     manualBackUp;
    DropDown*               durationDropDownBox;
    ControlButton*          manualBackupSelectCamera;
    CopyToCamera*           manualBackupCopyToCamera;
    CAMERA_BIT_MASK_t       manualBackupSelectedCameraNum;
    DropDown*               manualBackupLocationDropDownBox;

    CalendarTile            *startDateCalender, *endDateCalender;
    ClockSpinbox            *startTimeSpinbox, *endTimeSpinbox;
    CnfgButton              *startButton;
    configCopy_t            configCopy;
};

#endif // BACKUPMANAGMENT_H
