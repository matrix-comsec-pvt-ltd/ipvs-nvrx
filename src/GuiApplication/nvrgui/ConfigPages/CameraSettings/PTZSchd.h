#ifndef PTZSCHD_H
#define PTZSCHD_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/CnfgButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/Clockspinbox.h"
#include "Controls/InfoPage.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"

#include "KeyBoard.h"

typedef enum{
    SUNDAY,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    MAX_PTZ_WEEKDAYS
}PTZ_WEEKDAYS_e;

typedef enum{

    PTZ_SCHD_CLS_BTN,
    PTZ_SCHD_ENTIREDAY,
    PTZ_SCHD_ENTIREDAY_PRE_SPNBX = PTZ_SCHD_ENTIREDAY + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_TIME1_SRT_TIME = PTZ_SCHD_ENTIREDAY_PRE_SPNBX + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_TIME1_END_TIME = PTZ_SCHD_TIME1_SRT_TIME + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_TIME1_PRE_SPNBX = PTZ_SCHD_TIME1_END_TIME + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_TIME2_SRT_TIME = PTZ_SCHD_TIME1_PRE_SPNBX + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_TIME2_END_TIME = PTZ_SCHD_TIME2_SRT_TIME + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_TIME2_PRE_SPNBX = PTZ_SCHD_TIME2_END_TIME + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_OK_BTN = PTZ_SCHD_TIME2_PRE_SPNBX + MAX_PTZ_WEEKDAYS,
    PTZ_SCHD_CANCEL_BTN,

    MAX_PTZ_SCHD_CTRL
}PTZ_SCHD_CTRL_e;


class PTZSchd : public KeyBoard
{
    Q_OBJECT
public:
    explicit PTZSchd(QMap<quint8, QString>  preList,
                     bool* enitreDay,
                     QStringList &startTime1,
                     QStringList &stopTime1,
                     QStringList &startTime2,
                     QStringList &stopTime2,
                     quint8 *presetPos,
                     QWidget *parent = 0);
    ~PTZSchd();

    void createDefaultComponets();
    void assignIntialValues();
    void enableControls(quint8 tempIndex,bool state);
    bool checkValidationOfTime();

    void paintEvent (QPaintEvent *event);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent (QShowEvent *event);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);

signals:
    void sigObjectDel();

public slots:
    void slotButtonClick(int);
    void slotUpdateCurrentElement(int);
    void slotInfoPageBtnclick(int index);
    void slotCheckBoxClick(OPTION_STATE_TYPE_e,int);

private:

    Rectangle* backGround;
    CloseButtton* closeButton;
    Heading* heading;

    CnfgButton* okButton;
    CnfgButton* cancelButton;

    NavigationControl* m_elementlist[MAX_PTZ_SCHD_CTRL];
    quint8 currElement;

    ElementHeading*  eleheadings[4];
    BgTile*          headingTile;
    TextLabel*       textlabels[8];

    InfoPage* infoPage;

    QMap<quint8, QString>          presetList;

    OptionSelectButton* entireDayWeekdaySelection[MAX_PTZ_WEEKDAYS];
    DropDown*            entireDayPresetSpinBox[MAX_PTZ_WEEKDAYS];

    ClockSpinbox*       timePeriod1StartTime[MAX_PTZ_WEEKDAYS];
    ClockSpinbox*       timePeriod1EndTime[MAX_PTZ_WEEKDAYS];
    DropDown*           timePeriod1PresetSpinBox[MAX_PTZ_WEEKDAYS];

    ClockSpinbox*       timePeriod2StartTime[MAX_PTZ_WEEKDAYS];
    ClockSpinbox*       timePeriod2EndTime[MAX_PTZ_WEEKDAYS];
    DropDown*           timePeriod2PresetSpinBox[MAX_PTZ_WEEKDAYS];

    bool*   entireDaySelect;
    QStringList* m_startTime1;
    QStringList* m_startTime2;
    QStringList* m_stopTime1;
    QStringList* m_stopTime2;
    quint8*     m_presetPos;


};

#endif // PTZSCHD_H
