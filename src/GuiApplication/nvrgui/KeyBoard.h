#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QObject>

#include "EnumFile.h"

class KeyBoard : public QWidget
{
    Q_OBJECT
public:

    explicit KeyBoard(QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *event);

    bool focusNextPrevChild(bool next);
    virtual void unloadVirtualKeyboard();   
    virtual void asciiCharKeyPressed(QKeyEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);

    //misc key functions
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backspaceKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void shiftKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);

    //ShortCut Key Functions
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void ctrl_A_KeyPressed(QKeyEvent *event);
    virtual void ctrl_C_KeyPressed(QKeyEvent *event);
    virtual void ctrl_D_KeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);
    virtual void ctrl_V_KeyPressed(QKeyEvent *event);
    virtual void ctrl_X_KeyPressed(QKeyEvent *event);
    virtual void ctrl_Y_KeyPressed(QKeyEvent *event);
    virtual void ctrl_Z_KeyPressed(QKeyEvent *event);

};

#endif // KEYBOARD_H
