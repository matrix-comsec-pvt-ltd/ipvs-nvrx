#include "CameraSettings.h"
#include "Layout/Layout.h"
#include "ValidationMessage.h"

#define INNER_TILES_MARGIN              10
#define MAX_MOTION_INFO_CONFIG_OPTION   5
#define AUTO_ADD_IMAGE_PATH             ":/Images_Nvrx/CameraListIcon/AutoAddCamera.png"
#define LEFT_MARGIN_FROM_CENTER         SCALE_WIDTH(75)

typedef enum
{
    CAM_SET_CAM_NAME_SPINBOX,
    CAM_SET_CAM_TEST_BUTTON,
    CAM_SET_CAM_ENABLE_CHECKBOX,
    CAM_SET_CAM_NAME_TEXTBOX,
    CAM_SET_CAM_LOG_MOTION_CHECKBOX,
    CAM_SET_MOTION_DELAY_TEXTBOX,
    CAM_SET_MOBILENUMBER_TEXTBOX,
    CAM_SET_RECORDING_STREAM_MAIN,
    CAM_SET_RECORDING_STREAM_SUB,
    CAM_SET_CAM_MOTION_DETECTION_CHECKBOX,
    CAM_SET_CAM_MOTION_DETECTION_PAGEOPEN,
    CAM_MOTION_DETECTION_COPY_TO_CAMERA,
    CAM_SET_CAM_PRIVACY_MASK_CHECKBOX,
    CAM_SET_CAM_PRIVACY_MASK_PAGEOPEN,
    CAM_SET_CAM_OSD_PAGEOPEN,
    CAM_SET_ADDRESS_TEXTBOX,
    CAM_SET_IP_CHANGE_SET_BUTTON,
    CAM_SET_HTTP_PORT_TEXTBOX,
    CAM_SET_RTSP_PORT_TEXTBOX,
    CAM_SET_ONVIF_PORT_TEXTBOX,
    CAM_SET_URL_TEXTBOX,
    CAM_SET_ONVIF_CHECKBOX,
    CAM_SET_PROTOCOL_SPINBOX,
    CAM_SET_BRAND_SPINBOX,
    CAM_SET_MODEL_SPINBOX,
    CAM_SET_USERNAME_TEXTBOX,
    CAM_SET_PASSWORD_TEXTBOX,
    MAX_CAM_SET_CTRL
}CAM_SET_CTRL_e;

static const QStringList cameraTypeList = QStringList() << "Analog Camera" << "IP Camera" << "IP Camera";

static const QString cameraSettingsStrings[] =
{
    "Camera",
    "",
    "Enable",
    "Name",
    "Log Motion Events",
    "Motion Re-Detection Delay",
    "Contact No",
    "Recording Stream",
    "Sub",
    "Motion Detection",
    "Set",
    "Copy",
    "Privacy Mask",
    "Set",
    "OSD",
    "Camera Address",
    "",
    "HTTP Port",
    "RTSP Port",
    "ONVIF Port",
    "URL",
    "ONVIF Support",
    "Protocol",
    "Brand",
    "Model",
    "Username",
    "Password",
};

static const QMap<quint8, QString> protocolMapList =
{
    {0, "RTSP over TCP"},
    {1, "RTSP over UDP"},
    {2, "RTSP over HTTP"},
};

quint8 Layout::maxSupportedPrivacyMaskWindow = 0;

CameraSettings::CameraSettings(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent, MAX_CAM_SET_CTRL, devTabInfo), isCameraEnable(0), isCameraConfigured(0)
{
    INIT_OBJ(testCamera);
    INIT_OBJ(m_copyToCamera);
    INIT_OBJ(ipAddressChange);
    INIT_OBJ(cameraOsdSettings);
    INIT_OBJ(m_ipChangeProcessRequest);
    INIT_OBJ(osdSettingsPageOpenButton);
    INIT_OBJ(cameraMacAddressReadOnly);
    currentCameraType = MAX_CAMERA_TYPE;

    m_cameraListUpdateTimer = new QTimer();
    connect(m_cameraListUpdateTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotcameraListUpdateTimerTimeout()));
    m_cameraListUpdateTimer->setInterval(4000); // previously timer was set to 3 sec. It is changed to 4 sec to avoid possible communication gliches.
    m_cameraListUpdateTimer->setSingleShot(true);

    isMotionAreaSetAllow = false;
    isPrivacyAreaSetAllow = false;
    isIpChngEnable = false;

    m_camMacAddr.clear();
    createDefaultElements();

    m_currentCameraIndex = 1;
    CameraSettings::getConfig();

    this->setMouseTracking(true);
    this->installEventFilter(this);
    this->show();
}

