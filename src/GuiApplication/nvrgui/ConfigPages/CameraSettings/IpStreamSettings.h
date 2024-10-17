#ifndef IPSTREAMSETTINGS_H
#define IPSTREAMSETTINGS_H

#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/PickList.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextBox.h"
#include "Controls/ElementHeading.h"
#include "Controls/DropDown.h"
#include "Controls/PageOpenButton.h"
#include "CopyToCamera.h"

typedef enum
{
    IP_STREAM_SETTINGS_CAMERANAME_SPINBOX = 0,
    IP_STREAM_SETTINGS_MAIN_COPY_TO_CAM_LABEL,
    IP_STREAM_SETTINGS_MAIN_PROFILE_PICKLIST,
    IP_STREAM_SETTINGS_MAIN_VIDEOENCODING_PICKLIST,
    IP_STREAM_SETTINGS_MAIN_RESOLUTION_PICKLIST,
    IP_STREAM_SETTINGS_MAIN_FRAMERATE_PICKLIST,
    IP_STREAM_SETTINGS_MAIN_CBRBITRATETYPE_RADIOBUTTON,
    IP_STREAM_SETTINGS_MAIN_VBRBITRATETYPE_RADIOBUTTON,
    IP_STREAM_SETTINGS_MAIN_BITRATE_PICKLIST,
    IP_STREAM_SETTINGS_MAIN_QUALITY_PICKLIST,
    IP_STREAM_SETTINGS_MAIN_GOP_TEXTBOX,
    IP_STREAM_SETTINGS_MAIN_AUDIO_CHECKBOX,

    IP_STREAM_SETTINGS_SUB_COPY_TO_CAM_LABEL,
    IP_STREAM_SETTINGS_SUB_PROFILE_PICKLIST,
    IP_STREAM_SETTINGS_SUB_VIDEOENCODING_PICKLIST,
    IP_STREAM_SETTINGS_SUB_RESOLUTION_PICKLIST,
    IP_STREAM_SETTINGS_SUB_FRAMERATE_PICKLIST,
    IP_STREAM_SETTINGS_SUB_CBRBITRATETYPE_RADIOBUTTON,
    IP_STREAM_SETTINGS_SUB_VBRBITRATETYPE_RADIOBUTTON,
    IP_STREAM_SETTINGS_SUB_BITRATE_PICKLIST,
    IP_STREAM_SETTINGS_SUB_QUALITY_PICKLIST,
    IP_STREAM_SETTINGS_SUB_GOP_TEXTBOX,
    IP_STREAM_SETTINGS_SUB_AUDIO_CHECKBOX,
    MAX_IP_STREAM_SETTINGS_ELEMENT
}IP_STREAM_SETTINGS_ELEMENT_e;

class IpStreamSettings : public ConfigPageControl
{
    Q_OBJECT
private:
    DropDown*               m_cameraNameDropDownBox;

    ElementHeading*         m_mainStreamElementHeading;
    ElementHeading*         m_subStreamElementHeading;

    PickList*               m_mainProfilePicklist;
    PickList*               m_subProfilePicklist;

    PickList*               m_mainVideoEncodingPicklist;
    PickList*               m_subVideoEncodingPicklist;

    PickList*               m_mainResolutionPicklist;
    PickList*               m_subResolutionPicklist;

    PickList*               m_mainFrameRatePicklist;
    PickList*               m_subFrameRatePicklist;

    OptionSelectButton*     m_mainCBRTypeRadioButton;
    OptionSelectButton*     m_mainVBRTypeRadioButton;

    OptionSelectButton*     m_subCBRTypeRadioButton;
    OptionSelectButton*     m_subVBRTypeRadioButton;

    PickList*               m_mainBitRatePicklist;
    PickList*               m_subBitRatePicklist;

    PickList*               m_mainQualityPicklist;
    PickList*               m_subQualityPicklist;

    TextboxParam*           m_mainGOPParam;
    TextboxParam*           m_subGOPParam;

    TextBox*                m_mainGOPTextbox;
    TextBox*                m_subGOPTextbox;

    OptionSelectButton*     m_mainAudioCheckbox;
    OptionSelectButton*     m_subAudioCheckbox;

    PageOpenButton*         m_mainCopyToCamButton;
    PageOpenButton*         m_subCopyToCamButton;
    CopyToCamera*           m_copyToCamera;
    CAMERA_BIT_MASK_t       m_mainCopyToCameraField;
    CAMERA_BIT_MASK_t       m_subCopyToCameraField;

    quint8                  m_currentCameraIndex;
    quint8                  m_pickListIndexForCommand;

    QMap<quint8, QString>   m_cameraNameList;
    QMap<quint8, QString>   m_profileMap;
    QMap<quint8, QString>   m_mainVideoEncodingMap, m_subVideoEncodingMap;
    QMap<quint8, QString>   m_mainResolutionMap, m_subResolutionMap;
    QMap<quint8, QString>   m_mainFrameRateMap, m_subFrameRateMap;
    QMap<quint8, QString>   m_mainBitRateMap, m_subBitRateMap;
    QMap<quint8, QString>   m_mainQualityMap, m_subQualityMap;

    bool                    isOnvifSupport;
    bool                    media2Support;

public:
    explicit IpStreamSettings(QString deviceName,
                     QWidget* parent = 0,
                     DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~IpStreamSettings();

    void createCommandPayload(SET_COMMAND_e commandType, quint8 totalFields);
    void createConfigPayload(REQ_MSG_ID_e requestType);
    void fillRecords();
    void setRecord();
    quint8 findIndexofBitrateValue(QString bitRateStr);
    void upadateEnableStateForElements();
    void createDefaultElements();
    void getConfig();
    void defaultConfig();
    void saveConfig();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void updateAllParameterOfMainProfile(bool isMainStream = true);
    void getConfigOfIpCamera();

public slots:
    void slotSpinboxValueChanged(QString, quint32);
    void slotLoadPickListLoader(int indexInPage);
    void slotPickListValueChanged(quint8 key, QString value, int indexInPage);
    void slotRadioButtonClicked(OPTION_STATE_TYPE_e currentState, int indexInPage);
    void slotTextBoxLoadInfopage(int indexInPage, INFO_MSG_TYPE_e msgType);
    void slotTextBoxValueAppended(QString,int);
    void slotCopyToCamButtonClick(int index);
    void slotSubObjectDelete(quint8);
    bool validateRecord();
};

#endif // IPSTREAMSETTINGS_H
