#include "SliderControl.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>

#define SLIDER_CLICK_EFFECT_TIMEOUT (150)

SliderControl::SliderControl(int startX,
                             int startY,
                             int width,
                             int height,
                             int activeWidth,
                             int activeHeight,
                             QString imagePath,
                             int currentValue,
                             QWidget *parent,
                             SLIDER_TYPE_e sliderType,
                             int indexInPage,
                             bool isEnabled,
                             bool isActiveBarNeeded,
                             bool isControlOnEnter,
                             bool isValueChangeOnClickAllowed,
                             bool isHoverConfinedToImageOnly )
    : KeyBoard(parent),
      NavigationControl(indexInPage, isEnabled, (!isControlOnEnter)), m_sliderType(sliderType),
      m_startX(startX), m_startY(startY), m_width(width), m_height(height), m_activeWidth(activeWidth),
      m_activeHeight(activeHeight),  m_imagePath(imagePath), m_currentImageType(MAX_IMAGE_TYPE),  m_currentValue(currentValue),  m_isActiveBarNeeded(isActiveBarNeeded),
      m_isControlOnEnter(isControlOnEnter), m_isHoverConfinedToImageOnly(isHoverConfinedToImageOnly), m_isValueChangeOnClickAllowed(isValueChangeOnClickAllowed),
      m_activeAreaClicked(false),  m_mouseClickedOnImage(false), m_remoteSlidingMode(false),
      m_lastClickPoint(QPoint(0, 0))
{
    m_mouseClicked = false;
    clickEffectTimer = new QTimer();
    connect (clickEffectTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotClickEffectTimerTimeout()));
    clickEffectTimer->setInterval (SLIDER_CLICK_EFFECT_TIMEOUT);
    clickEffectTimer->setSingleShot (true);


    changeImage(IMAGE_TYPE_NORMAL);
    setGeometryForElements();

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->installEventFilter (this);
    this->show();
}

SliderControl::~SliderControl()
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

void SliderControl::setGeometryForElements()
{
    this->setGeometry(QRect(m_startX, m_startY, m_width, m_height));

    switch(m_sliderType)
    {
    case VERTICAL_SLIDER:
        m_horizontalMargin = (m_width - m_activeWidth) / 2;
        m_verticalMargin = (m_height - m_activeHeight) / 2;
        m_imageRect.setRect(((m_width - m_image.width()) / 2),
                            ((m_height - m_verticalMargin - m_currentValue) - (m_image.height() / 2)),
                            m_image.width(),
                            m_image.height());

        m_activeBar.setRect(m_horizontalMargin,
                            (m_height - m_verticalMargin - m_currentValue),
                            m_activeWidth,
                            m_currentValue);
        m_activeRect.setRect(0, m_verticalMargin,
                              m_width, m_activeHeight);
        break;

    case HORIZONTAL_SLIDER:
        m_horizontalMargin = (m_width - m_activeWidth) / 2;
        m_verticalMargin = (m_height - m_activeHeight) / 2;
        m_imageRect.setRect((m_horizontalMargin + m_currentValue - (m_image.width() / 2)),
                            ((m_height - m_image.height()) / 2),
                            m_image.width(),
                            m_image.height());

        m_activeBar.setRect(m_horizontalMargin,
                            m_verticalMargin,
                            m_currentValue,
                            m_activeHeight);
        m_activeRect.setRect (m_horizontalMargin, 0,
                              (m_activeWidth + 1), m_height);
        break;
    }
    m_rect.setRect (0, 0, m_width, m_height);
}

void SliderControl::changeImage(IMAGE_TYPE_e imageType)
{
    if(m_currentImageType != imageType)
    {
        m_currentImageType = imageType;
        QString imageSource = m_imagePath + imgTypePath[imageType];
        m_image = QPixmap(imageSource);
        SCALE_IMAGE(m_image);
        switch(m_sliderType)
        {
        case VERTICAL_SLIDER:
            m_imageRect.setRect(((m_width - m_image.width()) / 2),
                                ((m_height - m_verticalMargin - m_currentValue) - (m_image.height() / 2)),
                                m_image.width(),
                                m_image.height());
            break;

        case HORIZONTAL_SLIDER:
            m_imageRect.setRect((m_horizontalMargin + m_currentValue - (m_image.width() / 2)),
                                ((m_height - m_image.height()) / 2),
                                m_image.width(),
                                m_image.height());
            break;
        }
        update();
    }
}

