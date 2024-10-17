#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QWidget>
#include "EnumFile.h"
#include <QGraphicsOpacityEffect>
#include "KeyBoard.h"

class Rectangle: public KeyBoard
{
public:
    Rectangle( qreal xParam,
               qreal yParam,
               qreal width,
               qreal height,
               int radius,
               QString bdColor,
               QString bkgdColor,
               QWidget* parent = 0,
               quint8 tBorderWidth = 0,
               qreal opacityValue = 1.0);

    Rectangle(qreal startX,
               qreal startY,
               qreal width,
               qreal height,
              QString color = NORMAL_BKG_COLOR,
              QWidget* parent = 0,
              int radius = 0,
              int tBorderWidth = 0,
              QString tBorderColor = BORDER_1_COLOR,
              qreal opacityValue = 1.0);

    ~Rectangle();
    void paintEvent(QPaintEvent *event);
    void changeColor(QString color);
    void changeBorderColor(QString color);
    void setColor(QString color);
    void resetGeometry(qreal startX, qreal startY, qreal width, qreal height);
    void resetGeometry (qreal width);
    void resetGeometry(qreal startX, qreal width);
    void setRadius(int radius);
    void setBorderWidth(quint8 borderWidth1);
    void setOpacity(qreal opacityValue);

private:
    qreal rectXparam;
    qreal rectYparam;
    qreal rectWidth;
    qreal rectHeight;
    int rectRadius;
    int borderWidth;
    qreal m_opacityValue;
    QString borderColor;
    QString bkgColor;

    QRectF m_bottomRectangle;
    QRectF m_topRectangle;
    QGraphicsOpacityEffect* m_opacityEffect;
};

#endif // RECTANGLE_H
