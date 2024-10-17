//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskUtility.c
@brief      This file describes internals of disk manger modules.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/mount.h>
#include <mntent.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <ctype.h>

/* Library Includes */
#include <libudev.h>

/* Application Includes */
#include "DiskUtility.h"
#include "DebugLog.h"
#include "Utils.h"
#include "EventHandler.h"
#include "RecordManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define COREDUMP_FILE_NAME          "coredump.gz"
#define COREDUMP_SCRIPT_NAME        "coredump.sh"
#define COREDUMP_SCRIPT_SYMLINK     SCRIPTS_DIR_PATH "/coredump"
#define COREDUMP_DOWNLOAD_SYMLINK   LOG_DIR_PATH "/coredump_file"
#define COREDUMP_GZIP_GEN_CMD       "exec /bin/gzip - >%s"COREDUMP_FILE_NAME"\n"

#define DEVICE_ADD_ACTION			"add"
#define DEVICE_CHANGE_ACTION		"change"
#define DEVICE_REMOVE_ACTION		"remove"

#define STORAGE_DISK_PATH			"/dev/"
#define	PARTITION_NODE				"%s1"

#define RECORD_DISK_INFO_VERSION    1
#define CURRENT_RECORD_DISK_INFO    CONFIG_DIR_PATH "/RecordDiskInfo.cfg"
#define RAID_ARRAY_NAME				"/dev/md%d"
#define HDD_DISK_MNT				MEDIA_DIR_PATH "/" SINGLE_DISK_NAME
#define RAID_ARRAY_MOUNT_POINT		MEDIA_DIR_PATH "/" RAID_DISK_NAME

#define	MAX_LINE					(512)

// mdadm utility related defines
#define RAID_SPEED_FILE				"/proc/sys/dev/raid/speed_limit_min"

#define	RAID_STATUS_NO_DEV			"unused devices:"
#define RAID_STATUS_RESYNC			"resync"
#define RAID_STATUS_RECOVER			"recovery"
#define RAID_TOTAL_NODE_STR         "# TotalNodes="

#define GPT_PARTITION_TYPE_LEN		(40)
#define GPT_GUID_HEX_LEN            (16)
#define	MAX_SECTOR_SIZE				(512)
#define	NO_HDD_PARTITION			(0x00)
#define	NATIVE_HDD_FORMAT			(0xEE) /* GPT format */

#define	GPT_PRIMARY_PARTITION_SIZE	(128)
#define	PRIMARY_PARTITION_1			(0x01c2)
#define	PRIMARY_PARTITION_2			(0x01d2)
#define	PRIMARY_PARTITION_3 		(0x01e2)
#define	PRIMARY_PARTITION_4			(0x01f2)
#define	MAX_PARTITION_SUPPORT		4

#define	MIN_RAID_SPEED				50000	// KB/min
#define	NODE_NAME_SIZE				100     // Device Node name size
#define	SYMBOLIC_LINK_SIZE			50      // Device symbolink link size
#define	DEFAULT_DISK_SIZE			0
#define MOUNT_FS_SIZE               500

#define MEDIA_DETECT_WAIT_TMR       15  /* seconds */
#define MEDIA_DETECT_RETRY_CNT      20  /* 5 minutes = 15 seconds * 20 retries */

#define RAID_STATUS_FILE			"/proc/mdstat"
#define	MDADM_CONFIG_FILE			"/etc/mdadm%d.conf"
#define	UDEV_START_STOP_SCRIPT		"/etc/init.d/S10udev"

#define	PARTED_LABEL_CMD			"parted -s %s mklabel gpt"
#define	PARTED_PART_CMD				"parted -s %s mkpart primary ext4 100MB %dGB"

#define	PARTED_FIRST_PART_CMD		"parted -s %s mkpart primary ext4 %dMB %dGB"
#define	PARTED_ALL_PART_CMD			"parted -s %s mkpart primary ext4 %dGB %dGB"
#define MTAB_FILE_PATH				"/etc/mtab"

#define EHCI_BUS1					"/hiusb-ehci.0/usb1/1-2/1-2:1.0"
#define OHCI_BUS2					"/hiusb-ohci.0/usb2/2-2/2-2:1.0"
#define XHCI_BUS3					"/hiusb-xhci.0/usb3/3-1/3-1:1.0"
#define XHCI_BUS4					"/hiusb-xhci.0/usb4/4-1/4-1:1.0"

#define BYTES_IN_GB                 (1000 * 1000 * 1000)
#define BYTES_IN_MB                 (1000 * 1000)

#if defined(HI3536_NVRL) || defined(HI3536_NVRH)
#define MAX_SUPPORTED_HDD_SIZE_GB           (16 * 1000)     /* 16 TB */
#else
#define MAX_SUPPORTED_HDD_SIZE_GB           (18 * 1000)     /* 18 TB */
#endif

#if defined(RK3588_NVRH)
#define MAX_ALLOWED_RAID_VOLUME_SIZE_GB     (64 * 1000)     /* 64TB */
#else
#define MAX_ALLOWED_RAID_VOLUME_SIZE_GB     (12 * 1000)     /* 12TB */
#endif

#define GET_RAID_VOLUME_ID(grp, vol)        ((grp * MAX_RAID_VOLUME) + vol)
#define TOTAL_RAID_DEVICE                   (4)

#define BLOCK_DEVICE_INFO_FILE_PATH         "/sys/block/%s/%s/%s"   // example :: /sys/block/sda/sda1/size
#define MAX_HDD_VOL							(8)
#define IDENTICAL_OFFSET					(2000)       //2GB offset

/* Get hard-disk or raid volume info based on recording mode */
#define GET_MEDIA_VOLUME_INFO(mode, volume) (mode == SINGLE_DISK_VOLUME) ? &storageMedia[volume] : &raidVolumeInfo[volume]

/* Stack size for different threads */
#define UPDATE_BKP_DEVICE_THREAD_STACK_SZ   (2* MEGA_BYTE)
#define INIT_STORAGE_THREAD_STACK_SZ        (2* MEGA_BYTE)
#define IMP_SINGLE_DISK_THREAD_STACK_SZ     (0* MEGA_BYTE)
#define IMP_RAID_DISK_VOL_THREAD_STACK_SZ   (0* MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    GPT_GUID_TYPE_HISI = 0,
    GPT_GUID_TYPE_RK,
    GPT_GUID_TYPE_MAX
}GPT_GUID_TYPE_e;

typedef enum
{
    RAID_DISK_CNT_MIN = 0,
    RAID_DISK_CNT_MAX,
    RAID_DISK_CNT
}RAID_DISK_CNT_e;

typedef enum
{
    NOT_PRESENT,
    CREATING_PARTITION,
    DISK_FORMATTING,
    RAID_BUILDING,
    RAID_RESYNCING,
    UNMOUNTED,
    MOUNTED_READY,
    CLEANUP_RUNNING
}RAW_MEDIA_STATUS_e;

typedef enum
{
    RAID_STATUS_NO_DEVICE,
    RAID_STATUS_COMPLETE,
    RAID_STATUS_DEGRADED,
    RAID_STATUS_RESYNCHING,
    RAID_STATUS_RECOVERY,
    RAID_STATUS_MAX
}RAID_STATUS_e;

typedef struct
{
    BOOL                    diskState; /* Physical present status of disk: added or removed */

    PHYSICAL_DISK_STATUS_e  diskPhyStatus; /* Disk physical working status */

    CHAR                    serialNo[HD_SERIAL_NO_SIZE]; /* Serial number size of disk */

    UINT64                  physicalSize; /* Physical size of disk in MB */

    CHAR                    mediaNodeName[NODE_NAME_SIZE]; /* Disk node name: /dev/sda, /dev/sdb etc. */

    CHAR                    symbolicLink[SYMBOLIC_LINK_SIZE]; /* Disk base node: sda, sdb etc. */

    CHAR                    partitionNode[NODE_NAME_SIZE]; /* Disk partition node: /dev/sda1, /dev/sdb1 etc. */

    pthread_mutex_t         phyMediaMutex; /* mutex for locking */

}PHYSICAL_DISK_STATUS_t;

typedef struct
{
    UINT8                   partitionIndex; /* Disk partiton index: 1 for sda1, 2 for sda2 etc. */

    CHAR                    partitionNodeName[NODE_NAME_SIZE]; /* Disk partition node: /dev/sda1, /dev/sda2 etc. */

    CHAR                    partitionSymbLink[SYMBOLIC_LINK_SIZE]; /* Disk partition name: sda1, sda2 etc. */

    UINT32                  ptStartLoc; /* Disk partition start location in MB for first partition and GB for other partitions */

    UINT32                  ptEndLoc; /* Disk partition end location in GB */

}HDD_PARTITION_INFO_t;

typedef struct
{
    UINT8                   totalPartitions; /* Total partitions on a disk */

    HDD_PARTITION_INFO_t    partitionInfo[MAX_PARTITION_SUPPORT]; /* Partition related information */

}HDD_RAID_INFO_t;

typedef struct
{
    RAW_MEDIA_STATUS_e      mediaStatus; /* Media physical status: Not present, Mounted, Unmounted etc. */

    DISK_VOL_STATUS_e       diskVolStatus; /* Volume operational status: Working, Full, Faulty etc. */

    UINT16                  diskFormatPercentage; /* Volume format percentage */

    CHAR                    mountPoint[MOUNT_POINT_SIZE]; /* Volume mount path */

    UINT64                  totalVolSize; /* Volume size in MB */

    UINT64                  freeSize; /* Volume free size */

    UINT64                  usedSize; /* Volume used size */

    UINT8                   totalHardDisk; /* Total disks are part of raid volume */

    pthread_mutex_t         rawMediaMutex; /* mutex for locking */

}RAW_MEDIA_INFO_t;

typedef struct
{
    STORAGE_HEALTH_STATUS_e healthStatus; /* Overall system storage health status */

    UINT8                   currentRecordVolumeId; /* volume id in which currently recording is doing */

    CHAR                    mountPoint[MOUNT_POINT_SIZE]; /* Mount path of current recording volume */

    pthread_mutex_t         storageDiskMutex; /* mutex for locking */

}STORAGE_DISK_INFO_t;

typedef struct
{
    STORAGE_HEALTH_STATUS_e healthStatus; /* Overall system storage health status */

    pthread_mutex_t         mutexLock; /* mutex for locking */

}SYSTEM_STORAGE_HEALTH_STATUS_t;

typedef struct
{
    BOOL					threadActive; /* Usb device update thread status */

    UINT8					mediaNo; /* USB on which update operation to be performed */

    pthread_mutex_t			usbMutex; /* mutex for locking */

}USB_THREAD_INFO_t;

typedef struct
{
    RAID_STATUS_e		raidStatus;
    CHAR				deviceNode[TOTAL_RAID_DEVICE][NODE_NAME_SIZE];
    CHAR				raidPercentage[6];
    UINT8               totalNodes;
}RAID_RESULT_t;

typedef struct
{
    HDD_MODE_e          raidType;
    CHAR				deviceNode[TOTAL_RAID_DEVICE][NODE_NAME_SIZE];
    UINT8				hardDiskCnt;
    UINT8				totalRaidDevices;
    RAID_VOLUME_NO_e	raidVolNo;

}RAID_CREATE_INFO_t;

typedef	struct
{
    BOOL                isHddModeChanged;
    HDD_MODE_e			oldHddMode;

}HDD_CFG_CHNG_t;

