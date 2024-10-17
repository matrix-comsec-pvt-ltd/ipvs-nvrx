#ifndef MESSAGEBANNER_H
#define MESSAGEBANNER_H

#include <QWidget>
#include <QTimer>
#include "Controls/TextLabel.h"
#include "Rectangle.h"
#include "EnumFile.h"

class MessageBanner : public QWidget
{
    Q_OBJECT
private:
    QTimer* m_showTimer;
    QStringList m_messageList;
    TextLabel* m_textLabel;
    Rectangle* m_backgroundRectangle;

public:
    static MessageBanner* messageBanner;

public:
    explicit MessageBanner(QWidget* parent = 0);
    ~MessageBanner();

    static MessageBanner* getInstance(QWidget* parent);
    static void addMessageInBanner(QString message);
    static void flushQueueOfMsgBanner();

    void loadMessageBanner(QString message);
    void changeMessage(QString text);
    void flushMessageQueue();
    void updateGeometry();

public slots:
    void slotShowTimerTimeout();
};

#endif // MESSAGEBANNER_H
