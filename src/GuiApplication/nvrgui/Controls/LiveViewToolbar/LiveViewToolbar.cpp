#include "LiveViewToolbar.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include "Layout/Layout.h"

static const quint8 imageWidth[] = {35, 43, 58};

static const quint8 imageHeight[] = {24, 30, 40};

static const type_e closeButtonSize[] = { CLOSE_BTN_TYPE_5,
                                          CLOSE_BTN_TYPE_4,
                                          CLOSE_BTN_TYPE_3 };

static const QString toolTipString[MAX_LIVEVIEW_BUTTON][MAX_STATE_TYPE] = {{"Main Stream", "Sub Stream"},
                                                                           {"Camera Settings Menu"},
                                                                           {"PTZ Control"},
                                                                           {"Snapshot"},
                                                                           {"Zoom"},
                                                                           {"Start Recording", "Stop Recording"},
                                                                           {"Start Audio", "Stop Audio"},
																		   {"Speak Now", "Stop"},
                                                                           {"Instant Playback"},
                                                                           {"Start Sequencing", "Stop Sequencing"},
                                                                           {"Expand"},
                                                                           {"Close"}};

LiveViewToolbar::LiveViewToolbar(quint16 startX,
                                 quint16 startY,
                                 quint16 width,
                                 quint16 height,
                                 quint16 windowIndex,
                                 WINDOW_ICON_TYPE_e windowIconType,
                                 QWidget *parent)
    : KeyBoard(parent)
{
    ApplController      *applController;
    DEV_TABLE_INFO_t    deviceInfo;

    applController = ApplController::getInstance();
    applController->GetDeviceInfo(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName, deviceInfo);

    m_totalButtons = MAX_LIVEVIEW_BUTTON;
    m_windowIndex = windowIndex;
    m_windowIconType = windowIconType;
    m_startX = startX;
    m_startY = startY;
    m_width = width;
    m_height = height;

    for(quint8 index = 0; index < MAX_LIVEVIEW_BUTTON; index++)
    {
        m_toolbarButton[index] = new LiveViewToolbarButton((LIVEVIEW_TOOLBAR_BUTTON_TYPE_e)index,
                                                           m_windowIconType,
                                                           this,
                                                           index);

        m_width += m_toolbarButton[index]->width ();

        m_elementList[index] = m_toolbarButton[index];
        connect(m_toolbarButton[index],
                SIGNAL(sigButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)),
                this,
                SLOT(slotButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)));
        connect(m_toolbarButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_toolbarButton[index],
                SIGNAL(sigShowHideToolTip(int,bool)),
                this,
                SLOT(slotShowHideTooltip(int,bool)));

    }

    m_width += SCALE_WIDTH(66);
    m_height = m_toolbarButton[0]->height () + SCALE_HEIGHT(40);

    if(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_streamType == LIVE_STREAM_TYPE_MAIN)
    {
        m_toolbarButton[LIVEVIEW_STREAMTYPE_BUTTON]->changeButtonState(STATE_2);
    }
    else
    {
        m_toolbarButton[LIVEVIEW_STREAMTYPE_BUTTON]->changeButtonState(STATE_1);
    }

    quint8 manualRecordingStatus;
    applController->GetHealtStatusSingleParamSingleCamera(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                                          manualRecordingStatus,
                                                          MANUAL_RECORDING_STS,
                                                          (Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId - 1));
    if(manualRecordingStatus == 1)
    {
        m_toolbarButton[LIVEVIEW_RECORDING_BUTTON]->changeButtonState(STATE_2);
    }

    if(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_audioStatus == true)
    {
        m_toolbarButton[LIVEVIEW_AUDIO_BUTTON]->changeButtonState(STATE_2);
    }

	if(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_microPhoneStatus == true)
	{
		m_toolbarButton[LIVEVIEW_MICROPHONE_BUTTON]->changeButtonState(STATE_2);
	}

    if(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].sequenceStatus == true)
    {
        m_toolbarButton[LIVEVIEW_SEQUENCING_BUTTON]->changeButtonState(STATE_2);
    }

    bool isAnalogChannel = false, isLocalDeviceAnalogChannel = false, isLocalDeviceChannel = false;
    CAMERA_TYPE_e cameraType = applController->GetCameraType(Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName,
                                                             (Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_cameraId - 1));
    isAnalogChannel = ((cameraType == ANALOG_CAMERA) ? true : false);

    if (Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_deviceName == LOCAL_DEVICE_NAME)
    {
		isLocalDeviceAnalogChannel = isAnalogChannel;
        isLocalDeviceChannel = true;
    }

    if(isLocalDeviceAnalogChannel == true)
    {
        m_totalButtons--;
        m_toolbarButton[LIVEVIEW_STREAMTYPE_BUTTON]->setVisible(false);
        m_toolbarButton[LIVEVIEW_STREAMTYPE_BUTTON]->setIsEnabled (false);

        for(quint8 index = LIVEVIEW_MENUSETTINGS_BUTTON; index < MAX_LIVEVIEW_BUTTON; index++)
        {
            m_toolbarButton[index]->resetGeometry(-1);
        }
    }
    else if(isAnalogChannel == false)
    {
        m_totalButtons--;
        m_toolbarButton[LIVEVIEW_MENUSETTINGS_BUTTON]->setVisible(false);
        m_toolbarButton[LIVEVIEW_MENUSETTINGS_BUTTON]->setIsEnabled(false);

        for(quint8 index = LIVEVIEW_PTZCONTROL_BUTTON; index < MAX_LIVEVIEW_BUTTON; index++)
        {
            m_toolbarButton[index]->resetGeometry(-1);
        }
    }

    if ((isLocalDeviceChannel == false) || (deviceInfo.audioIn == 0))
    {
        m_totalButtons--;
        m_toolbarButton[LIVEVIEW_MICROPHONE_BUTTON]->setVisible(false);
        m_toolbarButton[LIVEVIEW_MICROPHONE_BUTTON]->setIsEnabled(false);

        for(quint8 index = LIVEVIEW_INSTANTPLAYBACK_BUTTON; index < MAX_LIVEVIEW_BUTTON; index++)
        {
            m_toolbarButton[index]->resetGeometry(-1);
        }
    }

    bool isMultipleChannel = Layout::isMultipleChannelAssigned(MAIN_DISPLAY, windowIndex);
    if(isMultipleChannel == false)
    {
        m_totalButtons -= 2;
        m_toolbarButton[LIVEVIEW_SEQUENCING_BUTTON]->setVisible(false);
        m_toolbarButton[LIVEVIEW_SEQUENCING_BUTTON]->setIsEnabled (false);
        m_toolbarButton[LIVEVIEW_EXPAND_BUTTON]->setVisible(false);
        m_toolbarButton[LIVEVIEW_EXPAND_BUTTON]->setIsEnabled (false);

        for(quint8 index = LIVEVIEW_CLOSE_BUTTON; index < MAX_LIVEVIEW_BUTTON; index++)
        {
            m_toolbarButton[index]->resetGeometry(-2);
        }
    }

    this->setGeometry(m_startX, m_startY, m_width, m_height);

   m_currentElement = (isLocalDeviceAnalogChannel) ? LIVEVIEW_MENUSETTINGS_BUTTON: LIVEVIEW_STREAMTYPE_BUTTON;

    m_toolTip = new ToolTip(m_toolbarButton[m_currentElement]->x(),
                            (m_toolbarButton[m_currentElement]->y() + m_toolbarButton[m_currentElement]->height()),
                            toolTipString[m_currentElement][m_toolbarButton[m_currentElement]->getButtonState()],
                            this,
                            START_X_START_Y);

    m_toolTip->setVisible(true);

    isUnloadNeeded = false;

    m_keepAliveTimer = new QTimer(this);
    connect(m_keepAliveTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotKeepAliveTimeOut()));
    m_keepAliveTimer->setInterval (5000);
    m_keepAliveTimer->start();

    m_elementList[m_currentElement]->forceActiveFocus();

    this->installEventFilter (this);
    this->setMouseTracking (true);
    this->show();
}

