#ifndef WIZARDCOMMON_H
#define WIZARDCOMMON_H

#include <QWidget>
#include <QPainter>
#include "DataStructure.h"
#include "ApplController.h"
#include "Controls/ProcessBar.h"
#include "Controls/Bgtile.h"

typedef enum
{
    WIZ_PG_INIT = 0,
    WIZ_PG_DTTIME,
    WIZ_PG_NTWCONF,
    WIZ_PG_DHCPSERVER,
    WIZ_PG_STRGCONF,
    WIZ_PG_HDDGROUP,
    WIZ_PG_AUTOCONF,
    WIZ_PG_CAMSRCH,
    WIZ_PG_STATUS,
    MAX_WIZ_PG
}WIZARD_PAGE_INDEXES_e;

typedef enum
{
    NEXT_PG,
    PREV_PG,
}PG_NAVG_OPT_e;

class InvisibleBackground : public QWidget
{
    Q_OBJECT
public:
    explicit InvisibleBackground(QWidget *parent = 0);
    ~InvisibleBackground();
    void paintEvent (QPaintEvent *);
};

class WizardCommon : public QWidget
{
     Q_OBJECT

private:
    BgTile                      *m_bgTile;

protected:
    WIZARD_PAGE_INDEXES_e       m_CurrentPage;
    bool                        m_deletePage;
    bool                        m_getConfig;

public:
    static QWidget              *m_parent;
    static InvisibleBackground  *m_invisibleBackGround;
    ProcessBar                  *processBar;

    explicit WizardCommon(QWidget* parent ,WIZARD_PAGE_INDEXES_e PageIndex);
    virtual ~WizardCommon();
    static void InfoPageImage();
    static void UnloadInfoPageImage();
    WIZARD_PAGE_INDEXES_e getCurrPageIndex() const;
    WIZARD_PAGE_INDEXES_e getNextPrevPageIndex(PG_NAVG_OPT_e option);
    bool isObjDeleteReq() const;
    void LoadProcessBar();
    void UnloadProcessBar();

    virtual void saveConfig() = 0;
    virtual void processDeviceResponse(DevCommParam *param, QString deviceName) = 0;

signals:
    void sigLanguageCfgChanged(QString str);

public slots:
};


#endif // WIZARDCOMMON_H
