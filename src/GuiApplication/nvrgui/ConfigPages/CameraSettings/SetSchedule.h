#ifndef SETSCHEDULE_H
#define SETSCHEDULE_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextLabel.h"
#include "Controls/Heading.h"
#include "Controls/ScheduleBar.h"
#include "Controls/Clockspinbox.h"
#include "Controls/InfoPage.h"

#include "KeyBoard.h"

#define MAX_TOTAL_SPINBOX       12
#define MAX_SPINBOX             6
#define MAX_WEEKDAY_WITH_ALL    8
#define MAX_WEEKDAYS            7
#define MAX_ADAPTIVE_CHECKBOX   MAX_SPINBOX
#define MAX_SCHD_CHECKBOX_PARAM 8

#define IS_SET_SCHD_ENTIREDAY_CHECKED          1
#define IS_SET_SCHD_ENTIREDAY_ADAPTIVE_CHECKED 2

typedef enum {
    SET_SCHD_CLOSE_BTN,
    SET_SCHD_ENTIRE_DAY_CTRL,
    SET_SCHD_ADAPTIVE_CTRL,
    SET_SCHD_ADAPTIVE_CHECKBOX,
    SET_SCHD_START_TIME_CLK_SPINBOX = SET_SCHD_ADAPTIVE_CHECKBOX + MAX_ADAPTIVE_CHECKBOX,
    SET_SCHD_END_TIME_CLK_SPINBOX,
    SET_SCHD_CPY_TO_CHECK_BOX = MAX_TOTAL_SPINBOX + SET_SCHD_START_TIME_CLK_SPINBOX,
    SET_SCHD_OK_CTRL = MAX_WEEKDAY_WITH_ALL + SET_SCHD_CPY_TO_CHECK_BOX,
    SET_SCHD_CANCEL_CTRL,
    MAX_SET_SCHD_CTRL
}SET_SCHD_CTRL_e;

typedef enum
{
    SET_SCHD_REC_FOR,
    SET_SCHD_SNAP_SHOT,
    MAX_SET_REC_TYPE
}SET_RECORD_TYPE_e;

class SetSchedule : public KeyBoard
{
    Q_OBJECT

    Rectangle* backGround;
    Heading* heading;
    CloseButtton* closeButton;
    InfoPage *infoPage;

    OptionSelectButton* entireDay;
    OptionSelectButton* entireDayAdaptiveCheckbox;
    OptionSelectButton* adaptiveCheckboxes[MAX_ADAPTIVE_CHECKBOX];
    ElementHeading* timePeriod;
    ElementHeading* adaptiveCheckboxHeading;
    ElementHeading* startTimeHeading;
    ElementHeading* stopTimeHeading;

    ClockSpinbox* start_time[MAX_SPINBOX];
    ClockSpinbox* stop_time[MAX_SPINBOX];

    ElementHeading* slotCountLabel[MAX_SPINBOX];
    ElementHeading* copyToDays;
    TextLabel* weekDaysLabel[MAX_WEEKDAY_WITH_ALL];
    OptionSelectButton* weekDays[MAX_WEEKDAY_WITH_ALL];

    CnfgButton* okButton;
    CnfgButton* cancelButton;

    NavigationControl* m_elementlist[MAX_SET_SCHD_CTRL];
    quint8 m_currentElement;

    SCHEDULE_TIMING_t *setScheduleTiming;
    quint8 *isEntireDayAndAdaptiveSelect;
    quint8 currentDayIndex;
    quint8 *isCopyToWeekdaysSelect;
    SET_RECORD_TYPE_e m_recordType;

public:
    SetSchedule(SCHEDULE_TIMING_t *setSchdleTiming,
                quint8 *entireDayAndAdaptiveSelect,
                quint8 *copyToWeekdaysFields,
                QWidget *parent = 0,
                quint8 dayIndex = 0,
                SET_RECORD_TYPE_e recordType = SET_SCHD_REC_FOR);

    ~SetSchedule();

    bool timeValidation();
    void paintEvent (QPaintEvent *);

    void takeLeftKeyAction ();
    void takeRightKeyAction ();
    void showEvent (QShowEvent *event);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);


signals:
    void sigDeleteObject();

public slots:
    void slotCloseButtonClick(int);
    void slotInfoPageCnfgBtnClick(int);
    void slotOkButtonClick(int);
    void slotUpdateCurrentElement(int index);
    void slotOptionButtonClicked(OPTION_STATE_TYPE_e,int);
};

#endif // SETSCHEDULE_H
