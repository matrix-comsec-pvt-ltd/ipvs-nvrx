#include "LiveViewToolbarButton.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>

#define LIVEVIEW_TOOLBAR_BUTTON_IMG_PATH    ":/Images_Nvrx/WindowIcon/"

const QString toolbarImgPath[] = {"Toolbar/StreamType/",
                                  "Toolbar/MenuSetting/",
                                  "Toolbar/PtzControl/",
                                  "Toolbar/SnapShot/",
                                  "Toolbar/Zoom/",
                                  "Toolbar/Recording/",
                                  "Toolbar/Audio/",
								  "Toolbar/Microphone/",
                                  "Toolbar/InstantPlayback/",
                                  "Toolbar/Sequencing/",
                                  "Toolbar/Expand/",
                                  "Toolbar/Close/"};

const QString stateImgPath[] = {"State_1/", "State_2/"};
const QString windowIconTypeImgPath[4] = {"3X3/", "4X4/" , "5X5/", "8X8/"};

LiveViewToolbarButton::LiveViewToolbarButton(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index,
                                             WINDOW_ICON_TYPE_e windowIconType,
                                             QWidget * parent,
                                             int indexInPage,
                                             bool isEnable)
    : KeyBoard(parent),
      NavigationControl(indexInPage, isEnable), m_windowIconType(windowIconType)
{
    m_clickEffectTimer = new QTimer(this);
    m_clickEffectTimer->setSingleShot(true);
    connect(m_clickEffectTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotClickEffectTimeOut()));

    m_isStateAvailable = ((index == LIVEVIEW_STREAMTYPE_BUTTON)
                          || (index == LIVEVIEW_RECORDING_BUTTON)
                          || (index == LIVEVIEW_AUDIO_BUTTON)
						  || (index == LIVEVIEW_MICROPHONE_BUTTON)
                          || (index == LIVEVIEW_SEQUENCING_BUTTON));
    m_currentState = STATE_1;
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

LiveViewToolbarButton::~LiveViewToolbarButton()
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

void LiveViewToolbarButton::changeButtonImage(IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    m_imageSource = QString(LIVEVIEW_TOOLBAR_BUTTON_IMG_PATH)
            + windowIconTypeImgPath[m_windowIconType]
            + toolbarImgPath[m_index]
            + (m_isStateAvailable ? stateImgPath[m_currentState] : "")
            + imgTypePath[m_currentImageType];
    m_buttonImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_buttonImage);
    update();
}

void LiveViewToolbarButton::changeButtonState(STATE_TYPE_e state)
{
    if(m_currentState != state)
    {
        m_currentState = state;
        changeButtonImage(m_currentImageType);
    }
}

void LiveViewToolbarButton::resetGeometry(qint16 offsetX)
{
    this->setGeometry((this->x() + (m_buttonImage.width() * offsetX)),
                      this->y(),
                      m_buttonImage.width(),
                      m_buttonImage.height());
}

STATE_TYPE_e LiveViewToolbarButton::getButtonState()
{
    return m_currentState;
}

void LiveViewToolbarButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeButtonImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void LiveViewToolbarButton::deSelectControl()
{
    if((m_currentImageType != IMAGE_TYPE_CLICKED))
    {
        changeButtonImage(IMAGE_TYPE_NORMAL);
    }
}

void LiveViewToolbarButton::takeEnterKeyAction()
{
    if(!m_clickEffectTimer->isActive())
    {
        changeButtonImage(IMAGE_TYPE_CLICKED);
        m_clickEffectTimer->start(75);
    }
}

void LiveViewToolbarButton::forceActiveFocus()
{
    this->setFocus();
}

void LiveViewToolbarButton::setIsEnabled(bool isEnable)
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

void LiveViewToolbarButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_buttonImage);
}

void LiveViewToolbarButton::mousePressEvent(QMouseEvent * event)
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

void LiveViewToolbarButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void LiveViewToolbarButton::mouseMoveEvent(QMouseEvent *event)
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


    m_mouseMovePoint = mapToGlobal (event->pos ());
    QWidget::mouseMoveEvent (event);
}

QPoint LiveViewToolbarButton::getMousePos () const
{
    return m_mouseMovePoint;
}

void LiveViewToolbarButton::focusInEvent(QFocusEvent *)
{
    selectControl();
    sigShowHideToolTip(m_index, true);
}

void LiveViewToolbarButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
    sigShowHideToolTip(m_index, false);
}

void LiveViewToolbarButton::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeEnterKeyAction();
}

void LiveViewToolbarButton::slotClickEffectTimeOut()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClicked(m_index, m_currentState);
}
