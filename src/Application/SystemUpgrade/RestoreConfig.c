//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   RestoreConfig.c
@brief  This module provides API to restore configurations. Before starting process it log outs all
        other users and after and reboots NVR after successful restore.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "RestoreConfig.h"
#include "EventLogger.h"
#include "Config.h"
#include "CsvParser.h"
#include "DhcpServer.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define RESTORE_CONFIG_THREAD_STACK_SZ  (2*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    CHAR    fileName[MAX_RESTORE_FILE_NAME_LEN + sizeof(RAMFS_DIR_PATH)];

}RESTORE_CONFIG_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BOOL 			currRestoreCnfgState = FREE;
static pthread_mutex_t	restoreStateMutex = PTHREAD_MUTEX_INITIALIZER;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static VOIDPTR restoreConfigThrd(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static void procBeforeConfigRestore(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API process restore configuration request and upon valid request creates a Thread
 *          that will further handle this process
 * @param   fileName
 * @return
 */
NET_CMD_STATUS_e StartRestoreConfig(CHARPTR fileName)
{
    NET_CMD_STATUS_e 	retVal;
	RESTORE_CONFIG_t	*restoreCnfgParam = NULL;

	// Get Current System Restore Config Status
    MUTEX_LOCK(restoreStateMutex);
    if (currRestoreCnfgState == BUSY)
	{
        MUTEX_UNLOCK(restoreStateMutex);
        return CMD_CONFIG_RESTORE_IN_PROCESS;
	}
    currRestoreCnfgState = BUSY;
    MUTEX_UNLOCK(restoreStateMutex);

    do
    {
        // PARASOFT : Release of memory for "restoreCnfgParam" has been taken care in "restoreConfigThrd" thread below
        restoreCnfgParam = (RESTORE_CONFIG_t *)malloc(sizeof(RESTORE_CONFIG_t));
        if (restoreCnfgParam == NULL)
        {
            retVal = CMD_PROCESS_ERROR;
            break;
        }

        // store callback and session index
        snprintf(restoreCnfgParam->fileName, sizeof(restoreCnfgParam->fileName), RAMFS_DIR_PATH"%s", fileName);

        // check if the file exists
        if (access(restoreCnfgParam->fileName, F_OK) != STATUS_OK)
        {
            EPRINT(SYSTEM_UPGRADE, "config restore file does not exist: [path=%s]", restoreCnfgParam->fileName);
            retVal = CMD_FILE_MISSING;
            break;
        }

        // create the detached thread to start file extraction
        if (FAIL == Utils_CreateThread(NULL, restoreConfigThrd, restoreCnfgParam, DETACHED_THREAD, RESTORE_CONFIG_THREAD_STACK_SZ))
        {
            EPRINT(SYSTEM_UPGRADE, "fail to create config restore thread");
            retVal = CMD_RESOURCE_LIMIT;
            break;
        }

        /* Config restore process started successfully */
        return CMD_SUCCESS;

    }while(0);

    // Make session FREE if it was free previously
    FREE_MEMORY(restoreCnfgParam);

    /* PARASOFT: BD-TRS-DIFCS: Variable used in multiple critical sections */
    MUTEX_LOCK(restoreStateMutex);
    currRestoreCnfgState = FREE;
    MUTEX_UNLOCK(restoreStateMutex);

    //deallocate memory in thread function
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API sets the Restore Configuration process busy so tuat
 * @param   arg - pointer to restore session information
 * @return
 */
static VOIDPTR restoreConfigThrd(VOIDPTR arg)
{
    RESTORE_CONFIG_t    *thRestoreConfigInfo = (RESTORE_CONFIG_t *)arg;
    NET_CMD_STATUS_e    retVal = CMD_PROCESS_ERROR;

	WriteEvent(LOG_OTHER_EVENT, LOG_RESTORE_CONFIG_STRAT, NULL, NULL, EVENT_ALERT);

    if(CopyFile(thRestoreConfigInfo->fileName, CONFIG_ZIP_FILE, 10, NULL, 0) == FAIL)
	{
        EPRINT(SYSTEM_UPGRADE, "fail to move config file");
	}
	else
	{
        /* Action to be taken before config restore */
        procBeforeConfigRestore();

		// All done, now Prepare system for a safe reboot
        DPRINT(SYSTEM_UPGRADE, "restart system for config upgrade");
		if (PrepForPowerAction(REBOOT_DEVICE, EVENT_AUTO, "Config Restore") == SUCCESS)
		{
            retVal = CMD_SUCCESS;
		}
	}

	// If something went wrong, set currRestoreCnfgState FREE so that user can try again
    if (retVal != CMD_SUCCESS)
	{
		WriteEvent(LOG_OTHER_EVENT, LOG_RESTORE_CONFIG_RESULT, NULL, NULL, EVENT_FAIL);
        MUTEX_LOCK(restoreStateMutex);
		currRestoreCnfgState = FREE;
        MUTEX_UNLOCK(restoreStateMutex);
	}
	else
	{
		WriteEvent(LOG_OTHER_EVENT, LOG_RESTORE_CONFIG_RESULT, NULL, NULL, EVENT_SUCCESS);
	}

	// FREE allocated memory
    free(thRestoreConfigInfo);
	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Perform actions before config restore into the system
 */
static void procBeforeConfigRestore(void)
{
    // In case of Restoring old custom files, current custom files must be deleted first.
    ExeSysCmd(TRUE, "rm -f $(find "LANGUAGES_DIR_PATH" -type f -name \"*\" ! -name \""SAMPLE_CSV_FILE_NAME"\")");
    NotifyLanguageUpdate();

    /* Notify config restore action to DHCP server */
    DhcpServerConfigRestoreNotify();
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
