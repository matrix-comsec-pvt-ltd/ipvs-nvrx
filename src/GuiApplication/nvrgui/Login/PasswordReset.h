#ifndef PASSWORD_RESET_H
#define PASSWORD_RESET_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "ApplController.h"
#include "DataStructure.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/BackGround.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "Controls/OptionSelectButton.h"
#include "NavigationControl.h"
#include "PayloadLib.h"
#include "Controls/TextLabel.h"
#include "KeyBoard.h"

typedef enum
{
    PWD_RST_LABEL_CLOSE_BUTTON = 0,
    PWD_RST_LABEL_USERNAME,
    PWD_RST_LABEL_PASSWORD,
    PWD_RST_LABEL_CONFIRM_PASSWORD,
    PWD_RST_LABEL_PASSWORD_BUTTON,
    PWD_RST_LABEL_MAX,
}PWD_RST_LABEL_e;

class PasswordReset : public BackGround
{
    Q_OBJECT

private:
    CloseButtton*           m_closeButton;
    TextboxParam*           m_usernameParam;
    TextBox*                m_usernameTextBox;
    TextboxParam*           m_passwordParam;
    TextBox*                m_passwordTextBox;
    TextboxParam*           m_confmpasswordParam;
    TextBox*                m_confmpasswordTextBox;
    CnfgButton*             m_logInButton;

    ApplController*         m_applController;
    ProcessBar*             m_processBar;
    PayloadLib*             m_payloadLib;
    InfoPage*               m_infoPage;
    quint8                  m_currentElement;
    NavigationControl*      m_elementList[PWD_RST_LABEL_MAX];
    QString                 m_currentDeviceName;

public:
    explicit PasswordReset(QString deviceName, QString username, QWidget *parent = 0);
    ~PasswordReset();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void sendCommand(PWD_RST_CMD_e cmdType, int totalfeilds);

signals:
    void sigExitPwdRstPage(void);

public slots:
    void slotUpdateCurrentElement(int indexInPage);
    void slotcloseButtonClick(int);
    void slotButtonClick(int);
    void slotInfoPageCnfgBtnClick(int);
};

#endif // PASSWORD_RESET_H
