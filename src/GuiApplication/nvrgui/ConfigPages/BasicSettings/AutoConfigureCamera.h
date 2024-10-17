#ifndef AUTOCONFIGURECAMERA_H
#define AUTOCONFIGURECAMERA_H

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
#include <QHostAddress>

typedef enum
{
    AUTO_CONFIG_CLOSE_BUTTON,
    AUTO_CONFIG_IP_RETAIN_FLAG,
    AUTO_CONFIG_START_IP,
    AUTO_CONFIG_END_IP,
    AUTO_CONFIG_USR_NAME,
    AUTO_CONFIG_PASSWD,
    AUTO_CONFIG_PROF_RETAIN_FLAG,

    AUTO_CONFIG_MAIN_VIDEOENCODING_PICKLIST,
    AUTO_CONFIG_MAIN_FRAMERATE_PICKLIST,
    AUTO_CONFIG_MAIN_RESOLUTION_PICKLIST,
    AUTO_CONFIG_MAIN_CBRBITRATETYPE_RADIOBUTTON,
    AUTO_CONFIG_MAIN_VBRBITRATETYPE_RADIOBUTTON,
    AUTO_CONFIG_MAIN_BITRATE_PICKLIST,
    AUTO_CONFIG_MAIN_QUALITY_PICKLIST,
    AUTO_CONFIG_MAIN_GOP_TEXTBOX,
    AUTO_CONFIG_MAIN_AUDIO_CHECKBOX,

    AUTO_CONFIG_SUB_VIDEOENCODING_PICKLIST,
    AUTO_CONFIG_SUB_FRAMERATE_PICKLIST,
    AUTO_CONFIG_SUB_RESOLUTION_PICKLIST,
    AUTO_CONFIG_SUB_CBRBITRATETYPE_RADIOBUTTON,
    AUTO_CONFIG_SUB_VBRBITRATETYPE_RADIOBUTTON,
    AUTO_CONFIG_SUB_BITRATE_PICKLIST,
    AUTO_CONFIG_SUB_QUALITY_PICKLIST,
    AUTO_CONFIG_SUB_GOP_TEXTBOX,
    AUTO_CONFIG_SUB_AUDIO_CHECKBOX,

    AUTO_CONFIG_SAVE_BUTTON,
    AUTO_CONFIG_OK_BUTTON,
    AUTO_CONFIG_CANCEL_BUTTON,

    MAX_AUTO_CONFIG_ELEMENT
}AUTO_CONFIG_ELEMENT_LIST_e;

typedef enum
{
    FIELD_AUTO_CONFIG_IP_RETAIN,
    FIELD_AUTO_CONFIG_START_IP,
    FIELD_AUTO_CONFIG_END_IP,
    FIELD_AUTO_CONFIG_PROF_RETAIN,

    FIELD_AUTO_CONFIG_MAIN_VIDEOENCODING,
    FIELD_AUTO_CONFIG_MAIN_FRAMERATE,
    FIELD_AUTO_CONFIG_MAIN_RESOLUTION,
    FIELD_AUTO_CONFIG_MAIN_BITERATE_TYPE,
    FIELD_AUTO_CONFIG_MAIN_BITRATE_VALUE,
    FIELD_AUTO_CONFIG_MAIN_QUALITY,
    FIELD_AUTO_CONFIG_MAIN_GOP,
    FIELD_AUTO_CONFIG_MAIN_AUDIO,

    FIELD_AUTO_CONFIG_SUB_VIDEOENCODING,
    FIELD_AUTO_CONFIG_SUB_FRAMERATE,
    FIELD_AUTO_CONFIG_SUB_RESOLUTION,
    FIELD_AUTO_CONFIG_SUB_BITRATE_TYPE,
    FIELD_AUTO_CONFIG_SUB_BITRATE_VALUE,
    FIELD_AUTO_CONFIG_SUB_QUALITY,
    FIELD_AUTO_CONFIG_SUB_GOP,
    FIELD_AUTO_CONFIG_SUB_AUDIO,

    AUTO_CONFIG_USER_NAME,
    AUTO_CONFIG_PASSWORD,

    MAX_FIELD_AUTO_CONFIG
}AUTO_CONFIG_FIELD_LIST_e;

class AutoConfigureCamera : public KeyBoard
{
    Q_OBJECT

public:
    explicit AutoConfigureCamera(QStringList *elementList, QString deviceName,bool flag, QWidget *parent = 0);
    ~AutoConfigureCamera();

    void createDefaultElements();
    void enableStreamRelatedElements(bool isEnable);
    void fillstreamRelatedParam();
    bool validationOnOkButton();
    void fillRecords();
    void setRecords();

    quint8 findIndexofBitrateValue(QString bitRateStr);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void upadateEnableStateForElements();

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);

signals:
    void sigObjectDelete();


private:
    Rectangle*              m_backGround;
    Heading*                m_heading;
    CloseButtton*           m_closeButton;

    QStringList*            m_valueStringList;
    OptionSelectButton*     m_ipRetainCheckBox;
    IpTextBox*              m_startIpAddress;
    IpTextBox*              m_endIpAddress;

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

    CnfgButton*             m_saveBtn;
    CnfgButton*             m_cancleBtn;
    CnfgButton*             m_okBtn;

    NavigationControl*      m_elementlist[MAX_AUTO_CONFIG_ELEMENT];
    quint8                  m_currElement;
    InfoPage*               m_infoPage;

    QString                 startIpAddress;
    QString                 endIpAddress;
    ApplController*         m_applController;
    PayloadLib*             m_payloadLib;
    QString                 m_currDeviceName;
    bool                    m_isEnable;


public slots:
    void slotCloseButtonClick(int index);
    void slotOptSelButtonClicked(OPTION_STATE_TYPE_e state, int index);
    void slotIpAddressEntryDone(quint32 );
    void slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e infoMsg);
    void slotInfoPageBtnclick(int index);
    void slotIpAddressLoadInfoPage(quint32 );
    void slotValueChanged(quint8, QString, int index);
    void slotUpdateCurrentElement(int index);
};

#endif // AUTOCONFIGURECAMERA_H
