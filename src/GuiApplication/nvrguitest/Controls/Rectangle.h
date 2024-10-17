#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QWidget>
#include "Enumfile.h"
#include <QGraphicsOpacityEffect>

class Rectangle: public QWidget
{
public:
    Rectangle( int xParam,
               int yParam,
               int width,
               int height,
               int radius,
               QString bdColor,
               QString bkgdColor,
               QWidget* parent = 0,
               quint8 borderWidth = 0,
               qreal opacityValue = 1.0);

    Rectangle(int startX,
              int startY,
              int width,
              int height,
              QString color = NORMAL_BKG_COLOR,
              QWidget* parent = 0,
              int radius = 0,
              int borderWidth = 0,
              QString borderColor = BORDER_1_COLOR,
              qreal opacityValue = 1.0);

    void paintEvent(QPaintEvent *event);
    void changeColor(QString color);
    void changeBorderColor(QString color);
    void setColor(QString color);
    void resetGeometry(quint32 startX, quint32 startY, quint32 width, quint32 height);
    void resetGeometry (quint32 width);
    void resetGeometry(quint32 startX, quint32 width);
    void setRadius(int radius);
    void setBorderWidth(quint8 borderWidth1);
    void setOpacity(qreal opacityValue);

private:
    int rectXparam;
    int rectYparam;
    int rectWidth;
    int rectHeight;
    int rectRadius;
    int borderWidth;
    qreal m_opacityValue;
    QString borderColor;
    QString bkgColor;

    QRect m_bottomRectangle;
    QRect m_topRectangle;
    QGraphicsOpacityEffect* m_opacityEffect;
};

#endif // RECTANGLE_H
