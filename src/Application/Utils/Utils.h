 #if !defined UTILS_H
#define UTILS_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		Utils.h
@brief      File containing the prototype of different utils functions for all modules
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <sys/prctl.h>

/* Library Includes */
#include "nm_iputility.h"

/* Application Includes */
#include "EventLogger.h"
#include "UtilCommon.h"
#include "CommonApi.h"
#include "DeviceInfo.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* System command executor handler message id */
#define	SYS_CMD_EXE_REQ_IPC_MSG_ID      3640
#define	SYS_CMD_EXE_RESP_IPC_MSG_ID     3641
#define SYS_CMD_MSG_LEN_MAX             250

#define GET_CAMERA_NO(x)				(x + 1)
#define GET_STREAM_INDEX(camIndex) 		(camIndex % getMaxCameraForCurrentVariant())
#define GET_STREAM_TYPE(camIndex)		(camIndex / getMaxCameraForCurrentVariant())

#define GET_REAL_CAMERA_ID(camera)                  GET_STREAM_INDEX(camera)
#define GET_STREAM_MAPPED_CAMERA_ID(camera, stream) (camera + (stream * getMaxCameraForCurrentVariant()))

#define THREAD_NAME_MAX                 16
#define THREAD_START(name)              CHAR threadName[THREAD_NAME_MAX]; \
                                        snprintf(threadName, THREAD_NAME_MAX, "%s", name); \
                                        prctl(PR_SET_NAME, threadName, 0, 0, 0);

#define THREAD_START_INDEX(name, num)   CHAR threadName[THREAD_NAME_MAX]; \
                                        snprintf(threadName, THREAD_NAME_MAX, "%s_%02d", name, num); \
                                        prctl(PR_SET_NAME, threadName, 0, 0, 0);

#define THREAD_START_INDEX2(name, num)  CHAR threadName[THREAD_NAME_MAX]; \
                                        snprintf(threadName, THREAD_NAME_MAX, "%s_%d", name, num); \
                                        prctl(PR_SET_NAME, threadName, 0, 0, 0);

#define USERWISE_LAYOUT_FILE(file, uId)	snprintf(file, sizeof(file), CONFIG_DIR_PATH "/xmlConfig/DeviceClientLayoutConfig_%d.xml", uId);

#define MAX_UUID_LEN                    37
#define GET_PLAIN_TO_BASE64_SIZE(x)     (((((x) + 2) / 3) * 4) + 1)
#define GET_BASE64_TO_PLAIN_SIZE(x)     ((((x) * 3) / 4) + 1)
#define SHA256_STR_LEN_MAX              65 /* 64 + null */

#define SOCK_ADDR_SIZE(s)               ((s.sockAddr.sa_family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef enum
{
	SHUTDOWN_DEVICE,
	REBOOT_DEVICE,
    POWER_ACTION_MAX
}POWER_ACTION_e;

typedef enum
{
	INT_SIG,
	KILL_SIG,
	SIG_USR,
	MAX_SIG,
}SEND_PROC_SIG_e;

typedef enum
{
    SYS_EXE_CTRL_CMD_ID_RESERVED = 0,
    SYS_EXE_CTRL_CMD_ID_DEBUG_CNFG,
    SYS_EXE_CTRL_CMD_ID_MAX,
}SYS_EXE_CTRL_CMD_ID_e;

typedef struct
{
    UINT32  txLoad;
    UINT32  rxLoad;
}NET_INTERFACE_LOAD_PARAM_t;

typedef struct
{
    long	cmdId;
    CHAR 	cmdData[SYS_CMD_MSG_LEN_MAX];
}SYS_CMD_MSG_INFO_t;

