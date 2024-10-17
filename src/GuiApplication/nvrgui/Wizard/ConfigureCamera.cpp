#include "ConfigureCamera.h"


#define AUTO_CONFIG_PAGE_WIDTH              SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH)
#define AUTO_CONFIG_PAGE_HEIGHT             SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT)

#define INTER_CONTROL_MARGIN                SCALE_WIDTH(10)
#define MAX_BITRATE_INDEX                   16
#define LEFT_MARGIN_FROM_CENTER             SCALE_WIDTH(70)

#define TILE_WIDTH                          (SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - (2 * INTER_CONTROL_MARGIN))
#define HALF_TILE_WIDTH                     (TILE_WIDTH - INTER_CONTROL_MARGIN) / 2

#define MJPEG_STRING                        "Motion JPEG"
#define AUTO_CNFG_FIELD                     20
#define CNFG_TO_INDEX                       1
#ifdef BGTILE_HEIGHT
#undef BGTILE_HEIGHT
#endif
#define BGTILE_HEIGHT                       SCALE_HEIGHT(35)

static const QString autoConfigureStr[] =
{
    "Auto Configure Settings",
    "Retain Same IP Address",
    "Start IP Address",
    "End IP Address",
    "Retain Camera Default Profile",
    "Video Encoding",
    "Resolution",
    "Frame Rate",
    "CBR",
    "VBR",
    "Bit Rate",
    "Quality",
    "GOP",
    "Audio",
    "Username",
    "Password",
};

static QStringList encodinOptions = QStringList() << "Motion JPEG"
                                           << "H264"
                                           << "H265";

static QStringList resolutionOptions = QStringList() << "176x120"   << "176x144"
                                              << "320x240"   << "352x240"
                                              << "352x288"   << "640x480"
                                              << "704x240"   << "704x288"
                                              << "704x576"   << "720x480"
                                              << "720x576"   << "1280x720"
                                              << "1920x1080" << "2048x1536"
                                              << "2592x1520" << "2592x1944"
                                              << "3200x1800" << "3840x2160";


const QString autoConfigBitRateArray[MAX_BITRATE_INDEX] = {"32 kbps", "64 kbps",
                                                           "128 kbps", "256 kbps",
                                                           "384 kbps", "512 kbps",
                                                           "768 kbps", "1024 kbps",
                                                           "1536 kbps", "2048 kbps",
                                                           "3072 kbps", "4096 kbps",
                                                           "6144 kbps", "8192 kbps",
                                                           "12288 kbps","16384 kbps"};


ConfigureCamera::ConfigureCamera(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId):WizardCommon(parent,pageId)
{
    INIT_OBJ(m_sub_pageHeading);
    INIT_OBJ(m_ipRetainCheckBox);
    INIT_OBJ(m_startIpRange);
    INIT_OBJ(m_endIpRange);
    INIT_OBJ(m_usrNameTextBox);
    INIT_OBJ(m_usrNameTextBoxParam);
    INIT_OBJ(m_passwordTextBox);
    INIT_OBJ(m_passwordTextBoxParam);
    INIT_OBJ(m_profRetainCheckbox);
    INIT_OBJ(m_mainStreamElementHeading);
    INIT_OBJ(m_mainVideoEncodingPicklist);
    INIT_OBJ(m_mainFrameRatePicklist);
    INIT_OBJ(m_mainResolutionPicklist);
    INIT_OBJ(m_mainCBRTypeRadioButton);
    INIT_OBJ(m_mainVBRTypeRadioButton);
    INIT_OBJ(m_mainBitRatePicklist);
    INIT_OBJ(m_mainQualityPicklist);
    INIT_OBJ(m_mainGOPTextbox);
    INIT_OBJ(m_mainAudioCheckbox);

    INIT_OBJ(m_subStreamElementHeading);
    INIT_OBJ(m_subVideoEncodingPicklist);
    INIT_OBJ(m_subFrameRatePicklist);
    INIT_OBJ(m_subResolutionPicklist);
    INIT_OBJ(m_subCBRTypeRadioButton);
    INIT_OBJ(m_subVBRTypeRadioButton);
    INIT_OBJ(m_subBitRatePicklist);
    INIT_OBJ(m_subQualityPicklist);
    INIT_OBJ(m_subGOPTextbox);
    INIT_OBJ(m_subAudioCheckbox);
    INIT_OBJ(m_payloadLib);
    INIT_OBJ(m_infoPage);
    INIT_OBJ(m_backGround);
    INIT_OBJ(m_closeButton);
    INIT_OBJ(m_heading);
    INIT_OBJ(m_valueStringList);

    currDevName = devName;
    m_payloadLib = new PayloadLib();
    m_applController = ApplController::getInstance ();
    WizardCommon:: LoadProcessBar();
    createDefaultElements(subHeadStr);
    getElementlistConfig();
    this->show ();
}

