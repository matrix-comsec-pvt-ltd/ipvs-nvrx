#ifndef DDNSCLIENT_H
#define DDNSCLIENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/ReadOnlyElement.h"
#include "DataStructure.h"

typedef enum
{
    DDNS_TEXTBOX_USERNAME,
    DDNS_TEXTBOX_HOSTNAME,
    DDNS_TEXTBOX_INTERVAL,

    MAX_DDNS_TEXTBOX
}DDNS_TEXTBOX_e;

class DDNSClient :public ConfigPageControl
{
    Q_OBJECT
private:
    OptionSelectButton *enableCheckbox;
    ReadOnlyElement *serNameReadOnly;

    TextboxParam *textboxParams[MAX_DDNS_TEXTBOX];
    TextBox *textBoxes[MAX_DDNS_TEXTBOX];

    TextboxParam *passwordParam;
    PasswordTextbox *passwordTextbox;

    ControlButton *updateButton;

    OPTION_STATE_TYPE_e saveEnable;
    QString saveUsrname;
    QString savePassword;
    QString saveHostname;
    QString saveInterval;

public:
    explicit DDNSClient(QString devName,
                   QWidget *parent = 0);
    ~DDNSClient();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

public slots:
    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state,int index);
    void slotUpdateButtonClick(int index);
    void slotTextboxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
};

#endif // DDNSCLIENT_H
