#ifndef USERVALIDATION_H
#define USERVALIDATION_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "DataStructure.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/BackGround.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "NavigationControl.h"
#include "KeyBoard.h"

typedef enum{
    USR_VLD_CLOSE_BTN,
    USR_VLD_USER_TEXTBOX,
    USR_VLD_PASSWORD_TEXTBOX,
    USR_VLD_OK_BTN,
    USR_VLD_CANCEL_BTN,
    MAX_USERVALIDATION_CONTROL
}USERVALIDATION_CONTROL_e;


class UsersValidation : public KeyBoard
{
    Q_OBJECT

    Rectangle* m_rectangle;
    Heading* m_heading;
    CloseButtton* m_closeButtton;

    TextboxParam* userParam;
    TextboxParam* passwordParam;
    TextBox* userTextBox;
    PasswordTextbox* passwordTextBox;

    InfoPage *infoPage;
    CnfgButton* okButton;

    quint32 m_currentElement;
    NavigationControl* m_elementList[MAX_USERVALIDATION_CONTROL];

    USER_VALD_PAGE_ID_e m_pageIndex;
    QString m_username, m_password;
public:
    explicit UsersValidation(QWidget *parent = 0,USER_VALD_PAGE_ID_e pageIndex = MAX_PAGE_ID);
    ~UsersValidation();

    void paintEvent (QPaintEvent *event);

    void takeLeftKeyAction ();
    void takeRightKeyAction ();
    void showEvent (QShowEvent *event);

    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void navigationKeyPressed(QKeyEvent *event);

signals:
    void sigOkButtonClicked(QString userName, QString password);

public slots:
    void slotClosePage(int);
    void slotOkButtonClick(int);
    void slotUpdateCurrentElement(int);
    void slotTextBoxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
    void slotInfoPageBtnclick(int);

};

#endif // USERVALIDATION_H
