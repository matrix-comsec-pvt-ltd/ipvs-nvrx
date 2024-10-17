#ifndef FORGOTPASSWORD_H
#define FORGOTPASSWORD_H

#include "ManagePages/ManagePasswordRecovery.h"
#include "Login/PasswordReset.h"

typedef enum
{
    FRGT_PWD_CLOSE_BTN,
    FRGT_PWD_VERIFY_MODE,
    FRGT_PWD_RECEIVE_OTP,
    FRGT_PWD_OTP_TXTBOX,
    FRGT_PWD_ANS_1_TXTBOX,
    FRGT_PWD_ANS_2_TXTBOX,
    FRGT_PWD_ANS_3_TXTBOX,
    FRGT_PWD_RESEND_OTP_TIMER,
    FRGT_PWD_VERIFY_BTN,
    FRGT_PWD_GET_OTP_BTN,
    FRGT_PWD_RESEND_OTP_BTN,
    FRGT_PWD_CANCEL_BTN,
    MAX_FRGT_PWD_CTRL
}FRGT_PWD_CTRL_e;

typedef enum
{
    PWD_RST_VERIFY_MODE_OTP = 0,
    PWD_RST_VERIFY_MODE_SEC_QA,
    PWD_RST_VERIFY_MODE_MAX,
}PWD_RST_VERIFY_MODE_e;

typedef struct
{
    bool        isModeCnfg[PWD_RST_VERIFY_MODE_MAX];
    QString     emailId;
    quint8      questionId[PASSWORD_RECOVERY_QA_MAX];
}PASSWORD_RESET_INFO_t;

class ForgotPassword : public BackGround
{
    Q_OBJECT

private:
    bool                    m_modeChangeAllow;
    CloseButtton*           m_closeButton;
    DropDown*               m_verificationModeDropDown;
    TextLabel*              m_receiveOtpLabel;
    TextLabel*              m_emailLabel;
    TextboxParam*           m_enterOtpParam;
    TextBox*                m_enterOtpTextBox;
    TextLabel*              m_otpTimerLabel;
    TextLabel*              m_resendOtpLabel;
    TextboxParam*           m_secQuestionTextboxParam[PASSWORD_RECOVERY_QA_MAX];
    TextBox*                m_secQuestionTextBox[PASSWORD_RECOVERY_QA_MAX];
    TextboxParam*           m_answerTextboxParam[PASSWORD_RECOVERY_QA_MAX];
    TextBox*                m_answerTextBox[PASSWORD_RECOVERY_QA_MAX];
    CnfgButton*             m_okButton;
    CnfgButton*             m_cancelButton;
    QTime*                  m_otpTime;
    QTimer*                 m_verifyOtpTimer;
    ApplController*         m_applController;
    ProcessBar*             m_processBar;
    PayloadLib*             m_payloadLib;
    InfoPage*               m_infoPage;
    quint8                  m_currentElement;
    NavigationControl*      m_elementList[MAX_FRGT_PWD_CTRL];
    QString                 m_username;
    QString                 m_currentDeviceName;
    PasswordReset*          m_passwordReset;

public:
    explicit ForgotPassword(QString deviceName, QString username, PASSWORD_RESET_INFO_t &pwdRstInfo, QWidget *parent = 0);
    ~ForgotPassword();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void sendCommand(PWD_RST_CMD_e cmdType, int totalfeilds);

signals:
    void sigExitPage(void);

public slots:
    void slotExitPwdRstPage(void);
    void slotSpinBoxValueChange(QString, quint32);
    void slotUpdateCurrentElement(int indexInPage);
    void slotUpdateVerifyOtpTime(void);
    void slotTextLableHover(int indexInPage = FRGT_PWD_RESEND_OTP_BTN, bool isHoverIn = false);
    void slotConfigButtonClick(int indexInPage);
    void slotcloseButtonClick(int);
    void slotInfoPageCnfgBtnClick(int);
};

#endif // FORGOTPASSWORD_H
