#ifndef LAYOUT_WINDOW_RECTANGLE_H
#define LAYOUT_WINDOW_RECTANGLE_H

#include <QWidget>
#include "EnumFile.h"
#include <QGraphicsOpacityEffect>
#include "KeyBoard.h"

class LayoutWindowRectangle: public KeyBoard
{
public:
	LayoutWindowRectangle( qreal xParam,
               qreal yParam,
               qreal width,
               qreal height,
			   QString bkgdColor,
               QWidget* parent = 0,
               qreal opacityValue = 1.0);

	~LayoutWindowRectangle();
    void paintEvent(QPaintEvent *event);
    void changeColor(QString color);
    void resetGeometry(qreal startX, qreal startY, qreal width, qreal height);
    void setOpacity(qreal opacityValue);

private:
    qreal rectXparam;
    qreal rectYparam;
    qreal rectWidth;
    qreal rectHeight;
    qreal m_opacityValue;
    QString bkgColor;

	QRectF* m_bottomRectangle;
    QGraphicsOpacityEffect* m_opacityEffect;
};

#endif // RECTANGLE_H
