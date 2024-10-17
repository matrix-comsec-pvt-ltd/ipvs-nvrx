#include "UserAccountManagment.h"
#include "ValidationMessage.h"

#define CAMERA_LIST_CELL_HEIGHT     SCALE_HEIGHT(32)

static quint8  m_deviceCount;
static quint8  m_updateDeviceCount;

typedef enum
{
    USR_ACC_MAN_SPINBOX_CTRL,
    USR_ACC_MAN_ADD_CTRL,
    USR_ACC_MAN_EDIT_CTRL,
    USR_ACC_MAN_DEL_CTRL,
    USR_ACC_MAN_USR_TXTBOX,
    USR_ACC_MAN_PSD_TXTBOX,
    USR_ACC_MAN_CNF_PSD_TXTBOX,
    USR_ACC_MAN_ENB_CHK_BOX_CTRL,
    USR_ACC_MAN_MLTI_LOGIN_CTRL,
    USR_ACC_MAN_GRP_SPINBOX_CTRL,
    USR_ACC_MAN_ACCESSTIME_TXTBOX,
    USR_ACC_MAN_SYNC_WITH_NETWK_DEV_CHK_BOX_CTRL,
    USR_ACC_MAN_ALL_MONITORING_CHECKBOX,
    USR_ACC_MAN_ALL_PLAYBACK_CHECKBOX,
    USR_ACC_MAN_ALL_PTZ_CHECKBOX,
    USR_ACC_MAN_ALL_AUDIO_IN_CHECKBOX,
    USR_ACC_MAN_ALL_AUDIO_OUT_CHECKBOX,
    USR_ACC_MAN_ALL_VIDEO_POPUP_CHECKBOX,
    USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX,
    USR_ACC_MAN_PERCAM_PLAYBACK_CHECKBOX = (USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + MAX_CAMERA_ON_PAGE),
    USR_ACC_MAN_PERCAM_PTZ_CHECKBOX = (USR_ACC_MAN_PERCAM_PLAYBACK_CHECKBOX+ MAX_CAMERA_ON_PAGE),
    USR_ACC_MAN_PERCAM_AUDIO_IN_CHECKBOX = (USR_ACC_MAN_PERCAM_PTZ_CHECKBOX + MAX_CAMERA_ON_PAGE),
    USR_ACC_MAN_PERCAM_AUDIO_OUT_CHECKBOX = (USR_ACC_MAN_PERCAM_AUDIO_IN_CHECKBOX + MAX_CAMERA_ON_PAGE),
    USR_ACC_MAN_PERCAM_VIDEO_POPUP_CHECKBOX=(USR_ACC_MAN_PERCAM_AUDIO_OUT_CHECKBOX + MAX_CAMERA_ON_PAGE),
    USR_ACC_MAN_PREV_CTRL = (USR_ACC_MAN_PERCAM_VIDEO_POPUP_CHECKBOX + MAX_CAMERA_ON_PAGE),
    USR_ACC_PAGE_NUM_BTTN,
    USR_ACC_MAN_NEXT_CTRL = (USR_ACC_PAGE_NUM_BTTN + MAX_PAGE_NUMBER),
    USR_ACC_MAN_LAN_SPINBOX_CTRL,
    USR_ACC_MAN_PUSH_NOTIFICATION_CHECKBOX,
    MAX_USR_MAN_CONTROL
}USR_ACC_MAN_CONTROL_e;

typedef enum
{
    USR_ACC_MAN_USER_NAME = 0,
    USR_ACC_MAN_USER_GROUP,
    USR_ACC_MAN_PASSWORD,
    USR_ACC_MAN_ENABLE_USER,
    USR_ACC_MAN_MULTI_LOGIN,
    USR_ACC_MAN_ACCESS_TIME,
    USR_ACC_MAN_CAMERA_RIGHTS_START,
    USR_ACC_MAN_CAMERA_RIGHTS_END = (USR_ACC_MAN_CAMERA_RIGHTS_START + MAX_CAMERAS),
    USR_ACC_MAN_SYNC_NET_DEV = USR_ACC_MAN_CAMERA_RIGHTS_END,
    USR_ACC_MAN_PREFFERED_LANGUAGE,
    USR_ACC_MAN_PUSH_NOTIFICATIONS,
    MAX_USR_ACC_MAN_FEILD_NO
}USR_ACC_MAN_FEILD_NO_e;

typedef enum
{
    PASS_POLICY_CFG_MIN_PASS_LEN,
    PASS_POLICY_CFG_PASS_VAL,
    PASS_POLICY_CFG_RST_PER,
    PASS_POLICY_CFG_LCK_ATM,
    PASS_POLICY_CFG_MAX_LOG_ATM,
    PASS_POLICY_CFG_UNLCK_TIMER,
    MAX_PASS_POLICY_TABLE_FEILDS
}PASS_POLICY_TABLE_FEILDS_e;

typedef enum
{
    USR_ACC_SPIN_LABEL,
    USR_ACC_SPIN_PLACE_HOLDER,
    USR_ACC_ADD_USR_STRNG,
    USR_ACC_EDIT_USR_STRNG,
    USR_ACC_DEL_USR_STRNG,
    USR_ACC_ELE_HEADING_STRNG,
    USR_ACC_USERNAME_STRING,
    USR_ACC_USERNAME_VALIDATION_STRING,
    USR_ACC_PASSWORD_STRING,
    USR_ACC_PASSWORD_VALIDATION_STRING,
    USR_ACC_ENBL_CHK_BOX_SRTNG,
    USR_ACC_MAN_MLTI_LOGIN_SRTNG,
    USR_ACC_CNFM_PASSWORD_STRING,
    USR_ACC_GRP_SPIN_LABEL,
    USR_ACC_GRP_ADM_STRING,
    USR_ACC_GRP_OPR_STRING,
    USR_ACC_GRP_VIWR_STRING,
    USR_ACC_MAN_ACCESSTIME_STRING,
    USR_ACC_MAN_SYNC_WITH_NETWK_DEV_CHK_BOX_STRING,
    USR_ACC_ELE_HEADING1_STRNG,
    USR_ACC_ELE_CAM_NAME_STRING,
    USR_ACC_ELE_MONTRNG_STRING,
    USR_ACC_ELE_PLAYBACK_STRING,
    USR_ACC_ELE_PTZ_CTRL_SRING,
    USR_ACC_ELE_AUDIO_IN_STRING,
    USR_ACC_ELE_AUDIO_OUT_STRING,
    USR_ACC_ELE_VIDEO_POPUP_STRING,
    USR_ACC_MAN_PREV_STRING,
    USR_ACC_MAN_NEXT_STRING,
    USR_ACC_MAN_LANGUAGE_STRING,
    USR_ACC_MAN_PUSH_NOTIFICATON_STRING,
    MAX_USR_ACC_MAN_STRING
}USR_ACC_MAN_STRING_e;

static const QString userAccountManagmentStrings[MAX_USR_ACC_MAN_STRING] =
{
    "Select User",
    "Type to search...",
    "Add",
    "Edit",
    "Delete",
    "Create/Edit User",
    "Username",
    "[a-zA-Z0-9-_.,():@!#$*+/\\]",
    "Password",
    "[a-zA-Z0-9-_.,()\\[\\]:@!#$*+/\\]",
    "Enable",
    "Multi-Login",
    "Confirm Password",
    "Group",
    "Admin",
    "Operator",
    "Viewer",
    "Access duration per day",
    "Sync with Network Devices",
    "Access Rights",
    "Camera Name",
    "Monitoring",
    "Playback",
    "PTZ",
    "Audio In",
    "Audio Out",
    "Video Pop-up",
    "Previous",
    "Next",
    "Language",
    "Allow Push Notifications"
};

typedef enum
{
    MAX_USER_ERROR_STRING,
    USER_DELETION_MSG,
    USER_NAME_ERROR_MSG,
    USER_EXISTS_ERROR_MSG,
    PASSWORD_ERROR_MSG = 7,
    CNFRM_PASSWORD_ERROR_MSG,
    MISMATCH_PASSWORD_ERROR,
    ACCESS_TIME_BLANK_ERROR,
    MAX_USR_ACC_MAN_ERROR_STRING
}USR_ACC_MAN_ERROR_STRING_e;

UserAccountManagment::UserAccountManagment(QString devName, QWidget *parent, DEV_TABLE_INFO_t *tableInfo)
    : ConfigPageControl(devName, parent, MAX_USR_MAN_CONTROL, tableInfo, CNFG_TYPE_REF_SAV_BTN)
{
    isInitDone = false;
    m_devListCount = 0;
    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        cnfgToIndex[index] = cnfgFromIndex[index] = 1;
    }

    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        currentUserIndex[index] = 0;
    }

    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        dropDownUserStringList[index].clear();
        deviceUserStringList[index].clear();
    }
    lanStringList.clear();

    maxCameraNum = 0;
    currentPageNum = 0;
    isValidationSuceess = false;
    nextPageSelected = false;
    currentGroupIndex = 0;
    m_deviceCount = 0;
    m_minPassLen = 0;
    maxCameraOnPage = 0;
    networkDeviceIndex = 0;

    for(quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
    {
        perCamAudioInCheckBoxStatus[index] = false;
        perCamAudioOutCheckBoxStatus[index] = false;
        perCamMonitorCheckBoxStatus[index] = false;
        perCamPTZCheckBoxStatus[index] = false;
        perCamPlaybackCheckBoxStatus[index] = false;
        perCamVideoPopupCheckBoxStatus[index] = false;
    }
    syncNtwrkDevice = false;
    userDeleteCmd = false;
    ntwrkDevicesAvailable = true;
    m_userChange = false;

    INIT_OBJ(prevLabel);
    applController = ApplController::getInstance();
    applController->GetEnableDevList (m_devListCount, m_devList);
    m_deviceCount = m_devListCount - 1;
    m_updateDeviceCount = 1;
    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        userListReq[index] = false;
    }

    cameraList.clear();
    applController->GetAllCameraNamesOfDevice(currDevName, cameraList);
    maxCameraNum = cameraList.length();
    totalPages = (maxCameraNum / MAX_CAMERA_ON_PAGE) + ((maxCameraNum % MAX_CAMERA_ON_PAGE) ? 1 : 0);

    createDefaultComponents();
    UserAccountManagment::getConfig();
}

