#if !defined FTPCLIENT_H
#define FTPCLIENT_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file       FtpClient.h
@brief      The module provides communication functionalities with FTP server. It implements FTP
            Client that connects to FTP server and performs data transfer. APIs are provided for
            upload and download of a file to/from specified FTP Server. It can handle multiple
            concurrent FTP requests. The result of specific request was given in callback of request.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define FTP_FILE_NAME_SIZE		600
#define FTP_REMOTE_PATH_LEN		512
#define INVALID_FTP_HANDLE 		255

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef UINT8       FTP_HANDLE;

//Gives information of what are the information needed to upload files on ftp server.
typedef struct
{
    // local filename with absolute path which is to be upload to or download from FTP server
	CHAR			localFileName[FTP_FILE_NAME_SIZE];

    // remote filename with absolute path which is to be upload to or download from FTP server
	CHAR			remoteFile[FTP_REMOTE_PATH_LEN];

	// FTP Server
	FTP_SERVER_e 	ftpServer;

}FTP_FILE_INFO_t;

// Response indicating result of FTP transfer.
typedef enum
{
	FTP_SUCCESS,			// FTP Transfer successfully
	FTP_CONNECTION_ERROR,	// Connection error while connecting to server
	FTP_AUTH_ERROR,			// Authorization to server failed.
	FTP_USER_ABORTED,		// user has aborted ftp transfer.
	FTP_WRITE_PROTECTED,	// user has no permission to change to remote directory.
    FTP_FAIL,				// Failed due to other reasons
    FTP_QUEUE_FULL,         // Internal FTP queue full
    FTP_STATUS_MAX

}FTP_RESPONSE_e;

typedef enum
{
	FTP_SYS_UNIX,
	FTP_SYS_WINDOWS_NT,
	MAX_FTP_SYS

}FTP_SYS_e;

//-------------------------------------------------------------------------------------------------
typedef void (*FTP_CALLBACK)(FTP_HANDLE ftpHandle, FTP_RESPONSE_e response, UINT16 userData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
BOOL InitFtpClient(void);
//-------------------------------------------------------------------------------------------------
BOOL DeinitFtpClient(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartFtpUpload(FTP_FILE_INFO_t * fileInfo, FTP_CALLBACK ftpCallback, UINT16 userData, FTP_HANDLE * ftpHandle);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e FtpImageUpload(UINT8 camIndex, FTP_FILE_INFO_t * fileInfo);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartFirmwareDownload(FTP_UPLOAD_CONFIG_t *ftpServerCfg, FTP_FILE_INFO_t *fileInfo, FTP_CALLBACK ftpCallback, UINT16 userData);
//-------------------------------------------------------------------------------------------------
BOOL StopFtpTransfer(FTP_HANDLE ftpHandle);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e TestFtpConn(FTP_UPLOAD_CONFIG_t * ftpDataPtr, NW_CMD_REPLY_CB callback, INT32 connFd);
//-------------------------------------------------------------------------------------------------
FTP_RESPONSE_e ListFromFtp(FTP_UPLOAD_CONFIG_t *ftpCfg, FTP_SYS_e sysType, BOOL isMatrixRecDir, CHARPTR path, VOIDPTR userData,
                           BOOL (*callback)(CHARPTR list, UINT32 size, VOIDPTR userData));
//-------------------------------------------------------------------------------------------------
FTP_RESPONSE_e DeleteFromFtp(FTP_UPLOAD_CONFIG_t *ftpCfg, FTP_SYS_e sysType, CHARPTR path, BOOL isDir, BOOL isEmpty);
//-------------------------------------------------------------------------------------------------
FTP_RESPONSE_e GetFtpServerSystemType(FTP_UPLOAD_CONFIG_t *ftpCfg, FTP_SYS_e *sysType);
//-------------------------------------------------------------------------------------------------
const CHARPTR GetFtpStatusStr(FTP_RESPONSE_e ftpStatus);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // IMAGEUPLOAD_H
