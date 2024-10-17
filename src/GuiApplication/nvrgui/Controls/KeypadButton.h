#ifndef KEYPADBUTTON_H
#define KEYPADBUTTON_H

#include <QWidget>
#include "Controls/TextLabel.h"

/* Width x Height of a alphanum key */
#define KEYPAD_KEY_SIZE     (38)

/* Margin between each key */
#define KEYPAD_KEY_MARGIN   (6)

/* key border radius */
#define KEYPAD_KEY_BORDER_RADIUS    (5)

/* Special key size (Done, Clear, Caps) */
#define KEYPAD_SPECIAL_KEY_1_SIZE   (KEYPAD_KEY_SIZE*1.5)

/* Special key size (Down Arrow, Up Arrow, Backspace, Enter) */
#define KEYPAD_SPECIAL_KEY_2_SIZE   ((KEYPAD_KEY_SIZE*2) + KEYPAD_KEY_MARGIN)

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

    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent (QMouseEvent *);

    void changeButtonText(QString str);
    void changeFontSize(quint8 fontSize);

private:
    quint16     m_index;
    quint16     m_width;
    quint16     m_height;
    quint16     m_startX, m_startY;
    QPixmap     m_image;
    QRect       m_imgRect;
    KEY_TYPE_e  keyType;
    QRect       *m_mainRect;
    QString     m_label;
    TextLabel   *m_textLabel;
    bool        m_isMousePressed;
    quint8      m_fontSize;

signals:
    void sigKeyPressed(KEY_TYPE_e keyType, quint16 index);

public slots:

};

#endif // KEYPADBUTTON_H
