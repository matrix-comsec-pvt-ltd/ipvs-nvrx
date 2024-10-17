#include "Rectangle.h"

#include <QPaintEvent>
#include <QPainter>

Rectangle::Rectangle(qreal startX,
                     qreal startY,
                     qreal width,
                     qreal height,
                     QString color,
                     QWidget* parent,
                     int radius,
                     int tBorderWidth,
                     QString tBorderColor,
                     qreal opacityValue) : KeyBoard(parent), rectXparam(startX),
    rectYparam(startY), rectWidth(width), rectHeight(height),
    rectRadius(radius), borderWidth(tBorderWidth), m_opacityValue(opacityValue),
    borderColor(tBorderColor), bkgColor(color)
{
    m_bottomRectangle.setRect(0, 0, rectWidth, rectHeight);
    m_topRectangle.setRect(tBorderWidth, tBorderWidth, (rectWidth - (2 * tBorderWidth)), (rectHeight - (2 * tBorderWidth)));
    m_opacityEffect = NULL;
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(m_opacityValue);
    this->setGraphicsEffect(m_opacityEffect);
    this->setMouseTracking (true);
    this->setGeometry(rectXparam, rectYparam, rectWidth, rectHeight);
    this->show();
}

Rectangle::Rectangle(qreal xParam,
                       qreal yParam,
                       qreal width,
                       qreal height,
                      int radius,
                      QString bdColor,
                      QString bkgdColor,
                      QWidget* parent,
                      quint8 tBorderWidth,
                      qreal opacityValue)
    :KeyBoard(parent), rectRadius(radius), borderWidth(tBorderWidth), m_opacityValue(opacityValue),
      borderColor(bdColor),bkgColor(bkgdColor)
{
    m_opacityEffect = NULL;
    resetGeometry(xParam, yParam, width, height);
    this->setMouseTracking (true);
    this->setEnabled(true);
    this->show();
}

Rectangle::~Rectangle()
{
    if(NULL != m_opacityEffect)
    {
        delete m_opacityEffect;
        m_opacityEffect = NULL;
    }
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

void Rectangle::resetGeometry (qreal width)
{
    resetGeometry(rectXparam, rectYparam, width, rectHeight);
}

void Rectangle::resetGeometry(qreal startX, qreal startY, qreal width, qreal height)
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

void Rectangle::resetGeometry(qreal startX, qreal width)
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
