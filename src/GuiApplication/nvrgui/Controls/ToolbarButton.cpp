#include "ToolbarButton.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include "ApplicationMode.h"
#include "ApplController.h"

#define TOOLBAR_BUTTON_IMG_PATH ":/Images_Nvrx/ToolbarButtons/"
#define BUTTON_CLICK_EFFECT_TIMEOUT (250)
const QString toolbarImgPath[MAX_TOOLBAR_BUTTON] = {"AboutUsButton/",
                                                    "LogButton/",
                                                    "LiveViewButton/",
                                                    "DisplayModeButton/",
                                                    "AsyncPlaybackButton/",
                                                    "SyncPlaybackButton/",
                                                    "EventLogButton/",
                                                    "SettingsButton/",
                                                    "ManageButton/",
                                                    "SystemStatusButton/",
                                                    "AudioControlButton/",
                                                    "SequenceButton/",
                                                    "QuickBackup/",
                                                    "VideoPopupButton/",
                                                    "WizardButton/",
                                                    "", /* Style Select */
                                                    "CollapseButton/",
                                                    "BuzzerControlButton/",
                                                    "LiveEventButton/",
                                                    "UsbControlButton/",
                                                    "CpuLoadsIcons/"};

const QString stateImgPath [] = {"State1/", "State2/"};

ToolbarButton::ToolbarButton(TOOLBAR_BUTTON_TYPE_e index,
                             QWidget * parent,
                             int indexInPage,
                             int horizontalOffset,
                             int verticalOffset,
                             bool isEnable,
                             bool isHoverEffectNeeded)
    : KeyBoard(parent), NavigationControl(indexInPage, isEnable),
       m_isMouseEffectNeeded(isHoverEffectNeeded), m_horizontalOffset(horizontalOffset),
       m_verticalOffset(verticalOffset), m_currentState(STATE_1), m_index(index), m_clickIndex(MAX_TOOLBAR_BUTTON)
{
    m_isStateAvailable = ((index == AUDIO_CONTROL_BUTTON)
                          || (index == SEQUENCE_BUTTON)
                          || (index == LOG_BUTTON)
                          || (index == QUICK_BACKUP)
                          || (index == VIDEO_POPUP_BUTTON));


    m_giveClickEffect = ((index == DISPLAY_MODE_BUTTON) || (index == AUDIO_CONTROL_BUTTON) || (index == USB_CONTROL_BUTTON));

    changeButtonImage(IMAGE_TYPE_NORMAL);
    this->setGeometry(QRect(((m_buttonImage.width() * (m_index % STYLE_SELECT_BUTTON)) + m_horizontalOffset),
                            m_verticalOffset,
                            m_buttonImage.width(),
                            m_buttonImage.height()));

    this->setEnabled(m_isEnabled);
    QWidget::setVisible(m_isEnabled);
    this->setMouseTracking(true);

        clickEffectTimer = new QTimer();
        connect (clickEffectTimer,
                SIGNAL(timeout()),
                 this,
                 SLOT(slotClickEffectTimerTimeout()));
        clickEffectTimer->setInterval (BUTTON_CLICK_EFFECT_TIMEOUT);
        clickEffectTimer->setSingleShot (true);
}

void ToolbarButton::changeButtonImage(IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    m_imageSource = QString(TOOLBAR_BUTTON_IMG_PATH) + toolbarImgPath[m_index] + (m_isStateAvailable ? stateImgPath[m_currentState] : "") + imgTypePath[m_currentImageType];
    m_buttonImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_buttonImage);
    repaint();
}

void ToolbarButton::changeButtonState(STATE_TYPE_e state)
{
    m_currentState = state;
    changeButtonImage(m_currentImageType);
}

STATE_TYPE_e ToolbarButton::getCurrentButtonState() const
{
    return m_currentState;
}

TOOLBAR_BUTTON_TYPE_e ToolbarButton::getButtonIndex() const
{
    return m_index;
}

IMAGE_TYPE_e ToolbarButton::getButtonImageType () const
{
    return m_currentImageType;
}


void ToolbarButton::selectControl()
{
    if((m_currentImageType != IMAGE_TYPE_MOUSE_HOVER) && (m_isMouseEffectNeeded))
    {
        changeButtonImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void ToolbarButton::deSelectControl()
{
    if((m_currentImageType != IMAGE_TYPE_CLICKED) && (m_isMouseEffectNeeded))
    {
        changeButtonImage(IMAGE_TYPE_NORMAL);
    }
}

void ToolbarButton::takeEnterKeyAction()
{
    if(m_isMouseEffectNeeded)
    {
        changeButtonImage(IMAGE_TYPE_CLICKED);
        if(!m_giveClickEffect)
        {
            changeButtonImage(IMAGE_TYPE_NORMAL);
        }
        if((m_index != AUDIO_CONTROL_BUTTON) || (!clickEffectTimer->isActive()) )
        {
            emit sigButtonClicked(m_index);
        }
    }
}

void ToolbarButton::setOffset(int horizontalOffset, int verticalOffset)
{
    m_horizontalOffset = horizontalOffset;
    m_verticalOffset = verticalOffset;
}

void ToolbarButton::updateGeometry()
{
    changeButtonImage(IMAGE_TYPE_NORMAL);
    this->setGeometry(QRect(((m_buttonImage.width() * (m_index % STYLE_SELECT_BUTTON)) + m_horizontalOffset),
                            m_verticalOffset,
                            m_buttonImage.width(),
                            m_buttonImage.height()));
}

void ToolbarButton::forceActiveFocus()
{
    this->setFocus();
}

void ToolbarButton::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
        setVisible(isEnable);
    }
}

void ToolbarButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_buttonImage);
}

void ToolbarButton::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == m_leftMouseButton)
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }

        if((!clickEffectTimer->isActive()) && (m_index == AUDIO_CONTROL_BUTTON))
        {
            clickEffectTimer->start();
        }
    }

}

void ToolbarButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void ToolbarButton::mouseMoveEvent(QMouseEvent * event)
{
    if((ApplicationMode::getApplicationMode() == TOOLBAR_MODE)
            && (m_isControlActivated))
    {
        if(this->hasFocus())
        {
            selectControl();
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void ToolbarButton::focusInEvent(QFocusEvent *)
{
    selectControl();
    sigShowHideToolTip(m_index, true);
}

void ToolbarButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
    sigShowHideToolTip(m_index, false);
}

void ToolbarButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void ToolbarButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    if((clickEffectTimer->isActive()))
    {
        clickEffectTimer->stop();
    }
    if(event->button() == m_leftMouseButton)
    {
        m_clickIndex = getButtonIndex();
        if(m_clickIndex == AUDIO_CONTROL_BUTTON)
        {
            emit sigChangeMuteUnmute();
        }
    }
}

void ToolbarButton::slotClickEffectTimerTimeout()
{
    emit sigButtonClicked(m_index);
}

ToolbarButton::~ToolbarButton()
{
    if(IS_VALID_OBJ(clickEffectTimer))
    {
        if(clickEffectTimer->isActive ())
        {
            clickEffectTimer->stop ();
        }
        disconnect (clickEffectTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotClickEffectTimerTimeout()));
        DELETE_OBJ (clickEffectTimer);
    }
}
