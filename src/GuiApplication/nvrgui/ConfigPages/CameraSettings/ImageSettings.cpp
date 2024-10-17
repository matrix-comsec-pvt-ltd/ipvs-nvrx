//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		ImageSettings.c
@brief      File containing the function defination of image settings GUI elements and set/get configuration
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ImageSettings.h"
#include "CameraSettings.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define GET_BIT(value, shift)               ((value >> shift) & 1)
#define IMAGE_SETTING_PARAM_BG_WIDTH        SCALE_WIDTH(850)
#define IMAGE_SETTING_PARAM_LABEL_WIDTH     SCALE_WIDTH(130)
#define IMAGE_SETTING_INTER_CTRL_MARGIN     SCALE_WIDTH(6)
#define IMAGE_SETTING_HEADER_LEFT_MARGIN    SCALE_WIDTH(330)
#define IMAGE_SETTING_ELEMENT_LEFT_MARGIN   SCALE_WIDTH(230)
#define IMAGE_SETTING_PARAM_IMAGE_PATH      ":/Images_Nvrx/AppearenceControl/"
#define IMAGE_SETTING_PARAM_BAR_IMAGE_PATH  IMAGE_SETTING_PARAM_IMAGE_PATH"bar.png"
#define HIDE_SHOW_UI_ELEMENT(element, sts)  if (element != NULL) element->setVisible(sts);

/* Get the best value for slider param */
#define GET_IMAGE_SETTING_VALUE(value, min, max)    ((value > max) ? max : ((value < min) ? min : value))

//#################################################################################################
// @DATA TYPES
//#################################################################################################
/* Elements used in UI creation */
typedef enum
{
    IMG_SET_CTRL_CAMERA_NAME_SPINBOX = 0,
    IMG_SET_CTRL_IMAGE_SETTINGS_SPINBOX,
    IMG_SET_CTRL_BRIGHTNESS_SLIDER,
    IMG_SET_CTRL_BRIGHTNESS_TEXTBOX,
    IMG_SET_CTRL_CONTRAST_SLIDER,
    IMG_SET_CTRL_CONTRAST_TEXTBOX,
    IMG_SET_CTRL_SATURATION_SLIDER,
    IMG_SET_CTRL_SATURATION_TEXTBOX,
    IMG_SET_CTRL_HUE_SLIDER,
    IMG_SET_CTRL_HUE_TEXTBOX,
    IMG_SET_CTRL_SHARPNESS_SLIDER,
    IMG_SET_CTRL_SHARPNESS_TEXTBOX,
    IMG_SET_CTRL_WHITE_BALANCE_SPINBOX,
    IMG_SET_CTRL_WDR_SPINBOX,
    IMG_SET_CTRL_EXPOSURE_RATIO_MODE_SPINBOX,
    IMG_SET_CTRL_BACKLIGHT_SPINBOX,
    IMG_SET_CTRL_EXPOSURE_RATIO_SLIDER,
    IMG_SET_CTRL_EXPOSURE_RATIO_TEXTBOX,
    IMG_SET_CTRL_WDR_STRENGTH_SLIDER,
    IMG_SET_CTRL_WDR_STRENGTH_TEXTBOX,
    IMG_SET_CTRL_EXPOSURE_MODE_SPINBOX,
    IMG_SET_CTRL_FLICKER_SPINBOX,
    IMG_SET_CTRL_FLICKER_STRENGTH_SLIDER,
    IMG_SET_CTRL_FLICKER_STRENGTH_TEXTBOX,
    IMG_SET_CTRL_HLC_SPINBOX,
    IMG_SET_CTRL_EXPOSURE_TIME_TEXTBOX,
    IMG_SET_CTRL_EXPOSURE_GAIN_SLIDER,
    IMG_SET_CTRL_EXPOSURE_GAIN_TEXTBOX,
    IMG_SET_CTRL_EXPOSURE_IRIS_SLIDER,
    IMG_SET_CTRL_EXPOSURE_IRIS_TEXTBOX,
    IMG_SET_CTRL_NORMAL_LIGHT_GAIN_SPINBOX,
    IMG_SET_CTRL_NORMAL_LIGHT_LUMINANCE_SLIDER,
    IMG_SET_CTRL_NORMAL_LIGHT_LUMINANCE_TEXTBOX,
    IMG_SET_CTRL_LED_MODE_SPINBOX,
    IMG_SET_CTRL_LED_SENSITIVITY_SLIDER,
    IMG_SET_CTRL_LED_SENSITIVITY_TEXTBOX,
    IMG_SET_CTRL_COPY_TO_CAM,
    IMG_SET_CTRL_MAX
}IMG_SET_CTRL_e;

typedef enum
{
    UI_LABEL_CAMERA_NAME = 0,
    UI_LABEL_IMAGE_SETTINGS_TYPE,
    UI_LABEL_BRIGHTNESS,
    UI_LABEL_CONTRAST,
    UI_LABEL_SATURATION,
    UI_LABEL_HUE,
    UI_LABEL_SHARPNESS,
    UI_LABEL_WHITE_BALANCE,
    UI_LABEL_WDR,
    UI_LABEL_EXPOSURE_RATIO_MODE,
    UI_LABEL_BACKLIGHT,
    UI_LABEL_EXPOSURE_RATIO,
    UI_LABEL_WDR_STRENGTH,
    UI_LABEL_DWDR_STRENGTH,
    UI_LABEL_EXPOSURE_MODE,
    UI_LABEL_FLICKER,
    UI_LABEL_FLICKER_STRENGTH,
    UI_LABEL_HLC,
    UI_LABEL_EXPOSURE_TIME,
    UI_LABEL_EXPOSURE_GAIN,
    UI_LABEL_EXPOSURE_IRIS,
    UI_LABEL_NORMAL_GAIN,
    UI_LABEL_NORMAL_LUMINANCE,
    UI_LABEL_LED_MODE,
    UI_LABEL_LED_SENSITIVITY,
    UI_LABEL_COPY_TO_CAMERA,
    UI_LABEL_MAX
}UI_LABEL_e;

typedef enum
{
    IMAGE_SETTING_CNFG_BRIGHTNESS = 0,
    IMAGE_SETTING_CNFG_CONTRAST,
    IMAGE_SETTING_CNFG_SATURATION,
    IMAGE_SETTING_CNFG_HUE,
    IMAGE_SETTING_CNFG_SHARPNESS,
    IMAGE_SETTING_CNFG_WHITE_BALANCE,
    IMAGE_SETTING_CNFG_WDR,
    IMAGE_SETTING_CNFG_WDR_STRENGTH,
    IMAGE_SETTING_CNFG_BACKLIGHT,
    IMAGE_SETTING_CNFG_EXPOSURE_RATIO_MODE,
    IMAGE_SETTING_CNFG_EXPOSURE_RATIO,
    IMAGE_SETTING_CNFG_EXPOSURE_MODE,
    IMAGE_SETTING_CNFG_FLICKER,
    IMAGE_SETTING_CNFG_FLICKER_STRENGTH,
    IMAGE_SETTING_CNFG_HLC,
    IMAGE_SETTING_CNFG_EXPOSURE_TIME,
    IMAGE_SETTING_CNFG_EXPOSURE_GAIN,
    IMAGE_SETTING_CNFG_EXPOSURE_IRIS,
    IMAGE_SETTING_CNFG_NORMAL_LIGHT_GAIN,
    IMAGE_SETTING_CNFG_NORMAL_LIGHT_LUMINANCE,
    IMAGE_SETTING_CNFG_LED_MODE,
    IMAGE_SETTING_CNFG_LED_SENSITIVITY,
    IMAGE_SETTING_CNFG_COPY_TO_CAM_START,
    IMAGE_SETTING_CNFG_COPY_TO_CAM_END = IMAGE_SETTING_CNFG_COPY_TO_CAM_START + CAMERA_MASK_MAX - 1,
    IMAGE_SETTING_CNFG_MAX
}IMAGE_SETTING_CNFG_e;

typedef enum
{
    IMAGE_SETTING_TYPE_BASIC = 0,
    IMAGE_SETTING_TYPE_ADVANCE,
    IMAGE_SETTING_TYPE_MAX,
}IMAGE_SETTING_TYPE_e;

