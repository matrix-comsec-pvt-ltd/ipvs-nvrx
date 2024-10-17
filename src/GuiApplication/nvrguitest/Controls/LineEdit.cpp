#include "LineEdit.h"
#include <QMouseEvent>

LineEdit::LineEdit(quint32 startX,
                   quint32 startY,
                   quint32 width,
                   quint32 height,
                   QWidget *parent,
                   quint8 index)
    :QLineEdit(parent)
{
    m_index = index;
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
    QWidget::keyPressEvent (evt);
}

void LineEdit::mouseMoveEvent (QMouseEvent *event)
{
    QWidget::mouseMoveEvent (event);
}

void LineEdit::focusOutEvent (QFocusEvent *evt)
{
    QLineEdit::focusOutEvent (evt);
    emit sigFocusChange(m_index, false, false);
}

void LineEdit::focusInEvent (QFocusEvent *evt)
{
    // This is for bug fixing....
    // As we found focus in lineedit by reason of mouse
    // then by forcfully give focus to else, so lineedit has no focus
    // then click us affected , at that time we give focus another time....
    if(evt->reason () == Qt::MouseFocusReason)
    {
        emit sigFocusChange(m_index, true, true);
        return;
    }
    QLineEdit::focusInEvent (evt);
    emit sigFocusChange(m_index, true, false);
}
