#include "DeviceSetting.h"
#include "ValidationMessage.h"
#include "ConfigField.h"

#define MAX_DEVICE_SETTING_ELEMETS      (10)
#define DEVICE_SETTINGS_TOP_MARGIN      (10)
#define ELEMENT_HEADING_OFFSET          SCALE_HEIGHT(3)
#define LEFT_MARGIN_DEVICE_NAME_LABEL   (0)
#define LEFT_MARGIN_FROM_CENTER         SCALE_WIDTH(50)
#define BGTILE_LARGE_SIZE_WIDTH_NW      SCALE_WIDTH(750)

#define DEV_NAME_STRING                 "Device"
#define REG_MODE_NAME_STRING            "Register Mode"
#define ADD_EDIT_HEADING_STR            "Add/Edit Device"
#define ENABLE_DEV_STRING               "Enable Device"
#define DEVICE_CREDENTIALS              "Prefer Native Device Credentials"
#define AUTO_LOGIN                      "Auto Login"
#define FORWARDED_PORT_STRING           "Forwarded TCP Port"
#define FORWARDED_PORT_SUFFIX_STRING    "(1024-65535)"

#define DEVICE_SETTING_FROM_FIELD       1
#define DEVICE_SETTING_MAX_FIELD        11

typedef enum
{
    DEVICE_SETTINGS_DEVICENAME_SPINBOX = 0,
    DEVICE_SETTINGS_DEVICE_ADD_BUTTON,
    DEVICE_SETTINGS_DEVICE_EDIT_BUTTON,
    DEVICE_SETTINGS_DEVICE_DELET_BUTTON,
    DEVICE_SETTING_DEVICE_MODEL_TEXTBOX,
    DEVICE_SETTINGS_DEVICE_NAME_TEXTBOX,
    DEVICE_SETTINGS_ENBL_DEV_CHECKBOX,
    DEVICE_SETTINGS_REGISTER_MODE_SPINBOX,
    DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX,
    DEVICE_SETTINGS_DDNS_TEXTBOX,
    DEVICE_SETTINGS_MATRIX_DNS_MAC_TEXTBOX,
    DEVICE_SETTINGS_MATRIX_DNS_HOSTNAME_TEXTBOX,
    DEVICE_SETTINGS_PORT_NO_TEXTBOX,
    DEVICE_SETTINGS_FORWARDED_PORT_NO_TEXTBOX,
    DEVICE_SETTINGS_USERNAME_TEXTBOX,
    DEVICE_SETTINGS_PASSWORD_TEXTBOX,
    DEVICE_SETTINGS_PREFER_NATIVE_DEV_CREDENTIAL_CHECKBOX,
    DEVICE_SETTINGS_AUTO_LOGIN_CHECKBOX,
    DEVICE_SETTINGS_MAIN_RADIO_BUTTON,
    DEVICE_SETTINGS_SUB_RADIO_BUTTON,
    DEVICE_SETTINGS_OPTIMIZED_RADIO_BUTTON,
    MAX_DEVICE_SETTINGS_ELEMENT

}DEVICE_SETTINGS_ELEMENT_e;

static const CONTROL_BUTTON_TYPE_e buttonType[MAX_CONTROL_BUTTONS] =
{
    ADD_BUTTON_INDEX,
    EDIT_BUTTON_INDEX,
    DELETE_BUTTON_INDEX
};

static const QString buttonName[MAX_CONTROL_BUTTONS] =
{
    "Add",
    "Edit",
    "Delete"
};

static const QString cnfgButtonName[] = {
    "Save" ,
    "Cancel"
};

static const QMap<quint8, QString> regModeMapList =
{
    {0, "IP Address"},
    {1, "DDNS"},
    #if !defined(OEM_JCI)
    {2, "Matrix DNS - MAC Address"},
    {3, "Matrix DNS - Host name"},
    #endif
};

static const QString regModeLabel[4] =
{
    "IP Address",
    "Host Name",
    "MAC Address",
    "Host Name"
};

static const QString textLabel[MAX_CONTROL_BUTTONS] =
{
    "Port",
    "Username",
    "Password"
};

static const QString suffixLabel[MAX_CONTROL_BUTTONS] =
{
    "(1024-65535)",
    "",
    ""
};

static const bool numEntry[MAX_CONTROL_BUTTONS] =
{
    true,
    false,
    false
};

static const quint32 minVal[MAX_CONTROL_BUTTONS] =
{
    1024,
    0,
    0
};

static const quint32 maxVal[MAX_CONTROL_BUTTONS] =
{
    65535,
    0,
    0
};

static const quint32 maxChar[MAX_CONTROL_BUTTONS] =
{
    5,
    24,
    16
};

static const quint32 minChar[MAX_CONTROL_BUTTONS] =
{
    0,
    0,
    4
};

static const QString validationStr[MAX_CONTROL_BUTTONS] =
{
    QString("[0-9]"),
    asciiset1ValidationStringWithoutSpace,
    asciiset1ValidationStringWithoutSpace
};

static const TEXTBOX_SIZE_e sizeOfBox[MAX_CONTROL_BUTTONS] =
{
    TEXTBOX_SMALL,
    TEXTBOX_LARGE,
    TEXTBOX_SMALL
};

static const quint16 buttonMargin[MAX_CONTROL_BUTTONS] =
{
    390, 480, 565
};

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
DeviceSetting::DeviceSetting(QString devName, QWidget *parent)
    :ConfigPageControl(devName,
                       parent,
                       MAX_DEVICE_SETTINGS_ELEMENT,
                       NULL,
                       CNFG_TYPE_DFLT_REF_SAV_BTN), m_deviceCount(0)
{
    enabled = false;
    m_applController = ApplController::getInstance();

    m_controlBtnIndex = MAX_DEVICE_SETTINGS_ELEMENT;
    m_currentDeviceIndex = 1;
    m_LocalDeviceName = "";

    isInitDone = false;
    getconfigCall = false;
    createDefaultElements ();

    DeviceSetting::getConfig ();
}

