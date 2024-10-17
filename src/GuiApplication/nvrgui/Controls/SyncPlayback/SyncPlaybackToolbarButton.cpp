#include "SyncPlaybackToolbarButton.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>

#define SYNCPB_TOOLBAR_BUTTON_IMG_PATH ":/Images_Nvrx/SyncPlayBackToolbar/"

const QString toolbarModeImgPath[] = {"NormalMode/",
                                      "FullMode/"};

const QString toolbarImgPath[] = {"Play/",
                                  "Stop/",
                                  "ReversePlay/",
                                  "SlowPlay/",
                                  "FastPlay/",
                                  "PreviousFrame/",
                                  "NextFrame/",
                                  "Audio/",
                                  "Zoom/",
                                  "CropAndBackup/",
                                  "List/",
                                  "Layout/",
                                  "ChangeMode/"};

const QString stateImgPath [] = {"State_1/", "State_2/"};

SyncPlaybackToolbarButton::SyncPlaybackToolbarButton(SYNCPB_TOOLBAR_BUTTON_TYPE_e index,
                                                     QWidget * parent,
                                                     int indexInPage,
                                                     bool isEnable)
    : KeyBoard(parent),
      NavigationControl(indexInPage, isEnable)
{
    m_clickEffectTimer = new QTimer(this);
    m_clickEffectTimer->setSingleShot(true);
    connect(m_clickEffectTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotClickEffectTimeOut()));

    m_isStateAvailable = ((index == AUDIO_BUTTON)           
                          || (index == CROP_AND_BACKUP_BUTTON)
                          || (index == PLAY_BUTTON)
                          || (index == REVERSE_PLAY_BUTTON)
                          || (index == ZOOM_BUTTON)
                          || (index == CHANGE_MODE_BUTTON));
    m_currentState = STATE_1;
    m_currentMode = NORMAL_MODE;
    m_index = index;

    if(m_isEnabled)
    {
        changeButtonImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        changeButtonImage(IMAGE_TYPE_DISABLE);
    }

    this->setGeometry(QRect((m_buttonImage.width() * m_index),
                            0,
                            m_buttonImage.width(),
                            m_buttonImage.height()));

    this->setEnabled(m_isEnabled);
    this->show();
    this->setMouseTracking(true);
}

SyncPlaybackToolbarButton::~SyncPlaybackToolbarButton()
{
    if(m_clickEffectTimer->isActive())
    {
        m_clickEffectTimer->stop();
    }
    disconnect(m_clickEffectTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotClickEffectTimeOut()));
    delete m_clickEffectTimer;
}

void SyncPlaybackToolbarButton::changeButtonImage(IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    m_imageSource = QString(SYNCPB_TOOLBAR_BUTTON_IMG_PATH)
            + toolbarModeImgPath[m_currentMode]
            + toolbarImgPath[m_index]
            + (m_isStateAvailable ? stateImgPath[m_currentState] : "")
            + imgTypePath[m_currentImageType];
    m_buttonImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_buttonImage);
    update();
}

void SyncPlaybackToolbarButton::changeButtonState(STATE_TYPE_e state)
{
    if(m_currentState != state)
    {
        m_currentState = state;
        changeButtonImage(m_currentImageType);
    }
}

void SyncPlaybackToolbarButton::changeMode(SYNCPB_TOOLBAR_MODE_TYPE_e mode)
{
    m_currentMode = mode;
    changeButtonImage(m_currentImageType);
    this->setGeometry(QRect((m_buttonImage.width() * m_index),
                            0,
                            m_buttonImage.width(),
                            m_buttonImage.height()));
}

STATE_TYPE_e SyncPlaybackToolbarButton::getButtonState()
{
    return m_currentState;
}

void SyncPlaybackToolbarButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeButtonImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void SyncPlaybackToolbarButton::deSelectControl()
{
    if((m_currentImageType != IMAGE_TYPE_CLICKED))
    {
        changeButtonImage(IMAGE_TYPE_NORMAL);
    }
}

void SyncPlaybackToolbarButton::takeEnterKeyAction()
{
    if(!m_clickEffectTimer->isActive())
    {
        changeButtonImage(IMAGE_TYPE_CLICKED);
        m_clickEffectTimer->start(75);
    }
}

void SyncPlaybackToolbarButton::forceActiveFocus()
{
    this->setFocus();
}

void SyncPlaybackToolbarButton::setIsEnabled(bool isEnable)
{
    if(isEnable != m_isEnabled)
    {
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
        if(m_isEnabled == true)
        {
            if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
            {
                changeButtonImage(IMAGE_TYPE_NORMAL);
            }
        }
        else
        {
            changeButtonImage(IMAGE_TYPE_DISABLE);
        }
    }
}

void SyncPlaybackToolbarButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_buttonImage);
}

void SyncPlaybackToolbarButton::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == m_leftMouseButton)
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void SyncPlaybackToolbarButton::mouseReleaseEvent(QMouseEvent* event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void SyncPlaybackToolbarButton::mouseMoveEvent(QMouseEvent *)
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

void SyncPlaybackToolbarButton::focusInEvent(QFocusEvent *)
{
    selectControl();
    sigShowHideToolTip(m_index, true);
}

void SyncPlaybackToolbarButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
    sigShowHideToolTip(m_index, false);
}

void SyncPlaybackToolbarButton::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeEnterKeyAction();
}

void SyncPlaybackToolbarButton::slotClickEffectTimeOut()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClicked(m_index, m_currentState);
}
