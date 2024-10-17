//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DiCommandHandle.c
@brief      This module provides command handling to communicate with SAMAS
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DeviceInitiation.h"
#include "DiCommandHandle.h"
#include "NetworkController.h"
#include "Utils.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define DEF_VALUE 					(0)
#define LOGIN_RESERVED_FOUR_BYTES 	"0000"
#define ADD_POLL_DURATION_BUF(x)    (x+2)

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL diParseLoginCmdResp(CHARPTR *msgPtr, VOIDPTR respData);
//-------------------------------------------------------------------------------------------------
static BOOL diParsePollCmdResp(CHARPTR *msgPtr, VOIDPTR respData);
//-------------------------------------------------------------------------------------------------
static void diLoginCmd(CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data);
//-------------------------------------------------------------------------------------------------
static void diLonPollCmd(CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data);
//-------------------------------------------------------------------------------------------------
static void diSendDid(CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const CHARPTR diCmdHeader [MAX_DI_CMD] =
{
    "REG_LOG",
    "REQ_POL",
    "RSP_PRT",
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   DiMakeXXXCmd
 * @param   cmdId
 * @param   msgBuf
 * @param   cmdOutLen
 * @param   data
 * @return
 */
BOOL DiMakeXXXCmd(DI_CLIENT_CMD_e cmdId, CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data)
{
	switch(cmdId)
	{
        case LOGIN_CMD:
            diLoginCmd(msgBuf, cmdOutLen, data);
            return SUCCESS;

        case LONG_POLL:
            diLonPollCmd(msgBuf, cmdOutLen, data);
            return SUCCESS;

        case SEND_DID:
            diSendDid(msgBuf, cmdOutLen, data);
            return SUCCESS;

        default :
            return FAIL;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   diLoginCmd
 * @param   msgBuf
 * @param   cmdOutLen
 * @param   data
 */
static void diLoginCmd(CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data)
{
    CHAR                packedMacAddr[MAX_MAC_ADDRESS_WIDTH];
    USER_GROUP_e        defGroup = ADMIN;
    NVR_DEVICE_INFO_t   deviceInfo;

    /* Get device information for device initiation login */
    GetNvrDeviceInfo(&deviceInfo);
    GetRawMacAddrStr(LAN1_PORT, packedMacAddr);

    *cmdOutLen = snprintf(msgBuf, MAX_CMD_SZ,
                          "%c"      // SOM
                          "%s"		// ACK_LOG
                          "%c"		// FSP
                          "%03d"    // Device Type resp
                          "%06d"    // session Id
                          "%02d"    // software version
                          "%02d"    // software revision
                          "%02d"    // comm ver
                          "%02d"    // comm rev
                          "%02d"    // resp time
                          "%02d"    // KLV time
                          "%02d"    // max camera
                          "%02d"    // max analog cam
                          "%02d"    // max Ip camera
                          "%02d"    // configured analog cam
                          "%02d"    // configured Ip cam
                          "%02d"    // max sensor input
                          "%02d"    // max alarm output
                          "%02d"    // max audio in
                          "%01d"    // max audio out
                          "%01d"    // no of HDD
                          "%01d"    // no of NDD
                          "%01d"    // no of LAN
                          "%01d"    // VGA Port
                          "%01d"    // HDMI1
                          "%01d"    // HDMI2
                          "%01d"    // CVBS Main
                          "%01d"    // CVBS Spot
                          "%01d"    // CVBS Spot Analog
                          "%01d"    // PTZ for Analog
                          "%01d"    // USB ports
                          "%01d"    // Main Stream Analog Resolution
                          "%01d"    // Sub Stream Analog Resolution
                          "%01d"    // Video Standard
                          "%04d"    // Max encoding Capacity
                          "%04d"    // Max decoding Capacity
                          "%01d"    // disk checking status
                          "%01d"    // login user type
                          "%s"		// Mac Addr
                          "%02d"    // Product Variant
                          "%s"		// Reserved 4 Bytes
                          "%c"		// FSP
                          "%c",		// EOM
                          SOM,
                          diCmdHeader[LOGIN_CMD],
                          FSP,
                          DEF_VALUE,
                          DEF_VALUE,
                          deviceInfo.softwareVersion, deviceInfo.softwareRevision,
                          deviceInfo.commVersion, deviceInfo.commRevision,
                          DEF_VALUE,
                          DEF_VALUE,
                          deviceInfo.maxCameras, deviceInfo.maxAnalogCameras, deviceInfo.maxIpCameras,
                          deviceInfo.configuredAnalogCameras, deviceInfo.configuredIpCameras,
                          deviceInfo.maxSensorInput, deviceInfo.maxAlarmOutput,
                          deviceInfo.audioIn, deviceInfo.audioOut,
                          deviceInfo.noOfHdd, deviceInfo.noOfNdd, deviceInfo.noOfLanPort,
                          deviceInfo.noOfVGA, deviceInfo.HDMI1, deviceInfo.HDMI2,
                          deviceInfo.CVBSMain, deviceInfo.CVBSSpot,deviceInfo.CVBSSpotAnalog,
                          deviceInfo.anlogPTZSupport,
                          deviceInfo.USBPort,
                          deviceInfo.maxMainAnalogResolution, deviceInfo.maxSubAnalogResolution,
                          deviceInfo.videoStandard,
                          deviceInfo.maxMainEncodingCap, deviceInfo.maxSubEncodingCap,
                          deviceInfo.diskCheckingReq,
                          defGroup,
                          packedMacAddr,
                          deviceInfo.productVariant,
                          LOGIN_RESERVED_FOUR_BYTES,
                          FSP, EOM);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   diLonPollCmd
 * @param   msgBuf
 * @param   cmdOutLen
 * @param   data
 */
static void diLonPollCmd(CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data)
{
    INT32 deviceId = *((INT32PTR)data);
    *cmdOutLen = snprintf(msgBuf, MAX_CMD_SZ, "%c" "%s" "%c" "%06d" "%c" "%c", SOM, diCmdHeader[LONG_POLL], FSP, deviceId, FSP, EOM);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   diSendDid
 * @param   msgBuf
 * @param   cmdOutLen
 * @param   data
 */
static void diSendDid(CHARPTR msgBuf, UINT16PTR cmdOutLen, VOIDPTR data)
{
    INT32 deviceId = *((INT32PTR)data);
    *cmdOutLen = snprintf(msgBuf, MAX_CMD_SZ, "%c" "%s" "%c" "%06d" "%c" "%c", SOM, diCmdHeader[SEND_DID], FSP, deviceId, FSP, EOM);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   DiParseXXXCmdResp
 * @param   msgId
 * @param   msgPtr
 * @param   respData
 * @return
 */
BOOL DiParseXXXCmdResp(UINT8 msgId, CHARPTR *msgPtr, VOIDPTR respData)
{
	switch(msgId)
	{
        case DI_ACK_LOG:
            return diParseLoginCmdResp(msgPtr, respData);

        case DI_ACK_POL:
            return diParsePollCmdResp(msgPtr, respData);

        default:
            return FAIL;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function out index of header if success otherwise error
 * @param   msgPtr
 * @param   respData
 * @return
 */
static BOOL diParseLoginCmdResp(CHARPTR *msgPtr,VOIDPTR respData)
{
    UINT64          tempDataBuf[MAX_LOGIN_ARG];
    LOGIN_RESP_t    *loginResp = (LOGIN_RESP_t *)respData;

    /* Updated the Parsing because in case of response "CMD_REQUEST_IN_PROGRESS" SAMAS was responding with one field only,
     * hence validated that response as well instead of considering it as fail case */
    if (ParseStringGetVal(msgPtr, &tempDataBuf[LOGIN_RESP_ARG], 1, FSP) == FAIL)
    {
        return FAIL;
    }

    if (tempDataBuf[LOGIN_RESP_ARG] == CMD_REQUEST_IN_PROGRESS)
    {
        return IN_PROGRESS;
    }

    if (tempDataBuf[LOGIN_RESP_ARG] != CMD_SUCCESS)
    {
        loginResp->loginState = REFUSE;
        return FAIL;
    }

    /* Get the remaining fields as it is not an error response */
    if (ParseStringGetVal(msgPtr, &tempDataBuf[LOGIN_DEVICE_ID], MAX_LOGIN_ARG - 1, FSP) == FAIL)
    {
        return FAIL;
    }

    loginResp->pollDuration = ADD_POLL_DURATION_BUF(tempDataBuf[LOGIN_POL_DUR]);
    loginResp->pollInterval = tempDataBuf[LOGIN_POL_INT];
    loginResp->deviceId = (UINT32)tempDataBuf[LOGIN_DEVICE_ID];
    loginResp->loginState = SUCCESS;
    DPRINT(DEVICE_INITIATION, "poll info: [duration=%d], [interval=%d]", loginResp->pollDuration, loginResp->pollInterval);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   diParsePollCmdResp
 * @param   msgPtr
 * @param   respData
 * @return
 */
static BOOL diParsePollCmdResp(CHARPTR *msgPtr, VOIDPTR respData)
{
	UINT64 		tempDataBuf;
	UINT8PTR 	portNo = (UINT8PTR)respData;

    if(ParseStringGetVal(msgPtr, &tempDataBuf, 1, FSP) == FAIL)
	{
        return FAIL;
	}

    *portNo = (UINT8)tempDataBuf;
    return SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
