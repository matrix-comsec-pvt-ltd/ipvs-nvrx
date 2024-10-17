#ifndef VIRTUALKEYPAD_H
#define VIRTUALKEYPAD_H

#include <QWidget>
#include "Controls/KeypadButton.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"

/* Total rows */
#define KEYPAD_KEY_ROWS     (5)

/* Total columns */
#define KEYPAD_KEY_COLUMNS  (14)

/* Virtual keyboard margin  */
#define KEYPAD_MARGIN       (30)

/* Virtual keyboard border width */
#define KEYPAD_BORDER_WIDTH     (2)

/* Virtual keyboard border radius */
#define KEYPAD_BORDER_RADIUS    (5)

/* Virtual Keyboard width */
#define KEYPAD_BG_WIDTH     ((SCALE_WIDTH(KEYPAD_KEY_SIZE)*KEYPAD_KEY_COLUMNS) + (SCALE_WIDTH(KEYPAD_KEY_MARGIN)*(KEYPAD_KEY_COLUMNS-1)) + (SCALE_WIDTH(KEYPAD_MARGIN)*2))

/* Virtual Keyboard height */
#define KEYPAD_BG_HEIGHT    ((SCALE_HEIGHT(KEYPAD_KEY_SIZE)*KEYPAD_KEY_ROWS) + (SCALE_HEIGHT(KEYPAD_KEY_MARGIN)*(KEYPAD_KEY_ROWS-1)) + (SCALE_HEIGHT(KEYPAD_MARGIN)*2))

#define KEYPAD_NUM_KEY_ROW          4
#define KEYPAD_NUM_KEY_COL          12
#define KEYPAD_IMG_KEY              10

class VirtualKeypad : public Rectangle
{
    Q_OBJECT
public:
    VirtualKeypad(int startx,
                  int starty,
                  QWidget *parent = 0);
    ~VirtualKeypad();
    void createDefaultComponent(void);

    void mousePressEvent (QMouseEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void mouseReleaseEvent (QMouseEvent *event);

private:
    bool isCapsOn;
    bool isMousePressed;
    int startX;
    int startY;
    QPoint pressedPoint;
    CloseButtton *closeButton;
    KeypadButton *alphaNumKeys[KEYPAD_NUM_KEY_ROW][KEYPAD_NUM_KEY_COL];
    KeypadButton *otherKeys[KEYPAD_IMG_KEY];

signals:
    void sigKeyDetected(KEY_TYPE_e keyType, QString str);

public slots:
    void slotKeyDeteceted(KEY_TYPE_e keyType, quint16 index);
    void slotCloseClicked(int index);
};

#endif // VIRTUALKEYPAD_H
