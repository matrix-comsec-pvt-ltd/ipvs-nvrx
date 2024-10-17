/////////////////////////////////////////////////////////////////////////////
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : DVR (Digital Video Recorder - TI)
//   Owner        : Tushar Rabadiya
//   File         : PageOpenButton.cpp
//   Description  :
/////////////////////////////////////////////////////////////////////////////
#include "HlthStsBgTile.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>

#define UNIT_WIDTH              1
#define UNIT_HEIGHT             1


HlthStsBgTile::HlthStsBgTile (int xParam,
                              int yParam,
                              int width,
                              int height,
                              int tileNo,
                              QWidget *parent,
                              QString labelStr,
                              bool isRectIn,
                              QString inRectColor)
    :QWidget(parent), innerRectColor(inRectColor), label(labelStr)
{
    tileIndentity = tileNo;
    innerRectIn = isRectIn;

    mainRect.setRect (0, 0, (width) , (height - 1));

    this->setGeometry (xParam, yParam, width +1 , height);
    this->setMouseTracking (true);
    this->installEventFilter (this);

    rightBorderRect.setRect (mainRect.x () + mainRect.width () - SCALE_WIDTH(2),
                             mainRect.y (),
                             SCALE_WIDTH(2), mainRect.height ());
    leftBorderRect.setRect (0,
                            0,
                            1, mainRect.height ());
    bottomBorderRect.setRect (mainRect.x (),
                              mainRect.y () + mainRect.height () - 1,
                              mainRect.width () -1, 1);
    topBorderRect.setRect (0,
                           0,
                           mainRect.width (), 1);


    QFont font = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);

    textLabel = new TextLabel(SCALE_WIDTH(10),
                              ((mainRect.height () - QFontMetrics(font).height ())/2),
                              NORMAL_FONT_SIZE, label, this);

    this->show ();
}

HlthStsBgTile::~HlthStsBgTile()
{
    delete textLabel;
}

void HlthStsBgTile::paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    QBrush brush;
    //    QFont font;

    //    textLabel->setGeometry (textLabel->x (),
    //                            ((this->height () - textLabel->height ())/2),
    //                            textLabel->width (),
    //                            textLabel->height ());

    drawRect(brush, &painter, NORMAL_BKG_COLOR, mainRect);

    drawRect(brush, &painter, BORDER_2_COLOR, rightBorderRect);
    drawRect(brush, &painter, BORDER_1_COLOR, bottomBorderRect);
    drawRect(brush, &painter, BORDER_2_COLOR, topBorderRect);
    drawRect(brush, &painter, BORDER_1_COLOR, leftBorderRect);

    if(innerRectIn == true)
    {
        QRect rect(((mainRect.width () - SCALE_WIDTH(12))/2),
                   ((mainRect.height () - SCALE_HEIGHT(12))/2),
                   SCALE_WIDTH(12), SCALE_HEIGHT(12));
        painter.setBrush ( QBrush(QColor(innerRectColor), Qt::SolidPattern) );
        painter.setPen (Qt::NoPen);
        painter.drawRect (rect);
    }
}

void HlthStsBgTile::drawRect(QBrush brush, QPainter* painter, QColor color, QRect rect)
{
    brush = QBrush( QColor(color));
    if(rect == mainRect)
    {
        painter->setPen (QColor(Qt::black));
    }
    else
    {
        painter->setPen (Qt::NoPen);
    }
    painter->setBrush ( brush );
    painter->drawRect (rect);
}

bool HlthStsBgTile::eventFilter (QObject *obj, QEvent *event)
{
    if((event->type () == QEvent::HoverEnter) || (event->type ()== QEvent::Enter))
    {
        emit sigMouseHover (tileIndentity, true);
        event->accept();
        return true;
    }
    else if((event->type() == QEvent::HoverLeave) || (event->type() == QEvent::Leave))
    {
        emit sigMouseHover (tileIndentity, false);
        event->accept();
        return true;
    }
    else
    {
        return QObject::eventFilter(obj,event);
    }
}

void HlthStsBgTile::changeLabelText(QString tLabel)
{
    textLabel->changeText (tLabel);
}

void HlthStsBgTile::changeInnerRectColor (QString color)
{
    innerRectColor = color;
}

void HlthStsBgTile::resetGeoMetry(int srtX, int srtY, int width, int height)
{
    this->setGeometry (QRect(srtX, srtY, width +1, height));
    mainRect.setRect (0, 0, width + 1, height -1);

    rightBorderRect.setRect (mainRect.x () + mainRect.width () - SCALE_WIDTH(2),
                             mainRect.y (),
                             SCALE_WIDTH(2), mainRect.height ());
    leftBorderRect.setRect (0,
                            0,
                            1, mainRect.height ());
    bottomBorderRect.setRect (mainRect.x (),
                              mainRect.y () + mainRect.height () - 1,
                              mainRect.width () -1, 1);
    topBorderRect.setRect (0,
                           0,
                           mainRect.width (), 1);
}
