#ifndef EMAILCLIENT_H
#define EMAILCLIENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/ElementHeading.h"
#include "DataStructure.h"
#include "Controls/DropDown.h"

typedef enum
{
    EMAIL_TEXTBOX_SERVER,
    EMAIL_TEXTBOX_PORT,
    EMAIL_TEXTBOX_USERNAME,
    EMAIL_TEXTBOX_SENDER_ID,
    EMAIL_TEXTBOX_RECEIVER_ID,
    MAX_EMAIL_TEXTBOX
}EMAIL_TEXTBOX_TYPE_e;

class EmailClient : public ConfigPageControl
{
    Q_OBJECT
private:
    OptionSelectButton  *enableEmailOpt;

    TextboxParam        *textboxParams[MAX_EMAIL_TEXTBOX];
    TextBox             *textboxes[MAX_EMAIL_TEXTBOX];

    TextboxParam        *passwordParam;
    PasswordTextbox     *passwordTextbox;

    DropDown            *encryptionType;

    ElementHeading      *testMailHeading;
    ControlButton       *testButton;

public:
    explicit EmailClient(QString devName, QWidget *parent = 0);
    ~EmailClient();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    bool checkDataIsSufficient();

public slots:
    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state, int index);
    void slotTestBtnClick(int index);
    void slotTextboxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
};

#endif // EMAILCLIENT_H