ConfigureCamera :: ~ConfigureCamera()
{
    WizardCommon:: UnloadProcessBar();

    DELETE_OBJ(m_sub_pageHeading);
    if(IS_VALID_OBJ(m_ipRetainCheckBox))
    {
        disconnect (m_ipRetainCheckBox,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_ipRetainCheckBox);
    }

    if(IS_VALID_OBJ(m_startIpRange))
    {
        disconnect (m_startIpRange,
                    SIGNAL(sigEntryDone(quint32)),
                    this,
                    SLOT(slotIpAddressEntryDone(quint32)));
        disconnect(m_startIpRange,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpAddressLoadInfoPage(quint32)));
        DELETE_OBJ (m_startIpRange);
    }

    if(IS_VALID_OBJ(m_endIpRange))
    {
        disconnect(m_endIpRange,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpAddressLoadInfoPage(quint32)));
        DELETE_OBJ (m_endIpRange);
    }

    DELETE_OBJ(m_usrNameTextBox);
    DELETE_OBJ(m_usrNameTextBoxParam);

    DELETE_OBJ(m_passwordTextBox);
    DELETE_OBJ(m_passwordTextBoxParam);

    if(IS_VALID_OBJ(m_profRetainCheckbox))
    {
        disconnect (m_profRetainCheckbox,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_profRetainCheckbox);
    }

    DELETE_OBJ(m_mainStreamElementHeading);
    if(IS_VALID_OBJ(m_mainVideoEncodingPicklist))
    {
        disconnect(m_mainVideoEncodingPicklist,
                   SIGNAL(sigValueChanged(quint8,QString,int)),
                   this,
                   SLOT(slotValueChanged(quint8,QString,int)));
        DELETE_OBJ (m_mainVideoEncodingPicklist);
    }

    DELETE_OBJ (m_mainFrameRatePicklist);
    DELETE_OBJ (m_mainResolutionPicklist);

    if(IS_VALID_OBJ(m_mainCBRTypeRadioButton))
    {
        disconnect(m_mainCBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_mainCBRTypeRadioButton);
    }

    if(IS_VALID_OBJ(m_mainVBRTypeRadioButton))
    {
        disconnect(m_mainVBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_mainVBRTypeRadioButton);
    }

    DELETE_OBJ (m_mainBitRatePicklist);
    DELETE_OBJ (m_mainQualityPicklist);

    if(IS_VALID_OBJ(m_mainGOPTextbox))
    {
        disconnect(m_mainGOPTextbox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ (m_mainGOPTextbox);
        DELETE_OBJ (m_mainGOPParam);
    }

    if(IS_VALID_OBJ(m_mainAudioCheckbox))
    {
        disconnect(m_mainAudioCheckbox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_mainAudioCheckbox);
    }

    DELETE_OBJ (m_subStreamElementHeading);
    if(IS_VALID_OBJ(m_subVideoEncodingPicklist))
    {
        disconnect(m_subVideoEncodingPicklist,
                   SIGNAL(sigValueChanged(quint8,QString,int)),
                   this,
                   SLOT(slotValueChanged(quint8,QString,int)));

        DELETE_OBJ (m_subVideoEncodingPicklist);
    }

    DELETE_OBJ (m_subFrameRatePicklist);
    DELETE_OBJ (m_subResolutionPicklist);

    if(IS_VALID_OBJ(m_subCBRTypeRadioButton))
    {
        disconnect(m_subCBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_subCBRTypeRadioButton);
    }

    if(IS_VALID_OBJ(m_subVBRTypeRadioButton))
    {
        disconnect(m_subVBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_subVBRTypeRadioButton);
    }

    DELETE_OBJ (m_subBitRatePicklist);
    DELETE_OBJ (m_subQualityPicklist);

    if(IS_VALID_OBJ(m_subGOPTextbox))
    {
        disconnect(m_subGOPTextbox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ (m_subGOPTextbox);
        DELETE_OBJ (m_subGOPParam);
    }

    if(IS_VALID_OBJ(m_subAudioCheckbox))
    {
        disconnect(m_subAudioCheckbox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ (m_subAudioCheckbox);
    }

    DELETE_OBJ (m_payloadLib);
    WizardCommon:: UnloadProcessBar();

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect(m_infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ (m_infoPage);
    }
}

void ConfigureCamera::navigationKeyPressed()
{

}

void ConfigureCamera::createDefaultElements(QString subHeadStr)
{
    m_payloadLib = new PayloadLib();
    QMap<quint8, QString> tempMap;

    m_sub_pageHeading = new TextLabel(SCALE_WIDTH(477),
                                      SCALE_HEIGHT(23),
                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                      subHeadStr,
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y,
                                      0,
                                      false,
                                      SCALE_WIDTH(970),
                                      0);

   m_ipRetainCheckBox = new OptionSelectButton(SCALE_WIDTH(50),
                                                SCALE_HEIGHT(50),
                                                TILE_WIDTH,
                                                SCALE_HEIGHT(30),
                                                CHECK_BUTTON_INDEX,
                                                autoConfigureStr[1],
                                                this,
                                                COMMON_LAYER,
                                                SCALE_WIDTH(20),
                                                MX_OPTION_TEXT_TYPE_SUFFIX,
                                                NORMAL_FONT_SIZE,
                                                QUICK_AUTO_CONFIG_IP_RETAIN_FLAG);
   connect (m_ipRetainCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_startIpRange = new IpTextBox(m_ipRetainCheckBox->x(),
                                   m_ipRetainCheckBox->y () + m_ipRetainCheckBox->height () + SCALE_WIDTH(5),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   QUICK_AUTO_CONFIG_START_IP,
                                   autoConfigureStr[2],
                                   IP_ADDR_TYPE_IPV4_AND_IPV6, this,
                                   COMMON_LAYER,
                                   true, 0, true,
                                   IP_FIELD_TYPE_IPV6_ADDR,
                                   IP_TEXTBOX_ULTRALARGE,
                                   SCALE_WIDTH(50));
    connect (m_startIpRange,
             SIGNAL(sigEntryDone(quint32)),
             this,
             SLOT(slotIpAddressEntryDone(quint32)));
    connect(m_startIpRange,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpAddressLoadInfoPage(quint32)));

    m_endIpRange = new IpTextBox(m_startIpRange->x(),
                                 m_startIpRange->y() + m_startIpRange->height(),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 QUICK_AUTO_CONFIG_END_IP,
                                 autoConfigureStr[3],
                                 IP_ADDR_TYPE_IPV4_AND_IPV6, this,
                                 COMMON_LAYER,
                                 true, 0, true,
                                 IP_FIELD_TYPE_IPV6_ADDR,
                                 IP_TEXTBOX_ULTRALARGE,
                                 SCALE_WIDTH(50));

    connect(m_endIpRange,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpAddressLoadInfoPage(quint32)));

    m_usrNameTextBoxParam = new TextboxParam();
    m_usrNameTextBoxParam->labelStr = autoConfigureStr[14];
    m_usrNameTextBoxParam->isTotalBlankStrAllow = true;
    m_usrNameTextBoxParam->maxChar = 24;
    m_usrNameTextBoxParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_usrNameTextBox = new TextBox(m_startIpRange->x() + m_startIpRange->width() + SCALE_WIDTH(10) ,
                                   m_ipRetainCheckBox->y() + m_ipRetainCheckBox->height() + SCALE_WIDTH(5),
                                   SCALE_WIDTH(488) ,
                                   BGTILE_HEIGHT,
                                   QUICK_AUTO_CONFIG_USR_NAME,
                                   TEXTBOX_LARGE,
                                   this,
                                   m_usrNameTextBoxParam,
                                   COMMON_LAYER, true, false, false,
                                   LEFT_MARGIN_FROM_CENTER);

    m_passwordTextBoxParam = new TextboxParam();
    m_passwordTextBoxParam->labelStr = autoConfigureStr[15];
    m_passwordTextBoxParam->suffixStr = "(Max 20 chars)";
    m_passwordTextBoxParam->isTotalBlankStrAllow = true;
    m_passwordTextBoxParam->maxChar = 20;
    m_passwordTextBoxParam->minChar = 4;
    m_passwordTextBoxParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_passwordTextBox = new PasswordTextbox(m_usrNameTextBox->x()  ,
                                            m_usrNameTextBox->y() + m_usrNameTextBox->height(),
                                            SCALE_WIDTH(488) ,
                                            BGTILE_HEIGHT,
                                            QUICK_AUTO_CONFIG_PASSWD,
                                            TEXTBOX_LARGE,
                                            this,
                                            m_passwordTextBoxParam,
                                            COMMON_LAYER, true, LEFT_MARGIN_FROM_CENTER);

    m_profRetainCheckbox = new OptionSelectButton(m_endIpRange->x (),
                                                  m_endIpRange->y () + m_endIpRange->height () + SCALE_HEIGHT(5),
                                                  TILE_WIDTH,
                                                  SCALE_HEIGHT(30),
                                                  CHECK_BUTTON_INDEX,
                                                  autoConfigureStr[4],
                                                  this,
                                                  COMMON_LAYER,
                                                  SCALE_WIDTH(20),
                                                  MX_OPTION_TEXT_TYPE_SUFFIX,
                                                  NORMAL_FONT_SIZE,
                                                  QUICK_AUTO_CONFIG_PROF_RETAIN_FLAG);
    connect (m_profRetainCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_mainStreamElementHeading = new ElementHeading(m_profRetainCheckbox->x (),
                                                    (m_profRetainCheckbox->y() + m_profRetainCheckbox->height() + SCALE_HEIGHT(5)),
                                                    HALF_TILE_WIDTH,
                                                    BGTILE_HEIGHT,
                                                    "Main Stream",
                                                    TOP_LAYER,
                                                    this,
                                                    false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    tempMap.clear ();
    for(quint8 index = 0; index < encodinOptions.length (); index++)
    {
        tempMap.insert(index, encodinOptions.at (index));
    }

    m_mainVideoEncodingPicklist = new PickList(m_mainStreamElementHeading->x (),
                                               (m_mainStreamElementHeading->y() + m_mainStreamElementHeading->height()),
                                               HALF_TILE_WIDTH,
                                               BGTILE_HEIGHT,
                                               SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                               autoConfigureStr[5],
                                               tempMap,
                                               0, "Select Video Encoding",
                                               this,
                                               MIDDLE_TABLE_LAYER,
                                               -1,
                                               QUICK_AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST,
                                               true, true);
    connect(m_mainVideoEncodingPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotValueChanged(quint8,QString,int)));

    tempMap.clear ();
    for(quint8 index = 0; index < 60; index++)
    {
        tempMap.insert(index, QString("%1").arg (index + 1));
    }

    m_mainFrameRatePicklist = new PickList(m_mainVideoEncodingPicklist->x (),
                                           (m_mainVideoEncodingPicklist->y() + m_mainVideoEncodingPicklist->height()),
                                           HALF_TILE_WIDTH,
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                           autoConfigureStr[7],
                                           tempMap,
                                           0, "Select Frame Rate",
                                           this,
                                           MIDDLE_TABLE_LAYER,
                                           -1,
                                           QUICK_AUTO_CONFIG_MAIN_FRAMERATE_PICKLIST,
                                           true, true);

    tempMap.clear ();
    for(quint8 index = 0; index < resolutionOptions.length (); index++)
    {
        tempMap.insert(index, resolutionOptions.at (index));
    }

    m_mainResolutionPicklist = new PickList(m_mainFrameRatePicklist->x (),
                                            (m_mainFrameRatePicklist->y() + m_mainFrameRatePicklist->height()),
                                            HALF_TILE_WIDTH,
                                            BGTILE_HEIGHT,
                                            SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                            autoConfigureStr[6],
                                            tempMap,
                                            0, "Select Resolution",
                                            this,
                                            MIDDLE_TABLE_LAYER,
                                            -1,
                                            QUICK_AUTO_CONFIG_MAIN_RESOLUTION_PICKLIST,
                                            true, true);

    m_mainCBRTypeRadioButton = new OptionSelectButton(m_mainResolutionPicklist->x (),
                                                      (m_mainResolutionPicklist->y() + m_mainResolutionPicklist->height()),
                                                      HALF_TILE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      RADIO_BUTTON_INDEX,
                                                      this,
                                                      MIDDLE_TABLE_LAYER,
                                                      "Bit Rate Type",
                                                      autoConfigureStr[8],
                                                      -1,
                                                      QUICK_AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON,
                                                      true,
                                                      NORMAL_FONT_SIZE,
                                                      NORMAL_FONT_COLOR);
    connect(m_mainCBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_mainVBRTypeRadioButton = new OptionSelectButton((m_mainCBRTypeRadioButton->x() + SCALE_WIDTH(335)),
                                                      m_mainCBRTypeRadioButton->y(),
                                                      HALF_TILE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      RADIO_BUTTON_INDEX,
                                                      autoConfigureStr[9],
                                                      this,
                                                      NO_LAYER,
                                                      -1,
                                                      MX_OPTION_TEXT_TYPE_SUFFIX,
                                                      NORMAL_FONT_SIZE,
                                                      QUICK_AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON);
    connect(m_mainVBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    tempMap.clear ();
    for(quint8 index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        tempMap.insert(index, autoConfigBitRateArray[index]);
    }

    m_mainBitRatePicklist = new PickList(m_mainCBRTypeRadioButton->x (),
                                         (m_mainCBRTypeRadioButton->y() + m_mainCBRTypeRadioButton->height()),
                                         HALF_TILE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         autoConfigureStr[10],
                                         tempMap,
                                         0, "Select Bit Rate",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         QUICK_AUTO_CONFIG_MAIN_BITRATE_PICKLIST,
                                         true, true);

    tempMap.clear ();
    for(quint8 index = 0; index < 10; index++)
    {
        tempMap.insert(index, QString("%1").arg (index + 1));
    }

    m_mainQualityPicklist = new PickList(m_mainBitRatePicklist->x (),
                                         (m_mainBitRatePicklist->y() + m_mainBitRatePicklist->height()),
                                         HALF_TILE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         autoConfigureStr[11],
                                         tempMap,
                                         0, "Select Quality",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         QUICK_AUTO_CONFIG_MAIN_QUALITY_PICKLIST,
                                         true, true);

    m_mainGOPParam = new TextboxParam();
    m_mainGOPParam->maxChar = 3;
    m_mainGOPParam->validation = QRegExp("[0-9]");
    m_mainGOPParam->minNumValue = 1;
    m_mainGOPParam->maxNumValue = 100;
    m_mainGOPParam->isNumEntry = true;
    m_mainGOPParam->labelStr = autoConfigureStr[12];
    m_mainGOPParam->suffixStr = "(1-100)";

    m_mainGOPTextbox = new TextBox(m_mainQualityPicklist->x (),
                                   (m_mainQualityPicklist->y() + m_mainQualityPicklist->height()),
                                   HALF_TILE_WIDTH,
                                   BGTILE_HEIGHT,
                                   QUICK_AUTO_CONFIG_MAIN_GOP_TEXTBOX,
                                   TEXTBOX_EXTRASMALL,
                                   this,
                                   m_mainGOPParam,
                                   MIDDLE_TABLE_LAYER,
                                   true);
    connect(m_mainGOPTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_mainAudioCheckbox = new OptionSelectButton(m_mainGOPTextbox->x (),
                                                 (m_mainGOPTextbox->y() + m_mainGOPTextbox->height()),
                                                 HALF_TILE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 BOTTOM_TABLE_LAYER,
                                                 autoConfigureStr[13],
                                                 "" , -1,
                                                 QUICK_AUTO_CONFIG_MAIN_AUDIO_CHECKBOX);
    connect(m_mainAudioCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_subStreamElementHeading = new ElementHeading(m_mainStreamElementHeading->x() + m_mainStreamElementHeading->width () + INTER_CONTROL_MARGIN,
                                                   m_mainStreamElementHeading->y(),
                                                   HALF_TILE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   "Sub Stream",
                                                   TOP_LAYER,
                                                   this,
                                                   false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    tempMap.clear ();
    for(quint8 index = 0; index < encodinOptions.length (); index++)
    {
        tempMap.insert(index, encodinOptions.at (index));
    }

    m_subVideoEncodingPicklist = new PickList(m_subStreamElementHeading->x (),
                                              (m_subStreamElementHeading->y() + m_subStreamElementHeading->height()),
                                              HALF_TILE_WIDTH,
                                              BGTILE_HEIGHT,
                                              SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                              autoConfigureStr[5],
                                              tempMap,
                                              0, "Select Video Encoding",
                                              this,
                                              MIDDLE_TABLE_LAYER,
                                              -1,
                                              QUICK_AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST,
                                              true, true);
    connect(m_subVideoEncodingPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotValueChanged(quint8,QString,int)));

    tempMap.clear ();
    for(quint8 index = 0; index < 60; index++)
    {
        tempMap.insert(index, QString("%1").arg (index + 1));
    }

    m_subFrameRatePicklist = new PickList(m_subVideoEncodingPicklist->x (),
                                          (m_subVideoEncodingPicklist->y() + m_subVideoEncodingPicklist->height()),
                                          HALF_TILE_WIDTH,
                                          BGTILE_HEIGHT,
                                          SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                          autoConfigureStr[7],
                                          tempMap,
                                          0, "Select Frame Rate",
                                          this,
                                          MIDDLE_TABLE_LAYER,
                                          -1,
                                          QUICK_AUTO_CONFIG_SUB_FRAMERATE_PICKLIST,
                                          true, true);

    tempMap.clear ();
    for(quint8 index = 0; index < resolutionOptions.length (); index++)
    {
        tempMap.insert(index, resolutionOptions.at (index));
    }

    m_subResolutionPicklist = new PickList(m_subFrameRatePicklist->x (),
                                           (m_subFrameRatePicklist->y() + m_subFrameRatePicklist->height()),
                                           HALF_TILE_WIDTH,
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                           autoConfigureStr[6],
                                           tempMap,
                                           0, "Select Resolution",
                                           this,
                                           MIDDLE_TABLE_LAYER,
                                           -1,
                                           QUICK_AUTO_CONFIG_SUB_RESOLUTION_PICKLIST,
                                           true, true);

    m_subCBRTypeRadioButton = new OptionSelectButton(m_subResolutionPicklist->x (),
                                                     (m_subResolutionPicklist->y() + m_subResolutionPicklist->height()),
                                                     HALF_TILE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     RADIO_BUTTON_INDEX,
                                                     this,
                                                     MIDDLE_TABLE_LAYER,
                                                     "Bit Rate Type",
                                                     autoConfigureStr[8],
                                                     -1,
                                                     QUICK_AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON,
                                                     true,
                                                     NORMAL_FONT_SIZE,
                                                     NORMAL_FONT_COLOR);
    connect(m_subCBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_subVBRTypeRadioButton = new OptionSelectButton((m_subCBRTypeRadioButton->x() + SCALE_WIDTH(335)),
                                                     m_subCBRTypeRadioButton->y(),
                                                     HALF_TILE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     RADIO_BUTTON_INDEX,
                                                     autoConfigureStr[9],
                                                     this,
                                                     NO_LAYER,
                                                     -1,
                                                     MX_OPTION_TEXT_TYPE_SUFFIX,
                                                     NORMAL_FONT_SIZE,
                                                     QUICK_AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON);
    connect(m_subVBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    tempMap.clear ();
    for(quint8 index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        tempMap.insert(index, autoConfigBitRateArray[index]);
    }

    m_subBitRatePicklist = new PickList(m_subCBRTypeRadioButton->x (),
                                        (m_subCBRTypeRadioButton->y() + m_subCBRTypeRadioButton->height()),
                                        HALF_TILE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        autoConfigureStr[10],
                                        tempMap,
                                        0, "Select Bit Rate",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        QUICK_AUTO_CONFIG_SUB_BITRATE_PICKLIST,
                                        true, true);

    tempMap.clear ();
    for(quint8 index = 0; index < 10; index++)
    {
        tempMap.insert(index, QString("%1").arg (index + 1));
    }

    m_subQualityPicklist = new PickList(m_subBitRatePicklist->x (),
                                        (m_subBitRatePicklist->y() + m_subBitRatePicklist->height()),
                                        HALF_TILE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        autoConfigureStr[11],
                                        tempMap,
                                        0, "Select Quality",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        QUICK_AUTO_CONFIG_SUB_QUALITY_PICKLIST,
                                        true, true);

    m_subGOPParam = new TextboxParam();
    m_subGOPParam->maxChar = 3;
    m_subGOPParam->validation = QRegExp("[0-9]");
    m_subGOPParam->minNumValue = 1;
    m_subGOPParam->maxNumValue = 100;
    m_subGOPParam->isNumEntry = true;
    m_subGOPParam->labelStr = autoConfigureStr[12];
    m_subGOPParam->suffixStr = "(1-100)";

    m_subGOPTextbox = new TextBox(m_subQualityPicklist->x (),
                                  (m_subQualityPicklist->y() + m_subQualityPicklist->height()),
                                  HALF_TILE_WIDTH,
                                  BGTILE_HEIGHT,
                                  QUICK_AUTO_CONFIG_SUB_GOP_TEXTBOX,
                                  TEXTBOX_EXTRASMALL,
                                  this,
                                  m_subGOPParam,
                                  MIDDLE_TABLE_LAYER,
                                  true);
    connect(m_subGOPTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_subAudioCheckbox = new OptionSelectButton(m_subGOPTextbox->x (),
                                                (m_subGOPTextbox->y() + m_subGOPTextbox->height()),
                                                HALF_TILE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                BOTTOM_TABLE_LAYER,
                                                autoConfigureStr[13],
                                                "" , -1,
                                                QUICK_AUTO_CONFIG_SUB_AUDIO_CHECKBOX);
    connect(m_subAudioCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(1145),
                             SCALE_HEIGHT(750),
                             MAX_INFO_PAGE_TYPE,
                             parentWidget(), true);
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));
}

void ConfigureCamera::getElementlistConfig()
{
    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               CNFG_TO_INDEX,
                                                               CNFG_FRM_FIELD,
                                                               MAX_FIELD_NO,
                                                               0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    if(IS_VALID_OBJ(processBar))
    {
        processBar->loadProcessBar();
    }
    m_applController->processActivity(currDevName, DEVICE_COMM, param);
}

void ConfigureCamera::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName != currDevName)
    {
        if(IS_VALID_OBJ(processBar))
        {
            processBar->unloadProcessBar();
        }
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
                case MSG_GET_CFG:
                {
                    m_payloadLib->parsePayload(param->msgType, param->payload);
                    if(m_payloadLib->getcnfgTableIndex () == GENERAL_TABLE_INDEX)
                    {
                        quickElementList.clear ();
                        for(quint8 index = QUICK_FIELD_AUTO_CONFIG_IP_RETAIN; index < AUTO_CNFG_FIELD; index++)
                        {
                            quickElementList.append ((m_payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG + index + 1)).toString ());
                        }

                        quickElementList.append ((m_payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG_USERNAME)).toString ());
                        quickElementList.append ((m_payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_CONFIG_PASSWORD)).toString ());
                        m_valueStringList = &quickElementList;
                        fillRecords();
                        m_getConfig = true;
                    }
                }
                break;

                case MSG_SET_CFG:
                {
                    m_deletePage = true;
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                }
                break;

                case MSG_DEF_CFG:
                {
                    if(IS_VALID_OBJ(processBar))
                    {
                        processBar->unloadProcessBar();
                    }
                    getElementlistConfig();
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

    if(IS_VALID_OBJ(processBar))
    {
        processBar->unloadProcessBar();
    }
}


void ConfigureCamera::fillRecords()
{
    startIpRange = m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_START_IP);
    endIpRange = m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_END_IP);

    m_startIpRange->setIpaddress (m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_START_IP));
    m_endIpRange->setIpaddress (m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_END_IP));

    m_usrNameTextBox->setInputText( m_valueStringList->at (QUICK_AUTO_CONFIG_USER_NAME));
    m_passwordTextBox->setInputText(m_valueStringList->at(QUICK_AUTO_CONFIG_PASSWORD));

    if(m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_IP_RETAIN).toInt ())
    {
        m_ipRetainCheckBox->changeState (ON_STATE);
        m_startIpRange->setIsEnabled (false);
        m_endIpRange->setIsEnabled (false);
    }
    else
    {
        m_ipRetainCheckBox->changeState (OFF_STATE);
        m_startIpRange->setIsEnabled (true);
        m_endIpRange->setIsEnabled (true);
    }

    fillstreamRelatedParam();
    if(m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_PROF_RETAIN).toInt ())
    {
        m_profRetainCheckbox->changeState (ON_STATE);
        enableStreamRelatedElements(false);
    }
    else
    {
        m_profRetainCheckbox->changeState (OFF_STATE);
        enableStreamRelatedElements (true);
        upadateEnableStateForElements();
    }
}

