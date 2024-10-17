#include "LiveEventStatus.h"
#include <QKeyEvent>

#define DEVICE_STRING           "Devices"
#define EVENT_STRING            "Event"

#define TILE_LEFT_MARGIN        SCALE_WIDTH(20)
#define TILE_TOP_MARGIN         SCALE_HEIGHT(60)
#define NAME_TILE_WIDTH         SCALE_WIDTH(260)
#define COUNT_TILE_WIDTH        SCALE_WIDTH(100)
#define TILE_HEIGHT             SCALE_HEIGHT(30)
#define MENU_TEXT_LEFT_MARGIN   SCALE_WIDTH(20)

LiveEventStatus::LiveEventStatus(int startX, int startY, QWidget *parent) :
    BackGround(startX,
        startY,
        LIVE_EVENT_STATUS_WIDTH,
        LIVE_EVENT_STATUS_HEIGHT,
        BACKGROUND_TYPE_4,
        LIVE_EVENT_BUTTON,
        parent,
        0,
        0,
        "Event Notification"), m_deviceCount(0)
{
    m_currentDeviceIndex = 0;
    m_applController = ApplController::getInstance();

    for(quint8 index = 0; index < MAX_LIVE_EVENT_STATUS_ELEMENTS; index++)
    {
        m_elementList[index] = NULL;
    }

    m_deviceNameHeadingTile = new BgTile(TILE_LEFT_MARGIN,
                                         TILE_TOP_MARGIN,
                                         NAME_TILE_WIDTH,
                                         TILE_HEIGHT,
                                         COMMON_LAYER,
                                         this);

    m_eventCountHeadingTile = new BgTile((TILE_LEFT_MARGIN + NAME_TILE_WIDTH),
                                         TILE_TOP_MARGIN,
                                         COUNT_TILE_WIDTH,
                                         TILE_HEIGHT,
                                         COMMON_LAYER,
                                         this);

    m_deviceNameHeadingLabel = new TextLabel((TILE_LEFT_MARGIN + (NAME_TILE_WIDTH / 2)),
                                             (TILE_TOP_MARGIN + (TILE_HEIGHT / 2)),
                                             NORMAL_FONT_SIZE,
                                             DEVICE_STRING,
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_CENTRE_X_CENTER_Y);

    m_eventCountHeadingLabel = new TextLabel((TILE_LEFT_MARGIN + NAME_TILE_WIDTH + (COUNT_TILE_WIDTH / 2)),
                                             (TILE_TOP_MARGIN + (TILE_HEIGHT / 2)),
                                             NORMAL_FONT_SIZE,
                                             EVENT_STRING,
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_CENTRE_X_CENTER_Y);

    m_elementList[LIVE_EVENT_STATUS_CLOSE_BUTTON] = m_mainCloseButton;

    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_mainCloseButton->setIndexInPage(LIVE_EVENT_STATUS_CLOSE_BUTTON);
    getEnabledDevices();

    quint32 newHeight = (TILE_TOP_MARGIN + ((m_deviceCount+1)*TILE_HEIGHT) + SCALE_HEIGHT(20));
    resetGeometry(startX, (parent->height() / 2 - newHeight/ 2), LIVE_EVENT_STATUS_WIDTH,newHeight);

    m_currentElement = LIVE_EVENT_STATUS_MENUBUTTONS;
}

LiveEventStatus::~LiveEventStatus()
{
    m_enabledDeviceList.clear();
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    delete m_deviceNameHeadingTile;
    delete m_eventCountHeadingTile;

    delete m_deviceNameHeadingLabel;
    delete m_eventCountHeadingLabel;

    for(quint8 index = 0; index < m_deviceCount; index++)
    {
        disconnect(m_devices[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT(slotMenuButtonClicked(int)));
        disconnect(m_devices[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_devices[index];

        delete m_eventCountTile[index];

        delete m_eventCountLabel[index];
    }
}

void LiveEventStatus::getEnabledDevices()
{
    //get enabled devices
    m_applController->GetEnableDevList(m_deviceCount, m_enabledDeviceList);
    for(quint8 index = 0; index < m_deviceCount; index++)
    {
        m_deviceNames[index] = m_applController->GetDispDeviceName(m_enabledDeviceList.at(index));
    }

    //create menubuttonlist only for enabled device
    for(quint8 index = 0; index < m_deviceCount; index++)
    {
        m_devices[index] = new MenuButton(index,
                                          NAME_TILE_WIDTH,
                                          TILE_HEIGHT,
                                          m_deviceNames[index],this,
                                          MENU_TEXT_LEFT_MARGIN,
                                          TILE_LEFT_MARGIN,
                                          (TILE_TOP_MARGIN + TILE_HEIGHT),
                                          (LIVE_EVENT_STATUS_MENUBUTTONS + index),
                                          true,
                                          true,
                                          false,
                                          false,
                                          true);
        m_elementList[LIVE_EVENT_STATUS_MENUBUTTONS + index] = m_devices[index];

        connect(m_devices[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotMenuButtonClicked(int)));
        connect(m_devices[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    m_currentDeviceIndex = 0;
    getDevicesEventCount();
}

void LiveEventStatus::getDevicesEventCount()
{
    QStringList eventList;

    for(quint8 index = 0; index < m_deviceCount; index++)
    {
        m_applController->GetDeviceEventList(m_applController->GetRealDeviceName(m_deviceNames[index]), eventList);

        m_eventCount[index] = QString("%1").arg(eventList.length());

        m_eventCountTile[index] = new BgTile((TILE_LEFT_MARGIN + NAME_TILE_WIDTH),
                                             (TILE_TOP_MARGIN + TILE_HEIGHT + (index * TILE_HEIGHT)),
                                             COUNT_TILE_WIDTH,
                                             TILE_HEIGHT,
                                             COMMON_LAYER,
                                             this);

        m_eventCountLabel[index] = new TextLabel((TILE_LEFT_MARGIN + NAME_TILE_WIDTH + (COUNT_TILE_WIDTH / 2)),
                                                 (TILE_TOP_MARGIN + TILE_HEIGHT + (TILE_HEIGHT / 2) + (index * TILE_HEIGHT)),
                                                 NORMAL_FONT_SIZE,
                                                 m_eventCount[index],
                                                 this,
                                                 NORMAL_FONT_COLOR,
                                                 NORMAL_FONT_FAMILY,
                                                 ALIGN_CENTRE_X_CENTER_Y);
    }
    eventList.clear();
}

void LiveEventStatus::forceActiveFocus()
{
    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveEventStatus::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_LIVE_EVENT_STATUS_ELEMENTS)
                % MAX_LIVE_EVENT_STATUS_ELEMENTS;
    }while((m_elementList[m_currentElement] == NULL) || (!m_elementList[m_currentElement]->getIsEnabled()));

    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveEventStatus::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_LIVE_EVENT_STATUS_ELEMENTS;
    }while((m_elementList[m_currentElement] == NULL) ||(!m_elementList[m_currentElement]->getIsEnabled()));

    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveEventStatus::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void LiveEventStatus::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void LiveEventStatus::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void LiveEventStatus::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void LiveEventStatus::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = LIVE_EVENT_STATUS_CLOSE_BUTTON;

    if((m_currentElement < MAX_LIVE_EVENT_STATUS_ELEMENTS)
            && (IS_VALID_OBJ(m_elementList[m_currentElement])))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void LiveEventStatus::slotMenuButtonClicked(int index)
{
    emit sigDeviceSelected(m_applController->GetRealDeviceName(m_deviceNames[index]));
}

void LiveEventStatus::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}
