#include "IpStreamSettings.h"
#include "ValidationMessage.h"
#include "CameraSettings.h"

#define STREAM_SETTINGS_LEFT_MARGIN     10
#define STREAM_SETTINGS_TOP_MARGIN      5
#define STREAM_SETTINGS_MIN_TOP_MARGIN  10
#define INTER_CONTROL_MARGIN            10

#define TILE_WIDTH                      (PAGE_RIGHT_PANEL_WIDTH - (2 * STREAM_SETTINGS_LEFT_MARGIN))
#define TILE_HEIGHT                     32
#define HALF_TILE_WIDTH                 (TILE_WIDTH - INTER_CONTROL_MARGIN) / 2

#define TOTAL_FIELD_STREAM_TABLE        (SUB_STREAM_COPY_TO_CAM_VALUE_END + 1)

#define MAX_BITRATE_INDEX               16
#define CODEC_STRING                    "Motion JPEG"

typedef enum
{
    MAIN_VIDEO_ENCODING_VALUE,
    MAIN_RESOLUTION_VALUE,
    MAIN_FRAME_RATE_VALUE,
    MAIN_QUALITY_VALUE,
    MAIN_AUDIO_STATUS_VALUE,
    MAIN_BIT_RATE_TYPE_VALUE,
    MAIN_BIT_RATE_VALUE,
    MAIN_GOP_VALUE,

    SUB_VIDEO_ENCODING_VALUE,
    SUB_RESOLUTION_VALUE,
    SUB_FRAME_RATE_VALUE,
    SUB_QUALITY_VALUE,
    SUB_AUDIO_STATUS_VALUE,
    SUB_BIT_RATE_TYPE_VALUE,
    SUB_BIT_RATE_VALUE,
    SUB_GOP_VALUE,

    MAIN_STREAM_PROFILE_VALUE,
    SUB_STREAM_PROFILE_VALUE,
    MAIN_STREAM_COPY_TO_CAM_VALUE_START,
    MAIN_STREAM_COPY_TO_CAM_VALUE_END = MAIN_STREAM_COPY_TO_CAM_VALUE_START + CAMERA_MASK_MAX - 1,
    SUB_STREAM_COPY_TO_CAM_VALUE_START,
    SUB_STREAM_COPY_TO_CAM_VALUE_END = SUB_STREAM_COPY_TO_CAM_VALUE_START + CAMERA_MASK_MAX - 1

}STREAM_SETTINGS_RECORD_e;

typedef enum
{
    STREAM_PROFILE_CAM_NUMBER,
    STREAM_PROFILE_PROFILE_NUMBER,
    STREAM_PROFILE_STREAM_TYPE,
    STREAM_PROFILE_VIDEO_ENCODING,
    STREAM_PROFILE_RESOLUTION,
    STREAM_PROFILE_FRAME_RATE,
    STREAM_PROFILE_BIT_TYPE,
    STREAM_PROFILE_BIT_RATE,
    STREAM_PROFILE_QUALITY,
    STREAM_PROFILE_GOP,
    STREAM_PROFILE_AUDIO

}STREAM_PROFILE_PARAM_e;

const QString labelString[MAX_IP_STREAM_SETTINGS_ELEMENT] = {"Camera",
                                                             "Copy to Camera",
                                                             "Profile",
                                                             "Video Encoding",
                                                             "Resolution",
                                                             "Frame Rate",
                                                             "CBR",
                                                             "VBR",
                                                             "Bit Rate",
                                                             "Quality",
                                                             "GOP",
                                                             "Audio",
                                                             "Copy to Camera",
                                                             "Profile",
                                                             "Video Encoding",
                                                             "Resolution",
                                                             "Frame Rate",
                                                             "CBR",
                                                             "VBR",
                                                             "Bit Rate",
                                                             "Quality",
                                                             "GOP",
                                                             "Audio"};

const QString bitRateArray[MAX_BITRATE_INDEX] = {"32 kbps", "64 kbps",
                                                 "128 kbps", "256 kbps",
                                                 "384 kbps", "512 kbps",
                                                 "768 kbps", "1024 kbps",
                                                 "1536 kbps", "2048 kbps",
                                                 "3072 kbps", "4096 kbps",
                                                 "6144 kbps", "8192 kbps",
                                                 "12288 kbps","16384 kbps"};

IpStreamSettings::IpStreamSettings(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent, MAX_IP_STREAM_SETTINGS_ELEMENT, devTabInfo), isOnvifSupport(0), media2Support(false)
{
    createDefaultElements();
    IpStreamSettings::getConfig();
}

