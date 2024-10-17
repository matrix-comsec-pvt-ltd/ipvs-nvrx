#ifndef KEYPADBUTTON_H
#define KEYPADBUTTON_H

#include <QWidget>
#include "Controls/TextLabel.h"

typedef enum
{
    KEY_ALPHANUM = 0,
    KEY_CLEAR,
    KEY_DONE,
    KEY_SPACE,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_CAPS,
    KEY_LEFT_ARROW,
    KEY_RIGHT_ARROW,
    KEY_UP_ARROW,
    KEY_DOWN_ARROW,

    KEY_MAX_TYPE,
    KEY_CLOSE,
    KEY_ALPHANUM_SAME_INDEX
}KEY_TYPE_e;

class KeypadButton : public QWidget
{
    Q_OBJECT
public:
    KeypadButton(int statrtX,
                 int statrtY,
                 int index,
                 KEY_TYPE_e key,
                 QString label,
                 QWidget *parent = 0,
                 quint8 fontSize = NORMAL_FONT_SIZE);
    ~KeypadButton();

//    static int unitBorderThickness;
//    static int twiceBorderThickness;
//    static int thriceBorderThickness;

    void setRectanglesGeometry();
    void drawRectangles(QPainter * painter, bool showClickedImage = false);
    void resetGeometry(int xOffset, int yOffset);

    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent (QMouseEvent *);

    void changeButtonText(QString str);
    void changeFontSize(quint8 fontSize);

private:
    quint16 m_index;
    quint16 m_width;
    quint16 m_height;
    quint16 m_horizontalOffset;
    quint16 m_startX, m_startY;
    QPixmap m_image;
    QRect m_imgRect;
    KEY_TYPE_e keyType;

    QPoint mousePressPoint;
    bool isMousePress;

    QRect * m_mainRect;
    QRect * m_topRect;
    QRect * m_bottomRect;
    QRect * m_leftRect;
    QRect * m_rightRect;
    QRect * m_bottomRect_1;
    QRect * m_bottomRect_2;
    QRect * m_bottomRect_3;
    QRect * m_leftRect_1;
    QRect * m_leftRect_2;
    QRect * m_leftRect_3;
    QString m_label;
    TextLabel* m_textLabel;

//    QString imgPath;
//    QPixmap image;

    bool m_isMouseHover;
    bool m_isMousePressed;

    quint8 m_fontSize;

signals:
    void sigKeyPressed(KEY_TYPE_e keyType, quint16 index);

public slots:

};

#endif // KEYPADBUTTON_H
