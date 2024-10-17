#ifndef MATRIXDNSCLIENT_H
#define MATRIXDNSCLIENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "DataStructure.h"

class MatrixDnsClient : public ConfigPageControl
{
    Q_OBJECT
private:
    OptionSelectButton *enableCheckbox;

    TextboxParam *hostNameParam;
    TextBox *hostNameTextbox;

    TextboxParam *portParam;
    TextBox *portTextbox;

    ControlButton *regButton;

    QString saveHostName;
    QString savePort;
public:
    explicit MatrixDnsClient(QString devName,
                   QWidget *parent = 0);
    ~MatrixDnsClient();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();
public slots:
    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state,int index);
    void slotTextboxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
    void slotRegBtnButtonClick(int index);
};

#endif // MATRIXDNSCLIENT_H
