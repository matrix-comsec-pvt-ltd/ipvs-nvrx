#ifndef MANAGEPASSWORDRECOVERY_H
#define MANAGEPASSWORDRECOVERY_H

#include "ManageMenuOptions.h"
#include "UserValidation.h"
#include "Controls/ControlButton.h"
#include "Controls/DropDown.h"
#include "Controls/TextBox.h"

#define MAX_SEC_QUESTIONS               9

/* enum for Password Recovery */
typedef enum
{
    PASSWORD_RECOVERY_QA_1 = 0,
    PASSWORD_RECOVERY_QA_2,
    PASSWORD_RECOVERY_QA_3,

    PASSWORD_RECOVERY_QA_MAX,
}PASSWORD_RECOVERY_QA_e;

/* enum for config fields */
typedef enum
{
    EMAIL_ID_FIELD = 0,
    SECURITY_QUESTION_1_FIELD,
    SECURITY_ANSWER_1_FIELD,
    SECURITY_QUESTION_2_FIELD,
    SECURITY_ANSWER_2_FIELD,
    SECURITY_QUESTION_3_FIELD,
    SECURITY_ANSWER_3_FIELD,

    PWD_RECOVERY_FIELD_MAX
}PWD_RECOVERY_FIELDS_e;

typedef enum
{
    EMAIL_ID = 0,
    TEST_BUTTON,
    CLOSE_BUTTON = TEST_BUTTON,
    SECURITY_QUESTION_1,
    SECURITY_ANSWER_1,
    SECURITY_QUESTION_2,
    SECURITY_ANSWER_2,
    SECURITY_QUESTION_3,
    SECURITY_ANSWER_3,
    OK_BUTTON,
    CANCEL_BUTTON,

    PWD_RECOVERY_STRINGS_MAX
}PWD_RECOVERY_STRING_e;

static const QString passwordRecoveryStrings[PWD_RECOVERY_STRINGS_MAX]={"Email ID",
                                                                        "Test",
                                                                        "Security Question",
                                                                        "Answer",
                                                                        "Security Question",
                                                                        "Answer",
                                                                        "Security Question",
                                                                        "Answer",
                                                                        "OK",
                                                                        "Cancel"};

static const QString securityQuestionStrings[MAX_SEC_QUESTIONS] = {"Which is your favourite movie?",
                                                                   "What is your favourite number?",
                                                                   "Who was your favourite childhood hero?",
                                                                   "What is your favourite cuisine?",
                                                                   "What is your favourite adventure sports?",
                                                                   "What was your last birthday gift?",
                                                                   "What is your favourite song?",
                                                                   "What primary school did you attend?",
                                                                   "What was your childhood nickname?"};

class ManagePasswordRecovery : public ManageMenuOptions
{
    Q_OBJECT

private:
    DropDown*               m_secQuestionDropDown[PASSWORD_RECOVERY_QA_MAX];
    TextboxParam*           m_answerTextboxParam[PASSWORD_RECOVERY_QA_MAX];
    TextBox*                m_answerTextBox[PASSWORD_RECOVERY_QA_MAX];
    TextboxParam*           m_emailIdTextboxParam;
    TextBox*                m_emailIdTextBox;
    ControlButton*          m_testButton;
    CnfgButton*             m_okButton;
    CnfgButton*             m_cancelButton;

public:
    explicit ManagePasswordRecovery(QString devName, QWidget *parent = 0);
    ~ManagePasswordRecovery();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds);
    void updateList(PASSWORD_RECOVERY_QA_e dropDownIndex_1, PASSWORD_RECOVERY_QA_e dropDownIndex_2, QMap<quint8, QString> &securityQuestionsList);
    void updateSecurityQuestionDropdownList(PASSWORD_RECOVERY_QA_e dropDownIndex, QMap<quint8, QString> securityQuestionsList);
    void getConfig(void);
    void saveConfig(void);
    void displayPwdRecoveryConfig(DevCommParam *param);

signals:
    void sigCancelbuttonClick(void);

public slots:
    void slotTextBoxInfoPage(int index, INFO_MSG_TYPE_e msgType);
    void slotTestBtnClick(int);
    void slotSpinBoxValueChange(QString, quint32 indexInPage);
    void slotConfigButtonClick(int index);
};

#endif // MANAGEPASSWORDRECOVERY_H