void ConfigureCamera::upadateEnableStateForElements()
{
    bool isMainStreamMjpeg = (m_mainVideoEncodingPicklist->getCurrentPickStr() == QString(MJPEG_STRING));
    bool isSubStreamMjpeg = (m_subVideoEncodingPicklist->getCurrentPickStr() == QString(MJPEG_STRING));

    /* Main Stream */
    if (true == isMainStreamMjpeg)
    {
        m_mainCBRTypeRadioButton->setIsEnabled(false);
        m_mainVBRTypeRadioButton->setIsEnabled(false);
        m_mainGOPTextbox->setIsEnabled(false);
    }
    else
    {
        m_mainCBRTypeRadioButton->setIsEnabled(true);
        m_mainVBRTypeRadioButton->setIsEnabled(true);
        m_mainGOPTextbox->setIsEnabled(true);
    }

    if ((true == isMainStreamMjpeg) || (m_mainVBRTypeRadioButton->getCurrentState() == OFF_STATE))
    {
        m_mainQualityPicklist->setIsEnabled(false);
    }
    else
    {
        m_mainQualityPicklist->setIsEnabled(true);
    }

    /* Sub Stream */
    if (true == isSubStreamMjpeg)
    {
        m_subCBRTypeRadioButton->setIsEnabled(false);
        m_subVBRTypeRadioButton->setIsEnabled(false);
        m_subGOPTextbox->setIsEnabled(false);
    }
    else
    {
        m_subCBRTypeRadioButton->setIsEnabled(true);
        m_subVBRTypeRadioButton->setIsEnabled(true);
        m_subGOPTextbox->setIsEnabled(true);
    }
	
    if ((true == isSubStreamMjpeg) || (m_subVBRTypeRadioButton->getCurrentState() == OFF_STATE))
    {
        m_subQualityPicklist->setIsEnabled(false);
    }
    else
    {
        m_subQualityPicklist->setIsEnabled(true);
    }
}

