#ifndef READONLYELEMENT_H
#define READONLYELEMENT_H

#include "Bgtile.h"
#include "TextLabel.h"

#define READONLY_EXTRA_SMALL_WIDTH      50
#define READONLY_SMALL_WIDTH            90
#define READONLY_MEDIAM_WIDTH           135
#define READONLY_LARGE_WIDTH            200
#define READONLY_EXTRA_LARGE_WIDTH      640

#define READONLY_HEIGHT                 SCALE_HEIGHT(30)

//#define MX_OPTION_TEXT_TYPE_LABEL                 0
//#define MX_OPTION_TEXT_TYPE_SUFFIX                1
#define DEFAULT_PIXEL_SIZE              -1

#define UNIT_WIDTH                      1

class ReadOnlyElement: public BgTile
{
private:
    QString m_label, m_value, m_suffix;
    QRect m_mainInnerRect;
    QRect m_topRect;
    QRect m_bottomRect;
    QRect m_leftRect;
    QRect m_rightRect;
    TextLabel * m_textLabel;
    TextLabel * m_textValue;
    TextLabel * m_textSuffix;
    int m_pixelAlign;
    int m_valueMargin, m_textLabelSize, m_textSuffixSize, m_textValueSize;
    int m_rectWidth, m_rectHeight;
    QString m_suffixFontColor;
    bool m_isOuterBorderNeeded;
public:
    ReadOnlyElement(int startX,
                    int startY,
                    int tileWidth,
                    int tileHeight,
                    int width,
                    int height,
                    QString value,
                    QWidget* parent = 0,
                    BGTILE_TYPE_e tileType = COMMON_LAYER,
                    int pixelAlign = -1,
                    int valueMargin = -1,
                    QString label = "",
                    QString suffix = "",
                    QString suffixFontColor = "",
                    int suffixTextSize = (10),
                    bool isOuterBorderNeeded = true);
    ~ReadOnlyElement();

    void setGeometryForElements();
    void setRectanglesGeometry(int xAlignment, int yAlingnment);
    void drawRectangles(QPainter * painter);
    void changeValue(QString text);
    QString getCurrentValue() const;

    void paintEvent(QPaintEvent* event);
};

#endif // READONLYELEMENT_H
