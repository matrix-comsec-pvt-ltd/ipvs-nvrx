#include "GeneralSetting.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX       1
#define Y_OFFSET            13
#define AUTO_CNFG_FIELD     20

/* The STATUS_IMAGE_HEIGHT indicates the height of the status image */
#define STATUS_IMAGE_HEIGHT SCALE_HEIGHT(22)

/* Defines the location of image for status */
#define STATUS_CONNECTED    ":/Images_Nvrx/StatusIcon/Connected.png"
#define STATUS_DISCONNECTED ":/Images_Nvrx/StatusIcon/Disconnected.png"
#define STATUS_DISABLED     ":/Images_Nvrx/StatusIcon/Disabled.png"

// List of control
typedef enum
{
    GN_STG_DEVICE_NO,
    GN_STG_DEVICE_NAME,
    GN_STG_SINGLE_FILEREC_DUR,
    GN_STG_HTTP_PORT,
    GN_STG_TCP_PORT,
    GN_STG_FORWD_TCP_PORT,
    GN_STG_RTP_PORT1,
    GN_STG_RTP_PORT2,
    GN_STG_OSD_DATEFORMAT,
    GN_STG_OSD_TIMEFORMAT,
    GN_STG_START_LIVE_VIEW,
    GN_STG_AUTO_CONFIG_FLAG,
    GN_STG_AUTO_CONFIG_SET,
    GN_STG_AUTO_ADD_CAM_FLAG,
    GN_STG_AUTO_ADD_CAM_SET,
    GN_STG_PREVIDEO_LOSS_DURATION,
    GN_STG_AUTO_CLOSE_REC_FAIL_ALERT,
    GN_STG_LIVE_VIEW_POP_UP_DURATION,
    GN_STG_RECORDING_FORMAT,
    GN_STG_INT_SAMAS_FLAG,
    GN_STG_INT_SAMAS_IP,
    GN_STG_INT_SAMAS_PORT,
    GN_STG_INT_COSEC,
    MAX_GEN_SETTING_ELEMETS
}GEN_SETTING_ELELIST_e;

typedef enum
{
    GN_SET_USER_NAME = 20,
    GN_SET_PASSWORD,
    MAX_GN_SET_ELEMENT
}GN_SET_ELELIST_e;

typedef enum
{
    DI_STATUS_CONNECTION_FAILURE,
    DI_STATUS_CONNECTED,
    DI_STATUS_CONNECTION_REFUSED,
    DI_STATUS_CONNECTION_IN_PROGRESS,
    DI_STATUS_REQUEST_PENDING,
    DI_STATUS_MAX
}DI_CONNECTION_STATUS_e;

static const QString generalSettingElementStrings[MAX_GEN_SETTING_ELEMETS] =
{
    "Device Number",
    "Device Name",
    "Single File Record Duration",
    "HTTP Port",
    "TCP Port",
    "Forwarded TCP Port",
    "RTP Port Range",
    "",
    "OSD Date Format",
    "OSD Time Format",
    "Start Live View",
    "Auto Configure",
    "Set",
    "Auto Add Camera",
    "Set",
    "Pre Video Loss Duration",
    "Auto Close Rec. Fail Alert",
    "Video Pop-up Duration",
    "Recording Format",
    "Integrate with SAMAS",
    "IP Address",
    "Port",
    "Integrate with COSEC",
};

static const QString generalSettingElementSuffixStrings[MAX_GEN_SETTING_ELEMETS] =
{
    "(1-255)",
    "",
    "(5-60 min)",
    "(80,1024-65535) ",
    "(1024-65535) ",
    "(1024-65535) ",
    "(1024-65534) ",
    "(1025-65535) ",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "(3-60 sec)",
    "",
    "(10-60 sec)",
    "",
    "",
    "",
    "(1025-65535)",
    "",
};

static const QString dateFormatString[] =
{
    "DD/MM/YYYY",
    "MM/DD/YYYY",
    "YYYY/MM/DD",
    "WWWDD/MM/YYYY"
};

static const QStringList timeFormatString = QStringList() << QString("12 ") + Multilang("Hour")
                                                          << QString("24 ") + Multilang("Hour");

static const QString recFormatString[] =
{
    "Native",
    "Both",
    "Avi"
};