void ConfigureCamera::enableStreamRelatedElements(bool isEnable)
{
    m_mainVideoEncodingPicklist->setIsEnabled (isEnable);
    m_subVideoEncodingPicklist->setIsEnabled (isEnable);

    m_mainResolutionPicklist->setIsEnabled (isEnable);
    m_subResolutionPicklist->setIsEnabled (isEnable);

    m_mainFrameRatePicklist->setIsEnabled (isEnable);
    m_subFrameRatePicklist->setIsEnabled (isEnable);

    m_mainCBRTypeRadioButton->setIsEnabled (isEnable);
    m_mainVBRTypeRadioButton->setIsEnabled (isEnable);

    m_subCBRTypeRadioButton->setIsEnabled (isEnable);
    m_subVBRTypeRadioButton->setIsEnabled (isEnable);

    m_mainBitRatePicklist->setIsEnabled (isEnable);
    m_subBitRatePicklist->setIsEnabled (isEnable);

    m_mainQualityPicklist->setIsEnabled (isEnable);
    m_subQualityPicklist->setIsEnabled (isEnable);

    m_mainGOPTextbox->setIsEnabled (isEnable);
    m_subGOPTextbox->setIsEnabled (isEnable);

    m_mainAudioCheckbox->setIsEnabled (isEnable);
    m_subAudioCheckbox->setIsEnabled (isEnable);
}