CameraSettings::~CameraSettings()
{
    if(m_cameraListUpdateTimer->isActive())
    {
        m_cameraListUpdateTimer->stop();
    }
    DELETE_OBJ(m_cameraListUpdateTimer);

    if(IS_VALID_OBJ(cameraNumberDropDownBox))
    {
        disconnect(cameraNumberDropDownBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(cameraNumberDropDownBox,
                   SIGNAL(sigValueChanged(QString,quint32)),
                   this,
                   SLOT(slotSpinBoxValueChanged(QString,quint32)));
        DELETE_OBJ(cameraNumberDropDownBox);
    }

    if(IS_VALID_OBJ(testCameraButton))
    {
        disconnect(testCameraButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(testCameraButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(testCameraButton);
    }

    DELETE_OBJ(cameraTypeReadOnly);
    if(IS_VALID_OBJ(cameraNameTextbox))
    {
        disconnect(cameraNameTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(cameraNameTextbox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(cameraNameTextbox);
    }
    DELETE_OBJ(cameraNameTextboxParam);

    if(IS_VALID_OBJ(enableCameraCheckBox))
    {
        disconnect(enableCameraCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(enableCameraCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(enableCameraCheckBox);
    }

    if(IS_VALID_OBJ(logMotionDetectionCheckBox))
    {
        disconnect(logMotionDetectionCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(logMotionDetectionCheckBox);
    }

    if(IS_VALID_OBJ(osdSettingsPageOpenButton))
    {
        disconnect(osdSettingsPageOpenButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(osdSettingsPageOpenButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(osdSettingsPageOpenButton);
    }

    if(IS_VALID_OBJ(motionDelayTextbox))
    {
        disconnect(motionDelayTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(motionDelayTextbox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(motionDelayTextbox);
    }
    DELETE_OBJ(motionDelayTextboxParam);

    if(IS_VALID_OBJ(mobileNumberTextBox))
    {
        disconnect(mobileNumberTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(mobileNumberTextBox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(mobileNumberTextBox);
    }
    DELETE_OBJ(mobileNumberParam);

    if(IS_VALID_OBJ(motionDetectionCheckBox))
    {
        disconnect(motionDetectionCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(motionDetectionCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(motionDetectionCheckBox);
    }

    if(IS_VALID_OBJ(motionDetectionSetButton))
    {
        disconnect(motionDetectionSetButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(motionDetectionSetButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(motionDetectionSetButton);
    }

    if(IS_VALID_OBJ(m_motionDetectionCopyToCamButton))
    {
        disconnect(m_motionDetectionCopyToCamButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        disconnect(m_motionDetectionCopyToCamButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_motionDetectionCopyToCamButton);
    }

    if(IS_VALID_OBJ(privacyMaskCheckBox))
    {
        disconnect(privacyMaskCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(privacyMaskCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(privacyMaskCheckBox);
    }

    if(IS_VALID_OBJ(privacyMaskSetButton))
    {
        disconnect(privacyMaskSetButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(privacyMaskSetButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(privacyMaskSetButton);
    }

    if(IS_VALID_OBJ(recordingStreamMainCheckBox))
    {
        disconnect(recordingStreamMainCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(recordingStreamMainCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(recordingStreamMainCheckBox);
    }

    if(IS_VALID_OBJ(recordingStreamSubCheckBox))
    {
        disconnect(recordingStreamSubCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(recordingStreamSubCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(recordingStreamSubCheckBox);
    }

    DELETE_OBJ(cameraMacAddressReadOnly);

    if(IS_VALID_OBJ(testCamera))
    {
        disconnect(testCamera,
                   SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                   this,
                   SLOT(slotCreateCMDRequest(SET_COMMAND_e,quint8)));
        disconnect(testCamera,
                   SIGNAL(sigDeleteObject(quint8)),
                   this,
                   SLOT(slotPopupPageDeleted(quint8)));
        DELETE_OBJ(testCamera);
    }

    if(IS_VALID_OBJ(cameraOsdSettings))
    {
        disconnect(cameraOsdSettings,
                   SIGNAL(sigDeleteObject(quint8)),
                   this,
                   SLOT(slotPopupPageDeleted(quint8)));
        DELETE_OBJ(cameraOsdSettings);
    }

    if(IS_VALID_OBJ(m_copyToCamera))
    {
        disconnect(m_copyToCamera,
                   SIGNAL(sigDeleteObject(quint8)),
                   this,
                   SLOT(slotPopupPageDeleted(quint8)));
        DELETE_OBJ(m_copyToCamera);
    }

    if(IS_VALID_OBJ(cameraAddressTextbox))
    {
        disconnect(cameraAddressTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(cameraAddressTextbox);
    }
    DELETE_OBJ(cameraAddressTextboxParam);

    if(IS_VALID_OBJ(onvifSupportCheckBox))
    {
        disconnect(onvifSupportCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(onvifSupportCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(onvifSupportCheckBox);
    }

    if(IS_VALID_OBJ(protocolSection))
    {
        disconnect(protocolSection,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(protocolSection);
    }

    DELETE_OBJ(autoAddCameraImage);

    if(IS_VALID_OBJ(autoAddToolTip))
    {
        disconnect(this,
                SIGNAL(sigToolTipShowHide(bool)),
                this,
                SLOT(slotToolTipShowHide(bool)));
        DELETE_OBJ(autoAddToolTip);
    }

    if(IS_VALID_OBJ(httpPortTextBox))
    {
        disconnect(httpPortTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(httpPortTextBox);
    }
    DELETE_OBJ(httpPortTextboxParam);

    if(IS_VALID_OBJ(brandNameDropdown))
    {
        disconnect(brandNameDropdown,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect(brandNameDropdown,
                SIGNAL(sigValueChanged(QString, quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString, quint32)));
        DELETE_OBJ(brandNameDropdown);
    }
    DELETE_OBJ(brandNameListParam);

    if(IS_VALID_OBJ(rtspPortTextBox))
    {
        disconnect(rtspPortTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(rtspPortTextBox);
    }
    DELETE_OBJ(rtspPortTextboxParam);

    if(IS_VALID_OBJ(modelNameDropdown))
    {
        disconnect(modelNameDropdown,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(modelNameDropdown,
                   SIGNAL(sigValueListEmpty(quint8)),
                   this,
                   SLOT(slotValueListEmpty(quint8)));
        DELETE_OBJ(modelNameDropdown);
    }
    DELETE_OBJ(modelNameListParam);

    if(IS_VALID_OBJ(onvifPortTextBox))
    {
        disconnect(onvifPortTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(onvifPortTextBox);
    }
    DELETE_OBJ(onvifPortTextboxParam);

    if(IS_VALID_OBJ(usernameTextBox))
    {
        disconnect(usernameTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(usernameTextBox);
    }
    DELETE_OBJ(usernameTextboxParam);

    if(IS_VALID_OBJ(urlTextBox))
    {
        disconnect(urlTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(urlTextBox);
    }
    DELETE_OBJ(urlTextboxParam);

    if(IS_VALID_OBJ(passwordTextBox))
    {
        disconnect(passwordTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(passwordTextBox);
    }
    DELETE_OBJ(passwordTextboxParam);

    if(IS_VALID_OBJ(ipAddressChange))
    {
        disconnect(ipAddressChange,
                   SIGNAL(sigObjectDelete()),
                   this,
                   SLOT(slotIpAddressChangeDelete()));
        disconnect(ipAddressChange,
                   SIGNAL(sigDataSelectedForIpChange(QString,QString,QString)),
                   this,
                   SLOT(slotIpAddressChangeData(QString,QString,QString)));
        DELETE_OBJ(ipAddressChange);
    }
    brandNameList.clear();
    modelNameList.clear();

    if(IS_VALID_OBJ(ipAddressChangeButton))
    {
        disconnect(ipAddressChangeButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(ipAddressChangeButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(ipAddressChangeButton);
    }
}

void CameraSettings::fillCameraList()
{
    cameraList.clear();

    for(quint8 index = 0; index < devTableInfo->totalCams ; index++)
    {
        QString tempStr = applController->GetCameraNameOfDevice(currDevName,index);

        if ((devTableInfo->totalCams > 10) && ((index + 1) < 10))
        {
            cameraList.insert(index, QString(" %1%2%3").arg(index + 1).arg(" : ").arg(tempStr));
        }
        else
        {
            cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg(tempStr));
        }
    }

    cameraNumberDropDownBox->setNewList(cameraList, (m_currentCameraIndex - 1));
}

void CameraSettings::createDefaultElements()
{
    quint16 leftMargin = ((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - (2 * BGTILE_MEDIUM_SIZE_WIDTH) - SCALE_WIDTH(INNER_TILES_MARGIN)) / 2);
    quint16 topMargin = (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) + SCALE_HEIGHT(30) - (7 * BGTILE_HEIGHT) - (6 * (BGTILE_HEIGHT + SCALE_HEIGHT(10)))) / 2;

    m_currentElement = CAM_SET_CAM_NAME_SPINBOX;
    memset(&m_motionDetectionCopyToCameraField, 0, sizeof(m_motionDetectionCopyToCameraField));

    cameraNumberDropDownBox = new DropDown(leftMargin,
                                           topMargin,
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           CAM_SET_CAM_NAME_SPINBOX,
                                           DROPDOWNBOX_SIZE_200,
                                           cameraSettingsStrings[CAM_SET_CAM_NAME_SPINBOX],
                                           cameraList,
                                           this,
                                           "",
                                           true,
                                           0,
                                           TOP_TABLE_LAYER);
    m_elementList[CAM_SET_CAM_NAME_SPINBOX] = cameraNumberDropDownBox;
    connect(cameraNumberDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChanged(QString,quint32)));
    connect(cameraNumberDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    fillCameraList();

    testCameraButton = new ControlButton(TEST_CAMERAS_BUTTON_INDEX,
                                         (cameraNumberDropDownBox->x() + SCALE_WIDTH(445)),
                                         (cameraNumberDropDownBox->y() + SCALE_HEIGHT(10)),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         this, NO_LAYER, -1,
                                         cameraSettingsStrings[CAM_SET_CAM_TEST_BUTTON],
                                         false,
                                         CAM_SET_CAM_TEST_BUTTON);
    m_elementList[CAM_SET_CAM_TEST_BUTTON] = testCameraButton;
    connect(testCameraButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(testCameraButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    cameraTypeReadOnly = new ReadOnlyElement((cameraNumberDropDownBox->x() + cameraNumberDropDownBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                             cameraNumberDropDownBox->y(),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             SCALE_WIDTH(180),
                                             READONLY_HEIGHT,
                                             "", this,
                                             TOP_TABLE_LAYER,
                                             -1, SCALE_WIDTH(10), "Type");

    cameraNameTextboxParam = new TextboxParam();
    cameraNameTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_CAM_NAME_TEXTBOX];
    cameraNameTextboxParam->maxChar = 16;
    cameraNameTextbox = new TextBox(leftMargin,
                                    (cameraNumberDropDownBox->y() + cameraNumberDropDownBox->height()),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    CAM_SET_CAM_NAME_TEXTBOX,
                                    TEXTBOX_LARGE,
                                    this,
                                    cameraNameTextboxParam,
                                    MIDDLE_TABLE_LAYER,
                                    true);
    m_elementList[CAM_SET_CAM_NAME_TEXTBOX] = cameraNameTextbox;
    connect(cameraNameTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(cameraNameTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    enableCameraCheckBox = new OptionSelectButton((cameraNameTextbox->x() + cameraNameTextbox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                                  cameraNameTextbox->y(),
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX, this,
                                                  MIDDLE_TABLE_LAYER,
                                                  cameraSettingsStrings[CAM_SET_CAM_ENABLE_CHECKBOX],
                                                  "", -1,
                                                  CAM_SET_CAM_ENABLE_CHECKBOX);
    m_elementList[CAM_SET_CAM_ENABLE_CHECKBOX] = enableCameraCheckBox;
    connect(enableCameraCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(enableCameraCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    logMotionDetectionCheckBox = new OptionSelectButton(leftMargin,
                                                        (cameraNameTextbox->y() + cameraNameTextbox->height()),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX, this,
                                                        MIDDLE_TABLE_LAYER,
                                                        cameraSettingsStrings[CAM_SET_CAM_LOG_MOTION_CHECKBOX],
                                                        "", -1,
                                                        CAM_SET_CAM_LOG_MOTION_CHECKBOX);
    m_elementList[CAM_SET_CAM_LOG_MOTION_CHECKBOX] = logMotionDetectionCheckBox;
    connect(logMotionDetectionCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    motionDetectionCheckBox = new OptionSelectButton((logMotionDetectionCheckBox->x() + logMotionDetectionCheckBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                                     logMotionDetectionCheckBox->y(),
                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     CHECK_BUTTON_INDEX,
                                                     this,
                                                     MIDDLE_TABLE_LAYER,
                                                     cameraSettingsStrings[CAM_SET_CAM_MOTION_DETECTION_CHECKBOX],
                                                     "", -1,
                                                     (CAM_SET_CAM_MOTION_DETECTION_CHECKBOX));
    m_elementList[CAM_SET_CAM_MOTION_DETECTION_CHECKBOX] = motionDetectionCheckBox;
    connect(motionDetectionCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(motionDetectionCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    motionDetectionSetButton = new PageOpenButton((motionDetectionCheckBox->x() + SCALE_WIDTH(280)),
                                                  motionDetectionCheckBox->y(),
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  (CAM_SET_CAM_MOTION_DETECTION_PAGEOPEN),
                                                  PAGEOPENBUTTON_SMALL,
                                                  cameraSettingsStrings[CAM_SET_CAM_MOTION_DETECTION_PAGEOPEN],
                                                  this, "", "",
                                                  false, 0, NO_LAYER,
                                                  false);
    m_elementList[CAM_SET_CAM_MOTION_DETECTION_PAGEOPEN] = motionDetectionSetButton;
    connect(motionDetectionSetButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(motionDetectionSetButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    //Copy to camera button
    m_motionDetectionCopyToCamButton = new PageOpenButton((motionDetectionCheckBox->x() + SCALE_WIDTH(350)),
                                           motionDetectionCheckBox->y(),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           CAM_MOTION_DETECTION_COPY_TO_CAMERA,
                                           PAGEOPENBUTTON_MEDIAM_NEXT,
                                           cameraSettingsStrings[CAM_MOTION_DETECTION_COPY_TO_CAMERA],
                                           this,
                                           "","",
                                           false,
                                           0,
                                           NO_LAYER, false);
    m_elementList[CAM_MOTION_DETECTION_COPY_TO_CAMERA] = m_motionDetectionCopyToCamButton;
    connect(m_motionDetectionCopyToCamButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_motionDetectionCopyToCamButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    motionDelayTextboxParam = new TextboxParam();
    motionDelayTextboxParam->isNumEntry =  true;
    motionDelayTextboxParam->minNumValue = 5;
    motionDelayTextboxParam->maxNumValue = 30;
    motionDelayTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_MOTION_DELAY_TEXTBOX];
    motionDelayTextboxParam->suffixStr = "(5-30 sec)";
    motionDelayTextboxParam->maxChar = 2;
    motionDelayTextbox = new TextBox(leftMargin,
                                     (logMotionDetectionCheckBox->y() + logMotionDetectionCheckBox->height()),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     CAM_SET_MOTION_DELAY_TEXTBOX,
                                     TEXTBOX_EXTRASMALL,
                                     this,
                                     motionDelayTextboxParam,
                                     MIDDLE_TABLE_LAYER);
    m_elementList[CAM_SET_MOTION_DELAY_TEXTBOX] = motionDelayTextbox;
    connect(motionDelayTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(motionDelayTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    privacyMaskCheckBox = new OptionSelectButton((motionDelayTextbox->x() + motionDelayTextbox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                                 motionDelayTextbox->y(),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 MIDDLE_TABLE_LAYER,
                                                 cameraSettingsStrings[CAM_SET_CAM_PRIVACY_MASK_CHECKBOX],
                                                 "", -1,
                                                 CAM_SET_CAM_PRIVACY_MASK_CHECKBOX);
    m_elementList[CAM_SET_CAM_PRIVACY_MASK_CHECKBOX] = privacyMaskCheckBox;
    connect(privacyMaskCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(privacyMaskCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    privacyMaskSetButton = new PageOpenButton((privacyMaskCheckBox->x() + SCALE_WIDTH(280)),
                                              privacyMaskCheckBox->y(),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              CAM_SET_CAM_PRIVACY_MASK_PAGEOPEN,
                                              PAGEOPENBUTTON_SMALL,
                                              cameraSettingsStrings[CAM_SET_CAM_PRIVACY_MASK_PAGEOPEN],
                                              this, "", "",
                                              false, 0, NO_LAYER,
                                              ((currDevName == LOCAL_DEVICE_NAME)
                                               && (Layout::currentDisplayConfig[MAIN_DISPLAY].seqStatus == false)));
    m_elementList[CAM_SET_CAM_PRIVACY_MASK_PAGEOPEN] = privacyMaskSetButton;
    connect(privacyMaskSetButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(privacyMaskSetButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    mobileNumberParam = new TextboxParam();
    mobileNumberParam->isNumEntry = false;
    mobileNumberParam->maxChar = 14;
    mobileNumberParam->minChar = 1;
    mobileNumberParam->labelStr = cameraSettingsStrings[CAM_SET_MOBILENUMBER_TEXTBOX];
    mobileNumberParam->validation = QRegExp(QString("[0-9]"));
    mobileNumberParam->isTotalBlankStrAllow = true;

    mobileNumberTextBox = new TextBox(leftMargin,
                                      (motionDelayTextbox->y() + motionDelayTextbox->height()),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      CAM_SET_MOBILENUMBER_TEXTBOX,
                                      TEXTBOX_LARGE,
                                      this,
                                      mobileNumberParam,
                                      MIDDLE_TABLE_LAYER);
    m_elementList[CAM_SET_MOBILENUMBER_TEXTBOX] = mobileNumberTextBox;
    connect(mobileNumberTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(mobileNumberTextBox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    recordingStreamMainCheckBox = new OptionSelectButton(leftMargin,
                                                         (mobileNumberTextBox->y() +
                                                          mobileNumberTextBox->height()),
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         RADIO_BUTTON_INDEX,
                                                         this,
                                                         BOTTOM_TABLE_LAYER,
                                                         cameraSettingsStrings[CAM_SET_RECORDING_STREAM_MAIN],
                                                         "Main",
                                                         -1,
                                                         (CAM_SET_RECORDING_STREAM_MAIN),
                                                         true,
                                                         NORMAL_FONT_SIZE,
                                                         NORMAL_FONT_COLOR);
    m_elementList[CAM_SET_RECORDING_STREAM_MAIN] = recordingStreamMainCheckBox;
    connect(recordingStreamMainCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(recordingStreamMainCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    recordingStreamSubCheckBox = new OptionSelectButton(recordingStreamMainCheckBox->x() + SCALE_WIDTH(345),
                                                        recordingStreamMainCheckBox->y(),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        RADIO_BUTTON_INDEX,
                                                        this,
                                                        NO_LAYER,
                                                        "",
                                                        cameraSettingsStrings[CAM_SET_RECORDING_STREAM_SUB],
                                                        SCALE_WIDTH(50),
                                                        (CAM_SET_RECORDING_STREAM_SUB),
                                                        true,
                                                        NORMAL_FONT_SIZE,
                                                        NORMAL_FONT_COLOR, true);
    m_elementList[CAM_SET_RECORDING_STREAM_SUB] = recordingStreamSubCheckBox;
    connect(recordingStreamSubCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(recordingStreamSubCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    /* Create last tile(OSD or MAC address) based on camera type */
    createOsdSetting(IP_CAMERA);

    /* create default camera address textbox */
    cameraAddressTextboxParam = new TextboxParam();
    cameraAddressTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_ADDRESS_TEXTBOX];
    cameraAddressTextboxParam->maxChar = 40;
    cameraAddressTextboxParam->validation = QRegExp(QString("[^\\ ]"));
    cameraAddressTextbox = new TextBox(leftMargin,
                                       (recordingStreamMainCheckBox->y() + recordingStreamMainCheckBox->height() + SCALE_HEIGHT(INNER_TILES_MARGIN)),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       CAM_SET_ADDRESS_TEXTBOX,
                                       TEXTBOX_ULTRALARGE,
                                       this,
                                       cameraAddressTextboxParam,
                                       TOP_TABLE_LAYER, true, false, false,
                                       LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_ADDRESS_TEXTBOX] = cameraAddressTextbox;
    connect(cameraAddressTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default ip address change button */
    ipAddressChangeButton = new ControlButton(SET_BUTTON_INDEX,
                                              cameraAddressTextbox->x() + SCALE_WIDTH(443),
                                              cameraAddressTextbox->y() + SCALE_HEIGHT(10),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              this,
                                              NO_LAYER,
                                              -1,"", false,
                                              CAM_SET_IP_CHANGE_SET_BUTTON);
    m_elementList[CAM_SET_IP_CHANGE_SET_BUTTON] = ipAddressChangeButton;
    connect(ipAddressChangeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(ipAddressChangeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    /* create default auto-added camera icon */
    autoAddCameraImage = new Image(cameraAddressTextbox->x() + SCALE_WIDTH(445),
                                   cameraAddressTextbox->y() + SCALE_WIDTH(15),
                                   AUTO_ADD_IMAGE_PATH,
                                   this,
                                   START_X_START_Y,
                                   0,
                                   false,
                                   true,
                                   true);
    autoAddCameraImage->setVisible(false);

    /* create default onvif checkbox */
    onvifSupportCheckBox = new OptionSelectButton((cameraAddressTextbox->x() +
                                                  cameraAddressTextbox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                                  (recordingStreamMainCheckBox->y() + recordingStreamMainCheckBox->height() + SCALE_HEIGHT(INNER_TILES_MARGIN)),
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  TOP_TABLE_LAYER,
                                                  cameraSettingsStrings[CAM_SET_ONVIF_CHECKBOX],
                                                  "", -1,
                                                  CAM_SET_ONVIF_CHECKBOX, true, -1, SUFFIX_FONT_COLOR,
                                                  false, LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_ONVIF_CHECKBOX] = onvifSupportCheckBox;
    connect(onvifSupportCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(onvifSupportCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    /* create default http port field */
    httpPortTextboxParam = new TextboxParam();
    httpPortTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_HTTP_PORT_TEXTBOX];
    httpPortTextboxParam->suffixStr = "(1-65535)";
    httpPortTextboxParam->textStr = "80";
    httpPortTextboxParam->isNumEntry = true;
    httpPortTextboxParam->minNumValue = 1;
    httpPortTextboxParam->maxNumValue = 65535;
    httpPortTextboxParam->maxChar = 5;

    httpPortTextBox = new TextBox(leftMargin,
                                  (cameraAddressTextbox->y() + cameraAddressTextbox->height()),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  CAM_SET_HTTP_PORT_TEXTBOX,
                                  TEXTBOX_SMALL,
                                  this,
                                  httpPortTextboxParam,
                                  MIDDLE_TABLE_LAYER, true, false, false,
                                  LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_HTTP_PORT_TEXTBOX] = httpPortTextBox;
    connect(httpPortTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default protocol dropdown */
    protocolSection = new DropDown((httpPortTextBox->x() + httpPortTextBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                   httpPortTextBox->y(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   CAM_SET_PROTOCOL_SPINBOX,
                                   DROPDOWNBOX_SIZE_200,
                                   cameraSettingsStrings[CAM_SET_PROTOCOL_SPINBOX],
                                   protocolMapList,
                                   this,
                                   "",true,0,
                                   MIDDLE_TABLE_LAYER, true, 8, false, false, 5,
                                   LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_PROTOCOL_SPINBOX] = protocolSection;
    connect(protocolSection,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default onvif port field */
    onvifPortTextboxParam = new TextboxParam();
    onvifPortTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_ONVIF_PORT_TEXTBOX];
    onvifPortTextboxParam->suffixStr = "(1-65535)";
    onvifPortTextboxParam->textStr = "80";
    onvifPortTextboxParam->isNumEntry = true;
    onvifPortTextboxParam->minNumValue = 1;
    onvifPortTextboxParam->maxNumValue = 65535;
    onvifPortTextboxParam->maxChar = 5;

    onvifPortTextBox = new TextBox(leftMargin,
                                   (httpPortTextBox->y() + httpPortTextBox->height()),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   CAM_SET_ONVIF_PORT_TEXTBOX,
                                   TEXTBOX_SMALL,
                                   this,
                                   onvifPortTextboxParam,
                                   MIDDLE_TABLE_LAYER, true, false, false,
                                   LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_ONVIF_PORT_TEXTBOX] = onvifPortTextBox;
    connect(onvifPortTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default brand dropdown */
    brandNameList.clear();
    brandNameList.insert(0, "");

    brandNameListParam = new TextboxParam();
    brandNameListParam->labelStr = cameraSettingsStrings[CAM_SET_BRAND_SPINBOX];
    brandNameListParam->textStr = brandNameList.value(0);
    brandNameListParam->isCentre = true;
    brandNameListParam->leftMargin = SCALE_WIDTH(20);
    brandNameListParam->maxChar = 30;
    brandNameListParam->isTotalBlankStrAllow = true;
    brandNameListParam->validation = QRegExp(QString("[a-zA-Z0-9]"));

    brandNameDropdown = new TextWithList(onvifPortTextBox->x() + onvifPortTextBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN),
                                         onvifPortTextBox->y(),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CAM_SET_BRAND_SPINBOX,
                                         brandNameList, this,
                                         brandNameListParam, MIDDLE_TABLE_LAYER, true, 6,
                                         TEXTBOX_ULTRALARGE, "Search", false,
                                         LIST_FILTER_TYPE_ANY_CHAR,
                                         LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_BRAND_SPINBOX] = brandNameDropdown;
    connect(brandNameDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(brandNameDropdown,
            SIGNAL(sigValueChanged(QString, quint32)),
            this,
            SLOT(slotSpinBoxValueChanged(QString, quint32)));

    /* create default rtsp port field */
    rtspPortTextboxParam = new TextboxParam();
    rtspPortTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_RTSP_PORT_TEXTBOX];
    rtspPortTextboxParam->suffixStr = "(1-65535)";
    rtspPortTextboxParam->textStr = "554";
    rtspPortTextboxParam->isNumEntry = true;
    rtspPortTextboxParam->minNumValue = 1;
    rtspPortTextboxParam->maxNumValue = 65535;
    rtspPortTextboxParam->maxChar = 5;

    rtspPortTextBox = new TextBox(leftMargin,
                                  (onvifPortTextBox->y() + onvifPortTextBox->height()),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  CAM_SET_RTSP_PORT_TEXTBOX,
                                  TEXTBOX_SMALL,
                                  this,
                                  rtspPortTextboxParam,
                                  MIDDLE_TABLE_LAYER, true, false, false,
                                  LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_RTSP_PORT_TEXTBOX] = rtspPortTextBox;
    connect(rtspPortTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default model dropdown */
    modelNameList.clear();
    modelNameList.insert(0, "");

    modelNameListParam = new TextboxParam();
    modelNameListParam->labelStr = cameraSettingsStrings[CAM_SET_MODEL_SPINBOX];
    modelNameListParam->textStr = modelNameList.value(0);
    modelNameListParam->isCentre = true;
    modelNameListParam->leftMargin = SCALE_WIDTH(20);
    modelNameListParam->maxChar = 30;
    modelNameListParam->isTotalBlankStrAllow = true;
    modelNameListParam->validation = QRegExp(QString("[a-zA-Z0-9]"));

    modelNameDropdown = new TextWithList((rtspPortTextBox->x() + rtspPortTextBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                         rtspPortTextBox->y(),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CAM_SET_MODEL_SPINBOX,
                                         modelNameList, this,
                                         modelNameListParam, MIDDLE_TABLE_LAYER, true, 6,
                                         TEXTBOX_ULTRALARGE, "Search", false,
                                         LIST_FILTER_TYPE_ANY_CHAR,
                                         LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_MODEL_SPINBOX] = modelNameDropdown;
    connect(modelNameDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(modelNameDropdown,
            SIGNAL(sigValueListEmpty(quint8)),
            this,
            SLOT(slotValueListEmpty(quint8)));

    /* create default url field */
    urlTextboxParam = new TextboxParam();
    urlTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_URL_TEXTBOX];
    urlTextboxParam->maxChar = 149;
    urlTextBox = new TextBox(leftMargin,
                             (rtspPortTextBox->y() + rtspPortTextBox->height()),
                             BGTILE_MEDIUM_SIZE_WIDTH,
                             BGTILE_HEIGHT,
                             CAM_SET_URL_TEXTBOX,
                             TEXTBOX_LARGE,
                             this,
                             urlTextboxParam,
                             BOTTOM_TABLE_LAYER,
                             false, false, false,
                             LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_URL_TEXTBOX] = urlTextBox;
    connect(urlTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default username field */
    usernameTextboxParam = new TextboxParam();
    usernameTextboxParam->maxChar = 24;
    usernameTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_USERNAME_TEXTBOX];
    usernameTextBox =  new TextBox((urlTextBox->x() + urlTextBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                   urlTextBox->y(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   CAM_SET_USERNAME_TEXTBOX,
                                   TEXTBOX_LARGE,
                                   this,
                                   usernameTextboxParam,
                                   MIDDLE_TABLE_LAYER, true, false, false,
                                   LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_USERNAME_TEXTBOX] = usernameTextBox;
    connect(usernameTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* create default password field */
    passwordTextboxParam = new TextboxParam();
    passwordTextboxParam->maxChar = 20;
    passwordTextboxParam->labelStr = cameraSettingsStrings[CAM_SET_PASSWORD_TEXTBOX];
    passwordTextboxParam->suffixStr = "(Max 20 chars)";
    passwordTextBox = new PasswordTextbox((urlTextBox->x() + urlTextBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                          usernameTextBox->y() + usernameTextBox->height(),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CAM_SET_PASSWORD_TEXTBOX,
                                          TEXTBOX_LARGE,
                                          this,
                                          passwordTextboxParam,
                                          BOTTOM_TABLE_LAYER, true,
                                          LEFT_MARGIN_FROM_CENTER);
    m_elementList[CAM_SET_PASSWORD_TEXTBOX] = passwordTextBox;
    connect(passwordTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    autoAddToolTip = new ToolTip(autoAddCameraImage->x(),
                                 autoAddCameraImage->y(),
                                 "Auto Added Camera",
                                 this,
                                 START_X_START_Y);
    autoAddToolTip->setVisible(false);

    connect(this,
            SIGNAL(sigToolTipShowHide(bool)),
            this,
            SLOT(slotToolTipShowHide(bool)));

    resetGeometryOfCnfgbuttonRow(SCALE_HEIGHT(30));
}

void CameraSettings::createCmdRequest(SET_COMMAND_e cmd, quint8 totalField)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmd;
    param->payload = payloadLib->createDevCmdPayload(totalField);

    if((!processBar->isVisible()) && (cmd != CHANGE_IP_CAM_ADDRS))
    {
        processBar->loadProcessBar();
    }
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraSettings::createPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             CAMERA_TABLE_INDEX,
                                                             m_currentCameraIndex,
                                                             m_currentCameraIndex,
                                                             1,
                                                             CAMERA_SETTINGS_FIELD_MAX,
                                                             CAMERA_SETTINGS_FIELD_MAX);

    payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                     IP_CAMERA_SETTING_TABLE_INDEX,
                                                     m_currentCameraIndex,
                                                     m_currentCameraIndex,
                                                     (IP_CAMERA_BRAND+1),
                                                     MAX_IP_CAMERA_SETTINGS_FIELDS,
                                                     MAX_IP_CAMERA_SETTINGS_FIELDS,
                                                     payloadString,
                                                     CAMERA_SETTINGS_FIELD_MAX);

    DevCommParam* param = new DevCommParam();
    param->msgType = requestType;
    param->payload = payloadString;

    if(!processBar->isVisible())
    {
        processBar->loadProcessBar();
    }
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraSettings::getConfig()
{
    fillCameraList();
    memset(&m_motionDetectionCopyToCameraField, 0, sizeof(m_motionDetectionCopyToCameraField));
    createPayload(MSG_GET_CFG);
}

void CameraSettings::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void CameraSettings::saveConfig()
{
    if(!(motionDelayTextbox->doneKeyValidation()))
    {
        return;
    }

    if((cameraNameTextbox->getInputText() == "") && (enableCameraCheckBox->getCurrentState()))
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_SETTING_CAM_NAME));
        return;
    }

    if ((currentCameraType != AUTO_ADD_IP_CAMERA) && (isCameraEnable))
    {
        if ((onvifSupportCheckBox->getCurrentState() == OFF_STATE) && (brandNameDropdown->getCurrValue(true) == ""))
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_VALID_BRAND_NM));
            return;
        }

        if ((onvifSupportCheckBox->getCurrentState() == OFF_STATE) && (modelNameDropdown->getCurrValue(true) == ""))
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_MODEL_NM));
            return;
        }

        if (cameraAddressTextbox->getInputText() == "")
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_CAM_ADD_LEFT_BLK));
            return;
        }

        if (usernameTextBox->getInputText() == "")
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VALID_USER));
            return;
        }

        if (passwordTextBox->getInputText() == "")
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_VALID_PASS));
            return;
        }
    }

    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ADDRESS), cameraAddressTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_HTTP_PORT), httpPortTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_RTSP_PORT), rtspPortTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ONVIF_PORT), onvifPortTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_URL), urlTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ONVIF_SUPPORT), onvifSupportCheckBox->getCurrentState());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_BRAND), brandNameDropdown->getCurrValue(true));
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_MODEL), modelNameDropdown->getCurrValue(true));
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_USERNAME), usernameTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_PASSWORD), passwordTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_PROTOCOL), protocolSection->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex((CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_MAC_ADDRESS), m_camMacAddr);

    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TYPE, currentCameraType);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_NAME, cameraNameTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_IS_ENABLE_STATUS, enableCameraCheckBox->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_LOG_DETECTION_STATUS, logMotionDetectionCheckBox->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_MOTION_REDETECTION_DELAY, motionDelayTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_DATE_TIME_OVERLAY, cameraOsdParams.dateTimeOverlay);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_DATE_TIME_POSITION, cameraOsdParams.dateTimePosition);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TEXT_OVERLAY, cameraOsdParams.textOverlay);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TEXT_0, cameraOsdParams.text[0]);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TEXT_POSITION_0, cameraOsdParams.textPosition[0]);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_NAME_OSD, cameraOsdParams.cameraNamePosition);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_STATUS_OSD, cameraOsdParams.cameraStatusPosition);
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_MOBILE_NUMBER, mobileNumberTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_PRIVACY_MASKING_STATUS, privacyMaskCheckBox->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_MOTION_DETECTION_STATUS, motionDetectionCheckBox->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_RECORDING_STREAM, (recordingStreamMainCheckBox->getCurrentState() == ON_STATE) ? 0 : 1);

    SET_CAMERA_MASK_BIT(m_motionDetectionCopyToCameraField, (m_currentCameraIndex - 1));
    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex(CAMERA_SETTINGS_MOTION_DETECTION_COPY_TO_CAMERA_START + maskIdx, m_motionDetectionCopyToCameraField.bitMask[maskIdx]);
    }

    for (quint8 index = 1, cnfgIdx = CAMERA_SETTINGS_CAMERA_TEXT_1; index < TEXT_OVERLAY_MAX; index++)
    {
        payloadLib->setCnfgArrayAtIndex(cnfgIdx++, cameraOsdParams.text[index]);
        payloadLib->setCnfgArrayAtIndex(cnfgIdx++, cameraOsdParams.textPosition[index]);
    }

    /* Reset CopytoCamFields */
    memset(&m_motionDetectionCopyToCameraField, 0, sizeof(m_motionDetectionCopyToCameraField));
    createPayload(MSG_SET_CFG);
}

void CameraSettings::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessBar = true;

    if (deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        if (param->msgType == MSG_SET_CMD)
        {
            if (param->cmdType == OTHR_SUP)
            {
                if (IS_VALID_OBJ(testCamera))
                {
                    testCamera->processDeviceResponse(param, deviceName);
                }

                isMotionAreaSetAllow = false;
                isPrivacyAreaSetAllow = false;
                processBar->unloadProcessBar();
                return;
            }

            if (param->cmdType == GET_PRIVACY_MASK_WINDOW)
            {
                for(quint8 index = 0; index < MAX_PRIVACYMASK_AREA; index++)
                {
                    privacyMask[index].startX = 0;
                    privacyMask[index].startY = 0;
                    privacyMask[index].width = 0;
                    privacyMask[index].height = 0;
                }
            }
            else if (param->cmdType == CHANGE_IP_CAM_ADDRS)
            {
                DELETE_OBJ(m_ipChangeProcessRequest);
                if(param->deviceStatus == CMD_IP_ADDRESS_CHANGE_FAIL)
                {
                    infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_IP_ADDRESS_CHANGE_FAIL) + QString("%1").arg(m_currentCameraIndex));
                    return;
                }
                else if(param->deviceStatus == CMD_CAM_REBOOT_FAILED)
                {
                    infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_CAM_REBOOT_FAILED) + QString("%1").arg(m_currentCameraIndex));
                    return;
                }

                isUnloadProcessBar = false;
            }
        }

        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        if (true == isUnloadProcessBar)
        {
            processBar->unloadProcessBar();
        }
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex(0) == CAMERA_TABLE_INDEX)
            {
                currentCameraType = (CAMERA_TYPE_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TYPE).toUInt();
                if ((currentCameraType != IP_CAMERA) && (currentCameraType != AUTO_ADD_IP_CAMERA))
                {
                    currentCameraType = IP_CAMERA;
                }
                cameraTypeReadOnly->changeValue((currentCameraType < MAX_CAMERA_TYPE) ? cameraTypeList.at(currentCameraType) : "-");

                /* Create last tile(OSD or MAC address) based on camera type */
                createOsdSetting(currentCameraType);

                cameraNameTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_NAME).toString());
                currentCameraName = payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_NAME).toString();

                enableCameraCheckBox->changeState(((payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_IS_ENABLE_STATUS).toInt() == 0) ? OFF_STATE : ON_STATE));

                isCameraEnable = payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_IS_ENABLE_STATUS).toBool();
                isCameraConfigured = isCameraEnable;
                testCameraButton->setIsEnabled(isCameraEnable);
                osdSettingsPageOpenButton->setIsEnabled(isCameraEnable);

                recordingStreamMainCheckBox->setIsEnabled(isCameraEnable);
                recordingStreamSubCheckBox->setIsEnabled(isCameraEnable);

                cameraNameTextbox->setIsEnabled(isCameraEnable);
                logMotionDetectionCheckBox->setIsEnabled(isCameraEnable);
                motionDelayTextbox->setIsEnabled(isCameraEnable);
                mobileNumberTextBox->setIsEnabled(isCameraEnable);
                mobileNumberTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_MOBILE_NUMBER).toString());

                logMotionDetectionCheckBox->changeState(((payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_LOG_DETECTION_STATUS) == 0) ? OFF_STATE : ON_STATE));
                motionDelayTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_MOTION_REDETECTION_DELAY).toString());

                cameraOsdParams.dateTimeOverlay = (OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_DATE_TIME_OVERLAY).toUInt();
                cameraOsdParams.dateTimePosition = (OSD_POSITION_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_DATE_TIME_POSITION).toUInt();
                cameraOsdParams.textOverlay = (OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TEXT_OVERLAY).toUInt();
                cameraOsdParams.text[0] = payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TEXT_0).toString();
                cameraOsdParams.textPosition[0] = (OSD_POSITION_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_TEXT_POSITION_0).toUInt();
                cameraOsdParams.cameraNamePosition = (OSD_POSITION_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_NAME_OSD).toUInt();
                cameraOsdParams.cameraStatusPosition = (OSD_POSITION_e)payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_CAMERA_STATUS_OSD).toUInt();
                cameraOsdParams.cameraType = currentCameraType;
                for (quint8 index = 1, cnfgIdx = CAMERA_SETTINGS_CAMERA_TEXT_1; index < TEXT_OVERLAY_MAX; index++)
                {
                    cameraOsdParams.text[index] = payloadLib->getCnfgArrayAtIndex(cnfgIdx++).toString();
                    cameraOsdParams.textPosition[index] = (OSD_POSITION_e)payloadLib->getCnfgArrayAtIndex(cnfgIdx++).toUInt();
                }

                bool privacymaskStatus = ((payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_PRIVACY_MASKING_STATUS).toInt() != 0));
                privacyMaskCheckBox->changeState(((payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_PRIVACY_MASKING_STATUS).toInt() == 1) ? ON_STATE : OFF_STATE));
                privacyMaskCheckBox->setIsEnabled(isCameraEnable);

                privacyMaskSetButton->setIsEnabled(privacyMaskCheckBox->getCurrentState() == OFF_STATE ? false: ((privacymaskStatus) && (isCameraEnable == true)));

                bool motiondetectionStatus = ((payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_MOTION_DETECTION_STATUS).toInt() != 0));
                motionDetectionCheckBox->changeState(((payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_MOTION_DETECTION_STATUS).toInt() == 1) ? ON_STATE : OFF_STATE));
                motionDetectionCheckBox->setIsEnabled(isCameraEnable);
                m_motionDetectionCopyToCamButton->setIsEnabled(isCameraEnable);
                motionDetectionSetButton->setIsEnabled(motionDetectionCheckBox->getCurrentState() == OFF_STATE ? false: ((motiondetectionStatus) && (isCameraEnable == true)));
                if(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_RECORDING_STREAM).toUInt() == 0)
                {
                    recordingStreamMainCheckBox->changeState(ON_STATE);
                    recordingStreamSubCheckBox->changeState(OFF_STATE);
                }
                else
                {
                    recordingStreamMainCheckBox->changeState(OFF_STATE);
                    recordingStreamSubCheckBox->changeState(ON_STATE);
                }

                enableIpCameraCntrls();
            }

            if((payloadLib->getcnfgTableIndex(1) == IP_CAMERA_SETTING_TABLE_INDEX))
            {
                if(currentCameraType == IP_CAMERA)
                {
                    cameraAddressTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ADDRESS).toString());
                    autoAddCameraImage->setVisible(false);
                    autoAddCameraImage->setIsEnabled(false);

                    autoAddToolTip->setVisible(false);

                    httpPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_HTTP_PORT).toString());
                    rtspPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_RTSP_PORT).toString());
                    onvifPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ONVIF_PORT).toString());
                    urlTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_URL).toString());

                    onvifSupportCheckBox->changeState(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ONVIF_SUPPORT).toUInt() == 1 ? ON_STATE : OFF_STATE);

                    QString brandName = payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_BRAND).toString();
                    brandNameList.clear();
                    brandNameList.insert(0, brandName);
                    brandNameDropdown->setNewList(brandNameList, 0, (brandName == ""), true);

                    QString modelName = payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_MODEL).toString();
                    modelNameList.clear();
                    modelNameList.insert(0, modelName);
                    modelNameDropdown->setNewList(modelNameList, 0, (modelName == ""), true);

                    usernameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_USERNAME).toString());
                    passwordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_PASSWORD).toString());

                    urlTextBox->setIsEnabled(((brandName == "GENERIC") && (isCameraEnable))? true : false);

                    if(onvifSupportCheckBox->getCurrentState() == ON_STATE)
                    {
                        brandNameDropdown->setIsEnabled(false);
                        modelNameDropdown->setIsEnabled(false);
                        httpPortTextBox->setIsEnabled(false);
                        protocolSection->setIsEnabled(isCameraEnable);
                        onvifPortTextBox->setIsEnabled(isCameraEnable);
                    }
                    else
                    {
                        brandNameDropdown->setIsEnabled(isCameraEnable);
                        modelNameDropdown->setIsEnabled(isCameraEnable);
                        httpPortTextBox->setIsEnabled(isCameraEnable);
                        protocolSection->setIsEnabled(false);
                        onvifPortTextBox->setIsEnabled(false);
                    }

                    protocolSection->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_PROTOCOL).toUInt());

                    enableChangeIpButtonState(isCameraEnable);
                    isIpChngEnable = ipAddressChangeButton->getIsEnabled();

                    m_camMacAddr.clear();
                    cameraMacAddressReadOnly->setVisible(false);

                    if (onvifSupportCheckBox->getCurrentState() == OFF_STATE)
                    {
                        createCmdRequest(BRND_NAME, 0);
                    }
                }
                else if(currentCameraType == AUTO_ADD_IP_CAMERA)
                {
                    cameraAddressTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_ADDRESS).toString());
                    cameraAddressTextbox->setIsEnabled(false);

                    httpPortTextBox->setInputText("");
                    httpPortTextBox->setIsEnabled(false);
                    rtspPortTextBox->setInputText("");
                    rtspPortTextBox->setIsEnabled(false);
                    onvifPortTextBox->setInputText("");
                    onvifPortTextBox->setIsEnabled(false);
                    urlTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_URL).toString());
                    urlTextBox->setIsEnabled(false);
                    onvifSupportCheckBox->changeState(OFF_STATE);
                    onvifSupportCheckBox->setIsEnabled(false);

                    brandNameList.clear();
                    brandNameList.insert(0, payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_BRAND).toString());
                    brandNameDropdown->setNewList(brandNameList, 0, false, true);
                    brandNameDropdown->setIsEnabled(false);

                    modelNameList.clear();
                    modelNameList.insert(0, payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_MODEL).toString());
                    modelNameDropdown->setNewList(modelNameList, 0, false, true);
                    modelNameDropdown->setIsEnabled(false);

                    usernameTextBox->setInputText("");
                    usernameTextBox->setIsEnabled(false);
                    passwordTextBox->setInputText("");
                    passwordTextBox->setIsEnabled(false);

                    ipAddressChangeButton->setVisible(false);
                    urlTextBox->setInputText("");

                    urlTextBox->setIsEnabled(false);

                    protocolSection->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_PROTOCOL).toUInt());
                    protocolSection->setIsEnabled(false);

                    autoAddCameraImage->setVisible(true);
                    autoAddCameraImage->setIsEnabled(true);

                    m_camMacAddr = payloadLib->getCnfgArrayAtIndex(CAMERA_SETTINGS_FIELD_MAX + IP_CAMERA_MAC_ADDRESS).toString();
                    cameraMacAddressReadOnly->changeValue(m_camMacAddr);
                    cameraMacAddressReadOnly->setVisible(true);
                }
                payloadLib->setCnfgArrayAtIndex(0, m_currentCameraIndex);
                createCmdRequest(OTHR_SUP, 1);
            }
        }
        break;

        case MSG_DEF_CFG:
        {
            isUnloadProcessBar = false;
            m_currentElement = CNFG_DEFAULT_BTN;
            getConfig();

            if(m_cameraListUpdateTimer->isActive())
            {
                m_cameraListUpdateTimer->stop();
            }
            m_cameraListUpdateTimer->start();
        }
        break;

        case MSG_SET_CFG:
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            currentCameraName = cameraNameTextbox->getInputText();

            getConfig();
            if(m_cameraListUpdateTimer->isActive())
            {
                m_cameraListUpdateTimer->stop();
            }
            m_cameraListUpdateTimer->start();

            if((currentCameraType == IP_CAMERA) || (currentCameraType == AUTO_ADD_IP_CAMERA))
            {
                enableChangeIpButtonState(isCameraEnable);
            }
        }
        break;

        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
                case TST_CAM:
                {
                    if (IS_VALID_OBJ(testCamera))
                    {
                        break;
                    }

                    testCamera = new TestCamera(m_currentCameraIndex,
                                                currentCameraName,
                                                payloadLib,
                                                parentWidget(),
                                                CAM_SET_CAM_TEST_BUTTON,
                                                currentCameraType);
                    connect(testCamera,
                            SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                            this,
                            SLOT(slotCreateCMDRequest(SET_COMMAND_e,quint8)));
                    connect(testCamera,
                            SIGNAL(sigDeleteObject(quint8)),
                            this,
                            SLOT(slotPopupPageDeleted(quint8)));
                }
                break;

                case GET_CAPABILITY:
                {
                    if (IS_VALID_OBJ(cameraOsdSettings))
                    {
                        break;
                    }

                    payloadLib->parseDevCmdReply(true, param->payload);
                    CAPABILITY_CMD_ID_e capability = (CAPABILITY_CMD_ID_e)payloadLib->getCnfgArrayAtIndex(0).toInt();
                    switch(capability)
                    {
                        case CAPABILITY_CMD_ID_TEXT_OVERLAY:
                        {
                            qint32 textOverlayMax = payloadLib->getCnfgArrayAtIndex(1).toInt();
                            cameraOsdSettings = new CameraOSDSettings(&cameraOsdParams, CAM_SET_CAM_OSD_PAGEOPEN,
                                                                      textOverlayMax, parentWidget());
                            connect(cameraOsdSettings,
                                    SIGNAL(sigDeleteObject(quint8)),
                                    this,
                                    SLOT(slotPopupPageDeleted(quint8)));
                        }
                        break;

                        default:
                        {
                            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_PROCESS_ERROR));
                        }
                        break;
                    }
                }
                break;

                case OTHR_SUP:
                {
                    if (IS_VALID_OBJ(testCamera))
                    {
                        testCamera->processDeviceResponse(param, deviceName);
                    }

                    payloadLib->parseDevCmdReply(true, param->payload);
                    isMotionAreaSetAllow = payloadLib->getCnfgArrayAtIndex(6).toBool();
                    isPrivacyAreaSetAllow = payloadLib->getCnfgArrayAtIndex(7).toBool();
                }
                break;

                case BRND_NAME:
                {
                    if (currentCameraType != IP_CAMERA)
                    {
                        break;
                    }

                    payloadLib->parseDevCmdReply(true, param->payload);
                    QString selectedBrand = brandNameDropdown->getCurrValue(true);
                    quint8 maxIndex = payloadLib->getTotalCmdFields();
                    quint8 brandIndex = maxIndex;
                    brandNameList.clear();
                    for (quint8 index = 0; index < maxIndex; index++)
                    {
                        brandNameList.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString ());
                        if ((brandIndex == maxIndex) && (selectedBrand != "") && (selectedBrand == brandNameList.value(index)))
                        {
                            brandIndex = index;
                        }
                    }

                    brandNameDropdown->setNewList(brandNameList, brandIndex, (selectedBrand == ""));
                    if ((onvifSupportCheckBox->getCurrentState() == OFF_STATE) && (selectedBrand != ""))
                    {
                        getModelList();
                    }
                }
                break;

                case MDL_NAME:
                {
                    if (currentCameraType != IP_CAMERA)
                    {
                        break;
                    }

                    payloadLib->parseDevCmdReply(true, param->payload);
                    QString selectedModel = modelNameDropdown->getCurrValue(true);
                    quint8 maxIndex = payloadLib->getTotalCmdFields();
                    quint8 modelIndex = maxIndex;
                    modelNameList.clear();

                    for (quint8 index = 0; index < maxIndex; index++)
                    {
                        modelNameList.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString ());
                        if ((modelIndex == maxIndex) && (selectedModel != "") && (selectedModel == modelNameList.value(index)))
                        {
                            modelIndex = index;
                        }
                    }

                    modelNameDropdown->setNewList(modelNameList, modelIndex, (selectedModel == ""));
                }
                break;

                case GET_USER_DETAIL:
                {
                    if (currentCameraType == IP_CAMERA)
                    {
                        payloadLib->parseDevCmdReply(true, param->payload);
                        usernameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(0).toString());
                        passwordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(1).toString());
                    }
                    else if (currentCameraType == AUTO_ADD_IP_CAMERA)
                    {
                        usernameTextBox->setInputText("");
                        usernameTextBox->setIsEnabled(false);
                        passwordTextBox->setInputText("");
                        passwordTextBox->setIsEnabled(false);
                    }
                }
                break;

                case GET_MOTION_WINDOW:
                {
                    quint32 motionInfo[MAX_MOTION_BYTE + MAX_MOTION_INFO_CONFIG_OPTION];

                    memset(motionInfo, 0, sizeof(motionInfo));

                    applController->GetMotionInfo(motionInfo);
                    motionDetectionConfig.motionSupportType = (MOTION_DETECTION_SUPPORT_TYPE_e)motionInfo[0];

                    if(motionDetectionConfig.motionSupportType == POINT_METHOD)
                    {
                        for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
                        {
                            quint8 fieldIndex = 0;
                            motionDetectionConfig.windowInfo[index].startCol = motionInfo[(2 + (index * MAX_MOTION_INFO_CONFIG_OPTION) + fieldIndex++)];
                            motionDetectionConfig.windowInfo[index].startRow = motionInfo[(2 + (index * MAX_MOTION_INFO_CONFIG_OPTION) + fieldIndex++)];
                            motionDetectionConfig.windowInfo[index].endCol = motionInfo[(2 + (index * MAX_MOTION_INFO_CONFIG_OPTION) + fieldIndex++)];
                            motionDetectionConfig.windowInfo[index].endRow = motionInfo[(2 + (index * MAX_MOTION_INFO_CONFIG_OPTION) + fieldIndex++)];
                            motionDetectionConfig.windowInfo[index].sensitivity = motionInfo[(2 + (index * MAX_MOTION_INFO_CONFIG_OPTION) + fieldIndex)];
                        }
                    }
                    else
                    {
                        motionDetectionConfig.sensitivity = motionInfo[1];
                        motionDetectionConfig.noMotionEventSupportF = motionInfo[2];
                        motionDetectionConfig.isNoMotionEvent = motionInfo[3];
                        motionDetectionConfig.noMotionDuration = motionInfo[4];
                        for(quint8 index = 0; index < MAX_MOTION_BYTE ; index++)
                        {
                            motionDetectionConfig.byteInfo[index] = (quint8)motionInfo[MAX_MOTION_INFO_CONFIG_OPTION + index];
                        }
                    }

                    processBar->unloadProcessBar();

                    /* emit signal for motion Detection */
                    emit sigOpenCameraFeature(&motionDetectionConfig, MOTION_DETECTION_FEATURE, m_currentCameraIndex, NULL, deviceName);
                }
                break;

                case SET_MOTION_WINDOW:
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_SETTING_MOTION_AREA_SET));
                }
                break;

                case GET_PRIVACY_MASK_WINDOW:
                {
                    payloadLib->parseDevCmdReply(true, param->payload);
                    Layout::maxSupportedPrivacyMaskWindow = payloadLib->getCnfgArrayAtIndex(0).toInt();

                    for(quint8 index = 0; index <  Layout::maxSupportedPrivacyMaskWindow; index++)
                    {
                        quint8 fieldIndex = 1;
                        privacyMask[index].startX = payloadLib->getCnfgArrayAtIndex(((index * 4) + fieldIndex++)).toInt();
                        privacyMask[index].startY = payloadLib->getCnfgArrayAtIndex(((index * 4) + fieldIndex++)).toInt();
                        privacyMask[index].width = payloadLib->getCnfgArrayAtIndex(((index * 4) + fieldIndex++)).toInt();
                        privacyMask[index].height = payloadLib->getCnfgArrayAtIndex(((index * 4) + fieldIndex++)).toInt();
                    }

                    processBar->unloadProcessBar();

                    /* emit signal for privacy masking */
                    emit sigOpenCameraFeature(privacyMask, PRIVACY_MASK_FEATURE, m_currentCameraIndex, NULL, deviceName);
                }
                break;

                case SET_PRIVACY_MASK_WINDOW:
                {
                    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_SETTING_PRIV_AREA_SET));
                }
                break;

                case CHANGE_IP_CAM_ADDRS:
                {
                    isUnloadProcessBar = false;
                    DELETE_OBJ(m_ipChangeProcessRequest);
                    cameraAddressTextbox->setInputText(changedIpAddress);
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if(isUnloadProcessBar)
    {
        processBar->unloadProcessBar();
    }
}

