#ifndef BROADBANDSETTING_H
#define BROADBANDSETTING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/PageOpenButton.h"
#include "Controls/DropDown.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "BroadbandStatus.h"

#include "DataStructure.h"
#include "NavigationControl.h"

class BroadbandSetting : public ConfigPageControl
{
    Q_OBJECT

private:

    DropDown*           activeProfileDropDownBox;
    PageOpenButton*     statusPageopenBtn;
    ElementHeading*     eleHeading;
    DropDown*           profileNoDropDownBox;

    TextboxParam*       profileNameParam;
    TextBox*            profileNameTextbox;

    TextboxParam*       dialNumberParam;
    TextBox*            dialNumberTextbox;

    TextboxParam*       usernameParam;
    TextBox*            usernameTextbox;

    TextboxParam*       passwordParam;
    PasswordTextbox*    passwordTextbox;

    TextboxParam*       apnParam;
    TextBox*            apnTextbox;

    BroadbandStatus*    broadbandStatus;

    bool                m_initDone;
    quint8              m_currActiveProfIndex;
    quint8              m_frmIndex;
    quint8              m_toIndex;
    quint8              m_prevProfIndex;
    quint8              m_currProfIndex;

    QMap<quint8, QString>   activeProfileList;

public:
    explicit BroadbandSetting(QString devName, QWidget *parent = 0);
    ~BroadbandSetting();

    QMap<quint8, QString> defaultProfileList();
    void createDefaultComponent();
    void getConfig ();
    void saveConfig ();
    void defaultConfig();
    void createPayload(REQ_MSG_ID_e requestType);
    void handleInfoPageMessage(int index);
    void processDeviceResponse (DevCommParam *param, QString deviceName);
    bool checkDataChange();
    void getModemStatus();

public slots:
    void slotProfNumChanged(QString str,quint32 index);
    void slotProfileNameTextValueAppended(QString str,int index);
    void slotStatusPageButtonClick(int index);
    void slotStatusPageClosed(void);
};

#endif // BROADBANDSETTING_H
