#include "VirtualKeypad.h"
#include <QMouseEvent>

#define KEYPAD_BG_WIDTH         (976)
#define KEYPAD_BG_HEIGHT        (210)

QString lowerCaseKeys[KEYPAD_NUM_KEY_ROW][KEYPAD_NUM_KEY_COL] =
{
    {"1", "2", "3", "4", "5", "6", "7", "8", "9","0", "-", "+"},
    {"q", "w", "e", "r", "t", "y","u", "i", "o", "p", "[", "]"},
    {"a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "\\"},
    {"z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "`", "="}
};

QString upperCaseKeys[KEYPAD_NUM_KEY_ROW][KEYPAD_NUM_KEY_COL] =
{
    {"!", "@", "#", "$", "%", "^", "&", "*", "(",")", "_", "+"},
    {"Q", "W", "E", "R", "T", "Y","U", "I", "O", "P", "{", "}"},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "|"},
    {"Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "~", "="}
};


VirtualKeypad::VirtualKeypad(int startx, int starty, QWidget *parent):
    Rectangle(startx, starty, KEYPAD_BG_WIDTH, KEYPAD_BG_HEIGHT, 0,
              BORDER_2_COLOR, NORMAL_BKG_COLOR, parent)
{
    isCapsOn = false;
    isMousePressed = false;
    startX = startx;
    startY = starty;
    createDefaultComponent();
    this->show();
}

VirtualKeypad::~VirtualKeypad()
{
    if(IS_VALID_OBJ(closeButton))
    {
        disconnect(closeButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotCloseClicked(int)));
        DELETE_OBJ(closeButton);
    }

    for(quint8 row = 0; row < KEYPAD_NUM_KEY_ROW; row ++)
    {
        for(quint8 col = 0; col < KEYPAD_NUM_KEY_COL; col++)
        {
            if(IS_VALID_OBJ(closeButton))
            {
                disconnect(alphaNumKeys[row][col],
                        SIGNAL(sigKeyPressed(KEY_TYPE_e,quint16)),
                        this,
                        SLOT(slotKeyDeteceted(KEY_TYPE_e,quint16)));
                DELETE_OBJ(alphaNumKeys[row][col]);
            }
        }
    }

    for(quint8 index=0; index < KEYPAD_IMG_KEY; index++)
    {
        if(IS_VALID_OBJ(closeButton))
        {
            disconnect(otherKeys[index],
                        SIGNAL(sigKeyPressed(KEY_TYPE_e,quint16)),
                        this,
                        SLOT(slotKeyDeteceted(KEY_TYPE_e,quint16)));
            DELETE_OBJ(otherKeys[index]);
        }
    }
}

void VirtualKeypad::createDefaultComponent()
{
    closeButton = new CloseButtton(KEYPAD_BG_WIDTH,
                                   0, 0, 0,this, CLOSE_BTN_TYPE_1, 0, true, false,
                                   true);
    connect(closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCloseClicked(int)));

    for(quint8 row = 0; row < KEYPAD_NUM_KEY_ROW; row ++)
    {
        for(quint8 col = 0; col < KEYPAD_NUM_KEY_COL; col++)
        {
            quint8 fontSize = ((row == 3) &&
                               ((col == 7) || (col == 8)))
                    ? (25) : NORMAL_FONT_SIZE;


            alphaNumKeys[row][col] = new KeypadButton((33) + (col*((65))),
                                                      (35) + (((30)) * row),
                                                      (col + (12 * row)),
                                                      KEY_ALPHANUM,
                                                      lowerCaseKeys[row][col],
                                                      this,
                                                      fontSize);
            connect(alphaNumKeys[row][col],
                     SIGNAL(sigKeyPressed(KEY_TYPE_e,quint16)),
                     this,
                     SLOT(slotKeyDeteceted(KEY_TYPE_e,quint16)));
        }
    }



    otherKeys[0] = new KeypadButton(alphaNumKeys[0][0]->x(),
                                    alphaNumKeys[3][0]->y() + alphaNumKeys[3][0]->height(),
                                    -1, KEY_DONE,"Done", this);

    otherKeys[1] = new KeypadButton(otherKeys[0]->x() + otherKeys[0]->width(),
                                    otherKeys[0]->y(),
                                    -1, KEY_CAPS,"", this);

    otherKeys[2] = new KeypadButton(otherKeys[1]->x() + otherKeys[1]->width(),
                                    otherKeys[1]->y(),
                                    -1, KEY_SPACE,"", this);

    otherKeys[3] = new KeypadButton(otherKeys[2]->x() + otherKeys[2]->width(),
                                    otherKeys[2]->y(),
                                    -1, KEY_CLEAR,"Clear", this);

    otherKeys[4] = new KeypadButton(alphaNumKeys[0][11]->x() + alphaNumKeys[0][11]->width(),
                                    alphaNumKeys[0][11]->y(),
                                    -1, KEY_BACKSPACE,"", this);

    otherKeys[5] = new KeypadButton(otherKeys[4]->x(),
                                    alphaNumKeys[1][11]->y(),
                                    -1, KEY_ENTER,"", this);

    otherKeys[6] = new KeypadButton(otherKeys[4]->x(),
                                    alphaNumKeys[2][11]->y(),
                                    -1, KEY_UP_ARROW,"", this);

    otherKeys[7] = new KeypadButton(otherKeys[4]->x(),
                                    alphaNumKeys[3][11]->y(),
                                    -1, KEY_LEFT_ARROW,"", this);

    otherKeys[8] = new KeypadButton(otherKeys[4]->x() + (65),
                                    alphaNumKeys[3][11]->y(),
                                    -1, KEY_RIGHT_ARROW,"", this);

    otherKeys[9] = new KeypadButton(otherKeys[4]->x(),
                                    otherKeys[7]->y()+ (30),
                                    -1, KEY_DOWN_ARROW,"", this);

    for(quint8 index=0; index < KEYPAD_IMG_KEY; index++)
    {
        connect(otherKeys[index],
                 SIGNAL(sigKeyPressed(KEY_TYPE_e,quint16)),
                 this,
                 SLOT(slotKeyDeteceted(KEY_TYPE_e,quint16)));
    }

}
void VirtualKeypad::mousePressEvent(QMouseEvent *event)
{
    isMousePressed = true;
    pressedPoint = event->pos();
}

void VirtualKeypad::mouseMoveEvent(QMouseEvent *event)
{
    if(isMousePressed)
    {
        if(event->pos().x() > pressedPoint.x())
        {
            startX += (event->pos().x() - pressedPoint.x());
        }
        else
        {
            startX -= pressedPoint.x() - event->pos().x();
        }

        if(event->pos().y() > pressedPoint.y())
        {
            startY += (event->pos().y() - pressedPoint.y());
        }
        else
        {
            startY -= pressedPoint.y() - event->pos().y();
        }

        if(startX < 0)
        {
            startX = 0;
        }

        if(startY < 0)
        {
            startY = 0;
        }

        if((startX + KEYPAD_BG_WIDTH) >= this->window()->width())
        {
            startX = (this->window()->width() - KEYPAD_BG_WIDTH - 1);
        }

        if((startY + KEYPAD_BG_HEIGHT) >= this->window()->height())
        {
            startY = (this->window()->height() - KEYPAD_BG_HEIGHT - 1);
        }

        resetGeometry(startX, startY, KEYPAD_BG_WIDTH, KEYPAD_BG_HEIGHT);
    }
}


void VirtualKeypad::mouseReleaseEvent(QMouseEvent *)
{
    isMousePressed = false;
}

void VirtualKeypad::slotKeyDeteceted(KEY_TYPE_e keyType, quint16 index)
{
    quint8 row, col;
    QString keySend;

    if(keyType == KEY_CAPS)
    {
        if(isCapsOn == false)
        {
            isCapsOn = true;
            for(quint8 row = 0; row < KEYPAD_NUM_KEY_ROW; row ++)
            {
                for(quint8 col = 0; col < KEYPAD_NUM_KEY_COL; col++)
                {
                    alphaNumKeys[row][col]->changeButtonText(upperCaseKeys[row][col]);
                    if((row == (KEYPAD_NUM_KEY_ROW -1)) &&
                                       ((col == 7) || (col == 8)))
                    {
                        alphaNumKeys[row][col]->changeFontSize((15));
                    }
                }
            }
        }
        else
        {
            isCapsOn = false;
            for(quint8 row = 0; row < KEYPAD_NUM_KEY_ROW; row ++)
            {
                for(quint8 col = 0; col < KEYPAD_NUM_KEY_COL; col++)
                {
                    alphaNumKeys[row][col]->changeButtonText(lowerCaseKeys[row][col]);
                    if((row == (KEYPAD_NUM_KEY_ROW -1)) &&
                                       ((col == 7) || (col == 8)))
                    {
                        alphaNumKeys[row][col]->changeFontSize((25));
                    }
                }
            }
        }
        update();
    }
    else if(keyType == KEY_DONE)
    {
        emit sigKeyDetected(keyType, keySend);
        this->deleteLater();
    }
    else
    {
        if(keyType == KEY_ALPHANUM)
        {
            row = (index / KEYPAD_NUM_KEY_COL);
            col = (index % KEYPAD_NUM_KEY_COL);
            keySend = (isCapsOn == true)?upperCaseKeys[row][col] : lowerCaseKeys[row][col];
        }
        emit sigKeyDetected(keyType, keySend);
    }
}

void VirtualKeypad::slotCloseClicked(int)
{
    emit sigKeyDetected(KEY_CLOSE, "");
    this->deleteLater();
}
