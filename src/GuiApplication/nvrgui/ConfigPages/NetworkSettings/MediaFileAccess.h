#ifndef MEDIAFILEACCESS_H
#define MEDIAFILEACCESS_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "DataStructure.h"

class MediaFileAccess :public ConfigPageControl
{
    Q_OBJECT
private:    
    OptionSelectButton *enableFtpOpt;

    TextboxParam *ftpPortParam;
    TextBox *ftpportTextbox;

public:
    explicit MediaFileAccess(QString devName, QWidget *parent = 0);
    ~MediaFileAccess();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

public slots:
    void slotEnableButtonClicked(OPTION_STATE_TYPE_e state ,int );
    void slotTextboxLoadInfopage(int index,INFO_MSG_TYPE_e msgType);
};
#endif // MEDIAFILEACCESS_H