typedef enum
{
    BRIGHTNESS_MIN = 0,
    BRIGHTNESS_MAX = 100,

    CONTRAST_MIN = 0,
    CONTRAST_MAX = 100,

    SATURATION_MIN = 0,
    SATURATION_MAX = 100,

    HUE_MIN = 0,
    HUE_MAX = 100,

    SHARPNESS_MIN = 0,
    SHARPNESS_MAX = 100,

    WDR_STRENGTH_MIN = 1,
    WDR_STRENGTH_MAX = 100,

    EXPOSURE_RATIO_MIN = 2,
    EXPOSURE_RATIO_MAX = 64,

    FLICKER_STRENGTH_MIN = 0,
    FLICKER_STRENGTH_MAX = 10,

    EXPOSURE_TIME_MIN = 10,
    EXPOSURE_TIME_MAX = 66666,

    EXPOSURE_GAIN_MIN = 0,
    EXPOSURE_GAIN_MAX = 100,

    EXPOSURE_IRIS_MIN = 0,
    EXPOSURE_IRIS_MAX = 4,

    NORMAL_LIGHT_LUMINANCE_MIN = 1,
    NORMAL_LIGHT_LUMINANCE_MAX = 7,

    LED_SENSITIVITY_MIN = 0,
    LED_SENSITIVITY_MAX = 15,

}IMAGE_SETTING_PARAM_RANGE_e;

typedef enum
{
    WHITE_BALANCE_MODE_AUTO = 0,
    WHITE_BALANCE_MODE_FLUORESCENT,
    WHITE_BALANCE_MODE_INCANDESCENT,
    WHITE_BALANCE_MODE_SUNNY,
    WHITE_BALANCE_MODE_MAX
}WHITE_BALANCE_MODE_e;

typedef enum
{
    WDR_MODE_OFF = 0,
    WDR_MODE_MANUAL,
    WDR_MODE_AUTO,
    WDR_MODE_MAX
}WDR_MODE_e;

typedef enum
{
    BACKLIGHT_MODE_OFF = 0,
    BACKLIGHT_MODE_BLC,
    BACKLIGHT_MODE_DWDR,
    BACKLIGHT_MODE_MAX
}BACKLIGHT_MODE_e;

typedef enum
{
    EXPOSURE_RATIO_MODE_AUTO = 0,
    EXPOSURE_RATIO_MODE_MANUAL,
    EXPOSURE_RATIO_MODE_MAX
}EXPOSURE_RATIO_MODE_e;

typedef enum
{
    EXPOSURE_MODE_AUTO = 0,
    EXPOSURE_MODE_MANUAL,
    EXPOSURE_MODE_MAX
}EXPOSURE_MODE_e;

typedef enum
{
    FLICKER_MODE_NONE = 0,
    FLICKER_MODE_50HZ,
    FLICKER_MODE_60HZ,
    FLICKER_MODE_AUTO,
    FLICKER_MODE_MAX
}FLICKER_MODE_e;

typedef enum
{
    HLC_MODE_OFF = 0,
    HLC_MODE_HIGH,
    HLC_MODE_MEDIUM,
    HLC_MODE_LOW,
    HLC_MODE_MAX
}HLC_MODE_e;

typedef enum
{
    NORMAL_LIGHT_GAIN_16X = 0,
    NORMAL_LIGHT_GAIN_32X,
    NORMAL_LIGHT_GAIN_48X,
    NORMAL_LIGHT_GAIN_64X,
    NORMAL_LIGHT_GAIN_96X,
    NORMAL_LIGHT_GAIN_128X,
    NORMAL_LIGHT_GAIN_MAX
}NORMAL_LIGHT_GAIN_e;

typedef enum
{
    LED_MODE_AUTO = 0,
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_MAX
}LED_MODE_e;

typedef enum
{
    UI_ELE_TYPE_SLIDER = 0,
    UI_ELE_TYPE_DROPDOWN,
    UI_ELE_TYPE_TEXTBOX,
    UI_ELE_TYPE_MAX,
}UI_ELE_TYPE_e;

typedef struct
{
    UI_ELE_TYPE_e   elementType;
    quint32         elementIdx;
    quint32         controlIdx;
    quint32         uiElementIdx;
    quint32         eleMinRange;
    quint32         eleMaxRange;
}IMAGE_SETTINGS_ELEMENT_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const IMAGE_SETTINGS_ELEMENT_INFO_t imageSettingsElementInfo[IMAGE_SETTING_CNFG_COPY_TO_CAM_START] =
{
    // IMAGE_SETTING_CNFG_BRIGHTNESS
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_BRIGHTNESS,                  IMG_SET_CTRL_BRIGHTNESS_SLIDER,
     UI_LABEL_BRIGHTNESS,           BRIGHTNESS_MIN,                             BRIGHTNESS_MAX},

    // IMAGE_SETTING_CNFG_CONTRAST
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_CONTRAST,                    IMG_SET_CTRL_CONTRAST_SLIDER,
     UI_LABEL_CONTRAST,             CONTRAST_MIN,                               CONTRAST_MAX},

    // IMAGE_SETTING_CNFG_SATURATION
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_SATURATION,                  IMG_SET_CTRL_SATURATION_SLIDER,
     UI_LABEL_SATURATION,           SATURATION_MIN,                             SATURATION_MAX},

    // IMAGE_SETTING_CNFG_HUE
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_HUE,                         IMG_SET_CTRL_HUE_SLIDER,
     UI_LABEL_HUE,                  HUE_MIN,                                    HUE_MAX},

    // IMAGE_SETTING_CNFG_SHARPNESS
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_SHARPNESS,                   IMG_SET_CTRL_SHARPNESS_SLIDER,
     UI_LABEL_SHARPNESS,            SHARPNESS_MIN,                              SHARPNESS_MAX},

    // IMAGE_SETTING_CNFG_WHITE_BALANCE
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_WHITE_BALANCE,             IMG_SET_CTRL_WHITE_BALANCE_SPINBOX,
     UI_LABEL_WHITE_BALANCE,        WHITE_BALANCE_MODE_AUTO,                    WHITE_BALANCE_MODE_MAX},

    // IMAGE_SETTING_CNFG_WDR
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_WDR_MODE,                  IMG_SET_CTRL_WDR_SPINBOX,
     UI_LABEL_WDR,                  WDR_MODE_OFF,                               WDR_MODE_MAX},

    // IMAGE_SETTING_CNFG_WDR_STRENGTH
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_WDR_STRENGTH,                IMG_SET_CTRL_WDR_STRENGTH_SLIDER,
     UI_LABEL_WDR_STRENGTH,         WDR_STRENGTH_MIN,                           WDR_STRENGTH_MAX},

    // IMAGE_SETTING_CNFG_BACKLIGHT
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_BACKLIGHT_MODE,            IMG_SET_CTRL_BACKLIGHT_SPINBOX,
     UI_LABEL_BACKLIGHT,            BACKLIGHT_MODE_OFF,                         BACKLIGHT_MODE_MAX},

    // IMAGE_SETTING_CNFG_EXPOSURE_RATIO_MODE
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_EXPOSURE_RATIO_MODE,       IMG_SET_CTRL_EXPOSURE_RATIO_MODE_SPINBOX,
     UI_LABEL_EXPOSURE_RATIO_MODE,  EXPOSURE_RATIO_MODE_AUTO,                   EXPOSURE_RATIO_MODE_MAX},

    // IMAGE_SETTING_CNFG_EXPOSURE_RATIO
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_EXPOSURE_RATIO,              IMG_SET_CTRL_EXPOSURE_RATIO_SLIDER,
     UI_LABEL_EXPOSURE_RATIO,       EXPOSURE_RATIO_MIN,                         EXPOSURE_RATIO_MAX},

    // IMAGE_SETTING_CNFG_EXPOSURE_MODE
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_EXPOSURE_MODE,             IMG_SET_CTRL_EXPOSURE_MODE_SPINBOX,
     UI_LABEL_EXPOSURE_MODE,        EXPOSURE_MODE_AUTO,                         EXPOSURE_MODE_MAX},

    // IMAGE_SETTING_CNFG_FLICKER
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_FLICKER_MODE,              IMG_SET_CTRL_FLICKER_SPINBOX,
     UI_LABEL_FLICKER,              FLICKER_MODE_NONE,                          FLICKER_MODE_MAX},

    // IMAGE_SETTING_CNFG_FLICKER_STRENGTH
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_FLICKER_STRENGTH,            IMG_SET_CTRL_FLICKER_STRENGTH_SLIDER,
     UI_LABEL_FLICKER_STRENGTH,     FLICKER_STRENGTH_MIN,                       FLICKER_STRENGTH_MAX},

    // IMAGE_SETTING_CNFG_HLC
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_HLC_MODE,                  IMG_SET_CTRL_HLC_SPINBOX,
     UI_LABEL_HLC,                  HLC_MODE_OFF,                               HLC_MODE_MAX},

    // IMAGE_SETTING_CNFG_EXPOSURE_TIME
    {UI_ELE_TYPE_TEXTBOX,           IMG_SET_TEXTBOX_EXPOSURE_TIME,              IMG_SET_CTRL_EXPOSURE_TIME_TEXTBOX,
     UI_LABEL_EXPOSURE_TIME,        EXPOSURE_TIME_MIN,                          EXPOSURE_TIME_MAX},

    // IMAGE_SETTING_CNFG_EXPOSURE_GAIN
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_EXPOSURE_GAIN,               IMG_SET_CTRL_EXPOSURE_GAIN_SLIDER,
     UI_LABEL_EXPOSURE_GAIN,        EXPOSURE_GAIN_MIN,                          EXPOSURE_GAIN_MAX},

    // IMAGE_SETTING_CNFG_EXPOSURE_IRIS
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_EXPOSURE_IRIS,               IMG_SET_CTRL_EXPOSURE_IRIS_SLIDER,
     UI_LABEL_EXPOSURE_IRIS,        EXPOSURE_IRIS_MIN,                          EXPOSURE_IRIS_MAX},

    // IMAGE_SETTING_CNFG_NORMAL_LIGHT_GAIN
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_NORMAL_LIGHT_GAIN,         IMG_SET_CTRL_NORMAL_LIGHT_GAIN_SPINBOX,
     UI_LABEL_NORMAL_GAIN,          NORMAL_LIGHT_GAIN_16X,                      NORMAL_LIGHT_GAIN_MAX},

    // IMAGE_SETTING_CNFG_NORMAL_LIGHT_LUMINANCE
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_NORMAL_LIGHT_LUMINANCE,      IMG_SET_CTRL_NORMAL_LIGHT_LUMINANCE_SLIDER,
     UI_LABEL_NORMAL_LUMINANCE,     NORMAL_LIGHT_LUMINANCE_MIN,                 NORMAL_LIGHT_LUMINANCE_MAX},

    // IMAGE_SETTING_CNFG_LED_MODE
    {UI_ELE_TYPE_DROPDOWN,          IMG_SET_DROPDOWN_LED_MODE,                  IMG_SET_CTRL_LED_MODE_SPINBOX,
     UI_LABEL_LED_MODE,             LED_MODE_AUTO,                              LED_MODE_MAX},

    // IMAGE_SETTING_CNFG_LED_SENSITIVITY
    {UI_ELE_TYPE_SLIDER,            IMG_SET_SLIDER_LED_SENSITIVITY,             IMG_SET_CTRL_LED_SENSITIVITY_SLIDER,
     UI_LABEL_LED_SENSITIVITY,      LED_SENSITIVITY_MIN,                        LED_SENSITIVITY_MAX},
};

