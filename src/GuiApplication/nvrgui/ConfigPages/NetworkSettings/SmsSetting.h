#ifndef SMSSETTING_H
#define SMSSETTING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/DropDown.h"
#include "Controls/ElementHeading.h"
#include "DataStructure.h"

typedef enum
{
    SMS_GATEWAY_CENTRE,
    SMS_LANE,
    BUSINESS_SMS,
    BULK_SMS,

    MAX_SMS_SERV_PROVIDER
}SMS_SER_PROVIDER_e;

class SmsSetting : public ConfigPageControl
{
    Q_OBJECT
private:
    SMS_SER_PROVIDER_e currProvider;
    ElementHeading *httpModeHeading, *testSmsHeading;
    DropDown*   serProviderDropDownBox;
    TextboxParam *usernameParam, *senderIdParam, *mobNumParam;
    TextBox *usernameTextbox, *senderIdTextbox, *mobNumTextbox;

    PasswordTextbox *passwordTextbox;
    TextboxParam *passwordParam;

    OptionSelectButton *flashMsgOpt;
    ControlButton *checkBalBtn, *testSmsBtn;

public:
    explicit SmsSetting(QString devName,
                   QWidget *parent = 0);
    ~SmsSetting();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    bool checkDataIsSufficient();

public slots:
    void slotSpinboxValueChanged(QString str,quint32 );
    void slotControlBtnClick(int index);
    void slotTextboxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
};

#endif // SMSSETTING_H
