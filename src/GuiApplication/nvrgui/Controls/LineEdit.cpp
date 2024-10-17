#include "LineEdit.h"
#include "EnumFile.h"
#include <QMouseEvent>

LineEdit::LineEdit(quint32 startX,
                   quint32 startY,
                   quint32 width,
                   quint32 height,
                   QWidget *parent,
                   quint8 index)
    :QLineEdit(parent), m_index(index)
{
    this->setGeometry (startX,
                       startY,
                       width,
                       height);

    this->setMouseTracking (true);
    this->setContextMenuPolicy (Qt::NoContextMenu);
    this->show ();
}

void LineEdit::mousePressEvent (QMouseEvent *event)
{
    if(event->button () == Qt::LeftButton)
    {
        QLineEdit::mousePressEvent (event);
        QWidget::mousePressEvent (event);
        this->setFocus ();
    }
}

void LineEdit::setActiveFocus ()
{
    this->setFocus ();
}

void LineEdit::keyPressEvent (QKeyEvent *evt)
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

void LineEdit::mouseMoveEvent (QMouseEvent *event)
{
    QWidget::mouseMoveEvent (event);
}

void LineEdit::focusOutEvent (QFocusEvent *evt)
{
    QLineEdit::focusOutEvent (evt);
}

void LineEdit::focusInEvent (QFocusEvent *evt)
{
    QLineEdit::focusInEvent (evt);
    emit sigFocusChange(m_index, true, false);
}