//-------------------------------------------------------------------------------------------------
typedef BOOL (*COPY_ABORT_CB)(UINT32 userData);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitUtils(void);
//-------------------------------------------------------------------------------------------------
BOOL InitSysCmdHandler(void);
//-------------------------------------------------------------------------------------------------
BOOL ExeSysCmd(BOOL useSysCmdExeF, const CHARPTR rSysCmd);
//-------------------------------------------------------------------------------------------------
BOOL ExeSensitiveInfoSysCmd(const CHARPTR rSysCmd);
//-------------------------------------------------------------------------------------------------
void SetCmdExeDebugConfig(void);
//-------------------------------------------------------------------------------------------------
BOOL IsGivenTimeInWindow(TIME_HH_MM_t cmpTime, TIME_HH_MM_t startTime, TIME_HH_MM_t endTime);
//-------------------------------------------------------------------------------------------------
void SetPowerAction(POWER_ACTION_e action);
//-------------------------------------------------------------------------------------------------
POWER_ACTION_e GetPowerAction(void);
//-------------------------------------------------------------------------------------------------
BOOL StartWatchDog(void);
//-------------------------------------------------------------------------------------------------
void KickWatchDog(void);
//-------------------------------------------------------------------------------------------------
void UtilsGeneralCfgUpdate(GENERAL_CONFIG_t newGeneralConfig, GENERAL_CONFIG_t *oldGeneralConfig);
//-------------------------------------------------------------------------------------------------
CHARPTR GetWeekName(WEEK_DAY_e weekDay);
//-------------------------------------------------------------------------------------------------
CHARPTR GetMonthName(UINT8 monthNo);
//-------------------------------------------------------------------------------------------------
UINT8 GetMonthNo(CHARPTR monStr);
//-------------------------------------------------------------------------------------------------
void ReplaceCharecter(CHARPTR address, UINT8 findChar, UINT8 repChar);
//-------------------------------------------------------------------------------------------------
BOOL StringNCopy(CHARPTR dst, CHARPTR src, UINT32 size);
//-------------------------------------------------------------------------------------------------
INT32 GetPidOfProcess(CHARPTR	processName);
//-------------------------------------------------------------------------------------------------
BOOL KillProcess(CHARPTR processName, SEND_PROC_SIG_e sigType);
//-------------------------------------------------------------------------------------------------
BOOL KillProcessId(INT32 processId, SEND_PROC_SIG_e sigType);
//-------------------------------------------------------------------------------------------------
BOOL FindFreeRtpPort(UINT16PTR rtpPortPtr);
//-------------------------------------------------------------------------------------------------
void ClearRtpPort(UINT16PTR rtpPortPtr);
//-------------------------------------------------------------------------------------------------
BOOL GetIpAddrFromDomainName(CHARPTR domainName, IP_ADDR_TYPE_e ipAddrType, CHARPTR resolvedIpAddr);
//-------------------------------------------------------------------------------------------------
BOOL PrepForPowerAction(POWER_ACTION_e powerAction, LOG_EVENT_STATE_e eventStatus, const CHARPTR userName);
//-------------------------------------------------------------------------------------------------
BOOL CopyFile(CHARPTR sourceFile, CHARPTR destFile, UINT32 delay, COPY_ABORT_CB callback, UINT32 userData);
//-------------------------------------------------------------------------------------------------
BOOL AddLinuxUser(CHARPTR username, CHARPTR password);
//-------------------------------------------------------------------------------------------------
BOOL DeleteLinuxUser(CHARPTR username, UINT8 usrIndx);
//-------------------------------------------------------------------------------------------------
void DeleteClientDispLayoutFile(UINT8 userIdx);
//-------------------------------------------------------------------------------------------------
BOOL FtpServiceStartStop(UINT16 port, BOOL action);
//-------------------------------------------------------------------------------------------------
BOOL WebServerServiceStartStop(BOOL action, UINT16 webport);
//-------------------------------------------------------------------------------------------------
void GetNetworkInterfaceLoad(NETWORK_PORT_e networkPort, NET_INTERFACE_LOAD_PARAM_t *ifLoad);
//-------------------------------------------------------------------------------------------------
UINT8 GetCurrCpuUsage(void);
//-------------------------------------------------------------------------------------------------
UINT64 GetDeviceUpTime(void);
//-------------------------------------------------------------------------------------------------
BOOL ParseFTPListReply(CHARPTR *list, CHARPTR entry, UINT16 maxSize, BOOL *isDir);
//-------------------------------------------------------------------------------------------------
void ParseIpAddressGetIntValue(CHARPTR ipAddrstr, UINT8PTR allOctect);
//-------------------------------------------------------------------------------------------------
void UnPackGridGeneral(CHARPTR input, UINT8 col, UINT8 raw, CHARPTR *output, UINT16 inputLength);
//-------------------------------------------------------------------------------------------------
void ConvertUnpackedGridTo44_36(CHARPTR *rendomGridPtr, CHARPTR *grid44_36Ptr, UINT8 column, UINT8 row);
//-------------------------------------------------------------------------------------------------
void Convert44_36ToUnpackedGrid(CHARPTR *grid44_36Ptr, CHARPTR *rendomGridPtr, UINT8 column, UINT8 row);
//-------------------------------------------------------------------------------------------------
void PackGridGeneral(CHARPTR *convertedBuf, UINT8 col, UINT8 raw, CHARPTR packgrid, UINT16PTR OutputLen);
//-------------------------------------------------------------------------------------------------
void **Allocate2DArray(UINT16 rows, UINT16 cols, UINT32 elementSize);
//-------------------------------------------------------------------------------------------------
void Free2DArray(void **matrix, UINT16 rows);
//-------------------------------------------------------------------------------------------------
BOOL ValidateIpSubnetWithLan1(CHARPTR startIp, CHARPTR endIp);
//-------------------------------------------------------------------------------------------------
UINT8 ConvertStringToIndex(CHARPTR strPtr, const CHARPTR *strBuffPtr, UINT8 maxIndex);
//-------------------------------------------------------------------------------------------------
BOOL ParseStr(CHARPTR *src, UINT8 delim, CHARPTR dest, UINT16 maxDestSize);
//-------------------------------------------------------------------------------------------------
BOOL ParseNStr(CHARPTR *src, UINT8 delim, CHARPTR dest, UINT16 maxDestSize);
//-------------------------------------------------------------------------------------------------
BOOL AsciiToInt(CHARPTR ascii, UINT64PTR integer);
//-------------------------------------------------------------------------------------------------
BOOL ParseStringGetVal(CHARPTR *commandStrPtr, UINT64PTR tempDataPtr, UINT8 maxCmdArg, UINT8 seperator);
//-------------------------------------------------------------------------------------------------
CHARPTR EncodeBase64(const CHAR *pPlainData, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
CHARPTR DecodeBase64(const CHAR *pEncData, UINT32 dataLen, UINT32 *pOutLen);
//-------------------------------------------------------------------------------------------------
UINT8 *EncryptAes256(const UINT8 *pDataBuff, UINT32 buffLen, const CHAR *pPassword, const CHAR *pSaltKey, UINT32 *pOutLen);
//-------------------------------------------------------------------------------------------------
UINT8 *DecryptAes256(const UINT8 *pDataBuff, UINT32 buffLen, const CHAR *pPassword, const CHAR *pSaltKey, UINT32 *pOutLen);
//-------------------------------------------------------------------------------------------------
UINT32 GetRandomNum(void);
//-------------------------------------------------------------------------------------------------
void GetRandomAlphaNumStr(CHARPTR pString, UINT8 length, BOOL onlyHexChar);
//-------------------------------------------------------------------------------------------------
void GenerateUuidStr(CHARPTR uuidStr);
//-------------------------------------------------------------------------------------------------
void RemoveSpecialCharacters(const CHAR *pSrcStr, const CHAR *pSpecialChars, CHAR *pDstStr);
//-------------------------------------------------------------------------------------------------
void GetStrSha256(CHAR *pInStr, CHAR *pOutHash);
//-------------------------------------------------------------------------------------------------
BOOL IsIpAddrInSameSubnet(CHAR *pIpAddr1, CHAR *pIpAddr2, CHAR *pSubnetMask);
//-------------------------------------------------------------------------------------------------
BOOL CreateNonBlockConnection(const CHAR *pIpAddr, UINT16 port, INT32 *pConnFd);
//-------------------------------------------------------------------------------------------------
void GetNetworkAddress(CHARPTR pIpString, VOIDPTR pSubNetInfo, UINT8 addrFamily, IP_NW_ADDR_u *pNwAddr);
//-------------------------------------------------------------------------------------------------
void GetHostAddress(CHARPTR pIpString, VOIDPTR pSubNetInfo, UINT8 addrFamily, IP_NW_ADDR_u *pNwAddr);
//-------------------------------------------------------------------------------------------------
void ConvertIpv4MappedIpv6SockAddr(SOCK_ADDR_INFO_u *sockAddr);
//-------------------------------------------------------------------------------------------------
void GetIpAddrNwFromSockAddr(const SOCK_ADDR_INFO_u *sockAddr, IP_NW_ADDR_u *ipAddrNw);
//-------------------------------------------------------------------------------------------------
void GetIpAddrStrFromSockAddr(const SOCK_ADDR_INFO_u *sockAddr, CHAR *ipAddrStr, UINT32 addrBufLen);
//-------------------------------------------------------------------------------------------------
UINT16 GetHostPortFromSockAddr(const SOCK_ADDR_INFO_u *sockAddr);
//-------------------------------------------------------------------------------------------------
BOOL GetSockAddr(const CHAR *ipAddr, UINT16 port, SOCK_ADDR_INFO_u *sockAddr);
//-------------------------------------------------------------------------------------------------
BOOL SendMulticastMessage(INT32 sockFd, CHARPTR srcAddr, CHARPTR multicastAddr, UINT32 port, const CHAR *ifname, CHARPTR message);
//-------------------------------------------------------------------------------------------------
BOOL ParseDelimiterValue(CHAR *dataBuffer, CHAR *delimiter, CHAR *parseString, UINT32 dataLen, CHAR **srcString);
//-------------------------------------------------------------------------------------------------
BOOL GetIpAddrAndPortFromUrl(const CHAR *urlStr, const CHAR *protocol, CHAR *ipAddress, UINT16PTR port);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // UTILS_H
