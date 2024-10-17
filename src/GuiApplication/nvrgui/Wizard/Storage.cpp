#include <QWheelEvent>
#include "Storage.h"
#include "ValidationMessage.h"

#define NO_DISK_FOUND_STR           "No disk found."
#define HDDCREATING                 "Creating"
#define HEADING_MARGIN              SCALE_WIDTH(25)
#define WIZARD_BG_TILE_HEIGHT       SCALE_HEIGHT(50)
#define WIZARD_BG_TILE_WIDTH        SCALE_WIDTH(995)
#define NO_DISK_FOUND_FOND_COLOR   "#484848"
#define CNFG_FRM_INDEX              1

typedef enum
{
    HDD_DISK_MODE,
    HDD_RECD_FORMAT_CTRL = 3,
    HDD_RECD_FORMAT_CTRL2,
    HDD_RECD_FORMAT_CTRL3,
    HDD_RECD_FORMAT_CTRL4,
    HDD_RECD_FORMAT_CTRL5,
    HDD_RECD_ABORT_CNTRL1,
    HDD_RECD_ABORT_CNTRL2,
    HDD_RECD_ABORT_CNTRL3,
    HDD_RECD_ABORT_CNTRL4,
    HDD_RECD_ABORT_CNTRL5,
    MAX_HDD_MNG_CTRL
}HDD_MNG_CTRL_e;

typedef enum
{
    HDD_MODE_HEADING,
    HDD_RECD_DRV_SPINBOX_STR,
    HDD_PHY_DRV_STR,
    HDD_DISK_HEDING_STR,
    HDD_SRL_NUM_HEDING_STR,
    HDD_CAPCTY_HEDING_STR,
    HDD_STS_HEDING_STR,
    HDD_LOG_VOL_HEADING_STR,
    HDD_LOG_VOL_STR,
    HDD_TOTAL_SIZE_STR,
    HDD_FREE_SIZE_STR,
    HDD_LOG_STS_STR,
    HDD_FORMAT_STR,
    MAX_HDD_MNG_STRG
}HDD_MNG_STRG_e;

typedef enum
{
    LOG_STS_NORMAL,
    LOG_STS_CREATING,
    LOG_STS_FORMATING,
    LOG_STS_FULL,
    LOG_STS_FAULT,
    LOG_STS_INCOMPLETE,
    LOG_STS_CLEANINGUP,
    LOG_STS_NODISK,
    LOG_STS_READ_ONLY,
    LOG_STS_NONE,
    MAX_LOG_STATE
}LOG_STATUS_RES_e;

typedef enum
{
    HDD_SIG_DSK_MODE = 0,
    HDD_RAID0_MODE,
    HDD_RAID1_MODE,
    HDD_RAID5_MODE,
    HDD_RAID10_MODE,
    MAX_HDD_MODE_CTRL
}HDD_MODE_e;

typedef enum
{
    PHY_SERIAL_NUM,
    PHY_CAPACITY_GB,
    PHY_STATUS,
    MAX_PHY_DRV_FEILDS
}PHY_DRV_FEILDS_e;

static const QString hddModeStrings[MAX_HDD_MODE_CTRL] =
{
    "Single Disk Volume",
    "RAID 0",
    "RAID 1",
    "RAID 5",
    "RAID 10",
};

static const QString hddManagmentStrings[MAX_HDD_MNG_STRG] =
{
    "Mode",
    "Recording On Drive",
    "Physical Disks",
    "Disk",
    "Serial Number",
    "Capacity(GB)",
    "Status",
    "Logical Volumes",
    "Volume",
    "Total Size (GB)",
    "Free Space (GB)",
    "Status",
    "Format",
};

static const quint16 modeBtnMargin[WIZ_MAX_HDD_MODE] = {150, 375, 500};

static const QStringList driveList = QStringList() << "Local Drive" <<  "Network Drive 1" << "Network Drive 2";

static const QString phyDrvStatus[] = { "No Disk", "Normal", "Fault", "None"};

static const QStringList driveListName = QStringList()
        << "Single Disk Volume: Drive 1"
        << "Single Disk Volume: Drive 2"
        << "Single Disk Volume: Drive 3"
        << "Single Disk Volume: Drive 4"
        << "Single Disk Volume: Drive 5"
        << "Single Disk Volume: Drive 6"
        << "Single Disk Volume: Drive 7"
        << "Single Disk Volume: Drive 8";

static const QStringList stripRaidVolumeNameForPartitions = QStringList()
        << "Striping Disk Volume 0 : Drive 1 2"
        << "Striping Disk Volume 1 : Drive 1 2"
        << "Striping Disk Volume 2 : Drive 1 2"
        << "Striping Disk Volume 3 : Drive 1 2"
        << "Striping Disk Volume 0 : Drive 3 4"
        << "Striping Disk Volume 1 : Drive 3 4"
        << "Striping Disk Volume 2 : Drive 3 4"
        << "Striping Disk Volume 3 : Drive 3 4"
        << "Striping Disk Volume 0 : Drive 5 6"
        << "Striping Disk Volume 1 : Drive 5 6"
        << "Striping Disk Volume 2 : Drive 5 6"
        << "Striping Disk Volume 3 : Drive 5 6"
        << "Striping Disk Volume 0 : Drive 7 8"
        << "Striping Disk Volume 1 : Drive 7 8"
        << "Striping Disk Volume 2 : Drive 7 8"
        << "Striping Disk Volume 3 : Drive 7 8";

static const QStringList mirrorRaidVolumeNameForPartitions = QStringList()
        << "Mirroring Disk Volume 0 : Drive 1 2"
        << "Mirroring Disk Volume 1 : Drive 1 2"
        << "Mirroring Disk Volume 2 : Drive 1 2"
        << "Mirroring Disk Volume 3 : Drive 1 2"
        << "Mirroring Disk Volume 0 : Drive 3 4"
        << "Mirroring Disk Volume 1 : Drive 3 4"
        << "Mirroring Disk Volume 2 : Drive 3 4"
        << "Mirroring Disk Volume 3 : Drive 3 4"
        << "Mirroring Disk Volume 0 : Drive 5 6"
        << "Mirroring Disk Volume 1 : Drive 5 6"
        << "Mirroring Disk Volume 2 : Drive 5 6"
        << "Mirroring Disk Volume 3 : Drive 5 6"
        << "Mirroring Disk Volume 0 : Drive 7 8"
        << "Mirroring Disk Volume 1 : Drive 7 8"
        << "Mirroring Disk Volume 2 : Drive 7 8"
        << "Mirroring Disk Volume 3 : Drive 7 8";

