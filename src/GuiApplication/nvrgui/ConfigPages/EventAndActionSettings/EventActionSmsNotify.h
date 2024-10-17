#ifndef EVENTACTIONSMSNOTIFY_H
#define EVENTACTIONSMSNOTIFY_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"

#include "Controls/InfoPage.h"
#include "Controls/MessageBox.h"
#include "Controls/TextBox.h"
#include "Controls/TextLabel.h"
#include "DataStructure.h"

typedef enum {

    EVNT_SMS_CLS_CTRL,
    EVNT_SMS_MOB_NUM1_TXTBOX_CTRL,
    EVNT_SMS_MOB_NUM2_TXTBOX_CTRL,
    EVNT_SMS_MSGBOX_CTRL,
    EVNT_SMS_OK_CTRL,
    EVNT_SMS_CANCEL_CTRL,

    MAX_SMS_EVENT_NOTIFY_CTRL
}EVNT_SMS_NOTIFY_CTRL_e;

class EventActionSmsNotify : public KeyBoard
{

    Q_OBJECT
    Rectangle* backGround;
    CloseButtton* closeButton;
    Heading* heading;

    TextboxParam* mobileNumber1Param;
    TextBox*      mobileNumber1TextBox;

    TextboxParam* mobileNumber2Param;
    TextBox*      mobileNumber2TextBox;

    MessageBox*  msgBox;

    CnfgButton* okButton;
    CnfgButton* cancelButton;

    InfoPage* infoPage;

    NavigationControl* m_elementlist[MAX_SMS_EVENT_NOTIFY_CTRL];
    quint8 currElement;
    quint8 m_index;

    QString* smsNum1;
    QString* smsNum2;
    QString* smsMsg;

    TextLabel*      msgCountLabel;

public:
    explicit EventActionSmsNotify(quint8 index,
                                  QString &smsnum1,
                                  QString &smsnum2,
                                  QString &smsmesage,
                                  QWidget *parent);
    ~EventActionSmsNotify();

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
    void slotTextBoxInfoPage(int,INFO_MSG_TYPE_e);
    void slotTextBoxValueAppend(QString,int);

private:
    quint16 countNumberOfCharInMessage(QString str);

};

#endif // EVENTACTIONSMSNOTIFY_H
