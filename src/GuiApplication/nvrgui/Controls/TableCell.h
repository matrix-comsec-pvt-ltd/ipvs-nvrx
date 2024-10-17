#ifndef TABLECELL_H
#define TABLECELL_H

#include <QWidget>
#include "EnumFile.h"

#define TABELCELL_SMALL_SIZE_WIDTH            SCALE_WIDTH(135)
#define TABELCELL_MEDIUM_SIZE_WIDTH           SCALE_WIDTH(200)
#define TABELCELL_LARGE_SIZE_WIDTH            SCALE_WIDTH(378)
#define TABELCELL_EXTRALARGE_SIZE_WIDTH       SCALE_WIDTH(445)
#define TABELCELL_EXTRAMEDIUM_SIZE_WIDTH      SCALE_WIDTH(303)
#define TABELCELL_EXTRASMALL_SIZE_WIDTH       SCALE_WIDTH(100)
#define TABELCELL_ULTRASMALL_SIZE_WIDTH       SCALE_WIDTH(32)

#define TABELCELL_HEIGHT                      SCALE_HEIGHT(30)

class TableCell : public QWidget
{
public:
    TableCell(int xParam,
              int yParam,
              int width,
              int height,
              QWidget* parent = 0,
              bool isHeaderCell = false);

    void paintEvent(QPaintEvent *event);
    void drawRect(QPainter* painter, QColor color, QRect rect);

private:
    QRect mainRect;
    QRect rightBorderRect;
    QRect leftBorderRect;
    QRect topBorderRect;
    QRect bottomBorderRect;

    bool isHeaderCell;
};

#endif // TABLECELL_H
