#include "TextEdit.h"

#include <QMouseEvent>

TextEdit::TextEdit(quint32 startX,
                   quint32 startY,
                   quint32 width,
                   quint32 height,
                   QWidget *parent)
    :QTextEdit(parent)
{
    this->setGeometry (startX,
                       startY,
                       width,
                       height);

    this->setMouseTracking (true);
    this->setContextMenuPolicy (Qt::NoContextMenu);
    setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
    setFrameShape (QTextEdit::NoFrame);
    textCursor = QTextEdit::textCursor ();
    this->show ();
}

void TextEdit::mousePressEvent (QMouseEvent *event)
{
    if(event->button () == Qt::LeftButton)
    {
        QTextEdit::mousePressEvent (event);

        // NOTE :: Textedit return curposition at clicked postion
        //         messagebox's child (textedit) and
        //         QTextedit cordinates are different so event position are also diffrent
        //         so have to take here to get correct current position
        textCursor = cursorForPosition (event->pos ());
        QWidget::mousePressEvent(event);
        setActiveFocus ();
    }
}

int TextEdit:: getCurrentCurrsorPosition () const
{
    return textCursor.position ();
}

void TextEdit::setCurrentCursorPosition (int pos)
{
    textCursor.setPosition (pos);
    setTextCursor (textCursor);
}

void TextEdit::setActiveFocus ()
{
    this->setFocus ();
}

void TextEdit::navigationKeyPressed(QKeyEvent *event)
{
    bool retVal = false;

    switch(event->key())
    {
    case Qt::Key_Left:
        if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
        {
            retVal = textCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
        }
        else
        {
            if(textCursor.position() != textCursor.anchor())
            {
                quint16 diff;

                if(textCursor.position() > textCursor.anchor())
                    diff = abs((textCursor.position() - textCursor.anchor()));
                else
                    diff = 0;

                textCursor.clearSelection();
                textCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, diff);
                retVal = true;
            }
            else
            {
                retVal = textCursor.movePosition(QTextCursor::Left);
            }
        }
        break;

    case Qt::Key_Right:
        if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
        {
            retVal = textCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
        }
        else
        {
            if(textCursor.position() != textCursor.anchor())
            {
                quint16 diff;

                if(textCursor.position() < textCursor.anchor())
                    diff = abs((textCursor.position() - textCursor.anchor()));
                else
                    diff = 0;

                textCursor.clearSelection();
                textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, diff);
                retVal = true;
            }
            else
            {
                retVal = textCursor.movePosition(QTextCursor::Right);
            }
        }
        break;

    case Qt::Key_Up:
        if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
        {
            retVal = textCursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
        }
        else
        {
            if(textCursor.position() != textCursor.anchor())
            {
                quint16 diff;

                if(textCursor.position() > textCursor.anchor())
                    diff = abs((textCursor.position() - textCursor.anchor()));
                else
                    diff = 0;

                textCursor.clearSelection();
                textCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, diff);
                retVal = true;
            }
            else
            {
                retVal = textCursor.movePosition(QTextCursor::Up);
            }
        }
        break;

    case Qt::Key_Down:
        if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
        {
            retVal = textCursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
        }
        else
        {
            if(textCursor.position() != textCursor.anchor())
            {
                quint16 diff;

                if(textCursor.position() < textCursor.anchor())
                    diff = abs((textCursor.position() - textCursor.anchor()));
                else
                    diff = 0;

                textCursor.clearSelection();
                textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, diff);
                retVal = true;
            }
            else
            {
                retVal = textCursor.movePosition(QTextCursor::Down);
            }
        }
        break;

    case Qt::Key_Home:
        if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
        {
            retVal = textCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        }
        else
        {
            if(textCursor.position() != textCursor.anchor())
            {
                textCursor.clearSelection();
                textCursor.movePosition(QTextCursor::Start);
                retVal = true;
            }
            else
            {
                retVal = textCursor.movePosition(QTextCursor::Start);
            }
        }
        break;

    case Qt::Key_End:
        if((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier)
        {
            retVal = textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        }
        else
        {
            if(textCursor.position() != textCursor.anchor())
            {
                textCursor.clearSelection();
                textCursor.movePosition(QTextCursor::End);
                retVal = true;
            }
            else
            {
            retVal = textCursor.movePosition(QTextCursor::End);
            }
        }
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        textCursor.setPosition(0);
        retVal = textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        break;

    default:
        break;
    }

    if(retVal)
    {
        setTextCursor (textCursor);
    }
}

void TextEdit::deleteKeyPressed(QKeyEvent *event)
{
    Q_UNUSED(event);
    textCursor.deleteChar();
    setTextCursor (textCursor);
}

void TextEdit::backspaceKeyPressed(QKeyEvent *event)
{
    Q_UNUSED(event);
    textCursor.deletePreviousChar();
    setTextCursor (textCursor);
}

bool TextEdit::hasSelectedText()
{
    return (textCursor.position() != textCursor.anchor());
}

void TextEdit::insertText(QString str)
{
    textCursor.insertText(str);
    setTextCursor (textCursor);
}

void TextEdit::selectAllText()
{
    textCursor.select(QTextCursor::BlockUnderCursor);
    setTextCursor (textCursor);
}

void TextEdit::truncateText(quint8 charCount)
{
    textCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, charCount);
    textCursor.removeSelectedText();
    setTextCursor (textCursor);
}

void TextEdit::keyPressEvent (QKeyEvent *evt)
{
    switch (evt->key())
    {
    case Qt::Key_Tab:
        break;

    case Qt::Key_Backtab:
        break;

    case Qt::Key_S:
    case Qt::Key_D:
        if(!((evt->modifiers() & Qt::ControlModifier) == Qt::ControlModifier))
        {
            QWidget::keyPressEvent (evt);
        }
        break;

    case Qt::Key_F5:
        break;

    default:
        QWidget::keyPressEvent (evt);
        break;
    }
}

void TextEdit::mouseMoveEvent (QMouseEvent *event)
{
    QWidget::mouseMoveEvent (event);
}

void TextEdit::focusOutEvent (QFocusEvent *evt)
{
    QTextEdit::focusOutEvent (evt);
    emit sigFocusChange(false, false);
}

void TextEdit::focusInEvent (QFocusEvent *evt)
{
    if(evt->reason () == Qt::MouseFocusReason)
    {
        emit sigFocusChange(true, true);
        return;
    }
    QTextEdit::focusInEvent (evt);
    emit sigFocusChange(true, false);
}
