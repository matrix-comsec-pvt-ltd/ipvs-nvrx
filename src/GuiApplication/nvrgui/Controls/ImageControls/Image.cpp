#include "Image.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>

#define BUTTON_PRESS_INTERVAL 220

Image::Image(int xParam,
             int yParam,
             QString imageSource,
             QWidget *parent,
             POINT_PARAM_TYPE_e pointparamType,
             int indexInPage,
             bool isEnabled,
             bool isCompleteImageSource,
             bool catchKey, bool isPressTimerReq) :
    KeyBoard(parent), NavigationControl(indexInPage, isEnabled, catchKey),
    m_pointParamType(pointparamType), m_imgSource(imageSource)
{
    m_isCompleteImageSource = isCompleteImageSource;
    currentIndex = indexInPage;
    m_xParam = xParam;
    m_yParam = yParam;
    m_alreadyHover = false;
    m_isPressTimerReq = isPressTimerReq;

    if(m_isPressTimerReq)
    {
        mouseButtonPressTimer = new QTimer(this);
        connect (mouseButtonPressTimer,
                 SIGNAL(timeout()),
                 this,
                 SLOT(slotmouseButtonPressTimerTimeout()));
        mouseButtonPressTimer->setInterval (BUTTON_PRESS_INTERVAL);
    }

    clickeffctTimer = new QTimer(this);
    clickeffctTimer->setInterval (75);
    clickeffctTimer->setSingleShot (true);
    connect (clickeffctTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotclickeffctTimerTimeout()));

    m_currentImageType = MAX_IMAGE_TYPE;

    if(m_isEnabled)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
    }
    setGeometryForElements();
    this->setEnabled(m_isEnabled);
    this->installEventFilter(this);
    this->setMouseTracking(true);
    this->show();
}

Image::~Image()
{
    if(m_isPressTimerReq)
    {
        if(mouseButtonPressTimer->isActive ())
        {
            mouseButtonPressTimer->stop ();
        }

        disconnect (mouseButtonPressTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotmouseButtonPressTimerTimeout()));
        delete mouseButtonPressTimer;
    }

    if(clickeffctTimer->isActive ())
    {
        clickeffctTimer->stop ();
    }
    disconnect (clickeffctTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotclickeffctTimerTimeout()));
    delete clickeffctTimer;
}

void Image::setGeometryForElements()
{
    switch(m_pointParamType)
    {
    case CENTER_X_CENTER_Y:
        m_startX = (m_xParam - (m_image.width() / 2));
        m_startY = (m_yParam - (m_image.height() / 2));
        break;

    case START_X_START_Y:
        m_startX = m_xParam;
        m_startY = m_yParam;
        break;

    case START_X_CENTER_Y:
        m_startX = m_xParam;
        m_startY = (m_yParam - (m_image.height() / 2));
        break;

    case END_X_END_Y:
        m_startX = (m_xParam - m_image.width());
        m_startY = (m_yParam - m_image.height());
        break;

    case END_X_CENTER_Y:
        m_startX = (m_xParam - m_image.width());
        m_startY = (m_yParam - (m_image.height() / 2));
        break;

    case END_X_START_Y:
        m_startX = (m_xParam - m_image.width());
        m_startY = m_yParam;
        break;

    case START_X_END_Y:
        m_startX = m_xParam;
        m_startY = (m_yParam - m_image.height());
        break;

    case CENTER_X_START_Y:
        m_startX = (m_xParam - (m_image.width() / 2));
        m_startY = m_yParam;
        break;

    default:
        break;
    }

    m_imageRect.setRect(0,
                        0,
                        m_image.width(),
                        m_image.height());
    if((m_image.width() == 0) || (m_image.height() == 0))
    {
        this->setGeometry(QRect(m_startX, m_startY, 1, 1));
    }
    else
    {
        this->setGeometry(QRect(m_startX, m_startY, m_image.width(), m_image.height()));
    }
}

void Image::changeImage(IMAGE_TYPE_e type, bool isForceupdate)
{
    quint16 imageWidth = m_image.width();
    quint16 imageHeight = m_image.height();
    bool updateFlag = isForceupdate;
    if(m_isCompleteImageSource)
    {
        m_image = QPixmap(m_imgSource);
        SCALE_IMAGE(m_image);
        updateFlag = true;
    }
    else
    {
        if((type != m_currentImageType) || (isForceupdate == true))
        {
            m_currentImageType = type;
            QString imageSource = m_imgSource + imgTypePath[type];
            m_image = QPixmap(imageSource);
            SCALE_IMAGE(m_image);
            updateFlag = true;
        }
    }

    if(updateFlag)
    {
        if((imageWidth != m_image.width())
                || (imageHeight != m_image.height()))
        {
            setGeometryForElements();
        }
        update();
    }
}

