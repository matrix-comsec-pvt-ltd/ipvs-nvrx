#include "Bgtile.h"
#include "Enumfile.h"
#include <QPainter>
#include <QPaintEvent>

#define UNIT_WIDTH                  1
#define UNIT_HEIGHT                 1

BgTile::BgTile(int startX,
               int startY,
               int width,
               int height,
               BGTILE_TYPE_e tileType,
               QWidget* parent, int indexInPage): QWidget(parent)
{
    m_startX = startX;
    m_startY = startY;
    m_width = width;
    m_height = height;
    m_bgTileType = tileType;
    m_bgTileIndex = indexInPage;
    setGeometryForRectangles();
    this->show();
    this->setMouseTracking(true);
}

void BgTile::setGeometryForRectangles()
{
    switch(m_bgTileType)
    {
    case TOP_LAYER:
        m_height += TOP_MARGIN;
        break;

    case BOTTOM_LAYER:
        m_height += TOP_MARGIN;
        break;

    case TOP_TABLE_LAYER:
        m_height += TOP_MARGIN;

        m_innerTopBorderRect.setRect(LEFT_MARGIN,
                                     TOP_MARGIN,
                                     (m_width - 2 * LEFT_MARGIN),
                                     UNIT_HEIGHT);

        m_innerLeftBorderRect.setRect(LEFT_MARGIN,
                                      TOP_MARGIN,
                                      UNIT_HEIGHT,
                                      (m_height - TOP_MARGIN));

        m_innerRightBorderRect.setRect((m_width - LEFT_MARGIN),
                                       TOP_MARGIN,
                                       UNIT_WIDTH,
                                       (m_height - TOP_MARGIN));
        break;

    case MIDDLE_TABLE_LAYER:

        m_innerTopBorderRect.setRect(LEFT_MARGIN,
                                     0,
                                     (m_width - 2 * LEFT_MARGIN),
                                     UNIT_HEIGHT);

        m_innerLeftBorderRect.setRect(LEFT_MARGIN,
                                      0,
                                      UNIT_WIDTH,
                                      m_height);

        m_innerRightBorderRect.setRect((m_width - LEFT_MARGIN),
                                       0,
                                       UNIT_WIDTH,
                                       m_height);
        m_innerBottomBorderRect.setRect(LEFT_MARGIN,
                                        m_height -UNIT_HEIGHT,
                                        (m_width - 2 * LEFT_MARGIN),
                                        UNIT_HEIGHT);
        break;

    case BOTTOM_TABLE_LAYER:
        m_height += TOP_MARGIN;

        m_innerTopBorderRect.setRect(LEFT_MARGIN,
                                     0,
                                     (m_width - 2 * LEFT_MARGIN),
                                     UNIT_HEIGHT);

        m_innerLeftBorderRect.setRect(LEFT_MARGIN,
                                      0,
                                      UNIT_HEIGHT,
                                      (m_height - TOP_MARGIN));

        m_innerRightBorderRect.setRect((m_width - LEFT_MARGIN),
                                       0,
                                       UNIT_WIDTH,
                                       (m_height - TOP_MARGIN));
        m_innerBottomBorderRect.setRect(LEFT_MARGIN,
                                        (m_height - TOP_MARGIN),
                                        (m_width - 2 * LEFT_MARGIN),
                                        UNIT_HEIGHT);
        break;

    default:
        break;
    }

    m_mainRect.setRect(0, 0, m_width, m_height);

    m_leftBorderRect.setRect(0,
                             0,
                             UNIT_WIDTH,
                             m_height);

    m_rightBorderRect.setRect((m_width - UNIT_WIDTH),
                              0,
                              UNIT_WIDTH,
                              m_height);

    m_topBorderRect.setRect(0,
                            0,
                            m_width,
                            UNIT_HEIGHT);

    m_bottomBorderRect.setRect(0,
                               m_height - UNIT_HEIGHT,
                               m_width,
                               UNIT_HEIGHT);

    this->setGeometry(QRect(m_startX, m_startY, m_width, m_height));
}

void BgTile::changeTileType(BGTILE_TYPE_e tileType, int height)
{
    m_bgTileType = tileType;
    m_height = height;
    setGeometryForRectangles();
}

void BgTile::forceActiveFocus()
{
    this->setFocus();
}

void BgTile::mousePressEvent(QMouseEvent *event)
{
    if((m_mainRect.contains(event->pos()))
            && (event->button() == Qt::LeftButton))
    {
        if(!this->hasFocus())
        {
            forceActiveFocus();
    //            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }

    QWidget:: mousePressEvent (event);

}

void BgTile::mouseReleaseEvent(QMouseEvent *event)
{
    if((m_mainRect.contains(event->pos()))
            && (event->button() == Qt::LeftButton))
    {
        emit sigBgTileClick(m_bgTileIndex);
    }
    QWidget::mouseReleaseEvent (event);

}

void BgTile::resetGeometry(int width, int height)
{
    m_width = width;
    m_height = height;
    setGeometryForRectangles();
}

void BgTile::resetGeometry (int startX, int StartY, int width, int height)
{
    m_startX = startX;
    m_startY = StartY;
    m_width = width;
    m_height = height;
    setGeometryForRectangles();
}

void BgTile:: drawRect(QBrush brush, QPainter *painter, QColor color, QRect rect)
{
    brush = QBrush(QColor(color));
    painter->setPen(Qt::NoPen);
    painter->setBrush(brush);
    painter->drawRect(rect);
}

void BgTile::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    QBrush brush;

    if(m_bgTileType != NO_LAYER)
    {
        drawRect(brush, &painter, NORMAL_BKG_COLOR, m_mainRect);
    }

    switch(m_bgTileType)
    {
    case COMMON_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_topBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_bottomBorderRect);
    }
    break;

    case COMMON_BOTTOM_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_topBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_bottomBorderRect);
    }
    break;

    case TOP_LAYER:
    case UP_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_topBorderRect);
    }
    break;

    case MIDDLE_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
    }
    break;

    case BOTTOM_LAYER:
    case DOWN_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_bottomBorderRect);
    }
    break;

    case TOP_TABLE_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_topBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_innerLeftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_innerRightBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_innerTopBorderRect);
    }
    break;

    case MIDDLE_TABLE_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_innerLeftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_innerRightBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_innerTopBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_innerBottomBorderRect);

    }
    break;

    case BOTTOM_TABLE_LAYER:
    {
        drawRect(brush, &painter, BORDER_2_COLOR, m_leftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_rightBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_bottomBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_innerTopBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_innerLeftBorderRect);
        drawRect(brush, &painter, BORDER_1_COLOR, m_innerRightBorderRect);
        drawRect(brush, &painter, BORDER_2_COLOR, m_innerBottomBorderRect);
    }
    break;

    case TIMEBAR_LAYER:
    {
        drawRect(brush, &painter, TIME_BAR_COLOR, m_mainRect);
    }
    break;

    case BLACK_COLOR_LAYER:
    {
        drawRect(brush, &painter, CLICKED_BKG_COLOR, m_mainRect);
    }
    break;

    default:
        break;
    }
}
