#ifndef MODIFY_PASSWORD_H
#define MODIFY_PASSWORD_H

#include "ManageMenuOptions.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/CnfgButton.h"

#define MAX_PASSWORD_BOX 3


class ModifyPassword : public ManageMenuOptions
{
    Q_OBJECT
private:
    TextboxParam* passwordParam[MAX_PASSWORD_BOX];
    PasswordTextbox* passwordTextBox[MAX_PASSWORD_BOX];

    CnfgButton* okButton;
    QString oldPasswordValue;
    QString oldPassword, newPassword, confirmPassword;
    STATE_TYPE_e m_logButtonState;

public:
    explicit ModifyPassword(QString devName,
                   QWidget *parent = 0,
                   STATE_TYPE_e state = STATE_1);
    ~ModifyPassword();

    void changePassword(QString devName);
    void getOldPassword(QString devName);

    void processDeviceResponse(DevCommParam *param, QString devName);

public slots:
    void slotTextBoxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
    void slotOkButtonClick(int);
};

#endif // MODIFY_PASSWORD_H
