#ifndef BGTILE_H
#define BGTILE_H

#include <QWidget>
#include <QObject>
#include "KeyBoard.h"

#define BGTILE_SMALL_SIZE_WIDTH            SCALE_WIDTH(410)
#define BGTILE_MEDIUM_SIZE_WIDTH           SCALE_WIDTH(486)
#define BGTILE_ULTRAMEDIUM_SIZE_WIDTH      SCALE_WIDTH(680)
#define BGTILE_LARGE_SIZE_WIDTH            SCALE_WIDTH(978)
#define BGTILE_EXTRALARGE_SIZE_WIDTH       SCALE_WIDTH(1245)
#define BGTILE_ULTRALARGE_SIZE_WIDTH       SCALE_WIDTH(1110)

#define BGTILE_HEIGHT                      SCALE_HEIGHT(40)
#define LEFT_MARGIN                        SCALE_WIDTH(10)
#define TOP_MARGIN                         SCALE_HEIGHT(10)

typedef enum
{
    COMMON_LAYER,
    TOP_LAYER,
    MIDDLE_LAYER,
    BOTTOM_LAYER,
    TOP_TABLE_LAYER,
    MIDDLE_TABLE_LAYER,
    BOTTOM_TABLE_LAYER,
    UP_LAYER,
    DOWN_LAYER,
    NO_LAYER,
    TIMEBAR_LAYER,
    COMMON_BOTTOM_LAYER,
    BLACK_COLOR_LAYER,
    MAX_BGTILE_LAYER
}BGTILE_TYPE_e;

class BgTile: public KeyBoard
{
    Q_OBJECT
protected:
    int m_startX, m_startY, m_width, m_height;
    BGTILE_TYPE_e m_bgTileType;
private:
    QRect m_mainRect;
    QRect m_rightBorderRect;
    QRect m_leftBorderRect;
    QRect m_topBorderRect;
    QRect m_bottomBorderRect;
    QRect m_innerLeftBorderRect;
    QRect m_innerRightBorderRect;
    QRect m_innerTopBorderRect;
    QRect m_innerBottomBorderRect;
    int   m_bgTileIndex;
public:
    BgTile(int xParam,
           int yParam,
           int width,
           int height,
           BGTILE_TYPE_e bgTileType = COMMON_LAYER,
           QWidget* parent = 0,
           int indexInPage = 0);

    void drawRect(QPainter* painter, QColor color, QRect rect);
    void resetGeometry(int width, int height);
    void resetGeometry(int startX, int StartY, int width, int height);
    void setGeometryForRectangles();
    void changeTileType(BGTILE_TYPE_e tileType, int height);
    void forceActiveFocus();
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);    

    void paintEvent(QPaintEvent *event);
    int getWidth();

signals:
    void sigBgTileClick(int);

};

#endif // BGTILE_H
