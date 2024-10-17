//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DoorCommand.c
@brief      This file contains functions to process the commands for Cosec integration
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Utils.h"
#include "DebugLog.h"
#include "DoorCommand.h"
#include "RecordManager.h"
#include "ImageUpload.h"
#include "FtpClient.h"
#include "SmtpClient.h"

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
COSEC_INTEGRATION_t cosecInfoVideoPopup;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR cosecCmdStr[MAX_COSEC_CMD]	=
{
    "SRT_MAN_REC",
    "UPLD_SNPSHT",
    "EMAIL_SNPSHT",
    "VIDEO_POP_UP",
    "CALLPRESET"
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void startCosecRecord(COSEC_INTEGRATION_t *pCosecInfo);
//-------------------------------------------------------------------------------------------------
static void startCosecImgUpload(COSEC_INTEGRATION_t *pCosecInfo);
//-------------------------------------------------------------------------------------------------
static void setCosecCameraPreset(COSEC_INTEGRATION_t *pCosecInfo);
//-------------------------------------------------------------------------------------------------
static void sendCosecVideoPopup(COSEC_INTEGRATION_t *pCosecInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API process Door command
 * @param   commandStrPtr
 * @return  SUCCESS / FAIL
 */
BOOL ProcessDoorCommand(CHARPTR commandStrPtr)
{
#if !defined(OEM_JCI)
    CHAR                tempStr[MAX_COMMAND_ARG_LEN];
    UINT64              tempData[3];
    COSEC_INTEGRATION_t cosecInfo = {0};
    GENERAL_CONFIG_t    generalConfig;

    /* Read cosec integration configuration and check the status */
    ReadGeneralConfig(&generalConfig);
    if (generalConfig.integrateCosec == FALSE)
    {
        /* Cosec integration is disabled */
        WPRINT(NETWORK_MANAGER, "cosec integration is disabled");
        return FAIL;
    }

    if (ParseStr(&commandStrPtr, FSP, tempStr, MAX_COMMAND_ARG_LEN) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door smart code");
        return FAIL;
    }

    if (strcmp(tempStr, SMART_CODE_COSEC) != STATUS_OK)
    {
        EPRINT(NETWORK_MANAGER, "invld cosec door smart code: [code=%s]", tempStr);
        return FAIL;
    }

    if(ParseStringGetVal(&commandStrPtr, tempData, 3, FSP) == FAIL)
	{
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door info");
        return FAIL;
    }

    cosecInfo.doorType = (UINT8)tempData[0];
    cosecInfo.doorMid = (UINT32)tempData[1];
    cosecInfo.doorDid = (UINT8)tempData[2];

    if(ParseStr(&commandStrPtr, FSP, cosecInfo.doorName, MAX_COSEC_DOORNAME_LEN) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door name");
        return FAIL;
    }

    if(ParseStr(&commandStrPtr, FSP, tempStr, MAX_COMMAND_ARG_LEN) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door cmd");
        return FAIL;
    }

    cosecInfo.doorCommand = ConvertStringToIndex(tempStr, cosecCmdStr, MAX_COSEC_CMD);
    if(cosecInfo.doorCommand >= MAX_COSEC_CMD)
    {
        EPRINT(NETWORK_MANAGER, "invld cosec door cmd found: [doorCommand=%d]", cosecInfo.doorCommand);
        return FAIL;
    }

    if(ParseStringGetVal(&commandStrPtr, tempData, 2, FSP) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door cmd data: [doorCommand=%d]", cosecInfo.doorCommand);
        return FAIL;
    }

    cosecInfo.cameraMask.bitMask[0] = tempData[0];
    cosecInfo.cameraMask.bitMask[1] = 0;
    cosecInfo.doorCmdData = (UINT32)tempData[1];

    if (ParseStr(&commandStrPtr, FSP, cosecInfo.userId, MAX_COSEC_USERID_LEN) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door cmd user id: [doorCommand=%d]", cosecInfo.doorCommand);
        return FAIL;
    }

    if (ParseStr(&commandStrPtr, FSP, cosecInfo.userName, MAX_COSEC_USERNAME_LEN) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door cmd username: [doorCommand=%d]", cosecInfo.doorCommand);
        return FAIL;
    }

    if (ParseStringGetVal(&commandStrPtr, tempData, 1, FSP) == FAIL)
    {
        EPRINT(NETWORK_MANAGER, "fail to parse cosec door cmd event code: [doorCommand=%d]", cosecInfo.doorCommand);
        return FAIL;
    }

    cosecInfo.eventCode = (COSEC_DOOR_EVENT_e)tempData[0];
    if(cosecInfo.eventCode >= COSEC_DOOR_EVENT_MAX)
    {
        EPRINT(NETWORK_MANAGER, "invld cosec door cmd event found: [doorCommand=%d], [eventCode=%d]", cosecInfo.doorCommand, cosecInfo.eventCode);
        return FAIL;
    }

    DPRINT(NETWORK_MANAGER, "cosec door msg: [name=%s], [cmd=%s], [type=%d], [mid=%d], [did=%d], [cameraMask1=0x%llx], [cameraMask2=0x%llx], [userName=%s], [userId=%s], [data=%d], [eventCode=%d]",
           cosecInfo.doorName, cosecCmdStr[cosecInfo.doorCommand], cosecInfo.doorType, cosecInfo.doorMid,
            cosecInfo.doorDid, cosecInfo.cameraMask.bitMask[0], cosecInfo.cameraMask.bitMask[1], cosecInfo.userName, cosecInfo.userId, cosecInfo.doorCmdData, cosecInfo.eventCode);

    switch(cosecInfo.doorCommand)
    {
        case COSEC_REC:
            startCosecRecord(&cosecInfo);
            break;

        case COSEC_IMAGE_UPLD:
            startCosecImgUpload(&cosecInfo);
            break;

        case COSEC_EMAIL_NOTF:
            if(ParseStr(&commandStrPtr, FSP, cosecInfo.emailId, MAX_COSEC_EMAIL_LEN) == FAIL)
            {
                EPRINT(NETWORK_MANAGER, "fail to parse door cmd email id: [doorCommand=%d]", cosecInfo.doorCommand);
                return FAIL;
            }

            startCosecImgUpload(&cosecInfo);
            break;

        case COSEC_POPUP:
            if(ParseStringGetVal(&commandStrPtr, tempData, 1, FSP) == FAIL)
            {
                EPRINT(NETWORK_MANAGER, "fail to parse door cmd date-time: [doorCommand=%d]", cosecInfo.doorCommand);
                return FAIL;
            }

            cosecInfo.dateTimeSec = (UINT32)tempData[0];
            sendCosecVideoPopup(&cosecInfo);
            break;

        case COSEC_PRESET_POS:
            setCosecCameraPreset(&cosecInfo);
            break;

        default:
            return FAIL;
    }
#endif
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will start cosec recording
 * @param   pCosecInfo
 */
static void startCosecRecord(COSEC_INTEGRATION_t *pCosecInfo)
{
    UINT8   camIndex;
    CHAR    cosecEventDetails[MAX_EVENT_ADVANCE_DETAIL_SIZE];

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
	{
        if (GET_CAMERA_MASK_BIT(pCosecInfo->cameraMask, camIndex) == 0)
		{
            continue;
        }

        if (pCosecInfo->userName[0] != '\0')
        {
            snprintf(cosecEventDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Username: %s", pCosecInfo->userName);
        }
        else if (pCosecInfo->userId[0] != '\0')
        {
            snprintf(cosecEventDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "UserID: %s", pCosecInfo->userId);
        }
        else if (pCosecInfo->doorName[0] != '\0')
        {
            snprintf(cosecEventDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "DeviceName: %s", pCosecInfo->doorName);
        }
        else
        {
            snprintf(cosecEventDetails, MAX_EVENT_ADVANCE_DETAIL_SIZE, "Event: %02d", pCosecInfo->eventCode);
        }

        /* Start manual cosec recording */
        StartRecord(camIndex, COSEC_RECORD, pCosecInfo->doorCmdData, cosecEventDetails);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will start cosec image upload
 * @param   pCosecInfo
 */
static void startCosecImgUpload(COSEC_INTEGRATION_t *pCosecInfo)
{
    UINT8 camIndex;

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
	{
        if (GET_CAMERA_MASK_BIT(pCosecInfo->cameraMask, camIndex) == 0)
        {
            continue;
        }

        UploadCosecSnapshot(camIndex, pCosecInfo);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will set ip camera PTZ preset
 * @param   pCosecInfo
 */
static void setCosecCameraPreset(COSEC_INTEGRATION_t *pCosecInfo)
{
    UINT8 	camIndex;

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
	{
        if (GET_CAMERA_MASK_BIT(pCosecInfo->cameraMask, camIndex) == 0)
        {
            continue;
        }

        GotoPtzPosition(camIndex, (UINT8)pCosecInfo->doorCmdData, NULL, 0, FALSE);
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will construct the message to display on screen
 * @param   pCosecInfo
 */
static void sendCosecVideoPopup(COSEC_INTEGRATION_t *pCosecInfo)
{
    UINT8 	camIndex;
	CHAR 	detail[MAX_EVENT_DETAIL_SIZE];

    for(camIndex = 0; camIndex < getMaxCameraForCurrentVariant(); camIndex++)
	{
        if (GET_CAMERA_MASK_BIT(pCosecInfo->cameraMask, camIndex) == 0)
        {
            continue;
        }

        // Detail = Camera No/User Name/Pop-up Duration -> xx/xxxxxxxxxxxxxxxx/xxxxx Adv Detail = DoorName
        snprintf(detail, MAX_EVENT_DETAIL_SIZE-1, "%02d/%s/%d/%s", GET_CAMERA_NO(camIndex), pCosecInfo->userName, pCosecInfo->doorCmdData, pCosecInfo->userId);
        WriteEvent(LOG_COSEC_EVENT, LOG_COSEC_VIDEO_POP_UP, detail, pCosecInfo->doorName, (LOG_EVENT_STATE_e)pCosecInfo->eventCode);
        cosecInfoVideoPopup = *pCosecInfo;
        break;
	}
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
