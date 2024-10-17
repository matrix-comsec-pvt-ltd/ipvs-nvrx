#ifndef HLTHSTSBGTILE_H
#define HLTHSTSBGTILE_H
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
#include <QWidget>
#include "../EnumFile.h"
#include "Controls/TextLabel.h"

#define HLT_BGTILE_LARGE_WIDTH          SCALE_WIDTH(250)
#define HLT_BGTILE_SMALL_WIDTH          SCALE_WIDTH(50)
#define HLT_BGTILE_HEIGHT               SCALE_HEIGHT(30)

class HlthStsBgTile : public QWidget
{
    Q_OBJECT
public:
    HlthStsBgTile (int xParam,
                   int yParam,
                   int width,
                   int height,
                   int tileNo,
                   QWidget* parent = 0,
                   QString labelStr = "",
                   bool isRectIn = false,
                   QString inRectColor = NORMAL_BKG_COLOR);

    ~HlthStsBgTile ();

    void paintEvent (QPaintEvent *event);
    void drawRect(QBrush brush, QPainter* painter, QColor color, QRect rect);
    bool eventFilter (QObject *obj, QEvent *event);

    void changeLabelText(QString tLabel);
    void changeInnerRectColor(QString color);
    void resetGeoMetry(int srtX, int srtY, int width, int height);

private:
    int tileIndentity;
    QRect mainRect;
    bool innerRectIn;
    QString innerRectColor;
    QString label;
    QRect rightBorderRect;
    QRect leftBorderRect;
    QRect topBorderRect;
    QRect bottomBorderRect;

    TextLabel * textLabel;

signals:
    void sigMouseHover(int tileNo, bool inHover);
};



#endif // HLTHSTSBGTILE_H