LiveViewToolbar::~LiveViewToolbar()
{
    for(quint8 index = 0; index < MAX_LIVEVIEW_BUTTON; index++)
    {
        disconnect(m_toolbarButton[index],
                   SIGNAL(sigButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)),
                   this,
                   SLOT(slotButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e,STATE_TYPE_e)));
        disconnect(m_toolbarButton[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_toolbarButton[index],
                   SIGNAL(sigShowHideToolTip(int,bool)),
                   this,
                   SLOT(slotShowHideTooltip(int,bool)));
        delete m_toolbarButton[index];
    }

    delete m_toolTip;

    if(m_keepAliveTimer->isActive())
    {
        m_keepAliveTimer->stop();
    }

    disconnect(m_keepAliveTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotKeepAliveTimeOut()));
    delete m_keepAliveTimer;
}

void LiveViewToolbar::changeButtonState(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e button,
                                        STATE_TYPE_e state)
{
    m_toolbarButton[button]->changeButtonState(state);
    if((m_toolTip->isVisible())
            && (m_toolbarButton[button]->hasFocus()))
    {
        m_toolTip->textChange(toolTipString[button][m_toolbarButton[button]->getButtonState()]);
        m_toolTip->resetGeometry(m_toolbarButton[button]->x(),
                                 (m_toolbarButton[button]->y() + m_toolbarButton[button]->height()));
    }
}

