#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QWidget>
#include "Enumfile.h"

#define BOLD            1
#define NORMAL          0

typedef enum
{
    ALIGN_START_X_START_Y,
    ALIGN_START_X_CENTRE_Y,
    ALIGN_START_X_END_Y,
    ALIGN_CENTRE_X_START_Y,
    ALIGN_CENTRE_X_CENTER_Y,
    ALIGN_CENTRE_X_END_Y,
    ALIGN_END_X_START_Y,
    ALIGN_END_X_CENTRE_Y,
    ALIGN_END_X_END_Y,
    MAX_ALIGN_TEXT
}TEXTLABEL_ALIGNMENT_e;

class TextLabel: public QWidget
{
     Q_OBJECT
private:
    TEXTLABEL_ALIGNMENT_e alignType;
    int m_startX, m_startY, m_wordSpacing, m_letterSpacing, m_fontSize;
    QString m_text, m_fontFamily, m_fontColor;
    QFont m_font;
    QRect textRect;
    bool isBold;
    quint8 lighterVal;
    bool m_isSetUnderLine;
    quint16 m_maxWidth;
    int m_indexInPage;

public:
    TextLabel(int startX,
              int startY,
              int fontSize,
              QString text,
              QWidget* parent = 0,
              QString fontColor = NORMAL_FONT_COLOR,
              QString fontFamily = NORMAL_FONT_FAMILY,
              TEXTLABEL_ALIGNMENT_e align = ALIGN_START_X_START_Y,
              int isLight = 0,
              bool isSetunderLine = false,
              quint16 maxWidth = 0,
              int indexInPage = 0);

    void changeText(QString text);
    QString getText();
    void changeColor(QString fontColor, int lightVal = 0);
    void SetBold(bool state);

    static QFont getFont(QString fontFamily, quint8 fontSize, bool isBold = false, bool isUnderline = false);
    static void getWidthHeight(QFont font, QString text, quint16 &width, quint16 &height);

    void setOffset(quint16 xOffset, quint16 yOffset);
    void setOffset(quint16 xOffset, quint16 yOffset, TEXTLABEL_ALIGNMENT_e alignment);

    void setGeometryForText(bool fontResetNeeded = true);
    void setFontSize (quint8 fontSize);
    void setUnderline(bool state);
    void resetGeometry(int startX);

    void paintEvent(QPaintEvent* event);
    void forceActiveFocus();
    void mouseMoveEvent (QMouseEvent *);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseDoubleClickEvent(QMouseEvent *);
    bool eventFilter(QObject *object, QEvent *event);

signals:
    void sigTextClick(int);
    void sigTextDoubleClicked(int);
    void sigMouseHover(int,bool);

};

#endif // TEXTLABEL_H
