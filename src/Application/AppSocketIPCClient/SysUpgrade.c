// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		SystemUpgrade.c
@brief      File containing the firmware and config upgrade related logic
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
/* OS Includes */
#include <linux/reboot.h>
#include <openssl/md5.h>
#include <sys/reboot.h>
#include <syslog.h>

/* Application Includes */
#include "DeviceDefine.h"

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define FU_ERROR_LEVEL           (LOG_LOCAL0 | LOG_ERR)
#define FU_DEBUG_LEVEL           (LOG_LOCAL0 | LOG_DEBUG)

#define PRE_UPGRADE_SCRIPT_PATH  SKIP_STRING_PREFIX(SCRIPTS_DIR_PATH "/preUpgrade.sh", 1)
#define POST_UPGRADE_SCRIPT_PATH SKIP_STRING_PREFIX(SCRIPTS_DIR_PATH "/postUpgrade.sh", 1)
#define FS_BASE_PATH             "/"
#define TMP_ZIP_FILE             TEMP_DIR_PATH "File.zip"
#define FILE_BLOCK_SIZE          100
#define SIZE_OF_MD5_CHECKSUM     16
#define FW_FILE_READ_END_OFFSET  (SIZE_OF_MD5_CHECKSUM + sizeof(UINT32))

#define POST_UPGRADE_SCRIPT      "/tmp/postUpgrade.sh"
#define PRE_UPGRADE_SCRIPT       "/tmp/preUpgrade.sh"

#if defined(HI3536_NVRH) || defined(HI3536_NVRL)
#define KERNEL_MTD                "/dev/mtd2 "
#define ERASE_BLOCKS              "0 40"  // 0 start eraseblock and 40 end eraseblock
#define UPGRADE_KERNEL_NAME       KERNEL_DIR_PATH "/hi3536_nvr_uImage"
#define UPGRADE_NAND_WRITE_KERNEL "nandwrite -p " KERNEL_MTD UPGRADE_KERNEL_NAME
#define FLASH_ERASE_KERNEL        "flash_erase " KERNEL_MTD
#define KERNEL_WRITE_COMMAND      FLASH_ERASE_KERNEL ERASE_BLOCKS "&& " UPGRADE_NAND_WRITE_KERNEL "&& rm " UPGRADE_KERNEL_NAME
#elif defined(RK3568_NVRL)
#define UPGRADE_KERNEL_NAME  KERNEL_DIR_PATH "/rk3568_nvr_uImage"
#define KERNEL_WRITE_COMMAND "dd if=" UPGRADE_KERNEL_NAME " of=/dev/mmcblk0p4 && rm " UPGRADE_KERNEL_NAME
#elif defined(RK3588_NVRH)
#define UPGRADE_KERNEL_NAME  KERNEL_DIR_PATH "/rk3588_nvr_uImage"
#define KERNEL_WRITE_COMMAND "dd if=" UPGRADE_KERNEL_NAME " of=/dev/mmcblk0p3 && rm " UPGRADE_KERNEL_NAME
#endif

#define KERNEL_VER_READ_LEN 2
#define FS_KERNEL_VER_FILE  "/etc/kernel_ver"
#define PKG_KERNEL_VER_FILE KERNEL_DIR_PATH "/kernel_ver"

// #################################################################################################
//  @STATIC VARIABLES
// #################################################################################################
/** @note: If any change in this value then it must be reflected in makefile also */
static const UINT32 firmwareSignData = 0x11223344;

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
static void firmwareUpgrade(void);
//-------------------------------------------------------------------------------------------------
static void configUpgrade(void);
//-------------------------------------------------------------------------------------------------
static BOOL verifyFirmwareFile(void);
//-------------------------------------------------------------------------------------------------
static BOOL extractFirmwareFile(CHARPTR fileName, BOOL *pNeedRebootF);
//-------------------------------------------------------------------------------------------------
static BOOL extractConfigFile(CHARPTR fileName);
//-------------------------------------------------------------------------------------------------
static BOOL copyFile(CHARPTR sourceFile, CHARPTR destFile);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @FUNCTIONS
// #################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Firmware upgrade binary process main function
 * @return  Process exit status
 */
