//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkFileTransfer.h
@brief      This module provides API to transfer file between CMS and NVR device for firmware
            upgrade/restore configurations/backup configuration.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>

/* Application Includes */
#include "Utils.h"
#include "NetworkManager.h"
#include "NetworkCommand.h"
#include "DebugLog.h"
#include "InputOutput.h"
#include "CsvParser.h"
#include "NetworkFileTransfer.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	FILE_PART_EXTENTION			".part"
#define	FW_FILE_EXTENTION			"zip"
#define LANG_FILE_EXTENTION			".csv"
#if defined(RK3568_NVRL) || defined(RK3588_NVRH)
#define MAX_FILE_SIZE 				(200 * MEGA_BYTE)
#else
#define MAX_FILE_SIZE 				(64 * MEGA_BYTE)
#endif
#define MAX_LANGUAGE_FILE_SIZE		(2 * MEGA_BYTE)

#define MAX_FTS_PACKET_SIZE 		1024*15
#define MAX_FTS_MSG_SIZE  			16383
#define MAX_FTS_MSG_WAIT_TIME 		5
#define MAX_FILE_NAME_LENGTH		MAX_LANGUAGE_FILE_NAME_LEN

#define MAX_FILE_PATH_LENGTH		((sizeof(RAMFS_DIR_PATH) > sizeof(TEMP_DIR_PATH) ?                      \
                                    sizeof(RAMFS_DIR_PATH) : sizeof(TEMP_DIR_PATH))	+ MAX_FILE_NAME_LENGTH  \
                                    + sizeof(LANG_FILE_EXTENTION) + sizeof(FILE_PART_EXTENTION))

#define	PARSE_SINGLE_ARGUMENT		1
#define FILE_TRANSFER_STACK_SZ      (2*MEGA_BYTE)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef	enum
{
    FILE_XFER_STP_EOF = 0,
	FILE_XFER_STP_CAN,
	MAX_FILE_XFER_STP

}FTS_STP_e;

typedef enum
{
    FIRMWARE_UPGRADE = 0,
	RESTORE_CONFIG,
	BACKUP_CONFIG,
	IMPORT_LANGUAGE,
	EXPORT_LANGUAGE,
	DELETE_LANGUAGE,
	PREFERRED_LANGUAGE,
	MAX_REQ_TYPE

}REQ_TYPE_e;

typedef struct
{
	REQ_TYPE_e			requestType;	// purpose of file transfer
	INT32 				clientFd;

	CHAR				fileName[MAX_FILE_NAME_LENGTH + sizeof(LANG_FILE_EXTENTION)];
	CHAR				absoluteFilePath[MAX_FILE_PATH_LENGTH];
	UINT32				fileSize;
	INT32				fileFd;			// session file descriptor
	UINT32				packetCount;	// current Packet Count

	BOOL 				removeFileFlag;	// whether to remove file or not
	HEADER_REQ_e 		header;			// message header
	NET_CMD_STATUS_e	status;			// status to be replied

	CHARPTR 			replyBuffer;	// buffer Pointer in which reply message is to be constructed
	UINT16				dataLength;		// Length of data to be replied

}FILE_XFER_SESSION_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static BOOL 			fileXferStatus = FREE;		// Status of file Xfer request
static pthread_mutex_t 	fileXferStatusLock = PTHREAD_MUTEX_INITIALIZER;
static CHAR 			msgBuffer[MAX_FTS_MSG_SIZE];

