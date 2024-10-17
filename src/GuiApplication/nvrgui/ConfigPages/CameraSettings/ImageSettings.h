#ifndef IMAGE_SETTINGS_H
#define IMAGE_SETTINGS_H

//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		ImageSettings.h
@brief      File containing the function prototype of image settings GUI elements and set/get configuration
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/TextBox.h"
#include "Controls/DropDown.h"
#include "Controls/SliderControl.h"

#include "CopyToCamera.h"
#include "Controls/PageOpenButton.h"
#include "ValidationMessage.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    IMG_SET_SLIDER_BRIGHTNESS = 0,
    IMG_SET_SLIDER_CONTRAST,
    IMG_SET_SLIDER_SATURATION,
    IMG_SET_SLIDER_HUE,
    IMG_SET_SLIDER_SHARPNESS,
    IMG_SET_SLIDER_EXPOSURE_RATIO,
    IMG_SET_SLIDER_WDR_STRENGTH,
    IMG_SET_SLIDER_FLICKER_STRENGTH,
    IMG_SET_SLIDER_EXPOSURE_GAIN,
    IMG_SET_SLIDER_EXPOSURE_IRIS,
    IMG_SET_SLIDER_NORMAL_LIGHT_LUMINANCE,
    IMG_SET_SLIDER_LED_SENSITIVITY,
    IMG_SET_SLIDER_MAX
}IMG_SET_SLIDER_e;

typedef enum
{
    IMG_SET_DROPDOWN_WHITE_BALANCE = 0,
    IMG_SET_DROPDOWN_WDR_MODE,
    IMG_SET_DROPDOWN_EXPOSURE_RATIO_MODE,
    IMG_SET_DROPDOWN_BACKLIGHT_MODE,
    IMG_SET_DROPDOWN_EXPOSURE_MODE,
    IMG_SET_DROPDOWN_FLICKER_MODE,
    IMG_SET_DROPDOWN_HLC_MODE,
    IMG_SET_DROPDOWN_NORMAL_LIGHT_GAIN,
    IMG_SET_DROPDOWN_LED_MODE,
    IMG_SET_DROPDOWN_MAX
}IMG_SET_DROPDOWN_e;

typedef enum
{
    IMG_SET_TEXTBOX_EXPOSURE_TIME = 0,
    IMG_SET_TEXTBOX_NORMAL_LIGHT_MINIMUM_FPS,
    IMG_SET_TEXTBOX_MAX
}IMG_SET_TEXTBOX_e;

typedef struct
{
    SliderControl   *sliderBar;
    Image           *backgroundImage;
    TextBox         *textboxValue;
    TextboxParam    *textboxParam;
    TextLabel       *parameterLabel;
    float           multipler;
}IMAGE_SETTING_SLIDER_t;

typedef struct
{
    DropDown                *box;
    QMap<quint8, QString>   list;
}IMAGE_SETTING_DROPDOWN_t;

typedef struct
{
    TextboxParam    *param;
    TextBox         *box;
}IMAGE_SETTING_TEXTBOX_t;

class ImageSettings : public ConfigPageControl
{
    Q_OBJECT

    bool                        isImageSettingsComponentCreated;
    bool                        isOnvifSupportEnabled;
    quint8                      currentCameraIndex;
    TextLabel                   *errorMsg;
    IMAGE_SETTING_DROPDOWN_t    cameraNameDropDown;
    IMAGE_SETTING_DROPDOWN_t    imageSettingsTypeDropDown;
    PageOpenButton              *copyToCameraBtn;
    CAMERA_BIT_MASK_t           copyToCameraFields;
    CopyToCamera                *copytoCamera;

    /* Camera supported image capability mask */
    UINT32                      imagingCapability;

    /* Image settings different types of controls */
    IMAGE_SETTING_SLIDER_t      sliderParam[IMG_SET_SLIDER_MAX];
    IMAGE_SETTING_DROPDOWN_t    dropdownParam[IMG_SET_DROPDOWN_MAX];
    IMAGE_SETTING_TEXTBOX_t     textboxParam[IMG_SET_TEXTBOX_MAX];

public:
    explicit ImageSettings(QString deviceName, QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~ImageSettings();

    void getConfig();
    void saveConfig();
    void defaultConfig();
    bool isUserChangeConfig();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void handleInfoPageMessage(int index);

    void createDefaultComponent(void);
    void fillCameraList(void);
    void saveConfigFields(void);
    void createPayload(REQ_MSG_ID_e msgType);

    void getCameraSupportedImageSettingsParameters(void);
    void createAllImageSettingsParameters(void);
    void recreateImageSettingsParameters(quint32 startUiEleIdx);
    void createImageSettingsParameters(quint32 startUiEleIdx);
    void deleteAllImageSettingsParameters(void);
    void deleteImageSettingsParameters(quint32 startUiEleIdx);
    void createSingleUiElement(quint32 cnfgIdx, quint32 startX, quint32 startY, QString uiLableStr);
    void deleteSingleUiElement(quint32 cnfgIdx);

    void updateImageSettingType(QString str);
    void setErrormsg(bool displayStatus, DEVICE_REPLY_TYPE_e replyType = CMD_FEATURE_NOT_SUPPORTED);
    quint16 getAllowedCharCnt(quint32 num);

public slots:
    void slotSpinboxValueChange(QString, quint32);
    void slotValueChanged(int changedValue, int indexInPage, bool);
    void slotSliderBoxValueChange(QString str, int indexInPage);
    void slotPageOpenBtnClick(int index);
    void slotCopytoCamDelete(quint8);
};

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