void CameraSettings::loadProcessBar()
{
    processBar->loadProcessBar();
}

void CameraSettings::enableIpCameraCntrls()
{
    if(currentCameraType == IP_CAMERA)
    {
        cameraAddressTextbox->setIsEnabled(isCameraEnable);
        onvifSupportCheckBox->setIsEnabled(isCameraEnable);
        httpPortTextBox->setIsEnabled(isCameraEnable);
        rtspPortTextBox->setIsEnabled(isCameraEnable);
        onvifPortTextBox->setIsEnabled(isCameraEnable);
        usernameTextBox->setIsEnabled(isCameraEnable);
        passwordTextBox->setIsEnabled(isCameraEnable);
        urlTextBox->setIsEnabled(((brandNameDropdown->getCurrValue(true) == "GENERIC") && (isCameraEnable)) ? true : false);
        enableChangeIpButtonState(isCameraEnable);
        if(onvifSupportCheckBox->getCurrentState() == ON_STATE)
        {
            brandNameDropdown->setIsEnabled(false);
            modelNameDropdown->setIsEnabled(false);
            httpPortTextBox->setIsEnabled(false);
            protocolSection->setIsEnabled(isCameraEnable);
            onvifPortTextBox->setIsEnabled(isCameraEnable);
        }
        else
        {
            brandNameDropdown->setIsEnabled(isCameraEnable);
            modelNameDropdown->setIsEnabled(isCameraEnable);
            httpPortTextBox->setIsEnabled(isCameraEnable);
            protocolSection->setIsEnabled(false);
            onvifPortTextBox->setIsEnabled(false);
        }
        ipAddressChangeButton->setVisible(true);
        ipAddressChangeButton->setIsEnabled(isCameraEnable);
        autoAddCameraImage->setVisible(false);
        autoAddCameraImage->setIsEnabled(false);
    }
    else if(currentCameraType == AUTO_ADD_IP_CAMERA)
    {
        cameraAddressTextbox->setIsEnabled(false);
        httpPortTextBox->setIsEnabled(false);
        rtspPortTextBox->setIsEnabled(false);
        onvifPortTextBox->setIsEnabled(false);
        urlTextBox->setIsEnabled(false);
        onvifSupportCheckBox->setIsEnabled(false);
        brandNameDropdown->setIsEnabled(false);
        modelNameDropdown->setIsEnabled(false);
        usernameTextBox->setIsEnabled(false);
        passwordTextBox->setIsEnabled(false);
        ipAddressChangeButton->setVisible(false);
        ipAddressChangeButton->setIsEnabled(false);
        urlTextBox->setIsEnabled(false);
        protocolSection->setIsEnabled(false);
        autoAddCameraImage->setVisible(true);
        autoAddCameraImage->setIsEnabled(true);
    }
}

