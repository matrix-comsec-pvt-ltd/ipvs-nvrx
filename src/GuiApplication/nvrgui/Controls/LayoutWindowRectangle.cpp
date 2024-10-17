#include "LayoutWindowRectangle.h"

#include <QPaintEvent>
#include <QPainter>

LayoutWindowRectangle::LayoutWindowRectangle(qreal startX,
                     qreal startY,
                     qreal width,
                     qreal height,
                     QString bkgdColor,
                     QWidget* parent,
                     qreal opacityValue) : KeyBoard(parent),
    rectXparam(startX), rectYparam(startY), rectWidth(width), rectHeight(height),
    m_opacityValue(opacityValue),bkgColor(bkgdColor)
{
    INIT_OBJ(m_bottomRectangle);
    INIT_OBJ(m_opacityEffect);
    m_bottomRectangle = new QRectF(0, 0, rectWidth, rectHeight);
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(m_opacityValue);
    this->setGraphicsEffect(m_opacityEffect);
    this->setMouseTracking (true);
    this->setGeometry(rectXparam, rectYparam, rectWidth, rectHeight);
    this->show();
}

LayoutWindowRectangle::~LayoutWindowRectangle()
{
    if(IS_VALID_OBJ(m_opacityEffect))
    {
        DELETE_OBJ(m_opacityEffect);
    }
    if(IS_VALID_OBJ(m_bottomRectangle))
    {
        DELETE_OBJ(m_bottomRectangle);
    }
}

void LayoutWindowRectangle::changeColor(QString color)
{
    bkgColor = color;
}

void LayoutWindowRectangle::resetGeometry(qreal startX, qreal startY, qreal width, qreal height)
{
    rectXparam = startX;
    rectYparam = startY;
    rectWidth = width;
    rectHeight = height;
    m_bottomRectangle->setRect(0, 0, rectWidth, rectHeight);
    this->setGeometry(rectXparam,
                      rectYparam,
                      rectWidth,
                      rectHeight);
}

void LayoutWindowRectangle::setOpacity(qreal opacityValue)
{
    m_opacityValue = opacityValue;
    if(IS_VALID_OBJ(m_opacityEffect))
    {
         m_opacityEffect->setOpacity(m_opacityValue);
    }
}

void LayoutWindowRectangle::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBackgroundMode(Qt::TransparentMode);

    QColor backgroundColor(bkgColor);
    backgroundColor.setAlphaF(m_opacityValue);

    painter.setBrush(Qt::NoBrush);
    QPen pen(backgroundColor);
    if(bkgColor == HIGHLITED_FONT_COLOR)
    {
        pen.setWidth(3);
    }
    painter.setPen(pen);
    painter.drawRect(*m_bottomRectangle);
    }
