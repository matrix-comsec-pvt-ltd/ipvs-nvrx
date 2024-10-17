//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiskManagerComn.c
@brief      This file describes functions that will used by whole disk management functionality.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>
#include <sys/vfs.h>

/* Application Includes */
#include "ConfigApi.h"
#include "DiskManagerComn.h"
#include "DiskController.h"
#include "Utils.h"
#include "DebugLog.h"
#include "DateTime.h"
#include "EventLogger.h"
#include "RecordManager.h"
#include "DiskUtility.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define HARDDISK_MAGIC_CODE			(28573)
#define FILE_MANAGEMENT_VERSION		(0001)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    // magic code
    UINT32					magicCode;

    // version number of file management
    UINT32					versionNo;

}DISK_SIGNATURE_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void removeBlankFolder(CHARPTR path);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function was gives the size of mounted file system.
 * @param   mountPoint
 * @param   diskSize
 * @return  SUCCESS/FAIL
 */
BOOL GetSizeOfMountFs(CHARPTR mountPoint, DISK_SIZE_t *diskSize)
{
    UINT32          totalSpace, freeSpace, usedSpace;
    struct statfs64 fs;

    // Get the used, free and totalsize using statfs
    if (statfs64(mountPoint, &fs) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "statfs64 fail: [path=%s], [err=%s]", mountPoint, STR_ERR);
        return FAIL;
    }

    if (fs.f_blocks == 0)
    {
        EPRINT(DISK_MANAGER, "total memory blocks not found: [path=%s], [err=%s]", mountPoint, STR_ERR);
        return FAIL;
    }

    totalSpace = (UINT32)((UINT64)((UINT64)fs.f_bsize * (UINT64)fs.f_blocks) / (UINT32)(MEGA_BYTE));
    freeSpace  = (UINT32)((UINT64)((UINT64)fs.f_bsize * (UINT64)fs.f_bavail)  / (UINT32)(MEGA_BYTE));
    usedSpace = totalSpace - freeSpace;
    diskSize->totalSize = totalSpace;
    diskSize->freeSize = freeSpace;
    diskSize->usedSize = usedSpace;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives size of directory. For that it will traverse through all the files
 *          in folder and get the size of that file this will sum up into final result.
 * @param   dirName
 * @param   dirSize
 * @return  SUCCESS/FAIL
 */