IpStreamSettings::~IpStreamSettings()
{
    disconnect(m_cameraNameDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_cameraNameDropDownBox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinboxValueChanged(QString,quint32)));
    delete m_cameraNameDropDownBox;

    delete m_mainStreamElementHeading;
    disconnect(m_mainProfilePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainProfilePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_mainProfilePicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPickListValueChanged(quint8,QString,int)));
    delete m_mainProfilePicklist;

    disconnect(m_mainVideoEncodingPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainVideoEncodingPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_mainVideoEncodingPicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPickListValueChanged(quint8,QString,int)));
    delete m_mainVideoEncodingPicklist;

    disconnect(m_mainResolutionPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainResolutionPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_mainResolutionPicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPickListValueChanged(quint8,QString,int)));
    delete m_mainResolutionPicklist;

    disconnect(m_mainFrameRatePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainFrameRatePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    delete m_mainFrameRatePicklist;

    disconnect(m_mainCBRTypeRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainCBRTypeRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete m_mainCBRTypeRadioButton;

    disconnect(m_mainVBRTypeRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainVBRTypeRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete m_mainVBRTypeRadioButton;

    disconnect(m_mainBitRatePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_mainBitRatePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_mainBitRatePicklist;

    disconnect(m_mainQualityPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainQualityPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    delete m_mainQualityPicklist;

    disconnect(m_mainGOPTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_mainGOPTextbox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    delete m_mainGOPTextbox;
    delete m_mainGOPParam;

    disconnect(m_mainAudioCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_mainAudioCheckbox;

    if(IS_VALID_OBJ(m_mainCopyToCamButton))
    {
        disconnect(m_mainCopyToCamButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        disconnect (m_mainCopyToCamButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotCopyToCamButtonClick(int)));
        DELETE_OBJ(m_mainCopyToCamButton);
    }

    delete m_subStreamElementHeading;
    disconnect(m_subProfilePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subProfilePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_subProfilePicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPickListValueChanged(quint8,QString,int)));
    delete m_subProfilePicklist;

    disconnect(m_subVideoEncodingPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subVideoEncodingPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_subVideoEncodingPicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPickListValueChanged(quint8,QString,int)));
    delete m_subVideoEncodingPicklist;

    disconnect(m_subResolutionPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subResolutionPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_subResolutionPicklist,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotPickListValueChanged(quint8,QString,int)));
    delete m_subResolutionPicklist;

    disconnect(m_subFrameRatePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subFrameRatePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    delete m_subFrameRatePicklist;

    disconnect(m_subCBRTypeRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subCBRTypeRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete m_subCBRTypeRadioButton;

    disconnect(m_subVBRTypeRadioButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subVBRTypeRadioButton,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete m_subVBRTypeRadioButton;

    disconnect(m_subBitRatePicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    disconnect(m_subBitRatePicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_subBitRatePicklist;

    disconnect(m_subQualityPicklist,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subQualityPicklist,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotLoadPickListLoader(int)));
    delete m_subQualityPicklist;

    disconnect(m_subGOPTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_subGOPTextbox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    delete m_subGOPTextbox;
    delete m_subGOPParam;

    disconnect(m_subAudioCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_subAudioCheckbox;

    if(IS_VALID_OBJ(m_subCopyToCamButton))
    {
        disconnect(m_subCopyToCamButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        disconnect (m_subCopyToCamButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotCopyToCamButtonClick(int)));
        DELETE_OBJ(m_subCopyToCamButton);
    }

    if(IS_VALID_OBJ(m_copyToCamera))
    {
        disconnect (m_copyToCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        DELETE_OBJ(m_copyToCamera);
    }

    m_mainVideoEncodingMap.clear();
    m_subVideoEncodingMap.clear();
    m_mainResolutionMap.clear();
    m_subResolutionMap.clear();
    m_mainFrameRateMap.clear();
    m_subFrameRateMap.clear();
    m_mainQualityMap.clear();
    m_subQualityMap.clear();
    m_cameraNameList.clear();
    m_profileMap.clear ();
}

void IpStreamSettings::createDefaultElements ()
{
    m_pickListIndexForCommand = MAX_IP_STREAM_SETTINGS_ELEMENT;
    m_currentCameraIndex = 0;
    m_cameraNameList.clear();

    //copy to camera field init with 0
    memset(&m_mainCopyToCameraField, 0, sizeof(m_mainCopyToCameraField));
    memset(&m_subCopyToCameraField, 0, sizeof(m_subCopyToCameraField));

    INIT_OBJ(m_copyToCamera);
    INIT_OBJ(m_mainCopyToCamButton);
    INIT_OBJ(m_subCopyToCamButton);

    for(quint8 index = 0; index < devTableInfo->ipCams; index++)
    {
        QString cameraName = applController->GetCameraNameOfDevice(currDevName, (index + devTableInfo->analogCams));
        if(((index + 1 + devTableInfo->analogCams ) < 10) && (devTableInfo->totalCams > 10))
        {
            m_cameraNameList.insert(index,QString(" %1 : ").arg(index + 1 + devTableInfo->analogCams) + cameraName);
        }
        else
        {
            m_cameraNameList.insert(index,QString("%1 : ").arg(index + 1 + devTableInfo->analogCams) + cameraName);
        }
    }

    m_cameraNameDropDownBox = new DropDown(((this->width() - SCALE_WIDTH(TILE_WIDTH)) / 2),
                                           (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON) - (SCALE_HEIGHT(INTER_CONTROL_MARGIN) + (8 * BGTILE_HEIGHT) + (2 * (BGTILE_HEIGHT + SCALE_HEIGHT(10))))) / 2,
                                           SCALE_WIDTH(TILE_WIDTH),
                                           BGTILE_HEIGHT,
                                           IP_STREAM_SETTINGS_CAMERANAME_SPINBOX,
                                           DROPDOWNBOX_SIZE_320,
                                           labelString[IP_STREAM_SETTINGS_CAMERANAME_SPINBOX],
                                           m_cameraNameList,
                                           this,
                                           "", false, SCALE_WIDTH(20));
    m_elementList[IP_STREAM_SETTINGS_CAMERANAME_SPINBOX] = m_cameraNameDropDownBox;
    connect(m_cameraNameDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_cameraNameDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinboxValueChanged(QString,quint32)));

    m_mainStreamElementHeading = new ElementHeading(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                                    (m_cameraNameDropDownBox->y() + m_cameraNameDropDownBox->height() + SCALE_HEIGHT(INTER_CONTROL_MARGIN)),
                                                    SCALE_WIDTH(HALF_TILE_WIDTH),
                                                    BGTILE_HEIGHT,
                                                    "Main Stream",
                                                    TOP_LAYER,
                                                    this,
                                                    false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    m_mainCopyToCamButton = new PageOpenButton((m_mainStreamElementHeading->x() + SCALE_HEIGHT(280)),
                                           (m_cameraNameDropDownBox->y() + m_cameraNameDropDownBox->height() + SCALE_HEIGHT(20)),
                                           SCALE_WIDTH(100),
                                           SCALE_HEIGHT(30),
                                           IP_STREAM_SETTINGS_MAIN_COPY_TO_CAM_LABEL,
                                           PAGEOPENBUTTON_EXTRALARGE,
                                           labelString[IP_STREAM_SETTINGS_MAIN_COPY_TO_CAM_LABEL],
                                           this,
                                           "","",
                                           false,
                                           0,
                                           NO_LAYER);
    m_elementList[IP_STREAM_SETTINGS_MAIN_COPY_TO_CAM_LABEL] = m_mainCopyToCamButton;
    connect(m_mainCopyToCamButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_mainCopyToCamButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCopyToCamButtonClick(int)));

    m_profileMap.clear ();
    m_profileMap.insert(0, "");
    m_mainProfilePicklist = new PickList(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                         (m_mainStreamElementHeading->y() + m_mainStreamElementHeading->height()),
                                         SCALE_WIDTH(HALF_TILE_WIDTH),
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         labelString[IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST],
                                         m_profileMap,
                                         0, "Select Profile",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST,
                                         true, true,true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST] = m_mainProfilePicklist;
    connect(m_mainProfilePicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_mainProfilePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainProfilePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_mainVideoEncodingMap.insert(0, "");
    m_mainVideoEncodingPicklist = new PickList(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                               (m_mainProfilePicklist->y() + m_mainProfilePicklist->height()),
                                               SCALE_WIDTH(HALF_TILE_WIDTH),
                                               BGTILE_HEIGHT,
                                               SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                               labelString[IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST],
                                               m_mainVideoEncodingMap,
                                               0, "Select Video Encoding",
                                               this,
                                               MIDDLE_TABLE_LAYER,
                                               -1,
                                               IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST,
                                               true, true, true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST] = m_mainVideoEncodingPicklist;
    connect(m_mainVideoEncodingPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainVideoEncodingPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_mainVideoEncodingPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_mainResolutionMap.insert(0, "");
    m_mainResolutionPicklist = new PickList(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                            (m_mainVideoEncodingPicklist->y() + m_mainVideoEncodingPicklist->height()),
                                            SCALE_WIDTH(HALF_TILE_WIDTH),
                                            BGTILE_HEIGHT,
                                            SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                            labelString[IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST],
                                            m_mainResolutionMap,
                                            0, "Select Resolution",
                                            this,
                                            MIDDLE_TABLE_LAYER,
                                            -1,
                                            IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST,
                                            true, true, true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST] = m_mainResolutionPicklist;
    connect(m_mainResolutionPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainResolutionPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_mainResolutionPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_mainFrameRateMap.insert(0, "");
    m_mainFrameRatePicklist = new PickList(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                           (m_mainResolutionPicklist->y() + m_mainResolutionPicklist->height()),
                                           SCALE_WIDTH(HALF_TILE_WIDTH),
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                           labelString[IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST],
                                           m_mainFrameRateMap,
                                           0, "Select Frame Rate",
                                           this,
                                           MIDDLE_TABLE_LAYER,
                                           -1,
                                           IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST,
                                           true, true, true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST] = m_mainFrameRatePicklist;
    connect(m_mainFrameRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainFrameRatePicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_mainFrameRatePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_mainCBRTypeRadioButton = new OptionSelectButton(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                                      (m_mainFrameRatePicklist->y() + m_mainFrameRatePicklist->height()),
                                                      SCALE_WIDTH(HALF_TILE_WIDTH),
                                                      BGTILE_HEIGHT,
                                                      RADIO_BUTTON_INDEX,
                                                      this,
                                                      MIDDLE_TABLE_LAYER,
                                                      "Bit Rate Type",
                                                      labelString[IP_STREAM_SETTINGS_MAIN_CBRBITRATETYPE_RADIOBUTTON],
                                                      -1,
                                                      IP_STREAM_SETTINGS_MAIN_CBRBITRATETYPE_RADIOBUTTON,
                                                      true,
                                                      NORMAL_FONT_SIZE,
                                                      NORMAL_FONT_COLOR);
    m_elementList[IP_STREAM_SETTINGS_MAIN_CBRBITRATETYPE_RADIOBUTTON] = m_mainCBRTypeRadioButton;
    connect(m_mainCBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainCBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_mainVBRTypeRadioButton = new OptionSelectButton((m_mainCBRTypeRadioButton->x() + SCALE_WIDTH(335)),
                                                      m_mainCBRTypeRadioButton->y(),
                                                      SCALE_WIDTH(HALF_TILE_WIDTH),
                                                      BGTILE_HEIGHT,
                                                      RADIO_BUTTON_INDEX,
                                                      labelString[IP_STREAM_SETTINGS_MAIN_VBRBITRATETYPE_RADIOBUTTON],
                                                      this,
                                                      NO_LAYER,
                                                      -1,
                                                      MX_OPTION_TEXT_TYPE_SUFFIX,
                                                      NORMAL_FONT_SIZE,
                                                      IP_STREAM_SETTINGS_MAIN_VBRBITRATETYPE_RADIOBUTTON);
    m_elementList[IP_STREAM_SETTINGS_MAIN_VBRBITRATETYPE_RADIOBUTTON] = m_mainVBRTypeRadioButton;
    connect(m_mainVBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainVBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_mainBitRateMap.insert(0, "");
    m_mainBitRatePicklist = new PickList(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                         (m_mainCBRTypeRadioButton->y() + m_mainCBRTypeRadioButton->height()),
                                         SCALE_WIDTH(HALF_TILE_WIDTH),
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         labelString[IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST],
                                         m_mainBitRateMap,
                                         0, "Select Bit Rate",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST,
                                         true, true, true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST] = m_mainBitRatePicklist;
    connect(m_mainBitRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_mainBitRatePicklist,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotLoadPickListLoader(int)));
    connect(m_mainBitRatePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_mainQualityMap.insert(0, "");
    m_mainQualityPicklist = new PickList(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                         (m_mainBitRatePicklist->y() + m_mainBitRatePicklist->height()),
                                         SCALE_WIDTH(HALF_TILE_WIDTH),
                                         BGTILE_HEIGHT,
                                         SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                         labelString[IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST],
                                         m_mainQualityMap,
                                         0, "Select Quality",
                                         this,
                                         MIDDLE_TABLE_LAYER,
                                         -1,
                                         IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST,
                                         true, true, true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST] = m_mainQualityPicklist;
    connect(m_mainQualityPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainQualityPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_mainQualityPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_mainGOPParam = new TextboxParam();
    m_mainGOPParam->maxChar = 3;
    m_mainGOPParam->validation = QRegExp("[0-9]");
    m_mainGOPParam->minNumValue = 1;
    m_mainGOPParam->maxNumValue = 100;
    m_mainGOPParam->isNumEntry = true;
    m_mainGOPParam->labelStr = labelString[IP_STREAM_SETTINGS_MAIN_GOP_TEXTBOX];
    m_mainGOPParam->suffixStr = "(1-100)";

    m_mainGOPTextbox = new TextBox(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                   (m_mainQualityPicklist->y() + m_mainQualityPicklist->height()),
                                   SCALE_WIDTH(HALF_TILE_WIDTH),
                                   BGTILE_HEIGHT,
                                   IP_STREAM_SETTINGS_MAIN_GOP_TEXTBOX,
                                   TEXTBOX_EXTRASMALL,
                                   this,
                                   m_mainGOPParam,
                                   MIDDLE_TABLE_LAYER,
                                   true);
    m_elementList[IP_STREAM_SETTINGS_MAIN_GOP_TEXTBOX] = m_mainGOPTextbox;
    connect(m_mainGOPTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainGOPTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (m_mainGOPTextbox,
             SIGNAL(sigTextValueAppended(QString,int)),
             this,
             SLOT(slotTextBoxValueAppended(QString,int)));

    m_mainAudioCheckbox = new OptionSelectButton(SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN),
                                                 (m_mainGOPTextbox->y() + m_mainGOPTextbox->height()),
                                                 SCALE_WIDTH(HALF_TILE_WIDTH),
                                                 BGTILE_HEIGHT,
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 BOTTOM_TABLE_LAYER,
                                                 labelString[IP_STREAM_SETTINGS_MAIN_AUDIO_CHECKBOX],
                                                 "" , -1,
                                                 IP_STREAM_SETTINGS_MAIN_AUDIO_CHECKBOX);
    m_elementList[IP_STREAM_SETTINGS_MAIN_AUDIO_CHECKBOX] = m_mainAudioCheckbox;
    connect(m_mainAudioCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_mainAudioCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_subStreamElementHeading = new ElementHeading((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + (SCALE_WIDTH(INTER_CONTROL_MARGIN) / 2)),
                                                   (m_cameraNameDropDownBox->y() + m_cameraNameDropDownBox->height() + SCALE_HEIGHT(INTER_CONTROL_MARGIN)),
                                                   SCALE_WIDTH(HALF_TILE_WIDTH),
                                                   BGTILE_HEIGHT,
                                                   "Sub Stream",
                                                   TOP_LAYER,
                                                   this,
                                                   false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    m_subCopyToCamButton = new PageOpenButton((m_subStreamElementHeading->x() + SCALE_HEIGHT(280)),
                                                (m_cameraNameDropDownBox->y() + m_cameraNameDropDownBox->height() + SCALE_HEIGHT(20)),
                                                SCALE_WIDTH(100),
                                                SCALE_HEIGHT(30),
                                                IP_STREAM_SETTINGS_SUB_COPY_TO_CAM_LABEL,
                                                PAGEOPENBUTTON_EXTRALARGE,
                                                labelString[IP_STREAM_SETTINGS_SUB_COPY_TO_CAM_LABEL],
                                                this,
                                                "","",
                                                false,
                                                0,
                                                NO_LAYER);
    m_elementList[IP_STREAM_SETTINGS_SUB_COPY_TO_CAM_LABEL] = m_subCopyToCamButton;
    connect(m_subCopyToCamButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_subCopyToCamButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCopyToCamButtonClick(int)));

    m_profileMap.insert(0, "");
    m_subProfilePicklist = new PickList((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                        (m_subStreamElementHeading->y() + m_subStreamElementHeading->height()),
                                        SCALE_WIDTH(HALF_TILE_WIDTH),
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        labelString[IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST],
                                        m_profileMap,
                                        0, "Select Profile",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST,
                                        true, true, true);
    m_elementList[IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST] = m_subProfilePicklist;
    connect(m_subProfilePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subProfilePicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_subProfilePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_subVideoEncodingMap.insert(0, "");
    m_subVideoEncodingPicklist = new PickList((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                              (m_subProfilePicklist->y() + m_subProfilePicklist->height()),
                                              SCALE_WIDTH(HALF_TILE_WIDTH),
                                              BGTILE_HEIGHT,
                                              SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                              labelString[IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST],
                                              m_subVideoEncodingMap,
                                              0, "Select Video Encoding",
                                              this,
                                              MIDDLE_TABLE_LAYER,
                                              -1,
                                              IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST,
                                              true, true, true);
    m_elementList[IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST] = m_subVideoEncodingPicklist;
    connect(m_subVideoEncodingPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subVideoEncodingPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_subVideoEncodingPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_subResolutionMap.insert(0, "");
    m_subResolutionPicklist = new PickList((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                           (m_subVideoEncodingPicklist->y() + m_subVideoEncodingPicklist->height()),
                                           SCALE_WIDTH(HALF_TILE_WIDTH),
                                           BGTILE_HEIGHT,
                                           SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                           labelString[IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST],
                                           m_subResolutionMap,
                                           0, "Select Resolution",
                                           this,
                                           MIDDLE_TABLE_LAYER,
                                           -1,
                                           IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST,
                                           true, true, true);
    m_elementList[IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST] = m_subResolutionPicklist;
    connect(m_subResolutionPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subResolutionPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_subResolutionPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_subFrameRateMap.insert(0, "");
    m_subFrameRatePicklist = new PickList((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                          (m_subResolutionPicklist->y() + m_subResolutionPicklist->height()),
                                          SCALE_WIDTH(HALF_TILE_WIDTH),
                                          BGTILE_HEIGHT,
                                          SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                          labelString[IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST],
                                          m_subFrameRateMap,
                                          0, "Select Frame Rate",
                                          this,
                                          MIDDLE_TABLE_LAYER,
                                          -1,
                                          IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST,
                                          true, true, true);
    m_elementList[IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST] = m_subFrameRatePicklist;
    connect(m_subFrameRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subFrameRatePicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_subFrameRatePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_subCBRTypeRadioButton = new OptionSelectButton((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                                     (m_subFrameRatePicklist->y() + m_subFrameRatePicklist->height()),
                                                     SCALE_WIDTH(HALF_TILE_WIDTH),
                                                     BGTILE_HEIGHT,
                                                     RADIO_BUTTON_INDEX,
                                                     this,
                                                     MIDDLE_TABLE_LAYER,
                                                     "Bit Rate Type",
                                                     labelString[IP_STREAM_SETTINGS_SUB_CBRBITRATETYPE_RADIOBUTTON],
                                                     -1,
                                                     IP_STREAM_SETTINGS_SUB_CBRBITRATETYPE_RADIOBUTTON,
                                                     true,
                                                     NORMAL_FONT_SIZE,
                                                     NORMAL_FONT_COLOR);
    m_elementList[IP_STREAM_SETTINGS_SUB_CBRBITRATETYPE_RADIOBUTTON] = m_subCBRTypeRadioButton;
    connect(m_subCBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subCBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_subVBRTypeRadioButton = new OptionSelectButton((m_subCBRTypeRadioButton->x() + SCALE_WIDTH(335)),
                                                     m_subCBRTypeRadioButton->y(),
                                                     SCALE_WIDTH(HALF_TILE_WIDTH),
                                                     BGTILE_HEIGHT,
                                                     RADIO_BUTTON_INDEX,
                                                     labelString[IP_STREAM_SETTINGS_SUB_VBRBITRATETYPE_RADIOBUTTON],
                                                     this,
                                                     NO_LAYER,
                                                     -1,
                                                     MX_OPTION_TEXT_TYPE_SUFFIX,
                                                     NORMAL_FONT_SIZE,
                                                     IP_STREAM_SETTINGS_SUB_VBRBITRATETYPE_RADIOBUTTON);
    m_elementList[IP_STREAM_SETTINGS_SUB_VBRBITRATETYPE_RADIOBUTTON] = m_subVBRTypeRadioButton;
    connect(m_subVBRTypeRadioButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subVBRTypeRadioButton,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_subBitRateMap.insert (0, "");
    m_subBitRatePicklist = new PickList((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                        (m_subCBRTypeRadioButton->y() + m_subCBRTypeRadioButton->height()),
                                        SCALE_WIDTH(HALF_TILE_WIDTH),
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        labelString[IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST],
                                        m_subBitRateMap,
                                        0, "Select Bit Rate",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST,
                                        true, true, true);
    m_elementList[IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST] = m_subBitRatePicklist;
    connect(m_subBitRatePicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (m_subBitRatePicklist,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotLoadPickListLoader(int)));
    connect(m_subBitRatePicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_subQualityMap.insert(0, "");
    m_subQualityPicklist = new PickList((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                        (m_subBitRatePicklist->y() + m_subBitRatePicklist->height()),
                                        SCALE_WIDTH(HALF_TILE_WIDTH),
                                        BGTILE_HEIGHT,
                                        SCALE_WIDTH(135), SCALE_HEIGHT(30),
                                        labelString[IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST],
                                        m_subQualityMap,
                                        0, "Select Quality",
                                        this,
                                        MIDDLE_TABLE_LAYER,
                                        -1,
                                        IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST,
                                        true, true, true);
    m_elementList[IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST] = m_subQualityPicklist;
    connect(m_subQualityPicklist,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subQualityPicklist,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotLoadPickListLoader(int)));
    connect(m_subQualityPicklist,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotPickListValueChanged(quint8,QString,int)));

    m_subGOPParam = new TextboxParam();
    m_subGOPParam->maxChar = 3;
    m_subGOPParam->validation = QRegExp("[0-9]");
    m_subGOPParam->minNumValue = 1;
    m_subGOPParam->maxNumValue = 100;
    m_subGOPParam->isNumEntry = true;
    m_subGOPParam->labelStr = labelString[IP_STREAM_SETTINGS_SUB_GOP_TEXTBOX];
    m_subGOPParam->suffixStr = "(1-100)";

    m_subGOPTextbox = new TextBox((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                  (m_subQualityPicklist->y() + m_subQualityPicklist->height()),
                                  SCALE_WIDTH(HALF_TILE_WIDTH),
                                  BGTILE_HEIGHT,
                                  IP_STREAM_SETTINGS_SUB_GOP_TEXTBOX,
                                  TEXTBOX_EXTRASMALL,
                                  this,
                                  m_subGOPParam,
                                  MIDDLE_TABLE_LAYER,
                                  true);
    m_elementList[IP_STREAM_SETTINGS_SUB_GOP_TEXTBOX] = m_subGOPTextbox;
    connect(m_subGOPTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subGOPTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    connect (m_subGOPTextbox,
             SIGNAL(sigTextValueAppended(QString,int)),
             this,
             SLOT(slotTextBoxValueAppended(QString,int)));

    m_subAudioCheckbox = new OptionSelectButton((SCALE_WIDTH(STREAM_SETTINGS_LEFT_MARGIN) + SCALE_WIDTH(HALF_TILE_WIDTH) + SCALE_WIDTH(INTER_CONTROL_MARGIN / 2)),
                                                (m_subGOPTextbox->y() + m_subGOPTextbox->height()),
                                                SCALE_WIDTH(HALF_TILE_WIDTH),
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                this,
                                                BOTTOM_TABLE_LAYER,
                                                labelString[IP_STREAM_SETTINGS_SUB_AUDIO_CHECKBOX],
                                                "" , -1,
                                                IP_STREAM_SETTINGS_SUB_AUDIO_CHECKBOX);
    m_elementList[IP_STREAM_SETTINGS_SUB_AUDIO_CHECKBOX] = m_subAudioCheckbox;
    connect(m_subAudioCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_subAudioCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotRadioButtonClicked(OPTION_STATE_TYPE_e,int)));
}

void IpStreamSettings::createCommandPayload(SET_COMMAND_e commandType, quint8 totalFields)
{
    QString payloadString = payloadLib->createDevCmdPayload(totalFields);

    DevCommParam *param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = commandType;
    param->payload = payloadString;

    if(applController->processActivity(currDevName, DEVICE_COMM, param))
    {
        processBar->loadProcessBar();
    }
}

void IpStreamSettings::createConfigPayload(REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             STREAM_TABLE_INDEX,
                                                             (m_currentCameraIndex + 1 + devTableInfo->analogCams),
                                                             (m_currentCameraIndex + 1 + devTableInfo->analogCams),
                                                             1,
                                                             TOTAL_FIELD_STREAM_TABLE,
                                                             TOTAL_FIELD_STREAM_TABLE);
    DevCommParam* param = new DevCommParam();
    param->msgType = requestType;
    param->payload = payloadString;
    if(applController->processActivity(currDevName, DEVICE_COMM, param))
    {
        processBar->loadProcessBar();
    }
}

void IpStreamSettings::fillRecords()
{
    QMap<quint8, QString> tempMap;

    m_profileMap.clear();
    m_profileMap.insert(0, QString("%1").arg (payloadLib->getCnfgArrayAtIndex(MAIN_STREAM_PROFILE_VALUE).toUInt ()));

    m_mainProfilePicklist->changeOptionList (m_profileMap,0,true);

    m_mainVideoEncodingMap.clear();
    m_mainVideoEncodingMap.insert(0, payloadLib->getCnfgArrayAtIndex(MAIN_VIDEO_ENCODING_VALUE).toString());
    m_mainVideoEncodingPicklist->changeOptionList(m_mainVideoEncodingMap,0,true);

    m_mainResolutionMap.clear();
    m_mainResolutionMap.insert(0, payloadLib->getCnfgArrayAtIndex(MAIN_RESOLUTION_VALUE).toString());
    m_mainResolutionPicklist->changeOptionList(m_mainResolutionMap,0,true);

    if(payloadLib->getCnfgArrayAtIndex(MAIN_FRAME_RATE_VALUE).toUInt() != 0)
    {
        m_mainFrameRateMap.clear();
        m_mainFrameRateMap.insert(0, payloadLib->getCnfgArrayAtIndex(MAIN_FRAME_RATE_VALUE).toString());
        m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap,0,true);
    }
    else
    {
        m_mainFrameRateMap.clear();
        m_mainFrameRateMap.insert(0, "");
        m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap,0,true);
    }

    if(payloadLib->getCnfgArrayAtIndex(MAIN_QUALITY_VALUE).toUInt() != 0)
    {
        m_mainQualityMap.clear();
        m_mainQualityMap.insert(0, payloadLib->getCnfgArrayAtIndex(MAIN_QUALITY_VALUE).toString());
        m_mainQualityPicklist->changeOptionList(m_mainQualityMap,0,true);
    }
    else
    {
        m_mainQualityMap.clear();
        m_mainQualityMap.insert(0, "");
        m_mainQualityPicklist->changeOptionList(m_mainQualityMap,0,true);
    }

    m_mainBitRateMap.clear();

    if((payloadLib->getCnfgArrayAtIndex(MAIN_BIT_RATE_VALUE).toInt ()) < MAX_BITRATE_INDEX)
    {
        m_mainBitRateMap.insert(0, bitRateArray[payloadLib->getCnfgArrayAtIndex(MAIN_BIT_RATE_VALUE).toInt ()]);
        m_mainBitRatePicklist->changeOptionList(m_mainBitRateMap,0,true);
    }

    m_mainAudioCheckbox->changeState((OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex(MAIN_AUDIO_STATUS_VALUE).toUInt());

    if(payloadLib->getCnfgArrayAtIndex(MAIN_BIT_RATE_TYPE_VALUE).toUInt() == 0)
    {
        m_mainCBRTypeRadioButton->changeState(OFF_STATE);
        m_mainVBRTypeRadioButton->changeState(ON_STATE);
    }
    else
    {
        m_mainCBRTypeRadioButton->changeState(ON_STATE);
        m_mainVBRTypeRadioButton->changeState(OFF_STATE);
    }

    m_mainGOPTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(MAIN_GOP_VALUE).toString());

    tempMap.clear();
    tempMap.insert(0, QString("%1").arg (payloadLib->getCnfgArrayAtIndex(SUB_STREAM_PROFILE_VALUE).toUInt ()));
    m_subProfilePicklist->changeOptionList (tempMap,0,true);

    m_subVideoEncodingMap.clear();
    m_subVideoEncodingMap.insert(0, payloadLib->getCnfgArrayAtIndex(SUB_VIDEO_ENCODING_VALUE).toString());
    m_subVideoEncodingPicklist->changeOptionList(m_subVideoEncodingMap,0,true);

    m_subResolutionMap.clear();
    m_subResolutionMap.insert(0, payloadLib->getCnfgArrayAtIndex(SUB_RESOLUTION_VALUE).toString());
    m_subResolutionPicklist->changeOptionList(m_subResolutionMap,0,true);

    if(payloadLib->getCnfgArrayAtIndex(SUB_FRAME_RATE_VALUE).toUInt() != 0)
    {
        m_subFrameRateMap.clear();
        m_subFrameRateMap.insert(0, payloadLib->getCnfgArrayAtIndex(SUB_FRAME_RATE_VALUE).toString());
        m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap,0,true);
    }
    else
    {
        m_subFrameRateMap.clear();
        m_subFrameRateMap.insert(0, "");
        m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap,0,true);
    }

    if(payloadLib->getCnfgArrayAtIndex(SUB_QUALITY_VALUE).toUInt() != 0)
    {
        m_subQualityMap.clear();
        m_subQualityMap.insert(0, payloadLib->getCnfgArrayAtIndex(SUB_QUALITY_VALUE).toString());
        m_subQualityPicklist->changeOptionList(m_subQualityMap,0,true);
    }
    else
    {
        m_subQualityMap.clear();
        m_subQualityMap.insert(0, "");
        m_subQualityPicklist->changeOptionList(m_subQualityMap,0,true);
    }

    m_subBitRateMap.clear();
    if((payloadLib->getCnfgArrayAtIndex(SUB_BIT_RATE_VALUE).toInt ()) < MAX_BITRATE_INDEX)
    {
        m_subBitRateMap.insert(0, bitRateArray[payloadLib->getCnfgArrayAtIndex(SUB_BIT_RATE_VALUE).toInt ()]);
        m_subBitRatePicklist->changeOptionList(m_subBitRateMap,0,true);
    }

    m_subAudioCheckbox->changeState((OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex(SUB_AUDIO_STATUS_VALUE).toUInt());

    if(payloadLib->getCnfgArrayAtIndex(SUB_BIT_RATE_TYPE_VALUE).toUInt() == 0)
    {
        m_subCBRTypeRadioButton->changeState(OFF_STATE);
        m_subVBRTypeRadioButton->changeState(ON_STATE);
    }
    else
    {
        m_subCBRTypeRadioButton->changeState(ON_STATE);
        m_subVBRTypeRadioButton->changeState(OFF_STATE);
    }

    m_subGOPTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(SUB_GOP_VALUE).toString());
}

void IpStreamSettings::setRecord()
{
    payloadLib->setCnfgArrayAtIndex(MAIN_VIDEO_ENCODING_VALUE, m_mainVideoEncodingPicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(MAIN_RESOLUTION_VALUE, m_mainResolutionPicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(MAIN_FRAME_RATE_VALUE, m_mainFrameRatePicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(MAIN_QUALITY_VALUE, m_mainQualityPicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(MAIN_AUDIO_STATUS_VALUE, (m_mainAudioCheckbox->getCurrentState() == ON_STATE ? 1 : 0));
    payloadLib->setCnfgArrayAtIndex(MAIN_BIT_RATE_TYPE_VALUE, (m_mainCBRTypeRadioButton->getCurrentState() == ON_STATE ? 1 : 0));
    payloadLib->setCnfgArrayAtIndex(MAIN_BIT_RATE_VALUE, findIndexofBitrateValue(m_mainBitRatePicklist->getCurrentPickStr()));
    payloadLib->setCnfgArrayAtIndex(MAIN_GOP_VALUE, m_mainGOPTextbox->getInputText());

    payloadLib->setCnfgArrayAtIndex(SUB_VIDEO_ENCODING_VALUE, m_subVideoEncodingPicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(SUB_RESOLUTION_VALUE, m_subResolutionPicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(SUB_FRAME_RATE_VALUE, m_subFrameRatePicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(SUB_QUALITY_VALUE, m_subQualityPicklist->getCurrentPickStr());
    payloadLib->setCnfgArrayAtIndex(SUB_AUDIO_STATUS_VALUE, (m_subAudioCheckbox->getCurrentState() == ON_STATE ? 1 : 0));
    payloadLib->setCnfgArrayAtIndex(SUB_BIT_RATE_TYPE_VALUE, (m_subCBRTypeRadioButton->getCurrentState() == ON_STATE ? 1 : 0));
    payloadLib->setCnfgArrayAtIndex(SUB_BIT_RATE_VALUE, findIndexofBitrateValue(m_subBitRatePicklist->getCurrentPickStr()));
    payloadLib->setCnfgArrayAtIndex(SUB_GOP_VALUE, m_subGOPTextbox->getInputText());

    payloadLib->setCnfgArrayAtIndex(MAIN_STREAM_PROFILE_VALUE, m_mainProfilePicklist->getCurrentPickStr ());
    payloadLib->setCnfgArrayAtIndex(SUB_STREAM_PROFILE_VALUE, m_subProfilePicklist->getCurrentPickStr ());

    SET_CAMERA_MASK_BIT(m_mainCopyToCameraField, m_currentCameraIndex);
    SET_CAMERA_MASK_BIT(m_subCopyToCameraField, m_currentCameraIndex);

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex(MAIN_STREAM_COPY_TO_CAM_VALUE_START + maskIdx, m_mainCopyToCameraField.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex(SUB_STREAM_COPY_TO_CAM_VALUE_START + maskIdx, m_subCopyToCameraField.bitMask[maskIdx]);
    }

    /* Reset CopytoCamFields */
    memset(&m_mainCopyToCameraField, 0, sizeof(m_mainCopyToCameraField));
    memset(&m_subCopyToCameraField, 0, sizeof(m_subCopyToCameraField));
}

quint8 IpStreamSettings::findIndexofBitrateValue(QString bitRateStr)
{
    quint8 index = 0;
    for(index = 0; index < MAX_BITRATE_INDEX; index++)
    {
        if(bitRateArray[index] == bitRateStr)
        {
            break;
        }
    }
    return index;
}

void IpStreamSettings::upadateEnableStateForElements()
{
    bool isMainStreamMjpeg = (m_mainVideoEncodingPicklist->getCurrentPickStr() == QString(CODEC_STRING));
    bool isSubStreamMjpeg = (m_subVideoEncodingPicklist->getCurrentPickStr() == QString(CODEC_STRING));

    /* ONVIF media1 (Profile-S) doesn't support VBR/CBR. It is available in media2 (Profile-T).
       MJPEG codec doesn't have VBR support */
    /* Main Stream */
    if ((true == isMainStreamMjpeg) || ((isOnvifSupport == true) && (media2Support == false)))
    {
        m_mainCBRTypeRadioButton->setIsEnabled(false);
        m_mainVBRTypeRadioButton->setIsEnabled(false);
    }
    else
    {
        m_mainCBRTypeRadioButton->setIsEnabled(true);
        m_mainVBRTypeRadioButton->setIsEnabled(true);
    }

    if ((true == isMainStreamMjpeg) || (m_mainVBRTypeRadioButton->getCurrentState() == OFF_STATE))
    {
        m_mainQualityPicklist->setIsEnabled(false);
    }
    else
    {
        m_mainQualityPicklist->setIsEnabled(true);
    }

    m_mainGOPTextbox->setIsEnabled(isMainStreamMjpeg ? FALSE : TRUE);

    /* Sub Stream */
    if ((true == isSubStreamMjpeg) || ((isOnvifSupport == true) && (media2Support == false)))
    {
        m_subCBRTypeRadioButton->setIsEnabled(false);
        m_subVBRTypeRadioButton->setIsEnabled(false);
    }
    else
    {
        m_subCBRTypeRadioButton->setIsEnabled(true);
        m_subVBRTypeRadioButton->setIsEnabled(true);

    }

    if ((true == isSubStreamMjpeg) || (m_subVBRTypeRadioButton->getCurrentState() == OFF_STATE))
    {
        m_subQualityPicklist->setIsEnabled(false);
    }
    else
    {
        m_subQualityPicklist->setIsEnabled(true);
    }

    m_subGOPTextbox->setIsEnabled(isSubStreamMjpeg ? FALSE : TRUE);
}

void IpStreamSettings::getConfig()
{
    //copy to camera field init with 0
    memset(&m_mainCopyToCameraField, 0, sizeof(m_mainCopyToCameraField));
    memset(&m_subCopyToCameraField, 0, sizeof(m_subCopyToCameraField));
    createConfigPayload(MSG_GET_CFG);
}

void IpStreamSettings::defaultConfig()
{
    createConfigPayload(MSG_DEF_CFG);
}

void IpStreamSettings::saveConfig()
{
    if(m_mainProfilePicklist->getCurrentPickStr() == m_subProfilePicklist->getCurrentPickStr())
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(IP_STRM_SET_PROFILE_ERROR));
    }
    else
    {
        if ((m_mainGOPTextbox->doneKeyValidation()) && (m_subGOPTextbox->doneKeyValidation()))
        {
            if (true == validateRecord())
            {
                setRecord();
                createConfigPayload(MSG_SET_CFG);
            }
        }
    }
}

void IpStreamSettings::getConfigOfIpCamera ()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             IP_CAMERA_SETTING_TABLE_INDEX,
                                                             (m_currentCameraIndex + 1 + devTableInfo->analogCams),
                                                             (m_currentCameraIndex + 1 + devTableInfo->analogCams),
                                                             IP_CAMERA_ONVIF_PORT,
                                                             IP_CAMERA_ONVIF_PORT,
                                                             MAX_IP_CAMERA_SETTINGS_FIELDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    if(applController->processActivity(currDevName, DEVICE_COMM, param))
    {
        if(!processBar->isVisible ())
        {
            processBar->loadProcessBar();
        }
    }
}

void IpStreamSettings::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        m_pickListIndexForCommand = MAX_IP_STREAM_SETTINGS_ELEMENT;
        processBar->unloadProcessBar();
        return;
    }

    if(param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        m_pickListIndexForCommand = MAX_IP_STREAM_SETTINGS_ELEMENT;
        processBar->unloadProcessBar();
        return;
    }

    quint64 totalFields = 0;
    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(MSG_GET_CFG, param->payload);
            if(payloadLib->getcnfgTableIndex(0) == STREAM_TABLE_INDEX)
            {
                fillRecords();
                getConfigOfIpCamera ();
            }
            if(payloadLib->getcnfgTableIndex (0) == IP_CAMERA_SETTING_TABLE_INDEX)
            {
                isOnvifSupport = payloadLib->getCnfgArrayAtIndex (0).toBool ();

                /* Onvif Media 2 Profile sends CBR/VBR detail, hence sending cmd to get media 2 supported/notsupported */
                if (isOnvifSupport == true)
                {
                    payloadLib->setCnfgArrayAtIndex(0, m_currentCameraIndex + devTableInfo->analogCams + 1);
                    payloadLib->setCnfgArrayAtIndex(1, CAPABILITY_CMD_ID_ONVIF_MEDIA2_SUPPORT);
                    createCommandPayload(GET_CAPABILITY, 2);
                }

                upadateEnableStateForElements();
            }
        }
        break;

        case MSG_DEF_CFG:
        {
            getConfig();
        }
        break;

        case MSG_SET_CFG:
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(IP_STRM_SET_PROFILE_SUCCESS));
        }
        break;

        case MSG_SET_CMD:
        {
            payloadLib->parseDevCmdReply(true, param->payload);
            switch(param->cmdType)
            {
                case ENCDR_SUP:
                {
                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST)
                    {
                        m_mainVideoEncodingMap.clear();
                        for(quint8 index = 0; index < payloadLib->getTotalCmdFields(); index++)
                        {
                            m_mainVideoEncodingMap.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                        }
                        m_mainVideoEncodingPicklist->loadPickListOnResponse(m_mainVideoEncodingMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_subVideoEncodingPicklist->changeOptionList (m_mainVideoEncodingMap,
                                                                          m_mainVideoEncodingMap.key(m_mainVideoEncodingPicklist->getCurrentPickStr()), true);
                        }
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST)
                    {
                        m_subVideoEncodingMap.clear();
                        for(quint8 index = 0; index < payloadLib->getTotalCmdFields(); index++)
                        {
                            m_subVideoEncodingMap.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                        }
                        m_subVideoEncodingPicklist->loadPickListOnResponse(m_subVideoEncodingMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_mainVideoEncodingPicklist->changeOptionList (m_subVideoEncodingMap,
                                                                           m_subVideoEncodingMap.key(m_subVideoEncodingPicklist->getCurrentPickStr()), true);
                        }
                    }
                }
                break;

                case RES_SUP:
                {
                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST)
                    {
                        m_mainResolutionMap.clear();
                        for(quint8 index = 0; index < payloadLib->getTotalCmdFields(); index++)
                        {
                            m_mainResolutionMap.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                        }
                        m_mainResolutionPicklist->loadPickListOnResponse(m_mainResolutionMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_subResolutionPicklist->changeOptionList(m_mainResolutionMap,
                                                                      m_mainResolutionMap.key (m_mainResolutionPicklist->getCurrentPickStr ()), true);
                        }
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST)
                    {
                        m_subResolutionMap.clear();
                        for(quint8 index = 0; index < payloadLib->getTotalCmdFields(); index++)
                        {
                            m_subResolutionMap.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                        }
                        m_subResolutionPicklist->loadPickListOnResponse(m_subResolutionMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_mainResolutionMap = m_subResolutionMap;
                            m_mainResolutionPicklist->changeOptionList(m_subResolutionMap,
                                                                       m_subResolutionMap.key (m_subResolutionPicklist->getCurrentPickStr ()), true);
                        }
                    }
                }
                break;

                case FR_SUP:
                {
                    totalFields = payloadLib->getCnfgArrayAtIndex(0).toULongLong();
                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST)
                    {
                        m_mainFrameRateMap.clear();
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST)
                    {
                        m_subFrameRateMap.clear();
                    }

                    for(quint8 index = 0; index < 61; index++)
                    {
                        quint8 value = (totalFields & (0x00000001));
                        if(value != 0)
                        {
                            if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST)
                            {
                                m_mainFrameRateMap.insert(index, QString("%1").arg(index + 1));
                            }
                            else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST)
                            {
                                m_subFrameRateMap.insert(index, QString("%1").arg(index + 1));
                            }
                        }
                        totalFields = totalFields >> 1;
                    }

                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST)
                    {
                        m_mainFrameRatePicklist->loadPickListOnResponse(m_mainFrameRateMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_subFrameRateMap.clear();
                            m_subFrameRateMap = m_mainFrameRateMap;
                            m_subFrameRatePicklist->changeOptionList(m_mainFrameRateMap,
                                                                     m_mainFrameRateMap.key (m_mainFrameRatePicklist->getCurrentPickStr ()), true);
                        }
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST)
                    {
                        m_subFrameRatePicklist->loadPickListOnResponse(m_subFrameRateMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_mainFrameRateMap.clear();
                            m_mainFrameRateMap = m_subFrameRateMap;
                            m_mainFrameRatePicklist->changeOptionList(m_subFrameRateMap,
                                                                      m_subFrameRateMap.key(m_subFrameRatePicklist->getCurrentPickStr ()), true);
                        }
                    }
                }
                break;

                case GET_BITRATE:
                {
                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST)
                    {
                        m_mainBitRateMap.clear();
                        for(quint8 index = 0; index < payloadLib->getTotalCmdFields(); index++)
                        {
                            m_mainBitRateMap.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                        }
                        m_mainBitRatePicklist->loadPickListOnResponse(m_mainBitRateMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_subBitRateMap.clear();
                            m_subBitRateMap = m_mainBitRateMap;
                            m_subBitRatePicklist->changeOptionList(m_mainBitRateMap,
                                                                   m_mainBitRateMap.key(m_mainBitRatePicklist->getCurrentPickStr ()), true);
                        }
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST)
                    {
                        m_subBitRateMap.clear();
                        for(quint8 index = 0; index < payloadLib->getTotalCmdFields(); index++)
                        {
                            m_subBitRateMap.insert(index, payloadLib->getCnfgArrayAtIndex(index).toString());
                        }
                        m_subBitRatePicklist->loadPickListOnResponse(m_subBitRateMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_mainBitRateMap.clear();
                            m_mainBitRateMap = m_subBitRateMap;
                            m_mainBitRatePicklist->changeOptionList(m_subBitRateMap,
                                                                    m_subBitRateMap.key(m_subBitRatePicklist->getCurrentPickStr ()), true);
                        }
                    }
                }
                break;

                case QLT_SUP:
                {
                    totalFields = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST)
                    {
                        m_mainQualityMap.clear();
                        for(quint8 index = 0; index < totalFields; index++)
                        {
                            m_mainQualityMap.insert(index, QString("%1").arg(index + 1));
                        }
                        m_mainQualityPicklist->loadPickListOnResponse(m_mainQualityMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_subQualityMap.clear();
                            m_subQualityMap = m_mainQualityMap;
                            m_subQualityPicklist->changeOptionList(m_subQualityMap,
                                                                   m_subQualityMap.key(m_mainQualityPicklist->getCurrentPickStr ()), true);
                        }
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST)
                    {
                        m_subQualityMap.clear();
                        for(quint8 index = 0; index < totalFields; index++)
                        {
                            m_subQualityMap.insert(index, QString("%1").arg(index + 1));
                        }
                        m_subQualityPicklist->loadPickListOnResponse(m_subQualityMap);

                        if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                        {
                            m_mainQualityMap.clear();
                            m_mainQualityMap = m_subQualityMap;
                            m_mainQualityPicklist->changeOptionList(m_subQualityMap,
                                                                    m_subQualityMap.key (m_subQualityPicklist->getCurrentPickStr ()), true);
                        }
                    }
                }
                break;

                case GET_MAX_SUPP_PROF:
                {
                    m_profileMap.clear();
                    for(quint8 index = 0; index < payloadLib->getCnfgArrayAtIndex(0).toUInt (); index++)
                    {
                        m_profileMap.insert(index, QString("%1").arg (index + 1));
                    }

                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST)
                    {
                        m_mainProfilePicklist->loadPickListOnResponse (m_profileMap);
                        m_subProfilePicklist->changeOptionList (m_profileMap,m_profileMap.key (m_subProfilePicklist->getCurrentPickStr ()));
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST)
                    {
                        m_subProfilePicklist->loadPickListOnResponse (m_profileMap);
                        m_mainProfilePicklist->changeOptionList (m_profileMap,m_profileMap.key (m_mainProfilePicklist->getCurrentPickStr ()));
                    }
                    upadateEnableStateForElements();

                }
                break;

                case GET_PROF_PARA:
                {
                    quint8 bitRateType;
                    if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST)
                    {
                        m_mainVideoEncodingMap.clear();
                        m_mainVideoEncodingMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_VIDEO_ENCODING).toString());
                        m_mainVideoEncodingPicklist->changeOptionList(m_mainVideoEncodingMap);

                        m_mainResolutionMap.clear();
                        m_mainResolutionMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_RESOLUTION).toString());
                        m_mainResolutionPicklist->changeOptionList(m_mainResolutionMap);

                        if(payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_FRAME_RATE).toUInt() != 0)
                        {
                            m_mainFrameRateMap.clear();
                            m_mainFrameRateMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_FRAME_RATE).toString());
                            m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap);
                        }
                        else
                        {
                            m_mainFrameRateMap.clear();
                            m_mainFrameRateMap.insert(0, "");
                            m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap);
                        }

                        if(payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_QUALITY).toUInt() != 0)
                        {
                            m_mainQualityMap.clear();
                            m_mainQualityMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_QUALITY).toString());
                            m_mainQualityPicklist->changeOptionList(m_mainQualityMap);
                        }
                        else
                        {
                            m_mainQualityMap.clear();
                            m_mainQualityMap.insert(0, "");
                            m_mainQualityPicklist->changeOptionList(m_mainQualityMap);
                        }

                        m_mainBitRateMap.clear();

                        if((payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_BIT_RATE).toInt ()) < MAX_BITRATE_INDEX)
                        {
                            m_mainBitRateMap.insert(0, bitRateArray[payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_BIT_RATE).toInt ()]);
                            m_mainBitRatePicklist->changeOptionList(m_mainBitRateMap);
                        }

                        m_mainAudioCheckbox->changeState((OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_AUDIO).toUInt());

                        bitRateType = payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_BIT_TYPE).toUInt();
                        if(bitRateType == 0)
                        {
                            m_mainCBRTypeRadioButton->changeState(OFF_STATE);
                            m_mainVBRTypeRadioButton->changeState(ON_STATE);
                        }
                        else
                        {
                            m_mainCBRTypeRadioButton->changeState(ON_STATE);
                            m_mainVBRTypeRadioButton->changeState(OFF_STATE);
                        }

                        m_mainGOPTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_GOP).toString());

                        if(m_subProfilePicklist->getCurrentPickStr () == m_mainProfilePicklist->getCurrentPickStr ())
                        {
                            m_subVideoEncodingPicklist->changeOptionList (m_mainVideoEncodingMap,
                                                                          m_mainVideoEncodingPicklist->getCurrentValue (),true);
                            m_subResolutionPicklist->changeOptionList (m_mainResolutionMap,m_mainResolutionPicklist->getCurrentValue (),true);
                            m_subFrameRatePicklist->changeOptionList (m_mainFrameRateMap,m_mainFrameRatePicklist->getCurrentValue (),true);
                            m_subBitRatePicklist->changeOptionList (m_mainBitRateMap,m_mainBitRatePicklist->getCurrentValue (),true);
                            m_subQualityPicklist->changeOptionList (m_mainQualityMap,m_mainQualityPicklist->getCurrentValue (),true);
                            m_subVBRTypeRadioButton->changeState (m_mainVBRTypeRadioButton->getCurrentState ());
                            m_subCBRTypeRadioButton->changeState (m_mainCBRTypeRadioButton->getCurrentState ());
                            m_subAudioCheckbox->changeState (m_mainAudioCheckbox->getCurrentState ());
                            m_subGOPTextbox->setInputText (m_mainGOPTextbox->getInputText ());
                        }
                    }
                    else if(m_pickListIndexForCommand == IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST)
                    {
                        m_subVideoEncodingMap.clear();
                        m_subVideoEncodingMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_VIDEO_ENCODING).toString());
                        m_subVideoEncodingPicklist->changeOptionList(m_subVideoEncodingMap);

                        m_subResolutionMap.clear();
                        m_subResolutionMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_RESOLUTION).toString());
                        m_subResolutionPicklist->changeOptionList(m_subResolutionMap);

                        if(payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_FRAME_RATE).toUInt() != 0)
                        {
                            m_subFrameRateMap.clear();
                            m_subFrameRateMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_FRAME_RATE).toString());
                            m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap);
                        }
                        else
                        {
                            m_subFrameRateMap.clear();
                            m_subFrameRateMap.insert(0, "");
                            m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap);
                        }

                        if(payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_QUALITY).toUInt() != 0)
                        {
                            m_subQualityMap.clear();
                            m_subQualityMap.insert(0, payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_QUALITY).toString());
                            m_subQualityPicklist->changeOptionList(m_subQualityMap);
                        }
                        else
                        {
                            m_subQualityMap.clear();
                            m_subQualityMap.insert(0, "");
                            m_subQualityPicklist->changeOptionList(m_subQualityMap);
                        }

                        m_subBitRateMap.clear();
                        if((payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_BIT_RATE).toInt ()) < MAX_BITRATE_INDEX)
                        {
                            m_subBitRateMap.insert(0, bitRateArray[payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_BIT_RATE).toInt ()]);
                            m_subBitRatePicklist->changeOptionList(m_subBitRateMap);
                        }

                        m_subAudioCheckbox->changeState((OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_AUDIO).toUInt());

                        bitRateType = payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_BIT_TYPE).toUInt();
                        if(bitRateType == 0)
                        {
                            m_subCBRTypeRadioButton->changeState(OFF_STATE);
                            m_subVBRTypeRadioButton->changeState(ON_STATE);
                        }
                        else
                        {
                            m_subCBRTypeRadioButton->changeState(ON_STATE);
                            m_subVBRTypeRadioButton->changeState(OFF_STATE);
                        }

                        m_subGOPTextbox->setInputText(payloadLib->getCnfgArrayAtIndex(STREAM_PROFILE_GOP).toString());

                        if(m_subProfilePicklist->getCurrentPickStr () == m_mainProfilePicklist->getCurrentPickStr ())
                        {
                            m_mainVideoEncodingPicklist->changeOptionList (m_subVideoEncodingMap,
                                                                           m_subVideoEncodingPicklist->getCurrentValue (),true);
                            m_mainResolutionPicklist->changeOptionList (m_subResolutionMap,m_subResolutionPicklist->getCurrentValue (),true);
                            m_mainFrameRatePicklist->changeOptionList (m_subFrameRateMap,m_subFrameRatePicklist->getCurrentValue (),true);
                            m_mainBitRatePicklist->changeOptionList (m_subBitRateMap,m_subBitRatePicklist->getCurrentValue (),true);
                            m_mainQualityPicklist->changeOptionList (m_subQualityMap,m_subQualityPicklist->getCurrentValue (),true);
                            m_mainVBRTypeRadioButton->changeState (m_subVBRTypeRadioButton->getCurrentState ());
                            m_mainCBRTypeRadioButton->changeState (m_subCBRTypeRadioButton->getCurrentState ());
                            m_mainAudioCheckbox->changeState (m_subAudioCheckbox->getCurrentState ());
                            m_mainGOPTextbox->setInputText (m_subGOPTextbox->getInputText ());
                        }
                    }
                    upadateEnableStateForElements();
                }
                break;

                /* storing onvif media 2 profile support for Enabling VBR/CBR radio Button */
                case GET_CAPABILITY:
                {
                    if (CAPABILITY_CMD_ID_ONVIF_MEDIA2_SUPPORT != (CAPABILITY_CMD_ID_e)payloadLib->getCnfgArrayAtIndex(0).toInt())
                    {
                        break;
                    }

                    media2Support = payloadLib->getCnfgArrayAtIndex(1).toBool();
                    upadateEnableStateForElements();
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

    m_pickListIndexForCommand = MAX_IP_STREAM_SETTINGS_ELEMENT;
    processBar->unloadProcessBar();
}

