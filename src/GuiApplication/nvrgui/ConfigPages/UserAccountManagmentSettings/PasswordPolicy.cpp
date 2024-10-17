#include "PasswordPolicy.h"
#include "ValidationMessage.h"

typedef enum{
    PASS_POLICY_CFG_MIN_PASS_LEN,
    PASS_POLICY_CFG_PASS_VAL,
    PASS_POLICY_CFG_RST_PER,
    PASS_POLICY_CFG_LCK_ATM,
    PASS_POLICY_CFG_MAX_LOG_ATM,
    PASS_POLICY_CFG_UNLCK_TIMER,

    MAX_PASS_POLICY_TABLE_FEILDS
}PASS_POLICY_TABLE_FEILDS_e;

typedef enum{

    PASS_POLICY_PASS_LEN_TEXTBOX,
    PASS_POLICY_PASS_VAL_CHECKBOX,
    PASS_POLICY_RST_PASS_TEXTBOX,
    PASS_POLICY_LOGOUT_POLICY_CHECKBOX,
    PASS_POLICY_LOGIN_ATTEM_ALLOW_TEXTBOX,
    PASS_POLICY_LOCK_ACC_TEXTBOX,

    MAX_PASS_POLICY_CONTROL
}PASS_POLICY_CONTROL_e;

static const QStringList passwordPolicyStr = QStringList()
        << "Password Strength"
        << "Minimum Password Length"
        << "Password Validity"
        << "Reset Password After"
        << "Account Lockout Policy"
        << "Maximum Failed Attempts Allowed"
        << "Lock Account for" ;

PasswordPolicy::PasswordPolicy(QString devName, QWidget *parent,
                               DEV_TABLE_INFO_t *tableInfo)
    :ConfigPageControl(devName, parent, MAX_PASS_POLICY_CONTROL,
                       tableInfo, CNFG_TYPE_DFLT_REF_SAV_BTN)

{
    createDefaultComponents();
    PasswordPolicy::getConfig();
}

