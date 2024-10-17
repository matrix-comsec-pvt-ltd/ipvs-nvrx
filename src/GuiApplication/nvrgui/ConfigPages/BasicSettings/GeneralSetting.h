#ifndef GENERALSETTING_H
#define GENERALSETTING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/SpinBox.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/CnfgButton.h"
#include "Controls/DropDown.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/PageOpenButton.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "AutoConfigureCamera.h"
#include "AutoAddCamera.h"
#include "ConfigField.h"
#include "Controls/ToolTip.h"

#define MAX_GS_TEXTBOX 7

class GeneralSetting : public ConfigPageControl
{
    Q_OBJECT

public:
    explicit GeneralSetting(QString devName,
                   QWidget *parent = 0,
                   DEV_TABLE_INFO_t* devTable = NULL);
    ~GeneralSetting();


    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();
    void handleInfoPageMessage (int index);
    void getConnStatus(void);
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds);

public slots:
    void slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
    void slotOptionClicked(OPTION_STATE_TYPE_e currentState, int indexInPage);
    void slotPageOpenButtonClicked(int indexInPage);
    void slotObjectDelete();
    void slotImageMouseHover(int, bool isMouserHover);

private:
    TextBox*            deviceNameTextBox;
    TextboxParam*       deviceNameTextBoxParam;

    TextBox*            deviceNumberTextBox;
    TextboxParam*       deviceNumberTextBoxParam;

    TextBox*            singleDiskRecordTextBox;
    TextboxParam*       singleDiskRecordTextBoxParam;

    TextBox*            httpPortTextBox;
    TextboxParam*       httpPortTextBoxParam;
    quint16             tcpPortValue;
    TextBox*            tcpPortTextBox;
    TextboxParam*       tcpPortTextBoxParam;

    TextBox*            forwardedTcpPortTextBox;
    TextboxParam*       forwardedTcpPortTextBoxParam;

    TextBox*            rtspPort1TextBox;
    TextboxParam*       rtspPort1TextBoxParam;

    TextBox*            rtspPort2TextBox;
    TextboxParam*       rtspPort2TextBoxParam;

    TextBox*            m_liveViewPopUpDurationTextBox;
    TextboxParam*       m_liveViewPopUpDurationTextBoxParam;

    OptionSelectButton* integratedCosecCheckBox;
    OptionSelectButton* integratedSamasCheckBox;
    Ipv4TextBox*        samasServerIpTextbox;
    TextBox*            samasServerPortTextBox;
    TextboxParam*       samasServerPortTextBoxParam;

    TextBox*            m_preVideoLossTextBox;
    TextboxParam*       m_preVideoLossTextBoxParam;

    OptionSelectButton* autoAddCameraCheckBox;
    OptionSelectButton* autoConfigCameraCheckBox;

    quint8              recordingFormat;
    QString             startIpRange;
    QString             endIpRange;
    QString             samasServerIp;
    AutoConfigureCamera *autoConfigureCamera;
    PageOpenButton      *autoConfigureSetButton;
    QStringList          autoConfigureStringList;

    AutoAddCamera       *autoAddCamera;
    PageOpenButton      *autoAddCameraSetButton;
    QStringList          autoAddCameraStringList;

    DropDown            *dateFormatDropdown;
    DropDown            *timeFormatDropdown;

    OptionSelectButton* m_autoCloseRecFailCheckBox;
    Image*              m_statusImage;
    ToolTip*            m_statusToolTip;

    ElementHeading*     m_advancedSettingTile;
    OptionSelectButton* startLiveViewCheckBox;
    DropDown*           m_recordingFormat;
    BOOL                m_defaultButtonClick;
    BOOL                tcpPortChangeFlag;
    QStringList         remoteDeviceList;

    void createDefaultComponent();
};

#endif // GENERALSETTING_H
