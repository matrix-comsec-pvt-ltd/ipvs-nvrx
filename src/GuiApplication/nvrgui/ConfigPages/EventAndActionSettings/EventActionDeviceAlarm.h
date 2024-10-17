#ifndef EVENTACTIONDEVICEALARM_H
#define EVENTACTIONDEVICEALARM_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/CnfgButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextLabel.h"

#define MAX_ALARM 1

typedef enum{

    EVNT_DEV_ALM_CLS_CTRL,
    EVNT_DEV_ALM_ALL_CHCKBOX,
    EVNT_DEV_ALM_ALARM1_CHCKBOX,
    EVNT_DEV_ALM_OK_CTRL = (MAX_DEV_ALARM + EVNT_DEV_ALM_ALARM1_CHCKBOX ),
    EVNT_DEV_ALM_CANCEL_CTRL,

    MAX_EVNT_DEV_ALM_NOTIFY_CTRL
}EVNT_DEV_ALM_NOTIFY_CTRL_e;

class EventActionDeviceAlarm : public KeyBoard
{
    Q_OBJECT

    Rectangle* backGround;
    CloseButtton* closeButton;
    Heading* heading;

    CnfgButton* okButton;
    CnfgButton* cancelButton;

    ElementHeading* elementHeading;
    BgTile* bgTile;
    TextLabel* alarmLabel[MAX_DEV_ALARM + 1];
    OptionSelectButton* alarmCheckBox[MAX_DEV_ALARM + 1];

    NavigationControl* m_elementlist[MAX_EVNT_DEV_ALM_NOTIFY_CTRL];
    quint8 currElement;
    quint8 m_index;
    bool *devAlm;
    quint8 isAllAlarmActivated;
    DEV_TABLE_INFO_t *m_devTableInfo;

public:
    explicit EventActionDeviceAlarm(quint8 index,bool *deviceAlrm, DEV_TABLE_INFO_t *devTableInfo, QWidget *parent = 0);
    ~EventActionDeviceAlarm();

    void paintEvent (QPaintEvent *event);

    void assignValues();
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent (QShowEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);

public slots:
    void slotButtonClick(int);
    void slotUpdateCurrentElement(int index);
    void slotCheckboxClicked(OPTION_STATE_TYPE_e,int);

};

#endif // EVENTACTIONDEVICEALARM_H