quint16 LiveViewToolbar::getWindowIndex () const
{
    return m_windowIndex;
}

void LiveViewToolbar::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_LIVEVIEW_BUTTON) % MAX_LIVEVIEW_BUTTON;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveViewToolbar::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_LIVEVIEW_BUTTON;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveViewToolbar::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
        event->accept();
        takeLeftKeyAction();
        break;

    case Qt::Key_Right:
        event->accept();
        takeRightKeyAction();
        break;
    }
}

void LiveViewToolbar::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(m_currentElement == LIVEVIEW_CLOSE_BUTTON)
    {
        m_toolbarButton[LIVEVIEW_CLOSE_BUTTON]->takeEnterKeyAction ();
    }
    else
    {
        m_currentElement = LIVEVIEW_CLOSE_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void LiveViewToolbar:: mouseMoveEvent (QMouseEvent *)
{
    if(this->geometry ().contains (m_toolbarButton[m_currentElement]->getMousePos ()))
    {
        isUnloadNeeded = false;
    }
    else
    {
        isUnloadNeeded = true;
    }
}

bool LiveViewToolbar::eventFilter (QObject *obj, QEvent *event)
{
    if((event->type () == QEvent::HoverEnter) || (event->type ()== QEvent::Enter))
    {
        isUnloadNeeded = false;
        event->accept();
        return true;
    }
    else if((event->type() == QEvent::HoverLeave) || (event->type() == QEvent::Leave))
    {
        isUnloadNeeded = true;
        event->accept();
        return true;
    }
    else
    {
        return QObject::eventFilter(obj,event);
    }
}

void LiveViewToolbar::slotButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index,
                                        STATE_TYPE_e state)
{
    emit sigToolbarButtonClicked(index, state, m_windowIndex);
}

void LiveViewToolbar::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void LiveViewToolbar::slotShowHideTooltip(int index, bool toShowTooltip)
{
    if(toShowTooltip)
    {
        if(this->isVisible() == true)
        {
            m_toolTip->textChange(toolTipString[index][m_toolbarButton[index]->getButtonState()]);
            m_toolTip->resetGeometry(m_toolbarButton[index]->x(),
                                     (m_toolbarButton[index]->y() + m_toolbarButton[index]->height()));
            m_toolTip->setVisible(true);
        }
    }
    else
    {
        m_toolTip->setVisible(false);
    }
}

void LiveViewToolbar::slotKeepAliveTimeOut()
{
    if(isUnloadNeeded)
    {
        emit sigCloseLiveViewToolbar();
    }
}
