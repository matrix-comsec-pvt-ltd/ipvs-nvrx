//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   SystemUpgrade.c
@brief  System upgrade module provides API to check for any upgrades available within the NVR system
        and if so it notifies all the modules and start the up gradation. When the system upgradation
        is completed, it reboots the system.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <openssl/md5.h>

/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "SystemUpgrade.h"
#include "InputOutput.h"
#include "CsvParser.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define FILE_BLOCK_SIZE				100
#define SIZE_OF_MD5_CHECKSUM		16
#define FW_FILE_READ_END_OFFSET		(SIZE_OF_MD5_CHECKSUM + sizeof(UINT32))

// When firmware upgrade is failed, turn on buzzer cadence for this particular time
#define FW_UPGRADE_FAIL_BUZ_TIME	60
#define SYS_UPGRADE_THREAD_STACK_SZ (3*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	CHAR fileName[MAX_FW_FILE_NAME_LEN + 1 + sizeof(RAMFS_DIR_PATH)];

}FIRMWARE_UPGRADE_INFO_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BOOL 			currSysUpgradeState = FREE;
static pthread_mutex_t	upgradeStateMutex = PTHREAD_MUTEX_INITIALIZER;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL verifyFirmwareFile(CHARPTR fileName);
//-------------------------------------------------------------------------------------------------
static VOIDPTR sysUpgradeThrd(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function initializes the System upgrade. The caller has to input the pointer of the
 *          callback function (which sends response of System Upgrade) and clientFd.This function searches
 *          for the system upgrade file and creates a thread after the firmware file search is successful.
 *          The thread initializes the System upgrade process.
 * @param   fileName
 * @return
 */
NET_CMD_STATUS_e StartSysUpgrade(CHARPTR fileName)
{
    NET_CMD_STATUS_e 		retVal = CMD_PROCESS_ERROR;
	FIRMWARE_UPGRADE_INFO_t	*currFirmwareInfo = NULL;

    DPRINT(SYSTEM_UPGRADE, "start firmware upgrade process: [path=%s]", fileName);
    MUTEX_LOCK(upgradeStateMutex);
    if (currSysUpgradeState == BUSY)
	{
        MUTEX_UNLOCK(upgradeStateMutex);
        return CMD_UPGRADE_IN_PROCESS;
	}
    currSysUpgradeState = BUSY;
    MUTEX_UNLOCK(upgradeStateMutex);

    do
    {
        // allocate memory for storing this session related information
        currFirmwareInfo = (FIRMWARE_UPGRADE_INFO_t	*)malloc(sizeof(FIRMWARE_UPGRADE_INFO_t));
        // PARASOFT : Release of memory for "currFirmwareInfo" has been taken care in "sysUpgradeThrd" thread below
        if (currFirmwareInfo == NULL)
        {
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        // store callback func and connFd index to the firmware info struct.
        snprintf(currFirmwareInfo->fileName, sizeof(currFirmwareInfo->fileName), RAMFS_DIR_PATH"%s", fileName);

        // check if file exists
        if (access(currFirmwareInfo->fileName, F_OK) != STATUS_OK)
        {
            EPRINT(SYSTEM_UPGRADE, "firmware file does not exist: [path=%s]", currFirmwareInfo->fileName);
            retVal = CMD_FILE_MISSING;
            break;
        }

        // create the detached thread to start further firmware upgrade process
        if (FAIL == Utils_CreateThread(NULL, sysUpgradeThrd, currFirmwareInfo, DETACHED_THREAD, SYS_UPGRADE_THREAD_STACK_SZ))
        {
            EPRINT(SYSTEM_UPGRADE, "fail to create firmware upgrade thread");
            retVal = CMD_RESOURCE_LIMIT;
            break;
        }

        /* Firmware upgrade process started successfully */
        return CMD_SUCCESS;

    }while(0);

    // free session info if allocated previously
    FREE_MEMORY(currFirmwareInfo);

    // If we set the status to BUSY make it FREE
    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(upgradeStateMutex);
    currSysUpgradeState = FREE;
    MUTEX_UNLOCK(upgradeStateMutex);

    //Deallocate memory in thread function
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function verifies the firmware file using MD5 checksum.
 * @param   fileName
 * @return
 */
static BOOL verifyFirmwareFile(CHARPTR fileName)
{
	INT32		fd;
	UINT8		srcMd5sum[SIZE_OF_MD5_CHECKSUM];
	UINT8		destMd5sum[SIZE_OF_MD5_CHECKSUM];
	BOOL		retVal = FAIL;
	UINT8PTR	buffer;
	UINT32		fileSize;
	UINT32		bytesProcessed = 0;
	MD5_CTX		md5Context;
    struct stat infoBuf = {0};
	UINT16 		readSize = FILE_BLOCK_SIZE;

	// get File Size
	if (stat(fileName, &infoBuf) != STATUS_OK)
	{
        EPRINT(SYSTEM_UPGRADE, "file size not found: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
	}

	// open file for reading
    fd = open(fileName, READ_SYNC_MODE);
    if (fd == INVALID_FILE_FD)
	{
        EPRINT(SYSTEM_UPGRADE, "fail to open file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
	}

	// allocate buffer for file reading
    buffer = (UINT8PTR)malloc(FILE_BLOCK_SIZE);
    if (buffer == NULL)
	{
		close(fd);
        EPRINT(SYSTEM_UPGRADE, "fail to alloc memory: [path=%s]", fileName);
        return FAIL;
	}

    fileSize = (UINT32)infoBuf.st_size;
    MD5_Init(&md5Context);

    /* Read whole file except last FW_FILE_READ_END_OFFSET Bytes, because these are
     * the MD5 sum + RDK Upgrade Flag appended while creating NVR Firmware file */
    while (TRUE)
    {
        if (read(fd, (VOIDPTR)buffer, readSize) < (INT16)readSize)
        {
            EPRINT(SYSTEM_UPGRADE, "fail to read file: [path=%s], [err=%s]", fileName, STR_ERR);
            break;
        }

        bytesProcessed += readSize;
        MD5_Update(&md5Context, (VOIDPTR)buffer, readSize);

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
        if (read(fd, (VOIDPTR)destMd5sum, SIZE_OF_MD5_CHECKSUM) < SIZE_OF_MD5_CHECKSUM)
        {
            EPRINT(SYSTEM_UPGRADE, "fail to read md5sum from file: [path=%s], [err=%s]", fileName, STR_ERR);
            retVal = FAIL;
        }
        // compare two MD5 checksum
        else if (memcmp(srcMd5sum, destMd5sum, SIZE_OF_MD5_CHECKSUM) != STATUS_OK)
        {
            EPRINT(SYSTEM_UPGRADE, "invld firmware file due to md5sum mismatch: [path=%s]", fileName);
            retVal = FAIL;
        }
    }

    // Free Allocated memory for file reading
    free(buffer);

    // Close file Fd
    close(fd);

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This thread which  it starts auto up-gradation process of the system. It than invokes
 *          callback function and sending CMD_SUCCESS or EXTRACTION_FAIL when up-gradation process
 *          succeeds or fails respectively.
 * @param   arg
 * @return
 */
static VOIDPTR sysUpgradeThrd(VOIDPTR arg)
{
	FIRMWARE_UPGRADE_INFO_t *thCurrFirmwareInfo = (FIRMWARE_UPGRADE_INFO_t *)arg;
    NET_CMD_STATUS_e		retVal = CMD_PROCESS_ERROR;

	WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_START, NULL, NULL, EVENT_ALERT);
    DPRINT(SYSTEM_UPGRADE, "verify firmware file: [path=%s]", thCurrFirmwareInfo->fileName);

	// verify Firmware file by MD5 mechanism
	if (verifyFirmwareFile(thCurrFirmwareInfo->fileName) == SUCCESS)
	{
        if(CopyFile(thCurrFirmwareInfo->fileName, FIRMWARE_FILE, 10, NULL, 0) == FAIL)
		{
            /* Remove both firmware files */
            EPRINT(SYSTEM_UPGRADE, "fail to move firmware file");
            unlink(FIRMWARE_FILE);
            unlink(thCurrFirmwareInfo->fileName);
		}
		else
		{
            DPRINT(SYSTEM_UPGRADE, "restart system for firmware upgrade");
            NotifyLanguageUpdate();

			// All done, now Prepare system for a safe reboot
			if (PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, "UPGRADE") == SUCCESS)
			{
                retVal = CMD_SUCCESS;
			}
		}
	}
	else
	{
		// remove firmware file
		unlink(thCurrFirmwareInfo->fileName);
	}

    if (retVal != CMD_SUCCESS)
	{
		WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_RESULT, NULL, NULL, EVENT_FAIL);

		// change LED State to indicate failure of FIRMWARE UPGRADE
		SetSystemStatusLed(SYS_UPGRADE_FAILED, ON);

		// Wait For some time
		sleep(FW_UPGRADE_FAIL_BUZ_TIME);

		SetSystemStatusLed(SYS_UPGRADE_FAILED, OFF);
		SetSystemStatusLed(SYS_FIRMWARE_UPGRADE_INPROCESS, OFF);

        MUTEX_LOCK(upgradeStateMutex);
		currSysUpgradeState = FREE;
        MUTEX_UNLOCK(upgradeStateMutex);
	}
	else
	{
		WriteEvent(LOG_OTHER_EVENT, LOG_UPGRADE_RESULT, NULL, NULL, EVENT_SUCCESS);
	}

	// Free Session
	free(thCurrFirmwareInfo);
	pthread_exit(NULL);
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