static const CHARPTR    reqTypeStr[MAX_REQ_TYPE] =
{
    "FIRMWARE_UPGRADE",
    "RESTORE_CONFIG",
    "BACKUP_CONFIG",
    "IMPORT_LANGUAGE",
    "EXPORT_LANGUAGE",
    "DELETE_LANGUAGE",
    "PREFERRED_LANGUAGE"
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL openFileXferSession(FILE_XFER_SESSION_t *sessionPtr);
//-------------------------------------------------------------------------------------------------
static void closeFileXferSession(FILE_XFER_SESSION_t *sessionPtr);
//-------------------------------------------------------------------------------------------------
static BOOL replyToClient(FILE_XFER_SESSION_t *sessionPtr, const UINT32 replyBufferLen);
//-------------------------------------------------------------------------------------------------
static VOIDPTR fileXferThread(VOIDPTR arg);
//-------------------------------------------------------------------------------------------------
static BOOL validateFileName(CHARPTR fileName, REQ_TYPE_e requestType, UINT16PTR version, UINT16PTR revision);
//-------------------------------------------------------------------------------------------------
static BOOL validateLanguageFileName(CHARPTR filename);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e checkLanguageCount(void);
//-------------------------------------------------------------------------------------------------
static BOOL validateForNotAllowedCharacters(CHARPTR fileName);
//-------------------------------------------------------------------------------------------------
static BOOL isLanguageModifyReq(CHARPTR fileName);
//-------------------------------------------------------------------------------------------------
static BOOL copyFileDataToOtherFile(CHARPTR fileName, CHARPTR srcfileName);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API process File transfer request and upon valid request Creates a Thread that will
 *          Handle File Transfer on respective Connection
 * @param   msg
 * @param   clientFd
 * @param   sessionIndex
 */
void ProcessFileTransferReq(CHARPTR *msg, INT32 clientFd, UINT8 sessionIndex)
{
	UINT64 					fieldVal;
    CHAR 					tempStr[MAX_FILE_NAME_LENGTH];
	USER_ACCOUNT_CONFIG_t 	userAccountConfig;
	FILE_XFER_SESSION_t		*fileXferSession;
	FILE_XFER_SESSION_t		ftsData;
    BOOL					curFileXferStatus;
	UINT16					version;
	UINT16					revision;

    // Initialisation
    ftsData.requestType = MAX_REQ_TYPE;
    ftsData.header = MAX_HEADER_REQ;
    ftsData.clientFd = clientFd;
    ftsData.status = CMD_SUCCESS;

	do
	{
        // check whether file Transfer is already in process or not
        MUTEX_LOCK(fileXferStatusLock);
        curFileXferStatus = fileXferStatus;
        if (fileXferStatus == BUSY)
		{
            MUTEX_UNLOCK(fileXferStatusLock);
			ftsData.status = CMD_REQUEST_IN_PROGRESS;
            EPRINT(NETWORK_MANAGER, "other file transfer request already in progress");
            break;
		}
        fileXferStatus = BUSY;
        MUTEX_UNLOCK(fileXferStatusLock);

		// parse request type
        if (ParseStringGetVal(msg, &fieldVal, PARSE_SINGLE_ARGUMENT, FSP) != SUCCESS)
		{
			ftsData.status = CMD_INVALID_SYNTAX;
            EPRINT(NETWORK_MANAGER, "fail to parse file transfer request type");
			break;
		}

		// validate request type Name
		if (fieldVal >= MAX_REQ_TYPE)
		{
			ftsData.status = CMD_INVALID_FIELD_VALUE;
            EPRINT(NETWORK_MANAGER, "invld file transfer request type: [requestType=%d]", (UINT32)fieldVal);
			break;
		}

		ftsData.requestType = fieldVal;

		// Read User Account Config
        ReadSingleUserAccountConfig(GetUserAccountIndex(sessionIndex), &userAccountConfig);

		// any user type can request for preferred language.
        if ((ftsData.requestType != PREFERRED_LANGUAGE) && (ftsData.requestType != EXPORT_LANGUAGE))
		{
			// Only Admin is Allowed to Transfer File
			if (userAccountConfig.userGroup != ADMIN)
			{
				ftsData.status = CMD_NO_PRIVILEGE;
                EPRINT(NETWORK_MANAGER, "user has no rights for file transfer request: [username=%s], [request=%s]",
                       userAccountConfig.username, reqTypeStr[ftsData.requestType]);
				break;
			}
		}

		// Case according to Request Type of REQ_FTS command
		switch(ftsData.requestType)
		{
            case BACKUP_CONFIG:
            {
                DPRINT(NETWORK_MANAGER, "%s request", reqTypeStr[ftsData.requestType]);
            }
            break;

            case FIRMWARE_UPGRADE:
            {
                // parse file name {...&filename&filesize&}
                if (ParseStr(msg, FSP, tempStr, MAX_FILE_NAME_LENGTH) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "fail to parse file name: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                if (validateFileName(tempStr, ftsData.requestType, &version, &revision) == FAIL)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "invld file name: [request=%s], [name=%s]", reqTypeStr[ftsData.requestType], tempStr);
                    break;
                }

                if ((version < SOFTWARE_VERSION) || ((version == SOFTWARE_VERSION) && (revision < SOFTWARE_REVISION)))
                {
                    ftsData.status = CMD_FILE_DOWNGRED_FAIL;
                    EPRINT(NETWORK_MANAGER, "firmware downgrade not allowed: [current=V%dR%d], [uploaded=V%dR%d]",
                           SOFTWARE_VERSION, SOFTWARE_REVISION, version, revision);
                    break;
                }

                // parse file length
                if (ParseStringGetVal(msg, &fieldVal, PARSE_SINGLE_ARGUMENT, FSP) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_SYNTAX;
                    EPRINT(NETWORK_MANAGER, "fail to parse file length: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                // validate file size
                if ((fieldVal == 0) || (fieldVal > MAX_FILE_SIZE))
                {
                    ftsData.status = CMD_INVALID_FILE_SIZE;
                    EPRINT(NETWORK_MANAGER, "invld file length: [request=%s] [length=%d]", reqTypeStr[ftsData.requestType], (UINT32)fieldVal);
                    break;
                }

                snprintf(ftsData.fileName, sizeof(ftsData.fileName), "%s", tempStr);
                ftsData.fileSize = (UINT32)fieldVal;
                DPRINT(NETWORK_MANAGER, "%s request: [fileName=%s]", reqTypeStr[ftsData.requestType], ftsData.fileName);
            }
            break;

            case RESTORE_CONFIG:
            {
                // parse file name {...&filename&filesize&}
                if (ParseStr(msg, FSP, tempStr, MAX_FILE_NAME_LENGTH) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "fail to parse file name: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                if (validateFileName(tempStr, ftsData.requestType, &version, &revision) == FAIL)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "invld file name: [request=%s], [name=%s]", reqTypeStr[ftsData.requestType], tempStr);
                    break;
                }

                if ((version > SOFTWARE_VERSION) || ((version == SOFTWARE_VERSION) && (revision > SOFTWARE_REVISION)))
                {
                    ftsData.status = CMD_FILE_DOWNGRED_FAIL; // new status command shold be specified.
                    EPRINT(NETWORK_MANAGER, "config downgrade not allowed: [current=V%dR%d], [uploaded=V%dR%d]",
                           SOFTWARE_VERSION, SOFTWARE_REVISION, version, revision);
                    break;
                }

                // parse file length
                if (ParseStringGetVal(msg, &fieldVal, PARSE_SINGLE_ARGUMENT, FSP) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_SYNTAX;
                    EPRINT(NETWORK_MANAGER, "fail to parse file length: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                // validate file size
                if ((fieldVal == 0) || (fieldVal > MAX_FILE_SIZE))
                {
                    ftsData.status = CMD_INVALID_FILE_SIZE;
                    EPRINT(NETWORK_MANAGER, "invld file length: [request=%s] [length=%d]", reqTypeStr[ftsData.requestType], (UINT32)fieldVal);
                    break;
                }

                snprintf(ftsData.fileName, sizeof(ftsData.fileName), "%s", tempStr);
                ftsData.fileSize = (UINT32)fieldVal;
                DPRINT(NETWORK_MANAGER, "%s request: [fileName=%s]", reqTypeStr[ftsData.requestType], ftsData.fileName);
            }
            break;

            case IMPORT_LANGUAGE:
            {
                // parse file name
                if (ParseStr(msg, FSP, tempStr, MAX_FILE_NAME_LENGTH) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "fail to parse file name: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                if (validateLanguageFileName(tempStr) == FAIL)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "invld file name: [request=%s], [name=%s]", reqTypeStr[ftsData.requestType], tempStr);
                    break;
                }

                // parse file length
                if (ParseStringGetVal(msg, &fieldVal, PARSE_SINGLE_ARGUMENT, FSP) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_SYNTAX;
                    EPRINT(NETWORK_MANAGER, "fail to parse file length: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                // validate file size
                if ((fieldVal == 0) || (fieldVal > MAX_LANGUAGE_FILE_SIZE))
                {
                    ftsData.status = CMD_INVALID_FILE_SIZE;
                    EPRINT(NETWORK_MANAGER, "invld file length: [request=%s] [length=%d]", reqTypeStr[ftsData.requestType], (UINT32)fieldVal);
                    break;
                }

                // append file extension to filename.
                snprintf(ftsData.fileName, sizeof(ftsData.fileName), "%s"LANG_FILE_EXTENTION, tempStr);

                //if user wants to import English.csv than don't allow
                if ((strcmp(ftsData.fileName, SAMPLE_CSV_FILE_NAME)) == STATUS_OK)
                {
                    ftsData.status = CMD_LANG_IMPORT_FAILED;
                    break;
                }

                ftsData.fileSize = (UINT32)fieldVal;
                // check if reuqested language is already imported. If yes then skip language count validation and overwrite that language.
                if (TRUE == isLanguageModifyReq(ftsData.fileName))
                {
                    DPRINT(NETWORK_MANAGER, "%s modify request: [fileName=%s]", reqTypeStr[ftsData.requestType], ftsData.fileName);
                    break;
                }

                DPRINT(NETWORK_MANAGER, "%s new request: [fileName=%s]", reqTypeStr[ftsData.requestType], ftsData.fileName);
                ftsData.status = checkLanguageCount();
            }
            break;

            case EXPORT_LANGUAGE:
            case DELETE_LANGUAGE:
            {
                // parse file name
                if (ParseStr(msg, FSP, tempStr, MAX_FILE_NAME_LENGTH) != SUCCESS)
                {
                    ftsData.status = CMD_INVALID_FILE_NAME;
                    EPRINT(NETWORK_MANAGER, "fail to parse file name: [request=%s]", reqTypeStr[ftsData.requestType]);
                    break;
                }

                // append file extension to filename.
                snprintf(ftsData.fileName, sizeof(ftsData.fileName), "%s"LANG_FILE_EXTENTION, tempStr);
                DPRINT(NETWORK_MANAGER, "%s request: [fileName=%s]", reqTypeStr[ftsData.requestType], ftsData.fileName);
            }
            break;

            case PREFERRED_LANGUAGE:
            {
                snprintf(ftsData.fileName, sizeof(ftsData.fileName), "%s", userAccountConfig.preferredLanguage);
                DPRINT(NETWORK_MANAGER, "%s request: [fileName=%s]", reqTypeStr[ftsData.requestType], ftsData.fileName);
            }
            break;

            default:
            {
                /* Nothing to do */
            }
            break;
		}

        if (ftsData.status != CMD_SUCCESS)
        {
            break;
        }

        fileXferSession = malloc(sizeof(FILE_XFER_SESSION_t));
        if (fileXferSession == NULL)
        {
            ftsData.status = CMD_RESOURCE_LIMIT;
            EPRINT(NETWORK_MANAGER, "fail to alloc memory: [request=%s]", reqTypeStr[ftsData.requestType]);
            break;
        }

        // Copy Session Data
        memcpy(fileXferSession, &ftsData, sizeof(FILE_XFER_SESSION_t));

        // Create Thread for File Transfer
        if (FAIL == Utils_CreateThread(NULL, fileXferThread, fileXferSession, DETACHED_THREAD, FILE_TRANSFER_STACK_SZ))
        {
            FREE_MEMORY(fileXferSession);
            ftsData.status = CMD_RESOURCE_LIMIT;
            EPRINT(NETWORK_MANAGER, "fail to create thread: [request=%s]", reqTypeStr[ftsData.requestType]);
            break;
        }

        /* Successfully handled file transfer request */
        return;

    } while(0);

    ftsData.header = RPL_FTS;
    ftsData.replyBuffer = tempStr;

    replyToClient(&ftsData, sizeof(tempStr));
    CloseSocket(&ftsData.clientFd); //TODO: handle callback for p2p client

    // make status FREE If it was FREE previously
    if (curFileXferStatus == FREE)
    {
        MUTEX_LOCK(fileXferStatusLock);
        fileXferStatus = FREE;
        MUTEX_UNLOCK(fileXferStatusLock);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function validates file name for given request type.
 * @param   fileName
 * @param   requestType
 * @param   version
 * @param   revision
 * @return
 */
static BOOL validateFileName(CHARPTR fileName, REQ_TYPE_e requestType, UINT16PTR version, UINT16PTR revision)
{
    CHAR	extension[4];
    CHARPTR pStrSearch;

    if (requestType == FIRMWARE_UPGRADE)
    {
        /* Search for firmware file name prefix */
        pStrSearch = strstr(fileName, FIRMWARE_NAME_PREFIX);
        if (pStrSearch == NULL)
        {
            return FAIL;
        }

        /* Skip file name prefix */
        pStrSearch += strlen(FIRMWARE_NAME_PREFIX);
    }
    else
    {
        /* Search for config file name prefix */
        pStrSearch = strstr(fileName, CONFIG_NAME_PREFIX);
        if (pStrSearch == NULL)
        {
            return FAIL;
        }

        /* Skip file name prefix */
        pStrSearch += strlen(CONFIG_NAME_PREFIX);
    }

    /* Check for file name separator */
    if (pStrSearch[0] != '_')
    {
        return FAIL;
    }

    /* Skip separator */
    pStrSearch++;

    /* Extract version, revision and file extension */
    if (sscanf(pStrSearch, FILE_VER_REV_SUFFIX, version, revision, extension) != 3)
    {
        return FAIL;
    }

    /* Validate file extension */
    if (strcmp(extension, FW_FILE_EXTENTION) != STATUS_OK)
    {
        return FAIL;
    }

    /* File name is valid */
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function validates Language File Name for IMPORT_LANGUAGE request.
 * @param   filename
 * @return
 */
static BOOL validateLanguageFileName(CHARPTR filename)
{
    if(memchr(filename, '/', strlen(filename)) != NULL)
	{
        EPRINT(NETWORK_MANAGER, "invld language file name");
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function opens file transfer session
 * @param   sessionPtr
 * @return  SUCCESS / FAIL
 */
static BOOL openFileXferSession(FILE_XFER_SESSION_t *sessionPtr)
{
    struct stat infoBuf = {0};
    INT32       mode = O_RDONLY;
    CHAR        cmdBuff[300];
    CHARPTR     subStr;
    UINT16      cnt=0;
    CHAR        fileName[MAX_LANGUAGE_FILE_NAME_LEN];
    CHAR        filePath[MAX_FILE_PATH_LENGTH];

	switch(sessionPtr->requestType)
	{
        case BACKUP_CONFIG:
        {
            snprintf(sessionPtr->fileName, sizeof(sessionPtr->fileName), CONFIG_BACKUP_FILE_NAME, SOFTWARE_VERSION, SOFTWARE_REVISION);
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, RAMFS_DIR_PATH"%s", sessionPtr->fileName);
            snprintf(cmdBuff, sizeof(cmdBuff), "zip -r %s %s %s -x %s",
                    sessionPtr->absoluteFilePath, APP_CONFIG_DIR_PATH, LANGUAGES_DIR_PATH, SAMPLE_LANGUAGE_FILE); // include language file excluding sample file.
            if(FAIL == ExeSysCmd(TRUE, cmdBuff))
            {
                EPRINT(NETWORK_MANAGER, "fail to create zip: [request=%s]", reqTypeStr[sessionPtr->requestType]);
                return FAIL;
            }

            // Get Information of file
            if (stat(sessionPtr->absoluteFilePath, &infoBuf) != STATUS_OK)
            {
                EPRINT(NETWORK_MANAGER, "fail to get file info: [request=%s], [err=%s]", reqTypeStr[sessionPtr->requestType], STR_ERR);
                return FAIL;
            }

            mode = O_RDONLY;
            sessionPtr->packetCount = 0;
            sessionPtr->fileSize = (UINT32)infoBuf.st_size;
        }
        break;

        case FIRMWARE_UPGRADE:
        case RESTORE_CONFIG:
        {
            // Change Led Status to indicate firmware upgrade process
            if (sessionPtr->requestType == FIRMWARE_UPGRADE)
            {
                SetSystemStatusLed(SYS_FIRMWARE_UPGRADE_INPROCESS, ON);
            }

            // Check if .part file exists
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, RAMFS_DIR_PATH"%s"FILE_PART_EXTENTION, sessionPtr->fileName);
            if (access(sessionPtr->absoluteFilePath, F_OK) == STATUS_OK)
            {
                // Get Information of File
                if (stat(sessionPtr->absoluteFilePath, &infoBuf) != STATUS_OK)
                {
                    EPRINT(NETWORK_MANAGER, "fail to get file info: [request=%s], [err=%s]", reqTypeStr[sessionPtr->requestType], STR_ERR);
                    return FAIL;
                }

                // Open File in append mode
                mode = 	O_WRONLY | O_APPEND;

                // Get no of packets
                sessionPtr->packetCount = (UINT32)(infoBuf.st_size / MAX_FTS_PACKET_SIZE);
            }
            else
            {
                // remove if the same file exists in /mnt/ramfs directory
                snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, RAMFS_DIR_PATH"%s", sessionPtr->fileName);
                if (access(sessionPtr->absoluteFilePath, F_OK) == STATUS_OK)
                {
                    if (remove(sessionPtr->absoluteFilePath) != STATUS_OK)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to remove file: [request=%s], [path=%s], [err=%s]",
                               reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath, STR_ERR);
                        return FAIL;
                    }
                }

                snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, RAMFS_DIR_PATH"%s"FILE_PART_EXTENTION, sessionPtr->fileName);

                // Create File
                mode = O_CREAT | O_WRONLY;
                sessionPtr->packetCount = 0;
            }
        }
        break;

        case IMPORT_LANGUAGE:
        {
            // Check if .part file exists
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, TEMP_DIR_PATH"%s"FILE_PART_EXTENTION, sessionPtr->fileName);
            if (access(sessionPtr->absoluteFilePath, F_OK) == STATUS_OK)
            {
                // Get Information of File
                if (stat(sessionPtr->absoluteFilePath, &infoBuf) != STATUS_OK)
                {
                    EPRINT(NETWORK_MANAGER, "fail to get file info: [request=%s], [err=%s]", reqTypeStr[sessionPtr->requestType], STR_ERR);
                    sessionPtr->status = CMD_LANG_IMPORT_FAILED;
                    return FAIL;
                }

                // Open File in append mode
                mode = 	O_WRONLY | O_APPEND;

                // Get no of packets
                sessionPtr->packetCount = (UINT32)(infoBuf.st_size/ MAX_FTS_PACKET_SIZE);
            }
            else
            {
                // remove if the same file exists in /tmp/ directory
                snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, TEMP_DIR_PATH"%s", sessionPtr->fileName);
                if (access(sessionPtr->absoluteFilePath, F_OK) == STATUS_OK)
                {
                    if (remove(sessionPtr->absoluteFilePath) != STATUS_OK)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to remove file: [request=%s], [path=%s], [err=%s]",
                               reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath, STR_ERR);
                        sessionPtr->status = CMD_LANG_IMPORT_FAILED;
                        return FAIL;
                    }
                }

                snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, TEMP_DIR_PATH"%s"FILE_PART_EXTENTION, sessionPtr->fileName);

                // Create File
                mode = O_CREAT | O_WRONLY;
                sessionPtr->packetCount = 0;
            }
        }
        break;

        case EXPORT_LANGUAGE:
        {
            sessionPtr->status = CMD_LANG_EXPORT_FAILED;
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, LANGUAGES_DIR_PATH "/%s", sessionPtr->fileName);
            if (access(sessionPtr->absoluteFilePath, F_OK) != STATUS_OK)
            {
                EPRINT(NETWORK_MANAGER, "file not found: [request=%s], [path=%s]", reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath);
                sessionPtr->status = CMD_LANG_FILE_NOT_FOUND;
                return FAIL;
            }

            // Get Information of File
            if (stat(sessionPtr->absoluteFilePath, &infoBuf) != STATUS_OK)
            {
                EPRINT(NETWORK_MANAGER, "fail to get file info: [request=%s], [err=%s]", reqTypeStr[sessionPtr->requestType], STR_ERR);
                sessionPtr->status = CMD_LANG_EXPORT_FAILED;
                return FAIL;
            }

            mode = O_RDONLY;
            sessionPtr->packetCount = 0;
            sessionPtr->fileSize = (UINT32)infoBuf.st_size;
        }
        break;

        case DELETE_LANGUAGE:
        {
            //if requested to delete English.csv file then don't allow
            if (strcmp(sessionPtr->fileName, SAMPLE_CSV_FILE_NAME) == STATUS_OK)
            {
                EPRINT(NETWORK_MANAGER, SAMPLE_CSV_FILE_NAME " file cannot be deleted");
                sessionPtr->status = CMD_LANG_DELETE_FAILED;
                return FAIL;
            }

            /* Get filename without .csv extension */
            subStr = strrchr(sessionPtr->fileName, '.');
            if (subStr == NULL)
            {
                EPRINT(NETWORK_MANAGER, "invld file name: [request=%s], [name=%s]", reqTypeStr[sessionPtr->requestType], sessionPtr->fileName);
                sessionPtr->status = CMD_LANG_FILE_NOT_FOUND;
                return FAIL;
            }

            /* Get File name and path without extension */
            *subStr = '\0';
            snprintf(fileName, sizeof(fileName), "%s", sessionPtr->fileName);
            snprintf(filePath, sizeof(filePath), LANGUAGES_DIR_PATH "/%s", fileName);
            *subStr = '.';

            /* Remove .csv file of given language */
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, "%s.csv", filePath);
            if (remove(sessionPtr->absoluteFilePath) != STATUS_OK)
            {
                WPRINT(NETWORK_MANAGER, "fail to remove file: [request=%s], [path=%s], [err=%s]",
                       reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath, STR_ERR);
                sessionPtr->status = CMD_LANG_DELETE_FAILED;
            }

            /* Remove .ts file of given language */
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, "%s.ts", filePath);
            if (remove(sessionPtr->absoluteFilePath) != STATUS_OK)
            {
                WPRINT(NETWORK_MANAGER, "fail to remove file: [request=%s], [path=%s], [err=%s]",
                       reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath, STR_ERR);
                sessionPtr->status = CMD_LANG_DELETE_FAILED;
            }

            /* Remove .qm file of given language */
            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, "%s.qm", filePath);
            if (remove(sessionPtr->absoluteFilePath) != STATUS_OK)
            {
                WPRINT(NETWORK_MANAGER, "fail to remove file: [request=%s], [path=%s], [err=%s]",
                       reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath, STR_ERR);
                sessionPtr->status = CMD_LANG_DELETE_FAILED;
            }

            USER_ACCOUNT_CONFIG_t userAccountCfg;
            for (cnt = 0; cnt < MAX_USER_ACCOUNT; cnt++)
            {
                ReadSingleUserAccountConfig(cnt, &userAccountCfg);
                if (strcmp(userAccountCfg.preferredLanguage, fileName) == 0)
                {
                    snprintf(userAccountCfg.preferredLanguage, MAX_LANGUAGE_FILE_NAME_LEN, "English");
                    WriteSingleUserAccountConfig(cnt, &userAccountCfg);
                }
            }
        }
        return SUCCESS;

        case PREFERRED_LANGUAGE:
        {
            sessionPtr->header = RPL_FTS;
            sessionPtr->status = CMD_SUCCESS;

            // check for preferred language of user
            if (strcmp(sessionPtr->fileName, "English") == STATUS_OK )
            {
                // {RPL_FTS&status&filename&}
                snprintf(msgBuffer, MAX_FTS_MSG_SIZE, "%c%s%c%d%c%s%c%c",
                         SOM, headerReq[sessionPtr->header], FSP, sessionPtr->status, FSP,sessionPtr->fileName,FSP,EOM);
                return SUCCESS;
            }

            snprintf(sessionPtr->absoluteFilePath, MAX_FILE_PATH_LENGTH, LANGUAGES_DIR_PATH "/%s" LANG_FILE_EXTENTION, sessionPtr->fileName);
            if (access(sessionPtr->absoluteFilePath, F_OK) != STATUS_OK)
            {
                EPRINT(NETWORK_MANAGER, "file not found: [request=%s], [path=%s]", reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath);
                sessionPtr->status = CMD_LANG_FILE_NOT_FOUND;
                return FAIL;
            }

            // Get Information of File
            if (stat(sessionPtr->absoluteFilePath, &infoBuf) != STATUS_OK)
            {
                EPRINT(NETWORK_MANAGER, "fail to get file info: [request=%s], [err=%s]", reqTypeStr[sessionPtr->requestType], STR_ERR);
                sessionPtr->status = CMD_PROCESS_ERROR;
                return FAIL;
            }

            mode = O_RDONLY;
            sessionPtr->packetCount = 0;
            sessionPtr->fileSize = (UINT32)infoBuf.st_size;

            // {RPL_FTS&status&filename&filesize&}
            snprintf(msgBuffer, MAX_FTS_MSG_SIZE, "%c%s%c%d%c%s%c%d%c%c",
                     SOM, headerReq[sessionPtr->header], FSP, sessionPtr->status, FSP, sessionPtr->fileName,FSP, sessionPtr->fileSize, FSP, EOM);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        return FAIL;
	}

    sessionPtr->fileFd = open(sessionPtr->absoluteFilePath, (mode | O_SYNC | O_CLOEXEC), USR_RW_GRP_R_OTH_R);
    if (sessionPtr->fileFd == INVALID_FILE_FD)
    {
        EPRINT(NETWORK_MANAGER, "fail to open file: [request=%s], [path=%s], [err=%s]",
               reqTypeStr[sessionPtr->requestType], sessionPtr->absoluteFilePath, STR_ERR);
        return FAIL;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function close File Transfer Session and Release related Resources.
 * @param   sessionPtr
 */
static void closeFileXferSession(FILE_XFER_SESSION_t *sessionPtr)
{
    MUTEX_LOCK(fileXferStatusLock);
	fileXferStatus = FREE;
    MUTEX_UNLOCK(fileXferStatusLock);

	// Close File fd
    CloseFileFd(&sessionPtr->fileFd);

	// Close Connection Fd
    CloseSocket(&sessionPtr->clientFd); //TODO: handle callback for p2p client

    // check if we need to delete file. always delete file for BACKUP_CONFIG request
    if ((sessionPtr->requestType == BACKUP_CONFIG) || (sessionPtr->requestType == IMPORT_LANGUAGE) || (sessionPtr->removeFileFlag == TRUE))
	{
		// check if file exists
		if (access(sessionPtr->absoluteFilePath, F_OK) == STATUS_OK)
		{
			// remove file
			if (remove(sessionPtr->absoluteFilePath) != STATUS_OK)
			{
                EPRINT(NETWORK_MANAGER, "fail to remove file: [path=%s]", sessionPtr->absoluteFilePath);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is to construct appropriate message and send it to client.
 * @param   sessionPtr
 * @param   replyBufferLen
 * @return  SUCCESS if message sent successfully else FAIL
 */
static BOOL replyToClient(FILE_XFER_SESSION_t *sessionPtr, const UINT32 replyBufferLen)
{
    UINT16 dataLen;

	// if the reply header SET_FTS message is already constructed by calling function
	if(sessionPtr->header != SET_FTS)
	{
        if ((sessionPtr->header == RPL_FTS) && (sessionPtr->status == CMD_SUCCESS))
		{
			switch(sessionPtr->requestType)
			{
                case BACKUP_CONFIG:
                case EXPORT_LANGUAGE:
                {
                    // {RPL_FTS&status&File_Name&File_Length&}
                    snprintf(sessionPtr->replyBuffer, replyBufferLen, "%c%s%c%d%c%s%c%d%c%c",
                             SOM, headerReq[sessionPtr->header], FSP, sessionPtr->status, FSP,
                            sessionPtr->fileName, FSP, sessionPtr->fileSize, FSP, EOM);
                }
                break;

                case DELETE_LANGUAGE:
                {
                    // {RPL_FTS&status&}
                    snprintf(sessionPtr->replyBuffer, replyBufferLen, "%c%s%c%d%c%c", SOM, headerReq[sessionPtr->header], FSP, sessionPtr->status, FSP,EOM);
                }
                break;

                case FIRMWARE_UPGRADE:
                case RESTORE_CONFIG:
                case IMPORT_LANGUAGE:
                {
                    // Need to send Current Packet count Additionally. {RPL_FTS&0&Packet_No&}
                    snprintf(sessionPtr->replyBuffer, replyBufferLen, "%c%s%c%d%c%d%c%c",
                            SOM, headerReq[sessionPtr->header], FSP, sessionPtr->status, FSP, (sessionPtr->packetCount + 1), FSP, EOM);
                } break;

                case PREFERRED_LANGUAGE:
                {
                    /* Nothing to prepare */
                }
                break;

                default:
                {
                    /* Invalid Type */
                }
                return FAIL;
			}
		}
		else
		{
            snprintf(sessionPtr->replyBuffer, replyBufferLen, "%c%s%c%d%c%c", SOM, headerReq[sessionPtr->header], FSP, sessionPtr->status, FSP, EOM);
		}

		// count data length
		dataLen = strlen(sessionPtr->replyBuffer);
	}
	else
	{
		// the data length
		dataLen = sessionPtr->dataLength;
	}

    //TODO: handle callback for p2p client
    if (SendToSocket(sessionPtr->clientFd, (UINT8PTR)sessionPtr->replyBuffer, dataLen, MESSAGE_REPLY_TIMEOUT) != SUCCESS)
	{
        EPRINT(NETWORK_MANAGER, "fail to send data: [request=%s], [header=%s], [status=%d]",
               reqTypeStr[sessionPtr->requestType], headerReq[sessionPtr->header], sessionPtr->status);
        return FAIL;
	}

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   copyFileDataToOtherFile
 * @param   fileName
 * @param   srcfileName
 * @return
 */
static BOOL copyFileDataToOtherFile(CHARPTR fileName, CHARPTR srcfileName)
{
    INT32   ch;
    FILE    *sourceFile, *targetFile;

    sourceFile = fopen(srcfileName, "r");
    if(sourceFile == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open source file: [path=%s], [err=%s]", srcfileName, STR_ERR);
    	return FAIL;
    }

    targetFile = fopen(fileName, "w");
    if(targetFile == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open destination file: [path=%s], [err=%s]", fileName, STR_ERR);
        fclose(sourceFile);
        return FAIL;
    }

    while((ch = fgetc(sourceFile)) != EOF)
    {
        fputc(ch, targetFile);
    }

    fclose(sourceFile);
    fclose(targetFile);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CreateCustomLanguageCsvFile
 * @param   fileName
 * @return
 */
BOOL CreateCustomLanguageCsvFile(CHARPTR fileName)
{
    UINT64      rowFinal, rowImported = 0;
    CHAR        lanFileName[MAX_LANGUAGE_FILE_NAME_LEN];
    CHAR        sampleFile[MAX_LANGUAGE_FILE_NAME_LEN];
    CHAR        tempLanFileName[MAX_LANGUAGE_FILE_NAME_LEN];
    CSV_BUFFER  *pFinalBuffer = CreateCsvBuffer();
    CSV_BUFFER  *pTranslatedBuffer = CreateCsvBuffer();
    BOOL        retVal = SUCCESS;
    UINT8       result;
    size_t      finalStringSize = MAX_LABEL_STRING_SIZE;
    size_t      translatedStringSize = MAX_LABEL_STRING_SIZE;
    CHARPTR     pFinalString = (CHARPTR)malloc(finalStringSize + 1);
    CHARPTR     pTranslatedString = (CHARPTR)malloc(translatedStringSize + 1);

    snprintf(lanFileName, MAX_LANGUAGE_FILE_NAME_LEN, LANGUAGES_DIR_PATH "/%s", fileName);
    snprintf(sampleFile, MAX_LANGUAGE_FILE_NAME_LEN, SAMPLE_LANGUAGE_FILE);
    snprintf(tempLanFileName, MAX_LANGUAGE_FILE_NAME_LEN, TEMP_DIR_PATH"%s", fileName);

    do
    {
        if (access(lanFileName, F_OK) != STATUS_OK)
    	{
    		// make copy of sample file as new language file.
    		if (copyFileDataToOtherFile(lanFileName, sampleFile) != SUCCESS)
    		{
                EPRINT(NETWORK_MANAGER, "fail to make copy of language file");
    			retVal = FAIL;
    			break;
    		}
    	}

    	// load sample format of language of cvs file into buffer
        if (LoadCsvFile(pFinalBuffer, lanFileName) != STATUS_OK )
    	{
            EPRINT(NETWORK_MANAGER, "fail to load language csv file");
    		retVal = FAIL;
    		break;
    	}

    	// load imported csv file into buffer
        if (LoadCsvFile(pTranslatedBuffer, tempLanFileName) != STATUS_OK)
    	{
            EPRINT(NETWORK_MANAGER, "fail to load imported language csv file");
    		retVal = FAIL;
    		break;
    	}

    	//verify format of imported language file
        if (pTranslatedBuffer->width[rowImported] < 2)
    	{
    		// imported file must have 2 columns English label and Translated label.
            EPRINT(NETWORK_MANAGER, "imported csv file doesn't have 2 columns");
    		retVal = REFUSE;
    		break;
    	}

    	//check if first row has not been modified or removed by user
        if ((GetCsvField(pTranslatedString, translatedStringSize, pTranslatedBuffer, 0, 0) != STATUS_OK))
    	{
            EPRINT(NETWORK_MANAGER, "fail to parse first column of imported csv file");
            retVal = FAIL;
            break;
        }

        if (strncmp(pTranslatedString, "English Label", strlen("English Label")) != STATUS_OK)
        {
            EPRINT(NETWORK_MANAGER, "first column of first row altered in imported csv file");
            retVal = REFUSE;
            break;
        }

        if ((GetCsvField(pTranslatedString, translatedStringSize, pTranslatedBuffer, 0, 1) != STATUS_OK))
    	{
            EPRINT(NETWORK_MANAGER, "fail to parse second column of imported csv file");
            retVal = FAIL;
            break;
        }

        if (strncmp(pTranslatedString, "Translated Label", strlen("Translated Label")) != STATUS_OK)
        {
            EPRINT(NETWORK_MANAGER, "second column of first row altered in imported csv file");
            retVal = REFUSE;
            break;
        }

    	/* Compare with English Label of file and copy translated label if both file index match then */
        for(rowFinal = 1; rowFinal < pFinalBuffer->rows; rowFinal++)
    	{
            if (GetCsvField(pFinalString, finalStringSize, pFinalBuffer, rowFinal, 0) != STATUS_OK)
    		{
                EPRINT(NETWORK_MANAGER, "fail to parse sample csv: [row=%lld]", rowFinal);
    			retVal = FAIL;
    			break;
            }

            for(rowImported = 1; rowImported < pTranslatedBuffer->rows; rowImported++ )
    		{
                if (GetCsvField(pTranslatedString, translatedStringSize, pTranslatedBuffer, rowImported, 0) != STATUS_OK)
    			{
                    EPRINT(NETWORK_MANAGER, "fail to parse imported csv: [row=%lld]", rowImported);
    				retVal = FAIL;
    				break;
    			}

                if (strcmp(pTranslatedString, pFinalString) != 0)
                {
                    continue;
                }

                result = GetCsvField(pTranslatedString, translatedStringSize, pTranslatedBuffer, rowImported, 1);
                if (result != STATUS_OK)
                {
                    if (result == 1) // transalated string is greater than 1024 characters
                    {
                        EPRINT(NETWORK_MANAGER, "translated label truncated in csv: [label=%s]", pFinalString);
                    }
                    else
                    {
                        EPRINT(NETWORK_MANAGER, "fail to get csv field: [row=%lld]", rowImported);
                        retVal = FAIL;
                        break;
                    }
                }

                // if translated label is kept blank in imported .csv file than don't set that field.
                if (pTranslatedString[0] != '\r')
                {
                    if(pTranslatedString[strlen(pTranslatedString)-1] == '\r')
                    {
                        pTranslatedString[strlen(pTranslatedString)-1] = '\0';
                    }

                    if (SetCsvField(pFinalBuffer, rowFinal, 1, pTranslatedString) != STATUS_OK)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to set csv field: [row=%lld]", rowImported);
                        retVal = FAIL;
                        break;
                    }
                }
                break;
    		}

    		if (retVal == FAIL)
    		{
    			break;
    		}
    	}

    }while(0);

    FREE_MEMORY(pFinalString);
    FREE_MEMORY(pTranslatedString);

    if (retVal != SUCCESS)
    {
    	if (access(lanFileName,F_OK) == STATUS_OK)
    	{
    		remove(lanFileName);
    	}

        DestroyCsvBuffer(pTranslatedBuffer);
        DestroyCsvBuffer(pFinalBuffer);
    	return retVal;
    }

    SaveCsvFile(tempLanFileName, pTranslatedBuffer);
    SaveCsvFile(lanFileName, pFinalBuffer);
    CreateTsFile(pFinalBuffer,lanFileName);
    DestroyCsvBuffer(pTranslatedBuffer);
    DestroyCsvBuffer(pFinalBuffer);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Thread performs File Transfer. It receives File Transfer related Message and upon
 *          successful validation of request it stores data and give appropriate acknowledgement
 *          back to Client.
 * @param   arg - pointer to structure of file transfer session
 * @return
 */
static VOIDPTR fileXferThread(VOIDPTR arg)
{
	BOOL 				threadExitFlag = FALSE;
    UINT8 				msgHeader;
	UINT64 				fieldVal;
	CHARPTR 			buffPtr;
	struct stat 		infoBuf;
    UINT32              recvLen=0;
	FILE_XFER_SESSION_t	*sessionPtr = (FILE_XFER_SESSION_t *)arg;
    BOOL				fileReceived = FALSE;
	BOOL				tempStatus = FAIL;

	// Initialisation
	sessionPtr->absoluteFilePath[0] = '\0';
	sessionPtr->fileFd = INVALID_FILE_FD;
	sessionPtr->packetCount = 0;
	sessionPtr->removeFileFlag = FALSE;
	sessionPtr->status = CMD_PROCESS_ERROR;

	// Open file transfer session for given request
	if (openFileXferSession(sessionPtr) == SUCCESS)
	{
		sessionPtr->status = CMD_SUCCESS;
        DPRINT(NETWORK_MANAGER, "starting file transfer session");
	}
	else
	{
		threadExitFlag = TRUE;
	}

	sessionPtr->replyBuffer = msgBuffer;
	sessionPtr->header = RPL_FTS;

    if (replyToClient(sessionPtr, sizeof(msgBuffer)) == FAIL)
	{
		threadExitFlag = TRUE;
	}

	// in case of delete language
    if (sessionPtr->requestType == DELETE_LANGUAGE)
	{
		threadExitFlag = TRUE;
	}

	while (threadExitFlag == FALSE)
	{
        // Make threadExitFlag TRUE. whoever wants thread keep running will set this flag as FALSE
		threadExitFlag = TRUE;
        buffPtr = msgBuffer;

        //TODO: need to handle for p2p client
		// Receive Message from Client
        if (RecvMessage(sessionPtr->clientFd, buffPtr, (UINT32PTR)&fieldVal, SOM, EOM, MAX_FTS_MSG_SIZE, MAX_FTS_MSG_WAIT_TIME) != SUCCESS)
		{
            EPRINT(NETWORK_MANAGER, "Msg recv timeout or connection closed");
			break;
        }

        // Actual Received Length of Message, kept for validating data length
        recvLen = fieldVal;

		// Increment buffer pointer for SOM
		buffPtr++;

		// find Header Index
		if (FindHeaderIndex(&buffPtr, &msgHeader) != CMD_SUCCESS)
		{
			if (sessionPtr->requestType != BACKUP_CONFIG)
			{
				sessionPtr->header = ACK_FTS;
				sessionPtr->status = CMD_INVALID_MESSAGE;
                replyToClient(sessionPtr, sizeof(msgBuffer));
			}
            EPRINT(NETWORK_MANAGER, "unable to find header index");
			break;
		}

		switch(msgHeader)
		{
            case SET_FTS:
            {
                // For SET_FTS reply header is ACK_FTS
                sessionPtr->header = ACK_FTS;

                // Parse Sequence Number
                if (ParseStringGetVal(&buffPtr, &fieldVal, PARSE_SINGLE_ARGUMENT,FSP) != SUCCESS)
                {
                    sessionPtr->status = CMD_INVALID_SYNTAX;
                }
                // Validate Sequence Number
                else if ((UINT32)fieldVal != (sessionPtr->packetCount + 1))
                {
                    sessionPtr->status = CMD_INVALID_SEQUENCE;
                }
                // Parse Data Length
                else if (ParseStringGetVal(&buffPtr, &fieldVal, PARSE_SINGLE_ARGUMENT, FSP) != SUCCESS)
                {
                    sessionPtr->status = CMD_INVALID_SYNTAX;
                    break;
                }
                // Validate Data Length
                else if ((fieldVal <= 0) || (fieldVal > MAX_FTS_PACKET_SIZE))
                {
                    sessionPtr->status = CMD_INVALID_DATA_LENGTH;
                }
                // Validate Data Packet Length
                // Note : when transfering firmware or any binary data we can encouter field delimeters such as FSP and EOM in any data segment.
                //        now, if in any case EOM is encounted inbetween chunks, no further reading from socket will be done as we consider it as
                //        end of message but actully data portion is incomplete. This is observed when packet size is increased to 15KB which is
                //        more than TCP MTU. so data will be send in smaller chunks. To avoid this we will validate the data portion recevied with
                //        the value advertise by the message field. And if data is incomplete we will read reamining data from socket and append to
                //        origianl buffer.
                else if ((((UINT64)recvLen-(buffPtr-msgBuffer)) != (fieldVal+2))) // (fieldVal + 2 ) : Data Length + 2 bytes for FSP and EOM
                {
                    sessionPtr->status = CMD_INVALID_DATA_LENGTH;
                    //TODO: need to handle for p2p client
                    if (SUCCESS != RecvFrame(sessionPtr->clientFd, &msgBuffer[recvLen], &recvLen,
                                             (UINT32)(fieldVal+2 - (recvLen-(buffPtr-msgBuffer))), MAX_FTS_MSG_WAIT_TIME, 0))
                    {
                        EPRINT(NETWORK_MANAGER, "fail to recv remaining msg frame");
                    }
                }

                // Verify That File size does not Exceed as specified previously
                if ((sessionPtr->packetCount * MAX_FTS_PACKET_SIZE + (UINT32)fieldVal) > sessionPtr->fileSize)
                {
                    // Remove File as It will lead to Error For Next Request also
                    sessionPtr->removeFileFlag = TRUE;
                    sessionPtr->status = CMD_PROCESS_ERROR;
                }
                // Write data to File
                else if (write(sessionPtr->fileFd, buffPtr, fieldVal) != (INT32)fieldVal)
                {
                    // remove File
                    sessionPtr->removeFileFlag = TRUE;
                    sessionPtr->status = CMD_PROCESS_ERROR;
                    EPRINT(NETWORK_MANAGER, "fail to write packet to file");
                }
                else
                {
                    // increment packet count
                    sessionPtr->packetCount++;
                    threadExitFlag = FALSE;
                    sessionPtr->status = CMD_SUCCESS;
                }
            }
            break;

            case ACK_FTS:
            {
                // check if request was for BACKUP_CONFIG or EXPORT_LANGUAGE or PREFERRED_LANGUAGE
                if (!((sessionPtr->requestType == BACKUP_CONFIG) || (sessionPtr->requestType == EXPORT_LANGUAGE)
                      || (sessionPtr->requestType == PREFERRED_LANGUAGE)))
                {
                    EPRINT(NETWORK_MANAGER, "invld msg header for backup_config/export_language/preferred_language");
                    break;
                }

                // always delete file for Back Up while closing session
                sessionPtr->header = SET_FTS;
                if (sessionPtr->requestType == EXPORT_LANGUAGE)
                {
                    sessionPtr->status = CMD_LANG_EXPORT_FAILED;
                }
                else
                {
                    sessionPtr->status = CMD_PROCESS_ERROR;
                }

                // parse status
                if (ParseStringGetVal(&buffPtr, &fieldVal, PARSE_SINGLE_ARGUMENT, FSP)!= SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld syntax for ACK_FTS");
                    break;
                }

                // response status must be success
                if (fieldVal != CMD_SUCCESS)
                {
                    EPRINT(NETWORK_MANAGER, "invld reply ACK_FTS: [status=%llu]", fieldVal);
                    break;
                }

                // check for reading the last packet
                if (((sessionPtr->packetCount + 1) * MAX_FTS_PACKET_SIZE) > sessionPtr->fileSize )
                {
                    // If End of File already reached give data length 0 in reply
                    if ((INT16)(sessionPtr->fileSize - (sessionPtr->packetCount * MAX_FTS_PACKET_SIZE)) <= 0)
                    {
                        fieldVal = 0;
                    }
                    else
                    {
                        // count number of bytes to be read
                        fieldVal = (sessionPtr->fileSize - ((UINT64)sessionPtr->packetCount * MAX_FTS_PACKET_SIZE));
                    }
                }
                else
                {
                    // read MAX_FTS_PACKET_SIZE bytes
                    fieldVal = MAX_FTS_PACKET_SIZE;
                }

                // {SET_FTS&Packet No&Data Length&Data&}
                // Adding 1 in packet count to indicate sending next packet
                buffPtr = msgBuffer;
                buffPtr += snprintf(buffPtr, MAX_FTS_MSG_SIZE, "%c%s%c%d%c%d%c",
                                    SOM,headerReq[SET_FTS], FSP,	sessionPtr->packetCount + 1, FSP,(UINT16)fieldVal, FSP);
                if (fieldVal == 0)
                {
                    // Append EOM
                    buffPtr += snprintf(buffPtr, MAX_FTS_MSG_SIZE, "%c", EOM);

                    // count number of bytes written in buffer
                    sessionPtr->dataLength = (buffPtr - msgBuffer);
                    threadExitFlag = FALSE;
                    sessionPtr->status = CMD_SUCCESS;
                }
                else if (read(sessionPtr->fileFd, (VOIDPTR)buffPtr, fieldVal)!= (INT64)fieldVal)
                {
                    EPRINT(NETWORK_MANAGER, "fail to read data from file");
                }
                else
                {
                    // set buffer pointer to end of packet
                    buffPtr += fieldVal;

                    // Append FSP and EOM
                    buffPtr += snprintf(buffPtr, MAX_FTS_MSG_SIZE, "%c%c", FSP, EOM);

                    // count number of bytes written in buffer
                    sessionPtr->dataLength = (buffPtr - msgBuffer);
                    sessionPtr->packetCount++;
                    threadExitFlag = FALSE;
                    sessionPtr->status = CMD_SUCCESS;
                }
            }
            break;

            case STP_FTS:
            {
                // terminate thread in case of BACKUP
                sessionPtr->status = CMD_PROCESS_ERROR;

                // Don't reply for BACKUP_CONFIG/PREFERRED_LANGUAGE/EXPORT_LANGUAGE close abnormally
                if (sessionPtr->requestType == BACKUP_CONFIG)
                {
                    break;
                }

                sessionPtr->header = RPL_STP;
                if ((sessionPtr->requestType == EXPORT_LANGUAGE) || (sessionPtr->requestType == PREFERRED_LANGUAGE))
                {
                    sessionPtr->status = CMD_SUCCESS;
                    break;
                }

                // parse request type
                if (ParseStringGetVal(&buffPtr, &fieldVal, PARSE_SINGLE_ARGUMENT,FSP) != SUCCESS)
                {
                    sessionPtr->status = CMD_INVALID_SYNTAX;
                    break;
                }

                switch (fieldVal)
                {
                    case FILE_XFER_STP_EOF:
                    {
                        DPRINT(NETWORK_MANAGER, "request to stop file Transfer: EOF");
                        if(sessionPtr->requestType == IMPORT_LANGUAGE)
                        {
                            snprintf(msgBuffer, MAX_FTS_MSG_SIZE, TEMP_DIR_PATH"%s", sessionPtr->fileName);
                        }
                        else
                        {
                            snprintf(msgBuffer, MAX_FTS_MSG_SIZE, RAMFS_DIR_PATH"%s", sessionPtr->fileName);
                        }

                        //Make it zero size file at init
                        infoBuf.st_size = 0;

                        // Get Information of File
                        if (stat(sessionPtr->absoluteFilePath, &infoBuf)!= STATUS_OK)
                        {
                            sessionPtr->status = CMD_PROCESS_ERROR;
                        }
                        // Match file size with Stored file size
                        else if ((UINT32)infoBuf.st_size != sessionPtr->fileSize)
                        {
                            sessionPtr->status = CMD_INVALID_FILE_LENGTH;
                            EPRINT(NETWORK_MANAGER, "unexpected end of file request");
                        }
                        // Change file name form ".part" to ".zip or .csv"
                        else if (rename(sessionPtr->absoluteFilePath, msgBuffer) != STATUS_OK)
                        {
                            sessionPtr->status = CMD_PROCESS_ERROR;
                            EPRINT(NETWORK_MANAGER, "fail to rename file: [err=%s]", STR_ERR);
                        }
                        else
                        {
                            //file received successfully
                            snprintf(sessionPtr->absoluteFilePath,MAX_FILE_PATH_LENGTH,"%s",msgBuffer);
                            sessionPtr->status = CMD_SUCCESS;
                            fileReceived = TRUE;
                        }

                        if (sessionPtr->status != CMD_SUCCESS)
                        {
                            sessionPtr->removeFileFlag = TRUE;
                        }
                    }
                    break;

                    case FILE_XFER_STP_CAN:
                    {
                        // Set Led Status to IDLE in case of cancelling FIRMWARE UPGRADE
                        DPRINT(NETWORK_MANAGER, "request to stop file Transfer: CANCEL");
                        if (sessionPtr->requestType == FIRMWARE_UPGRADE)
                        {
                            SetSystemStatusLed(SYS_FIRMWARE_UPGRADE_INPROCESS, OFF);
                        }

                        // remove File
                        sessionPtr->removeFileFlag = TRUE;
                        sessionPtr->status = CMD_SUCCESS;
                    }
                    break;

                    default:
                    {
                        sessionPtr->status = CMD_INVALID_FIELD_VALUE;
                    }
                    break;
                }
            }
            break;

            default:
            {
                sessionPtr->header = ACK_FTS;
                sessionPtr->status = CMD_INVALID_MESSAGE;
                EPRINT(NETWORK_MANAGER, "invld msg header: [header=%d]", msgHeader);
            }
            break;
		}

        if((sessionPtr->requestType == IMPORT_LANGUAGE) && (fileReceived == TRUE))
		{
            // After receiving language file first validate that is it contain '&' or '<' characters. If Yes then Reply with Status Code : 1017
            if (SUCCESS == validateForNotAllowedCharacters(sessionPtr->absoluteFilePath))
            {
                /* here file will be copied in sample file format at language resource folder.
                 * if any error occurred then relevant status code will be provided to device client. */
                DPRINT(NETWORK_MANAGER, "file imported succesfully, starting ts file conversion");
                tempStatus = CreateCustomLanguageCsvFile(sessionPtr->fileName);
                if (tempStatus != SUCCESS)
                {
                    if (tempStatus == FAIL)
                    {
                        EPRINT(NETWORK_MANAGER, "fail to translate imported language file in ts");
                        sessionPtr->status = CMD_LANG_TRANSLATION_FAILED;
                        sessionPtr->removeFileFlag = TRUE;
                    }
                    else
                    {
                        EPRINT(NETWORK_MANAGER, "invld language file for ts translation");
                        sessionPtr->status = CMD_LANG_INVALID_FILE;
                        sessionPtr->removeFileFlag = TRUE;
                    }
                }
            }
            else
            {
                sessionPtr->status = CMD_LANG_TRANSLATING_ERROR;
                sessionPtr->removeFileFlag = TRUE;
            }

            fileReceived = FALSE;
		}

		// in case of any failure in EXPORT LANGUAGE reply [RPL_FTS&status]
        if ((sessionPtr->requestType == EXPORT_LANGUAGE) && (sessionPtr->status == CMD_LANG_EXPORT_FAILED))
		{
			sessionPtr->header = RPL_FTS;
		}

		// terminate process without reply in case of error while BACK UP
        if (!((sessionPtr->requestType == BACKUP_CONFIG) && (sessionPtr->status != CMD_SUCCESS)))
		{
			sessionPtr->replyBuffer = msgBuffer;
            if (replyToClient(sessionPtr, sizeof(msgBuffer)) == FAIL)
			{
				threadExitFlag = TRUE;
			}
		}
	}

    DPRINT(NETWORK_MANAGER, "closing file transfer session: [status=%d]", sessionPtr->status);
	closeFileXferSession(sessionPtr);

	// Free Session
    FREE_MEMORY(sessionPtr);
	pthread_exit(NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function is called to check how many languages are imported. If it is equal to 5 than,
 *          send error message to client that maximum languages are already imported.
 * @return
 */
static NET_CMD_STATUS_e	checkLanguageCount(void)
{
    struct dirent	*entry;
    DIR             *dir = opendir(LANGUAGES_DIR_PATH);
    CHARPTR         subStr;
    UINT8           languageCount = 0;

	if (dir == NULL)
	{
        EPRINT(NETWORK_MANAGER, "fail to open dir: [path=%s], [err=%s]", LANGUAGES_DIR_PATH, STR_ERR);
        return CMD_PROCESS_ERROR;
	}

    while ((entry = readdir(dir)) != NULL)
	{
        if ((strcmp(entry->d_name,".") == 0) || (strcmp(entry->d_name,"..") == 0))
		{
            continue;
        }

        subStr = strstr(entry->d_name, LANG_FILE_EXTENTION);
        if (subStr != NULL)
        {
            languageCount++;
        }
	}
	closedir(dir);

	if (languageCount >= MAX_LANGUAGE)
	{
        return CMD_MAX_LANG_IMPORTED;
	}

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function is called to validate that is language file contains any unwanted characters
 *          such as '&' or '<'.
 * @param   fileName
 * @return  If invalid characters then return FAIL.
 */
static BOOL validateForNotAllowedCharacters(CHARPTR fileName)
{
    FILE    *fd;
    INT32   ch;

    fd = fopen(fileName, "r");
    if (fd == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open imported csv file: [path=%s], [err=%s]", fileName, STR_ERR);
        return FAIL;
    }

    while ((ch = fgetc(fd)) != EOF)
    {
        if ((ch == '&') || (ch == '<'))
        {
            fclose(fd);
            EPRINT(NETWORK_MANAGER, "imported language csv file contains invld charasters('&' or '<')");
            return FAIL;
        }
    }

    fclose(fd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function is called to check if requested file is already imported then treat as a modification.
 * @param   fileName
 * @return
 */
static BOOL isLanguageModifyReq(CHARPTR fileName)
{
    struct dirent   *entry;
    DIR             *dir = opendir(LANGUAGES_DIR_PATH);
    CHARPTR         subStr = NULL;

    if (dir == NULL)
    {
        EPRINT(NETWORK_MANAGER, "fail to open dir: [path=%s], [err=%s]", LANGUAGES_DIR_PATH, STR_ERR);
        return FAIL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if ((strcmp(entry->d_name,".") == 0) || (strcmp(entry->d_name,"..") == 0))
        {
            continue;
        }

        subStr = strstr(entry->d_name, LANG_FILE_EXTENTION);
        if(subStr == NULL)
        {
            continue;
        }

        if (strcmp(fileName, entry->d_name) == STATUS_OK)
        {
            DPRINT(NETWORK_MANAGER, "file already exits and will be overwritten: [path=%s]", fileName);
            closedir(dir);
            return TRUE;
        }
    }

    closedir(dir);
    return FALSE;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
