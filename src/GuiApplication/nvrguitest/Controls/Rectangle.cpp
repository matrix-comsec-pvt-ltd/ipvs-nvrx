#include "Rectangle.h"

#include <QPaintEvent>
#include <QPainter>

Rectangle::Rectangle(int startX,
                     int startY,
                     int width,
                     int height,
                     QString color,
                     QWidget* parent,
                     int radius,
                     int borderWidth,
                     QString borderColor,
                     qreal opacityValue) : QWidget(parent)
{
    rectXparam = startX;
    rectYparam = startY;
    rectWidth = width;
    rectHeight = height;
    rectRadius = radius;
    m_opacityValue = opacityValue;
    this->borderColor = borderColor;
    bkgColor = color;
    this->borderWidth = borderWidth;
    m_bottomRectangle.setRect(0, 0, rectWidth, rectHeight);
    m_topRectangle.setRect(borderWidth, borderWidth, (rectWidth - (2 * borderWidth)), (rectHeight - (2 * borderWidth)));
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(m_opacityValue);
    this->setGraphicsEffect(m_opacityEffect);
    this->setMouseTracking (true);
    this->setGeometry(rectXparam, rectYparam, rectWidth, rectHeight);
    this->show();
}

Rectangle::Rectangle( int xParam,
                      int yParam,
                      int width,
                      int height,
                      int radius,
                      QString bdColor,
                      QString bkgdColor,
                      QWidget* parent,
                      quint8 borderWidth1,
                      qreal opacityValue)
    :QWidget(parent)
{
    rectRadius = radius;
    borderColor = bdColor;
    bkgColor = bkgdColor;
    borderWidth = borderWidth1;
    m_opacityValue = opacityValue;
    resetGeometry(xParam, yParam, width, height);
    this->setMouseTracking (true);
    this->setEnabled(true);
    this->show();
}

void Rectangle::changeBorderColor(QString color)
{
    borderColor = color;
}

void Rectangle::setColor(QString color)
{
    borderColor = bkgColor = color;
}

void Rectangle::changeColor(QString color)
{
    bkgColor = color;
}

void Rectangle::resetGeometry (quint32 width)
{
    resetGeometry(rectXparam, rectYparam, width, rectHeight);
}

void Rectangle::resetGeometry(quint32 startX, quint32 startY, quint32 width, quint32 height)
{
    rectXparam = startX;
    rectYparam = startY;
    rectWidth = width;
    rectHeight = height;    
    m_bottomRectangle.setRect(0, 0, rectWidth, rectHeight);
    m_topRectangle.setRect(borderWidth,
                           borderWidth,
                           (rectWidth - (2 * borderWidth)),
                           (rectHeight - (2 * borderWidth)));
    this->setGeometry(rectXparam,
                      rectYparam,
                      rectWidth,
                      rectHeight);
}

void Rectangle::resetGeometry(quint32 startX, quint32 width)
{
    resetGeometry(startX, rectYparam, width, rectHeight);
}

void Rectangle::setRadius(int radius)
{
    rectRadius = radius;
}

void Rectangle::setBorderWidth(quint8 borderWidth1)
{
    borderWidth = borderWidth1;
    m_bottomRectangle.setRect(0, 0, rectWidth, rectHeight);
    m_topRectangle.setRect(borderWidth, borderWidth, (rectWidth - (2 * borderWidth)), (rectHeight - (2 * borderWidth)));
}

void Rectangle::setOpacity(qreal opacityValue)
{
    m_opacityValue = opacityValue;
    if(IS_VALID_OBJ(m_opacityEffect))
    {
         m_opacityEffect->setOpacity(m_opacityValue);
    }
}

void Rectangle::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBackgroundMode(Qt::TransparentMode);

    QColor borderClr(borderColor);
    borderClr.setAlphaF(m_opacityValue);
    QColor backgroundColor(bkgColor);
    backgroundColor.setAlphaF(m_opacityValue);

    //draw background rect which gives border effect
    if(borderWidth == 0)
    {
        painter.setBrush(QBrush(backgroundColor, Qt::SolidPattern));
    }
    else
    {
        painter.setBrush(QBrush(borderClr, Qt::SolidPattern));
    }
    if(rectRadius == 0)
    {
        painter.drawRect(m_bottomRectangle);
    }
    else
    {
        painter.drawRoundedRect(m_bottomRectangle, rectRadius, rectRadius);
    }

    //draw foregroung rect which gives rectangle effect
    painter.setBrush(QBrush(backgroundColor, Qt::SolidPattern));
    if(rectRadius == 0)
    {
        painter.drawRect(m_topRectangle);
    }
    else
    {
        painter.drawRoundedRect(m_topRectangle, (rectRadius - borderWidth), (rectRadius - borderWidth));
    }
}