void CameraSettings::createOsdSetting(CAMERA_TYPE_e camType)
{
    if(IS_VALID_OBJ(osdSettingsPageOpenButton))
    {
        disconnect(osdSettingsPageOpenButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(osdSettingsPageOpenButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(osdSettingsPageOpenButton);
    }

    DELETE_OBJ(cameraMacAddressReadOnly);

    osdSettingsPageOpenButton = new PageOpenButton((mobileNumberTextBox->x() +
                                                   mobileNumberTextBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                                   mobileNumberTextBox->y(),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   CAM_SET_CAM_OSD_PAGEOPEN,
                                                   PAGEOPENBUTTON_SMALL,
                                                   "Set", this,
                                                   cameraSettingsStrings[CAM_SET_CAM_OSD_PAGEOPEN],
                                                   "", true, 0,
                                                   (camType != AUTO_ADD_IP_CAMERA) ? BOTTOM_TABLE_LAYER : MIDDLE_TABLE_LAYER);
    m_elementList[CAM_SET_CAM_OSD_PAGEOPEN] = osdSettingsPageOpenButton;
    connect(osdSettingsPageOpenButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(osdSettingsPageOpenButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    cameraMacAddressReadOnly = new ReadOnlyElement((recordingStreamMainCheckBox->x()
                                                  + recordingStreamMainCheckBox->width() + SCALE_WIDTH(INNER_TILES_MARGIN)),
                                                  recordingStreamMainCheckBox->y(),
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  SCALE_WIDTH(READONLY_LARGE_WIDTH),
                                                  READONLY_HEIGHT , "", this,
                                                  BOTTOM_TABLE_LAYER, -1,
                                                  SCALE_WIDTH(10), "MAC Address");
    cameraMacAddressReadOnly->setVisible((camType == AUTO_ADD_IP_CAMERA));
}

void CameraSettings::getModelList()
{
    if(brandNameDropdown->getCurrValue(true) != "")
    {
        payloadLib->setCnfgArrayAtIndex(0, brandNameDropdown->getCurrValue(true));
        createCmdRequest(MDL_NAME, 1);
    }
    else
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_VALID_BRAND_NM));
    }
}

void CameraSettings::slotSpinBoxValueChanged(QString str, quint32 index)
{
    if (index == CAM_SET_CAM_NAME_SPINBOX)
    {
        m_currentCameraIndex = cameraNumberDropDownBox->getIndexofCurrElement() + 1;
        getConfig();
    }
    else if (CAM_SET_BRAND_SPINBOX == index)
    {
        urlTextBox->setIsEnabled((str == "GENERIC") ? true : false);
        enableChangeIpButtonState(isCameraEnable && isIpChngEnable);

        modelNameList.clear();
        modelNameDropdown->setNewList(modelNameList, 0, true);

        payloadLib->setCnfgArrayAtIndex(0, brandNameDropdown->getCurrValue(true));
        createCmdRequest(GET_USER_DETAIL, 1);
        getModelList();
    }
}

void CameraSettings::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    switch(msgType)
    {
        case INFO_MSG_STRAT_CHAR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(START_CHAR_ERROR_MSG));
            break;

        case INFO_MSG_END_CHAR:
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(DEV_SETTING_END_CHAR_ERROR_MSG));
            break;

        case INFO_MSG_ERROR:
            if(index == CAM_SET_MOBILENUMBER_TEXTBOX)
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VALID_MOB_NM));
            }
            else if(index == CAM_SET_CAM_NAME_TEXTBOX)
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_SETTING_CAM_NAME));
            }
            break;

        default:
            break;
    }
}