PasswordPolicy::~PasswordPolicy()
{
    // Password Strength
    delete m_PasswordStrengthHeading;

    delete m_minPassTextBox;
    delete m_minPassLenParam;

    // Password validity
    disconnect(m_setPassValidityCheckBox,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    delete m_setPassValidityCheckBox;

    delete m_resetPassTextBox;
    delete m_resetPassParam;

    // Account Logout Policy
    disconnect(m_logoutPolicyCheckBox,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    delete m_logoutPolicyCheckBox;

    delete m_loginAttemTextBox;
    delete m_loginAttemParam;

    delete m_lockAccountTextBox;
    delete m_lockAccountParam;
}

void PasswordPolicy::createDefaultComponents()
{
    // Password Strength
    m_PasswordStrengthHeading = new ElementHeading(SCALE_WIDTH(110),
                                                   SCALE_HEIGHT(60),
                                                   (BGTILE_ULTRAMEDIUM_SIZE_WIDTH + SCALE_WIDTH(80)),
                                                   BGTILE_HEIGHT,
                                                   passwordPolicyStr.at(0),
                                                   TOP_LAYER,
                                                   this,
                                                   false,
                                                   SCALE_WIDTH(10), NORMAL_FONT_SIZE, true);
    m_minPassLenParam = new TextboxParam();
    m_minPassLenParam->labelStr = passwordPolicyStr.at(1);
	m_minPassLenParam->suffixStr = Multilang("characters") + QString(" (5-16)");
    m_minPassLenParam->isNumEntry = true;
    m_minPassLenParam->isCentre = true;
	m_minPassLenParam->minNumValue = 5;
    m_minPassLenParam->maxNumValue = 16;
    m_minPassLenParam->maxChar = 2;
	m_minPassLenParam->textStr = "5";
    m_minPassLenParam->validation = QRegExp(QString("[0-9]"));

    m_minPassTextBox = new TextBox(m_PasswordStrengthHeading->x(),
                                   (m_PasswordStrengthHeading->y() +
                                    m_PasswordStrengthHeading->height()),
                                   m_PasswordStrengthHeading->width(),
                                   m_PasswordStrengthHeading->height(),
                                   PASS_POLICY_PASS_LEN_TEXTBOX,
                                   TEXTBOX_SMALL,
                                   this,
                                   m_minPassLenParam);

    m_elementList[PASS_POLICY_PASS_LEN_TEXTBOX] = m_minPassTextBox;

    // Password validity
	m_setPassValidityCheckBox = new OptionSelectButton(m_minPassTextBox->x(),
													   (m_minPassTextBox->y() +
														m_minPassTextBox->height() + SCALE_HEIGHT(10)),
													   m_minPassTextBox->width(),
													   m_minPassTextBox->height(),
                                                       CHECK_BUTTON_INDEX,
													   passwordPolicyStr.at(2),
                                                       this,
                                                       MIDDLE_LAYER,
                                                       SCALE_WIDTH(10),
                                                       MX_OPTION_TEXT_TYPE_SUFFIX,
                                                       NORMAL_FONT_SIZE,
                                                       PASS_POLICY_PASS_VAL_CHECKBOX,
                                                       true,
                                                       HIGHLITED_FONT_COLOR, true);
    m_elementList[PASS_POLICY_PASS_VAL_CHECKBOX] = m_setPassValidityCheckBox;

    connect(m_setPassValidityCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

    m_resetPassParam = new TextboxParam();
	m_resetPassParam->labelStr = passwordPolicyStr.at(3);
    m_resetPassParam->suffixStr = Multilang("day(s)") + QString(" (1-365)");
    m_resetPassParam->isNumEntry = true;
    m_resetPassParam->isCentre = true;
    m_resetPassParam->minNumValue = 1;
    m_resetPassParam->maxNumValue = 365;
    m_resetPassParam->maxChar = 3;
    m_resetPassParam->textStr = "90";
    m_resetPassParam->validation = QRegExp(QString("[0-9]"));

    m_resetPassTextBox = new TextBox(m_setPassValidityCheckBox->x(),
                                     (m_setPassValidityCheckBox->y() +
                                      m_setPassValidityCheckBox->height()),
                                     m_setPassValidityCheckBox->width(),
                                     m_setPassValidityCheckBox->height(),
                                     PASS_POLICY_RST_PASS_TEXTBOX,
                                     TEXTBOX_SMALL,
                                     this,
                                     m_resetPassParam);

    m_elementList[PASS_POLICY_RST_PASS_TEXTBOX] = m_resetPassTextBox;
    // Account Logout Policy
    m_logoutPolicyCheckBox = new OptionSelectButton(m_resetPassTextBox->x(),
                                                    (m_resetPassTextBox->y() +
                                                     m_resetPassTextBox->height() + SCALE_HEIGHT(10)),
                                                    m_resetPassTextBox->width(),
                                                    m_resetPassTextBox->height(),
                                                    CHECK_BUTTON_INDEX,
													passwordPolicyStr.at(4),
                                                    this,
                                                    MIDDLE_LAYER,
                                                    SCALE_WIDTH(10),
                                                    MX_OPTION_TEXT_TYPE_SUFFIX,
                                                    NORMAL_FONT_SIZE,
                                                    PASS_POLICY_LOGOUT_POLICY_CHECKBOX,
                                                    true,
                                                    HIGHLITED_FONT_COLOR, true);
    m_elementList[PASS_POLICY_LOGOUT_POLICY_CHECKBOX] = m_logoutPolicyCheckBox;

    connect(m_logoutPolicyCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));


    m_loginAttemParam = new TextboxParam();
	m_loginAttemParam->labelStr = passwordPolicyStr.at(5);
    m_loginAttemParam->suffixStr = "(1-20)";
    m_loginAttemParam->isNumEntry = true;
    m_loginAttemParam->isCentre = true;
    m_loginAttemParam->minNumValue = 1;
    m_loginAttemParam->maxNumValue = 20;
    m_loginAttemParam->maxChar = 2;
    m_loginAttemParam->textStr = "5";
    m_loginAttemParam->validation = QRegExp(QString("[0-9]"));

    m_loginAttemTextBox = new TextBox(m_logoutPolicyCheckBox->x(),
                                      (m_logoutPolicyCheckBox->y() +
                                       m_logoutPolicyCheckBox->height()),
                                      m_logoutPolicyCheckBox->width(),
                                      m_logoutPolicyCheckBox->height(),
                                      PASS_POLICY_LOGIN_ATTEM_ALLOW_TEXTBOX,
                                      TEXTBOX_SMALL,
                                      this,
                                      m_loginAttemParam);
    m_elementList[PASS_POLICY_LOGIN_ATTEM_ALLOW_TEXTBOX] = m_loginAttemTextBox;

    m_lockAccountParam = new TextboxParam();
	m_lockAccountParam->labelStr = passwordPolicyStr.at(6);
    m_lockAccountParam->suffixStr = Multilang("minute(s)") + QString(" (1-999)");
    m_lockAccountParam->isNumEntry = true;
    m_lockAccountParam->isCentre = true;
    m_lockAccountParam->minNumValue = 1;
    m_lockAccountParam->maxNumValue = 999;
    m_lockAccountParam->maxChar = 3;
    m_lockAccountParam->textStr = "5";
    m_lockAccountParam->validation = QRegExp(QString("[0-9]"));

    m_lockAccountTextBox = new TextBox(m_loginAttemTextBox->x(),
                                       (m_loginAttemTextBox->y() +
                                        m_loginAttemTextBox->height()),
                                       m_loginAttemTextBox->width(),
                                       m_loginAttemTextBox->height(),
                                       PASS_POLICY_LOCK_ACC_TEXTBOX,
                                       TEXTBOX_SMALL,
                                       this,
                                       m_lockAccountParam);
    m_elementList[PASS_POLICY_LOCK_ACC_TEXTBOX] = m_lockAccountTextBox;
}

void PasswordPolicy::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString =
            payloadLib->createDevCnfgPayload(msgType,
                                             PASSWORD_POLICY_SETTING_TABLE_INDEX,
                                             CNFG_FRM_INDEX,
                                             CNFG_FRM_INDEX,
                                             CNFG_FRM_INDEX,
                                             MAX_PASS_POLICY_TABLE_FEILDS,
                                             MAX_PASS_POLICY_TABLE_FEILDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void PasswordPolicy::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
            case MSG_GET_CFG:
            {
                payloadLib->parsePayload (param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == PASSWORD_POLICY_SETTING_TABLE_INDEX)
                {
                    QString resStr = "";
                    quint8 resInt = 0;

                    resStr =  payloadLib->getCnfgArrayAtIndex (PASS_POLICY_CFG_MIN_PASS_LEN).toString();
                    m_minPassTextBox->setInputText(resStr);

                    resInt =  payloadLib->getCnfgArrayAtIndex (PASS_POLICY_CFG_PASS_VAL).toUInt();
                    m_setPassValidityCheckBox->changeState((resInt == 1) ? ON_STATE: OFF_STATE);
                    m_resetPassTextBox->setIsEnabled((resInt == 1));

                    resStr =  payloadLib->getCnfgArrayAtIndex (PASS_POLICY_CFG_RST_PER).toString();
                    m_resetPassTextBox->setInputText(resStr);

                    resInt =  payloadLib->getCnfgArrayAtIndex (PASS_POLICY_CFG_LCK_ATM).toUInt();
                    m_logoutPolicyCheckBox->changeState((resInt == 1) ? ON_STATE: OFF_STATE);
                    m_loginAttemTextBox->setIsEnabled((resInt == 1));
                    m_lockAccountTextBox->setIsEnabled((resInt == 1));

                    resStr =  payloadLib->getCnfgArrayAtIndex (PASS_POLICY_CFG_MAX_LOG_ATM).toString();
                    m_loginAttemTextBox->setInputText(resStr);

                    resStr =  payloadLib->getCnfgArrayAtIndex (PASS_POLICY_CFG_UNLCK_TIMER).toString();
                    m_lockAccountTextBox->setInputText(resStr);
                }
            }
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            case MSG_SET_CFG:
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            }
                break;

            default:
                break;
            }
        }
            break;

        default:
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
}

