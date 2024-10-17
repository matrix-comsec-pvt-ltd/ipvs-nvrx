#ifndef LIVEEVENT_H
#define LIVEEVENT_H

#include <QWidget>
#include "LiveEventStatus.h"
#include "LiveEventDetail.h"

class LiveEvent : public QWidget
{
    Q_OBJECT
private:
    LiveEventStatus* m_liveEventStatus;
    LiveEventDetail* m_liveEventDetail;
public:
    explicit LiveEvent(QWidget *parent = 0);
    ~LiveEvent();

    void forceActiveFocus();
    void updateDeviceList(void);

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);
    void sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index ,bool state);

public slots:
    void slotClosePage(TOOLBAR_BUTTON_TYPE_e index);
    void slotDeviceSelected(QString deviceName);
    void slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index ,bool state);
};

#endif // LIVEEVENT_H