DeviceSetting::~DeviceSetting ()
{
    disconnect(m_deviceNameDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotDropDownBoxValueChanged(QString,quint32)));

    disconnect(m_deviceNameDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_deviceNameDropDownBox;
    m_deviceList.clear ();

    for(quint8 index = 0; index < MAX_CONTROL_BUTTONS; index ++)
    {
        disconnect(m_controlButton[index],
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotControlButtonClicked(int)));

        disconnect(m_controlButton[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_controlButton[index];
    }

    delete m_addEditHeading;

    disconnect (m_deviceName,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    disconnect(m_deviceName,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_deviceName;
    delete m_deviceNameParam;


    disconnect(m_deviceEnableBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_deviceEnableBox;

    disconnect(m_registerMode,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotDropDownBoxValueChanged(QString,quint32)));

    disconnect(m_registerMode,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_registerMode;

    disconnect (m_ipAddressTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_ipAddressTextbox;

    disconnect (m_DDNSHostName,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_DDNSHostName;
    delete m_DDNSHostNameParam;

    disconnect (m_macAddressBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_macAddressBox;

    disconnect (m_matrixDDNSTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (m_matrixDDNSTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_matrixDDNSTextBox;
    delete m_matrixDDNSParam;

    for(quint8 index = 0; index < (MAX_CONTROL_BUTTONS -1); index ++)
    {
        disconnect (m_textbox[index],
                    SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                    this,
                    SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

        disconnect(m_textbox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_textbox[index];
        delete m_textboxParam[index];
    }

    disconnect(m_forwardedTcpPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect(m_forwardedTcpPortTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    delete m_forwardedTcpPortTextBox;
    delete m_forwardedTcpPortTextBoxParam;

    disconnect (m_passwordBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    disconnect(m_passwordBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_passwordBox;
    delete m_textboxParam[2];

    disconnect(m_preferDevCredential,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_preferDevCredential;


    disconnect(m_autoLogin,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_autoLogin;

    disconnect(m_mainRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(m_mainRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_mainRadioButton;

    disconnect(m_subRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(m_subRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_subRadioButton;

    disconnect(m_optimizedRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(m_optimizedRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_optimizedRadioButton;

    if(m_deviceModelName != NULL)
    {
        disconnect(m_deviceModelName,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        disconnect(m_deviceModelName,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_deviceModelName);
    }
    if(m_deviceModel != NULL)
    {
        DELETE_OBJ(m_deviceModel);
    }
}

void DeviceSetting::createDefaultElements ()
{
    QMap<quint8, QString>  deviceList;
    deviceList.clear ();

    m_deviceList.append ("");
    for(quint8 index = 0; index < m_deviceList.length (); index++)
    {
        deviceList.insert (index,m_deviceList.at (index));
    }

    m_deviceNameDropDownBox = new DropDown(((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_LARGE_SIZE_WIDTH_NW) / 2),
                                           SCALE_HEIGHT(DEVICE_SETTINGS_TOP_MARGIN),
                                           BGTILE_LARGE_SIZE_WIDTH_NW,
                                           BGTILE_HEIGHT,
                                           DEVICE_SETTINGS_DEVICENAME_SPINBOX,
                                           DROPDOWNBOX_SIZE_225,
                                           DEV_NAME_STRING,
                                           deviceList,
                                           this,
                                           "",
                                           false,
                                           SCALE_WIDTH(76),
                                           TOP_LAYER);

    m_elementList[DEVICE_SETTINGS_DEVICENAME_SPINBOX] = m_deviceNameDropDownBox;

    connect(m_deviceNameDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotDropDownBoxValueChanged(QString,quint32)));

    connect(m_deviceNameDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < MAX_CONTROL_BUTTONS; index ++)
    {
        m_controlButton[index] = new ControlButton(buttonType[index],
                                                   m_deviceNameDropDownBox->x() + SCALE_WIDTH(buttonMargin[index]),
                                                   m_deviceNameDropDownBox->y(),
                                                   BGTILE_LARGE_SIZE_WIDTH_NW,
                                                   BGTILE_HEIGHT,
                                                   this,
                                                   NO_LAYER,
                                                   SCALE_WIDTH(360),
                                                   buttonName[index],
                                                   true,
                                                   index + DEVICE_SETTINGS_DEVICE_ADD_BUTTON);

        m_elementList[index + DEVICE_SETTINGS_DEVICE_ADD_BUTTON] = m_controlButton[index];

        connect(m_controlButton[index],
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlButtonClicked(int)));


        connect(m_controlButton[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }
    m_deviceModel = NULL;
    m_deviceModel = new TextboxParam();

    m_deviceModel->labelStr = "Device Model";
    m_deviceModel->isCentre = false;
    m_deviceModel->leftMargin = SCALE_WIDTH(19);
    m_deviceModel->maxChar = 16;
    m_deviceModelName = NULL;
    m_deviceModelName = new TextBox(m_deviceNameDropDownBox->x(),
                                    m_deviceNameDropDownBox->y() + m_deviceNameDropDownBox->height(),
                                    BGTILE_LARGE_SIZE_WIDTH_NW,
                                    BGTILE_HEIGHT,
                                    DEVICE_SETTING_DEVICE_MODEL_TEXTBOX,
                                    TEXTBOX_LARGE,
                                    this,
                                    m_deviceModel,
                                    TOP_LAYER,
                                    false);

    m_elementList[DEVICE_SETTING_DEVICE_MODEL_TEXTBOX] = m_deviceModelName;

    connect(m_deviceModelName,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect(m_deviceModelName,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_addEditHeading = new ElementHeading(m_deviceNameDropDownBox->x(),
                                         (m_deviceModelName->y() +
                                          m_deviceModelName->height() + ELEMENT_HEADING_OFFSET),
                                          BGTILE_LARGE_SIZE_WIDTH_NW,
                                          BGTILE_HEIGHT,
                                          ADD_EDIT_HEADING_STR,
                                          TOP_LAYER,
                                          this,
                                          false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    m_deviceNameParam = new TextboxParam();

    m_deviceNameParam->labelStr = "Device Name";
    m_deviceNameParam->maxChar = 16;

    m_deviceName = new TextBox(m_deviceNameDropDownBox->x(),
                               m_addEditHeading->y() + m_addEditHeading->height(),
                               BGTILE_LARGE_SIZE_WIDTH_NW,
                               BGTILE_HEIGHT,
                               DEVICE_SETTINGS_DEVICE_NAME_TEXTBOX,
                               TEXTBOX_LARGE,
                               this,
                               m_deviceNameParam,
                               MIDDLE_TABLE_LAYER,
                               false, false, false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_DEVICE_NAME_TEXTBOX] = m_deviceName;
    connect(m_deviceName,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect(m_deviceName,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_deviceEnableBox = new OptionSelectButton(m_deviceNameDropDownBox->x(),
                                               m_deviceName->y() + m_deviceName->height(),
                                               BGTILE_LARGE_SIZE_WIDTH_NW,
                                               BGTILE_HEIGHT,
                                               CHECK_BUTTON_INDEX,
                                               this,
                                               MIDDLE_TABLE_LAYER,
                                               ENABLE_DEV_STRING,
                                               "",
                                               -1,
                                               DEVICE_SETTINGS_ENBL_DEV_CHECKBOX,
                                               true, -1, SUFFIX_FONT_COLOR, false,
											   LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_ENBL_DEV_CHECKBOX] = m_deviceEnableBox;
    connect(m_deviceEnableBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_registerMode = new DropDown(m_deviceNameDropDownBox->x(),
                                  m_deviceEnableBox->y() + m_deviceEnableBox->height(),
                                  BGTILE_LARGE_SIZE_WIDTH_NW,
                                  BGTILE_HEIGHT,
                                  DEVICE_SETTINGS_REGISTER_MODE_SPINBOX,
                                  DROPDOWNBOX_SIZE_320,
                                  REG_MODE_NAME_STRING,
                                  regModeMapList,
                                  this,
                                  "",
                                  true,
                                  LEFT_MARGIN_DEVICE_NAME_LABEL,
                                  MIDDLE_TABLE_LAYER,
                                  false, 8, false, false, 5,
								  LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_REGISTER_MODE_SPINBOX] = m_registerMode;
    connect(m_registerMode,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotDropDownBoxValueChanged(QString,quint32)));
    connect(m_registerMode,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_ipAddressTextbox = new IpTextBox(m_deviceNameDropDownBox->x(),
                                       m_registerMode->y() + m_registerMode->height(),
                                       BGTILE_LARGE_SIZE_WIDTH_NW,
                                       BGTILE_HEIGHT,
                                       DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX,
                                       regModeLabel[BY_IP_ADDRESS],
                                       IP_ADDR_TYPE_IPV4_AND_IPV6,
                                       this,
                                       MIDDLE_TABLE_LAYER,
                                       true, 0, false, IP_FIELD_TYPE_IPV6_ADDR,
                                       IP_TEXTBOX_ULTRALARGE,
                                       LEFT_MARGIN_FROM_CENTER);

    m_elementList[DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX] = m_ipAddressTextbox;
    connect(m_ipAddressTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_DDNSHostNameParam = new TextboxParam();
    m_DDNSHostNameParam->labelStr = regModeLabel[BY_DDNS];
    m_DDNSHostNameParam->maxChar = 40;
    m_DDNSHostNameParam->validation = QRegExp(validationStr[2]);
    m_DDNSHostName = new TextBox(m_deviceNameDropDownBox->x(),
                                 m_registerMode->y() + m_registerMode->height(),
                                 BGTILE_LARGE_SIZE_WIDTH_NW,
                                 BGTILE_HEIGHT,
                                 DEVICE_SETTINGS_DDNS_TEXTBOX,
                                 TEXTBOX_MEDIAM,
                                 this,
                                 m_DDNSHostNameParam,
                                 MIDDLE_TABLE_LAYER,
                                 false, false, false,
                                 LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_DDNS_TEXTBOX] = m_DDNSHostName;
    connect(m_DDNSHostName,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_DDNSHostName->setVisible(false);

    m_macAddressBox =  new MacTextBox(m_deviceNameDropDownBox->x(),
                                      m_registerMode->y() + m_registerMode->height(),
                                      BGTILE_LARGE_SIZE_WIDTH_NW,
                                      BGTILE_HEIGHT,
                                      DEVICE_SETTINGS_MATRIX_DNS_MAC_TEXTBOX,
                                      regModeLabel[BY_MATRIX_MAC],
                                      this,
                                      MIDDLE_TABLE_LAYER,
                                      true, 0, false,
									  LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_MATRIX_DNS_MAC_TEXTBOX] = m_macAddressBox;
    connect(m_macAddressBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_macAddressBox->setVisible(false);

    m_matrixDDNSParam = new TextboxParam();
    m_matrixDDNSParam->labelStr = regModeLabel[BY_MATRIX_HOSTNAME];
    m_matrixDDNSParam->maxChar = 30;
    m_matrixDDNSParam->minChar = 3;
    m_matrixDDNSParam->validation = QRegExp(QString("[a-zA-Z0-9_ ]"));
    m_matrixDDNSParam->startCharVal = QRegExp(QString("[a-zA-Z]"));
    m_matrixDDNSTextBox = new TextBox(m_deviceNameDropDownBox->x(),
                                      m_registerMode->y() + m_registerMode->height(),
                                      BGTILE_LARGE_SIZE_WIDTH_NW,
                                      BGTILE_HEIGHT,
                                      DEVICE_SETTINGS_MATRIX_DNS_HOSTNAME_TEXTBOX,
                                      TEXTBOX_MEDIAM,
                                      this,
                                      m_matrixDDNSParam,
                                      MIDDLE_TABLE_LAYER,
                                      false, false, false,
                                      LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_MATRIX_DNS_HOSTNAME_TEXTBOX] = m_matrixDDNSTextBox;
    connect(m_matrixDDNSTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_matrixDDNSTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_matrixDDNSTextBox->setVisible(false);

    m_textboxParam[0] = new TextboxParam();
    m_textboxParam[0]->labelStr = textLabel[0];
    m_textboxParam[0]->suffixStr = suffixLabel[0];
    m_textboxParam[0]->isNumEntry = numEntry[0];
    m_textboxParam[0]->minNumValue = minVal[0];
    m_textboxParam[0]->maxNumValue = maxVal[0];
    m_textboxParam[0]->maxChar = maxChar[0];
    m_textboxParam[0]->minChar = minChar[0];
    m_textboxParam[0]->validation = QRegExp(validationStr[0]);

    m_textbox[0] = new TextBox(m_deviceNameDropDownBox->x(),
                               m_ipAddressTextbox->y() + m_ipAddressTextbox->height(),
                               BGTILE_LARGE_SIZE_WIDTH_NW,
                               BGTILE_HEIGHT,
                               DEVICE_SETTINGS_PORT_NO_TEXTBOX,
                               sizeOfBox[0],
                               this,
                               m_textboxParam[0],
                               MIDDLE_TABLE_LAYER,
                               false, false, false,
                               LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_PORT_NO_TEXTBOX] = m_textbox[0];
    connect (m_textbox[0],
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect(m_textbox[0],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_forwardedTcpPortTextBoxParam = new TextboxParam();
    m_forwardedTcpPortTextBoxParam->labelStr = FORWARDED_PORT_STRING;
    m_forwardedTcpPortTextBoxParam->suffixStr = FORWARDED_PORT_SUFFIX_STRING;
    m_forwardedTcpPortTextBoxParam->isNumEntry = true;
    m_forwardedTcpPortTextBoxParam->minNumValue = 1024;
    m_forwardedTcpPortTextBoxParam->maxNumValue = 65535;
    m_forwardedTcpPortTextBoxParam->maxChar = 5;
    m_forwardedTcpPortTextBoxParam->minChar = 0;
    m_forwardedTcpPortTextBoxParam->validation = QRegExp(QString("[0-9]"));

    m_forwardedTcpPortTextBox = new TextBox(m_deviceNameDropDownBox->x(),
                                            m_textbox[0]->y() + m_textbox[0]->height(),
                                            BGTILE_LARGE_SIZE_WIDTH_NW,
                                            BGTILE_HEIGHT,
                                            DEVICE_SETTINGS_FORWARDED_PORT_NO_TEXTBOX,
                                            TEXTBOX_SMALL,
                                            this,
                                            m_forwardedTcpPortTextBoxParam,
                                            MIDDLE_TABLE_LAYER,
                                            false, false, false,
                                            LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_FORWARDED_PORT_NO_TEXTBOX] = m_forwardedTcpPortTextBox;
    connect (m_forwardedTcpPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect(m_forwardedTcpPortTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_textboxParam[1] = new TextboxParam();
    m_textboxParam[1]->labelStr = textLabel[1];
    m_textboxParam[1]->suffixStr = suffixLabel[1];
    m_textboxParam[1]->isNumEntry = numEntry[1];
    m_textboxParam[1]->minNumValue = minVal[1];
    m_textboxParam[1]->maxNumValue = maxVal[1];
    m_textboxParam[1]->maxChar = maxChar[1];
    m_textboxParam[1]->minChar = minChar[1];
    m_textboxParam[1]->validation = QRegExp(validationStr[1]);

    m_textbox[1] = new TextBox(m_deviceNameDropDownBox->x(),
                               m_forwardedTcpPortTextBox->y() + m_forwardedTcpPortTextBox->height(),
                               BGTILE_LARGE_SIZE_WIDTH_NW,
                               BGTILE_HEIGHT,
                               DEVICE_SETTINGS_USERNAME_TEXTBOX,
                               sizeOfBox[1],
                               this,
                               m_textboxParam[1],
                               MIDDLE_TABLE_LAYER,
                               false, false, false,
                               LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_USERNAME_TEXTBOX] = m_textbox[1];
    connect (m_textbox[1],
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect(m_textbox[1],
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_textboxParam[2] = new TextboxParam();

    m_textboxParam[2]->labelStr = textLabel[2];
    m_textboxParam[2]->suffixStr = suffixLabel[2];
    m_textboxParam[2]->isNumEntry = numEntry[2];
    m_textboxParam[2]->minNumValue = minVal[2];
    m_textboxParam[2]->maxNumValue = maxVal[2];
    m_textboxParam[2]->maxChar = maxChar[2];
    m_textboxParam[2]->minChar = minChar[2];
    m_textboxParam[2]->validation = QRegExp(validationStr[2]);

    m_passwordBox = new PasswordTextbox(m_deviceNameDropDownBox->x(),
                                        m_textbox[1]->y() +  m_textbox[1]->height(),
                                        BGTILE_LARGE_SIZE_WIDTH_NW,
                                        BGTILE_HEIGHT,
                                        DEVICE_SETTINGS_PASSWORD_TEXTBOX,
                                        sizeOfBox[1],
                                        this,
                                        m_textboxParam[2],
                                        MIDDLE_TABLE_LAYER,
                                        false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_PASSWORD_TEXTBOX] = m_passwordBox;
    connect(m_passwordBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect(m_passwordBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

   m_preferDevCredential = new OptionSelectButton(m_deviceNameDropDownBox->x(),
                                                  m_passwordBox->y() + m_passwordBox->height(),
                                                  BGTILE_LARGE_SIZE_WIDTH_NW,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  MIDDLE_TABLE_LAYER,
                                                  DEVICE_CREDENTIALS,
                                                  "",
                                                  -1,
                                                  DEVICE_SETTINGS_PREFER_NATIVE_DEV_CREDENTIAL_CHECKBOX,
                                                  false, -1, SUFFIX_FONT_COLOR, false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_PREFER_NATIVE_DEV_CREDENTIAL_CHECKBOX] = m_preferDevCredential;
    connect(m_preferDevCredential,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));


    m_autoLogin = new OptionSelectButton(m_deviceNameDropDownBox->x(),
                                         m_preferDevCredential->y() + m_preferDevCredential->height(),
                                         BGTILE_LARGE_SIZE_WIDTH_NW,
                                         BGTILE_HEIGHT,
                                         CHECK_BUTTON_INDEX,
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         AUTO_LOGIN,
                                         "",
                                         -1,
                                         DEVICE_SETTINGS_AUTO_LOGIN_CHECKBOX,
                                         false, -1, SUFFIX_FONT_COLOR, false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[DEVICE_SETTINGS_AUTO_LOGIN_CHECKBOX] = m_autoLogin;
    connect(m_autoLogin,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));


    m_mainRadioButton = new OptionSelectButton(m_autoLogin->x(),
                                               m_autoLogin->y() + m_autoLogin->height(),
                                               BGTILE_LARGE_SIZE_WIDTH_NW,
                                               BGTILE_HEIGHT,
                                               RADIO_BUTTON_INDEX,
                                               this,
                                               BOTTOM_TABLE_LAYER,
                                               "Live View Stream",
                                               "Main",
                                               -1,
                                               DEVICE_SETTINGS_MAIN_RADIO_BUTTON,
                                               false,
											   NORMAL_FONT_SIZE,
											   NORMAL_FONT_COLOR, false,
                                               LEFT_MARGIN_FROM_CENTER);
    m_mainRadioButton->changeState(ON_STATE);
    m_elementList[DEVICE_SETTINGS_MAIN_RADIO_BUTTON] = m_mainRadioButton;
    connect(m_mainRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_mainRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_subRadioButton = new OptionSelectButton((m_mainRadioButton->x() + SCALE_WIDTH(462) - LEFT_MARGIN_FROM_CENTER),
                                               m_mainRadioButton->y(),
                                               BGTILE_LARGE_SIZE_WIDTH_NW,
                                               BGTILE_HEIGHT,
                                               RADIO_BUTTON_INDEX,
                                               "Sub",
                                               this,
                                               NO_LAYER,
                                               -1,
                                               MX_OPTION_TEXT_TYPE_SUFFIX,
                                               NORMAL_FONT_SIZE,
                                               DEVICE_SETTINGS_SUB_RADIO_BUTTON,
                                               false);
    m_elementList[DEVICE_SETTINGS_SUB_RADIO_BUTTON] = m_subRadioButton;
    connect(m_subRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_subRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_optimizedRadioButton = new OptionSelectButton((m_subRadioButton->x() + m_subRadioButton->width() + SCALE_WIDTH(15)),
                                                    m_mainRadioButton->y(),
                                                    BGTILE_LARGE_SIZE_WIDTH_NW,
                                                    BGTILE_HEIGHT,
                                                    RADIO_BUTTON_INDEX,
                                                    "Optimized",
                                                    this,
                                                    NO_LAYER,
                                                    -1,
                                                    MX_OPTION_TEXT_TYPE_SUFFIX,
                                                    NORMAL_FONT_SIZE,
                                                    DEVICE_SETTINGS_OPTIMIZED_RADIO_BUTTON,
                                                    false);
    m_elementList[DEVICE_SETTINGS_OPTIMIZED_RADIO_BUTTON] = m_optimizedRadioButton;
    connect(m_optimizedRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_optimizedRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    resetGeometryOfCnfgbuttonRow(SCALE_HEIGHT(50));
}

void DeviceSetting::enableDisableAllControl(bool state)
{
    bool isAnyRDeviceConfigure = false;

    for(quint8 index = 0 ; index < m_deviceList.length (); index++)
    {
        if(m_deviceList.at (index) != "")
            isAnyRDeviceConfigure = true;
    }

    m_currentElement = (state == true) ? DEVICE_SETTINGS_DEVICE_NAME_TEXTBOX
                                       : (isAnyRDeviceConfigure == true) ?
                                             DEVICE_SETTINGS_DEVICENAME_SPINBOX
                                           : DEVICE_SETTINGS_DEVICE_ADD_BUTTON;

    if(!m_deviceList.contains (""))
    {
        m_controlButton[0]->setIsEnabled (false);
    }
    else
    {
        m_controlButton[0]->setIsEnabled (!state);
    }

    if(isAnyRDeviceConfigure)
    {
        m_controlButton[1]->setIsEnabled (!state);
        m_controlButton[2]->setIsEnabled (!state);
        m_deviceNameDropDownBox->setIsEnabled (!state);
        m_currentElement = DEVICE_SETTINGS_DEVICE_ADD_BUTTON;
    }
    else
    {
        m_deviceNameDropDownBox->setIsEnabled (false);
        m_controlButton[1]->setIsEnabled (false);
        m_controlButton[2]->setIsEnabled (false);
    }

    m_deviceName->setIsEnabled(state);
    m_deviceEnableBox->setIsEnabled(state);
    m_registerMode->setIsEnabled(state);
    m_ipAddressTextbox->setIsEnabled(state);
    m_macAddressBox->setIsEnabled (state);
    m_matrixDDNSTextBox->setIsEnabled (state);
    m_DDNSHostName->setIsEnabled (state);

    for(quint8 cnt = 0; cnt < 2; cnt ++)
    {
        m_textbox[cnt]->setIsEnabled(state);
    }

    m_forwardedTcpPortTextBox->setIsEnabled(state);
    m_passwordBox->setIsEnabled (state);

    m_preferDevCredential->setIsEnabled(state);

    m_autoLogin->setIsEnabled(state);

    m_mainRadioButton->setIsEnabled(state);
    m_subRadioButton->setIsEnabled(state);
    m_optimizedRadioButton->setIsEnabled(state);

    if(isInitDone)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void DeviceSetting::defaultFields()
{
    m_deviceName->setInputText(REMOTE_DEVICE_NAME);

    m_deviceModelName->setInputText("");
    m_deviceEnableBox->changeState(OFF_STATE);

    m_registerMode->setCurrValue(regModeMapList.value(REMOTE_CONNECTION_TYPE));

    switch(m_registerMode->getIndexofCurrElement ())
    {
    case BY_IP_ADDRESS:
        m_ipAddressTextbox->setIpaddress (REMOTE_IP_ADDRESS);
        m_DDNSHostName->setInputText ("");
        m_macAddressBox->setMacaddress ("");
        m_matrixDDNSTextBox->setInputText ("");
        m_ipAddressTextbox->setVisible (true);
        m_DDNSHostName->setVisible (false);
        m_macAddressBox->setVisible (false);
        m_matrixDDNSTextBox->setVisible (false);
        break;

    case BY_DDNS:
        m_ipAddressTextbox->setIpaddress ("");
        m_DDNSHostName->setInputText (REMOTE_IP_ADDRESS);
        m_macAddressBox->setMacaddress ("");
        m_matrixDDNSTextBox->setInputText ("");
        m_ipAddressTextbox->setVisible (false);
        m_DDNSHostName->setVisible (true);
        m_macAddressBox->setVisible (false);
        m_matrixDDNSTextBox->setVisible (false);
        break;

    case BY_MATRIX_MAC:
        m_ipAddressTextbox->setIpaddress ("");
        m_DDNSHostName->setInputText ("");
        m_macAddressBox->setMacaddress (REMOTE_IP_ADDRESS);
        m_matrixDDNSTextBox->setInputText ("");
        m_ipAddressTextbox->setVisible (false);
        m_DDNSHostName->setVisible (false);
        m_macAddressBox->setVisible (true);
        m_matrixDDNSTextBox->setVisible (false);
        break;

    case BY_MATRIX_HOSTNAME:
        m_ipAddressTextbox->setIpaddress ("");
        m_DDNSHostName->setInputText ("");
        m_macAddressBox->setMacaddress ("");
        m_matrixDDNSTextBox->setInputText (REMOTE_IP_ADDRESS);
        m_ipAddressTextbox->setVisible (false);
        m_DDNSHostName->setVisible (false);
        m_macAddressBox->setVisible (false);
        m_matrixDDNSTextBox->setVisible (true);
        break;

    default:
        break;
    }

    m_textbox[0]->setInputText(QString("%1").arg(REMOTE_TCP_PORT));
    m_textbox[1]->setInputText(REMOTE_USERNAME);
    m_forwardedTcpPortTextBox->setInputText(QString("%1").arg(REMOTE_FORWARDED_TCP_PORT));
    m_passwordBox->setInputText(REMOTE_PASSWORD);

    m_preferDevCredential->changeState(OFF_STATE);

    m_autoLogin->changeState(ON_STATE);
    m_mainRadioButton->changeState(ON_STATE);
    m_subRadioButton->changeState (OFF_STATE);
    m_optimizedRadioButton->changeState (OFF_STATE);

    m_controlBtnIndex = MAX_DEVICE_SETTINGS_ELEMENT;

    m_currentElement = DEVICE_SETTINGS_DEVICE_NAME_TEXTBOX;
    m_elementList[m_currentElement]->forceActiveFocus ();
}

bool DeviceSetting::dataVerification()
{
    if(m_deviceName->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DEV_NAME));
        return false;
    }

    if((m_registerMode->getIndexofCurrElement () == BY_IP_ADDRESS) && ((m_ipAddressTextbox->getIpaddress() == "") || (m_ipAddressTextbox->getIpaddress() == "0.0.0.0")))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VALID_IP_ADD));
        return false;
    }

    if((m_registerMode->getIndexofCurrElement () == BY_DDNS) && ((m_DDNSHostName->getInputText () == "")))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_HOSTNAME));
        return false;
    }

    if((m_registerMode->getIndexofCurrElement () == BY_MATRIX_MAC) && (m_macAddressBox->getMacaddress () == ""))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DEV_SETTING_ENT_MAC_ADDR));
        return false;
    }

    if (m_registerMode->getIndexofCurrElement () == BY_MATRIX_HOSTNAME)
    {
        if (m_matrixDDNSTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_HOSTNAME));
            return false;
        }

        if (!m_matrixDDNSTextBox->doneKeyValidation())
        {
            return false;
        }
    }

    if ((!m_textbox[0]->doneKeyValidation()) || (!m_forwardedTcpPortTextBox->doneKeyValidation()))
    {
        return false;
    }

    if (m_textbox[1]->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
        return false;
    }

    if (m_passwordBox->getInputText() == "")
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
        return false;
    }

    if (m_passwordBox->passwordDoneValidation() == false)
    {
        return false;
    }

    if (m_deviceName->getInputText() == LOCAL_DEVICE_NAME)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DEV_SETTING_DEV_NAME_MATCH_DEFAULT_NAME));
        return false;
    }

    if (m_deviceName->getInputText() == m_LocalDeviceName)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DEV_SETTING_DEV_NAME_MATCH_LOCAL));
        return false;
    }

    if ((m_controlBtnIndex == DEVICE_SETTINGS_DEVICE_ADD_BUTTON) && ((m_deviceList.contains (m_deviceName->getInputText())) == true))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DEV_SETTING_ENT_DEV_NAME_ALR_EXIT));
        return false;
    }

    if (m_controlBtnIndex != DEVICE_SETTINGS_DEVICE_EDIT_BUTTON)
    {
        return true;
    }

    QStringList  deviceList;

    for(quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
    {
        if(m_deviceList.at (index) != "")
        {
            deviceList.append (m_deviceList.at (index));
        }
    }

    if((m_deviceNameDropDownBox->getIndexofCurrElement () != (deviceList.indexOf (m_deviceName->getInputText()))) &&
            ((deviceList.contains (m_deviceName->getInputText())) == true))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DEV_SETTING_ENT_DEV_NAME_ALR_EXIT));
        return false;
    }

    return true;
}

void DeviceSetting::getDeviceList()
{
    isDeviceListEnable =  true;

    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             GENERAL_TABLE_INDEX,
                                                             1,
                                                             1,
                                                             FIELD_DEV_NAME+1,
                                                             FIELD_DEV_NAME+1,
                                                             1);

    payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                     NETWORK_DEVICE_SETTING_TABLE_INDEX,
                                                     1,
                                                     MAX_REMOTE_DEVICES,
                                                     DEVICE_SETTING_FROM_FIELD,
                                                     DEVICE_SETTING_FROM_FIELD,
                                                     DEVICE_SETTING_FROM_FIELD,
                                                     payloadString,
                                                     1);
    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    if(!processBar->isVisible ())
    {
        processBar->loadProcessBar();
    }
    m_applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DeviceSetting::setUserDataForConfigForDefault()
{
    m_deviceModelName->setInputText("");

    payloadLib->setCnfgArrayAtIndex(DEV_SET_DEVICE_NAME, REMOTE_DEVICE_NAME);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE, REMOTE_CONNECTION_TYPE);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE_ADDRESS, REMOTE_IP_ADDRESS);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_PORT, QString("%1").arg(REMOTE_TCP_PORT));

    payloadLib->setCnfgArrayAtIndex(DEV_SET_USERNAME, REMOTE_USERNAME);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_PASSWORD, REMOTE_PASSWORD);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_AUTOLOGIN, 1);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_ENABLE, 0);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_LIVE_STREAM, 0);

    payloadLib->setCnfgArrayAtIndex(DEV_SET_PREFER_NATIVE_DEV_CREDENTIAL, 0);
}

void DeviceSetting::setUserDataForConfig()
{
    quint8 liveStreamType = ((m_mainRadioButton->getCurrentState() == ON_STATE)
                             ? 0 : ((m_subRadioButton->getCurrentState() == ON_STATE)
                                    ? 1 : 2));
    payloadLib->setCnfgArrayAtIndex(DEV_SET_DEVICE_NAME,
                                    m_deviceName->getInputText());


    payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE,
                                    m_registerMode->getIndexofCurrElement());

    switch(m_registerMode->getIndexofCurrElement ())
    {
    case BY_IP_ADDRESS:
        payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE_ADDRESS,
                                        m_ipAddressTextbox->getIpaddress());
        break;

    case BY_DDNS:
        payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE_ADDRESS,
                                        m_DDNSHostName->getInputText());

        break;

    case BY_MATRIX_MAC:
        payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE_ADDRESS,
                                        m_macAddressBox->getMacaddress());
        break;

    case BY_MATRIX_HOSTNAME:
        payloadLib->setCnfgArrayAtIndex(DEV_SET_REGISTER_MODE_ADDRESS,
                                        m_matrixDDNSTextBox->getInputText());
        break;

    default:
        break;
    }

    payloadLib->setCnfgArrayAtIndex(DEV_SET_PORT,
                                    m_textbox[0]->getInputText());

    payloadLib->setCnfgArrayAtIndex(DEV_SET_USERNAME,
                                    m_textbox[1]->getInputText());

    payloadLib->setCnfgArrayAtIndex(DEV_SET_PASSWORD,
                                    m_passwordBox->getInputText());

    payloadLib->setCnfgArrayAtIndex(DEV_SET_AUTOLOGIN,
                                    m_autoLogin->getCurrentState());

    payloadLib->setCnfgArrayAtIndex(DEV_SET_ENABLE,
                                    m_deviceEnableBox->getCurrentState());

    payloadLib->setCnfgArrayAtIndex(DEV_SET_LIVE_STREAM,
                                    liveStreamType);

     payloadLib->setCnfgArrayAtIndex(DEV_SET_PREFER_NATIVE_DEV_CREDENTIAL,
                                     m_preferDevCredential->getCurrentState());

     payloadLib->setCnfgArrayAtIndex(DEV_SET_FORWARDED_PORT,
                                     m_forwardedTcpPortTextBox->getInputText());

}

void DeviceSetting::createPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             NETWORK_DEVICE_SETTING_TABLE_INDEX,
                                                             m_currentDeviceIndex,
                                                             m_currentDeviceIndex,
                                                             DEVICE_SETTING_FROM_FIELD,
                                                             DEVICE_SETTING_MAX_FIELD,
                                                             DEVICE_SETTING_MAX_FIELD);
    DevCommParam* param = new DevCommParam();

    param->msgType = requestType;
    param->payload = payloadString;

    if(!processBar->isVisible ())
    {
        processBar->loadProcessBar();
    }
    m_applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DeviceSetting::getConfig()
{
    if(!getconfigCall)
    {
        isDeviceListEnable = true;
        getconfigCall = true;
        getDeviceList();
    }
}

void DeviceSetting::getDeviceConfig()
{
    createPayload(MSG_GET_CFG);
}

void DeviceSetting::saveConfig()
{
    if(m_controlBtnIndex >= DEVICE_SETTINGS_DEVICE_ADD_BUTTON && m_controlBtnIndex <= DEVICE_SETTINGS_DEVICE_DELET_BUTTON)
    {
        DPRINT(CONFIG_PAGES, "[NETWORK_DEVICES]:[SAVE_REQ][%s]",m_deviceName->getInputText().toUtf8().constData());
    }

    switch(m_controlBtnIndex)
    {
        case DEVICE_SETTINGS_DEVICE_ADD_BUTTON:
            if(dataVerification())
            {
                m_DeviceName = m_deviceName->getInputText();
                setUserDataForConfig();

                qint8 tempIndex = m_deviceList.indexOf("");
                if(tempIndex < 0)
                {
                    tempIndex = (m_deviceCount);
                }
                m_currentDeviceIndex = (tempIndex + 1);

                createPayload(MSG_SET_CFG);
            }
            break;

        case DEVICE_SETTINGS_DEVICE_EDIT_BUTTON:
            if(dataVerification())
            {
                m_DeviceName = m_deviceNameDropDownBox->getCurrValue();
                m_currentDeviceIndex = (m_deviceList.indexOf (m_DeviceName)+ 1);

                setUserDataForConfig();

                createPayload(MSG_SET_CFG);
            }
            break;

        case DEVICE_SETTINGS_DEVICE_DELET_BUTTON:
            m_DeviceName = m_deviceNameDropDownBox->getCurrValue();
            m_currentDeviceIndex = (m_deviceList.indexOf (m_DeviceName) + 1);
            setUserDataForConfigForDefault();
            createPayload(MSG_SET_CFG);
            break;

        default:
            break;

    }//switch Case
}

void DeviceSetting::defaultConfig()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             NETWORK_DEVICE_SETTING_TABLE_INDEX,
                                                             1,
                                                             MAX_REMOTE_DEVICES,
                                                             DEVICE_SETTING_FROM_FIELD,
                                                             DEVICE_SETTING_MAX_FIELD,
                                                             DEVICE_SETTING_MAX_FIELD);
    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    if(!processBar->isVisible ())
    {
        processBar->loadProcessBar();
    }
    m_applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DeviceSetting::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == ValidationMessage::getValidationMessage(DEV_SETTING_DEL_DEVICE_CNFORM))
        {
            saveConfig();
        }
    }
    else
    {
        m_controlBtnIndex = MAX_DEVICE_SETTINGS_ELEMENT;
        enableDisableAllControl(false);
        if(infoPage->getText() == ValidationMessage::getValidationMessage(CONFI_CONTROL_DEFAULT_INFO_MSG))
        {
            m_currentDeviceIndex = (m_deviceList.indexOf (m_deviceNameDropDownBox->getCurrValue()) + 1);
            getConfig();
        }
    }
}

void DeviceSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    bool isProccessBarUnload = false;
    quint8 devModel;

    if(deviceName == currDevName)
    {
        if(param->deviceStatus == CMD_SUCCESS)
        {
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                if(param->deviceStatus == CMD_SUCCESS)
                {
                    quint8 tableIdx = 0, cnfgIdx = 0;

                    payloadLib->parsePayload(param->msgType, param->payload);
                    if(payloadLib->getcnfgTableIndex(tableIdx) == GENERAL_TABLE_INDEX)
                    {
                        tableIdx++;
                        m_LocalDeviceName = payloadLib->getCnfgArrayAtIndex(cnfgIdx++).toString();
                    }

                    if(payloadLib->getcnfgTableIndex(tableIdx) == NETWORK_DEVICE_SETTING_TABLE_INDEX)
                    {
                        if(isDeviceListEnable)
                        {
                            isDeviceListEnable = false;

                            QMap<quint8, QString>  deviceList;
                            m_deviceList.clear ();
                            quint8 devIndex = 0;

                            for(quint8 index = 0; index < MAX_REMOTE_DEVICES; index++)
                            {
                                QString tempDeviceName = payloadLib->getCnfgArrayAtIndex(index + cnfgIdx).toString();
                                if(tempDeviceName != "")
                                {
                                    deviceList.insert(devIndex, tempDeviceName);
                                    devIndex++;
                                }
                                m_deviceList.append(tempDeviceName);
                            }

                            m_deviceNameDropDownBox->setNewList(deviceList,(m_currentDeviceIndex-1));

                            if(isInitDone == false || (m_deviceList.at(m_currentDeviceIndex-1) == ""))
                            {
                                m_currentDeviceIndex = m_deviceList.indexOf (deviceList.value (0)) + 1;
                                isInitDone = true;
                            }
                            enableDisableAllControl(false);
                            getDeviceConfig();
                        }
                        else
                        {
                            m_deviceName->setInputText(payloadLib->getCnfgArrayAtIndex(DEV_SET_DEVICE_NAME + cnfgIdx).toString());

                            m_deviceNameDropDownBox->setCurrValue (payloadLib->getCnfgArrayAtIndex(DEV_SET_DEVICE_NAME + cnfgIdx).toString());

                            m_deviceEnableBox->changeState((payloadLib->getCnfgArrayAtIndex(DEV_SET_ENABLE + cnfgIdx).toUInt() == 1) ? ON_STATE : OFF_STATE);

                            quint8 tempIndex = payloadLib->getCnfgArrayAtIndex(DEV_SET_REGISTER_MODE + cnfgIdx).toUInt();
                            if(tempIndex >= regModeMapList.size())
                            {
                                tempIndex = 0;
                            }
                            m_registerMode->setCurrValue(regModeMapList.value(tempIndex));

                            QString regModeAddress =  payloadLib->getCnfgArrayAtIndex(DEV_SET_REGISTER_MODE_ADDRESS + cnfgIdx).toString();

                            switch(tempIndex)
                            {
                                case BY_IP_ADDRESS:
                                    m_ipAddressTextbox->setIpaddress (regModeAddress);
                                    m_DDNSHostName->setInputText ("");
                                    m_macAddressBox->setMacaddress ("");
                                    m_matrixDDNSTextBox->setInputText ("");
                                    break;

                                case BY_DDNS:
                                    m_ipAddressTextbox->setIpaddress ("");
                                    m_DDNSHostName->setInputText (regModeAddress);
                                    m_macAddressBox->setMacaddress ("");
                                    m_matrixDDNSTextBox->setInputText ("");
                                    break;

                                case BY_MATRIX_MAC:
                                    m_ipAddressTextbox->setIpaddress ("");
                                    m_DDNSHostName->setInputText ("");
                                    m_macAddressBox->setMacaddress (regModeAddress);
                                    m_matrixDDNSTextBox->setInputText ("");
                                    break;

                                case BY_MATRIX_HOSTNAME:
                                    m_ipAddressTextbox->setIpaddress ("");
                                    m_DDNSHostName->setInputText ("");
                                    m_macAddressBox->setMacaddress ("");
                                    m_matrixDDNSTextBox->setInputText (regModeAddress);
                                    break;

                                default:
                                    break;
                            }

                            slotDropDownBoxValueChanged (regModeMapList.value(tempIndex),DEVICE_SETTINGS_REGISTER_MODE_SPINBOX);

                            m_textbox[0]->setInputText(payloadLib->getCnfgArrayAtIndex(DEV_SET_PORT + cnfgIdx).toString());

                            m_textbox[1]->setInputText(payloadLib->getCnfgArrayAtIndex(DEV_SET_USERNAME + cnfgIdx).toString());

                            m_passwordBox->setInputText(payloadLib->getCnfgArrayAtIndex(DEV_SET_PASSWORD + cnfgIdx).toString());

                            m_preferDevCredential->changeState(payloadLib->getCnfgArrayAtIndex(DEV_SET_PREFER_NATIVE_DEV_CREDENTIAL + cnfgIdx).toUInt() == 1 ? ON_STATE : OFF_STATE);

                            m_autoLogin->changeState(payloadLib->getCnfgArrayAtIndex(DEV_SET_AUTOLOGIN + cnfgIdx).toUInt() == 1 ? ON_STATE:OFF_STATE);

                            quint8 liveStreamType = payloadLib->getCnfgArrayAtIndex(DEV_SET_LIVE_STREAM + cnfgIdx).toUInt();

                            m_forwardedTcpPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(DEV_SET_FORWARDED_PORT + cnfgIdx).toString());

                            switch(liveStreamType)
                            {
                                case 0:
                                    m_mainRadioButton->takeEnterKeyAction();
                                    break;

                                case 1:
                                    m_subRadioButton->takeEnterKeyAction();
                                    break;

                                case 2:
                                    m_optimizedRadioButton->takeEnterKeyAction();
                                    break;

                                default:
                                    break;
                            }

                            m_controlBtnIndex = MAX_DEVICE_SETTINGS_ELEMENT;

                            isProccessBarUnload = true;
                        }
                    }

                    m_applController->GetDevModel(payloadLib->getCnfgArrayAtIndex(DEV_SET_DEVICE_NAME + cnfgIdx).toString(), devModel);
                    m_deviceModelName->setInputText((devModel < NVR_VARIANT_MAX) ? deviceModelString[devModel] : "NA");
                    if(enabled == true)
                    {
                        enableDisableAllControl(true);
                        enabled = false;
                    }
                }
                break;

            case MSG_SET_CFG:
                if(m_controlBtnIndex == DEVICE_SETTINGS_DEVICE_ADD_BUTTON)
                {
                    MessageBanner::addMessageInBanner(m_deviceName->getInputText() + QString(" : ") + Multilang((ValidationMessage::getValidationMessage(DEV_SETTING_DEVICE_ADDED_SUCCESS)).toUtf8().constData()));
                }
                else if(m_controlBtnIndex == DEVICE_SETTINGS_DEVICE_EDIT_BUTTON)
                {
                    MessageBanner::addMessageInBanner(m_deviceName->getInputText() + QString(" : ") + Multilang((ValidationMessage::getValidationMessage(DEV_SETTING_DEVICE_MODIFIED_SUCCESS)).toUtf8().constData()));
                }
                else if(m_controlBtnIndex == DEVICE_SETTINGS_DEVICE_DELET_BUTTON)
                {
                    MessageBanner::addMessageInBanner(m_DeviceName + QString(" : ") + (Multilang(ValidationMessage::getValidationMessage(DEV_SETTING_DEVICE_DEL_SUCCESS).toUtf8().constData())));

                    qint8 tempIndex = m_deviceList.indexOf((m_DeviceName));
                    if(tempIndex >= 0)
                    {
                        m_deviceList.replace (tempIndex,"");
                    }

                    qint8 index = (tempIndex + 1);
                    bool isNextDevFound = false;
                    for(; index< m_deviceList.length (); index++)
                    {
                        if(m_deviceList.value (index) != "")
                        {
                            isNextDevFound = true;
                            break;
                        }
                    }

                    if(!isNextDevFound)
                    {
                        for(index = 0; index< tempIndex; index++)
                        {
                            if(m_deviceList.value (index) != "")
                            {
                                isNextDevFound = true;
                                break;
                            }
                        }
                    }

                    if(!isNextDevFound)
                    {
                        index =  0;
                    }

                    m_currentDeviceIndex = (index + 1);
                }
                getConfig();

                m_controlBtnIndex = MAX_DEVICE_SETTINGS_ELEMENT;
                m_DeviceName = "";
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            default:
                break;
            }
        }
        else
        {
            EPRINT(CONFIG_PAGES,"[NETWORK_DEVICES]:[DEVICE_REPLY: %d]",param->deviceStatus);
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));

            m_deviceModelName->setInputText("");
            if((m_controlBtnIndex != DEVICE_SETTINGS_DEVICE_DELET_BUTTON) && (m_controlBtnIndex != MAX_DEVICE_SETTINGS_ELEMENT))
            {
                quint8 index = m_deviceList.indexOf (m_deviceNameDropDownBox->getCurrValue ());
                m_currentDeviceIndex = (index + 1);
                getconfigCall = false;
                getConfig();
            }
            else
            {
                isProccessBarUnload = true;
            }
        }
    }

    if(isProccessBarUnload)
    {
        getconfigCall = false;
        processBar->unloadProcessBar ();
    }
}

