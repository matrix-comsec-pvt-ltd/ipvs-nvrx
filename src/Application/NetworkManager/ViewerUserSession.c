//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		ViewerUserSession.c
@brief      This module provides the information related to viewer user session
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <pthread.h>

/* Application Includes */
#include "ViewerUserSession.h"
#include "DateTime.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define USER_SESSION_FILE			EVENT_DIR_PATH "/userSession.log"

#define VER_DAT_TIME_ELEMENTS       (4)
#define DATE_TIME_YEAR_OFFSET       (VER_DAT_TIME_ELEMENTS * sizeof(UINT32))
#define USER_VIEWER_FILE_VERSION    (2)

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static pthread_mutex_t              fileReadWriteMutex = PTHREAD_MUTEX_INITIALIZER;

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   InitViewerUserSession
 * @return
 */
void InitViewerUserSession(void)
{
    FILE        *pFile;
    struct tm   systemTimeInTmStruct = { 0 };
    UINT32      dateMonthYear[VER_DAT_TIME_ELEMENTS] = { 0 };

    if (SUCCESS != GetLocalTimeInBrokenTm(&systemTimeInTmStruct))
    {
        EPRINT(NETWORK_MANAGER, "fail to read local time");
    }

    /* Check the existence of folder, if not present then create it */
    if (access(EVENT_DIR_PATH, F_OK) != STATUS_OK)
    {
        mkdir(EVENT_DIR_PATH, USR_RWE_GRP_RE_OTH_RE);
    }

    /* Check the existence of user session file, if not present then create it */
    if (access(USER_SESSION_FILE, F_OK) != STATUS_OK)
    {
        WPRINT(NETWORK_MANAGER, "user session file not preset, need to create it");
        ResetAllDefaultUser(TRUE);
        return;
    }

    /* Open user session file */
    pFile = fopen(USER_SESSION_FILE, "r");
    if (pFile == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        ResetAllDefaultUser(TRUE);
        return;
    }

    /* Read data time elements */
    if (fread(dateMonthYear, sizeof(UINT32), VER_DAT_TIME_ELEMENTS, pFile) != VER_DAT_TIME_ELEMENTS)
    {
        EPRINT(NETWORK_MANAGER, "fail to read file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        fclose(pFile);
        ResetAllDefaultUser(TRUE);
        return;
    }

    /* Close the file */
    fclose(pFile);
    if ((dateMonthYear[0] != USER_VIEWER_FILE_VERSION) || (dateMonthYear[1] != (UINT32)systemTimeInTmStruct.tm_mday)
            || (dateMonthYear[2] != (UINT32)systemTimeInTmStruct.tm_mon) || (dateMonthYear[3] != (UINT32)systemTimeInTmStruct.tm_year))
    {
        DPRINT(NETWORK_MANAGER, "date changed, reset user session file");
        ResetAllDefaultUser(FALSE);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ResetAllDefaultUser
 * @param   isInit
 */
void ResetAllDefaultUser(BOOL isInit)
{
    FILE                        *pFile;
    UINT8                       loop;
    USER_SESSION_FILE_INFO_t    userSessionInfo[MAX_USER_ACCOUNT];
    struct tm                   systemTimeInTmStruct = { 0 };
    UINT32                      dateMonthYear[VER_DAT_TIME_ELEMENTS] = { 0 };

	if(SUCCESS != GetLocalTimeInBrokenTm(&systemTimeInTmStruct))
	{
        EPRINT(NETWORK_MANAGER, "fail to read local time");
	}

	dateMonthYear[0] = USER_VIEWER_FILE_VERSION;
	dateMonthYear[1] = systemTimeInTmStruct.tm_mday;
	dateMonthYear[2] = systemTimeInTmStruct.tm_mon;
	dateMonthYear[3] = systemTimeInTmStruct.tm_year;

	for(loop = 0; loop < MAX_USER_ACCOUNT; loop++)
	{
        userSessionInfo[loop].username[0] = '\0';
		userSessionInfo[loop].totalRemainingSessionTime = 0;
		userSessionInfo[loop].totalSessionTime = 0;
		userSessionInfo[loop].totalElapsedSessionTime = 0;
	}

    MUTEX_LOCK(fileReadWriteMutex);
    pFile = fopen(USER_SESSION_FILE, (isInit == TRUE) ? "w" : "r+");
    if(pFile == NULL)
	{
        MUTEX_UNLOCK(fileReadWriteMutex);
        EPRINT(NETWORK_MANAGER, "fail to open user session file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        return;
    }

    if(fseek(pFile, 0, SEEK_SET) != -1)
    {
        if(fwrite(dateMonthYear, sizeof(UINT32), VER_DAT_TIME_ELEMENTS, pFile) != VER_DAT_TIME_ELEMENTS)
        {
            EPRINT(NETWORK_MANAGER, "fail to write date-time in user session file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        }
        else if(fwrite(userSessionInfo, sizeof(USER_SESSION_FILE_INFO_t), MAX_USER_ACCOUNT, pFile) != MAX_USER_ACCOUNT)
        {
            EPRINT(NETWORK_MANAGER, "fail to write user session data in file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        }
    }
    fclose(pFile);
    MUTEX_UNLOCK(fileReadWriteMutex);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   IfUserAvailableInCfg
 * @param   indexNo
 * @param   userName
 * @param   userSessInfo
 * @return
 */
BOOL IfUserAvailableInCfg(UINT8PTR indexNo, CHARPTR userName, USER_SESSION_FILE_INFO_t *userSessInfo)
{
    FILE                        *pFile;
    UINT8                       loop;
    USER_SESSION_FILE_INFO_t    userSessionInfo[MAX_USER_ACCOUNT];

    MUTEX_LOCK(fileReadWriteMutex);
    pFile = fopen(USER_SESSION_FILE, "r");
    if(pFile == NULL)
    {
        MUTEX_UNLOCK(fileReadWriteMutex);
        EPRINT(NETWORK_MANAGER, "fail to open user session file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        return FALSE;
    }

    if(fseek(pFile, DATE_TIME_YEAR_OFFSET, SEEK_SET) == -1)
    {
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }

    if(fread(userSessionInfo, sizeof(USER_SESSION_FILE_INFO_t), MAX_USER_ACCOUNT, pFile) != MAX_USER_ACCOUNT)
    {
        EPRINT(NETWORK_MANAGER, "fail to read user session data in file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }
    fclose(pFile);
    MUTEX_UNLOCK(fileReadWriteMutex);

    for(loop = 0; loop < MAX_USER_ACCOUNT; loop++)
    {
        if((strlen(userSessionInfo[loop].username) != 0) && (strcmp(userSessionInfo[loop].username, userName) == 0))
        {
            *indexNo = loop;
            memcpy(userSessInfo, &userSessionInfo[loop], sizeof(USER_SESSION_FILE_INFO_t));
            break;
        }
    }

    return (loop >= MAX_USER_ACCOUNT) ? FALSE : TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   FindFreeIndexInCfg
 * @param   indexNo
 * @return
 */
BOOL FindFreeIndexInCfg(UINT8PTR indexNo)
{
    FILE                        *pFile;
    UINT8                       loop;
    USER_SESSION_FILE_INFO_t    userSessionInfo[MAX_USER_ACCOUNT];

    MUTEX_LOCK(fileReadWriteMutex);
    pFile = fopen(USER_SESSION_FILE, "r");
    if(pFile == NULL)
	{
        MUTEX_UNLOCK(fileReadWriteMutex);
        EPRINT(NETWORK_MANAGER, "fail to open user session file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        return FALSE;
    }

    if(fseek(pFile, DATE_TIME_YEAR_OFFSET, SEEK_SET) == -1)
    {
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }

    if(fread(userSessionInfo, sizeof(USER_SESSION_FILE_INFO_t), MAX_USER_ACCOUNT, pFile) != MAX_USER_ACCOUNT)
    {
        EPRINT(NETWORK_MANAGER, "fail to read user session data from file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }
    fclose(pFile);
    MUTEX_UNLOCK(fileReadWriteMutex);

    for(loop = 0; loop < MAX_USER_ACCOUNT; loop++)
    {
        if(strlen(userSessionInfo[loop].username) == 0)
        {
            *indexNo = loop;
            break;
        }
    }

    return (loop >= MAX_USER_ACCOUNT) ? FALSE : TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   UpdateUserSessionData
 * @param   indexNo
 * @param   userSessInfo
 * @return
 */
BOOL UpdateUserSessionData(UINT8 indexNo, USER_SESSION_FILE_INFO_t *userSessInfo)
{
    FILE *pFile;

    if (indexNo >= MAX_USER_ACCOUNT)
	{
        return FALSE;
    }

    MUTEX_LOCK(fileReadWriteMutex);
    pFile = fopen(USER_SESSION_FILE, "r+");
    if(pFile == NULL)
    {
        MUTEX_UNLOCK(fileReadWriteMutex);
        EPRINT(NETWORK_MANAGER, "fail to open user session file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        return FALSE;
    }

    if(fseek(pFile, (DATE_TIME_YEAR_OFFSET + (indexNo * sizeof(USER_SESSION_FILE_INFO_t))), SEEK_SET) == -1)
    {
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }

    if(fwrite(userSessInfo, sizeof(USER_SESSION_FILE_INFO_t), 1, pFile) != 1)
    {
        EPRINT(NETWORK_MANAGER, "fail to write user session data in file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }
    fclose(pFile);
    MUTEX_UNLOCK(fileReadWriteMutex);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ReadUserSessionData
 * @param   indexNo
 * @param   userSessInfo
 * @return
 */
BOOL ReadUserSessionData(UINT8 indexNo, USER_SESSION_FILE_INFO_t *userSessInfo)
{
    FILE *pFile;

    if(indexNo >= MAX_USER_ACCOUNT)
	{
        return FALSE;
    }

    MUTEX_LOCK(fileReadWriteMutex);
    pFile = fopen(USER_SESSION_FILE, "r");
    if(pFile == NULL)
    {
        MUTEX_UNLOCK(fileReadWriteMutex);
        EPRINT(NETWORK_MANAGER, "fail to open user session file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        return FALSE;
    }

    if(fseek(pFile, (DATE_TIME_YEAR_OFFSET + (indexNo * sizeof(USER_SESSION_FILE_INFO_t))), SEEK_SET) == -1)
    {
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }

    if(fread(userSessInfo, sizeof(USER_SESSION_FILE_INFO_t), 1, pFile) != 1)
    {
        EPRINT(NETWORK_MANAGER, "fail to read user session data from file: [path=%s], [err=%s]", USER_SESSION_FILE, STR_ERR);
        fclose(pFile);
        MUTEX_UNLOCK(fileReadWriteMutex);
        return FALSE;
    }
    fclose(pFile);
    MUTEX_UNLOCK(fileReadWriteMutex);
    return TRUE;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