/* strings for SAMAS connection status */
static const QString samasConnStatusString[DI_STATUS_MAX] =
{
    "Connection Failed",
    "Connected",
    "Rejected by SAMAS",
    "Connection in Progress",
    "Request Pending at SAMAS",
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
GeneralSetting::GeneralSetting(QString devName, QWidget *parent,DEV_TABLE_INFO_t *devTable)
    : ConfigPageControl(devName, parent, MAX_GEN_SETTING_ELEMETS,devTable)
{
    remoteDeviceList.clear();
    createDefaultComponent();
    autoConfigureCamera = NULL;
    autoAddCamera = NULL;
    m_defaultButtonClick = false;
    tcpPortChangeFlag = false;
    GeneralSetting::getConfig();
}

GeneralSetting::~GeneralSetting()
{
    slotObjectDelete();

    disconnect (deviceNumberTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (deviceNumberTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete deviceNumberTextBox;
    delete deviceNumberTextBoxParam;

    disconnect (singleDiskRecordTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (singleDiskRecordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete singleDiskRecordTextBox;
    delete singleDiskRecordTextBoxParam;

    disconnect (httpPortTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (httpPortTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete httpPortTextBox;
    delete httpPortTextBoxParam;

    disconnect (tcpPortTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    disconnect (tcpPortTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete tcpPortTextBox;
    delete tcpPortTextBoxParam;

    disconnect (forwardedTcpPortTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (forwardedTcpPortTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete forwardedTcpPortTextBox;
    delete forwardedTcpPortTextBoxParam;

    disconnect (rtspPort1TextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (rtspPort1TextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete rtspPort1TextBox;
    delete rtspPort1TextBoxParam;

    disconnect (rtspPort2TextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (rtspPort2TextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete rtspPort2TextBox;
    delete rtspPort2TextBoxParam;

    disconnect (m_liveViewPopUpDurationTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (m_liveViewPopUpDurationTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_liveViewPopUpDurationTextBox);
    DELETE_OBJ(m_liveViewPopUpDurationTextBoxParam);

    disconnect (deviceNameTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (deviceNameTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete deviceNameTextBox ;
    delete deviceNameTextBoxParam;

    disconnect (integratedCosecCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete integratedCosecCheckBox;

    disconnect(autoConfigCameraCheckBox,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));
    disconnect (autoConfigCameraCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete autoConfigCameraCheckBox;

    disconnect(autoConfigureSetButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(autoConfigureSetButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotPageOpenButtonClicked(int)));
    delete autoConfigureSetButton;

    disconnect(dateFormatDropdown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete dateFormatDropdown;

    disconnect(timeFormatDropdown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete timeFormatDropdown;

    disconnect(m_autoCloseRecFailCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    delete m_autoCloseRecFailCheckBox;

    disconnect(m_recordingFormat,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_recordingFormat;

    disconnect (integratedSamasCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (integratedSamasCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));
    delete integratedSamasCheckBox;

    disconnect(m_statusImage,
             SIGNAL(sigImageMouseHover(int,bool)),
             this,
             SLOT(slotImageMouseHover(int,bool)));
    DELETE_OBJ(m_statusImage);
    DELETE_OBJ(m_statusToolTip);

    disconnect (samasServerIpTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete samasServerIpTextbox;

    disconnect (samasServerPortTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (samasServerPortTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete samasServerPortTextBox;
    delete samasServerPortTextBoxParam;

    disconnect (m_preVideoLossTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    disconnect (m_preVideoLossTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(m_preVideoLossTextBox);
    DELETE_OBJ(m_preVideoLossTextBoxParam);

    disconnect (startLiveViewCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (startLiveViewCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));
    delete startLiveViewCheckBox;

    disconnect (autoAddCameraCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));
    disconnect (autoAddCameraCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    DELETE_OBJ(autoAddCameraCheckBox);

    disconnect (autoAddCameraSetButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (autoAddCameraSetButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPageOpenButtonClicked(int)));
    DELETE_OBJ(autoAddCameraSetButton);
    DELETE_OBJ(m_advancedSettingTile);
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void GeneralSetting::createDefaultComponent()
{
    recordingFormat = 0;
    tcpPortValue = 0;

    // Device Number Field
    deviceNumberTextBoxParam = new TextboxParam();
    deviceNumberTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_DEVICE_NO];
    deviceNumberTextBoxParam->suffixStr = generalSettingElementSuffixStrings[GN_STG_DEVICE_NO];
    deviceNumberTextBoxParam->isNumEntry = true;
    deviceNumberTextBoxParam->minNumValue = 1;
    deviceNumberTextBoxParam->maxNumValue = 255;
    deviceNumberTextBoxParam->maxChar = 3;
    deviceNumberTextBoxParam->validation = QRegExp(QString("[0-9]"));

    //  Device Name Field
    deviceNameTextBoxParam = new TextboxParam();
    deviceNameTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_DEVICE_NAME];
    deviceNameTextBoxParam->minChar = 1;
    deviceNameTextBoxParam->maxChar = 15;
    deviceNameTextBoxParam->validation = QRegExp(QString("[a-zA-Z0-9-]"));

    // Single File Record Duration Field
    singleDiskRecordTextBoxParam = new TextboxParam();
    singleDiskRecordTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_SINGLE_FILEREC_DUR];
    singleDiskRecordTextBoxParam->suffixStr = generalSettingElementSuffixStrings[GN_STG_SINGLE_FILEREC_DUR];
    singleDiskRecordTextBoxParam->isNumEntry = true;
    singleDiskRecordTextBoxParam->minNumValue = 5;
    singleDiskRecordTextBoxParam->maxNumValue = 60;
    singleDiskRecordTextBoxParam->maxChar = 2;
    singleDiskRecordTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // http port text box property
    httpPortTextBoxParam = new TextboxParam();
    httpPortTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_HTTP_PORT];
    httpPortTextBoxParam->suffixStr = generalSettingElementSuffixStrings[GN_STG_HTTP_PORT];
    httpPortTextBoxParam->isNumEntry = true;
    httpPortTextBoxParam->minNumValue = 1024;
    httpPortTextBoxParam->maxNumValue = 65535;
    httpPortTextBoxParam->extraNumValue = 80;
    httpPortTextBoxParam->maxChar = 5;
    httpPortTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // rtp port 1 text box property
    rtspPort1TextBoxParam = new TextboxParam();
    rtspPort1TextBoxParam->labelStr = generalSettingElementStrings[GN_STG_RTP_PORT1];
    rtspPort1TextBoxParam->isNumEntry = true;
    rtspPort1TextBoxParam->minNumValue = 1024;
    rtspPort1TextBoxParam->maxNumValue = 65534;
    rtspPort1TextBoxParam->extraNumValue = 80;
    rtspPort1TextBoxParam->maxChar = 5;
    rtspPort1TextBoxParam->validation = QRegExp(QString("[0-9]"));

    // rtp port 2 text box property
    rtspPort2TextBoxParam = new TextboxParam();
    rtspPort2TextBoxParam->labelStr = generalSettingElementStrings[GN_STG_RTP_PORT2];
    rtspPort2TextBoxParam->isNumEntry = true;
    rtspPort2TextBoxParam->minNumValue = 1025;
    rtspPort2TextBoxParam->maxNumValue = 65535;
    rtspPort2TextBoxParam->extraNumValue = 80;
    rtspPort2TextBoxParam->maxChar = 5;
    rtspPort2TextBoxParam->validation = QRegExp(QString("[0-9]"));

    // live View PopUp Time text box property
    m_liveViewPopUpDurationTextBoxParam = new TextboxParam();
    m_liveViewPopUpDurationTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_LIVE_VIEW_POP_UP_DURATION];
    m_liveViewPopUpDurationTextBoxParam->suffixStr = QString("(10-60 ")+ Multilang("sec")+QString(")");
    m_liveViewPopUpDurationTextBoxParam->isNumEntry = true;
    m_liveViewPopUpDurationTextBoxParam->minNumValue = 10;
    m_liveViewPopUpDurationTextBoxParam->maxNumValue = 60;
    m_liveViewPopUpDurationTextBoxParam->maxChar = 2;
    m_liveViewPopUpDurationTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // tcp port text box property
    tcpPortTextBoxParam = new TextboxParam();
    tcpPortTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_TCP_PORT];
    tcpPortTextBoxParam->suffixStr = generalSettingElementSuffixStrings[GN_STG_TCP_PORT];
    tcpPortTextBoxParam->isNumEntry = true;
    tcpPortTextBoxParam->minNumValue = 1024;
    tcpPortTextBoxParam->maxNumValue = 65535;
    tcpPortTextBoxParam->maxChar = 5;
    tcpPortTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // tcp port text box property
    forwardedTcpPortTextBoxParam = new TextboxParam();
    forwardedTcpPortTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_FORWD_TCP_PORT];
    forwardedTcpPortTextBoxParam->suffixStr = generalSettingElementSuffixStrings[GN_STG_FORWD_TCP_PORT];
    forwardedTcpPortTextBoxParam->isNumEntry = true;
    forwardedTcpPortTextBoxParam->minNumValue = 1024;
    forwardedTcpPortTextBoxParam->maxNumValue = 65535;
    forwardedTcpPortTextBoxParam->maxChar = 5;
    forwardedTcpPortTextBoxParam->validation = QRegExp(QString("[0-9]"));

    // samasServer Text Box property
    samasServerPortTextBoxParam = new TextboxParam();
    samasServerPortTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_INT_SAMAS_PORT];
    samasServerPortTextBoxParam->suffixStr = generalSettingElementSuffixStrings[GN_STG_INT_SAMAS_PORT];
    samasServerPortTextBoxParam->isNumEntry = true;
    samasServerPortTextBoxParam->minNumValue = 1025;
    samasServerPortTextBoxParam->maxNumValue = 65535;
    samasServerPortTextBoxParam->maxChar = 5;
    samasServerPortTextBoxParam->validation = QRegExp(QString("[0-9]"));
    samasServerPortTextBoxParam->isCentre = false;
    samasServerPortTextBoxParam->leftMargin = SCALE_WIDTH(172);

    // Pre video loss text box
    m_preVideoLossTextBoxParam = new TextboxParam();
    m_preVideoLossTextBoxParam->labelStr = generalSettingElementStrings[GN_STG_PREVIDEO_LOSS_DURATION];
    m_preVideoLossTextBoxParam->suffixStr= QString("(3-60 ")+ Multilang("sec")+QString(")");
    m_preVideoLossTextBoxParam->isNumEntry = true;
    m_preVideoLossTextBoxParam->minNumValue = 3;
    m_preVideoLossTextBoxParam->maxNumValue = 60;
    m_preVideoLossTextBoxParam->maxChar = 2;
    m_preVideoLossTextBoxParam->validation = QRegExp(QString("[0-9]"));

    deviceNumberTextBox = new TextBox(SCALE_WIDTH(10),
                                      (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) - (Y_OFFSET*BGTILE_HEIGHT))/2 - SCALE_HEIGHT(8),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      GN_STG_DEVICE_NO,
                                      TEXTBOX_SMALL,
                                      this,
                                      deviceNumberTextBoxParam);
    m_elementList[GN_STG_DEVICE_NO] = deviceNumberTextBox;
    connect (deviceNumberTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (deviceNumberTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    deviceNameTextBox = new TextBox(deviceNumberTextBox->x(),
                                    (deviceNumberTextBox->y() + deviceNumberTextBox->height()),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    GN_STG_DEVICE_NAME,
                                    TEXTBOX_LARGE,
                                    this,
                                    deviceNameTextBoxParam);
    m_elementList[GN_STG_DEVICE_NAME] = deviceNameTextBox;
    connect (deviceNameTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (deviceNameTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    singleDiskRecordTextBox = new TextBox(deviceNameTextBox->x(),
                                          deviceNameTextBox->y() + deviceNameTextBox->height(),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          GN_STG_SINGLE_FILEREC_DUR,
                                          TEXTBOX_SMALL,
                                          this,
                                          singleDiskRecordTextBoxParam);
    m_elementList[GN_STG_SINGLE_FILEREC_DUR] = singleDiskRecordTextBox;
    connect (singleDiskRecordTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (singleDiskRecordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    httpPortTextBox = new TextBox(singleDiskRecordTextBox->x(),
                                  singleDiskRecordTextBox->y() + singleDiskRecordTextBox->height(),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  GN_STG_HTTP_PORT,
                                  TEXTBOX_SMALL,
                                  this,
                                  httpPortTextBoxParam);
    m_elementList[GN_STG_HTTP_PORT] = httpPortTextBox;
    connect (httpPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (httpPortTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    tcpPortTextBox = new TextBox(httpPortTextBox->x(),
                                 httpPortTextBox->y() + httpPortTextBox->height(),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 GN_STG_TCP_PORT,
                                 TEXTBOX_SMALL,
                                 this,
                                 tcpPortTextBoxParam);
    m_elementList[GN_STG_TCP_PORT] = tcpPortTextBox;
    connect (tcpPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (tcpPortTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    forwardedTcpPortTextBox = new TextBox(tcpPortTextBox->x(),
                                 tcpPortTextBox->y() + tcpPortTextBox->height(),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 GN_STG_FORWD_TCP_PORT,
                                 TEXTBOX_SMALL,
                                 this,
                                 forwardedTcpPortTextBoxParam);
    m_elementList[GN_STG_FORWD_TCP_PORT] = forwardedTcpPortTextBox;
    connect (forwardedTcpPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (forwardedTcpPortTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    rtspPort1TextBox = new TextBox(forwardedTcpPortTextBox->x(),
                                   forwardedTcpPortTextBox->y() + forwardedTcpPortTextBox->height(),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   GN_STG_RTP_PORT1,
                                   TEXTBOX_SMALL,
                                   this,
                                   rtspPort1TextBoxParam);
    m_elementList[GN_STG_RTP_PORT1] = rtspPort1TextBox;
    connect (rtspPort1TextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (rtspPort1TextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    rtspPort2TextBox = new TextBox(rtspPort1TextBox->x() + SCALE_WIDTH(350),
                                   rtspPort1TextBox->y(),
                                   SCALE_WIDTH(80),
                                   BGTILE_HEIGHT,
                                   GN_STG_RTP_PORT2,
                                   TEXTBOX_SMALL,
                                   this,
                                   rtspPort2TextBoxParam,
                                   NO_LAYER);
    m_elementList[GN_STG_RTP_PORT2] = rtspPort2TextBox;
    connect (rtspPort2TextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (rtspPort2TextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8,QString> dateFormatStringList;
    dateFormatStringList.clear();
    for(quint8 index = 0; index < MAX_DATE_FORMAT_TYPE; index++)
    {
        dateFormatStringList.insert (index,dateFormatString[index]);
    }

    dateFormatDropdown = new DropDown(rtspPort1TextBox->x(),
                                      (rtspPort1TextBox->y() + BGTILE_HEIGHT),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      GN_STG_OSD_DATEFORMAT,
                                      DROPDOWNBOX_SIZE_200,
                                      generalSettingElementStrings[GN_STG_OSD_DATEFORMAT],
                                      dateFormatStringList, this);
    m_elementList[GN_STG_OSD_DATEFORMAT] = dateFormatDropdown;
    connect(dateFormatDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8,QString> timeFormatStringList;
    timeFormatStringList.clear();
    for(quint8 index = 0; index < MAX_TIME_FORMAT_TYPE; index++)
    {
        timeFormatStringList.insert(index,timeFormatString.at(index));
    }

    timeFormatDropdown = new DropDown(dateFormatDropdown->x(),
                                      (dateFormatDropdown->y() + BGTILE_HEIGHT),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      GN_STG_OSD_TIMEFORMAT,
                                      DROPDOWNBOX_SIZE_200,
                                      generalSettingElementStrings[GN_STG_OSD_TIMEFORMAT],
                                      timeFormatStringList, this);
    m_elementList[GN_STG_OSD_TIMEFORMAT] = timeFormatDropdown;
    connect(timeFormatDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    startLiveViewCheckBox = new OptionSelectButton((deviceNumberTextBox->x() + deviceNumberTextBox->width() + SCALE_WIDTH(10)),
                                                   deviceNumberTextBox->y(),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT, CHECK_BUTTON_INDEX,
                                                   this, COMMON_LAYER,
                                                   generalSettingElementStrings[GN_STG_START_LIVE_VIEW],
                                                   "", -1,
                                                   GN_STG_START_LIVE_VIEW,true,
                                                   NORMAL_FONT_SIZE);
    m_elementList[GN_STG_START_LIVE_VIEW] = startLiveViewCheckBox;
    connect (startLiveViewCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (startLiveViewCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));

    autoConfigCameraCheckBox = new OptionSelectButton(startLiveViewCheckBox->x(),
                                                      (startLiveViewCheckBox->y() + startLiveViewCheckBox->height()),
                                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,CHECK_BUTTON_INDEX,
                                                      this, COMMON_LAYER,
                                                      generalSettingElementStrings[GN_STG_AUTO_CONFIG_FLAG], "",-1,
                                                      GN_STG_AUTO_CONFIG_FLAG, true,
                                                      NORMAL_FONT_SIZE);
    m_elementList[GN_STG_AUTO_CONFIG_FLAG] = autoConfigCameraCheckBox;
    connect(autoConfigCameraCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));
    connect (autoConfigCameraCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    autoConfigureSetButton = new PageOpenButton((autoConfigCameraCheckBox->x() + SCALE_WIDTH(280)),
                                                autoConfigCameraCheckBox->y(),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                (GN_STG_AUTO_CONFIG_SET),
                                                PAGEOPENBUTTON_SMALL,
                                                generalSettingElementStrings[GN_STG_AUTO_CONFIG_SET],
                                                this, "", "",
                                                false, 0, NO_LAYER,
                                                false);
    m_elementList[GN_STG_AUTO_CONFIG_SET] = autoConfigureSetButton;
    connect(autoConfigureSetButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(autoConfigureSetButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotPageOpenButtonClicked(int)));

    autoAddCameraCheckBox = new OptionSelectButton(autoConfigCameraCheckBox->x(),
                                                    autoConfigCameraCheckBox->y() + autoConfigCameraCheckBox->height(),
                                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                                    BGTILE_HEIGHT,CHECK_BUTTON_INDEX,
                                                    this, COMMON_LAYER,
                                                    generalSettingElementStrings[GN_STG_AUTO_ADD_CAM_FLAG], "",-1,
                                                    GN_STG_AUTO_ADD_CAM_FLAG, true,
                                                    NORMAL_FONT_SIZE);
    m_elementList[GN_STG_AUTO_ADD_CAM_FLAG] = autoAddCameraCheckBox;
    connect(autoAddCameraCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));
    connect (autoAddCameraCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    autoAddCameraSetButton = new PageOpenButton((autoAddCameraCheckBox->x() + SCALE_WIDTH(280)),
                                                 autoAddCameraCheckBox->y(),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 (GN_STG_AUTO_ADD_CAM_SET),
                                                 PAGEOPENBUTTON_SMALL,
                                                 generalSettingElementStrings[GN_STG_AUTO_ADD_CAM_SET],
                                                 this, "", "",
                                                 false, 0, NO_LAYER,
                                                 false);
    m_elementList[GN_STG_AUTO_ADD_CAM_SET] = autoAddCameraSetButton;
    autoAddCameraSetButton-> setIsEnabled(true);
    connect(autoAddCameraSetButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(autoAddCameraSetButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotPageOpenButtonClicked(int)));

    m_preVideoLossTextBox = new TextBox(autoAddCameraCheckBox->x(),
                                        autoAddCameraCheckBox->y() + autoAddCameraCheckBox->height(),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        GN_STG_PREVIDEO_LOSS_DURATION,
                                        TEXTBOX_SMALL,
                                        this,
                                        m_preVideoLossTextBoxParam);
     m_elementList[GN_STG_PREVIDEO_LOSS_DURATION] = m_preVideoLossTextBox;
     connect (m_preVideoLossTextBox,
           SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
           this,
           SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
     connect (m_preVideoLossTextBox,
           SIGNAL(sigUpdateCurrentElement(int)),
           this,
           SLOT(slotUpdateCurrentElement(int)));

     m_autoCloseRecFailCheckBox = new OptionSelectButton(m_preVideoLossTextBox->x(),
                                                         (m_preVideoLossTextBox->y() + m_preVideoLossTextBox->height()),
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,CHECK_BUTTON_INDEX,
                                                         this, COMMON_LAYER,
                                                         generalSettingElementStrings[GN_STG_AUTO_CLOSE_REC_FAIL_ALERT], "",-1,
                                                         GN_STG_AUTO_CLOSE_REC_FAIL_ALERT, true,
                                                         NORMAL_FONT_SIZE);
    m_elementList[GN_STG_AUTO_CLOSE_REC_FAIL_ALERT] = m_autoCloseRecFailCheckBox;
    connect(m_autoCloseRecFailCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_liveViewPopUpDurationTextBox = new TextBox(m_autoCloseRecFailCheckBox->x(),
                                                 m_autoCloseRecFailCheckBox->y() + m_autoCloseRecFailCheckBox->height(),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 GN_STG_LIVE_VIEW_POP_UP_DURATION,
                                                 TEXTBOX_SMALL,
                                                 this,
                                                 m_liveViewPopUpDurationTextBoxParam);
    m_elementList[GN_STG_LIVE_VIEW_POP_UP_DURATION] = m_liveViewPopUpDurationTextBox;
    connect (m_liveViewPopUpDurationTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (m_liveViewPopUpDurationTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8,QString> recordingFormatList;
    recordingFormatList.clear();
    for(quint8 index = 0; index < MAX_REC_FORMAT; index++)
    {
        recordingFormatList.insert(index, recFormatString[index]);
    }

    m_recordingFormat = new DropDown(m_liveViewPopUpDurationTextBox->x(),
                                     (m_liveViewPopUpDurationTextBox->y() + BGTILE_HEIGHT),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     GN_STG_RECORDING_FORMAT,
                                     DROPDOWNBOX_SIZE_200,
                                     generalSettingElementStrings[GN_STG_RECORDING_FORMAT],
                                     recordingFormatList, this);
    m_elementList[GN_STG_RECORDING_FORMAT] = m_recordingFormat;
    connect(m_recordingFormat,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_advancedSettingTile = new ElementHeading(timeFormatDropdown->x(),
                                               (timeFormatDropdown->y()+timeFormatDropdown->height() + SCALE_HEIGHT(15)),
                                               BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(4),
                                               BGTILE_HEIGHT,
                                               "Advanced Settings",
                                               TOP_LAYER, this,
                                               false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    integratedSamasCheckBox = new OptionSelectButton(m_advancedSettingTile->x(),
                                                     (m_advancedSettingTile->y() + m_advancedSettingTile->height()),
                                                     m_advancedSettingTile->width(),
                                                     BGTILE_HEIGHT,CHECK_BUTTON_INDEX,
                                                     this, MIDDLE_TABLE_LAYER,
                                                     generalSettingElementStrings[GN_STG_INT_SAMAS_FLAG],"",SCALE_WIDTH(20),
                                                     GN_STG_INT_SAMAS_FLAG,true,
                                                     NORMAL_FONT_SIZE);
    m_elementList[GN_STG_INT_SAMAS_FLAG] = integratedSamasCheckBox;
    connect (integratedSamasCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (integratedSamasCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionClicked(OPTION_STATE_TYPE_e,int)));

    samasServerIpTextbox = new Ipv4TextBox(integratedSamasCheckBox->x(),
                                           (integratedSamasCheckBox->y() +
                                           integratedSamasCheckBox->height()),
                                           integratedSamasCheckBox->width(),
                                           BGTILE_HEIGHT, GN_STG_INT_SAMAS_IP,
                                           generalSettingElementStrings[GN_STG_INT_SAMAS_IP], this,
                                           MIDDLE_TABLE_LAYER,false,SCALE_WIDTH(120));
    m_elementList[GN_STG_INT_SAMAS_IP] = samasServerIpTextbox;
    connect (samasServerIpTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    /* Draws the image of tick or cross to indicate the connection status */
    m_statusImage = new Image(samasServerIpTextbox->x() + (samasServerIpTextbox->width()/2 - SCALE_WIDTH(90)),
                              samasServerIpTextbox->y() + (samasServerIpTextbox->height() - STATUS_IMAGE_HEIGHT)/2,
                              STATUS_DISABLED, this, START_X_START_Y, 0, true, true);
    connect (m_statusImage,
             SIGNAL(sigImageMouseHover(int,bool)),
             this,
             SLOT(slotImageMouseHover(int,bool)));

    /* Display the connection status info in tooltip */
    m_statusToolTip = new ToolTip(m_statusImage->x() + SCALE_WIDTH(25), m_statusImage->y() - SCALE_HEIGHT(4), "", this, START_X_START_Y);
    m_statusToolTip->setVisible(false);

    samasServerPortTextBox = new TextBox(samasServerIpTextbox->x(),
                                         samasServerIpTextbox->y() + samasServerIpTextbox->height(),
                                         samasServerIpTextbox->width(),
                                         BGTILE_HEIGHT,
                                         GN_STG_INT_SAMAS_PORT,
                                         TEXTBOX_SMALL,
                                         this,
                                         samasServerPortTextBoxParam,
                                         MIDDLE_TABLE_LAYER);
    m_elementList[GN_STG_INT_SAMAS_PORT] = samasServerPortTextBox;
    connect (samasServerPortTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (samasServerPortTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    integratedCosecCheckBox = new OptionSelectButton(samasServerPortTextBox->x(),
                                                     (samasServerPortTextBox->y() +
                                                      samasServerPortTextBox->height()),
                                                     samasServerIpTextbox->width(),
                                                     BGTILE_HEIGHT,CHECK_BUTTON_INDEX,
                                                     this, BOTTOM_TABLE_LAYER,
                                                     generalSettingElementStrings[GN_STG_INT_COSEC], "",SCALE_HEIGHT(20),
                                                     GN_STG_INT_COSEC,true,
                                                     NORMAL_FONT_SIZE);
    m_elementList[GN_STG_INT_COSEC] = integratedCosecCheckBox;
    connect (integratedCosecCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(devTableInfo->maxIpCam > 8)
    {
        m_recordingFormat->setVisible(false);
        m_recordingFormat->setIsEnabled(false);
    }
    else
    {
        m_recordingFormat->resetGeometry(m_liveViewPopUpDurationTextBox->x(),
                                         (m_liveViewPopUpDurationTextBox->y() + m_liveViewPopUpDurationTextBox->height()),
                                         BGTILE_MEDIUM_SIZE_WIDTH, BGTILE_HEIGHT);
    }

    #if defined(OEM_JCI)
    autoAddCameraCheckBox->setVisible(false);
    autoAddCameraCheckBox->setIsEnabled(false);
    autoAddCameraSetButton->setVisible(false);
    autoAddCameraSetButton->setIsEnabled(false);
    m_preVideoLossTextBox->resetGeometry(m_preVideoLossTextBox->x(), (m_preVideoLossTextBox->y() - (BGTILE_HEIGHT)),
                                         m_preVideoLossTextBox->width(), m_preVideoLossTextBox->height());
    m_autoCloseRecFailCheckBox->resetGeometry(m_autoCloseRecFailCheckBox->x(), (m_autoCloseRecFailCheckBox->y() - (BGTILE_HEIGHT)),
                                              m_autoCloseRecFailCheckBox->width(), m_autoCloseRecFailCheckBox->height());
    m_liveViewPopUpDurationTextBox->resetGeometry(m_liveViewPopUpDurationTextBox->x(), (m_liveViewPopUpDurationTextBox->y() - (BGTILE_HEIGHT)),
                                                  m_liveViewPopUpDurationTextBox->width(), m_liveViewPopUpDurationTextBox->height());
    m_recordingFormat->resetGeometry(m_recordingFormat->x(), (m_recordingFormat->y() - (BGTILE_HEIGHT)),
                                     m_recordingFormat->width(), m_recordingFormat->height());

    /* Hide Advanced Settings */
    m_advancedSettingTile->setVisible(false);
    integratedSamasCheckBox->setVisible(false);
    integratedSamasCheckBox->setIsEnabled(false);
    samasServerIpTextbox->setVisible(false);
    samasServerIpTextbox->setIsEnabled(false);
    m_statusImage->setVisible(false);
    samasServerPortTextBox->setVisible(false);
    samasServerPortTextBox->setIsEnabled(false);
    integratedCosecCheckBox->setVisible(false);
    integratedCosecCheckBox->setIsEnabled(false);
    #else
    resetGeometryOfCnfgbuttonRow(SCALE_HEIGHT(35));
    #endif
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void GeneralSetting::getConfig()
{
    /* create the payload for Get Cnfg */
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             GENERAL_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                     NETWORK_DEVICE_SETTING_TABLE_INDEX,
                                                     1,
                                                     MAX_REMOTE_DEVICES,
                                                     1,
                                                     1,
                                                     1,
                                                     payloadString);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void GeneralSetting::defaultConfig()
{
    if((recordingFormat != REC_NATIVE_FORMAT))
    {
        m_defaultButtonClick = true;
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_CFG_CHANGE_RESTART_SYS),true);
    }
    else
    {
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                                 GENERAL_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_TO_INDEX,
                                                                 CNFG_FRM_FIELD,
                                                                 MAX_FIELD_NO,
                                                                 0);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_DEF_CFG;
        param->payload = payloadString;
        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void GeneralSetting::saveConfig()
{
    if ((deviceNumberTextBox->doneKeyValidation()) && (singleDiskRecordTextBox->doneKeyValidation()) && (httpPortTextBox->doneKeyValidation())
            && (tcpPortTextBox->doneKeyValidation()) && (forwardedTcpPortTextBox->doneKeyValidation()) && (m_preVideoLossTextBox->doneKeyValidation())
            && (m_liveViewPopUpDurationTextBox->doneKeyValidation()) && (samasServerPortTextBox->doneKeyValidation()) && (deviceNameTextBox->doneKeyValidation()))
    {
        quint8 deviceCount = remoteDeviceList.count();
        for (quint8 index = 0; index < deviceCount; index++)
        {
            if (deviceNameTextBox->getInputText() == remoteDeviceList.at(index))
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETTING_LOCAL_NAME_MATCH_REMOTE));
                return;
            }
        }

        quint8 index = 0;
        qint32 httpPortVal = httpPortTextBox->getInputText().toInt();
        qint32 tcpPortVal = tcpPortTextBox->getInputText().toInt();
        qint32 rtpPort1 = rtspPort1TextBox->getInputText().toInt();
        qint32 rtpPort2 = rtspPort2TextBox->getInputText().toInt();
        QString samasServerIpStr = samasServerIpTextbox->getIpaddress();
        if (samasServerIpStr == "0.0.0.0")
        {
            samasServerIpStr = "";
        }

        if((tcpPortVal >= rtpPort1) && (tcpPortVal <= rtpPort2)) //if tcp port value is between the rtp port range values
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_TCP_NOT_IN_RTP_RANGE));
        }
        else if(httpPortVal == tcpPortVal) //if tcp port value is same as http Port value
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_HTTP_TCP_DIFF));
        }
        else if((httpPortVal >= rtpPort1) && (httpPortVal <= rtpPort2)) //if http port value is between the rtp port range value
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_HTTP_NOT_IN_RTP_RANGE));
        }
        else if((rtpPort1 % 2) == 1) //if rtp start port is odd number
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_INVALID_RTP_START));
        }
        else if((rtpPort2 - rtpPort1) <= 0) //if rtp start port and rtp end port both are same
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_RTP_RANGE_START_END));
        }
        else if((rtpPort2 - rtpPort1) < 64)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_VALUE_64_DIFF));
        }
        else if((m_recordingFormat->getIndexofCurrElement() == REC_NATIVE_FORMAT) && (recordingFormat != m_recordingFormat->getIndexofCurrElement())) //if recording Format change
        {
            if(recordingFormat == REC_AVI_FORMAT)
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(REC_FRM_NATIVE_TO_AVI),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else if (recordingFormat == REC_BOTH_FORMAT)
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GEN_SETT_CFG_CHANGE_RESTART_SYS),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
        }
        else if(((m_recordingFormat->getIndexofCurrElement() == REC_BOTH_FORMAT) && recordingFormat != m_recordingFormat->getIndexofCurrElement()))
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(REC_BOTH_WARNING),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else if(((m_recordingFormat->getIndexofCurrElement() == REC_AVI_FORMAT) && recordingFormat != m_recordingFormat->getIndexofCurrElement()))
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(REC_AVI_WARNING),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else if((integratedSamasCheckBox->getCurrentState() == ON_STATE) && (samasServerIpStr == ""))
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR));
        }
        else if((integratedSamasCheckBox->getCurrentState() == ON_STATE) && (samasServerPortTextBox->getInputText() == ""))
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(GEN_SETT_SAMAS_PORT_IN_RANGE));
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex (FIELD_DEV_NAME, deviceNameTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_SINGL_FILE_REC_DUR, singleDiskRecordTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_HTTP_PORT, httpPortTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_TCP_PORT, tcpPortTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_DEV_NO, deviceNumberTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_RTP_START_PORT, rtspPort1TextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_RTP_END_PORT, rtspPort2TextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_INTGT_COSEC, integratedCosecCheckBox->getCurrentState());
            payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_CONFIG, autoConfigCameraCheckBox->getCurrentState());
            payloadLib->setCnfgArrayAtIndex (FIELD_RECORDING_FORMAT, m_recordingFormat->getIndexofCurrElement());
            payloadLib->setCnfgArrayAtIndex (FIELD_DATE_FORMAT, dateFormatDropdown->getIndexofCurrElement());
            payloadLib->setCnfgArrayAtIndex (FIELD_TIME_FORMAT, timeFormatDropdown->getIndexofCurrElement());

            for(index = FIELD_AUTO_CONFIG_IP_RETAIN; index < AUTO_CNFG_FIELD; index++)
            {
                payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_CONFIG+index+1, autoConfigureStringList.at (index));
            }

            payloadLib->setCnfgArrayAtIndex (FIELD_INT_WITH_SAMAS_FLAG, (integratedSamasCheckBox->getCurrentState() == ON_STATE) ? 1 : 0);
            payloadLib->setCnfgArrayAtIndex (FIELD_INT_WITH_SAMAS_IP, samasServerIpStr);
            payloadLib->setCnfgArrayAtIndex (FIELD_INT_WITH_SAMAS_PORT, samasServerPortTextBox->getInputText());
            payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_CLOSE_REC_FAIL_ALERT, (m_autoCloseRecFailCheckBox->getCurrentState() == ON_STATE) ? 1 : 0);
            payloadLib->setCnfgArrayAtIndex (FIELD_VIDEO_POP_UP_DURATION, (m_liveViewPopUpDurationTextBox->getInputText()));
            payloadLib->setCnfgArrayAtIndex (FIELD_PRE_VIDEO_LOSS_DURATION, m_preVideoLossTextBox->getInputText());

            payloadLib->setCnfgArrayAtIndex ((FIELD_AUTO_CONFIG_USERNAME), autoConfigureStringList.at (GN_SET_USER_NAME));
            payloadLib->setCnfgArrayAtIndex ((FIELD_AUTO_CONFIG_PASSWORD), autoConfigureStringList.at (GN_SET_PASSWORD));

            payloadLib->setCnfgArrayAtIndex (FIELD_START_LIVE_VIEW_FLAG, (startLiveViewCheckBox->getCurrentState() == ON_STATE) ? 1 : 0);
            payloadLib->setCnfgArrayAtIndex (FIELD_FORWD_TCP_PORT, (forwardedTcpPortTextBox->getInputText()));
            payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_ADD_CAM, autoAddCameraCheckBox->getCurrentState());

            for(index = FIELD_AUTO_ADD_TCP_PORT; index < MAX_FIELD_AUTO_ADD_CAM; index++)
            {
                payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_ADD_CAM+index+1, autoAddCameraStringList.at (index));
            }

            QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                     GENERAL_TABLE_INDEX,
                                                                     CNFG_FRM_INDEX,
                                                                     CNFG_TO_INDEX,
                                                                     CNFG_FRM_FIELD,
                                                                     MAX_FIELD_NO,
                                                                     MAX_FIELD_NO);
            DevCommParam* param = new DevCommParam();
            param->msgType = MSG_SET_CFG;
            param->payload = payloadString;
            processBar->loadProcessBar();
            applController->processActivity(currDevName, DEVICE_COMM, param);
        }
    }
}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void GeneralSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    bool flagValue = false;
    processBar->unloadProcessBar();
    if(deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        if(tcpPortChangeFlag == true)
        {
            infoPage->loadInfoPageNoButton (QString("\n\n") + Multilang((ValidationMessage::getValidationMessage(MAIN_WINDOW_LOGIN_EXPIRED)).toUtf8().constData()), false, false);
            tcpPortChangeFlag = false;
        }
        else
        {
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }

        if(IS_VALID_OBJ(autoConfigureCamera))
        {
            infoPage->raise();
        }

        if(IS_VALID_OBJ(autoAddCamera))
        {
            infoPage->raise();
        }
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex(0) == GENERAL_TABLE_INDEX)
            {
                /* Fill all field value */
                deviceNumberTextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_DEV_NO).toString());
                deviceNameTextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_DEV_NAME).toString());
                singleDiskRecordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_SINGL_FILE_REC_DUR).toString());
                httpPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_HTTP_PORT).toString());
                tcpPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_TCP_PORT).toString());
                tcpPortValue = tcpPortTextBox->getInputText().toInt();
                rtspPort1TextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_RTP_START_PORT).toString());
                rtspPort2TextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_RTP_END_PORT).toString());

                integratedCosecCheckBox->changeState ((payloadLib->getCnfgArrayAtIndex (FIELD_INTGT_COSEC).toUInt() == 1) ? ON_STATE : OFF_STATE);
                flagValue = payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG).toBool();
                autoConfigureStringList.clear();
                for(quint8 index = FIELD_AUTO_CONFIG_IP_RETAIN; index < AUTO_CNFG_FIELD; index++)
                {
                    autoConfigureStringList.append ((payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG + index + 1)).toString());
                }

                autoConfigureStringList.append ((payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG_USERNAME)).toString());
                autoConfigureStringList.append ((payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG_PASSWORD)).toString());
                if((!flagValue) || (devTableInfo->maxIpCam == 0))
                {
                    autoConfigCameraCheckBox->changeState(OFF_STATE);
                    autoConfigureSetButton->setIsEnabled (false);
                }
                else
                {
                    autoConfigCameraCheckBox->changeState(ON_STATE);
                    autoConfigureSetButton->setIsEnabled (true);
                }

                recordingFormat = payloadLib->getCnfgArrayAtIndex (FIELD_RECORDING_FORMAT).toUInt();
                m_recordingFormat->setIndexofCurrElement(recordingFormat);
                dateFormatDropdown->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex (FIELD_DATE_FORMAT).toUInt());
                timeFormatDropdown->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex (FIELD_TIME_FORMAT).toUInt());

                flagValue = payloadLib->getCnfgArrayAtIndex (FIELD_INT_WITH_SAMAS_FLAG).toBool();
                integratedSamasCheckBox->changeState((flagValue == true)? ON_STATE : OFF_STATE);
                samasServerPortTextBox->setInputText (payloadLib->getCnfgArrayAtIndex (FIELD_INT_WITH_SAMAS_PORT).toString());
                samasServerIpTextbox->setIpaddress (payloadLib->getCnfgArrayAtIndex (FIELD_INT_WITH_SAMAS_IP).toString());
                samasServerPortTextBox->setIsEnabled (flagValue);
                samasServerIpTextbox->setIsEnabled (flagValue);
                flagValue = payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CLOSE_REC_FAIL_ALERT).toBool();
                m_autoCloseRecFailCheckBox->changeState((flagValue == true)? ON_STATE : OFF_STATE);

                //fill the text box for video pop-up duration
                m_liveViewPopUpDurationTextBox->setInputText(payloadLib->getCnfgArrayAtIndex (FIELD_VIDEO_POP_UP_DURATION).toString());

                //fill the text box for pre video loss duration
                m_preVideoLossTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_PRE_VIDEO_LOSS_DURATION).toString());

                flagValue = payloadLib->getCnfgArrayAtIndex (FIELD_START_LIVE_VIEW_FLAG).toBool();
                startLiveViewCheckBox->changeState((flagValue == true) ? ON_STATE : OFF_STATE);

                forwardedTcpPortTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(FIELD_FORWD_TCP_PORT).toString());

                flagValue = payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_ADD_CAM).toBool();
                autoAddCameraStringList.clear();
                for(quint8 index = FIELD_AUTO_ADD_TCP_PORT; index < MAX_FIELD_AUTO_ADD_CAM; index++)
                {
                    autoAddCameraStringList.append ((payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_ADD_CAM + index + 1)).toString());
                }

                autoAddCameraCheckBox->changeState(((!flagValue) || (devTableInfo->maxIpCam == 0)) ? OFF_STATE : ON_STATE);
            }

            if (payloadLib->getcnfgTableIndex(1) == NETWORK_DEVICE_SETTING_TABLE_INDEX)
            {
                QString remoteDeviceName;
                remoteDeviceList.clear();
                for (quint8 deviceIndex = 0; deviceIndex < MAX_REMOTE_DEVICES; deviceIndex++)
                {
                    remoteDeviceName = payloadLib->getCnfgArrayAtIndex(MAX_FIELD_NO + deviceIndex).toString();
                    if (remoteDeviceName != "")
                    {
                        remoteDeviceList.append(remoteDeviceName);
                    }
                }
            }

            /* get samas conn status*/
            getConnStatus();
        }
        break;

        case MSG_SET_CFG:
        {
            //load info page with msg
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            if(tcpPortValue != tcpPortTextBox->getInputText().toInt())
            {
                infoPage->loadInfoPageNoButton (QString("\n\n") + Multilang((ValidationMessage::getValidationMessage(MAIN_WINDOW_LOGIN_EXPIRED)).toUtf8().constData()), false, false);
            }

            slotObjectDelete();

            getConfig();
        }
        break;

        case MSG_DEF_CFG:
        {
            tcpPortValue = tcpPortTextBox->getInputText().toInt();
            if(tcpPortValue != DFLT_TCP_PORT)
            {
                tcpPortChangeFlag = true;
            }
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            if (param->cmdType != GET_STATUS)
            {
                break;
            }

            payloadLib->parseDevCmdReply(true, param->payload);
            if (STATUS_CMD_ID_SAMAS != (STATUS_CMD_ID_e)payloadLib->getCnfgArrayAtIndex(0).toUInt())
            {
                /* It is not for SAMAS status */
                break;
            }

            /* Parse the status and based on the status update the image and the tooltip */
            DI_CONNECTION_STATUS_e status = (DI_CONNECTION_STATUS_e)payloadLib->getCnfgArrayAtIndex(1).toUInt();
            m_statusImage->updateImageSource((status == DI_STATUS_CONNECTED) ? STATUS_CONNECTED : STATUS_DISCONNECTED, true);
            if (status < DI_STATUS_MAX)
            {
                m_statusToolTip->textChange(samasConnStatusString[status]);
            }
            else
            {
                m_statusToolTip->setVisible(false);
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

void GeneralSetting::handleInfoPageMessage (int index)
{
    if((m_defaultButtonClick == true) && (index == INFO_OK_BTN) && (infoPage->getText() == ValidationMessage::getValidationMessage(GEN_SETT_CFG_CHANGE_RESTART_SYS)))
    {
         recordingFormat = REC_NATIVE_FORMAT;
         defaultConfig();
    }
    else if((index == INFO_OK_BTN) && ((infoPage->getText() == ValidationMessage::getValidationMessage(GEN_SETT_CFG_CHANGE_RESTART_SYS))
                              || (infoPage->getText() == ValidationMessage::getValidationMessage(REC_BOTH_WARNING))
                              || (infoPage->getText() == ValidationMessage::getValidationMessage(REC_AVI_WARNING))
                              || (infoPage->getText() == ValidationMessage::getValidationMessage(REC_FRM_NATIVE_TO_AVI))))
    {
        recordingFormat = m_recordingFormat->getIndexofCurrElement();
        tcpPortValue = tcpPortTextBox->getInputText().toInt();
        saveConfig();
    }
   else
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    m_defaultButtonClick = false;
}

void GeneralSetting::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType)
{
    QString tempStr = "";
    switch(msgType)
    {
        case INFO_MSG_STRAT_CHAR:
            tempStr = Multilang("Please enter valid Device name.") + "\n" + Multilang(ValidationMessage::getValidationMessage(START_CHAR_ERROR_MSG).toUtf8().constData());
            break;

        case INFO_MSG_END_CHAR:
            tempStr = ValidationMessage::getValidationMessage(DEV_SETTING_END_CHAR_ERROR_MSG);
            break;

        case INFO_MSG_ERROR:
            switch(index)
            {
                case GN_STG_DEVICE_NO:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_DEV_RANGE_NO);
                    break;

                case GN_STG_DEVICE_NAME:
                    tempStr = ValidationMessage::getValidationMessage(DEV_NAME);
                    break;

                case GN_STG_SINGLE_FILEREC_DUR:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_SINGLE_FILE_RECORD);
                    break;

                case GN_STG_HTTP_PORT:
                    tempStr = ValidationMessage::getValidationMessage(HTTP_PORT_RANGE);
                    break;

                case GN_STG_TCP_PORT:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_TCP_PORT_RANGE);
                    break;

                case GN_STG_INT_SAMAS_PORT:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_SAMAS_PORT_IN_RANGE);
                    break;

                case GN_STG_LIVE_VIEW_POP_UP_DURATION:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_VIDEO_POP_UP_RANGE);
                    break;

                case GN_STG_PREVIDEO_LOSS_DURATION:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_PRE_VIDEO_LOSS_RANGE);
                    break;

                case GN_STG_FORWD_TCP_PORT:
                    tempStr = ValidationMessage::getValidationMessage(GEN_SETT_FORWARD_TCP_PORT_RANGE);
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    if(tempStr != "")
    {
        infoPage->loadInfoPage(tempStr);
    }
}

