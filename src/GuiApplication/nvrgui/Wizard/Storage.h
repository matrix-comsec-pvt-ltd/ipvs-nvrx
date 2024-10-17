#ifndef STORAGE_H
#define STORAGE_H

#include "Controls/Bgtile.h"
#include "Controls/Heading.h"
#include "Controls/ElementHeading.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "Controls/ScrollBar.h"
#include "Controls/MessageBanner.h"
#include "Controls/InfoPage.h"
#include "Controls/DropDown.h"
#include "EnumFile.h"
#include "WizardCommon.h"

#define MAX_LOGICAL_CELL            6
#define MAX_HDD_PAGE                5
#define NO_DISK_FOUND_IMAGE_PATH    ":/Images_Nvrx/NoDiskFound/NoDiskFound.png"

#define WIZ_MAX_HDD_MODE            3
#define WIZ_MAX_PHY_CELL            5
#define WIZ_MAX_PHY_HDD             8
#define WIZ_MAX_LOG_HDD             10

typedef struct
{
    QString     m_volumeName;
    QString     m_totalSize;
    QString     m_FreeSpace;
    QString     m_status;
    quint8      m_logStatus;

}WIZ_LOGICAL_DATA_t;

typedef struct
{
    QString     m_diskName;
    QString     m_serialNumber;
    QString     m_capacity;
    QString     m_phyStatus;

}WIZ_PHYSICAL_DATA_t;


class Storage : public WizardCommon
{
    Q_OBJECT
public:
    explicit Storage(QString devName, QString subHeadStr, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);
    virtual ~Storage();

    void getConfig ();
    void saveConfig ();
    void defaultConfig ();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void wheelEvent(QWheelEvent *event);

    //private
private:
    QTimer                  *statusRepTimer;
    TextLabel               *m_storageHeading;
    ElementHeading*         logDrvHeading;
    BgTile*                 logCellBgTile;
    BgTile*                 logCellBottomBgTile;
    TableCell*              volumCells[MAX_LOGICAL_CELL];
    TextLabel*              volumCellsLabel[MAX_LOGICAL_CELL];
    TableCell*              totalSize[MAX_LOGICAL_CELL];
    TextLabel*              totalSizeLabel[MAX_LOGICAL_CELL];
    TableCell*              freeSize[MAX_LOGICAL_CELL];
    TextLabel*              freeSizeLabel[MAX_LOGICAL_CELL];
    TableCell*              logStatus[MAX_LOGICAL_CELL];
    TextLabel*              logStatusLabel[MAX_LOGICAL_CELL];
    TableCell*              format[MAX_LOGICAL_CELL];
    TextLabel*              formatLabel;
    ControlButton*          formatCntrl[MAX_LOGICAL_CELL-1];
    ControlButton*          abortCntrl[MAX_LOGICAL_CELL-1];
    QString                 currDevName;
    InfoPage                *m_infoPage;
    PayloadLib              *payloadLib;
    ApplController          *applController;
    ScrollBar               *m_logScrollBar;
    ScrollBar               *m_phyScrollbar;
    quint8                  m_firstIndex,m_lastIndex;
    quint8                  clickIndex;
    bool                    m_scrollValue;
    WIZ_LOGICAL_DATA_t      logVolumnData[MAX_LOGICAL_VOLUME + 1];
    WIZ_PHYSICAL_DATA_t     phyVolumnData[WIZ_MAX_PHY_HDD + 1];
    ElementHeading*         phyDrvHeading;
    BgTile*                 phyCellBgTile;
    BgTile*                 phyCellBottomBgTile;
    TableCell*              diskCells[WIZ_MAX_PHY_CELL];
    TextLabel*              diskCellsLabel[WIZ_MAX_PHY_CELL];
    TableCell*              serialNumbers[WIZ_MAX_PHY_CELL];
    TextLabel*              serialNumbersLabel[WIZ_MAX_PHY_CELL];
    TableCell*              capacity[WIZ_MAX_PHY_CELL];
    TextLabel*              capacityLabel[WIZ_MAX_PHY_CELL];
    TableCell*              status[WIZ_MAX_PHY_CELL];
    TextLabel*              statusLabel[WIZ_MAX_PHY_CELL];
    quint8                  m_firstPhyIndex,m_LastPhyIndex;
    quint8                  m_prevHddCount;
    quint8                  hddMode;
    quint8                  numberOfHdd;
    Image                   *m_noDiskFoundImg;
    TextLabel               *m_noDiskFoundLabel;
    TextLabel               *m_noDiskFoundStr;
    TextLabel               *m_noDiskFoundStr1;
    DropDown                *m_ModeDropDownBox;
    DEV_TABLE_INFO_t        devTableInfo;
    QString                 strInfo, strInfo1;
    bool                    m_phyDiskStatus;

    void inilizeVariable();
    void createDefaultElements(QString subHeadStr);
    void createPayload(REQ_MSG_ID_e msgType);
    void getLogicalVol();
    void getPhysicalVol();
    bool getVolumeNumber(QString volumeName, qint8 &volumeNumber);
    void formatVolume();
    void abortCreateProcess ();
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds = 0);
    void resetGeometryForLogicalVolumn();
    void resetGeometryForPhysicalDisk();
    void diskStatus();

signals:

public slots:
    void slotButtonClick(int);
    void slotScrollbarClick(int);
    void slotStatusRepTimerTimeout();
    void slotInfoPageBtnclick(int index);
    void slotPhyScrollbarClick(int numberOfSteps);
};

#endif // STORAGE_H
