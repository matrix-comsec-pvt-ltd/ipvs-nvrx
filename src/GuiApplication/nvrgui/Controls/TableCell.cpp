#include "TableCell.h"
#include <QPainter>
#include <QPaintEvent>

TableCell::TableCell(int xParam,
                     int yParam,
                     int width,
                     int height,
                     QWidget* parent,
                     bool isHdrCell) :
    QWidget(parent)
{
    isHeaderCell = isHdrCell;
    this->setGeometry (QRect(xParam, yParam, (width + 1), height));
    mainRect.setRect (0, 0, width, height);

    topBorderRect.setRect (mainRect.x (),
                           mainRect.y (),
                           mainRect.width (), 1);

    leftBorderRect.setRect (mainRect.x (),
                            mainRect.y (),
                            1, mainRect.height ());

    rightBorderRect.setRect (mainRect.x ()  + mainRect.width () ,
                             mainRect.y (),
                             1, (mainRect.height ()));

    bottomBorderRect.setRect (mainRect.x (),
                              mainRect.y () + mainRect.height () - 1,
                              mainRect.width (), 1);

    this->show ();
}

void TableCell:: drawRect(QPainter *painter, QColor color, QRect rect)
{
    QBrush brush = QBrush( QColor(color));
    painter->setPen (Qt::NoPen);
    painter->setBrush ( brush );
    painter->drawRect (rect);
}

void TableCell::paintEvent (QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    if( isHeaderCell == false )
    {
        drawRect(&painter, NORMAL_BKG_COLOR, mainRect);
    }
    else
    {
        drawRect(&painter, CLICKED_BKG_COLOR, mainRect);
    }

    drawRect(&painter, BORDER_2_COLOR, topBorderRect);
    drawRect(&painter, BORDER_2_COLOR, rightBorderRect);
    drawRect(&painter, BORDER_1_COLOR, leftBorderRect);
    drawRect(&painter, BORDER_1_COLOR, bottomBorderRect);
}
