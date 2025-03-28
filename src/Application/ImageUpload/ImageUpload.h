#if !defined IMAGEUPLOAD_H
#define IMAGEUPLOAD_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		ImageUpload.h
@brief      The Image Uploader module provides mechanism to retrieve snapshot images (jpg) from
            specified camera and send it (upload) to FTP server and/or via email. The images are
            uploaded at user configurable interval. APIs are provided to start/stop image uploading.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	IMAGE_UPLOAD_PENDING,
	IMAGE_UPLOAD_SUCCESS,
	IMAGE_UPLOAD_FAIL,
}TEST_UPLOAD_STATUS_e;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitImageUpload(void);
//-------------------------------------------------------------------------------------------------
void DeinitImageUpload(void);
//-------------------------------------------------------------------------------------------------
BOOL StartImageUpload(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
BOOL StopImageUpload(UINT8 channelNo);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e UploadSnapshot(UINT8 channelNo, NW_CMD_REPLY_CB callback, INT32 connFd);
//-------------------------------------------------------------------------------------------------
BOOL UploadCosecSnapshot(UINT8 channelNo,VOIDPTR cosecInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // IMAGEUPLOAD_H
