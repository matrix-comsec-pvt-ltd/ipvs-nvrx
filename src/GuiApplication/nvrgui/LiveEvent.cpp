#include "LiveEvent.h"
#include <QPainter>

LiveEvent::LiveEvent(QWidget *parent) : QWidget(parent)
{
    this->show();
    this->setGeometry(ApplController::getXPosOfScreen(),
                      ApplController::getYPosOfScreen(),
                      ApplController::getWidthOfScreen(),
                      ApplController::getHeightOfScreen());

    m_liveEventStatus = new LiveEventStatus((this->width() / 2 - LIVE_EVENT_STATUS_WIDTH / 2),
                                            (this->height() / 2 - LIVE_EVENT_STATUS_HEIGHT / 2),
                                            this);
    connect(m_liveEventStatus,
            SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));

    connect(m_liveEventStatus,
            SIGNAL(sigDeviceSelected(QString)),
            this,
            SLOT(slotDeviceSelected(QString)));

    m_liveEventDetail = new LiveEventDetail((this->width() / 2 - LIVE_EVENT_DETAIL_WIDTH / 2),
                                            (this->height() / 2 - LIVE_EVENT_DETAIL_HEIGHT / 2),
                                            this);

    connect(m_liveEventDetail,
            SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));

    connect(m_liveEventDetail,
            SIGNAL(sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e,bool)),
            this,
            SLOT(slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e,bool)));

    m_liveEventDetail->setVisible(false);
}

LiveEvent::~LiveEvent()
{
    disconnect(m_liveEventStatus,
               SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
               this,
               SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));

    disconnect(m_liveEventStatus,
               SIGNAL(sigDeviceSelected(QString)),
               this,
               SLOT(slotDeviceSelected(QString)));
    delete m_liveEventStatus;

    disconnect(m_liveEventDetail,
               SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
               this,
               SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
    disconnect(m_liveEventDetail,
               SIGNAL(sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e,bool)),
               this,
               SLOT(slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e,bool)));
    delete m_liveEventDetail;
}

void LiveEvent::forceActiveFocus()
{
    m_liveEventStatus->forceActiveFocus();
}

void LiveEvent::slotClosePage(TOOLBAR_BUTTON_TYPE_e index)
{
    emit sigClosePage(index);
}

void LiveEvent::slotDeviceSelected(QString deviceName)
{
    m_liveEventStatus->setVisible(false);    
    m_liveEventDetail->setVisible(true);
    m_liveEventDetail->forceActiveFocus();
    m_liveEventDetail->changeDevice(deviceName);
}

void LiveEvent::slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index, bool state)
{
    emit sigNotifyStatusIcon(index, state);
}

void LiveEvent::updateDeviceList(void)
{
    if ((m_liveEventStatus != NULL) && (m_liveEventDetail != NULL))
    {
        if (m_liveEventStatus->isVisible())
        {
            disconnect(m_liveEventStatus,
                       SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                       this,
                       SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));

            disconnect(m_liveEventStatus,
                       SIGNAL(sigDeviceSelected(QString)),
                       this,
                       SLOT(slotDeviceSelected(QString)));
            delete m_liveEventStatus;

            m_liveEventStatus = new LiveEventStatus((this->width() / 2 - LIVE_EVENT_STATUS_WIDTH / 2),
                                                    (this->height() / 2 - LIVE_EVENT_STATUS_HEIGHT / 2),
                                                    this);
            connect(m_liveEventStatus,
                    SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
                    this,
                    SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));

            connect(m_liveEventStatus,
                    SIGNAL(sigDeviceSelected(QString)),
                    this,
                    SLOT(slotDeviceSelected(QString)));
            m_liveEventDetail->setVisible(false);
        }

        m_liveEventDetail->updateDeviceList();
    }
}