void DeviceSetting::slotDropDownBoxValueChanged(QString string,quint32 indexInPage)
{
    switch(indexInPage)
    {
    case DEVICE_SETTINGS_DEVICENAME_SPINBOX:
        m_currentDeviceIndex = m_deviceList.indexOf(string) + 1;
        getConfig ();
        break;

    case DEVICE_SETTINGS_REGISTER_MODE_SPINBOX:
    {
        quint8 tempIndex = regModeMapList.key(string);

        switch(tempIndex)
        {
        case BY_IP_ADDRESS:
        {
            m_ipAddressTextbox->setVisible (true);
            m_DDNSHostName->setVisible (false);
            m_macAddressBox->setVisible (false);
            m_matrixDDNSTextBox->setVisible (false);

            m_elementList[DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX] = m_ipAddressTextbox;
        }
            break;

        case BY_DDNS:
        {
            m_ipAddressTextbox->setVisible (false);
            m_DDNSHostName->setVisible (true);
            m_macAddressBox->setVisible (false);
            m_matrixDDNSTextBox->setVisible (false);

            m_elementList[DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX] = m_DDNSHostName;
        }
            break;

        case BY_MATRIX_MAC:
        {
            m_ipAddressTextbox->setVisible (false);
            m_DDNSHostName->setVisible (false);
            m_macAddressBox->setVisible (true);
            m_matrixDDNSTextBox->setVisible (false);

            m_elementList[DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX] = m_macAddressBox;
        }
            break;

        case BY_MATRIX_HOSTNAME:
        {
            m_ipAddressTextbox->setVisible (false);
            m_DDNSHostName->setVisible (false);
            m_macAddressBox->setVisible (false);
            m_matrixDDNSTextBox->setVisible (true);

            m_elementList[DEVICE_SETTINGS_IP_ADDRESS_TEXTBOX] = m_matrixDDNSTextBox;
        }
            break;

        default:
            break;
        }

    }
        break;

    default:
        break;
    }
}

