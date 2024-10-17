#ifndef SNAPSHOTSCHEDULE_H
#define SNAPSHOTSCHEDULE_H

#include "DataStructure.h"
#include "CopyToCamera.h"
#include "SetSchedule.h"

#include "Controls/TextBox.h"
#include "Controls/DropDown.h"
#include "Controls/MessageBox.h"
#include "Controls/PageOpenButton.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"

//#define MAX_WEEKDAYS 7

class SnapshotSchedule : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit SnapshotSchedule(QString deviceName,
                              QWidget* parent = 0,
                              DEV_TABLE_INFO_t *devTabInfo = NULL);

    ~SnapshotSchedule();


    void createPayload(REQ_MSG_ID_e msgType);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    void processDeviceResponse (DevCommParam *param, QString deviceName);
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);

signals:
    
public slots:
    void slotPageOpenBtnClick (int index );
    void slotCopytoCamDelete(quint8);
    void slotSchdObjectDelete();
    void slotDropDownValueChange(QString,quint32);
    void slotCheckboxClicked(OPTION_STATE_TYPE_e,int);
    void slotTextBoxInfoPage(int index ,INFO_MSG_TYPE_e msgType);

private:
    quint8                  currentCameraIndex;
    CAMERA_BIT_MASK_t       copyToCameraFields;
    quint8                  copyToWeekdaysFields;
    QMap<quint8, QString>   cameraList;

    DropDown*               cameraListDropDownBox;
    PageOpenButton*         copyToCameraBtn;

    CopyToCamera*           copytoCamera;

    OptionSelectButton*     enableSnapshotCheckBox;
    DropDown*               uploadListDropDownBox;

    TextboxParam*           emailTextBoxParam;
    TextBox*                emailTextBox;

    TextboxParam*           subjectTextBoxParam;
    TextBox*                subjectTextBox;

    MessageBox*             msgBox;

    DropDown*               imageRateDropDownBox;
    TextLabel*              emailNoteLabel;

    PageOpenButton*         schdSetBtn;
    SetSchedule*            setSchedule;

    quint8 isScheduleSetForEntireDay;

    SCHEDULE_TIMING_t scheduleTimeing;

    void getCameraList();
    bool saveConfigFeilds ();
    void createDefaultComponents ();
};

#endif // SNAPSHOTSCHEDULE_H