void PasswordPolicy::getConfig()
{
    createPayload(MSG_GET_CFG);
}

void PasswordPolicy::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void PasswordPolicy::saveConfig()
{
    if ((IS_VALID_OBJ(m_minPassTextBox)) && (IS_VALID_OBJ(m_resetPassTextBox))
         && (IS_VALID_OBJ(m_lockAccountTextBox)) && (IS_VALID_OBJ(m_loginAttemTextBox)))
    {
        if ((!m_minPassTextBox->doneKeyValidation())
                || (!m_resetPassTextBox->doneKeyValidation())
                || (!m_lockAccountTextBox->doneKeyValidation())
                || (!m_loginAttemTextBox->doneKeyValidation()))
        {
            return;
        }
    }

    payloadLib->setCnfgArrayAtIndex (PASS_POLICY_CFG_MIN_PASS_LEN,
                                     m_minPassTextBox->getInputText());

    payloadLib->setCnfgArrayAtIndex (PASS_POLICY_CFG_PASS_VAL,
                                     (m_setPassValidityCheckBox->getCurrentState() == ON_STATE) ? 1 : 0);

    payloadLib->setCnfgArrayAtIndex (PASS_POLICY_CFG_RST_PER,
                                     m_resetPassTextBox->getInputText());

    payloadLib->setCnfgArrayAtIndex (PASS_POLICY_CFG_LCK_ATM,
                                     (m_logoutPolicyCheckBox->getCurrentState() == ON_STATE) ? 1 : 0);

    payloadLib->setCnfgArrayAtIndex (PASS_POLICY_CFG_MAX_LOG_ATM,
                                     m_loginAttemTextBox->getInputText());

    payloadLib->setCnfgArrayAtIndex (PASS_POLICY_CFG_UNLCK_TIMER,
                                     m_lockAccountTextBox->getInputText());

    createPayload(MSG_SET_CFG);
}

void PasswordPolicy::slotCheckBoxClicked(OPTION_STATE_TYPE_e state, int index)
{
    switch (index)
    {
    case PASS_POLICY_LOGOUT_POLICY_CHECKBOX:
        m_loginAttemTextBox->setIsEnabled((state == ON_STATE));
        m_lockAccountTextBox->setIsEnabled((state == ON_STATE));
        break;

    case PASS_POLICY_PASS_VAL_CHECKBOX:
        m_resetPassTextBox->setIsEnabled((state == ON_STATE));
        break;

    default:
        break;
    }
}
