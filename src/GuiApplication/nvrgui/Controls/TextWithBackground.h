#ifndef TEXTWITHBACKGROUND_H
#define TEXTWITHBACKGROUND_H

#include "Controls/TextLabel.h"
#include "NavigationControl.h"
#include "KeyBoard.h"

class TextWithBackground : public KeyBoard , public NavigationControl
{
    Q_OBJECT

public:
    explicit TextWithBackground(int startX,
                                int startY,
                                int fontSize,
                                QString text,
                                QWidget* parent = 0,
                                QString fontColor = NORMAL_FONT_COLOR,
                                QString fontFamily = NORMAL_FONT_FAMILY,
                                TEXTLABEL_ALIGNMENT_e align = ALIGN_START_X_START_Y,
                                int isLight = 0,
                                bool isSetunderLine = false,
                                QString backGroundColor = TRANSPARENTKEY_COLOR,
                                bool isMouseEffectNeeded = false,
                                int indexInPage = 255,
                                quint16 maxWidth = 0,
                                bool isEllipssisNeeded = false);

    ~TextWithBackground();
    void setAlignment();
    void setBold(bool isBold);
    void setBackGroundColor(QString backColor);
    void setOffset(quint16 xOffset, quint16 yOffset);
    void setOffset(quint16 xOffset, quint16 yOffset, TEXTLABEL_ALIGNMENT_e alignment);
    void setFontSize(quint8 fontSize);
    void setHeight(quint32 height);
    void changeText(QString text);
    QString getText() const;
    void changeTextColor(QString fontColor);
    void setMaxWidth(quint16 width);
    bool getIsTooltipNeeded();

    void selectControl();
    void deSelectControl();
    void forceActiveFocus();

    void paintEvent (QPaintEvent*);
    void mouseMoveEvent (QMouseEvent *);
    void mousePressEvent (QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    bool eventFilter(QObject *object, QEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);


    
signals:
    void sigUpdateCurrentElement(int index);
    void sigMousePressClick (QString);
    void sigMouseHover(int,bool);
    
public slots:
    

private:

    QRect m_rect;

    quint16 m_height;
    quint16 m_width;
    quint16 m_startX;
    quint16 m_startY;
    quint16 m_maxWidth;

    quint16 m_startTextX;
    quint16 m_startTextY;
    quint16 m_fontSize;

    TEXTLABEL_ALIGNMENT_e alignType;

    QString m_backColor;
    QString m_fontFamily;

    bool m_isSetunderLine;
    bool m_isMouseEffectNeeded;
    bool m_isBoldNedded;
    bool m_isEllipssisNeeded;

    TextLabel*  m_textLabel;
};

#endif // TEXTWITHBACKGROUND_H