void GeneralSetting::slotOptionClicked(OPTION_STATE_TYPE_e currentState, int indexInPage)
{
    switch(indexInPage)
    {
        case GN_STG_AUTO_CONFIG_FLAG:
            autoConfigureSetButton->setIsEnabled((currentState == ON_STATE) ? true : false);
            break;

        case GN_STG_INT_SAMAS_FLAG:
            if(currentState == ON_STATE)
            {
                samasServerIpTextbox->setIsEnabled(true);
                samasServerPortTextBox->setIsEnabled(true);
            }
            else
            {
                samasServerIpTextbox->setIsEnabled(false);
                samasServerPortTextBox->setIsEnabled(false);
            }
            break;

        default:
            break;
    }
}

void GeneralSetting::slotPageOpenButtonClicked(int indexInPage)
{
    switch(indexInPage)
    {
        case GN_STG_AUTO_CONFIG_SET:
            if(autoConfigureCamera == NULL)
            {
                autoConfigureCamera = new AutoConfigureCamera(&autoConfigureStringList,currDevName,true,parentWidget());
                if(IS_VALID_OBJ(autoConfigureCamera))
                {
                    connect (autoConfigureCamera,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotObjectDelete()));
                }
            }
            break;

        case GN_STG_AUTO_ADD_CAM_SET:
            if(autoAddCamera == NULL)
            {
                autoAddCamera = new AutoAddCamera(&autoAddCameraStringList, parentWidget());

                if(IS_VALID_OBJ(autoAddCamera))
                {
                    connect (autoAddCamera,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotObjectDelete()));
                }
            }
            break;

        default:
            break;
    }
}

