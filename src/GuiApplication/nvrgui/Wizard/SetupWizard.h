#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QWidget>
#include "ApplController.h"
#include "Controls/BackGround.h"
#include "EnumFile.h"
#include "Controls/Heading.h"
#include "Controls/CnfgButton.h"
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"
#include "Controls/ElementHeading.h"
#include "InitPage.h"
#include "DateAndTime.h"
#include "NetworkSetting.h"
#include "DhcpServer.h"
#include "Storage.h"
#include "HDDGroup.h"
#include "ConfigureCamera.h"
#include "SearchCamera.h"
#include "WizardStatus.h"

class SetupWizard : public BackGround, public NavigationControl
{
    Q_OBJECT

private:
    Heading                 *m_pageHeading;
    CnfgButton              *m_nextBtn;
    CnfgButton              *m_quitBtn;
    CnfgButton              *m_backBtn;
    TextLabel               *m_datenTime;
    TextLabel               *m_network;
    TextLabel               *m_dhcpServer;
    TextLabel               *m_storage;
    TextLabel               *m_hddGroup;
    TextLabel               *m_configureCamera;
    TextLabel               *m_searchCamera;
    TextLabel               *m_status;
    ElementHeading          *m_heading_1;

    InfoPage*               m_infoPage;
    WizardCommon*           m_setupPage;
    QString                 m_currentDevName;
    WIZARD_PAGE_INDEXES_e   nextPIndex;
    bool                    m_visitPageStatus[MAX_WIZ_PG];
    bool                    m_isLanguagePageCreated;
    bool                    isHddGrpCreate;
    ApplController*         applController;
    DEV_TABLE_INFO_t        devTable;

public:
    explicit SetupWizard(QString devName, QWidget *parent = 0);
    void openNextPrevPage(WIZARD_PAGE_INDEXES_e);
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void updatePage(WIZARD_PAGE_INDEXES_e, bool, QString);
    void updateMouseHover(int pageId, QString color);
    void OnClickOpenPage(WIZARD_PAGE_INDEXES_e pgId);
    void quitButton();
    void hideStepper();
    void showStepper();

    ~SetupWizard();

signals:
    void sigQuitSetupWiz();
    void sigLanguageCfgModified(QString str);

public slots:
    void slotButtonOrLinkClick(int);
    void slotTextLableHover(int index, bool isHoverIn);
    void slotTextClicked(int pgId);
    void slotInfoPageBtnclick(int index);
};

#endif // WIZARD_H
