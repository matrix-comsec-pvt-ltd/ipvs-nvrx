#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QWidget>
#include <QObject>
#include "ApplController.h"
#include "EnumFile.h"
#include "KeyBoard.h"
#include "NavigationControl.h"


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

class TextLabel: public KeyBoard, public NavigationControl
{
     Q_OBJECT
private:
    TEXTLABEL_ALIGNMENT_e alignType;
    qreal m_startX, m_startY;
    int m_wordSpacing, m_letterSpacing, m_fontSize;
    QString m_text, m_ellipssisText, m_ellipssisText2, m_fontFamily, m_fontColor;

    QByteArray test1;
    QFont m_font;
    QRectF textRect;
    bool isBold;
    quint8 lighterVal;
    bool m_isSetUnderLine;
    qreal m_maxWidth;
    bool m_isEllipssisNeeded;
    bool m_isTooltipNeeded, m_isTruncted;
    Qt::AlignmentFlag alignFlag;
    qreal m_leftMargin;

public:
    TextLabel(qreal startX,
              qreal startY,
              int fontSize,
              QString text,
              QWidget* parent = 0,
              QString fontColor = NORMAL_FONT_COLOR,
              QString fontFamily = NORMAL_FONT_FAMILY,
              TEXTLABEL_ALIGNMENT_e align = ALIGN_START_X_START_Y,
              int isLight = 0,
              bool isSetunderLine = false,
              qreal maxWidth = 0,
              int indexInPage = 0,
              bool isEllipssisNeeded = false,
              Qt::AlignmentFlag alignF = Qt::AlignVCenter,
              qreal leftMargin = 0);

    void changeText(QString text, qreal maxWidth = 0, bool forceWidthSetF = false);
    QString getText();
    void changeColor(QString fontColor, int lightVal = 0);
    void SetBold(bool state);

    static QFont getFont(QString fontFamily, quint8 fontSize, bool tIsBold = false, bool isUnderline = false);
    static void getWidthHeight(QFont font, QString text, qreal &width, qreal &height);
    static void getWidthHeight(QFont font, QString text, quint16 &width, quint16 &height);

    void setOffset(qreal xOffset, qreal yOffset);
    void setOffset(qreal xOffset, qreal yOffset, TEXTLABEL_ALIGNMENT_e alignment);

    void setGeometryForText(bool fontResetNeeded = true);
    void setFontSize (quint8 fontSize);
    void setUnderline(bool state);
	void resetGeometry(qreal startX);
	void resetGeometry(qreal iStartX, qreal iStartY);
    void generateEllipssisText();
    void setMaxWidth(qreal width);
    bool getIsTooltipNeeded();

    void enterKeyPressed(QKeyEvent *event);

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    void paintEvent(QPaintEvent* event);
    void forceActiveFocus();
    void mouseMoveEvent (QMouseEvent *);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseDoubleClickEvent(QMouseEvent *);
    bool eventFilter(QObject *object, QEvent *event);
    void slotShowHideToolTip(bool isHoverIn);
    void setAlignment(Qt::AlignmentFlag align);

signals:
    void sigTextClick(int);
    void sigTextDoubleClicked(int);
    void sigMouseHover(int,bool);
};
#endif // TEXTLABEL_H