BOOL GetFolderSize(CHARPTR dirName, UINT64PTR dirSize)
{
    DIR 			*dir;
    struct dirent 	*entry;
    struct stat 	tFileInfo;
    CHAR			filename[MAX_FILE_NAME_SIZE];
    UINT64			totalSize = 0;
    INT32           fileFd = INVALID_FILE_FD;

    dir = opendir(dirName);
    if(dir == NULL)
    {
        *dirSize = 0;
        return FAIL;
    }

    while((entry = readdir(dir)) != NULL)
    {
        /* Skip system files */
        if ((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
        {
            continue;
        }

        // check this file was stream file
        snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s", dirName, entry->d_name);
        fileFd = open(filename, READ_ONLY_MODE);
        if (fileFd == INVALID_FILE_FD)
        {
            EPRINT(DISK_MANAGER, "fail to open file: [path=%s], [err=%s]", filename, STR_ERR);
            closedir(dir);
            return FAIL;
        }

        if (STATUS_OK != fstat(fileFd, &tFileInfo))
        {
            EPRINT(DISK_MANAGER, "fail to get file size: [path=%s], [err=%s]", filename, STR_ERR);
            close(fileFd);
            closedir(dir);
            return FAIL;
        }

        totalSize += tFileInfo.st_size;
        close(fileFd);
    }

    closedir(dir);
    *dirSize = totalSize;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes recovery folder because of recovery was failed.
 * @param   dirPath
 * @return  SUCCESS/FAIL
 */
BOOL RemoveDirectory(CHARPTR dirPath)
{
    DIR 			*dir;
    struct dirent 	*entry;
    CHAR			filename[MAX_FILE_NAME_SIZE];

    dir = opendir(dirPath);
    if(dir == NULL)
    {
        EPRINT(DISK_MANAGER, "fail to open dir: [path=%s], [err=%s]", dirPath, STR_ERR);
        return FAIL;
    }

    while((entry = readdir(dir)) != NULL)
    {
        // These are system file and that can not removed
        if ((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
        {
            continue;
        }

        // check this file was stream file
        snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s", dirPath, entry->d_name);
        if(entry->d_type == DT_REG)
        {
            if(unlink(filename) != STATUS_OK)
            {
                EPRINT(DISK_MANAGER, "fail to remove file: [path=%s], [err=%s]", filename, STR_ERR);
                closedir(dir);
                removeBlankFolder(dirPath);
                return FAIL;
            }
        }
        else if(entry->d_type == DT_DIR)
        {
            // Call Recursive
            snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s/", dirPath, entry->d_name);
            if(RemoveDirectory(filename) == FAIL)
            {
                EPRINT(DISK_MANAGER, "fail to remove dir: [path=%s], [err=%s]", filename, STR_ERR);
                closedir(dir);
                removeBlankFolder(dirPath);
                return FAIL;
            }
        }
    }

    closedir(dir);
    removeBlankFolder(dirPath);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function removes folder which is blank in whole path.
 * @param   path
 */
static void removeBlankFolder(CHARPTR path)
{
    CHAR    folder[MAX_FILE_NAME_SIZE];
    CHARPTR subtoken;

    snprintf(folder, MAX_FILE_NAME_SIZE, "%s", path);
    while(TRUE)
    {
        subtoken = strrchr(folder, '/');
        if (subtoken == NULL)
        {
            EPRINT(DISK_MANAGER, "no delemeter found: [path=%s]", folder);
            return;
        }

        if (strstr(subtoken, CHANNEL_NAME_REC) != NULL)
        {
            return;
        }

        folder[strlen(folder)-strlen(subtoken)] = '\0';
        if(rmdir(folder) != STATUS_OK)
        {
            return;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This is file access config notify function.
 * @param   newFileAccessCfg
 * @param   oldFileAccessCfg
 */
void DmFileAccessCfgUpdate(FILE_ACCESS_CONFIG_t newFileAccessCfg, FILE_ACCESS_CONFIG_t *oldFileAccessCfg)
{
    if(oldFileAccessCfg->ftpAccess != newFileAccessCfg.ftpAccess)
    {
        if(oldFileAccessCfg->ftpport != newFileAccessCfg.ftpport)
        {
            FtpServiceStartStop(newFileAccessCfg.ftpport, newFileAccessCfg.ftpAccess);
        }
        else
        {
            FtpServiceStartStop(newFileAccessCfg.ftpport, newFileAccessCfg.ftpAccess);
        }
    }
    else if(oldFileAccessCfg->ftpport != newFileAccessCfg.ftpport)
    {
        FtpServiceStartStop(newFileAccessCfg.ftpport, newFileAccessCfg.ftpAccess);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates a signature file and also writes a signature into file.
 * @param   mountPoint
 * @return  SUCCESS/FAIL
 */
BOOL VerifyHddSignature(CHARPTR	mountPoint)
{
    CHAR				filename[MAX_FILE_NAME_SIZE];
    CHAR				recordFolder[MAX_FILE_NAME_SIZE];
    CHAR				recoveryFolder[MAX_FILE_NAME_SIZE];
    INT32				fileFd;
    INT32				readCnt;
    DISK_SIGNATURE_t	diskSignature;

    if(NULL == mountPoint)
    {
        return FAIL;
    }

    CreateCoredumpScript(mountPoint);

    // check first file was present
    snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s", mountPoint, HARDDISK_SIGNATURE_FILE);
    if(access(filename, F_OK) != STATUS_OK)
    {
        EPRINT(DISK_MANAGER, "hdd signature file not present: [path=%s]", filename);
        return FAIL;
    }

    fileFd = open(filename, READ_ONLY_MODE, FILE_PERMISSION);
    if(fileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open hdd signature file: [path=%s], [err=%s]", filename, STR_ERR);
        return FAIL;
    }

    readCnt = read(fileFd, &diskSignature, sizeof(DISK_SIGNATURE_t));
    if(readCnt != (INT32)sizeof(DISK_SIGNATURE_t))
    {
        EPRINT(DISK_MANAGER, "fail to read hdd signature file: [path=%s], [err=%s]", filename, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    if ((diskSignature.magicCode != HARDDISK_MAGIC_CODE) || (diskSignature.versionNo != FILE_MANAGEMENT_VERSION))
    {
        // signatur was not proper
        EPRINT(DISK_MANAGER, "invld hdd signature: [path=%s], [magicCode=%d], [versionNo=%d]", mountPoint, diskSignature.magicCode, diskSignature.versionNo);
        close(fileFd);
        return FAIL;
    }

    //check record folder was present
    snprintf(recordFolder, MAX_FILE_NAME_SIZE, "%s", mountPoint);
    if(access(recordFolder, F_OK) != STATUS_OK)
    {
        mkdir(recordFolder, FOLDER_PERMISSION);
    }

    //check recovery folder was present
    snprintf(recoveryFolder, MAX_FILE_NAME_SIZE, RECOVERY_FOLDER, mountPoint);
    if(access(recoveryFolder, F_OK) != STATUS_OK)
    {
        mkdir(recoveryFolder, FOLDER_PERMISSION);
    }

    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function creates a signature file and also writes a signature into file.
 * @param   mountPoint
 * @return  SUCCESS/FAIL
 */
BOOL WriteHddSignature(CHARPTR mountPoint)
{
    BOOL				retVal = FAIL;
    INT32				fileFd;
    INT32				writeCnt;
    CHAR				filename[MAX_FILE_NAME_SIZE];
    CHAR				recordFolder[MAX_FILE_NAME_SIZE];
    CHAR				recoveryFolder[MAX_FILE_NAME_SIZE];
    DISK_SIGNATURE_t	diskSignature;

    if(NULL == mountPoint)
    {
        return retVal;
    }

    CreateCoredumpScript(mountPoint);

    snprintf(filename, MAX_FILE_NAME_SIZE, "%s%s", mountPoint, HARDDISK_SIGNATURE_FILE);
    fileFd = open(filename, CREATE_RDWR_MODE, FILE_PERMISSION);
    if(fileFd == INVALID_FILE_FD)
    {
        EPRINT(DISK_MANAGER, "fail to open hdd signature file: [path=%s], [err=%s]", filename, STR_ERR);
        return FAIL;
    }

    diskSignature.magicCode = HARDDISK_MAGIC_CODE;
    diskSignature.versionNo = FILE_MANAGEMENT_VERSION;

    writeCnt = write(fileFd, &diskSignature, sizeof(DISK_SIGNATURE_t));
    if(writeCnt != (INT32)sizeof(DISK_SIGNATURE_t))
    {
        EPRINT(DISK_MANAGER, "fail to write hdd signature file: [path=%s], [err=%s]", filename, STR_ERR);
        close(fileFd);
        return FAIL;
    }

    //check record folder was present
    snprintf(recordFolder, MAX_FILE_NAME_SIZE,  "%s", mountPoint);
    if(access(recordFolder, F_OK) != STATUS_OK)
    {
        mkdir(recordFolder, FOLDER_PERMISSION);
    }

    //check recovery folder was present
    snprintf(recoveryFolder, MAX_FILE_NAME_SIZE, RECOVERY_FOLDER, mountPoint);
    if(access(recoveryFolder,F_OK) != STATUS_OK)
    {
        mkdir(recoveryFolder, FOLDER_PERMISSION);
    }

    close(fileFd);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks file access config and start/stop smb and ftp.
 * @param   status
 */
void UpdateServices(BOOL status)
{
#if !defined(RK3588_NVRH)
    FILE_ACCESS_CONFIG_t fileAcessConfig;

    ReadFileAccessConfig(&fileAcessConfig);

    /* update service only if service is enable */
    if(fileAcessConfig.ftpAccess == ENABLE)
    {
        FtpServiceStartStop(fileAcessConfig.ftpport, status);
        if (status == STOP)
        {
            /* Sleep is needed for daemon to stop */
            sleep(2);
        }
    }
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   CreateRecursiveDir
 * @param   path
 * @param   pErrorCode - Error code pointer, meaningful only when return value is -1
 * @return  SUCCESS/FAIL
 */
BOOL CreateRecursiveDir(CHARPTR path, UINT32PTR pErrorCode)
{
    CHARPTR			tmpPath;
    CHAR 			dir[100];
    UINT8	 		strCnt = 1;
    UINT32			pathLen;
    struct stat     tFileInfoBuff;

    /* set default error code */
    SET_ERROR_NUMBER(pErrorCode, INVALID_ERROR_CODE);

    do
    {
        tmpPath = strchr((path + strCnt), '/');
        if(tmpPath != NULL)
        {
            pathLen = (strlen(path) - strlen(tmpPath));
            snprintf(dir, pathLen < sizeof(dir) ? pathLen+1 : sizeof(dir), "%s", path);

            if(stat(dir, &tFileInfoBuff) != STATUS_OK)
            {
                if(Utils_Mkdir(dir, FOLDER_PERMISSION, pErrorCode) != STATUS_OK)
                {
                    return FAIL;
                }
            }
            strCnt = strlen(dir) + 1;
        }
        else
        {
            if(stat(path, &tFileInfoBuff) != STATUS_OK)
            {
                if(Utils_Mkdir(path, FOLDER_PERMISSION, pErrorCode) != STATUS_OK)
                {
                    return FAIL;
                }
            }
        }

    } while(tmpPath != NULL);

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives only name of disk (HDD0, HDD1, RAID, NDD0, NDD1) from disk.
 * @param   srcStr
 * @param   destStr
 * @return  SUCCESS/FAIL
 */
BOOL GetDiskNameFromMountPoint(CHARPTR srcStr, CHARPTR destStr)
{
    BOOL		retVal = SUCCESS;
    UINT8		cnt;
    CHAR		tmpStr[MAX_FILE_NAME_SIZE];
    CHARPTR 	subtoken, src, savePtr;

    snprintf(tmpStr, MAX_FILE_NAME_SIZE, "%s", srcStr);

    for (src = tmpStr, cnt = 0; cnt < 3; src = NULL, cnt++)
    {
        subtoken = strtok_r(src, "/", &savePtr);
        if (subtoken == NULL)
        {
            return FAIL;
        }

        snprintf(destStr, MAX_FILE_NAME_SIZE, "%s", subtoken);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives only name of disk (HDD0, HDD1, RAID, NDD0, NDD1) and Camera file
 *          name from input file name
 * @param   srcStr
 * @param   destStr
 * @return  SUCCESS/FAIL
 */
BOOL ParseRemoteStrmFileName(CHARPTR srcStr, CHARPTR destStr)
{
    CHAR	tmpDiskName[MAX_FILE_NAME_SIZE];
    CHARPTR	tmpResPtr;

    if(GetDiskNameFromMountPoint(srcStr, tmpDiskName) == FAIL)
    {
        return FAIL;
    }

    tmpResPtr = strstr(srcStr, CHANNEL_NAME_REC);
    if(tmpResPtr == NULL)
    {
        return FAIL;
    }

    snprintf(destStr, MAX_FILE_NAME_SIZE, "%s/%s", tmpDiskName, tmpResPtr);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function used to stop recording, backup, and playback while performing hard disk
 *          related functionality and also it provides blocking call.
 * @param   action
 */
void PreFormatDiskFunc(DISK_ACT_e action)
{
    NOTIFY_PRE_FORMAT_HDD(action);

    /* Wait for channel writer to exit */
    ChannelWriterExitWait();

    while(GetAviWritterStatus() == FAIL)
    {
        sleep(1);
        DPRINT(DISK_MANAGER, "wait for avi writter thread to complete");
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives hard disk configuration from disk name.
 * @param   diskName
 * @param   hddConfig
 * @return
 */
BOOL GetHddConfigFromDiskName(CHARPTR diskName, HDD_CONFIG_t *hddConfig)
{
    UINT8	cnt;
    CHAR	driveName[MOUNT_POINT_SIZE];

    if ((hddConfig == NULL) || (diskName == NULL))
    {
        return FAIL;
    }

    if(strstr(diskName, SINGLE_DISK) != NULL)
    {
        for(cnt = 0; cnt < getMaxHardDiskForCurrentVariant(); cnt++)
        {
            snprintf(driveName, MOUNT_POINT_SIZE, SINGLE_DISK"%d", (cnt + 1));
            if(strstr(diskName, driveName) != NULL)
            {
                hddConfig->mode = SINGLE_DISK_VOLUME;
                hddConfig->recordDisk = LOCAL_HARD_DISK;
                return SUCCESS;
            }
        }
    }
    else if(strstr(diskName, RAID_DISK) != NULL)
    {
        ReadHddConfig(hddConfig);
        hddConfig->recordDisk = LOCAL_HARD_DISK;
        return SUCCESS;
    }
    else if(strstr(diskName, NDD_DRIVE) != NULL)
    {
        for(cnt = 0; cnt < MAX_NW_DRIVE; cnt++)
        {
            snprintf(driveName, MOUNT_POINT_SIZE, NDD_DRIVE"%d", (cnt + 1));
            if(strstr(diskName, driveName) != NULL)
            {
                hddConfig->mode = SINGLE_DISK_VOLUME;
                hddConfig->recordDisk = (cnt + REMOTE_NAS1);
                return SUCCESS;
            }
        }
    }

    ReadHddConfig(hddConfig);
    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function enters data into one file of Hard disk. The folder which is not removed
 *          by system, its entry is update in that file.
 * @param   folderName
 * @return
 */
BOOL EntryOldestDeleteFile(CHARPTR folderName)
{
    BOOL            retVal = FAIL;
    BOOL            doNotDeletFldr = TRUE;
    CHAR            mountPoint[MOUNT_POINT_SIZE];
    CHAR            fileName[MAX_FILE_NAME_SIZE];
    CHAR            delemName[MAX_FILE_NAME_SIZE];
    CHAR            data[MAX_FILE_NAME_SIZE];
    CHAR            subDir[200];
    CHARPTR         temp;
    UINT32          cnt;
    FILE            *fileFd;
    CHARPTR         subtoken;
    DIR             *dir;
    struct dirent   *entry;

    temp = strstr(folderName, CHANNEL_NAME_REC);
    if(temp != NULL)
    {
        UINT8 nameLen = strlen(folderName) - strlen(temp);
        snprintf(mountPoint, nameLen < MOUNT_POINT_SIZE ? nameLen+1 : MOUNT_POINT_SIZE, "%s", folderName);
        if(strstr(mountPoint, MEDIA_DIR_PATH) != NULL)
        {
            DPRINT(DISK_MANAGER, "media mount point: [path=%s]", mountPoint);
            snprintf(fileName, MAX_FILE_NAME_SIZE, "%s%s", mountPoint, OLDEST_CORRUPT_FILE);
            fileFd = fopen(fileName, "a");
            if(fileFd != NULL)
            {
                snprintf(data, MAX_FILE_NAME_SIZE, "%s\n", folderName);
                cnt = fwrite(data, 1, strlen(data), fileFd);
                if(cnt != strlen(data))
                {
                    EPRINT(DISK_MANAGER, "fail to write file: [path=%s], [err=%s]", fileName, STR_ERR);
                }

                fclose(fileFd);
                retVal = SUCCESS;
            }
        }
    }

    snprintf(fileName, MAX_FILE_NAME_SIZE, "%s%s", folderName, SKIPP_FILE);
    fileFd = fopen(fileName, "a");
    if(fileFd != NULL)
    {
        snprintf(data, MAX_FILE_NAME_SIZE, FILE_NOT_REMOVED_STR);
        cnt = fwrite(data, 1, strlen(data), fileFd);
        if(cnt != strlen(data))
        {
            EPRINT(DISK_MANAGER, "fail to write file: [path=%s], [err=%s]", fileName, STR_ERR);
        }

        fclose(fileFd);
    }

    snprintf(delemName, MAX_FILE_NAME_SIZE, "%s", folderName);
    for(cnt=0; cnt < 2; cnt++)
    {
        subtoken = strrchr(delemName, '/');
        if (subtoken == NULL)
        {
            EPRINT(DISK_MANAGER, "no delemeter found: [path=%s]", delemName);
            break;
        }
        else
        {
            delemName[strlen(delemName)-strlen(subtoken)] = '\0';
        }
    }

    dir = opendir(delemName);
    if(dir != NULL)
    {
        doNotDeletFldr = TRUE;

        while((entry = readdir(dir)) != NULL)
        {
            if ((strcmp(entry->d_name, ".") == STATUS_OK) || (strcmp(entry->d_name, "..") == STATUS_OK))
            {
                continue;
            }

            snprintf(subDir, sizeof(subDir), "%s/%s/%s", delemName, entry->d_name, SKIPP_FILE);
            if(access(subDir, F_OK) != STATUS_OK)
            {
                doNotDeletFldr = FALSE;
                break;
            }
        }
        closedir(dir);
    }

    if(doNotDeletFldr == TRUE)
    {
        DPRINT(DISK_MANAGER, "entry made for date: [path=%s]", delemName);
        snprintf(fileName, MAX_FILE_NAME_SIZE, "%s/%s", delemName, SKIPP_FILE);
        fileFd = fopen(fileName, "a");
        if(fileFd != NULL)
        {
            snprintf(data, MAX_FILE_NAME_SIZE, FILE_NOT_REMOVED_STR);
            cnt = fwrite(data, 1, strlen(data), fileFd);
            if(cnt != strlen(data))
            {
                EPRINT(DISK_MANAGER, "fail to write file: [path=%s], [err=%s]", fileName, STR_ERR);
            }

            fclose(fileFd);
        }
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function checks whether folderName entry is present in skip files.
 * @param   folderName
 * @return
 */
BOOL CheckSkipOldestFolder(CHARPTR folderName)
{
    CHAR 		mountPoint[MOUNT_POINT_SIZE];
    CHAR 		fileName[MAX_FILE_NAME_SIZE];
    CHAR 		data[MAX_FILE_NAME_SIZE];
    CHARPTR		temp;
    FILE		*fileFd;

    temp = strstr(folderName, CHANNEL_NAME_REC);
    if(temp == NULL)
    {
        return FAIL;
    }

    UINT8 nameLen = strlen(folderName) - strlen(temp);
    snprintf(mountPoint, nameLen < MOUNT_POINT_SIZE ? nameLen+1 : MOUNT_POINT_SIZE, "%s", folderName);
    snprintf(fileName, MAX_FILE_NAME_SIZE, "%s%s", mountPoint, OLDEST_CORRUPT_FILE);
    /* PARASOFT : No need to validate file path */
    fileFd = fopen(fileName, "r");
    if(fileFd == NULL)
    {
        return FAIL;
    }

    while(fgets(data, MAX_FILE_NAME_SIZE, fileFd) != NULL)
    {
        data[strlen(data) - 1] = '\0';

        // (SD-2851)Changed from strncmp to strcmp to match exact name
        if(strcmp(data, folderName) == STATUS_OK)
        {
            DPRINT(DISK_MANAGER, "oldest folder should skip: [path=%s]", folderName);
            fclose(fileFd);
            return SUCCESS;
        }
    }

    fclose(fileFd);
    return FAIL;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
