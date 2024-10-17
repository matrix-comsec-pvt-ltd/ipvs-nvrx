#ifndef TCPNOTIFICATION_H
#define TCPNOTIFICATION_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "DataStructure.h"

typedef enum
{
    TCP_TEXTBOX_SERVER,
    TCP_TEXTBOX_PORT,

    MAX_TCP_TEXTBOX
}TCP_TEXTBOX_TYPE_e;

class TcpNotification : public ConfigPageControl
{
    Q_OBJECT
private:
    OptionSelectButton *enableTcpOpt;

    TextboxParam *textboxParams[MAX_TCP_TEXTBOX];
    TextBox *textboxes[MAX_TCP_TEXTBOX];

    ControlButton *testConnButton;

public:
    explicit TcpNotification(QString devName,
                   QWidget *parent = 0);
    ~TcpNotification();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    bool checkDataIsSufficient();

public slots:
    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int index);
    void slotTestBtnClick(int );
    void slotTextboxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
};

#endif // TCPNOTIFICATION_H