void CameraSettings::slotOptionButtonClicked(OPTION_STATE_TYPE_e state, int indexInPage)
{
    switch(indexInPage)
    {
        case CAM_SET_CAM_ENABLE_CHECKBOX:
        {
            isCameraEnable = ((state == ON_STATE) ? true : false);

            cameraNameTextbox->setIsEnabled(isCameraEnable);
            logMotionDetectionCheckBox->setIsEnabled(isCameraEnable);
            motionDelayTextbox->setIsEnabled(isCameraEnable);
            mobileNumberTextBox->setIsEnabled(isCameraEnable);
            testCameraButton->setIsEnabled(isCameraEnable);
            osdSettingsPageOpenButton->setIsEnabled(isCameraEnable);
            motionDetectionCheckBox->setIsEnabled(isCameraEnable);
            m_motionDetectionCopyToCamButton->setIsEnabled(isCameraEnable);
            motionDetectionSetButton->setIsEnabled(((motionDetectionCheckBox->getCurrentState() == OFF_STATE) ?
                                                         false : (((currentCameraType == IP_CAMERA) || (currentCameraType == AUTO_ADD_IP_CAMERA))
                                                         ? (isCameraEnable && isMotionAreaSetAllow) : isCameraEnable)));
            privacyMaskCheckBox->setIsEnabled(isCameraEnable);
            privacyMaskSetButton->setIsEnabled(((privacyMaskCheckBox->getCurrentState() == OFF_STATE) ?
                                                        false : (((currentCameraType == IP_CAMERA) ||(currentCameraType == AUTO_ADD_IP_CAMERA))
                                                        ? (isCameraEnable && isPrivacyAreaSetAllow) : isCameraEnable)));

            recordingStreamMainCheckBox->setIsEnabled(isCameraEnable);
            recordingStreamSubCheckBox->setIsEnabled(isCameraEnable);

            enableIpCameraCntrls();
            if(currentCameraType != AUTO_ADD_IP_CAMERA)
            {
                enableChangeIpButtonState(isCameraConfigured && isCameraEnable);
            }
        }
        break;

        case CAM_SET_CAM_MOTION_DETECTION_CHECKBOX:
        {
            motionDetectionSetButton->setIsEnabled((state == ON_STATE) ? true : false);
        }
        break;

        case CAM_SET_CAM_PRIVACY_MASK_CHECKBOX:
        {
            privacyMaskSetButton->setIsEnabled((state == ON_STATE) ? true : false);
        }
        break;

        case CAM_SET_RECORDING_STREAM_MAIN:
        {
            recordingStreamSubCheckBox->changeState(OFF_STATE);
        }
        break;

        case CAM_SET_RECORDING_STREAM_SUB:
        {
            recordingStreamMainCheckBox->changeState(OFF_STATE);
        }
        break;

        case CAM_SET_ONVIF_CHECKBOX:
        {
            bool isEnable = ((state == ON_STATE) ? true : false);

            protocolSection->setIsEnabled(isEnable);
            brandNameDropdown->setIsEnabled(!isEnable);
            modelNameDropdown->setIsEnabled(!isEnable);
            httpPortTextBox->setIsEnabled(!isEnable);
            onvifPortTextBox->setIsEnabled(isEnable);

            brandNameList.clear();
            brandNameDropdown->setNewList(brandNameList, 0, true);

            modelNameList.clear();
            modelNameDropdown->setNewList(modelNameList, 0, true);

            if(isEnable == false)
            {
                createCmdRequest(BRND_NAME,0);
                urlTextBox->setInputText("");
                urlTextBox->setIsEnabled(false);
            }
            else
            {
                payloadLib->setCnfgArrayAtIndex(0, "ONVIF");
                urlTextBox->setIsEnabled(false);
                createCmdRequest(GET_USER_DETAIL, 1);
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

void CameraSettings::slotCreateCMDRequest(SET_COMMAND_e cmd, quint8 totalFields)
{
    createCmdRequest(cmd,totalFields);
}

void CameraSettings::slotButtonClick(int indexInPage)
{
    switch(indexInPage)
    {
        case CAM_SET_CAM_TEST_BUTTON:
        {
            payloadLib->setCnfgArrayAtIndex(0, m_currentCameraIndex);
            createCmdRequest(TST_CAM, 1);
        }
        break;

        case CAM_SET_CAM_OSD_PAGEOPEN:
        {
            payloadLib->setCnfgArrayAtIndex(0, m_currentCameraIndex);
            payloadLib->setCnfgArrayAtIndex(1, CAPABILITY_CMD_ID_TEXT_OVERLAY);
            createCmdRequest(GET_CAPABILITY, 2);
        }
        break;

        case CAM_SET_CAM_MOTION_DETECTION_PAGEOPEN:
        {
            if ((isCameraConfigured == false) && (cameraAddressTextbox->getInputText() == ""))
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(CAM_SETTING_CONF_CAM_BEF_PROCESS));
            }

            if (isMotionAreaSetAllow == false)
            {
                DEV_TABLE_INFO_t deviceInfo;
                if ((true == applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo)) && (deviceInfo.startLiveView == false))
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED_CLICK_ACTION));
                }
                else
                {
                    infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_FEATURE_NOT_SUPPORTED));
                }
            }
            else if (Layout::currentModeType[MAIN_DISPLAY] == STATE_EXPAND_WINDOW)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(WINDOW_EXPANDING_ON_MSG));
            }
            else if (Layout::isPlaybackRunning() == true)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(PB_RUNNING_MESSAGE));
            }
            else
            {
                payloadLib->setCnfgArrayAtIndex(0, m_currentCameraIndex);
                createCmdRequest(GET_MOTION_WINDOW, 1);
            }
        }
        break;

        case CAM_MOTION_DETECTION_COPY_TO_CAMERA:
        {
            if(false == IS_VALID_OBJ(m_copyToCamera))
            {
                memset(&m_motionDetectionCopyToCameraField, 0, sizeof(m_motionDetectionCopyToCameraField));
                SET_CAMERA_MASK_BIT(m_motionDetectionCopyToCameraField, (m_currentCameraIndex - 1));

                m_copyToCamera = new CopyToCamera(cameraList,
                                                  m_motionDetectionCopyToCameraField,
                                                  parentWidget(),
                                                  QString("Motion Detection Area Setting"),
                                                  indexInPage);
                connect(m_copyToCamera,
                         SIGNAL(sigDeleteObject(quint8)),
                         this,
                         SLOT(slotPopupPageDeleted(quint8)));
            }
        }
        break;

        case CAM_SET_CAM_PRIVACY_MASK_PAGEOPEN:
        {
            if ((isCameraConfigured == false) && (cameraAddressTextbox->getInputText() == ""))
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(CAM_SETTING_CONF_CAM_BEF_PROCESS));
            }

            if (isPrivacyAreaSetAllow == false)
            {
                DEV_TABLE_INFO_t deviceInfo;
                if ((true == applController->GetDeviceInfo(LOCAL_DEVICE_NAME, deviceInfo)) && (deviceInfo.startLiveView == false))
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(LOCAL_DECODING_DISABLED_CLICK_ACTION));
                }
                else
                {
                    infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(CMD_FEATURE_NOT_SUPPORTED));
                }
            }
            else if (Layout::currentModeType[MAIN_DISPLAY] == STATE_EXPAND_WINDOW)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(WINDOW_EXPANDING_ON_MSG));
            }
            else if (Layout::isPlaybackRunning() == true)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(PB_RUNNING_MESSAGE));
            }
            else
            {
                payloadLib->setCnfgArrayAtIndex(0, m_currentCameraIndex);
                createCmdRequest(GET_PRIVACY_MASK_WINDOW, 1);
            }
        }
        break;

        case CAM_SET_IP_CHANGE_SET_BUTTON:
        {
            if (IS_VALID_OBJ(ipAddressChange))
            {
                break;
            }

            ipAddressChange = new IpAddressChange(cameraAddressTextbox->getInputText(), "", "", parentWidget());
            connect(ipAddressChange,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotIpAddressChangeDelete()));
            connect(ipAddressChange,
                    SIGNAL(sigDataSelectedForIpChange(QString,QString,QString)),
                    this,
                    SLOT(slotIpAddressChangeData(QString,QString,QString)));
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void CameraSettings::slotPopupPageDeleted(quint8 indexInPage)
{
    if(indexInPage == CAM_SET_CAM_TEST_BUTTON)
    {
        if(IS_VALID_OBJ(testCamera))
        {
            disconnect(testCamera,
                       SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                       this,
                       SLOT(slotCreateCMDRequest(SET_COMMAND_e,quint8)));
            disconnect(testCamera,
                       SIGNAL(sigDeleteObject(quint8)),
                       this,
                       SLOT(slotPopupPageDeleted(quint8)));
            DELETE_OBJ(testCamera);
        }
    }
    else if(indexInPage == CAM_SET_CAM_OSD_PAGEOPEN)
    {
        if(IS_VALID_OBJ(cameraOsdSettings))
        {
            disconnect(cameraOsdSettings,
                       SIGNAL(sigDeleteObject(quint8)),
                       this,
                       SLOT(slotPopupPageDeleted(quint8)));
            DELETE_OBJ(cameraOsdSettings);
        }
    }
    else if(indexInPage == CAM_MOTION_DETECTION_COPY_TO_CAMERA)
    {
        if(IS_VALID_OBJ(m_copyToCamera))
        {
            disconnect(m_copyToCamera,
                       SIGNAL(sigDeleteObject(quint8)),
                       this,
                       SLOT(slotPopupPageDeleted(quint8)));
            DELETE_OBJ(m_copyToCamera);
        }
    }

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void CameraSettings::slotcameraListUpdateTimerTimeout()
{
    fillCameraList();
}

void CameraSettings::enableChangeIpButtonState(bool isEnable)
{
    if(currentCameraType == IP_CAMERA)
    {
        ipAddressChangeButton->setIsEnabled(((cameraAddressTextbox->getInputText() == "")
                                             || (brandNameDropdown->getCurrValue(true) == "GENERIC")) ? false : isEnable);
    }
}

void CameraSettings::slotIpAddressChangeDelete()
{
    if(IS_VALID_OBJ(ipAddressChange))
    {
        disconnect(ipAddressChange,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotIpAddressChangeDelete()));
        disconnect(ipAddressChange,
                    SIGNAL(sigDataSelectedForIpChange(QString,QString,QString)),
                    this,
                    SLOT(slotIpAddressChangeData(QString,QString,QString)));
        DELETE_OBJ(ipAddressChange);
    }
}