typedef struct
{
    UINT32              version;
    UINT8               mode;
    UINT8               volumeId[MAX_CAMERA];

}RECORD_DISK_INFO_t;    /* Version 1: Camera wise recording disk */

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR initStorageMediaThread(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static void performHddConfigMode(void);
//-------------------------------------------------------------------------------------------------
static BOOL removeAllHardDisk(HDD_MODE_e hddMode);
//-------------------------------------------------------------------------------------------------
static BOOL removeAllUsbDisk(void);
//-------------------------------------------------------------------------------------------------
static VOIDPTR impSingleDiskVolume(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR impRaidDiskVolume(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static VOIDPTR updateBackupDevice(VOIDPTR threadArg);
//-------------------------------------------------------------------------------------------------
static BOOL makeUsbDiskReady(UINT8 usbDiskCnt, RAW_MEDIA_FORMAT_TYPE_e formatType);
//-------------------------------------------------------------------------------------------------
static BOOL isValidGptGuidType(CHARPTR pGptGuidStr);
//-------------------------------------------------------------------------------------------------
static BOOL getPartitionNoAndType(CHARPTR deviceNode, UINT8PTR partitionInfo);
//-------------------------------------------------------------------------------------------------
static BOOL createPartition(CHARPTR deviceNode, UINT64 partitionSize);
//-------------------------------------------------------------------------------------------------
static BOOL formatWithExt4(CHARPTR deviceNode);
//-------------------------------------------------------------------------------------------------
static void migrateCurrentRecordDiskInfo(void);
//-------------------------------------------------------------------------------------------------
static BOOL updateRecordDiskInfoFile(RECORD_DISK_INFO_t *pDiskIdInfo);
//-------------------------------------------------------------------------------------------------
static BOOL updateRecordDiskInfoFileForCamera(UINT8 cameraIndex, UINT8 volumeId, HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
static BOOL formatWithFat(CHARPTR deviceNode);
//-------------------------------------------------------------------------------------------------
static BOOL mountDevice(CHARPTR deviceNode, CHARPTR mountPath, RAW_MEDIA_FORMAT_TYPE_e fsType);
//-------------------------------------------------------------------------------------------------
static BOOL unmountDevice(CHARPTR mountPath);
//-------------------------------------------------------------------------------------------------
static BOOL checkRaidStatus(const RAID_CREATE_INFO_t *raidInfo, RAID_RESULT_t *raidResult);
//-------------------------------------------------------------------------------------------------
static BOOL getActualRaidArrayDisks(UINT8 raidVolNo, UINT8 *pTotalNode);
//-------------------------------------------------------------------------------------------------
static BOOL isMdadmConfigFilePresent(RAID_VOLUME_NO_e raidVolNo);
//-------------------------------------------------------------------------------------------------
static void writeRaidVolumeEvent(HDD_MODE_e mode, RAID_VOLUME_NO_e raidVolNo, CHARPTR advncDetail, LOG_EVENT_STATE_e eventState);
//-------------------------------------------------------------------------------------------------
static BOOL assembleRaid(RAID_VOLUME_NO_e raidVolNo);
//-------------------------------------------------------------------------------------------------
static void updateMdadmConfFile(const RAID_CREATE_INFO_t *raidInfo);
//-------------------------------------------------------------------------------------------------
static BOOL monitorRaidStatusContinues(const RAID_CREATE_INFO_t *raidInfo, RAID_RESULT_t *raidResult);
//-------------------------------------------------------------------------------------------------
static UINT8 getTotalRaidVolume(HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
static BOOL scanStorageDevices(void);
//-------------------------------------------------------------------------------------------------
static BOOL getGptPartitionNoAndType(CHARPTR deviceNode, CHARPTR partitionInfo, UINT8 partitionMax, INT32 *blockSize);
//-------------------------------------------------------------------------------------------------
static BOOL stopRaidArray(RAID_VOLUME_NO_e raidVolNo);
//-------------------------------------------------------------------------------------------------
static BOOL makeMdadmConfFile(const RAID_CREATE_INFO_t *raidInfo);
//-------------------------------------------------------------------------------------------------
static void removeAllMdadmConfFile(void);
//-------------------------------------------------------------------------------------------------
static BOOL addRaidDevice(RAID_VOLUME_NO_e raidVolNo, CHARPTR deviceNode);
//-------------------------------------------------------------------------------------------------
static void cleanRaidArray(const RAID_CREATE_INFO_t *raidInfo);
//-------------------------------------------------------------------------------------------------
static BOOL createRaidArray(const RAID_CREATE_INFO_t *raidInfo);
//-------------------------------------------------------------------------------------------------
static BOOL readyRaidPartition(HDD_MODE_e mode, RAID_VOLUME_NO_e raidVolNo, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
static BOOL getSizeOfDisk(CHARPTR deviceNode, UINT64PTR diskSize);
//-------------------------------------------------------------------------------------------------
static BOOL makeHarddiskReady(UINT8 hardDiskCnt, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
static BOOL unmountRunningDevice(CHARPTR mountPoint);
//-------------------------------------------------------------------------------------------------
static void storageMediaDetectWaitTimerCb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void resetStorageVolumeInfo(RAW_MEDIA_INFO_t *pMediaInfo, RAW_MEDIA_STATUS_e mediaStatus, DISK_VOL_STATUS_e diskVolStatus);
//-------------------------------------------------------------------------------------------------
static void compareDiskSize(STORAGE_MEDIA_TYPE_e startDiskId, STORAGE_MEDIA_TYPE_e endDiskId, STORAGE_MEDIA_TYPE_e *pLeastSizeDiskId);
//-------------------------------------------------------------------------------------------------
static void predictLogicalRaidVolume(HDD_MODE_e mode, UINT64 diskSizeInMb, UINT8 hddPresentCnt, HDD_RAID_INFO_t *hddPartitionData);
//-------------------------------------------------------------------------------------------------
static BOOL createRaidPartition(RAID_CREATE_INFO_t *raidInfo);
//-------------------------------------------------------------------------------------------------
static BOOL createPartirionsForRaid(UINT8 hddIndex);
//-------------------------------------------------------------------------------------------------
static BOOL verifyAndCreatePartitionsInMissingDisk(UINT8 source, UINT8 dest);
//-------------------------------------------------------------------------------------------------
static BOOL getDiskPartitionsInfo(UINT8 hddIndex);
//-------------------------------------------------------------------------------------------------
static UINT8 getPossibleRaidGroupVolumes(HDD_MODE_e mode, UINT8 hddPresentCnt, UINT8 raidGrpId);
//-------------------------------------------------------------------------------------------------
static BOOL readBlockDeviceInfo(UINT8 hddIndex, UINT8 partitionIndex, CHARPTR mediaSymbolicLink, INT32 blockSize, CHARPTR partitionSymbLink);
//-------------------------------------------------------------------------------------------------
static BOOL updateRaidVolumeInfo(UINT8 raidVolNo, BOOL status);
//-------------------------------------------------------------------------------------------------
static BOOL isLegacyRaidArrayPresent(void);
//-------------------------------------------------------------------------------------------------
static void legacyRaidConfigMigration(void);
//-------------------------------------------------------------------------------------------------
static void updateStorageStatus(HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
static void handleDiskFaultAlert(UINT32 status);
//-------------------------------------------------------------------------------------------------
static void prepareRaidVolumeName(UINT8 mode, UINT8 volumeNum, CHAR *pNameBuff);
//-------------------------------------------------------------------------------------------------
static BOOL isAnyStorageVolumePresent(HDD_MODE_e mode, STORAGE_HEALTH_STATUS_e *pHealthStatus);
//-------------------------------------------------------------------------------------------------
static UINT8 getNormalAllocatedCameraVolume(UINT8 cameraIndex, HDD_MODE_e mode, UINT8 preferVolId);
//-------------------------------------------------------------------------------------------------
static BOOL isStorageGroupVolumePresent(UINT8 volGrpId, HDD_MODE_e mode);
//-------------------------------------------------------------------------------------------------
static void setCameraStorageVolume(UINT8 volumeId, HDD_MODE_e mode, CHAR *pMountPoint);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
const CHARPTR mediaNameStr[MAX_RAW_MEDIA] =
{
    "Harddisk 1",
    "Harddisk 2",
    "Harddisk 3",
    "Harddisk 4",
    "Harddisk 5",
    "Harddisk 6",
    "Harddisk 7",
    "Harddidk 8",
    "Manual USB",
    "Schedule USB",
};

const CHARPTR storageModeStr[MAX_HDD_MODE+1] =
{
    "harddisk",
    "raid0",
    "raid1",
    "raid5",
    "raid10",
    "invalid",
};

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR rawMediaStr[2][MAX_RAW_MEDIA] =
{
    #if defined(RK3568_NVRL)
    {
        "/devices/platform/fc400000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "/devices/platform/fc800000.sata/ata2/host1/target1:0:0/1:0:0:0/",  /* SATA Port 2 */
        "", "", "", "", "", "",                                             /* No SATA Port Present */
        "/devices/platform/usbdrd/fcc00000.dwc3/xhci-hcd.4.auto/",          /* USB3.0 Manual Backup */
        "/devices/platform/fd880000.usb/usb2/2-1/2-1:1.0/",                 /* USB2.0 Scheduled Backup */
    },
    {
        "/devices/platform/fc400000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "", "", "", "", "", "", "",                                         /* No SATA Port Present */
        "/devices/platform/usbdrd/fcc00000.dwc3/xhci-hcd.4.auto/",          /* USB3.0 Manual Backup */
        "/devices/platform/fd800000.usb/usb1/1-1/1-1:1.0/",                 /* USB2.0 Scheduled Backup */
    }
    #elif defined(RK3588_NVRH)
    {
        "/devices/platform/fe210000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:0:0/1:0:0:0/",  /* SATA Port 2 */
        "", "", "", "", "", "",                                             /* No SATA Port Present */
        "/devices/platform/usbdrd3_0/fc000000.usb/xhci-hcd.5.auto/",        /* USB3.0 Manual Backup */
        "/devices/platform/fc880000.usb/usb2/2-1/2-1:1.0/",                 /* USB2.0 Scheduled Backup */
    },
    {
        "/devices/platform/fe210000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "/devices/platform/fe210000.sata/ata1/host0/target0:1:0/0:1:0:0/",  /* SATA Port 2 */
        "/devices/platform/fe210000.sata/ata1/host0/target0:2:0/0:2:0:0/",  /* SATA Port 3 */
        "/devices/platform/fe210000.sata/ata1/host0/target0:3:0/0:3:0:0/",  /* SATA Port 4 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:0:0/1:0:0:0/",  /* SATA Port 5 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:1:0/1:1:0:0/",  /* SATA Port 6 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:2:0/1:2:0:0/",  /* SATA Port 7 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:3:0/1:3:0:0/",  /* SATA Port 8 */
        "/devices/platform/usbdrd3_0/fc000000.usb/xhci-hcd.5.auto/",        /* USB3.0 Manual Backup */
        "/devices/platform/fc880000.usb/usb2/2-1/2-1:1.0/",                 /* USB2.0 Scheduled Backup */
    }
    #else
    {
        "/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:0:0/1:0:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:0:0/2:0:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:0:0/3:0:0:0/",
        "/devices/platform/ahci.0/ata1/host0/target0:1:0/0:1:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:1:0/1:1:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:1:0/2:1:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:1:0/3:1:0:0/",
        "/devices/platform/hiusb-xhci.0/usb3/3-1/3-1:1.0/",
        "/devices/platform/hiusb-ehci.0/usb1/1-2/1-2:1.0/"
    },
    {
        "/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/",
        "/devices/platform/ahci.0/ata1/host0/target0:1:0/0:1:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:0:0/1:0:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:1:0/1:1:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:0:0/2:0:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:1:0/2:1:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:0:0/3:0:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:1:0/3:1:0:0/",
        "/devices/platform/hiusb-xhci.0/usb3/3-1/3-1:1.0/",
        "/devices/platform/hiusb-ehci.0/usb1/1-2/1-2:1.0/"
    }
    #endif
};

static const UINT8 raidArrayDiskCnt[MAX_HDD_MODE][RAID_DISK_CNT] =
{
    {0, 0}, /* Harddisk (NA) */
    {2, 2}, /* Raid 0 */
    {2, 2}, /* Raid 1 */
    {3, 4}, /* Raid 5 */
    {4, 4}, /* Raid 10 */
};

static const CHARPTR diskVolumePrefixStr[MAX_HDD_MODE] =
{
    "Single Disk Volume",
    "Striping Disk Volume",
    "Mirroring Disk Volume",
    "RAID 5 Volume",
    "RAID 10 Volume"
};

static const CHARPTR  mediaStatusStr[] =
{
    "REMOVED",
    "ADDED",
    "UNKNOWN"
};

static const CHARPTR raidStatusStr[RAID_STATUS_MAX] =
{
    "NO_DEVICE",
    "COMPLETE",
    "DEGRADED",
    "RESYNCHING",
    "RECOVERY",
};

static const CHARPTR usbMointPoint[DM_MAX_BACKUP] =
{
    MEDIA_DIR_PATH "/Manual_Backup/",
    MEDIA_DIR_PATH "/Schedule_Backup/"
};

static const CHARPTR fsTypeName[FORMAT_MAX] ={"ext4", "vfat", "ntfs"};

static const CHARPTR fsOptionsStr[FORMAT_MAX] = {"data=writeback", "", ""};

static const CHARPTR formatTypeName[FORMAT_MAX] = {"mkfs.ext4", "mkfs.vfat", "mkfs.ntfs"};

static const CHAR *pGptGuidType[GPT_GUID_TYPE_MAX] =
{
    "ebd0a0a2-b9e5-4433-87c0-68b6b72699c7", /* It is Windows basic data partition GUID for Hi-Silicon */
    "0fc63daf-8483-4772-8e79-3d69d8477de4", /* It is Linux filesystem data partition GUID for Rockchip */
};

/* Physical information of disk */
static PHYSICAL_DISK_STATUS_t 	physicalMediaInfo[MAX_RAW_MEDIA];

/* Volume information of hard-disk and usb */
static RAW_MEDIA_INFO_t         storageMedia[MAX_RAW_MEDIA];

/* Logical volume information of raid */
static RAW_MEDIA_INFO_t         raidVolumeInfo[MAX_VOLUME];

/* Variables for new device detection and device removal at bootup and runtime */
static pthread_t                initStorageThreadId;
static BOOL                     stopStorageDevDetectF;
static BOOL                     newDevDetectSignalF;
static pthread_mutex_t          storageCondMutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t           storageCondSignal = PTHREAD_COND_INITIALIZER;
static UINT8                    mediaDeviceDetectCnt = 0;
static UINT8                    totalAttachedDevice = 0;
static UINT8                    mediaDetectWaitRetryCnt = 0;
static TIMER_HANDLE             mediaDetectWaitTimerHandle = INVALID_TIMER_HANDLE;
static TIMER_HANDLE             diskFaultInactiveTimerHandle = INVALID_TIMER_HANDLE;
static pthread_mutex_t          diskFaultInactiveTimerMutex = PTHREAD_MUTEX_INITIALIZER;

/* Current recording volume information */
static STORAGE_DISK_INFO_t      storageDiskInfo[MAX_CAMERA];

/* USB detection status update related information */
static USB_THREAD_INFO_t        usbThreadInfo[DM_MAX_BACKUP];

/* Indicates stop or start hdd related file IO (Recording, Playback, Searching, Backup) */
static pthread_mutex_t          stopHddFunctMutex = PTHREAD_MUTEX_INITIALIZER;
static BOOL                     stopHddFunction;

/* Disk preparation for recording */
static pthread_mutex_t          diskVolBuildMutex;
static BOOL                     diskVolBuildStatus;
static HDD_MODE_e               lastHddMode = MAX_HDD_MODE;

/* Recording disk configuration change status */
static HDD_CFG_CHNG_t           hddCfgChng;

/* information related to all disks and its partitions for raid */
static HDD_RAID_INFO_t   		raidDiskPartInfo[MAX_HDD_VOL];

/* Over all system's storage health status */
static SYSTEM_STORAGE_HEALTH_STATUS_t   systemStorageHealthStatus;

/* Storage allocation configuration */
static STORAGE_ALLOCATION_CONFIG_t      storageAllocCfg;

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes disk manager, it was recover the entire file in hardisk and
 *          insert proper information in proper file. If event was present in stream file but
 *          corresponding references not present then it will put information of event into event file.
 *          And also creates a thread thread for flushing stream data into file.
 * @return  SUCCESS/FAIL
 */
BOOL InitDiskUtility(void)
{
    UINT8   mediaId, cameraIndex;
    FILE    *pFile;

    for(mediaId = 0; mediaId < DM_MAX_BACKUP; mediaId++)
    {
        usbThreadInfo[mediaId].mediaNo = MAX_RAW_MEDIA;
        usbThreadInfo[mediaId].threadActive = FALSE;
        MUTEX_INIT(usbThreadInfo[mediaId].usbMutex, NULL);
    }

    diskVolBuildStatus = FALSE;
    MUTEX_INIT(diskVolBuildMutex, NULL);

    // Stop hard disk functionality
    stopHddFunction = TRUE;

    //creates a thread for configure hard disk as per configuration manager
    stopStorageDevDetectF = FALSE;
    newDevDetectSignalF = FALSE;

    /* Change Raid min speed parameter */
    pFile = fopen(RAID_SPEED_FILE, "w");
    if(pFile != NULL)
    {
        fprintf(pFile, "%d\n", MIN_RAID_SPEED);
        fclose(pFile);
    }

    for(mediaId = 0; mediaId < MAX_RAW_MEDIA; mediaId++)
    {
        MUTEX_INIT(storageMedia[mediaId].rawMediaMutex, NULL);
        resetStorageVolumeInfo(&storageMedia[mediaId], NOT_PRESENT, DM_DISK_VOL_MAX);
        storageMedia[mediaId].totalHardDisk = 0;

        physicalMediaInfo[mediaId].diskPhyStatus = DM_HDD_NO_DISK;
        physicalMediaInfo[mediaId].diskState = UNKNOWN;
        physicalMediaInfo[mediaId].physicalSize = DEFAULT_DISK_SIZE;
        memset(physicalMediaInfo[mediaId].serialNo, 0, sizeof(physicalMediaInfo[mediaId].serialNo));
        memset(physicalMediaInfo[mediaId].mediaNodeName, 0, sizeof(physicalMediaInfo[mediaId].mediaNodeName));
        memset(physicalMediaInfo[mediaId].symbolicLink, 0, sizeof(physicalMediaInfo[mediaId].symbolicLink));
        memset(physicalMediaInfo[mediaId].partitionNode, 0, sizeof(physicalMediaInfo[mediaId].partitionNode));
        MUTEX_INIT(physicalMediaInfo[mediaId].phyMediaMutex, NULL);
    }

    for(mediaId = 0; mediaId < MAX_VOLUME; mediaId++)
    {
        MUTEX_INIT(raidVolumeInfo[mediaId].rawMediaMutex, NULL);
        resetStorageVolumeInfo(&raidVolumeInfo[mediaId], NOT_PRESENT, DM_DISK_VOL_MAX);
        raidVolumeInfo[mediaId].totalHardDisk = 0;
    }

    for(cameraIndex = 0; cameraIndex < MAX_CAMERA; cameraIndex++)
    {
        storageDiskInfo[cameraIndex].currentRecordVolumeId = MAX_VOLUME;
        RESET_STR_BUFF(storageDiskInfo[cameraIndex].mountPoint);
        storageDiskInfo[cameraIndex].healthStatus = STRG_HLT_MAX;
        MUTEX_INIT(storageDiskInfo[cameraIndex].storageDiskMutex, NULL);
    }

    systemStorageHealthStatus.healthStatus = STRG_HLT_MAX;
    MUTEX_INIT(systemStorageHealthStatus.mutexLock, NULL);

    hddCfgChng.isHddModeChanged = FALSE;
    hddCfgChng.oldHddMode = MAX_HDD_MODE;
    ReadStorageAllocationConfig(&storageAllocCfg);

    /* Migrate current record disk info file from old file to new file if old file is present */
    migrateCurrentRecordDiskInfo();

    /* Scan for devices to detect at bootup */
    if (FAIL == scanStorageDevices())
    {
        EPRINT(DISK_MANAGER, "fail to scan storage devices");
        return FAIL;
    }

    TIMER_INFO_t mediaDetectTimerInfo;
    mediaDetectTimerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(MEDIA_DETECT_WAIT_TMR);
    mediaDetectTimerInfo.data = 0;
    mediaDetectTimerInfo.funcPtr = storageMediaDetectWaitTimerCb;

    /* Start the media detect wait timer to wait for sometime. So that all media get detect properly */
    if (FAIL == StartTimer(mediaDetectTimerInfo, &mediaDetectWaitTimerHandle))
    {
        EPRINT(DISK_MANAGER, "fail to start media detect wait timer");
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function de initialize disk manager module. It will clear all memory resources used
 *          by disk manager module and also destroy its thread and corresponding conditional variables.
 * @return  SUCCESS/FAIL.
 */
BOOL DeInitDiskUtility(void)
{
    MUTEX_LOCK(storageCondMutex);
    stopStorageDevDetectF = TRUE;
    pthread_cond_signal(&storageCondSignal);
    MUTEX_UNLOCK(storageCondMutex);

    pthread_join(initStorageThreadId, NULL);
    DPRINT(SYS_LOG, "disk utility deinit successfully");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever any disk was inserted or removed. It will stores the
 *          information of storage media and wake up the thread which will take the action against
 *          storage media.
 * @param   deviceInfo
 * @return  SUCCESS/FAIL.
 */
BOOL DetectStorageMedia(UDEV_DEVICE_INFO_t *deviceInfo)
{
    BOOL                    isMultiplierAvail;
    BOOL					diskState;
    STORAGE_MEDIA_TYPE_e	mediaType;

    if (NULL == deviceInfo)
    {
        return FAIL;
    }

    /* deviceInfo contains fixed format value that represent information of whole disk */
    DPRINT(DISK_MANAGER, "storage device info: [action=%s], [path=%s], [serial=%s], [baseNode=%s]",
           deviceInfo->action, deviceInfo->path, deviceInfo->serial, deviceInfo->baseNode);

    /* Get disk multiplier status */
    isMultiplierAvail = IsDiskMultiplierAvailable();

    /* We have added device nodes at first index of rawMediaStr for NVR0801XSP2 & NVR1601XSP2 */
    #if defined(RK3568_NVRL)
    if ((GetNvrModelType() == NVR0401XSP2) || (GetNvrModelType() == NVR0801XSP2) || (GetNvrModelType() == NVR1601XSP2))
    {
        isMultiplierAvail = TRUE;
    }
    #endif

    for(mediaType = HDD1; mediaType < MAX_RAW_MEDIA; mediaType++)
    {
        #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
        /* Device path is null then skip this media */
        if (rawMediaStr[isMultiplierAvail][mediaType][0] == '\0')
        {
            /* This is not a valid media for comparision */
            continue;
        }

        /* Compare detected path with our media path to identify device */
        if(NULL != strstr(deviceInfo->path, rawMediaStr[isMultiplierAvail][mediaType]))
        {
            break;
        }
        #else
        if(strstr(deviceInfo->path, "ahci.0") != NULL)
        {
            if(NULL != strstr(deviceInfo->path, rawMediaStr[isMultiplierAvail][mediaType]))
            {
                break;
            }
        }
        else if((strstr(deviceInfo->path, XHCI_BUS3) != NULL) || (strstr(deviceInfo->path, XHCI_BUS4) != NULL))
        {
            mediaType = MANUAL_BACKUP_DISK;
            break;
        }
        else if((strstr(deviceInfo->path, EHCI_BUS1) != NULL) || (strstr(deviceInfo->path, OHCI_BUS2) != NULL))
        {
            mediaType = SCHEDULE_BACKUP_DISK;
            break;
        }
        #endif
    }

    /* It is not required media */
    if (mediaType >= MAX_RAW_MEDIA)
    {
        return FAIL;
    }

    /* NOTE: This condition was for ignore the partition create because whenever partition
     * was created udev will detect device but this device was already in process */
    MUTEX_LOCK(physicalMediaInfo[mediaType].phyMediaMutex);
    diskState = physicalMediaInfo[mediaType].diskState;
    MUTEX_UNLOCK(physicalMediaInfo[mediaType].phyMediaMutex);

    /* Take action as per udev data */
    if ((strcasecmp(deviceInfo->action, DEVICE_ADD_ACTION) == 0) || (strcasecmp(deviceInfo->action, DEVICE_CHANGE_ACTION) == 0))
    {
        if((mediaType < getMaxHardDiskForCurrentVariant()) && (diskState == ADDED))
        {
            return SUCCESS;
        }

        diskState = ADDED;
        mediaDeviceDetectCnt++;
    }
    else if(strcasecmp(deviceInfo->action, DEVICE_REMOVE_ACTION) == 0)
    {
        diskState = REMOVED;
        if (mediaDeviceDetectCnt) mediaDeviceDetectCnt--;
    }

    MUTEX_LOCK(physicalMediaInfo[mediaType].phyMediaMutex);
    physicalMediaInfo[mediaType].diskState = diskState;
    snprintf(physicalMediaInfo[mediaType].mediaNodeName, NODE_NAME_SIZE, "%s%s", STORAGE_DISK_PATH, deviceInfo->baseNode);
    snprintf(physicalMediaInfo[mediaType].symbolicLink, SYMBOLIC_LINK_SIZE, "%s", deviceInfo->baseNode);
    snprintf(physicalMediaInfo[mediaType].serialNo, HD_SERIAL_NO_SIZE, "%s", deviceInfo->serial);
    DPRINT(DISK_MANAGER, "media info: [name=%s], [status=%s], [node=%s], [link=%s], [serial=%s]",
           mediaNameStr[mediaType], mediaStatusStr[physicalMediaInfo[mediaType].diskState], physicalMediaInfo[mediaType].mediaNodeName,
            physicalMediaInfo[mediaType].symbolicLink, physicalMediaInfo[mediaType].serialNo);
    MUTEX_UNLOCK(physicalMediaInfo[mediaType].phyMediaMutex);

    MUTEX_LOCK(storageCondMutex);
    newDevDetectSignalF = TRUE;
    pthread_cond_signal(&storageCondSignal);
    MUTEX_UNLOCK(storageCondMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever configuration was changed of Hard disk config.
 * @param   newHddConfig
 * @param   oldHddConfig
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e CheckHddCofigUpdate(HDD_CONFIG_t *newHddConfig, HDD_CONFIG_t *oldHddConfig)
{
    UINT8               hddCnt;
    NET_CMD_STATUS_e	retVal = CMD_SUCCESS;
    UINT64              totalDiskSize = 0;
    UINT8               hddId, phyDiskCnt = 0;
    UINT8               raidGrpId;

    if ((GetHddVolBuildStatus() == TRUE) || (newHddConfig->mode >= MAX_HDD_MODE))
    {
        EPRINT(DISK_MANAGER, "hdd build process is going on or invld mode: [mode=%d]", newHddConfig->mode);
        return CMD_PROCESS_ERROR;
    }

    if (oldHddConfig->mode == newHddConfig->mode)
    {
        return CMD_SUCCESS;
    }

    if (newHddConfig->mode == SINGLE_DISK_VOLUME)
    {
        return CMD_SUCCESS;
    }

    retVal = CMD_HDD_PORT_COMBINATION_WRONG;
    for(raidGrpId = 0; raidGrpId < getTotalRaidVolume(newHddConfig->mode); raidGrpId++)
    {
        phyDiskCnt = 0;
        totalDiskSize = 0;
        for(hddCnt = 0; hddCnt < raidArrayDiskCnt[newHddConfig->mode][RAID_DISK_CNT_MAX]; hddCnt++)
        {
            hddId = (raidGrpId * raidArrayDiskCnt[newHddConfig->mode][RAID_DISK_CNT_MAX]) + hddCnt;
            MUTEX_LOCK(physicalMediaInfo[hddId].phyMediaMutex);
            if(physicalMediaInfo[hddId].diskState == ADDED)
            {
                phyDiskCnt++;
                totalDiskSize += physicalMediaInfo[hddId].physicalSize;
            }
            MUTEX_UNLOCK(physicalMediaInfo[hddId].phyMediaMutex);
        }

        if (phyDiskCnt < raidArrayDiskCnt[newHddConfig->mode][RAID_DISK_CNT_MIN])
        {
            continue;
        }

        /* We can implement raid array */
        return CMD_SUCCESS;
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function invokes whenever configuration was changed of Hard disk config.
 * @param   newHddConfig
 * @param   oldHddConfig
 * @return  SUCCESS/FAIL
 */
BOOL DmHddCfgUpdate(HDD_CONFIG_t *newHddConfig, HDD_CONFIG_t *oldHddConfig)
{
    if (oldHddConfig->mode != newHddConfig->mode)
    {
        if (newHddConfig->mode >= MAX_HDD_MODE)
        {
            EPRINT(DISK_MANAGER, "invld recording mode: [mode=%d]", newHddConfig->mode);
            return FALSE;
        }

        DPRINT(DISK_MANAGER, "recording hdd mode changed: [old=%s], [new=%s]", storageModeStr[oldHddConfig->mode], storageModeStr[newHddConfig->mode]);
        SetHddNonFuncStatus(TRUE);
        SetHddVolBuildStatus(TRUE);

        /* Default storage allocation config */
        DfltStorageAllocationConfig();

        /* stop file access services */
        UpdateServices(STOP);

        /* Remove all mdadm config files */
        removeAllMdadmConfFile();

        /* Unmount all the disks */
        removeAllHardDisk(oldHddConfig->mode);

        /* restart local disk mode with updated mode type */
        hddCfgChng.isHddModeChanged = TRUE;
        hddCfgChng.oldHddMode = oldHddConfig->mode;
        performHddConfigMode();
    }
    else if(oldHddConfig->recordDisk != newHddConfig->recordDisk)
    {
        DPRINT(DISK_MANAGER, "recording drive config changed: [old=%d], [new=%d]", oldHddConfig->recordDisk, newHddConfig->recordDisk);
        SetHddNonFuncStatus(TRUE);
        SetHddVolBuildStatus(TRUE);

        /* Update storage volume information. Update in which disk camera recording was enable */
        UpdateLocalStorageVolumeInfo(FALSE, TRUE);
        UpdateRecordingMedia(LOCAL_HARD_DISK);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @return  TRUE/FALSE
 */
BOOL GetHddVolBuildStatus(void)
{
    MUTEX_LOCK(diskVolBuildMutex);
    BOOL retVal = diskVolBuildStatus;
    MUTEX_UNLOCK(diskVolBuildMutex);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @param   status
 */
void SetHddVolBuildStatus(BOOL status)
{
    MUTEX_LOCK(diskVolBuildMutex);
    diskVolBuildStatus = status;
    MUTEX_UNLOCK(diskVolBuildMutex);
    DPRINT(DISK_MANAGER, "storage volume status: %s", status ? "non-usable" : "usable");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function starts the media detect thread. The purpose was to wait some time for detecting
 *          all connected disks from kernel module.
 * @param   data
 */
static void storageMediaDetectWaitTimerCb(UINT32 data)
{
    /* Handle media detect wait retry */
    mediaDetectWaitRetryCnt++;
    if (mediaDetectWaitRetryCnt < MEDIA_DETECT_RETRY_CNT)
    {
        /* Is required media device detected? */
        if (mediaDeviceDetectCnt < totalAttachedDevice)
        {
            /* Wait for all device detection */
            WPRINT(DISK_MANAGER, "waiting for storage devices to detect: [attached=%d], [detected=%d], [retry=%d]",
                   totalAttachedDevice, mediaDeviceDetectCnt, mediaDetectWaitRetryCnt);
            return;
        }

        /* All required devices are detected */
        DPRINT(DISK_MANAGER, "all storage devices detected: [attached=%d], [detected=%d], [retry=%d]",
               totalAttachedDevice, mediaDeviceDetectCnt, mediaDetectWaitRetryCnt);
    }
    else
    {
        /* Required devices are not detected till timeout */
        EPRINT(DISK_MANAGER, "max retry reached, fail to identify all storage devices: [attached=%d], [detected=%d]",
               totalAttachedDevice, mediaDeviceDetectCnt);
    }

    /* Delete the timer and start init of detected devices */
    mediaDetectWaitRetryCnt = 0;
    DeleteTimer(&mediaDetectWaitTimerHandle);
    if (FAIL == Utils_CreateThread(&initStorageThreadId, initStorageMediaThread, NULL, JOINABLE_THREAD, INIT_STORAGE_THREAD_STACK_SZ))
    {
        EPRINT(DISK_MANAGER, "fail to create media storage init thread");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reset storage volume information
 * @param   pMediaInfo
 * @param   mediaStatus
 * @param   diskVolStatus
 */
static void resetStorageVolumeInfo(RAW_MEDIA_INFO_t *pMediaInfo, RAW_MEDIA_STATUS_e mediaStatus, DISK_VOL_STATUS_e diskVolStatus)
{
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    pMediaInfo->mediaStatus = mediaStatus;
    pMediaInfo->diskVolStatus = diskVolStatus;
    memset(pMediaInfo->mountPoint, 0, sizeof(pMediaInfo->mountPoint));
    pMediaInfo->diskFormatPercentage = 0;
    pMediaInfo->totalVolSize = DEFAULT_DISK_SIZE;
    pMediaInfo->freeSize = DEFAULT_DISK_SIZE;
    pMediaInfo->usedSize = DEFAULT_DISK_SIZE;
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was thread function which invokes whenever any storage media was inserted
 *          or removed from system, it was storage media and implement file system as per configuration.
 *          It will create a thread for implementing file system on storage media.
 * @param   threadArg
 * @return
 */
static VOIDPTR initStorageMediaThread(VOIDPTR threadArg)
{
    UINT8					usbDiskCnt;
    STORAGE_MEDIA_TYPE_e	diskCnt;
    HDD_CONFIG_t			hddConfig;

    THREAD_START("INIT_STORAGE");

    /* Bydefault linux kernel creates /dev/md0 node. Stop it */
    stopRaidArray(RAID_VOLUME_1);

    /* Check raid implementation, if it is legacy then migrate it */
    if (TRUE == isLegacyRaidArrayPresent())
    {
        /* Migrate legacy raid to new raid */
        legacyRaidConfigMigration();
    }

    performHddConfigMode();
    while(TRUE)
    {
        for(usbDiskCnt = DM_MANUAL_BACKUP; usbDiskCnt < DM_MAX_BACKUP; usbDiskCnt++)
        {
            diskCnt = (usbDiskCnt + MANUAL_BACKUP_DISK);
            MUTEX_LOCK(physicalMediaInfo[diskCnt].phyMediaMutex);
            if(physicalMediaInfo[diskCnt].diskState == UNKNOWN)
            {
                MUTEX_UNLOCK(physicalMediaInfo[diskCnt].phyMediaMutex);
                continue;
            }
            MUTEX_UNLOCK(physicalMediaInfo[diskCnt].phyMediaMutex);

            MUTEX_LOCK(usbThreadInfo[usbDiskCnt].usbMutex);
            if(usbThreadInfo[usbDiskCnt].threadActive == TRUE)
            {
                MUTEX_UNLOCK(usbThreadInfo[usbDiskCnt].usbMutex);
                continue;
            }
            usbThreadInfo[usbDiskCnt].threadActive = TRUE;
            MUTEX_UNLOCK(usbThreadInfo[usbDiskCnt].usbMutex);

            usbThreadInfo[usbDiskCnt].mediaNo = diskCnt;
            if (FAIL == Utils_CreateThread(NULL, updateBackupDevice, &usbThreadInfo[usbDiskCnt], DETACHED_THREAD, UPDATE_BKP_DEVICE_THREAD_STACK_SZ))
            {
                EPRINT(DISK_MANAGER, "fail to create backup disk thread: [usbDisk=%d]", usbDiskCnt);
                MUTEX_LOCK(usbThreadInfo[usbDiskCnt].usbMutex);
                usbThreadInfo[usbDiskCnt].threadActive = FALSE;
                MUTEX_UNLOCK(usbThreadInfo[usbDiskCnt].usbMutex);
            }
        }

        ReadHddConfig(&hddConfig);
        for(diskCnt = 0; diskCnt < getMaxHardDiskForCurrentVariant(); diskCnt++)
        {
            MUTEX_LOCK(physicalMediaInfo[diskCnt].phyMediaMutex);
            if(physicalMediaInfo[diskCnt].diskState != REMOVED)
            {
                MUTEX_UNLOCK(physicalMediaInfo[diskCnt].phyMediaMutex);
                continue;
            }
            physicalMediaInfo[diskCnt].diskPhyStatus = DM_HDD_NO_DISK;
            MUTEX_UNLOCK(physicalMediaInfo[diskCnt].phyMediaMutex);

            if (hddConfig.mode == SINGLE_DISK_VOLUME)
            {
                /* Reset storage volume information */
                resetStorageVolumeInfo(&storageMedia[diskCnt], NOT_PRESENT, DM_DISK_VOL_MAX);
            }
        }

        MUTEX_LOCK(storageCondMutex);
        if(stopStorageDevDetectF == TRUE)
        {
            MUTEX_UNLOCK(storageCondMutex);
            break;
        }

        if(newDevDetectSignalF == FALSE)
        {
            pthread_cond_wait(&storageCondSignal, &storageCondMutex);
        }

        newDevDetectSignalF = FALSE;
        if(stopStorageDevDetectF == TRUE)
        {
            MUTEX_UNLOCK(storageCondMutex);
            break;
        }
        MUTEX_UNLOCK(storageCondMutex);
    }

    ReadHddConfig(&hddConfig);
    removeAllHardDisk(hddConfig.mode);
    removeAllUsbDisk();
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function unmount all the hard disk and also stop the hard disk volume.
 * @param   hddMode
 * @return
 */
static BOOL removeAllHardDisk(HDD_MODE_e hddMode)
{
    UINT8               diskVol, totalVolCnt, volCnt, waitCntInSec = 30;
    RAW_MEDIA_INFO_t    *pMediaInfo;
    BOOL                mediaStatus[MAX_VOLUME];

    /* Unmount all storage volumes */
    DPRINT(DISK_MANAGER, "removing all hard disks: [mode=%s]", storageModeStr[hddMode]);

    /* Get total volumes */
    totalVolCnt = GetTotalDiskNumber(hddMode);

    /* all volume's current status */
    for(diskVol = 0; diskVol < totalVolCnt; diskVol++)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(hddMode, diskVol);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        mediaStatus[diskVol] = (pMediaInfo->mediaStatus == MOUNTED_READY) ? MOUNTED_READY : NOT_PRESENT;
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Make few retries to unmount and stop it */
    while(TRUE)
    {
        /* Init for a new retry */
        volCnt = 0;

        /* Check all volumes which are mounted or need to stop raid array */
        for(diskVol = 0; diskVol < totalVolCnt; diskVol++)
        {
            /* If media status is not present then we don't have to do anything */
            if (mediaStatus[diskVol] == NOT_PRESENT)
            {
                /* We have already proccessed this volume */
                volCnt++;
                continue;
            }

            /* Check media status of current volume. If mounted ready then first unmount it */
            pMediaInfo = GET_MEDIA_VOLUME_INFO(hddMode, diskVol);
            MUTEX_LOCK(pMediaInfo->rawMediaMutex);
            if (mediaStatus[diskVol] == MOUNTED_READY)
            {
                /* Try to unmount the volume */
                if(unmountDevice(pMediaInfo->mountPoint) == FAIL)
                {
                    /* Fail to unmount the volume */
                    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
                    continue;
                }

                /* Volume unmounted successfully */
                pMediaInfo->mediaStatus = UNMOUNTED;

                /* For raid mode, We have to stop the raid array. Now unmount is not required */
                mediaStatus[diskVol] = (hddMode == SINGLE_DISK_VOLUME) ? NOT_PRESENT : UNMOUNTED;
            }

            /* For raid mode, if volume is unmounted successfully then try to stop the raid */
            if ((hddMode != SINGLE_DISK_VOLUME) && (mediaStatus[diskVol] == UNMOUNTED))
            {
                /* Try to stop the raid array */
                if (stopRaidArray(diskVol) == FAIL)
                {
                    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
                    continue;
                }

                /* Raid array stopped successfully */
                mediaStatus[diskVol] = NOT_PRESENT;
            }
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

            /* Update success count */
            volCnt++;
        }

        /* Is all volumes umounted and stopped? */
        if (volCnt >= totalVolCnt)
        {
            /* Yes then provide the success status */
            break;
        }

        /* Update wait counter */
        waitCntInSec--;

        /* Is wait counter expired? */
        if (waitCntInSec == 0)
        {
            /* Fail to unmount all volumes */
            EPRINT(DISK_MANAGER, "fail to remove all hard disks: [mode=%s]", storageModeStr[hddMode]);
            return FAIL;
        }

        /* Wait for some time and retry */
        sleep(1);
    }

    /* Successfully unmounted all volumes */
    DPRINT(DISK_MANAGER, "all hard disks removed: [mode=%s]", storageModeStr[hddMode]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function unmount all the hard disk and also stop the hard disk volume.
 * @return
 */
static BOOL removeAllUsbDisk(void)
{
    BOOL	status = SUCCESS;
    UINT8	usbDiskCnt;

    for(usbDiskCnt = MANUAL_BACKUP_DISK; usbDiskCnt < MAX_RAW_MEDIA; usbDiskCnt++)
    {
        MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
        if(storageMedia[usbDiskCnt].mediaStatus == MOUNTED_READY)
        {
            if(unmountDevice(storageMedia[usbDiskCnt].mountPoint) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to remove backup disk: [usbDiskCnt=%d]", usbDiskCnt);
                status = FAIL;
            }
        }
        MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function performs the harddisk volume changes.
 */
static void performHddConfigMode(void)
{
    UINT8           hardDiskCnt;
    UINT64          diskSize;
    HDD_CONFIG_t    hddConfig;
    UINT8           raidVol;

    /* Read Hdd Config */
    ReadHddConfig(&hddConfig);
    if (hddConfig.mode >= MAX_HDD_MODE)
    {
        EPRINT(DISK_MANAGER, "invld recording mode: [mode=%d]", hddConfig.mode);
        return;
    }

    /* start file access services */
    UpdateServices(START);
    for(hardDiskCnt = 0; hardDiskCnt < getMaxHardDiskForCurrentVariant(); hardDiskCnt++)
    {
        MUTEX_LOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
        if(physicalMediaInfo[hardDiskCnt].diskState == ADDED)
        {
            if(getSizeOfDisk(physicalMediaInfo[hardDiskCnt].mediaNodeName, &diskSize) == SUCCESS)
            {
                physicalMediaInfo[hardDiskCnt].physicalSize = (diskSize / KILO_BYTE);
                physicalMediaInfo[hardDiskCnt].diskPhyStatus = DM_HDD_DISK_NORMAL;
                storageMedia[hardDiskCnt].diskVolStatus = DM_DISK_VOL_MAX;
            }
        }
        MUTEX_UNLOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
    }

    /* Set volume status as under building */
    SetHddVolBuildStatus(TRUE);

    if (hddConfig.mode == SINGLE_DISK_VOLUME)
    {
        for(hardDiskCnt = 0; hardDiskCnt < MAX_HDD_VOL; hardDiskCnt++)
        {
            /* Reset storage volume information */
            resetStorageVolumeInfo(&storageMedia[hardDiskCnt], NOT_PRESENT, DM_DISK_VOL_MAX);
        }

        /* Create a thread which will implement the single disk volume on detected disk */
        if (FAIL == Utils_CreateThread(NULL, impSingleDiskVolume, NULL, DETACHED_THREAD, IMP_SINGLE_DISK_THREAD_STACK_SZ))
        {
            SetHddVolBuildStatus(FALSE);
            EPRINT(DISK_MANAGER, "fail to create single disk volume thread");
        }
    }
    else
    {
        for(raidVol = 0; raidVol < MAX_VOLUME; raidVol++)
        {
            /* Reset storage volume information */
            resetStorageVolumeInfo(&raidVolumeInfo[raidVol], NOT_PRESENT, DM_DISK_VOL_MAX);
            raidVolumeInfo[raidVol].totalHardDisk = 0;
        }

        /* Create a thread which will implement the raid array volume on detected disk */
        if (FAIL == Utils_CreateThread(NULL, impRaidDiskVolume, NULL, DETACHED_THREAD, IMP_RAID_DISK_VOL_THREAD_STACK_SZ))
        {
            SetHddVolBuildStatus(FALSE);
            EPRINT(DISK_MANAGER, "fail to create raid disk volume thread");
        }
    }

    /* Update HDD mode to provide the logical volume status */
    DPRINT(UTILS, "hdd mode updated: [lastHddMode=%s], [newHddMode=%s]", storageModeStr[lastHddMode], storageModeStr[hddConfig.mode]);
    lastHddMode = hddConfig.mode;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function for implementing file system as per config. It will check that
 *          whether the disk was inserted  is suitable for system if this disk was not suitable then
 *          we need to format disk and create suitable for system. It was recover the entire file in
 *          hardisk and insert proper information in proper file. If event was present in stream file
 *          but corresponding references not present then it will put information of event into event file.
 * @param   threadArg
 * @return
 */
static VOIDPTR impSingleDiskVolume(VOIDPTR threadArg)
{
    BOOL 					status, isDiskFormatNeeded;
    BOOL 					forceAllDiskFormatF = FALSE;
    CHAR					mountPoint[MOUNT_POINT_SIZE];
    UINT8					hardDiskCnt;
    UINT8					partitionInfo[MAX_PARTITION_SUPPORT];
    PHYSICAL_DISK_STATUS_e	diskPhyStatus;
    DISK_SIZE_t				diskSizeInfo;
    PHYSICAL_DISK_STATUS_t	phyDiskInfo;
    CHAR					gptPratitionInfo[GPT_PARTITION_TYPE_LEN];
    HDD_CONFIG_t			hddConfig;

    THREAD_START("SINGLE DISK");
    if (TRUE == hddCfgChng.isHddModeChanged)
    {
        hddCfgChng.isHddModeChanged = FALSE;
        SetDiskHealthStatus(STRG_HLT_MAX);
        SetAllCameraVolumeHealthStatus(STRG_HLT_MAX);

        /* Read Hdd Config */
        ReadHddConfig(&hddConfig);
        WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, LOCAL_HARD_DISK), NULL, EVENT_DISK_BUSY);

        if((hddCfgChng.oldHddMode == RAID_0) || (hddCfgChng.oldHddMode == RAID_1) || (hddCfgChng.oldHddMode == RAID_5) || (hddCfgChng.oldHddMode == RAID_10))
        {
            forceAllDiskFormatF = TRUE;
        }
    }

    for(hardDiskCnt = 0; hardDiskCnt < getMaxHardDiskForCurrentVariant(); hardDiskCnt++)
    {
        MUTEX_LOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
        phyDiskInfo.diskState = physicalMediaInfo[hardDiskCnt].diskState;
        snprintf(phyDiskInfo.mediaNodeName, NODE_NAME_SIZE, "%s", physicalMediaInfo[hardDiskCnt].mediaNodeName);
        MUTEX_UNLOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);

        if(phyDiskInfo.diskState == ADDED)
        {
            do
            {
                status = FAIL;
                isDiskFormatNeeded = TRUE;
                if (forceAllDiskFormatF == TRUE)
                {
                    EPRINT(DISK_MANAGER, "recording mode changed: [media=%s]", mediaNameStr[hardDiskCnt]);
                    break;
                }

                if (getPartitionNoAndType(phyDiskInfo.mediaNodeName, partitionInfo) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "partition information not found: [media=%s]", mediaNameStr[hardDiskCnt]);
                    break;
                }

                if (partitionInfo[0] == NO_HDD_PARTITION)
                {
                    EPRINT(DISK_MANAGER, "no valid partition found: [media=%s]", mediaNameStr[hardDiskCnt]);
                    break;
                }

                if (partitionInfo[0] != NATIVE_HDD_FORMAT)
                {
                    /* Don't format the disk and generate the disk fault */
                    isDiskFormatNeeded = FALSE;
                    EPRINT(DISK_MANAGER, "native hdd formated disk not found: [media=%s]", mediaNameStr[hardDiskCnt]);
                    break;
                }

                if (getGptPartitionNoAndType(phyDiskInfo.mediaNodeName, gptPratitionInfo, 1, NULL) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to read gpt partition table: [media=%s]", mediaNameStr[hardDiskCnt]);
                    break;
                }

                if (FALSE == isValidGptGuidType(gptPratitionInfo))
                {
                    EPRINT(DISK_MANAGER, "no valid gpt partition present: [media=%s], [gpt=%s]", mediaNameStr[hardDiskCnt], gptPratitionInfo);
                    break;
                }

                /* Harddisk is ready to mount */
                isDiskFormatNeeded = FALSE;
                DPRINT(DISK_MANAGER, "native hdd formated disk found: [media=%s]", mediaNameStr[hardDiskCnt]);
                snprintf(phyDiskInfo.partitionNode, NODE_NAME_SIZE, PARTITION_NODE, phyDiskInfo.mediaNodeName);

                /* Store partition node for future use */
                MUTEX_LOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
                snprintf(physicalMediaInfo[hardDiskCnt].partitionNode, NODE_NAME_SIZE, "%s", phyDiskInfo.partitionNode);
                MUTEX_UNLOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);

                /* Make created partition name */
                snprintf(mountPoint, MOUNT_POINT_SIZE, HDD_DISK_MNT, (hardDiskCnt + 1));
                MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
                snprintf(storageMedia[hardDiskCnt].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
                MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

                /* Mount the device */
                status = mountDevice(phyDiskInfo.partitionNode, mountPoint, EXT_4);
                if (status == FAIL)
                {
                    EPRINT(DISK_MANAGER, "mounting fail: [media=%s]", mediaNameStr[hardDiskCnt]);
                    break;
                }

                MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
                storageMedia[hardDiskCnt].mediaStatus = MOUNTED_READY;
                MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

                /* Check signature was present */
                if(VerifyHddSignature(mountPoint) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "signature not proper: [media=%s]", mediaNameStr[hardDiskCnt]);
                    isDiskFormatNeeded = TRUE;
                    break;
                }

            }while(0);

            /* Make partitions and format the disk */
            if (TRUE == isDiskFormatNeeded)
            {
                status = makeHarddiskReady(hardDiskCnt, NULL);
            }

            if (status == SUCCESS)
            {
                MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
                snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", storageMedia[hardDiskCnt].mountPoint);
                MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

                status = GetSizeOfMountFs(mountPoint, &diskSizeInfo);
                if (status == FAIL)
                {
                    EPRINT(DISK_MANAGER, "size of disk not proper: [media=%s]", mediaNameStr[hardDiskCnt]);
                    if(unmountDevice(mountPoint) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to unmount: [media=%s]", mediaNameStr[hardDiskCnt]);
                    }
                }
            }

            if (status == SUCCESS)
            {
                diskPhyStatus = DM_HDD_DISK_NORMAL;
                MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
                storageMedia[hardDiskCnt].mediaStatus = MOUNTED_READY;
                storageMedia[hardDiskCnt].diskVolStatus = (diskSizeInfo.freeSize >= HDD_NORMAL_STATUS_SIZE) ? DM_DISK_VOL_NORMAL : DM_DISK_VOL_FULL;
                storageMedia[hardDiskCnt].totalVolSize = diskSizeInfo.totalSize;
                storageMedia[hardDiskCnt].freeSize = diskSizeInfo.freeSize;
                storageMedia[hardDiskCnt].usedSize = diskSizeInfo.usedSize;
                MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);
                DPRINT(DISK_MANAGER, "media mounted successfully: [media=%s]", mediaNameStr[hardDiskCnt]);
            }
            else
            {
                diskPhyStatus = DM_HDD_DISK_FAULT;
                MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
                storageMedia[hardDiskCnt].mediaStatus = UNMOUNTED;
                storageMedia[hardDiskCnt].diskVolStatus = DM_DISK_VOL_FAULT;
                MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

                /* Remove folder if present */
                snprintf(mountPoint, MOUNT_POINT_SIZE, HDD_DISK_MNT, (hardDiskCnt + 1));
                rmdir(mountPoint);
                EPRINT(DISK_MANAGER, "media is faulty: [media=%s]", mediaNameStr[hardDiskCnt]);
            }
        }
        else
        {
            diskPhyStatus = DM_HDD_NO_DISK;
            MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
            storageMedia[hardDiskCnt].mediaStatus = NOT_PRESENT;
            storageMedia[hardDiskCnt].diskVolStatus = DM_DISK_VOL_INCOMP_VOLUME;
            MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

            /* Remove folder if present */
            snprintf(mountPoint, MOUNT_POINT_SIZE, HDD_DISK_MNT, (hardDiskCnt + 1));
            rmdir(mountPoint);
            DPRINT(DISK_MANAGER, "media is not present: [media=%s]", mediaNameStr[hardDiskCnt]);
        }

        MUTEX_LOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
        physicalMediaInfo[hardDiskCnt].diskPhyStatus = diskPhyStatus;
        MUTEX_UNLOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
    }

    /* Update disk id in which disk recording was enabled */
    UpdateLocalStorageVolumeInfo(FALSE, TRUE);

    /* Now storage is ready for recording */
    UpdateRecordingMedia(LOCAL_HARD_DISK);

    /* Exit from the thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This will make harddisk ready for use.
 * @param   hardDiskCnt
 * @param   advncDetail
 * @return  SUCCESS/FAIL
 */
static BOOL makeHarddiskReady(UINT8 hardDiskCnt, CHARPTR advncDetail)
{
    BOOL                retVal = FAIL;
    CHAR                detail[MAX_EVENT_DETAIL_SIZE];
    CHAR                mediaNodeName[NODE_NAME_SIZE];
    CHAR                partitionNode[NODE_NAME_SIZE];
    CHAR                mountPoint[MOUNT_POINT_SIZE];
    RAW_MEDIA_STATUS_e  mediaStatus;

    /* Set LED cadence for disk format */
    SetSystemStatusLed(SYS_FORMATTING, ON);

    do
    {
        MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        mediaStatus = storageMedia[hardDiskCnt].mediaStatus;
        snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", storageMedia[hardDiskCnt].mountPoint);
        MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

        if(mediaStatus == MOUNTED_READY)
        {
            /* Unmount running device by taking care of ftp service */
            if (FAIL == unmountRunningDevice(mountPoint))
            {
                DPRINT(DISK_MANAGER, "fail to unmount: [media=%s]", mediaNameStr[hardDiskCnt]);
                break;
            }
        }

        /* create one partition with default size */
        resetStorageVolumeInfo(&storageMedia[hardDiskCnt], CREATING_PARTITION, DM_DISK_VOL_CREATING);
        if (GetHddNonFuncStatus() == TRUE)
        {
            SetDiskHealthStatus(STRG_HLT_MAX);
            SetCameraVolumeHealthStatus(hardDiskCnt, STRG_HLT_MAX);
            WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(SINGLE_DISK_VOLUME, LOCAL_HARD_DISK), NULL, EVENT_DISK_BUSY);
        }

        MUTEX_LOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
        snprintf(mediaNodeName, NODE_NAME_SIZE, "%s", physicalMediaInfo[hardDiskCnt].mediaNodeName);
        MUTEX_UNLOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
        DPRINT(DISK_MANAGER, "creating partition: [media=%s]", mediaNameStr[hardDiskCnt]);
        if(createPartition(mediaNodeName, physicalMediaInfo[hardDiskCnt].physicalSize) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to create partition: [media=%s]", mediaNameStr[hardDiskCnt]);
            break;
        }

        snprintf(partitionNode, NODE_NAME_SIZE, PARTITION_NODE, mediaNodeName);
        MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        storageMedia[hardDiskCnt].mediaStatus = DISK_FORMATTING;
        storageMedia[hardDiskCnt].diskVolStatus = DM_DISK_VOL_FORMATTING;
        MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        DPRINT(DISK_MANAGER, "media formating: [media=%s]", mediaNameStr[hardDiskCnt]);

        /* Waiting for Disk node to ready */
        sleep(5);

        /* format the device with ext4 file system */
        if(formatWithExt4(partitionNode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to format: [media=%s]", mediaNameStr[hardDiskCnt]);
            break;
        }

        MUTEX_LOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);
        snprintf(physicalMediaInfo[hardDiskCnt].partitionNode, NODE_NAME_SIZE, "%s", partitionNode);
        MUTEX_UNLOCK(physicalMediaInfo[hardDiskCnt].phyMediaMutex);

        MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        storageMedia[hardDiskCnt].mediaStatus = UNMOUNTED;
        MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);

        snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", (hardDiskCnt + 1));
        WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_VOLUME, detail, advncDetail, EVENT_FORMAT);

        snprintf(mountPoint, MOUNT_POINT_SIZE, HDD_DISK_MNT, (hardDiskCnt + 1));
        MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        snprintf(storageMedia[hardDiskCnt].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
        MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        DPRINT(DISK_MANAGER, "media mounting: [media=%s]", mediaNameStr[hardDiskCnt]);

        /* mount the device */
        if(mountDevice(partitionNode, mountPoint, EXT_4) != SUCCESS)
        {
            EPRINT(DISK_MANAGER, "fail to mount: [media=%s]", mediaNameStr[hardDiskCnt]);
            break;
        }

        MUTEX_LOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        storageMedia[hardDiskCnt].mediaStatus = MOUNTED_READY;
        MUTEX_UNLOCK(storageMedia[hardDiskCnt].rawMediaMutex);
        DPRINT(DISK_MANAGER, "write signature: [media=%s]", mediaNameStr[hardDiskCnt]);

        /* now we need to write our signature */
        if(WriteHddSignature(mountPoint) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to write signature: [media=%s]", mediaNameStr[hardDiskCnt]);
            break;
        }

        retVal = SUCCESS;
    }
    while(0);

    /* Stop LED cadence for disk format */
    SetSystemStatusLed(SYS_FORMATTING, OFF);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Unmount the running device by taking care of service start stop
 * @param   mountPoint
 * @return
 */
static BOOL unmountRunningDevice(CHARPTR mountPoint)
{
    BOOL status;

    /* Stop file access services */
    UpdateServices(STOP);

    /* Unmount the media */
    status = unmountDevice(mountPoint);

    /* Start file access services */
    UpdateServices(START);

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on configuration this function sets working disk size, mount point information.
 * @param   logEventF - Generate and log event or not
 * @note    Earlier there was no version of file. Hence, Marsked that version as 0 and start
 *          maintaining version from file version 1
 */
void UpdateLocalStorageVolumeInfo(BOOL isStorageCfgChange, BOOL logEventF)
{
    BOOL                    status = FAIL;
    INT32                   fileFd;
    UINT8                   cameraIndex, volGrpId, totalVolCnt;
    UINT8                   totalCamera = getMaxCameraForCurrentVariant();
    CHAR                    mountPoint[MOUNT_POINT_SIZE];
    RECORD_DISK_INFO_t      diskIdInfo = {0};
    LOG_EVENT_STATE_e       evtStatus;
    HDD_CONFIG_t            hddConfig;
    RAW_MEDIA_INFO_t        *pMediaInfo = NULL;
    STORAGE_HEALTH_STATUS_e healthStatus;

    ReadStorageAllocationConfig(&storageAllocCfg);
    ReadHddConfig(&hddConfig);
    totalVolCnt = GetTotalDiskNumber(hddConfig.mode);

    do
    {
        /* Open disk info file to get the last recording volume of all cameras */
        fileFd = open(CURRENT_RECORD_DISK_INFO, READ_ONLY_MODE, FILE_PERMISSION);
        if(fileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open file: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
            break;
        }

        /* Read record disk file info */
        if(read(fileFd, &diskIdInfo, sizeof(RECORD_DISK_INFO_t)) != sizeof(RECORD_DISK_INFO_t))
        {
            EPRINT(DISK_MANAGER, "fail to read file info: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
            break;
        }

        /* Is valid record disk file version? */
        if (diskIdInfo.version != RECORD_DISK_INFO_VERSION)
        {
            EPRINT(DISK_MANAGER, "invld record disk file version: [path=%s], [version=%d]", CURRENT_RECORD_DISK_INFO, RECORD_DISK_INFO_VERSION);
            break;
        }

        /* Is recording mode changed? */
        if (diskIdInfo.mode != hddConfig.mode)
        {
            EPRINT(DISK_MANAGER, "recording mode is changed: [config=%d], [stored=%d]", hddConfig.mode, diskIdInfo.mode);
            break;
        }

        /* There is no change in recording mode */
        status = SUCCESS;
        DPRINT(DISK_MANAGER, "no change in recording mode: [mode=%d]", hddConfig.mode);

    }while(0);

    /* Close the file */
    CloseFileFd(&fileFd);

    do
    {
        /* Is any storage volume present for recording? */
        if (FALSE == isAnyStorageVolumePresent(hddConfig.mode, &healthStatus))
        {
            DPRINT(DISK_MANAGER, "no volume found for recording: [mode=%d]", hddConfig.mode);
            break;
        }

        /* Set storage volume for all cameras */
        for (cameraIndex = 0; cameraIndex < totalCamera; cameraIndex++)
        {
            /* If recording mode is changed then we don't have any preferred volume. Hence provide invalid volume.
             * Get camera's storage volume by providing last recording volume as preferred volume */
            if ((FAIL == status) || (isStorageCfgChange == TRUE))
            {
                diskIdInfo.volumeId[cameraIndex] = MAX_VOLUME;
            }

            diskIdInfo.volumeId[cameraIndex] = getNormalAllocatedCameraVolume(cameraIndex, hddConfig.mode, diskIdInfo.volumeId[cameraIndex]);
            if (diskIdInfo.volumeId[cameraIndex] >= MAX_VOLUME)
            {
                EPRINT(DISK_MANAGER, "no valid volume for camera: [camera=%d], [mode=%d]", cameraIndex, hddConfig.mode);
            }
        }

    }while(0);

    /* Set system storage health status */
    SetDiskHealthStatus(healthStatus);

    /* Check storage health status */
    if (healthStatus != STRG_HLT_NO_DISK)
    {
        /* After building media (Single Disk or Raid), check all volume status, if any volume is faulty then generate alert */
        evtStatus = (healthStatus == STRG_HLT_ERROR) ? EVENT_FAULT : EVENT_NORMAL;

        /* Check allocated volume of all cameras */
        for (cameraIndex = 0; cameraIndex < totalCamera; cameraIndex++)
        {
            /* Is valid volume allocated to camera? */
            if (diskIdInfo.volumeId[cameraIndex] < totalVolCnt)
            {
                /* PARASOFT : No need to validate tainted data */
                /* Valid volume is available for camera. Store the volume info for camera's recording */
                pMediaInfo = GET_MEDIA_VOLUME_INFO(hddConfig.mode, diskIdInfo.volumeId[cameraIndex]);
                MUTEX_LOCK(pMediaInfo->rawMediaMutex);
                snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", pMediaInfo->mountPoint);
                MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

                MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
                snprintf(storageDiskInfo[cameraIndex].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
                storageDiskInfo[cameraIndex].currentRecordVolumeId = diskIdInfo.volumeId[cameraIndex];
                storageDiskInfo[cameraIndex].healthStatus = STRG_HLT_DISK_NORMAL;
                MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
                DPRINT(DISK_MANAGER, "camera recording volume allocated: [camera=%d], [volume=%d], [volumeMask=0x%X]",
                       cameraIndex, diskIdInfo.volumeId[cameraIndex], GetCameraStorageVolumeAllocationMask(cameraIndex, hddConfig.mode));
            }
            else
            {
                /* Get camera's storage volume group and set camera's storage health status. */
                volGrpId = GetCameraStorageAllocationGroup(cameraIndex, hddConfig.mode);
                if ((volGrpId >= STORAGE_ALLOCATION_GROUP_MAX) || (FALSE == isStorageGroupVolumePresent(volGrpId, hddConfig.mode)))
                {
                    healthStatus = STRG_HLT_NO_DISK;
                    WPRINT(DISK_MANAGER, "camera recording volume not present: [camera=%d], [volGrpId=%d]", cameraIndex, volGrpId);
                }
                else
                {
                    healthStatus = STRG_HLT_ERROR;
                    WPRINT(DISK_MANAGER, "camera recording volume faulty: [camera=%d], [volGrpId=%d]", cameraIndex, volGrpId);
                }

                /* No valid recording volume available for camera */
                MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
                RESET_STR_BUFF(storageDiskInfo[cameraIndex].mountPoint);
                storageDiskInfo[cameraIndex].currentRecordVolumeId = MAX_VOLUME;
                storageDiskInfo[cameraIndex].healthStatus = healthStatus;
                MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
            }
        }

        /* Dump the info in file for future use */
        DPRINT(DISK_MANAGER, "update storage volume in disk file for all cameras: [mode=%d]", hddConfig.mode);
        diskIdInfo.mode = hddConfig.mode;
        updateRecordDiskInfoFile(&diskIdInfo);
    }
    else
    {
        /* No recording volumes present in the system */
        evtStatus = EVENT_NO_DISK;
        DPRINT(DISK_MANAGER, "no volume present for recording: [mode=%d]", hddConfig.mode);
        for (cameraIndex = 0; cameraIndex < totalCamera; cameraIndex++)
        {
            MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
            RESET_STR_BUFF(storageDiskInfo[cameraIndex].mountPoint);
            storageDiskInfo[cameraIndex].currentRecordVolumeId = MAX_VOLUME;
            storageDiskInfo[cameraIndex].healthStatus = STRG_HLT_NO_DISK;
            MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        }
    }

    /* Do we need event? */
    if (FALSE == logEventF)
    {
        /* Event is not needed */
        return;
    }

    /* Write the event and if faulty volume found then generate the disk fault alert */
    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, LOCAL_HARD_DISK), NULL, evtStatus);
    if (evtStatus == EVENT_FAULT)
    {
        handleDiskFaultAlert(ACTIVE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Migrate current recording disk information file to camera-wise recording disk info file
 */
static void migrateCurrentRecordDiskInfo(void)
{
    const CHARPTR oldDiskInfoFile = "/etc/.diskId"; /* It is old file name */

    /* Is old disk info file present? */
    if (access(oldDiskInfoFile, F_OK) != STATUS_OK)
    {
        /* Old record info file is not present. Migration is not needed. */
        return;
    }

    /* This is legacy record disk info structure and now not needed anymore */
    typedef struct
    {
        UINT32  diskId : 8;
        UINT32  mode : 8;
        UINT32  reserved1 : 16;
    }LEGACY_RECORD_DISK_INFO_t;

    UINT8                       cameraIndex;
    LEGACY_RECORD_DISK_INFO_t   legacyRecordDiskInfo;
    RECORD_DISK_INFO_t          recordDiskInfo;

    do
    {
        /* Open disk info file for migration */
        INT32 fileFd = open(oldDiskInfoFile, READ_WRITE_MODE, FILE_PERMISSION);
        if(fileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open file, migration failed: [path=%s], [err=%s]", oldDiskInfoFile, STR_ERR);
            break;
        }

        /* Read disk file version info */
        if(read(fileFd, &legacyRecordDiskInfo, sizeof(LEGACY_RECORD_DISK_INFO_t)) != sizeof(LEGACY_RECORD_DISK_INFO_t))
        {
            EPRINT(DISK_MANAGER, "fail to read file, migration failed: [path=%s], [err=%s]", oldDiskInfoFile, STR_ERR);
            close(fileFd);
            break;
        }

        /* Close the file */
        close(fileFd);

        /* Copy old version info to new version info */
        DPRINT(DISK_MANAGER, "old disk info file found, migrate it");
        recordDiskInfo.version = RECORD_DISK_INFO_VERSION;
        recordDiskInfo.mode = legacyRecordDiskInfo.mode;
        for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            /* Set last recording volume for all cameras */
            recordDiskInfo.volumeId[cameraIndex] = legacyRecordDiskInfo.diskId;
        }

        /* Write info in new file */
        updateRecordDiskInfoFile(&recordDiskInfo);
        DPRINT(DISK_MANAGER, "old disk info file migrated in new record info file");

    }while(0);

    /* Remove old file */
    unlink(oldDiskInfoFile);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will update current recording disk info for all camera.
 * @param   pDiskIdInfo
 * @return  SUCCESS/FAIL
 */
static BOOL updateRecordDiskInfoFile(RECORD_DISK_INFO_t *pDiskIdInfo)
{
    INT32 fileFd;

    /* Open file in write mode and create the new one if doesn't exist */
    fileFd = open(CURRENT_RECORD_DISK_INFO, CREATE_WRITE_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        /* failed to open or create the file */
        EPRINT(DISK_MANAGER, "fail to open file: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
        return FAIL;
    }

    /* Update file version and write disk file info of cameras into the file */
    pDiskIdInfo->version = RECORD_DISK_INFO_VERSION;
    if (write(fileFd, pDiskIdInfo, sizeof(RECORD_DISK_INFO_t)) != sizeof(RECORD_DISK_INFO_t))
    {
        EPRINT(DISK_MANAGER, "fail to write file info: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    /* We have written data successfully. Close the file */
    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will update current recording disk info for single camera
 * @param   cameraIndex
 * @param   volumeId
 * @param   mode
 * @return  SUCCESS/FAIL
 */
static BOOL updateRecordDiskInfoFileForCamera(UINT8 cameraIndex, UINT8 volumeId, HDD_MODE_e mode)
{
    INT32               fileFd;
    RECORD_DISK_INFO_t  diskIdInfo;

    /* Open the file in read/write mode */
    fileFd = open(CURRENT_RECORD_DISK_INFO, READ_WRITE_MODE, FILE_PERMISSION);
    if (fileFd == INVALID_FILE_FD)
    {
        /* Failed to open the file */
        EPRINT(DISK_MANAGER, "fail to open file: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
        return FAIL;
    }

    /* Read the disk info from file */
    if (read(fileFd, &diskIdInfo, sizeof(RECORD_DISK_INFO_t)) != sizeof(RECORD_DISK_INFO_t))
    {
        EPRINT(DISK_MANAGER, "fail to read file info: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    /* Update recording mode and volume for camera */
    diskIdInfo.version = RECORD_DISK_INFO_VERSION;
    diskIdInfo.mode = mode;
    diskIdInfo.volumeId[cameraIndex] = volumeId;

    /* Dump the updated data into the file */
    if(write(fileFd, &diskIdInfo, sizeof(RECORD_DISK_INFO_t)) != sizeof(RECORD_DISK_INFO_t))
    {
        EPRINT(DISK_MANAGER, "fail to write file: [path=%s], [err=%s]", CURRENT_RECORD_DISK_INFO, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    /* We have written data successfully. Close the file */
    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates a one partition which has default size available on disk.
 * @param   deviceNode
 * @param   partitionSize
 * @return  SUCCESS/FAIL
 */
static BOOL createPartition(CHARPTR deviceNode, UINT64 partitionSize)
{
    UINT32  diskSize;
    CHAR    sysCmd[SYSTEM_COMMAND_SIZE];

    /* Example: parted -s /dev/sda mklabel gpt */
    snprintf(sysCmd, sizeof(sysCmd), PARTED_LABEL_CMD, deviceNode);
    if(FALSE == ExeSysCmd(TRUE, sysCmd))
    {
        return FAIL;
    }

    /* Derive exact value in GB */
    diskSize = ((partitionSize * KILO_BYTE)/(1000000000));
    if (diskSize > MAX_SUPPORTED_HDD_SIZE_GB)
    {
        WPRINT(DISK_MANAGER, "disk partition truncated: [node=%s], [diskSize=%d], [maxSupportedSize=%d]", deviceNode, diskSize, MAX_SUPPORTED_HDD_SIZE_GB);
        diskSize = MAX_SUPPORTED_HDD_SIZE_GB;
    }

    /* Example: parted -s /dev/sda mkpart primary ext4 100MB 1000GB */
    snprintf(sysCmd, sizeof(sysCmd), PARTED_PART_CMD, deviceNode, diskSize);
    if (FALSE == ExeSysCmd(TRUE, sysCmd))
    {
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "disk partition creation: [node=%s], [size=%d]", deviceNode, diskSize);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates a one partition which has default size available on disk.
 * @param   deviceNode
 * @return  SUCCESS/FAIL
 */
static BOOL formatWithExt4(CHARPTR deviceNode)
{
    CHAR sysCmd[SYSTEM_COMMAND_SIZE];

    /* Example: mkfs.ext4 -O uninit_bg -E lazy_itable_init=1 -F -t ext4 /dev/sda1 */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "%s -O uninit_bg -E lazy_itable_init=1 -F -t ext4 %s", formatTypeName[EXT_4], deviceNode);
    if (FALSE == ExeSysCmd(TRUE, sysCmd))
    {
        EPRINT(DISK_MANAGER, "fail to format media with ext4: [node=%s]", deviceNode);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function format the input device node with FAT32 file system.
 * @param   deviceNode
 * @return
 */
static BOOL formatWithFat(CHARPTR deviceNode)
{
    CHAR sysCmd[SYSTEM_COMMAND_SIZE];

    /* Example: mkfs.vfat /dev/sdb1 */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "%s %s", formatTypeName[FAT], deviceNode);
    if (FALSE == ExeSysCmd(TRUE, sysCmd))
    {
        EPRINT(DISK_MANAGER, "fail to format media with fat: [node=%s]", deviceNode);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function mounts the give device at given node in inpur parameters.
 * @param   deviceNode
 * @param   mountPath
 * @param   fsType
 * @return  SUCCESS/FAIL
 */
static BOOL mountDevice(CHARPTR deviceNode, CHARPTR mountPath, RAW_MEDIA_FORMAT_TYPE_e fsType)
{
    do
    {
        if(access(mountPath, F_OK) != STATUS_OK)
        {
            if(mkdir(mountPath, FOLDER_PERMISSION) != STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "fail to create dir: [node=%s], [path=%s], [err=%s]", deviceNode, mountPath, STR_ERR);
                break;
            }
        }

        if(mount(deviceNode, mountPath, fsTypeName[fsType], (MS_MGC_VAL | MS_NOATIME | MS_NODIRATIME), fsOptionsStr[fsType]) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to mount: [node=%s], [path=%s], [err=%s]", deviceNode, mountPath, STR_ERR);
            break;
        }

        /* Get information about mounted filesystem */
        struct statfs64 fs;
        if (statfs64(mountPath, &fs) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "statfs64 fail: [node=%s], [path=%s], [err=%s]", deviceNode, mountPath, STR_ERR);
            unmountDevice(mountPath);
            break;
        }

        /* Is filesystem mounted with read-only? */
        if ((fs.f_flags & ST_RDONLY) != 0)
        {
            EPRINT(DISK_MANAGER, "file system mounted read-only: [node=%s], [path=%s]", deviceNode, mountPath);
            unmountDevice(mountPath);
            break;
        }

        /* File system mounted with read and write */
        return SUCCESS;

    }while(0);

    EPRINT(DISK_MANAGER, "fail to mount device: [node=%s]", deviceNode);
    if (rmdir(mountPath) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "fail to remove mount point: [path=%s]", mountPath);
    }
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function unmount the device as from given mount point
 * @param   mountPath
 * @return  SUCCESS/FAIL
 */
static BOOL unmountDevice(CHARPTR mountPath)
{
    CHAR sysCmd[SYSTEM_COMMAND_SIZE];

    /* check mount point was present */
    if (access(mountPath, F_OK) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "mount point not present: [path=%s]", mountPath);
        return FAIL;
    }

    /* unmount device */
    if (umount(mountPath) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "fail to unmount device: [path=%s], [err=%s]", mountPath, STR_ERR);
        if(umount2(mountPath, MNT_FORCE) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to unmount device forcefully: [path=%s], [err=%s]", mountPath, STR_ERR);
            if(umount2(mountPath, MNT_DETACH) != STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "fail to detach device: [path=%s], [err=%s]", mountPath, STR_ERR);
                return FAIL;
            }
        }
    }

    /* remove mount point */
    if (rmdir(mountPath) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "fail to remove dir: [path=%s]", mountPath);
        snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "rm -rf %s", mountPath);
        if (ExeSysCmd(TRUE, sysCmd) == FALSE)
        {
            EPRINT(DISK_MANAGER, "fail to remove dir forcefully: [path=%s]", mountPath);
            return FAIL;
        }
    }

    DPRINT(DISK_MANAGER, "unmount device successfully: [path=%s]", mountPath);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It will check GPT partition GUID with matrix's supported GUID
 * @param   pGptGuidStr
 * @return  Returns TRUE if matched otherwise returns FALSE
 */
static BOOL isValidGptGuidType(CHARPTR pGptGuidStr)
{
    GPT_GUID_TYPE_e guidType;

    for (guidType = 0; guidType < GPT_GUID_TYPE_MAX; guidType++)
    {
        if (strcasecmp(pGptGuidStr, pGptGuidType[guidType]) == 0)
        {
            /* GPT partition GUID matched */
            return TRUE;
        }
    }

    /* GPT partition GUID not matched */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is gives partition information on given device node. It will gives only the
 *          information of all primary partition.
 * @param   deviceNode
 * @param   partitionInfo
 * @return  SUCCESS/FAIL
 */
static BOOL getPartitionNoAndType(CHARPTR deviceNode, UINT8PTR partitionInfo)
{
    INT32 fd, sectorSize;

    /* open the device node file */
    fd = open(deviceNode, READ_ONLY_MODE);
    if(fd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open file: [node=%s]", deviceNode);
        return FAIL;
    }

    /* get the sector size for reading master boot record */
    if(ioctl(fd, BLKSSZGET, &sectorSize) != 0)
    {
        EPRINT(DISK_MANAGER, "fail to read sector size: [err=%s]", STR_ERR);
        close(fd);
        return FAIL;
    }

    CHAR *mbr = malloc(sectorSize);
    if (mbr == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to alloc memory for sector size: [err=%s]", STR_ERR);
        close(fd);
        return FAIL;
    }

    /* read the master boot record */
    if(read(fd, mbr, sectorSize) != sectorSize)
    {
        EPRINT(DISK_MANAGER, "fail to read mbr data: [err=%s]", STR_ERR);
        close(fd);
        free(mbr);
        return FAIL;
    }

    /* gives the partition information */
    close(fd);
    partitionInfo[0] = mbr[PRIMARY_PARTITION_1];
    partitionInfo[1] = mbr[PRIMARY_PARTITION_2];
    partitionInfo[2] = mbr[PRIMARY_PARTITION_3];
    partitionInfo[3] = mbr[PRIMARY_PARTITION_4];
    free(mbr);

    DPRINT(DISK_MANAGER, "mbr info: [node=%s], [sectorSize=%d], [0x01c2=0x%X], [0x01d2=0x%X], [0x01e2=0x%X], [0x01f2=0x%X]",
           deviceNode, sectorSize, partitionInfo[0], partitionInfo[1], partitionInfo[2], partitionInfo[3]);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is gives partition information on given device node. It will gives only the
 *          information of all primary partition.
 * @param   deviceNode
 * @param   partitionInfo
 * @param   partitionMax
 * @param   blockSize
 * @return  SUCCESS/FAIL
 */
static BOOL getGptPartitionNoAndType(CHARPTR deviceNode, CHARPTR partitionInfo, UINT8 partitionMax, INT32 *blockSize)
{
    INT32       fd, sectorSize;
    CHAR        mbr[MAX_SECTOR_SIZE];
    UINT8       byteCnt, partCnt;
    INT32       bufCnt = 0;
    CHAR        buff[GPT_PARTITION_TYPE_LEN];
    const UINT8 diskGuidByteOrder[GPT_GUID_HEX_LEN] = {3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15};

    /* Open device node to read gpt information */
    fd = open(deviceNode, READ_ONLY_MODE);
    if (fd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open file: [node=%s]", deviceNode);
        return FAIL;
    }

    /* Get the sector size for reading master boot record */
    if(ioctl(fd, BLKSSZGET, &sectorSize) != 0)
    {
        EPRINT(DISK_MANAGER, "fail to read sector size: [err=%s]", STR_ERR);
        close(fd);
        return FAIL;
    }

    /* One is MBR and second is GUID Header */
    if (NULL != blockSize)
    {
        *blockSize = sectorSize;
    }

    /* Skip initial 2 sectors */
    sectorSize *= 2;
    if(lseek(fd, sectorSize, SEEK_CUR) != sectorSize)
    {
        EPRINT(DISK_MANAGER, "fail to seek file: [err=%s]", STR_ERR);
        close(fd);
        return FAIL;
    }

    for(partCnt = 0; partCnt < partitionMax; partCnt++)
    {
        /* Read the master boot record */
        if(read(fd, mbr, GPT_PRIMARY_PARTITION_SIZE) != GPT_PRIMARY_PARTITION_SIZE)
        {
            EPRINT(DISK_MANAGER, "fail to read gpt primary partition data: [partCnt=%d], [err=%s]", partCnt+1, STR_ERR);
            close(fd);
            return FAIL;
        }

        bufCnt = 0;
        memset(buff, 0, GPT_PARTITION_TYPE_LEN);

        /* Prepare disk GUID from initial 16 characters: Example: "0fc63daf-8483-4772-8e79-3d69d8477de4" */
        for (byteCnt = 0; byteCnt < GPT_GUID_HEX_LEN; byteCnt++)
        {
            /* Add '-' on specific places */
            if (byteCnt == 4 || byteCnt == 6 || byteCnt == 8 || byteCnt == 10)
            {
                bufCnt += snprintf(buff + bufCnt, GPT_PARTITION_TYPE_LEN - bufCnt, "-");
            }

            /* Convert hex to string */
            bufCnt += snprintf(buff + bufCnt, GPT_PARTITION_TYPE_LEN - bufCnt, "%02x", (UINT8)mbr[diskGuidByteOrder[byteCnt]]);
        }

        DPRINT(DISK_MANAGER, "gpt primary partition: [partCnt=%d], [info=%s]", partCnt+1, buff);
        snprintf(&partitionInfo[partCnt * GPT_PARTITION_TYPE_LEN], GPT_PARTITION_TYPE_LEN, "%s", buff);
    }

    close(fd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function for implementing Raid file system. It will also formats the hard
 *          disk if needed. It was recover the entire file in hardisk and insert proper information
 *          in proper file. If event was present in stream file but corresponding references not present
 *          then it will put information of event into event file.
 * @param   threadArg
 * @return
 */
static VOIDPTR impRaidDiskVolume(VOIDPTR threadArg)
{
    BOOL					  status, isHddModeCnfgChanged = FALSE, createNewRaidArray;
    UINT8					  hddId, hddStartId, hddEndId;
    CHAR					  mountPoint[MOUNT_POINT_SIZE];
    CHAR				   	  partitionNode[NODE_NAME_SIZE];
    HDD_CONFIG_t			  hddConfig;
    RAID_CREATE_INFO_t		  raidInfo;
    RAID_RESULT_t			  raidResult;
    RAID_VOLUME_GROUP_NO_e	  raidGrpId;
    RAID_VOLUME_NO_e		  raidGrpVol;
    UINT8					  totalPartition, hddPresentCnt, raidDiskCntMax;
    STORAGE_MEDIA_TYPE_e	  startDiskId, endDiskId, leastDiskSizeId;
    UINT64					  diskSize;
    HDD_RAID_INFO_t  		  hddPartitionData;

    THREAD_START("RAID_VOLUME");

    /* On bootup, this will be invalid value and mode change will be set on recording mode config change */
    if (TRUE == hddCfgChng.isHddModeChanged)
    {
        /* Recording mode changed. Hence we have to create new raid array */
        hddCfgChng.isHddModeChanged = FALSE;
        isHddModeCnfgChanged = TRUE;
        SetDiskHealthStatus(STRG_HLT_MAX);
        SetAllCameraVolumeHealthStatus(STRG_HLT_MAX);
    }

    /* Read recording mode configuration */
    ReadHddConfig(&hddConfig);
    raidDiskCntMax = raidArrayDiskCnt[hddConfig.mode][RAID_DISK_CNT_MAX];
    memset(&raidDiskPartInfo, 0, sizeof(raidDiskPartInfo));

    for(raidGrpId = 0; raidGrpId < getTotalRaidVolume(hddConfig.mode); raidGrpId++)
    {
        hddPresentCnt = 0;
        hddStartId = (raidGrpId * raidDiskCntMax);
        hddEndId = hddStartId + raidDiskCntMax;

        /* For raid0 and raid1 --> start disk: 0, 2, 4, 6 and end disk: 1, 3, 5, 7 and
         * For raid5 --> start disk: 0, 4 and end disk: 2 or 3, 6 or 7 and
         * For raid10 --> start disk: 0, 4 and end disk: 3 */
        status = SUCCESS;
        for(hddId = hddStartId; hddId < hddEndId; hddId++)
        {
            MUTEX_LOCK(physicalMediaInfo[hddId].phyMediaMutex);
            if(physicalMediaInfo[hddId].diskState == ADDED)
            {
                hddPresentCnt++;
            }
            else
            {
                /* If last disk of hdd group is not present in raid 5 then ok otherwise don't create the raid */
                if ((hddConfig.mode == RAID_5) && (hddId < (hddEndId - 1)))
                {
                    /* Don't create the raid */
                    status = FAIL;
                }
            }
            MUTEX_UNLOCK(physicalMediaInfo[hddId].phyMediaMutex);
        }

        /* If minimum required hard-disk are not present then don't create/assemble raid array */
        if ((status == FAIL) || (hddPresentCnt < raidArrayDiskCnt[hddConfig.mode][RAID_DISK_CNT_MIN]))
        {
            if (hddPresentCnt)
            {
                WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_VOLUME_AT_INIT, NULL, NULL, EVENT_MISSING_DISK);
                EPRINT(DISK_MANAGER, "insufficient disks to create raid: [mode=%d], [raidGrpId=%d], [hddPresentCnt=%d]",
                       hddConfig.mode, raidGrpId, hddPresentCnt);
            }

            for(raidGrpVol = 0; raidGrpVol < MAX_RAID_VOLUME; raidGrpVol++)
            {
                raidInfo.raidVolNo = GET_RAID_VOLUME_ID(raidGrpId, raidGrpVol);
                MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                raidVolumeInfo[raidInfo.raidVolNo].mediaStatus = NOT_PRESENT;
                MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
            }
            continue;
        }

        /* If config file of all volumes are not present then create fresh raid array with all disks */
        createNewRaidArray = TRUE;
        for(raidGrpVol = 0; raidGrpVol < MAX_RAID_VOLUME; raidGrpVol++)
        {
            if (TRUE == isMdadmConfigFilePresent(GET_RAID_VOLUME_ID(raidGrpId, raidGrpVol)))
            {
                createNewRaidArray = FALSE;
                break;
            }
        }

        /* Set raid array start disk and end disk */
        startDiskId = hddStartId;
        endDiskId = startDiskId + (hddPresentCnt - 1);

        /* Set raid start and end harddisk index */
        hddStartId = (raidGrpId * raidDiskCntMax);
        hddEndId = hddStartId + hddPresentCnt;

        /* If recording config changed or all config file of raid are not present then create fresh raid array */
        status = SUCCESS;
        if ((TRUE == isHddModeCnfgChanged) || (TRUE == createNewRaidArray))
        {
            DPRINT(DISK_MANAGER, "create fresh raid: [mode=%s], [raidGrpId=%d], [hddPresentCnt=%d], [modeConfigChanged=%s]",
                   storageModeStr[hddConfig.mode], raidGrpId, hddPresentCnt, isHddModeCnfgChanged ? "YES" : "NO");

            /* Compare all disk size which are part of raid array and derive lowest size disk */
            compareDiskSize(startDiskId, endDiskId, &leastDiskSizeId);
            MUTEX_LOCK(physicalMediaInfo[leastDiskSizeId].phyMediaMutex);
            diskSize = physicalMediaInfo[leastDiskSizeId].physicalSize;
            MUTEX_UNLOCK(physicalMediaInfo[leastDiskSizeId].phyMediaMutex);

            /* Derive possible partitions size for raid arrays */
            predictLogicalRaidVolume(hddConfig.mode, diskSize, hddPresentCnt, &hddPartitionData);
            for(hddId = hddStartId; hddId < hddEndId; hddId++)
            {
                /* Create all partitions in disk for raid arrays */
                raidDiskPartInfo[hddId] = hddPartitionData;
                status = createPartirionsForRaid(hddId);
                if (status == FAIL)
                {
                    break;
                }
            }

            /* Fail to create partitions for raid */
            if (FAIL == status)
            {
                EPRINT(DISK_MANAGER, "fail to create fresh raid: [mode=%d], [raidGrpId=%d], [hddPresentCnt=%d]", hddConfig.mode, raidGrpId, hddPresentCnt);
                for(raidGrpVol = 0; raidGrpVol < MAX_RAID_VOLUME; raidGrpVol++)
                {
                    updateRaidVolumeInfo(GET_RAID_VOLUME_ID(raidGrpId, raidGrpVol), FALSE);
                }

                /* Create next raid array */
                continue;
            }

            /* Get possible raid volumes for raid group and create raid array for all volumes */
            UINT8 totalRaidGrpVol = getPossibleRaidGroupVolumes(hddConfig.mode, hddPresentCnt, raidGrpId);
            DPRINT(DISK_MANAGER, "creating raid array partition: [mode=%d], [raidGrpId=%d], [totalPartition=%d]", hddConfig.mode, raidGrpId, totalRaidGrpVol);
            for(raidGrpVol = 0; raidGrpVol < totalRaidGrpVol && raidGrpVol < MAX_RAID_VOLUME; raidGrpVol++)
            {
                raidInfo.raidVolNo = GET_RAID_VOLUME_ID(raidGrpId, raidGrpVol);
                raidInfo.raidType = hddConfig.mode;
                raidInfo.hardDiskCnt = hddPresentCnt;
                raidInfo.totalRaidDevices = hddPresentCnt;

                totalPartition = 0;
                for(hddId = 0; hddId < hddPresentCnt; hddId++)
                {
                    snprintf(raidInfo.deviceNode[hddId], NODE_NAME_SIZE, "%s", raidDiskPartInfo[hddStartId + hddId].partitionInfo[raidGrpVol].partitionNodeName);
                    if (raidInfo.deviceNode[hddId][0] != '\0')
                    {
                        DPRINT(DISK_MANAGER, "node info: [mode=%d], [raidVolNo=%d], [node=%s]", hddConfig.mode, raidInfo.raidVolNo, raidInfo.deviceNode[hddId]);
                        totalPartition++;
                    }
                }

                /* Partitions are not present */
                if (totalPartition == 0)
                {
                    raidVolumeInfo[raidInfo.raidVolNo].totalHardDisk = 0;
                    DPRINT(DISK_MANAGER, "partitions are not present in any disk: [mode=%d], [raidVolNo=%d]", hddConfig.mode, raidInfo.raidVolNo);
                    continue;
                }

                /* Store total harddisk count */
                raidVolumeInfo[raidInfo.raidVolNo].totalHardDisk = hddPresentCnt;

                /* Create the raid array for volume */
                if (FAIL == createRaidPartition(&raidInfo))
                {
                    EPRINT(DISK_MANAGER, "fail to create raid volume: [mode=%d], [raidGrpId=%d], [raidVolNo=%d]", hddConfig.mode, raidGrpId, raidInfo.raidVolNo);
                }
            }

            /* Create next raid array */
            continue;
        }

        /* Get harddisk partitions information */
        for(hddId = hddStartId; hddId < hddEndId; hddId++)
        {
            if (getDiskPartitionsInfo(hddId) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to get partitions info: [mode=%d], [raidGrpId=%d], [hddId=%d]", hddConfig.mode, raidGrpId, hddId);
            }
        }

        for(raidGrpVol = 0; raidGrpVol < MAX_RAID_VOLUME; raidGrpVol++)
        {
            raidInfo.raidVolNo = GET_RAID_VOLUME_ID(raidGrpId, raidGrpVol);
            raidInfo.raidType = hddConfig.mode;
            raidInfo.hardDiskCnt = hddPresentCnt;
            raidInfo.totalRaidDevices = hddPresentCnt;

            /* Check volume is present or not for raid assemble */
            totalPartition = 0;
            for(hddId = 0; hddId < hddPresentCnt; hddId++)
            {
                snprintf(raidInfo.deviceNode[hddId], NODE_NAME_SIZE, "%s", raidDiskPartInfo[hddStartId + hddId].partitionInfo[raidGrpVol].partitionNodeName);
                if (raidInfo.deviceNode[hddId][0] != '\0')
                {
                    DPRINT(DISK_MANAGER, "node info: [mode=%d], [raidVolNo=%d], [node=%s]", hddConfig.mode, raidInfo.raidVolNo, raidInfo.deviceNode[hddId]);
                    totalPartition++;
                }
            }

            /* Partitions are not present */
            if (totalPartition == 0)
            {
                raidVolumeInfo[raidInfo.raidVolNo].totalHardDisk = 0;
                DPRINT(DISK_MANAGER, "partitions are not present in any disk: [mode=%d], [raidVolNo=%d]", hddConfig.mode, raidInfo.raidVolNo);
                continue;
            }

            /* Store total harddisk count */
            raidVolumeInfo[raidInfo.raidVolNo].totalHardDisk = hddPresentCnt;

            /* Get actual raid array disks with raid5 was created */
            if ((hddConfig.mode == RAID_5) && (SUCCESS == getActualRaidArrayDisks(raidInfo.raidVolNo, &raidInfo.totalRaidDevices)))
            {
                /* Update raid 5 built with total devices if mismatched with present harddisk */
                if (hddPresentCnt < raidInfo.totalRaidDevices)
                {
                    EPRINT(DISK_MANAGER, "insufficient disks to create raid5: [mode=%d], [raidVolNo=%d], [hddPresentCnt=%d], [totalRaidDevices=%d]",
                           hddConfig.mode, raidInfo.raidVolNo, hddPresentCnt, raidInfo.totalRaidDevices);
                    raidVolumeInfo[raidInfo.raidVolNo].totalHardDisk = 0;
                    continue;
                }

                DPRINT(DISK_MANAGER, "raid 5 creation info: [raidVolNo=%d], [hddPresentCnt=%d], [totalRaidDevices=%d]",
                       raidInfo.raidVolNo, hddPresentCnt, raidInfo.totalRaidDevices);
                raidVolumeInfo[raidInfo.raidVolNo].totalHardDisk = raidInfo.totalRaidDevices;
            }

            /* Update mdadm config files with new device nodes if required */
            updateMdadmConfFile(&raidInfo);

            /* Assemble raid array */
            if (FAIL == assembleRaid(raidInfo.raidVolNo))
            {
                /* Raid assemble failed but mdadm config is present then will not create fresh array. It will lost recording data */
                if (TRUE == isMdadmConfigFilePresent(raidInfo.raidVolNo))
                {
                    EPRINT(DISK_MANAGER, "raid assemble failed and mdadm config file is present: [mode=%d], [raidVolNo=%d], [hardDiskCnt=%d]",
                           hddConfig.mode, raidInfo.raidVolNo, hddId);
                    continue;
                }

                /* Raid assemble failed because mdadm config file is not present. It is possible when system rebooted during fresh array creation */
                EPRINT(DISK_MANAGER, "raid assemble failed because mdadm config file is not present: [mode=%d], [raidVolNo=%d], [hardDiskCnt=%d]",
                       hddConfig.mode, raidInfo.raidVolNo, hddId);

                /* Compare all disk size which are part of raid array and derive lowest size disk */
                compareDiskSize(startDiskId, endDiskId, &leastDiskSizeId);
                MUTEX_LOCK(physicalMediaInfo[leastDiskSizeId].phyMediaMutex);
                diskSize = physicalMediaInfo[leastDiskSizeId].physicalSize;
                MUTEX_UNLOCK(physicalMediaInfo[leastDiskSizeId].phyMediaMutex);

                /* Derive possible partitions size for raid arrays */
                predictLogicalRaidVolume(hddConfig.mode, diskSize, hddPresentCnt, &hddPartitionData);

                /* Get possible raid volumes for raid group and it should match with disk partitions */
                totalPartition = getPossibleRaidGroupVolumes(hddConfig.mode, hddPresentCnt, raidGrpId);
                if ((totalPartition != hddPartitionData.totalPartitions) || (raidDiskPartInfo[hddStartId].partitionInfo[raidGrpVol].partitionIndex == 0))
                {
                    EPRINT(DISK_MANAGER, "required partitions not available to create new raid array: [present=%d], [required=%d]",
                           totalPartition, hddPartitionData.totalPartitions);
                    continue;
                }

                /* Create new raid array with available partitions */
                if (createRaidPartition(&raidInfo) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "fail to create new raid array with available partitions: [mode=%d], [raidVolNo=%d], [totalPartition=%d]",
                           hddConfig.mode, raidInfo.raidVolNo, totalPartition);
                    continue;
                }

                /* New raid array created with available partitions */
                DPRINT(DISK_MANAGER, "new raid array created with available partitions: [mode=%d], [raidVolNo=%d], [totalPartition=%d]",
                       hddConfig.mode, raidInfo.raidVolNo, totalPartition);
                continue;
            }

            /* Raid assemble succcessfully */
            DPRINT(DISK_MANAGER, "raid assembled successfully: [mode=%d], [raidVolNo=%d], [hddPresentCnt=%d]", hddConfig.mode, raidInfo.raidVolNo, hddPresentCnt);

            /* NOTE: Must need some time for start recovery process */
            sleep(2);

            /* Check raid status after assembling it */
            if (FAIL == checkRaidStatus(&raidInfo, &raidResult))
            {
                EPRINT(DISK_MANAGER, "raid assemble status failed: [mode=%d], [raidVolNo=%d]", hddConfig.mode, raidInfo.raidVolNo);
                continue;
            }

            /* Raid assembled with degrade mode */
            if (raidResult.raidStatus == RAID_STATUS_DEGRADED)
            {
                /* If raid was created with more devices and now lesser disks are present then we can not add missing disk */
                if (hddPresentCnt < raidInfo.totalRaidDevices)
                {
                    EPRINT(DISK_MANAGER, "insufficient disks are present in raid degraded mode: [mode=%d], [raidVolNo=%d], [hddPresentCnt=%d], [totalRaidDevices=%d]",
                           hddConfig.mode, raidInfo.raidVolNo, hddPresentCnt, raidInfo.totalRaidDevices);
                    stopRaidArray(raidInfo.raidVolNo);
                    continue;
                }

                DPRINT(DISK_MANAGER, "raid assembled in degraded mode: [mode=%d], [raidVolNo=%d]", hddConfig.mode, raidInfo.raidVolNo);
                WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_VOLUME_AT_INIT, NULL, NULL, EVENT_MISSING_DISK);

                /* Find the missing disk based on present disk and raid assemble with list of disks */
                UINT8 sourceDisk = MAX_HDD_VOL, missingDisk = MAX_HDD_VOL;
                for (hddId = hddStartId; hddId < hddEndId; hddId++)
                {
                    /* If source and missing disks are found then stop searching */
                    if ((sourceDisk != MAX_HDD_VOL) && (missingDisk != MAX_HDD_VOL))
                    {
                        break;
                    }

                    /* Get hard-disk node for matching with raid array assembled with list of disk's node */
                    snprintf(partitionNode, NODE_NAME_SIZE, "%s", raidDiskPartInfo[hddId].partitionInfo[raidGrpVol].partitionNodeName);
                    if (partitionNode[0] == '\0')
                    {
                        missingDisk = hddId;
                        continue;
                    }

                    /* Node matches means this disk is already part of array */
                    if ((raidResult.deviceNode[0][0] != '\0') && (strstr(raidResult.deviceNode[0], partitionNode) != NULL))
                    {
                        sourceDisk = hddId;
                        continue;
                    }

                    /* Skip for raid 0 and raid 1 */
                    if (raidInfo.totalRaidDevices >= raidArrayDiskCnt[RAID_5][RAID_DISK_CNT_MIN])
                    {
                        if ((raidResult.deviceNode[1][0] != '\0') && (strstr(raidResult.deviceNode[1], partitionNode) != NULL))
                        {
                            continue;
                        }
                    }

                    /* Skip for raid 0, raid 1 and raid 5 with 3 disks */
                    if (raidInfo.totalRaidDevices >= raidArrayDiskCnt[RAID_10][RAID_DISK_CNT_MIN])
                    {
                        if ((raidResult.deviceNode[2][0] != '\0') && (strstr(raidResult.deviceNode[2], partitionNode) != NULL))
                        {
                            continue;
                        }
                    }

                    /* Node not matched for missing disk */
                    missingDisk = hddId;
                }

                /* Have we found source and missing disks? */
                if ((sourceDisk == MAX_HDD_VOL) || (missingDisk == MAX_HDD_VOL) || (sourceDisk == missingDisk))
                {
                    EPRINT(DISK_MANAGER, "source or missing disk not found: [sourceDisk=%d], [missingDisk=%d], [raidVolNo=%d]",
                           sourceDisk, missingDisk, raidInfo.raidVolNo);
                    continue;
                }

                DPRINT(DISK_MANAGER, "missing disk in raid: [sourceDisk=%s], [missingDisk=%s], [raidVolNo=%d]",
                       mediaNameStr[sourceDisk], mediaNameStr[missingDisk], raidInfo.raidVolNo);

                /* When fresh disk added in raid array, we will do partitions during first volume process only because
                 * it will create all required partitions for other volume also. */
                if (raidGrpVol == RAID_VOLUME_1)
                {
                    /* Verify required partitions are possible or not in missing disk */
                    if (verifyAndCreatePartitionsInMissingDisk(sourceDisk, missingDisk) == FAIL)
                    {
                        EPRINT(DISK_MANAGER, "fail to create partition in missing disk: [missingDisk=%s], [raidVolNo=%d]",
                               mediaNameStr[missingDisk], raidInfo.raidVolNo);
                        continue;
                    }
                }
                else
                {
                    /* No need to create partition because it is expected that it is already created during first volume process */
                    WPRINT(DISK_MANAGER, "raid assembled in degraded, partition already created, add missing disk: "
                                         "[mode=%d], [partition=%d], [raidVolNo=%d]", hddConfig.mode, raidGrpVol+1, raidInfo.raidVolNo);
                }

                snprintf(partitionNode, NODE_NAME_SIZE, "%s", raidDiskPartInfo[missingDisk].partitionInfo[raidGrpVol].partitionNodeName);
                MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                raidVolumeInfo[raidInfo.raidVolNo].mediaStatus = RAID_RESYNCING;
                MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);

                /* Add missing disk in raid array */
                if(addRaidDevice(raidInfo.raidVolNo, partitionNode) == FAIL)
                {
                    EPRINT(DISK_MANAGER, "device not added in raid array: [node=%s], [missingDisk=%s], [raidVolNo=%d]",
                           partitionNode, mediaNameStr[missingDisk], raidInfo.raidVolNo);
                    continue;
                }

                MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                raidVolumeInfo[raidInfo.raidVolNo].diskVolStatus = DM_DISK_VOL_CREATING;
                MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);

                /* Monitor raid array status continuesly till it gets completed */
                monitorRaidStatusContinues(&raidInfo, &raidResult);
            }

            if ((raidResult.raidStatus == RAID_STATUS_RECOVERY) || (raidResult.raidStatus == RAID_STATUS_RESYNCHING))
            {
                DPRINT(DISK_MANAGER, "raid building info: [mode=%d], [raidVolNo=%d], [status=%s]",
                       hddConfig.mode, raidInfo.raidVolNo, raidStatusStr[raidResult.raidStatus]);
                MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                raidVolumeInfo[raidInfo.raidVolNo].mediaStatus = RAID_RESYNCING;
                raidVolumeInfo[raidInfo.raidVolNo].diskVolStatus = DM_DISK_VOL_CREATING;
                MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);

                /* Monitor raid array status continuesly till it gets completed */
                monitorRaidStatusContinues(&raidInfo, &raidResult);
            }

            /* Is raid arry is not present or inrerrupted? */
            if ((raidResult.raidStatus == RAID_STATUS_NO_DEVICE) || (raidResult.raidStatus == RAID_STATUS_MAX))
            {
                EPRINT(DISK_MANAGER, "raid array not assembled or interrupted externally: [mode=%d], [raidVolNo=%d], [raidStatus=%d]",
                       hddConfig.mode, raidInfo.raidVolNo, raidResult.raidStatus);
                MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                raidVolumeInfo[raidInfo.raidVolNo].diskVolStatus = DM_DISK_VOL_FAULT;
                MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                continue;
            }

            /* Raid array build successfully. So now mount raid array volume */
            snprintf(partitionNode, NODE_NAME_SIZE, RAID_ARRAY_NAME, raidInfo.raidVolNo);
            snprintf(mountPoint, MOUNT_POINT_SIZE, RAID_ARRAY_MOUNT_POINT, raidInfo.raidVolNo);
            status = mountDevice(partitionNode, mountPoint, EXT_4);
            if (status == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to mount raid volume: [mode=%d], [raidVolNo=%d]", hddConfig.mode, raidInfo.raidVolNo);
                MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                raidVolumeInfo[raidInfo.raidVolNo].diskVolStatus = DM_DISK_VOL_FAULT;
                MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
                continue;
            }

            MUTEX_LOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);
            snprintf(raidVolumeInfo[raidInfo.raidVolNo].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
            raidVolumeInfo[raidInfo.raidVolNo].mediaStatus = MOUNTED_READY;
            MUTEX_UNLOCK(raidVolumeInfo[raidInfo.raidVolNo].rawMediaMutex);

            /* Check signature was present or not */
            if(VerifyHddSignature(mountPoint) == FAIL)
            {
                /* Ready raid partition with format, mount and write signature */
                EPRINT(DISK_MANAGER, "hard-disk signature not proper");
                status = readyRaidPartition(hddConfig.mode, raidInfo.raidVolNo, NULL);
            }

            /* Update raid volume information */
            updateRaidVolumeInfo(raidInfo.raidVolNo, status);
        }
    }

    /* Update storage volume information */
    UpdateLocalStorageVolumeInfo(FALSE, TRUE);

    /* Now storage is ready for recording */
    UpdateRecordingMedia(LOCAL_HARD_DISK);

    /* Exit from the thread */
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks raid status every 10 Second and return after getting raid status
 *          result. This is bocking call.
 * @param   raidInfo
 * @param   raidResult
 * @return  SUCCESS/FAIL
 */
static BOOL monitorRaidStatusContinues(const RAID_CREATE_INFO_t *raidInfo, RAID_RESULT_t *raidResult)
{
    do
    {
        /* Wait till building not complete. After this we need to check array build up success or still in process.
         * Wait till 100% build was not complete cat /proc/mdstat */
        sleep(10);

        /* Here we need to check after every 10 second that raid building complete or not */
        if (checkRaidStatus(raidInfo, raidResult) == FAIL)
        {
            return FAIL;
        }

        MUTEX_LOCK(raidVolumeInfo[raidInfo->raidVolNo].rawMediaMutex);
        raidVolumeInfo[raidInfo->raidVolNo].diskFormatPercentage = (atof(raidResult->raidPercentage) * 10);
        MUTEX_UNLOCK(raidVolumeInfo[raidInfo->raidVolNo].rawMediaMutex);

    }while(raidResult->raidStatus != RAID_STATUS_COMPLETE);

    DPRINT(DISK_MANAGER, "raid array building completed");
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function format, mount and write signature in raid partition.
 * @param   mode
 * @param   raidVolNo
 * @param   advncDetail
 * @return  SUCCESS/FAIL
 */
static BOOL readyRaidPartition(HDD_MODE_e mode, RAID_VOLUME_NO_e raidVolNo, CHARPTR advncDetail)
{
    BOOL                retVal = FAIL;
    RAW_MEDIA_STATUS_e  mediaStats;
    CHAR                mountPoint[MOUNT_POINT_SIZE];
    CHAR                raidNode[MOUNT_POINT_SIZE];

    /* Set LED cadence for disk format */
    SetSystemStatusLed(SYS_FORMATTING, ON);

    do
    {
        MUTEX_LOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        mediaStats = raidVolumeInfo[raidVolNo].mediaStatus;
        MUTEX_UNLOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);

        snprintf(raidNode, MOUNT_POINT_SIZE, RAID_ARRAY_NAME, raidVolNo);
        snprintf(mountPoint, MOUNT_POINT_SIZE, RAID_ARRAY_MOUNT_POINT, raidVolNo);
        if(mediaStats == MOUNTED_READY)
        {
            /* Unmount running device by taking care of ftp service */
            if (FAIL == unmountRunningDevice(mountPoint))
            {
                break;
            }
        }
        else if(access(raidNode, F_OK) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "raid node not present: [raidNode=%s]", raidNode);
            break;
        }

        /* Reset storage volume information */
        resetStorageVolumeInfo(&raidVolumeInfo[raidVolNo], DISK_FORMATTING, DM_DISK_VOL_FORMATTING);
        if (GetHddNonFuncStatus() == TRUE)
        {
            SetDiskHealthStatus(STRG_HLT_MAX);
            SetCameraVolumeHealthStatus(raidVolNo, STRG_HLT_MAX);
            WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(mode, LOCAL_HARD_DISK), NULL, EVENT_DISK_BUSY);
        }

        /* Format device: mkfs.ext4 /dev/md0: format the device with ext4 file system */
        if(formatWithExt4(raidNode) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fai to format raid partition: [raidNode=%s]", raidNode);
            break;
        }

        /* Write event details as per raid type */
        writeRaidVolumeEvent(mode, raidVolNo, advncDetail, EVENT_FORMAT);

        /* After that your device was ready to use make the mount point */
        MUTEX_LOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        snprintf(raidVolumeInfo[raidVolNo].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
        MUTEX_UNLOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        if(mountDevice(raidNode, mountPoint, EXT_4) != SUCCESS)
        {
            EPRINT(DISK_MANAGER, "raid device not mounted");
            break;
        }

        /* Now we need to write our signature */
        if(WriteHddSignature(mountPoint) == FAIL)
        {
            EPRINT(DISK_MANAGER, "hard-disk signature not written");
            break;
        }

        MUTEX_LOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        raidVolumeInfo[raidVolNo].mediaStatus = MOUNTED_READY;
        raidVolumeInfo[raidVolNo].diskVolStatus = DM_DISK_VOL_NORMAL;
        MUTEX_UNLOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        retVal = SUCCESS;
    }
    while(0);

    if(retVal == FAIL)
    {
        MUTEX_LOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        raidVolumeInfo[raidVolNo].mediaStatus = UNMOUNTED;
        raidVolumeInfo[raidVolNo].diskVolStatus = DM_DISK_VOL_FAULT;
        MUTEX_UNLOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
    }

    /* Set LED cadence for disk format */
    SetSystemStatusLed(SYS_FORMATTING, OFF);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stop raid array, creat it and check array created or not. make mdadm config
 *          file and ready raid partition.
 * @param   raidInfo
 * @return  SUCCESS/FAIL
 */
static BOOL createRaidPartition(RAID_CREATE_INFO_t *raidInfo)
{
    BOOL            status = FALSE;
    CHAR            deviceNode[MOUNT_POINT_SIZE];
    CHAR            mountPoint[MOUNT_POINT_SIZE];
    RAID_RESULT_t   raidResult;

    do
    {
        /* NOTE: this was for only checking purpose Stop the array */
        DPRINT(DISK_MANAGER, "create raid partition: [raidVolNo=%d]", raidInfo->raidVolNo);
        if (stopRaidArray(raidInfo->raidVolNo) == FAIL)
        {
            DPRINT(DISK_MANAGER, "raid device not running: [raidVolNo=%d]", raidInfo->raidVolNo);
        }

        /* Write event details as per raid type */
        writeRaidVolumeEvent(raidInfo->raidType, raidInfo->raidVolNo, NULL, EVENT_CREATING);

        MUTEX_LOCK(raidVolumeInfo[raidInfo->raidVolNo].rawMediaMutex);
        raidVolumeInfo[raidInfo->raidVolNo].diskFormatPercentage = 0;
        raidVolumeInfo[raidInfo->raidVolNo].mediaStatus = RAID_BUILDING;
        raidVolumeInfo[raidInfo->raidVolNo].diskVolStatus = DM_DISK_VOL_CREATING;
        MUTEX_UNLOCK(raidVolumeInfo[raidInfo->raidVolNo].rawMediaMutex);

        /* Clean previous raid array info if present */
        cleanRaidArray(raidInfo);

        /* Create a raid array */
        if (createRaidArray(raidInfo) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to create raid array: [raidVolNo=%d]", raidInfo->raidVolNo);
            break;
        }

        /* Check raid creation status */
        if (monitorRaidStatusContinues(raidInfo, &raidResult) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to check raid status: [raidVolNo=%d]", raidInfo->raidVolNo);
            if (raidResult.raidStatus == RAID_STATUS_MAX)
            {
                EPRINT(DISK_MANAGER, "raid status fetching failed, may be raid creation interrupted externally: [raidVolNo=%d]", raidInfo->raidVolNo);
            }
            break;
        }

        /* Store its configuration in files mdadm --detail --scan > /etc/mdadm.conf */
        if (makeMdadmConfFile(raidInfo) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to create mdadm config file: [raidVolNo=%d]", raidInfo->raidVolNo);
            break;
        }

        snprintf(deviceNode, MOUNT_POINT_SIZE, RAID_ARRAY_NAME, raidInfo->raidVolNo);
        snprintf(mountPoint, MOUNT_POINT_SIZE, RAID_ARRAY_MOUNT_POINT, raidInfo->raidVolNo);
        if (mountDevice(deviceNode, mountPoint, EXT_4) == SUCCESS)
        {
            DPRINT(DISK_MANAGER, "raid partition mounted successfully: [raidVolNo=%d]", raidInfo->raidVolNo);
            MUTEX_LOCK(raidVolumeInfo[raidInfo->raidVolNo].rawMediaMutex);
            snprintf(raidVolumeInfo[raidInfo->raidVolNo].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
            raidVolumeInfo[raidInfo->raidVolNo].mediaStatus = MOUNTED_READY;
            MUTEX_UNLOCK(raidVolumeInfo[raidInfo->raidVolNo].rawMediaMutex);

            /* Check signature is present or not */
            if (VerifyHddSignature(mountPoint) == SUCCESS)
            {
                DPRINT(DISK_MANAGER, "raid partition signature found: [raidVolNo=%d]", raidInfo->raidVolNo);
                status = TRUE;
                break;
            }
        }

        /* Ready raid partition with format, mount and write signature */
        EPRINT(DISK_MANAGER, "fail to mount raid or partition signature not found: [raidVolNo=%d], [node=%s]", raidInfo->raidVolNo, deviceNode);
        if (readyRaidPartition(raidInfo->raidType, raidInfo->raidVolNo, NULL) == FAIL)
        {
            EPRINT(DISK_MANAGER, "raid partition not ready: [raidVolNo=%d]", raidInfo->raidVolNo);
            break;
        }

        /* Successfully created formated raid array */
        status = TRUE;

    }while(0);

    /* Update raid volume info */
    return updateRaidVolumeInfo(raidInfo->raidVolNo, status);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function cleans superblocks from the all partitions.
 * @param   raidInfo
 */
static void cleanRaidArray(const RAID_CREATE_INFO_t *raidInfo)
{
    UINT8   diskCnt;
    CHAR    sysCmd[SYSTEM_COMMAND_SIZE];

    /* Clear raid super blocks from all partitions */
    for (diskCnt = 0; diskCnt < raidInfo->hardDiskCnt; diskCnt++)
    {
        /* Example: mdadm --zero-superblock /dev/sda1 */
        snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "mdadm --zero-superblock %s", raidInfo->deviceNode[diskCnt]);

        /* Execute raid superblock cleanup commnad */
        ExeSysCmd(TRUE, sysCmd);
    }

    DPRINT(DISK_MANAGER, "raid array superblock cleanup done");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function cretaes a raid array of raid number given in input parameter.
 * @param   raidInfo
 * @return  SUCCESS/FAIL
 */
static BOOL createRaidArray(const RAID_CREATE_INFO_t *raidInfo)
{
    UINT8   diskCnt;
    UINT32  strLen;
    CHAR    sysCmd[SYSTEM_COMMAND_SIZE];

    /* Example:
     * Raid0: mdadm --create /dev/md0 --level=raid0 --assume-clean --raid-devices=2 /dev/sda1 /dev/sdb1
     * Raid1: mdadm --create /dev/md0 --level=raid1 --assume-clean --raid-devices=2 /dev/sda1 /dev/sdb1
     * Raid5 (3 disks): mdadm --create /dev/md0 --level=raid5 --assume-clean --raid-devices=3 /dev/sda1 /dev/sdb1 /dev/sdc1
     * Raid5 (4 disks): mdadm --create /dev/md0 --level=raid5 --assume-clean --raid-devices=4 /dev/sda1 /dev/sdb1 /dev/sdc1 /dev/sdd1
     * Raid10: mdadm --create /dev/md0 --level=raid10 --assume-clean --raid-devices=4 /dev/sda1 /dev/sdb1 /dev/sdc1 /dev/sdd1
     */
    /* Prepare raid create command with type and node count. "yes" will give the input to mdadm prompt in raid creation */
    strLen = snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "yes | mdadm --create " RAID_ARRAY_NAME " --level=%s --assume-clean --raid-devices=%d",
             raidInfo->raidVolNo, storageModeStr[raidInfo->raidType], raidInfo->hardDiskCnt);

    /* Add device node name in raid create commmand */
    for (diskCnt = 0; diskCnt < raidInfo->hardDiskCnt; diskCnt++)
    {
        strLen += snprintf(&sysCmd[strLen], SYSTEM_COMMAND_SIZE - strLen, " %s", raidInfo->deviceNode[diskCnt]);
    }

    /* Execute raid creation commnad */
    if (ExeSysCmd(TRUE, sysCmd) == FALSE)
    {
        EPRINT(DISK_MANAGER, "fail to exe cmd: [cmd=%s]", sysCmd);
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "cmd exe success: [cmd=%s]", sysCmd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function create a mdadm configuration file for assemble raid array device.
 * @param   raidInfo
 * @return  SUCCESS/FAIL
 */
static BOOL makeMdadmConfFile(const RAID_CREATE_INFO_t *raidInfo)
{
    INT32	fileFd, dataCnt, nodeCnt;
    CHAR 	fileName[MAX_FILE_NAME_SIZE];
    CHAR	sysCmd[SYSTEM_COMMAND_SIZE];

    /* Example: mdadm --detail --scan > /etc/mdadm0.conf */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "mdadm --detail --scan > "MDADM_CONFIG_FILE, raidInfo->raidVolNo);
    if(ExeSysCmd(TRUE, sysCmd) == FALSE)
    {
        EPRINT(DISK_MANAGER, "fail to exe cmd: [cmd=%s]", sysCmd);
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "cmd exe success: [cmd=%s]", sysCmd);
    snprintf(fileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidInfo->raidVolNo);
    fileFd = open(fileName, READ_WRITE_MODE, FILE_PERMISSION);
    if(fileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
    }

    lseek(fileFd, 0, SEEK_END);
    dataCnt = snprintf(sysCmd, SYSTEM_COMMAND_SIZE, RAID_TOTAL_NODE_STR "%d\n", raidInfo->hardDiskCnt);
    if (dataCnt != write(fileFd, sysCmd, dataCnt))
    {
        EPRINT(DISK_MANAGER, "fail to write file: [path=%s], [err=%s]", fileName, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    /* Add disk node in mdadm config file as per disk present count */
    for (nodeCnt = 0; nodeCnt < raidInfo->hardDiskCnt; nodeCnt++)
    {
        dataCnt = snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "DEVICE %s\n", raidInfo->deviceNode[nodeCnt]);
        if (dataCnt != write(fileFd, sysCmd, dataCnt))
        {
            EPRINT(DISK_MANAGER, "fail to write file: [path=%s], [err=%s]", fileName, STR_ERR);
            close(fileFd);
            return FAIL;
        }
    }

    close(fileFd);
    DPRINT(DISK_MANAGER, "mdadm config file created successfully: [path=%s]", fileName);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Remove all mdadm config files
 */
static void removeAllMdadmConfFile(void)
{
    UINT8   raidVolNo;
    CHAR 	fileName[MAX_FILE_NAME_SIZE];

    for(raidVolNo = 0; raidVolNo < MAX_VOLUME; raidVolNo++)
    {
        snprintf(fileName, sizeof(fileName), MDADM_CONFIG_FILE, raidVolNo);
        if(access(fileName, F_OK) == STATUS_OK)
        {
            DPRINT(DISK_MANAGER, "mdadm config file removed: [path=%s]", fileName);
            remove(fileName);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops the raid array.
 * @param   raidVolNo
 * @return  SUCCESS/FAIL
 */
static BOOL stopRaidArray(RAID_VOLUME_NO_e raidVolNo)
{
    struct  stat64 fs;
    CHAR    sysCmd[SYSTEM_COMMAND_SIZE];

    /* Prepare raid array node name and check whether it is present or not */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, RAID_ARRAY_NAME, raidVolNo);
    if ((stat64(sysCmd, &fs) < 0) || (FALSE == S_ISBLK(fs.st_mode)))
    {
        DPRINT(DISK_MANAGER, "raid array node is not present: [node=%s]", sysCmd);
        return SUCCESS;
    }

    /* Example: mdadm --stop /dev/md0 */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "mdadm --stop "RAID_ARRAY_NAME, raidVolNo);
    if(ExeSysCmd(TRUE, sysCmd) == FALSE)
    {
        EPRINT(DISK_MANAGER, "fail to exe cmd: [cmd=%s]", sysCmd);
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "cmd exe success: [cmd=%s]", sysCmd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get actual raid array node with raid array was created
 * @param   raidVolNo
 * @param   pTotalNode
 * @return  Returns SUCCESS if raid array disks are updated else returns FAIL
 */
static BOOL getActualRaidArrayDisks(UINT8 raidVolNo, UINT8 *pTotalNode)
{
    UINT32 totalNode = 0;
    CHAR fileName[MAX_FILE_NAME_SIZE];
    FILE *pFileHandle = NULL;
    CHAR lineBuff[MAX_LINE];

    /* Open mdadm config file to get total nodes. It is our custom field */
    snprintf(fileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidVolNo);
    pFileHandle = fopen(fileName, "r");
    if (NULL == pFileHandle)
    {
        return FAIL;
    }

    /* Parse all lines of file and get total nodes */
    while(fgets(lineBuff, sizeof(lineBuff), pFileHandle))
    {
        if (strstr(lineBuff, RAID_TOTAL_NODE_STR))
        {
            sscanf(lineBuff, RAID_TOTAL_NODE_STR "%u", &totalNode);
            *pTotalNode = totalNode;
            fclose(pFileHandle);
            return SUCCESS;
        }
    }

    /* Close the file */
    fclose(pFileHandle);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief  It provides status of mdadm config file
 * @param  raidVolNo
 * @return Returns TRUE if file present otherwise returns FALSE
 */
static BOOL isMdadmConfigFilePresent(RAID_VOLUME_NO_e raidVolNo)
{
    CHAR fileName[MAX_FILE_NAME_SIZE];

    snprintf(fileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidVolNo);
    if (access(fileName, F_OK) != STATUS_OK)
    {
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write raid volume event details
 * @param   mode
 * @param   raidVolNo
 * @param   advncDetail
 * @param   eventState
 */
static void writeRaidVolumeEvent(HDD_MODE_e mode, RAID_VOLUME_NO_e raidVolNo, CHARPTR advncDetail, LOG_EVENT_STATE_e eventState)
{
    CHAR    detail[MAX_EVENT_DETAIL_SIZE];
    UINT8   raidEvtLogIdx[MAX_HDD_MODE] = {0, 1, 5, 9, 11};

    /* Prepare event details and write event log */
    snprintf(detail, MAX_EVENT_DETAIL_SIZE, "%02d", ((raidVolNo / MAX_RAID_VOLUME) + MAX_HDD_VOL + raidEvtLogIdx[mode]));
    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_VOLUME, detail, advncDetail, eventState);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function assemble the already created array.
 * @param   raidVolNo
 * @return  SUCCESS/FAIL
 * @note    If we have migrated mdadm config from legacy to new then we have to update the raid array
 *          name in each disks we are part of raid array. To do that, we have provide the "--update=name"
 *          option in raid assemble. Hence we have added that option unconditionally.
 */
static BOOL assembleRaid(RAID_VOLUME_NO_e raidVolNo)
{
    CHAR	sysCmd[SYSTEM_COMMAND_SIZE];
    CHAR	fileName[MAX_FILE_NAME_SIZE];

    /* first check assemble file was present */
    snprintf(fileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidVolNo);
    if (FALSE == isMdadmConfigFilePresent(raidVolNo))
    {
        EPRINT(DISK_MANAGER, "assemble file not present for raid: [file=%s]", fileName);
        return FAIL;
    }

    /* Example: mdadm --assemble --verbose --update=name --config=/etc/mdadm0.conf /dev/md0 */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "mdadm --assemble --verbose --update=name --config=%s "RAID_ARRAY_NAME, fileName, raidVolNo);
    if(ExeSysCmd(TRUE, sysCmd) == FALSE)
    {
        /* Array assemble command fail, try with force option */
        EPRINT(DISK_MANAGER, "fail to exe raid assemble cmd: [cmd=%s]", sysCmd);

        /* Example: mdadm --assemble --verbose --force --config=/etc/mdadm0.conf /dev/md0 */
        snprintf(sysCmd, SYSTEM_COMMAND_SIZE, "mdadm --assemble --verbose --force --update=name --config=%s "RAID_ARRAY_NAME, fileName, raidVolNo);
        if(ExeSysCmd(TRUE, sysCmd) == FALSE)
        {
            /* Array assemble command with force option */
            EPRINT(DISK_MANAGER, "fail to exe raid force assemble cmd: [cmd=%s]", sysCmd);
            return FAIL;
        }
    }

    DPRINT(DISK_MANAGER, "cmd exe success: [cmd=%s]", sysCmd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function update the mdadm configuration file with new device nodes (if device nodes
 *          are changed) for assemble raid array device.
 * @param   raidInfo
 */
static void updateMdadmConfFile(const RAID_CREATE_INFO_t *raidInfo)
{
    UINT32  fileLen = 0;
    FILE    *pFile;
    UINT8   nodeCntMax = 0, nodeIdx, tempNode, newNodeCnt = 0;
    CHAR    lineBuff[MAX_LINE];
    CHAR    fileData[PATH_MAX];
    CHAR    deviceNode[TOTAL_RAID_DEVICE][NODE_NAME_SIZE];
    CHAR    fileName[MAX_FILE_NAME_SIZE];
    CHAR    tmpFileName[MAX_FILE_NAME_SIZE];
    CHAR    *searchPtr;

    /* Prepare mdadm config file path */
    snprintf(fileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidInfo->raidVolNo);
    if (access(fileName, F_OK) != STATUS_OK)
    {
        DPRINT(DISK_MANAGER, "mdadm config not present: [file=%s]", fileName);
        return;
    }

    /* Open mdadm config file for reading */
    pFile = fopen(fileName, "r");
    if (pFile == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to open mdadm config file: [file=%s], [err=%s]", fileName, STR_ERR);
        return;
    }

    /* Read file line by line */
    memset(fileData, 0, sizeof(fileData));
    memset(deviceNode, 0, sizeof(deviceNode));
    while(fgets(lineBuff, sizeof(lineBuff), pFile))
    {
        if (NULL != (searchPtr = strstr(lineBuff, "DEVICE")))
        {
            /* Get actual node name by skipping "DEVICE" string and space character */
            searchPtr += strlen("DEVICE") + 1;
            snprintf(deviceNode[nodeCntMax], sizeof(deviceNode[nodeCntMax]), "%s", searchPtr);
            nodeCntMax++;
        }
        else
        {
            /* Store other lines as it is */
            fileLen += snprintf(&fileData[fileLen], sizeof(fileData) - fileLen, "%s", lineBuff);
        }
    }

    /* Close the file */
    fclose(pFile);

    /* Check all nodes which were part of the raid */
    for (nodeIdx = 0; nodeIdx < nodeCntMax; nodeIdx++)
    {
        /* Check with all detected nodes */
        for (tempNode = 0; tempNode < raidInfo->totalRaidDevices; tempNode++)
        {
            /* Was current detected node part of the raid? */
            if (strncmp(deviceNode[nodeIdx], raidInfo->deviceNode[tempNode], strlen(raidInfo->deviceNode[tempNode])) == 0)
            {
                /* Yes. It was */
                newNodeCnt++;
                break;
            }
        }
    }

    /* No change in nodes name */
    if (newNodeCnt == nodeCntMax)
    {
        DPRINT(DISK_MANAGER, "no change in nodes name in mdadm config: [file=%s], [nodes=%d]", fileName, nodeCntMax);
        return;
    }

    /* Get min value between detected nodes or raid create nodes */
    for (nodeIdx = 0; nodeIdx < nodeCntMax; nodeIdx++)
    {
        /* Add newly detected nodes in config file */
        fileLen += snprintf(&fileData[fileLen], sizeof(fileData) - fileLen, "DEVICE %s\n", raidInfo->deviceNode[nodeIdx]);
    }

    /* Prepare new temp mdadm config file to avoid data loss */
    snprintf(tmpFileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE".tmp", raidInfo->raidVolNo);
    pFile = fopen(tmpFileName, "w+");
    if (pFile == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to open temp mdadm config file: [file=%s], [err=%s]", tmpFileName, STR_ERR);
        return;
    }

    /* Dump new mdadm config data in temp file */
    if ((INT32)fileLen != fprintf(pFile, "%s", fileData))
    {
        EPRINT(DISK_MANAGER, "fail to dump mdadm config in temp file: [file=%s], [fileLen=%d], [err=%s]", tmpFileName, fileLen, STR_ERR);
        fclose(pFile);
        unlink(tmpFileName);
        return;
    }

    /* Close the old file */
    fclose(pFile);

    /* Rename file from temp name to actual name */
    if (rename(tmpFileName, fileName))
    {
        EPRINT(DISK_MANAGER, "fail to dump mdadm config in temp file: [file=%s], [fileLen=%d], [err=%s]", tmpFileName, fileLen, STR_ERR);
        return;
    }

    /* Raid mdadm config file updated with new nodes */
    DPRINT(DISK_MANAGER, "mdadm config updated with new nodes: [file=%s], [nodes=%d]", fileName, nodeCntMax);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function assemble the already created array.
 * @param   raidVolNo
 * @param   deviceNode
 * @return  SUCCESS/FAIL
 */
static BOOL addRaidDevice(RAID_VOLUME_NO_e raidVolNo, CHARPTR deviceNode)
{
    CHAR sysCmd[SYSTEM_COMMAND_SIZE];

    /* Example: mdadm --manage /dev/md0 --add /dev/sda1 */
    snprintf(sysCmd, SYSTEM_COMMAND_SIZE,"mdadm --manage " RAID_ARRAY_NAME " --add %s", raidVolNo, deviceNode);
    if(ExeSysCmd(TRUE, sysCmd) == FALSE)
    {
        EPRINT(DISK_MANAGER, "fail to exe cmd: [cmd=%s]", sysCmd);
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "cmd exe success: [cmd=%s]", sysCmd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function stops Raid array building process.
 * @param   raidVolNo
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e StopRaidVolume(RAID_VOLUME_NO_e raidVolNo)
{
    HDD_CONFIG_t hddConfig;

    /* Read Hdd Config */
    ReadHddConfig(&hddConfig);

    /* Check raid was building or resyncing then stop it */
    if ((GetHddVolBuildStatus() == TRUE) && (raidVolNo < GetTotalDiskNumber(hddConfig.mode)))
    {
        if(stopRaidArray(raidVolNo) == SUCCESS)
        {
            return CMD_SUCCESS;
        }
    }

    return CMD_PROCESS_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks whether raid was building or recovery was started. And it will
 *          reflect the status of raid as output parameter.
 * @param   raidInfo
 * @param   raidResult
 * @return  SUCCESS/FAIL
 * @example
 * "/proc/mdstat" file example
 * 1st
 * Personalities : [linear] [raid0] [raid1]
 * unused devices: <none>
 *
 * 2nd
 * Personalities : [linear] [raid0] [raid1]
 * md0 : active raid1 sdc1[1] sdd1[0]
 *       976256 blocks [2/2] [UU]
 * [>....................]  resync = 1.6% (16064/976256) finish=1.9min speed=8032K/sec
 * unused devices: <none>
 *
 * 3rd
 * Personalities : [linear] [raid0] [raid1]
 * md0 : active raid1 sdc1[1] sdd1[0]
 *       976256 blocks [2/2] [UU]
 * unused devices: <none>
 *
 * 4th
 * Personalities : [linear] [raid0] [raid1]
 * md0 : active raid1 sdd1[2] sdc1[1]
 *       976256 blocks [2/1] [_U]
 * [==========>..........]  recovery = 54.8% (535616/976256) finish=0.8min speed=8428K/sec
 * unused devices: <none>
 */
static BOOL checkRaidStatus(const RAID_CREATE_INFO_t *raidInfo, RAID_RESULT_t *raidResult)
{
    CHARPTR		pSearchStatus, pSearchStr;
    CHAR		dataBuff[MAX_LINE];
    UINT8		lineReadCnt = 0;
    INT32       dataLength = 0;
    UINT8		hardDiskCnt = 0;
    FILE 		*pFile;
    CHAR		scanBuff[5][100];
    UINT8		partitionCnt = 0;
    CHAR		raidNode[5];
    UINT8       nodeCnt;
    HDD_MODE_e  modeCnt;

    /* Prepare raid array node to be scan */
    memset(raidResult, 0, sizeof(RAID_RESULT_t));
    raidResult->raidStatus = RAID_STATUS_MAX;

    snprintf(raidNode, sizeof(raidNode), "md%d", raidInfo->raidVolNo);
    pFile = fopen(RAID_STATUS_FILE, "r");
    if(pFile == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to open file: [path=%s]", RAID_STATUS_FILE);
        return FAIL;
    }

    /* Discard first line of status. This will handle in while loop */
    while(fgets(dataBuff, sizeof(dataBuff), pFile))
    {
        /* Update line read counter */
        lineReadCnt++;

        if (strstr(dataBuff, RAID_STATUS_NO_DEV))
        {
            /* If unused devices found at line 2 means no raid array is present */
            if (lineReadCnt == 2)
            {
                raidResult->raidStatus = RAID_STATUS_NO_DEVICE;
            }

            /* It is last line of mdstate output */
            break;
        }

        if (strstr(dataBuff, raidNode))
        {
            /* raidInfo->raidVolNo which indicates the current raid volume which is in creating mode */
            memset(scanBuff, 0, sizeof(scanBuff));
            sscanf(dataBuff, "%*s %*s %*s %s %s %s %s %s", scanBuff[0], scanBuff[1], scanBuff[2], scanBuff[3], scanBuff[4]);

            for (modeCnt = RAID_0; modeCnt < MAX_HDD_MODE; modeCnt++)
            {
                if (strcmp(scanBuff[0], storageModeStr[modeCnt]) == STATUS_OK)
                {
                    break;
                }
            }

            /* Is valid raid mode found? */
            if (modeCnt >= MAX_HDD_MODE)
            {
                break;
            }

            raidResult->totalNodes = 0;
            for (hardDiskCnt = 0; hardDiskCnt < getMaxHardDiskForCurrentVariant(); hardDiskCnt++)
            {
                for (partitionCnt = 0; partitionCnt < MAX_PARTITION_SUPPORT; partitionCnt++)
                {
                    /* Is partition present? */
                    if (raidDiskPartInfo[hardDiskCnt].partitionInfo[partitionCnt].partitionSymbLink[0] == '\0')
                    {
                        continue;
                    }

                    /* Node 0 and 1 will be present for all raid mode but node 3 will be only raid 5 and raid 10
                     * & node 4 will be only for raid5 with 4 disks and raid10 */
                    for (nodeCnt = 0; nodeCnt < TOTAL_RAID_DEVICE; nodeCnt++)
                    {
                        /* No need to check other nodes */
                        if (raidInfo->totalRaidDevices < (nodeCnt+1))
                        {
                            break;
                        }

                        /* Node is not present */
                        if (scanBuff[nodeCnt+1][0] == '\0')
                        {
                            continue;
                        }

                        /* Search for the required node */
                        if (NULL == strstr(scanBuff[nodeCnt+1], raidDiskPartInfo[hardDiskCnt].partitionInfo[partitionCnt].partitionSymbLink))
                        {
                            continue;
                        }

                        /* Store the found node */
                        snprintf(raidResult->deviceNode[raidResult->totalNodes], NODE_NAME_SIZE, "%s",
                                raidDiskPartInfo[hardDiskCnt].partitionInfo[partitionCnt].partitionNodeName);
                        raidResult->totalNodes++;
                        break;
                    }
                }
            }

            /* Update raid status based on detected nodes */
            raidResult->raidStatus = (raidResult->totalNodes == raidInfo->totalRaidDevices) ? RAID_STATUS_COMPLETE : RAID_STATUS_DEGRADED;
            continue;
        }

        /* Is raid array resyncing or recovering? */
        if ((pSearchStatus = strstr(dataBuff, RAID_STATUS_RESYNC)) != NULL)
        {
            raidResult->raidStatus = RAID_STATUS_RESYNCHING;
            dataLength = strlen(RAID_STATUS_RESYNC);
        }
        else if ((pSearchStatus = strstr(dataBuff, RAID_STATUS_RECOVER)) != NULL)
        {
            raidResult->raidStatus = RAID_STATUS_RECOVERY;
            dataLength = strlen(RAID_STATUS_RECOVER);
        }
        else
        {
            continue;
        }

        /* Is raid array resyncing or recovering? */
        if (NULL != pSearchStatus)
        {
            pSearchStatus += dataLength;
            while((*pSearchStatus == '=') || (*pSearchStatus == ' '))
            {
                pSearchStatus++;
            }

            /* Get resync percentage */
            pSearchStr = strchr(pSearchStatus, '%');
            if (pSearchStr == NULL)
            {
                break;
            }

            /* Get the percentage in sting format */
            dataLength = (pSearchStr - pSearchStatus);
            if ((dataLength > 0) && (dataLength < (INT32)(sizeof(raidResult->raidPercentage) - 1)))
            {
                strncpy(raidResult->raidPercentage, pSearchStatus, dataLength);
                raidResult->raidPercentage[dataLength] = '\0';
            }
            break;
        }
    }

    /* close the file */
    fclose(pFile);

    if ((raidResult->raidStatus == RAID_STATUS_NO_DEVICE) || (raidResult->raidStatus >= RAID_STATUS_MAX))
    {
        EPRINT(DISK_MANAGER, "required volume is not present in /proc/mdstat: [raidStatus=%d]", raidResult->raidStatus);
        return FAIL;
    }

    dataLength = 0;
    dataBuff[0] = '\0';
    for (nodeCnt = 0; nodeCnt < raidResult->totalNodes; nodeCnt++)
    {
        dataLength += snprintf(&dataBuff[dataLength], sizeof(dataBuff) - dataLength, ", [node%d=%s]", nodeCnt, raidResult->deviceNode[nodeCnt]);
    }

    DPRINT(DISK_MANAGER, "raid build info: [status=%s], [percent=%s]%s", raidStatusStr[raidResult->raidStatus], raidResult->raidPercentage, dataBuff);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used whenever user was request for physical disk info. It will gives Model
 *          & Serial No,Disk Capacity and status as out put parameters.
 * @param   hddCnt
 * @param   diskInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetPhysicalDiskInfo(UINT8 hddCnt, PHYSICAL_DISK_INFO_t *diskInfo)
{
    float hardiskSize;

    if ((diskInfo == NULL) || (hddCnt >= getMaxHardDiskForCurrentVariant()))
    {
        return FAIL;
    }

    MUTEX_LOCK(physicalMediaInfo[hddCnt].phyMediaMutex);
    snprintf(diskInfo->serialNo, HD_SERIAL_NO_SIZE, "%s", physicalMediaInfo[hddCnt].serialNo);
    hardiskSize = ((float)physicalMediaInfo[hddCnt].physicalSize / MEGA_BYTE);
    snprintf(diskInfo->capacity, MAX_HDD_CAPACITY,"%.02f", hardiskSize);
    diskInfo->status = physicalMediaInfo[hddCnt].diskPhyStatus;
    MUTEX_UNLOCK(physicalMediaInfo[hddCnt].phyMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of hard disk.
 * @return  STORAGE_HEALTH_STATUS_e
 */
STORAGE_HEALTH_STATUS_e GetDiskHealthStatus(void)
{
    MUTEX_LOCK(systemStorageHealthStatus.mutexLock);
    STORAGE_HEALTH_STATUS_e healthStatus = systemStorageHealthStatus.healthStatus;
    MUTEX_UNLOCK(systemStorageHealthStatus.mutexLock);
    return healthStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set system storage health status
 * @param   healthStatus
 */
void SetDiskHealthStatus(STORAGE_HEALTH_STATUS_e healthStatus)
{
    MUTEX_LOCK(systemStorageHealthStatus.mutexLock);
    systemStorageHealthStatus.healthStatus = healthStatus;
    MUTEX_UNLOCK(systemStorageHealthStatus.mutexLock);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives health status of USB Media
 * @param   usbIndex
 * @param   dummy
 * @return  Status
 */
BOOL GetUsbHealthStatus(UINT8 usbIndex, UINT8 dummy)
{
    usbIndex += MANUAL_BACKUP_DISK;
    MUTEX_LOCK(storageMedia[usbIndex].rawMediaMutex);
    BOOL status = (storageMedia[usbIndex].mediaStatus == NOT_PRESENT) ? REMOVED : ADDED;
    MUTEX_UNLOCK(storageMedia[usbIndex].rawMediaMutex);
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used whenever user was request to get status of logical volume of disk.
 *          It will gives volume name, total size, free size and status as out put parameters.
 * @param   volumeNum
 * @param   diskVolumeInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetDiskVolumeInfo(UINT8 volumeNum, DISK_VOLUME_INFO_t *diskVolumeInfo)
{
    float               hardiskSize;
    HDD_CONFIG_t        hddConfig;
    RAW_MEDIA_INFO_t    *pMediaInfo;

    if (diskVolumeInfo == NULL)
    {
        return FAIL;
    }

    /* Read Hdd Config */
    ReadHddConfig(&hddConfig);

    /* If hdd mode changed then don't provide the status and wait to update it */
    if (lastHddMode != hddConfig.mode)
    {
        WPRINT(UTILS, "waiting for hdd mode change: [volume=%d], [lastHddMode=%s], [newHddMode=%s]",
               volumeNum, storageModeStr[lastHddMode], storageModeStr[hddConfig.mode]);
        return UNKNOWN;
    }

    /* Is valid volume number? */
    if (volumeNum >= GetTotalDiskNumber(hddConfig.mode))
    {
        return UNKNOWN;
    }

    if (hddConfig.mode == SINGLE_DISK_VOLUME)
    {
        snprintf(diskVolumeInfo->volumeName, MAX_VOLUME_NAME_SIZE, "%s: Drive %d", diskVolumePrefixStr[hddConfig.mode], (volumeNum + 1));
        pMediaInfo = &storageMedia[volumeNum];
    }
    else
    {
        /* Is minimum required harddisk present for raid mode? */
        if (raidVolumeInfo[volumeNum].totalHardDisk < raidArrayDiskCnt[hddConfig.mode][RAID_DISK_CNT_MIN])
        {
            return UNKNOWN;
        }

        /* Prepare raid array volume name */
        prepareRaidVolumeName(hddConfig.mode, volumeNum, diskVolumeInfo->volumeName);
        pMediaInfo = &raidVolumeInfo[volumeNum];
    }

    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    if(pMediaInfo->mediaStatus == NOT_PRESENT)
    {
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
        return UNKNOWN;
    }

    hardiskSize = ((float)pMediaInfo->totalVolSize / KILO_BYTE);
    snprintf(diskVolumeInfo->totalSize, sizeof(diskVolumeInfo->totalSize), "%.02f", hardiskSize);
    hardiskSize = ((float)pMediaInfo->freeSize / KILO_BYTE);
    snprintf(diskVolumeInfo->freeSize, sizeof(diskVolumeInfo->freeSize), "%.02f", hardiskSize);
    diskVolumeInfo->status = pMediaInfo->diskVolStatus;
    snprintf(diskVolumeInfo->percentageFormat, sizeof(diskVolumeInfo->percentageFormat), "%.01f", (float)pMediaInfo->diskFormatPercentage/10);
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @param   mode
 * @param   recordingDisk
 * @param   status
 */
void SetDiskVolumeStatus(HDD_MODE_e mode, UINT8 recordingDisk, DISK_VOL_STATUS_e status)
{
    if (recordingDisk >= GetTotalDiskNumber(mode))
    {
        return;
    }

    RAW_MEDIA_INFO_t *pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, recordingDisk);
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    pMediaInfo->diskVolStatus = status;
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used whenever user was request to get status of connected USB disk on USB
 *          (Backup) port. It will gives type of disk, total size, free size and status of disk.
 * @param   backupType
 * @param   diskInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetBackupDiskInfo(DM_BACKUP_TYPE_e backupType, USB_DISK_INFO_t	*diskInfo)
{
    float               hardiskSize;
    UINT8               diskCnt;
    RAW_MEDIA_STATUS_e  mediaStatus;

    if(backupType >= DM_MAX_BACKUP)
    {
        return FAIL;
    }

    diskCnt = (backupType + MANUAL_BACKUP_DISK);
    MUTEX_LOCK(storageMedia[diskCnt].rawMediaMutex);
    snprintf(diskInfo->diskType, MAX_USB_DISK_TYPE, "USB-Disk");
    hardiskSize = ((float)storageMedia[diskCnt].freeSize / KILO_BYTE);
    snprintf(diskInfo->freeSize, MAX_HDD_CAPACITY,"%.02f", hardiskSize);
    hardiskSize = ((float)physicalMediaInfo[diskCnt].physicalSize / MEGA_BYTE);
    snprintf(diskInfo->totalSize, MAX_HDD_CAPACITY,"%.02f", hardiskSize);
    snprintf(diskInfo->percentageFormat, MAX_PERCENTAGE_CHAR,"%.01f", (float)storageMedia[diskCnt].diskFormatPercentage / 10);
    diskInfo->backupStatus = GetBackupInfo(backupType);
    mediaStatus = storageMedia[diskCnt].mediaStatus;
    MUTEX_UNLOCK(storageMedia[diskCnt].rawMediaMutex);

    switch(mediaStatus)
    {
        default:
        case NOT_PRESENT:
            diskInfo->status = BACKUP_NO_DISK;
            snprintf(diskInfo->diskType, MAX_USB_DISK_TYPE, "-");
            break;

        case MOUNTED_READY:
            diskInfo->status = BACKUP_DISK_DETECTED;
            break;

        case CREATING_PARTITION:
        case DISK_FORMATTING:
            diskInfo->status = BACKUP_DISK_FORMATTING;
            break;

        case UNMOUNTED:
            diskInfo->status = BACKUP_DISK_FAULT;
            break;
    }

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(physicalMediaInfo[diskCnt].phyMediaMutex);
    if(physicalMediaInfo[diskCnt].diskState == ADDED)
    {
        /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
        diskInfo->status = BACKUP_DISK_DETECTED;
    }
    else if(physicalMediaInfo[diskCnt].diskState == REMOVED)
    {
        diskInfo->status = BACKUP_DISK_UNPLUGGING;
    }
    MUTEX_UNLOCK(physicalMediaInfo[diskCnt].phyMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function and it will update the back up device as per user action. It will
 *          Check whether this disk was suitable for nvr system. If it was not suitable then format
 *          whole the disk and make suitable for use. In case of device was removed then it was removed
 *          the mount point.
 * @param   threadArg
 * @return
 */
static VOIDPTR updateBackupDevice(VOIDPTR threadArg)
{
    BOOL					status = SUCCESS;
    BOOL					diskState;
    UINT8					usbDiskCnt;
    USB_THREAD_INFO_t		*usbDiskInfo = (USB_THREAD_INFO_t *)threadArg;
    UINT64					diskSize;
    CHAR					mediaNodeName[NODE_NAME_SIZE];
    CHAR					partitionNode[NODE_NAME_SIZE];
    CHAR					mountPoint[MOUNT_POINT_SIZE];
    CHAR					eventDetail[MAX_EVENT_DETAIL_SIZE];
    LOG_EVENT_STATE_e		eventState = EVENT_UNKNOWN;
    DISK_SIZE_t				diskSizeInfo;

    MUTEX_LOCK(usbDiskInfo->usbMutex);
    usbDiskCnt = usbDiskInfo->mediaNo;
    MUTEX_UNLOCK(usbDiskInfo->usbMutex);

    THREAD_START_INDEX2("UP_USB_DEV", usbDiskCnt);

    DPRINT(DISK_MANAGER, "device thread started: [media=%s]", mediaNameStr[usbDiskCnt]);
    sleep(2);

    MUTEX_LOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);
    diskState = physicalMediaInfo[usbDiskCnt].diskState;
    MUTEX_UNLOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);

    if(diskState == ADDED)
    {
        do
        {
            DPRINT(DISK_MANAGER, "get disk information: [media=%s]", mediaNameStr[usbDiskCnt]);
            MUTEX_LOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);
            snprintf(mediaNodeName, NODE_NAME_SIZE, "%s", physicalMediaInfo[usbDiskCnt].mediaNodeName);
            snprintf(partitionNode, NODE_NAME_SIZE, PARTITION_NODE, mediaNodeName);
            snprintf(physicalMediaInfo[usbDiskCnt].partitionNode, NODE_NAME_SIZE, "%s", partitionNode);
            MUTEX_UNLOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);
            if(getSizeOfDisk(mediaNodeName, &diskSize) == FAIL)
            {
                status = FAIL;
                EPRINT(DISK_MANAGER, "size not found: [media=%s]", mediaNameStr[usbDiskCnt]);
                break;
            }

            MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
            physicalMediaInfo[usbDiskCnt].physicalSize = (diskSize / KILO_BYTE);
            MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);
            snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", usbMointPoint[usbDiskCnt - MANUAL_BACKUP_DISK]);
            if(mountDevice(partitionNode, mountPoint, FAT) != SUCCESS)
            {
                status = FAIL;
                EPRINT(DISK_MANAGER, "fail to mount device: [media=%s]", mediaNameStr[usbDiskCnt]);
                break;
            }

            if(GetSizeOfMountFs(mountPoint, &diskSizeInfo) == FAIL)
            {
                status = FAIL;
                EPRINT(DISK_MANAGER, "size not found: [media=%s]", mediaNameStr[usbDiskCnt]);
                break;
            }

            MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
            storageMedia[usbDiskCnt].mediaStatus = MOUNTED_READY;
            snprintf(storageMedia[usbDiskCnt].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
            storageMedia[usbDiskCnt].freeSize = diskSizeInfo.freeSize;
            storageMedia[usbDiskCnt].usedSize = diskSizeInfo.usedSize;
            storageMedia[usbDiskCnt].totalVolSize = diskSizeInfo.totalSize;
            MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);
            if(usbDiskCnt == MANUAL_BACKUP_DISK)
            {
                /* if mediaType is Manual Backup, enable syslogCapture in USB as per disk status. */
                SetBackupLedState(BACKUP_NO_ACITIVITY);
                UpdateSyslogApp(diskState, storageMedia[usbDiskCnt].freeSize);
            }
            DPRINT(DISK_MANAGER, "device mounted ready: [media=%s]", mediaNameStr[usbDiskCnt]);
        }
        while(0);

        if(status == FAIL)
        {
            MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
            storageMedia[usbDiskCnt].mediaStatus = UNMOUNTED;
            MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);
        }
        eventState = EVENT_CONNECT;
    }
    else if(diskState == REMOVED)
    {
        MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
        snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", storageMedia[usbDiskCnt].mountPoint);
        MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);

        DPRINT(DISK_MANAGER, "device removed: [media=%s]", mediaNameStr[usbDiskCnt]);
        unmountDevice(mountPoint);

        /* Reset storage volume information */
        resetStorageVolumeInfo(&storageMedia[usbDiskCnt], NOT_PRESENT, DM_DISK_VOL_MAX);

        MUTEX_LOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);
        physicalMediaInfo[usbDiskCnt].physicalSize = DEFAULT_DISK_SIZE;
        MUTEX_UNLOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);

        if(usbDiskCnt == MANUAL_BACKUP_DISK)
        {
            /* if mediaType was Manual Backup, disable syslogCapture in USB */
            SetBackupLedState(BACKUP_NOT_GOING_ON);
            UpdateSyslogApp(diskState, storageMedia[usbDiskCnt].freeSize);
        }
        eventState = EVENT_DISCONNECT;
    }

    MUTEX_LOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);
    physicalMediaInfo[usbDiskCnt].diskState = UNKNOWN;
    MUTEX_UNLOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);

    UpdateBackupStatus(usbDiskCnt - MANUAL_BACKUP_DISK);
    snprintf(eventDetail, MAX_EVENT_DETAIL_SIZE,"%02d", (usbDiskCnt - MANUAL_BACKUP_DISK + 1));
    WriteEvent(LOG_OTHER_EVENT, LOG_USB_STATUS, eventDetail, NULL, eventState);
    DPRINT(DISK_MANAGER, "device thread exit: [media=%s]", mediaNameStr[usbDiskCnt]);

    MUTEX_LOCK(usbDiskInfo->usbMutex);
    usbDiskInfo->threadActive = FALSE;
    MUTEX_UNLOCK(usbDiskInfo->usbMutex);
    pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This will make harddisk ready for use.
 * @param   usbDiskCnt
 * @param   formatType
 * @return
 */
static BOOL makeUsbDiskReady(UINT8 usbDiskCnt, RAW_MEDIA_FORMAT_TYPE_e formatType)
{
    BOOL 					status = FAIL;
    PHYSICAL_DISK_STATUS_t	usbInfo;

    MUTEX_LOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);
    memcpy(&usbInfo, &physicalMediaInfo[usbDiskCnt], sizeof(PHYSICAL_DISK_STATUS_t));
    MUTEX_UNLOCK(physicalMediaInfo[usbDiskCnt].phyMediaMutex);

    /* Make created partition name */
    MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
    storageMedia[usbDiskCnt].diskFormatPercentage = 0;
    storageMedia[usbDiskCnt].mediaStatus = DISK_FORMATTING;
    storageMedia[usbDiskCnt].diskVolStatus = DM_DISK_VOL_FORMATTING;
    MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);

    DPRINT(DISK_MANAGER, "usb device formating: [media=%s]", mediaNameStr[usbDiskCnt]);
    if(usbDiskCnt == MANUAL_BACKUP_DISK)
    {
        SetBackupLedState(FORMATTING_BACKUP_DEVICE);
    }

    status = formatWithFat(usbInfo.partitionNode);
    if(status == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to format usb device: [media=%s]", mediaNameStr[usbDiskCnt]);
        KillProcess(formatTypeName[formatType], KILL_SIG);

        MUTEX_LOCK(storageMedia[usbDiskCnt].rawMediaMutex);
        storageMedia[usbDiskCnt].diskFormatPercentage = 0;
        storageMedia[usbDiskCnt].mediaStatus = UNMOUNTED;
        storageMedia[usbDiskCnt].diskVolStatus = DM_DISK_VOL_FAULT;
        MUTEX_UNLOCK(storageMedia[usbDiskCnt].rawMediaMutex);
    }
    else
    {
        DPRINT(DISK_MANAGER, "usb format done: [media=%s]", mediaNameStr[usbDiskCnt]);
    }

    if(usbDiskCnt == MANUAL_BACKUP_DISK)
    {
        SetBackupLedState(BACKUP_NOT_GOING_ON);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Switch recording of all camera of same volume
 * @param   forceAllStop
 * @param   fullVolumeId
 * @return
 */
void StopAllCameraRecordingOnVolumeFull(BOOL forceAllStop, UINT8 fullVolumeId)
{
    UINT8               cameraIndex;
    CAMERA_BIT_MASK_t   cameraMask;

    /* If it is not force stop and volume is invalid then nothing to do */
    memset(&cameraMask, 0, sizeof(cameraMask));
    if ((FALSE == forceAllStop) && (fullVolumeId >= MAX_VOLUME))
    {
        /* It is invalid volume */
        return;
    }

    /* Check for all cameras */
    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        /* If force stop then don't check volume and stop all camera recording */
        if (TRUE == forceAllStop)
        {
            /* Set camera bit */
            SET_CAMERA_MASK_BIT(cameraMask, cameraIndex);
        }
        else
        {
            MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);

            /* Is current camera having same volume? */
            if (fullVolumeId == storageDiskInfo[cameraIndex].currentRecordVolumeId)
            {
                /* Yes. This camera have same volume */
                SET_CAMERA_MASK_BIT(cameraMask, cameraIndex);

                /* Set camere health status as volume full */
                storageDiskInfo[cameraIndex].healthStatus = STRG_HLT_DISK_FULL;
            }

            MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        }
    }

    /* Is any bit set in mask then proceed for the recording stop */
    if (FALSE == IS_ALL_CAMERA_MASK_BIT_CLR(cameraMask))
    {
        /* Stop camera recording on storage full which recording is running on given volume */
        StopCameraRecordOnStorageFull(cameraMask);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Switch recording of all camera of same volume
 * @param   switchVolumeId
 * @param   forceSwitch
 * @return
 */
BOOL SwitchAllCameraRecordingVolume(UINT8 switchVolumeId, BOOL forceSwitch)
{
    BOOL                status = SUCCESS;
    UINT8               volumeId;
    UINT8               cameraIndex;
    CAMERA_BIT_MASK_t   cameraMask;

    /* Check for all cameras */
    memset(&cameraMask, 0, sizeof(cameraMask));
    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        /* Get current volume of current camera */
        MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        volumeId = storageDiskInfo[cameraIndex].currentRecordVolumeId;
        MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);

        /* Is current camera having same volume? */
        if (switchVolumeId != volumeId)
        {
            /* No. This camera have not same volume */
            continue;
        }

        /* Force switch the recording volume on fault */
        status = SwitchRecordingMediaVolume(cameraIndex, forceSwitch);
        SET_CAMERA_MASK_BIT(cameraMask, cameraIndex);
    }

    /* Is any bit set in mask then proceed for the recording switch */
    if (FALSE == IS_ALL_CAMERA_MASK_BIT_CLR(cameraMask))
    {
        /* Restart record session in new volume */
        SwitchRecordSession(cameraMask);
    }

    /* Switch all same volume camera in new volume */
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is switch the recording media volume on media volume full
 * @param   cameraIndex
 * @param   forceSwitch
 * @return  SUCCESS/FAIL
 */
BOOL SwitchRecordingMediaVolume(UINT8 cameraIndex, BOOL forceSwitch)
{
    CHAR					mntPoint[MOUNT_POINT_SIZE];
    UINT8					volumeId, totalVolCnt;
    BOOL                    isVolumeUsable = FALSE;
    UINT8                   lastUsableVolumeId;
    UINT32                  volumeAllocMask;
    STORAGE_MEDIA_TYPE_e	recordVolumeId;
    DISK_SIZE_t				diskSize;
    HDD_CONFIG_t			hddConfig;
    RAW_MEDIA_INFO_t        *pMediaInfo;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d]", cameraIndex);
        return FAIL;
    }

    ReadHddConfig(&hddConfig);
    MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    recordVolumeId = storageDiskInfo[cameraIndex].currentRecordVolumeId;
    MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);

    totalVolCnt = GetTotalDiskNumber(hddConfig.mode);
    pMediaInfo = GET_MEDIA_VOLUME_INFO(hddConfig.mode, recordVolumeId);

    /* NOTE: If run time one disk remove and remove disk was recording disk then condition is required */
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    if((pMediaInfo->mediaStatus != NOT_PRESENT) && (pMediaInfo->mediaStatus != UNMOUNTED))
    {
        pMediaInfo->diskVolStatus = DM_DISK_VOL_FULL;
    }
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

    /* Get camera's volume allocation mask for volume switching */
    volumeAllocMask = GetCameraStorageVolumeAllocationMask(cameraIndex, hddConfig.mode);
    if (volumeAllocMask == 0)
    {
        EPRINT(DISK_MANAGER, "no valid volume allocation to camera for recording: [camera=%d], [mode=%s], [volume=%d]",
               cameraIndex, storageModeStr[hddConfig.mode], recordVolumeId);
        return SUCCESS;
    }

    /* Check all volumes to find suitable allocated volume */
    for(volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        /* Check next volume for recording */
        recordVolumeId++;
        if (recordVolumeId >= totalVolCnt)
        {
            /* Rollover recording volume to 0 */
            recordVolumeId = 0;
        }

        /* Is current volume part of camera's storage group */
        if (FALSE == GET_BIT(volumeAllocMask, recordVolumeId))
        {
            /* No. Current volume is not part of current group */
            continue;
        }

        pMediaInfo = GET_MEDIA_VOLUME_INFO(hddConfig.mode, recordVolumeId);
        DPRINT(DISK_MANAGER, "checking new recording volume: [camera=%d], [mode=%s], [volume=%d], [volumeMask=0x%X]",
               cameraIndex, storageModeStr[hddConfig.mode], recordVolumeId, volumeAllocMask);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus != MOUNTED_READY)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            continue;
        }
        snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", pMediaInfo->mountPoint);
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        DPRINT(DISK_MANAGER, "get the size of volume: [camera=%d], [mntPoint=%s]", cameraIndex, mntPoint);
        if(GetSizeOfMountFs(mntPoint, &diskSize) == FAIL)
        {
            continue;
        }

        // free space was less than or equal to 10 GB
        DPRINT(DISK_MANAGER, "current volume info: [camera=%d], [volume=%d], [freeSize=%d]", cameraIndex, recordVolumeId, diskSize.freeSize);
        if(diskSize.freeSize >= MINIMUM_HDD_SIZE)
        {
            MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
            snprintf(storageDiskInfo[cameraIndex].mountPoint, MOUNT_POINT_SIZE, "%s", mntPoint);
            storageDiskInfo[cameraIndex].currentRecordVolumeId = recordVolumeId;
            MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);

            MUTEX_LOCK(pMediaInfo->rawMediaMutex);
            pMediaInfo->diskVolStatus = DM_DISK_VOL_NORMAL;
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

            updateRecordDiskInfoFileForCamera(cameraIndex, recordVolumeId, hddConfig.mode);
            DPRINT(DISK_MANAGER, "previous recording volume was full/faulty, start recording on new volume: [camera=%d], [volume=%d]", cameraIndex, recordVolumeId);
            return SUCCESS;
        }

        /* If it is force switch and other volumes are full then we have to allow switching */
        if (TRUE == forceSwitch)
        {
            /* This volume is usable but it is full */
            isVolumeUsable = TRUE;
            lastUsableVolumeId = recordVolumeId;
            DPRINT(DISK_MANAGER, "recording volume is available but full: [camera=%d], [volume=%d]", cameraIndex, recordVolumeId);
        }

        DPRINT(DISK_MANAGER, "sufficient space not available: [camera=%d], [volume=%d]", cameraIndex, recordVolumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if((pMediaInfo->mediaStatus != NOT_PRESENT) && (pMediaInfo->mediaStatus != UNMOUNTED))
        {
            pMediaInfo->diskVolStatus = DM_DISK_VOL_FULL;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Force switch the recording on disk full drive */
    if (TRUE == isVolumeUsable)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(hddConfig.mode, lastUsableVolumeId);
        WPRINT(DISK_MANAGER, "force switch recording volume: [camera=%d], [volume=%d]", cameraIndex, lastUsableVolumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", pMediaInfo->mountPoint);
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        snprintf(storageDiskInfo[cameraIndex].mountPoint, MOUNT_POINT_SIZE, "%s", mntPoint);
        storageDiskInfo[cameraIndex].currentRecordVolumeId = lastUsableVolumeId;
        MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        updateRecordDiskInfoFileForCamera(cameraIndex, lastUsableVolumeId, hddConfig.mode);
        return SUCCESS;
    }

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was gives the size of whole disk including all partition present on the disk.
 * @param   deviceNode
 * @param   diskSize
 * @return  SUCCESS/FAIL
 */
static BOOL getSizeOfDisk(CHARPTR deviceNode, UINT64PTR diskSize)
{
    INT32 devFd;

    devFd = open(deviceNode, READ_ONLY_MODE);
    if (devFd == INVALID_FILE_FD)
    {
        return FAIL;
    }

    if (ioctl(devFd, BLKGETSIZE64, diskSize) < 0)
    {
        close(devFd);
        return FAIL;
    }

    close(devFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CheckHddFormatDevice
 * @param   formatDevice
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e CheckHddFormatDevice(UINT8 formatDevice)
{
    HDD_CONFIG_t        hddConfig;
    UINT8               cameraIndex;
    RAW_MEDIA_INFO_t    *pMediaInfo = NULL;

    /* Read Hdd Config */
    ReadHddConfig(&hddConfig);
    if (formatDevice < GetTotalDiskNumber(hddConfig.mode))
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(hddConfig.mode, formatDevice);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus == NOT_PRESENT)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            return CMD_NO_DISK_FOUND;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        /* User has requested to format the device, if this device was also used in recording purpose then we need to
         * stop all the functionality related to this hard disk like recording, playback, searching, backup from this device */
        for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
        {
            MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
            if (storageDiskInfo[cameraIndex].currentRecordVolumeId == formatDevice)
            {
                MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
                SetHddNonFuncStatus(TRUE);
                break;
            }
            MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        }
    }

    if (GetHddVolBuildStatus() == TRUE)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was format the hard disk present on system.
 * @param   volumeNum
 * @param   advncDetail
 * @return  SUCCESS/FAIL
 */
BOOL ReadyHddPartion(UINT8 volumeNum, CHARPTR advncDetail)
{
    BOOL                status;
    CHAR                mountPoint[MOUNT_POINT_SIZE];
    DISK_SIZE_t         diskSizeInfo;
    HDD_CONFIG_t        hddConfig;
    RAW_MEDIA_INFO_t    *pMediaInfo;

    ReadHddConfig(&hddConfig);
    if (hddConfig.mode == SINGLE_DISK_VOLUME)
    {
        status = makeHarddiskReady(volumeNum, advncDetail);
    }
    else
    {
        status = readyRaidPartition(hddConfig.mode, volumeNum, advncDetail);
    }

    pMediaInfo = GET_MEDIA_VOLUME_INFO(hddConfig.mode, volumeNum);
    if (status == FAIL)
    {
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        pMediaInfo->mediaStatus = UNMOUNTED;
        pMediaInfo->diskVolStatus = DM_DISK_VOL_FAULT;
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        SetDiskHealthStatus(STRG_HLT_ERROR);
        SetCameraVolumeHealthStatus(volumeNum, STRG_HLT_ERROR);
        return FAIL;
    }

    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", pMediaInfo->mountPoint);
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

    if(GetSizeOfMountFs(mountPoint, &diskSizeInfo) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get size: [media=%s]", mediaNameStr[volumeNum]);
        return FAIL;
    }

    if(hddConfig.mode == SINGLE_DISK_VOLUME)
    {
        MUTEX_LOCK(physicalMediaInfo[volumeNum].phyMediaMutex);
        physicalMediaInfo[volumeNum].diskPhyStatus = DM_HDD_DISK_NORMAL;
        MUTEX_UNLOCK(physicalMediaInfo[volumeNum].phyMediaMutex);
    }

    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    pMediaInfo->mediaStatus = MOUNTED_READY;
    pMediaInfo->diskVolStatus = DM_DISK_VOL_NORMAL;
    pMediaInfo->totalVolSize = diskSizeInfo.totalSize;
    pMediaInfo->freeSize = diskSizeInfo.freeSize;
    pMediaInfo->usedSize = diskSizeInfo.usedSize;
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

    if(GetHddNonFuncStatus() == TRUE)
    {
        WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(hddConfig.mode, LOCAL_HARD_DISK), NULL, EVENT_NORMAL);
        handleDiskFaultAlert(INACTIVE);
        DPRINT(DISK_MANAGER, "media mounted ready: [media=%s]", mediaNameStr[volumeNum]);
    }

    /* Set current storage volume for recording if applicable */
    setCameraStorageVolume(volumeNum, hddConfig.mode, mountPoint);

    /* Update storage status */
    updateStorageStatus(hddConfig.mode);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was format the hard disk present on system.
 * @param   backupDevice
 * @return  SUCCESS/FAIL
 */
NET_CMD_STATUS_e CheckBackupDiskFormatStatus(DM_BACKUP_TYPE_e backupDevice)
{
    if (backupDevice >= DM_MAX_BACKUP)
    {
        return CMD_INVALID_INDEX_ID;
    }

    if (CMD_SUCCESS != CheckBackupDiskPresent(backupDevice))
    {
        return CMD_NO_DISK_FOUND;
    }

    if (GetHddVolBuildStatus() == TRUE)
    {
        return CMD_RESOURCE_LIMIT;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was thread function which will format the hdd media.
 * @param   formatType
 * @param   backupDevice
 */
void ReadyUsbMedia(RAW_MEDIA_FORMAT_TYPE_e formatType, DM_BACKUP_TYPE_e backupDevice)
{
    UINT8               deviceIndex;
    RAW_MEDIA_STATUS_e  mediaStatus;
    CHAR                deviceNode[NODE_NAME_SIZE];
    CHAR                mountPoint[MOUNT_POINT_SIZE];
    DISK_SIZE_t         diskSizeInfo;

    if (backupDevice >= DM_MAX_BACKUP)
    {
        return;
    }

    deviceIndex = (backupDevice + MANUAL_BACKUP_DISK);
    MUTEX_LOCK(physicalMediaInfo[deviceIndex].phyMediaMutex);
    snprintf(deviceNode, NODE_NAME_SIZE, "%s", physicalMediaInfo[deviceIndex].partitionNode);
    MUTEX_UNLOCK(physicalMediaInfo[deviceIndex].phyMediaMutex);

    MUTEX_LOCK(storageMedia[deviceIndex].rawMediaMutex);
    snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", storageMedia[deviceIndex].mountPoint);
    mediaStatus = storageMedia[deviceIndex].mediaStatus;
    MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);

    do
    {
        if(mediaStatus == MOUNTED_READY)
        {
            /* Unmount running device by taking care of ftp service */
            if (FAIL == unmountRunningDevice(mountPoint))
            {
                EPRINT(DISK_MANAGER, "media not unmountd: [media=%s]", mediaNameStr[deviceIndex]);
                break;
            }
        }
        else
        {
            snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", usbMointPoint[backupDevice]);
            MUTEX_LOCK(storageMedia[deviceIndex].rawMediaMutex);
            snprintf(storageMedia[deviceIndex].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
            MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);
        }

        if(makeUsbDiskReady(deviceIndex, FAT) == FAIL)
        {
            EPRINT(DISK_MANAGER, "usb disk not ready: [media=%s]", mediaNameStr[deviceIndex]);
            break;
        }

        DPRINT(DISK_MANAGER, "mount the usb device: [node=%s], [mountPoint=%s]", deviceNode, mountPoint);
        if(mountDevice(deviceNode, mountPoint, FAT) != SUCCESS)
        {
            EPRINT(DISK_MANAGER, "usb device not mounted: [node=%s], [mountPoint=%s]", deviceNode, mountPoint);
            break;
        }

        if(GetSizeOfMountFs(mountPoint, &diskSizeInfo) == FAIL)
        {
            EPRINT(DISK_MANAGER, "usb disk size not found: [media=%s]", mediaNameStr[deviceIndex]);
            break;
        }

        MUTEX_LOCK(storageMedia[deviceIndex].rawMediaMutex);
        storageMedia[deviceIndex].mediaStatus = MOUNTED_READY;
        storageMedia[deviceIndex].freeSize = diskSizeInfo.freeSize;
        storageMedia[deviceIndex].usedSize = diskSizeInfo.usedSize;
        storageMedia[deviceIndex].totalVolSize = diskSizeInfo.totalSize;
        MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);
        DPRINT(DISK_MANAGER, "usb mounted ready: [media=%s]", mediaNameStr[deviceIndex]);

        if(deviceIndex == MANUAL_BACKUP_DISK)
        {
            SetBackupLedState(BACKUP_NO_ACITIVITY);
        }

        /* Update backup status */
        UpdateBackupStatus(backupDevice);
        return;

    }while(0);

    if(physicalMediaInfo[deviceIndex].diskState == REMOVED)
    {
        MUTEX_LOCK(storageMedia[deviceIndex].rawMediaMutex);
        storageMedia[deviceIndex].mediaStatus = NOT_PRESENT;
        MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will check whether backup usb disk is present or not
 * @param   backupDevice
 * @return
 */
NET_CMD_STATUS_e CheckBackupDiskPresent(DM_BACKUP_TYPE_e backupDevice)
{
    UINT8 deviceIndex;

    if (backupDevice >= DM_MAX_BACKUP)
    {
        return CMD_PROCESS_ERROR;
    }

    deviceIndex = (backupDevice + MANUAL_BACKUP_DISK);
    MUTEX_LOCK(storageMedia[deviceIndex].rawMediaMutex);
    if(storageMedia[deviceIndex].mediaStatus == NOT_PRESENT)
    {
        MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);
        return CMD_NO_DISK_FOUND;
    }
    MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is thread function which removes backup device. It is removing disk by unmounting disk.
 * @param   deviceIndex
 * @return  SUCCESS/FAIL
 */
BOOL RemoveBackupDisk(UINT8	deviceIndex)
{
    CHAR mountPoint[MOUNT_POINT_SIZE];

    MUTEX_LOCK(storageMedia[deviceIndex].rawMediaMutex);
    snprintf(mountPoint, MOUNT_POINT_SIZE, "%s", storageMedia[deviceIndex].mountPoint);
    MUTEX_UNLOCK(storageMedia[deviceIndex].rawMediaMutex);
    if(unmountDevice(mountPoint) == FAIL)
    {
        return FAIL;
    }

    /* Reset storage volume information */
    resetStorageVolumeInfo(&storageMedia[deviceIndex], NOT_PRESENT, DM_DISK_VOL_MAX);

    MUTEX_LOCK(physicalMediaInfo[deviceIndex].phyMediaMutex);
    physicalMediaInfo[deviceIndex].physicalSize = DEFAULT_DISK_SIZE;
    MUTEX_UNLOCK(physicalMediaInfo[deviceIndex].phyMediaMutex);
    if(deviceIndex == MANUAL_BACKUP_DISK)
    {
        SetBackupLedState(BACKUP_NOT_GOING_ON);
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives recording path.
 * @param   path
 */
void GetHddRecordingPath(UINT8 cameraIndex, CHARPTR path)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        RESET_STR_BUFF(path);
        return;
    }

    MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    snprintf(path, MOUNT_POINT_SIZE,"%s", storageDiskInfo[cameraIndex].mountPoint);
    MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives hard disk status in which recording is enable
 * @param   hddMode
 * @return
 */
NET_CMD_STATUS_e GiveHddStatus(HDD_MODE_e hddMode)
{
    NET_CMD_STATUS_e 		status = CMD_SUCCESS;
    UINT8					diskNo, totalDiskCnt;
    DISK_VOLUME_INFO_t		diskVolInfo = { 0 };
    PHYSICAL_DISK_INFO_t	diskPhyInfo = { 0 };

    totalDiskCnt = GetTotalDiskNumber(hddMode);
    for(diskNo = 0; diskNo < totalDiskCnt; diskNo++)
    {
        GetDiskVolumeInfo(diskNo, &diskVolInfo);
        GetPhysicalDiskInfo(diskNo, &diskPhyInfo);

        if (diskPhyInfo.status == DM_HDD_NO_DISK)
        {
            status = CMD_NO_DISK_FOUND;
        }
        else if((diskVolInfo.status == DM_DISK_VOL_FORMATTING) || (diskVolInfo.status == DM_DISK_VOL_CREATING))
        {
            status = CMD_FORMAT_IN_PROCESS;
        }
        else
        {
            status = CMD_PROCESS_ERROR;
        }
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   When Writing data into hard disk generates error then this function invokes and stops
 *          hard disk related functionality.
 * @param   cameraIndex
 * @param   hddCfgPtr
 */
void HddErrorHandle(UINT8 cameraIndex, HDD_CONFIG_t *hddCfgPtr)
{
    static BOOL rebootWaitF = FALSE;
    CHAR        advanceDetails[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    /* Is action already taken? */
    if (TRUE == rebootWaitF)
    {
        return;
    }

    /* Action taken */
    rebootWaitF = TRUE;

    /* Set disk volume fault status */
    SetDiskVolumeFault(cameraIndex, hddCfgPtr->mode, DM_DISK_VOL_FAULT, advanceDetails);

    /* Stop all recording related operations */
    StopRecordDueToDiskFaultEvent(LOCAL_HARD_DISK);

    /* Then restart system because HDD in fault */
    EPRINT(SYS_LOG, "disk error found, so restart the system: [advnceDetail=%s]", advanceDetails);
    PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, advanceDetails);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set fault status in disk volume
 * @param   cameraIndex
 * @param   mode
 * @param   volStatus
 * @param   advanceDetails
 * @return  Returns SUCCESS if recording possible in other volume else returns FAIL
 */
BOOL SetDiskVolumeFault(UINT8 cameraIndex, HDD_MODE_e mode, DISK_VOL_STATUS_e volStatus, CHAR *advanceDetails)
{
    UINT8               recordVolumeId;
    CHAR                mntPoint[MOUNT_POINT_SIZE] = {0};
    RAW_MEDIA_INFO_t    *pMediaInfo;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(SYS_LOG, "invld camera index: [camera=%d]", cameraIndex);
        return FAIL;
    }

    MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    recordVolumeId = storageDiskInfo[cameraIndex].currentRecordVolumeId;
    snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", storageDiskInfo[cameraIndex].mountPoint);
    MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);

    /* Unmount the faulty volume */
    if (FAIL == unmountDevice(mntPoint))
    {
        EPRINT(DISK_MANAGER, "fail to unmount volume: [mntPoint=%s]", mntPoint);
    }

    WPRINT(DISK_MANAGER, "storage volume faulty: [camera=%d], [mode=%d], [diskId=%d], [mntPoint=%s], [volStatus=%d]",
           cameraIndex, mode, recordVolumeId, mntPoint, volStatus);
    if (mode == SINGLE_DISK_VOLUME)
    {
        /* Set media info of hard-disk volume */
        snprintf(advanceDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Disk Fault | %s", mediaNameStr[recordVolumeId]);
    }
    else
    {
        /* Get media info of raid volume */
        CHAR volumeName[MAX_VOLUME_NAME_SIZE];
        prepareRaidVolumeName(mode, recordVolumeId, volumeName);
        snprintf(advanceDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Disk Fault | %s", volumeName);
    }

    pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, recordVolumeId);
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    pMediaInfo->mediaStatus = UNMOUNTED;
    pMediaInfo->diskVolStatus = volStatus;
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

    /* Set system health status as faulty */
    SetDiskHealthStatus(STRG_HLT_ERROR);
    handleDiskFaultAlert(ACTIVE);
    WriteEvent(LOG_STORAGE_EVENT, LOG_HDD_STATUS, GetStorageEventDetail(mode, LOCAL_HARD_DISK), NULL, EVENT_FAULT);

    UINT8 totalVolume = GetVolumeNo(mode);
    if (totalVolume == 0)
    {
        EPRINT(SYS_LOG, "disk error found but don't have spare volume for recording: [camera=%d]", cameraIndex);
        return FAIL;
    }

    DPRINT(SYS_LOG, "disk error found but other spare volume available for recording: [camera=%d], [totalVolume=%d]", cameraIndex, totalVolume);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total number of disk which is being as whole volume.
 * @param   mode
 * @return
 */
UINT32 GetTotalDiskNumber(HDD_MODE_e mode)
{
    if (mode == SINGLE_DISK_VOLUME)
    {
        return getMaxHardDiskForCurrentVariant();
    }
    else
    {
        return (getTotalRaidVolume(mode) * MAX_RAID_VOLUME);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives mount point of given disk id,if disk mounted proper.
 * @param   diskId
 * @param   path
 * @param   hddMode
 * @return  SUCCESS/FAIL
 */
BOOL GetMountPointFromLocalDiskId(UINT32 diskId, CHARPTR path, HDD_MODE_e hddMode)
{
    if (diskId >= GetTotalDiskNumber(hddMode))
    {
        return FAIL;
    }

    RAW_MEDIA_INFO_t *pMediaInfo = GET_MEDIA_VOLUME_INFO(hddMode, diskId);
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    if(pMediaInfo->mediaStatus != MOUNTED_READY)
    {
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
        path[0] = '\0';
        return FAIL;
    }
    snprintf(path, MOUNT_POINT_SIZE,"%s", pMediaInfo->mountPoint);
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives mount point of given disk id,if disk mounted proper.
 * @param   diskId
 * @param   path
 * @return  SUCCESS/FAIL
 */
BOOL GetMountPointForBackupDevice(UINT32 diskId, CHARPTR path)
{
    if((diskId != MANUAL_BACKUP_DISK) && (diskId != SCHEDULE_BACKUP_DISK))
    {
        return FAIL;
    }

    MUTEX_LOCK(storageMedia[diskId].rawMediaMutex);
    if(storageMedia[diskId].mediaStatus != MOUNTED_READY)
    {
        MUTEX_UNLOCK(storageMedia[diskId].rawMediaMutex);
        path[0] = '\0';
        return FAIL;
    }
    snprintf(path, MOUNT_POINT_SIZE,"%s", storageMedia[diskId].mountPoint);
    MUTEX_UNLOCK(storageMedia[diskId].rawMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates size of given disk Id.
 * @param   diskId
 * @return  SUCCESS/FAIL
 * @note    This function only used from backup manager to updated size of copied data.
 */
BOOL UpdateUsbDiskSize(UINT32 diskId)
{
    CHAR        mountPoint[MOUNT_POINT_SIZE];
    DISK_SIZE_t diskSize;

    if(GetMountPointForBackupDevice(diskId, mountPoint) == FAIL)
    {
        return FAIL;
    }

    if(GetSizeOfMountFs(mountPoint, &diskSize) == FAIL)
    {
        return FAIL;
    }

    MUTEX_LOCK(storageMedia[diskId].rawMediaMutex);
    storageMedia[diskId].totalVolSize = diskSize.totalSize;
    storageMedia[diskId].freeSize = diskSize.freeSize;
    storageMedia[diskId].usedSize = diskSize.usedSize;
    MUTEX_UNLOCK(storageMedia[diskId].rawMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will provide the size of given camera's recording volume
 * @param   cameraIndex
 * @param   mode
 * @param   diskSize
 * @return  SUCCESS/FAIL
 */
BOOL GetRecordingDiskSize(UINT8 cameraIndex, HDD_MODE_e mode, DISK_SIZE_t *diskSize)
{
    UINT8   recordVolumeId;
    CHAR    mntPiont[MOUNT_POINT_SIZE];

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d], [mode=%d]", cameraIndex, mode);
        return FAIL;
    }

    MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    snprintf(mntPiont, MOUNT_POINT_SIZE, "%s", storageDiskInfo[cameraIndex].mountPoint);
    recordVolumeId = storageDiskInfo[cameraIndex].currentRecordVolumeId;
    MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);

    if (recordVolumeId >= GetTotalDiskNumber(mode))
    {
        EPRINT(DISK_MANAGER, "invld recording volume: [camera=%d], [mode=%d], [curDiskId=%d]", cameraIndex, mode, recordVolumeId);
        return FAIL;
    }

    if (GetSizeOfMountFs(mntPiont, diskSize) == FAIL)
    {
        return FAIL;
    }

    /* Update disk storage also so user can check */
    RAW_MEDIA_INFO_t *pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, recordVolumeId);
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    pMediaInfo->freeSize = diskSize->freeSize;
    pMediaInfo->usedSize = diskSize->usedSize;
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total number of volume present in system.
 * @param   mode
 * @return  SUCCESS/FAIL
 */
UINT8 GetVolumeNo(HDD_MODE_e mode)
{
    UINT8               volumeId;
    UINT8               volCnt = 0;
    UINT8               totalVolCnt = GetTotalDiskNumber(mode);
    RAW_MEDIA_INFO_t    *pMediaInfo;

    for(volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus == MOUNTED_READY)
        {
            volCnt++;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    return volCnt;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function updates size of given disk Id.
 * @param   mode
 * @param   diskSize
 */
void GetVolumeSize(HDD_MODE_e mode, DISK_SIZE_t *diskSize)
{
    UINT8               volumeId;
    UINT8               totalVolCnt;
    CHAR                mntPoint[MOUNT_POINT_SIZE];
    DISK_SIZE_t         diskInfo;
    RAW_MEDIA_INFO_t    *pMediaInfo;

    memset(diskSize, 0, sizeof(DISK_SIZE_t));
    totalVolCnt = GetTotalDiskNumber(mode);

    for(volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus != MOUNTED_READY)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            continue;
        }
        snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", pMediaInfo->mountPoint);
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        if (GetSizeOfMountFs(mntPoint, &diskInfo) == SUCCESS)
        {
            diskSize->freeSize += diskInfo.freeSize;
            diskSize->totalSize += diskInfo.totalSize;
            diskSize->usedSize += diskInfo.usedSize;
        }
    }

    if(diskSize->totalSize != (diskSize->freeSize + diskSize->usedSize))
    {
        EPRINT(DISK_MANAGER, "mismatch in disk size: [total=%d], [free=%d], [used=%d]", diskSize->totalSize, diskSize->freeSize, diskSize->usedSize);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function provides storage information of camera's group
 * @param   cameraIndex
 * @param   mode
 * @param   diskSize
 */
void GetCameraGroupVolumeSize(UINT8 cameraIndex, HDD_MODE_e mode, DISK_SIZE_t *diskSize)
{
    UINT8               volumeId;
    UINT8               totalVolCnt;
    CHAR                mntPoint[MOUNT_POINT_SIZE];
    UINT32              volumeAllocMask;
    DISK_SIZE_t         diskInfo;
    RAW_MEDIA_INFO_t    *pMediaInfo;

    /* Get total volumes and camera volume allocation mask */
    memset(diskSize, 0, sizeof(DISK_SIZE_t));
    totalVolCnt = GetTotalDiskNumber(mode);
    volumeAllocMask = GetCameraStorageVolumeAllocationMask(cameraIndex, mode);
    if (volumeAllocMask == 0)
    {
        return;
    }

    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        /* Is volume is part of current group? */
        if (FALSE == GET_BIT(volumeAllocMask, volumeId))
        {
            continue;
        }

        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus != MOUNTED_READY)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            continue;
        }
        snprintf(mntPoint, MOUNT_POINT_SIZE, "%s", pMediaInfo->mountPoint);
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        if (GetSizeOfMountFs(mntPoint, &diskInfo) == SUCCESS)
        {
            diskSize->freeSize += diskInfo.freeSize;
            diskSize->totalSize += diskInfo.totalSize;
            diskSize->usedSize += diskInfo.usedSize;
        }
    }

    if (diskSize->totalSize != (diskSize->freeSize + diskSize->usedSize))
    {
        EPRINT(DISK_MANAGER, "mismatch in disk size: [total=%d], [free=%d], [used=%d]", diskSize->totalSize, diskSize->freeSize, diskSize->usedSize);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @return  TRUE/FALSE
 */
BOOL GetHddNonFuncStatus(void)
{
    MUTEX_LOCK(stopHddFunctMutex);
    BOOL retVal = stopHddFunction;
    MUTEX_UNLOCK(stopHddFunctMutex);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function returns hdd function status
 * @param   status
 */
void SetHddNonFuncStatus(BOOL status)
{
    MUTEX_LOCK(stopHddFunctMutex);
    stopHddFunction = status;
    MUTEX_UNLOCK(stopHddFunctMutex);
    DPRINT(DISK_MANAGER, "harddisk functional status: %s", status ? "non-usable" : "usable");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives current recoding disk number of camera
 * @return  Current recording disk of camera
 */
UINT8 GetCurRecordingDisk(UINT8 cameraIndex)
{
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        EPRINT(DISK_MANAGER, "invld camera index: [camera=%d]", cameraIndex);
        return 0;
    }

    MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    UINT8 recordVolumeId = storageDiskInfo[cameraIndex].currentRecordVolumeId;
    MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    return recordVolumeId;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function writes partitionNode into one file.
 * @param   mode
 * @return  SUCCESS/FAIL
 */
static UINT8 getTotalRaidVolume(HDD_MODE_e mode)
{
    if ((mode == RAID_0) || (mode == RAID_1))
    {
        return (getMaxHardDiskForCurrentVariant() / 2);
    }

    if ((mode == RAID_5) || (mode == RAID_10))
    {
        return (getMaxHardDiskForCurrentVariant() / 4);
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function iterates through the udev enumerator list and updates action for each list
 *          entry in the uevent file.
 * @param   udev_enumerate
 * @param   action
 */
static BOOL scanStorageDevices(void)
{
    UINT8                   waitCnt;
    INT32                   ueventFd, udevPid;
    CHAR                    ueventFile[1024];
    const CHAR              *path = NULL, *subsystem = NULL, *devtype = NULL, *devnode = NULL;
    struct udev             *udev = NULL;
    struct udev_device      *device = NULL;
    struct udev_enumerate	*udevEnum = NULL;
    struct udev_list_entry  *devices = NULL, *entry = NULL;

    do
    {
        /* Create a new udev context */
        udev = udev_new();
        if (NULL == udev)
        {
            EPRINT(DISK_MANAGER, "fail to create udev context");
            break;
        }

        /* Create a new udev enumerate object */
        udevEnum = udev_enumerate_new(udev);
        if (NULL == udevEnum)
        {
            EPRINT(DISK_MANAGER, "fail to create udev enumerate object");
            udev_unref(udev);
            break;
        }

        /* Scan for all devices */
        udev_enumerate_scan_devices(udevEnum);
        DPRINT(DISK_MANAGER, "start scan devices using udev");

        /* Get a list of devices from the enumerate object */
        devices = udev_enumerate_get_list_entry(udevEnum);

        /* Iterate over each device and get information about it */
        udev_list_entry_foreach(entry, devices)
        {
            /* Get the syspath for the device */
            path = udev_list_entry_get_name(entry);

            /* Get device and check if the device is a block device */
            device = udev_device_new_from_syspath(udev, path);
            subsystem = udev_device_get_subsystem(device);
            devtype = udev_device_get_devtype(device);

            /* It must be block device of disk type */
            if ((subsystem != NULL) && (strcmp(subsystem, "block") == 0) && (devtype != NULL) && (strcmp(devtype, "disk") == 0))
            {
                /* Get the device node */
                devnode = udev_device_get_devnode(device);

                /* Check if the device is a main block device (not a partition) and if it starts with /dev/sd */
                if ((devnode != NULL) && (strncmp(devnode, "/dev/sd", strlen("/dev/sd")) == 0) && (isdigit(devnode[strlen(devnode)-1]) == 0))
                {
                    /* Update total connected media count */
                    totalAttachedDevice++;
                }
            }

            /* Clean up */
            udev_device_unref(device);

            /* Prepare device uevent path */
            snprintf(ueventFile, sizeof(ueventFile), "%s/uevent", path);
            ueventFd = open(ueventFile, WRITE_ONLY_MODE);
            if (ueventFd == INVALID_FILE_FD)
            {
                continue;
            }

            if (write(ueventFd, DEVICE_ADD_ACTION, strlen(DEVICE_ADD_ACTION)) < 0)
            {
                EPRINT(DISK_MANAGER, "fail to write file: [path=%s]", ueventFile);
            }

            close(ueventFd);
        }

        udev_enumerate_unref(udevEnum);
        udev_unref(udev);
        return SUCCESS;

    }while(0);

    waitCnt = 0;
    sleep(2);
    while(TRUE)
    {
        /* Stop udev first and then start again */
        ExeSysCmd(TRUE, UDEV_START_STOP_SCRIPT " stop");
        sleep(2);

        udevPid = GetPidOfProcess("udevd");
        if(udevPid == (INT32)NILL)
        {
            DPRINT(SYS_LOG, "udev daemon stopped");
            break;
        }

        if (++waitCnt >= 10)
        {
            EPRINT(DISK_MANAGER, "fail to stop udev daemon");
            return FAIL;
        }
    }

    waitCnt = 0;
    sleep(2);
    while(TRUE)
    {
        /* start udev to scan devices */
        ExeSysCmd(TRUE, UDEV_START_STOP_SCRIPT " start");
        sleep(2);

        udevPid = GetPidOfProcess("udevd");
        if(udevPid != (INT32)NILL)
        {
            DPRINT(SYS_LOG, "udev daemon started: [pid=%d]", udevPid);
            break;
        }

        if (++waitCnt >= 10)
        {
            EPRINT(DISK_MANAGER, "fail to start udev daemon");
            return FAIL;
        }
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Api is for finding the lowest size disk from set of drives.
 * @param   startDiskId
 * @param   endDiskId
 * @param   pLeastSizeDiskId
 */
static void compareDiskSize(STORAGE_MEDIA_TYPE_e startDiskId, STORAGE_MEDIA_TYPE_e endDiskId, STORAGE_MEDIA_TYPE_e *pLeastSizeDiskId)
{
    STORAGE_MEDIA_TYPE_e  	outLoop, inLoop;
    UINT64 					sizeDiff, outSize, inSize, leastSize;
    BOOL                    isAllDiskSizeIdentical = TRUE;

    MUTEX_LOCK(physicalMediaInfo[startDiskId].phyMediaMutex);
    leastSize = physicalMediaInfo[startDiskId].physicalSize;
    MUTEX_UNLOCK(physicalMediaInfo[startDiskId].phyMediaMutex);
    *pLeastSizeDiskId = startDiskId;

    for(outLoop = startDiskId; outLoop <= endDiskId; outLoop++)
    {
        MUTEX_LOCK(physicalMediaInfo[outLoop].phyMediaMutex);
        outSize = physicalMediaInfo[outLoop].physicalSize;
        MUTEX_UNLOCK(physicalMediaInfo[outLoop].phyMediaMutex);
        if(outSize < leastSize)
        {
            leastSize = outSize;
            *pLeastSizeDiskId = outLoop;
        }

        DPRINT(DISK_MANAGER, "compare disk size: [disk=%s], [size=%lluGB]", mediaNameStr[outLoop], ((outSize * KILO_BYTE)/(1000000000)));
        if (FALSE == isAllDiskSizeIdentical)
        {
            continue;
        }

        for(inLoop = startDiskId + 1; inLoop <= endDiskId; inLoop++)
        {
            MUTEX_LOCK(physicalMediaInfo[inLoop].phyMediaMutex);
            inSize = physicalMediaInfo[inLoop].physicalSize;
            MUTEX_UNLOCK(physicalMediaInfo[inLoop].phyMediaMutex);

            sizeDiff = (outSize > inSize) ? (outSize - inSize) : (inSize - outSize);
            if (sizeDiff >= IDENTICAL_OFFSET)
            {
                isAllDiskSizeIdentical = FALSE;
                break;
            }
        }
    }

    DPRINT(DISK_MANAGER, "compare disk size: [isAllDiskSizeIdentical=%s], [leastSizeDiskId=%d]", isAllDiskSizeIdentical ? "YES" : "NO", *pLeastSizeDiskId);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function calculates the resulting raid volume and compares it with max value.
 *          if out of range then decides the partition sizes and numbers
 * @param   mode
 * @param   diskSizeInMb
 * @param   hddPresentCnt
 * @param   hddPartitionData
 */
static void predictLogicalRaidVolume(HDD_MODE_e mode, UINT64 diskSizeInMb, UINT8 hddPresentCnt, HDD_RAID_INFO_t *hddPartitionData)
{
    UINT32  diskSize, logicalRaidVolume = 0, maxPartitionSize;
    UINT32  startPartition, endPartition;
    UINT8   divisionValue = 1, totalPartitions;
    UINT8   partCnt;

    /* convert size into GB format */
    diskSize = ((diskSizeInMb * KILO_BYTE)/(1000000000));

    /* offset to recover manufacturer size variations */
    diskSize = (diskSize - 3);

    switch(mode)
    {
        case RAID_0:
            logicalRaidVolume = (diskSize * hddPresentCnt);
            divisionValue = 2;                      /* logical volume size is sum of both disks */
            break;

        case RAID_1:
            logicalRaidVolume = (diskSize * 1);
            divisionValue = 1;                      /* logical volume size is equal to lowest size disk */
            break;

        case RAID_5:
            logicalRaidVolume = (diskSize * (hddPresentCnt - 1));
            divisionValue = (hddPresentCnt - 1);    /* logical volume size is equal to sum of drives except 1 parity drive */
            break;

        case RAID_10:
            logicalRaidVolume = (diskSize * (hddPresentCnt - 2));
            divisionValue = (hddPresentCnt - 2);    /* logical volume size is equal to sum of drives except 2 parity drive */
            break;

        default:
            break;
    }

    maxPartitionSize = diskSize;
    totalPartitions = 1;

    /* Reason for considering the maximum array size as 12TB is that, more 16 TB volume was not mounted successfully */
    if(logicalRaidVolume >= MAX_ALLOWED_RAID_VOLUME_SIZE_GB)
    {
        maxPartitionSize = (MAX_ALLOWED_RAID_VOLUME_SIZE_GB / divisionValue);
        if(diskSize > maxPartitionSize)
        {
            totalPartitions = (diskSize / maxPartitionSize);

            /* if more than 50GB then only create partition else keep it redundant */
            if((diskSize - (totalPartitions * maxPartitionSize)) > 50)
            {
                totalPartitions++;
            }
        }
    }

    DPRINT(DISK_MANAGER, "raid volume info: [mode=%d], [logicalRaidVolume=%dGB], [maxPartitionSize=%dGB], [totalPartitions=%d]",
           mode, logicalRaidVolume, maxPartitionSize, totalPartitions);
    memset(hddPartitionData, 0, sizeof(HDD_RAID_INFO_t));

    /* for first partition the start location should be 100MB for proper alignment */
    startPartition = 100;
    endPartition = maxPartitionSize;
    if(totalPartitions > MAX_PARTITION_SUPPORT)
    {
        totalPartitions = MAX_PARTITION_SUPPORT;
    }

    hddPartitionData->totalPartitions = totalPartitions;
    for(partCnt = 0; partCnt < totalPartitions; partCnt++)
    {
        hddPartitionData->partitionInfo[partCnt].partitionIndex = (partCnt + 1);
        hddPartitionData->partitionInfo[partCnt].ptStartLoc = startPartition;
        hddPartitionData->partitionInfo[partCnt].ptEndLoc = endPartition;
        DPRINT(DISK_MANAGER, "[partition=%d], [startPartition=%d], [endPartition=%d]", (partCnt + 1), startPartition, endPartition);

        /* Set partition start and end size for next partition */
        startPartition = endPartition;
        endPartition += maxPartitionSize;
        if (endPartition > diskSize)
        {
            endPartition = diskSize;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   fires the system commands for creating the partitions using parted utility
 * @param   hddIndex
 * @return  Returns SUCCESS or FAIL
 */
static BOOL createPartirionsForRaid(UINT8 hddIndex)
{
    BOOL    retVal = SUCCESS;
    CHAR    sysCmd[SYSTEM_COMMAND_SIZE];
    UINT8   partition, totalPartition;
    UINT32  startPartition, endPartition;
    CHAR    deviceNode[NODE_NAME_SIZE];
    CHAR    symbolicLink[SYMBOLIC_LINK_SIZE];

    MUTEX_LOCK(physicalMediaInfo[hddIndex].phyMediaMutex);
    snprintf(deviceNode, NODE_NAME_SIZE, "%s", physicalMediaInfo[hddIndex].mediaNodeName);
    snprintf(symbolicLink, SYMBOLIC_LINK_SIZE, "%s", physicalMediaInfo[hddIndex].symbolicLink);
    MUTEX_UNLOCK(physicalMediaInfo[hddIndex].phyMediaMutex);

    /* Example: parted -s /dev/sda mklabel gpt */
    snprintf(sysCmd, sizeof(sysCmd), PARTED_LABEL_CMD, deviceNode);
    DPRINT(DISK_MANAGER, "creating partition: [media=%s], [node=%s]", mediaNameStr[hddIndex], deviceNode);
    if (FAIL == ExeSysCmd(TRUE, sysCmd))
    {
        EPRINT(DISK_MANAGER, "partition label cmd fail: [media=%s], [cmd=%s]", mediaNameStr[hddIndex], sysCmd);
        return FAIL;
    }

    /* Set LED cadence for disk format */
    SetSystemStatusLed(SYS_FORMATTING, ON);

    /* To get exact value in GB */
    totalPartition = raidDiskPartInfo[hddIndex].totalPartitions;
    for(partition = 0; partition < totalPartition; partition++)
    {
        if(raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionIndex != (partition + 1))
        {
            continue;
        }

        startPartition = raidDiskPartInfo[hddIndex].partitionInfo[partition].ptStartLoc;
        endPartition = raidDiskPartInfo[hddIndex].partitionInfo[partition].ptEndLoc;

        /* first partition should be start from 100MB for proper alignment */
        if (partition == 0)
        {
            /* Example: parted -s /dev/sda mkpart primary ext4 100MB 4000GB */
            snprintf(sysCmd, sizeof(sysCmd), PARTED_FIRST_PART_CMD, deviceNode, startPartition, endPartition);
        }
        else
        {
            /* Example: parted -s /dev/sda mkpart primary ext4 4000GB 8000GB */
            snprintf(sysCmd, sizeof(sysCmd), PARTED_ALL_PART_CMD, deviceNode, startPartition, endPartition);
        }

        if(FAIL == ExeSysCmd(TRUE, sysCmd))
        {
            EPRINT(DISK_MANAGER, "partition create cmd fail: [media=%s], [cmd=%s]", mediaNameStr[hddIndex], sysCmd);
            retVal = FAIL;
            break;
        }

        snprintf(raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionNodeName, NODE_NAME_SIZE, "%s%d", deviceNode, (partition + 1));
        snprintf(raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionSymbLink, SYMBOLIC_LINK_SIZE, "%s%d", symbolicLink, (partition + 1));
        DPRINT(DISK_MANAGER, "disk partition created and formating: [media=%s], [partitionNodeName=%s], [partitionSymbLink=%s]",
               mediaNameStr[hddIndex], raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionNodeName,
               raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionSymbLink);
        sleep(5);
        if(formatWithExt4(raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionNodeName) == FAIL)
        {
            EPRINT(DISK_MANAGER, "fail to format partition: [media=%s], [node=%s]",
                   mediaNameStr[hddIndex], raidDiskPartInfo[hddIndex].partitionInfo[partition].partitionNodeName);
            retVal = FAIL;
            break;
        }
    }

    /* Stop LED cadence for disk format */
    SetSystemStatusLed(SYS_FORMATTING, OFF);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is a very important function which derives the partition information for all required
 *          disks after boot up and stores in global database for future raid validation
 * @param   hddIndex
 * @return
 */
static BOOL getDiskPartitionsInfo(UINT8 hddIndex)
{
    UINT8		partitionInfo[MAX_PARTITION_SUPPORT], partitionCnt, totalPartitions = 0;
    CHAR		gptPratitionInfo[MAX_PARTITION_SUPPORT][GPT_PARTITION_TYPE_LEN];
    CHAR		mediaSymbolicLink[SYMBOLIC_LINK_SIZE];
    CHAR		partitionSymbolicLink[SYMBOLIC_LINK_SIZE];
    CHAR		mediaNodeName[NODE_NAME_SIZE];
    INT32 		blockSize;

    MUTEX_LOCK(physicalMediaInfo[hddIndex].phyMediaMutex);
    snprintf(mediaNodeName, NODE_NAME_SIZE, "%s", physicalMediaInfo[hddIndex].mediaNodeName);
    snprintf(mediaSymbolicLink, SYMBOLIC_LINK_SIZE, "%s", physicalMediaInfo[hddIndex].symbolicLink);
    MUTEX_UNLOCK(physicalMediaInfo[hddIndex].phyMediaMutex);

    if (getPartitionNoAndType(mediaNodeName, partitionInfo) == FAIL)
    {
        EPRINT(DISK_MANAGER, "partition info not found: [media=%s]", mediaNameStr[hddIndex]);
        raidDiskPartInfo[hddIndex].totalPartitions = 0;
        return FAIL;
    }

    if (getGptPartitionNoAndType(mediaNodeName, &gptPratitionInfo[0][0], MAX_PARTITION_SUPPORT, &blockSize) == FAIL)
    {
        EPRINT(DISK_MANAGER, "fail to get gpt partition table: [media=%s]", mediaNameStr[hddIndex]);
        raidDiskPartInfo[hddIndex].totalPartitions = 0;
        return FAIL;
    }

    for(partitionCnt = 0; partitionCnt < MAX_PARTITION_SUPPORT; partitionCnt++)
    {
        if (FALSE == isValidGptGuidType(gptPratitionInfo[partitionCnt]))
        {
            continue;
        }

        snprintf(partitionSymbolicLink, SYMBOLIC_LINK_SIZE, "%s%d", mediaSymbolicLink, (partitionCnt + 1));
        if(readBlockDeviceInfo(hddIndex, partitionCnt, mediaSymbolicLink, blockSize, partitionSymbolicLink) == FAIL)
        {
            continue;
        }

        totalPartitions++;
        DPRINT(DISK_MANAGER, "[media=%s], [partition=%d], [gtpPartId=%s]", mediaNameStr[hddIndex], partitionCnt, gptPratitionInfo[partitionCnt]);
        raidDiskPartInfo[hddIndex].partitionInfo[partitionCnt].partitionIndex = (partitionCnt + 1);
        snprintf(raidDiskPartInfo[hddIndex].partitionInfo[partitionCnt].partitionNodeName, NODE_NAME_SIZE, "%s%d", mediaNodeName, (partitionCnt + 1));
        snprintf(raidDiskPartInfo[hddIndex].partitionInfo[partitionCnt].partitionSymbLink, SYMBOLIC_LINK_SIZE, "%s%d", mediaSymbolicLink, (partitionCnt + 1));
    }

    DPRINT(DISK_MANAGER, "[media=%s], [totalPartitions=%d]", mediaNameStr[hddIndex], totalPartitions);
    raidDiskPartInfo[hddIndex].totalPartitions = totalPartitions;
    return totalPartitions ? SUCCESS : FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Provides the raid array count for raid group by considering the number of partitions in
 *          disk (same no of partitions available in all disks)
 * @param   mode
 * @param   hddPresentCnt
 * @param   raidGrpId
 * @return  Total partitions
 */
static UINT8 getPossibleRaidGroupVolumes(HDD_MODE_e mode, UINT8 hddPresentCnt, UINT8 raidGrpId)
{
    UINT8   hdd, totalPartitions = 0;
    UINT8   firstHdd, secondHdd;

    firstHdd = raidGrpId * raidArrayDiskCnt[mode][RAID_DISK_CNT_MAX];
    totalPartitions = raidDiskPartInfo[firstHdd].totalPartitions;
    for(hdd = 1; hdd < hddPresentCnt; hdd++)
    {
        secondHdd = firstHdd + hdd;
        if(totalPartitions != raidDiskPartInfo[secondHdd].totalPartitions)
        {
            EPRINT(DISK_MANAGER, "mismatch in partitions: [totalPartitions=%d], [firstHdd=%d], [secondHdd=%d]", totalPartitions, firstHdd, secondHdd);
            return 0;
        }
    }

    DPRINT(DISK_MANAGER, "total partitions in disks: [totalPartitions=%d]", totalPartitions);
    return totalPartitions;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Reads the /sys/block/ folder for getting the drives partition data including start and stop address.
 * @param   hddIndex
 * @param   partitionIndex
 * @param   mediaSymbolicLink
 * @param   blockSize
 * @param   partitionSymbLink
 * @return
 */
static BOOL readBlockDeviceInfo(UINT8 hddIndex, UINT8 partitionIndex, CHARPTR mediaSymbolicLink, INT32 blockSize, CHARPTR partitionSymbLink)
{
    INT32   fileFd = INVALID_FILE_FD;
    CHAR    fileName[SYMBOLIC_LINK_SIZE];
    CHAR    buf[25];
    UINT64  value, size, start, end;
    INT32   readBytes;

    /* /sys/block/sda/sda1/partition */
    snprintf(fileName, SYMBOLIC_LINK_SIZE, BLOCK_DEVICE_INFO_FILE_PATH, mediaSymbolicLink, partitionSymbLink, "partition");
    if(access(fileName, F_OK) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "block device partition file not present: [path=%s]", fileName);
        return FAIL;
    }

    fileFd = open(fileName, READ_ONLY_MODE);
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open block device partition file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
    }

    readBytes = read(fileFd, buf, sizeof(buf) - 1);
    if (readBytes < 0)
    {
        EPRINT(DISK_MANAGER, "fail to read block device partition file: [path=%s], [err=%s]", fileName, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    buf[readBytes] = '\0';
    close(fileFd);

    value = atoll(buf);
    if(value != ((UINT64)partitionIndex + 1))
    {
        EPRINT(DISK_MANAGER, "partition num mismatch: [path=%s], [fileNum=%lld], [sourceNum=%d]", fileName, value, (partitionIndex + 1));
        return FAIL;
    }

    DPRINT(DISK_MANAGER, "block device partition: [path=%s], [partitionNum=%lld]", fileName, value);

    /* /sys/block/sda/sda1/start */
    snprintf(fileName, SYMBOLIC_LINK_SIZE, BLOCK_DEVICE_INFO_FILE_PATH, mediaSymbolicLink, partitionSymbLink, "start");
    if(access(fileName, F_OK) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "block device start location file not present: [fileName=%s]", fileName);
        return FAIL;
    }

    if((fileFd = open(fileName, READ_ONLY_MODE)) == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open block device start location file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
    }

    readBytes = read(fileFd, buf, sizeof(buf) - 1);
    if(readBytes < 0)
    {
        EPRINT(DISK_MANAGER, "fail to read block device start location file: [path=%s], [err=%s]", fileName, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    buf[readBytes] = '\0';
    close(fileFd);

    start = atoll(buf);
    if(partitionIndex == 0)
    {
        DPRINT(DISK_MANAGER, "partition start location: [path=%s], [location=%lldMB]", fileName, (UINT64)((start * blockSize) / (BYTES_IN_MB)));
        raidDiskPartInfo[hddIndex].partitionInfo[partitionIndex].ptStartLoc = 100;
    }
    else
    {
        DPRINT(DISK_MANAGER, "partition start location: [path=%s], [location=%lldGB]", fileName, (UINT64)(((start * blockSize) / (BYTES_IN_GB))));
        raidDiskPartInfo[hddIndex].partitionInfo[partitionIndex].ptStartLoc = ((start * blockSize) / (BYTES_IN_GB));
    }

    /* /sys/block/sda/sda1/size  */
    snprintf(fileName, SYMBOLIC_LINK_SIZE, BLOCK_DEVICE_INFO_FILE_PATH, mediaSymbolicLink, partitionSymbLink, "size");
    if(access(fileName, F_OK) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "block device size file not present: [path=%s]", fileName);
        return FAIL;
    }

    if((fileFd = open(fileName, READ_ONLY_MODE)) == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open block device file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
    }

    readBytes = read(fileFd, buf, sizeof(buf) - 1);
    if(readBytes < 0)
    {
        EPRINT(DISK_MANAGER, "fail to read block device file: [path=%s], [err=%s]", fileName, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    buf[readBytes] = '\0';
    close(fileFd);

    size = atoll(buf);
    end = (start + size - 1);
    DPRINT(DISK_MANAGER, "partition end location: [path=%s], [location=%lldGB]", fileName, (UINT64)(((end * blockSize) / (BYTES_IN_GB))));
    raidDiskPartInfo[hddIndex].partitionInfo[partitionIndex].ptEndLoc = ((end * blockSize) / (BYTES_IN_GB));
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   creates the required numbers of partitions in newly replaced disk in array the data is
 *          derived from current working disk in array
 * @param   source
 * @param   dest
 * @return
 */
static BOOL verifyAndCreatePartitionsInMissingDisk(UINT8 source, UINT8 dest)
{
    HDD_RAID_INFO_t hddPartitionData;
    UINT32          startPartition, endPartition;
    UINT64          destDiskSize, srcDiskSize, partSize;
    UINT8           srcPartitions, part, destPartitions = 0;

    MUTEX_LOCK(physicalMediaInfo[dest].phyMediaMutex);
    destDiskSize = physicalMediaInfo[dest].physicalSize;
    MUTEX_UNLOCK(physicalMediaInfo[dest].phyMediaMutex);

    MUTEX_LOCK(physicalMediaInfo[source].phyMediaMutex);
    srcDiskSize = physicalMediaInfo[source].physicalSize;
    MUTEX_UNLOCK(physicalMediaInfo[source].phyMediaMutex);

    /* Convert disk size into GBs */
    destDiskSize = ((destDiskSize * KILO_BYTE)/(1000000000));
    srcDiskSize = ((srcDiskSize * KILO_BYTE)/(1000000000));
    srcPartitions = raidDiskPartInfo[source].totalPartitions;
    destPartitions = raidDiskPartInfo[dest].totalPartitions;

    if ((srcPartitions > 0) && (srcPartitions <= MAX_PARTITION_SUPPORT) && (raidDiskPartInfo[source].partitionInfo[(srcPartitions - 1)].ptEndLoc > destDiskSize))
    {
        EPRINT(DISK_MANAGER, "missing disk size is not same as current pair size");
        return FAIL;
    }

    partSize = (UINT64)raidDiskPartInfo[source].partitionInfo[0].ptEndLoc + 1;
    DPRINT(DISK_MANAGER, "creating partitions in missing disk: [size=%lld], [destDiskSize=%lld]", partSize, destDiskSize);
    if(destDiskSize > partSize)
    {
        destPartitions = (destDiskSize / partSize);

        /* if more than 50GB then only create partition else keep it redundant */
        if((destDiskSize - (destPartitions * partSize)) > 50)
        {
            destPartitions++;
        }

        if (destPartitions > MAX_PARTITION_SUPPORT)
        {
            destPartitions = MAX_PARTITION_SUPPORT;
        }
    }
    else
    {
        partSize = destDiskSize;
        destPartitions = 1;
    }

    /* If less partitons available in new disk then don't add that device in raid array */
    if (destPartitions < srcPartitions)
    {
        EPRINT(DISK_MANAGER, "required partitions are not possible: [srcDiskId=%d], [dstDiskId=%d], [required=%d], [possible=%d]",
               source, dest, srcPartitions, destPartitions);
        return FAIL;
    }

    startPartition = 100;
    endPartition = partSize;

    memset(&hddPartitionData, 0, sizeof(hddPartitionData));
    for(part = 0; part < destPartitions; part++)
    {
        hddPartitionData.partitionInfo[part].partitionIndex = (part + 1);
        hddPartitionData.partitionInfo[part].ptStartLoc = startPartition;
        hddPartitionData.partitionInfo[part].ptEndLoc = endPartition;
        DPRINT(DISK_MANAGER, "[partition=%d], [start=%d], [end=%d]", (part + 1), startPartition, endPartition);

        /* Derive size for next partition */
        startPartition = endPartition;
        endPartition += partSize;
        if (endPartition > destDiskSize)
        {
            endPartition = destDiskSize;
        }
    }

    hddPartitionData.totalPartitions = destPartitions;
    DPRINT(DISK_MANAGER, "required partitions are possible: [srcDiskId=%d], [dstDiskId=%d], [required=%d], [possible=%d]",
           source, dest, srcPartitions, destPartitions);
    raidDiskPartInfo[dest] = hddPartitionData;

    /* Create partitions and make it ready for raid */
    return createPartirionsForRaid(dest);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   updates the database for given raid volume
 * @param   raidVolNo
 * @param   status
 * @return
 */
static BOOL updateRaidVolumeInfo(UINT8 raidVolNo, BOOL status)
{
    DISK_SIZE_t diskSizeInfo;
    CHAR        mountPoint[MOUNT_POINT_SIZE];

    snprintf(mountPoint, MOUNT_POINT_SIZE, RAID_ARRAY_MOUNT_POINT, raidVolNo);
    if (status == TRUE)
    {
        if (GetSizeOfMountFs(mountPoint, &diskSizeInfo) == FAIL)
        {
            status = FALSE;
            EPRINT(DISK_MANAGER, "disk size not found: [raidVolNo=%d], [mountPoint=%s]", raidVolNo, mountPoint);
            if(unmountDevice(mountPoint) == FAIL)
            {
                EPRINT(DISK_MANAGER, "raid array is not unmounted: [raidVolNo=%d], [mountPoint=%s]", raidVolNo, mountPoint);
            }
        }
    }

    if (status == TRUE)
    {
        MUTEX_LOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        snprintf(raidVolumeInfo[raidVolNo].mountPoint, MOUNT_POINT_SIZE, "%s", mountPoint);
        raidVolumeInfo[raidVolNo].mediaStatus = MOUNTED_READY;
        raidVolumeInfo[raidVolNo].diskVolStatus = (diskSizeInfo.freeSize >= HDD_NORMAL_STATUS_SIZE) ? DM_DISK_VOL_NORMAL : DM_DISK_VOL_FULL;
        raidVolumeInfo[raidVolNo].diskFormatPercentage = 0;
        raidVolumeInfo[raidVolNo].totalVolSize = diskSizeInfo.totalSize;
        raidVolumeInfo[raidVolNo].freeSize = diskSizeInfo.freeSize;
        raidVolumeInfo[raidVolNo].usedSize = diskSizeInfo.usedSize;
        MUTEX_UNLOCK(raidVolumeInfo[raidVolNo].rawMediaMutex);
        DPRINT(DISK_MANAGER, "raid array built properly: [raidVolNo=%d], [mountPoint=%s]", raidVolNo, mountPoint);
    }
    else
    {
        rmdir(mountPoint);
        stopRaidArray(raidVolNo);

        /* Reset storage volume information */
        resetStorageVolumeInfo(&raidVolumeInfo[raidVolNo], UNMOUNTED, DM_DISK_VOL_FAULT);
        EPRINT(DISK_MANAGER, "raid volume faulty: [raidVolNo=%d], [mountPoint=%s]", raidVolNo, mountPoint);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is meant for backward compatibility for old raid volumes. if present then
 *          configure them using old method
 * @return  Returns TRUE if legacy raid present otherwise returns FALSE
 * @note    In legacy raid, "# TotalNodes" info was not available and we have added it in new raid.
 */
static BOOL isLegacyRaidArrayPresent(void)
{
    BOOL    legacyRaidF = FALSE;
    UINT8   raidGrpId;
    CHAR    fileName[MAX_FILE_NAME_SIZE];
    FILE    *pFile = NULL;
    CHAR    lineBuff[MAX_LINE];

    /* In older raid, we have only 4 volumes max for any type of raid */
    for(raidGrpId = 0; raidGrpId < MAX_RAID_VOLUME_GRP; raidGrpId++)
    {
        /* Prepare mdadm config file path as per old raid */
        snprintf(fileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidGrpId);
        if(access(fileName, F_OK) != STATUS_OK)
        {
            continue;
        }

        /* Open mdadm config file for reading */
        pFile = fopen(fileName, "r");
        if(pFile == NULL)
        {
            continue;
        }

        /* Assume it is legacy raid, if "# TotalNodes" in file then it is new raid */
        legacyRaidF = TRUE;
        while(fgets(lineBuff, sizeof(lineBuff), pFile))
        {
            if (NULL != strstr(lineBuff, RAID_TOTAL_NODE_STR))
            {
                /* It is new raid */
                legacyRaidF = FALSE;
            }
        }

        /* Close the file */
        fclose(pFile);

        /* Is legacy raid found? */
        if (TRUE == legacyRaidF)
        {
            DPRINT(DISK_MANAGER, "legacy raid mdadm config found");
            break;
        }
    }

    return legacyRaidF;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will migrate configuration from older raid implementation to latest one
 * @note    In legacy raid, "# TotalNodes" info was not available and maximum 4 raid array can be
 *          created. Each disk can be part of single raid because we had allowed only one partition
 *          in a disk.
 *          In new raid, we can create maximum 16 raid array(ideally). Each disk can be part of upto
 *          4 raid array. We allow 4 partitions in a disk.
 *          In migration, we will update the raid array node name, raid array name and TotalNodes in
 *          legacy config file and will change the config file name as per new raid.
 *          When we change raid array name in config file, we must have to update the same in raid
 *          disk super blocks forcefully.
 *          Hence we must have to execute raid assemble command with --update=name.
 */
static void legacyRaidConfigMigration(void)
{
    BOOL    updateRaidConfigF;
    UINT32  fileLen;
    FILE    *pFile;
    UINT8   nodeCnt, raidGrpId, raidGrpVol;
    CHAR    scanData[5][MAX_LINE];
    CHAR    lineBuff[MAX_LINE];
    CHAR    fileData[PATH_MAX];
    CHAR    nodeName[NODE_NAME_SIZE];
    CHAR    oldFileName[MAX_FILE_NAME_SIZE];
    CHAR    tmpFileName[MAX_FILE_NAME_SIZE];
    CHAR    newFileName[MAX_FILE_NAME_SIZE];

    /* In older raid, we have only 4 volumes max for any type of raid */
    DPRINT(DISK_MANAGER, "legacy raid mdadm config migration start");
    for (raidGrpId = 0; raidGrpId < MAX_RAID_VOLUME_GRP; raidGrpId++)
    {
        /* Prepare mdadm config file path as per old raid */
        snprintf(oldFileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidGrpId);
        if (access(oldFileName, F_OK) != STATUS_OK)
        {
            DPRINT(DISK_MANAGER, "mdadm config not present: [file=%s]", oldFileName);
            continue;
        }

        /* Open mdadm config file for reading */
        pFile = fopen(oldFileName, "r");
        if (pFile == NULL)
        {
            EPRINT(DISK_MANAGER, "fail to open legacy raid mdadm config file: [file=%s], [err=%s]", oldFileName, STR_ERR);
            continue;
        }

        /* Init local variables with default values */
        nodeCnt = fileLen = 0;
        updateRaidConfigF = FALSE;
        memset(fileData, '\0', sizeof(fileData));

        /* Prepare raid array name */
        snprintf(nodeName, NODE_NAME_SIZE, RAID_ARRAY_NAME, raidGrpId);
        raidGrpVol = GET_RAID_VOLUME_ID(raidGrpId, 0);

        /* Read file line by line */
        while(fgets(lineBuff, sizeof(lineBuff), pFile))
        {
            /* Scan file line for raid array details: "ARRAY /dev/md0 metadata=1.2 name=Matrix-NVR:0 UUID=db8d1d31:2da97017:bd605655:e5d8eaa0" */
            if (sscanf(lineBuff, "ARRAY %s %s name=%[^:]:%[^ ] %s", scanData[0], scanData[1], scanData[2], scanData[3], scanData[4]) == 5)
            {
                /* Node name should be matched with expected node */
                if (NULL == strstr(scanData[0], nodeName))
                {
                    continue;
                }

                /* Prepare new raid array name as per latest implementation */
                fileLen = snprintf(fileData, sizeof(fileData), "ARRAY /dev/md%d %s name=%s:%d %s\n",
                                   raidGrpVol, scanData[1], scanData[2], raidGrpVol, scanData[4]);

                /* We have to create new config file */
                updateRaidConfigF = TRUE;
            }
            else if (strstr(lineBuff, "DEVICE"))
            {
                /* Raid array must be found */
                if (FALSE == updateRaidConfigF)
                {
                    EPRINT(DISK_MANAGER, "fail to open mdadm config file: [file=%s]", oldFileName);
                    break;
                }

                /* Update device node name config */
                nodeCnt++;
                fileLen += snprintf(&fileData[fileLen], sizeof(fileData) - fileLen, "%s", lineBuff);
            }
            else if (strstr(lineBuff, RAID_TOTAL_NODE_STR))
            {
                /* If "TotalNodes" found means it is new raid impementation. No need to update config file */
                updateRaidConfigF = FALSE;
                EPRINT(DISK_MANAGER, "new mdadm config file found: [file=%s]", oldFileName);
                break;
            }
        }

        /* Add total device node config */
        fileLen += snprintf(&fileData[fileLen], sizeof(fileData) - fileLen, RAID_TOTAL_NODE_STR "%d\n", nodeCnt);

        /* Close the old file */
        fclose(pFile);

        /* Need to create new mdadm config file? */
        if (FALSE == updateRaidConfigF)
        {
            EPRINT(DISK_MANAGER, "mdadm config file migration skipped: [file=%s]", oldFileName);
            continue;
        }

        /* Prepare new temp mdadm config file to avoid data loss */
        snprintf(tmpFileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE".tmp", raidGrpVol);
        pFile = fopen(tmpFileName, "w+");
        if (pFile == NULL)
        {
            EPRINT(DISK_MANAGER, "fail to open temp mdadm config file: [file=%s], [err=%s]", tmpFileName, STR_ERR);
            continue;
        }

        /* Dump new mdadm config data in temp file */
        if ((INT32)fileLen != fprintf(pFile, "%s", fileData))
        {
            EPRINT(DISK_MANAGER, "fail to dump mdadm config in temp file: [file=%s], [fileLen=%d], [err=%s]", tmpFileName, fileLen, STR_ERR);
            fclose(pFile);
            unlink(tmpFileName);
            continue;
        }

        /* Close the old file */
        fclose(pFile);

        /* Prepare actual file name for new implementation */
        snprintf(newFileName, MAX_FILE_NAME_SIZE, MDADM_CONFIG_FILE, raidGrpVol);

        /* Rename file from temp name to actual name */
        if (rename(tmpFileName, newFileName))
        {
            EPRINT(DISK_MANAGER, "fail to dump mdadm config in temp file: [file=%s], [fileLen=%d], [err=%s]", tmpFileName, fileLen, STR_ERR);
        }

        /* For raid group 0, legacy config and new config file name will be same */
        if (raidGrpId)
        {
            /* Remove legacy raid mdadm config file */
            unlink(oldFileName);
        }

        /* Legacy raid mdadm config file migration done */
        DPRINT(DISK_MANAGER, "legacy raid mdadm config migration done: [legacy=%s], [new=%s]", oldFileName, newFileName);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Create coredup script in storage volume because coredump requires big size for dump data.
 * @param   pHardDiskMountPoint
 */
void CreateCoredumpScript(CHARPTR mountPoint)
{
    FILE    *pFile = NULL;
    INT32   fileLen;
    CHAR    fileData[SYSTEM_COMMAND_SIZE];
    CHAR    fileName[NAME_MAX];

    if((NULL == mountPoint) || (access(mountPoint, F_OK) != STATUS_OK))
    {
        EPRINT(DISK_MANAGER, "hard-disk path is null or does not exist");
        return;
    }

    /* Check if symbolic link is present then check the real path exists or not
     * if real path does exsists then do not create symbolic link and script again */
    struct stat tbuffer;

    /* Note: here we do not use access() and stat() api's as they give information about the file, if symbolic link
     * is given in above API they first dereference it to the real path and give info about the linnked path */
    if(STATUS_OK == lstat(COREDUMP_SCRIPT_SYMLINK, &tbuffer))
    {
        /* Here we are checking the script reffered by soft link exists or not, internally stat() derefernces
         * the symbolic link, Note: here we have not used access() API because In case of network drive when
         * network drive is down then access() API give invalid results */
        if(STATUS_OK == stat(COREDUMP_SCRIPT_SYMLINK, &tbuffer))
        {
            return;
        }

        if(unlink(COREDUMP_SCRIPT_SYMLINK) != STATUS_OK)
        {
            EPRINT(DISK_MANAGER, "fail to delete coredump symblink: [err=%s]", STR_ERR);
        }
    }

    /* Coredump script creation start */
    snprintf(fileName, sizeof(fileName), "%s%s", mountPoint, COREDUMP_SCRIPT_NAME);
    pFile = fopen(fileName, "w+");
    if(NULL == pFile)
    {
        EPRINT(DISK_MANAGER, "fail to open the coredump script: [file=%s], [err=%s]", fileName, STR_ERR);
        return;
    }

    fileLen = snprintf(fileData, sizeof(fileData), "#!/bin/sh\n" COREDUMP_GZIP_GEN_CMD, mountPoint);
    if (fileLen != fprintf(pFile, "%s", fileData))
    {
        EPRINT(DISK_MANAGER, "fail to write in coredump script: [file=%s], [err=%s]", fileName, STR_ERR);
        fclose(pFile);
        unlink(fileName);
        return;
    }

    DPRINT(DISK_MANAGER, "coredump generation cmd info: [file=%s], [data=%s]", fileName, fileData);
    if(STATUS_OK != chmod(fileName, USR_RWE_GRP_RE_OTH_RE))
    {
        EPRINT(DISK_MANAGER, "fail to give permission to script: [err=%s]", STR_ERR);
    }

    /* Coredump script creation end */
    fclose(pFile);

    /* Create symbolic link of coredump script file start */
    if(STATUS_OK != symlink(fileName, COREDUMP_SCRIPT_SYMLINK))
    {
        EPRINT(DISK_MANAGER, "fail to create the soft link path: [link=%s], [err=%s]", fileName, STR_ERR);
    }

    if((STATUS_OK == lstat(COREDUMP_DOWNLOAD_SYMLINK, &tbuffer)) && (unlink(COREDUMP_DOWNLOAD_SYMLINK) != STATUS_OK))
    {
        EPRINT(DISK_MANAGER, "fail to delete the soft link: [link=%s], [err=%s]", COREDUMP_DOWNLOAD_SYMLINK, STR_ERR);
    }

    /* This softlink is used to download coredump from web */
    snprintf(fileName, sizeof(fileName), "%s%s", mountPoint, COREDUMP_FILE_NAME);
    if(STATUS_OK != symlink(fileName, COREDUMP_DOWNLOAD_SYMLINK))
    {
        EPRINT(DISK_MANAGER, "fail to create the soft link to path: [link=%s], [err=%s]", fileName, STR_ERR);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check drive is mounted or not. setmntent API failure case handle with proper care.
 * @param   mountPath
 * @return  Returns SUCCESS if drive mounted, FAIL on failure and REFUSE on setmntent API failure
 */
BOOL isDriveMounted(CHARPTR mountPath)
{
    UINT8           nameLen;
    FILE            *mtab = NULL;
    struct mntent   *part = NULL;
    struct mntent	mntBuf;
    CHAR			buf[MOUNT_FS_SIZE];
    CHAR            nddMountPath[MAX_FILE_NAME_SIZE] = "\0";

    if((NULL == mountPath) || (mountPath[0] == '\0'))
    {
        EPRINT(DISK_MANAGER, "invld mount point is received");
        return FAIL;
    }

    /* Remove '/' from the end of mount path as mnt_dir do not contain '/' at the end of directory name */
    nameLen = snprintf(nddMountPath, MAX_FILE_NAME_SIZE, "%s", mountPath);
    if ((nameLen > 0) && ('/' == nddMountPath[nameLen-1]))
    {
        nddMountPath[nameLen-1] = '\0';
    }

    /* Take appropriate decision at caller's place on setmntent failure */
    mtab = setmntent("/etc/mtab", "r");
    if (mtab == NULL)
    {
        EPRINT(DISK_MANAGER, "setmntent fail: [err=%s]", STR_ERR);
        return REFUSE;
    }

    while (TRUE)
    {
        memset(buf, 0, sizeof(buf));
        memset(&mntBuf, 0, sizeof(struct mntent));
        part = getmntent_r(mtab, &mntBuf, buf, MOUNT_FS_SIZE);
        if (part == NULL)
        {
            break;
        }

        if (part->mnt_fsname == NULL)
        {
            continue;
        }

        if (strcmp(part->mnt_dir, nddMountPath) == 0)
        {
            endmntent(mtab);
            return SUCCESS;
        }
    }

    endmntent(mtab);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is given media volume is normal or not
 * @param   hddMode
 * @param   mediaVol
 * @return  Returns TRUE if media volume is normal else returns FALSE
 */
BOOL IsMediaVolumeNormal(HDD_MODE_e hddMode, UINT8 mediaVol)
{
    RAW_MEDIA_INFO_t *pMediaInfo = GET_MEDIA_VOLUME_INFO(hddMode, mediaVol);
    MUTEX_LOCK(pMediaInfo->rawMediaMutex);
    if ((pMediaInfo->mediaStatus == MOUNTED_READY) &&
            ((pMediaInfo->diskVolStatus == DM_DISK_VOL_NORMAL) || (pMediaInfo->diskVolStatus == DM_DISK_VOL_FULL)))
    {
        /* Media volume is normal */
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
        return TRUE;
    }
    MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

    /* Media volume is not normal */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is any normal storage volume available
 * @param   hddMode
 * @return  Returns TRUE if normal volume available else returns FALSE
 */
BOOL IsNormalStorageVolumeAvailable(HDD_MODE_e hddMode)
{
    UINT8 volCnt;
    UINT8 totalVolume = GetTotalDiskNumber(hddMode);

    /* Check all storage volumes */
    for (volCnt = 0; volCnt < totalVolume; volCnt++)
    {
        /* Is storage volume in normal condition? */
        if (TRUE == IsMediaVolumeNormal(hddMode, volCnt))
        {
            /* Normal storage volume found */
            return TRUE;
        }
    }

    /* Normal volume not found */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get event details based on hdd mode
 * @param   eventType
 * @return  Event details
 */
const CHAR *GetStorageEventDetail(HDD_MODE_e mode, RECORD_ON_DISK_e recordDisk)
{
    UINT8                   eventType = 0;
    static const CHARPTR    storageEventDetail[] = {"", "1", "2", "3", "4", "5", "6", "7"};

    if (recordDisk == LOCAL_HARD_DISK)
    {
        if (mode < MAX_HDD_MODE)
        {
            eventType = mode + 1;
        }
    }
    else if ((recordDisk == REMOTE_NAS1) || (recordDisk == REMOTE_NAS2))
    {
        eventType = MAX_HDD_MODE + recordDisk;
    }

    /* Get event details */
    return storageEventDetail[eventType];
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update storage status
 * @param   mode
 */
static void updateStorageStatus(HDD_MODE_e mode)
{
    UINT8                   volumeId;
    UINT8                   totalVolCnt = GetTotalDiskNumber(mode);
    STORAGE_HEALTH_STATUS_e healthStatus = STRG_HLT_DISK_NORMAL;
    RAW_MEDIA_INFO_t        *pMediaInfo;

    /* After building media (Single Disk or Raid), check all volume status, if any volume is faulty then generate alert */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->diskVolStatus == DM_DISK_VOL_FAULT)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            healthStatus = STRG_HLT_ERROR;
            break;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Set system health status */
    SetDiskHealthStatus(healthStatus);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Handle media disk fault alert. If status is active then start the timer and generate
 *          alert. On timer expiry stop the disk fault alert.
 * @param   status
 */
static void handleDiskFaultAlert(UINT32 status)
{
    TIMER_INFO_t diskFaultInactiveTimerInfo;

    /* Mutex for disk fault inactive timer */
    MUTEX_LOCK(diskFaultInactiveTimerMutex);

    /* Delete timer if running */
    DeleteTimer(&diskFaultInactiveTimerHandle);

    /* If disk fault generated then start the timer to make it inactive */
    if (status == ACTIVE)
    {
        diskFaultInactiveTimerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(60);
        diskFaultInactiveTimerInfo.data = INACTIVE;
        diskFaultInactiveTimerInfo.funcPtr = handleDiskFaultAlert;
        StartTimer(diskFaultInactiveTimerInfo, &diskFaultInactiveTimerHandle);
    }

    /* Mutex for disk fault inactive timer */
    MUTEX_UNLOCK(diskFaultInactiveTimerMutex);

    /* Set disk fault event status */
    DiskFaultEvent(status);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Prepare raid volume name for display
 * @param   mode
 * @param   volumeNum
 * @param   pNameBuff
 */
static void prepareRaidVolumeName(UINT8 mode, UINT8 volumeNum, CHAR *pNameBuff)
{
    UINT8   diskCnt;
    UINT8   raidGrpId = volumeNum / MAX_RAID_VOLUME;
    UINT8   raidGrpVol = (volumeNum % MAX_RAID_VOLUME);

    /* Prepare prefix of raid array volume */
    UINT32  strLen = snprintf(pNameBuff, MAX_VOLUME_NAME_SIZE, "%s %d : Drive", diskVolumePrefixStr[mode], raidGrpVol);

    /* Add disk number in raid array volume name */
    for (diskCnt = 0; diskCnt < raidVolumeInfo[volumeNum].totalHardDisk; diskCnt++)
    {
        strLen += snprintf(&pNameBuff[strLen], MAX_VOLUME_NAME_SIZE - strLen, " %d", (raidGrpId * raidArrayDiskCnt[mode][RAID_DISK_CNT_MAX]) + diskCnt + 1);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set all camera's volume health status
 * @param   healthStatus
 */
void SetAllCameraVolumeHealthStatus(STORAGE_HEALTH_STATUS_e healthStatus)
{
    UINT8 cameraIndex;

    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        storageDiskInfo[cameraIndex].healthStatus = healthStatus;
        MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set camera volume health status if given volume is currently set for the camera
 * @param   volumeId
 * @param   healthStatus
 */
void SetCameraVolumeHealthStatus(UINT8 volumeId, STORAGE_HEALTH_STATUS_e healthStatus)
{
    UINT8 cameraIndex;

    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        if (storageDiskInfo[cameraIndex].currentRecordVolumeId == volumeId)
        {
            storageDiskInfo[cameraIndex].healthStatus = healthStatus;
        }
        MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera volume health status if given volume is currently set for the camera
 * @param   cameraIndex
 * @param   recordDisk
 * @return  camera health status
 */
STORAGE_HEALTH_STATUS_e GetCameraVolumeHealthStatus(UINT8 cameraIndex, RECORD_ON_DISK_e recordDisk)
{
    STORAGE_HEALTH_STATUS_e healthStatus = STRG_HLT_NO_DISK;

    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return STRG_HLT_MAX;
    }

    if (recordDisk == LOCAL_HARD_DISK)
    {
        MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        healthStatus = storageDiskInfo[cameraIndex].healthStatus;
        MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    }
    else if ((recordDisk == REMOTE_NAS1) || (recordDisk == REMOTE_NAS2))
    {
        healthStatus = GetNddHealthStatus(recordDisk - REMOTE_NAS1);
    }

    return healthStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check any storage volume present or not and provide the overall health status of volume
 * @param   mode
 * @param   pHealthStatus
 * @return  Returns FALSE if no volumes available else returns TRUE and set appropriate health status.
 */
static BOOL isAnyStorageVolumePresent(HDD_MODE_e mode, STORAGE_HEALTH_STATUS_e *pHealthStatus)
{
    UINT8               volumeId, totalVolCnt = GetTotalDiskNumber(mode);
    RAW_MEDIA_INFO_t	*pMediaInfo = NULL;

    /* Check all volumes if any volume present or not */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus != NOT_PRESENT)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            break;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Is any volume present? */
    if (volumeId >= totalVolCnt)
    {
        /* No volumes present */
        *pHealthStatus = STRG_HLT_NO_DISK;
        return FALSE;
    }

    /* Check all volumes again if any normal volume present or not */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->diskVolStatus == DM_DISK_VOL_FAULT)
        {
            /* Volumes are present but current volume is faulty */
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            *pHealthStatus = STRG_HLT_ERROR;
            return TRUE;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Volumes are present and operational */
    *pHealthStatus = STRG_HLT_DISK_NORMAL;
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get volume allocation mask of camera's storage group
 * @param   cameraIndex
 * @param   mode
 * @return  Volume allocation mask
 */
UINT32 GetCameraStorageVolumeAllocationMask(UINT8 cameraIndex, HDD_MODE_e mode)
{
    UINT8 volGrpId;

    /* Get camera's storage allocation group */
    volGrpId = GetCameraStorageAllocationGroup(cameraIndex, mode);
    if (volGrpId >= STORAGE_ALLOCATION_GROUP_MAX)
    {
        return 0;
    }

    /* Get volume allocation mask of camera */
    return storageAllocCfg.storageAllocation[volGrpId].volumeAllocationMask;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives total number of normal volume present for camera's storage group.
 * @param   cameraIndex
 * @param   mode
 * @return  Total ready volumes of camera's storage group
 */
UINT8 GetCameraStorageGroupNoramlVolumeCnt(UINT8 cameraIndex, HDD_MODE_e mode)
{
    UINT8               volumeId, volCnt = 0;
    UINT32              volumeAllocationMask;
    UINT8               totalVolCnt = GetTotalDiskNumber(mode);
    RAW_MEDIA_INFO_t	*pMediaInfo = NULL;

    /* Get storage volume mask of camera's storage allocation group */
    volumeAllocationMask = GetCameraStorageVolumeAllocationMask(cameraIndex, mode);
    if (volumeAllocationMask == 0)
    {
        /* No volumes allocated */
        return 0;
    }

    /* Check all volumes to get camera's allocated volume */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        /* Is current volume is part of current group? */
        if (FALSE == GET_BIT(volumeAllocationMask, volumeId))
        {
            /* No. Current volume is not part of current group */
            continue;
        }

        /* Check volume is mounted or not */
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus == MOUNTED_READY)
        {
            /* Volume is ready to use */
            volCnt++;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    return volCnt;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is any normal storage volume available in camera storage group
 * @param   cameraIndex
 * @param   hddMode
 * @return  Returns TRUE if normal volume available in camera storage group else returns FALSE
 */
BOOL IsNormalVolumeAvailableInCameraStorageGroup(UINT8 cameraIndex, HDD_MODE_e mode)
{
    UINT8   volumeId;
    UINT32  volumeAllocationMask;
    UINT8   totalVolCnt = GetTotalDiskNumber(mode);

    /* Get storage volume mask of camera's storage allocation group */
    volumeAllocationMask = GetCameraStorageVolumeAllocationMask(cameraIndex, mode);
    if (volumeAllocationMask == 0)
    {
        /* No volumes allocated */
        return 0;
    }

    /* Check all volumes to get camera's allocated volume */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        /* Is current volume is part of current group? */
        if (FALSE == GET_BIT(volumeAllocationMask, volumeId))
        {
            /* No. Current volume is not part of current group */
            continue;
        }

        /* Is storage volume in normal condition? */
        if (TRUE == IsMediaVolumeNormal(mode, volumeId))
        {
            /* Normal storage volume found */
            return TRUE;
        }
    }

    /* Normal volume not found */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera's storage allocation group
 * @param   cameraIndex
 * @param   mode
 * @return  Storage allocation group for camera
 */
UINT8 GetCameraStorageAllocationGroup(UINT8 cameraIndex, HDD_MODE_e mode)
{
    UINT8   volGrpId;
    UINT32  totalVolumeMask = SET_ALL_BIT(GetTotalDiskNumber(mode));

    /* Validate camera index */
    if (cameraIndex >= getMaxCameraForCurrentVariant())
    {
        return STORAGE_ALLOCATION_GROUP_MAX;
    }

    /* Check all groups whether camera is allocated in valid group or not */
    for (volGrpId = 0; volGrpId < STORAGE_ALLOCATION_GROUP_MAX; volGrpId++)
    {
        /* Is any volume allocated in group? */
        if ((storageAllocCfg.storageAllocation[volGrpId].volumeAllocationMask & totalVolumeMask) == 0)
        {
            /* No volumes allocated in group */
            continue;
        }

        /* Is camera part of current group? */
        if (TRUE == GET_CAMERA_MASK_BIT(storageAllocCfg.storageAllocation[volGrpId].cameraAllocationMask, cameraIndex))
        {
            /* Yes. Camera is part of current group */
            return volGrpId;
        }
    }

    /* No valid storage allocation group found */
    return STORAGE_ALLOCATION_GROUP_MAX;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get valid allocated camera's volume from storage group
 * @param   cameraIndex
 * @param   mode
 * @param   preferVolId
 * @return  Last allocated volume if it is in valid state or other valid volume else invalid volume
 */
static UINT8 getNormalAllocatedCameraVolume(UINT8 cameraIndex, HDD_MODE_e mode, UINT8 preferVolId)
{
    UINT8               volGrpId, volumeId;
    UINT8               totalVolCnt = GetTotalDiskNumber(mode);
    RAW_MEDIA_INFO_t	*pMediaInfo = NULL;

    /* Get camera's allocated storage group */
    volGrpId = GetCameraStorageAllocationGroup(cameraIndex, mode);
    if (volGrpId >= STORAGE_ALLOCATION_GROUP_MAX)
    {
        /* There is no storage group for camera */
        return MAX_VOLUME;
    }

    /* Check preferred volume id is valid not and also check whether it is available or not. */
    if ((preferVolId < totalVolCnt) && (TRUE == GET_BIT(storageAllocCfg.storageAllocation[volGrpId].volumeAllocationMask, preferVolId)))
    {
        /* If preferred volume is ready then provide that volume only */
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, preferVolId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus == MOUNTED_READY)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            return preferVolId;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Check all volumes to get valid available volume */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        /* Is current volume is part of current group? */
        if (FALSE == GET_BIT(storageAllocCfg.storageAllocation[volGrpId].volumeAllocationMask, volumeId))
        {
            /* No. Current volume is not part of current group */
            continue;
        }

        /* Check this volume is ready or not */
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus != MOUNTED_READY)
        {
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            continue;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);

        /* Provide current volume */
        return volumeId;
    }

    /* Valid volume not found */
    return MAX_VOLUME;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check whether storage volume group present or not
 * @param   volGrpId
 * @param   mode
 * @return  Returns TRUE is storage volume present of the group else returns FALSE
 */
static BOOL isStorageGroupVolumePresent(UINT8 volGrpId, HDD_MODE_e mode)
{
    UINT8               volumeId;
    UINT8               totalVolCnt = GetTotalDiskNumber(mode);
    RAW_MEDIA_INFO_t	*pMediaInfo = NULL;

    /* Validate storage allocation group */
    if (volGrpId >= STORAGE_ALLOCATION_GROUP_MAX)
    {
        return FALSE;
    }

    /* Check all volumes to get valid available volume */
    for (volumeId = 0; volumeId < totalVolCnt; volumeId++)
    {
        /* Is current volume is part of current group? */
        if (FALSE == GET_BIT(storageAllocCfg.storageAllocation[volGrpId].volumeAllocationMask, volumeId))
        {
            /* No. Current volume is not part of current group */
            continue;
        }

        /* Check this volume is present or not */
        pMediaInfo = GET_MEDIA_VOLUME_INFO(mode, volumeId);
        MUTEX_LOCK(pMediaInfo->rawMediaMutex);
        if(pMediaInfo->mediaStatus != NOT_PRESENT)
        {
            /* Volume is present */
            MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
            return TRUE;
        }
        MUTEX_UNLOCK(pMediaInfo->rawMediaMutex);
    }

    /* Valid volume not found */
    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set camera storage volume if camera is part of current volume's group and camera volume
 *          status id not normal
 * @param   volumeId
 * @param   mode
 * @param   pMountPoint
 */
static void setCameraStorageVolume(UINT8 volumeId, HDD_MODE_e mode, CHAR *pMountPoint)
{
    UINT8 cameraIndex, volGrpId;

    /* Check all volumes to get valid available volume */
    for (volGrpId = 0; volGrpId < STORAGE_ALLOCATION_GROUP_MAX; volGrpId++)
    {
        /* Is current volume is part of current group? */
        if (TRUE == GET_BIT(storageAllocCfg.storageAllocation[volGrpId].volumeAllocationMask, volumeId))
        {
            /* Yes. Current volume is part of current group */
            break;
        }
    }

    /* Current volume is not part of any storage group */
    if (volGrpId >= STORAGE_ALLOCATION_GROUP_MAX)
    {
        return;
    }

    /* Check for all camera's volume */
    for (cameraIndex = 0; cameraIndex < getMaxCameraForCurrentVariant(); cameraIndex++)
    {
        /* Current camera is not part of current volume's storage group */
        if (volGrpId != GetCameraStorageAllocationGroup(cameraIndex, mode))
        {
            continue;
        }

        /* If current volume set in camera recording or currently set volume status is not normal then use this volume */
        MUTEX_LOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
        if ((storageDiskInfo[cameraIndex].currentRecordVolumeId == volumeId) || (storageDiskInfo[cameraIndex].healthStatus != STRG_HLT_DISK_NORMAL))
        {
            /* If volume status is not normal then also update current volume in the file */
            if (storageDiskInfo[cameraIndex].healthStatus != STRG_HLT_DISK_NORMAL)
            {
                updateRecordDiskInfoFileForCamera(cameraIndex, volumeId, mode);
            }

            /* Update mount point, volume and status */
            snprintf(storageDiskInfo[cameraIndex].mountPoint, MOUNT_POINT_SIZE, "%s", pMountPoint);
            storageDiskInfo[cameraIndex].currentRecordVolumeId = volumeId;
            storageDiskInfo[cameraIndex].healthStatus = STRG_HLT_DISK_NORMAL;
        }
        MUTEX_UNLOCK(storageDiskInfo[cameraIndex].storageDiskMutex);
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
