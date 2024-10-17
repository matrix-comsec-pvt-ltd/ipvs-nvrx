#include "MessageBanner.h"

#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>

MessageBanner::MessageBanner(QWidget *parent) :
    QWidget(parent)
{
    m_showMsgTimer = new QTimer(this);
    connect (m_showMsgTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotTimerOut()));
    m_showMsgTimer->setInterval (2000);

    m_textLabel = new TextLabel(10,
                                -1,
                                16,
                                "",
                                this,
                                QString(RED_COLOR));
    this->hide ();
}

MessageBanner::~MessageBanner ()
{
    delete m_textLabel;
}

void MessageBanner::loadInfoMessage(QString tempStr)
{
    quint16 width,height;
    QFont font = TextLabel::getFont(WINDOW_FONT_FAMILY,22,false,false);
    TextLabel::getWidthHeight (font,tempStr,width,height);

    this->setGeometry (40,
                       800,
                       (width + 20),60);

    m_mainRect.setRect (0,0,0,0);
    m_innerRect.setRect (0,0,0,0);

    m_textLabel->changeText (tempStr);

    if(!(m_showMsgTimer->isActive ()))
        m_showMsgTimer->start ();

    this->show ();
}

QString MessageBanner::getMessageStr()
{
    return m_textLabel->getText();
}

void MessageBanner::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QBrush("#555555",Qt::SolidPattern));
    painter.drawRect(m_mainRect);

    painter.setBrush(QBrush("#1e1e1e",Qt::SolidPattern));
    painter.drawRect(m_innerRect);
}

void MessageBanner::slotTimerOut ()
{
    this->hide ();
    m_showMsgTimer->stop ();
    emit sigLoadNextPage ();
}
