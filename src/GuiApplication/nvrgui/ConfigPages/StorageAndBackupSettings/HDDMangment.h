#ifndef HDDMANGMENT_H
#define HDDMANGMENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"

#define MAX_HDD_MODE        3
#define MAX_PHY_CELL        5
#define MAX_LOG_CELL        6
#define MAX_HDD_ON_PAGE     5
#define MAX_PHY_HDD         8
#define MAX_LOG_HDD         10
#define MAX_NETWORK_DRIVE   2

typedef struct
{
    QString     m_volumeName;
    QString     m_totalSize;
    QString     m_FreeSpace;
    QString     m_status;
    quint8      m_logStatus;

}LOGICAL_DATA_t;

typedef struct
{
    QString     m_diskName;
    QString     m_serialNumber;
    QString     m_capacity;
    QString     m_phyStatus;

}PHYSICAL_DATA_t;

class HDDMangment : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit HDDMangment(QString deviceName, QWidget *parent = 0,
                         DEV_TABLE_INFO_t *devTabInfo = NULL);

    ~HDDMangment();

    void getConfig ();
    void saveConfig ();
    void defaultConfig ();
    void handleInfoPageMessage(int index);

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void wheelEvent(QWheelEvent *event);

signals:
    
public slots:
    void slotStatusRepTimerTimeout();
    void slotButtonClick(int);
    void slotScrollbarClick(int);
    void slotPhyScrollbarClick(int );

private:

    // private Variable

    QTimer*             statusRepTimer;
    quint8              clickIndex;
    quint8              hddMode;
    quint8              m_prevHddCount;
    bool                m_isPhyScrollActive;
    bool                m_isLogScrollActive;
    quint8              m_firstIndex,m_lastIndex;
    quint8              m_firstPhyIndex,m_LastPhyIndex;
    bool                m_scrollValue;
    quint8              numberOfHdd;

    ElementHeading*     driveHeading;
    DropDown*           driveDropDownBox;
	DropDown*           m_ModeDropDownBox;

    ElementHeading*     modeSelection;

    ElementHeading*     phyDrvHeading;
    BgTile*             phyCellBgTile;
    BgTile*             phyCellBottomBgTile;
    TableCell*          diskCells[MAX_PHY_CELL];
    TextLabel*          diskCellsLabel[MAX_PHY_CELL];
    TableCell*          serialNumbers[MAX_PHY_CELL];
    TextLabel*          serialNumbersLabel[MAX_PHY_CELL];
    TableCell*          capacity[MAX_PHY_CELL];
    TextLabel*          capacityLabel[MAX_PHY_CELL];
    TableCell*          status[MAX_PHY_CELL];
    TextLabel*          statusLabel[MAX_PHY_CELL];

    ElementHeading*     logDrvHeading;
    BgTile*             logCellBgTile;
    BgTile*             logCellBottomBgTile;
    TableCell*          volumCells[MAX_LOG_CELL];
    TextLabel*          volumCellsLabel[MAX_LOG_CELL];
    TableCell*          totalSize[MAX_LOG_CELL];
    TextLabel*          totalSizeLabel[MAX_LOG_CELL];
    TableCell*          freeSize[MAX_LOG_CELL];
    TextLabel*          freeSizeLabel[MAX_LOG_CELL];
    TableCell*          logStatus[MAX_LOG_CELL];
    TextLabel*          logStatusLabel[MAX_LOG_CELL];
    TableCell*          format[MAX_LOG_CELL];
    TextLabel*          formatLabel;
    ControlButton*      formatCntrl[MAX_LOG_CELL-1];
    ControlButton*      abortCntrl[MAX_LOG_CELL-1];

    ScrollBar*          m_phyScrollbar;
	ScrollBar*          m_logScrollBar;
    LOGICAL_DATA_t      logVolumnData[MAX_LOGICAL_VOLUME + MAX_NETWORK_DRIVE + 1];
    PHYSICAL_DATA_t     phyVolumnData[MAX_PHY_HDD + 1];

    // private Function

    void inilizeVariable();
    void createDefaultComponents();

    void createPayload(REQ_MSG_ID_e msgType);
    void getPhysicalVol();
    void getLogicalVol();
    bool getVolumeNumber(QString volumeName, qint8 &volumeNumber);
    void formatVolume();
    void abortCreateProcess();
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds = 0);
    void resetGeometryForLogicalVolumn();
    void resetGeometryForPhysicalDisk();

};

#endif // HDDMANGMENT_H