void DeviceSetting::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    QString tempStr = "";
    switch(index)
    {
        case DEVICE_SETTINGS_DEVICE_NAME_TEXTBOX:
            tempStr = ValidationMessage::getValidationMessage(DEV_NAME);
            break;

        case DEVICE_SETTINGS_MATRIX_DNS_HOSTNAME_TEXTBOX:
            if (msgType == INFO_MSG_STRAT_CHAR)
            {
                tempStr = ValidationMessage::getValidationMessage(START_CHAR_ERROR_MSG);
            }
            else
            {
                tempStr = ValidationMessage::getValidationMessage(DEV_SETTING_HOSTNAME_ATLEST_3_CHAR);
            }
            break;

        case DEVICE_SETTINGS_PORT_NO_TEXTBOX:
            tempStr = ValidationMessage::getValidationMessage(DEV_SETTING_ENT_PORT_DEFI_RANGE);
            break;

        case DEVICE_SETTINGS_PASSWORD_TEXTBOX:
            tempStr = ValidationMessage::getValidationMessage(DEV_SETTING_PASS_MIN_4_CHAR);
            break;

        case DEVICE_SETTINGS_FORWARDED_PORT_NO_TEXTBOX:
            tempStr = ValidationMessage::getValidationMessage(DEV_SETTING_ENT_FORWARDED_TCP_PORT_DEFI_RANGE);
            break;

        case DEVICE_SETTING_DEVICE_MODEL_TEXTBOX:
            tempStr = ValidationMessage::getValidationMessage(DEV_SETTING_ENT_VAL_DEV_MODEL_NAME);
            break;

        default:
            break;
    }

    infoPage->loadInfoPage (tempStr);
}