static const QStringList raid5with3DiskVolNameForPartitions = QStringList()
        << "RAID 5 Volume 0 : Drive 1 2 3"
        << "RAID 5 Volume 1 : Drive 1 2 3"
        << "RAID 5 Volume 2 : Drive 1 2 3"
        << "RAID 5 Volume 3 : Drive 1 2 3"
        << "RAID 5 Volume 0 : Drive 5 6 7"
        << "RAID 5 Volume 1 : Drive 5 6 7"
        << "RAID 5 Volume 2 : Drive 5 6 7"
        << "RAID 5 Volume 3 : Drive 5 6 7";

static const QStringList raid5with4DiskVolNameForPartitions = QStringList()
        << "RAID 5 Volume 0 : Drive 1 2 3 4"
        << "RAID 5 Volume 1 : Drive 1 2 3 4"
        << "RAID 5 Volume 2 : Drive 1 2 3 4"
        << "RAID 5 Volume 3 : Drive 1 2 3 4"
        << "RAID 5 Volume 0 : Drive 5 6 7 8"
        << "RAID 5 Volume 1 : Drive 5 6 7 8"
        << "RAID 5 Volume 2 : Drive 5 6 7 8"
        << "RAID 5 Volume 3 : Drive 5 6 7 8";

static const QStringList raid10VolNameForPartitions = QStringList()
        << "RAID 10 Volume 0 : Drive 1 2 3 4"
        << "RAID 10 Volume 1 : Drive 1 2 3 4"
        << "RAID 10 Volume 2 : Drive 1 2 3 4"
        << "RAID 10 Volume 3 : Drive 1 2 3 4"
        << "RAID 10 Volume 0 : Drive 5 6 7 8"
        << "RAID 10 Volume 1 : Drive 5 6 7 8"
        << "RAID 10 Volume 2 : Drive 5 6 7 8"
        << "RAID 10 Volume 3 : Drive 5 6 7 8";

static const QStringList networkdriveList = QStringList()
        << "Network Drive 1"
        << "Network Drive 2";

static const QString logDrvResponse[]={"Normal",
                                       "Creating",
                                       "Formatting",
                                       "Full",
                                       "Fault",
                                       "Incomplete Volume",
                                       "Cleanup",
                                       "Not Connected",
                                       "Read Only",
                                       "None"};

typedef enum
{
    HDD_MODE,
    HDD_RECORDING_DRIVE,
    MAX_HDD_TABLE_FEILDS
}HDD_TABLE_FEILDS_e;

static const QStringList logResponce = QStringList()
                                    << "Creating"
                                    << "Formatting"
                                    << "Cleanup"
                                    << "Not Connected"
                                    << "Read Only"
                                    << "None";

Storage::Storage(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId) :WizardCommon(parent, pageId)
{
    m_phyDiskStatus = false;
    currDevName = devName;
    hddMode = 0;
    numberOfHdd = 0;
    m_lastIndex = 0;
    m_prevHddCount = 0;
    m_scrollValue = false;
    m_logScrollBar = NULL;
    m_phyScrollbar = NULL;
    m_firstIndex = 0;
    m_firstPhyIndex = 0;
    m_LastPhyIndex = 0;
    createDefaultElements(subHeadStr);

    statusRepTimer = new QTimer(this);
    connect (statusRepTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotStatusRepTimerTimeout()));
    statusRepTimer->setInterval (10000);

    getConfig ();
    getPhysicalVol();
}

Storage::~Storage()
{
    if(IS_VALID_OBJ(statusRepTimer))
    {
        statusRepTimer->stop ();
        DELETE_OBJ(statusRepTimer);
    }

    for(quint8 index = 0 ; index < WIZ_MAX_PHY_CELL; index++)
    {
        if(IS_VALID_OBJ(diskCells[index]))
        {
            DELETE_OBJ(diskCells[index]);
            DELETE_OBJ(diskCellsLabel[index]);
            DELETE_OBJ(serialNumbers[index]);
            DELETE_OBJ(serialNumbersLabel[index]) ;
            DELETE_OBJ(capacity[index]);
            DELETE_OBJ(capacityLabel[index]);
            DELETE_OBJ(status[index]);
            DELETE_OBJ(statusLabel[index]);
        }
    }

    DELETE_OBJ (logDrvHeading);
    for(quint8 index = 0; index < MAX_LOGICAL_CELL; index++)
    {
        if(IS_VALID_OBJ(volumCells[index]))
        {
            DELETE_OBJ(volumCells[index]);
            DELETE_OBJ(volumCellsLabel[index]);
            DELETE_OBJ(totalSize[index]);
            DELETE_OBJ(totalSizeLabel[index]);
            DELETE_OBJ(freeSize[index]);
            DELETE_OBJ(freeSizeLabel[index]);
            DELETE_OBJ(logStatus[index]);
            DELETE_OBJ(logStatusLabel[index]);
            DELETE_OBJ(format[index]);

            if(index > 0)
            {
                disconnect (formatCntrl[index-1],
                        SIGNAL(sigButtonClick(int)),
                        this,
                        SLOT(slotButtonClick(int)));
                DELETE_OBJ(formatCntrl[(index - 1)]) ;
            }
        }
    }

    DELETE_OBJ(phyCellBgTile);
    DELETE_OBJ(phyCellBottomBgTile);
    DELETE_OBJ(logCellBgTile);
    DELETE_OBJ(logCellBottomBgTile);
    DELETE_OBJ(formatLabel);
    DELETE_OBJ(m_ModeDropDownBox);

    if(IS_VALID_OBJ(m_logScrollBar))
    {
        disconnect(m_logScrollBar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScrollbarClick(int)));
        DELETE_OBJ(m_logScrollBar);
    }

    if(IS_VALID_OBJ(m_phyScrollbar))
    {
        disconnect (m_phyScrollbar,
                SIGNAL(sigScroll(int)),
                this,
                SLOT(slotPhyScrollbarClick(int)));
        DELETE_OBJ(m_phyScrollbar);
    }

    DELETE_OBJ(m_noDiskFoundImg);
    DELETE_OBJ(m_noDiskFoundLabel);
    DELETE_OBJ(m_noDiskFoundStr);
    DELETE_OBJ(m_noDiskFoundStr1);
    DELETE_OBJ(m_infoPage)
    DELETE_OBJ(payloadLib);
    DELETE_OBJ(m_storageHeading);
    DELETE_OBJ(phyDrvHeading);

    m_firstIndex = m_lastIndex = 0;
}

