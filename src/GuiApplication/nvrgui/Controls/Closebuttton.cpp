#include "Closebuttton.h"

#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>

CloseButtton::CloseButtton(int centerX,
                           int centerY,
                           QWidget* parent,
                           type_e buttonType,
                           int indexInPage,
                           bool isEnabled,
                           bool catchKey):
   KeyBoard(parent), NavigationControl(indexInPage, isEnabled, catchKey)
{
    m_buttonType = buttonType;
    m_centerX = centerX;
    m_centerY = centerY;
    m_onlyHoverImg = false;
    changeImage(IMAGE_TYPE_NORMAL);

    this->setGeometry((m_centerX - m_image.width() / 2),
                      (m_centerY - m_image.height() / 2) ,
                      m_image.width(),
                      m_image.height());

    this->setMouseTracking(true);
    this->show();
}

CloseButtton::CloseButtton(int endX,
                           int startY,
                           int topMargin,
                           int rightMargin,
                           QWidget* parent,
                           type_e btnType,
                           int indexInPage,
                           bool isEnabled,
                           bool catchKey,
                           bool onlyHoverImage):
    KeyBoard(parent), NavigationControl(indexInPage, isEnabled, catchKey)
{
    m_onlyHoverImg = onlyHoverImage;
    m_buttonType = btnType;
    m_centerX = 0;
    m_centerY = 0;
    changeImage(IMAGE_TYPE_NORMAL);

    this->setGeometry((endX - rightMargin - m_image.width()),
                      (startY + topMargin) ,
                      m_image.width(),
                      m_image.height());

    this->setMouseTracking(true);
    this->show();
}

void CloseButtton::changeImage(IMAGE_TYPE_e type)
{
    m_imageType = type;
    m_imageSource = CLOSE_BTN_PATH + closeBtnSubFolder[m_buttonType] + imgTypePath[m_imageType];
    m_image = QPixmap(m_imageSource);
    SCALE_IMAGE(m_image);
    m_imageRect.setRect(0, 0, m_image.width(), m_image.height());
    update();
}

void CloseButtton::changeImageState(type_e buttonType)
{
    m_buttonType = buttonType;
    changeImage(m_imageType);
}

void CloseButtton::resetGeometry(int centerX, int centerY)
{
    m_centerX = centerX;
    m_centerY = centerY;

    this->setGeometry((m_centerX - m_image.width() / 2),
                      (m_centerY - m_image.height() / 2) ,
                      m_image.width(),
                      m_image.height());
}

void CloseButtton::selectControl()
{
    if(m_imageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void CloseButtton::deSelectControl()
{
    changeImage(IMAGE_TYPE_NORMAL);
}

void CloseButtton::forceActiveFocus()
{
    this->setFocus();
}

void CloseButtton::takeEnterKeyAction()
{
    emit sigButtonClick(m_indexInPage);
}

void CloseButtton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(m_imageRect, m_image);
}

void CloseButtton::mousePressEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void CloseButtton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton)
            && (m_mouseClicked))
    {
        m_mouseClicked = false;
        takeEnterKeyAction();
    }
    else
    {
        m_mouseClicked = false;
    }
}

void CloseButtton::mouseMoveEvent(QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_isControlActivated))
    {
        if(m_onlyHoverImg)
        {
            selectControl ();
        }
        else
        {
            if(this->hasFocus())
            {
                selectControl ();
            }
            else
            {
                forceActiveFocus();
                emit sigUpdateCurrentElement(m_indexInPage);
            }
        }
    }
}

void CloseButtton::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void CloseButtton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void CloseButtton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void CloseButtton::escKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}