void DeviceSetting::slotControlButtonClicked(int index)
{
    if (index == DEVICE_SETTINGS_DEVICE_ADD_BUTTON)
    {
        defaultFields();
    }
    else if(index == DEVICE_SETTINGS_DEVICE_DELET_BUTTON)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(DEV_SETTING_DEL_DEVICE_CNFORM), true );
    }

    if(index != DEVICE_SETTINGS_DEVICE_DELET_BUTTON)
    {
        enableDisableAllControl(true);
    }

    if ((index >= DEVICE_SETTINGS_DEVICE_ADD_BUTTON) && (index <= DEVICE_SETTINGS_DEVICE_DELET_BUTTON))
    {
        m_controlBtnIndex = index;
    }
}

void DeviceSetting::slotRadioButtonClicked(OPTION_STATE_TYPE_e, int indexInPage)
{
    switch(indexInPage)
    {
    case DEVICE_SETTINGS_MAIN_RADIO_BUTTON:
        m_subRadioButton->changeState(OFF_STATE);
        m_optimizedRadioButton->changeState(OFF_STATE);
        break;

    case DEVICE_SETTINGS_SUB_RADIO_BUTTON:
        m_mainRadioButton->changeState(OFF_STATE);
        m_optimizedRadioButton->changeState(OFF_STATE);
        break;

    case DEVICE_SETTINGS_OPTIMIZED_RADIO_BUTTON:
        m_mainRadioButton->changeState(OFF_STATE);
        m_subRadioButton->changeState(OFF_STATE);
        break;
    }
}
