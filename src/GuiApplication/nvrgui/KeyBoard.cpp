#include "KeyBoard.h"
#include <QKeyEvent>

KeyBoard::KeyBoard(QWidget* parent):QWidget(parent)
{

}

void KeyBoard::keyPressEvent(QKeyEvent *event)
{
    unloadVirtualKeyboard();

    switch (event->key())
    {
    case Qt::Key_Space:
    case Qt::Key_Exclam:
    case Qt::Key_QuoteDbl:
    case Qt::Key_NumberSign:
    case Qt::Key_Dollar:
    case Qt::Key_Percent:
    case Qt::Key_Ampersand:
    case Qt::Key_Apostrophe:
    case Qt::Key_ParenLeft:
    case Qt::Key_ParenRight:
    case Qt::Key_Asterisk:
    case Qt::Key_Plus:
    case Qt::Key_Comma:
    case Qt::Key_Minus:
    case Qt::Key_Period:
    case Qt::Key_Slash:
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
    case Qt::Key_Colon:
    case Qt::Key_Semicolon:
    case Qt::Key_Less:
    case Qt::Key_Equal:
    case Qt::Key_Greater:
    case Qt::Key_Question:
    case Qt::Key_At:
    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
    case Qt::Key_E:
    case Qt::Key_F:
    case Qt::Key_G:
    case Qt::Key_H:
    case Qt::Key_I:
    case Qt::Key_J:
    case Qt::Key_K:
    case Qt::Key_L:
    case Qt::Key_M:
    case Qt::Key_N:
    case Qt::Key_O:
    case Qt::Key_P:
    case Qt::Key_Q:
    case Qt::Key_R:
    case Qt::Key_S:
    case Qt::Key_T:
    case Qt::Key_U:
    case Qt::Key_V:
    case Qt::Key_W:
    case Qt::Key_X:
    case Qt::Key_Y:
    case Qt::Key_Z:
    case Qt::Key_BracketLeft:
    case Qt::Key_Backslash:
    case Qt::Key_BracketRight:
    case Qt::Key_AsciiCircum:
    case Qt::Key_Underscore:
    case Qt::Key_QuoteLeft:
    case Qt::Key_BraceLeft:
    case Qt::Key_Bar:
    case Qt::Key_BraceRight:
    case Qt::Key_AsciiTilde:
        if((event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)
        {
            switch (event->key())
            {
            case Qt::Key_A:
                    ctrl_A_KeyPressed(event);
                break;

            case Qt::Key_C:
                    ctrl_C_KeyPressed(event);
                break;

            case Qt::Key_D:
                    ctrl_D_KeyPressed(event);
                break;

            case Qt::Key_S:
                    ctrl_S_KeyPressed(event);
                break;

            case Qt::Key_V:
                    ctrl_V_KeyPressed(event);
                break;

            case Qt::Key_X:
                    ctrl_X_KeyPressed(event);
                break;

            case Qt::Key_Y:
                    ctrl_Y_KeyPressed(event);
                break;

            case Qt::Key_Z:
                    ctrl_Z_KeyPressed(event);
                break;

            default:
                break;
            }
        }
        else
        {
            asciiCharKeyPressed(event);
        }
        break;

    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
        navigationKeyPressed(event);
        break;

    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4:
    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8:
    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12:
        functionKeyPressed(event);
        break;

    case Qt::Key_Escape:
        escKeyPressed(event);
        break;

    case Qt::Key_Tab:
        tabKeyPressed(event);
        break;

    case Qt::Key_Backspace:
        backspaceKeyPressed(event);
        break;

    case Qt::Key_Return:
    case Qt::Key_Enter:
        enterKeyPressed(event);
        break;

    case Qt::Key_Shift:
        shiftKeyPressed(event);
        break;

    case Qt::Key_Backtab:
         backTab_KeyPressed(event);
        break;

    case Qt::Key_Insert:
        insertKeyPressed(event);
        break;

    case Qt::Key_Delete:
        deleteKeyPressed(event);
        break;

    default:
        break;
    }
}

// Uncomment To avoid default functionality of Tab key
bool KeyBoard::focusNextPrevChild(bool next)
{
    Q_UNUSED(next);
    return false;
}

void KeyBoard::unloadVirtualKeyboard()
{

}

void KeyBoard::asciiCharKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::navigationKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::functionKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::escKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::tabKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::backspaceKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::enterKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::shiftKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::insertKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::deleteKeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_A_KeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_C_KeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_D_KeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_S_KeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_V_KeyPressed(QKeyEvent *event)
{
     QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_X_KeyPressed(QKeyEvent *event)
{
     QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_Y_KeyPressed(QKeyEvent *event)
{
     QWidget::keyPressEvent(event);
}

void KeyBoard::ctrl_Z_KeyPressed(QKeyEvent *event)
{
     QWidget::keyPressEvent(event);
}

void KeyBoard::backTab_KeyPressed(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}