void ConfigureCamera::fillstreamRelatedParam()
{
    quint8 selectedIndex = 0;
    for(quint8 index = 0; index < encodinOptions.length (); index++)
    {
        if(encodinOptions.at (index) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING))
        {
            selectedIndex = index;
        }
    }
    m_mainVideoEncodingPicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < 30; index++)
    {
        if((index + 1) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_FRAMERATE).toInt ())
        {
            selectedIndex = index;
        }
    }
    m_mainFrameRatePicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < resolutionOptions.length (); index++)
    {
        if(resolutionOptions.at (index) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_RESOLUTION))
        {
            selectedIndex = index;
        }
    }
    m_mainResolutionPicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        if( index == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE).toInt ())
        {
            selectedIndex = index;
        }
    }
    m_mainBitRatePicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < 10; index++)
    {
        if( (index + 1) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_QUALITY).toInt ())
        {
            selectedIndex = index;
        }
    }
    m_mainQualityPicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < encodinOptions.length (); index++)
    {
        if(encodinOptions.at (index) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_VIDEOENCODING))
        {
            selectedIndex = index;
        }
    }
    m_subVideoEncodingPicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < 30; index++)
    {
        if((index + 1) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_FRAMERATE).toInt ())
        {
            selectedIndex = index;
        }
    }
    m_subFrameRatePicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < resolutionOptions.length (); index++)
    {
        if(resolutionOptions.at (index) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_RESOLUTION))
        {
            selectedIndex = index;
        }
    }
    m_subResolutionPicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        if( index == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE).toInt ())
        {
            selectedIndex = index;
        }
    }
    m_subBitRatePicklist->changeValue(selectedIndex);

    for(quint8 index = 0; index < 10; index++)
    {
        if( (index + 1) == m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_QUALITY).toInt ())
        {
            selectedIndex = index;
        }
    }
    m_subQualityPicklist->changeValue(selectedIndex);

    m_mainVideoEncodingPicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING));
    m_subVideoEncodingPicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_VIDEOENCODING));

    m_mainResolutionPicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_RESOLUTION));
    m_subResolutionPicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_RESOLUTION));

    m_mainFrameRatePicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_FRAMERATE));
    m_subFrameRatePicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_FRAMERATE));

    if(m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_BITERATE_TYPE).toInt ())
    {
        m_mainVBRTypeRadioButton->changeState (OFF_STATE);
        m_mainCBRTypeRadioButton->changeState (ON_STATE);
    }
    else
    {
        m_mainVBRTypeRadioButton->changeState (ON_STATE);
        m_mainCBRTypeRadioButton->changeState (OFF_STATE);
    }

    if(m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_TYPE).toInt ())
    {
        m_subVBRTypeRadioButton->changeState (OFF_STATE);
        m_subCBRTypeRadioButton->changeState (ON_STATE);
    }
    else
    {
        m_subVBRTypeRadioButton->changeState (ON_STATE);
        m_subCBRTypeRadioButton->changeState (OFF_STATE);
    }

    m_mainBitRatePicklist->setValue (autoConfigBitRateArray[m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE).toInt ()]);
    m_subBitRatePicklist->setValue (autoConfigBitRateArray[m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE).toInt ()]);

    m_mainQualityPicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_QUALITY));
    m_subQualityPicklist->setValue (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_QUALITY));

    m_mainGOPTextbox->setInputText (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_GOP));
    m_subGOPTextbox->setInputText (m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_GOP));

    m_mainAudioCheckbox->changeState ((OPTION_STATE_TYPE_e)m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_MAIN_AUDIO).toInt ());
    m_subAudioCheckbox->changeState ((OPTION_STATE_TYPE_e)m_valueStringList->at (QUICK_FIELD_AUTO_CONFIG_SUB_AUDIO).toInt ());
}

