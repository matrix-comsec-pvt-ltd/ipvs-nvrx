#ifndef VIRTUALKEYPAD_H
#define VIRTUALKEYPAD_H

#include <QWidget>
#include "Controls/KeypadButton.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"

#define KEYPAD_NUM_KEY_ROW          4
#define KEYPAD_NUM_KEY_COL          12
#define KEYPAD_IMG_KEY              10

class VirtualKeypad : public Rectangle
{
    Q_OBJECT
public:
    VirtualKeypad(int startx, int starty, QWidget *parent = 0);
    ~VirtualKeypad();
    void createDefaultComponent(void);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    bool            isCapsOn;
    bool            isMousePressed;
    int             startX;
    int             startY;
    QPoint          pressedPoint;
    CloseButtton*   closeButton;
    KeypadButton*   alphaNumKeys[KEYPAD_NUM_KEY_ROW][KEYPAD_NUM_KEY_COL];
    KeypadButton*   otherKeys[KEYPAD_IMG_KEY];

signals:
    void sigKeyDetected(KEY_TYPE_e keyType, QString str);

public slots:
    void slotKeyDeteceted(KEY_TYPE_e keyType, quint16 index);
    void slotCloseClicked(int index);
};
#endif // VIRTUALKEYPAD_H