static const QString imageSettingsLabelStr[UI_LABEL_MAX] =
{
    "Camera",
    "Type",
    "Brightness",
    "Contrast",
    "Saturation",
    "Hue",
    "Sharpness",
    "White Balance",
    "Wide Dynamic Range",
    "Exposure Ratio Mode",
    "Backlight Control",
    "Exposure Ratio",
    "WDR Strength",
    "D-WDR Strength",
    "Exposure Mode",
    "Flicker",
    "Strength",
    "HLC",
    "Time",
    "Gain",
    "Iris",
    "Max Gain",
    "Average Luminance",
    "LED Modes",
    "LED Sensitivity",
    "Copy to Camera",
};

static const QString imageSettingsTypeStr[IMAGE_SETTING_TYPE_MAX] = {"Basic", "Advanced"};

static const QString whitebalanceModeStr[2][WHITE_BALANCE_MODE_MAX] =
{
    {"Auto", "Fluorescent", "Incandescent", "Sunny"},   /* BM & CI */
    {"Auto", "Manual",      "",             ""}         /* ONVIF */
};

static const QString wdrModeStr[2][WDR_MODE_MAX] =
{
    {"Off", "Manual", "Auto"},  /* BM & CI */
    {"Off", "On",     ""},      /* ONVIF */
};

static const QString backLightModeStr[2][BACKLIGHT_MODE_MAX] =
{
    {"None", "BLC", "D-WDR"},   /* BM & CI */
    {"Off",  "On",  ""},        /* ONVIF */
};

static const QString exposureRatioModeStr[EXPOSURE_RATIO_MODE_MAX] = {"Auto", "Manual"};

static const QString exposureModeStr[EXPOSURE_MODE_MAX] = {"Auto", "Manual"};

static const QString flickerModeStr[FLICKER_MODE_MAX] = {"None", "50 Hz", "60 Hz", "Auto"};

static const QString hlcModeStr[HLC_MODE_MAX] = {"Off", "High", "Medium", "Low"};

static const QString normalLightGainModeStr[NORMAL_LIGHT_GAIN_MAX] = {"16x", "32x", "48x", "64x", "96x", "128x"};