void SliderControl::resetGeometry(int Offset, bool sliderMove, bool toEmitSignal)
{
    bool updateFlag = false;

    switch(m_sliderType)
    {
    case VERTICAL_SLIDER:
        if(((m_currentValue + Offset) >= 0) && ((m_currentValue + Offset) <= m_activeHeight))
        {
            m_currentValue += Offset;
        }
        else if((m_currentValue + Offset) < 0)
        {
            m_currentValue = 0;
        }
        else
        {
            m_currentValue = m_activeHeight;
        }
        m_activeBar.setRect(m_horizontalMargin,
                            (m_height - m_verticalMargin - m_currentValue),
                            m_activeWidth,
                            m_currentValue);
        m_imageRect.setRect(m_imageRect.x(),
                            ((m_height - m_verticalMargin - m_currentValue) - (m_image.height() / 2)),
                            m_imageRect.width(),
                            m_imageRect.height());
        updateFlag = true;
        break;

    case HORIZONTAL_SLIDER:
        if(((m_currentValue + Offset) >= 0) && ((m_currentValue + Offset) <= m_activeWidth))
        {
            m_currentValue += Offset;
        }
        else if((m_currentValue + Offset) < 0)
        {
            m_currentValue = 0;
        }
        else
        {
            m_currentValue = m_activeWidth;
        }

        m_activeBar.setRect(m_activeBar.x(),
                            m_verticalMargin,
                            m_currentValue,
                            m_activeHeight);
        m_imageRect.setRect((m_horizontalMargin + m_currentValue - (m_image.width() / 2)),
                            m_imageRect.y(),
                            m_imageRect.width(),
                            m_imageRect.height());
        updateFlag = true;
        break;

    default:
        break;
    }

    if(updateFlag)
    {
        update();

        if(toEmitSignal)
        {
            emit sigValueChanged(m_currentValue, m_indexInPage, sliderMove);
        }

        if(m_remoteSlidingMode)
        {
            emit sigHoverOnSlider(m_currentValue, m_indexInPage);
        }
    }
}

void SliderControl::changeValue(int value)
{
    if(clickEffectTimer->isActive() == false)
    {
        if(!((m_mouseClickedOnImage) || (m_activeAreaClicked) || (m_remoteSlidingMode)))
        {
            resetGeometry(value - m_currentValue);
        }
    }
}

int SliderControl::getCurrentValue()
{
    return m_currentValue;
}

void SliderControl::setCurrentValue(int value)
{
    resetGeometry((value - m_currentValue), false, false);
}

void SliderControl::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_CLICKED)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void SliderControl::deSelectControl()
{
    changeImage(IMAGE_TYPE_NORMAL);
}

void SliderControl::forceActiveFocus()
{
    this->setFocus();
}

void SliderControl::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        setEnabled(m_isEnabled);

        if(isEnable)
        {
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
        }
    }
}

void SliderControl::takeUpKeyAction()
{
    resetGeometry(1,true);
}

void SliderControl::takeDownKeyAction()
{
    resetGeometry(-1,true);
}

void SliderControl::takeEnterKeyAction()
{
    if(m_currentImageType == IMAGE_TYPE_CLICKED)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
    else
    {
        changeImage(IMAGE_TYPE_CLICKED);
    }

    if(m_isControlOnEnter)
    {
        m_catchKey = !m_catchKey;
        m_remoteSlidingMode = !m_remoteSlidingMode;

        if(m_remoteSlidingMode == false)
        {
            emit sigMouseReleaseOnSlider(m_currentValue, m_indexInPage);
            emit sigHoverInOutOnSlider(false, m_indexInPage);
        }
        else
        {
            emit sigMousePressedOnSlider(m_currentValue, m_indexInPage);
            emit sigHoverOnSlider(m_currentValue, m_indexInPage);
        }
    }
}

void SliderControl::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush (QBrush(QColor(HIGHLITED_FONT_COLOR), Qt::SolidPattern));
    if(m_isActiveBarNeeded)
    {
        painter.drawRect(m_activeBar);
    }
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(m_imageRect, m_image);
}

void SliderControl::mousePressEvent(QMouseEvent * event)
{
    if((event->button() == m_leftMouseButton) &&
            (m_mouseClicked == false))
    {
        m_mouseClicked = true;
        if(m_imageRect.contains(event->pos()))
        {
            m_mouseClickedOnImage = true;
            m_lastClickPoint = QPoint(event->pos());
            changeImage(IMAGE_TYPE_CLICKED);
            emit sigMousePressedOnSlider(m_currentValue, m_indexInPage);
        }
        else if((m_isValueChangeOnClickAllowed)
                && (m_activeRect.contains(event->pos())))
        {
            m_activeAreaClicked = true;
            switch(m_sliderType)
            {
            case HORIZONTAL_SLIDER:                
                resetGeometry(event->x() - (m_imageRect.x() + m_imageRect.width() / 2), true);
                break;
            case VERTICAL_SLIDER:
                resetGeometry((m_imageRect.y() + m_imageRect.height() / 2) - event->y(), true);
                break;
            }
            emit sigMousePressedOnSlider(m_currentValue, m_indexInPage);
        }
    }
}

