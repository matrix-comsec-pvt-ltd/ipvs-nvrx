#if !defined BACKUPMANAGER_H
#define BACKUPMANAGER_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		BackupManager.h
@brief      The Backup Management module provides mechanism for storing data from system to user
            selected section (e.g. USB, FTP etc.). This module stores file in native format.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DiskManager.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_MANUAL_BACK_RCD		10

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	BACKUP_HLT_STOP,
	BACKUP_HLT_START,
	BACKUP_HLT_COMPLETE,
	BACKUP_HLT_INCOMPLETE,
	BACKUP_HLT_DISABLE,
	BACKUP_HLT_STATUS_MAX
}BACKUP_HLT_STATUS_e;

typedef enum
{
	BACKUP_FULL,
	BACKUP_IN_PROGRESS,
	BACKUP_COMPLETE,
	BACKUP_INCOMPLETE,
	BACKUP_DISABLE,
	BACKUP_READY,
	BACKUP_DISK_NOT_ENOUGH_SPACE,
	BACKUP_STATUS_MAX
}BACKUP_DISK_STATUS_e;

typedef enum
{
	BACKUP_FORMAT_NATIVE,
	BACKUP_FORMAT_AVI,
	MAX_BACKUP_FORMAT

}BACKUP_FORMAT_e;

//-------------------------------------------------------------------------------------------------
typedef void (*NW_CMD_BKUP_RCD_CB)(NET_CMD_STATUS_e status, INT32 clientSocket, float freeSize, float recordSize,
                                   BOOL closeConn, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitBackupManager(void);
//-------------------------------------------------------------------------------------------------
void DeInitBackupManager(void);
//-------------------------------------------------------------------------------------------------
void ManualBackupCfgUpdate(MANUAL_BACKUP_CONFIG_t newCfg);
//-------------------------------------------------------------------------------------------------
void UpdateBackupStatus(DM_BACKUP_TYPE_e diskType);
//-------------------------------------------------------------------------------------------------
void StopBackupOnHddFormat(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopManualBackup(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopScheduleBackup(void);
//-------------------------------------------------------------------------------------------------
void TerminateBackup(UINT8 nasDriveNo);
//-------------------------------------------------------------------------------------------------
BOOL GetBackupStatus(UINT8 temp, UINT8 backupType);
//-------------------------------------------------------------------------------------------------
BACKUP_DISK_STATUS_e GetBackupInfo(UINT8 backupType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ManualBackUpOfRecords(SEARCH_RESULT_t *records, RECORD_ON_DISK_e recStorageDrive, UINT8 noOfRecords,
                                       BACKUP_FORMAT_e format, NW_CMD_BKUP_RCD_CB callback, INT32 connFd, CHARPTR userName,
                                       MB_LOCATION_e backupLocation, CLIENT_CB_TYPE_e callbackType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ClipBackUpOfRecords(SYNC_CLIP_DATA_t *records, RECORD_ON_DISK_e recStorageDrive, UINT8 noOfRecords,
                                     NW_CMD_BKUP_RCD_CB callback, INT32 connFd, CHARPTR userName, CLIENT_CB_TYPE_e clientCbType);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartManualBackup(struct tm startTime, struct tm endTime);
//-------------------------------------------------------------------------------------------------
BOOL GetBackupTermFlag(void);
//-------------------------------------------------------------------------------------------------
void SetBackupStopReq(DM_BACKUP_TYPE_e BackupType, BOOL Status);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif
