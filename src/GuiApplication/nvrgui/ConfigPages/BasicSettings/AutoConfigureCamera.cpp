#include "AutoConfigureCamera.h"
#include "ValidationMessage.h"
#include <QKeyEvent>

#define AUTO_CONFIG_PAGE_WIDTH              SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH)
#define AUTO_CONFIG_PAGE_HEIGHT             SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT)

#define INTER_CONTROL_MARGIN                SCALE_WIDTH(10)
#define MAX_BITRATE_INDEX                   16

#define TILE_WIDTH                         (SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - (2 * INTER_CONTROL_MARGIN))
#define HALF_TILE_WIDTH                    (TILE_WIDTH - INTER_CONTROL_MARGIN) / 2

#define MJPEG_STRING                        "Motion JPEG"
#define AUTO_CNFG_FIELD                     20
#define CNFG_TO_INDEX                       1

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

const QString autoConfigBitRateArray[MAX_BITRATE_INDEX] = {"32 kbps", "64 kbps",
                                                           "128 kbps", "256 kbps",
                                                           "384 kbps", "512 kbps",
                                                           "768 kbps", "1024 kbps",
                                                           "1536 kbps", "2048 kbps",
                                                           "3072 kbps", "4096 kbps",
                                                           "6144 kbps", "8192 kbps",
                                                           "12288 kbps","16384 kbps"};

AutoConfigureCamera::AutoConfigureCamera(QStringList *elementList, QString deviceName, bool flag, QWidget *parent) : KeyBoard(parent)
{
    this->setGeometry(0, 0, parent->width(), parent->height());
    m_valueStringList = elementList;
    m_applController = ApplController::getInstance();
    m_currDeviceName = deviceName;
    m_isEnable = flag;
    createDefaultElements();
    fillRecords();
    startIpAddress = m_valueStringList->at(FIELD_AUTO_CONFIG_START_IP);
    endIpAddress = m_valueStringList->at(FIELD_AUTO_CONFIG_END_IP);

    m_currElement = AUTO_CONFIG_IP_RETAIN_FLAG;
    m_elementlist[m_currElement]->forceActiveFocus();
    this->show();
}

AutoConfigureCamera::~AutoConfigureCamera()
{
    DELETE_OBJ(m_backGround);
    if(IS_VALID_OBJ(m_closeButton))
    {
        disconnect(m_closeButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClick(int)));
        disconnect(m_closeButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_closeButton);
    }

    DELETE_OBJ(m_heading);
    if(IS_VALID_OBJ(m_ipRetainCheckBox))
    {
        disconnect(m_ipRetainCheckBox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        disconnect(m_ipRetainCheckBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_ipRetainCheckBox);
    }

    if(IS_VALID_OBJ(m_startIpAddress))
    {
        disconnect(m_startIpAddress,
                   SIGNAL(sigEntryDone(quint32)),
                   this,
                   SLOT(slotIpAddressEntryDone(quint32)));
        disconnect(m_startIpAddress,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpAddressLoadInfoPage(quint32)));

        disconnect(m_startIpAddress,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_startIpAddress);
    }

    if(IS_VALID_OBJ(m_endIpAddress))
    {
        disconnect(m_endIpAddress,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_endIpAddress,
                   SIGNAL(sigLoadInfopage(quint32)),
                   this,
                   SLOT(slotIpAddressLoadInfoPage(quint32)));
        DELETE_OBJ(m_endIpAddress);
    }

    if(IS_VALID_OBJ(m_usrNameTextBox))
    {
        disconnect(m_usrNameTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_usrNameTextBox);
    }
    DELETE_OBJ(m_usrNameTextBoxParam);

    if(IS_VALID_OBJ(m_passwordTextBox))
    {
        disconnect(m_passwordTextBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_passwordTextBox);
    }
    DELETE_OBJ(m_passwordTextBoxParam);

    if(IS_VALID_OBJ(m_profRetainCheckbox))
    {
        disconnect(m_profRetainCheckbox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        disconnect(m_profRetainCheckbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_profRetainCheckbox);
    }

    delete m_mainStreamElementHeading;

    if(IS_VALID_OBJ(m_mainVideoEncodingPicklist))
    {
        disconnect(m_mainVideoEncodingPicklist,
                   SIGNAL(sigValueChanged(quint8,QString,int)),
                   this,
                   SLOT(slotValueChanged(quint8,QString,int)));
        disconnect(m_mainVideoEncodingPicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_mainVideoEncodingPicklist);
    }

    if(IS_VALID_OBJ(m_mainFrameRatePicklist))
    {
        disconnect(m_mainFrameRatePicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_mainFrameRatePicklist);
    }

    if(IS_VALID_OBJ(m_mainResolutionPicklist))
    {
        disconnect(m_mainResolutionPicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_mainResolutionPicklist);
    }

    if(IS_VALID_OBJ(m_mainCBRTypeRadioButton))
    {
        disconnect(m_mainCBRTypeRadioButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_mainCBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_mainCBRTypeRadioButton);
    }

    if(IS_VALID_OBJ(m_mainVBRTypeRadioButton))
    {
        disconnect(m_mainVBRTypeRadioButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_mainVBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_mainVBRTypeRadioButton);
    }

    if(IS_VALID_OBJ(m_mainBitRatePicklist))
    {
        disconnect(m_mainBitRatePicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_mainBitRatePicklist);
    }

    if(IS_VALID_OBJ(m_mainQualityPicklist))
    {
        disconnect(m_mainQualityPicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_mainQualityPicklist);
    }

    if(IS_VALID_OBJ(m_mainGOPTextbox))
    {
        disconnect(m_mainGOPTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_mainGOPTextbox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(m_mainGOPTextbox);
        DELETE_OBJ(m_mainGOPParam);
    }

    if(IS_VALID_OBJ(m_mainAudioCheckbox))
    {
        disconnect(m_mainAudioCheckbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_mainAudioCheckbox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_mainAudioCheckbox);
    }

    // Sub elements
    DELETE_OBJ(m_subStreamElementHeading);

    if(IS_VALID_OBJ(m_subVideoEncodingPicklist))
    {
        disconnect(m_subVideoEncodingPicklist,
                   SIGNAL(sigValueChanged(quint8,QString,int)),
                   this,
                   SLOT(slotValueChanged(quint8,QString,int)));
        disconnect(m_subVideoEncodingPicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_subVideoEncodingPicklist);
    }

    if(IS_VALID_OBJ(m_subFrameRatePicklist))
    {
        disconnect(m_subFrameRatePicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_subFrameRatePicklist);
    }

    if(IS_VALID_OBJ(m_subResolutionPicklist))
    {
        disconnect(m_subResolutionPicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_subResolutionPicklist);
    }

    if(IS_VALID_OBJ(m_subCBRTypeRadioButton))
    {
        disconnect(m_subCBRTypeRadioButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_subCBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_subCBRTypeRadioButton);
    }

    if(IS_VALID_OBJ(m_subVBRTypeRadioButton))
    {
        disconnect(m_subVBRTypeRadioButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_subVBRTypeRadioButton,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_subVBRTypeRadioButton);
    }

    if(IS_VALID_OBJ(m_subBitRatePicklist))
    {
        disconnect(m_subBitRatePicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_subBitRatePicklist);
    }

    if(IS_VALID_OBJ(m_subQualityPicklist))
    {
        disconnect(m_subQualityPicklist,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_subQualityPicklist);
    }

    if(IS_VALID_OBJ(m_subGOPTextbox))
    {
        disconnect(m_subGOPTextbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_subGOPTextbox,
                   SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                   this,
                   SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
        DELETE_OBJ(m_subGOPTextbox);
        DELETE_OBJ(m_subGOPParam);
    }

    if(IS_VALID_OBJ(m_subAudioCheckbox))
    {
        disconnect(m_subAudioCheckbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_subAudioCheckbox,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_subAudioCheckbox);
    }

    if(IS_VALID_OBJ(m_saveBtn))
    {
        disconnect(m_saveBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClick(int)));
        disconnect(m_saveBtn,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
         DELETE_OBJ(m_saveBtn);
    }

    if(IS_VALID_OBJ(m_okBtn))
    {
        disconnect(m_okBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClick(int)));
        disconnect(m_okBtn,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_okBtn);
    }

    if(IS_VALID_OBJ(m_cancleBtn))
    {
        disconnect(m_cancleBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCloseButtonClick(int)));

        disconnect(m_cancleBtn,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
         DELETE_OBJ(m_cancleBtn);
    }

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect(m_infoPage,
                   SIGNAL(sigInfoPageCnfgBtnClick(int)),
                   this,
                   SLOT(slotInfoPageBtnclick(int)));
         DELETE_OBJ(m_infoPage);
    }

    DELETE_OBJ(m_payloadLib);
}

void AutoConfigureCamera::createDefaultElements()
{
    quint8 selectedKey = 0;
    QMap<quint8, QString> tempMap;
    QStringList encodinOptions = QStringList() << "Motion JPEG" << "H264" << "H265";
    QStringList resolutionOptions = QStringList() << "176x120"   << "176x144"
                                                  << "320x240"   << "352x240"
                                                  << "352x288"   << "640x480"
                                                  << "704x240"   << "704x288"
                                                  << "704x576"   << "720x480"
                                                  << "720x576"   << "1280x720"
                                                  << "1920x1080" << "2048x1536"
                                                  << "2592x1520" << "2592x1944"
                                                  << "3200x1800" << "3840x2160";
    m_payloadLib = new PayloadLib();

    m_backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(36),
                                 SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + SCALE_HEIGHT(50),
                                 AUTO_CONFIG_PAGE_WIDTH,
                                 AUTO_CONFIG_PAGE_HEIGHT,
                                 0,
                                 NORMAL_BKG_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this);

    m_closeButton = new CloseButtton((m_backGround->x() + m_backGround->width() - SCALE_WIDTH(20)),
                                    (m_backGround->y() + SCALE_HEIGHT(30)),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     AUTO_CONFIG_CLOSE_BUTTON);
    m_elementlist[AUTO_CONFIG_CLOSE_BUTTON] = m_closeButton;
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClick(int)));
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_heading = new Heading((m_backGround->x() +(m_backGround->width() / 2)),
                           (m_backGround->y() + SCALE_HEIGHT(30)),
                            autoConfigureStr[0],
                            this,
                            HEADING_TYPE_2);

    m_ipRetainCheckBox = new OptionSelectButton(m_backGround->x() + SCALE_WIDTH(10),
                                                m_backGround->y() + SCALE_HEIGHT(60),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this, COMMON_LAYER,
                                                autoConfigureStr[1],
                                                " ", SCALE_WIDTH(10), AUTO_CONFIG_IP_RETAIN_FLAG);
    m_elementlist[AUTO_CONFIG_IP_RETAIN_FLAG] = m_ipRetainCheckBox;
    connect(m_ipRetainCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_ipRetainCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_startIpAddress = new IpTextBox(m_ipRetainCheckBox->x(),
                                   m_ipRetainCheckBox->y() + m_ipRetainCheckBox->height(),
                                   HALF_TILE_WIDTH,
                                   BGTILE_HEIGHT,
                                   AUTO_CONFIG_START_IP,
                                   autoConfigureStr[2],
                                   IP_ADDR_TYPE_IPV4_AND_IPV6, this,
                                   COMMON_LAYER,
                                   true, 0, true,
                                   IP_FIELD_TYPE_IPV6_ADDR,
                                   IP_TEXTBOX_ULTRALARGE,
                                   SCALE_WIDTH(50));

    m_elementlist[AUTO_CONFIG_START_IP] = m_startIpAddress;
    connect(m_startIpAddress,
            SIGNAL(sigEntryDone(quint32)),
            this,
            SLOT(slotIpAddressEntryDone(quint32)));
    connect(m_startIpAddress,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpAddressLoadInfoPage(quint32)));
    connect(m_startIpAddress,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_endIpAddress = new IpTextBox(m_startIpAddress->x(),
                                 m_startIpAddress->y() + m_startIpAddress->height(),
                                 HALF_TILE_WIDTH,
                                 BGTILE_HEIGHT,
                                 AUTO_CONFIG_END_IP,
                                 autoConfigureStr[3],
                                 IP_ADDR_TYPE_IPV4_AND_IPV6, this,
                                 COMMON_LAYER,
                                 true, 0, true,
                                 IP_FIELD_TYPE_IPV6_ADDR,
                                 IP_TEXTBOX_ULTRALARGE,
                                 SCALE_WIDTH(50));

    m_elementlist[AUTO_CONFIG_END_IP] = m_endIpAddress;
    connect(m_endIpAddress,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_endIpAddress,
            SIGNAL(sigLoadInfopage(quint32)),
            this,
            SLOT(slotIpAddressLoadInfoPage(quint32)));

    m_usrNameTextBoxParam = new TextboxParam();
    m_usrNameTextBoxParam->labelStr = autoConfigureStr[14];
    m_usrNameTextBoxParam->isTotalBlankStrAllow = true;
    m_usrNameTextBoxParam->maxChar = 24;
    m_usrNameTextBoxParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_usrNameTextBox = new TextBox(m_ipRetainCheckBox->x() + m_ipRetainCheckBox->width() + INTER_CONTROL_MARGIN,
                                   m_ipRetainCheckBox->y(),
                                   HALF_TILE_WIDTH ,
                                   BGTILE_HEIGHT,
                                   AUTO_CONFIG_USR_NAME,
                                   TEXTBOX_LARGE,
                                   this,
                                   m_usrNameTextBoxParam,
                                   COMMON_LAYER, true, false, false,
                                   SCALE_WIDTH(70));
    m_elementlist[AUTO_CONFIG_USR_NAME] = m_usrNameTextBox;
    connect(m_usrNameTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_passwordTextBoxParam = new TextboxParam();
    m_passwordTextBoxParam->labelStr = autoConfigureStr[15];
    m_passwordTextBoxParam->suffixStr = "(Max 20 chars)";
    m_passwordTextBoxParam->isTotalBlankStrAllow = true;
    m_passwordTextBoxParam->maxChar = 20;
    m_passwordTextBoxParam->minChar = 4;
    m_passwordTextBoxParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_passwordTextBox = new PasswordTextbox(m_usrNameTextBox->x(),
                                            m_usrNameTextBox->y() + m_usrNameTextBox->height(),
                                            HALF_TILE_WIDTH ,
                                            BGTILE_HEIGHT,
                                            AUTO_CONFIG_PASSWD,
                                            TEXTBOX_LARGE,
                                            this,
                                            m_passwordTextBoxParam,
                                            COMMON_LAYER, true, SCALE_WIDTH(70));

    m_elementlist[AUTO_CONFIG_PASSWD] = m_passwordTextBox;
    connect(m_passwordTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_profRetainCheckbox = new OptionSelectButton(m_endIpAddress->x(),
                                                  m_endIpAddress->y() + m_endIpAddress->height() + SCALE_HEIGHT(40),
                                                  TILE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this, COMMON_LAYER,
                                                  autoConfigureStr[4],
                                                  " ", SCALE_WIDTH(10), AUTO_CONFIG_PROF_RETAIN_FLAG);
    m_elementlist[AUTO_CONFIG_PROF_RETAIN_FLAG] = m_profRetainCheckbox;
    connect(m_profRetainCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_profRetainCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_mainStreamElementHeading = new ElementHeading(m_profRetainCheckbox->x(),
                                                   (m_profRetainCheckbox->y() + m_profRetainCheckbox->height() + SCALE_HEIGHT(10)),
                                                    HALF_TILE_WIDTH,
                                                    BGTILE_HEIGHT,
                                                    "Main Stream",
                                                    TOP_LAYER,
                                                    this,
                                                    false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    tempMap.clear();
    for(quint8 index = 0; index < encodinOptions.length(); index++)
    {
        tempMap.insert(index, encodinOptions.at(index));
        if(encodinOptions.at(index) == m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING))
        {
            selectedKey = index;
        }
    }

    m_mainVideoEncodingPicklist = new PickList(m_mainStreamElementHeading->x(),
                                              (m_mainStreamElementHeading->y() + m_mainStreamElementHeading->height()),
                                               HALF_TILE_WIDTH,
                                               BGTILE_HEIGHT,
                                               SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                               autoConfigureStr[5],
                                               tempMap,
                                               selectedKey, "Select Video Encoding",
                                               this,
                                               MIDDLE_TABLE_LAYER,
                                               -1,
                                               AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST,
                                               true, true);
    m_elementlist[AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST] = m_mainVideoEncodingPicklist;
    connect(m_mainVideoEncodingPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotValueChanged(quint8,QString,int)));
    connect(m_mainVideoEncodingPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    tempMap.clear();
    for(quint8 index = 0; index < 60; index++)
    {
        tempMap.insert(index, QString("%1").arg(index + 1));
        if((index + 1) == m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_FRAMERATE).toInt())
        {
            selectedKey = index;
        }
    }

    m_mainFrameRatePicklist = new PickList(m_mainVideoEncodingPicklist->x(),
                                          (m_mainVideoEncodingPicklist->y() + m_mainVideoEncodingPicklist->height()),
                                           HALF_TILE_WIDTH,
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                           autoConfigureStr[7],
                                           tempMap,
                                           selectedKey, "Select Frame Rate",
                                           this,
                                           MIDDLE_TABLE_LAYER,
                                           -1,
                                           AUTO_CONFIG_MAIN_FRAMERATE_PICKLIST,
                                           true, true);
    m_elementlist[AUTO_CONFIG_MAIN_FRAMERATE_PICKLIST] = m_mainFrameRatePicklist;
    connect(m_mainFrameRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    tempMap.clear();
    for(quint8 index = 0; index < resolutionOptions.length(); index++)
    {
        tempMap.insert(index, resolutionOptions.at(index));
        if(resolutionOptions.at(index) == m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_RESOLUTION))
        {
            selectedKey = index;
        }
    }

    m_mainResolutionPicklist = new PickList(m_mainFrameRatePicklist->x(),
                                           (m_mainFrameRatePicklist->y() + m_mainFrameRatePicklist->height()),
                                            HALF_TILE_WIDTH,
                                            BGTILE_HEIGHT,
                                            SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                            autoConfigureStr[6],
                                            tempMap,
                                            selectedKey, "Select Resolution",
                                            this,
                                            MIDDLE_TABLE_LAYER,
                                            -1,
                                            AUTO_CONFIG_MAIN_RESOLUTION_PICKLIST,
                                            true, true);
    m_elementlist[AUTO_CONFIG_MAIN_RESOLUTION_PICKLIST] = m_mainResolutionPicklist;
    connect(m_mainResolutionPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_mainCBRTypeRadioButton = new OptionSelectButton(m_mainResolutionPicklist->x(),
                                                     (m_mainResolutionPicklist->y() + m_mainResolutionPicklist->height()),
                                                      HALF_TILE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      RADIO_BUTTON_INDEX,
                                                      this,
                                                      MIDDLE_TABLE_LAYER,
                                                      "Bit Rate Type",
                                                      autoConfigureStr[8],
                                                      -1,
                                                      AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON,
                                                      true,
                                                      NORMAL_FONT_SIZE,
                                                      NORMAL_FONT_COLOR);
    m_elementlist[AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON] = m_mainCBRTypeRadioButton;
    connect(m_mainCBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
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
                                                      AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON);
    m_elementlist[AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON] = m_mainVBRTypeRadioButton;
    connect(m_mainVBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainVBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    tempMap.clear();
    for(quint8 index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        tempMap.insert(index, autoConfigBitRateArray[index]);
        if( index == m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE).toInt())
        {
            selectedKey = index;
        }
    }

    m_mainBitRatePicklist = new PickList(m_mainCBRTypeRadioButton->x(),
                                        (m_mainCBRTypeRadioButton->y() + m_mainCBRTypeRadioButton->height()),
                                         HALF_TILE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         autoConfigureStr[10],
                                         tempMap,
                                         selectedKey, "Select Bit Rate",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         AUTO_CONFIG_MAIN_BITRATE_PICKLIST,
                                         true, true);
    m_elementlist[AUTO_CONFIG_MAIN_BITRATE_PICKLIST] = m_mainBitRatePicklist;
    connect(m_mainBitRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    tempMap.clear();
    for(quint8 index = 0; index < 10; index++)
    {
        tempMap.insert(index, QString("%1").arg(index + 1));
        if((index + 1) == m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_QUALITY).toInt())
        {
            selectedKey = index;
        }
    }

    m_mainQualityPicklist = new PickList(m_mainBitRatePicklist->x(),
                                        (m_mainBitRatePicklist->y() + m_mainBitRatePicklist->height()),
                                         HALF_TILE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         autoConfigureStr[11],
                                         tempMap,
                                         selectedKey, "Select Quality",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         AUTO_CONFIG_MAIN_QUALITY_PICKLIST,
                                         true, true);
    m_elementlist[AUTO_CONFIG_MAIN_QUALITY_PICKLIST] = m_mainQualityPicklist;
    connect(m_mainQualityPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_mainGOPParam = new TextboxParam();
    m_mainGOPParam->maxChar = 3;
    m_mainGOPParam->validation = QRegExp("[0-9]");
    m_mainGOPParam->minNumValue = 1;
    m_mainGOPParam->maxNumValue = 100;
    m_mainGOPParam->isNumEntry = true;
    m_mainGOPParam->labelStr = autoConfigureStr[12];
    m_mainGOPParam->suffixStr = "(1-100)";

    m_mainGOPTextbox = new TextBox(m_mainQualityPicklist->x(),
                                  (m_mainQualityPicklist->y() + m_mainQualityPicklist->height()),
                                   HALF_TILE_WIDTH,
                                   BGTILE_HEIGHT,
                                   AUTO_CONFIG_MAIN_GOP_TEXTBOX,
                                   TEXTBOX_EXTRASMALL,
                                   this,
                                   m_mainGOPParam,
                                   MIDDLE_TABLE_LAYER,
                                   true);
    m_elementlist[AUTO_CONFIG_MAIN_GOP_TEXTBOX] = m_mainGOPTextbox;
    connect(m_mainGOPTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainGOPTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_mainAudioCheckbox = new OptionSelectButton(m_mainGOPTextbox->x(),
                                                (m_mainGOPTextbox->y() + m_mainGOPTextbox->height()),
                                                 HALF_TILE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 BOTTOM_TABLE_LAYER,
                                                 autoConfigureStr[13],
                                                 "" , -1,
                                                 AUTO_CONFIG_MAIN_AUDIO_CHECKBOX);
    m_elementlist[AUTO_CONFIG_MAIN_AUDIO_CHECKBOX] = m_mainAudioCheckbox;
    connect(m_mainAudioCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainAudioCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    // Sub elements
    m_subStreamElementHeading = new ElementHeading(m_mainStreamElementHeading->x() + m_mainStreamElementHeading->width() + INTER_CONTROL_MARGIN,
                                                   m_mainStreamElementHeading->y(),
                                                   HALF_TILE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   "Sub Stream",
                                                   TOP_LAYER,
                                                   this,
                                                   false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    tempMap.clear();
    for(quint8 index = 0; index < encodinOptions.length(); index++)
    {
        tempMap.insert(index, encodinOptions.at(index));
        if(encodinOptions.at(index) == m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_VIDEOENCODING))
        {
            selectedKey = index;
        }
    }

    m_subVideoEncodingPicklist = new PickList(m_subStreamElementHeading->x(),
                                             (m_subStreamElementHeading->y() + m_subStreamElementHeading->height()),
                                              HALF_TILE_WIDTH,
                                              BGTILE_HEIGHT,
                                              SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                              autoConfigureStr[5],
                                              tempMap,
                                              selectedKey, "Select Video Encoding",
                                              this,
                                              MIDDLE_TABLE_LAYER,
                                              -1,
                                              AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST,
                                              true, true);
    m_elementlist[AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST] = m_subVideoEncodingPicklist;
    connect(m_subVideoEncodingPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotValueChanged(quint8,QString,int)));
    connect(m_subVideoEncodingPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    tempMap.clear();
    for(quint8 index = 0; index < 60; index++)
    {
        tempMap.insert(index, QString("%1").arg(index + 1));
        if((index + 1) == m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_FRAMERATE).toInt())
        {
            selectedKey = index;
        }
    }

    m_subFrameRatePicklist = new PickList(m_subVideoEncodingPicklist->x(),
                                         (m_subVideoEncodingPicklist->y() + m_subVideoEncodingPicklist->height()),
                                          HALF_TILE_WIDTH,
                                          BGTILE_HEIGHT,
                                          SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                          autoConfigureStr[7],
                                          tempMap,
                                          selectedKey, "Select Frame Rate",
                                          this,
                                          MIDDLE_TABLE_LAYER,
                                          -1,
                                          AUTO_CONFIG_SUB_FRAMERATE_PICKLIST,
                                          true, true);
    m_elementlist[AUTO_CONFIG_SUB_FRAMERATE_PICKLIST] = m_subFrameRatePicklist;
    connect(m_subFrameRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    tempMap.clear();
    for(quint8 index = 0; index < resolutionOptions.length(); index++)
    {
        tempMap.insert(index, resolutionOptions.at(index));
        if(resolutionOptions.at(index) == m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_RESOLUTION))
        {
            selectedKey = index;
        }
    }

    m_subResolutionPicklist = new PickList(m_subFrameRatePicklist->x(),
                                          (m_subFrameRatePicklist->y() + m_subFrameRatePicklist->height()),
                                           HALF_TILE_WIDTH,
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                           autoConfigureStr[6],
                                           tempMap,
                                           selectedKey, "Select Resolution",
                                           this,
                                           MIDDLE_TABLE_LAYER,
                                           -1,
                                           AUTO_CONFIG_SUB_RESOLUTION_PICKLIST,
                                           true, true);
    m_elementlist[AUTO_CONFIG_SUB_RESOLUTION_PICKLIST] = m_subResolutionPicklist;
    connect(m_subResolutionPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_subCBRTypeRadioButton = new OptionSelectButton(m_subResolutionPicklist->x(),
                                                    (m_subResolutionPicklist->y() + m_subResolutionPicklist->height()),
                                                     HALF_TILE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     RADIO_BUTTON_INDEX,
                                                     this,
                                                     MIDDLE_TABLE_LAYER,
                                                     "Bit Rate Type",
                                                     autoConfigureStr[8],
                                                     -1,
                                                     AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON,
                                                     true,
                                                     NORMAL_FONT_SIZE,
                                                     NORMAL_FONT_COLOR);
    m_elementlist[AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON] = m_subCBRTypeRadioButton;
    connect(m_subCBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
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
                                                     AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON);
    m_elementlist[AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON] = m_subVBRTypeRadioButton;
    connect(m_subVBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subVBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    tempMap.clear();
    for(quint8 index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        tempMap.insert(index, autoConfigBitRateArray[index]);
        if( index == m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE).toInt())
        {
            selectedKey = index;
        }
    }

    m_subBitRatePicklist = new PickList(m_subCBRTypeRadioButton->x(),
                                       (m_subCBRTypeRadioButton->y() + m_subCBRTypeRadioButton->height()),
                                        HALF_TILE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        autoConfigureStr[10],
                                        tempMap,
                                        selectedKey, "Select Bit Rate",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        AUTO_CONFIG_SUB_BITRATE_PICKLIST,
                                        true, true);
    m_elementlist[AUTO_CONFIG_SUB_BITRATE_PICKLIST] = m_subBitRatePicklist;
    connect(m_subBitRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    tempMap.clear();
    for(quint8 index = 0; index < 10; index++)
    {
        tempMap.insert(index, QString("%1").arg(index + 1));
        if((index + 1) == m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_QUALITY).toInt())
        {
            selectedKey = index;
        }
    }

    m_subQualityPicklist = new PickList(m_subBitRatePicklist->x(),
                                       (m_subBitRatePicklist->y() + m_subBitRatePicklist->height()),
                                        HALF_TILE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        autoConfigureStr[11],
                                        tempMap,
                                        selectedKey, "Select Quality",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        AUTO_CONFIG_SUB_QUALITY_PICKLIST,
                                        true, true);
    m_elementlist[AUTO_CONFIG_SUB_QUALITY_PICKLIST] = m_subQualityPicklist;
    connect(m_subQualityPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_subGOPParam = new TextboxParam();
    m_subGOPParam->maxChar = 3;
    m_subGOPParam->validation = QRegExp("[0-9]");
    m_subGOPParam->minNumValue = 1;
    m_subGOPParam->maxNumValue = 100;
    m_subGOPParam->isNumEntry = true;
    m_subGOPParam->labelStr = autoConfigureStr[12];
    m_subGOPParam->suffixStr = "(1-100)";

    m_subGOPTextbox = new TextBox(m_subQualityPicklist->x(),
                                 (m_subQualityPicklist->y() + m_subQualityPicklist->height()),
                                  HALF_TILE_WIDTH,
                                  BGTILE_HEIGHT,
                                  AUTO_CONFIG_SUB_GOP_TEXTBOX,
                                  TEXTBOX_EXTRASMALL,
                                  this,
                                  m_subGOPParam,
                                  MIDDLE_TABLE_LAYER,
                                  true);
    m_elementlist[AUTO_CONFIG_SUB_GOP_TEXTBOX] = m_subGOPTextbox;
    connect(m_subGOPTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subGOPTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_subAudioCheckbox = new OptionSelectButton(m_subGOPTextbox->x(),
                                               (m_subGOPTextbox->y() + m_subGOPTextbox->height()),
                                                HALF_TILE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                BOTTOM_TABLE_LAYER,
                                                autoConfigureStr[13],
                                                "" , -1,
                                                AUTO_CONFIG_SUB_AUDIO_CHECKBOX);
    m_elementlist[AUTO_CONFIG_SUB_AUDIO_CHECKBOX] = m_subAudioCheckbox;
    connect(m_subAudioCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subAudioCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptSelButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_saveBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               m_mainAudioCheckbox->x() + m_mainAudioCheckbox->width() - SCALE_WIDTH(60),
                               m_mainAudioCheckbox->y() + m_mainAudioCheckbox->height() + SCALE_HEIGHT(40),
                               "Save",
                               this,
                               AUTO_CONFIG_SAVE_BUTTON);
    m_elementlist[AUTO_CONFIG_SAVE_BUTTON] = m_saveBtn;
    connect(m_saveBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClick(int)));

    connect(m_saveBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_okBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                             m_mainAudioCheckbox->x() + m_mainAudioCheckbox->width() - SCALE_WIDTH(60),
                             m_mainAudioCheckbox->y() + m_mainAudioCheckbox->height() + SCALE_HEIGHT(40),
                             "OK",
                             this,
                             AUTO_CONFIG_OK_BUTTON);
    m_elementlist[AUTO_CONFIG_OK_BUTTON] = m_okBtn;
    connect(m_okBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClick(int)));

    connect(m_okBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    if(m_isEnable)
    {
        m_okBtn->setVisible(false);
        m_okBtn->setIsEnabled(false);
    }
    else
    {
        m_saveBtn->setVisible(false);
        m_saveBtn->setIsEnabled(false);
    }

    m_cancleBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 m_mainAudioCheckbox->x() + m_mainAudioCheckbox->width() + SCALE_WIDTH(70),
                                 m_mainAudioCheckbox->y() + m_mainAudioCheckbox->height() + SCALE_HEIGHT(40),
                                 "Cancel",
                                 this,
                                 AUTO_CONFIG_CANCEL_BUTTON);
    m_elementlist[AUTO_CONFIG_CANCEL_BUTTON] = m_cancleBtn;
    connect(m_cancleBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClick(int)));

    connect(m_cancleBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_infoPage = new InfoPage(0, 0,
                              SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                              SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                              INFO_CONFIG_PAGE,
                              parentWidget(), true);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageBtnclick(int)));
}

void AutoConfigureCamera::fillRecords()
{
    m_startIpAddress->setIpaddress(m_valueStringList->at(FIELD_AUTO_CONFIG_START_IP));
    m_endIpAddress->setIpaddress(m_valueStringList->at(FIELD_AUTO_CONFIG_END_IP));
	
    m_usrNameTextBox->setInputText( m_valueStringList->at(AUTO_CONFIG_USER_NAME));
    m_passwordTextBox->setInputText(m_valueStringList->at(AUTO_CONFIG_PASSWORD));

    if(m_valueStringList->at(FIELD_AUTO_CONFIG_IP_RETAIN).toInt())
    {
        m_ipRetainCheckBox->changeState(ON_STATE);
        m_startIpAddress->setIsEnabled(false);
        m_endIpAddress->setIsEnabled(false);
    }
    else
    {
        m_ipRetainCheckBox->changeState(OFF_STATE);
        m_startIpAddress->setIsEnabled(true);
        m_endIpAddress->setIsEnabled(true);
    }

    fillstreamRelatedParam();
    if(m_valueStringList->at(FIELD_AUTO_CONFIG_PROF_RETAIN).toInt())
    {
        m_profRetainCheckbox->changeState(ON_STATE);
        enableStreamRelatedElements(false);
    }
    else
    {
        m_profRetainCheckbox->changeState(OFF_STATE);
        enableStreamRelatedElements(true);
        upadateEnableStateForElements();
    }
}

void AutoConfigureCamera::enableStreamRelatedElements(bool isEnable)
{
    m_mainVideoEncodingPicklist->setIsEnabled(isEnable);
    m_subVideoEncodingPicklist->setIsEnabled(isEnable);

    m_mainResolutionPicklist->setIsEnabled(isEnable);
    m_subResolutionPicklist->setIsEnabled(isEnable);

    m_mainFrameRatePicklist->setIsEnabled(isEnable);
    m_subFrameRatePicklist->setIsEnabled(isEnable);

    m_mainCBRTypeRadioButton->setIsEnabled(isEnable);
    m_mainVBRTypeRadioButton->setIsEnabled(isEnable);

    m_subCBRTypeRadioButton->setIsEnabled(isEnable);
    m_subVBRTypeRadioButton->setIsEnabled(isEnable);

    m_mainBitRatePicklist->setIsEnabled(isEnable);
    m_subBitRatePicklist->setIsEnabled(isEnable);

    m_mainQualityPicklist->setIsEnabled(isEnable);
    m_subQualityPicklist->setIsEnabled(isEnable);

    m_mainGOPTextbox->setIsEnabled(isEnable);
    m_subGOPTextbox->setIsEnabled(isEnable);

    m_mainAudioCheckbox->setIsEnabled(isEnable);
    m_subAudioCheckbox->setIsEnabled(isEnable);
}

void AutoConfigureCamera::fillstreamRelatedParam()
{
    m_mainVideoEncodingPicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING));
    m_subVideoEncodingPicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_VIDEOENCODING));

    m_mainResolutionPicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_RESOLUTION));
    m_subResolutionPicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_RESOLUTION));

    m_mainFrameRatePicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_FRAMERATE));
    m_subFrameRatePicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_FRAMERATE));

    if(m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_BITERATE_TYPE).toInt())
    {
        m_mainVBRTypeRadioButton->changeState(OFF_STATE);
        m_mainCBRTypeRadioButton->changeState(ON_STATE);
    }
    else
    {
        m_mainVBRTypeRadioButton->changeState(ON_STATE);
        m_mainCBRTypeRadioButton->changeState(OFF_STATE);
    }

    if(m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_BITRATE_TYPE).toInt())
    {
        m_subVBRTypeRadioButton->changeState(OFF_STATE);
        m_subCBRTypeRadioButton->changeState(ON_STATE);
    }
    else
    {
        m_subVBRTypeRadioButton->changeState(ON_STATE);
        m_subCBRTypeRadioButton->changeState(OFF_STATE);
    }

    m_mainBitRatePicklist->setValue(autoConfigBitRateArray[m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE).toInt()]);
    m_subBitRatePicklist->setValue(autoConfigBitRateArray[m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE).toInt()]);

    m_mainQualityPicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_QUALITY));
    m_subQualityPicklist->setValue(m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_QUALITY));

    m_mainGOPTextbox->setInputText(m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_GOP));
    m_subGOPTextbox->setInputText(m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_GOP));

    m_mainAudioCheckbox->changeState((OPTION_STATE_TYPE_e)m_valueStringList->at(FIELD_AUTO_CONFIG_MAIN_AUDIO).toInt());
    m_subAudioCheckbox->changeState((OPTION_STATE_TYPE_e)m_valueStringList->at(FIELD_AUTO_CONFIG_SUB_AUDIO).toInt());
}

void AutoConfigureCamera::setRecords()
{
    m_valueStringList->replace(FIELD_AUTO_CONFIG_IP_RETAIN, QString("%1").arg((int)m_ipRetainCheckBox->getCurrentState()));
    m_valueStringList->replace(FIELD_AUTO_CONFIG_PROF_RETAIN, QString("%1").arg((int)m_profRetainCheckbox->getCurrentState()));
    m_valueStringList->replace(FIELD_AUTO_CONFIG_START_IP, m_startIpAddress->getIpaddress());
    m_valueStringList->replace(FIELD_AUTO_CONFIG_END_IP, m_endIpAddress->getIpaddress());

    m_valueStringList->replace(AUTO_CONFIG_USER_NAME, m_usrNameTextBox->getInputText());
    m_valueStringList->replace(AUTO_CONFIG_PASSWORD, m_passwordTextBox->getInputText());

    if(m_profRetainCheckbox->getCurrentState() == OFF_STATE)
    {
        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING, m_mainVideoEncodingPicklist->getCurrentPickStr());
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_VIDEOENCODING, m_subVideoEncodingPicklist->getCurrentPickStr());

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_RESOLUTION, m_mainResolutionPicklist->getCurrentPickStr());
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_RESOLUTION, m_subResolutionPicklist->getCurrentPickStr());

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_FRAMERATE, m_mainFrameRatePicklist->getCurrentPickStr());
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_FRAMERATE, m_subFrameRatePicklist->getCurrentPickStr());

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_BITERATE_TYPE, QString("%1").arg((int)m_mainCBRTypeRadioButton->getCurrentState()));
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_BITRATE_TYPE, QString("%1").arg((int)m_subCBRTypeRadioButton->getCurrentState()));

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE, QString("%1").arg(findIndexofBitrateValue(m_mainBitRatePicklist->getCurrentPickStr())));
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE, QString("%1").arg(findIndexofBitrateValue(m_subBitRatePicklist->getCurrentPickStr())));

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_QUALITY, m_mainQualityPicklist->getCurrentPickStr());
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_QUALITY, m_subQualityPicklist->getCurrentPickStr());

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_GOP, m_mainGOPTextbox->getInputText());
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_GOP, m_subGOPTextbox->getInputText());

        m_valueStringList->replace(FIELD_AUTO_CONFIG_MAIN_AUDIO, QString("%1").arg((int)m_mainAudioCheckbox->getCurrentState()));
        m_valueStringList->replace(FIELD_AUTO_CONFIG_SUB_AUDIO, QString("%1").arg((int)m_subAudioCheckbox->getCurrentState()));
    }

    for(quint8 index = FIELD_AUTO_CONFIG_IP_RETAIN; index < AUTO_CNFG_FIELD; index++)
    {
        m_payloadLib->setCnfgArrayAtIndex(index, m_valueStringList->at(index));
    }

    for(quint8 index = AUTO_CNFG_FIELD; index < MAX_FIELD_AUTO_CONFIG; index++)
    {
        m_payloadLib->setCnfgArrayAtIndex(index, m_valueStringList->at(index));
    }

    QString payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                               GENERAL_TABLE_INDEX,
                                                               CNFG_FRM_INDEX,
                                                               CNFG_TO_INDEX,
                                                               FIELD_AUTO_CONFIG+2,         // start with Auto Retain IP flag
                                                               FIELD_INT_WITH_SAMAS_FLAG,   // sub stream Audio.
                                                               AUTO_CNFG_FIELD);

     payloadString = m_payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                        GENERAL_TABLE_INDEX,
                                                        CNFG_FRM_INDEX,
                                                        CNFG_TO_INDEX,
                                                        FIELD_AUTO_CONFIG_USERNAME+1,       // start with UserName
                                                        FIELD_AUTO_CONFIG_PASSWORD+1,       // password.
                                                        2,
                                                        payloadString,AUTO_CNFG_FIELD);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;
    m_applController->processActivity(m_currDeviceName, DEVICE_COMM, param);
}

bool AutoConfigureCamera::validationOnOkButton()
{
    QString startIp = m_startIpAddress->getIpaddress();
    QString endIp = m_endIpAddress->getIpaddress();
    QString startIpNetworkAddr;
    QString endIpNetworkAddr;
    qint8 startIpSeparatorIndex;
    qint8 endIpSeparatorIndex;

    if (m_ipRetainCheckBox->getCurrentState() == OFF_STATE)
    {
        if ((startIp == "") || (endIp == ""))
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_IP_ADDR_RANGE));
            return false;
        }

        if (QHostAddress(startIp).protocol() != QHostAddress(endIp).protocol())
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_IP_TYPE_DIFF));
            return false;
        }

        if (startIp == endIp)
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_IP_STR_END_DIFF));
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
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_IP_STR_END_SAME));
            return false;
        }

        if(*m_startIpAddress > *m_endIpAddress)
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(START_IP_ADDR_LESS_END_IP_ADDR));
            return false;
        }
    }

    if (m_usrNameTextBoxParam->textStr == "")
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_USER_NAME_ERROR));
        return false;
    }

    if (m_passwordTextBoxParam->textStr == "")
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(LOGIN_PASSWORD_ERROR));
        return false;
    }

    return true;
}

quint8 AutoConfigureCamera::findIndexofBitrateValue(QString bitRateStr)
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

void AutoConfigureCamera::upadateEnableStateForElements()
{
    bool isMainStreamMjpeg = (m_mainVideoEncodingPicklist->getCurrentPickStr() == QString(MJPEG_STRING));
    bool isSubStreamMjpeg = (m_subVideoEncodingPicklist->getCurrentPickStr() == QString(MJPEG_STRING));

    /* Main Stream */
    if(true == isMainStreamMjpeg)
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
    if(true == isSubStreamMjpeg)
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

void AutoConfigureCamera::navigationKeyPressed(QKeyEvent *event)
{
   event->accept();
}

void AutoConfigureCamera::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = AUTO_CONFIG_CLOSE_BUTTON;
    m_elementlist[m_currElement]->forceActiveFocus();
}

void AutoConfigureCamera::takeLeftKeyAction()
{
    bool status = true;
    do 
    {
        if(m_currElement == 0)
        {
            m_currElement =(MAX_AUTO_CONFIG_ELEMENT);
        }

        if(m_currElement)
        {
            m_currElement =(m_currElement - 1);
        }
        else
        {
              status = false;
              break;
        }

    }while((m_elementlist[m_currElement] == NULL) || (!m_elementlist[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementlist[m_currElement]->forceActiveFocus();
    }
}

void AutoConfigureCamera::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(m_currElement ==(MAX_AUTO_CONFIG_ELEMENT - 1))
        {
            m_currElement = -1;
        }

        if(m_currElement !=(MAX_AUTO_CONFIG_ELEMENT - 1))
        {
            m_currElement =(m_currElement + 1);
        }
        else
        {
              status = false;
              break;
        }

    }while((m_elementlist[m_currElement] == NULL) || (!m_elementlist[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementlist[m_currElement]->forceActiveFocus();
    }
}

void AutoConfigureCamera::slotCloseButtonClick(int index)
{
    switch(index)
    {
        case AUTO_CONFIG_SAVE_BUTTON:
        case AUTO_CONFIG_OK_BUTTON:
            if(validationOnOkButton() == true)
            {
                if ((m_ipRetainCheckBox->getCurrentState() == OFF_STATE) &&
                        ((m_startIpAddress->getIpaddress() != startIpAddress) || (m_endIpAddress->getIpaddress() != endIpAddress)))
                {
                    m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_NEW_CAM), true);
                }
                else
                {
                    setRecords();
                }
            }
            break;

        case AUTO_CONFIG_CLOSE_BUTTON:
        case AUTO_CONFIG_CANCEL_BUTTON:
            emit sigObjectDelete();
            break;

        default:
            break;
    }
}

void AutoConfigureCamera::slotOptSelButtonClicked(OPTION_STATE_TYPE_e state, int index)
{
    switch(index)
    {
        case AUTO_CONFIG_IP_RETAIN_FLAG:
            if(state == ON_STATE)
            {
                m_startIpAddress->setIsEnabled(false);
                m_endIpAddress->setIsEnabled(false);
            }
            else
            {
                m_startIpAddress->setIsEnabled(true);
                m_endIpAddress->setIsEnabled(true);
            }
            break;

        case AUTO_CONFIG_PROF_RETAIN_FLAG:
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

        case AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON:
            m_mainVBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        case AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON:
            m_mainCBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        case AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON:
            m_subVBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        case AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON:
            m_subCBRTypeRadioButton->changeState(OFF_STATE);
            upadateEnableStateForElements();
            break;

        default:
            break;
    }
}

void AutoConfigureCamera::slotIpAddressEntryDone(quint32)
{
    QString startIp = m_startIpAddress->getIpaddress();
    QString endIp = m_endIpAddress->getIpaddress();

    if(QHostAddress(startIp).protocol() == QAbstractSocket::IPv4Protocol)
    {
        QStringList startIpOctes = startIp.split('.');
        QStringList endIpOctes = endIp.split('.');
        if ((startIpOctes.size() == 4) && (endIpOctes.size() == 4))
        {
            endIpOctes[0] = startIpOctes[0];
            endIpOctes[1] = startIpOctes[1];
            endIpOctes[2] = startIpOctes[2];

            m_endIpAddress->setIpaddress(endIpOctes.join('.'));
        }
        else
        {
            startIpOctes[3] = "255";
            m_endIpAddress->setIpaddress(startIpOctes.join('.'));
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
            m_endIpAddress->setIpaddress(startIpInitialGroups + ":" + endIpLastGroup);
        }
        else
        {
            m_endIpAddress->setIpaddress(startIpInitialGroups + ":ffff");
        }
    }
}

