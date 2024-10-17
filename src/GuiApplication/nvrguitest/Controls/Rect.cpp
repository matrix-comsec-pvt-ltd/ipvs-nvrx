#include "Rect.h"
#include <QPainter>

Rect::Rect(quint16 xPos, quint16 yPos, quint16 width, quint16 height,
           QString bgColor, QWidget *parent) :
    QWidget(parent)
{
    m_xPos = xPos;
    m_yPos = yPos;
    m_width = width;
    m_height = height;
    m_bgColor = bgColor;

    this->setGeometry (m_xPos, m_yPos, m_width, m_height);
    this->show ();
}


void Rect::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QBrush(m_bgColor,Qt::SolidPattern));
    painter.drawRect(QRect(0, 0, this->width(), this->height()));
}

void Rect::changeBgColor(QString bgColor)
{
    m_bgColor = bgColor;
    update();
}

QString Rect::getBgColor()
{
    return m_bgColor;
}