void UserAccountManagment::createDefaultComponents()
{
    quint8 deviceIndex = 0;
    applController->getDeviceIndex(currDevName,deviceIndex);
    getLanguage();

    for(quint8 index = 0; index < MAX_USR_MAN_CONTROL; index++)
    {
        m_elementList[index] = NULL;
    }
    currentPageNum = 0;

    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        currentUserIndex[index] = 0;
    }

    clickedUserButton = MAX_USR_MAN_CONTROL;

    if(dropDownUserStringList[deviceIndex].isEmpty())
    {
        dropDownUserStringList[deviceIndex].insert (0,"");
    }

    userListParam = new TextboxParam();
    userListParam->labelStr = userAccountManagmentStrings[USR_ACC_SPIN_LABEL];
    userListParam->textStr = dropDownUserStringList[deviceIndex].value(0);
    userListParam->isCentre = false;
    userListParam->leftMargin = SCALE_WIDTH(20);
    userListParam->maxChar = 24;
    userListParam->isTotalBlankStrAllow = true;
    userListParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    userListDropDown = new TextWithList(SCALE_WIDTH(13),SCALE_HEIGHT(8),
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        USR_ACC_MAN_SPINBOX_CTRL,
                                        dropDownUserStringList[deviceIndex],
                                        this,
                                        userListParam, TOP_LAYER,
                                        true, 8, TEXTBOX_ULTRAMEDIAM,
                                        userAccountManagmentStrings[USR_ACC_SPIN_PLACE_HOLDER]);
    m_elementList[USR_ACC_MAN_SPINBOX_CTRL] = userListDropDown;
    connect(userListDropDown,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect (userListDropDown,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    createBtn = new ControlButton(ADD_BUTTON_INDEX,
                                  userListDropDown->x() + userListDropDown->width() - SCALE_WIDTH(325),
                                  userListDropDown->y() + SCALE_HEIGHT(5) ,
                                  SCALE_WIDTH(40),SCALE_HEIGHT(30),
                                  this,
                                  NO_LAYER,SCALE_WIDTH(20),
                                  userAccountManagmentStrings[USR_ACC_ADD_USR_STRNG],
                                  true,
                                  USR_ACC_MAN_ADD_CTRL);
    m_elementList[USR_ACC_MAN_ADD_CTRL] = createBtn ;
    connect(createBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotControlBtnClicked(int)));
    connect (createBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    editBtn = new ControlButton(EDIT_BUTTON_INDEX,
                                createBtn->x() + SCALE_WIDTH(100),
                                userListDropDown->y() + SCALE_HEIGHT(5) ,
                                SCALE_WIDTH(40),SCALE_HEIGHT(30),
                                this,
                                NO_LAYER,SCALE_WIDTH(20),
                                userAccountManagmentStrings[USR_ACC_EDIT_USR_STRNG],
                                true,
                                USR_ACC_MAN_EDIT_CTRL);
    m_elementList[USR_ACC_MAN_EDIT_CTRL] = editBtn ;
    connect(editBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotControlBtnClicked(int)));
    connect (editBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    deleteBtn = new ControlButton(DELETE_BUTTON_INDEX,
                                  editBtn->x() + SCALE_WIDTH(100),
                                  userListDropDown->y() + SCALE_HEIGHT(5) ,
                                  SCALE_WIDTH(40),SCALE_HEIGHT(30),
                                  this,
                                  NO_LAYER,SCALE_WIDTH(20),
                                  userAccountManagmentStrings[USR_ACC_DEL_USR_STRNG],
                                  false,
                                  USR_ACC_MAN_DEL_CTRL);
    m_elementList[USR_ACC_MAN_DEL_CTRL] = deleteBtn ;
    connect(deleteBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotControlBtnClicked(int)));
    connect (deleteBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    usernameParam = new TextboxParam();
    usernameParam->labelStr = userAccountManagmentStrings[USR_ACC_USERNAME_STRING];
    usernameParam->textStr = dropDownUserStringList[deviceIndex].value(0);
    usernameParam->isCentre = false;
    usernameParam->leftMargin = SCALE_WIDTH(160);
    usernameParam->maxChar = 24;
    usernameParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    usernameTextBox = new TextBox(userListDropDown->x(),
                                  userListDropDown->y() + userListDropDown->height(),
                                  BGTILE_LARGE_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  USR_ACC_MAN_USR_TXTBOX,
                                  TEXTBOX_MEDIAM,
                                  this,
                                  usernameParam,
                                  MIDDLE_TABLE_LAYER,false);
    m_elementList[USR_ACC_MAN_USR_TXTBOX] = usernameTextBox;
    connect(usernameTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    connect (usernameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    enableUserCheckBox = new OptionSelectButton(userListDropDown->x() + userListDropDown->width()/2 + SCALE_WIDTH(170),
                                                usernameTextBox->y(),
                                                BGTILE_LARGE_SIZE_WIDTH/2,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                NO_LAYER,
                                                userAccountManagmentStrings[USR_ACC_ENBL_CHK_BOX_SRTNG],
                                                "",
                                                SCALE_WIDTH(30),
                                                USR_ACC_MAN_ENB_CHK_BOX_CTRL,
                                                false);
    m_elementList[USR_ACC_MAN_ENB_CHK_BOX_CTRL] = enableUserCheckBox;
    connect (enableUserCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    enableUserCheckBox->changeState(OFF_STATE);

    passwordParam = new TextboxParam();
    passwordParam->labelStr = userAccountManagmentStrings[USR_ACC_PASSWORD_STRING] ;
    passwordParam->textStr = "******";
    passwordParam->isCentre = false;
    passwordParam->leftMargin = SCALE_WIDTH(165);
    passwordParam->maxChar  = 16;
    passwordParam->minChar = 4;
    passwordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);
    passwordParam->isTotalBlankStrAllow =false;

    passwordTextBox = new PasswordTextbox(userListDropDown->x(),
                                          usernameTextBox->y() +
                                          usernameTextBox->height() ,
                                          BGTILE_LARGE_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          USR_ACC_MAN_PSD_TXTBOX,
                                          TEXTBOX_MEDIAM,
                                          this,
                                          passwordParam,
                                          MIDDLE_TABLE_LAYER,false);
    m_elementList[USR_ACC_MAN_PSD_TXTBOX] = passwordTextBox;
    connect(passwordTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    connect (passwordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    multiLoginCheckBox = new OptionSelectButton(userListDropDown->x() +
                                                userListDropDown->width()/2 + SCALE_WIDTH(140),
                                                passwordTextBox->y(),
                                                BGTILE_LARGE_SIZE_WIDTH/2,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                NO_LAYER,
                                                userAccountManagmentStrings[USR_ACC_MAN_MLTI_LOGIN_SRTNG],
                                                "",
                                                SCALE_WIDTH(160),
                                                USR_ACC_MAN_MLTI_LOGIN_CTRL,
                                                false);
    m_elementList[USR_ACC_MAN_MLTI_LOGIN_CTRL] = multiLoginCheckBox;
    connect (multiLoginCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    confirmpasswordParam = new TextboxParam();
    confirmpasswordParam->labelStr = userAccountManagmentStrings[USR_ACC_CNFM_PASSWORD_STRING] ;
    confirmpasswordParam->isCentre = false;
    confirmpasswordParam->leftMargin = SCALE_WIDTH(95);
    confirmpasswordParam->maxChar  = 16;
    confirmpasswordParam->minChar = 4;
    confirmpasswordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    confirmpasswordTextBox = new PasswordTextbox(userListDropDown->x(),
                                                 passwordTextBox->y() + passwordTextBox->height() ,
                                                 BGTILE_LARGE_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 USR_ACC_MAN_CNF_PSD_TXTBOX,
                                                 TEXTBOX_MEDIAM,
                                                 this,
                                                 confirmpasswordParam,
                                                 MIDDLE_TABLE_LAYER,
                                                 false);
    m_elementList[USR_ACC_MAN_CNF_PSD_TXTBOX] = confirmpasswordTextBox;
    connect(confirmpasswordTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    connect (confirmpasswordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    enableSynDevicescheckBox = new OptionSelectButton(confirmpasswordTextBox->x() +
                                                      confirmpasswordTextBox->width()/2 +SCALE_WIDTH(8),
                                                      confirmpasswordTextBox->y(),
                                                      BGTILE_LARGE_SIZE_WIDTH/2,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      this,
                                                      NO_LAYER,
                                                      userAccountManagmentStrings[USR_ACC_MAN_SYNC_WITH_NETWK_DEV_CHK_BOX_STRING],
                                                      "",
                                                      SCALE_WIDTH(30),
                                                      USR_ACC_MAN_SYNC_WITH_NETWK_DEV_CHK_BOX_CTRL,
                                                      false);
    m_elementList[USR_ACC_MAN_SYNC_WITH_NETWK_DEV_CHK_BOX_CTRL] = enableSynDevicescheckBox;
    connect (enableSynDevicescheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    enableSynDevicescheckBox->changeState(OFF_STATE);

    if(currDevName != LOCAL_DEVICE_NAME)
    {
        enableSynDevicescheckBox->setVisible(false);
    }

    grpStringList.clear();
    grpStringList.insert (0,userAccountManagmentStrings[USR_ACC_GRP_ADM_STRING]);
    grpStringList.insert (1,userAccountManagmentStrings[USR_ACC_GRP_OPR_STRING]);
    grpStringList.insert (2,userAccountManagmentStrings[USR_ACC_GRP_VIWR_STRING]);

    groupDropDownBox = new DropDown(userListDropDown->x(),
                                    confirmpasswordTextBox->y() + confirmpasswordTextBox->height(),
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    USR_ACC_MAN_GRP_SPINBOX_CTRL,
                                    DROPDOWNBOX_SIZE_200,
                                    userAccountManagmentStrings[USR_ACC_GRP_SPIN_LABEL],
                                    grpStringList,
                                    this,
                                    "",
                                    false, SCALE_WIDTH(185),
                                    MIDDLE_TABLE_LAYER, false, 8, false, false, SCALE_WIDTH(15));
    m_elementList[USR_ACC_MAN_GRP_SPINBOX_CTRL] = groupDropDownBox;
    connect(groupDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect (groupDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    languageDropDownBox = new DropDown(groupDropDownBox->x() +
                                       groupDropDownBox->width()/2 + SCALE_WIDTH(145),
                                       groupDropDownBox->y(),
                                       BGTILE_LARGE_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       USR_ACC_MAN_LAN_SPINBOX_CTRL,
                                       DROPDOWNBOX_SIZE_200,
                                       userAccountManagmentStrings[USR_ACC_MAN_LANGUAGE_STRING],
                                       lanStringList,
                                       this,
                                       "",
                                       false,SCALE_WIDTH(0),
                                       NO_LAYER, false, 8, false, false, SCALE_WIDTH(10));
    m_elementList[USR_ACC_MAN_LAN_SPINBOX_CTRL] = languageDropDownBox;
    connect(languageDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect (languageDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    accessTimeLimitParam = new TextboxParam();
    accessTimeLimitParam->labelStr = userAccountManagmentStrings[USR_ACC_MAN_ACCESSTIME_STRING];
    accessTimeLimitParam->suffixStr = "(5-1440 min)";
    accessTimeLimitParam->isCentre = false;
    accessTimeLimitParam->leftMargin = SCALE_WIDTH(37);
    accessTimeLimitParam->isNumEntry = true;
    accessTimeLimitParam->minNumValue = 5;
    accessTimeLimitParam->maxNumValue = 1440;
    accessTimeLimitParam->maxChar = 4;
    accessTimeLimitParam->validation = QRegExp(QString("[0-9]"));

    accessTimeLimitTextBox = new TextBox(userListDropDown->x(),
                                         groupDropDownBox->y() +
                                         groupDropDownBox->height(),
                                         BGTILE_LARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         USR_ACC_MAN_ACCESSTIME_TXTBOX,
                                         TEXTBOX_SMALL,
                                         this,
                                         accessTimeLimitParam,
                                         BOTTOM_TABLE_LAYER,
                                         false);
    m_elementList[USR_ACC_MAN_ACCESSTIME_TXTBOX] = accessTimeLimitTextBox;
    connect(accessTimeLimitTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    connect (accessTimeLimitTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    pushNotificationCheckBox = new OptionSelectButton(accessTimeLimitTextBox->x() +
                                                      accessTimeLimitTextBox->width()/2 + SCALE_WIDTH(35),
                                                      accessTimeLimitTextBox->y(),
                                                      BGTILE_LARGE_SIZE_WIDTH/2,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      this,
                                                      NO_LAYER,
                                                      userAccountManagmentStrings[USR_ACC_MAN_PUSH_NOTIFICATON_STRING],
                                                      "",
                                                      SCALE_WIDTH(30),
                                                      USR_ACC_MAN_PUSH_NOTIFICATON_STRING,
                                                      false);
    m_elementList[USR_ACC_MAN_PUSH_NOTIFICATON_STRING] = pushNotificationCheckBox;
    connect (pushNotificationCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    pushNotificationCheckBox->changeState(OFF_STATE);

    elementHeading1 = new ElementHeading(accessTimeLimitTextBox->x(),
                                         accessTimeLimitTextBox->y() +
                                         accessTimeLimitTextBox->height() + SCALE_HEIGHT(5),
                                         BGTILE_LARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         userAccountManagmentStrings[USR_ACC_ELE_HEADING1_STRNG],
                                         UP_LAYER,
                                         this,
                                         false,
                                         SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    bgTile_m = new BgTile (accessTimeLimitTextBox->x(),
                           elementHeading1->y() +
                           elementHeading1->height(),
                           BGTILE_LARGE_SIZE_WIDTH,
                           CAMERA_LIST_CELL_HEIGHT*9,
                           MIDDLE_LAYER,
                           this);

    quint16 tableCellWidth = (TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(20));

    tabelCellC1_H = new TableCell((confirmpasswordTextBox->x() + SCALE_WIDTH(10)),
                                  bgTile_m->y(),
                                  (TABELCELL_MEDIUM_SIZE_WIDTH),
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    textLabelC1_H = new TextLabel(tabelCellC1_H->x() + SCALE_WIDTH(10),
                                  bgTile_m->y() + CAMERA_LIST_CELL_HEIGHT/2 ,
                                  NORMAL_FONT_SIZE,
                                  userAccountManagmentStrings[USR_ACC_ELE_CAM_NAME_STRING],
                                  this,
                                  HIGHLITED_FONT_COLOR,
                                  NORMAL_FONT_FAMILY,
                                  ALIGN_START_X_CENTRE_Y);

    tabelCellC2_H = new TableCell(tabelCellC1_H->x() +
                                  tabelCellC1_H->width(),
                                  bgTile_m->y(),
                                  tableCellWidth- SCALE_WIDTH(50) ,
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    allMonitoringCheckBox = new OptionSelectButton(tabelCellC2_H->x() + SCALE_WIDTH(5),
                                                   bgTile_m->y(),
                                                   TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(70),
                                                   CAMERA_LIST_CELL_HEIGHT,
                                                   CHECK_BUTTON_INDEX,
                                                   this,
                                                   NO_LAYER,
                                                   userAccountManagmentStrings[USR_ACC_ELE_MONTRNG_STRING],"",0,
                                                   USR_ACC_MAN_ALL_MONITORING_CHECKBOX,
                                                   false);
    m_elementList[USR_ACC_MAN_ALL_MONITORING_CHECKBOX] = allMonitoringCheckBox;
    connect (allMonitoringCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (allMonitoringCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    allMonitoringCheckBox->changeFontColor(MX_OPTION_TEXT_TYPE_LABEL,HIGHLITED_FONT_COLOR);

    tabelCellC3_H = new TableCell(tabelCellC2_H->x() + tabelCellC2_H->width(),
                                  bgTile_m->y(),
                                  tableCellWidth - SCALE_WIDTH(60),
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    allPlaybackCheckBox = new OptionSelectButton(tabelCellC3_H->x() + SCALE_WIDTH(10),
                                                 bgTile_m->y(),
                                                 TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(80),
                                                 CAMERA_LIST_CELL_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 NO_LAYER,
                                                 userAccountManagmentStrings[USR_ACC_ELE_PLAYBACK_STRING],"",0,
                                                 USR_ACC_MAN_ALL_PLAYBACK_CHECKBOX,
                                                 false);
    allPlaybackCheckBox->changeFontColor(MX_OPTION_TEXT_TYPE_LABEL,HIGHLITED_FONT_COLOR);
    m_elementList[USR_ACC_MAN_ALL_PLAYBACK_CHECKBOX] = allPlaybackCheckBox;
    connect (allPlaybackCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (allPlaybackCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    tabelCellC4_H = new TableCell(tabelCellC3_H->x() + tabelCellC3_H->width(),
                                  bgTile_m->y(),
                                  tableCellWidth - SCALE_WIDTH(95),
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    allPTZCheckBox = new OptionSelectButton(tabelCellC4_H->x() + SCALE_WIDTH(10),
                                            bgTile_m->y(),
                                            TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(115),
                                            CAMERA_LIST_CELL_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            this,
                                            NO_LAYER,
                                            userAccountManagmentStrings[USR_ACC_ELE_PTZ_CTRL_SRING],"",0,
                                            USR_ACC_MAN_ALL_PTZ_CHECKBOX);
    allPTZCheckBox->changeFontColor(MX_OPTION_TEXT_TYPE_LABEL,HIGHLITED_FONT_COLOR);
    m_elementList[USR_ACC_MAN_ALL_PTZ_CHECKBOX] = allPTZCheckBox;
    connect (allPTZCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (allPTZCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    tabelCellC5_H = new TableCell(tabelCellC4_H->x() + tabelCellC4_H->width(),
                                  bgTile_m->y(),
                                  tableCellWidth - SCALE_WIDTH(60),
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    allAudioInCheckBox = new OptionSelectButton(tabelCellC5_H->x() + SCALE_WIDTH(10),
                                                bgTile_m->y(),
                                                TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(80),
                                                CAMERA_LIST_CELL_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                NO_LAYER,
                                                userAccountManagmentStrings[USR_ACC_ELE_AUDIO_IN_STRING],"",0,
                                                USR_ACC_MAN_ALL_AUDIO_IN_CHECKBOX);
    allAudioInCheckBox->changeFontColor(MX_OPTION_TEXT_TYPE_LABEL,HIGHLITED_FONT_COLOR);
    m_elementList[USR_ACC_MAN_ALL_AUDIO_IN_CHECKBOX] = allAudioInCheckBox;
    connect (allAudioInCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (allAudioInCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    tabelCellC6_H = new TableCell(tabelCellC5_H->x() + tabelCellC5_H->width(),
                                  bgTile_m->y(),
                                  tableCellWidth - SCALE_WIDTH(50),
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    allAudioOutCheckBox = new OptionSelectButton(tabelCellC6_H->x() + SCALE_WIDTH(10),
                                                 bgTile_m->y(),
                                                 TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(70),
                                                 CAMERA_LIST_CELL_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 NO_LAYER,
                                                 userAccountManagmentStrings[USR_ACC_ELE_AUDIO_OUT_STRING],"",0,
                                                 USR_ACC_MAN_ALL_AUDIO_OUT_CHECKBOX);
    allAudioOutCheckBox->changeFontColor(MX_OPTION_TEXT_TYPE_LABEL,HIGHLITED_FONT_COLOR);
    m_elementList[USR_ACC_MAN_ALL_AUDIO_OUT_CHECKBOX] = allAudioOutCheckBox;
    connect (allAudioOutCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (allAudioOutCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    tabelCellC7_H = new TableCell(tabelCellC6_H->x() + tabelCellC6_H->width(),
                                  bgTile_m->y(),
                                  tableCellWidth - SCALE_WIDTH(17) ,
                                  CAMERA_LIST_CELL_HEIGHT,
                                  this,
                                  true);

    allVideoCheckBox = new OptionSelectButton(tabelCellC7_H->x() + SCALE_WIDTH(10),
                                              bgTile_m->y(),
                                              TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(10),
                                              CAMERA_LIST_CELL_HEIGHT,
                                              CHECK_BUTTON_INDEX,
                                              this,
                                              NO_LAYER,
                                              userAccountManagmentStrings[USR_ACC_ELE_VIDEO_POPUP_STRING],"",0,
                                              USR_ACC_MAN_ALL_VIDEO_POPUP_CHECKBOX);
    allVideoCheckBox->changeFontColor(MX_OPTION_TEXT_TYPE_LABEL,HIGHLITED_FONT_COLOR);
    m_elementList[USR_ACC_MAN_ALL_VIDEO_POPUP_CHECKBOX] = allVideoCheckBox;
    connect (allVideoCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (allVideoCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0;index < MAX_CAMERA_ON_PAGE; index++)
    {
        tabelCellC1[index] = new TableCell(tabelCellC1_H->x(),
                                           tabelCellC1_H->y()+
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC1_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        textLabelC1[index] = new TextLabel(tabelCellC1_H->x() + SCALE_WIDTH(10) ,
                                           tabelCellC1[index]->y() +
                                           CAMERA_LIST_CELL_HEIGHT/2,
                                           NORMAL_FONT_SIZE,
                                           (index < maxCameraNum) ? cameraList.at(index) : "",
                                           this,
                                           NORMAL_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y);

        tabelCellC2[index] = new TableCell(tabelCellC2_H->x(),
                                           tabelCellC2_H->y() +
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC2_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        perCamMonitorCheckBox[index] = new OptionSelectButton(tabelCellC2[index]->x() +
                                                              tabelCellC2[index]->width()/2 - SCALE_WIDTH(15),
                                                              tabelCellC2[index]->y(),
                                                              TABELCELL_MEDIUM_SIZE_WIDTH,
                                                              CAMERA_LIST_CELL_HEIGHT,
                                                              CHECK_BUTTON_INDEX,
                                                              this,
                                                              NO_LAYER,
                                                              "",
                                                              "",
                                                              -1,
                                                              (USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + index),
                                                              false);
        m_elementList[USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + index*6] = perCamMonitorCheckBox[index];
        connect (perCamMonitorCheckBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        connect (perCamMonitorCheckBox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        tabelCellC3[index] = new TableCell(tabelCellC3_H->x(),
                                           tabelCellC3_H->y()  +
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC3_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        perCamPlaybackCheckBox[index] = new OptionSelectButton(tabelCellC3[index]->x() +
                                                               tabelCellC3[index]->width()/2 - SCALE_WIDTH(15),
                                                               tabelCellC3[index]->y(),
                                                               TABELCELL_MEDIUM_SIZE_WIDTH,
                                                               CAMERA_LIST_CELL_HEIGHT,
                                                               CHECK_BUTTON_INDEX,
                                                               this,
                                                               NO_LAYER,
                                                               "","",
                                                               -1,
                                                               (USR_ACC_MAN_PERCAM_PLAYBACK_CHECKBOX + index),
                                                               false);
        m_elementList[USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + 1 + index*6] = perCamPlaybackCheckBox[index];
        connect (perCamPlaybackCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));

        connect (perCamPlaybackCheckBox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        tabelCellC4[index] = new TableCell(tabelCellC4_H->x(),
                                           tabelCellC4_H->y()  +
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC4_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        perCamPTZCheckBox[index] = new OptionSelectButton(tabelCellC4[index]->x() +
                                                          tabelCellC4[index]->width()/2 - SCALE_WIDTH(15),
                                                          tabelCellC4[index]->y(),
                                                          TABELCELL_MEDIUM_SIZE_WIDTH,
                                                          CAMERA_LIST_CELL_HEIGHT,
                                                          CHECK_BUTTON_INDEX,
                                                          this,
                                                          NO_LAYER,
                                                          "","",
                                                          -1,
                                                          (USR_ACC_MAN_PERCAM_PTZ_CHECKBOX + index),
                                                          false);
        m_elementList[USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + 2 + index*6] = perCamPTZCheckBox[index];
        connect (perCamPTZCheckBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        connect (perCamPTZCheckBox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        tabelCellC5[index] = new TableCell(tabelCellC5_H->x(),
                                           tabelCellC5_H->y()  +
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC5_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        perCamAudioInCheckBox[index] = new OptionSelectButton(tabelCellC5[index]->x() +
                                                              tabelCellC5[index]->width()/2 - SCALE_WIDTH(15),
                                                              tabelCellC5[index]->y(),
                                                              TABELCELL_MEDIUM_SIZE_WIDTH,
                                                              CAMERA_LIST_CELL_HEIGHT,
                                                              CHECK_BUTTON_INDEX,
                                                              this,
                                                              NO_LAYER,
                                                              "","",-1,
                                                              (USR_ACC_MAN_PERCAM_AUDIO_IN_CHECKBOX + index),
                                                              false);
        m_elementList[USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + 3 + index*6] = perCamAudioInCheckBox[index];
        connect (perCamAudioInCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));
        connect (perCamAudioInCheckBox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        tabelCellC6[index] = new TableCell(tabelCellC6_H->x(),
                                           tabelCellC6_H->y()  +
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC6_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        perCamAudioOutCheckBox[index] = new OptionSelectButton(tabelCellC6[index]->x() +
                                                               tabelCellC6[index]->width()/2 - SCALE_WIDTH(15),
                                                               tabelCellC6[index]->y(),
                                                               TABELCELL_MEDIUM_SIZE_WIDTH,
                                                               CAMERA_LIST_CELL_HEIGHT,
                                                               CHECK_BUTTON_INDEX,
                                                               this,
                                                               NO_LAYER,
                                                               "","",-1,
                                                               (USR_ACC_MAN_PERCAM_AUDIO_OUT_CHECKBOX + index),
                                                               false);
        m_elementList[USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + 4 + index*6] = perCamAudioOutCheckBox[index];
        connect (perCamAudioOutCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));
        connect (perCamAudioOutCheckBox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));

        tableCellC7[index] = new TableCell(tabelCellC7_H->x(),
                                           tabelCellC7_H->y()  +
                                           CAMERA_LIST_CELL_HEIGHT*(index + 1),
                                           tabelCellC7_H->width(),
                                           CAMERA_LIST_CELL_HEIGHT,
                                           this);

        perCamVideoPopupCheckBox[index] = new OptionSelectButton(tableCellC7[index]->x() +
                                                                 tableCellC7[index]->width()/2 - SCALE_WIDTH(15),
                                                                 tableCellC7[index]->y(),
                                                                 TABELCELL_MEDIUM_SIZE_WIDTH,
                                                                 CAMERA_LIST_CELL_HEIGHT,
                                                                 CHECK_BUTTON_INDEX,
                                                                 this,
                                                                 NO_LAYER,
                                                                 "","",-1,
                                                                 (USR_ACC_MAN_PERCAM_VIDEO_POPUP_CHECKBOX + index),
                                                                 false);
        m_elementList[USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + 5 + index*6] = perCamVideoPopupCheckBox[index];
        if(IS_VALID_OBJ(perCamVideoPopupCheckBox))
        {
            connect (perCamVideoPopupCheckBox[index],
                      SIGNAL(sigUpdateCurrentElement(int)),
                      this,
                      SLOT(slotUpdateCurrentElement(int)));
            connect (perCamVideoPopupCheckBox[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
        }
    }

    bgTile_Bottom = new BgTile (confirmpasswordTextBox->x(),
                                bgTile_m->y() + bgTile_m->height(),
                                BGTILE_LARGE_SIZE_WIDTH,
                                ((maxCameraNum > MAX_CAMERA_ON_PAGE) ? (BGTILE_HEIGHT) : SCALE_HEIGHT(20)),
                                DOWN_LAYER,
                                this);

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                   confirmpasswordTextBox->x() + SCALE_WIDTH(10),
                                   bgTile_Bottom->y(),
                                   BGTILE_HEIGHT,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,0,
                                   userAccountManagmentStrings[USR_ACC_MAN_PREV_STRING],
                                   false,
                                   USR_ACC_MAN_PREV_CTRL,
                                   false);
    prevButton->setVisible((maxCameraNum > MAX_CAMERA_ON_PAGE) ? true : false);
    m_elementList[USR_ACC_MAN_PREV_CTRL] = prevButton;
    connect (prevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (prevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClicked(int)));

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_PageNumberLabel[index] = new TextWithBackground(prevButton->x() +
                                                          (SCALE_WIDTH(405) + (index*SCALE_WIDTH(40))),
                                                          prevButton->y() + SCALE_HEIGHT(5),
                                                          NORMAL_FONT_SIZE,
                                                          "",
                                                          this,
                                                          NORMAL_FONT_COLOR,
                                                          NORMAL_FONT_FAMILY,
                                                          ALIGN_START_X_START_Y,
                                                          0,
                                                          false,
                                                          TRANSPARENTKEY_COLOR,
                                                          true,
                                                          (USR_ACC_PAGE_NUM_BTTN + index));
        m_elementList[(USR_ACC_PAGE_NUM_BTTN + index)] = m_PageNumberLabel[index];
        connect(m_PageNumberLabel[index],
                SIGNAL(sigMousePressClick(QString)),
                this,
                SLOT(slotPageNumberButtonClick(QString)));
        connect(m_PageNumberLabel[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg (currentPageNum  + 1 + index) + QString(" "));

        if((index + currentPageNum) == currentPageNum)
        {
            m_PageNumberLabel[index]->setBackGroundColor(CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor(HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(true);
            m_PageNumberLabel[index]->forceActiveFocus();
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor(TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor(NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(false);
            m_PageNumberLabel[index]->deSelectControl();
        }

        m_PageNumberLabel[index]->update();
    }

    if(totalPages < MAX_PAGE_NUMBER)
    {
        for(quint8 index = totalPages; index < MAX_PAGE_NUMBER; index++)
        {
            m_PageNumberLabel[index]->setVisible(false);
            m_PageNumberLabel[index]->setIsEnabled(false);
        }
    }

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   confirmpasswordTextBox->x() +
                                   BGTILE_LARGE_SIZE_WIDTH  - SCALE_WIDTH(85) ,
                                   bgTile_Bottom->y(),
                                   BGTILE_HEIGHT,
                                   BGTILE_HEIGHT,
                                   this,NO_LAYER,0,
                                   userAccountManagmentStrings[USR_ACC_MAN_NEXT_STRING],
                                   true,
                                   USR_ACC_MAN_NEXT_CTRL);
    m_elementList[USR_ACC_MAN_NEXT_CTRL] = nextButton;
    nextButton->setVisible((maxCameraNum > MAX_CAMERA_ON_PAGE) ? true : false);
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClicked(int)));
    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(maxCameraNum <= MAX_CAMERA_ON_PAGE)
    {
        prevButton->setVisible(false);
        prevButton->setIsEnabled(false);
        nextButton->setVisible(false);
        nextButton->setIsEnabled(false);

        for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
        {
            m_PageNumberLabel[index]->setVisible(false);
            m_PageNumberLabel[index]->setIsEnabled(false);
        }
    }

    cnfgBtnPair[0]->resetGeometry(SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) / 2 - SCALE_WIDTH(70),
                                  (bgTile_Bottom->y() + bgTile_Bottom->height()) +
                                  (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - (bgTile_Bottom->y() + bgTile_Bottom->height()))/2);

    cnfgBtnPair[1]->resetGeometry(SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) / 2 + SCALE_WIDTH(70),
                                  (bgTile_Bottom->y() + bgTile_Bottom->height()) +
                                  (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - (bgTile_Bottom->y() + bgTile_Bottom->height()))/2);
}

UserAccountManagment::~UserAccountManagment()
{
    disconnect (nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClicked(int)));
    delete nextButton;

    disconnect (prevButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (prevButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClicked(int)));
    delete prevButton;

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigMousePressClick(QString)),
                   this,
                   SLOT(slotPageNumberButtonClick(QString)));
        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_PageNumberLabel[index];
    }

    delete bgTile_Bottom;

    for(quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
    {
        DELETE_OBJ(perCamVideoPopupCheckBox[index]);
        DELETE_OBJ(tableCellC7[index]);
        delete perCamAudioOutCheckBox[index];
        delete tabelCellC6[index];
        delete perCamAudioInCheckBox[index];
        delete tabelCellC5[index];
        delete perCamPTZCheckBox[index];
        delete tabelCellC4[index];
        delete perCamPlaybackCheckBox[index];
        delete tabelCellC3[index];
        delete perCamMonitorCheckBox[index];
        delete tabelCellC2[index] ;
        delete textLabelC1[index];
        delete tabelCellC1[index];
    }

    if(IS_VALID_OBJ(allVideoCheckBox))
    {
        disconnect (allVideoCheckBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (allVideoCheckBox,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e, int)));
        delete allVideoCheckBox;
        delete tabelCellC7_H;
    }

    if(IS_VALID_OBJ(allAudioOutCheckBox))
    {
        disconnect (allAudioOutCheckBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete allAudioOutCheckBox;
        delete tabelCellC6_H;
    }

    disconnect (allAudioInCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete allAudioInCheckBox;
    delete tabelCellC5_H;

    disconnect (allPTZCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete allPTZCheckBox;
    delete tabelCellC4_H;

    disconnect (allPlaybackCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete allPlaybackCheckBox;
    delete tabelCellC3_H;

    disconnect (allMonitoringCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete allMonitoringCheckBox;
    delete tabelCellC2_H;

    delete textLabelC1_H;
    delete tabelCellC1_H;

    delete bgTile_m;
    delete elementHeading1;

    disconnect (enableSynDevicescheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete enableSynDevicescheckBox;

    disconnect(accessTimeLimitTextBox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    disconnect (accessTimeLimitTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete accessTimeLimitTextBox;
    delete accessTimeLimitParam;

    disconnect(groupDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinBoxValueChange(QString,quint32)));
    disconnect (groupDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete groupDropDownBox;
    grpStringList.clear();

    disconnect (confirmpasswordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(confirmpasswordTextBox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    delete confirmpasswordTextBox;
    delete confirmpasswordParam;

    disconnect (multiLoginCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete multiLoginCheckBox;

    disconnect (passwordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(passwordTextBox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    delete passwordTextBox;
    delete passwordParam;

    disconnect (enableUserCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete enableUserCheckBox;

    disconnect (usernameTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(usernameTextBox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage (int,INFO_MSG_TYPE_e)));
    delete usernameTextBox;
    delete usernameParam;

    disconnect(deleteBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotControlBtnClicked(int)));
    disconnect (deleteBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete deleteBtn;

    disconnect(editBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotControlBtnClicked(int)));
    disconnect (editBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete editBtn;

    disconnect(createBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotControlBtnClicked(int)));

    disconnect(createBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete createBtn;

    disconnect (userListDropDown,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (userListDropDown,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    delete userListDropDown;
    delete userListParam;

    disconnect (languageDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    disconnect (languageDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete languageDropDownBox;

    disconnect (pushNotificationCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    delete pushNotificationCheckBox;

    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        dropDownUserStringList[index].clear();
        deviceUserStringList[index].clear();
    }
}

void UserAccountManagment::getLanguage()
{
    /* GET_LANGUAGE command should be sent to NVR-X device only. */
    if(devTableInfo->productVariant >= NVRX_SUPPORT_START_VARIANT)
    {
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = GET_LANGUAGE;
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

void UserAccountManagment::getUserConfig (quint8 nwDeviceIndex)
{
    cnfgFromIndex[nwDeviceIndex] = 1;
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             USER_ACCOUNT_MANAGMENT_TABLE_INDEX,
                                                             currentUserIndex[nwDeviceIndex],
                                                             currentUserIndex[nwDeviceIndex],
                                                             cnfgFromIndex[nwDeviceIndex],
                                                             MAX_USR_ACC_MAN_FEILD_NO,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

bool UserAccountManagment::setCnfgField(quint8 nwDeviceIndex,QString nwDevName)
{
    bool userStatus = true;
    bool userEditStatus = false;

    if(currDevName == nwDevName)
    {
        if(clickedUserButton == USR_ACC_MAN_ADD_CTRL || clickedUserButton == USR_ACC_MAN_EDIT_CTRL)
        {
            usernameTextBox->getInputText(userName);
            if(userName == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_USER_NM));
                userName = userListDropDown->getCurrValue();
                return false;
            }

            quint8 index;
            for (index = 0; index < deviceUserStringList[nwDeviceIndex].length(); index++)
            {
                if (index == (deviceUserStringList[nwDeviceIndex].indexOf(userName)))
                {
                    if (clickedUserButton != USR_ACC_MAN_ADD_CTRL)
                    {
                        break;
                    }

                    usernameTextBox->setInputText("");
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USR_ACC_MANAGE_USER_NAME_EXIST));
                    currentUserIndex[nwDeviceIndex] = 1;
                    getUserConfig (nwDeviceIndex);
                    return false;
                }
            }

            passwordTextBox->getInputText(password);
            confirmpasswordTextBox->getInputText(cnfrmPassword);

            if (password == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_PASSWORD));
                return false;
            }

            if (cnfrmPassword == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_CONFIRM_PASS));
                return false;
            }

            if (password != cnfrmPassword)
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PASS_MISMATCH));
                return false;
            }

            if (m_minPassLen > password.length())
            {
                infoPage->loadInfoPage(USER_PASSWROD_MIN_LEN_MSG(m_minPassLen));
                return false;
            }

            if (!((password.contains(QRegExp(QString("[0-9]")))) && (password.contains(QRegExp(QString("[A-Z]"))))
                  && (password.contains(QRegExp(QString("[a-z]")))) && (password.contains(QRegExp(QString("[\\-_.,():@!#$*+\\  \\[\\]/]"))))))
            {
                infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_HIGH_PASSWORD_SEC_REQ));
                return false;
            }

            if ((accessTimeLimitTextBox->getInputText() == "") &&
                    (groupDropDownBox->getCurrValue() == userAccountManagmentStrings[USR_ACC_GRP_VIWR_STRING]))
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USR_ACC_MANAGE_ENT_LOGIN_LIM_DURATION));
                return false;
            }

            if (clickedUserButton == USR_ACC_MAN_ADD_CTRL)
            {
                currentUserIndex[nwDeviceIndex] = deviceUserStringList[nwDeviceIndex].indexOf("") + 1 ;
                cnfgToIndex[nwDeviceIndex] = cnfgFromIndex[nwDeviceIndex] = currentUserIndex[nwDeviceIndex] ;
            }
            else
            {
                cnfgToIndex[nwDeviceIndex] = cnfgFromIndex[nwDeviceIndex] = (index + 1);
                currentUserIndex[nwDeviceIndex] = cnfgFromIndex[nwDeviceIndex];
            }

            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_USER_NAME,userName);
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_USER_GROUP, groupDropDownBox->getIndexofCurrElement());

            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PASSWORD,password);
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PREFFERED_LANGUAGE,languageDropDownBox->getCurrValue());

            for(quint8 index = 0; index < maxCameraNum; index++)
            {
                quint8 tmpAccessField = 0;
                tmpAccessField |= (((perCamAudioInCheckBoxStatus[index] == true) ? 1 : 0) << AUDIO_BITPOSITION);
                tmpAccessField |= (((perCamPTZCheckBoxStatus[index] == true) ? 1 : 0) << PTZ_BITPOSITION);
                tmpAccessField |= (((perCamPlaybackCheckBoxStatus[index] == true) ? 1: 0) << PLAYBACK_BITPOSITION);
                tmpAccessField |= (((perCamMonitorCheckBoxStatus[index] == true) ? 1: 0) << MONITORING_BITPOSITION);
                tmpAccessField |= (((perCamVideoPopupCheckBoxStatus[index] == true) ? 1: 0) << VIDEO_POPUP_BITPOSITION);
                tmpAccessField |= (((perCamAudioOutCheckBoxStatus[index] == true) ? 1: 0) << AUDIO_OUT_BITPOSITION);
                tmpAccessField |= (1 << RESEV_BITS);
                payloadLib->setCnfgArrayAtIndex((index + USR_ACC_MAN_CAMERA_RIGHTS_START), tmpAccessField);
            }

            for(quint8 index = maxCameraNum; index < MAX_USR_ACC_MAN_FEILD_NO ;index++)
            {
                payloadLib->setCnfgArrayAtIndex((index + USR_ACC_MAN_CAMERA_RIGHTS_START), 127);
            }

            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_ENABLE_USER, enableUserCheckBox->getCurrentState());
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_MULTI_LOGIN, multiLoginCheckBox->getCurrentState());
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_ACCESS_TIME, accessTimeLimitTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_SYNC_NET_DEV, enableSynDevicescheckBox->getCurrentState());
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PREFFERED_LANGUAGE, languageDropDownBox->getCurrValue());
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PUSH_NOTIFICATIONS, pushNotificationCheckBox->getCurrentState());
            userDeleteCmd = false;
        }
        else if (clickedUserButton == USR_ACC_MAN_DEL_CTRL)
        {
            cnfgFromIndex[nwDeviceIndex] = cnfgToIndex[nwDeviceIndex] = currentUserIndex[nwDeviceIndex] ;
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_USER_NAME,"");
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_USER_GROUP,0);
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PASSWORD,"");
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PREFFERED_LANGUAGE, 0);

            for(quint8 index = USR_ACC_MAN_CAMERA_RIGHTS_START; index < USR_ACC_MAN_CAMERA_RIGHTS_END; index++)
            {
                payloadLib->setCnfgArrayAtIndex(index, 63);
            }

            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_ENABLE_USER,OFF_STATE);
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_MULTI_LOGIN,OFF_STATE);
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PUSH_NOTIFICATIONS, OFF_STATE);
            payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_SYNC_NET_DEV,OFF_STATE);
            userDeleteCmd = true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        quint8 index;
        quint8 deviceIndex = 0;
        usernameTextBox->getInputText(userName);
        applController->getDeviceIndex(nwDevName, deviceIndex);
        for (index = 0; index < deviceUserStringList[deviceIndex].length(); index++)
        {
            if (index == (deviceUserStringList[deviceIndex].indexOf(userName)))
            {
                userStatus = false;
                userEditStatus = true;
                break;
            }
        }

        if (userStatus == true)
        {
            currentUserIndex[deviceIndex] = deviceUserStringList[deviceIndex].indexOf("") + 1 ;
            cnfgToIndex[deviceIndex] = cnfgFromIndex[deviceIndex] = currentUserIndex[deviceIndex];
        }

        if (userEditStatus == true)
        {
            cnfgToIndex[deviceIndex] = cnfgFromIndex[deviceIndex] = (index + 1);
            currentUserIndex[deviceIndex] = cnfgFromIndex[deviceIndex];
        }

        passwordTextBox->getInputText(password);
        confirmpasswordTextBox->getInputText(cnfrmPassword);

        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_USER_NAME,userName);
        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_USER_GROUP, groupDropDownBox->getIndexofCurrElement());
        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PASSWORD,password);
        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_PREFFERED_LANGUAGE, languageDropDownBox->getCurrValue());
        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_ENABLE_USER, enableUserCheckBox->getCurrentState());
        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_MULTI_LOGIN, multiLoginCheckBox->getCurrentState());
        payloadLib->setCnfgArrayAtIndex(USR_ACC_MAN_ACCESS_TIME, accessTimeLimitTextBox->getInputText());
        userDeleteCmd = false;
    }

    return true;
}

void UserAccountManagment::getUserList(quint8 nwDeviceIndex)
{
    userListReq[nwDeviceIndex] = true;
    cnfgFromIndex[nwDeviceIndex] = 1;
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             USER_ACCOUNT_MANAGMENT_TABLE_INDEX,
                                                             cnfgFromIndex[nwDeviceIndex],
                                                             MAX_CNFG_USERS,
                                                             cnfgFromIndex[nwDeviceIndex],
                                                             cnfgFromIndex[nwDeviceIndex],
                                                             MAX_USR_ACC_MAN_FEILD_NO);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void UserAccountManagment::getUserListForNtwrkDevice(quint8 nwDeviceIndex,QString nwDevName)
{
    if(applController->GetDeviceConnectionState(nwDevName) == CONNECTED)
    {
        quint8 deviceIndex = 0;
        applController->getDeviceIndex(m_devList.at(nwDeviceIndex),deviceIndex);
        userListReq[nwDeviceIndex] = true;
        cnfgFromIndex[deviceIndex] = 1;
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                                 USER_ACCOUNT_MANAGMENT_TABLE_INDEX,
                                                                 cnfgFromIndex[deviceIndex],
                                                                 MAX_CNFG_USERS,
                                                                 cnfgFromIndex[deviceIndex],
                                                                 cnfgFromIndex[deviceIndex],
                                                                 MAX_USR_ACC_MAN_FEILD_NO);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_GET_CFG;
        param->payload = payloadString;

        processBar->loadProcessBar();
        applController->processActivity(nwDevName, DEVICE_COMM, param);
    }
    else
    {
        m_updateDeviceCount++;
        m_deviceCount--;

        if(m_deviceCount > 0)
        {
            getUserListForNtwrkDevice(m_updateDeviceCount,m_devList.at(m_updateDeviceCount));
        }
    }
}

void UserAccountManagment::saveConfig()
{
    if(!accessTimeLimitTextBox->doneKeyValidation())
    {
        return;
    }

    quint8 deviceIndex = 0;

    applController->getDeviceIndex(currDevName, deviceIndex);
    if(setCnfgField(deviceIndex,currDevName) == true)
    {
        isValidationSuceess = true;
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 USER_ACCOUNT_MANAGMENT_TABLE_INDEX,
                                                                 cnfgFromIndex[deviceIndex],
                                                                 cnfgToIndex[deviceIndex],
                                                                 CNFG_FRM_FIELD,
                                                                 MAX_USR_ACC_MAN_FEILD_NO,
                                                                 MAX_USR_ACC_MAN_FEILD_NO);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;

        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }

    ntwrkDevicesAvailable = true;

    applController->GetEnableDevList (m_devListCount, m_devList);
    m_deviceCount = m_devListCount - 1;
    m_updateDeviceCount = 1;

    if ((dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "admin") ||
            (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "operator") ||
            (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "viewer"))
    {
        return;
    }

    if (OFF_STATE == enableSynDevicescheckBox->getCurrentState())
    {
        return;
    }

    if ((clickedUserButton != USR_ACC_MAN_ADD_CTRL) && (clickedUserButton != USR_ACC_MAN_EDIT_CTRL))
    {
        return;
    }

    if ((currDevName != LOCAL_DEVICE_NAME) || (m_devListCount <= 1))
    {
        return;
    }

    for(quint8 index = 1; index < m_devList.length(); index++)
    {
        if (applController->GetDeviceConnectionState(m_devList.at(index)) != CONNECTED)
        {
            continue;
        }

        quint8 deviceIndex = 0;
        applController->getDeviceIndex(m_devList.at(index),deviceIndex);
        if (false == setCnfgField(deviceIndex,m_devList.at(index)))
        {
            continue;
        }

        isValidationSuceess = true;
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 USER_ACCOUNT_MANAGMENT_TABLE_INDEX,
                                                                 cnfgFromIndex[deviceIndex],
                                                                 cnfgToIndex[deviceIndex],
                                                                 CNFG_FRM_FIELD,
                                                                 USR_ACC_MAN_ACCESS_TIME + 1,
                                                                 USR_ACC_MAN_ACCESS_TIME + 1);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;

        processBar->loadProcessBar();
        applController->processActivity(m_devList.at(index), DEVICE_COMM, param);
    }
}

void UserAccountManagment::defaultConfig()
{
    quint8 deviceIndex = 0;
    applController->getDeviceIndex(currDevName,deviceIndex);

    cnfgFromIndex[deviceIndex] = cnfgToIndex[deviceIndex] = 1;
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             USER_ACCOUNT_MANAGMENT_TABLE_INDEX,
                                                             cnfgFromIndex[deviceIndex],
                                                             cnfgToIndex[deviceIndex],
                                                             cnfgFromIndex[deviceIndex],
                                                             MAX_USR_ACC_MAN_FEILD_NO,
                                                             MAX_USR_ACC_MAN_FEILD_NO);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void UserAccountManagment::getConfig()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             PASSWORD_POLICY_SETTING_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             MAX_PASS_POLICY_TABLE_FEILDS,
                                                             MAX_PASS_POLICY_TABLE_FEILDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void UserAccountManagment::intialMode(quint8 currDeviceIndex)
{
    bool isEnable = true;

    if(dropDownUserStringList[currDeviceIndex].size() == MAX_CNFG_USERS)
    {
        isEnable = false;
    }

    if(createBtn->isEnabled() != isEnable)
    {
        createBtn->setIsEnabled(isEnable);
    }

    isEnable = true;

    if (dropDownUserStringList[currDeviceIndex].value((currentUserIndex[currDeviceIndex])) == DEFAULT_LOGIN_USER)
    {
        editBtn->setIsEnabled(!isEnable);
    }
    else
    {
        editBtn->setIsEnabled(isEnable);
    }

    if((deleteBtn->isEnabled() != isEnable) && (currentUserIndex[currDeviceIndex] > 3))
    {
        deleteBtn->setIsEnabled(isEnable);
    }
    else
    {
        if(currentUserIndex[currDeviceIndex] < 3)
        {
            deleteBtn->setIsEnabled(!isEnable);
        }
    }

    userListDropDown->setIsEnabled(isEnable);
    enableUserCheckBox->setIsEnabled(!isEnable);
    multiLoginCheckBox->setIsEnabled(!isEnable);
    usernameTextBox->setIsEnabled(!isEnable);
    passwordTextBox->setIsEnabled(!isEnable);
    confirmpasswordTextBox->setIsEnabled(!isEnable);
    groupDropDownBox->setIsEnabled(!isEnable);
    languageDropDownBox->setIsEnabled(!isEnable);
    pushNotificationCheckBox->setIsEnabled(!isEnable);
    enableSynDevicescheckBox->setIsEnabled(!isEnable);

    if((currentGroupIndex == ADMIN) || (currentGroupIndex == OPERATOR))
    {
        accessTimeLimitTextBox->setIsEnabled(false);
    }
    else
    {
        accessTimeLimitTextBox->setIsEnabled(!isEnable);
    }

    allMonitoringCheckBox->setIsEnabled(!isEnable);
    allPlaybackCheckBox->setIsEnabled(!isEnable);
    allPTZCheckBox->setIsEnabled(!isEnable);
    allAudioInCheckBox->setIsEnabled(!isEnable);
    allAudioOutCheckBox->setIsEnabled(!isEnable);
    allVideoCheckBox->setIsEnabled(!isEnable);

    for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
    {
        perCamMonitorCheckBox[index]->setIsEnabled(!isEnable);
        perCamPlaybackCheckBox[index]->setIsEnabled(!isEnable);
        perCamPTZCheckBox[index]->setIsEnabled(!isEnable);
        perCamAudioInCheckBox[index]->setIsEnabled(!isEnable);
        perCamAudioOutCheckBox[index]->setIsEnabled(!isEnable);
        perCamVideoPopupCheckBox[index]->setIsEnabled(!isEnable);
    }

    if(isInitDone)
    {
        m_currentElement--;
        takeRightKeyAction();
    }

    isValidationSuceess = false;
}

void UserAccountManagment::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    bool unloadProcessBar = true;
    if(deviceName == currDevName)
    {
        quint8 deviceIndex = 0;

        applController->getDeviceIndex(currDevName, deviceIndex);
        if (param->deviceStatus == CMD_SUCCESS)
        {
            switch(param->msgType)
            {
                case MSG_GET_CFG:
                {
                    payloadLib->parsePayload (param->msgType, param->payload);
                    if(payloadLib->getcnfgTableIndex() == USER_ACCOUNT_MANAGMENT_TABLE_INDEX)
                    {
                        if(userListReq[deviceIndex] == true)
                        {
                            dropDownUserStringList[deviceIndex].clear();
                            deviceUserStringList[deviceIndex].clear();

                            quint8 insertIndex = 0;
                            for (quint8 index = 0; index < MAX_CNFG_USERS; index++)
                            {
                                QString tempString = payloadLib->getCnfgArrayAtIndex(index).toString();
                                deviceUserStringList[deviceIndex].append(tempString);
                                if(tempString != "")
                                {
                                    dropDownUserStringList[deviceIndex].insert (insertIndex++, tempString);
                                }
                            }
                            userListDropDown->setNewList(dropDownUserStringList[deviceIndex]);

                            if (clickedUserButton == USR_ACC_MAN_DEL_CTRL)
                            {
                                currentUserIndex[deviceIndex] = 0 ;
                            }
                            else if (clickedUserButton == USR_ACC_MAN_ADD_CTRL || clickedUserButton == USR_ACC_MAN_EDIT_CTRL)
                            {
                                qint8 tempIndex = deviceUserStringList[deviceIndex].indexOf(userName);
                                if(tempIndex >= 0)
                                {
                                    currentUserIndex[deviceIndex] = deviceUserStringList[deviceIndex].indexOf(userName);
                                }
                                else
                                {
                                    currentUserIndex[deviceIndex] = 0;
                                    userName = deviceUserStringList[deviceIndex].value(0);
                                }
                            }

                            userListDropDown->setIndexofCurrElement(currentUserIndex[deviceIndex]);
                            usernameTextBox->setInputText(userName);
                            currentUserIndex[deviceIndex]++;
                            userListReq[deviceIndex] = false;
                            unloadProcessBar = false;
                            getLanguage();
                            getUserConfig (deviceIndex);
                        }
                        else
                        {
                            userName = payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_USER_NAME).toString();
                            currentPassword = payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_PASSWORD).toString();
                            if(userName != "")
                            {
                                currentUserIndex[deviceIndex] = dropDownUserStringList[deviceIndex].key(userName);
                                userListDropDown->setCurrValue(userName, m_userChange);
                                usernameTextBox->setInputText(dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]));

                                passwordTextBox->setInputText(currentPassword);
                                confirmpasswordTextBox->setInputText(currentPassword);
                                currentLangIndex = payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_PREFFERED_LANGUAGE).toString();
                                languageDropDownBox->setCurrValue(currentLangIndex);
                                currentGroupIndex = payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_USER_GROUP).toUInt();
                                groupDropDownBox->setCurrValue(grpStringList.value(currentGroupIndex));

                                quint32  eleIndex = USR_ACC_MAN_CAMERA_RIGHTS_START;
                                for(quint8 index = 0; index < maxCameraNum; index++, eleIndex++)
                                {
                                    quint32 rights = payloadLib->getCnfgArrayAtIndex(eleIndex).toUInt();
                                    perCamMonitorCheckBoxStatus[index] = (((rights >> MONITORING_BITPOSITION) & 0x01) == 0)? false : true;
                                    perCamPlaybackCheckBoxStatus[index] = (((rights >> PLAYBACK_BITPOSITION) & 0x01) == 0) ? false : true;
                                    perCamPTZCheckBoxStatus[index] = (((rights >> PTZ_BITPOSITION) & 0x01) == 0) ? false : true;
                                    perCamAudioInCheckBoxStatus[index] = (((rights >> AUDIO_BITPOSITION) & 0x01) == 0) ? false : true;
                                    perCamAudioOutCheckBoxStatus[index] = (((rights >> AUDIO_OUT_BITPOSITION) & 0x01) == 0) ? false : true;
                                    perCamVideoPopupCheckBoxStatus[index] = (((rights >> VIDEO_POPUP_BITPOSITION) & 0x01) == 0) ? false : true;
                                }

                                showFieldData();
                                setAllCheckBox();

                                enableUserCheckBox->changeState((payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_ENABLE_USER).toUInt() == 0 ? OFF_STATE : ON_STATE));
                                multiLoginCheckBox->changeState((payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_MULTI_LOGIN).toUInt() == 0 ? OFF_STATE : ON_STATE));
                                accessTimeLimitTextBox->setInputText((payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_ACCESS_TIME).toString()));
                                enableSynDevicescheckBox->changeState((payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_SYNC_NET_DEV).toUInt() == 0 ? OFF_STATE : ON_STATE));
                                pushNotificationCheckBox->changeState((payloadLib->getCnfgArrayAtIndex(USR_ACC_MAN_PUSH_NOTIFICATIONS).toUInt() == 0 ? OFF_STATE : ON_STATE));
                                intialMode (deviceIndex);
                                isInitDone = true;
                                m_userChange = false;
                            }

                            // If Network device present then get User list for Network device
                            if(ntwrkDevicesAvailable == true)
                            {
                                if((m_devListCount > 1) && (currDevName == LOCAL_DEVICE_NAME))
                                {
                                    ntwrkDevicesAvailable = false;
                                    getUserListForNtwrkDevice(1,m_devList.at(1));
                                }
                            }
                        }

                     clickedUserButton = MAX_USR_MAN_CONTROL;
                    }
                    else if(payloadLib->getcnfgTableIndex() == PASSWORD_POLICY_SETTING_TABLE_INDEX)
                    {
                        unloadProcessBar = false;
                        m_minPassLen = payloadLib->getCnfgArrayAtIndex(PASS_POLICY_CFG_MIN_PASS_LEN).toInt();
                        getUserList(deviceIndex);
                    }
                }
                break;

                case MSG_SET_CFG:
                {
                    if(userDeleteCmd)
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(USR_ACC_MANAGE_USER_DEL_SUCCESS));
                        userDeleteCmd = false;
                    }
                    else if(clickedUserButton == USR_ACC_MAN_ADD_CTRL)
                    {
                        m_userChange = true;
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(USR_ACC_MANAGE_USER_ADD_SUCCES));
                    }
                    else  if(clickedUserButton == USR_ACC_MAN_EDIT_CTRL)
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(USR_ACC_MANAGE_USER_MODIFY_SUCCESS));
                    }
                    else
                    {
                        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                    }
                    unloadProcessBar = false;
                    getConfig();
                }
                break;

                case MSG_SET_CMD:
                {
                    if(param->cmdType != GET_LANGUAGE)
                    {
                        break;
                    }

                    lanStringList.clear();
                    payloadLib->parseDevCmdReply(true, param->payload);
                    for (quint8 index = 0; index < MAX_SYSTEM_LANGUAGE; index++)
                    {
                        lanStringList.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                    }

                    languageDropDownBox->setNewList(lanStringList, languageDropDownBox->getIndexofCurrElement());
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        else
        {
            if(param->deviceStatus == CMD_INVALID_INDEX_ID)
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(USR_ACC_MANAGE_MAX_LIMIT_USER));
            }
            else
            {
                infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            }

            if(param->msgType == MSG_SET_CFG)
            {
                if(param->deviceStatus != CMD_SERVER_NOT_RESPONDING)
                {
                    unloadProcessBar = false;
                    getConfig();
                }
                else
                {
                    intialMode (deviceIndex);
                }
            }
            else if(param->msgType == MSG_GET_CFG)
            {
                clickedUserButton = MAX_USR_MAN_CONTROL;
                intialMode (deviceIndex);
            }
        }
    }
    else
    {
        quint8 deviceIndex = 0;

        applController->getDeviceIndex(deviceName, deviceIndex);
        if (param->deviceStatus == CMD_SUCCESS)
        {
            switch(param->msgType)
            {
                case MSG_GET_CFG:
                {
                    payloadLib->parsePayload (param->msgType, param->payload);
                    if(payloadLib->getcnfgTableIndex() != USER_ACCOUNT_MANAGMENT_TABLE_INDEX)
                    {
                        break;
                    }

                    if(userListReq[m_updateDeviceCount] == true)
                    {
                        dropDownUserStringList[deviceIndex].clear();
                        deviceUserStringList[deviceIndex].clear();

                        quint8 insertIndex = 0;
                        for (quint8 index = 0; index < MAX_CNFG_USERS; index++)
                        {
                            QString tempString = payloadLib->getCnfgArrayAtIndex(index).toString();
                            deviceUserStringList[deviceIndex].append (tempString);
                            if(tempString != "")
                            {
                                dropDownUserStringList[deviceIndex].insert (insertIndex++,tempString);
                            }

                        }

                        if (clickedUserButton == USR_ACC_MAN_ADD_CTRL || clickedUserButton == USR_ACC_MAN_EDIT_CTRL)
                        {
                            qint8 tempIndex = deviceUserStringList[deviceIndex].indexOf(userName);
                            if(tempIndex >= 0)
                            {
                                currentUserIndex[deviceIndex] = deviceUserStringList[deviceIndex].indexOf(userName);
                            }
                            else
                            {
                                currentUserIndex[deviceIndex] = 0;
                                userName = deviceUserStringList[deviceIndex].value(0);
                            }
                        }

                        currentUserIndex[deviceIndex]++;
                        userListReq[deviceIndex] = false;

                        m_updateDeviceCount++;
                        m_deviceCount--;
                        if(m_deviceCount > 0)
                        {
                            getUserListForNtwrkDevice(m_updateDeviceCount,m_devList.at(m_updateDeviceCount));
                        }
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
    }

    if(unloadProcessBar)
    {
        processBar->unloadProcessBar();
    }
}


void UserAccountManagment::showFieldData()
{
    quint8 fieldsOnPage;

    if(maxCameraNum < (MAX_CAMERA_ON_PAGE*(currentPageNum + 1)))
    {
        fieldsOnPage = maxCameraNum - ((MAX_CAMERA_ON_PAGE*(currentPageNum)));
    }
    else
    {
        fieldsOnPage = MAX_CAMERA_ON_PAGE;
    }

    for (quint8 index = 0; index < fieldsOnPage; index++)
    {
        quint8 eleIndex = (index + (MAX_CAMERA_ON_PAGE*currentPageNum));

        textLabelC1[index]->changeText(INT_TO_QSTRING(eleIndex + 1) + ": " + cameraList.at(eleIndex));
        textLabelC1[index]->update();

        perCamMonitorCheckBox[index]->changeState((perCamMonitorCheckBoxStatus[eleIndex] == false) ? OFF_STATE : ON_STATE);
        perCamPlaybackCheckBox[index]->changeState((perCamPlaybackCheckBoxStatus[eleIndex] == false) ? OFF_STATE : ON_STATE);
        perCamPTZCheckBox[index]->changeState((perCamPTZCheckBoxStatus[eleIndex] == false) ? OFF_STATE : ON_STATE);
        perCamAudioInCheckBox[index]->changeState((perCamAudioInCheckBoxStatus[eleIndex] == false) ? OFF_STATE : ON_STATE);
        perCamAudioOutCheckBox[index]->changeState((perCamAudioOutCheckBoxStatus[eleIndex] == false) ? OFF_STATE : ON_STATE);
        perCamVideoPopupCheckBox[index]->changeState((perCamVideoPopupCheckBoxStatus[eleIndex] == false) ? OFF_STATE : ON_STATE);

        if(perCamMonitorCheckBox[index]->isHidden())
        {
            perCamMonitorCheckBox[index]->setVisible(true);
        }

        if(perCamPlaybackCheckBox[index]->isHidden())
        {
            perCamPlaybackCheckBox[index]->setVisible(true);
        }

        if(perCamPTZCheckBox[index]->isHidden())
        {
            perCamPTZCheckBox[index]->setVisible(true);
        }

        if(perCamAudioInCheckBox[index]->isHidden())
        {
            perCamAudioInCheckBox[index]->setVisible(true);
        }

        if(perCamAudioOutCheckBox[index]->isHidden())
        {
            perCamAudioOutCheckBox[index]->setVisible(true);
        }

        if(perCamVideoPopupCheckBox[index]->isHidden())
        {
            perCamVideoPopupCheckBox[index]->setVisible(true);
        }
    }

    for(quint8 index = fieldsOnPage; index < MAX_CAMERA_ON_PAGE; index++)
    {
        textLabelC1[index]->changeText("");
        perCamMonitorCheckBox[index]->setVisible(false);
        perCamPlaybackCheckBox[index]->setVisible(false);
        perCamPTZCheckBox[index]->setVisible(false);
        perCamAudioInCheckBox[index]->setVisible(false);
        perCamAudioOutCheckBox[index]->setVisible(false);
        perCamVideoPopupCheckBox[index]->setVisible(false);
    }

    if((maxCameraNum > MAX_CAMERA_ON_PAGE) && (isInitDone))
    {
        if(currentPageNum != 0)
        {
            prevButton->setIsEnabled(true);
            m_elementList[m_currentElement]->forceActiveFocus();
        }
        else
        {
            if((m_currentElement == USR_ACC_MAN_NEXT_CTRL) || (m_currentElement == USR_ACC_MAN_PREV_CTRL))
            {
                if(!nextButton->isEnabled())
                {
                    nextButton->setIsEnabled(true);
                }
                m_currentElement = USR_ACC_MAN_NEXT_CTRL;
                m_elementList[m_currentElement]->forceActiveFocus();
            }
            prevButton->setIsEnabled(false);
        }

        if(currentPageNum == (totalPages - 1))
        {
            if((m_currentElement == USR_ACC_MAN_NEXT_CTRL) || (m_currentElement == USR_ACC_MAN_PREV_CTRL))
            {
                if(!prevButton->isEnabled())
                {
                    prevButton->setIsEnabled(true);
                }
                m_currentElement = USR_ACC_MAN_PREV_CTRL;
                m_elementList[m_currentElement]->forceActiveFocus();
            }
            nextButton->setIsEnabled(false);
        }
        else
        {
            if(!nextButton->isEnabled())
            {
                nextButton->setIsEnabled(true);
            }
            m_elementList[m_currentElement]->forceActiveFocus();
        }
    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        /* Calculate pagelable index (Possible values: 0, 1, 2, 3) */
        quint8 tempIndex = currentPageNum % MAX_PAGE_NUMBER;

        /* Derive first page index from currently visible page numbers (Possible values: 0, 4, 8, 12, ...) */
        quint8 pageOffset = currentPageNum - (currentPageNum % MAX_PAGE_NUMBER);

        if(nextPageSelected == true)
        {
            /* Check if we need to change page number text */
            if((pageOffset + index) < totalPages)
            {
                m_PageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg(pageOffset + 1 + index) + QString(" "));
            }
            else
            {
                m_PageNumberLabel[index]->changeText ("");
            }
        }
        else
        {
           /* Check if we need to change page number text */
           if(((currentPageNum + 1) % MAX_PAGE_NUMBER) == 0)
           {
               m_PageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg ((currentPageNum + 1) - (MAX_PAGE_NUMBER - index) + 1) + QString(" "));
           }
        }

        if((index == tempIndex) && ((pageOffset + index) < totalPages))
        {
            m_PageNumberLabel[index]->setBackGroundColor(CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor(HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(true);
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor(TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor(NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold(false);
            m_PageNumberLabel[index]->deSelectControl();
        }

        m_PageNumberLabel[index]->update();
    }
}

void UserAccountManagment::setAllCheckBox()
{
    quint8 index1 = 0,index2 = 0,index3 = 0,index4 = 0,index5 = 0,index6 = 0;

    for(quint8 index = 0; index < maxCameraNum; index++)
    {
        if (perCamMonitorCheckBoxStatus[index] == true) index1++;
        if (perCamPlaybackCheckBoxStatus[index] == true) index2++;
        if (perCamPTZCheckBoxStatus[index] == true) index3++;
        if (perCamAudioInCheckBoxStatus[index] == true) index4++;
        if (perCamAudioOutCheckBoxStatus[index] == true) index5++;
        if (perCamVideoPopupCheckBoxStatus[index] == true) index6++;
    }

    allMonitoringCheckBox->changeState(index1 == maxCameraNum ? ON_STATE : OFF_STATE);
    allPlaybackCheckBox->changeState(index2 == maxCameraNum ? ON_STATE : OFF_STATE);
    allPTZCheckBox->changeState(index3 == maxCameraNum ? ON_STATE : OFF_STATE);
    allAudioInCheckBox->changeState(index4 == maxCameraNum ? ON_STATE : OFF_STATE);
    allAudioOutCheckBox->changeState(index5 == maxCameraNum ? ON_STATE : OFF_STATE);
    allVideoCheckBox-> changeState(index6 == maxCameraNum ? ON_STATE : OFF_STATE);
}

void UserAccountManagment::changeAllCheckboxState()
{
    bool isEnable = true;
    QString currUserName;
    usernameTextBox->getInputText(currUserName);

    if((currUserName == "admin") || (currUserName == "operator"))
    {
        isEnable = false;
    }

    for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
    {
        perCamMonitorCheckBox[index]->setIsEnabled(isEnable);
        perCamPlaybackCheckBox[index]->setIsEnabled(isEnable);
        perCamPTZCheckBox[index]->setIsEnabled(isEnable);
        perCamAudioInCheckBox[index]->setIsEnabled(isEnable);
        perCamAudioOutCheckBox[index]->setIsEnabled(isEnable);
        perCamVideoPopupCheckBox[index]->setIsEnabled(isEnable);
    }

    allMonitoringCheckBox->setIsEnabled(isEnable);
    allPlaybackCheckBox->setIsEnabled(isEnable);
    allPTZCheckBox->setIsEnabled(isEnable);
    allAudioInCheckBox->setIsEnabled(isEnable);
    allAudioOutCheckBox->setIsEnabled(isEnable);
    allVideoCheckBox->setIsEnabled(isEnable);
}

void UserAccountManagment::handleInfoPageMessage(int index)
{
    quint8 deviceIndex = 0;

    applController->getDeviceIndex(currDevName,deviceIndex);
    if((clickedUserButton == USR_ACC_MAN_DEL_CTRL) && (index == INFO_OK_BTN))
    {
        if (infoPage->getText() == ValidationMessage::getValidationMessage(USR_ACC_MANAGE_SURE_DELETE_USER))
        {
            m_userChange = true;
            saveConfig();
        }
        else
        {
            getConfig();
        }
    }
    else if((index == INFO_OK_BTN) && ((clickedUserButton == USR_ACC_MAN_ADD_CTRL) || (clickedUserButton == USR_ACC_MAN_EDIT_CTRL)) && (isValidationSuceess == true))
    {
        getConfig();
    }
    else
    {
        currentUserIndex[deviceIndex] = userListDropDown->getIndexofCurrElement();
    }
}

void UserAccountManagment::slotSpinBoxValueChange(QString str,quint32 index)
{
    quint8 deviceIndex = 0;

    applController->getDeviceIndex(currDevName,deviceIndex);

    if(index == USR_ACC_MAN_SPINBOX_CTRL)
    {
        if(str != usernameTextBox->getInputText())
        {
            currentUserIndex[deviceIndex] = userListDropDown->getIndexofCurrElement();
            currentUserIndex[deviceIndex] = (deviceUserStringList[deviceIndex].indexOf(str)) + 1;
            getUserConfig(deviceIndex);
        }
    }
    else if (index == USR_ACC_MAN_GRP_SPINBOX_CTRL)
    {
        currentGroupIndex = groupDropDownBox->getIndexofCurrElement();
        if((currentGroupIndex == ADMIN) || (currentGroupIndex == OPERATOR))
        {
            accessTimeLimitTextBox->setIsEnabled(false);
        }
        else
        {
            accessTimeLimitTextBox->setIsEnabled(true);
        }
        changeAllCheckboxState();
    }
}

void UserAccountManagment::slotControlBtnClicked (int index)
{
    quint8 deviceIndex = 0;

    applController->getDeviceIndex(currDevName,deviceIndex);

    switch(index)
    {
        case USR_ACC_MAN_ADD_CTRL:
        {
            usernameTextBox->setIsEnabled(true);
            passwordTextBox->setIsEnabled(true);
            confirmpasswordTextBox->setIsEnabled(true);
            enableUserCheckBox->setIsEnabled(true);
            multiLoginCheckBox->setIsEnabled(true);
            pushNotificationCheckBox->setIsEnabled(true);

            enableUserCheckBox->changeState(ON_STATE);
            multiLoginCheckBox->changeState(ON_STATE);

            if(currDevName == LOCAL_DEVICE_NAME)
            {
                enableSynDevicescheckBox->setIsEnabled(true);
            }
            enableSynDevicescheckBox->changeState(ON_STATE);
            groupDropDownBox->setIsEnabled(true);
            groupDropDownBox->setIndexofCurrElement(0);
            languageDropDownBox->setIsEnabled(true);
            languageDropDownBox->setIndexofCurrElement(0);
            accessTimeLimitTextBox->setIsEnabled(false);
            accessTimeLimitTextBox->setInputText("5");

            usernameTextBox->setInputText("");
            passwordTextBox->setInputText("");
            confirmpasswordTextBox->setInputText("");

            m_currentElement = USR_ACC_MAN_USR_TXTBOX;
            m_elementList[m_currentElement]->forceActiveFocus();

            editBtn->setIsEnabled(false);
            createBtn->setIsEnabled(false);
            deleteBtn->setIsEnabled(false);
            userListDropDown->setIsEnabled(false);

            userDeleteCmd = false;
            clickedUserButton = USR_ACC_MAN_ADD_CTRL;

            for(quint8 index = 0; index < maxCameraNum; index++)
            {
                perCamMonitorCheckBoxStatus[index] = true;
                perCamPlaybackCheckBoxStatus[index] = true;
                perCamPTZCheckBoxStatus[index] = true;
                perCamAudioInCheckBoxStatus[index] = true;
                perCamAudioOutCheckBoxStatus[index] = true;
                perCamVideoPopupCheckBoxStatus[index] =true;
            }

            for(quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamMonitorCheckBox[index]->changeState(ON_STATE);
                perCamPlaybackCheckBox[index]->changeState(ON_STATE);
                perCamPTZCheckBox[index]->changeState(ON_STATE);
                perCamAudioInCheckBox[index]->changeState(ON_STATE);
                perCamAudioOutCheckBox[index]->changeState(ON_STATE);
                perCamVideoPopupCheckBox[index]->changeState(ON_STATE);
            }

            currentGroupIndex = groupDropDownBox->getIndexofCurrElement();
            changeAllCheckboxState();
            setAllCheckBox();
        }
        break;

        case USR_ACC_MAN_EDIT_CTRL:
        {
            if ((dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "admin") ||
                    (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "operator") ||
                    (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "viewer"))
            {
                if ((dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "admin"))
                {
                    enableUserCheckBox->setIsEnabled(false);
                }
                else if((dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "operator") ||
                        (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "viewer"))
                {
                    enableUserCheckBox->setIsEnabled(true);
                }
                multiLoginCheckBox->setIsEnabled(true);
                passwordTextBox->setIsEnabled(true);
                confirmpasswordTextBox->setIsEnabled(true);
                groupDropDownBox->setIsEnabled(false);
                languageDropDownBox->setIsEnabled(true);
                enableSynDevicescheckBox->setIsEnabled(false);
                pushNotificationCheckBox->setIsEnabled(true);
            }
            else if (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]) == "local")
            {
                enableUserCheckBox->setIsEnabled(false);
                multiLoginCheckBox->setIsEnabled(false);
                passwordTextBox->setIsEnabled(false);
                confirmpasswordTextBox->setIsEnabled(false);
                groupDropDownBox->setIsEnabled(false);
                languageDropDownBox->setIsEnabled(false);
                enableSynDevicescheckBox->setIsEnabled(false);
                pushNotificationCheckBox->setIsEnabled(false);
            }
            else
            {
                enableUserCheckBox->setIsEnabled(true);
                multiLoginCheckBox->setIsEnabled(true);
                passwordTextBox->setIsEnabled(true);
                confirmpasswordTextBox->setIsEnabled(true);
                groupDropDownBox->setIsEnabled(true);
                languageDropDownBox->setIsEnabled(true);
                enableSynDevicescheckBox->setIsEnabled(true);
                pushNotificationCheckBox->setIsEnabled(true);
            }

            if((currentGroupIndex == ADMIN) || (currentGroupIndex == OPERATOR))
            {
                accessTimeLimitTextBox->setIsEnabled(false);
            }
            else
            {
                accessTimeLimitTextBox->setIsEnabled(true);
            }

            usernameTextBox->setInputText(userName);
            passwordTextBox->setInputText(currentPassword);
            confirmpasswordTextBox->setInputText(currentPassword);

            m_currentElement = USR_ACC_MAN_PSD_TXTBOX;
            m_elementList[m_currentElement]->forceActiveFocus();

            editBtn->setIsEnabled(false);
            createBtn->setIsEnabled(false);
            deleteBtn->setIsEnabled(false);
            userListDropDown->setIsEnabled(false);

            userDeleteCmd = false;
            clickedUserButton = USR_ACC_MAN_EDIT_CTRL;

            changeAllCheckboxState();
        }
        break;

        case  USR_ACC_MAN_NEXT_CTRL:
        {
            if (currentPageNum != (totalPages - 1))
            {
                currentPageNum++;
            }
            nextPageSelected = true;
            showFieldData();
        }
        break;

        case USR_ACC_MAN_PREV_CTRL:
        {
            if(currentPageNum > 0)
            {
                currentPageNum--;
            }
            nextPageSelected = false;
            showFieldData();
        }
        break;

        case USR_ACC_MAN_DEL_CTRL:
        {
            currentUserIndex[deviceIndex] = (deviceUserStringList[deviceIndex].indexOf (dropDownUserStringList[deviceIndex].value(currentUserIndex[deviceIndex]))) + 1;
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USR_ACC_MANAGE_SURE_DELETE_USER),true);
            clickedUserButton = USR_ACC_MAN_DEL_CTRL;
            userDeleteCmd = true;
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void UserAccountManagment::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        switch (index)
        {
            case USR_ACC_MAN_USR_TXTBOX:
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VALID_USER));
                break;

            case USR_ACC_MAN_PSD_TXTBOX:
            case USR_ACC_MAN_CNF_PSD_TXTBOX:
                infoPage->loadInfoPage (USER_PASSWROD_MIN_LEN_MSG(m_minPassLen));
                break;

            case USR_ACC_MAN_ACCESSTIME_TXTBOX:
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USR_ACC_MANAGE_ENT_LOGIN_LIM_DEFI_RANGE));
                break;

            default:
                break;
        }
    }
}

void UserAccountManagment::slotCheckBoxClicked(OPTION_STATE_TYPE_e status, int checkBoxindex)
{
    switch(checkBoxindex)
    {
        case USR_ACC_MAN_ALL_MONITORING_CHECKBOX:
        {
            for (quint8 index = 0; index < MAX_CAMERAS; index++)
            {
                perCamMonitorCheckBoxStatus[index] = (status == ON_STATE ? true : false);
            }

            for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamMonitorCheckBox[index]->changeState(status);
            }
        }
        break;

        case USR_ACC_MAN_ALL_PLAYBACK_CHECKBOX:
        {
            for (quint8 index = 0; index < MAX_CAMERAS; index++)
            {
                perCamPlaybackCheckBoxStatus[index] = (status == ON_STATE ? true : false);
            }

            for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamPlaybackCheckBox[index]->changeState(status);
            }
        }
        break;

        case USR_ACC_MAN_ALL_PTZ_CHECKBOX:
        {
            for (quint8 index = 0; index < MAX_CAMERAS; index++)
            {
                perCamPTZCheckBoxStatus[index] = (status == ON_STATE ? true : false);
            }

            for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamPTZCheckBox[index]->changeState(status);
            }
        }
        break;

        case USR_ACC_MAN_ALL_AUDIO_IN_CHECKBOX:
        {
            for (quint8 index = 0; index < MAX_CAMERAS; index++)
            {
                perCamAudioInCheckBoxStatus[index] = (status == ON_STATE ? true : false);
            }

            for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamAudioInCheckBox[index]->changeState(status);
            }
        }
        break;

        case USR_ACC_MAN_ALL_AUDIO_OUT_CHECKBOX:
        {
            for (quint8 index = 0; index < MAX_CAMERAS; index++)
            {
                perCamAudioOutCheckBoxStatus[index] = (status == ON_STATE ? true : false);
            }

            for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamAudioOutCheckBox[index]->changeState(status);
            }
        }
        break;

        case USR_ACC_MAN_ALL_VIDEO_POPUP_CHECKBOX:
        {
            for (quint8 index = 0; index < MAX_CAMERAS; index++)
            {
                perCamVideoPopupCheckBoxStatus[index] = (status == ON_STATE ? true : false);
            }

            for (quint8 index = 0; index < MAX_CAMERA_ON_PAGE; index++)
            {
                perCamVideoPopupCheckBox[index]->changeState(status);
            }
        }
        break;

        default:
        {
            if((checkBoxindex >= USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX) && (checkBoxindex <(USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX + MAX_CAMERA_ON_PAGE)))
            {
                perCamMonitorCheckBoxStatus[(checkBoxindex - USR_ACC_MAN_PERCAM_MONITOR_CHECKBOX) + (currentPageNum*MAX_CAMERA_ON_PAGE)]
                        = (status == ON_STATE ? true : false);
            }
            else if((checkBoxindex >= USR_ACC_MAN_PERCAM_PLAYBACK_CHECKBOX) && (checkBoxindex <(USR_ACC_MAN_PERCAM_PLAYBACK_CHECKBOX + MAX_CAMERA_ON_PAGE)))
            {
                perCamPlaybackCheckBoxStatus[(checkBoxindex - USR_ACC_MAN_PERCAM_PLAYBACK_CHECKBOX) + (currentPageNum*MAX_CAMERA_ON_PAGE)]
                        = (status == ON_STATE ? true : false);
            }
            else if ((checkBoxindex >= USR_ACC_MAN_PERCAM_PTZ_CHECKBOX) && (checkBoxindex <(USR_ACC_MAN_PERCAM_PTZ_CHECKBOX + MAX_CAMERA_ON_PAGE)))
            {
                perCamPTZCheckBoxStatus[(checkBoxindex - USR_ACC_MAN_PERCAM_PTZ_CHECKBOX) + (currentPageNum*MAX_CAMERA_ON_PAGE)]
                        = (status == ON_STATE ? true : false);
            }
            else if ((checkBoxindex >= USR_ACC_MAN_PERCAM_AUDIO_IN_CHECKBOX) && (checkBoxindex < (USR_ACC_MAN_PERCAM_AUDIO_IN_CHECKBOX + MAX_CAMERA_ON_PAGE)))
            {
                perCamAudioInCheckBoxStatus[(checkBoxindex - USR_ACC_MAN_PERCAM_AUDIO_IN_CHECKBOX) + (currentPageNum*MAX_CAMERA_ON_PAGE)]
                        = (status == ON_STATE ? true : false);
            }
            else if ((checkBoxindex >= USR_ACC_MAN_PERCAM_AUDIO_OUT_CHECKBOX) && (checkBoxindex < (USR_ACC_MAN_PERCAM_AUDIO_OUT_CHECKBOX + MAX_CAMERA_ON_PAGE)))
            {
                perCamAudioOutCheckBoxStatus[(checkBoxindex - USR_ACC_MAN_PERCAM_AUDIO_OUT_CHECKBOX) + (currentPageNum*MAX_CAMERA_ON_PAGE)]
                        = (status == ON_STATE ? true : false);
            }
            else if ((checkBoxindex >= USR_ACC_MAN_PERCAM_VIDEO_POPUP_CHECKBOX) && (checkBoxindex < (USR_ACC_MAN_PERCAM_VIDEO_POPUP_CHECKBOX + MAX_CAMERA_ON_PAGE)))
            {
                perCamVideoPopupCheckBoxStatus[(checkBoxindex - USR_ACC_MAN_PERCAM_VIDEO_POPUP_CHECKBOX) + (currentPageNum*MAX_CAMERA_ON_PAGE)]
                        = (status == ON_STATE ? true : false);
            }
        }
        break;
    }
    setAllCheckBox();
}

void UserAccountManagment::slotPageNumberButtonClick(QString str)
{
    currentPageNum = ((quint8)str.toUInt() - 1);
    showFieldData();
}