void AutoConfigureCamera::slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e infoMsg)
{
    if(infoMsg == INFO_MSG_ERROR)
    {
        if(index == AUTO_CONFIG_MAIN_GOP_TEXTBOX)
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_MAIN_GOP));
        }
        else if(index == AUTO_CONFIG_SUB_GOP_TEXTBOX)
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_SUB_GOP));
        }
    }
}

void AutoConfigureCamera::slotInfoPageBtnclick(int index)
{
    if ((index == INFO_OK_BTN) && (m_infoPage->getText() == ValidationMessage::getValidationMessage(AUTO_CONF_NEW_CAM)))
    {
        startIpAddress = m_startIpAddress->getIpaddress();
        endIpAddress = m_endIpAddress->getIpaddress();
        setRecords();
    }

    m_elementlist[m_currElement]->forceActiveFocus();
}

void AutoConfigureCamera::slotIpAddressLoadInfoPage(quint32)
{
    m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_VALID_IP_ADD));
    m_startIpAddress->setIpaddress(m_valueStringList->at(FIELD_AUTO_CONFIG_START_IP));
    m_endIpAddress->setIpaddress(m_valueStringList->at(FIELD_AUTO_CONFIG_END_IP));
}

void AutoConfigureCamera::slotValueChanged(quint8, QString, int index)
{
    switch(index)
    {
        case AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST:
        case AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST:
            upadateEnableStateForElements();
            break;

        default:
            break;
    }
}

void AutoConfigureCamera::slotUpdateCurrentElement(int index)
{
    m_currElement = index;
}

void AutoConfigureCamera::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void AutoConfigureCamera::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void AutoConfigureCamera::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void AutoConfigureCamera::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
    slotCloseButtonClick(AUTO_CONFIG_SAVE_BUTTON);
}