void CameraSettings::slotIpAddressChangeData(QString ipAddr, QString subnet, QString gateWay)
{
    changedIpAddress = ipAddr;
    quint8 fieldIdx = 0;
    payloadLib->setCnfgArrayAtIndex(fieldIdx++, m_currentCameraIndex);
    payloadLib->setCnfgArrayAtIndex(fieldIdx++, ipAddr);
    payloadLib->setCnfgArrayAtIndex(fieldIdx++, subnet);
    payloadLib->setCnfgArrayAtIndex(fieldIdx++, gateWay);

    if(m_ipChangeProcessRequest == NULL)
    {
        /* PARASOFT: Memory deallocated in process Device Response */
        m_ipChangeProcessRequest = new IpChangeProcessRequest(parentWidget());
    }

    createCmdRequest(CHANGE_IP_CAM_ADDRS, fieldIdx);

    if(IS_VALID_OBJ(ipAddressChange))
    {
        disconnect(ipAddressChange,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotIpAddressChangeDelete()));
        disconnect(ipAddressChange,
                    SIGNAL(sigDataSelectedForIpChange(QString,QString,QString)),
                    this,
                    SLOT(slotIpAddressChangeData(QString,QString,QString)));
        DELETE_OBJ(ipAddressChange);
    }
}

void CameraSettings::slotToolTipShowHide(bool status)
{
    autoAddToolTip->setVisible(status);
    if(status == true)
    {
        autoAddToolTip->raise();
    }
}

void CameraSettings::slotValueListEmpty(quint8 indexInPage)
{
    if (indexInPage == CAM_SET_MODEL_SPINBOX)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_CAM_SET_ENT_VALID_BRAND_NM));
    }
}

void CameraSettings::mouseMoveEvent(QMouseEvent *event)
{
    if(currentCameraType != AUTO_ADD_IP_CAMERA)
    {
        return;
    }

    sigToolTipShowHide((autoAddCameraImage->frameGeometry().contains(event->pos())) ? true : false);
}
