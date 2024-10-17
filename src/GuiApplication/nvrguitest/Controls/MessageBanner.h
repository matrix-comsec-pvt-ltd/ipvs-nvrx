#ifndef MESSAGEBANNER_H
#define MESSAGEBANNER_H

#include <QWidget>
#include <QTimer>
#include "Controls/TextLabel.h"

class MessageBanner : public QWidget
{
    Q_OBJECT
public:
    explicit MessageBanner(QWidget *parent = 0);
    ~MessageBanner();

    void loadInfoMessage(QString tempStr);
    QString getMessageStr();

signals:
    void sigLoadNextPage();
    
public slots:
    void slotTimerOut();

private:
    QRect       m_mainRect;
    QRect       m_innerRect;
    QTimer*     m_showMsgTimer;
    TextLabel*  m_textLabel;

    void paintEvent (QPaintEvent *);    
};

#endif // MESSAGEBANNER_H