void SliderControl::mouseReleaseEvent(QMouseEvent *event)
{
    changeImage(IMAGE_TYPE_MOUSE_HOVER);
    if((event->button() == m_leftMouseButton) && (m_mouseClicked == true))
    {
        if(m_mouseClickedOnImage == true)
        {
            m_mouseClickedOnImage = false;            
        }
        else if(m_activeAreaClicked == true)
        {
            m_activeAreaClicked = false;            
        }
        if((!clickEffectTimer->isActive()))
        {
            clickEffectTimer->start();
        }
    }
}

/* When mouse is pressed and if ypu move the mouse then you get mouse move event outside the widget too and when mouse is released QEvent::Enter signal is generated */
void SliderControl::mouseMoveEvent(QMouseEvent * event)
{
    if(m_isControlActivated)
    {
        if(((m_isHoverConfinedToImageOnly) && (m_imageRect.contains(event->pos())))
                || ((!m_isHoverConfinedToImageOnly) && (m_activeRect.contains(event->pos()))))
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

        if(m_mouseClickedOnImage)
        {
            if(m_imageRect.contains(event->pos()))
            {
                m_lastClickPoint = QPoint(event->pos());
                return;
            }
            switch(m_sliderType)
            {
            case VERTICAL_SLIDER:
                resetGeometry(m_lastClickPoint.y() - event->y(), true);
                break;
            case HORIZONTAL_SLIDER:
                resetGeometry(event->x() - m_lastClickPoint.x(), true);
                break;
            }
            m_lastClickPoint = QPoint(event->pos());
            emit sigHoverOnSlider(m_currentValue, m_indexInPage);
        }
        else if(m_activeRect.contains(event->pos()))
        {
            // only works in horizontal slider
            emit sigHoverOnSlider((event->x() - m_horizontalMargin), m_indexInPage);
        }
    }
}

void SliderControl::resetFlags()
{
    m_activeAreaClicked = false;
    m_mouseClickedOnImage = false;
    m_remoteSlidingMode = false;
}

bool SliderControl::eventFilter(QObject *obj, QEvent *event)
{
    if((event->type () == QEvent::HoverEnter) || (event->type ()== QEvent::Enter))
    {
        emit sigHoverInOutOnSlider(true, m_indexInPage);
        event->accept();
        return true;
    }
    else if((event->type() == QEvent::HoverLeave) || (event->type() == QEvent::Leave))
    {
        emit sigHoverInOutOnSlider(false, m_indexInPage);
        if((m_mouseClickedOnImage) || (m_activeAreaClicked))
        {
            m_mouseClickedOnImage = false;
            m_activeAreaClicked = false;
            emit sigMouseReleaseOnSlider(m_currentValue, m_indexInPage);
        }
        event->accept();
        return true;
    }
    else
    {
        return QObject::eventFilter(obj,event);
    }
}

void SliderControl::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void SliderControl::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
    emit sigHoverInOutOnSlider(false, m_indexInPage);
}

void SliderControl::navigationKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        if(m_sliderType == VERTICAL_SLIDER)
        {
            switch(event->key())
            {
            case Qt::Key_Up:
                if(m_catchKey)
                {
                    event->accept();
                    takeUpKeyAction();
                }
                else
                {
                    event->accept();
                }
                break;

            case Qt::Key_Down:
                if(m_catchKey)
                {
                    event->accept();
                    takeDownKeyAction();
                }
                else
                {
                    event->accept();
                }
                break;
            }
        }
        else if(m_sliderType == HORIZONTAL_SLIDER)
        {
            switch(event->key())
            {
            case Qt::Key_Right:
                if(m_catchKey)
                {
                    event->accept();
                    takeUpKeyAction();
                }
                else
                {
                    event->accept();
                }
                break;

            case Qt::Key_Left:
                if(m_catchKey)
                {
                    event->accept();
                    takeDownKeyAction();
                }
                else
                {
                    event->accept();
                }
                break;
            }
        }
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void SliderControl::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeEnterKeyAction();
}

void SliderControl::tabKeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_isControlOnEnter))
    {
        event->accept();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void SliderControl::backTab_KeyPressed(QKeyEvent *event)
{
    if((m_catchKey) && (m_isControlOnEnter))
    {
        event->accept();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}


void SliderControl::slotClickEffectTimerTimeout()
{
    m_mouseClicked = false;
    emit sigMouseReleaseOnSlider(m_currentValue, m_indexInPage);
}

void SliderControl ::wheelEvent(QWheelEvent* event)
{
    int offSetVal;

    switch(m_sliderType)
    {
    case VERTICAL_SLIDER:
        offSetVal = (event->delta()/120) *5;
        if((m_currentValue + offSetVal) > m_activeHeight)
        {
            offSetVal =m_activeHeight- m_currentValue;
        }
        else if((m_currentValue + offSetVal) < 0 )
        {
            offSetVal =  m_currentValue*(-1);
        }

        resetGeometry(offSetVal, true);
        break;
    case HORIZONTAL_SLIDER:
        break;
    }
}