void ConfigureCamera::saveConfig()
{
    if(m_getConfig == false)
    {
        return;
    }

    if(validationOnOkButton() == false)
    {
        return;
    }
    else
    {
        if((m_startIpRange->getIpaddress () != startIpRange) || (m_endIpRange->getIpaddress () != endIpRange))
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(AUTO_CONF_NEW_CAM), true);
        }
        else
        {
            setRecords();
        }
    }
}

void ConfigureCamera::setRecords()
{
    m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_IP_RETAIN, QString("%1").arg ((int)m_ipRetainCheckBox->getCurrentState ()));
    m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_PROF_RETAIN, QString("%1").arg ((int)m_profRetainCheckbox->getCurrentState ()));
    m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_START_IP, m_startIpRange->getIpaddress ());
    m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_END_IP, m_endIpRange->getIpaddress ());
    m_valueStringList->replace (QUICK_AUTO_CONFIG_USER_NAME, m_usrNameTextBox->getInputText());
    m_valueStringList->replace (QUICK_AUTO_CONFIG_PASSWORD, m_passwordTextBox->getInputText());

    if(m_profRetainCheckbox->getCurrentState () == OFF_STATE)
    {
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING, m_mainVideoEncodingPicklist->getCurrentPickStr ());
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_VIDEOENCODING, m_subVideoEncodingPicklist->getCurrentPickStr ());

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_RESOLUTION, m_mainResolutionPicklist->getCurrentPickStr ());
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_RESOLUTION, m_subResolutionPicklist->getCurrentPickStr ());

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_FRAMERATE, m_mainFrameRatePicklist->getCurrentPickStr ());
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_FRAMERATE, m_subFrameRatePicklist->getCurrentPickStr ());

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_BITERATE_TYPE, QString("%1").arg ((int)m_mainCBRTypeRadioButton->getCurrentState ()));
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_TYPE, QString("%1").arg ((int)m_subCBRTypeRadioButton->getCurrentState ()));

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE, QString("%1").arg (findIndexofBitrateValue(m_mainBitRatePicklist->getCurrentPickStr ())));
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE, QString("%1").arg (findIndexofBitrateValue(m_subBitRatePicklist->getCurrentPickStr ())));

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_QUALITY, m_mainQualityPicklist->getCurrentPickStr ());
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_QUALITY, m_subQualityPicklist->getCurrentPickStr ());

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_GOP, m_mainGOPTextbox->getInputText ());
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_GOP, m_subGOPTextbox->getInputText ());

        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_MAIN_AUDIO, QString("%1").arg ((int)m_mainAudioCheckbox->getCurrentState ()));
        m_valueStringList->replace (QUICK_FIELD_AUTO_CONFIG_SUB_AUDIO, QString("%1").arg ((int)m_subAudioCheckbox->getCurrentState ()));
    }


    for(quint8 index = QUICK_FIELD_AUTO_CONFIG_IP_RETAIN; index < AUTO_CNFG_FIELD; index++)
    {
        m_payloadLib->setCnfgArrayAtIndex ((index ), m_valueStringList->at (index));
    }
    for(quint8 index = AUTO_CNFG_FIELD; index < QUICK_MAX_AUTO_CONFIG; index++)
    {
        m_payloadLib->setCnfgArrayAtIndex ((index ), m_valueStringList->at (index));
    }

    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               CNFG_TO_INDEX,
                                                               FIELD_AUTO_CONFIG +2,    // start with Auto Retain IP flag
                                                               38,                      // sub stream Audio.
                                                               AUTO_CNFG_FIELD);

    payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                       GENERAL_TABLE_INDEX,
                                                       CNFG_FRM_INDEX,
                                                       CNFG_TO_INDEX,
                                                       FIELD_AUTO_CONFIG_USERNAME +1,   // start with UserName
                                                       FIELD_AUTO_CONFIG_PASSWORD +1,   // password.
                                                       2,
                                                       payloadString,AUTO_CNFG_FIELD);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    m_applController->processActivity(currDevName, DEVICE_COMM, param);
}

