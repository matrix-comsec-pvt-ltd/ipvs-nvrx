#ifndef PASSWORDRECOVERY_H
#define PASSWORDRECOVERY_H

#include <QWidget>
#include "Controls/Closebuttton.h"
#include "Controls/ElementHeading.h"
#include "Controls/DropDown.h"
#include "Controls/TextBox.h"
#include "Controls/CnfgButton.h"
#include "Controls/ProcessBar.h"
#include "Controls/InfoPage.h"
#include "PayloadLib.h"
#include "ManagePages/ManagePasswordRecovery.h"

class PasswordRecovery : public BackGround
{
    Q_OBJECT

private:
    CloseButtton*           m_closeButton;
    ElementHeading*         m_heading;
    DropDown*               m_secQuestionDropDown[PASSWORD_RECOVERY_QA_MAX];
    TextboxParam*           m_answerTextboxParam[PASSWORD_RECOVERY_QA_MAX];
    TextBox*                m_answerTextBox[PASSWORD_RECOVERY_QA_MAX];
    TextboxParam*           m_emailIdTextboxParam;
    TextBox*                m_emailIdTextBox;
    CnfgButton*             m_okButton;
    CnfgButton*             m_cancelButton;
    ApplController*         m_applController;
    ProcessBar*             m_processBar;
    PayloadLib*             m_payloadLib;
    InfoPage*               m_infoPage;
    quint8                  m_currentElement;
    NavigationControl*      m_elementList[PWD_RECOVERY_STRINGS_MAX];
    QString                 m_currentDeviceName;

public:
    explicit PasswordRecovery(QWidget *parent = 0);
    ~PasswordRecovery();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds);
    void updateList(PASSWORD_RECOVERY_QA_e dropDownIndex_1, PASSWORD_RECOVERY_QA_e dropDownIndex_2, QMap<quint8, QString> &securityQuestionsList);
    void updateSecurityQuestionDropdownList(PASSWORD_RECOVERY_QA_e dropDownIndex, QMap<quint8, QString> securityQuestionsList);
    void saveConfig(void);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent(QShowEvent *event);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

signals:    
    void sigExitPage(void);

public slots:
    void slotTextBoxInfoPage(int indexInPage, INFO_MSG_TYPE_e msgType);
    void slotcloseButtonClick(int indexInPage);
    void slotSpinBoxValueChange(QString, quint32 indexInPage);
    void slotConfigButtonClick(int indexInPage);
    void slotUpdateCurrentElement(int);
};

#endif // PASSWORDRECOVERY_H
