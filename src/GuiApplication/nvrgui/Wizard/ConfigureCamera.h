#ifndef CONFIGURECAMERA_H
#define CONFIGURECAMERA_H

#include <QWidget>
#include "Controls/OptionSelectButton.h"
#include "Controls/IpTextBox.h"
#include "Controls/PickList.h"
#include "Controls/TextBox.h"
#include "Controls/ElementHeading.h"
#include "Controls/CnfgButton.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/InfoPage.h"
#include "Controls/PasswordTextbox.h"
#include "ConfigField.h"
#include "KeyBoard.h"
#include "EnumFile.h"
#include "DataStructure.h"
#include "WizardCommon.h"
#include <QHostAddress>

typedef enum
{
    QUICK_AUTO_CONFIG_CLOSE_BUTTON,
    QUICK_AUTO_CONFIG_IP_RETAIN_FLAG,
    QUICK_AUTO_CONFIG_START_IP,
    QUICK_AUTO_CONFIG_END_IP,
    QUICK_AUTO_CONFIG_USR_NAME,
    QUICK_AUTO_CONFIG_PASSWD,
    QUICK_AUTO_CONFIG_PROF_RETAIN_FLAG,

    QUICK_AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST,
    QUICK_AUTO_CONFIG_MAIN_FRAMERATE_PICKLIST,
    QUICK_AUTO_CONFIG_MAIN_RESOLUTION_PICKLIST,
    QUICK_AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON,
    QUICK_AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON,
    QUICK_AUTO_CONFIG_MAIN_BITRATE_PICKLIST,
    QUICK_AUTO_CONFIG_MAIN_QUALITY_PICKLIST,
    QUICK_AUTO_CONFIG_MAIN_GOP_TEXTBOX,
    QUICK_AUTO_CONFIG_MAIN_AUDIO_CHECKBOX,

    QUICK_AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST,
    QUICK_AUTO_CONFIG_SUB_FRAMERATE_PICKLIST,
    QUICK_AUTO_CONFIG_SUB_RESOLUTION_PICKLIST,
    QUICK_AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON,
    QUICK_AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON,
    QUICK_AUTO_CONFIG_SUB_BITRATE_PICKLIST,
    QUICK_AUTO_CONFIG_SUB_QUALITY_PICKLIST,
    QUICK_AUTO_CONFIG_SUB_GOP_TEXTBOX,
    QUICK_AUTO_CONFIG_SUB_AUDIO_CHECKBOX,

    QUICK_AUTO_CONFIG_SAVE_BUTTON,
    QUICK_AUTO_CONFIG_OK_BUTTON,
    QUICK_AUTO_CONFIG_CANCEL_BUTTON,

    QUICK_MAX_CONFIG_ELEMENT
}CONFIG_ELEMENT_LIST_e;

typedef enum
{
    QUICK_FIELD_AUTO_CONFIG_IP_RETAIN,
    QUICK_FIELD_AUTO_CONFIG_START_IP,
    QUICK_FIELD_AUTO_CONFIG_END_IP,
    QUICK_FIELD_AUTO_CONFIG_PROF_RETAIN,

    QUICK_FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING,
    QUICK_FIELD_AUTO_CONFIG_MAIN_FRAMERATE,
    QUICK_FIELD_AUTO_CONFIG_MAIN_RESOLUTION,
    QUICK_FIELD_AUTO_CONFIG_MAIN_BITERATE_TYPE,
    QUICK_FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE,
    QUICK_FIELD_AUTO_CONFIG_MAIN_QUALITY,
    QUICK_FIELD_AUTO_CONFIG_MAIN_GOP,
    QUICK_FIELD_AUTO_CONFIG_MAIN_AUDIO,

    QUICK_FIELD_AUTO_CONFIG_SUB_VIDEOENCODING,
    QUICK_FIELD_AUTO_CONFIG_SUB_FRAMERATE,
    QUICK_FIELD_AUTO_CONFIG_SUB_RESOLUTION,
    QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_TYPE,
    QUICK_FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE,
    QUICK_FIELD_AUTO_CONFIG_SUB_QUALITY,
    QUICK_FIELD_AUTO_CONFIG_SUB_GOP,
    QUICK_FIELD_AUTO_CONFIG_SUB_AUDIO,
    QUICK_AUTO_CONFIG_USER_NAME = 20,
    QUICK_AUTO_CONFIG_PASSWORD,

    QUICK_MAX_AUTO_CONFIG
}CONFIG_FIELD_LIST_e;


class ConfigureCamera: public WizardCommon
{
    Q_OBJECT
public:
    explicit ConfigureCamera(QString devName, QString subHeadStr, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);
    virtual ~ConfigureCamera();
    virtual void navigationKeyPressed();

    void createDefaultElements(QString subHeadStr);
    void enableStreamRelatedElements(bool isEnable);
    void fillstreamRelatedParam();
    bool validationOnOkButton();
    void fillRecords();
    void saveConfig();
    void setRecords();

    quint8 findIndexofBitrateValue(QString bitRateStr);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void upadateEnableStateForElements();
    void getElementlistConfig();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

private:
    Rectangle*              m_backGround;
    Heading*                m_heading;
    CloseButtton*           m_closeButton;

    QStringList*            m_valueStringList;
    OptionSelectButton*     m_ipRetainCheckBox;
    IpTextBox*              m_startIpRange;
    IpTextBox*              m_endIpRange;

    OptionSelectButton*     m_profRetainCheckbox;

    ElementHeading*         m_mainStreamElementHeading;
    ElementHeading*         m_subStreamElementHeading;

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

    TextBox*                m_usrNameTextBox;
    TextboxParam*           m_usrNameTextBoxParam;

    PasswordTextbox*        m_passwordTextBox;
    TextboxParam*           m_passwordTextBoxParam;

    OptionSelectButton*     m_mainAudioCheckbox;
    OptionSelectButton*     m_subAudioCheckbox;

    TextLabel*              m_sub_pageHeading;
    InfoPage*               m_infoPage;

    QString                 startIpRange;
    QString                 endIpRange;
    ApplController*         m_applController;
    PayloadLib*             m_payloadLib;
    QStringList             quickElementList;
    QString                 currDevName;

public slots:

    void slotOptSelButtonClicked(OPTION_STATE_TYPE_e state, int index);
    void slotIpAddressEntryDone(quint32 );
    void slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e infoMsg);
    void slotInfoPageBtnclick(int);
    void slotIpAddressLoadInfoPage(quint32 );
    void slotValueChanged(quint8, QString, int index);
};

#endif // CONFIGURECAMERA_H
