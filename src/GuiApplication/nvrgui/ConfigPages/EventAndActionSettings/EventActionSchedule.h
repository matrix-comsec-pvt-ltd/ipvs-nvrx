#ifndef EVENTACTIONSCHEDULE_H
#define EVENTACTIONSCHEDULE_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/CnfgButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/Clockspinbox.h"
#include "Controls/ScheduleBar.h"
#include "Controls/InfoPage.h"
#include "DataStructure.h"
#include "Controls/TextLabel.h"
#include "Elidedlabel.h"

#define MAX_CNTRL_ON_ROW    EVNT_SCHD_START_TIME_SPINBOX_CTRL
#define MAX_STR_LABEL       3
#define MAX_WEEKDAYS        7

typedef enum
{
    EVNT_BUZZER = 0,
    EVNT_CAM_ALARM,
    EVNT_SYS_DEVICE_ALARM,
    EVNT_PTZ,
    EVNT_SMS,
    EVNT_TCP,
    EVNT_EMAIL,
    EVNT_IMAGE_UPLOAD,
    EVNT_ALM_REC,
    EVNT_VIDEO_POP_UP,
    EVNT_PUSH_NOTIFICATION,
    MAX_EVNT_SCHD_EVNT
}EVNT_SCHD_EVNT_e;

typedef enum
{
    EVNT_SCHD_CLS_CTRL,
    EVNT_SCHD_ENTIRE_DAY_CNTRL,

    EVNT_SCHD_REC_ENTDAY_CTRL,
    EVNT_SCHD_IMG_ENTDAY_CTRL,
    EVNT_SCHD_EMAIL_ENTDAY_CTRL,
    EVNT_SCHD_TCP_ENTDAY_CTRL,
    EVNT_SCHD_SMS_ENTDAY_CTRL,
    EVNT_SCHD_PTZ_ENTDAY_CTRL,
    EVNT_SCHD_DVALM_ENTDAY_CTRL,
    EVNT_SCHD_CAMERA_ENTDAY_CTRL,
    EVNT_SCHD_BUZZER_ENTDAY_CTRL,
    EVNT_SCHD_VIDEO_POPUP_ENTDAY_CTRL,
    EVNT_SCHD_PUSH_NOTIFICATION_ENTDAY_CTRL,

    EVNT_SCHD_START_TIME_SPINBOX_CTRL,
    EVNT_SCHD_END_TIME_SPINBOX_CTRL,

    EVNT_SCHD_REC_CTRL,
    EVNT_SCHD_IMAGE_CTRL,
    EVNT_SCHD_EMAIL_CTRL,
    EVNT_SCHD_TCP_CTRL,
    EVNT_SCHD_SMS_CTRL,
    EVNT_SCHD_PTZ_CTRL,
    EVNT_SCHD_DEVICE_ALARM_CTRL,
    EVNT_SCHD_CAMERA_ALARM_CTRL,
    EVNT_SCHD_BUZZER_CTRL,
    EVNT_SCHD_VIDEO_POPUP_CTRL,
    EVNT_SCHD_PUSH_NOTIFICATION_CTRL,
    EVNT_SCHD_ALL_WEEKDAY_CTRL = (((MAX_TIME_SLOT - 1) * MAX_CNTRL_ON_ROW) + EVNT_SCHD_PUSH_NOTIFICATION_CTRL),


    EVNT_SCHD_OK_CTRL = (MAX_WEEKDAYS + EVNT_SCHD_ALL_WEEKDAY_CTRL+1),
    EVNT_SCHD_CANCEL_CTRL,

    MAX_EVNT_SCHD_CTRL
}EVNT_SCHD_CTRL_e;

typedef enum
{
    MX_VIDEOPOPUP_UPDATE_MODE,
    MX_VIDEOPOPUP_DISABLE_MODE,
    MAX_MX_EVNT_ACTION_SCHD_MODE
}MX_EVNT_ACTION_SCHD_MODE_e;

class EventActionSchedule : public KeyBoard
{
    Q_OBJECT

public:
    explicit EventActionSchedule(quint32 index,
                                 quint8 dayIndex,
                                 SCHEDULE_TIMING_t *schdRec,
                                 bool *entireDay,
                                 quint32 *action, QWidget *parent = 0,
                                 DEV_TABLE_INFO_t *devTabInfo = NULL,
                                 MX_EVNT_ACTION_SCHD_MODE_e eventModeType = MAX_MX_EVNT_ACTION_SCHD_MODE);
    ~EventActionSchedule();

    void initVariable();
    void createDefaultElement();
    void changesAcordingBoardType();
    void getCurrentActionStatus();
    bool timeValidation ();
    void enableControls(bool);
    void takeLeftKeyAction();
    void takeRightKeyAction();

    void paintEvent (QPaintEvent *event);
    void showEvent (QShowEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);

public slots:
    void slotButtonClick(int);
    void slotUpdateCurrentElement(int index);
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);
    void slotInfoPageBtnclick(int);

private:

    Rectangle*                 backGround;
    CloseButtton*              closeButton;
    Heading*                   heading;

    CnfgButton*                okButton;
    CnfgButton*                cancelButton;

    SCHEDULE_TIMING_t          scheduleTimeing[MAX_TIME_SLOT];
    BgTile*                    m_eleHeadingTile;
    ElementHeading*            elementHeading[MAX_EVNT_SCHD_EVNT];
    ElidedLabel*               m_elementHeadingElide[MAX_EVNT_SCHD_EVNT];

    OptionSelectButton*        entireDayCheckBox;
    OptionSelectButton*        entireDayForEventCheckBox[MAX_EVNT_SCHD_EVNT];
    OptionSelectButton*        recordCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        imageCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        emailCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        tcpCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        ptzCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        smsCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        alarmCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        camAlarmCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        buzzerCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        m_videoPopUpCheckBox[MAX_TIME_SLOT];
    OptionSelectButton*        m_pushNotificationCheckBox[MAX_TIME_SLOT];

    InfoPage*                  infoPage;

    BgTile*                    bgTile;
    TextLabel*                 label[MAX_STR_LABEL];
    TextLabel*                 slotLabel[MAX_TIME_SLOT];

    ClockSpinbox*              start_time[MAX_TIME_SLOT];
    ClockSpinbox*              stop_time[MAX_TIME_SLOT];
    BgTile*                    bgTileBtm;
    TextLabel*                 textLabelBtm;

    OptionSelectButton*        weekdayCheckBox[(MAX_WEEKDAYS + 1)];

    NavigationControl*         m_elementlist[MAX_EVNT_SCHD_CTRL];
    quint8                     currElement;
    quint8                     m_index;
    quint8                     m_currentDayIndex;
    SCHEDULE_TIMING_t*         camSchdRec;
    bool*                      isEntireDaySelected;
    quint32*                   actionStatus;
    bool                       isDaySelectforCopy[(MAX_WEEKDAYS + 1)];

    DEV_TABLE_INFO_t*          deviceTableInfo;

    MX_EVNT_ACTION_SCHD_MODE_e  m_eventModeType;

    void updateEventModes();
};

#endif // EVENTACTIONSCHEDULE_H