void GeneralSetting::slotObjectDelete()
{
    if(IS_VALID_OBJ(autoConfigureCamera))
    {
        disconnect (autoConfigureCamera,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotObjectDelete()));
        DELETE_OBJ(autoConfigureCamera);
    }

    if(IS_VALID_OBJ(autoAddCamera))
    {
        disconnect (autoAddCamera,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotObjectDelete()));
        DELETE_OBJ(autoAddCamera);
    }

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

/**
 * @brief GeneralSetting::getConnStatus
 */
void GeneralSetting::getConnStatus(void)
{
    if (integratedSamasCheckBox->getCurrentState() == ON_STATE)
    {
        /* Enable image state*/
        m_statusImage->setIsEnabled(true);
        payloadLib->setCnfgArrayAtIndex(0, STATUS_CMD_ID_SAMAS);

        /* send the command to get the connection status */
        sendCommand(GET_STATUS, 1);
    }
    else
    {
        /* when samas checkbox is disabled make image disabled */
        m_statusImage->updateImageSource(STATUS_DISABLED, false);

        /* Disable image state */
        m_statusImage->setIsEnabled(false);

        /* Hide tooltip */
        m_statusToolTip->setVisible(false);
    }
}

/**
 * @brief GeneralSetting::sendCommand
 * @param cmdType
 * @param totalfeilds
 */
void GeneralSetting::sendCommand(SET_COMMAND_e cmdType, int totalfeilds)
{
    /* Create payload to send cmd */
    QString payloadString = payloadLib->createDevCmdPayload(totalfeilds);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

/**
 * @brief GeneralSetting::slotImageMouseHover
 */
void GeneralSetting::slotImageMouseHover(int, bool isMouserHover)
{
    /* When mouse hover on status image, make tooltip enable or disable */
    m_statusToolTip->setVisible(isMouserHover);
    m_statusToolTip->raise();
}
