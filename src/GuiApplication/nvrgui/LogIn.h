#ifndef LOGIN_H
#define LOGIN_H

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
#include "Controls/OptionSelectButton.h"
#include "NavigationControl.h"
#include "PayloadLib.h"
#include "Controls/TextLabel.h"
#include "KeyBoard.h"
#include "Login/PasswordRecovery.h"
#include "Login/ForgotPassword.h"
#include "Configuration/DeviceConfig.h"

typedef enum
{
    LOG_IN_CLS_BTN,
    LOG_IN_USR_TXTBOX,
    LOG_IN_PASS_TXTBOX,
    LOG_IN_REM_PASS_CHEKBOX,
    LOG_IN_LOG_BTN,
    LOG_IN_FORGOT_PASS_LINK,
    MAX_LOG_IN_CTRL
}LOG_IN_CTRL_e;

typedef enum
{
    LOGIN_NORMAL,
    LOGIN_RESET_PASS,
    LOGIN_EXPIRE_PASS,
    LOGIN_LOG_OUT,
    MAX_LOGIN_STATE

}LOGIN_STATE_e;


class LogIn : public BackGround
{
    Q_OBJECT
public:
    explicit LogIn(STATE_TYPE_e imgType,QWidget *parent = 0);
    ~LogIn();

    void getlocalUserParam();
    void getOtherUserParam();
    bool sendChangeUserCmd();
    void sendChangePsdCmd();
    void getDeviceConfig();
    void rememberStatusToFile();

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void showEvent (QShowEvent *event);

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void deleteElements();

    LOGIN_STATE_e getCurrentLoginState() const;
    void setCurrentLoginState(const LOGIN_STATE_e &currentLoginState);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

signals:
    void sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state);

public slots:
    void slotExitPage(void);
    void slotButtonClick(int);
    void slotInfoPageBtnclick(int);
    void slotUpdateCurrentElement(int);
    void slotTextClicked(int);
    void slotTextLableHover(int, bool isHoverIn);    

private:

    STATE_TYPE_e            m_imgType;
    OTHER_LOGIN_CONFIG_t    m_otherLoginParam;

    QString                 m_username;
    QString                 m_password;
    QString                 m_currentDevName;

    ApplController*         m_applController;
    PayloadLib*             m_payloadLib;

    TextboxParam*           m_userParam;
    TextBox*                m_userTextBox;

    TextboxParam*           m_currentPasswordParam;
    PasswordTextbox*        m_currentPasswordTextBox;

    TextboxParam*           m_passwordParam;
    PasswordTextbox*        m_passwordTextBox;

    TextboxParam*           m_confmpasswordParam;
    PasswordTextbox*        m_confmpasswordTextBox;

    OptionSelectButton*     m_rmbPwdChkBox;

    InfoPage*               m_infoPage;
    CnfgButton*             m_logInButton;

    TextLabel*              m_logInHeadings;
    TextLabel*              m_forgotPasswordLabel;
    PasswordRecovery*       m_passwordRecovery;  
	ForgotPassword*         m_forgotPassword;
    NavigationControl*      m_elementList[MAX_LOG_IN_CTRL];
    quint8                  m_currElement;

    bool                    m_isLogInRequestSend;

    LOGIN_STATE_e           m_currentLoginState;
protected:
    DEVICE_CONFIG_t rdevTempConfig[MAX_REMOTE_DEVICES];
};

#endif // LOGIN_H
