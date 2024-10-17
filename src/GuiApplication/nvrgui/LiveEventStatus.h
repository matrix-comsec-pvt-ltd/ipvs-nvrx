#ifndef LIVEEVENTSTATUS_H
#define LIVEEVENTSTATUS_H

#include "Controls/BackGround.h"
#include "Controls/MenuButton.h"
#include "ApplController.h"
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"

#define LIVE_EVENT_STATUS_WIDTH             SCALE_WIDTH(400)
#define LIVE_EVENT_STATUS_HEIGHT            SCALE_HEIGHT(350)

typedef enum
{
    LIVE_EVENT_STATUS_CLOSE_BUTTON = 0,
    LIVE_EVENT_STATUS_MENUBUTTONS,
    MAX_LIVE_EVENT_STATUS_ELEMENTS = 22
}LIVE_EVENT_STATUS_ELEMENT_e;

class LiveEventStatus : public BackGround
{
    Q_OBJECT
private:
    QString m_deviceNames[MAX_DEVICES];
    QString m_eventCount[MAX_DEVICES];
    MenuButton* m_devices[MAX_DEVICES];
    BgTile* m_deviceNameHeadingTile;
    BgTile* m_eventCountHeadingTile;
    BgTile* m_eventCountTile[MAX_DEVICES];
    TextLabel* m_deviceNameHeadingLabel;
    TextLabel* m_eventCountHeadingLabel;
    TextLabel* m_eventCountLabel[MAX_DEVICES];
    QStringList m_enabledDeviceList;

    quint8 m_deviceCount;
    int m_currentDeviceIndex;
    ApplController* m_applController;

    NavigationControl* m_elementList[MAX_LIVE_EVENT_STATUS_ELEMENTS];
    int m_currentElement;

public:
    LiveEventStatus(int startX, int startY, QWidget *parent = 0);
    ~LiveEventStatus();

    void getEnabledDevices();
    void getDevicesEventCount();
    void forceActiveFocus();

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent (QShowEvent *event);

    //keyboard support added
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);

signals:
    void sigDeviceSelected(QString deviceName);

public slots:
    void slotMenuButtonClicked(int index);
    void slotUpdateCurrentElement(int index);
};

#endif // LIVEEVENTSTATUS_H
