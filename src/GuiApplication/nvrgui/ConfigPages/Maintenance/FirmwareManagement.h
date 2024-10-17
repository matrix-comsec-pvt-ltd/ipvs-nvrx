#ifndef FIRMWARE_MANAGEMENT_H
#define FIRMWARE_MANAGEMENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/Clockspinbox.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/DropDown.h"
#include "Controls/ControlButton.h"

class FirmwareManagement :public ConfigPageControl
{
    Q_OBJECT
private:
    DropDown        *m_autoFirmwareUpgradeOptionDropDownBox;
    ClockSpinbox    *m_autoFirmwareUpgradeSyncTime;
    DropDown        *m_ftpServerOptionDropDownBox;
    TextboxParam    *m_ftpAddrParam;
    TextBox         *m_ftpAddrTextbox;
    TextboxParam    *m_ftpPortParam;
    TextBox         *m_ftpPortTextbox;
    TextboxParam    *m_ftpUserNameParam;
    TextBox         *m_ftpUserNameTextbox;
    TextboxParam    *m_ftpPasswordParam;
    PasswordTextbox *m_ftpPasswordTextbox;
    TextboxParam    *m_ftpPathParam;
    TextBox         *m_ftpPathTextbox;
    ControlButton   *m_checkForUpdateButton;
    InfoPage        *m_infoPage;

    QString         m_ftpAddr;
    QString         m_ftpPort;
    QString         m_ftpUserName;
    QString         m_ftpPassword;
    QString         m_ftpPath;

public:
    explicit FirmwareManagement(QString devName, QWidget *parent = 0, DEV_TABLE_INFO_t* devTabInfo = NULL);
    ~FirmwareManagement();

    void createDefaultComponent();
    void getConfig();
    void saveConfig();
    void defaultConfig();
    void createPayload(REQ_MSG_ID_e requestType);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

public slots:
    void slotSpinboxValueChanged(QString str, quint32 index);
    void slotButtonClicked(int index);
    void slotInfoPageCnfgBtnClick(int index);
};

#endif // FIRMWARE_MANAGEMENT_H