bool ConfigureCamera::validationOnOkButton()
{
    QString startIp = m_startIpRange->getIpaddress();
    QString endIp = m_endIpRange->getIpaddress();
    QString startIpNetworkAddr;
    QString endIpNetworkAddr;
    qint8 startIpSeparatorIndex;
    qint8 endIpSeparatorIndex;

    if(m_ipRetainCheckBox->getCurrentState () == OFF_STATE)
    {
        if((startIp == "") || (endIp == ""))
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_IP_ADDR_RANGE));
            return false;
        }

        if (QHostAddress(startIp).protocol() != QHostAddress(endIp).protocol())
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_IP_TYPE_DIFF));
            return false;
        }

        if (startIp == endIp)
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(AUTO_CONF_IP_STR_END_DIFF));
            return false;
        }

        if (QHostAddress(startIp).protocol() == QAbstractSocket::IPv4Protocol)
        {
            startIpSeparatorIndex = startIp.lastIndexOf('.');
            startIpNetworkAddr = startIp.left(startIpSeparatorIndex);
            endIpSeparatorIndex = endIp.lastIndexOf('.');
            endIpNetworkAddr = endIp.left(endIpSeparatorIndex);
        }
        else
        {
            startIpSeparatorIndex = startIp.lastIndexOf(':');
            startIpNetworkAddr = startIp.left(startIpSeparatorIndex);
            endIpSeparatorIndex = endIp.lastIndexOf(':');
            endIpNetworkAddr = endIp.left(endIpSeparatorIndex);
        }

        if (startIpNetworkAddr != endIpNetworkAddr)
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(AUTO_CONF_IP_STR_END_SAME));
            return false;
        }

        if(*m_startIpRange > *m_endIpRange)
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(START_IP_ADDR_LESS_END_IP_ADDR));
            return false;
        }
    }

    if(m_usrNameTextBoxParam->textStr == "")
    {
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_USER_NAME_ERROR));
        return false;
    }

    if(m_passwordTextBoxParam->textStr == "")
    {
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(LOGIN_PASSWORD_ERROR));
        return false;
    }

    return true;
}

