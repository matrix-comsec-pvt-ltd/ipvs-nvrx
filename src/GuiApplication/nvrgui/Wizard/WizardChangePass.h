#ifndef WIZARDCHANGEPASS_H
#define WIZARDCHANGEPASS_H

#include <QWidget>
#include "Controls/PasswordTextbox.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "ValidationMessage.h"
#include "PayloadLib.h"
#include "ApplController.h"
#include "WizardCommon.h"
#include "Controls/ElementHeading.h"
#include "Elidedlabel.h"

#define MAX_PASSWORD_BOX 3

class WizardChangePass : public QWidget
{
    Q_OBJECT
private:
    TextboxParam* 			passwordParam[MAX_PASSWORD_BOX];
    PasswordTextbox* 		passwordTextBox[MAX_PASSWORD_BOX];

    CnfgButton* 			okButton;
    QString 				oldPasswordValue;
    QString 				oldPassword, newPassword, confirmPassword;
    STATE_TYPE_e 			m_logButtonState;
    QString 				m_currentDeviceName;
    ApplController*			m_applController;
    PayloadLib*				m_payloadLib;
    QTimer*                 MessageTimer;
    Rectangle*              backGround;
    Heading*                pageHeading;
    CloseButtton*           closeButtton;
    ElementHeading*         elementHeadingElide;
    ElidedLabel*            textLabelElide;

public:
    explicit WizardChangePass(QString devName,
                              QWidget *parent = 0,
                              STATE_TYPE_e state = STATE_1);
    ~WizardChangePass();
    void changePassword(QString devName);
    void getOldPassword(QString devName);
    void InfoMessage(QString StrInfo);

    void paintEvent(QPaintEvent *);
    void processDeviceResponse(DevCommParam *param, QString devName);
signals:
    void sigExitPage(TOOLBAR_BUTTON_TYPE_e);

public slots:

    void slotStatusRepTimerTimeout();
    void slotTextBoxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
    void slotOkButtonClick(int);
    void slotcloseButtonClick(int);
};

#endif // WIZARDCHANGEPASS_H
