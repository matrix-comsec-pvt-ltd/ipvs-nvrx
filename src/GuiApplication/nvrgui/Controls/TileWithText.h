#ifndef TILEWITHTEXT_H
#define TILEWITHTEXT_H
#include "Bgtile.h"
#include "TextLabel.h"

class TileWithText : public BgTile
{
    Q_OBJECT
private:
    TextLabel* m_textLable;
    int m_textLabelOffset;
    bool m_isTileAreaEnable;
    int m_tileIndentity;
    QString m_textColor;
    QString m_textString;
    int m_indexInPage;
public:
    TileWithText(int xParam,
                 int yParam,
                 int width,
                 int height,
                 int textLableOffset,
                 QString textString,
                 BGTILE_TYPE_e bgTileType = COMMON_LAYER,
                 QWidget* parent = 0,
                 bool isTileAreaEnable = false,
                 int tileIdentity = 0,
                 QString textFontColor = NORMAL_FONT_COLOR,
                 int indexInPage = 0);

    ~TileWithText();
    void changeColor(QString color);
    void changeText(QString text);
    QString getText();

    bool eventFilter(QObject *, QEvent *);

signals:
    void sigMouseHoverInOutForTile(int index, bool isHoverIn);
    void sigTileWithTextClick(int);

public slots:
    void slotMouseClick(int indexInPage);
};

#endif // TILEWITHTEXT_H