quint8 ConfigureCamera::findIndexofBitrateValue(QString bitRateStr)
{
    quint8 index = 0;
    for(index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        if(autoConfigBitRateArray[index] == bitRateStr)
        {
            break;
        }
    }
    return index;
}

void ConfigureCamera::slotOptSelButtonClicked(OPTION_STATE_TYPE_e state, int index)
{
    switch(index)
    {
        case QUICK_AUTO_CONFIG_IP_RETAIN_FLAG:
            if(state == ON_STATE)
            {
                m_startIpRange->setIsEnabled (false);
                m_endIpRange->setIsEnabled (false);
            }
            else
            {
                m_startIpRange->setIsEnabled (true);
                m_endIpRange->setIsEnabled (true);
            }
            break;

        case QUICK_AUTO_CONFIG_PROF_RETAIN_FLAG:
            if(state == ON_STATE)
            {
                enableStreamRelatedElements(false);
            }
            else
            {
                enableStreamRelatedElements(true);
                upadateEnableStateForElements();
            }
            break;

        case QUICK_AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON:
            m_mainVBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        case QUICK_AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON:
            m_mainCBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        case QUICK_AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON:
            m_subVBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        case QUICK_AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON:
            m_subCBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        default:
            break;
    }
}

void ConfigureCamera::slotIpAddressEntryDone(quint32)
{
    QString startIp = m_startIpRange->getIpaddress();
    QString endIp = m_endIpRange->getIpaddress();

    if(QHostAddress(startIp).protocol() == QAbstractSocket::IPv4Protocol)
    {
        QStringList startIpOctes = startIp.split('.');
        QStringList endIpOctes = endIp.split('.');
        if ((startIpOctes.size() == 4) && (endIpOctes.size() == 4))
        {
            endIpOctes[0] = startIpOctes[0];
            endIpOctes[1] = startIpOctes[1];
            endIpOctes[2] = startIpOctes[2];

            m_endIpRange->setIpaddress(endIpOctes.join('.'));
        }
        else
        {
            startIpOctes[3] = "255";
            m_endIpRange->setIpaddress(startIpOctes.join('.'));
        }
    }
    else
    {
        qint8 startIpLastColonIndex = startIp.lastIndexOf(':');
        QString startIpInitialGroups = startIp.left(startIpLastColonIndex);

        qint8 endIpLastColonIndex = endIp.lastIndexOf(':');
        if (endIpLastColonIndex != -1)
        {
            QString endIpLastGroup = endIp.mid(endIpLastColonIndex + 1);
            m_endIpRange->setIpaddress(startIpInitialGroups + ":" + endIpLastGroup);
        }
        else
        {
            m_endIpRange->setIpaddress(startIpInitialGroups + ":ffff");
        }
    }
}

void ConfigureCamera::slotIpAddressLoadInfoPage(quint32)
{
    WizardCommon::InfoPageImage();
    m_infoPage->raise();
    m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VALID_IP_ADD));
    m_startIpRange->setIpaddress (m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_START_IP));
    m_endIpRange->setIpaddress (m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_END_IP));
}

void ConfigureCamera::slotValueChanged(quint8, QString, int index)
{
    switch(index)
    {
        case QUICK_AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST:
        case QUICK_AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST:
            upadateEnableStateForElements ();
            break;

        default:
            break;
    }
}

void ConfigureCamera::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e infoMsg)
{
    if(infoMsg == INFO_MSG_ERROR)
    {
        if(index == QUICK_AUTO_CONFIG_MAIN_GOP_TEXTBOX)
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_MAIN_GOP));
        }
        else if(index == QUICK_AUTO_CONFIG_SUB_GOP_TEXTBOX)
        {
            WizardCommon::InfoPageImage();
            m_infoPage->raise();
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_SUB_GOP));
        }
    }
}

void ConfigureCamera::slotInfoPageBtnclick(int index)
{
    WizardCommon::UnloadInfoPageImage();

    if ((m_infoPage->getText() != ValidationMessage::getValidationMessage(AUTO_CONF_NEW_CAM)))
    {
        return;
    }

    if (index == INFO_OK_BTN)
    {
        startIpRange = m_startIpRange->getIpaddress();
        endIpRange = m_endIpRange->getIpaddress();
        setRecords();
    }
    else
    {
        m_startIpRange->setIpaddress (m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_START_IP));
        m_endIpRange->setIpaddress (m_valueStringList->at(QUICK_FIELD_AUTO_CONFIG_END_IP));
    }
}
