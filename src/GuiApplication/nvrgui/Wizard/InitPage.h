#ifndef INITPAGE_H
#define INITPAGE_H

#include "Controls/TextLabel.h"
#include "Controls/OptionSelectButton.h"
#include "NavigationControl.h"
#include "Configuration/DeviceConfig.h"
#include "Controls/Bgtile.h"
#include "WizardCommon.h"
#include "EnumFile.h"
#include "Controls/BackGround.h"
#include "WizardChangePass.h"
#include "Configuration/DeviceConfig.h"

class InitPage : public WizardCommon
{
    Q_OBJECT
private:
    TextLabel               *m_welcomeTextLabel;
    TextLabel               *m_wizardSetupTextLabel;
    TextLabel               *m_wizardRunTextLabel;
    OptionSelectButton      *m_wizardRunCheckBox;
    TextLabel               *m_initChangePass;
    WizardChangePass        *m_modifyPassword;
    QString                 m_currentDevName;
    QString                 m_imageSource;
    InvisibleWidgetCntrl*   m_inVisibleWidget;
    WIZ_OPEN_CONFIG_t       WizOpenStatus;
    ApplController*         m_applController;
    PayloadLib*             m_payloadLib;
public:
    explicit InitPage(QString devName, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);
    void createDefaultElements();
    void rememberStatusToFile();
    void getOtherUserParam();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void saveConfig();
    ~InitPage();
public slots:
    void slotTextClicked(int);
    void slotTextLableHover(int, bool isHoverIn);
    void slotcloseButtonClick(TOOLBAR_BUTTON_TYPE_e);
};
#endif // INITPAGE_H