void Image::updateImageSource(QString imgSource, bool isForceupdate)
{
    if(m_imgSource != imgSource)
    {
        m_imgSource = imgSource;
        changeImage(m_currentImageType, isForceupdate);
    }
}

QString Image::getImageSource()
{
    return m_imgSource;
}

IMAGE_TYPE_e Image::getImageType()
{
    return m_currentImageType;
}

void Image::resetGeometry(int xParam, int yParam)
{
    m_xParam = xParam;
    m_yParam = yParam;
    setGeometryForElements();
}

void Image::selectControl()
{
    if(m_isEnabled)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
        emit sigImageMouseHover(m_indexInPage,true);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
    }
}

void Image::deSelectControl()
{
    if(m_isEnabled)
    {
        changeImage(IMAGE_TYPE_NORMAL);
        emit sigImageMouseHover(m_indexInPage,false);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
    }
}

void Image::forceActiveFocus()
{
    this->setFocus();
}

void Image::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);

        if(isEnable == true)
        {
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
        }
    }
}

void Image::takeEnterKeyAction()
{
    emit sigImageClicked(m_indexInPage);
}

void Image::takeDoubleClickEnterKeyAction()
{
    emit sigImageDoubleClicked(m_indexInPage);
}

void Image::takeClickAction ()
{
    if(!clickeffctTimer->isActive())
    {
        changeImage(IMAGE_TYPE_CLICKED);
        clickeffctTimer->start();
    }
}

void Image::scale(int width,int height)
{
    m_image = m_image.scaled (width,height,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    this->setGeometry(QRect(m_startX, m_startY, m_image.width(), m_image.height()));
    m_imageRect.setRect(0,0,width,height);

    update ();
}

void Image::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(m_imageRect, m_image);
}

void Image::mousePressEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        if(m_isPressTimerReq)
        {
            mouseButtonPressTimer->start();
        }

        if (true == m_isPressTimerReq)
        {
            takeEnterKeyAction();
        }

        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
        changeImage(IMAGE_TYPE_CLICKED);
    }
    else
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
    QWidget:: mousePressEvent (event);
}

void Image::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
        if (false == m_isPressTimerReq)
        {
            takeEnterKeyAction();
        }
    }
    m_mouseClicked = false;

    if(m_isPressTimerReq)
    {
        if(mouseButtonPressTimer->isActive ())
        {
            mouseButtonPressTimer->stop ();
        }
    }
    QWidget:: mouseReleaseEvent (event);
}

void Image::mouseDoubleClickEvent(QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
        takeDoubleClickEnterKeyAction();
    }
}

void Image::mouseMoveEvent(QMouseEvent *event)
{
    if(m_imageRect.contains(event->pos()))
    {
        if(this->hasFocus())
        {
            if(m_mouseClicked)
            {
                changeImage(IMAGE_TYPE_CLICKED);
            }
            else
            {
                 selectControl();
            }
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
    QWidget:: mouseMoveEvent (event);
}

bool Image::eventFilter(QObject *object, QEvent *event)
{
    if(event->type () == QEvent::Leave)
    {
        if(m_isEnabled)
        {
            if(m_alreadyHover)
            {
                m_alreadyHover = false;
                changeImage(IMAGE_TYPE_NORMAL);
                emit sigImageMouseHover(m_indexInPage,false);
            }
        }
    }
    else if(event->type () == QEvent::Enter)
    {
        if(m_isEnabled)
        {
            if((!m_alreadyHover))
            {
                m_alreadyHover = true;
                changeImage(IMAGE_TYPE_MOUSE_HOVER);
                emit sigImageMouseHover(m_indexInPage,true);
            }
        }
    }

    return QWidget::eventFilter (object, event);
}

void Image::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void Image::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void Image::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeClickAction ();
    }
}

void Image::notifyError ()
{
    if(mouseButtonPressTimer->isActive ())
    {
        mouseButtonPressTimer->stop ();
    }
    m_mouseClicked = false;
}

bool Image::getImageClickSts()
{
    return m_mouseClicked;
}

void Image::slotmouseButtonPressTimerTimeout ()
{
    if(m_mouseClicked)
    {
        takeEnterKeyAction ();
    }
}

void Image::slotclickeffctTimerTimeout()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    changeImage(IMAGE_TYPE_MOUSE_HOVER);
    takeEnterKeyAction ();
}