void IpStreamSettings::slotSpinboxValueChanged(QString string, quint32)
{
    m_currentCameraIndex = m_cameraNameList.key(string);
    getConfig();
}

void IpStreamSettings::slotLoadPickListLoader(int indexInPage)
{
    bool createCommandFlag = true;
    SET_COMMAND_e cmdType = MAX_NET_COMMAND;
    quint8 totalFields = 0;
    LIVE_STREAM_TYPE_e streamType = LIVE_STREAM_TYPE_MAIN;

    switch(indexInPage)
    {
        case IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST:
        case IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST:
        {
            createCommandFlag = false;
            m_pickListIndexForCommand = indexInPage;
            payloadLib->setCnfgArrayAtIndex(0, (m_currentCameraIndex + devTableInfo->analogCams + 1));
            createCommandPayload(GET_MAX_SUPP_PROF,1);
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST:
        {
            streamType = LIVE_STREAM_TYPE_MAIN;
            cmdType = ENCDR_SUP;
            totalFields = 3;
            payloadLib->setCnfgArrayAtIndex(2, m_mainProfilePicklist->getCurrentPickStr());
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST:
        {
            if(m_mainVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_MAIN;
                cmdType = RES_SUP;
                totalFields = 4;
                payloadLib->setCnfgArrayAtIndex(1, m_mainVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(3, m_mainProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST:
        {
            if(m_mainVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else if(m_mainResolutionPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_RESOLUTION));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_MAIN;
                cmdType = FR_SUP;
                totalFields = 5;
                payloadLib->setCnfgArrayAtIndex(1, m_mainVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(2, m_mainResolutionPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(4, m_mainProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST:
        {
            if(m_mainVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_MAIN;
                cmdType = GET_BITRATE;
                totalFields = 4;
                payloadLib->setCnfgArrayAtIndex(1, m_mainVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(3, m_mainProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST:
        {
            if(m_mainVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_MAIN;
                cmdType = QLT_SUP;
                totalFields = 4;
                payloadLib->setCnfgArrayAtIndex(1, m_mainVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(3, m_mainProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST:
        {
            streamType = LIVE_STREAM_TYPE_SUB;
            cmdType = ENCDR_SUP;
            totalFields = 3;
            payloadLib->setCnfgArrayAtIndex(2, m_subProfilePicklist->getCurrentPickStr());
        }
        break;

        case IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST:
        {
            if(m_subVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_SUB;
                cmdType = RES_SUP;
                totalFields = 4;
                payloadLib->setCnfgArrayAtIndex(1, m_subVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(3, m_subProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST:
        {
            if(m_subVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else if(m_subResolutionPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_RESOLUTION));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_SUB;
                cmdType = FR_SUP;
                totalFields = 5;
                payloadLib->setCnfgArrayAtIndex(1, m_subVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(2, m_subResolutionPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(4, m_subProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST:
        {
            if(m_subVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_SUB;
                cmdType = GET_BITRATE;
                totalFields = 4;
                payloadLib->setCnfgArrayAtIndex(1, m_subVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(3, m_subProfilePicklist->getCurrentPickStr());
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST:
        {
            if(m_subVideoEncodingPicklist->getCurrentPickStr() == "")
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(IP_STRM_SET_VIDEO_ENCODE));
                createCommandFlag = false;
            }
            else
            {
                streamType = LIVE_STREAM_TYPE_SUB;
                cmdType = QLT_SUP;
                totalFields = 4;
                payloadLib->setCnfgArrayAtIndex(1, m_subVideoEncodingPicklist->getCurrentPickStr());
                payloadLib->setCnfgArrayAtIndex(3, m_subProfilePicklist->getCurrentPickStr());
            }
        }
        break;
    }

    if(createCommandFlag == true)
    {
        m_pickListIndexForCommand = indexInPage;
        payloadLib->setCnfgArrayAtIndex(0, (m_currentCameraIndex + devTableInfo->analogCams + 1));
        payloadLib->setCnfgArrayAtIndex((totalFields - 2), streamType);
        createCommandPayload(cmdType, totalFields);
    }
}

void IpStreamSettings::slotPickListValueChanged(quint8 key, QString str, int indexInPage)
{
    switch(indexInPage)
    {
        case IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST:
        {
            m_pickListIndexForCommand = IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST;
            payloadLib->setCnfgArrayAtIndex(0, (m_currentCameraIndex + devTableInfo->analogCams + 1));
            payloadLib->setCnfgArrayAtIndex(1, LIVE_STREAM_TYPE_MAIN);
            payloadLib->setCnfgArrayAtIndex(2, (key + 1));
            createCommandPayload(GET_PROF_PARA,3);
        }
        break;

        case IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST:
        {
            m_pickListIndexForCommand = IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST;
            payloadLib->setCnfgArrayAtIndex(0, (m_currentCameraIndex + devTableInfo->analogCams + 1));
            payloadLib->setCnfgArrayAtIndex(1, LIVE_STREAM_TYPE_SUB);
            payloadLib->setCnfgArrayAtIndex(2, (key + 1));
            createCommandPayload(GET_PROF_PARA,3);
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST:
        {
            m_mainResolutionMap.clear();
            m_mainResolutionMap.insert(0, "");
            m_mainResolutionPicklist->changeOptionList(m_mainResolutionMap,0,true);

            m_mainFrameRateMap.clear();
            m_mainFrameRateMap.insert(0, "");
            m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap,0,true);

            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_subVideoEncodingPicklist->changeOptionList(m_mainVideoEncodingMap, m_mainVideoEncodingMap.key (str), true);

                m_subResolutionMap.clear();
                m_subResolutionMap.insert(0, "");
                m_subResolutionPicklist->changeOptionList(m_subResolutionMap,0,true);

                m_subFrameRateMap.clear();
                m_subFrameRateMap.insert(0, "");
                m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap,0,true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST:
        {
            m_subResolutionMap.clear();
            m_subResolutionMap.insert(0, "");
            m_subResolutionPicklist->changeOptionList(m_subResolutionMap,0,true);

            m_subFrameRateMap.clear();
            m_subFrameRateMap.insert(0, "");
            m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap,0,true);

            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_mainVideoEncodingPicklist->changeOptionList(m_subVideoEncodingMap, m_subVideoEncodingMap.key (str), true);

                m_mainResolutionMap.clear();
                m_mainResolutionMap.insert(0, "");
                m_mainResolutionPicklist->changeOptionList(m_mainResolutionMap,0,true);

                m_mainFrameRateMap.clear();
                m_mainFrameRateMap.insert(0, "");
                m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap,0,true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST:
        {
            m_mainFrameRateMap.clear();
            m_mainFrameRateMap.insert(0, "");
            m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap,0,true);

            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_subResolutionPicklist->changeOptionList(m_mainResolutionMap, m_mainResolutionMap.key (str), true);
                m_subFrameRateMap.clear();
                m_subFrameRateMap.insert(0, "");
                m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap,0,true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST:
        {
            m_subFrameRateMap.clear();
            m_subFrameRateMap.insert(0, "");
            m_subFrameRatePicklist->changeOptionList(m_subFrameRateMap,0,true);

            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_mainResolutionPicklist->changeOptionList(m_subResolutionMap, m_subResolutionMap.key (str), true);
                m_mainFrameRateMap.clear();
                m_mainFrameRateMap.insert(0, "");
                m_mainFrameRatePicklist->changeOptionList(m_mainFrameRateMap,0,true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST:
        {
            if((m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                    && (m_mainFrameRatePicklist->getCurrentPickStr () != m_subFrameRatePicklist->getCurrentPickStr ()))
            {
                m_subFrameRatePicklist->changeOptionList(m_mainFrameRateMap, m_mainFrameRateMap.key (str), true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST:
        {
            if((m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                    && (m_mainFrameRatePicklist->getCurrentPickStr () != m_subFrameRatePicklist->getCurrentPickStr ()))
            {
                m_mainFrameRatePicklist->changeOptionList(m_subFrameRateMap, m_subFrameRateMap.key(str), true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST:
        {
            if((m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                    && (m_mainBitRatePicklist->getCurrentPickStr () != m_subBitRatePicklist->getCurrentPickStr ()))
            {
                m_subBitRatePicklist->changeOptionList(m_mainBitRateMap, m_mainBitRateMap.key (str), true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST:
        {
            if((m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                    && (m_mainQualityPicklist->getCurrentPickStr () != m_subQualityPicklist->getCurrentPickStr ()))
            {
                m_subQualityPicklist->changeOptionList(m_mainQualityMap, m_mainQualityMap.key (str), true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST:
        {
            if((m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                    && (m_mainBitRatePicklist->getCurrentPickStr () != m_subBitRatePicklist->getCurrentPickStr ()))
            {
                m_mainBitRatePicklist->changeOptionList(m_subBitRateMap, m_subBitRateMap.key (str), true);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST:
        {
            if((m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
                    && (m_mainQualityPicklist->getCurrentValue () != m_subQualityPicklist->getCurrentValue ()))
            {
                m_mainQualityPicklist->changeOptionList(m_subQualityMap, m_subQualityMap.key (str), true);
            }
        }
        break;
    }
    upadateEnableStateForElements();
}

void IpStreamSettings::slotRadioButtonClicked(OPTION_STATE_TYPE_e state, int indexInPage)
{
    switch(indexInPage)
    {
        case IP_STREAM_SETTINGS_MAIN_CBRBITRATETYPE_RADIOBUTTON:
        {
            m_mainVBRTypeRadioButton->changeState(OFF_STATE);
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_subCBRTypeRadioButton->changeState(ON_STATE);
                m_subVBRTypeRadioButton->changeState(OFF_STATE);
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_VBRBITRATETYPE_RADIOBUTTON:
        {
            m_mainCBRTypeRadioButton->changeState(OFF_STATE);
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_subCBRTypeRadioButton->changeState(OFF_STATE);
                m_subVBRTypeRadioButton->changeState(ON_STATE);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_CBRBITRATETYPE_RADIOBUTTON:
        {
            m_subVBRTypeRadioButton->changeState(OFF_STATE);
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_mainCBRTypeRadioButton->changeState(ON_STATE);
                m_mainVBRTypeRadioButton->changeState(OFF_STATE);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_VBRBITRATETYPE_RADIOBUTTON:
        {
            m_subCBRTypeRadioButton->changeState(OFF_STATE);
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_mainCBRTypeRadioButton->changeState(OFF_STATE);
                m_mainVBRTypeRadioButton->changeState(ON_STATE);
            }
        }
        break;

        case IP_STREAM_SETTINGS_MAIN_AUDIO_CHECKBOX:
        {
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_subAudioCheckbox->changeState (state);
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_AUDIO_CHECKBOX:
        {
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_mainAudioCheckbox->changeState (state);
            }
        }
        break;
    }
    upadateEnableStateForElements();
}

void IpStreamSettings::slotTextBoxLoadInfopage(int indexInPage, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        if(indexInPage == IP_STREAM_SETTINGS_MAIN_GOP_TEXTBOX)
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_MAIN_GOP));
        }
        else if(indexInPage == IP_STREAM_SETTINGS_SUB_GOP_TEXTBOX)
        {
             infoPage->loadInfoPage(ValidationMessage::getValidationMessage(AUTO_CONF_SUB_GOP));
        }
    }
}

void IpStreamSettings::slotTextBoxValueAppended (QString, int indexInPage)
{
    switch(indexInPage)
    {
        case IP_STREAM_SETTINGS_MAIN_GOP_TEXTBOX:
        {
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_subGOPTextbox->setInputText (m_mainGOPTextbox->getInputText ());
            }
        }
        break;

        case IP_STREAM_SETTINGS_SUB_GOP_TEXTBOX:
        {
            if(m_mainProfilePicklist->getCurrentPickStr () == m_subProfilePicklist->getCurrentPickStr ())
            {
                m_mainGOPTextbox->setInputText (m_subGOPTextbox->getInputText ());
            }
        }
        break;
    }
}

void IpStreamSettings::slotCopyToCamButtonClick(int index)
{
    if (true == IS_VALID_OBJ(m_copyToCamera))
    {
        return;
    }

    CAMERA_BIT_MASK_t &pCopyToCameraFields = ((IP_STREAM_SETTINGS_MAIN_COPY_TO_CAM_LABEL == index) ? m_mainCopyToCameraField : m_subCopyToCameraField);
    memset(&pCopyToCameraFields, 0, sizeof(pCopyToCameraFields));
    SET_CAMERA_MASK_BIT(pCopyToCameraFields, m_currentCameraIndex);

    m_copyToCamera = new CopyToCamera(m_cameraNameList,
                                pCopyToCameraFields,
                                parentWidget(),
                                QString((IP_STREAM_SETTINGS_MAIN_COPY_TO_CAM_LABEL == index) ? "Main Stream" : "Sub Stream"),
                                index);
    connect (m_copyToCamera,
         SIGNAL(sigDeleteObject(quint8)),
         this,
         SLOT(slotSubObjectDelete(quint8)));
}

void IpStreamSettings::slotSubObjectDelete(quint8)
{
    if(IS_VALID_OBJ(m_copyToCamera))
    {
        disconnect (m_copyToCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        DELETE_OBJ(m_copyToCamera);
    }

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

bool IpStreamSettings::validateRecord()
{
    /* Main Stream */
    if (NULL == m_mainVideoEncodingPicklist->getCurrentPickStr())
    {
         infoPage->loadInfoPage(Multilang("Mandatory") + QString(": ") + Multilang(labelString[IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST].toUtf8().constData()));
         return false;
    }

    if (NULL == m_mainResolutionPicklist->getCurrentPickStr())
    {
        infoPage->loadInfoPage(Multilang("Mandatory") + QString(": ") + Multilang(labelString[IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST].toUtf8().constData()));
        return false;
    }

    if (NULL == m_mainFrameRatePicklist->getCurrentPickStr())
    {
        infoPage->loadInfoPage(Multilang("Mandatory") + QString(": ") + Multilang(labelString[IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST].toUtf8().constData()));
        return false;
    }

    /* Sub Stream */
    if (NULL == m_subVideoEncodingPicklist->getCurrentPickStr())
    {
        infoPage->loadInfoPage(Multilang("Mandatory") + QString(": ") + Multilang(labelString[IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST].toUtf8().constData()));
        return false;
    }

    if (NULL == m_subResolutionPicklist->getCurrentPickStr())
    {
        infoPage->loadInfoPage(Multilang("Mandatory") + QString(": ") + Multilang(labelString[IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST].toUtf8().constData()));
        return false;
    }

    if (NULL == m_subFrameRatePicklist->getCurrentPickStr())
    {
        infoPage->loadInfoPage(Multilang("Mandatory") + QString(": ") + Multilang(labelString[IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST].toUtf8().constData()));
        return false;
    }

    return true;
}
