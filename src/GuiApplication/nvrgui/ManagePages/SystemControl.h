#ifndef SYSTEM_CONTROL_H
#define SYSTEM_CONTROL_H

#include "ManageMenuOptions.h"
#include "UserValidation.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"

typedef enum
{
    ACTIVATE_CONFIG = 0,
    SHUTDOWN_CONFIG,
    RESTART_CONFIG,
    CHECKBOX,
    OK_CONFIG = 6,
    MAX_SYSTEM_CONTROL_ELEMENT
}SYSTEM_CONTROL_ELEMENT_e;

typedef enum
{
    MANUAL_TRIGGER_NORMAL = 0,
    MANUAL_TRIGGER_ACTIVE
}MANUAL_TRIGGER_MODE_e;

class SystemControl : public ManageMenuOptions
{
    Q_OBJECT

private:

    ElementHeading* m_heading_1;
    ElementHeading* m_heading_2;
    ElementHeading* m_heading_3;

    CnfgButton* m_activate;
    CnfgButton* m_shutdown;
    CnfgButton* m_restart;

    BgTile* m_bgTile_M[3];
    OptionSelectButton* m_checkBox[3];

    BgTile* m_bgTile_Bottom;
    CnfgButton* m_ok;

    quint8 manualTriggerStatus[MAX_CAMERAS];
    quint8 m_currentConfig;

    UsersValidation* userValidation;

    bool isManualTriggerActive;
    QString m_userName, m_password;

public:
    explicit SystemControl(QString devName,
                  QWidget *parent = 0);
    ~SystemControl();

    void manualTrigger(QString devName);
    void shutdownDevice(QString devName);
    void restartDevice(QString devName);
    void getManualTriggerStatus(QString devName);
    void changeText();
    void factoryDefault(QString devName);

    void processDeviceResponse(DevCommParam *param, QString devName);
    void updateStatus(QString devName, qint8 status, qint8 index,quint8 eventType,
                      quint8 eventSubType);
    void handleInfoPageMessage(int buttonIndex);

public slots:
    void slotButtonClicked(OPTION_STATE_TYPE_e ,int);
    void slotConfigButtonClick(int index);
    void slotOkButtonClicked(QString userName, QString password);
};

#endif // SYSTEM_CONTROL_H
