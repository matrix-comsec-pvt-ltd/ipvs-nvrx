#include "InvisibleWidgetCntrl.h"
#include <QMouseEvent>

InvisibleWidgetCntrl::InvisibleWidgetCntrl(QWidget *parent)
    :   QWidget(parent)
{
    this->setGeometry (parent->geometry ());
    this->show ();
}

void InvisibleWidgetCntrl :: mouseMoveEvent (QMouseEvent *event)
{
    event->accept ();
}

void InvisibleWidgetCntrl :: mouseDoubleClickEvent (QMouseEvent *event)
{
    event->accept ();
}

void InvisibleWidgetCntrl :: mouseReleaseEvent (QMouseEvent *event)
{
    event->accept ();
    emit sigMouseClick();
}

void InvisibleWidgetCntrl :: mousePressEvent (QMouseEvent *event)
{
    event->accept ();
}
