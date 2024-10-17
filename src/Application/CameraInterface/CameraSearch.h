#ifndef CAMERASEARCH_H_
#define CAMERASEARCH_H_
//#################################################################################################
// @FILE BRIEF
//#################################################################################################
/**
@file       CameraSearch.h
@brief      This File Provides to Search Camera using UPnP Protocol & ONVIF Device Discovery Service.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"
#include "HttpClient.h"
#include "NetworkInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_CAM_SEARCH_IN_ONE_SHOT          250
#define	MAX_BUFFER_SIZE                     10000//32768
#define MAX_RECEIVE_MESSAGE_TIME_OUT_MS     3000
#define MAX_CONN_TIME                       1
#define IPV4_MULTICAST_ADDRESS              "239.255.255.250"
#define	IPV4_MULTICAST_RECV_ADDRESS         "224.0.0.0"
#define IPV6_MULTICAST_ADDRESS              "ff02::c"
#define MULTICAST_PORT                      1900
#define MAX_LOCATION_NAME_LEN               128
#define MAX_DEVICE_TYPE_LEN                 128
#define MAX_CAMERA_SEARCH_SESSION           (MAX_NW_CLIENT + 1)

#define LOCATION_FIELD                      "Location:"
#define XML_TAG_DEVICE_TYPE                 "deviceType"
#define XML_TAG_BRAND                       "manufacturer"
#define XML_TAG_MODEL                       "modelNumber"
#define	UPNP_DEFLT_STR                      "urn:schemas-upnp-org:device:"

#define SEARCH_MESSAGE_FORMAT               "M-SEARCH * HTTP/1.1\r\n"\
                                            "HOST: %s:%d\r\n"\
                                            "ST: upnp:rootdevice\r\n"\
                                            "MAN: \"ssdp:discover\"\r\n"\
                                            "MX: 1\r\n"\
                                            "\r\n"

#define ADD_MULTICAST_ROUTE_CMD             "ip route add %s/3 dev %s"
#define DEL_MULTICAST_ROUTE_CMD             "ip route del %s/3 dev %s"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	CAM_IDENTIFIED,
	CAM_ADDED,
	CAM_UNIDENTIFIED,
	CAM_STATUS_MAX
}CAM_STATUS_e;

typedef enum
{
	MX_CAM_SEARCH_NORMAL,
	MX_CAM_SEARCH_AUTO_CONFIG_ON_BOOT
}MX_CAM_SEARCH_TYPE_e;

typedef struct
{
	BOOL					updationOnNewSearch;
    CHAR					ipv4Addr[IPV4_ADDR_LEN_MAX];
    CHAR                    ipv6Addr[MAX_IPV6_ADDR_TYPE][IPV6_ADDR_LEN_MAX];
	CHAR					brand[MAX_BRAND_NAME_LEN];
	CHAR					model[MAX_MODEL_NAME_LEN];
	UINT16					httpPort;
	UINT16					onvifPort;
	BOOL					onvifSupport;
	CAM_STATUS_e			camStatus;
	UINT8					camIndex; // for added cam only ..
	CHAR					camName[MAX_CAMERA_NAME_WIDTH];
}IP_CAM_SEARCH_RESULT_t;

typedef struct
{
	// data allocated using malloc and freed after use, to avoid unnecessary memory occupation
	IP_CAM_SEARCH_RESULT_t	result[MAX_CAM_SEARCH_IN_ONE_SHOT];
	CHAR					location[MAX_CAM_SEARCH_IN_ONE_SHOT][MAX_LOCATION_NAME_LEN];
	HTTP_INFO_t				httpInfo;
	CHAR  					msgBuffer[MAX_BUFFER_SIZE + 1];
}DYNAMIC_DATA_t;

typedef struct
{
	BOOL					requestStatus;
	UINT8					totalDevices;
    UINT8					devicesFoundInUpnP;
	UINT8					requestCount;
	BOOL					httpReqStatus;
    DYNAMIC_DATA_t			*pData;
	pthread_mutex_t			reqStatusLock;
}IP_CAM_SEARCH_SESSION_PARAM_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitCameraSearch(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartCameraSearch(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
void StopCameraSearch(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
BOOL GetXMLTag(CHARPTR *source, CHARPTR tag, CHARPTR dest, UINT16 maxSize);
//-------------------------------------------------------------------------------------------------
UINT8 RecvUpnpResponse(INT32 *connFd);
//-------------------------------------------------------------------------------------------------
BOOL ValidateDevice(CHARPTR * source);
//-------------------------------------------------------------------------------------------------
BOOL IsCamSearchActiveForClient(UINT8 sessionIndex);
//-------------------------------------------------------------------------------------------------
BOOL IsCameraAlreadyFound(const CHAR *ipv4AddrCamList, const CHAR *ipv4Addr,
                          CHAR ipv6AddrCamList[MAX_IPV6_ADDR_TYPE][IPV6_ADDR_LEN_MAX], const CHAR *globalIpv6Addr, const CHAR *linkLocalIpv6Addr);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e GetAcqListOfCameras(IP_CAM_SEARCH_RESULT_t *result, UINT8PTR numOfResult);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* CAMERASEARCH_H_ */
