#ifndef RECTWITHTEXT_H
#define RECTWITHTEXT_H

#include <QWidget>
#include "NavigationControl.h"
#include "EnumFile.h"
#include "Controls/Rectangle.h"
#include "Controls/TextLabel.h"

class RectWithText : public Rectangle, public NavigationControl
{
    Q_OBJECT

private:
    quint16 m_startX, m_startY, m_width, m_height;
    QString m_text;
    QString m_fontColor;
    int m_fontSize;
    TextLabel *m_textLabel;

public:
    RectWithText(quint16 xParam,
                 quint16 yParam,
                 quint16 rectWidth,
                 quint16 rectHeight,
                 QString text,
                 QWidget *parent = 0,
                 QString fontcolor = NORMAL_FONT_COLOR,
                 int fontSize = NORMAL_FONT_SIZE,
                 int indexInPage = 0,
                 bool isEnabled = true);

    ~RectWithText();

    void changeText(QString text);
    void changeFontColor(QString color);
    QString getText(void);
    QString getFontColor();
    void selectRect();
    void deselectRect();
    void selectControl();
    void deSelectControl();

    void forceActiveFocus();

    void takeEnterKeyAction();

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals:
    void sigMouseClicked(quint32 indexInPage);
    void sigUpdateCurrentElement(int index);
};

#endif // RECTWITHTEXT_H