static const QString ledModeStr[LED_MODE_MAX] = {"Auto", "Always Off", "Always On"};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
ImageSettings::ImageSettings(QString deviceName, QWidget *parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent, IMG_SET_CTRL_MAX, devTabInfo, CNFG_TYPE_DFLT_REF_SAV_BTN),
      isOnvifSupportEnabled(false), currentCameraIndex(1), imagingCapability(0)
{
    memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
    isImageSettingsComponentCreated = false;

    INIT_OBJ(imageSettingsTypeDropDown.box);
    INIT_OBJ(errorMsg);
    INIT_OBJ(copytoCamera);

    for(quint8 sliderIdx = 0; sliderIdx < IMG_SET_SLIDER_MAX; sliderIdx++)
    {
        INIT_OBJ(sliderParam[sliderIdx].sliderBar);
        INIT_OBJ(sliderParam[sliderIdx].backgroundImage);
        INIT_OBJ(sliderParam[sliderIdx].textboxValue);
        INIT_OBJ(sliderParam[sliderIdx].textboxParam);
        INIT_OBJ(sliderParam[sliderIdx].parameterLabel);
        sliderParam[sliderIdx].multipler = 0.0f;
    }

    for(quint8 dropdownIdx = 0; dropdownIdx < IMG_SET_DROPDOWN_MAX; dropdownIdx++)
    {
        dropdownParam[dropdownIdx].list.clear();
        INIT_OBJ(dropdownParam[dropdownIdx].box);
    }

    for(quint8 textboxIdx = 0; textboxIdx < IMG_SET_TEXTBOX_MAX; textboxIdx++)
    {
        INIT_OBJ(textboxParam[textboxIdx].param);
        INIT_OBJ(textboxParam[textboxIdx].box);
    }

    createDefaultComponent();
    fillCameraList();
    getCameraSupportedImageSettingsParameters();
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::createDefaultComponent(void)
{
    /* Camera List Label */
    cameraNameDropDown.list.clear();

    cameraNameDropDown.box = new DropDown((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - IMAGE_SETTING_PARAM_BG_WIDTH)/2 - SCALE_WIDTH(31),
                                          SCALE_HEIGHT(50), IMAGE_SETTING_PARAM_BG_WIDTH, BGTILE_HEIGHT,
                                          IMG_SET_CTRL_CAMERA_NAME_SPINBOX, DROPDOWNBOX_SIZE_320,
                                          imageSettingsLabelStr[UI_LABEL_CAMERA_NAME], cameraNameDropDown.list, this, "", true, 0, COMMON_LAYER,
                                          true, 8, false, false, 5, IMAGE_SETTING_HEADER_LEFT_MARGIN);
    m_elementList[IMG_SET_CTRL_CAMERA_NAME_SPINBOX] = cameraNameDropDown.box;
    connect(cameraNameDropDown.box,
            SIGNAL(sigValueChanged(QString, quint32)),
            this,
            SLOT(slotSpinboxValueChange(QString, quint32)));
    connect(cameraNameDropDown.box,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    /* Copy to camera button */
    copyToCameraBtn = new PageOpenButton((cameraNameDropDown.box->x() + cameraNameDropDown.box->width()) - SCALE_WIDTH(210),
                                          cameraNameDropDown.box->y() + SCALE_HEIGHT(5),
                                          SCALE_WIDTH(100), SCALE_HEIGHT(30),
                                          IMG_SET_CTRL_COPY_TO_CAM, PAGEOPENBUTTON_EXTRALARGE,
                                          imageSettingsLabelStr[UI_LABEL_COPY_TO_CAMERA], this, "", "", false, 0, NO_LAYER);
    m_elementList[IMG_SET_CTRL_COPY_TO_CAM] = copyToCameraBtn;
    copyToCameraBtn->setIsEnabled(true);
    connect(copyToCameraBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotPageOpenBtnClick(int)));
    connect(copyToCameraBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
}

//-------------------------------------------------------------------------------------------------
ImageSettings::~ImageSettings()
{
    cameraNameDropDown.list.clear();
    if(IS_VALID_OBJ(cameraNameDropDown.box))
    {
        disconnect(cameraNameDropDown.box,
                   SIGNAL(sigValueChanged(QString, quint32)),
                   this,
                   SLOT(slotSpinboxValueChange(QString, quint32)));
        disconnect(cameraNameDropDown.box,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(cameraNameDropDown.box);
    }

    if(IS_VALID_OBJ(copyToCameraBtn))
    {
        disconnect(copyToCameraBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotPageOpenBtnClick(int)));
        disconnect(copyToCameraBtn,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(copyToCameraBtn);
    }

    if (IS_VALID_OBJ(copytoCamera))
    {
        disconnect(copytoCamera,
                   SIGNAL(sigDeleteObject(quint8)),
                   this,
                   SLOT(slotCopytoCamDelete(quint8)));
        DELETE_OBJ(copytoCamera);
    }

    DELETE_OBJ(errorMsg);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::slotPageOpenBtnClick(int index)
{
    /* Check Copy to Camera button clicked */
    if ((index != IMG_SET_CTRL_COPY_TO_CAM) || (IS_VALID_OBJ(copytoCamera)))
    {
        return;
    }

    /* Remove self bit from copy to camera mask */
    memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
    SET_CAMERA_MASK_BIT(copyToCameraFields, currentCameraIndex - 1);
    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        m_configResponse[IMAGE_SETTING_CNFG_COPY_TO_CAM_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
    }

    copytoCamera = new CopyToCamera(cameraNameDropDown.list,
                                    copyToCameraFields,
                                    parentWidget(),
                                    "Image Settings",
                                    IMG_SET_CTRL_COPY_TO_CAM);
    connect(copytoCamera,
            SIGNAL(sigDeleteObject(quint8)),
            this,
            SLOT(slotCopytoCamDelete(quint8)));
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::slotCopytoCamDelete(quint8)
{
    /* Delete copy to camera if exist */
    if (IS_VALID_OBJ(copytoCamera))
    {
        disconnect(copytoCamera,
                   SIGNAL(sigDeleteObject(quint8)),
                   this,
                   SLOT(slotCopytoCamDelete(quint8)));
        DELETE_OBJ(copytoCamera);
    }

    if (IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::getCameraSupportedImageSettingsParameters(void)
{
    /* Delete previous image settings param */
    deleteAllImageSettingsParameters();

    // Camera index
    payloadLib->setCnfgArrayAtIndex(0, cameraNameDropDown.box->getIndexofCurrElement()+1);
    payloadLib->setCnfgArrayAtIndex(1, CAPABILITY_CMD_ID_IMAGING_CAPABILITY);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_CAPABILITY;
    param->payload = payloadLib->createDevCmdPayload(2);

    if(processBar->isLoadedProcessBar() == false)
    {
        processBar->loadProcessBar();
    }

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::createAllImageSettingsParameters(void)
{
    imageSettingsTypeDropDown.list.clear();
    for (quint32 listCnt = 0; listCnt < IMAGE_SETTING_TYPE_MAX; listCnt++)
    {
        imageSettingsTypeDropDown.list.insert(listCnt, imageSettingsTypeStr[listCnt]);
    }

    imageSettingsTypeDropDown.box = new DropDown(cameraNameDropDown.box->x(),
                                                 (cameraNameDropDown.box->y() + cameraNameDropDown.box->height() + SCALE_HEIGHT(5)),
                                                 IMAGE_SETTING_PARAM_BG_WIDTH, BGTILE_HEIGHT,
                                                 IMG_SET_CTRL_IMAGE_SETTINGS_SPINBOX, DROPDOWNBOX_SIZE_225,
                                                 imageSettingsLabelStr[UI_LABEL_IMAGE_SETTINGS_TYPE], imageSettingsTypeDropDown.list, this, "",
                                                 true, 0, COMMON_LAYER, true, 8, false, false, 5, IMAGE_SETTING_HEADER_LEFT_MARGIN);
    m_elementList[IMG_SET_CTRL_IMAGE_SETTINGS_SPINBOX] = imageSettingsTypeDropDown.box;
    connect(imageSettingsTypeDropDown.box,
            SIGNAL(sigValueChanged(QString, quint32)),
            this,
            SLOT(slotSpinboxValueChange(QString, quint32)));
    connect(imageSettingsTypeDropDown.box,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    createImageSettingsParameters(UI_LABEL_BRIGHTNESS);
    updateImageSettingType(imageSettingsTypeStr[IMAGE_SETTING_TYPE_BASIC]);
    setErrormsg(false);
    isImageSettingsComponentCreated = true;
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::recreateImageSettingsParameters(quint32 startUiEleIdx)
{
    deleteImageSettingsParameters(startUiEleIdx);
    createImageSettingsParameters(startUiEleIdx);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::createImageSettingsParameters(quint32 startUiEleIdx)
{
    quint32 cnfgIdx, eleIdx, listCnt;

    /* Define start cordinate[x and y position] */
    quint32 startX = imageSettingsTypeDropDown.box->x();
    quint32 startY = (imageSettingsTypeDropDown.box->y() + imageSettingsTypeDropDown.box->height() + SCALE_HEIGHT(5));

    /* Configure for camera Supported Parameter settings: Brightness, Contrast, Saturation, Hue, Sharpness */
    for(cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_WHITE_BALANCE; cnfgIdx++)
    {
        /* Current param is not supported by camera */
        if (GET_BIT(imagingCapability, cnfgIdx) == 0)
        {
            /* Current param is not supported by camera */
            continue;
        }

        if (startUiEleIdx <= imageSettingsElementInfo[cnfgIdx].uiElementIdx)
        {
            /* Create the slider element */
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[cnfgIdx + UI_LABEL_BRIGHTNESS]);
            startX = imageSettingsTypeDropDown.box->x();
            startY = startY + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- WHITE BALANCE ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_WHITE_BALANCE;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_WHITE_BALANCE) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        dropdownParam[eleIdx].list.clear();
        for (listCnt = 0; listCnt < WHITE_BALANCE_MODE_MAX; listCnt++)
        {
            dropdownParam[eleIdx].list.insert(listCnt, whitebalanceModeStr[isOnvifSupportEnabled][listCnt]);
        }

        createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_WHITE_BALANCE]);
        startX = imageSettingsTypeDropDown.box->x();
        startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
    }

    /* Reset X and Y cordinates for advance parameters */
    startX = imageSettingsTypeDropDown.box->x();
    startY = (imageSettingsTypeDropDown.box->y() + imageSettingsTypeDropDown.box->height() + SCALE_HEIGHT(5));

    /* ----------------------- WDR ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_WDR;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_WDR) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        dropdownParam[eleIdx].list.clear();
        for(listCnt = 0; listCnt < WDR_MODE_MAX; listCnt++)
        {
            dropdownParam[eleIdx].list.insert(listCnt, wdrModeStr[isOnvifSupportEnabled][listCnt]);
        }

        createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_WDR]);
        startY = startY + BGTILE_HEIGHT;
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Exposure Ratio Mode ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_EXPOSURE_RATIO_MODE;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_EXPOSURE_RATIO_MODE) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((false == isOnvifSupportEnabled) && (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->getCurrValue() == wdrModeStr[isOnvifSupportEnabled][WDR_MODE_MANUAL]))
        {
            dropdownParam[eleIdx].list.clear();
            for(listCnt = 0; listCnt < EXPOSURE_RATIO_MODE_MAX; listCnt++)
            {
                dropdownParam[eleIdx].list.insert(listCnt, exposureRatioModeStr[listCnt]);
            }

            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_EXPOSURE_RATIO_MODE]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Backlight ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_BACKLIGHT;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_BACKLIGHT) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->getCurrValue() == wdrModeStr[isOnvifSupportEnabled][WDR_MODE_OFF]))
        {
            dropdownParam[eleIdx].list.clear();
            for(listCnt = 0; listCnt < BACKLIGHT_MODE_MAX; listCnt++)
            {
                dropdownParam[eleIdx].list.insert(listCnt, backLightModeStr[isOnvifSupportEnabled][listCnt]);
            }

            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_BACKLIGHT]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Exposure Ratio ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_EXPOSURE_RATIO;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_EXPOSURE_RATIO) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_RATIO_MODE].box)
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_RATIO_MODE].box->getCurrValue() == exposureRatioModeStr[EXPOSURE_RATIO_MODE_MANUAL]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_EXPOSURE_RATIO]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            startY = sliderParam[eleIdx].textboxValue->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- WDR/D-WDR Strength ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_WDR_STRENGTH;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_WDR_STRENGTH) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box)
                && (dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->getCurrValue() != wdrModeStr[isOnvifSupportEnabled][WDR_MODE_OFF]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_WDR_STRENGTH]);
            startY = startY + BGTILE_HEIGHT;
        }
        else if ((false == isOnvifSupportEnabled) && (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_BACKLIGHT_MODE].box))
                 && (dropdownParam[IMG_SET_DROPDOWN_BACKLIGHT_MODE].box->getCurrValue() == backLightModeStr[isOnvifSupportEnabled][BACKLIGHT_MODE_DWDR]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_DWDR_STRENGTH]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            startY = sliderParam[eleIdx].textboxValue->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Exposure Mode ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_EXPOSURE_MODE;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_EXPOSURE_MODE) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((false == isOnvifSupportEnabled) && (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box)))
        {
            dropdownParam[eleIdx].list.clear();
            for(listCnt = 0; listCnt < EXPOSURE_MODE_MAX; listCnt++)
            {
                dropdownParam[eleIdx].list.insert(listCnt, exposureModeStr[listCnt]);
            }

            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_EXPOSURE_MODE]);
            startY = startY + BGTILE_HEIGHT;

            if (dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->getCurrValue() == wdrModeStr[isOnvifSupportEnabled][WDR_MODE_AUTO])
            {
                dropdownParam[eleIdx].box->setIndexofCurrElement(EXPOSURE_MODE_AUTO);
                dropdownParam[eleIdx].box->setIsEnabled(false);
            }
        }
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Flicker Mode ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_FLICKER;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_FLICKER) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_AUTO]))
        {
            dropdownParam[eleIdx].list.clear();
            for(listCnt = 0; listCnt < FLICKER_MODE_MAX; listCnt++)
            {
                dropdownParam[eleIdx].list.insert(listCnt, flickerModeStr[listCnt]);
            }

            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_FLICKER]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Flicker Strength ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_FLICKER_STRENGTH;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_FLICKER_STRENGTH) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_FLICKER_MODE].box)
                && ((dropdownParam[IMG_SET_DROPDOWN_FLICKER_MODE].box->getCurrValue() == flickerModeStr[FLICKER_MODE_50HZ])
                    || (dropdownParam[IMG_SET_DROPDOWN_FLICKER_MODE].box->getCurrValue() == flickerModeStr[FLICKER_MODE_60HZ])))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_FLICKER_STRENGTH]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            startY = sliderParam[eleIdx].textboxValue->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- HLC Mode ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_HLC;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_HLC) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->getCurrValue() == wdrModeStr[isOnvifSupportEnabled][WDR_MODE_OFF])
                && (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_AUTO]))
        {
            dropdownParam[eleIdx].list.clear();
            for(listCnt = 0; listCnt < HLC_MODE_MAX; listCnt++)
            {
                dropdownParam[eleIdx].list.insert(listCnt, hlcModeStr[listCnt]);
            }

            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_HLC]);
            startY = startY + BGTILE_HEIGHT;
            slotSpinboxValueChange(hlcModeStr[m_configResponse[cnfgIdx].toInt()], IMG_SET_CTRL_HLC_SPINBOX);
        }
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    if ((false == IS_VALID_OBJ(dropdownParam[eleIdx].box)) && (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box)))
    {
        dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->setIsEnabled(true);
    }

    /* ----------------------- Exposure Time ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_EXPOSURE_TIME;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_EXPOSURE_TIME) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box)
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_MANUAL]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_EXPOSURE_TIME]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(textboxParam[eleIdx].box))
        {
            startY = textboxParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Exposure Gain ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_EXPOSURE_GAIN;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_EXPOSURE_GAIN) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box)
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_MANUAL]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_EXPOSURE_GAIN]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            startY = sliderParam[eleIdx].textboxValue->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Exposure Iris ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_EXPOSURE_IRIS;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_EXPOSURE_IRIS) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box)
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_MANUAL]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_EXPOSURE_IRIS]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            startY = sliderParam[eleIdx].textboxValue->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Normal Light Max Gain Mode ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_NORMAL_LIGHT_GAIN;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_NORMAL_GAIN) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_AUTO]))
        {
            dropdownParam[eleIdx].list.clear();
            for(listCnt = 0; listCnt < NORMAL_LIGHT_GAIN_MAX; listCnt++)
            {
                dropdownParam[eleIdx].list.insert(listCnt, normalLightGainModeStr[listCnt]);
            }

            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_NORMAL_GAIN]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- Normal Light Average Luminance ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_NORMAL_LIGHT_LUMINANCE;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_NORMAL_LUMINANCE) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if ((IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box))
                && (dropdownParam[IMG_SET_DROPDOWN_EXPOSURE_MODE].box->getCurrValue() == exposureModeStr[EXPOSURE_MODE_AUTO]))
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_NORMAL_LUMINANCE]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
    else
    {
        if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            startY = sliderParam[eleIdx].textboxValue->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- LED Mode ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_LED_MODE;
    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    if ((startUiEleIdx <= UI_LABEL_LED_MODE) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        dropdownParam[eleIdx].list.clear();
        for(listCnt = 0; listCnt < LED_MODE_MAX; listCnt++)
        {
            dropdownParam[eleIdx].list.insert(listCnt, ledModeStr[listCnt]);
        }

        createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_LED_MODE]);
        startY = startY + BGTILE_HEIGHT;
    }
    else
    {
        if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            startY = dropdownParam[eleIdx].box->y() + BGTILE_HEIGHT;
        }
    }

    /* ----------------------- LED Sensitivity Slider ----------------------- */
    cnfgIdx = IMAGE_SETTING_CNFG_LED_SENSITIVITY;
    if ((startUiEleIdx <= UI_LABEL_LED_SENSITIVITY) && (GET_BIT(imagingCapability, cnfgIdx)))
    {
        if (isOnvifSupportEnabled == false)
        {
            createSingleUiElement(cnfgIdx, startX, startY, imageSettingsLabelStr[UI_LABEL_LED_SENSITIVITY]);
            startY = startY + BGTILE_HEIGHT;
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::deleteAllImageSettingsParameters(void)
{
    isImageSettingsComponentCreated = false;
    isOnvifSupportEnabled = false;
    memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
    m_configResponse.clear();
    imagingCapability = 0;

    /* Destroy all the sliders */
    deleteImageSettingsParameters(UI_LABEL_BRIGHTNESS);

    /* Delete image Settings Type */
    imageSettingsTypeDropDown.list.clear();
    if(IS_VALID_OBJ(imageSettingsTypeDropDown.box))
    {
        disconnect(imageSettingsTypeDropDown.box,
                   SIGNAL(sigValueChanged(QString, quint32)),
                   this,
                   SLOT(slotSpinboxValueChange(QString, quint32)));
        disconnect(imageSettingsTypeDropDown.box,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(imageSettingsTypeDropDown.box);
        m_elementList[IMG_SET_CTRL_IMAGE_SETTINGS_SPINBOX] = NULL;
    }

    setErrormsg(false);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::deleteImageSettingsParameters(quint32 startUiEleIdx)
{
    quint32 cnfgIdx;

    for(cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_COPY_TO_CAM_START; cnfgIdx++)
    {
        if (startUiEleIdx <= imageSettingsElementInfo[cnfgIdx].uiElementIdx)
        {
            deleteSingleUiElement(cnfgIdx);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::createSingleUiElement(quint32 cnfgIdx, quint32 startX, quint32 startY, QString uiLableStr)
{
    quint32 eleIdx, minRange, maxRange, controlIdx;

    if (cnfgIdx >= IMAGE_SETTING_CNFG_COPY_TO_CAM_START)
    {
        return;
    }

    eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    controlIdx = imageSettingsElementInfo[cnfgIdx].controlIdx;
    minRange = imageSettingsElementInfo[cnfgIdx].eleMinRange;
    maxRange = imageSettingsElementInfo[cnfgIdx].eleMaxRange;
    if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
    {
        sliderParam[eleIdx].textboxParam = new TextboxParam();
        sliderParam[eleIdx].textboxParam->suffixStr = QString("(%1-%2)").arg(minRange).arg(maxRange);
        sliderParam[eleIdx].textboxParam->minNumValue = minRange;
        sliderParam[eleIdx].textboxParam->maxNumValue = maxRange;
        sliderParam[eleIdx].textboxParam->maxChar = getAllowedCharCnt(maxRange);
        sliderParam[eleIdx].textboxParam->isNumEntry = true;
        sliderParam[eleIdx].textboxParam->isCentre = false;
        sliderParam[eleIdx].textboxParam->leftMargin = IMAGE_SETTING_PARAM_BG_WIDTH - IMAGE_SETTING_PARAM_LABEL_WIDTH - SCALE_WIDTH(10);
        sliderParam[eleIdx].textboxParam->textStr = m_configResponse[cnfgIdx].toString();

        sliderParam[eleIdx].textboxValue = new TextBox(startX, startY, IMAGE_SETTING_PARAM_BG_WIDTH, BGTILE_HEIGHT,
                                                       controlIdx+1, TEXTBOX_EXTRASMALL, this, sliderParam[eleIdx].textboxParam);
        m_elementList[controlIdx+1] = sliderParam[eleIdx].textboxValue;
        connect(sliderParam[eleIdx].textboxValue,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(sliderParam[eleIdx].textboxValue,
                SIGNAL(sigTextValueAppended(QString,int)),
                this,
                SLOT(slotSliderBoxValueChange(QString,int)));

        sliderParam[eleIdx].backgroundImage = new Image((sliderParam[eleIdx].textboxValue->x() + SCALE_WIDTH(60) + IMAGE_SETTING_PARAM_LABEL_WIDTH + IMAGE_SETTING_INTER_CTRL_MARGIN),
                                                        (sliderParam[eleIdx].textboxValue->y() + (BGTILE_HEIGHT / 2)),
                                                        IMAGE_SETTING_PARAM_BAR_IMAGE_PATH, this, START_X_CENTER_Y, 0, false, true);

        /* Slider multiplier array is used to set multipler value for slider bars, set multipler for slider bar based on width and minimum range and maximum range */
        sliderParam[eleIdx].multipler = (float)sliderParam[eleIdx].backgroundImage->width()/((float)(maxRange - minRange));

        sliderParam[eleIdx].parameterLabel = new TextLabel(sliderParam[eleIdx].backgroundImage->x() - SCALE_WIDTH(10),
                                                           (sliderParam[eleIdx].textboxValue->y() + (BGTILE_HEIGHT / 2)),
                                                           NORMAL_FONT_SIZE, uiLableStr, this,
                                                           NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_END_X_CENTRE_Y);

        quint32 configValue = ((quint32)m_configResponse[cnfgIdx].toInt() < minRange) ? 0 : (m_configResponse[cnfgIdx].toInt() - minRange);
        sliderParam[eleIdx].sliderBar = new SliderControl((sliderParam[eleIdx].backgroundImage->x() - SCALE_WIDTH(7)),
                                                          sliderParam[eleIdx].backgroundImage->y(),
                                                          (sliderParam[eleIdx].backgroundImage->width() + SCALE_WIDTH(14)),
                                                          sliderParam[eleIdx].backgroundImage->height(),
                                                          sliderParam[eleIdx].backgroundImage->width(),
                                                          sliderParam[eleIdx].textboxValue->height(),
                                                          IMAGE_SETTING_PARAM_IMAGE_PATH,
                                                          (configValue*sliderParam[eleIdx].multipler),
                                                          this, HORIZONTAL_SLIDER, controlIdx, true, false, true, true);
        m_elementList[controlIdx] = sliderParam[eleIdx].sliderBar;
        connect(sliderParam[eleIdx].sliderBar,
                SIGNAL(sigValueChanged(int, int, bool)),
                this,
                SLOT(slotValueChanged(int, int, bool)));
        connect(sliderParam[eleIdx].sliderBar,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }
    else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_DROPDOWN)
    {
        dropdownParam[eleIdx].box = new DropDown(startX, startY, IMAGE_SETTING_PARAM_BG_WIDTH, BGTILE_HEIGHT,
                                                 controlIdx, DROPDOWNBOX_SIZE_225,
                                                 uiLableStr, dropdownParam[eleIdx].list, this, "", true, 0, COMMON_LAYER,
                                                 true, 8, false, false, 5, IMAGE_SETTING_ELEMENT_LEFT_MARGIN);
        m_elementList[controlIdx] = dropdownParam[eleIdx].box;
        connect(dropdownParam[eleIdx].box,
                SIGNAL(sigValueChanged(QString, quint32)),
                this,
                SLOT(slotSpinboxValueChange(QString, quint32)));
        connect(dropdownParam[eleIdx].box,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        /* Set configured value */
        dropdownParam[eleIdx].box->setIndexofCurrElement(m_configResponse[cnfgIdx].toInt());
    }
    else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_TEXTBOX)
    {
        textboxParam[eleIdx].param = new TextboxParam();
        textboxParam[eleIdx].param->labelStr = uiLableStr;
        textboxParam[eleIdx].param->suffixStr = QString("(%1-%2)").arg(minRange).arg(maxRange);
        textboxParam[eleIdx].param->minNumValue = minRange;
        textboxParam[eleIdx].param->maxNumValue = maxRange;
        textboxParam[eleIdx].param->maxChar = getAllowedCharCnt(maxRange);
        textboxParam[eleIdx].param->isNumEntry = true;
        textboxParam[eleIdx].param->isCentre = true;
        textboxParam[eleIdx].param->textStr = m_configResponse[cnfgIdx].toString();

        textboxParam[eleIdx].box = new TextBox(startX, startY, IMAGE_SETTING_PARAM_BG_WIDTH, BGTILE_HEIGHT,
                                               controlIdx, TEXTBOX_SMALL, this, textboxParam[eleIdx].param,
                                               COMMON_LAYER, true, false, false, IMAGE_SETTING_ELEMENT_LEFT_MARGIN);
        m_elementList[controlIdx] = textboxParam[eleIdx].box;
        connect(textboxParam[eleIdx].box,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::deleteSingleUiElement(quint32 cnfgIdx)
{
    if (cnfgIdx >= IMAGE_SETTING_CNFG_COPY_TO_CAM_START)
    {
        return;
    }

    quint32 eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
    quint32 controlIdx = imageSettingsElementInfo[cnfgIdx].controlIdx;
    if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
    {
        sliderParam[eleIdx].multipler = 0.0f;
        if(IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
        {
            disconnect(sliderParam[eleIdx].textboxValue,
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            disconnect(sliderParam[eleIdx].textboxValue,
                       SIGNAL(sigTextValueAppended(QString, int)),
                       this,
                       SLOT(slotSliderBoxValueChange(QString, int)));
            DELETE_OBJ(sliderParam[eleIdx].textboxValue);
            DELETE_OBJ(sliderParam[eleIdx].textboxParam);
            DELETE_OBJ(sliderParam[eleIdx].backgroundImage);
            DELETE_OBJ(sliderParam[eleIdx].parameterLabel);

            disconnect(sliderParam[eleIdx].sliderBar,
                       SIGNAL(sigValueChanged(int, int, bool)),
                       this,
                       SLOT(slotValueChanged(int, int, bool)));
            disconnect(sliderParam[eleIdx].sliderBar,
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(sliderParam[eleIdx].sliderBar);
            m_elementList[controlIdx] = NULL;
            m_elementList[controlIdx+1] = NULL;
        }
    }
    else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_DROPDOWN)
    {
        dropdownParam[eleIdx].list.clear();
        if(IS_VALID_OBJ(dropdownParam[eleIdx].box))
        {
            disconnect(dropdownParam[eleIdx].box,
                       SIGNAL(sigValueChanged(QString, quint32)),
                       this,
                       SLOT(slotSpinboxValueChange(QString, quint32)));
            disconnect(dropdownParam[eleIdx].box,
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(dropdownParam[eleIdx].box);
            m_elementList[controlIdx] = NULL;
        }
    }
    else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_TEXTBOX)
    {
        if(IS_VALID_OBJ(textboxParam[eleIdx].box))
        {
            disconnect(textboxParam[eleIdx].box,
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(textboxParam[eleIdx].box);
            m_elementList[controlIdx] = NULL;
        }
        DELETE_OBJ(textboxParam[eleIdx].param);
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::slotSpinboxValueChange(QString str, quint32 index)
{
    switch(index)
    {
        case IMG_SET_CTRL_IMAGE_SETTINGS_SPINBOX:
        {
            updateImageSettingType(str);
        }
        break;

        case IMG_SET_CTRL_WDR_SPINBOX:
        {
            recreateImageSettingsParameters(UI_LABEL_WDR+1);
        }
        break;

        case IMG_SET_CTRL_EXPOSURE_RATIO_MODE_SPINBOX:
        {
            recreateImageSettingsParameters(UI_LABEL_EXPOSURE_RATIO_MODE+1);
        }
        break;

        case IMG_SET_CTRL_BACKLIGHT_SPINBOX:
        {
            recreateImageSettingsParameters(UI_LABEL_BACKLIGHT+1);
        }
        break;

        case IMG_SET_CTRL_EXPOSURE_MODE_SPINBOX:
        {
            recreateImageSettingsParameters(UI_LABEL_EXPOSURE_MODE+1);
        }
        break;

        case IMG_SET_CTRL_FLICKER_SPINBOX:
        {
            recreateImageSettingsParameters(UI_LABEL_FLICKER+1);
        }
        break;

        case IMG_SET_CTRL_HLC_SPINBOX:
        {
            if (IS_VALID_OBJ(dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box))
            {
                dropdownParam[IMG_SET_DROPDOWN_WDR_MODE].box->setIsEnabled(str == hlcModeStr[HLC_MODE_OFF]);
            }
        }
        break;

        case IMG_SET_CTRL_CAMERA_NAME_SPINBOX:
        {
            /* Check any image setting parameter's value is changed before switching to other camera option */
            if (isUserChangeConfig() == true)
            {
                /* Display message for parameter value change */
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
            }
            else
            {
                /* Update camera index as per selection */
                currentCameraIndex = (cameraNameDropDown.box->getIndexofCurrElement() + 1);

                /* Get current selected camera's Image settings Parameters */
                getCameraSupportedImageSettingsParameters();
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

//-------------------------------------------------------------------------------------------------
void ImageSettings::slotValueChanged(int changedValue, int indexInPage, bool)
{
    for (quint32 cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_COPY_TO_CAM_START; cnfgIdx++)
    {
        if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
        {
            if (imageSettingsElementInfo[cnfgIdx].controlIdx != (quint32)indexInPage)
            {
                continue;
            }

            /* Update textbox value on slider position change */
            quint32 eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
            quint32 minRange = imageSettingsElementInfo[cnfgIdx].eleMinRange;
            quint32 tempChangedValue = (quint32)((float)changedValue / sliderParam[eleIdx].multipler);
            sliderParam[eleIdx].textboxValue->setInputText(QString("%1").arg(minRange + tempChangedValue));
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::slotSliderBoxValueChange(QString currValue, int indexInPage)
{
    for (quint32 cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_COPY_TO_CAM_START; cnfgIdx++)
    {
        if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
        {
            if ((imageSettingsElementInfo[cnfgIdx].controlIdx + 1) != (quint32)indexInPage)
            {
                continue;
            }

            /* Update slider position on textbox value change */
            quint32 eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
            quint32 minRange = imageSettingsElementInfo[cnfgIdx].eleMinRange;
            quint32 configValue = (currValue.toUInt() < minRange) ? 0 : (currValue.toUInt() - minRange);
            sliderParam[eleIdx].sliderBar->setCurrentValue((configValue * sliderParam[eleIdx].multipler));
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
quint16 ImageSettings::getAllowedCharCnt(quint32 num)
{
    /* Get maximum characters supported in textbox */
    quint16 maxChar = 0;
    while(num > 0)
    {
        num = num/10;
        maxChar++;
    }
    return maxChar;
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::fillCameraList(void)
{
    cameraNameDropDown.list.clear();

    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        /* Add extra space for 1 to 9 index to set proper alignment */
        cameraNameDropDown.list.insert(index, QString("%1%2%3%4").arg((((index + 1) < 10) && (devTableInfo->totalCams > 10)) ? " " : "")
                              .arg(index + 1).arg(" : ").arg(applController->GetCameraNameOfDevice(currDevName, index)));
    }

    cameraNameDropDown.box->setNewList(cameraNameDropDown.list, (currentCameraIndex-1));
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::updateImageSettingType(QString str)
{
    bool    visible, showBasicElement;
    quint32 cnfgIdx, eleIdx;

    showBasicElement = (str == imageSettingsTypeStr[IMAGE_SETTING_TYPE_BASIC]);
    for(cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_COPY_TO_CAM_START; cnfgIdx++)
    {
        visible = (cnfgIdx < IMAGE_SETTING_CNFG_WDR) ? showBasicElement : !showBasicElement;
        eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
        if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
        {
            HIDE_SHOW_UI_ELEMENT(sliderParam[eleIdx].sliderBar, visible);
            HIDE_SHOW_UI_ELEMENT(sliderParam[eleIdx].backgroundImage, visible);
            HIDE_SHOW_UI_ELEMENT(sliderParam[eleIdx].textboxValue, visible);
            HIDE_SHOW_UI_ELEMENT(sliderParam[eleIdx].parameterLabel, visible);
        }
        else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_DROPDOWN)
        {
            HIDE_SHOW_UI_ELEMENT(dropdownParam[eleIdx].box, visible);
        }
        else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_TEXTBOX)
        {
            HIDE_SHOW_UI_ELEMENT(textboxParam[eleIdx].box, visible);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::saveConfig()
{
    /* Check for Camera Support not Found */
    if (errorMsg->isVisible() == true)
    {
        return;
    }

    /* Save config field and send set config message */
    saveConfigFields();
    createPayload(MSG_SET_CFG);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::saveConfigFields(void)
{
    quint32 cnfgIdx, eleIdx, minRange, maxRange, value;

    for(cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_COPY_TO_CAM_START; cnfgIdx++)
    {
        eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
        if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
        {
            if (IS_VALID_OBJ(sliderParam[eleIdx].textboxValue))
            {
                value = sliderParam[eleIdx].textboxValue->getInputText().toInt();
                minRange = imageSettingsElementInfo[cnfgIdx].eleMinRange;
                maxRange = imageSettingsElementInfo[cnfgIdx].eleMaxRange;
                m_configResponse[cnfgIdx] = GET_IMAGE_SETTING_VALUE(value, minRange, maxRange);
            }
        }
        else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_DROPDOWN)
        {
            if (IS_VALID_OBJ(dropdownParam[eleIdx].box))
            {
                m_configResponse[cnfgIdx] = dropdownParam[eleIdx].box->getIndexofCurrElement();
            }
        }
        else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_TEXTBOX)
        {
            if (IS_VALID_OBJ(textboxParam[eleIdx].box))
            {
                m_configResponse[cnfgIdx] = textboxParam[eleIdx].box->getInputText().toInt();
            }
        }
    }

    SET_CAMERA_MASK_BIT(copyToCameraFields, (currentCameraIndex - 1));
    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        m_configResponse[IMAGE_SETTING_CNFG_COPY_TO_CAM_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
    }

    for(quint8 index = 0; index < IMAGE_SETTING_CNFG_MAX; index++)
    {
        payloadLib->setCnfgArrayAtIndex(index, m_configResponse[index]);
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::createPayload(REQ_MSG_ID_e msgType)
{
    quint8  cnfgField = 0;
    QString payloadString = "";

    if (msgType == MSG_GET_CFG)
    {
        cnfgField = 1;
        payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                         IP_CAMERA_SETTING_TABLE_INDEX,
                                                         currentCameraIndex,
                                                         currentCameraIndex,
                                                         IP_CAMERA_ONVIF_SUPPORT+1,
                                                         IP_CAMERA_ONVIF_SUPPORT+1,
                                                         cnfgField);
    }

    payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                     IMAGE_SETTINGS_TABLE_INDEX,
                                                     currentCameraIndex,
                                                     currentCameraIndex,
                                                     CNFG_FRM_INDEX,
                                                     IMAGE_SETTING_CNFG_MAX,
                                                     IMAGE_SETTING_CNFG_MAX,
                                                     payloadString,
                                                     cnfgField);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    /* Check LoadProcessBar status if it is  true then do not load ProcessBar else load ProcessBar */
    if(processBar->isLoadedProcessBar() == false)
    {
        processBar->loadProcessBar();

    }
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

//-------------------------------------------------------------------------------------------------
bool ImageSettings::isUserChangeConfig()
{
    quint32 cnfgIdx, eleIdx;

    for(cnfgIdx = IMAGE_SETTING_CNFG_BRIGHTNESS; cnfgIdx < IMAGE_SETTING_CNFG_COPY_TO_CAM_START; cnfgIdx++)
    {
        eleIdx = imageSettingsElementInfo[cnfgIdx].elementIdx;
        if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_SLIDER)
        {
            if ((IS_VALID_OBJ(sliderParam[eleIdx].textboxValue)) && (m_configResponse[cnfgIdx] != sliderParam[eleIdx].textboxValue->getInputText().toInt()))
            {
                return true;
            }
        }
        else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_DROPDOWN)
        {
            if ((IS_VALID_OBJ(dropdownParam[eleIdx].box)) && (m_configResponse[cnfgIdx] != dropdownParam[eleIdx].box->getIndexofCurrElement()))
            {
                return true;
            }
        }
        else if (imageSettingsElementInfo[cnfgIdx].elementType == UI_ELE_TYPE_TEXTBOX)
        {
            if ((IS_VALID_OBJ(textboxParam[eleIdx].box)) && (m_configResponse[cnfgIdx] != textboxParam[eleIdx].box->getInputText().toInt()))
            {
                return true;
            }
        }
    }

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if (copyToCameraFields.bitMask[maskIdx] != m_configResponse[IMAGE_SETTING_CNFG_COPY_TO_CAM_START + maskIdx].toULongLong())
        {
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::getConfig()
{
    /* Check when camera support not found */
    if (errorMsg->isVisible() == true)
    {
        return;
    }

    createPayload(MSG_GET_CFG);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::defaultConfig()
{
    /* Check when camera support not found */
    if (errorMsg->isVisible() == true)
    {
        return;
    }

    createPayload(MSG_DEF_CFG);
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        /* Fail response received */
        cameraNameDropDown.box->setIndexofCurrElement(currentCameraIndex-1);

        /* Disable copy to camera button in fail to receive response */
        copyToCameraBtn->setIsEnabled(false);
        processBar->unloadProcessBar();

        /* We will not create UI in case of command response */
        if (((param->msgType == MSG_SET_CMD) && (param->cmdType == GET_CAPABILITY))
                || ((param->msgType == MSG_SET_CFG) && ((param->deviceStatus == CMD_CAM_DISCONNECTED) || (param->deviceStatus == CMD_CHANNEL_DISABLED))))
        {
            deleteAllImageSettingsParameters();
            setErrormsg(true, param->deviceStatus);
        }
        else
        {
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }

        /* Error response received */
        return;
    }

    switch(param->msgType)
    {
        case MSG_SET_CMD:
        {
            if (param->cmdType != GET_CAPABILITY)
            {
                break;
            }

            payloadLib->parseDevCmdReply(true, param->payload);
            CAPABILITY_CMD_ID_e capability = (CAPABILITY_CMD_ID_e)payloadLib->getCnfgArrayAtIndex(0).toInt();
            switch(capability)
            {
                case CAPABILITY_CMD_ID_IMAGING_CAPABILITY:
                {
                    imagingCapability = payloadLib->getCnfgArrayAtIndex(1).toInt();
                    getConfig();
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

        case MSG_GET_CFG:
        {
            quint8 cnfgOffset = 0, cnfgIdx;

            processBar->unloadProcessBar();
            m_configResponse.clear();
            copyToCameraBtn->setIsEnabled(true);
            payloadLib->parsePayload(param->msgType, param->payload);
            if (payloadLib->getcnfgTableIndex(0) != IP_CAMERA_SETTING_TABLE_INDEX)
            {
                break;
            }

            isOnvifSupportEnabled = payloadLib->getCnfgArrayAtIndex(cnfgOffset++).toBool();
            if (payloadLib->getcnfgTableIndex(1) != IMAGE_SETTINGS_TABLE_INDEX)
            {
                break;
            }

            /* Get all Image Parameters' configured value and set in m_configResponse */
            for(cnfgIdx = 0; cnfgIdx < IMAGE_SETTING_CNFG_MAX; cnfgIdx++)
            {
                m_configResponse[cnfgIdx] = payloadLib->getCnfgArrayAtIndex(cnfgIdx + cnfgOffset);
            }

            /* Set only selected camera */
            SET_CAMERA_MASK_BIT(copyToCameraFields, (currentCameraIndex - 1));
            for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
            {
                m_configResponse[IMAGE_SETTING_CNFG_COPY_TO_CAM_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
            }

            if (isImageSettingsComponentCreated == false)
            {
                createAllImageSettingsParameters();
            }
            else
            {
                recreateImageSettingsParameters(UI_LABEL_BRIGHTNESS);
                updateImageSettingType(imageSettingsTypeDropDown.box->getCurrValue());
            }
        }
        break;

        case MSG_SET_CFG:
        {
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            if (currentCameraIndex != (cameraNameDropDown.box->getIndexofCurrElement() + 1))
            {
                currentCameraIndex = (cameraNameDropDown.box->getIndexofCurrElement() + 1);
                getCameraSupportedImageSettingsParameters();
            }
            else
            {
                /* Get config */
                currentCameraIndex = (cameraNameDropDown.box->getIndexofCurrElement() + 1);
                getConfig();
            }
        }
        break;

        case MSG_DEF_CFG:
        {
            /* Get latest config */
            getConfig();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::handleInfoPageMessage(int index)
{
    if (index == INFO_OK_BTN)
    {
        if (infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            /* Save config changed by user */
            saveConfig();
        }
    }
    else
    {
        if (infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            /* Update camera index as per selection */
            currentCameraIndex = (cameraNameDropDown.box->getIndexofCurrElement() + 1);

            /* Get current selected camera's Image settings Parameters */
            getCameraSupportedImageSettingsParameters();
        }
    }
}

//-------------------------------------------------------------------------------------------------
void ImageSettings::setErrormsg(bool displayStatus, DEVICE_REPLY_TYPE_e replyType)
{
    DELETE_OBJ(errorMsg);
    errorMsg = new TextLabel((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH))/2-SCALE_HEIGHT(160),
                             ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)/2)-40), NORMAL_FONT_SIZE,
                             ValidationMessage::getDeviceResponceMessage(replyType), this);
    errorMsg->setVisible(displayStatus);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