INT32 main(void)
{
    /* Check and upgrade firmware if available */
    firmwareUpgrade();

    /* Check and restore config if available */
    configUpgrade();

    /* Exit from application */
    return EXIT_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and upgrade firmware if available
 */
static void firmwareUpgrade(void)
{
    BOOL rebootDevice = FALSE;

    // check if Firmware Upgrade File exists
    if (access(FIRMWARE_FILE, F_OK) != STATUS_OK)
    {
        /* Firmware upgrade file not present */
        return;
    }

    // verify our firmware file with MD5 mechanism
    if (verifyFirmwareFile() == FAIL)
    {
        /* Firmware upgrade file verification fail */
        syslog(FU_ERROR_LEVEL, "firmware verification fail");
        unlink(FIRMWARE_FILE);
        return;
    }

    if (copyFile(FIRMWARE_FILE, TMP_ZIP_FILE) == FAIL)
    {
        /* Firmware upgrade file copy fail in tempfs */
        syslog(FU_ERROR_LEVEL, "fail to copy firmware in tempfs");
        unlink(FIRMWARE_FILE);
        unlink(TMP_ZIP_FILE);
        return;
    }

    // extract it to file-System
    if (extractFirmwareFile(TMP_ZIP_FILE, &rebootDevice) == FAIL)
    {
        /* Fail to extract firmware in rootfs */
        syslog(FU_ERROR_LEVEL, "fail to extract firmware in filesystem");
        unlink(FIRMWARE_FILE);
        unlink(TMP_ZIP_FILE);
        return;
    }

    // remove firmware file
    unlink(FIRMWARE_FILE);
    unlink(TMP_ZIP_FILE);
    unlink(FIRMWARE_UPGRADE_CONFIRMATION_FILE);
    sync();

    /*
     * Added unconditional reboot which is required in web upgrade. Currently reboot is done only if there is ungrade of kernel
     * But if there are any filesystem changes like nvr-x.sh, system is not rebooting.
     * Issue: If we have added some logic in nvr-x.sh after running sysUpgrade.bin, All changes are not executed after upgrading firmware.
     * Root Cause: Currently sysUpgrade.bin is run from nvr-x.sh. Assume nvr-x.sh is devided in three parts.
     * [Part-1] [Run sysUpgrade.bin for firmware upgrade]
     * [Part-2] Now assume nvr-x.sh is itself upgraded in firmware upgrade. But as we have not done reboot here. Part-2 of script will
     * be executed of older nvr-x.sh which should not be the case. If nvr-x.sh is upgraded, system should exit from old script after
     * upgrading firmware and should start new nvr-x.sh script OR reboot the system.
     * issue happens as older script was already loaded into RAM memory. So older script will get executed though it is updated in FLASH memory.
     * To handle these type of cases easily. we will reboot unconditionally. */
    // if (rebootDevice == TRUE)
    {
        syslog(FU_DEBUG_LEVEL, "firmware upgraded. system is going to reboot now..!!!");
        sleep(5);
        sync();
        reboot(LINUX_REBOOT_CMD_RESTART);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check and restore config if available
 */
static void configUpgrade(void)
{
    // check if Configuration File exists
    if (access(CONFIG_ZIP_FILE, F_OK) != STATUS_OK)
    {
        /* Config restore file not present */
        return;
    }

    if (copyFile(CONFIG_ZIP_FILE, TMP_ZIP_FILE) == FAIL)
    {
        /* Config restore file copy fail in tempfs */
        syslog(FU_ERROR_LEVEL, "fail to copy config file in tempfs");
        unlink(CONFIG_ZIP_FILE);
        unlink(TMP_ZIP_FILE);
        return;
    }

    // extract it to file-System
    if (extractConfigFile(TMP_ZIP_FILE) == FAIL)
    {
        /* Fail to extract config restore file */
        syslog(FU_ERROR_LEVEL, "fail to extract config in filesystem");
    }
    else
    {
        syslog(FU_DEBUG_LEVEL, "config restored successfully");
    }

    unlink(CONFIG_ZIP_FILE);
    unlink(TMP_ZIP_FILE);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function verifies the firmware file MD5 checksum and firmware sign data
 * @return  SUCCESS/FAIL
 */
static BOOL verifyFirmwareFile(void)
{
    INT32       fd;
    UINT8       srcMd5sum[SIZE_OF_MD5_CHECKSUM];
    UINT8       destMd5sum[SIZE_OF_MD5_CHECKSUM];
    BOOL        retVal = FAIL;
    UINT8       buffer[FILE_BLOCK_SIZE];
    UINT32      fileSize;
    UINT32      bytesProcessed = 0;
    UINT32      signData;
    MD5_CTX     md5Context;
    struct stat infoBuf = {0};
    UINT16      readSize = FILE_BLOCK_SIZE;

    // get file information
    if (stat(FIRMWARE_FILE, &infoBuf) != STATUS_OK)
    {
        syslog(FU_ERROR_LEVEL, "fail to get firmware file size for validation");
        return FAIL;
    }

    // open in read only mode
    fd = open(FIRMWARE_FILE, READ_SYNC_MODE);
    if (fd == INVALID_FILE_FD)
    {
        syslog(FU_ERROR_LEVEL, "fail to open firmware file for validation: [err=%s]", STR_ERR);
        return FAIL;
    }

    fileSize = (UINT32)infoBuf.st_size;

    // Initialise MD5 context
    MD5_Init(&md5Context);

    // Read whole file except last FW_FILE_READ_END_OFFSET Bytes, because these are the MD5 sum + RDK Upgrade Flag appended while creating NVR
    // Firmware file
    while (TRUE)
    {
        if (read(fd, buffer, readSize) < (INT16)readSize)
        {
            break;
        }

        bytesProcessed += readSize;
        MD5_Update(&md5Context, buffer, readSize);

        // Check If whole file has been read or not
        if ((INT32)((INT32)fileSize - (INT32)bytesProcessed - FW_FILE_READ_END_OFFSET) <= 0)
        {
            retVal = SUCCESS;
            break;
        }

        // check if sufficient bytes remaining to read, if exists read FILE_BLOCK_SIZE bytes
        if ((INT32)((INT32)fileSize - (INT32)bytesProcessed - FILE_BLOCK_SIZE - FW_FILE_READ_END_OFFSET) > 0)
        {
            readSize = FILE_BLOCK_SIZE;
        }
        // read only remaining bytes
        else
        {
            readSize = fileSize - bytesProcessed - FW_FILE_READ_END_OFFSET;
        }
    }

    // check if whole file has been read successfully till MD5 sum
    if (retVal == SUCCESS)
    {
        // get final MD5
        MD5_Final(srcMd5sum, &md5Context);

        // read Appended MD5 sum
        if (read(fd, destMd5sum, SIZE_OF_MD5_CHECKSUM) < SIZE_OF_MD5_CHECKSUM)
        {
            syslog(FU_ERROR_LEVEL, "fail to read firmware file checksum: [err=%s]", STR_ERR);
            retVal = FAIL;
        }
        // compare it with computed MD5 sum
        else if (memcmp(srcMd5sum, destMd5sum, SIZE_OF_MD5_CHECKSUM) != STATUS_OK)
        {
            syslog(FU_ERROR_LEVEL, "firmware file checksum mismatched");
            retVal = FAIL;
        }
    }

    if (retVal == SUCCESS)
    {
        // Read Appended Upgrade Type Data Flag
        if (read(fd, &signData, sizeof(UINT32)) != sizeof(UINT32))
        {
            syslog(FU_ERROR_LEVEL, "fail to read firmware sign data: [err=%s]", STR_ERR);
            retVal = FAIL;
        }
        /* It should firmware upgrade sign value */
        else if (signData != firmwareSignData)
        {
            syslog(FU_ERROR_LEVEL, "firmware file sign mismatched: [sign=0x%x]", signData);
            retVal = FAIL;
        }
    }

    // Close file Fd
    close(fd);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will verify kernel version for upgrade. It will cross check system kernel
 *          version and available kernel version from package.
 * @return  Returns TRUE if higher kernel version available in package else FALSE
 */
static BOOL isKernelUpgradeAvailable(void)
{
    INT32  fd;
    INT32  readCnt;
    CHAR   kernelVerStr[KERNEL_VER_READ_LEN + 1];
    UINT32 oldKernelVer = 0, newKernelVer = 0;

    if (access(FS_KERNEL_VER_FILE, F_OK) == STATUS_OK)
    {
        fd = open(FS_KERNEL_VER_FILE, (O_RDONLY | O_SYNC));
        if (fd != INVALID_FILE_FD)
        {
            readCnt = read(fd, kernelVerStr, KERNEL_VER_READ_LEN);
            if (readCnt > 0)
            {
                kernelVerStr[readCnt] = '\0';
                oldKernelVer = atoi(kernelVerStr);
            }
            close(fd);
        }
    }

    syslog(FU_DEBUG_LEVEL, "current kernel version: [version=%d]", oldKernelVer);
    if (access(PKG_KERNEL_VER_FILE, F_OK) == STATUS_OK)
    {
        fd = open(PKG_KERNEL_VER_FILE, (O_RDONLY | O_SYNC));
        if (fd != INVALID_FILE_FD)
        {
            readCnt = read(fd, kernelVerStr, KERNEL_VER_READ_LEN);
            if (readCnt > 0)
            {
                kernelVerStr[readCnt] = '\0';
                newKernelVer = atoi(kernelVerStr);
            }
            close(fd);
        }
    }

    syslog(FU_DEBUG_LEVEL, "available kernel version: [version=%d]", newKernelVer);
    return (newKernelVer > oldKernelVer) ? TRUE : FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function extracts firmware file in rootfs
 * @param   fileName
 * @param   pNeedRebootF
 * @return  SUCCESS/FAIL
 */
static BOOL extractFirmwareFile(CHARPTR fileName, BOOL *pNeedRebootF)
{
    CHAR        sysCmd[512];
    struct stat infoBuf = {0};

    // get file information
    if (stat(fileName, &infoBuf) != STATUS_OK)
    {
        syslog(FU_ERROR_LEVEL, "fail to get firmware file size for extract");
        return FAIL;
    }

    // truncate file to remove appended MD5 sum
    if (truncate(fileName, (infoBuf.st_size - FW_FILE_READ_END_OFFSET)) != STATUS_OK)
    {
        syslog(FU_ERROR_LEVEL, "fail to truncate firmware file for extract");
        return FAIL;
    }

    /* unzip pre upgrade script */
    snprintf(sysCmd, sizeof(sysCmd), "unzip -p %s %s > %s", fileName, PRE_UPGRADE_SCRIPT_PATH, PRE_UPGRADE_SCRIPT);
    system(sysCmd);

    /* Check pre upgrade script present or not */
    if (access(PRE_UPGRADE_SCRIPT, F_OK) == STATUS_OK)
    {
        /* Execute Pre Upgrade script */
        if (system("sh " PRE_UPGRADE_SCRIPT) != STATUS_OK)
        {
            syslog(FU_ERROR_LEVEL, "fail to execute pre-upgrade script");
        }
    }

    /* give a sync to system */
    sync();

    // unzip config file to our file-system
    snprintf(sysCmd, sizeof(sysCmd), "unzip -o %s -d %s", fileName, FS_BASE_PATH);
    if (system(sysCmd) != STATUS_OK)
    {
        syslog(FU_ERROR_LEVEL, "fail to extract firmware file in rootfs");
        return FAIL;
    }

    // give a sync to system
    sync();
    syslog(FU_DEBUG_LEVEL, "firmware upgraded successfully");
    sleep(1);

    // get file information
    if (stat(UPGRADE_KERNEL_NAME, &infoBuf) == STATUS_OK)
    {
        if (TRUE == isKernelUpgradeAvailable())
        {
            syslog(FU_DEBUG_LEVEL, "kernel upgrade started");
            if (system(KERNEL_WRITE_COMMAND) < 0)
            {
                syslog(FU_ERROR_LEVEL, "fail to upgrade kernel");
                return FAIL;
            }

            syslog(FU_DEBUG_LEVEL, "updating kernel version file");
            if (rename(PKG_KERNEL_VER_FILE, FS_KERNEL_VER_FILE) != STATUS_OK)
            {
                syslog(FU_ERROR_LEVEL, "fail to move kernel version file: [srcPath=%s], [dstPath=%s]", PKG_KERNEL_VER_FILE, FS_KERNEL_VER_FILE);
            }

            *pNeedRebootF = TRUE;
            syslog(FU_DEBUG_LEVEL, "kernel upgraded successfully");
        }
        else
        {
            syslog(FU_DEBUG_LEVEL, "system already upgraded with latest kernel");
        }
    }

    sleep(1);

    /* unzip post upgrade script */
    snprintf(sysCmd, sizeof(sysCmd), "unzip -p %s %s > %s", fileName, POST_UPGRADE_SCRIPT_PATH, POST_UPGRADE_SCRIPT);
    system(sysCmd);

    /* Check post upgrade script present or not */
    if (access(POST_UPGRADE_SCRIPT, F_OK) == STATUS_OK)
    {
        /* Execute post upgrade script */
        if (system("sh " POST_UPGRADE_SCRIPT) != STATUS_OK)
        {
            syslog(FU_ERROR_LEVEL, "fail to execute post-upgrade script");
        }
    }

    /* give a sync to system */
    sync();
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function extracts config files
 * @param   fileName
 * @return  SUCCESS/FAIL
 */
static BOOL extractConfigFile(CHARPTR fileName)
{
    CHAR sysCmd[50];

    // unzip config file to our file-system
    snprintf(sysCmd, sizeof(sysCmd), "unzip -o %s -d %s", fileName, FS_BASE_PATH);
    if (system(sysCmd) != STATUS_OK)
    {
        syslog(FU_ERROR_LEVEL, "fail to extract config file");
        return FAIL;
    }

    // give a sync to system
    sync();
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function copy source file to destination file using manual FILE I/O function.
 * @param   sourceFile
 * @param   destFile
 * @return  SUCCESS/FAIL
 */
static BOOL copyFile(CHARPTR sourceFile, CHARPTR destFile)
{
    BOOL        retVal = SUCCESS;
    CHAR        buff[KILO_BYTE];
    INT32       sourceFileFd;
    INT32       destFileFd;
    INT32       rdWrSize = 0;
    UINT32      chunkRdWrSize;
    INT64       copySize = 0;
    struct stat statInfo;

    if (stat(sourceFile, &statInfo) < STATUS_OK)
    {
        syslog(FU_ERROR_LEVEL, "fail to get file size for copy");
        return FAIL;
    }

    sourceFileFd = open(sourceFile, READ_WRITE_SYNC_MODE, USR_RWE_GRP_RE_OTH_RE);
    if (sourceFileFd == INVALID_FILE_FD)
    {
        syslog(FU_ERROR_LEVEL, "fail to open source file for copy: [file=%s], [err=%s]", sourceFile, STR_ERR);
        return FAIL;
    }

    destFileFd = open(destFile, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if (destFileFd == INVALID_FILE_FD)
    {
        syslog(FU_ERROR_LEVEL, "fail to open dest file for copy: [file=%s], [err=%s]", destFile, STR_ERR);
        close(sourceFileFd);
        return FAIL;
    }

    while (copySize < statInfo.st_size)
    {
        if ((statInfo.st_size - copySize) >= KILO_BYTE)
        {
            chunkRdWrSize = KILO_BYTE;
        }
        else
        {
            chunkRdWrSize = (statInfo.st_size - copySize);
        }

        rdWrSize = read(sourceFileFd, buff, chunkRdWrSize);
        if (rdWrSize != (INT32)chunkRdWrSize)
        {
            syslog(FU_ERROR_LEVEL, "fail to read source file for copy: [file=%s], [err=%s]", sourceFile, STR_ERR);
            retVal = FAIL;
            break;
        }

        rdWrSize = write(destFileFd, buff, chunkRdWrSize);
        if (rdWrSize != (INT32)chunkRdWrSize)
        {
            syslog(FU_ERROR_LEVEL, "fail to write dest file for copy: [file=%s], [err=%s]", destFile, STR_ERR);
            retVal = FAIL;
            break;
        }

        copySize += chunkRdWrSize;
    }

    close(sourceFileFd);
    close(destFileFd);
    return retVal;
}

// #################################################################################################
//  @END OF FILE
// #################################################################################################
