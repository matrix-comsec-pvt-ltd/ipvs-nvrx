#ifndef CONFIGPAGECONTROL_H
#define CONFIGPAGECONTROL_H

#include <QWidget>
#include "Controls/BackGround.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "Controls/CnfgButton.h"
#include "Controls/ProcessBar.h"
#include "Controls/InfoPage.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define PAGE_RIGHT_PANEL_STARTX                     ( SETTING_LEFT_PANEL_WIDTH + INNER_LEFT_MARGIN  )
#define PAGE_RIGHT_PANEL_STARTY                     ( INNER_TOP_MARGIN + OUTER_TOP_MARGIN  + 30 )
#define PAGE_RIGHT_PANEL_WIDTH                      ( SETTING_RIGHT_PANEL_WIDTH - INNER_RIGHT_MARGIN - INNER_LEFT_MARGIN)
#define PAGE_RIGHT_PANEL_HEIGHT                     ( SETTING_RIGHT_PANEL_HEIGHT -INNER_BOTTOM_MARGIN  -INNER_TOP_MARGIN )
#define PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON  ( PAGE_RIGHT_PANEL_HEIGHT - 101 )

#define CNFG_FRM_INDEX          1
#define CNFG_FRM_FIELD          1


typedef enum
{
    CNFG_TYPE_DFLT_REF_SAV_BTN,
    CNFG_TYPE_REF_SAV_BTN,
    CNFG_TYPE_SAV_BTN,

    MAX_CNFG_BTN_TYPE
}CNFG_BTN_PAIR_TYPE_e;

typedef enum
{
    CNFG_DEFAULT_BTN,
    CNFG_REFRESH_BTN,
    CNFG_SAVE_BTN,

    MAX_CNFG_BTN
}CNFG_BTN_PAIR_e;

class ConfigPageControl : public KeyBoard, public NavigationControl
{
    Q_OBJECT

protected:
    DEV_TABLE_INFO_t *devTableInfo;
    ApplController *applController;
    QString currDevName;
    CNFG_BTN_PAIR_TYPE_e cnfgPairType;
    PayloadLib *payloadLib;
    CnfgButton *cnfgBtnPair[3];
    ProcessBar *processBar;
    InfoPage *infoPage;
    qint32 m_maxControlInPage;
    int m_currentElement;

    NavigationControl* m_elementList[255];    

    QMap<quint32,QVariant> m_configResponse;

public:
    explicit ConfigPageControl(QString devName,
                      QWidget *parent = 0,
                      int maxControlInPage = 0,
                      DEV_TABLE_INFO_t *tableInfo = NULL,
                      CNFG_BTN_PAIR_TYPE_e pairType = CNFG_TYPE_DFLT_REF_SAV_BTN);

    ~ConfigPageControl();

    void resetGeometryOfCnfgbuttonRow(quint8 yParam);
    void showHideCnfgBtn(CNFG_BTN_PAIR_e, bool);

    virtual void processDeviceResponse(DevCommParam *param, QString deviceName) = 0;
    virtual void getConfig();
    virtual void defaultConfig();
    virtual void saveConfig();
    virtual void handleInfoPageMessage(int index = INFO_OK_BTN);
    virtual void loadProcessBar();
    virtual bool isUserChangeConfig();
    virtual void loadCamsearchOnAutoConfig();
    virtual void updateList(DevCommParam *param);

    bool takeLeftKeyAction();
    bool takeRightKeyAction();

    void forceFocusToPage(bool isFirstElement);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

    virtual void ctrl_D_KeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);

    void showEvent(QShowEvent *event);   
    int indexOfFstEnable();
    int indexOfLstEnable();
signals:
    void sigOpenCameraFeature(void *param, CAMERA_FEATURE_TYPE_e featureType, quint8 cameraIndex,
                              void *configParam = NULL, QString devName = LOCAL_DEVICE_NAME);

public slots:
    void slotCnfgBtnClicked(int index);
    void slotInfoPageBtnclick(int index);
    void slotUpdateCurrentElement(int index);
};

#endif // CONFIGPAGECONTROL_H
