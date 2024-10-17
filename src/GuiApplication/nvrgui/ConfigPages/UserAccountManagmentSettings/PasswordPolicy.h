#ifndef PASSWORDPOLICY_H
#define PASSWORDPOLICY_H

#include <QWidget>
#include "Controls/ConfigPageControl.h"

#include "Controls/ElementHeading.h"
#include "Controls/DropDown.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"


class PasswordPolicy : public ConfigPageControl
{
     Q_OBJECT

public:

    explicit PasswordPolicy(QString devName,QWidget *parent = 0,
                   DEV_TABLE_INFO_t *tableInfo = NULL);
    ~PasswordPolicy();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getConfig();
    void defaultConfig();
    void saveConfig();

public slots:
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);

private:

    // Password Strength
    ElementHeading*         m_PasswordStrengthHeading;
    TextboxParam*           m_minPassLenParam;
    TextBox*                m_minPassTextBox;

    // Password validity
    OptionSelectButton*     m_setPassValidityCheckBox;
    TextboxParam*           m_resetPassParam;
    TextBox*                m_resetPassTextBox;


    // Account Logout Policy
    OptionSelectButton*     m_logoutPolicyCheckBox;
    TextboxParam*           m_loginAttemParam;
    TextBox*                m_loginAttemTextBox;
    TextboxParam*           m_lockAccountParam;
    TextBox*                m_lockAccountTextBox;


    void createDefaultComponents();
    void createPayload(REQ_MSG_ID_e msgType);
};

#endif // PASSWORDPOLICY_H
