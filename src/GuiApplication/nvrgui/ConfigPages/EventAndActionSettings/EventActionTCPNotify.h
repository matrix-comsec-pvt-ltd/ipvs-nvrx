#ifndef EVENTACTIONTCPNOTIFY_H
#define EVENTACTIONTCPNOTIFY_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/InfoPage.h"
#include "Controls/MessageBox.h"
#include "Controls/TextLabel.h"

typedef enum{

    EVNT_TCP_CLS_CTRL,
    EVNT_TCP_MSGBOX_CTRL,
    EVNT_TCP_OK_CTRL,
    EVNT_TCP_CANCEL_CTRL,

    MAX_EVNT_TCP_NOTIFY_CTRL
}EVNT_TCP_NOTIFY_CTRL_e;

class EventActionTCPNotify : public KeyBoard
{
    Q_OBJECT

    Rectangle* backGround;
    CloseButtton* closeButton;
    Heading* heading;

    InfoPage* infoPage;
    MessageBox* msgBox;
    CnfgButton* okButton;
    CnfgButton* cancelButton;

    NavigationControl* m_elementlist[MAX_EVNT_TCP_NOTIFY_CTRL];
    quint8 currElement;

    quint8 m_index;

    QString*      message;
    TextLabel*    msgCountLabel;

public:
    explicit EventActionTCPNotify(quint8 index,QString &tcpmessage, QWidget *parent = 0);
    ~EventActionTCPNotify();

    void paintEvent (QPaintEvent *event);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent (QShowEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);

public slots:
    void slotButtonClick(int);
    void slotUpdateCurrentElement(int index);
    void slotInfoPageCnfgBtnClick(int index);
    void slotTextBoxValueAppend(QString str, int);

private:
    quint32 countNumberOfCharInMessage(QString str);
};

#endif // EVENTACTIONTCPNOTIFY_H
