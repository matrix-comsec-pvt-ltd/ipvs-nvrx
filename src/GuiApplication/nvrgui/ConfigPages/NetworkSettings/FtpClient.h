#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/DropDown.h"
#include "DataStructure.h"

typedef enum
{
    FTP_TEXTBOX_SER_ADDRESS,
    FTP_TEXTBOX_PORT,
    FTP_TEXTBOX_USERNAME,
    FTP_TEXTBOX_UPLOAD_PATH,

    MAX_FTP_TEXTBOX
}FTP_TEXTBOX_TYPE_e;

class FtpClient :public ConfigPageControl
{
    Q_OBJECT
private:
    quint8 frmIndex, toIndex;
    DropDown *ftpSerNoDropDownBox;
    OptionSelectButton *enableFtpOpt;

    TextboxParam *textboxParams[MAX_FTP_TEXTBOX];
    TextBox *textboxes[MAX_FTP_TEXTBOX];

    TextboxParam *passwordParam;
    PasswordTextbox *passwordTextbox;

    ControlButton *testConnButton;

public:
    explicit FtpClient(QString devName,
                   QWidget *parent = 0);
    ~FtpClient();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    bool checkDataIsSufficient();

public slots:
    void slotSpinboxValueChanged(QString ,quint32 );
    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int index);
    void slotTestBtnClick(int );
    void slotTextboxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
};

#endif // FTPCLIENT_H