void Storage::createDefaultElements(QString subHeadStr)
{
    QMap<quint8, QString> modeList;

    applController = ApplController::getInstance ();
    applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTableInfo);
    payloadLib = new PayloadLib();

    m_storageHeading = new TextLabel((SCALE_WIDTH(500)),
                                     (SCALE_HEIGHT(23)),
                                     SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                     subHeadStr,
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_START_X_CENTRE_Y,
                                     0,
                                     false,
                                     WIZARD_BG_TILE_WIDTH,
                                     0);
    logCellBgTile = NULL;
    logCellBottomBgTile = NULL;
    phyCellBgTile = NULL;
    phyCellBottomBgTile = NULL;

    for(quint8 index = 0 ; index < WIZ_MAX_PHY_CELL; index++ )
    {
        diskCells[index] = NULL;
        diskCellsLabel[index] = NULL;
        serialNumbers[index] =  NULL;
        serialNumbersLabel[index] = NULL;
        capacity[index] = NULL;
        capacityLabel[index] = NULL;
        status[index] = NULL;
        statusLabel[index] = NULL;
    }

    for(quint8 index = 0; index < MAX_LOGICAL_CELL; index++)
    {
        volumCells[index] = NULL;
        volumCellsLabel[index] = NULL;
        totalSize[index] = NULL;
        totalSizeLabel[index] = NULL;
        freeSize[index] = NULL;
        freeSizeLabel[index] = NULL;
        logStatus[index] = NULL;
        logStatusLabel[index] = NULL;
        format[index] = NULL;

        if(index < (MAX_LOGICAL_CELL - 1))
        {
            formatCntrl[index] = NULL;
        }
    }

    clickIndex = MAX_HDD_MNG_CTRL;
    modeList.clear();
    quint8 suppMode = (devTableInfo.numOfHdd >= 4) ? HDD_RAID10_MODE : ((devTableInfo.numOfHdd >= 2) ? HDD_RAID1_MODE : HDD_SIG_DSK_MODE);
    for(quint8 index = 0; index <= suppMode; index++)
    {
        modeList.insert (index, hddModeStrings[index]);
    }

    m_ModeDropDownBox = new DropDown(SCALE_WIDTH(40),
                                     SCALE_HEIGHT(40),
                                     WIZARD_BG_TILE_WIDTH,
                                     WIZARD_BG_TILE_HEIGHT,
                                     0,
                                     DROPDOWNBOX_SIZE_225,
                                     "Mode",
                                     modeList,
                                     this,
                                     "",
                                     false,
                                     SCALE_WIDTH(350),
                                     COMMON_LAYER);
    m_ModeDropDownBox->hide();

    phyDrvHeading = new ElementHeading(SCALE_WIDTH(40),
                                       SCALE_HEIGHT(95),
                                       WIZARD_BG_TILE_WIDTH,
                                       SCALE_HEIGHT(30),
                                       hddManagmentStrings[HDD_PHY_DRV_STR],
                                       TOP_LAYER,
                                       this,
                                       false,
                                       HEADING_MARGIN, NORMAL_FONT_SIZE, true);
    phyDrvHeading->hide();

    phyCellBgTile = new BgTile(phyDrvHeading->x (),
                               phyDrvHeading->y() + phyDrvHeading->height (),
                               WIZARD_BG_TILE_WIDTH,
                               (WIZ_MAX_PHY_CELL-1)* BGTILE_HEIGHT,
                               MIDDLE_LAYER,
                               this);
    phyCellBgTile->hide();

    phyCellBottomBgTile = new BgTile(phyCellBgTile->x (),
                                     phyCellBgTile->y() + phyCellBgTile->height (),
                                     WIZARD_BG_TILE_WIDTH,
                                     SCALE_HEIGHT(10),
                                     DOWN_LAYER,
                                     this);
    phyCellBottomBgTile->hide();

    for(quint8 index = 0 ; index < WIZ_MAX_PHY_CELL; index++)
    {
        diskCells[index] = new TableCell(phyDrvHeading->x () + SCALE_WIDTH(28),
                                         phyDrvHeading->y() + TABELCELL_HEIGHT*(index) + phyDrvHeading->height (),
                                         TABELCELL_SMALL_SIZE_WIDTH,
                                         TABELCELL_HEIGHT,
                                         this,
                                         (index == 0) ? true : false);
        diskCells[index]->hide();

        diskCellsLabel[index] = new TextLabel(diskCells[index]->x () + SCALE_WIDTH(10),
                                              diskCells[index]->y () + TABELCELL_HEIGHT/2,
                                              NORMAL_FONT_SIZE,
                                              (index == 0) ? hddManagmentStrings[HDD_DISK_HEDING_STR] : "",
                                              this,
                                              (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y);
        diskCellsLabel[index]->hide();

        serialNumbers[index] = new TableCell(diskCells[index]-> x () + TABELCELL_SMALL_SIZE_WIDTH,
                                             phyDrvHeading->y() + TABELCELL_HEIGHT*(index) +  phyDrvHeading->height (),
                                             TABELCELL_EXTRALARGE_SIZE_WIDTH + TABELCELL_EXTRASMALL_SIZE_WIDTH ,
                                             TABELCELL_HEIGHT,
                                             this,
                                             (index == 0) ? true : false);
        serialNumbers[index]->hide();

        serialNumbersLabel[index] = new TextLabel(serialNumbers[index]->x () + SCALE_WIDTH(10),
                                                  serialNumbers[index]->y () + TABELCELL_HEIGHT/2,
                                                  NORMAL_FONT_SIZE,
                                                  (index == 0) ? hddManagmentStrings[HDD_SRL_NUM_HEDING_STR] : "",
                                                  this,
                                                  (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y);
        serialNumbersLabel[index]->hide();

        capacity[index] = new TableCell(serialNumbers[index]-> x () + TABELCELL_EXTRALARGE_SIZE_WIDTH + TABELCELL_EXTRASMALL_SIZE_WIDTH,
                                        phyDrvHeading->y() + TABELCELL_HEIGHT*(index) +  phyDrvHeading->height (),
                                        TABELCELL_SMALL_SIZE_WIDTH,
                                        TABELCELL_HEIGHT,
                                        this,
                                        (index == 0) ? true : false);
        capacity[index]->hide();

        capacityLabel[index] = new TextLabel(capacity[index]->x () + SCALE_WIDTH(10),
                                             capacity[index]->y () + TABELCELL_HEIGHT/2,
                                             NORMAL_FONT_SIZE,
                                             (index == 0) ? hddManagmentStrings[HDD_CAPCTY_HEDING_STR] : "",
                                             this,
                                             (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y);
        capacityLabel[index]->hide();

        status[index] = new TableCell(capacity[index]-> x () + TABELCELL_SMALL_SIZE_WIDTH,
                                      phyDrvHeading->y() + TABELCELL_HEIGHT*(index) +  phyDrvHeading->height (),
                                      TABELCELL_EXTRASMALL_SIZE_WIDTH + SCALE_WIDTH(23),
                                      TABELCELL_HEIGHT,
                                      this,
                                      (index == 0) ? true : false);
        status[index]->hide();

        statusLabel[index] = new TextLabel(status[index]->x () + SCALE_WIDTH(10),
                                           status[index]->y () + TABELCELL_HEIGHT/2,
                                           NORMAL_FONT_SIZE,
                                           (index == 0) ? hddManagmentStrings[HDD_STS_HEDING_STR] : "",
                                           this,
                                           (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y);
        statusLabel[index]->hide();
    }

    logDrvHeading = new ElementHeading(phyCellBottomBgTile->x (),
                                       phyCellBottomBgTile->y () + phyCellBottomBgTile->height () + SCALE_HEIGHT(5),
                                       WIZARD_BG_TILE_WIDTH,
                                       SCALE_HEIGHT(30),
                                       hddManagmentStrings[HDD_LOG_VOL_HEADING_STR],
                                       TOP_LAYER,
                                       this,
                                       false,
                                       HEADING_MARGIN, NORMAL_FONT_SIZE, true);
    logDrvHeading->hide();

    logCellBgTile = new BgTile(logDrvHeading->x (),
                               logDrvHeading->y() + logDrvHeading->height (),
                               WIZARD_BG_TILE_WIDTH,
                               (MAX_LOGICAL_CELL - 1)*BGTILE_HEIGHT,
                               MIDDLE_LAYER,
                               this);
    logCellBgTile->hide();

    logCellBottomBgTile = new BgTile(logDrvHeading->x (),
                                     logDrvHeading->y() + logDrvHeading->height (),
                                     WIZARD_BG_TILE_WIDTH,
                                     ((MAX_LOGICAL_CELL - 1)*BGTILE_HEIGHT ),
                                     DOWN_LAYER,
                                     this);
    logCellBottomBgTile->hide();

    for(quint8 index = 0; index < MAX_LOGICAL_CELL; index++)
    {
        volumCells[index] = new TableCell(logDrvHeading->x () + SCALE_WIDTH(28),
                                          logDrvHeading->y() + TABELCELL_HEIGHT*(index) + logDrvHeading->height (),
                                          TABELCELL_LARGE_SIZE_WIDTH,
                                          TABELCELL_HEIGHT,
                                          this,
                                          (index == 0) ? true : false);
        volumCells[index]->hide();

        volumCellsLabel[index] = new TextLabel(volumCells[index]->x () + SCALE_WIDTH(10),
                                               volumCells[index]->y () + TABELCELL_HEIGHT/2,
                                               NORMAL_FONT_SIZE,
                                               (index == 0) ? hddManagmentStrings[HDD_LOG_VOL_STR] : "",
                                               this,
                                               (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y);
        volumCellsLabel[index]->hide();

        totalSize[index] = new TableCell(volumCells[index]-> x () + TABELCELL_LARGE_SIZE_WIDTH,
                                         logDrvHeading->y() + TABELCELL_HEIGHT*(index) +  logDrvHeading->height (),
                                         TABELCELL_SMALL_SIZE_WIDTH,
                                         TABELCELL_HEIGHT,
                                         this,
                                         (index == 0) ? true : false);
        totalSize[index]->hide();

        totalSizeLabel[index] = new TextLabel(totalSize[index]->x () + SCALE_WIDTH(10),
                                              totalSize[index]->y () + TABELCELL_HEIGHT/2,
                                              NORMAL_FONT_SIZE,
                                              (index == 0) ? hddManagmentStrings[HDD_TOTAL_SIZE_STR] : "",
                                              this,
                                              (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y);
        totalSizeLabel[index]->hide();

        freeSize[index] = new TableCell(totalSize[index]-> x () + TABELCELL_SMALL_SIZE_WIDTH,
                                        logDrvHeading->y() + TABELCELL_HEIGHT*(index) +  logDrvHeading->height (),
                                        TABELCELL_SMALL_SIZE_WIDTH + SCALE_WIDTH(12),
                                        TABELCELL_HEIGHT,
                                        this,
                                        (index == 0) ? true : false);
        freeSize[index]->hide();

        freeSizeLabel[index] = new TextLabel(freeSize[index]->x () + (SCALE_WIDTH(10)),
                                             freeSize[index]->y () + TABELCELL_HEIGHT/2,
                                             NORMAL_FONT_SIZE,
                                             (index == 0) ? hddManagmentStrings[HDD_FREE_SIZE_STR] : "",
                                             this,
                                             (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y);
        freeSizeLabel[index]->hide();

        logStatus[index] = new TableCell(freeSize[index]-> x () + TABELCELL_SMALL_SIZE_WIDTH + SCALE_WIDTH(12),
                                         logDrvHeading->y() + TABELCELL_HEIGHT*(index) +  logDrvHeading->height (),
                                         TABELCELL_MEDIUM_SIZE_WIDTH,
                                         TABELCELL_HEIGHT,
                                         this,
                                         (index == 0) ? true : false);
        logStatus[index]->hide();

        logStatusLabel[index] = new TextLabel(logStatus[index]->x () + SCALE_WIDTH(10),
                                              logStatus[index]->y () + TABELCELL_HEIGHT/2,
                                              NORMAL_FONT_SIZE,
                                              (index == 0) ? hddManagmentStrings[HDD_LOG_STS_STR] : "",
                                              this,
                                              (index == 0) ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y);
        logStatusLabel[index]->hide();

        format[index] = new TableCell(logStatus[index]-> x () + TABELCELL_MEDIUM_SIZE_WIDTH,
                                      logDrvHeading->y() + TABELCELL_HEIGHT*(index) +  logDrvHeading->height (),
                                      SCALE_WIDTH(78),
                                      TABELCELL_HEIGHT,
                                      this,
                                      (index == 0) ? true : false);
        format[index]->hide();

        if(index > 0)
        {
            abortCntrl[(index -1)] = new ControlButton(ABORT_BUTTON_INDEX,
                                                       logStatus[index]->x () +
                                                       logStatus[index]->width () - SCALE_WIDTH(35),
                                                       logStatus[index]->y () - SCALE_HEIGHT(5),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       this,
                                                       NO_LAYER,
                                                       -1,
                                                       "",
                                                       true,
                                                       HDD_RECD_ABORT_CNTRL1 + (index - 1),
                                                       false);
            abortCntrl[(index - 1)]->setVisible (false);
            connect (abortCntrl[index-1],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));

            formatCntrl[(index - 1)] = new ControlButton(FORMAT_BUTTON_INDEX,
                                                         format[index]->x () + SCALE_WIDTH(25),
                                                         format[index]->y () - SCALE_HEIGHT(5),
                                                         BGTILE_SMALL_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         this,
                                                         NO_LAYER,
                                                         -1,
                                                         "",
                                                         true,
                                                         HDD_RECD_FORMAT_CTRL + (index - 1),
                                                         false);
            formatCntrl[(index - 1)]->setVisible (false);
            connect (formatCntrl[index-1],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        }
    }

    if(devTableInfo.numOfHdd > MAX_HDD_PAGE -1)
    {
        m_firstPhyIndex = 0;
        m_LastPhyIndex = (WIZ_MAX_PHY_CELL-1);
        m_phyScrollbar = new ScrollBar((phyDrvHeading->x () + TABELCELL_EXTRALARGE_SIZE_WIDTH + SCALE_WIDTH(510)),
                                       (phyDrvHeading->y () + phyDrvHeading->height () ),
                                       SCALE_WIDTH(13),
                                       4,
                                       SCALE_HEIGHT(37),
                                       (devTableInfo.numOfHdd),
                                       m_firstPhyIndex,
                                       this,
                                       VERTICAL_SCROLLBAR,
                                       255,
                                       (devTableInfo.numOfHdd > (MAX_HDD_PAGE -1)),
                                       false);
        connect(m_phyScrollbar,
                SIGNAL(sigScroll(int)),
                this,
                SLOT(slotPhyScrollbarClick(int)));
        m_phyScrollbar->setIsEnabled (false);
        m_phyScrollbar->setVisible (false);
    }

    m_logScrollBar = new ScrollBar((volumCells[0]->x () + TABELCELL_EXTRALARGE_SIZE_WIDTH + SCALE_WIDTH(481)),
                                   (volumCells[0]->y ()),
                                   SCALE_WIDTH(13),
                                   5,
                                   SCALE_HEIGHT(178),
                                   1,
                                   m_firstIndex,
                                   this,
                                   VERTICAL_SCROLLBAR,
                                   255,
                                   false,
                                   false);
    connect(m_logScrollBar,
            SIGNAL(sigScroll(int)),
            this,
            SLOT(slotScrollbarClick(int)));
    m_logScrollBar->setIsEnabled (false);
    m_logScrollBar->setVisible (false);

    formatLabel = new TextLabel(format[0]->x () + SCALE_WIDTH(10),
                                format[0]->y () + TABELCELL_HEIGHT/2,
                                NORMAL_FONT_SIZE,
                                hddManagmentStrings[HDD_FORMAT_STR],
                                this,
                                HIGHLITED_FONT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y);
    formatLabel->hide();

    m_noDiskFoundImg = new Image(SCALE_WIDTH (530),
                                 SCALE_HEIGHT(200),
                                 QString(NO_DISK_FOUND_IMAGE_PATH),
                                 this,
                                 CENTER_X_CENTER_Y,
                                 0,
                                 false,
                                 true,
                                 true);
    m_noDiskFoundImg->hide();

    int textLabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(NO_DISK_FOUND_STR);
    m_noDiskFoundLabel = new TextLabel(SCALE_WIDTH (525),
                                       SCALE_HEIGHT(150),
                                       SCALE_FONT(20),
                                       NO_DISK_FOUND_STR,
                                       this,
                                       NO_DISK_FOUND_FOND_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_CENTER_Y, 0, false, (textLabelWidth + SCALE_WIDTH (50)));
    m_noDiskFoundLabel->SetBold(true);
    m_noDiskFoundLabel->hide();

    strInfo = QString("You can proceed with the setup process but it is highly recommended to connect");
    strInfo1 = QString("a storage drive to save camera recordings.");

    m_noDiskFoundStr = new TextLabel(SCALE_WIDTH(250) , SCALE_HEIGHT(350), SCALE_FONT(14), strInfo, this);
    m_noDiskFoundStr->hide();

    m_noDiskFoundStr1 = new TextLabel( SCALE_WIDTH(375) , SCALE_HEIGHT(370), SCALE_FONT(14), strInfo1, this);
    m_noDiskFoundStr1->hide();

    m_infoPage = new InfoPage(0, 0,
                              SCALE_WIDTH(1145),
                              SCALE_HEIGHT(750),
                              MAX_INFO_PAGE_TYPE,
                              parentWidget());
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    this->show();
}

void Storage::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             HDD_MANAGMENT_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX);
    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Storage::defaultConfig ()
{
    createPayload(MSG_DEF_CFG);
}

void Storage::getConfig ()
{
    createPayload(MSG_GET_CFG);
}

void Storage::saveConfig ()
{
    if (m_getConfig == false)
    {
        return;
    }

    quint8 tempMode = m_ModeDropDownBox->getIndexofCurrElement();
    if ((tempMode != hddMode) && (tempMode == HDD_SIG_DSK_MODE) && (hddMode != HDD_RAID0_MODE))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(HDD_MANAGE_FORMAT_HDD_USE_MAZ_SIZE));
    }

    hddMode = tempMode;
    payloadLib->setCnfgArrayAtIndex(HDD_MODE, hddMode);
    createPayload(MSG_SET_CFG);
}

void Storage::getPhysicalVol()
{
    if(!statusRepTimer->isActive ())
    {
        statusRepTimer->start ();
    }

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = PHYSICAL_DISK_STS;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Storage::getLogicalVol()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = LOGICAL_VOLUME_STS;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

bool Storage::getVolumeNumber(QString volumeName, qint8 &volumeNumber)
{
    /* Is single disk volume? */
    volumeNumber = driveListName.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is network drive volume? */
    volumeNumber = networkdriveList.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        volumeNumber += MAX_LOGICAL_VOLUME;
        return true;
    }

    /* Is RAID0 volume? */
    volumeNumber = stripRaidVolumeNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID1 volume? */
    volumeNumber = mirrorRaidVolumeNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID5 disk-3 volume? */
    volumeNumber = raid5with3DiskVolNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID5 disk-4 volume? */
    volumeNumber = raid5with4DiskVolNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID10 volume? */
    volumeNumber = raid10VolNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Volume not found */
    return false;
}

void Storage::formatVolume()
{
    qint8 volumeNumber = 0;

    if (false == getVolumeNumber(logVolumnData[clickIndex].m_volumeName, volumeNumber))
    {
        return;
    }

    volumeNumber += 1;
    payloadLib->setCnfgArrayAtIndex(0, volumeNumber);
    sendCommand(HDDFORMAT, 1);
}

void Storage::abortCreateProcess()
{
    qint8 volumeNumber = 0;

    if (false == getVolumeNumber(logVolumnData[clickIndex].m_volumeName, volumeNumber))
    {
        return;
    }

    volumeNumber += 1;
    payloadLib->setCnfgArrayAtIndex(0, volumeNumber);
    sendCommand(STOP_RAID, 1);
}

void Storage::sendCommand(SET_COMMAND_e cmdType, int totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void Storage::slotStatusRepTimerTimeout()
{
    getPhysicalVol();
}

void Storage::slotButtonClick (int index)
{
    if ((index >= HDD_RECD_FORMAT_CTRL) && (index < HDD_RECD_ABORT_CNTRL1))
    {
        clickIndex = (index - HDD_RECD_FORMAT_CTRL) + 1 + m_firstIndex;
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(HDD_MANAGE_DO_REALY_WANT_FORMAT), true);
    }
    else if ((index >= HDD_RECD_ABORT_CNTRL1) && (index < MAX_HDD_MNG_CTRL))
    {
        clickIndex = (index - HDD_RECD_ABORT_CNTRL1) + 1 + m_firstIndex;
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(HDD_MANAGE_PROCESS_NOT_ABORTED_CHANGE_HDD),true);
    }
}

void Storage::slotScrollbarClick(int numberOfSteps)
{
    m_firstIndex += numberOfSteps;
    m_lastIndex += numberOfSteps;
    m_scrollValue = true;
    resetGeometryForLogicalVolumn();
}

void Storage::slotPhyScrollbarClick(int numberOfSteps)
{
    m_firstPhyIndex += numberOfSteps;
    m_LastPhyIndex += numberOfSteps;
    resetGeometryForPhysicalDisk();
}

void Storage::resetGeometryForLogicalVolumn()
{
    qint8 matchIndex;
    bool  isCreating = false;

    for(quint8 index = 1; index < MAX_LOGICAL_CELL; index++)
    {
        if ((index < m_firstIndex) && (index > m_lastIndex))
        {
            continue;
        }

        if((logVolumnData[m_firstIndex+index].m_volumeName) != "")
        {
            volumCellsLabel[index]->changeText (logVolumnData[m_firstIndex+index].m_volumeName);
            totalSizeLabel[index]->changeText (logVolumnData[m_firstIndex+index].m_totalSize);
            freeSizeLabel[index]->changeText (logVolumnData[m_firstIndex+index].m_FreeSpace);
            logStatusLabel[index]->changeText (logVolumnData[m_firstIndex+index].m_status);
            isCreating = logStatusLabel[index]->getText().contains(HDDCREATING,Qt::CaseInsensitive);
            if(abortCntrl[index-1]->isVisible() && (m_scrollValue == true)) // cancle button up-down when scrollbar show
            {
                if(m_firstIndex > 0)
                {
                    abortCntrl[index-1]->setVisible (true);
                    abortCntrl[index-1]->setIsEnabled (true);
                    m_scrollValue = false;
                }
                else
                {
                    abortCntrl[index - 1]->setVisible (true);
                    abortCntrl[index - 1]->setIsEnabled (true);
                    m_scrollValue = false;
                }
                abortCntrl[index-1]->setVisible (false);
                abortCntrl[index-1]->setIsEnabled (false);
            }

            // format button show - hide when scroll.
            formatCntrl[index-1]->setVisible (true);
            formatCntrl[index-1]->setIsEnabled (true);

            if(((matchIndex = logResponce.indexOf(logStatusLabel[index]->getText())) != -1) || (isCreating))
            {
                formatCntrl[index-1]->setVisible (false);
                formatCntrl[index-1]->setIsEnabled (false);
            }
        }

        if(IS_VALID_OBJ(volumCellsLabel[index]))
        {
            volumCellsLabel[index]->update ();
            totalSizeLabel[index]->update ();
            freeSizeLabel[index]->update ();
            logStatusLabel[index]->update ();
        }
    }
}

void Storage::resetGeometryForPhysicalDisk()
{
    for(quint8 index = 1; index < WIZ_MAX_PHY_CELL; index++)
    {
        if((index < m_firstPhyIndex) && (index > m_LastPhyIndex))
        {
            continue;
        }

        if((phyVolumnData[m_firstPhyIndex+index].m_diskName) == "")
        {
            continue;
        }

        if(IS_VALID_OBJ(diskCellsLabel[index]))
        {
            diskCellsLabel[index]->changeText (phyVolumnData[m_firstPhyIndex+index].m_diskName);
            serialNumbersLabel[index]->changeText (phyVolumnData[m_firstPhyIndex+index].m_serialNumber);
            capacityLabel[index]->changeText (phyVolumnData[m_firstPhyIndex+index].m_capacity);
            statusLabel[index]->changeText (phyVolumnData[m_firstPhyIndex+index].m_phyStatus);
        }

        if(IS_VALID_OBJ(diskCellsLabel[index]))
        {
            diskCellsLabel[index]->update ();
            serialNumbersLabel[index]->update ();
            capacityLabel[index]->update ();
            statusLabel[index]->update ();
        }
    }
}

void Storage::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        m_ModeDropDownBox->setCurrValue(hddManagmentStrings[1]);
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_SET_CMD:
        {
            payloadLib->parseDevCmdReply(true, param->payload);
            switch(param->cmdType)
            {
                case PHYSICAL_DISK_STS:
                {
                    for(quint8 loop = 0; loop < (devTableInfo.numOfHdd) ; loop++)
                    {
                        phyVolumnData[loop+1].m_diskName = (QString("Disk") + QString(" ") + QString("%1").arg (loop + 1));
                        if((payloadLib->getCnfgArrayAtIndex (loop*MAX_PHY_DRV_FEILDS + PHY_SERIAL_NUM).toString ()) != "")
                        {
                            m_phyDiskStatus = true;
                            phyVolumnData[loop+1].m_serialNumber = (payloadLib->getCnfgArrayAtIndex (loop*MAX_PHY_DRV_FEILDS + PHY_SERIAL_NUM).toString ());
                            phyVolumnData[loop+1].m_capacity= (payloadLib->getCnfgArrayAtIndex (loop*MAX_PHY_DRV_FEILDS + PHY_CAPACITY_GB).toString ());
                            phyVolumnData[loop+1].m_phyStatus= (phyDrvStatus[payloadLib->getCnfgArrayAtIndex (loop*MAX_PHY_DRV_FEILDS + PHY_STATUS).toUInt ()]);
                        }
                        else
                        {
                            phyVolumnData[loop+1].m_serialNumber = "-- ";
                            phyVolumnData[loop+1].m_capacity = "-- ";
                            phyVolumnData[loop+1].m_phyStatus = "-- ";
                        }
                    }

                    numberOfHdd = (devTableInfo.numOfHdd < WIZ_MAX_PHY_CELL) ? devTableInfo.numOfHdd : (WIZ_MAX_PHY_CELL - 1);
                    for(quint8 index = 0; index < numberOfHdd; index++)
                    {
                        diskCellsLabel[index + 1]->changeText (QString("Disk") + QString(" ") + QString("%1").arg (m_firstPhyIndex + 1 + index));
                        if(phyVolumnData[m_firstPhyIndex + 1 + index].m_serialNumber != "")
                        {
                           serialNumbersLabel[index + 1]->changeText (phyVolumnData[m_firstPhyIndex + 1 + index].m_serialNumber);
                           capacityLabel[index + 1]->changeText (phyVolumnData[m_firstPhyIndex + 1 + index].m_capacity);
                           statusLabel[index + 1]->changeText (phyVolumnData[m_firstPhyIndex + 1 + index].m_phyStatus);
                        }
                        else
                        {
                            serialNumbersLabel[index + 1]->changeText("-- ");
                            capacityLabel[index + 1]->changeText("-- ");
                            statusLabel[index + 1]->changeText("-- ");
                        }

                        diskCellsLabel[index + 1]->update ();
                        serialNumbersLabel[index + 1]->update ();
                        capacityLabel[index + 1]->update ();
                        statusLabel[index + 1]->update ();
                    }

                    getLogicalVol();
                }
                break;

                case LOGICAL_VOLUME_STS:
                {
                    bool    isAnyCreation = false;
                    quint8  logDiskCount = 0;
                    quint8  loop,index;
                    quint8  logSts;
                    QString volumName;

                    for(loop = 0; loop < MAX_LOGICAL_VOLUME; loop++)
                    {
                        logVolumnData[loop+1].m_volumeName = "";
                        logVolumnData[loop+1].m_totalSize = "";
                        logVolumnData[loop+1].m_FreeSpace = "";
                        logVolumnData[loop+1].m_status = "";
                        logVolumnData[loop+1].m_logStatus = MAX_LOG_STATE;

                        volumName = payloadLib->getCnfgArrayAtIndex(loop*MAX_LOG_DRV_FEILDS + LOG_VOL_NAME).toString();
                        if (volumName == "")
                        {
                            continue;
                        }

                        if ((volumName != networkdriveList.at(0)) && (volumName != networkdriveList.at(1)))
                        {
                            logVolumnData[loop+1].m_volumeName = volumName;
                            logVolumnData[loop+1].m_totalSize = (payloadLib->getCnfgArrayAtIndex (loop*MAX_LOG_DRV_FEILDS + LOG_TOTAL_SIZE).toString ());
                            logVolumnData[loop+1].m_FreeSpace = (payloadLib->getCnfgArrayAtIndex (loop*MAX_LOG_DRV_FEILDS + LOG_FREE_SIZE).toString ());
                            logVolumnData[loop+1].m_logStatus = payloadLib->getCnfgArrayAtIndex (loop*MAX_LOG_DRV_FEILDS + LOG_STATUS).toUInt ();

                            logSts = logVolumnData[loop+1].m_logStatus;
                            if(logSts == LOG_STS_CREATING)
                            {
                                QString percentStr = payloadLib->getCnfgArrayAtIndex(loop*MAX_LOG_DRV_FEILDS + LOG_PERCENTAGE).toString();
                                logVolumnData[loop+1].m_status = logDrvResponse[logSts] + ((percentStr != "0.0") ? (" ( " + percentStr + "%)") : "");
                            }
                            else
                            {
                                logVolumnData[loop+1].m_status = (logSts < MAX_LOG_STATE) ? logDrvResponse[logSts] : "";
                            }
                            logDiskCount++;

                            if((logSts == LOG_STS_FORMATING) || (logSts == LOG_STS_CREATING))
                            {
                                isAnyCreation = true;
                            }
                        }
                    }

                    diskStatus();

                    if(IS_VALID_OBJ(m_logScrollBar))
                    {
                        if(logDiskCount <= MAX_HDD_PAGE)
                        {
                            m_logScrollBar->setIsEnabled (false);
                            m_logScrollBar->setVisible (false);
                        }
                        else
                        {
                            m_logScrollBar->setIsEnabled (true);
                            m_logScrollBar->setVisible (true);
                        }

                        if (logDiskCount != m_prevHddCount)
                        {
                            m_prevHddCount = logDiskCount;
                            m_logScrollBar->updateTotalElement (logDiskCount);
                            m_firstIndex = 0;
                            m_lastIndex = (logDiskCount > MAX_HDD_PAGE) ? (MAX_HDD_PAGE -1) : (logDiskCount - 1);
                        }
                    }

                    for(index = 0; index <(MAX_LOGICAL_CELL -1); index++)
                    {
                        if((logVolumnData[m_firstIndex+1+index].m_volumeName) != "")
                        {
                            volumCellsLabel[index + 1]->changeText (logVolumnData[m_firstIndex+1+index].m_volumeName);
                            totalSizeLabel[index + 1]->changeText (logVolumnData[m_firstIndex+1+index].m_totalSize);
                            freeSizeLabel[index + 1]->changeText (logVolumnData[m_firstIndex+1+index].m_FreeSpace);

                            logSts = logVolumnData[m_firstIndex+1+index].m_logStatus;
                            logStatusLabel[index + 1]->changeText ((logSts < MAX_LOG_STATE) ? logVolumnData[m_firstIndex+1+index].m_status : "");

                            switch (logSts)
                            {
                                case LOG_STS_NORMAL:
                                {
                                    formatCntrl[index]->setVisible (true);
                                    formatCntrl[index]->setIsEnabled (true);
                                    abortCntrl[index]->setVisible (false);
                                    abortCntrl[index]->setIsEnabled (false);
                                }
                                break;

                                case LOG_STS_FULL:
                                case LOG_STS_FAULT:
                                case LOG_STS_INCOMPLETE:
                                {
                                    formatCntrl[index]->setVisible (true);
                                    formatCntrl[index]->setIsEnabled (true);
                                    abortCntrl[index]->setVisible (false);
                                    abortCntrl[index]->setIsEnabled (false);
                                    m_ModeDropDownBox->setIsEnabled(true);
                                }
                                break;

                                case LOG_STS_CREATING:
                                {
                                    QString percentStr = payloadLib->getCnfgArrayAtIndex((m_firstIndex + index)*MAX_LOG_DRV_FEILDS + LOG_PERCENTAGE).toString();
                                    if (percentStr != "0.0")
                                    {
                                        logStatusLabel[index + 1]->changeText(logDrvResponse[logSts] + " ( " + percentStr + "%)");
                                        abortCntrl[index]->setVisible (true);
                                        abortCntrl[index]->setIsEnabled (true);
                                    }
                                    formatCntrl[index]->setVisible (false);
                                    formatCntrl[index]->setIsEnabled (false);
                                }
                                break;

                                case LOG_STS_FORMATING:
                                {
                                    formatCntrl[index]->setVisible (false);
                                    formatCntrl[index]->setIsEnabled (false);
                                    abortCntrl[index]->setVisible (false);
                                    abortCntrl[index]->setIsEnabled (false);
                                }
                                break;

                                default:
                                {
                                    formatCntrl[index]->setVisible (false);
                                    formatCntrl[index]->setIsEnabled (false);
                                    abortCntrl[index]->setVisible (false);
                                    abortCntrl[index]->setIsEnabled (false);
                                }
                                break;
                            }
                        }
                        else
                        {
                            volumCellsLabel[index + 1]->changeText ("");
                            totalSizeLabel[index + 1]->changeText ("");
                            freeSizeLabel[index + 1]->changeText ("");
                            logStatusLabel[index + 1]->changeText ("");
                            formatCntrl[index]->setIsEnabled (false);
                            formatCntrl[index]->setVisible (false);
                            abortCntrl[index]->setVisible (false);
                            abortCntrl[index]->setIsEnabled (false);
                        }

                        volumCellsLabel[index + 1]->update ();
                        totalSizeLabel[index + 1]->update ();
                        freeSizeLabel[index + 1]->update ();
                        logStatusLabel[index + 1]->update ();

                    }

                    m_ModeDropDownBox->setIsEnabled(!isAnyCreation);
                    if(!statusRepTimer->isActive ())
                    {
                        statusRepTimer->start ();
                    }
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex() == HDD_MANAGMENT_TABLE_INDEX)
            {
                hddMode =  payloadLib->getCnfgArrayAtIndex (HDD_MODE).toUInt ();
                m_ModeDropDownBox->setIndexofCurrElement (payloadLib->getCnfgArrayAtIndex (HDD_MODE).toUInt ());
                m_getConfig = true;
            }
        }
        break;

        case MSG_SET_CFG:
        {
            m_deletePage = true;
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            getConfig ();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void Storage::diskStatus()
{
    if(m_phyDiskStatus == false)
    {
        m_ModeDropDownBox->hide();
        phyDrvHeading->hide();
        phyCellBgTile->hide();
        phyCellBottomBgTile->hide();
        logDrvHeading->hide();
        logCellBgTile->hide();
        logCellBottomBgTile->hide();
        if(devTableInfo.numOfHdd > (MAX_HDD_PAGE -1))
        {
            m_phyScrollbar->setVisible (false);
            m_phyScrollbar->setIsEnabled (false);
        }

        for(quint8 index = 0; index < (WIZ_MAX_PHY_CELL); index++)
        {

            diskCells[index]->hide();
            diskCellsLabel[index]->hide();
            serialNumbers[index]->hide();
            serialNumbersLabel[index]->hide();
            capacity[index]->hide();
            capacityLabel[index]->hide();
            status[index]->hide();
            statusLabel[index]->hide();
        }

        for(quint8 index = 0; index < (MAX_LOGICAL_CELL); index++)
        {
            volumCells[index]->hide();
            volumCellsLabel[0]->hide();
            totalSize[index]->hide();
            totalSizeLabel[0]->hide();
            freeSize[index]->hide();
            freeSizeLabel[0]->hide();
            logStatus[index]->hide();
            logStatusLabel[0]->hide();
            format[index]->hide();
            formatLabel->hide();
        }

        m_noDiskFoundImg->show();
        m_noDiskFoundLabel->show();
        m_noDiskFoundStr->show();
        m_noDiskFoundStr1->show();
    }
    else
    {
        m_noDiskFoundImg->hide();
        m_noDiskFoundLabel->hide();
        m_noDiskFoundStr->hide();
        m_noDiskFoundStr1->hide();

        m_ModeDropDownBox->show();
        phyDrvHeading->show();
        phyCellBgTile->show();
        phyCellBottomBgTile->show();
        logDrvHeading->show();
        logCellBgTile->show();
        logCellBottomBgTile->show();
        if(devTableInfo.numOfHdd > (MAX_HDD_PAGE -1))
        {
            m_phyScrollbar->setVisible (true);
            m_phyScrollbar->setIsEnabled(true);
        }

        for(quint8 index = 0; index < (WIZ_MAX_PHY_CELL); index++)
        {
            diskCells[index]->show();
            diskCellsLabel[index]->show();
            serialNumbers[index]->show();
            serialNumbersLabel[index]->show();
            capacity[index]->show();
            capacityLabel[index]->show();
            status[index]->show();
            statusLabel[index]->show();
        }

        for(quint8 index = 0; index < (MAX_LOGICAL_CELL); index++)
        {
            volumCells[index]->show();
            volumCellsLabel[index]->show();
            totalSize[index]->show();
            totalSizeLabel[index]->show();
            freeSize[index]->show();
            freeSizeLabel[index]->show();
            logStatus[index]->show();
            logStatusLabel[index]->show();
            format[index]->show();
            formatLabel->show();
        }
    }
    m_phyDiskStatus = false;
}

void Storage::slotInfoPageBtnclick(int index)
{
    WizardCommon::UnloadInfoPageImage();
    if (index != INFO_OK_BTN)
    {
        return;
    }

    if (m_infoPage->getText() == ValidationMessage::getValidationMessage(HDD_MANAGE_DO_REALY_WANT_FORMAT))
    {
        formatVolume();
        clickIndex = MAX_HDD_MNG_CTRL;
        return;
    }

    if (m_infoPage->getText() == ValidationMessage::getValidationMessage(HDD_MANAGE_PROCESS_NOT_ABORTED_CHANGE_HDD))
    {
        abortCreateProcess();
        clickIndex = MAX_HDD_MNG_CTRL;
        return;
    }
}

void Storage::wheelEvent(QWheelEvent *event)
{
    if ((IS_VALID_OBJ(m_phyScrollbar))
            && (event->x() >= diskCells[0]->x()) && (event->x() <= (m_phyScrollbar->x() + m_phyScrollbar->width()))
            && (event->y() >= m_phyScrollbar->y()) && (event->y() <= (m_phyScrollbar->y() + m_phyScrollbar->height())))
    {
        m_phyScrollbar->wheelEvent(event);
    }
    else if ((IS_VALID_OBJ(m_logScrollBar))
             && (event->x() >= volumCells[0]->x()) && (event->x() <= (m_logScrollBar->x() + m_logScrollBar->width()))
             && (event->y() >= m_logScrollBar->y()) && (event->y() <= (m_logScrollBar->y() + m_logScrollBar->height())))
    {
        m_logScrollBar->wheelEvent(event);
    }
    else
    {
        QWidget::wheelEvent(event);
    }
}
