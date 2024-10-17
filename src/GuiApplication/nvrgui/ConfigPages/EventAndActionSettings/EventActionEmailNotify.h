#ifndef EVENTACTIONEMAILNOTIFY_H
#define EVENTACTIONEMAILNOTIFY_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/MessageBox.h"
#include "Controls/TextBox.h"
#include "DataStructure.h"

typedef enum {

    EVNT_NOFY_CLS_CTRL,
    EVNT_NOFY_EMAIL_TXTBOX_CTRL,
    EVNT_NOFY_SUB_TXTBOX_CTRL,
    EVNT_NOFY_MSGBOX_CTRL,
    EVNT_NOFY_OK_CTRL,
    EVNT_NOFY_CANCEL_CTRL,

    MAX_EMAIL_EVENT_NOTIFY_CTRL
}EVNT_EMAIL_NOTIFY_CTRL_e;

class EventActionEmailNotify : public KeyBoard
{
    Q_OBJECT

    Rectangle*         backGround;
    CloseButtton*      closeButton;
    Heading*           heading;

    TextboxParam*      emailTextBoxParam;
    TextBox*           emailTextBox;

    TextboxParam*      subjectTextBoxParam;
    TextBox*           subjectTextBox;

    TextLabel*         emailNoteLabel;
    MessageBox*        msgBox;

    CnfgButton*        okButton;
    CnfgButton*        cancelButton;

    InfoPage*          infoPage;

    NavigationControl* m_elementlist[MAX_EMAIL_EVENT_NOTIFY_CTRL];
    quint8             currElement;
    quint8             m_index;

    QString*           m_address;
    QString*           m_subject;
    QString*           m_message;

    TextLabel*         msgCountLabel;

public:
    explicit EventActionEmailNotify(quint8 index,QString &address,
                                    QString &subject,
                                    QString &message,
                                    QWidget *parent = 0);
    ~EventActionEmailNotify();

    void paintEvent (QPaintEvent *event);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent (QShowEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);

public slots:
    void slotButtonClick(int);
    void slotUpdateCurrentElement(int index);
    void slotInfoPageCnfgBtnClick(int index);
    void slotTextBoxInfoPage(int,INFO_MSG_TYPE_e);
    void slotTextBoxValueAppend(QString str, int);

private:
    quint8 countNumberOfCharInMessage(QString str);
};

#endif // EVENTACTIONEMAILNOTIFY_H
