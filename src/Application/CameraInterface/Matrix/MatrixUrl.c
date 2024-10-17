//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file       MatrixUrl.c
@brief      This File provides APIs to construct Urls for All Matrix cameras.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "TimeZone.h"
#include "Utils.h"
#include "UrlRequest.h"
#include "DebugLog.h"
#include "DateTime.h"
#include "CameraDatabase.h"
#include "CameraInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAIN_STR                "1"
#define MAX_RES_STRING          (7)
#define DIRECTORY_DELIM_CHAR    '/'
#define MAX_SIZE_OF_STRING      (24)
#define MAX_FILE_NAME_LEN       (100)
#define PROFILE_ONE             "1"
#define PROFILE_TWO             "2"
#define MAX_PUT_REQ_BUF_SIZE    (3072)  /* 3KB */

#define PUT_STREAM_FILE_NAME                "/tmp/MatrixMainSubPutFile%d.txt"
#define PUT_CHANGE_CAM_IP_FILE_NAME         "/tmp/MatrixChangeCamIpPutFile%d.txt"
#define PUT_OSD_FILE_NAME                   "/tmp/MatrixosdFile%d.txt"
#define PUT_SET_MOTION_WIN_FILE_NAME        "/tmp/MatrixSetMotionWin%d.txt"
#define PUT_SET_PRIVACYMASK_FILE_NAME       "/tmp/MatrixSetPrivacyMask%d.txt"
#define PUT_SET_PTZ_FILE_NAME               "/tmp/MatrixSetPtz%d.txt"
#define PUT_SET_IRIS_FILE_NAME              "/tmp/MatrixSetIris%d.txt"
#define PUT_SET_FOCUS_FILE_NAME             "/tmp/MatrixSetFocus%d.txt"
#define PUT_SET_PRESET_FILE_NAME            "/tmp/MatrixSetPreset%d.txt"
#define PUT_SET_OUTPUT_TRIGGER_FILE_NAME    "/tmp/MatrixSetoutputTrg%d.txt"
#define PUT_OPEN_TWO_WAY_AUDIO_FILE_NAME    "/tmp/MatrixTwoWayAudioFile.txt"
#define PUT_DATE_TIME_FILE_NAME             "/tmp/MatrixDateTimeFile%d.txt"
#define PUT_IMAGE_SETTING_FILE_NAME         "/tmp/MatrixImageSettingFile%d.txt"
#define REBOOT_STR                          "reboot"

#define MATRIX_NORMALIZE_WIDTH              704
#define MATRIX_NORMALIZE_HEIGHT             576
#define OSD_DATE_TIME_LEN                   420
#define OSD_HIEGHT_STEP_SIZE                32
#define TIANDY_NORMALIZE_WIDTH              10000
#define TIANDY_NORMALIZE_HEIGHT             10000
#define TIANDY_OSD_LINE_OFFSET              800

#define TIANDY_CAMERA_FIXED_QUALITY_MAX     5

/*************************************************************************************************/
#define XML_VERSION_STR                 "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
#define STD_CGI_URL_STR                 " version=\"2.0\" xmlns=\"http://www.std-cgi.com/ver20/XMLSchema\""
#define ISAPI_URL_STR                   " version=\"2.0\" xmlns=\"http://www.isapi.org/ver20/XMLSchema\""
#define HIKVISION_URL_STR               " version=\"2.0\" xmlns=\"http://www.hikvision.com/ver20/XMLSchema\""
#define ID_TAG_STR                      "<id>%d</id>\n"

#define STREAM_PARA_HDR_STR_OEM(url)    XML_VERSION_STR "<StreamingChannel"url">\n"
#define DEFAULT_CHANNEL_STRING          "<channelName>%s</channelName>\n"
#define PROTOCOL_LIST                   "<Transport>\n" \
                                        "<maxPacketSize>1000</maxPacketSize>\n" \
                                        "<ControlProtocolList>\n" \
                                        "<ControlProtocol>\n<streamingTransport>RTSP</streamingTransport>\n</ControlProtocol>\n" \
                                        "<ControlProtocol>\n<streamingTransport>HTTP</streamingTransport>\n</ControlProtocol>\n" \
                                        "<ControlProtocol>\n<streamingTransport>SHTTP</streamingTransport>\n</ControlProtocol>\n" \
                                        "</ControlProtocolList>\n" \
                                        "<Unicast>\n<enabled>true</enabled>\n<rtpTransportType>RTP/TCP</rtpTransportType>\n</Unicast>\n" \
                                        "<Security>\n<enabled>true</enabled>\n</Security>\n" \
                                        "</Transport>\n"
#define VIDEO_STR						"<Video>\n"
#define ENABLE_FIELD_STR				"<enabled>true</enabled>\n"
#define VIDEO_CHANNEL_ID_STR			"<videoInputChannelID>1</videoInputChannelID>\n"
#define VIDEO_CODEC_TYPE_STR			"<videoCodecType>%s</videoCodecType>\n"
#define VIDEO_SCAN_TYPE_STR				"<videoScanType>progressive</videoScanType>\n"
#define VIDEO_RESOLUTION_WIDTH_STR		"<videoResolutionWidth>%s</videoResolutionWidth>\n"
#define VIDEO_RESOLUTION_HEIGHT_STR		"<videoResolutionHeight>%s</videoResolutionHeight>\n"
#define VIDEO_QUALITY_CONTROL_TYPE_STR	"<videoQualityControlType>%s</videoQualityControlType>\n"
#define CONST_BIT_RATE_STR				"<constantBitRate>%s</constantBitRate>\n"
#define FIXED_QUALITY 					"<fixedQuality>%s</fixedQuality>\n"
#define CODEC_PROFILE                   "<H264Profile>Baseline</H264Profile>\n"
#define MAX_FRAME_RATE_STR 				"<maxFrameRate>%d</maxFrameRate>\n"
#define SNAP_SHOT_IMAGE_STR				"<snapShotImageType>JPEG</snapShotImageType>\n"
#define GOV_LENGTH_STR					"<GovLength>%d</GovLength>\n"
#define SCALABLE_VIDEO_CODING_STR       "<SVC>auto</SVC>\n"
#define NP_MODE_STR                     "<np-Mode>PAL</np-Mode>\n"
#define PRIORITY_MODE_STR               "<priorityMode>FramRate</priorityMode>\n<corridorMode>OFF</corridorMode>\n"
#define CHANNEL_TYPE_STR                "<channelType>LocalChannel</channelType>\n<enctypeType>NoEnctype</enctypeType>\n" \
                                        "<password/>\n<electronicImageStab>OFF</electronicImageStab>\n<electronicImageLevel>50</electronicImageLevel>\n<smoothing>50</smoothing>\n"
#define SPLUS_265_STR                   "<SPlus265>false</SPlus265>\n"
#define VBR_LIMIT_STR                   "<vbrUpperCap>2048</vbrUpperCap>\n<vbrLowerCap>128</vbrLowerCap>\n"
#define VIDEO_END_STR                   "</Video>\n"

#define AUDIO_STR                       "<Audio>\n<enabled>%s</enabled>\n<audioInputChannelID>1</audioInputChannelID>\n<audioCompressionType>G.711ulaw</audioCompressionType>\n</Audio>\n"
#define AUDIO_CHANNEL_STR               "<Audio>\n<enabled>%s</enabled>\n</Audio>\n"
#define STREAM_END_STR                  "</StreamingChannel>\n"

/*************************************************************************************************/
#define NET_INF_TAG_STR             "<NetworkInterface>\n"
#define NET_INF_CLOSE_TAG_STR       "</NetworkInterface>\n"
#define LINK_TAG_STR                "<Link>\n<autoNegotiation>false</autoNegotiation>\n<manualSetDns>true</manualSetDns>\n<MTU>1500</MTU>\n</Link>\n"
#define IP_ADDR_TAG_STR(url)        "<IPAddress"url">\n"
#define IP_ADDR_END_TAG				"</IPAddress>\n"
#define IP_ADDR_TYPE_STR			"<ipVersion>dual</ipVersion>\n<addressingType>static</addressingType>\n"
#define IP_ADDR_STR					"<ipAddress>%s</ipAddress>\n"
#define IP_SUBNET_STR				"<subnetMask>%s</subnetMask>\n"
#define IPV6_ADDR_STR				"<ipv6Address>::</ipv6Address>\n<bitMask>0</bitMask>\n"
#define IP_DFLT_GATEWAY_STR			"<DefaultGateway>\n<ipAddress>%s</ipAddress>\n</DefaultGateway>\n"
#define	PRIMARY_DNS_STR				"<PrimaryDNS>\n<ipAddress>8.8.8.8</ipAddress>\n</PrimaryDNS>\n"
#define SECONDRY_DNS_STR			"<SecondaryDNS>\n<ipAddress>%s</ipAddress>\n</SecondaryDNS>\n"
#define IPV6_MODE_STR				"<Ipv6Mode>\n<ipV6AddressingType>ra</ipV6AddressingType>\n<ipv6AddressList>\n<v6Address>\n<id>1</id>\n<type>manual</type>\n<address>2001::4619:b7ff:fe16:2f96</address>\n<bitMask>64</bitMask>\n</v6Address>\n</ipv6AddressList>\n</Ipv6Mode>\n"


/*************************************************************************************************/
#define OSD_HDR_STR_OEM(url)        XML_VERSION_STR "<VideoOverlay"url">\n"
#define OSD_SCREEN_SIZE_STR         "<normalizedScreenSize>\n<normalizedScreenWidth>%d</normalizedScreenWidth>\n<normalizedScreenHeight>%d</normalizedScreenHeight>\n</normalizedScreenSize>\n"
#define OSD_ATTRIBUTE_STR(url)      "<attribute>\n<transparent>false</transparent>\n<flashing>false</flashing>\n</attribute>\n<fontSize>0</fontSize>\n<TextOverlayList"url">\n"
#define OSD_TEXT_SETTING_STR        "<TextOverlay>\n<id>1</id>\n<enabled>%s</enabled>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n<displayText>%s</displayText>\n</TextOverlay>\n"
#define OSD_TEXT_END_STR            "</TextOverlayList>\n"
#define OSD_DATE_TIME_STR(url)      "<DateTimeOverlay"url">\n<enabled>%s</enabled>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n<dateStyle>%s</dateStyle>\n<timeStyle>%s</timeStyle>\n<displayWeek>%s</displayWeek>\n</DateTimeOverlay>\n"
#define OSD_CHNL_NAME_STR(url)      "<channelNameOverlay"url">\n<enabled>false</enabled>\n<positionX>600</positionX>\n<positionY>32</positionY>\n</channelNameOverlay>\n"
#define OSD_COLOR_STR               "<frontColorMode>auto</frontColorMode>\n<frontColor>000000</frontColor>\n</VideoOverlay>\n"

/*************************************************************************************************/
#define MOTION_GRID_STR_OEM(url)    XML_VERSION_STR "<MotionDetection"url">\n<enabled>%s</enabled>\n<regionType>grid</regionType>\n<Grid>\n<rowGranularity>18</rowGranularity>\n<columnGranularity>22</columnGranularity>\n</Grid>\n"
#define MOTION_LAYOUTSTR_OEM(url)   "<MotionDetectionLayout"url">\n<sensitivityLevel>%d</sensitivityLevel>\n<layout>\n<gridMap>%s</gridMap>\n</layout>\n</MotionDetectionLayout>\n</MotionDetection>"

/*************************************************************************************************/
#define REGIONS_STR                 "regions"

/*************************************************************************************************/
//tags  used for parsing
#define PRIVACY_ENABLE_TAG			"<enabled>\n"
#define PRIVACY_WINDOW_WIDTH_TAG 	"<normalizedScreenWidth>%d</normalizedScreenWidth>\n"
#define PRIVACY_WINDOW_WIDTH_STR	"<normalizedScreenWidth>\n"
#define PRIVACY_WINDOW_HEIGHT_TAG 	"<normalizedScreenHeight>%d</normalizedScreenHeight>\n"
#define PRIVACY_WINDOW_HEIGHT_STR	"<normalizedScreenHeight>\n"
#define POSITIONX_TAG				"<positionX>"
#define POSITIONY_TAG				"<positionY>"
#define REGION_LIST_TAG				"<RegionCoordinatesList>"
#define REGION_LIST 				"<RegionCoordinatesList>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>700</positio\n<positionY>2</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>2</positionX>\n<positionY>574</positionY>\n</RegionCoordinates>\n</RegionCoordinatesList>"

/*************************************************************************************************/
//tags used for set privacy
#define PRIVACY_HEADER_STR_OEM(url)	XML_VERSION_STR "<PrivacyMask"url">\n"
#define ENABLE_TRUE_TAG_STR			"<enabled>true</enabled>\n"
#define NORMALIZED_SIZE_STR 		"<normalizedScreenSize>\n<normalizedScreenWidth>%d</normalizedScreenWidth>\n<normalizedScreenHeight>%d</normalizedScreenHeight>\n</normalizedScreenSize>\n"
#define	PRIVACY_REGION_LIST_STR		"<PrivacyMaskRegionList>\n"

#define PRIVACY_REGION_STR			"<PrivacyMaskRegion>\n<id>%d</id>\n<enabled>true</enabled>\n<RegionCoordinatesList>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n</RegionCoordinatesList>\n</PrivacyMaskRegion>\n"
#define PRIVACY_REGION_STR_OEM_PTZ	"<PrivacyMaskRegion>\n<id>%d</id>\n<enabled>true</enabled>\n<RegionCoordinatesList>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n</RegionCoordinatesList>\n<privacymaskName>Mask%d</privacymaskName>\n<maskType>blue</maskType>\n<zoomdoorlimit>10</zoomdoorlimit>\n</PrivacyMaskRegion>\n"

#define PRIVACY_REGION_FALSE_STR	"<PrivacyMaskRegion>\n<id>%d</id>\n<enabled>false</enabled>\n<RegionCoordinatesList>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n<RegionCoordinates>\n<positionX>%d</positionX>\n<positionY>%d</positionY>\n</RegionCoordinates>\n</RegionCoordinatesList>\n</PrivacyMaskRegion>\n"
#define PRIVACY_REGION_LIST_END_TAG	"</PrivacyMaskRegionList>\n"
#define PRIVACY_END_TAG				"</PrivacyMask>"
#define END_TAG_DELIM				"</"

/*************************************************************************************************/
#define PTZ_HEADER(url)             XML_VERSION_STR "<PTZData"url">"
#define PAN_DATA					"<pan>%d</pan>"
#define TILT_DATA					"<tilt>%d</tilt>"
#define ZOOM_DATA					"<zoom>%d</zoom>"
#define PTZ_END						"</PTZData>"

/*************************************************************************************************/
#define IRIS_HEADER(url)            XML_VERSION_STR "<IrisData"url">"
#define IRIS_DATA					"<iris>%d</iris>"
#define IRIS_END					"</IrisData>"

/*************************************************************************************************/
#define FOCUS_HEADER(url)           XML_VERSION_STR "<FocusData"url">"
#define FOCUS_DATA					"<focus>%d</focus>"
#define FOCUS_END					"</FocusData>"

/*************************************************************************************************/
#define FOCUS_CONFIG_HEADER(mode)	"<FocusConfiguration>\n" \
                                    "<focusStyle>"mode"</focusStyle>\n" \
                                    "<focusLimited>600</focusLimited>" \
                                    "</FocusConfiguration>"


#define FOCUS_MODE_STR(mode)        "<FocusMode>\n<mode>"mode"</mode>\n</FocusMode>"
/*************************************************************************************************/
#define PRESET_HEADER(url)          XML_VERSION_STR "<PTZPreset"url">\n"
#define PRESET_ENABLE				"<enabled>true</enabled>\n"
#define PRESET_NAME					"<presetName>Preset%d</presetName>\n"
#define PRESET_END					"</PTZPreset>\n"

/*************************************************************************************************/
#define OUTPUT_TRIGGER_DEFAULT      "IOPortData" ISAPI_URL_STR
#define OUTPUT_PORT_TRIGGER_DEFAULT XML_VERSION_STR "<IOOutputPort" ISAPI_URL_STR ">\n<id>1</id>\n"
#define OUTPUT_TRIGGER_ID           "<outputState>%s</outputState>\n"
#define OUTPUT_PORT_TRIGGER_ID      "<PowerOnState>\n"OUTPUT_TRIGGER_ID"<delayTime>2</delayTime>\n</PowerOnState>\n</IOOutputPort>\n"
#define INPUT_TRIGGER_ID            "<inputIOPortID>%d</inputIOPortID>"

/*************************************************************************************************/
#define TWO_WAY_AUDIO_STR(url)      XML_VERSION_STR "<TwoWayAudioChannel"url">\n<id>1</id>\n<enabled>false</enabled>\n<audioCompressionType>G.711ulaw</audioCompressionType>\n<audioInputType>LineIn</audioInputType>\n<speakerVolume>90</speakerVolume>\n<noisereduce>false</noisereduce>\n</TwoWayAudioChannel>\n"

/*************************************************************************************************/
#define DATE_TIME_STR               XML_VERSION_STR "<Time><timeMode>manual</timeMode>\n<localTime>%04d-%02d-%02dT%02d:%02d:%02d</localTime><timeZone>%s</timeZone></Time>"

/*************************************************************************************************/
#define IMAGE_SETTING_STR_PART1     "<Color>\n<brightnessLevel>%d</brightnessLevel>\n<contrastLevel>%d</contrastLevel>\n<saturationLevel>%d</saturationLevel>\n<hueLevel>%d</hueLevel>\n</Color>\n<Sharpness>\n"
#define IMAGE_SETTING_STR_PART2     "</Sharpness>\n<WDR>\n<mode>%s</mode>\n<WDRLevel>%d</WDRLevel>\n</WDR>\n<WhiteBalance>\n<WhiteBalanceStyle>%s</WhiteBalanceStyle>\n<WhiteBalanceRed>50</WhiteBalanceRed>\n<WhiteBalanceBlue>50</WhiteBalanceBlue>\n</WhiteBalance>\n"
#define TIANDY_SHARPNESS_STR        "<sharpnessLevel>%d</sharpnessLevel>\n"
#define HIKVISION_SHARPNESS_STR     "<SharpnessLevel>%d</SharpnessLevel>\n"
#define TIANDY_IMAGE_SETTING_STR    XML_VERSION_STR "<ImageParam>\n" IMAGE_SETTING_STR_PART1 TIANDY_SHARPNESS_STR IMAGE_SETTING_STR_PART2 "</ImageParam>\n"
#define HIKVISION_IMAGE_SETTING_STR XML_VERSION_STR "<ImageChannel " STD_CGI_URL_STR ">\n" ID_TAG_STR IMAGE_SETTING_STR_PART1 HIKVISION_SHARPNESS_STR IMAGE_SETTING_STR_PART2 "</ImageChannel>\n"

/*************************************************************************************************/
#define MAX_WINDOW                              (4)
#define MATRIX_IP_CAM_EVENT_DETECT_TAG			"event-type="
#define MATRIX_IP_CAM_EVENT_SEPERATOR_TAG		','

#define	MATRIX_IP_CAM_STATIC_IP_ADDR			(0)
#define	MATRIX_IP_CAM_ALARM_OUT_TIME_INTERVAL	(0)	// in Sec

#define MOTION_AREA_BLOCK_BYTES_11x9            (13)
#define MOTION_AREA_BLOCK_BYTES_24x18           (54)

#define MATRIX_IP_CAM_IMAGE_SETTING_TEMPLATE    "template-index=0"

#define HAVE_CAMERA_CAPABILITY(model, cap)      ((MatrixModelInfo[model].modelParameter->capabilitySupported & cap) ? TRUE : FALSE)
#define BOOL_STR(value)                         ((value) ? "true" : "false")

//#################################################################################################
// @DATA_TYPES
//#################################################################################################
typedef struct
{
    CHARPTR rootFolder;
    CHARPTR cgiFolder;
    CHARPTR	isapiFolder;
    CHARPTR	streamingFolder;
    CHARPTR	channelsFolder;
    CHARPTR systemFolder;
    CHARPTR	networkFolder;
    CHARPTR interfaceFolder;
    CHARPTR typeFolder;
    CHARPTR ptzFolder;
    CHARPTR imageFolder;
    CHARPTR ptzContinous;
    CHARPTR iris;
    CHARPTR focus;
    CHARPTR focusConfiguration;
    CHARPTR presets;
    CHARPTR gotoPreset;

    CHARPTR imageCapture;
    CHARPTR eventFolder;
    CHARPTR ipAddrStr;

    CHARPTR videoFolder;
    CHARPTR inputFolder;
    CHARPTR overlayFolder;

    CHARPTR	IOFolder;
    CHARPTR	outputFolders;
    CHARPTR	triggerArg;

}MATRIX_OEM_CAM_REQ_PARAM_t;

typedef struct
{
    CHARPTR rootFolder;
    CHARPTR cgiFolder;
    CHARPTR streamProfile;
    CHARPTR action;
    CHARPTR profileNoArg;
    CHARPTR codecArg;
    CHARPTR resolutionArg;
    CHARPTR fpsArg;
    CHARPTR bitRateControlArg;
    CHARPTR bitRateArg;
    CHARPTR imageQualityArg;
    CHARPTR gopArg;

    CHARPTR snapshotArg;
    CHARPTR eventAndAction;
    CHARPTR getEventStatus;
    CHARPTR unicastStreamFolder;
    CHARPTR basicSetting;
    CHARPTR aboutFolder;
    CHARPTR modelNameArg;

    CHARPTR overlaySetting;
    CHARPTR osdArg;
    CHARPTR dateFormatArg;
    CHARPTR timeFormatArg;
    CHARPTR dispalyPosArg;
    CHARPTR textOverlayArg;
    CHARPTR textArg;
    CHARPTR textDisplayPosArg;

    CHARPTR motionDetectionFolder;
    CHARPTR motionDetectionEnableArg;
    CHARPTR motionCellsArg;
    CHARPTR sensitivityArg;
    CHARPTR motionEventTypeArg;
    CHARPTR noMotionEventDurationArg;
    CHARPTR startXArg;
    CHARPTR startYArg;
    CHARPTR widthArg;
    CHARPTR heightArg;

    CHARPTR privacyMaskArg;
    CHARPTR privacyMaskNoArg;
    CHARPTR user;
    CHARPTR userName;
    CHARPTR currPasswd;
    CHARPTR passwd;
    CHARPTR confPasswd;
    CHARPTR alarmOut;
    CHARPTR audio;
    CHARPTR timeArg;
    CHARPTR focus;
    CHARPTR step;

}MATRIX_IP_CAM_REQ_PARAM_t;

typedef enum
{
    MATRIX_CAMERA_EVENT_MOTION_DETECTION    = 1,
    MATRIX_CAMERA_EVENT_TEMPER_DETECTION    = 2,
    MATRIX_CAMERA_EVENT_LINE_CROSS          = 3,
    MATRIX_CAMERA_EVENT_OBJECT_INTRUSION    = 5,
    MATRIX_CAMERA_EVENT_MISSING_OBJECT      = 7,
    MATRIX_CAMERA_EVENT_SUSPICIOUS_OBJECT   = 8,
    MATRIX_CAMERA_EVENT_OBJECT_COUNTING     = 9,
    MATRIX_CAMERA_EVENT_SENSOR_INPUT_1      = 10,
    MATRIX_CAMERA_EVENT_AUDIO_EXCEPTION     = 11,
    MATRIX_CAMERA_EVENT_LOITERING           = 12,
    MATRIX_CAMERA_EVENT_NO_MOTION_DETECTION = 14,
    MATRIX_CAMERA_EVENT_SENSOR_INPUT_2      = 20,
    MATRIX_CAMERA_EVENT_MAX,

}MATRIX_CAMERA_EVENT_e;

typedef enum
{
    EVENT_TYPE = 0,
    EVENT_STATE = 1,
    MAX_XML_EVENT_TAG

}EVENT_TAG_INFO_e;

typedef enum
{
    CAMERA_OEM_HIKVISION = 0,
    CAMERA_OEM_TIANDY,
    CAMERA_OEM_BRAND_MAX

}CAMERA_OEM_BRAND_e;

typedef enum
{
    VIDEO_CODEC 			= 0,
    VIDEO_RESOLUTION_WIDTH	= 1,
    VIDEO_RESOLUTION_HEIGHT = 2,
    QUALITY_CONTROL_TYPE	= 3,
    BIT_RATE				= 4,
    QUALITY					= 5,
    FRAME_RATE				= 6,
    GOP						= 7,
    MAX_TAG_TO_SET

}STREAM_CONFIG_TAG_e;

typedef enum
{
    ACTION_GET			= 0,
    ACTION_SET			= 1,
    ACTION_STARTVIEW	= 2,
    ACTION_SET_PASSWD	= 3,
    ACTION_ACTIVATE		= 4,
    ACTION_DEACTIVATE	= 5,
    ACTION_GET_CAP      = 6,
    MAX_ACTION_FIELD

}MATRIX_IP_CAM_ACTION_TYPE_e;

typedef enum
{
    MATRIX_IP_CAM_H264 = 0,
    MATRIX_IP_CAM_MJPEG,
    MATRIX_IP_CAM_H265,
    MATRIX_IP_CAM_MAX_CODEC

}MATRIX_IP_CAM_CODEC_e;

typedef struct
{
    BOOL    isRangeParam;
    UINT32  minRange;
    UINT32  maxRange;

}IMAGE_SETTING_PARAM_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL isTiandyOemCamera(CAMERA_MODEL_e modelNo);
//-------------------------------------------------------------------------------------------------
static CAMERA_OEM_BRAND_e getOemCameraBrand(CAMERA_MODEL_e modelNo);
//-------------------------------------------------------------------------------------------------
static BOOL isMatrixOemCameraModel(CAMERA_MODEL_e modelNo);
//-------------------------------------------------------------------------------------------------
static BOOL isMatrixStandardCameraModel(CAMERA_MODEL_e modelNo);
//-------------------------------------------------------------------------------------------------
static BOOL isMatrixPremiumCameraModel(CAMERA_MODEL_e modelNo);
//-------------------------------------------------------------------------------------------------
static BOOL isMatrixPtzCameraModel(CAMERA_MODEL_e modelNo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getStreamUrlForMatrixCamera(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                        UINT8PTR numOfReq, BOOL considerCfg, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getStreamUrlMatrixOemCamera(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                        UINT8PTR numOfReq, BOOL considerConfig, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getStreamUrlMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                        UINT8PTR numOfReq, BOOL considerConfig, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getStreamUrlMatrixIpCameraProfessional(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                        UINT8PTR numOfReq, BOOL considerConfig, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static BOOL getCodecNoforMatrixIpCamera(CHARPTR codecStr, MATRIX_IP_CAM_CODEC_e *codecNo);
//-------------------------------------------------------------------------------------------------
static BOOL getBitrateNoforMatrixIpCamera(CHARPTR bitrateStr, BITRATE_VALUE_e *bitRateNo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getImageUrlForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getImageUrlMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getImageUrlMatrixIpCameraGeneral(URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getEvStatusForMatrixCamera(CAMERA_MODEL_e modelNo, CAMERA_EVENT_e camEvent, URL_REQUEST_t *urlReqPtr,
                                                   UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getEvStatusMatrixOemCamera(CAMERA_MODEL_e modelNo, CAMERA_EVENT_e camEvent, URL_REQUEST_t *urlReqPtr,
                                                   UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getEvStatusMatrixIpCameraGeneral(CAMERA_EVENT_e camEvent, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseEvStatusForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseEvStatusMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseEvStatusMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseEvStatusMatrixIpCameraProfessional(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getCurrentStreamCfgForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 profileIndex, URL_REQUEST_t *urlReqPtr,
                                                           UINT8PTR numOfReq, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getCurrentStreamCfgMatrixIpProfessional(CAMERA_MODEL_e modelNo, UINT8 profileIndex, URL_REQUEST_t *urlReqPtr,
                                                                UINT8PTR numOfReq, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getCurrentStreamCfgMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 profileIndex, URL_REQUEST_t *urlReqPtr,
                                                           UINT8PTR numOfReq, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getCurrentStreamCfgMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT8 profileIndex, URL_REQUEST_t *urlReqPtr,
                                                                 UINT8PTR numOfReq, VIDEO_TYPE_e streamType);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseStreamResponseMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                         CHARPTR data, UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseStreamResponseMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                           CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType, UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseStreamResponseMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                                 CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType,UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseStreamResponseMatrixIpCameraProfessional(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                                      CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType,UINT8 profileIndex);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getDeviceInfoForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getDeviceInfoForMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getDeviceInfoForMatrixIpCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseDeviceInfoMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseDeviceInfoMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseDeviceInfoForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e changeMatrixCamIpAddrUrl(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e changeCamIpAddrMatrixOemCamera(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e changeCamIpAddrMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam,
                                                             URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setOsdforMatrixCamera(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setOsdMatrixOemCamera(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setOsdMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getMotionWindowUrlforMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getMotionWindowUrlMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getMotionWindowUrlMatrixIpCameraGeneral(URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseGetMotionWindowForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseGetMotionWindowMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseGetMotionWindowMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseGetMotionWindowMatrixIpNetraCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setMotionWindowforMatrixCamera(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setMotionWindowMatrixOemCamera(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setMotionWindowMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf,
                                                             URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setMotionWindowMatrixIpNetraCameraGeneral(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf,
                                                                  URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getPrivacyMaskForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getPrivacyMaskMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getPrivacyMaskMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parsePrivacyMaskResponseForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                                PRIVACY_MASK_CONFIG_t *privacyMaskArea, CHARPTR *privacyMaskName);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parsePrivacyMaskResponseMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize,
                                                                CHARPTR data, PRIVACY_MASK_CONFIG_t *privacyMaskArea);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parsePrivacyMaskResponseMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize,
                                                                      CHARPTR data, PRIVACY_MASK_CONFIG_t *privacyMaskArea);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPrivacyMaskForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                      PRIVACY_MASK_CONFIG_t *privacyMaskArea, BOOL privacyMaskStatus, CHARPTR *privacyMaskName);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPrivacyMaskMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                      PRIVACY_MASK_CONFIG_t *privacyMaskArea, BOOL privacyMaskStatus);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPrivacyMaskMatrixIpCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                     PRIVACY_MASK_CONFIG_t *privacyMaskArea, BOOL privacyMaskStatus);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getMaxPrivacyMaskWindowForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e getMaxPrivacyMaskWindowIpStandardProfessionalCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPtzUrlForMatrixCamera(CAMERA_MODEL_e modelNo, PTZ_OPTION_e pan, PTZ_OPTION_e tilt,
                                                 PTZ_OPTION_e zoom, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPtzUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom,
                                                    BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPtzUrlForMatrixIpCamera(PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action, UINT8 speed,URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPtzUrlForMatrixPtzCamera(PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action, UINT8 speed,URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setIrisUrlForMatrixCamera(CAMERA_MODEL_e modelNo, CAMERA_IRIS_e iris, BOOL action, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setIrisUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, CAMERA_IRIS_e iris, BOOL action, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setFocusUrlForMatrixCamera(CAMERA_MODEL_e modelNo, CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setFocusUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setFocusUrlForMatrixIpCamera(CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e gotoPtzUrlForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e gotoPtzUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e gotoPtzUrlForMatrixPtzCamera(UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e storePtzUrlForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, CHAR *presetName, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e storePtzUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setAlarmUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 alarmIndex, BOOL alarmState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e storePtzUrlForMatrixPtzCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, CHARPTR presetName, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setAlarmUrlForMatrixIpCamera(UINT8 alarmIndex, BOOL alarmState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setAlarmOutForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 alarmIndex, BOOL alarmState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setPasswordUrlForMatrixCamera(CAMERA_MODEL_e brandNo,CHARPTR userName,CHARPTR passwd, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendAudioToMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                UINT8PTR numOfStopReq, SEND_AUDIO_INFO_t *sendAudInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendAudioToMatrixIpCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                  UINT8PTR numOfStopReq, SEND_AUDIO_INFO_t *sendAudInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e sendAudioToMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                   UINT8PTR numOfStopReq, SEND_AUDIO_INFO_t *sendAudInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setDateTimeToMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setDateTimeToMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e setDateTimeToMatrixCameraGeneral(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e imageSettingForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                    IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e imageSettingForMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                       IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e imageSettingForMatrixCameraGeneral(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                           IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseImageSettingForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                         IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseImageSettingForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                            IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
static NET_CMD_STATUS_e parseImageSettingForMatrixCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                                IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const MATRIX_OEM_CAM_REQ_PARAM_t MatrixOEM =
{
    .rootFolder         = "/",
    .cgiFolder          = "CGI/",
    .isapiFolder        = "ISAPI/",
    .streamingFolder    = "Streaming/",
    .channelsFolder     = "channels/",
    .systemFolder       = "System/",
    .networkFolder      = "Network/",
    .interfaceFolder    = "interfaces/",
    .typeFolder         = "type/",
    .ptzFolder          = "PTZCtrl/",
    .imageFolder        = "Image/",
    .ptzContinous       = "continuous",
    .iris               = "iris",
    .focus              = "focus",
    .imageCapture       = "picture/",
    .focusConfiguration = "focusConfiguration",
    .presets            = "presets/",
    .gotoPreset         = "goto",

    .eventFolder        = "Event/",

    .ipAddrStr          = "ipAddress",

    .videoFolder        = "Video/",
    .inputFolder        = "inputs/",
    .overlayFolder      = "overlays/",

    .IOFolder           = "IO/",
    .outputFolders      = "outputs/",
    .triggerArg         = "trigger",
};

static MATRIX_IP_CAM_REQ_PARAM_t MatrixIP =
{
    .rootFolder         = "/",
    .cgiFolder          = "matrix-cgi/",
    .streamProfile      = "streamprofile",
    .action             = "action",
    .profileNoArg       = "profile-no",
    .codecArg           = "codec",
    .resolutionArg      = "resolution",
    .fpsArg             = "fps",
    .bitRateControlArg  = "bit-rate-control",
    .bitRateArg         = "bit-rate",
    .imageQualityArg    = "image-quality",
    .gopArg             = "gop",

    .snapshotArg        = "snapshot",

    .eventAndAction     = "eventaction",
    .getEventStatus     = "geteventstatus",

    .unicastStreamFolder= "unicaststream/",

    .basicSetting       = "basic",

    .aboutFolder        = "about",
    .modelNameArg       = "model",

    .overlaySetting     = "overlay",
    .osdArg             = "date-time-overlay",
    .dateFormatArg      = "date-format",
    .timeFormatArg      = "time-format",
    .dispalyPosArg      = "display",
    .textOverlayArg     = "text-overlay",
    .textArg            = "text",
    .textDisplayPosArg  = "text-display",

    .motionDetectionFolder      = "motiondetection",
    .motionDetectionEnableArg   = "enable",
    .motionCellsArg     = "cells",
    .sensitivityArg     = "sensitivity",
    .motionEventTypeArg = "event-type",
    .noMotionEventDurationArg = "duration",
    .startXArg          = "x",
    .startYArg          = "y",
    .widthArg           = "w",
    .heightArg          = "h",

    .privacyMaskArg     = "privacy",
    .privacyMaskNoArg   = "privacy-mask",
    .user               = "users",
    .userName           = "user-name",
    .currPasswd         = "cur_passwd",
    .passwd             = "password",
    .confPasswd         = "conf_pass",
    .alarmOut           = "alarmoutput",
    .audio              = "audio",
    .timeArg            = "time",
    .focus              = "focus",
    .step               = "step",
};

static const CHARPTR codecStr[MAX_VIDEO_CODEC][CAMERA_OEM_BRAND_MAX] =
{
    {     "",              ""}, //VIDEO_CODEC_NONE
    {"MJPEG",    "MotionJPEG"}, //VIDEO_MJPG
    {"H.264",         "H.264"}, //VIDEO_H264
    {     "",              ""}, //VIDEO_MPEG4
    {     "",              ""}, //UNUSED
    {     "",         "H.265"}  //VIDEO_H265
};

static const CHARPTR channelNameStr[MAX_STREAM] =
{
    "MainStream",
    "SubStream"
};

static const CHARPTR bitRateType[MAX_BITRATE_MODE] =
{
    "VBR", "CBR"
};

static const CHARPTR providedBitrateVal[MAX_BITRATE_VALUE]=
{
    "32", "64", "128", "256", "384", "512", "768", "1024", "1536", "2048", "3072", "4096", "6144", "8192", "12288", "16384"
};

static const CHARPTR fixedQuality[] =
{
    "0", "1", "20", "40", "40", "50", "60", "80", "80", "100", "100"
};

static const CHARPTR fixedQualityString[TIANDY_CAMERA_FIXED_QUALITY_MAX] =
{
    "poor", "normal", "good", "better", "best"
};

static const CHARPTR evParseTag[MAX_XML_EVENT_TAG] =
{
    "eventType", "eventState"
};

static const CHARPTR evValue[MAX_CAMERA_EVENT][CAMERA_OEM_BRAND_MAX] =
{
    {"VMD",             "VMD"},                 /* 00: MOTION_DETECT        */
    {"shelteralarm",    "videoDetection"},      /* 01: VIEW_TEMPERING       */
    {"IO",              NULL},                  /* 02: CAMERA_SENSOR_1      */
    {"IO",              NULL},                  /* 03: CAMERA_SENSOR_2      */
    {NULL,              NULL},                  /* 04: CAMERA_SENSOR_3      */
    {NULL,              NULL},                  /* 05: CONNECTION_FAILURE   */
    {NULL,              NULL},                  /* 06: RECORDING_FAIL       */
    {"linedetection",   "linedetection"},       /* 07: LINE_CROSS           */
    {"fielddetection",  "fielddetection"},      /* 08: OBJECT_INTRUSION     */
    {"audioexception",  "audioDetection"},      /* 09: AUDIO_EXCEPTION      */
    {NULL,              "attendedBaggage"},     /* 10: MISSING_OBJECT       */
    {NULL,              "unattendedBaggage"},   /* 11: SUSPICIOUS_OBJECT    */
    {NULL,              "loitering"},           /* 12: LOITERING            */
    {NULL,              NULL},                  /* 13: CAMERA_ONLINE        */
    {NULL,              NULL},                  /* 14: RECORDING_START      */
    {NULL,              NULL},                  /* 15: OBJECT_COUNTING      */
    {NULL,              NULL}                   /* 16: NO_MOTION_DETECTION  */
};

static const CHARPTR streamCnfgResponseTag[MAX_TAG_TO_SET] =
{
    "videoCodecType",			//index 0
    "videoResolutionWidth",		//index 1
    "videoResolutionHeight",	//index 2
    "videoQualityControlType",	//index 3
    "constantBitRate",			//index 4
    "fixedQuality",				//index 5
    "maxFrameRate",				//index 6
    "GovLength"					//index 7
};

static const CHARPTR osdMatrixDateTimeFormatOem[MAX_DATE_FORMAT][CAMERA_OEM_BRAND_MAX] =
{
    {"DD-MM-YYYY",      "DD/MM/YYYY"},
    {"MM-DD-YYYY",      "MM/DD/YYYY"},
    {"YYYY-MM-DD",      "YYYY/MM/DD"},
    {"CHR-DD/MM/YYYY",  "DD/MM/YYYY"}
};

static const CHARPTR timeFormatStr[] = {"12hour", "24hour"};

static const CHARPTR matrixIpCamAction[MAX_ACTION_FIELD] =
{
    "get",						//0
    "set",						//1
    "startview",				//2
    "changepwd",				//3
    "activate",					//4
    "deactivate",				//5
    "getcapabilities",          //6
};

static const UINT8 sensitivityTableOem[11][CAMERA_OEM_BRAND_MAX] =
{
    { 0,    0},
    { 20,  10},
    { 20,  20},
    { 40,  30},
    { 40,  40},
    { 60,  50},
    { 60,  60},
    { 80,  70},
    { 80,  80},
    {100,  90},
    {100, 100}
};

static const INT16 panSpeedStep[MAX_PTZ_PAN_OPTION] = {-10, 10};

static const INT16 tiltSpeedStep[MAX_PTZ_TILT_OPTION] = {10, -10};

static const INT16 zoomSpeedStep[MAX_PTZ_ZOOM_OPTION] = {-10, 10};

static const INT16 focusSpeedStep[MAX_FOCUS_OPTION] = {-10, 10, 0};

static const INT16 irisSpeedStep[MAX_IRIS_OPTION] = {-10, 10, 0};

// For NVR to IP cam mapping
static const CHARPTR codecStrForMatrixIP[MATRIX_IP_CAM_MAX_CODEC] =
{
    "H264",         //index 0
    "Motion JPEG",	//index 1
    "H265",
};

// date format for OSD
static const CHARPTR dateFormatForSetMatrixIP[MAX_DATE_FORMAT] =
{
    "DD/MM/YYYY",		//index 0
    "MM/DD/YYYY",		//index 1
    "YYYY/MM/DD",		//index 2
    "WWW%20DD/MM/YYYY",	//index 2
};

// time format for OSD
static const CHARPTR timeFormatForSetMatrixIP[MAX_TIME_FORMAT] =
{
    "12%20hour",		//index 0
    "24%20hour",		//index 1
};

static const CHARPTR outputTriggerStatus[2] =
{
    "low",
    "high",
};

static const CHARPTR responseStatusStr[MAX_MATRIX_SDK] = {"statusCode", "response-code"};

static const CHARPTR imageSettingReqStr[CAM_REQ_IMG_MAX] =
{
    "appearance",
    "advancedappearance",
    "daynight",
};

static const CHARPTR imageSettingParamStr[IMAGE_SETTING_CAPABILITY_MAX][MAX_MATRIX_SDK] =
{
    {"brightnessLevel",     "brightness"},
    {"contrastLevel",       "contrast"},
    {"saturationLevel",     "saturation"},
    {"hueLevel",            "hue"},
    {"sharpnessLevel",      "sharpness"},
    {"WhiteBalanceStyle",   "wb"},
    {"WDR",                 "wdr"},
    {"WDRLevel",            "dwdr-strength"},
    {"",                    "backlight-control"},
    {"",                    "exposure-ratio-mode"},
    {"",                    "exposure-ratio"},
    {"",                    "exposure-mode"},
    {"",                    "flicker"},
    {"",                    "flicker-strength"},
    {"",                    "hlc"},
    {"",                    "time"},
    {"",                    "gain"},
    {"",                    "p-iris-state"},
    {"",                    "normal-gain"},
    {"",                    "normal-average-luminance"},
    {"",                    "ir-leds"},
    {"",                    "sensitivity_level"},
};

static const CHARPTR imageSettingOemWbStr[WHITE_BALANCE_MODE_MAX][CAMERA_OEM_BRAND_MAX] =
{
    {"auto",         "auto"},
    {"daylightLamp", "fluorescent_lamp"},
    {"sodiumlight",  "warm_light"},
    {"outdoor",      "sunny"},
};

static const CHARPTR imageSettingOemWdrStr[WDR_MODE_MAX] ={"close", "open", "auto"};

static const IMAGE_SETTING_PARAM_t imageSettingParamRange[IMAGE_SETTING_CAPABILITY_MAX] =
{
    {TRUE,  BRIGHTNESS_MIN,                 BRIGHTNESS_MAX},                /* Brigthness */
    {TRUE,  CONTRAST_MIN,                   CONTRAST_MAX},                  /* Contrast */
    {TRUE,  SATURATION_MIN,                 SATURATION_MAX},                /* Saturation */
    {TRUE,  HUE_MIN,                        HUE_MAX},                       /* Hue */
    {TRUE,  SHARPNESS_MIN,                  SHARPNESS_MAX},                 /* Sharpness */
    {FALSE, WHITE_BALANCE_MODE_AUTO,        WHITE_BALANCE_MODE_MAX - 1},    /* White Balance */
    {FALSE, WDR_MODE_OFF,                   WDR_MODE_MAX - 1},              /* Wide Dynamic Range */
    {TRUE,  WDR_STRENGTH_MIN,               WDR_STRENGTH_MAX},              /* D-WDR Strength */
    {FALSE, BACKLIGHT_MODE_OFF,             BACKLIGHT_MODE_MAX - 1},        /* Backlight Control */
    {FALSE, EXPOSURE_RATIO_MODE_AUTO,       EXPOSURE_RATIO_MODE_MAX - 1},   /* Exposure Ratio Mode */
    {TRUE,  EXPOSURE_RATIO_MIN,             EXPOSURE_RATIO_MAX},            /* Exposure Ratio */
    {FALSE, EXPOSURE_MODE_AUTO,             EXPOSURE_MODE_MANUAL - 1},      /* Exposure Mode */
    {FALSE, FLICKER_MODE_NONE,              FLICKER_MODE_MAX - 1},          /* Flicker Mode */
    {TRUE,  FLICKER_STRENGTH_MIN,           FLICKER_STRENGTH_MAX},          /* Flicker Strength */
    {FALSE, HLC_MODE_OFF,                   HLC_MODE_MAX - 1},              /* HLC */
    {TRUE,  EXPOSURE_TIME_MIN,              EXPOSURE_TIME_MAX},             /* Exposure Time */
    {TRUE,  EXPOSURE_GAIN_MIN,              EXPOSURE_GAIN_MAX},             /* Exposure Gain */
    {TRUE,  EXPOSURE_IRIS_MIN,              EXPOSURE_IRIS_MAX},             /* Exposure Iris */
    {FALSE, NORMAL_LIGHT_GAIN_16X,          NORMAL_LIGHT_GAIN_MAX - 1},     /* Normal Light Gain */
    {TRUE,  NORMAL_LIGHT_LUMINANCE_MIN,     NORMAL_LIGHT_LUMINANCE_MAX},    /* Normal Light luminance */
    {FALSE, LED_MODE_AUTO,                  LED_MODE_MAX - 1},              /* LED Modes */
    {TRUE,  LED_SENSITIVITY_MIN,            LED_SENSITIVITY_MAX},           /* LED Sensitivity */
};

//#################################################################################################
// @GLOBAL VARIABLES
//#################################################################################################
const PARSER_FUNC 						MatrixParser = parseEvStatusForMatrixCamera;
const PARSER_CONFIG_FUNC 				MatrixCnfgParser = parseStreamResponseMatrixCamera;
const PARSER_DEVICE_INFO_FUNC 			MatrixDeviceInfoParser = parseDeviceInfoForMatrixCamera;
const PARSER_PRIVACYMSAK_RESPONSE 		MatrixPrivacyMaskParser = parsePrivacyMaskResponseForMatrixCamera;
const PARSER_GET_MOTION_WINDOW_RESPONSE MatrixGetMotionWindowParser = parseGetMotionWindowForMatrixCamera;
const PARSER_IMAGE_SETTING_RESPONSE     MatrixImageSettingParser = parseImageSettingForMatrixCamera;

const URL_FUNCTION_t MatrixUrl =
{
    .toGetStream                = getStreamUrlForMatrixCamera,
    .toGetImage                 = getImageUrlForMatrixCamera,
    .toSetPtz                   = setPtzUrlForMatrixCamera,
    .toStorePtz                 = storePtzUrlForMatrixCamera,
    .toGotoPtz                  = gotoPtzUrlForMatrixCamera,
    .toSetIris                  = setIrisUrlForMatrixCamera,
    .toSetFocus                 = setFocusUrlForMatrixCamera,
    .toSetAlarm                 = setAlarmOutForMatrixCamera,
    .toReqEvent                 = getEvStatusForMatrixCamera,
    .toGetCurrStrmCfg           = getCurrentStreamCfgForMatrixCamera,
    .toChangeCamIpAddr          = changeMatrixCamIpAddrUrl,
    .toGetDeviceInfo            = getDeviceInfoForMatrixCamera,
    .toSetOsd                   = setOsdforMatrixCamera,
    .toGetMotionWindow          = getMotionWindowUrlforMatrixCamera,
    .toSetMotionWindow          = setMotionWindowforMatrixCamera,
    .toGetPrivacyMask           = getPrivacyMaskForMatrixCamera,
    .toSetPrivacyMask           = setPrivacyMaskForMatrixCamera,
    .toSendAudio                = sendAudioToMatrixCamera,
    .toSetPasswd                = setPasswordUrlForMatrixCamera,
    .toGetMaxPrivacyMaskWindow	= getMaxPrivacyMaskWindowForMatrixCamera,
    .toSetDateTime              = setDateTimeToMatrixCamera,
    .toImageSetting             = imageSettingForMatrixCamera,
};

const CHARPTR actualCodecStr[MAX_VIDEO_CODEC]=
{
    "",				//VIDEO_CODEC_NONE  index 0
    "Motion JPEG",	//VIDEO_MJPG 	  	index 1
    "H264",			//VIDEO_H264 		index 2
    "MPEG4",		//VIDEO_MPEG4 		index 3
    "",				//UNUSED            index 4
    "H265"			//VIDEO_H265        index 5
};

// For NVR to IP cam mapping
const CHARPTR resolutionStrForMatrixIP[MATRIX_IP_CAM_MAX_RESOLUTION] =
{
    "3840x2160",
    "3200x1800",
    "2592x1944",
    "2592x1520",
    "2048x1536",
    "1920x1080",
    "1280x720",
    "704x576",
    "704x288",
    "640x480",
    "352x288",
    "176x144",
};

// For IP to NVR mapping
const CHARPTR resolutionStrForSetMatrixIP[MATRIX_IP_CAM_MAX_RESOLUTION] =
{
    "3840x2160",
    "3200x1800",
    "2592x1944",
    "2592x1520",
    "2048x1536",
    "1920x1080",
    "720p",
    "D1",
    "2CIF",
    "VGA",
    "CIF",
    "QCIF",
};

// For IP to NVR mapping
const UINT32 bitRateValueForMatrixIP[MAX_BITRATE_VALUE] =
{
    32,         //index 0
    64,         //index 1
    128,		//index 2
    256,		//index 3
    384,		//index 4
    512,		//index 5
    768,		//index 6
    1024,		//index 7
    1536,		//index 8
    2048,		//index 9
    3072,		//index 10
    4096,		//index 11
    6144,		//index 12
    8192,		//index 13
    12288,		//index 14
    16384,		//index 15
};

const CHARPTR bitRateModeStrForMatrixIP[MAX_BITRATE_MODE] =
{
    "VBR",		//index 0
    "CBR",		//index 1
};

// For NVR to IP cam mapping
const CHARPTR codecStrForSetMatrixIP[MATRIX_IP_CAM_MAX_CODEC] =
{
    "H.264",		//index 0
    "MJPEG",	    //index 1
    "H.265",		//index 2
};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Check whether it is tiandy camera model or not
 * @param   modelNo
 * @return
 */
static BOOL isTiandyOemCamera(CAMERA_MODEL_e modelNo)
{
    if ((modelNo == MATRIX_MODEL_SATATYA_PZCR20ML_25CWP) || (modelNo == MATRIX_MODEL_SATATYA_PZCR20ML_33CWP))
    {
        return TRUE;
    }

    return FALSE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get camera OEM brand
 * @param   modelNo
 * @return  Camera OEM brand name
 */
static CAMERA_OEM_BRAND_e getOemCameraBrand(CAMERA_MODEL_e modelNo)
{
    if (TRUE == isTiandyOemCamera(modelNo))
    {
        return CAMERA_OEM_TIANDY;
    }

    return CAMERA_OEM_HIKVISION;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getTheBitRateModeNum
 * @param   tempArray
 * @param   mode
 */
static void getTheBitRateModeNum(CHARPTR tempArray, BITRATE_MODE_e *mode)
{
    UINT8 setCnt;

    for (setCnt = VBR; setCnt < MAX_BITRATE_MODE; setCnt++)
    {
        if (strcmp(tempArray, bitRateType[setCnt]) == FALSE)
        {
            *mode = setCnt;
            break;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function gives value of requested tag from given source XML data.
 * @param   source
 * @param   tag
 * @param   dest
 * @param   maxSize
 * @param   ignoreCase
 * @return  SUCCESS/FAIL
 */
static BOOL getXMLvalue(CHARPTR *source, CHARPTR tag, CHARPTR dest, UINT16 maxSize, BOOL ignoreCase)
{
    UINT16  destSize;
    CHARPTR	startPtr;
    CHARPTR	endPtr;

    // Find Tag in Source
    startPtr = (TRUE == ignoreCase) ? strcasestr(*source, tag) : strstr(*source, tag);
    if (startPtr == NULL)
    {
        return FAIL;
    }

    /* Jump after tag */
    startPtr += strlen(tag);

    // Find tag value
    startPtr = strchr(startPtr, '>');
    if (startPtr == NULL)
    {
        return FAIL;
    }

    // Find End of Tag
    startPtr++;
    endPtr = strchr(startPtr, '<');
    if (endPtr == NULL)
    {
        return FAIL;
    }

    /* here the size is kept +1 to fill up the null char at the end of actual string */
    destSize = (endPtr - startPtr) + 1;
    if (destSize < maxSize)
    {
        maxSize = destSize;
    }

    /* Copy tag value */
    snprintf(dest, maxSize, "%s", startPtr);

    /* Set next tag position */
    startPtr = strchr(endPtr, '>');
    if (startPtr == NULL)
    {
        *source = endPtr;
    }
    else
    {
        *source = startPtr;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is it matrix's oem camera model?
 * @param   modelNo
 * @return  Returns TRUE if OEM camera model else returns FALSE
 */
static BOOL isMatrixOemCameraModel(CAMERA_MODEL_e modelNo)
{
    switch(modelNo)
    {
        case MATRIX_MODEL_SATATYA_PZCR20ML_25CWP:
        case MATRIX_MODEL_SATATYA_PZCR20ML_33CWP:
        case MATRIX_MODEL_SATATYA_CIBR13FL_40CW:
        case MATRIX_MODEL_SATATYA_CIDR13FL_40CW:
        case MATRIX_MODEL_SATATYA_CIDRP20VL_130CW:
            return TRUE;

        default:
            return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is it matrix's standard camera model?
 * @param   modelNo
 * @return  Returns TRUE if standard camera model else returns FALSE
 */
static BOOL isMatrixStandardCameraModel(CAMERA_MODEL_e modelNo)
{
    switch(modelNo)
    {
        case MATRIX_MODEL_SATATYA_CIBR30FL_36CG:
        case MATRIX_MODEL_SATATYA_CIBR30FL_60CG:
        case MATRIX_MODEL_SATATYA_CIDR30FL_36CW:
        case MATRIX_MODEL_SATATYA_CIDR30FL_60CW:
        case MATRIX_MODEL_SATATYA_CIDR20FL_36CW_S:
        case MATRIX_MODEL_SATATYA_CIDR20FL_60CW_S:
        case MATRIX_MODEL_SATATYA_CIDR20VL_12CW_S:
        case MATRIX_MODEL_SATATYA_CIBR20FL_36CW_S:
        case MATRIX_MODEL_SATATYA_CIBR20FL_60CW_S:
        case MATRIX_MODEL_SATATYA_CIBR20VL_12CW_S:
        case MATRIX_MODEL_SATATYA_CIDR30FL_36CW_S:
        case MATRIX_MODEL_SATATYA_CIDR30FL_60CW_S:
        case MATRIX_MODEL_SATATYA_CIDR30VL_12CW_S:
        case MATRIX_MODEL_SATATYA_CIBR30FL_36CW_S:
        case MATRIX_MODEL_SATATYA_CIBR30FL_60CW_S:
        case MATRIX_MODEL_SATATYA_CIBR30VL_12CW_S:
        case MATRIX_MODEL_SATATYA_CIDR20FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIBR20FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIDR30FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIBR30FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIDR20FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIDR20FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MIDR20FL_60CW_S:

        case MATRIX_MODEL_SATATYA_MIDR30FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIDR30FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MIDR30FL_60CW_S:
        case MATRIX_MODEL_SATATYA_MIBR30FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIBR30FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MIBR30FL_60CW_S:
        case MATRIX_MODEL_SATATYA_MIBR20FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIBR20FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MIBR20FL_60CW_S:

        //5 MP Camera Standard Variants of Netra Models
        case MATRIX_MODEL_SATATYA_CIBR50FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIBR50FL_40CW_S:
        case MATRIX_MODEL_SATATYA_CIBR50FL_60CW_S:
        case MATRIX_MODEL_SATATYA_CIDR50FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIDR50FL_40CW_S:
        case MATRIX_MODEL_SATATYA_CIDR50FL_60CW_S:
        case MATRIX_MODEL_SATATYA_CIBR50VL_12CW_S:
        case MATRIX_MODEL_SATATYA_CIDR50VL_12CW_S:

        //5 MP Camera Standard Variants of Samiksha Models
        case MATRIX_MODEL_SATATYA_MIBR50FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIBR50FL_40CW_S:
        case MATRIX_MODEL_SATATYA_MIBR50FL_60CW_S:
        case MATRIX_MODEL_SATATYA_MIDR50FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIDR50FL_40CW_S:
        case MATRIX_MODEL_SATATYA_MIDR50FL_60CW_S:

        // NETRA-8MP Project Series Standard Variants
        case MATRIX_MODEL_SATATYA_CIBR80FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIBR80FL_36CW_S:
        case MATRIX_MODEL_SATATYA_CIBR80FL_60CW_S:

        case MATRIX_MODEL_SATATYA_CIDR80FL_28CW_S:
        case MATRIX_MODEL_SATATYA_CIDR80FL_36CW_S:
        case MATRIX_MODEL_SATATYA_CIDR80FL_60CW_S:

        // SAMIKSHA-8MP Professional Series Standard Variants
        case MATRIX_MODEL_SATATYA_MIBR80FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIBR80FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MIBR80FL_60CW_S:

        case MATRIX_MODEL_SATATYA_MIDR80FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MIDR80FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MIDR80FL_60CW_S:

        // TURRET-2MP Project Series Premium Variants
        case MATRIX_MODEL_SATATYA_MITR20FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MITR20FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MITR20FL_60CW_S:

        // TURRET-5MP Project Series Premium Variants
        case MATRIX_MODEL_SATATYA_MITR50FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MITR50FL_40CW_S:
        case MATRIX_MODEL_SATATYA_MITR50FL_60CW_S:

        // TURRET-2MP All Color
        case MATRIX_MODEL_SATATYA_MITC20FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MITC20FL_36CW_S:
        case MATRIX_MODEL_SATATYA_MITC20FL_60CW_S:

        // TURRET-5MP All Color
        case MATRIX_MODEL_SATATYA_MITC50FL_28CW_S:
        case MATRIX_MODEL_SATATYA_MITC50FL_40CW_S:
        case MATRIX_MODEL_SATATYA_MITC50FL_60CW_S:
            return TRUE;

        default:
            return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is it matrix's premium camera model?
 * @param   modelNo
 * @return  Returns TRUE if premium camera model else returns FALSE
 */
static BOOL isMatrixPremiumCameraModel(CAMERA_MODEL_e modelNo)
{
    switch(modelNo)
    {
        case MATRIX_MODEL_SATATYA_CIDR20VL_12CW_P:
        case MATRIX_MODEL_SATATYA_CIBR20VL_12CW_P:
        case MATRIX_MODEL_SATATYA_CIDR30VL_12CW_P:
        case MATRIX_MODEL_SATATYA_CIBR30VL_12CW_P:
        case MATRIX_MODEL_SATATYA_MIDR20FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIDR20FL_36CW_P:
        case MATRIX_MODEL_SATATYA_MIDR20FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIDR20FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIBR20FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIDR20FL_36CW_P:
        case MATRIX_MODEL_SATATYA_CIBR20FL_36CW_P:
        case MATRIX_MODEL_SATATYA_CIDR20FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIBR20FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIDR30FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIBR30FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIDR30FL_36CW_P:
        case MATRIX_MODEL_SATATYA_CIBR30FL_36CW_P:
        case MATRIX_MODEL_SATATYA_CIDR30FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIBR30FL_60CW_P:

        case MATRIX_MODEL_SATATYA_MIDR30FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIDR30FL_36CW_P:
        case MATRIX_MODEL_SATATYA_MIDR30FL_60CW_P:
        case MATRIX_MODEL_SATATYA_MIBR30FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIBR30FL_36CW_P:
        case MATRIX_MODEL_SATATYA_MIBR30FL_60CW_P:
        case MATRIX_MODEL_SATATYA_MIBR20FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIBR20FL_36CW_P:
        case MATRIX_MODEL_SATATYA_MIBR20FL_60CW_P:

        //5 MP Camera Premium Variants of Netra Models
        case MATRIX_MODEL_SATATYA_CIBR50FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIBR50FL_40CW_P:
        case MATRIX_MODEL_SATATYA_CIBR50FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIDR50FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIDR50FL_40CW_P:
        case MATRIX_MODEL_SATATYA_CIDR50FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIBR50VL_12CW_P:
        case MATRIX_MODEL_SATATYA_CIDR50VL_12CW_P:

        //5 MP Camera Premium Variants of Samiksha Models
        case MATRIX_MODEL_SATATYA_MIBR50FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIBR50FL_40CW_P:
        case MATRIX_MODEL_SATATYA_MIBR50FL_60CW_P:
        case MATRIX_MODEL_SATATYA_MIDR50FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIDR50FL_40CW_P:
        case MATRIX_MODEL_SATATYA_MIDR50FL_60CW_P:

        // NETRA-8MP Project Series Premium Variants
        case MATRIX_MODEL_SATATYA_CIBR80FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIBR80FL_36CW_P:
        case MATRIX_MODEL_SATATYA_CIBR80FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIBR80ML_12CW_P:

        case MATRIX_MODEL_SATATYA_CIDR80FL_28CW_P:
        case MATRIX_MODEL_SATATYA_CIDR80FL_36CW_P:
        case MATRIX_MODEL_SATATYA_CIDR80FL_60CW_P:
        case MATRIX_MODEL_SATATYA_CIDR80ML_12CW_P:

        // SAMIKSHA-8MP Professional Series Premium Variants
        case MATRIX_MODEL_SATATYA_MIBR80FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIBR80FL_36CW_P:
        case MATRIX_MODEL_SATATYA_MIBR80FL_60CW_P:

        case MATRIX_MODEL_SATATYA_MIDR80FL_28CW_P:
        case MATRIX_MODEL_SATATYA_MIDR80FL_36CW_P:
        case MATRIX_MODEL_SATATYA_MIDR80FL_60CW_P:

        // RUGGEDIZED-2MP
        case MATRIX_MODEL_SATATYA_RIDR20FL_28CW_P:
        case MATRIX_MODEL_SATATYA_RIDR20FL_36CW_P:
        case MATRIX_MODEL_SATATYA_RIDR20FL_60CW_P:

        // RUGGEDIZED-5MP
        case MATRIX_MODEL_SATATYA_RIDR50FL_28CW_P:
        case MATRIX_MODEL_SATATYA_RIDR50FL_40CW_P:
        case MATRIX_MODEL_SATATYA_RIDR50FL_60CW_P:
            return TRUE;

        default:
            return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Is it matrix's PTZ camera model?
 * @param   modelNo
 * @return  Returns TRUE if PTZ camera model else returns FALSE
 */
static BOOL isMatrixPtzCameraModel(CAMERA_MODEL_e modelNo)
{
    switch(modelNo)
    {
        case MATRIX_MODEL_SATATYA_PTZ_2040_P:
            return TRUE;

        default:
            return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   streamConfig
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   considerConfig
 * @param   streamType
 * @return  Network command status
 */
static NET_CMD_STATUS_e getStreamUrlForMatrixCamera(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                                                    UINT8PTR numOfReq, BOOL considerConfig,VIDEO_TYPE_e streamType)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return getStreamUrlMatrixOemCamera(modelNo, streamConfig, urlReqPtr, numOfReq, considerConfig, streamType);
    }
    else if (TRUE == isMatrixStandardCameraModel(modelNo))
    {
        return getStreamUrlMatrixIpCameraGeneral(modelNo, streamConfig, urlReqPtr, numOfReq, considerConfig, streamType);
    }
    else if ((TRUE == isMatrixPremiumCameraModel(modelNo)) || (TRUE == isMatrixPtzCameraModel(modelNo)))
    {
        return getStreamUrlMatrixIpCameraProfessional(modelNo, streamConfig, urlReqPtr, numOfReq, considerConfig, streamType);
    }

    return CMD_PROCESS_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   streamConfig
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   considerConfig
 * @param   streamType
 * @return  Network command status
 */
static NET_CMD_STATUS_e getStreamUrlMatrixOemCamera(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                                                    UINT8PTR numOfReq, BOOL considerConfig, VIDEO_TYPE_e streamType)
{
    UINT16 			 	reqCnt = 0;
    UINT8 				profileIndex;
    STREAM_CODEC_TYPE_e codecNo;
    CHAR 				putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t				bufSize = 0;
    CHAR 				putFileName[MAX_FILE_NAME_LEN];
    static UINT32		putStreamFileCnt = 0;
    INT32				putFileFd = INVALID_FILE_FD;
    CHAR                videoWidth[MAX_RES_STRING];
    CHAR                videoHeight[MAX_RES_STRING];
    CHAR 				*pVideoEncoding;
    CHAR 				*pResolution;
    UINT8 				framerate;
    UINT8 				quality;
    BOOL 				enableAudio;
    BITRATE_MODE_e 		bitrateMode;
    BITRATE_VALUE_e 	bitrateValue;
    UINT8 				gop;

    if (streamType == MAIN_STREAM)
    {
        profileIndex = streamConfig->mainStreamProfile + 100;
    }
    else if (streamType == SUB_STREAM)
    {
        profileIndex = streamConfig->subStreamProfile + 100;
    }
    else
    {
        EPRINT(CAMERA_INTERFACE, "invld stream found: [streamType=%d]", streamType);
        return CMD_PROCESS_ERROR;
    }

    if (considerConfig == TRUE)
    {
        if (streamType == MAIN_STREAM)
        {
            pVideoEncoding = streamConfig->videoEncoding;
            pResolution = streamConfig->resolution;
            framerate = streamConfig->framerate;
            quality = streamConfig->quality;
            enableAudio = streamConfig->enableAudio;
            bitrateMode = streamConfig->bitrateMode;
            bitrateValue = streamConfig->bitrateValue;
            gop = streamConfig->gop;
        }
        else
        {
            pVideoEncoding = streamConfig->videoEncodingSub;
            pResolution = streamConfig->resolutionSub;
            framerate = streamConfig->framerateSub;
            quality = streamConfig->qualitySub;
            enableAudio = streamConfig->enableAudioSub;
            bitrateMode = streamConfig->bitrateModeSub;
            bitrateValue = streamConfig->bitrateValueSub;
            gop = streamConfig->gopSub;
        }

        if (GetVideoCodecNum(pVideoEncoding, &codecNo) == FAIL)
        {
            EPRINT(CAMERA_INTERFACE, "invld codec type: [videoEncoding=%s]", pVideoEncoding);
            return CMD_PROCESS_ERROR;
        }

        if (GetResolutionHeightWidthString(pResolution, videoHeight, videoWidth, sizeof(videoHeight), sizeof(videoWidth)) != TRUE)
        {
            EPRINT(CAMERA_INTERFACE, "invld resolution: [resolution=%s]", pResolution);
            return CMD_PROCESS_ERROR;
        }

        if(modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
        {
            if ((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, STREAM_PARA_HDR_STR_OEM(STD_CGI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else
        {
            if ((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, STREAM_PARA_HDR_STR_OEM(ISAPI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, ID_TAG_STR, (streamType == MAIN_STREAM)?streamConfig->mainStreamProfile:streamConfig->subStreamProfile )) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, DEFAULT_CHANNEL_STRING, channelNameStr[streamType])) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, ID_TAG_STR, profileIndex)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            /* for other camera channel name was 'Camera 01' previously so kept as it is */
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, DEFAULT_CHANNEL_STRING, "Camera 01")) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, ENABLE_FIELD_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PROTOCOL_LIST)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, ENABLE_FIELD_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_CHANNEL_ID_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_CODEC_TYPE_STR, codecStr[codecNo][getOemCameraBrand(modelNo)])) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_SCAN_TYPE_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_RESOLUTION_WIDTH_STR, videoWidth)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_RESOLUTION_HEIGHT_STR, videoHeight)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_QUALITY_CONTROL_TYPE_STR, bitRateType[bitrateMode])) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if (bitrateMode == CBR)
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, CONST_BIT_RATE_STR, providedBitrateVal[bitrateValue])) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if (bitrateMode == VBR)
        {
            if (TRUE == isTiandyOemCamera(modelNo))
            {
                if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, FIXED_QUALITY, fixedQualityString[quality-1])) > MAX_PUT_REQ_BUF_SIZE)
                {
                    return CMD_PROCESS_ERROR;
                }
            }
            else
            {
                if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, FIXED_QUALITY, fixedQuality[quality])) > MAX_PUT_REQ_BUF_SIZE)
                {
                    return CMD_PROCESS_ERROR;
                }
            }
        }

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, MAX_FRAME_RATE_STR, framerate)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, MAX_FRAME_RATE_STR, (framerate * 100))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, SNAP_SHOT_IMAGE_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, GOV_LENGTH_STR, gop)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, CODEC_PROFILE)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, SCALABLE_VIDEO_CODING_STR)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, NP_MODE_STR)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PRIORITY_MODE_STR)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, CHANNEL_TYPE_STR)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, SPLUS_265_STR)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_QUALITY_CONTROL_TYPE_STR, bitRateType[bitrateMode])) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VBR_LIMIT_STR)) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, CONST_BIT_RATE_STR, providedBitrateVal[bitrateValue])) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, VIDEO_END_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, AUDIO_CHANNEL_STR, BOOL_STR(enableAudio))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if (modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
        {
            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, AUDIO_STR, BOOL_STR(enableAudio))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, STREAM_END_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        snprintf(putFileName, sizeof(putFileName), PUT_STREAM_FILE_NAME, putStreamFileCnt++);
        putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
        if (putFileFd == INVALID_FILE_FD)
        {
            EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
            return CMD_PROCESS_ERROR;
        }

        if (write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
        {
            EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
            close(putFileFd);
            return CMD_PROCESS_ERROR;
        }

        close(putFileFd);

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            /* http://<ip>:<port>/CGI/Streaming/channels/<id>/type/<id> */
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d%c%s%d",
                     MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.streamingFolder, MatrixOEM.channelsFolder,
                     1, DIRECTORY_DELIM_CHAR, MatrixOEM.typeFolder, (streamType == MAIN_STREAM) ? streamConfig->mainStreamProfile : streamConfig->subStreamProfile);
        }
        else
        {
            /* http://<ip>:<port>/ISAPI/Streaming/channels/<id> */
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d",
                     MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.streamingFolder, MatrixOEM.channelsFolder, profileIndex);
        }

        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
        urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
        reqCnt++;
    }

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        /* Note: Tiandy provides main stream only for 2nd stream and 3rd stream through BM APIs.
         * Hence we have replaced, stream URL with ONVIF RTSP stream URL to make it work */
        /* http://<ip>:<port>/1/<profileIndex> */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%c%d%c%d",
                 DIRECTORY_DELIM_CHAR,1,DIRECTORY_DELIM_CHAR,(streamType == MAIN_STREAM) ? streamConfig->mainStreamProfile : streamConfig->subStreamProfile);

        #if 0
        /* http://<ip>:<port>/CGI/Streaming/channels/<id>/type/<id> */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d%c%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.streamingFolder, MatrixOEM.channelsFolder,
                 (streamType == MAIN_STREAM) ? streamConfig->mainStreamProfile : streamConfig->subStreamProfile, DIRECTORY_DELIM_CHAR, MatrixOEM.typeFolder, 1);
        #endif
    }
    else
    {
        /* http://<ip>:<port>/ISAPI/Streaming/channels/<id> */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.streamingFolder, MatrixOEM.channelsFolder, profileIndex);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_RTSP_PROTOCOL;
    urlReqPtr[reqCnt].rtspTransportType = TCP_INTERLEAVED;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    DPRINT(CAMERA_INTERFACE, "stream url: [streamType=%d,] [considerConfig=%d], [numOfReq=%d]",streamType,considerConfig, *numOfReq);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   streamConfig
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   considerConfig
 * @param   streamType
 * @return  Network command status
 */
static NET_CMD_STATUS_e getStreamUrlMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t *streamConfig, URL_REQUEST_t *urlReqPtr,
                                                          UINT8PTR numOfReq, BOOL considerConfig, VIDEO_TYPE_e streamType)
{
    UINT32                      outLen = 0;
    UINT16 			 			reqCnt = 0;
    MATRIX_IP_CAM_CODEC_e 		codecNo;
    MATRIX_IP_CAM_RESOLUTION_e	resolutionNo = 0;

    if(streamType == MAIN_STREAM)
    {
        // stream parameter set URL
        if(considerConfig == TRUE)
        {
            if(GetResolutionNoforMatrixIpCamera(streamConfig->resolution, &resolutionNo) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld resolution: [resolution=%s]", streamConfig->resolution);
                return CMD_PROCESS_ERROR;
            }

            if(getCodecNoforMatrixIpCamera(streamConfig->videoEncoding, &codecNo) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld codec type: [videoEncoding=%s]", streamConfig->videoEncoding);
                return CMD_PROCESS_ERROR;
            }

            outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                              "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d",
                              MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.streamProfile, URL_DELIM,
                              MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                              MatrixIP.profileNoArg, ASSIGN_VAL, (streamConfig->mainStreamProfile - 1), ARG_DELIM,
                              MatrixIP.codecArg, ASSIGN_VAL, codecStrForSetMatrixIP[codecNo], ARG_DELIM,
                              MatrixIP.resolutionArg, ASSIGN_VAL, resolutionStrForSetMatrixIP[resolutionNo], ARG_DELIM,
                              MatrixIP.fpsArg, ASSIGN_VAL, streamConfig->framerate, ARG_DELIM,
                              MatrixIP.bitRateControlArg, ASSIGN_VAL, bitRateModeStrForMatrixIP[streamConfig->bitrateMode], ARG_DELIM,
                              MatrixIP.bitRateArg, ASSIGN_VAL, bitRateValueForMatrixIP[streamConfig->bitrateValue], ARG_DELIM,
                              MatrixIP.gopArg, ASSIGN_VAL, streamConfig->gop);

            if (streamConfig->bitrateMode == VBR)
            {
                snprintf(&urlReqPtr[reqCnt].relativeUrl[outLen], MAX_CAMERA_URI_WIDTH - reqCnt, "%c" "%s%c%d",
                         ARG_DELIM, MatrixIP.imageQualityArg, ASSIGN_VAL, (streamConfig->quality - 1));
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            reqCnt++;
        }
    }
    else
    {
        // stream parameter set URL
        if(considerConfig == TRUE)
        {
            if(GetResolutionNoforMatrixIpCamera(streamConfig->resolutionSub, &resolutionNo) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld resolution: [resolution=%s]", streamConfig->resolutionSub);
                return CMD_PROCESS_ERROR;
            }

            if(getCodecNoforMatrixIpCamera(streamConfig->videoEncodingSub, &codecNo ) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld codec type: [videoEncoding=%s]", streamConfig->videoEncodingSub);
                return CMD_PROCESS_ERROR;
            }

            outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                              "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d",
                              MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.streamProfile, URL_DELIM,
                              MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                              MatrixIP.profileNoArg, ASSIGN_VAL, (streamConfig->subStreamProfile - 1), ARG_DELIM,
                              MatrixIP.codecArg, ASSIGN_VAL, codecStrForSetMatrixIP[codecNo], ARG_DELIM,
                              MatrixIP.resolutionArg, ASSIGN_VAL, resolutionStrForSetMatrixIP[resolutionNo], ARG_DELIM,
                              MatrixIP.fpsArg, ASSIGN_VAL, streamConfig->framerateSub, ARG_DELIM,
                              MatrixIP.bitRateControlArg, ASSIGN_VAL, bitRateModeStrForMatrixIP[streamConfig->bitrateModeSub], ARG_DELIM,
                              MatrixIP.bitRateArg, ASSIGN_VAL, bitRateValueForMatrixIP[streamConfig->bitrateValueSub], ARG_DELIM,
                              MatrixIP.gopArg, ASSIGN_VAL, streamConfig->gopSub);

            if (streamConfig->bitrateModeSub == VBR)
            {
                snprintf(&urlReqPtr[reqCnt].relativeUrl[outLen], MAX_CAMERA_URI_WIDTH - reqCnt, "%c" "%s%c%d",
                         ARG_DELIM, MatrixIP.imageQualityArg, ASSIGN_VAL, (streamConfig->qualitySub - 1));
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            reqCnt++;
        }
    }

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%d", MatrixIP.rootFolder, MatrixIP.unicastStreamFolder,
             (streamType ? streamConfig->subStreamProfile : streamConfig->mainStreamProfile));

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_RTSP_PROTOCOL;
    urlReqPtr[reqCnt].rtspTransportType = TCP_INTERLEAVED;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   streamConfig
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   considerConfig
 * @param   streamType
 * @return  Network command status
 */
static NET_CMD_STATUS_e getStreamUrlMatrixIpCameraProfessional(CAMERA_MODEL_e modelNo, STREAM_CONFIG_t* streamConfig, URL_REQUEST_t *urlReqPtr,
                                                               UINT8PTR numOfReq, BOOL considerConfig, VIDEO_TYPE_e streamType)
{
    UINT32                      outLen = 0;
    UINT16 			 			reqCnt = 0;
    MATRIX_IP_CAM_CODEC_e 		codecNo;
    MATRIX_IP_CAM_RESOLUTION_e	resolutionNo = 0;

    if(streamType == MAIN_STREAM)
    {
        // stream parameter set URL
        if(considerConfig == TRUE)
        {
            if(GetResolutionNoforMatrixIpCamera(streamConfig->resolution, &resolutionNo) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld resolution: [resolution=%s]", streamConfig->resolution);
                return CMD_PROCESS_ERROR;
            }

            if(getCodecNoforMatrixIpCamera(streamConfig->videoEncoding, &codecNo ) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld codec type: [videoEncoding=%s]", streamConfig->videoEncoding);
                return CMD_PROCESS_ERROR;
            }

            outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                              "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%d",
                              MatrixIP.rootFolder,
                              MatrixIP.cgiFolder, MatrixIP.streamProfile, URL_DELIM,
                              MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                              MatrixIP.profileNoArg, ASSIGN_VAL, (streamConfig->mainStreamProfile - 1), ARG_DELIM,
                              MatrixIP.codecArg, ASSIGN_VAL, codecStrForSetMatrixIP[codecNo], ARG_DELIM,
                              MatrixIP.resolutionArg, ASSIGN_VAL, resolutionStrForSetMatrixIP[resolutionNo], ARG_DELIM,
                              MatrixIP.fpsArg, ASSIGN_VAL, streamConfig->framerate, ARG_DELIM,
                              MatrixIP.bitRateControlArg, ASSIGN_VAL, bitRateModeStrForMatrixIP[streamConfig->bitrateMode], ARG_DELIM,
                              MatrixIP.bitRateArg, ASSIGN_VAL, bitRateValueForMatrixIP[streamConfig->bitrateValue], ARG_DELIM,
                              MatrixIP.gopArg, ASSIGN_VAL, streamConfig->gop, ARG_DELIM,
                              MatrixIP.audio, ASSIGN_VAL, streamConfig->enableAudio);

            if (streamConfig->bitrateMode == VBR)
            {
                snprintf(&urlReqPtr[reqCnt].relativeUrl[outLen], MAX_CAMERA_URI_WIDTH - reqCnt, "%c" "%s%c%d",
                         ARG_DELIM, MatrixIP.imageQualityArg, ASSIGN_VAL, (streamConfig->quality - 1));
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            reqCnt++;
        }
    }
    else
    {
        // stream parameter set URL
        if(considerConfig == TRUE)
        {
            if(GetResolutionNoforMatrixIpCamera(streamConfig->resolutionSub, &resolutionNo) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld resolution: [resolution=%s]", streamConfig->resolutionSub);
                return CMD_PROCESS_ERROR;
            }

            if(getCodecNoforMatrixIpCamera(streamConfig->videoEncodingSub, &codecNo ) != SUCCESS)
            {
                EPRINT(CAMERA_INTERFACE, "invld codec type: [videoEncoding=%s]", streamConfig->videoEncodingSub);
                return CMD_PROCESS_ERROR;
            }

            outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                              "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%d",
                              MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.streamProfile, URL_DELIM,
                              MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                              MatrixIP.profileNoArg, ASSIGN_VAL, (streamConfig->subStreamProfile - 1), ARG_DELIM,
                              MatrixIP.codecArg, ASSIGN_VAL, codecStrForSetMatrixIP[codecNo], ARG_DELIM,
                              MatrixIP.resolutionArg, ASSIGN_VAL, resolutionStrForSetMatrixIP[resolutionNo], ARG_DELIM,
                              MatrixIP.fpsArg, ASSIGN_VAL, streamConfig->framerateSub, ARG_DELIM,
                              MatrixIP.bitRateControlArg, ASSIGN_VAL, bitRateModeStrForMatrixIP[streamConfig->bitrateModeSub], ARG_DELIM,
                              MatrixIP.bitRateArg, ASSIGN_VAL, bitRateValueForMatrixIP[streamConfig->bitrateValueSub], ARG_DELIM,
                              MatrixIP.gopArg, ASSIGN_VAL, streamConfig->gopSub, ARG_DELIM,
                              MatrixIP.audio, ASSIGN_VAL, streamConfig->enableAudioSub);

            if (streamConfig->bitrateModeSub == VBR)
            {
                snprintf(&urlReqPtr[reqCnt].relativeUrl[outLen], MAX_CAMERA_URI_WIDTH - reqCnt, "%c" "%s%c%d",
                         ARG_DELIM, MatrixIP.imageQualityArg, ASSIGN_VAL, (streamConfig->qualitySub - 1));
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            reqCnt++;
        }
    }

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%d", MatrixIP.rootFolder, MatrixIP.unicastStreamFolder,
             (streamType ? streamConfig->subStreamProfile : streamConfig->mainStreamProfile));

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_RTSP_PROTOCOL;
    urlReqPtr[reqCnt].rtspTransportType = TCP_INTERLEAVED;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetResolutionNoforMatrixIpCam
 * @param   resolutionStr
 * @param   resolutionNo
 * @return  SUCCESS/FAIL
 */
BOOL GetResolutionNoforMatrixIpCamera(CHARPTR resolutionStr, MATRIX_IP_CAM_RESOLUTION_e* resolutionNo)
{
    MATRIX_IP_CAM_RESOLUTION_e frameRes = ConvertStringToIndex(resolutionStr, resolutionStrForMatrixIP, MATRIX_IP_CAM_MAX_RESOLUTION);

    if (frameRes >= MATRIX_IP_CAM_MAX_RESOLUTION)
    {
        return FAIL;
    }

    *resolutionNo = frameRes;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetResolutionNoGetProfileforMatrixIpCam
 * @param   resolutionStr
 * @param   resolutionNo
 * @return  SUCCESS/FAIL
 */
BOOL GetResolutionNoGetProfileforMatrixIpCamera(CHARPTR resolutionStr, MATRIX_IP_CAM_RESOLUTION_e* resolutionNo)
{
    MATRIX_IP_CAM_RESOLUTION_e frameRes = ConvertStringToIndex(resolutionStr, resolutionStrForSetMatrixIP, MATRIX_IP_CAM_MAX_RESOLUTION);

    if (frameRes >= MATRIX_IP_CAM_MAX_RESOLUTION)
    {
        return FAIL;
    }

    *resolutionNo = frameRes;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetCodecNoforMatrixIpCam
 * @param   codecStr
 * @param   codecNo
 * @return  SUCCESS/FAIL
 */
static BOOL getCodecNoforMatrixIpCamera(CHARPTR codecStr, MATRIX_IP_CAM_CODEC_e* codecNo)
{
    MATRIX_IP_CAM_CODEC_e frameCodec = ConvertStringToIndex(codecStr, codecStrForMatrixIP, MATRIX_IP_CAM_MAX_CODEC);

    if (frameCodec >= MATRIX_IP_CAM_MAX_CODEC)
    {
        return FAIL;
    }

    *codecNo = frameCodec;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetBitrateNoforMatrixIpCam
 * @param   bitrateStr
 * @param   bitRateNo
 * @return  SUCCESS/FAIL
 */
static BOOL getBitrateNoforMatrixIpCamera(CHARPTR bitrateStr, BITRATE_VALUE_e* bitRateNo)
{
    UINT8   bitRate;
    CHAR    data[15];

    for(bitRate = 0; bitRate < MAX_BITRATE_VALUE; bitRate++)
    {
        snprintf(data, sizeof(data), "%d", bitRateValueForMatrixIP[bitRate]);
        if(strcmp(bitrateStr, data) == STATUS_OK)
        {
            break;
        }
    }

    if (bitRate >= MAX_BITRATE_VALUE)
    {
        return FAIL;
    }

    *bitRateNo = bitRate;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API constructs the URL/s, required to get image from zavio camera model type1.
 *          It also specifies the protocol to be used,	request type, transport method [if any]
 *          and number of URLs that is output.
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return  Network command status
 */
static NET_CMD_STATUS_e getImageUrlForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return getImageUrlMatrixOemCamera(modelNo, urlReqPtr, numOfReq);
    }
    else
    {
        return getImageUrlMatrixIpCameraGeneral(urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API constructs the URL/s, required to get image from zavio/hikvision/tiandy camera model type.
 *          It also specifies the protocol to be used,	request type, transport method [if any]
 *          and number of URLs that is output.
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return  Network command status
 */
static NET_CMD_STATUS_e getImageUrlMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        /* Note: URL is not available in API document. It is given over mail by tiandy */
        /* http://<ip>:<port>/CGI/Image/Channels/<ID>/Shoot */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                 "%s%s%s%s%s%c%s", MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.imageFolder,
                 "Channels/", PROFILE_ONE, DIRECTORY_DELIM_CHAR, "Shoot");
    }
    else
    {
        /* http://<ip>:<port>/ISAPI/Streaming/channels/<id>/picture/ */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                 "%s%s%s%s%s%c%s", MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.streamingFolder,
                 MatrixOEM.channelsFolder, PROFILE_ONE, DIRECTORY_DELIM_CHAR, MatrixOEM.imageCapture);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API constructs the URL/s, required to get image from zavio camera model type1.
 *          It also specifies the protocol to be used,	request type, transport method [if any]
 *          and number of URLs that is output.
 * @param   urlReqPtr
 * @param   numOfReq
 * @return  Network command status
 */
static NET_CMD_STATUS_e getImageUrlMatrixIpCameraGeneral(URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    // http://<ip>:<port>/matrix-cgi/snapshot
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s%s%s%c%s%c%s", MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.snapshotArg,
             URL_DELIM, MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET]);

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API constructs the URL/s, required to set normal state of alarm of specified index
 *          in matrix ip camera professinal models. It also specifies the protocol to be used, request
 *          type / transport method [if any] and number of URLs that is output.
 * @param   urlReqPtr
 * @param   alarmState
 * @param   numOfReq
 * @return  Network command status
 */
static NET_CMD_STATUS_e setAlarmUrlForMatrixIpCamera(UINT8 alarmIndex, BOOL alarmState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    /* http://<ip>:<port>/matrix-cgi/alarmoutput?action=activate&time=0&alarm_no=0 */
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s%s%s%c%s%c%s%c%s%c%d%c%s%c%d",
             MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.alarmOut, URL_DELIM,
             MatrixIP.action, ASSIGN_VAL, alarmState == ACTIVE ? matrixIpCamAction[ACTION_ACTIVATE] : matrixIpCamAction[ACTION_DEACTIVATE],
             ARG_DELIM, MatrixIP.timeArg, ASSIGN_VAL, MATRIX_IP_CAM_ALARM_OUT_TIME_INTERVAL, ARG_DELIM, "alarm_no", ASSIGN_VAL, alarmIndex);

    urlReqPtr[reqCnt].requestType = (alarmState == ACTIVE ? CAM_ALARM_ACTIVE : CAM_ALARM_INACTIVE);
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API constructs the URL/s, required to set normal state of alarm. It also specifies
 *          the protocol to be used, request type / transport method [if any] and number of URLs that is output.
 * @param   modelNo
 * @param   alarmIndex
 * @param   alarmState
 * @param   urlReqPtr
 * @param   numOfReq
 * @return  Network command status
 */
static NET_CMD_STATUS_e setAlarmUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 alarmIndex, BOOL alarmState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    INT16           reqCnt = 0;
    CHAR            putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t          bufSize = 0;
    CHAR            putFileName[MAX_FILE_NAME_LEN];
    INT32           putFileFd = INVALID_FILE_FD;
    static UINT32   putOutputTrgFileCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OUTPUT_PORT_TRIGGER_DEFAULT)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OUTPUT_PORT_TRIGGER_ID, outputTriggerStatus[alarmState])) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OUTPUT_TRIGGER_DEFAULT)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OUTPUT_TRIGGER_ID, outputTriggerStatus[alarmState])) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    snprintf(putFileName, sizeof(putFileName), PUT_SET_OUTPUT_TRIGGER_FILE_NAME, putOutputTrgFileCnt++);
    putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if (write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }
    close(putFileFd);

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        //http://<ip>:<port>/CGI/System/IO/outputs/<ID>
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%d", MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.systemFolder,
                 MatrixOEM.IOFolder, MatrixOEM.outputFolders, (alarmIndex + 1));
    }
    else
    {
        //http://<ip>:<port>/ISAPI/System/IO/outputs/1/trigger
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%d%c%s", MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder,
                 MatrixOEM.IOFolder, MatrixOEM.outputFolders, (alarmIndex + 1), DIRECTORY_DELIM_CHAR, MatrixOEM.triggerArg);
    }

    urlReqPtr[reqCnt].requestType = (alarmState == ACTIVE ? CAM_ALARM_ACTIVE : CAM_ALARM_INACTIVE);
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to Set Alarm Port.
 * @param   modelNo
 * @param   alarmIndex
 * @param   alarmState
 * @param   urlReqPtr
 * @param   numOfReq
 * @return  Network command status
 */
static NET_CMD_STATUS_e setAlarmOutForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 alarmIndex, BOOL alarmState, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (modelNo >= MAX_MATRIX_CAMERA_MODEL)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == HAVE_CAMERA_CAPABILITY(modelNo, CAMERA_ALARM_OUTPUT1_SUPPORT))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setAlarmUrlForMatrixOemCamera(modelNo, alarmIndex, alarmState, urlReqPtr, numOfReq);
    }
    else
    {
        return setAlarmUrlForMatrixIpCamera(alarmIndex, alarmState, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getEvStatusForMatrixCamera
 * @param   modelNo
 * @param   camEvent
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   evRespInfo
 * @return  Network command status
 */
static NET_CMD_STATUS_e getEvStatusForMatrixCamera(CAMERA_MODEL_e modelNo, CAMERA_EVENT_e camEvent,
                                                   URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return getEvStatusMatrixOemCamera(modelNo, camEvent, urlReqPtr, numOfReq, evRespInfo);
    }
    else
    {
        return getEvStatusMatrixIpCameraGeneral(camEvent, urlReqPtr, numOfReq, evRespInfo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getEvStatusMatrixOemCamera
 * @param   modelNo
 * @param   camEvent
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   evRespInfo
 * @return  Network command status
 */
static NET_CMD_STATUS_e getEvStatusMatrixOemCamera(CAMERA_MODEL_e modelNo, CAMERA_EVENT_e camEvent,
                                                   URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo)
{
    UINT8 reqCnt = MAX_CAMERA_EVENT;

    evRespInfo->evRespType = EV_RESP_MULTI_PART;
    evRespInfo->evDetType = GET_MULTIPLE_EV_INFO;
    urlReqPtr[reqCnt].requestType = MAX_CAM_REQ_TYPE;

    if ((camEvent == MOTION_DETECT) || (camEvent == VIEW_TEMPERING) || (camEvent == CAMERA_SENSOR_1) || (camEvent == CAMERA_SENSOR_2)
            || (camEvent == LINE_CROSS) || (camEvent == OBJECT_INTRUSION) || (camEvent == AUDIO_EXCEPTION) || (camEvent == LOITERING))
    {
        if (TRUE == isTiandyOemCamera(modelNo))
        {
            /* http://<ip>:<port>/CGI/Event/notification/alertState */
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s",
                     MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.eventFolder, "subscription/alertState");
        }
        else
        {
            /* http://<ip>:<port>/ISAPI/Event/notification/alertStream */
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s",
                     MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.eventFolder, "notification/alertStream");
        }

        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
        reqCnt++;
    }
    else
    {
        *numOfReq = 0;
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getEvStatusMatrixIpCameraGeneral
 * @param   camEvent
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   evRespInfo
 * @return  Network command status
 */
static NET_CMD_STATUS_e getEvStatusMatrixIpCameraGeneral(CAMERA_EVENT_e camEvent, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, EVENT_RESP_INFO_t *evRespInfo)
{
    UINT8 reqCnt = MAX_CAMERA_EVENT;

    evRespInfo->evRespType = EV_RESP_MULTI_PART;
    evRespInfo->evDetType = GET_MULTIPLE_EV_INFO;
    urlReqPtr[reqCnt].requestType = MAX_CAM_REQ_TYPE;

    if ((camEvent == MOTION_DETECT) || (camEvent == NO_MOTION_DETECTION) || (camEvent == VIEW_TEMPERING) || (camEvent == LINE_CROSS)
            || (camEvent == AUDIO_EXCEPTION) || (camEvent == OBJECT_INTRUSION) || (camEvent == MISSING_OBJECT) || (camEvent == LOITERING)
            || (camEvent == SUSPICIOUS_OBJECT) || (camEvent == CAMERA_SENSOR_1) || (camEvent == CAMERA_SENSOR_2) || (camEvent == OBJECT_COUNTING))
    {
        /* http://<ip>:<port>/matrix-cgi/eventaction?action=geteventstatus */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%c%s%c%s",
                 MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.eventAndAction, URL_DELIM, MatrixIP.action, ASSIGN_VAL, MatrixIP.getEventStatus);

        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        reqCnt++;
    }
    else
    {
        *numOfReq = 0;
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseEvStatusForMatrixCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   eventData
 * @return  Network command status
 */
static NET_CMD_STATUS_e parseEvStatusForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return parseEvStatusMatrixOemCamera(modelNo, dataSize, data, eventData);
    }
    else if (TRUE == isMatrixStandardCameraModel(modelNo))
    {
        return parseEvStatusMatrixIpCameraGeneral(modelNo, dataSize, data, eventData);
    }
    else if ((TRUE == isMatrixPremiumCameraModel(modelNo)) || (TRUE == isMatrixPtzCameraModel(modelNo)))
    {
        return parseEvStatusMatrixIpCameraProfessional(modelNo, dataSize, data, eventData);
    }

    return CMD_PROCESS_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseEvStatusMatrixOemCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   eventData
 * @return  Network command status
 */
static NET_CMD_STATUS_e parseEvStatusMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData)
{
    EVENT_RESULT_t	*matrixEventResult = (EVENT_RESULT_t *)eventData;
    CAMERA_EVENT_e  cameraEvent;
    CHAR            eventString[MAX_SIZE_OF_STRING];
    UINT8           sensorNo = 0;
    CHAR            sensorTag[100];
    UINT8           evtValIdx = getOemCameraBrand(modelNo);

    if (data == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    matrixEventResult->eventStatus[MOTION_DETECT] = INACTIVE;
    matrixEventResult->eventStatus[VIEW_TEMPERING] = INACTIVE;
    matrixEventResult->eventStatus[LINE_CROSS] = INACTIVE;
    matrixEventResult->eventStatus[OBJECT_INTRUSION] = INACTIVE;
    matrixEventResult->eventStatus[AUDIO_EXCEPTION] = INACTIVE;
    matrixEventResult->eventStatus[CAMERA_SENSOR_1] = INACTIVE;
    matrixEventResult->eventStatus[CAMERA_SENSOR_2] = INACTIVE;
    matrixEventResult->eventStatus[OBJECT_COUNTING] = INACTIVE;
    matrixEventResult->eventStatus[LOITERING] = INACTIVE;
    matrixEventResult->eventStatus[MISSING_OBJECT] = INACTIVE;
    matrixEventResult->eventStatus[SUSPICIOUS_OBJECT] = INACTIVE;
    matrixEventResult->eventStatus[NO_MOTION_DETECTION] = INACTIVE;

    //	the response is of type XML
    //	<EventNotificationAlert version="1.0" xmlns="http://www.hikvision.com/ver20/XMLSchema">
    //	<ipAddress>xxx.xxx.xxx.xxx</ipAddress>
    //	<portNo>yy</portNo>
    //	<protocol>HTTP</protocol>
    //	<macAddress>macAddr</macAddress>
    //	<channelID>1</channelID>
    //	<dateTime>2014-08-08T07:34:56-00:00</dateTime>
    //	<activePostCount>1</activePostCount>
    //	<eventType>VMD</eventType>
    //	<eventState>active</eventState>
    //	<eventDescription>Motion alarm</eventDescription>
    //	<eventType>fielddetection</eventType>
    //	<eventState>active</eventState>
    //	<eventDescription>fielddetection alarm</eventDescription>
    //	<DetectionRegionList>
    //	</DetectionRegionList>
    //	</EventNotificationAlert>
    while(TRUE)
    {
        /* Search "eventType" tag */
        if (getXMLvalue(&data, evParseTag[EVENT_TYPE], eventString, MAX_SIZE_OF_STRING, FALSE) == FAIL)
        {
            /* Tag not found */
            break;
        }

        /* Check all event tags */
        for (cameraEvent = MOTION_DETECT; cameraEvent < MAX_CAMERA_EVENT; cameraEvent++)
        {
            if (evValue[cameraEvent][evtValIdx] == NULL)
            {
                continue;
            }

            /* Compare with supported tag */
            if (strcmp(eventString, evValue[cameraEvent][evtValIdx]) == 0)
            {
                /* Supported tag found */
                break;
            }
        }

        /* Check supported tag found or not */
        if (cameraEvent >= MAX_CAMERA_EVENT)
        {
            /* This is not a supported tag */
            continue;
        }

        /* Search "eventState" of the tag */
        if (getXMLvalue(&data, evParseTag[EVENT_STATE], eventString, MAX_SIZE_OF_STRING, FALSE) == FAIL)
        {
            /* Tag not found */
            break;
        }

        /* Bydefault we consider all event inactive */
        if (strcmp(eventString, "active") != 0)
        {
            /* Nothing to do for inactive state */
            continue;
        }

        if ((cameraEvent == CAMERA_SENSOR_1) || (cameraEvent == CAMERA_SENSOR_2))
        {
            for(sensorNo = 0; sensorNo < 2; sensorNo++)
            {
                snprintf(sensorTag, sizeof(sensorTag), INPUT_TRIGGER_ID, (sensorNo + 1));
                if (strstr(data, sensorTag) != NULL)
                {
                    if ((cameraEvent + sensorNo) < MAX_CAMERA_EVENT)
                    {
                        matrixEventResult->eventStatus[cameraEvent + sensorNo] = ACTIVE;
                    }
                }
            }
        }
        else
        {
            matrixEventResult->eventStatus[cameraEvent] = ACTIVE;
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseEvStatusMatrixIpCameraGeneral
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   eventData
 * @return  Network command status
 */
static NET_CMD_STATUS_e parseEvStatusMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData)
{
    EVENT_RESULT_t		*matrixEventResult = (EVENT_RESULT_t *)eventData;
    CHARPTR				parseData = NULL;
    CHAR				eventState[10];
    INT32				eventCode;

    matrixEventResult->eventStatus[MOTION_DETECT] = INACTIVE;
    matrixEventResult->eventStatus[VIEW_TEMPERING] = INACTIVE;
    matrixEventResult->eventStatus[LINE_CROSS] = INACTIVE;
    matrixEventResult->eventStatus[OBJECT_INTRUSION] = INACTIVE;
    matrixEventResult->eventStatus[OBJECT_COUNTING] = INACTIVE;
    matrixEventResult->eventStatus[NO_MOTION_DETECTION] = INACTIVE;

    //event-type=0
    //{Note: Motion detected before 30 seconds}
    //event-type=1
    //motion-region=0
    //{Note: Motion detected in other window before 30 seconds}
    //event-type=1
    //motion-region=0,1
    //{Note: 30 seconds timer expired}
    //event-type=1
    //motion-region=0,1
    //{Note: Motion detection status changed before 30 seconds}
    //event-type=0
    //{Note: 30 seconds timer expired}
    //event-type=1,2
    //motion-region=0,1
    //{Note: Tamper status changed before 30 seconds}
    //event-type=1
    //motion-region=0,1

    if (data == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    parseData = strstr(data, MATRIX_IP_CAM_EVENT_DETECT_TAG);
    if(parseData == NULL)
    {
        return CMD_SUCCESS;
    }

    data += strlen(MATRIX_IP_CAM_EVENT_DETECT_TAG);
    while((parseData = strchr(data, MATRIX_IP_CAM_EVENT_SEPERATOR_TAG)) != NULL)
    {
        snprintf(eventState, (strlen(data) - strlen(parseData))+1, "%s", data);

        data += ((strlen(data) - strlen(parseData)) + 1);

        eventCode = atoi(eventState);

        if(eventCode == MATRIX_CAMERA_EVENT_MOTION_DETECTION)
        {
            matrixEventResult->eventStatus[MOTION_DETECT] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_TEMPER_DETECTION)
        {
            matrixEventResult->eventStatus[VIEW_TEMPERING] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_LINE_CROSS)
        {
            matrixEventResult->eventStatus[LINE_CROSS] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_INTRUSION)
        {
            matrixEventResult->eventStatus[OBJECT_INTRUSION] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_COUNTING)
        {
            matrixEventResult->eventStatus[OBJECT_COUNTING] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_NO_MOTION_DETECTION)
        {
            matrixEventResult->eventStatus[NO_MOTION_DETECTION] = ACTIVE;
        }
    }

    eventCode = atoi(data);

    if(eventCode == MATRIX_CAMERA_EVENT_MOTION_DETECTION)
    {
        matrixEventResult->eventStatus[MOTION_DETECT] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_TEMPER_DETECTION)
    {
        matrixEventResult->eventStatus[VIEW_TEMPERING] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_LINE_CROSS)
    {
        matrixEventResult->eventStatus[LINE_CROSS] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_INTRUSION)
    {
        matrixEventResult->eventStatus[OBJECT_INTRUSION] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_COUNTING)
    {
        matrixEventResult->eventStatus[OBJECT_COUNTING] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_NO_MOTION_DETECTION)
    {
        matrixEventResult->eventStatus[NO_MOTION_DETECTION] = ACTIVE;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseEvStatusMatrixIpCameraProfessional
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   eventData
 * @return  Network command status
 */
static NET_CMD_STATUS_e parseEvStatusMatrixIpCameraProfessional(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, VOIDPTR eventData)
{
    EVENT_RESULT_t		*matrixEventResult = (EVENT_RESULT_t *)eventData;
    CHARPTR				parseData = NULL;
    CHAR				eventState[10];
    INT32				eventCode;

    matrixEventResult->eventStatus[MOTION_DETECT] =  INACTIVE;
    matrixEventResult->eventStatus[VIEW_TEMPERING] = INACTIVE;
    matrixEventResult->eventStatus[LINE_CROSS] = INACTIVE;
    matrixEventResult->eventStatus[OBJECT_INTRUSION] = INACTIVE;
    matrixEventResult->eventStatus[AUDIO_EXCEPTION] = INACTIVE;
    matrixEventResult->eventStatus[MISSING_OBJECT] = INACTIVE;
    matrixEventResult->eventStatus[SUSPICIOUS_OBJECT] = INACTIVE;
    matrixEventResult->eventStatus[CAMERA_SENSOR_1] = INACTIVE;
    matrixEventResult->eventStatus[CAMERA_SENSOR_2] = INACTIVE;
    matrixEventResult->eventStatus[LOITERING] = INACTIVE;
    matrixEventResult->eventStatus[OBJECT_COUNTING] = INACTIVE;
    matrixEventResult->eventStatus[NO_MOTION_DETECTION] = INACTIVE;

    //event-type=0
    //{Note: Motion detected before 30 seconds}
    //event-type=1
    //motion-region=0
    //{Note: Motion detected in other window before 30 seconds}
    //event-type=1
    //motion-region=0,1
    //{Note: 30 seconds timer expired}
    //event-type=1
    //motion-region=0,1
    //{Note: Motion detection status changed before 30 seconds}
    //event-type=0
    //{Note: 30 seconds timer expired}
    //event-type=1,2
    //motion-region=0,1
    //{Note: Tamper status changed before 30 seconds}
    //event-type=1
    //motion-region=0,1

    if(data == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    parseData = strstr(data, MATRIX_IP_CAM_EVENT_DETECT_TAG);
    if (parseData == NULL)
    {
        return CMD_SUCCESS;
    }

    data += strlen(MATRIX_IP_CAM_EVENT_DETECT_TAG);
    while((parseData = strchr(data, MATRIX_IP_CAM_EVENT_SEPERATOR_TAG)) != NULL)
    {
        snprintf(eventState, (strlen(data) - strlen(parseData))+1, "%s", data);
        data += ((strlen(data) - strlen(parseData)) + 1);
        eventCode = atoi(eventState);

        if(eventCode == MATRIX_CAMERA_EVENT_MOTION_DETECTION)
        {
            matrixEventResult->eventStatus[MOTION_DETECT] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_TEMPER_DETECTION)
        {
            matrixEventResult->eventStatus[VIEW_TEMPERING] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_LINE_CROSS)
        {
            matrixEventResult->eventStatus[LINE_CROSS] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_AUDIO_EXCEPTION)
        {
            matrixEventResult->eventStatus[AUDIO_EXCEPTION] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_INTRUSION)
        {
            matrixEventResult->eventStatus[OBJECT_INTRUSION] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_MISSING_OBJECT)
        {
            matrixEventResult->eventStatus[MISSING_OBJECT] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_SUSPICIOUS_OBJECT)
        {
            matrixEventResult->eventStatus[SUSPICIOUS_OBJECT] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_SENSOR_INPUT_1)
        {
            matrixEventResult->eventStatus[CAMERA_SENSOR_1] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_SENSOR_INPUT_2)
        {
            matrixEventResult->eventStatus[CAMERA_SENSOR_2] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_LOITERING)
        {
            matrixEventResult->eventStatus[LOITERING] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_COUNTING)
        {
            matrixEventResult->eventStatus[OBJECT_COUNTING] = ACTIVE;
        }
        else if(eventCode == MATRIX_CAMERA_EVENT_NO_MOTION_DETECTION)
        {
            matrixEventResult->eventStatus[NO_MOTION_DETECTION] = ACTIVE;
        }
    }

    eventCode = atoi(data);

    if(eventCode == MATRIX_CAMERA_EVENT_MOTION_DETECTION)
    {
        matrixEventResult->eventStatus[MOTION_DETECT] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_TEMPER_DETECTION)
    {
        matrixEventResult->eventStatus[VIEW_TEMPERING] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_LINE_CROSS)
    {
        matrixEventResult->eventStatus[LINE_CROSS] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_AUDIO_EXCEPTION)
    {
        matrixEventResult->eventStatus[AUDIO_EXCEPTION] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_INTRUSION)
    {
        matrixEventResult->eventStatus[OBJECT_INTRUSION] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_MISSING_OBJECT)
    {
        matrixEventResult->eventStatus[MISSING_OBJECT] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_SUSPICIOUS_OBJECT)
    {
        matrixEventResult->eventStatus[SUSPICIOUS_OBJECT] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_SENSOR_INPUT_1)
    {
        matrixEventResult->eventStatus[CAMERA_SENSOR_1] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_SENSOR_INPUT_2)
    {
        matrixEventResult->eventStatus[CAMERA_SENSOR_2] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_LOITERING)
    {
        matrixEventResult->eventStatus[LOITERING] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_OBJECT_COUNTING)
    {
        matrixEventResult->eventStatus[OBJECT_COUNTING] = ACTIVE;
    }
    else if(eventCode == MATRIX_CAMERA_EVENT_NO_MOTION_DETECTION)
    {
        matrixEventResult->eventStatus[NO_MOTION_DETECTION] = ACTIVE;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   profileIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   streamType
 * @return
 */
static NET_CMD_STATUS_e getCurrentStreamCfgMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 profileIndex,
                                                           URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, VIDEO_TYPE_e streamType)
{
    UINT8 reqCnt = 0;

    if (streamType >= MAX_STREAM)
    {
        return CMD_PROCESS_ERROR;
    }

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        /* http://<ip>:<port>/CGI/Streaming/channels/<id>/type/<id> */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d%c%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.streamingFolder, MatrixOEM.channelsFolder,
                 1, DIRECTORY_DELIM_CHAR, MatrixOEM.typeFolder, profileIndex);
    }
    else
    {
        /* http://<ip>:<port>/ISAPI/Streaming/channels/<id> */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.streamingFolder, MatrixOEM.channelsFolder, profileIndex);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONFIG;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    DPRINT(CAMERA_INTERFACE, "get stream configuration: [streamType=%d], [totalRequest=%d]", streamType, *(numOfReq));
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   profileIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   streamType
 * @return
 */
static NET_CMD_STATUS_e getCurrentStreamCfgMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT8 profileIndex,
                                                                 URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, VIDEO_TYPE_e streamType)
{
    UINT8 reqCnt = 0;

    if(streamType >= MAX_STREAM)
    {
        return CMD_PROCESS_ERROR;
    }

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c",
             MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.streamProfile, URL_DELIM,
             MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET], ARG_DELIM,
             MatrixIP.profileNoArg, ASSIGN_VAL, (profileIndex - 1), ARG_DELIM,
             MatrixIP.codecArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.resolutionArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.fpsArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.bitRateControlArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.bitRateArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.imageQualityArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.gopArg, ASSIGN_VAL);

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONFIG;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   profileIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   streamType
 * @return
 */
static NET_CMD_STATUS_e getCurrentStreamCfgMatrixIpProfessional(CAMERA_MODEL_e modelNo, UINT8 profileIndex,
                                                                URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, VIDEO_TYPE_e streamType)
{
    UINT8 reqCnt = 0;

    if(streamType >= MAX_STREAM)
    {
        return CMD_PROCESS_ERROR;
    }

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c" "%c" "%s%c",
             MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.streamProfile, URL_DELIM,
             MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET], ARG_DELIM,
             MatrixIP.profileNoArg, ASSIGN_VAL, (profileIndex - 1), ARG_DELIM,
             MatrixIP.codecArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.resolutionArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.fpsArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.bitRateControlArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.bitRateArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.imageQualityArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.gopArg, ASSIGN_VAL, ARG_DELIM,
             MatrixIP.audio, ASSIGN_VAL);

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONFIG;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Based on Model Number this API calls respective function to constructs URL to get stream.
 * @param   modelNo
 * @param   profileIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   streamType
 * @return
 */
static NET_CMD_STATUS_e getCurrentStreamCfgForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 profileIndex,
                                                           URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq, VIDEO_TYPE_e streamType)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return getCurrentStreamCfgMatrixOemCamera(modelNo, profileIndex, urlReqPtr, numOfReq, streamType);
    }
    else if (TRUE == isMatrixStandardCameraModel(modelNo))
    {
        return getCurrentStreamCfgMatrixIpCameraGeneral(modelNo, profileIndex, urlReqPtr, numOfReq, streamType);
    }
    else if ((TRUE == isMatrixPremiumCameraModel(modelNo)) || (TRUE == isMatrixPtzCameraModel(modelNo)))
    {
        return getCurrentStreamCfgMatrixIpProfessional(modelNo, profileIndex, urlReqPtr, numOfReq, streamType);
    }

    return CMD_PROCESS_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse response of get stream settings parameters for matrix camera
 * @param   modelNo
 * @param   dataSize
 * @param   streamConfig
 * @param   data
 * @param   camIndex
 * @param   streamType
 * @param   profileIndex
 * @return
 */
static NET_CMD_STATUS_e parseStreamResponseMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                        CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType,UINT8 profileIndex)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return parseStreamResponseMatrixOemCamera(modelNo, dataSize, streamConfig, data, camIndex, streamType, profileIndex);
    }
    else if (TRUE == isMatrixStandardCameraModel(modelNo))
    {
        return parseStreamResponseMatrixIpCameraGeneral(modelNo, dataSize, streamConfig, data, camIndex, streamType, profileIndex);
    }
    else if ((TRUE == isMatrixPremiumCameraModel(modelNo)) || (TRUE == isMatrixPtzCameraModel(modelNo)))
    {
        return parseStreamResponseMatrixIpCameraProfessional(modelNo, dataSize, streamConfig, data, camIndex, streamType, profileIndex);
    }

    return CMD_PROCESS_ERROR;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse response of get stream settings parameters for matrix oem camera
 * @param   modelNo
 * @param   dataSize
 * @param   streamConfig
 * @param   data
 * @param   camIndex
 * @param   streamType
 * @param   profileIndex
 * @return
 */
static NET_CMD_STATUS_e parseStreamResponseMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                           CHARPTR data, UINT8 camIndex, VIDEO_TYPE_e streamType, UINT8 profileIndex)
{
    CHAR                width[5];
    CHAR                height[5];
    UINT64              tagValue;
    CHAR                tempArray[MAX_SIZE_OF_STRING];
    UINT8               setCnt;
    CHARPTR             searchStrPtr;
    UINT8               codecIdx = getOemCameraBrand(modelNo);
    STREAM_CODEC_TYPE_e codec;
    CHAR                *pVideoEncoding;
    CHAR 				*pResolution;
    UINT8 				*pFramerate;
    UINT8 				*pQuality;
    BITRATE_MODE_e 		*pBitrateMode;
    BITRATE_VALUE_e 	*pBitrateValue;
    UINT8 				*pGop;

    /* Copy all parameters */
    if (streamType == MAIN_STREAM)
    {
        pVideoEncoding = streamConfig->videoEncoding;
        pResolution = streamConfig->resolution;
        pFramerate = &streamConfig->framerate;
        pBitrateMode = &streamConfig->bitrateMode;
        pBitrateValue = &streamConfig->bitrateValue;
        pQuality = &streamConfig->quality;
        pGop = &streamConfig->gop;
    }
    else if (streamType == SUB_STREAM)
    {
        pVideoEncoding = streamConfig->videoEncodingSub;
        pResolution = streamConfig->resolutionSub;
        pFramerate = &streamConfig->framerateSub;
        pBitrateMode = &streamConfig->bitrateModeSub;
        pBitrateValue = &streamConfig->bitrateValueSub;
        pQuality = &streamConfig->qualitySub;
        pGop = &streamConfig->gopSub;
    }
    else
    {
        return CMD_PROCESS_ERROR;
    }

    for(setCnt = VIDEO_CODEC; setCnt < MAX_TAG_TO_SET; setCnt++)
    {
        searchStrPtr = data;
        if ((getXMLvalue(&searchStrPtr, streamCnfgResponseTag[setCnt], tempArray, MAX_SIZE_OF_STRING, FALSE)) == FALSE)
        {
            EPRINT(CAMERA_INTERFACE, "required tag-values not found: [camera=%d], [tag=%s]", camIndex, streamCnfgResponseTag[setCnt]);
            return CMD_PROCESS_ERROR;
        }

        switch(setCnt)
        {
            case VIDEO_CODEC:
            {
                for (codec = VIDEO_CODEC_NONE; codec < MAX_VIDEO_CODEC; codec++)
                {
                    if ((codecStr[codec][codecIdx] == NULL) || (codecStr[codec][codecIdx][0] == '\0'))
                    {
                        continue;
                    }

                    if (strncmp(tempArray, codecStr[codec][codecIdx], strlen(codecStr[codec][codecIdx])) == 0)
                    {
                        snprintf(pVideoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[codec]);
                        break;
                    }
                }

                if (codec == MAX_VIDEO_CODEC)
                {
                    EPRINT(CAMERA_INTERFACE, "Unknown codec string found: [%s]", tempArray);
                    pVideoEncoding[0] = '\0';
                }
            }
            break;

            case VIDEO_RESOLUTION_WIDTH:
            {
                snprintf(width, sizeof(width), "%s", tempArray);
            }
            break;

            case VIDEO_RESOLUTION_HEIGHT:
            {
                snprintf(height, sizeof(height), "%s", tempArray);
                snprintf(pResolution, MAX_RESOLUTION_NAME_LEN, "%sx%s", width, height);
            }
            break;

            case QUALITY_CONTROL_TYPE:
            {
                getTheBitRateModeNum(tempArray, pBitrateMode);
            }
            break;

            case BIT_RATE:
            {
                GetTheBitRateValue(tempArray, pBitrateValue);
            }
            break;

            case QUALITY:
            {
                if (TRUE == isTiandyOemCamera(modelNo))
                {
                    UINT8 quality;
                    *pQuality = 1;

                    for (quality = 0; quality < TIANDY_CAMERA_FIXED_QUALITY_MAX; quality++)
                    {
                        if (strcmp(tempArray, fixedQualityString[quality]) == 0)
                        {
                            *pQuality = quality+1;
                            break;
                        }
                    }
                }
                else
                {
                    AsciiToInt(tempArray, &tagValue);
                    *pQuality = (UINT8)(tagValue/10);
                    if (*pQuality == 0)
                    {
                        *pQuality = 1;
                    }
                }
            }
            break;

            case FRAME_RATE:
            {
                AsciiToInt(tempArray, &tagValue);
                if (TRUE == isTiandyOemCamera(modelNo))
                {
                    *pFramerate =(UINT8)(tagValue);
                }
                else
                {
                    *pFramerate =(UINT8)(tagValue/100);
                }
            }
            break;

            case GOP:
            {
                AsciiToInt(tempArray, &tagValue);
                if (tagValue < 5)
                {
                    tagValue = 5;
                }

                *pGop = (UINT8)tagValue;
            }
            break;

            default:
            {
                /* Do nothing */
            }
            break;
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse response of get stream settings parameters for matrix standard camera
 * @param   modelNo
 * @param   dataSize
 * @param   streamConfig
 * @param   data
 * @param   camIndex
 * @param   streamType
 * @param   profileIndex
 * @return
 */
static NET_CMD_STATUS_e parseStreamResponseMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                                 CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType,UINT8 profileIndex)
{
    CHAR                        oneLineData[200];
    CHARPTR                     mainData = NULL;
    CHARPTR                     linePtr = NULL;
    CHARPTR                     parseData = NULL;
    INT32                       convertedData = 0;
    STREAM_CONFIG_t             tmpStrmConfig;
    MATRIX_IP_CAM_RESOLUTION_e	resolutionNo;

    tmpStrmConfig.videoEncoding[0] = '\0';
    tmpStrmConfig.resolution[0] = '\0';
    tmpStrmConfig.framerate = INVALID_VALUE;
    tmpStrmConfig.bitrateMode = MAX_BITRATE_MODE;
    tmpStrmConfig.bitrateValue = MAX_BITRATE_VALUE;
    tmpStrmConfig.quality = INVALID_VALUE;
    tmpStrmConfig.gop = INVALID_VALUE;

    mainData = data;
    while((parseData = strstr(mainData, "\n")) != NULL)
    {
        parseData++;
        snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);

        mainData = data + (strlen(data) - strlen(parseData));
        linePtr = oneLineData;

        if((parseData = strstr(linePtr, MatrixIP.codecArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.codecArg) + 1);

            if(strstr(linePtr, codecStrForSetMatrixIP[MATRIX_IP_CAM_H264]) != NULL)
            {
                snprintf(tmpStrmConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_H264]);
            }
            else if(strstr(linePtr, codecStrForSetMatrixIP[MATRIX_IP_CAM_MJPEG]) != NULL)
            {
                snprintf(tmpStrmConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_MJPG]);
            }
            else if(strstr(linePtr, codecStrForSetMatrixIP[MATRIX_IP_CAM_H265]) != NULL)
            {
                snprintf(tmpStrmConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_H265]);
            }
            else
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.resolutionArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.resolutionArg) + 1);
            linePtr[strlen(linePtr) - 2] = '\0';
            if(GetResolutionNoGetProfileforMatrixIpCamera(linePtr, &resolutionNo) == SUCCESS)
            {
                snprintf(tmpStrmConfig.resolution, MAX_RESOLUTION_NAME_LEN, "%s", resolutionStrForMatrixIP[resolutionNo]);
            }
            else
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.fpsArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.fpsArg) + 1);
            convertedData = atoi(linePtr);
            tmpStrmConfig.framerate = convertedData;
        }
        else if(((parseData = strstr(linePtr, MatrixIP.bitRateControlArg)) != NULL))
        {
            linePtr += (strlen(MatrixIP.bitRateControlArg) + 1);
            linePtr[strlen(linePtr) - 2] = '\0';
            tmpStrmConfig.bitrateMode = ConvertStringToIndex(linePtr, bitRateModeStrForMatrixIP, MAX_BITRATE_MODE);
        }
        else if((parseData = strstr(linePtr, MatrixIP.bitRateArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.bitRateArg) + 1);
            linePtr[strlen(linePtr) - 2] = '\0';
            if(getBitrateNoforMatrixIpCamera(linePtr, &tmpStrmConfig.bitrateValue) != SUCCESS)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.imageQualityArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.imageQualityArg) + 1);
            convertedData = atoi(linePtr);
            tmpStrmConfig.quality = (convertedData + 1);
        }
        else if((parseData = strstr(linePtr, MatrixIP.gopArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.gopArg) + 1);
            convertedData = atoi(linePtr);
            tmpStrmConfig.gop = convertedData;
        }
        else if((parseData = strstr(linePtr, responseStatusStr[MATRIX_IP_SDK])) != NULL)
        {
            linePtr += (strlen(responseStatusStr[MATRIX_IP_SDK]) + 1);
            convertedData = atoi(linePtr);
            if(convertedData != 0)
            {
                return CMD_PROCESS_ERROR;
            }
        }
    }

    if ((tmpStrmConfig.videoEncoding[0] == '\0') || (tmpStrmConfig.resolution[0] == '\0')
            || (tmpStrmConfig.framerate == INVALID_VALUE) || (tmpStrmConfig.bitrateMode == MAX_BITRATE_MODE)
            || (tmpStrmConfig.bitrateValue == MAX_BITRATE_VALUE) || (tmpStrmConfig.quality == INVALID_VALUE) || (tmpStrmConfig.gop == INVALID_VALUE))
    {
        return CMD_PROCESS_ERROR;
    }

    if(streamType == MAIN_STREAM)
    {
        snprintf(streamConfig->videoEncoding, MAX_ENCODER_NAME_LEN, "%s", tmpStrmConfig.videoEncoding);
        snprintf(streamConfig->resolution, MAX_RESOLUTION_NAME_LEN, "%s", tmpStrmConfig.resolution);
        streamConfig->framerate = tmpStrmConfig.framerate;
        streamConfig->bitrateMode = tmpStrmConfig.bitrateMode;
        streamConfig->bitrateValue = tmpStrmConfig.bitrateValue;
        streamConfig->quality = tmpStrmConfig.quality;
        streamConfig->gop = tmpStrmConfig.gop;
    }
    else
    {
        snprintf(streamConfig->videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", tmpStrmConfig.videoEncoding);
        snprintf(streamConfig->resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", tmpStrmConfig.resolution);
        streamConfig->framerateSub = tmpStrmConfig.framerate;
        streamConfig->bitrateModeSub = tmpStrmConfig.bitrateMode;
        streamConfig->bitrateValueSub = tmpStrmConfig.bitrateValue;
        streamConfig->qualitySub = tmpStrmConfig.quality;
        streamConfig->gopSub = tmpStrmConfig.gop;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse response of get stream settings parameters for matrix premium camera
 * @param   modelNo
 * @param   dataSize
 * @param   streamConfig
 * @param   data
 * @param   camIndex
 * @param   streamType
 * @param   profileIndex
 * @return
 */
static NET_CMD_STATUS_e parseStreamResponseMatrixIpCameraProfessional(CAMERA_MODEL_e modelNo, UINT32 dataSize, STREAM_CONFIG_t *streamConfig,
                                                                      CHARPTR data, UINT8 camIndex,VIDEO_TYPE_e streamType,UINT8 profileIndex)
{
    CHAR                        oneLineData[200];
    CHARPTR                     mainData = NULL;
    CHARPTR                     linePtr = NULL;
    CHARPTR                     parseData = NULL;
    INT32                       convertedData = 0;
    STREAM_CONFIG_t             tmpStrmConfig;
    MATRIX_IP_CAM_RESOLUTION_e	resolutionNo;

    tmpStrmConfig.videoEncoding[0] = '\0';
    tmpStrmConfig.resolution[0] = '\0';
    tmpStrmConfig.framerate = INVALID_VALUE;
    tmpStrmConfig.bitrateMode = MAX_BITRATE_MODE;
    tmpStrmConfig.bitrateValue = MAX_BITRATE_VALUE;
    tmpStrmConfig.quality = INVALID_VALUE;
    tmpStrmConfig.gop = INVALID_VALUE;
    tmpStrmConfig.enableAudio = INVALID_VALUE;

    mainData = data;
    while((parseData = strstr(mainData, "\n")) != NULL)
    {
        parseData++;
        snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);

        mainData = data + (strlen(data) - strlen(parseData));
        linePtr = oneLineData;

        if((parseData = strstr(linePtr, MatrixIP.codecArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.codecArg) + 1);

            if(strstr(linePtr, codecStrForSetMatrixIP[MATRIX_IP_CAM_H264]) != NULL)
            {
                snprintf(tmpStrmConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_H264]);
            }
            else if(strstr(linePtr, codecStrForSetMatrixIP[MATRIX_IP_CAM_MJPEG]) != NULL)
            {
                snprintf(tmpStrmConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_MJPG]);
            }
            else if(strstr(linePtr, codecStrForSetMatrixIP[MATRIX_IP_CAM_H265]) != NULL)
            {
                snprintf(tmpStrmConfig.videoEncoding, MAX_ENCODER_NAME_LEN, "%s", actualCodecStr[VIDEO_H265]);
            }
            else
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.resolutionArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.resolutionArg) + 1);
            linePtr[strlen(linePtr) - 2] = '\0';
            if(GetResolutionNoGetProfileforMatrixIpCamera(linePtr, &resolutionNo) == SUCCESS)
            {
                snprintf(tmpStrmConfig.resolution, MAX_RESOLUTION_NAME_LEN, "%s", resolutionStrForMatrixIP[resolutionNo]);
            }
            else
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.fpsArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.fpsArg) + 1);
            convertedData = atoi(linePtr);
            tmpStrmConfig.framerate = convertedData;
        }
        else if(((parseData = strstr(linePtr, MatrixIP.bitRateControlArg)) != NULL))
        {
            linePtr += (strlen(MatrixIP.bitRateControlArg) + 1);
            linePtr[strlen(linePtr) - 2] = '\0';
            tmpStrmConfig.bitrateMode = ConvertStringToIndex(linePtr, bitRateModeStrForMatrixIP, MAX_BITRATE_MODE);
        }
        else if((parseData = strstr(linePtr, MatrixIP.bitRateArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.bitRateArg) + 1);
            linePtr[strlen(linePtr) - 2] = '\0';
            if(getBitrateNoforMatrixIpCamera(linePtr, &tmpStrmConfig.bitrateValue) != SUCCESS)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.imageQualityArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.imageQualityArg) + 1);
            convertedData = atoi(linePtr);
            tmpStrmConfig.quality = (convertedData + 1);
        }
        else if((parseData = strstr(linePtr, MatrixIP.gopArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.gopArg) + 1);
            convertedData = atoi(linePtr);
            tmpStrmConfig.gop = convertedData;
        }
        else if((parseData = strstr(linePtr, MatrixIP.audio)) != NULL)
        {
            linePtr += (strlen(MatrixIP.audio) + 1);
            convertedData = atoi(linePtr);
            if(streamType == MAIN_STREAM)
            {
                tmpStrmConfig.enableAudio = convertedData;
            }
            else
            {
                tmpStrmConfig.enableAudioSub = convertedData;
            }
        }
        else if((parseData = strstr(linePtr, responseStatusStr[MATRIX_IP_SDK])) != NULL)
        {
            linePtr += (strlen(responseStatusStr[MATRIX_IP_SDK]) + 1);
            convertedData = atoi(linePtr);
            if(convertedData != 0)
            {
                return CMD_PROCESS_ERROR;
            }
        }
    }

    if ((tmpStrmConfig.videoEncoding[0] == '\0') || (tmpStrmConfig.resolution[0] == '\0')
            || (tmpStrmConfig.framerate == INVALID_VALUE) || (tmpStrmConfig.bitrateMode == MAX_BITRATE_MODE)
            || (tmpStrmConfig.bitrateValue == MAX_BITRATE_VALUE) || (tmpStrmConfig.quality == INVALID_VALUE) || (tmpStrmConfig.gop == INVALID_VALUE))
    {
        return CMD_PROCESS_ERROR;
    }

    if(streamType == MAIN_STREAM)
    {
        snprintf(streamConfig->videoEncoding, MAX_ENCODER_NAME_LEN, "%s", tmpStrmConfig.videoEncoding);
        snprintf(streamConfig->resolution, MAX_RESOLUTION_NAME_LEN, "%s", tmpStrmConfig.resolution);
        streamConfig->framerate = tmpStrmConfig.framerate;
        streamConfig->bitrateMode = tmpStrmConfig.bitrateMode;
        streamConfig->bitrateValue = tmpStrmConfig.bitrateValue;
        streamConfig->quality = tmpStrmConfig.quality;
        streamConfig->gop = tmpStrmConfig.gop;
        streamConfig->enableAudio = tmpStrmConfig.enableAudio;
    }
    else
    {
        snprintf(streamConfig->videoEncodingSub, MAX_ENCODER_NAME_LEN, "%s", tmpStrmConfig.videoEncoding);
        snprintf(streamConfig->resolutionSub, MAX_RESOLUTION_NAME_LEN, "%s", tmpStrmConfig.resolution);
        streamConfig->framerateSub = tmpStrmConfig.framerate;
        streamConfig->bitrateModeSub = tmpStrmConfig.bitrateMode;
        streamConfig->bitrateValueSub = tmpStrmConfig.bitrateValue;
        streamConfig->qualitySub = tmpStrmConfig.quality;
        streamConfig->gopSub = tmpStrmConfig.gop;
        streamConfig->enableAudioSub = tmpStrmConfig.enableAudioSub;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   changeMatrixCamIpAddrUrl
 * @param   modelNo
 * @param   networkParam
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e changeMatrixCamIpAddrUrl(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return changeCamIpAddrMatrixOemCamera(modelNo, networkParam, urlReqPtr, numOfReq);
    }
    else
    {
        return changeCamIpAddrMatrixIpCameraGeneral(modelNo, networkParam, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   changeCamIpAddrMatrixOemCamera
 * @param   modelNo
 * @param   networkParam
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e changeCamIpAddrMatrixOemCamera(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    CHAR            putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t          bufSize = 0;
    CHAR            putFileName[MAX_FILE_NAME_LEN];
    INT32           putFileFd = INVALID_FILE_FD;
    UINT8           channelId = 1;
    UINT8           reqCnt = 0;
    UINT32          subnetMask;
    CHAR            subnetMaskStr[IPV4_ADDR_LEN_MAX];
    static UINT32   putChngIpFileCnt = 0;

    if (networkParam->ipAddrType != IP_ADDR_TYPE_IPV4)
    {
        return CMD_PROCESS_ERROR;
    }

    if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, XML_VERSION_STR)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if (TRUE == isTiandyOemCamera(modelNo))
    {

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, NET_INF_TAG_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, ID_TAG_STR, channelId)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, LINK_TAG_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_ADDR_TAG_STR())) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else if (modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_ADDR_TAG_STR(STD_CGI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_ADDR_TAG_STR(HIKVISION_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_ADDR_TYPE_STR)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_ADDR_STR, networkParam->ipAddr)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    /* Construct subnet mask in string format */
    subnetMask = htonl((__UINT32_MAX__ << (32 - networkParam->prefixLen)));
    inet_ntop(AF_INET, &subnetMask, subnetMaskStr, sizeof(subnetMaskStr));

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_SUBNET_STR, subnetMaskStr)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == isTiandyOemCamera(modelNo))
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IPV6_ADDR_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_DFLT_GATEWAY_STR, networkParam->gateway)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PRIMARY_DNS_STR)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, SECONDRY_DNS_STR, networkParam->gateway)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == isTiandyOemCamera(modelNo))
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IPV6_MODE_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, IP_ADDR_END_TAG)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, NET_INF_CLOSE_TAG_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    snprintf(putFileName, MAX_FILE_NAME_LEN, PUT_CHANGE_CAM_IP_FILE_NAME, putChngIpFileCnt++);
    putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if (putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if (write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }
    close(putFileFd);

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%d",
                 MatrixOEM.rootFolder,MatrixOEM.isapiFolder,MatrixOEM.systemFolder,MatrixOEM.networkFolder,
                 MatrixOEM.interfaceFolder,channelId);
    }
    else
    {
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.isapiFolder,MatrixOEM.systemFolder,MatrixOEM.networkFolder,
                 MatrixOEM.interfaceFolder,channelId,DIRECTORY_DELIM_CHAR,MatrixOEM.ipAddrStr);
    }
    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    reqCnt++;

    /* /ISAPI/System/reboot */
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s",
             MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder, REBOOT_STR);

    urlReqPtr[reqCnt].requestType = CAM_REQ_REBOOT;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REBOOT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    DPRINT(CAMERA_INTERFACE, "camera ip address change request");
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   changeCamIpAddrMatrixIpCameraGeneral
 * @param   modelNo
 * @param   networkParam
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e changeCamIpAddrMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, IP_ADDR_PARAM_t *networkParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8           reqCnt = 0;
    UINT32          subnetMask;
    CHAR            subnetStr[IPV4_ADDR_LEN_MAX] = { 0 };

    if (networkParam->ipAddrType == IP_ADDR_TYPE_IPV4)
    {
        /* Construct subnet mask in string format */
        subnetMask = htonl((__UINT32_MAX__ << (32 - networkParam->prefixLen)));
        inet_ntop(AF_INET, &subnetMask, subnetStr, sizeof(subnetStr));

        /* Example : http://192.168.101.55:80/matrix-cgi/basic?action=set&ipv4-configuration=0&ipv4-address=192.168.101.45&subnet-mask=255.255.255.0&default-gateway=192.168.101.1 */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                 "%s" "%s%s%c" "%s%c%s" "&ipv4-configuration=0&ipv4-address=%s&subnet-mask=%s&default-gateway=%s",
                 MatrixIP.rootFolder,
                 MatrixIP.cgiFolder, MatrixIP.basicSetting, URL_DELIM,
                 MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET],
                 networkParam->ipAddr, subnetStr, networkParam->gateway);
    }
    else
    {
        /* Example : http://[2001:1234::55]:80/matrix-cgi/basic?action=set&ipv6=1&ipv6-configuration=0&ipv6-address=2001:1234::155&subnet-prefix=64&ipv6-default-gateway=2001:1234::1 */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                 "%s" "%s%s%c" "%s%c%s" "&ipv6=1&ipv6-configuration=0&ipv6-address=%s&subnet-prefix=%d&ipv6-default-gateway=%s",
                 MatrixIP.rootFolder,
                 MatrixIP.cgiFolder, MatrixIP.basicSetting, URL_DELIM,
                 MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET],
                 networkParam->ipAddr, networkParam->prefixLen, networkParam->gateway);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    DPRINT(CAMERA_INTERFACE, "camera ip address change request");
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getDeviceInfoForMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getDeviceInfoForMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8   reqCnt = 0;
    CHARPTR deviceInfoStr = "deviceInfo";

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        //http://<ip>:<port>/CGI/System/deviceInfo
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.systemFolder, deviceInfoStr);
    }
    else
    {
        //http://<ip>:<port>/ISAPI/System/deviceInfo
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder, deviceInfoStr);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getDeviceInfoForMatrixIpCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getDeviceInfoForMatrixIpCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s",
             MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.aboutFolder, URL_DELIM,
             MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET]);

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getDeviceInfoForMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getDeviceInfoForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    /* When we don't have model number known */
    if (modelNo == CAMERA_MODEL_NONE)
    {
        getDeviceInfoForMatrixOemCamera(modelNo, urlReqPtr, &reqCnt);
        *numOfReq = reqCnt;

        reqCnt = 0;
        getDeviceInfoForMatrixIpCamera(modelNo, &urlReqPtr[*numOfReq], &reqCnt);
        *numOfReq += reqCnt;

        reqCnt = 0;
        getDeviceInfoForMatrixOemCamera(MATRIX_MODEL_SATATYA_PZCR20ML_25CWP, &urlReqPtr[*numOfReq], &reqCnt);
        *numOfReq += reqCnt;
    }
    else
    {
        if (TRUE == isMatrixOemCameraModel(modelNo))
        {
            getDeviceInfoForMatrixOemCamera(modelNo, urlReqPtr, &reqCnt);
            *numOfReq = reqCnt;
        }
        else
        {
            getDeviceInfoForMatrixIpCamera(modelNo, &urlReqPtr[*numOfReq], &reqCnt);
            *numOfReq = reqCnt;
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseDeviceInfoMatrixOemCamera
 * @param   modelNo
 * @param   dataSize
 * @param   modelName
 * @param   data
 * @return
 */
static NET_CMD_STATUS_e parseDeviceInfoMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data)
{
    CHARPTR modelStrStart = "<model>";
    CHARPTR modelStrEnd = "</model>";
    CHARPTR deviceNameStart = "<deviceName>";
    CHARPTR deviceNameEnd = "</deviceName>";
    CHARPTR tempDataPtr;
    CHARPTR endDataPtr;

    /* for hikvision OEM camera */
    do
    {
        tempDataPtr = strstr(data, modelStrStart);
        if (tempDataPtr == NULL)
        {
            break;
        }

        tempDataPtr += strlen(modelStrStart);
        endDataPtr = strstr(tempDataPtr, modelStrEnd);
        if (endDataPtr == NULL)
        {
            break;
        }

        strncpy(modelName, tempDataPtr, (endDataPtr - tempDataPtr));
        modelName[endDataPtr - tempDataPtr] = '\0';
        return CMD_SUCCESS;

    }while(0);

    /* for Tiandy camera search for deviceName tag */
    tempDataPtr = strstr(data, deviceNameStart);
    if (tempDataPtr == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    tempDataPtr += strlen(deviceNameStart);
    endDataPtr = strstr(tempDataPtr, deviceNameEnd);
    if (endDataPtr == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    strncpy(modelName, tempDataPtr, (endDataPtr - tempDataPtr));
    modelName[endDataPtr - tempDataPtr] = '\0';
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseDeviceInfoMatrixIpCameraGeneral
 * @param   modelNo
 * @param   dataSize
 * @param   modelName
 * @param   data
 * @return
 */
static NET_CMD_STATUS_e parseDeviceInfoMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data)
{
    CHAR    oneLineData[200];
    CHARPTR mainData = NULL;
    CHARPTR linePtr = NULL;
    CHARPTR parseData = NULL;

    //	response-code=0
    //	product-name=Matrix IP Camera
    //	model=CIDR30FL60CW
    //	serial-no=ad
    //	firmware-version=01.01.01
    //	ipaddress=192.168.102.191
    //	macaddress=00:01:09:1b:55:66

    modelName[0] = '\0';
    mainData = data;
    while((parseData = strstr(mainData, "\n")) != NULL)
    {
        parseData++;
        snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);

        mainData = data + (strlen(data) - strlen(parseData));
        linePtr = oneLineData;

        if((parseData = strstr(linePtr, MatrixIP.modelNameArg)) != NULL)
        {
            strncpy(modelName, linePtr + (strlen(MatrixIP.modelNameArg) + 1), ((strlen(linePtr) - (strlen(MatrixIP.modelNameArg) + 1)) - 1));
            modelName[((strlen(linePtr) - (strlen(MatrixIP.modelNameArg) + 1)) - 1)] = '\0';
        }
    }

    if (modelName[0] == '\0')
    {
        return CMD_PROCESS_ERROR;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseDeviceInfoForMatrixCamera
 * @param   modelNo
 * @param   dataSize
 * @param   modelName
 * @param   data
 * @return
 */
static NET_CMD_STATUS_e parseDeviceInfoForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR modelName, CHARPTR data)
{
    switch(modelNo)
    {
        /* Note: this is not model number. This is request count for camera device info, as at this stage we does not have model of the camera */
        case 0:
        case 2:
        {
            return parseDeviceInfoMatrixOemCamera(modelNo, dataSize, modelName, data);
        }

        case 1:
        {
            return parseDeviceInfoMatrixIpCameraGeneral(modelNo, dataSize, modelName, data);
        }

        default:
        {
            return CMD_PROCESS_ERROR;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setOsdforMatrixCamera
 * @param   modelNo
 * @param   osdParam
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setOsdforMatrixCamera(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setOsdMatrixOemCamera(modelNo, osdParam, urlReqPtr, numOfReq);
    }
    else
    {
        return setOsdMatrixIpCameraGeneral(modelNo, osdParam, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setOsdMatrixOemCamera
 * @param   modelNo
 * @param   osdParam
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setOsdMatrixOemCamera(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    CHAR            putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t          bufSize = 0;
    CHAR            putFileName[MAX_FILE_NAME_LEN];
    INT32           putFileFd = INVALID_FILE_FD;
    UINT8           reqCnt = 0;
    UINT32          posX = 0;
    UINT32          posY = 0;
    static UINT32   putOsdFileCnt = 0;

    // YYYY-MM-DD,MM-DD-YYYY,DD-MM-YYYY,CHR-YYYY/MM/DD,CHR-MM/DD/YYYY,CHR-DD/MM/YYYY
    // DATE_FORMAT_DDMMYYY,
    // DATE_FORMAT_MMDDYYY,
    // DATE_FORMAT_YYYYMMDD,
    // DATE_FORMAT_WWWDDMMYYYY,
    if (TRUE == isTiandyOemCamera(modelNo))
    {
        if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, OSD_HDR_STR_OEM(ISAPI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_SCREEN_SIZE_STR, 1920, 1080)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_ATTRIBUTE_STR(ISAPI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if (osdParam->textoverlayChanged == TRUE)
        {
            if (osdParam->channelNameOverlay == TRUE)
            {
                if ((osdParam->dateTimeOverlay == TRUE) && (osdParam->channelNamePos[0] == osdParam->dateTimePos))
                {
                    if(osdParam->channelNamePos[0] == OSD_POS_TOP_LEFT)
                    {
                        posX = 0;
                        posY = TIANDY_OSD_LINE_OFFSET;
                    }
                    else if(osdParam->channelNamePos[0] == OSD_POS_TOP_RIGHT)
                    {
                        posX = TIANDY_NORMALIZE_WIDTH;
                        posY = TIANDY_OSD_LINE_OFFSET;
                    }
                    else if(osdParam->channelNamePos[0] == OSD_POS_BOTTOM_LEFT)
                    {
                        posX = 0;
                        posY = TIANDY_NORMALIZE_HEIGHT - (TIANDY_OSD_LINE_OFFSET * 2); /* 1 for date-time and 1 for text */
                    }
                    else if(osdParam->channelNamePos[0] == OSD_POS_BOTTOM_RIGHT)
                    {
                        posX = TIANDY_NORMALIZE_WIDTH;
                        posY = TIANDY_NORMALIZE_HEIGHT - (TIANDY_OSD_LINE_OFFSET * 2); /* 1 for date-time and 1 for text */
                    }
                }
                else
                {
                    if(osdParam->channelNamePos[0] == OSD_POS_TOP_LEFT)
                    {
                        posX = 0;
                        posY = 0;
                    }
                    else if(osdParam->channelNamePos[0] == OSD_POS_TOP_RIGHT)
                    {
                        posX = TIANDY_NORMALIZE_WIDTH;
                        posY = 0;
                    }
                    else if(osdParam->channelNamePos[0] == OSD_POS_BOTTOM_LEFT)
                    {
                        posX = 0;
                        posY = TIANDY_NORMALIZE_HEIGHT - TIANDY_OSD_LINE_OFFSET;
                    }
                    else if(osdParam->channelNamePos[0] == OSD_POS_BOTTOM_RIGHT)
                    {
                        posX = TIANDY_NORMALIZE_WIDTH;
                        posY = TIANDY_NORMALIZE_HEIGHT - TIANDY_OSD_LINE_OFFSET;
                    }
                }
            }
            else
            {
                posX = 0;
                posY = 0;
                osdParam->channelName[0][0] = '\0';
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_TEXT_SETTING_STR,
                                     BOOL_STR(osdParam->channelNameOverlay), posX, posY, osdParam->channelName[0])) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_TEXT_END_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if (osdParam->timeOverlayChanged == TRUE)
        {
            if (osdParam->dateTimeOverlay == TRUE)
            {
                if(osdParam->dateTimePos == OSD_POS_TOP_LEFT)
                {
                    posX = 0;
                    posY = 0;
                }
                else if(osdParam->dateTimePos == OSD_POS_TOP_RIGHT)
                {
                    posX = TIANDY_NORMALIZE_WIDTH;
                    posY = 0;
                }
                else if(osdParam->dateTimePos == OSD_POS_BOTTOM_LEFT)
                {
                    posX = 0;
                    posY = TIANDY_NORMALIZE_HEIGHT - TIANDY_OSD_LINE_OFFSET;
                }
                else if(osdParam->dateTimePos == OSD_POS_BOTTOM_RIGHT)
                {
                    posX = TIANDY_NORMALIZE_WIDTH;
                    posY = TIANDY_NORMALIZE_HEIGHT - TIANDY_OSD_LINE_OFFSET;
                }
            }
            else
            {
                posX = 0;
                posY = 0;
            }

            GENERAL_CONFIG_t generalCnfg;
            ReadGeneralConfig(&generalCnfg);

            if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_DATE_TIME_STR(ISAPI_URL_STR), BOOL_STR(osdParam->dateTimeOverlay), posX, posY,
                                    osdMatrixDateTimeFormatOem[generalCnfg.dateFormat][CAMERA_OEM_TIANDY], timeFormatStr[generalCnfg.timeFormat],
                                    BOOL_STR(generalCnfg.dateFormat == DATE_FORMAT_WWWDDMMYYYY))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_CHNL_NAME_STR(ISAPI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        if(modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
        {
            if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, OSD_HDR_STR_OEM(STD_CGI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }
        else
        {
            if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, OSD_HDR_STR_OEM(HIKVISION_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_SCREEN_SIZE_STR, MATRIX_NORMALIZE_WIDTH, MATRIX_NORMALIZE_HEIGHT)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_ATTRIBUTE_STR(" size=\"4\""))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if(osdParam->textoverlayChanged == TRUE)
        {
            if(osdParam->channelNameOverlay == TRUE)
            {
                if(osdParam->channelNamePos[0] == OSD_POS_TOP_LEFT)
                {
                    posX = 0;
                    posY = MATRIX_NORMALIZE_HEIGHT;
                }
                else if(osdParam->channelNamePos[0] == OSD_POS_TOP_RIGHT)
                {
                    posX = MATRIX_NORMALIZE_WIDTH - (15*(strlen(osdParam->channelName[0])+2));
                    posY = MATRIX_NORMALIZE_HEIGHT;
                }
                else if(osdParam->channelNamePos[0] == OSD_POS_BOTTOM_LEFT)
                {
                    if(osdParam->dateTimeOverlay == TRUE && osdParam->dateTimePos == OSD_POS_BOTTOM_LEFT)
                    {
                        posX = 0;
                        posY = 64;
                    }
                    else
                    {
                        posX = 0;
                        posY = OSD_HIEGHT_STEP_SIZE;
                    }
                }
                else if(osdParam->channelNamePos[0] == OSD_POS_BOTTOM_RIGHT)
                {
                    if(osdParam->dateTimeOverlay == TRUE && osdParam->dateTimePos ==OSD_POS_BOTTOM_RIGHT)
                    {
                        posX = MATRIX_NORMALIZE_WIDTH - (15*(strlen(osdParam->channelName[0])+2));
                        posY = 64;
                    }
                    else
                    {
                        posX = MATRIX_NORMALIZE_WIDTH - (15*(strlen(osdParam->channelName[0])+2));
                        posY = OSD_HIEGHT_STEP_SIZE;
                    }
                }
            }

            if(osdParam->channelNameOverlay == FALSE)
            {
                snprintf(osdParam->channelName[0], MAX_CHANNEL_NAME_WIDTH, "matrixCam");
            }

            if ((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_TEXT_SETTING_STR,
                                     BOOL_STR(osdParam->channelNameOverlay), posX, posY, osdParam->channelName[0])) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }

            if(osdParam->channelNameOverlay == TRUE)
            {
                if(osdParam->dateTimeOverlay == TRUE)
                {
                    if(osdParam->channelNamePos[0] == osdParam->dateTimePos)
                    {
                        if (osdParam->channelNamePos[0] == OSD_POS_TOP_LEFT)
                        {
                            posX = 0;
                            posY = MATRIX_NORMALIZE_HEIGHT-OSD_HIEGHT_STEP_SIZE;
                        }
                        else if (osdParam->channelNamePos[0] == OSD_POS_TOP_RIGHT)
                        {
                            posX = (MATRIX_NORMALIZE_WIDTH -OSD_DATE_TIME_LEN);
                            posY = MATRIX_NORMALIZE_HEIGHT - OSD_HIEGHT_STEP_SIZE;
                        }
                        else if (osdParam->channelNamePos[0] == OSD_POS_BOTTOM_LEFT)
                        {
                            posX = 0;
                            posY = OSD_HIEGHT_STEP_SIZE;
                        }
                        else if (osdParam->channelNamePos[0] == OSD_POS_BOTTOM_RIGHT)
                        {
                            posX = (MATRIX_NORMALIZE_WIDTH -OSD_DATE_TIME_LEN);
                            posY =	OSD_HIEGHT_STEP_SIZE;
                        }
                    }
                    else if(osdParam->channelNamePos[0] != osdParam->dateTimePos)
                    {
                        if (osdParam->dateTimePos == OSD_POS_TOP_LEFT)
                        {
                            posX = 0;
                            posY = MATRIX_NORMALIZE_HEIGHT;
                        }
                        else if (osdParam->dateTimePos == OSD_POS_TOP_RIGHT)
                        {
                            posX = (MATRIX_NORMALIZE_WIDTH -OSD_DATE_TIME_LEN);
                            posY = MATRIX_NORMALIZE_HEIGHT;
                        }
                        else if (osdParam->dateTimePos == OSD_POS_BOTTOM_LEFT)
                        {
                            posX = 0;
                            posY = OSD_HIEGHT_STEP_SIZE;
                        }
                        else if (osdParam->dateTimePos == OSD_POS_BOTTOM_RIGHT)
                        {
                            posX = (MATRIX_NORMALIZE_WIDTH - OSD_DATE_TIME_LEN);
                            posY =	OSD_HIEGHT_STEP_SIZE;
                        }
                    }
                }
                else
                {
                    posX = 0;
                    posY = 0;
                }
            }
            else if(osdParam->channelNameOverlay == FALSE)
            {
                if(osdParam->dateTimeOverlay == TRUE)
                {
                    if (osdParam->channelNamePos[0] == OSD_POS_TOP_LEFT)
                    {
                        posX = 0;
                        posY = MATRIX_NORMALIZE_HEIGHT;
                    }
                    else if (osdParam->channelNamePos[0] == OSD_POS_TOP_RIGHT)
                    {
                        posX = (MATRIX_NORMALIZE_WIDTH -OSD_DATE_TIME_LEN);
                        posY = MATRIX_NORMALIZE_HEIGHT;
                    }
                    else if (osdParam->channelNamePos[0] == OSD_POS_BOTTOM_LEFT)
                    {
                        posX = 0;
                        posY = OSD_HIEGHT_STEP_SIZE;
                    }
                    else if (osdParam->channelNamePos[0] == OSD_POS_BOTTOM_RIGHT)
                    {
                        posX = (MATRIX_NORMALIZE_WIDTH -OSD_DATE_TIME_LEN);
                        posY =	OSD_HIEGHT_STEP_SIZE;
                    }
                }
                else if(osdParam->dateTimeOverlay == FALSE)
                {
                    posX = 0;
                    posY = 0;
                }
            }
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_TEXT_END_STR)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if(osdParam->timeOverlayChanged == TRUE)
        {
            if(osdParam->dateTimeOverlay == TRUE && osdParam->channelNameOverlay == FALSE)
            {
                if (osdParam->channelNamePos[0] == OSD_POS_TOP_LEFT)
                {
                    posX = 0;
                    posY = MATRIX_NORMALIZE_HEIGHT;
                }
                else if (osdParam->channelNamePos[0] == OSD_POS_TOP_RIGHT)
                {
                    posX = (MATRIX_NORMALIZE_WIDTH -(OSD_DATE_TIME_LEN));
                    posY = MATRIX_NORMALIZE_HEIGHT;
                }
                else if (osdParam->channelNamePos[0] == OSD_POS_BOTTOM_LEFT)
                {
                    posX = 0;
                    posY = OSD_HIEGHT_STEP_SIZE;
                }
                else if (osdParam->channelNamePos[0] == OSD_POS_BOTTOM_RIGHT)
                {
                    posX = (MATRIX_NORMALIZE_WIDTH -(OSD_DATE_TIME_LEN));
                    posY =	OSD_HIEGHT_STEP_SIZE;
                }
            }
            else
            {
                posX = 0;
                posY = 0;
            }

            GENERAL_CONFIG_t generalCnfg;
            ReadGeneralConfig(&generalCnfg);

            if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_DATE_TIME_STR(""), BOOL_STR(osdParam->dateTimeOverlay), posX, posY,
                                    osdMatrixDateTimeFormatOem[generalCnfg.dateFormat][CAMERA_OEM_HIKVISION], timeFormatStr[generalCnfg.timeFormat],
                                    BOOL_STR(FALSE))) > MAX_PUT_REQ_BUF_SIZE)
            {
                return CMD_PROCESS_ERROR;
            }
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_CHNL_NAME_STR(HIKVISION_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, OSD_COLOR_STR)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    snprintf(putFileName, MAX_FILE_NAME_LEN, PUT_OSD_FILE_NAME, putOsdFileCnt++);
    putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put osd file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if(write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put osd file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }
    close(putFileFd);

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/System/Video/inputs/channels/<ID>/overlays/type/<ID>
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder, MatrixOEM.inputFolder,
                 MatrixOEM.channelsFolder, 1, DIRECTORY_DELIM_CHAR, MatrixOEM.overlayFolder, MatrixOEM.typeFolder, 1);
    }
    else
    {
        // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/overlays/
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder, MatrixOEM.inputFolder,
                 MatrixOEM.channelsFolder, 1, DIRECTORY_DELIM_CHAR, MatrixOEM.overlayFolder);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setOsdMatrixIpCameraGeneral
 * @param   modelNo
 * @param   osdParam
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setOsdMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, OSD_PARAM_t *osdParam, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8               overlayNum;
    UINT8 				reqCnt = 0;
    UINT32 				outLen = 0;
    GENERAL_CONFIG_t	genConfig;

    ReadGeneralConfig(&genConfig);
    outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                      "%s" "%s%s%c" "%s%c%s",
                      MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.overlaySetting, URL_DELIM,
                      MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET]);
    if(outLen > MAX_CAMERA_URI_WIDTH)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
        return CMD_PROCESS_ERROR;
    }

    if(osdParam->dateTimeOverlay == ENABLE)
    {
        outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                           "%c" "%s%c%d" "%c" "%s%c%s" "%c" "%s%c%s" "%c" "%s%c%d", ARG_DELIM,
                           MatrixIP.osdArg, ASSIGN_VAL, osdParam->dateTimeOverlay, ARG_DELIM,
                           MatrixIP.dateFormatArg, ASSIGN_VAL, dateFormatForSetMatrixIP[genConfig.dateFormat], ARG_DELIM,
                           MatrixIP.timeFormatArg, ASSIGN_VAL, timeFormatForSetMatrixIP[genConfig.timeFormat], ARG_DELIM,
                           MatrixIP.dispalyPosArg, ASSIGN_VAL, (osdParam->dateTimePos - 1));
    }
    else
    {
        outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                           "%c" "%s%c%d", ARG_DELIM, MatrixIP.osdArg, ASSIGN_VAL, osdParam->dateTimeOverlay);
    }

    if(outLen > MAX_CAMERA_URI_WIDTH)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
        return CMD_PROCESS_ERROR;
    }

    /* Apply text overlay settings only if text overlay param changed */
    if(osdParam->textoverlayChanged == TRUE)
    {
        /* Enable or disable text overlay in camera */
        outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                           "%c" "%s%c%d", ARG_DELIM, MatrixIP.textOverlayArg, ASSIGN_VAL, osdParam->channelNameOverlay);

        /* Add text and position if overlay is enabled */
        if(osdParam->channelNameOverlay == ENABLE)
        {
            /* New "text-overlay-no-enable" field added for 6 text overlay camera (8MP). Remaining will work with older config fields */
            if (osdParam->channelNameOverlayMax > TEXT_OVERLAY_MIN)
            {
                for (overlayNum = 0; overlayNum < osdParam->channelNameOverlayMax; overlayNum++)
                {
                    outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                                       "%c" "%s%d%c%d" "%c" "%s%d%c%s" "%c" "%s%d%c%d", ARG_DELIM,
                                       "text-overlay-no-enable", overlayNum, ASSIGN_VAL, ACTIVE, ARG_DELIM,
                                       MatrixIP.textArg, overlayNum, ASSIGN_VAL, osdParam->channelName[overlayNum], ARG_DELIM,
                                       MatrixIP.textDisplayPosArg, overlayNum, ASSIGN_VAL, (osdParam->channelNamePos[overlayNum] - 1));
                }
            }
            else
            {
                outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                                   "%c" "%s%c%s" "%c" "%s%c%d", ARG_DELIM,
                                   MatrixIP.textArg, ASSIGN_VAL, osdParam->channelName[0], ARG_DELIM,
                                   MatrixIP.textDisplayPosArg, ASSIGN_VAL, (osdParam->channelNamePos[0] - 1));
            }
        }
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getMotionWindowUrlforMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getMotionWindowUrlforMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return getMotionWindowUrlMatrixOemCamera(modelNo, urlReqPtr, numOfReq);
    }
    else
    {
        return getMotionWindowUrlMatrixIpCameraGeneral( urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief getMotionWindowUrlMatrixOemCamera
 * @param modelNo
 * @param urlReqPtr
 * @param numOfReq
 * @return
 */
static NET_CMD_STATUS_e getMotionWindowUrlMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8   reqCnt = 0;
    CHAR    *pMotionDetectionFolder = "MotionDetection/";
    CHAR    *pLayoutFolder = "layout/";
    CHAR    *pGridLayoutStr = "gridLayout";

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        //http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/motionDetection
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s" "%d%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder, MatrixOEM.inputFolder,
                 MatrixOEM.channelsFolder, 1, DIRECTORY_DELIM_CHAR, "motionDetection/");
    }
    else
    {
        //http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/MotionDetection/layout/gridLayout
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s" "%d%c" "%s%s%s",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder, MatrixOEM.inputFolder,
                 MatrixOEM.channelsFolder, 1, DIRECTORY_DELIM_CHAR, pMotionDetectionFolder, pLayoutFolder, pGridLayoutStr);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_GET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getMotionWindowUrlMatrixIpCameraGeneral
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getMotionWindowUrlMatrixIpCameraGeneral(URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    //http://<ip>:<port>/matrix-cgi/motiondetection?action=get
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s" "%s%s%c" "%s%c%s", MatrixIP.rootFolder,
             MatrixIP.cgiFolder, MatrixIP.motionDetectionFolder, URL_DELIM, MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET]);

    urlReqPtr[reqCnt].requestType = CAM_REQ_GET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseGetMotionWindowForMatrixCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   blockString
 * @return
 */
static NET_CMD_STATUS_e parseGetMotionWindowForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return parseGetMotionWindowMatrixOemCamera(modelNo, dataSize, data, motionBlock);
    }
    else
    {
        if ((modelNo >= MATRIX_MODEL_SATATYA_CIBR30FL_36CG) && (modelNo <= MATRIX_MODEL_SATATYA_CIDR30FL_60CW))
        {
            return parseGetMotionWindowMatrixIpCameraGeneral(modelNo, dataSize, data, motionBlock);
        }
        else
        {
            return parseGetMotionWindowMatrixIpNetraCameraGeneral(modelNo, dataSize, data, motionBlock);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseGetMotionWindowMatrixOemCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   motionBlock
 * @return
 */
static NET_CMD_STATUS_e parseGetMotionWindowMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock)
{
    //	<MotionDetectionGridLayout version="2.0">
    //	<sensitivityLevel>80</sensitivityLevel>
    //	<gridMap>ffff</gridMap>
    //	</MotionDetectionGridLayout>

    CHAR    *pSensitivityStr = "<sensitivityLevel>";
    CHAR    *pGridMapStr = "<gridMap>";
    CHARPTR tempDataPtr;
    CHARPTR strtPtr;
    UINT32  sensitivity = 0;
    CHAR    **matrixWindowBuf;
    CHAR    **convertedMatrixBuf;
    UINT8   cnt = 0;
    UINT16  packedLen = 0;
    CHAR    packedToPass[MOTION_AREA_BLOCK_BYTES_24x18+1];
    UINT32  packByte;

    tempDataPtr = strstr(data, pSensitivityStr);
    if ((tempDataPtr != NULL) && (EOF == sscanf(tempDataPtr, "<sensitivityLevel>%d</sensitivityLevel>", &sensitivity)))
    {
        return CMD_PROCESS_ERROR;
    }

    tempDataPtr = strstr(data, pGridMapStr);
    if(tempDataPtr == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    strtPtr = strchr(tempDataPtr,'>');
    if ((strtPtr == NULL) || ((strtPtr+1) == NULL))
    {
        return CMD_PROCESS_ERROR;
    }

    strtPtr++;
    for(cnt = 0; cnt < MOTION_AREA_BLOCK_BYTES_24x18; cnt++)
    {
        sscanf(strtPtr, "%02x", &packByte);
        packedToPass[cnt] = (UINT8)packByte;
        strtPtr = strtPtr + 2;
    }

    convertedMatrixBuf = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
    if (NULL == convertedMatrixBuf)
    {
        return CMD_PROCESS_ERROR;
    }

    matrixWindowBuf = (CHAR **)Allocate2DArray(18, 24, sizeof(CHAR));
    if (NULL == matrixWindowBuf)
    {
        Free2DArray((void**)convertedMatrixBuf, 36);
        return CMD_PROCESS_ERROR;
    }

    UnPackGridGeneral(packedToPass, 24, 18, matrixWindowBuf, MOTION_AREA_BLOCK_BYTES_24x18);
    ConvertUnpackedGridTo44_36(matrixWindowBuf, convertedMatrixBuf, 22, 18);
    PackGridGeneral(convertedMatrixBuf, 44, 36, motionBlock->blockBitString, &packedLen);
    Free2DArray((void**)matrixWindowBuf, 18);
    Free2DArray((void**)convertedMatrixBuf, 36);

    motionBlock->sensitivity = (UINT8)sensitivity/10;
    motionBlock->noMotionSupportF = FALSE;
    motionBlock->isNoMotionEvent = FALSE;
    motionBlock->noMotionDuration = 5;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseGetMotionWindowMatrixIpCameraGeneral
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   motionBlock
 * @return
 */
static NET_CMD_STATUS_e parseGetMotionWindowMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock)
{
//	response-code=0
//	enable=0
//	re-detection-delay=60
//	window0=0
//	sensitivity0=50
//	threshold0=0
//	x0=0
//	y0=0
//	w0=320
//	h0=180
//	window1=0
//	sensitivity1=50
//	threshold1=0
//	x1=1600
//	y1=0
//	w1=320
//	h1=180
//	window2=0
//	sensitivity2=50
//	threshold2=0
//	x2=1600
//	y2=900
//	w2=320
//	h2=180

    CHAR    oneLineData[200];
    CHARPTR mainData = data;
    CHARPTR linePtr = NULL;
    CHARPTR parseData = NULL;
    INT32   convertedData = 0;
    UINT32  sensitivity = 5;
    CHAR    **matrixWindowBuf;
    CHAR    **convertedMatrixBuf;
    UINT16  packedLen = 0;
    CHAR    packedToPass[MOTION_AREA_BLOCK_BYTES_24x18+1];
    UINT32  packByte;
    UINT8   cnt;

    while((parseData = strstr(mainData, "\n")) != NULL)
    {
        parseData++;
        snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);

        mainData = data + (strlen(data) - strlen(parseData));
        linePtr = oneLineData;

        if((parseData = strstr(linePtr, MatrixIP.motionDetectionEnableArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.motionDetectionEnableArg) + 1);
            convertedData = atoi(linePtr);
            if(convertedData == DISABLE)
            {
				/* fill default value of sensitivity to show in UI */
                motionBlock->sensitivity = sensitivity;
                break;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.motionCellsArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.motionCellsArg) + 1);

            convertedMatrixBuf = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
            if (NULL == convertedMatrixBuf)
            {
                return CMD_PROCESS_ERROR;
            }

            matrixWindowBuf = (CHAR **)Allocate2DArray(9, 11, sizeof(CHAR));
            if (NULL == matrixWindowBuf)
            {
                Free2DArray((void**)convertedMatrixBuf, 36);
                return CMD_PROCESS_ERROR;
            }

            for(cnt = 0; cnt < (MOTION_AREA_BLOCK_BYTES_11x9*2); cnt++)
            {
                sscanf(linePtr, "%02x", &packByte);
                packedToPass[cnt] = (UINT8)packByte;
                linePtr = linePtr + 2;
            }

            UnPackGridGeneral(packedToPass, 11, 9, matrixWindowBuf, MOTION_AREA_BLOCK_BYTES_11x9);
            ConvertUnpackedGridTo44_36(matrixWindowBuf, convertedMatrixBuf, 11, 9);
            PackGridGeneral(convertedMatrixBuf, 44, 36, motionBlock->blockBitString, &packedLen);
            Free2DArray((void**)matrixWindowBuf, 9);
            Free2DArray((void**)convertedMatrixBuf, 36);
        }
        else if((parseData = strstr(linePtr, MatrixIP.sensitivityArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.sensitivityArg) + 1);
            motionBlock->sensitivity = (atoi(linePtr))/10;
        }
    }

	/* default value as no motion event is not supported in these cameras */
    motionBlock->noMotionSupportF = FALSE;
    motionBlock->isNoMotionEvent = FALSE;
    motionBlock->noMotionDuration = 5;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseGetMotionWindowMatrixIpNetraCameraGeneral
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   motionBlock
 * @return
 */
static NET_CMD_STATUS_e parseGetMotionWindowMatrixIpNetraCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, MOTION_BLOCK_METHOD_PARAM_t *motionBlock)
{
    CHAR    oneLineData[410];
    CHARPTR mainData = data;
    CHARPTR linePtr = NULL;
    CHARPTR parseData = NULL;
    UINT32  sensitivity = 5;
    CHAR    **convertedMatrixBuf;
    UINT16  packedLen = 0;
    CHAR    packedToPass[MOTION_AREA_BLOCK_BYTES*2];
    UINT32  packByte;
    UINT8   cnt = 0;

    motionBlock->noMotionSupportF = FALSE;
    motionBlock->isNoMotionEvent = FALSE;
    motionBlock->noMotionDuration = 5;
    motionBlock->sensitivity = sensitivity;
    while((parseData = strstr(mainData, "\n")) != NULL)
    {
        parseData++;
        snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);

        mainData = data + (strlen(data) - strlen(parseData));
        linePtr = oneLineData;

        if((parseData = strstr(linePtr, MatrixIP.motionDetectionEnableArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.motionDetectionEnableArg) + 1);
            if(atoi(linePtr) == DISABLE)
            {
                break;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.motionCellsArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.motionCellsArg) + 1);

            convertedMatrixBuf = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
            if (NULL == convertedMatrixBuf)
            {
                return CMD_PROCESS_ERROR;
            }

            for(cnt = 0; cnt < MOTION_AREA_BLOCK_BYTES; cnt++)
            {
                sscanf(linePtr, "%02x", &packByte);
                packedToPass[cnt] = (UINT8)packByte;
                linePtr = linePtr + 2;
            }

            UnPackGridGeneral(packedToPass, 44, 36, convertedMatrixBuf, MOTION_AREA_BLOCK_BYTES);
            PackGridGeneral(convertedMatrixBuf, 44, 36, motionBlock->blockBitString, &packedLen);
            Free2DArray((void**)convertedMatrixBuf, 36);
        }
        else if((parseData = strstr(linePtr, MatrixIP.sensitivityArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.sensitivityArg) + 1);
            motionBlock->sensitivity = (atoi(linePtr) / 10);
        }
        else if((parseData = strstr(linePtr, MatrixIP.motionEventTypeArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.motionEventTypeArg) + 1);
			/* mark no motion event support flag if we receive 'event-type' tag */
            motionBlock->noMotionSupportF = TRUE;
            motionBlock->isNoMotionEvent = GET_BOOL_VALUE((UINT8)atoi(linePtr));
        }
        else if((parseData = strstr(linePtr, MatrixIP.noMotionEventDurationArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.noMotionEventDurationArg) + 1);
            motionBlock->noMotionDuration = atoi(linePtr);
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setMotionWindowforMatrixCamera
 * @param   modelNo
 * @param   gridDataBuf
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setMotionWindowforMatrixCamera(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setMotionWindowMatrixOemCamera(modelNo, gridDataBuf, urlReqPtr, numOfReq);
    }
    else
    {
        if ((modelNo >= MATRIX_MODEL_SATATYA_CIBR30FL_36CG) && (modelNo <= MATRIX_MODEL_SATATYA_CIDR30FL_60CW))
        {
            return setMotionWindowMatrixIpCameraGeneral(modelNo, gridDataBuf, urlReqPtr, numOfReq);
        }
        else
        {
            return setMotionWindowMatrixIpNetraCameraGeneral(modelNo, gridDataBuf, urlReqPtr, numOfReq);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setMotionWindowMatrixOemCamera
 * @param   modelNo
 * @param   motionArea
 * @param   gridDataBuf
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setMotionWindowMatrixOemCamera(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf,
                                                       URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    CHAR            putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t          bufSize = 0;
    CHAR            putFileName[MAX_FILE_NAME_LEN];
    INT32           putFileFd = INVALID_FILE_FD;
    UINT8           reqCnt = 0;
    UINT8           sensitivity = 0;
    UINT8           tempCnt = 0;
    CHAR            **grid44_36Ptr;
    CHAR            **gridRandomPtr;
    CHAR            packedGridBytes[MOTION_AREA_BLOCK_BYTES_24x18+1];
    CHAR            packedString[(MOTION_AREA_BLOCK_BYTES_24x18*2)+1];
    UINT16          packedLen = 0;
    static UINT32   putSetMotionWinCnt = 0;

    grid44_36Ptr = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
    if (NULL == grid44_36Ptr)
    {
        return CMD_PROCESS_ERROR;
    }

    gridRandomPtr = (CHAR **)Allocate2DArray(18, 24, sizeof(CHAR));
    if (NULL == gridRandomPtr)
    {
        Free2DArray((void**)grid44_36Ptr, 36);
        return CMD_PROCESS_ERROR;
    }

    UnPackGridGeneral(gridDataBuf->blockBitString, 44, 36, grid44_36Ptr, MOTION_AREA_BLOCK_BYTES);
    Convert44_36ToUnpackedGrid(grid44_36Ptr, gridRandomPtr, 22, 18);
    PackGridGeneral(gridRandomPtr, 24, 18, packedGridBytes, &packedLen);
    Free2DArray((void**)grid44_36Ptr, 36);
    Free2DArray((void**)gridRandomPtr, 18);

    for(tempCnt = 0; tempCnt < MOTION_AREA_BLOCK_BYTES_24x18; tempCnt++)
    {
        snprintf(&packedString[tempCnt*2], sizeof(packedString), "%02x", (UINT8)(packedGridBytes[tempCnt]));
    }
    sensitivity = gridDataBuf->sensitivity;

    if (modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
    {
        if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, MOTION_GRID_STR_OEM(STD_CGI_URL_STR), BOOL_STR(TRUE))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, MOTION_LAYOUTSTR_OEM(STD_CGI_URL_STR),
                                sensitivityTableOem[sensitivity][getOemCameraBrand(modelNo)], packedString)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else if (TRUE == isTiandyOemCamera(modelNo))
    {
        if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, MOTION_GRID_STR_OEM(ISAPI_URL_STR), BOOL_STR(TRUE))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, MOTION_LAYOUTSTR_OEM(ISAPI_URL_STR),
                                sensitivityTableOem[sensitivity][getOemCameraBrand(modelNo)], packedString)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, MOTION_GRID_STR_OEM(HIKVISION_URL_STR), BOOL_STR(TRUE))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, MOTION_LAYOUTSTR_OEM(HIKVISION_URL_STR),
                                sensitivityTableOem[sensitivity][getOemCameraBrand(modelNo)], packedString)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    snprintf(putFileName, MAX_FILE_NAME_LEN, PUT_SET_MOTION_WIN_FILE_NAME, putSetMotionWinCnt++);
    putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if(write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }

    close(putFileFd);

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/System/Video/inputs/channels/<ID>/motionDetection
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.cgiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder,MatrixOEM.channelsFolder,1,DIRECTORY_DELIM_CHAR, "motionDetection");
    }
    else
    {
        // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/MotionDetection
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.isapiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder,MatrixOEM.channelsFolder,1,DIRECTORY_DELIM_CHAR, "MotionDetection");
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_SET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setMotionWindowMatrixIpCameraGeneral
 * @param   modelNo
 * @param   gridDataBuf
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setMotionWindowMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8   reqCnt = 0;
    UINT8   tempCnt = 0;
    CHAR    **grid44_36Ptr;
    CHAR    **gridRandomPtr;
    CHAR    packedGridBytes[MOTION_AREA_BLOCK_BYTES_24x18+1];
    CHAR    packedString[(MOTION_AREA_BLOCK_BYTES_11x9*2)+1];
    UINT8   sensitivity = 0;
    UINT16  packedLen = 0;

    grid44_36Ptr = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
    if (NULL == grid44_36Ptr)
    {
        return CMD_PROCESS_ERROR;
    }

    gridRandomPtr = (CHAR **)Allocate2DArray(9, 11, sizeof(CHAR));
    if (NULL == gridRandomPtr)
    {
        Free2DArray((void**)grid44_36Ptr, 36);
        return CMD_PROCESS_ERROR;
    }

    UnPackGridGeneral(gridDataBuf->blockBitString, 44, 36, grid44_36Ptr, MOTION_AREA_BLOCK_BYTES);
    Convert44_36ToUnpackedGrid(grid44_36Ptr, gridRandomPtr, 11, 9);
    PackGridGeneral(gridRandomPtr, 11, 9, packedGridBytes, &packedLen);
    Free2DArray((void**)grid44_36Ptr, 36);
    Free2DArray((void**)gridRandomPtr, 9);

    for(tempCnt = 0; tempCnt < MOTION_AREA_BLOCK_BYTES_11x9; tempCnt++)
    {
        snprintf(&packedString[tempCnt*2], sizeof(packedString), "%02x", (UINT8)(packedGridBytes[tempCnt]));
    }
    sensitivity = (gridDataBuf->sensitivity * 10);

    //http://<ip>:<port>/matrix-cgi/motiondetection?action=set
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%s",
             MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.motionDetectionFolder, URL_DELIM,
             MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
             MatrixIP.motionDetectionEnableArg, ASSIGN_VAL, ENABLE, ARG_DELIM,
             MatrixIP.sensitivityArg, ASSIGN_VAL, sensitivity, ARG_DELIM,
             MatrixIP.motionCellsArg, ASSIGN_VAL, packedString);

    urlReqPtr[reqCnt].requestType = CAM_REQ_SET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set motion window for matrix IP camera of netra series
 * @param   modelNo
 * @param   gridDataBuf
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setMotionWindowMatrixIpNetraCameraGeneral(CAMERA_MODEL_e modelNo, MOTION_BLOCK_METHOD_PARAM_t *gridDataBuf, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8   reqCnt = 0;
    UINT8   tempCnt = 0;
    CHAR    **grid44_36Ptr;
    CHAR    packedGridBytes[MOTION_AREA_BLOCK_BYTES];
    CHAR    packedString[(MOTION_AREA_BLOCK_BYTES*2)+1];
    UINT16  packedLen = 0;

    grid44_36Ptr = (CHAR **)Allocate2DArray(36, 44, sizeof(CHAR));
    if (NULL != grid44_36Ptr)
    {
        UnPackGridGeneral(gridDataBuf->blockBitString, 44, 36, grid44_36Ptr, MOTION_AREA_BLOCK_BYTES);
        PackGridGeneral(grid44_36Ptr, 44, 36, packedGridBytes, &packedLen);
        Free2DArray((void**)grid44_36Ptr, 36);

        for(tempCnt = 0; tempCnt < MOTION_AREA_BLOCK_BYTES; tempCnt++)
        {
            snprintf(&packedString[tempCnt*2], sizeof(packedString), "%02x", (UINT8)(packedGridBytes[tempCnt]));
        }
    }

    if ((modelNo < MAX_MATRIX_CAMERA_MODEL) && (TRUE == HAVE_CAMERA_CAPABILITY(modelNo, NO_MOTION_DETECTION_SUPPORT)))
    {
        //http://<ip>:<port>/matrix-cgi/motiondetection?action=set
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                 "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%s",
                 MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.motionDetectionFolder, URL_DELIM,
                 MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                 MatrixIP.motionDetectionEnableArg, ASSIGN_VAL, ENABLE, ARG_DELIM,
                 MatrixIP.sensitivityArg, ASSIGN_VAL, (gridDataBuf->sensitivity * 10), ARG_DELIM,
                 MatrixIP.motionEventTypeArg, ASSIGN_VAL, gridDataBuf->isNoMotionEvent, ARG_DELIM,
                 MatrixIP.noMotionEventDurationArg, ASSIGN_VAL, gridDataBuf->noMotionDuration, ARG_DELIM,
                 MatrixIP.motionCellsArg, ASSIGN_VAL, packedString);
    }
    else
    {
        //http://<ip>:<port>/matrix-cgi/motiondetection?action=set
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                 "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d" "%c" "%s%c%d" "%c" "%s%c%s",
                 MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.motionDetectionFolder, URL_DELIM,
                 MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                 MatrixIP.motionDetectionEnableArg, ASSIGN_VAL, ENABLE, ARG_DELIM,
                 MatrixIP.sensitivityArg, ASSIGN_VAL, (gridDataBuf->sensitivity * 10), ARG_DELIM,
                 MatrixIP.motionCellsArg, ASSIGN_VAL, packedString);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_SET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getPrivacyMaskForMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getPrivacyMaskForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return getPrivacyMaskMatrixOemCamera(modelNo, urlReqPtr, numOfReq);
    }
    else
    {
        return getPrivacyMaskMatrixIpCameraGeneral(modelNo, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getPrivacyMaskMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getPrivacyMaskMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        //http://<ip>:<port>/CGI/System/Video/inputs/channels/<ID>/PrivacyMask
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s" "%s%d%c" "%s",
                 MatrixOEM.rootFolder,MatrixOEM.cgiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,MatrixOEM.inputFolder,
                 MatrixOEM.channelsFolder,1, DIRECTORY_DELIM_CHAR, "PrivacyMask");
    }
    else
    {
        //http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/privacyMask
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s" "%s%d%c" "%s",
                 MatrixOEM.rootFolder,MatrixOEM.isapiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,MatrixOEM.inputFolder,
                 MatrixOEM.channelsFolder,1, DIRECTORY_DELIM_CHAR, "privacyMask");
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_GET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getPrivacyMaskMatrixIpCameraGeneral
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getPrivacyMaskMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8   reqCnt = 0;
    UINT32  outLen;
    UINT8   cnt = 0;
    UINT8   maxPrivacyMask = 0;

    // http://<ip>:<port>/matrix-cgi/overlay?action=get&privacy=&privacy-mask0=&
    // x0=&y0=&w0=&h0=&privacy-mask1=&x1=&y1=&w1=&h1=&privacy-mask2=&x2=&y2=&w2=&h2=
    outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
                      "%s" "%s%s%c" "%s%c%s" "%c" "%s%c" "%c",
                      MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.overlaySetting, URL_DELIM,
                      MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET], ARG_DELIM,
                      MatrixIP.privacyMaskArg, ASSIGN_VAL, ARG_DELIM);
    if(outLen > MAX_CAMERA_URI_WIDTH)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
        return CMD_PROCESS_ERROR;
    }

    /* get supported privacy mask for given matrix camera model */
    maxPrivacyMask = GetMaxSupportedPrivacyMaskWindow(CAMERA_BRAND_MATRIX, modelNo);

    for(cnt = 0; cnt < maxPrivacyMask; cnt++)
    {
        outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                           "%s%d%c" "%c" "%s%d%c" "%c" "%s%d%c" "%c" "%s%d%c" "%c" "%s%d%c" "%c",
                           MatrixIP.privacyMaskNoArg, cnt, ASSIGN_VAL, ARG_DELIM,
                           MatrixIP.startXArg, cnt, ASSIGN_VAL, ARG_DELIM,
                           MatrixIP.startYArg, cnt, ASSIGN_VAL, ARG_DELIM,
                           MatrixIP.widthArg, cnt, ASSIGN_VAL, ARG_DELIM,
                           MatrixIP.heightArg, cnt, ASSIGN_VAL, ARG_DELIM);
        if(outLen > MAX_CAMERA_URI_WIDTH)
        {
            EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
            return CMD_PROCESS_ERROR;
        }
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_GET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parsePrivacyMaskResponseForMatrixCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   privacyMaskArea
 * @param   privacyMaskName
 * @return
 */
static NET_CMD_STATUS_e parsePrivacyMaskResponseForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                                PRIVACY_MASK_CONFIG_t *privacyMaskArea, CHARPTR *privacyMaskName)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return parsePrivacyMaskResponseMatrixOemCamera(modelNo, dataSize, data, privacyMaskArea);
    }
    else
    {
        return parsePrivacyMaskResponseMatrixIpCameraGeneral(modelNo, dataSize, data, privacyMaskArea);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parsePrivacyMaskResponseMatrixOemCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   privacyMaskArea
 * @return
 */
static NET_CMD_STATUS_e parsePrivacyMaskResponseMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, PRIVACY_MASK_CONFIG_t *privacyMaskArea)
{
    CHAR                    tempBuf[100] = "";
    UINT16                  tempValueArray[8] = {0,0,0,0,0,0,0,0};
    BOOL                    getPointF = TRUE;
    UINT8                   tempSize = 0;
    UINT8                   regionCnt = 0;
    UINT8                   tempCnt = 0;
    UINT32                  winWidth;
    UINT32                  winHeight;
    UINT32                  tempYpoint;
    UINT64                  tagValue;
    CHARPTR                 tempDataPtr = NULL;
    CHARPTR                 tempPtr = NULL;
    CHARPTR                 tempDataStrPtr = NULL;
    PRIVACY_MASK_CONFIG_t   tempPrivacyConfig[MAX_WINDOW] = {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}};

    tempDataPtr = strstr(data,PRIVACY_WINDOW_WIDTH_STR);
    if(tempDataPtr != NULL)
    {
        sscanf(tempDataPtr, PRIVACY_WINDOW_WIDTH_TAG, &winWidth);
    }
    else
    {
        winWidth = 704;
    }

    tempDataPtr = strstr(data, PRIVACY_WINDOW_HEIGHT_STR);
    if(tempDataPtr != NULL)
    {
        sscanf(tempDataPtr, PRIVACY_WINDOW_HEIGHT_TAG, &winHeight);
    }
    else
    {
        winHeight = 576;
    }

    tempDataPtr = strstr(data,PRIVACY_ENABLE_TAG);
    if(tempDataPtr == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    tempPtr = strstr(tempDataPtr,END_TAG_DELIM);
    if(tempPtr == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    tempSize = tempPtr - tempDataPtr;
    if(tempSize > 0)
    {
        memcpy(tempBuf,tempDataPtr,tempSize);
        tempBuf[tempSize] = '\0';
    }

    tempPtr = strchr(tempBuf, '>');

    if (NULL == tempPtr)
    {
        return CMD_PROCESS_ERROR;
    }

    tempPtr++;
    if(strcmp(tempPtr, BOOL_STR(TRUE)) != 0)
    {
        return CMD_SUCCESS;
    }

    while(regionCnt < MAX_WINDOW)
    {
        snprintf(tempBuf, sizeof(tempBuf), ID_TAG_STR, regionCnt+1);
        tempDataPtr= strstr(data,tempBuf);
        if (tempDataPtr == NULL)
        {
            regionCnt++;
            continue;
        }

        tempDataStrPtr = strstr(tempDataPtr,PRIVACY_ENABLE_TAG);
        if(tempDataStrPtr == NULL)
        {
            regionCnt++;
            continue;
        }

        tempDataPtr = strstr(tempDataStrPtr,END_TAG_DELIM);
        if(tempDataPtr == NULL)
        {
            regionCnt++;
            continue;
        }

        tempSize =  tempDataPtr - tempDataStrPtr;
        if(tempSize > 0)
        {
            memcpy(tempBuf,tempDataStrPtr,tempSize);
            tempBuf[tempSize] = '\0';
        }

        tempPtr = strchr(tempBuf,'>');

        if (NULL == tempPtr)
        {
            return CMD_PROCESS_ERROR;
        }

        tempPtr++;
        if(strcmp(tempPtr, BOOL_STR(TRUE)) != 0)
        {
            regionCnt++;
            continue;
        }

        tempPtr = strstr(tempDataPtr,REGION_LIST_TAG);
        if (tempPtr == NULL)
        {
            regionCnt++;
            continue;
        }

        for(tempCnt = 0;tempCnt < 4;tempCnt++)
        {
            tempDataPtr = strstr(tempPtr,POSITIONX_TAG);
            if(tempDataPtr == NULL)
            {
                getPointF = FALSE;
                break;
            }

            tempPtr = strstr(tempDataPtr,END_TAG_DELIM);
            if(tempPtr == NULL)
            {
                getPointF = FALSE;
                break;
            }

            tempSize = tempPtr - tempDataPtr;
            if (tempSize == 0)
            {
                continue;
            }

            memcpy(tempBuf,tempDataPtr,tempSize);
            tempBuf[tempSize] = '\0';
            tempDataStrPtr = strchr(tempBuf,'>');
            tempDataStrPtr++;
            AsciiToInt(tempDataStrPtr, &tagValue);
            tempValueArray[2*tempCnt] = (UINT16)tagValue;

            tempDataPtr = strstr(tempPtr,POSITIONY_TAG);
            if (tempDataPtr == NULL)
            {
                getPointF = FALSE;
                break;
            }

            tempPtr = strstr(tempDataPtr,END_TAG_DELIM);
            if(tempPtr == NULL)
            {
                getPointF = FALSE;
                break;
            }

            tempSize = tempPtr - tempDataPtr;
            if(tempSize == 0)
            {
                continue;
            }

            memcpy(tempBuf,tempDataPtr,tempSize);
            tempBuf[tempSize] = '\0';
            tempDataStrPtr = strchr(tempBuf,'>');
            tempDataStrPtr++;
            AsciiToInt(tempDataStrPtr, &tagValue);
            tempValueArray[(2*tempCnt)+1] = (UINT16)tagValue;
        }

        if (FALSE == getPointF)
        {
            regionCnt++;
            continue;
        }

        while(tempCnt < 8)
        {
            tempCnt++;
        }

        if(modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
        {
            tempPrivacyConfig[regionCnt].startXPoint = tempValueArray[4]; // x1
            tempPrivacyConfig[regionCnt].startYPoint= tempValueArray[5]; //y1
            tempPrivacyConfig[regionCnt].width = tempValueArray[0];	//x2
            tempPrivacyConfig[regionCnt].height = tempValueArray[1]; //y2
            tempYpoint = (MATRIX_NORMALIZE_HEIGHT - ((tempValueArray[3]* MATRIX_NORMALIZE_HEIGHT)/255));
        }
        else
        {
            tempPrivacyConfig[regionCnt].startXPoint = tempValueArray[0]; // x1
            tempPrivacyConfig[regionCnt].startYPoint= tempValueArray[1]; //y1
            tempPrivacyConfig[regionCnt].width = tempValueArray[4];	//x2
            tempPrivacyConfig[regionCnt].height = tempValueArray[5]; //y2
            tempYpoint = MATRIX_NORMALIZE_HEIGHT - tempValueArray[7];
        }

        privacyMaskArea[regionCnt].startXPoint = (((MATRIX_NORMALIZE_WIDTH) * (tempPrivacyConfig[regionCnt].startXPoint)) / winWidth);
        privacyMaskArea[regionCnt].startYPoint = (((MATRIX_NORMALIZE_HEIGHT) * (tempPrivacyConfig[regionCnt].startYPoint)) / winHeight);
        privacyMaskArea[regionCnt].width = ((((MATRIX_NORMALIZE_WIDTH) * (tempPrivacyConfig[regionCnt].width)) / winWidth) - (privacyMaskArea[regionCnt].startXPoint));
        privacyMaskArea[regionCnt].height = ((((MATRIX_NORMALIZE_HEIGHT) * (tempPrivacyConfig[regionCnt].height)) / winHeight) - (privacyMaskArea[regionCnt].startYPoint));
        privacyMaskArea[regionCnt].startYPoint = tempYpoint;
        regionCnt++;
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parsePrivacyMaskResponseMatrixIpCameraGeneral
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   privacyMaskArea
 * @return
 */
static NET_CMD_STATUS_e parsePrivacyMaskResponseMatrixIpCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data, PRIVACY_MASK_CONFIG_t *privacyMaskArea)
{
    CHAR    oneLineData[512];
    CHARPTR mainData = data;
    CHARPTR linePtr = NULL;
    CHARPTR parseData = NULL;
    INT32   convertedData = 0;
    INT32   maskNo = 0, maskStatus = 0;

//	"privacy=1\n"
//	"privacy-mask0=1\n"
//	"x0=288\n"
//	"y0=288\n"
//	"w0=320\n"
//	"h0=240\n"
//	"privacy-mask1=1\n"
//	"x1=808\n"
//	"y1=320\n"
//	"w1=320\n"
//	"h1=240\n"
//	"privacy-mask2=1\n"
//	"x2=1012\n"
//	"y2=840\n"
//	"w2=908\n"
//	"h2=240\n"

    while((parseData = strstr(mainData, "\n")) != NULL)
    {
        parseData++;
        snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);

        mainData = data + (strlen(data) - strlen(parseData));
        linePtr = oneLineData;

        if((parseData = strstr(linePtr, MatrixIP.privacyMaskNoArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.privacyMaskNoArg));
            maskNo = ((*linePtr) - 0x30);
            linePtr +=2;

            /* Parse all masks but discards mask which is beyond the buffer size */
            maskStatus = (maskNo < MAX_PRIVACY_MASKS) ? atoi(linePtr) : DISABLE;
        }
        else if((parseData = strstr(linePtr, MatrixIP.privacyMaskArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.privacyMaskArg) + 1);
            convertedData = atoi(linePtr);
            if(convertedData == DISABLE)
            {
                break;
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.startXArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.startXArg));
            convertedData = ((*linePtr) - 0x30);
            linePtr +=2;
            privacyMaskArea[maskNo].startXPoint = 0;
            if((maskNo == convertedData) && (maskStatus == ENABLE))
            {
                privacyMaskArea[maskNo].startXPoint = atoi(linePtr);
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.startYArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.startYArg));
            convertedData = ((*linePtr) - 0x30);
            linePtr +=2;
            privacyMaskArea[maskNo].startYPoint = 0;
            if((maskNo == convertedData) && (maskStatus == ENABLE))
            {
                privacyMaskArea[maskNo].startYPoint = atoi(linePtr);
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.widthArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.widthArg));
            convertedData = ((*linePtr) - 0x30);
            linePtr +=2;
            privacyMaskArea[maskNo].width = 0;
            if((maskNo == convertedData) && (maskStatus == ENABLE))
            {
                privacyMaskArea[maskNo].width = atoi(linePtr);
            }
        }
        else if((parseData = strstr(linePtr, MatrixIP.heightArg)) != NULL)
        {
            linePtr += (strlen(MatrixIP.heightArg));
            convertedData = ((*linePtr) - 0x30);
            linePtr +=2;
            privacyMaskArea[maskNo].height = 0;
            if((maskNo == convertedData) && (maskStatus == ENABLE))
            {
                privacyMaskArea[maskNo].height = atoi(linePtr);
            }
        }
    }

    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPrivacyMaskForMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   privacyMaskArea
 * @param   privacyMaskStatus
 * @param   privacyMaskName
 * @return
 */
static NET_CMD_STATUS_e setPrivacyMaskForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                      PRIVACY_MASK_CONFIG_t *privacyMaskArea, BOOL privacyMaskStatus, CHARPTR *privacyMaskName)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setPrivacyMaskMatrixOemCamera(modelNo, urlReqPtr,numOfReq, privacyMaskArea, privacyMaskStatus);
    }
    else
    {
        return setPrivacyMaskMatrixIpCamera(modelNo, urlReqPtr,numOfReq, privacyMaskArea, privacyMaskStatus);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPrivacyMaskMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   privacyMaskArea
 * @param   privacyMaskStatus
 * @return
 */
static NET_CMD_STATUS_e setPrivacyMaskMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                      PRIVACY_MASK_CONFIG_t *privacyMaskArea, BOOL privacyMaskStatus)
{
    UINT8           regionCnt = 0;
    CHAR            putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t          bufSize = 0;
    CHAR            putFileName[MAX_FILE_NAME_LEN];
    INT32           putFileFd = INVALID_FILE_FD;
    UINT8           regionId = 1;
    UINT8           reqCnt = 0;
    UINT8           channelNo = 1;
    UINT32          normalizedWidth;
    UINT32          normalizedHeight;
    static UINT32   putSetPrivacyMaskCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/System/Video/inputs/channels/<ID>/PrivacyMask
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.cgiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder,MatrixOEM.channelsFolder,channelNo,DIRECTORY_DELIM_CHAR,"PrivacyMask");
    }
    else
    {
        // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/privacyMask/regions
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.isapiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder,MatrixOEM.channelsFolder,channelNo,DIRECTORY_DELIM_CHAR,"privacyMask",DIRECTORY_DELIM_CHAR,REGIONS_STR);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_SET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = DELETE_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    if (privacyMaskStatus == DISABLE)
    {
        *numOfReq = reqCnt;
        return CMD_SUCCESS;
    }

    if (modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
    {
        if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, PRIVACY_HEADER_STR_OEM(STD_CGI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        normalizedWidth = MATRIX_NORMALIZE_WIDTH;
        normalizedHeight = MATRIX_NORMALIZE_HEIGHT;
    }
    else
    {
        if((bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, PRIVACY_HEADER_STR_OEM(ISAPI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            normalizedWidth = 255;
            normalizedHeight = 255;
        }
        else
        {
            normalizedWidth = MATRIX_NORMALIZE_WIDTH;
            normalizedHeight = MATRIX_NORMALIZE_HEIGHT;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, ENABLE_TRUE_TAG_STR)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == isTiandyOemCamera(modelNo))
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, NORMALIZED_SIZE_STR, normalizedWidth, normalizedHeight)) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PRIVACY_REGION_LIST_STR)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if ((privacyMaskArea[0].height != 0 && privacyMaskArea[0].width != 0) || (privacyMaskArea[1].height != 0 && privacyMaskArea[1].width != 0) ||
            (privacyMaskArea[2].height != 0 && privacyMaskArea[2].width != 0) || (privacyMaskArea[3].height != 0 && privacyMaskArea[3].width != 0))
    {
        while(regionCnt < MAX_WINDOW)
        {
            if ((privacyMaskArea[regionCnt].height == 0) || (privacyMaskArea[regionCnt].width == 0))
            {
                regionCnt++;
                continue;
            }

            if ((TRUE == isTiandyOemCamera(modelNo)) || (modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW))
            {
                if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize,
                                        (TRUE == isTiandyOemCamera(modelNo)) ? PRIVACY_REGION_STR : PRIVACY_REGION_STR_OEM_PTZ, regionId,
                                        (((privacyMaskArea[regionCnt].startXPoint)* normalizedWidth)/MATRIX_NORMALIZE_WIDTH),
                                        (normalizedHeight - (((privacyMaskArea[regionCnt].startYPoint)* normalizedHeight)/MATRIX_NORMALIZE_HEIGHT)),
                                        (((privacyMaskArea[regionCnt].startXPoint + privacyMaskArea[regionCnt].width)* normalizedWidth)/MATRIX_NORMALIZE_WIDTH),
                                        (normalizedHeight - (((privacyMaskArea[regionCnt].startYPoint)* normalizedHeight)/MATRIX_NORMALIZE_HEIGHT)),
                                        (((privacyMaskArea[regionCnt].startXPoint + privacyMaskArea[regionCnt].width)* normalizedWidth)/MATRIX_NORMALIZE_WIDTH),
                                        (normalizedHeight - (((privacyMaskArea[regionCnt].startYPoint + privacyMaskArea[regionCnt].height)* normalizedHeight)/MATRIX_NORMALIZE_HEIGHT)),
                                        (((privacyMaskArea[regionCnt].startXPoint)* normalizedWidth)/MATRIX_NORMALIZE_WIDTH),
                                        (normalizedHeight - (((privacyMaskArea[regionCnt].startYPoint+ privacyMaskArea[regionCnt].height)* normalizedHeight)/MATRIX_NORMALIZE_HEIGHT)),
                                        regionId)) > MAX_PUT_REQ_BUF_SIZE)
                {
                    return CMD_PROCESS_ERROR;
                }
            }
            else
            {
                if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PRIVACY_REGION_STR, regionId, privacyMaskArea[regionCnt].startXPoint,
                                        normalizedHeight - (privacyMaskArea[regionCnt].startYPoint),
                                        privacyMaskArea[regionCnt].startXPoint + privacyMaskArea[regionCnt].width,
                                        normalizedHeight - (privacyMaskArea[regionCnt].startYPoint),
                                        privacyMaskArea[regionCnt].startXPoint + privacyMaskArea[regionCnt].width,
                                        normalizedHeight - (privacyMaskArea[regionCnt].startYPoint + privacyMaskArea[regionCnt].height),
                                        privacyMaskArea[regionCnt].startXPoint,
                                        normalizedHeight - (privacyMaskArea[regionCnt].startYPoint+ privacyMaskArea[regionCnt].height ))) > MAX_PUT_REQ_BUF_SIZE)
                {
                    return CMD_PROCESS_ERROR;
                }
            }

            regionId++;
            regionCnt++;
        }
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PRIVACY_REGION_LIST_END_TAG)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, PRIVACY_END_TAG)) > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    snprintf(putFileName, MAX_FILE_NAME_LEN, PUT_SET_PRIVACYMASK_FILE_NAME, putSetPrivacyMaskCnt++);
    putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if(write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }

    close(putFileFd);

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/System/Video/inputs/channels/<ID>/PrivacyMask
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.cgiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder,MatrixOEM.channelsFolder,channelNo,DIRECTORY_DELIM_CHAR,"PrivacyMask");
    }
    else
    {
        // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/1/privacyMask
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s%s%d%c%s",
                 MatrixOEM.rootFolder,MatrixOEM.isapiFolder,MatrixOEM.systemFolder,MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder,MatrixOEM.channelsFolder,channelNo,DIRECTORY_DELIM_CHAR,"privacyMask");
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_SET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPrivacyMaskMatrixIpCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   privacyMaskArea
 * @param   privacyMaskStatus
 * @return
 */
static NET_CMD_STATUS_e setPrivacyMaskMatrixIpCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                     PRIVACY_MASK_CONFIG_t *privacyMaskArea, BOOL privacyMaskStatus)
{
    NET_CMD_STATUS_e 	requestStatus = CMD_SUCCESS;
    UINT8   			reqCnt = 0;
    UINT8				cnt = 0;
    UINT32				outLen = 0;
    UINT8               maxPrivacyMask = 0;

    //http://<ip>:<port>/matrix-cgi/overlay?action=set
    outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s" "%c" "%s%c%d",
                      MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.overlaySetting, URL_DELIM,
                      MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM,
                      MatrixIP.privacyMaskArg, ASSIGN_VAL, privacyMaskStatus);
    if (outLen > MAX_CAMERA_URI_WIDTH)
    {
        EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
        return CMD_PROCESS_ERROR;
    }

    /* get supported privacy mask for given matrix camera model */
    maxPrivacyMask = GetMaxSupportedPrivacyMaskWindow(CAMERA_BRAND_MATRIX, modelNo);

    if(privacyMaskStatus == ENABLE)
    {
        for(cnt = 0; cnt < maxPrivacyMask; cnt++)
        {
            if ((privacyMaskArea[cnt].startXPoint != 0) || (privacyMaskArea[cnt].startYPoint != 0) ||
                    (privacyMaskArea[cnt].width != 0) || (privacyMaskArea[cnt].height != 0))
            {
                outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                                   "%c" "%s%d%c%d" "%c" "%s%d%c%d" "%c" "%s%d%c%d" "%c" "%s%d%c%d" "%c" "%s%d%c%d",
                                   ARG_DELIM, MatrixIP.privacyMaskNoArg, cnt, ASSIGN_VAL, ENABLE,
                                   ARG_DELIM, MatrixIP.startXArg, cnt, ASSIGN_VAL, privacyMaskArea[cnt].startXPoint,
                                   ARG_DELIM, MatrixIP.startYArg, cnt, ASSIGN_VAL, privacyMaskArea[cnt].startYPoint,
                                   ARG_DELIM, MatrixIP.widthArg, cnt, ASSIGN_VAL, privacyMaskArea[cnt].width,
                                   ARG_DELIM, MatrixIP.heightArg, cnt, ASSIGN_VAL, privacyMaskArea[cnt].height);
            }
            else
            {
                outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen,
                                   "%c" "%s%d%c%d", ARG_DELIM, MatrixIP.privacyMaskNoArg, cnt, ASSIGN_VAL, DISABLE);
            }

            if (outLen > MAX_CAMERA_URI_WIDTH)
            {
                EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
                return CMD_PROCESS_ERROR;
            }
        }
    }
    else
    {
        outLen += snprintf(urlReqPtr[reqCnt].relativeUrl + outLen, MAX_CAMERA_URI_WIDTH - outLen, "%c", ARG_DELIM);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_SET_WINDOW;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return requestStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPtzUrlForMatrixCamera
 * @param   modelNo
 * @param   pan
 * @param   tilt
 * @param   zoom
 * @param   action
 * @param   speed
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setPtzUrlForMatrixCamera(CAMERA_MODEL_e modelNo, PTZ_OPTION_e pan, PTZ_OPTION_e tilt,
                                                 PTZ_OPTION_e zoom, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (modelNo >= MAX_MATRIX_CAMERA_MODEL)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == HAVE_CAMERA_CAPABILITY(modelNo, PTZ_SUPPORT))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setPtzUrlForMatrixOemCamera(modelNo, pan, tilt, zoom, action, speed, urlReqPtr, numOfReq);
    }
    else if (TRUE == isMatrixPtzCameraModel(modelNo))
    {
        return setPtzUrlForMatrixPtzCamera(pan, tilt, zoom, action, speed, urlReqPtr, numOfReq);
    }
    else
    {
        return setPtzUrlForMatrixIpCamera(pan, tilt, zoom, action, speed, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPtzUrlForMatrixOemCam
 * @param   modelNo
 * @param   pan
 * @param   tilt
 * @param   zoom
 * @param   action
 * @param   speed
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setPtzUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom,
                                                    BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8           reqCnt = 0;
    UINT8           channelNo = 1;
    CHAR            fileName[MAX_FILE_NAME_LEN];
    UINT32          fileSize = 0;
    FILE            *optFp = NULL;
    static UINT32   putSetPtzReqCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/PTZCtrl/channels/<ID>/continuous
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s" "%s%s%d" "%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.ptzFolder,
                 MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.ptzContinous);
    }
    else
    {
        // http://<ip>:<port>/ISAPI/PTZCtrl/channels/<ID>/continuous
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s" "%s%s%d" "%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.ptzFolder,
                 MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.ptzContinous);
    }

    if ((pan != MAX_PTZ_PAN_OPTION) || (tilt != MAX_PTZ_TILT_OPTION))
    {
        snprintf(fileName, MAX_FILE_NAME_LEN, PUT_SET_PTZ_FILE_NAME, putSetPtzReqCnt++);

        //Open & Write into options.usb File
        optFp = fopen(fileName, "w");
        if(optFp == NULL)
        {
            return CMD_PROCESS_ERROR;
        }

        fileSize += fprintf(optFp, (TRUE == isTiandyOemCamera(modelNo)) ? PTZ_HEADER(ISAPI_URL_STR) : PTZ_HEADER(""));
        fileSize += fprintf(optFp, PAN_DATA, (((action == START) && (pan != MAX_PTZ_PAN_OPTION)) ? (INT16)(panSpeedStep[pan] * speed) : 0));
        fileSize += fprintf(optFp, TILT_DATA, (((action == START) && (tilt != MAX_PTZ_TILT_OPTION)) ? (INT16)(tiltSpeedStep[tilt] * speed) : 0));
        fileSize += fprintf(optFp, PTZ_END);
        fclose(optFp);

        snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", fileName);
        urlReqPtr[reqCnt].sizeOfPutFile = fileSize;
        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
        reqCnt++;
    }
    else if(zoom != MAX_PTZ_ZOOM_OPTION)
    {
        snprintf(fileName, MAX_FILE_NAME_LEN, PUT_SET_PTZ_FILE_NAME, putSetPtzReqCnt++);

        //Open & Write into options.usb File
        optFp = fopen(fileName, "w");
        if(optFp == NULL)
        {
            return CMD_PROCESS_ERROR;
        }

        fileSize += fprintf(optFp, (TRUE == isTiandyOemCamera(modelNo)) ? PTZ_HEADER(ISAPI_URL_STR) : PTZ_HEADER(""));
        fileSize += fprintf(optFp, ZOOM_DATA, ((action == START) ? (INT16)(zoomSpeedStep[zoom] * speed) : 0));
        fileSize += fprintf(optFp, PTZ_END);
        fclose(optFp);

        snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", fileName);
        urlReqPtr[reqCnt].sizeOfPutFile = fileSize;
        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
        reqCnt++;
    }

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPtzUrlForMatrixIPCam
 * @param   pan
 * @param   tilt
 * @param   zoom
 * @param   action
 * @param   speed
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setPtzUrlForMatrixIpCamera(PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    *numOfReq = 0;
    if (action == STOP)
    {
        return CMD_SUCCESS;
    }

    if (zoom >= MAX_PTZ_ZOOM_OPTION)
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    speed *= (MATRIX_CAMERA_SPEED_MAX / MAX_PTZ_SPEED_STEPS);

    /* http://<ip>:<port>/matrix-cgi/command?action=zoom&step=1&speed=100&format=text */
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/command?action=zoom&step=%d&speed=%d&format=text", zoom, speed);

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPtzUrlForMatrixPtzCamera
 * @param   pan
 * @param   tilt
 * @param   zoom
 * @param   action
 * @param   speed
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setPtzUrlForMatrixPtzCamera(PTZ_OPTION_e pan, PTZ_OPTION_e tilt, PTZ_OPTION_e zoom, BOOL action,
                                                    UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8                   reqCnt = 0;
    MATRIX_PTZ_ACTION_e     operation = 0;

    *numOfReq = 0;
    speed *= (MATRIX_CAMERA_SPEED_MAX / MAX_PTZ_SPEED_STEPS);

    if (zoom == MAX_PTZ_ZOOM_OPTION)
    {
        if ((pan != MAX_PTZ_PAN_OPTION) && (tilt != MAX_PTZ_TILT_OPTION))
        {
            if ((pan == PTZ_PAN_LEFT) && (tilt == PTZ_TILT_UP))
            {
                operation = MATRIX_PTZ_ACTION_TOP_LEFT;
            }
            else if ((pan == PTZ_PAN_RIGHT) && (tilt == PTZ_TILT_UP))
            {
                operation = MATRIX_PTZ_ACTION_TOP_RIGHT;
            }
            else if ((pan == PTZ_PAN_RIGHT) && (tilt == PTZ_TILT_DOWN))
            {
                operation = MATRIX_PTZ_ACTION_BOTTOM_RIGHT;
            }
            else
            {
                operation = MATRIX_PTZ_ACTION_BOTTOM_LEFT;
            }
        }
        else
        {
            if (pan != MAX_PTZ_PAN_OPTION)
            {
                operation = (pan == PTZ_PAN_LEFT) ? MATRIX_PTZ_ACTION_LEFT : MATRIX_PTZ_ACTION_RIGHT;
            }
            else
            {
                operation = (tilt == PTZ_TILT_UP) ? MATRIX_PTZ_ACTION_TOP : MATRIX_PTZ_ACTION_BOTTOM;
            }
        }

        /* http://<ip>:<port>/matrix-cgi/ptzcontrol?action=setpantilt&operation=1&speed=100&state=1&format=text */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/ptzcontrol?action=setpantilt&operation=%d&speed=%d&state=%d&format=text", operation, speed, action);
    }
    else
    {
        /* http://<ip>:<port>/matrix-cgi/command?action=zoom&step=1&speed=100&state=1&format=text */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/command?action=zoom&step=%d&speed=%d&state=%d&format=text", zoom, speed, action);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Based on Model Number this API calls respective function to constructs URL to Set Iris.
 * @param modelNo
 * @param iris
 * @param action
 * @param urlReqPtr
 * @param numOfReq
 * @return
 */
static NET_CMD_STATUS_e setIrisUrlForMatrixCamera(CAMERA_MODEL_e modelNo, CAMERA_IRIS_e iris, BOOL action, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (modelNo >= MAX_MATRIX_CAMERA_MODEL)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == HAVE_CAMERA_CAPABILITY(modelNo, PTZ_SUPPORT))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setIrisUrlForMatrixOemCamera(modelNo, iris, action, urlReqPtr, numOfReq);
    }
    else
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setIrisUrlForMatrixOemCam
 * @param   modelNo
 * @param   iris
 * @param   action
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setIrisUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, CAMERA_IRIS_e iris, BOOL action, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8           reqCnt = 0;
    UINT8           channelNo = 1;
    CHAR            fileName[MAX_FILE_NAME_LEN];
    UINT32          fileSize = 0;
    FILE            *optFp = NULL;
    static UINT32   putSetIrisReqCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/System/Video/inputs/channels/<ID>/iris
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%s%s%d" "%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder, MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.iris);
    }
    else
    {
        // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/<ID>/iris
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%s%s%d" "%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder,
                 MatrixOEM.inputFolder, MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.iris);
    }

    snprintf(fileName, MAX_FILE_NAME_LEN, PUT_SET_IRIS_FILE_NAME, putSetIrisReqCnt++);

    //Open & Write into options.usb File
    optFp = fopen(fileName, "w");
    if(optFp == NULL)
    {
        return CMD_PROCESS_ERROR;
    }

    fileSize += fprintf(optFp, (TRUE == isTiandyOemCamera(modelNo)) ? IRIS_HEADER(ISAPI_URL_STR) : IRIS_HEADER(""));
    fileSize += fprintf(optFp, IRIS_DATA, (((action == START) && (iris != MAX_IRIS_OPTION)) ? (INT16)(irisSpeedStep[iris]) : 0));
    fileSize += fprintf(optFp, IRIS_END);
    fclose(optFp);

    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", fileName);
    urlReqPtr[reqCnt].sizeOfPutFile = fileSize;
    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setFocusUrlForMatrixCamera
 * @param   modelNo
 * @param   focus
 * @param   action
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setFocusUrlForMatrixCamera(CAMERA_MODEL_e modelNo, CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (modelNo >= MAX_MATRIX_CAMERA_MODEL)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == HAVE_CAMERA_CAPABILITY(modelNo, PTZ_SUPPORT))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setFocusUrlForMatrixOemCamera(modelNo, focus, action, speed, urlReqPtr, numOfReq);
    }
    else
    {
        return setFocusUrlForMatrixIpCamera(focus, action, speed, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setFocusUrlForMatrixOemCam
 * @param   modelNo
 * @param   focus
 * @param   speed
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setFocusUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8           reqCnt = 0;
    UINT8           channelNo = 1;
    CHAR            fileName[MAX_FILE_NAME_LEN];
    UINT32          fileSize;
    FILE            *optFp = NULL;
    static UINT32   putSetFocusReqCnt = 0;

    if (action != STOP)
    {
        if (TRUE == isTiandyOemCamera(modelNo))
        {
            /* Note: As per tiandy suggestion, We have changed CGI to IAPI as focus was not working in PZCR20ML_25CWP camera */
            //http://<ip>:<port>/ISAPI/Image/channels/1/FocusMode/template/1/type/1
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%d" "%c" "%s",
                     MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.imageFolder,
                     MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, "FocusMode/template/1/type/0");
        }
        else
        {
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%d" "%c" "%s",
                     MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.imageFolder,
                     MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.focusConfiguration);
        }

        snprintf(fileName, MAX_FILE_NAME_LEN, PUT_SET_FOCUS_FILE_NAME, putSetFocusReqCnt++);
        optFp = fopen(fileName, "w");
        if(optFp == NULL)
        {
            return CMD_PROCESS_ERROR;
        }

        fileSize = fprintf(optFp,XML_VERSION_STR);

        if (TRUE == isTiandyOemCamera(modelNo))
        {
            if(focus == FOCUS_AUTO)
            {
                fileSize += fprintf(optFp, FOCUS_MODE_STR("auto"));
            }
            else
            {
                fileSize += fprintf(optFp, FOCUS_MODE_STR("manual"));
            }
        }
        else
        {
            if(focus == FOCUS_AUTO)
            {
                fileSize = fprintf(optFp, FOCUS_CONFIG_HEADER("AUTO"));
            }
            else
            {
                fileSize = fprintf(optFp, FOCUS_CONFIG_HEADER("MANUAL"));
            }
        }

        fclose(optFp);

        snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", fileName);
        urlReqPtr[reqCnt].sizeOfPutFile = fileSize;
        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
        reqCnt++;
    }

    if (focus != FOCUS_AUTO)
    {
        if (TRUE == isTiandyOemCamera(modelNo))
        {
            // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/<ID>/focus
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%s%s%d" "%c" "%s",
                     MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder, MatrixOEM.videoFolder,
                     MatrixOEM.inputFolder, MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.focus);
        }
        else
        {
            // http://<ip>:<port>/ISAPI/System/Video/inputs/channels/<ID>/focus
            snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%s%s%d" "%c" "%s",
                     MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.systemFolder,
                     MatrixOEM.videoFolder, MatrixOEM.inputFolder, MatrixOEM.channelsFolder, channelNo,
                     DIRECTORY_DELIM_CHAR, MatrixOEM.focus);
        }

        snprintf(fileName, MAX_FILE_NAME_LEN, PUT_SET_FOCUS_FILE_NAME, putSetFocusReqCnt++);

        optFp = fopen(fileName, "w");
        if(optFp == NULL)
        {
            return CMD_PROCESS_ERROR;
        }

        fileSize = fprintf(optFp, (TRUE == isTiandyOemCamera(modelNo)) ? FOCUS_HEADER(ISAPI_URL_STR) : FOCUS_HEADER(""));
        fileSize += fprintf(optFp, FOCUS_DATA, (((action == START) && (focus != MAX_FOCUS_OPTION)) ? (INT16)(focusSpeedStep[focus] * speed) : 0));
        fileSize += fprintf(optFp, FOCUS_END);
        fclose(optFp);

        snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", fileName);
        urlReqPtr[reqCnt].sizeOfPutFile = fileSize;
        urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
        urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
        urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
        urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
        reqCnt++;
    }

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setFocusUrlForMatrixIPCam
 * @param   focus
 * @param   action
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setFocusUrlForMatrixIpCamera(CAMERA_FOCUS_e focus, BOOL action, UINT8 speed, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    if (action == STOP)
    {
        return CMD_SUCCESS;
    }

    if (focus >= MAX_FOCUS_OPTION)
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    speed *= (MATRIX_CAMERA_SPEED_MAX / MAX_PTZ_SPEED_STEPS);

    /* http://<ip>:<port>/matrix-cgi/command?action=focus&step=1&speed=100&format=text */
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/command?action=focus&step=%d&speed=%d&state=%d&format=text", focus, speed, action);

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   gotoPtzUrlForMatrixCamera
 * @param   modelNo
 * @param   ptzIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e gotoPtzUrlForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (modelNo >= MAX_MATRIX_CAMERA_MODEL)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == HAVE_CAMERA_CAPABILITY(modelNo, PTZ_SUPPORT))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return gotoPtzUrlForMatrixOemCamera(modelNo, ptzIndex, urlReqPtr, numOfReq);
    }
    else if (TRUE == isMatrixPtzCameraModel(modelNo))
    {
        return gotoPtzUrlForMatrixPtzCamera(ptzIndex, urlReqPtr, numOfReq);
    }
    else
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   gotoPtzUrlForMatrixOemCamera
 * @param   ptzIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e gotoPtzUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8   reqCnt = 0;
    UINT8   channelNo = 1;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/PTZCtrl/channels/<ID>/presets/<ID>/goto
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%d" "%c" "%s%d" "%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.ptzFolder,
                 MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR,
                 MatrixOEM.presets, ptzIndex, DIRECTORY_DELIM_CHAR, MatrixOEM.gotoPreset);
    }
    else
    {
        // http://<ip>:<port>/ISAPI/PTZCtrl/channels/<ID>/presets/<ID>/goto
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%d" "%c" "%s%d" "%c" "%s",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.ptzFolder,
                 MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR,
                 MatrixOEM.presets, ptzIndex, DIRECTORY_DELIM_CHAR, MatrixOEM.gotoPreset);
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REBOOT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   gotoPtzUrlForMatrixPtzCamera
 * @param   ptzIndex
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e gotoPtzUrlForMatrixPtzCamera(UINT8 ptzIndex, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    /* http://<ip>:<port>/matrix-cgi/ptzpreset?action=call&preset-no=1&format=text */
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/ptzpreset?action=call&preset-no=%d&format=text", ptzIndex);

    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   storePtzUrlForMatrixCamera
 * @param   modelNo
 * @param   ptzIndex
 * @param   presetName
 * @param   presetAction
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e storePtzUrlForMatrixCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, CHAR *presetName, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (modelNo >= MAX_MATRIX_CAMERA_MODEL)
    {
        return CMD_PROCESS_ERROR;
    }

    if (FALSE == HAVE_CAMERA_CAPABILITY(modelNo, PTZ_SUPPORT))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return storePtzUrlForMatrixOemCamera(modelNo, ptzIndex, presetAction, urlReqPtr, numOfReq);
    }
    else if (TRUE == isMatrixPtzCameraModel(modelNo))
    {
        return storePtzUrlForMatrixPtzCamera(modelNo, ptzIndex, presetName, presetAction, urlReqPtr, numOfReq);
    }
    else
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   storePtzUrlForMatrixOemCamera
 * @param   modelNo
 * @param   ptzIndex
 * @param   presetAction
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e storePtzUrlForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8           reqCnt = 0;
    UINT8           channelNo = 1;
    CHAR            fileName[MAX_FILE_NAME_LEN];
    UINT32          fileSize;
    FILE            *optFp = NULL;
    static UINT32   putSetFocusReqCnt = 0;

    if (TRUE == isTiandyOemCamera(modelNo))
    {
        // http://<ip>:<port>/CGI/PTZCtrl/channels/<ID>/presets/<ID>
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%d" "%c" "%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.ptzFolder,
                 MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.presets, ptzIndex);
    }
    else
    {
        // http://<ip>:<port>/ISAPI/PTZCtrl/channels/<ID>/presets/<ID>
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s" "%s%d" "%c" "%s%d",
                 MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.ptzFolder,
                 MatrixOEM.channelsFolder, channelNo, DIRECTORY_DELIM_CHAR, MatrixOEM.presets, ptzIndex);
    }

    if (presetAction == ADDED)
    {
        snprintf(fileName, MAX_FILE_NAME_LEN, PUT_SET_PRESET_FILE_NAME, putSetFocusReqCnt++);

        //Open & Write into options.usb File
        optFp = fopen(fileName, "w");
        if(optFp == NULL)
        {
            return CMD_PROCESS_ERROR;
        }

        fileSize = fprintf(optFp, (TRUE == isTiandyOemCamera(modelNo)) ? PRESET_HEADER(ISAPI_URL_STR) : PRESET_HEADER(""));
        fileSize += fprintf(optFp, PRESET_ENABLE);
        fileSize += fprintf(optFp, ID_TAG_STR, ptzIndex);
        fileSize += fprintf(optFp, PRESET_NAME, ptzIndex);
        fileSize += fprintf(optFp, PRESET_END);
        fclose(optFp);

        snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", fileName);
        urlReqPtr[reqCnt].sizeOfPutFile = fileSize;
        urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
        urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    }
    else
    {
        urlReqPtr[reqCnt].httpRequestType = DELETE_REQUEST;
    }

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   storePtzUrlForMatrixPtzCamera
 * @param   modelNo
 * @param   ptzIndex
 * @param   presetName
 * @param   presetAction
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e storePtzUrlForMatrixPtzCamera(CAMERA_MODEL_e modelNo, UINT8 ptzIndex, CHARPTR presetName, BOOL presetAction, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    if (presetAction == ADDED)
    {
        /* http://<ip>:<port>/matrix-cgi/ptzpreset?action=set&preset-no=1&name=position_name&format=text */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/ptzpreset?action=set&preset-no=%d&name=%s&format=text", ptzIndex, presetName);
    }
    else
    {
        /* http://<ip>:<port>/matrix-cgi/ptzpreset?action=delete&preset-no=1&format=text */
        snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/ptzpreset?action=delete&preset-no=%d&format=text", ptzIndex);
    }

    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setPasswordUrlForMatrixCamera
 * @param   modelNo
 * @param   userName
 * @param   passwd
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setPasswordUrlForMatrixCamera(CAMERA_MODEL_e modelNo,CHARPTR userName,CHARPTR passwd, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    if ((modelNo < MATRIX_MODEL_SATATYA_CIDR20FL_36CW_S) || (modelNo >= MAX_MATRIX_CAMERA_MODEL))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "%s" "%s%s%c" "%s%c%s%c" "%s%c%s%c" "%s%c%s%c" "%s%c%s%c" "%s%c%s",
             MatrixIP.rootFolder, MatrixIP.cgiFolder, MatrixIP.user, URL_DELIM,
             MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET_PASSWD],ARG_DELIM,
             MatrixIP.userName,ASSIGN_VAL,userName,ARG_DELIM, MatrixIP.currPasswd,ASSIGN_VAL,passwd,ARG_DELIM,
             MatrixIP.passwd,ASSIGN_VAL,passwd,ARG_DELIM, MatrixIP.confPasswd,ASSIGN_VAL,passwd);

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = POST_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendAudioToMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   numOfStopReq
 * @param   sendAudInfo
 * @return
 */
static NET_CMD_STATUS_e sendAudioToMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                UINT8PTR numOfStopReq, SEND_AUDIO_INFO_t* sendAudInfo)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        if (modelNo == MATRIX_MODEL_SATATYA_CIDRP20VL_130CW)
        {
            return sendAudioToMatrixOemCamera(modelNo, urlReqPtr, numOfReq, numOfStopReq, sendAudInfo);
        }
    }
    else if ((TRUE == isMatrixPremiumCameraModel(modelNo)) || (TRUE == isMatrixPtzCameraModel(modelNo)))
    {
        return sendAudioToMatrixIpCamera(modelNo, urlReqPtr, numOfReq, numOfStopReq, sendAudInfo);
    }

    return CMD_FEATURE_NOT_SUPPORTED;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendAudioToMatrixIpCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   numOfStopReq
 * @param   sendAudInfo
 * @return
 */
static NET_CMD_STATUS_e sendAudioToMatrixIpCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                  UINT8PTR numOfStopReq, SEND_AUDIO_INFO_t* sendAudInfo)
{
    UINT8   reqCnt = 0;
    UINT8   stopReqCnt = 0;
    CHAR 	usrPwStr[MAX_USERNAME_WIDTH + MAX_PASSWORD_WIDTH];
    CHARPTR encodedStrPtr;
    UINT32 	usrPwStrLen;

    sendAudInfo->httpUserAgent = CURL_USER_AGENT;

    // first url to configure audio channel
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/audio?action=set&audio-out=1");
    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    // Encoding userName and password in Base64 method
    snprintf(usrPwStr, sizeof(usrPwStr), "%s:%s", sendAudInfo->userName, sendAudInfo->password);
    usrPwStrLen = strlen(usrPwStr);

    /* encode into base64 */
    encodedStrPtr = EncodeBase64(usrPwStr, usrPwStrLen);
    if (encodedStrPtr == NULL)
    {
        EPRINT(CAMERA_INTERFACE, "error while encoding base64");
        return CMD_PROCESS_ERROR;
    }

    // second url open audio channel and it is directly send on TCP socket
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "POST /networkaudio?action=get HTTP/1.1\r\n"
             "Authorization: Basic %s\r\n"
             "Content-Length: 0\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             encodedStrPtr,
             AUDIO_MIME_TYPE_STR);

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = POST_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    *(numOfStopReq) = stopReqCnt;

    /* free memory for encoded string */
    free(encodedStrPtr);
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   sendAudioToMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   numOfStopReq
 * @param   sendAudInfo
 * @return
 */
static NET_CMD_STATUS_e sendAudioToMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                   UINT8PTR numOfStopReq, SEND_AUDIO_INFO_t* sendAudInfo)
{
    NET_CMD_STATUS_e 	retVal = CMD_SUCCESS;
    UINT8 				reqCnt = 0;
    UINT8 				stopReqCnt = 0;
    CHAR 				usrPwStr[MAX_USERNAME_WIDTH + MAX_PASSWORD_WIDTH];
    CHARPTR				encodedStrPtr;
    UINT32 				usrPwStrLen;
    INT32				putFileFd = INVALID_FILE_FD;
    CHAR 				putBuf[MAX_PUT_REQ_BUF_SIZE];
    size_t				bufSize = 0;

    sendAudInfo->httpUserAgent = NS_HTTP_USER_AGENT;

    // STEP 1 : Configure Audio Channel 1
    if (TRUE == isTiandyOemCamera(modelNo))
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, TWO_WAY_AUDIO_STR(ISAPI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }
    else
    {
        if((bufSize += snprintf(putBuf+bufSize, MAX_PUT_REQ_BUF_SIZE-bufSize, TWO_WAY_AUDIO_STR(STD_CGI_URL_STR))) > MAX_PUT_REQ_BUF_SIZE)
        {
            return CMD_PROCESS_ERROR;
        }
    }

    putFileFd = open(PUT_OPEN_TWO_WAY_AUDIO_FILE_NAME, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if(write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }
    close(putFileFd);

    // http://<ip>:<port><ID>
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/ISAPI/System/TwoWayAudio/channels/1");

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", PUT_OPEN_TWO_WAY_AUDIO_FILE_NAME);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    reqCnt++;

    // STEP 2 :  Open Audio Channel 1
    // http://<ip>:<port>
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/ISAPI/System/TwoWayAudio/channels/1/open");

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    urlReqPtr[reqCnt].fileForPutReq[0] = '\0';
    urlReqPtr[reqCnt].sizeOfPutFile = 0;
    reqCnt++;

    // STEP 3 : Create Channel for sending data

    // Encoding userName and password in Base64 method
    snprintf(usrPwStr, sizeof(usrPwStr),"%s:%s",sendAudInfo->userName,sendAudInfo->password);
    usrPwStrLen = strlen(usrPwStr);

    /* encode into base64 */
    encodedStrPtr = EncodeBase64(usrPwStr, usrPwStrLen);

    if (encodedStrPtr == NULL)
    {
        EPRINT(CAMERA_INTERFACE, "error while encoding base64");
        return CMD_PROCESS_ERROR;
    }

    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "PUT /ISAPI/System/TwoWayAudio/channels/1/audioData HTTP/1.1\r\n"
             "Authorization: Basic %s\r\n"
             "User-Agent: NS-HTTP/1.0\r\n"
             "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*.*;q=0.8\r\n"
             "Accept-Language: ZH-cn;zh;q=0.5\r\n"
             "Accept-Charset: gb2312,utf8;q=0.7,*;q=0.7\r\n"
             "Connection: keep-alive\r\n"
             "Content-Length: 0\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             encodedStrPtr,
             AUDIO_MIME_TYPE_STR);

    urlReqPtr[reqCnt].requestType = CAM_REQ_MEDIA;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    // STEP 4 : Close channel
    // http://<ip>:<port>
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/ISAPI/System/TwoWayAudio/channels/1/close");

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    urlReqPtr[reqCnt].fileForPutReq[0] = '\0';
    urlReqPtr[reqCnt].sizeOfPutFile = 0;
    reqCnt++;
    stopReqCnt++;

    *numOfReq = reqCnt;
    *(numOfStopReq) = stopReqCnt;

    /* free memory for encoded string */
    free(encodedStrPtr);
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getMaxPrivacyMaskWindowForMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getMaxPrivacyMaskWindowForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return CMD_FEATURE_NOT_SUPPORTED;
    }
    else
    {
        if ((modelNo >= MATRIX_MODEL_SATATYA_CIBR30FL_36CG) && (modelNo <= MATRIX_MODEL_SATATYA_CIDR30FL_60CW))
        {
            return CMD_FEATURE_NOT_SUPPORTED;
        }

        return getMaxPrivacyMaskWindowIpStandardProfessionalCamera(modelNo, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getMaxPrivacyMaskWindowIpStandardProfessionalCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e getMaxPrivacyMaskWindowIpStandardProfessionalCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    UINT8 reqCnt = 0;

    // first url to configure audio channel
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/matrix-cgi/command?action=maximum-privacy-mask");

    urlReqPtr[reqCnt].requestType = CAM_REQ_GET_MAXWIN;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setDateTimeToMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setDateTimeToMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return setDateTimeToMatrixOemCamera(modelNo, urlReqPtr, numOfReq);
    }
    else
    {
        return setDateTimeToMatrixCameraGeneral(modelNo, urlReqPtr, numOfReq);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setDateTimeToMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setDateTimeToMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    struct tm           localDateTime;
    UINT8               reqCnt = 0;
    CHAR                putBuf[MAX_PUT_REQ_BUF_SIZE];
    CHAR                *pCharSearch;
    CHAR                timeZoneStr[15];
    size_t              bufSize = 0;
    CHAR                putFileName[MAX_FILE_NAME_LEN];
    INT32               putFileFd = INVALID_FILE_FD;
    static UINT32       putSetDateTimeReqCnt = 0;
    DATE_TIME_CONFIG_t  dateTimeConfig;

    if (SUCCESS != GetLocalTimeInBrokenTm(&localDateTime))
    {
        EPRINT(CAMERA_INTERFACE, "failed to get local time");
        return CMD_PROCESS_ERROR;
    }

    ReadDateTimeConfig(&dateTimeConfig);

    /* Tiandy required timezone in CST+05:30:00 format. We stored timezone with inverted sign */
    snprintf(timeZoneStr, sizeof(timeZoneStr), "%s:00", OnviftimeZoneNames[dateTimeConfig.timezone-1][1]);

    /* Invert the sign from - to + and + to - */
    pCharSearch = strchr(timeZoneStr, '-');
    if (pCharSearch != NULL)
    {
        pCharSearch[0] = '+';
    }
    else
    {
        pCharSearch = strchr(timeZoneStr, '+');
        if (pCharSearch != NULL)
        {
            pCharSearch[0] = '-';
        }
    }

    bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, DATE_TIME_STR, localDateTime.tm_year, (localDateTime.tm_mon + 1), localDateTime.tm_mday,
                       localDateTime.tm_hour, localDateTime.tm_min, localDateTime.tm_sec, timeZoneStr);
    if (bufSize > MAX_PUT_REQ_BUF_SIZE)
    {
        return CMD_PROCESS_ERROR;
    }

    snprintf(putFileName, MAX_FILE_NAME_LEN, PUT_DATE_TIME_FILE_NAME, putSetDateTimeReqCnt++);
    putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
    if(putFileFd == INVALID_FILE_FD)
    {
        EPRINT(CAMERA_INTERFACE, "failed to open put file: [err=%s]", STR_ERR);
        return CMD_PROCESS_ERROR;
    }

    if(write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
    {
        EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
        close(putFileFd);
        return CMD_PROCESS_ERROR;
    }

    close(putFileFd);

    // http://<ip>:<port>/ISAPI/System/time
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "/ISAPI/System/time");

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
    snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
    urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   setDateTimeToMatrixCameraGeneral
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @return
 */
static NET_CMD_STATUS_e setDateTimeToMatrixCameraGeneral(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq)
{
    struct tm   localDateTime;
    UINT8       reqCnt = 0;

    if(SUCCESS != GetLocalTimeInBrokenTm(&localDateTime))
    {
        EPRINT(CAMERA_INTERFACE, "failed to get local time");
        return CMD_PROCESS_ERROR;
    }

    // first url to configure audio channel
    snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH,
             "/matrix-cgi/datetime?action=setrtc&date=%d&month=%d&year=%d&hour=%d&minute=%d&second=%d",
             localDateTime.tm_mday,(localDateTime.tm_mon + 1),localDateTime.tm_year,localDateTime.tm_hour,
             localDateTime.tm_min,localDateTime.tm_sec);

    urlReqPtr[reqCnt].requestType = CAM_REQ_CONTROL;
    urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
    urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
    urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
    reqCnt++;

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   imageSettingForMatrixCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   action
 * @param   pImageCapsInfo
 * @return
 */
static NET_CMD_STATUS_e imageSettingForMatrixCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                    IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return imageSettingForMatrixOemCamera(modelNo, urlReqPtr, numOfReq, action, pImageCapsInfo);
    }
    else
    {
        return imageSettingForMatrixCameraGeneral(modelNo, urlReqPtr, numOfReq, action, pImageCapsInfo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getImageSettingForMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   action
 * @param   pImageCapsInfo
 * @return
 */
static NET_CMD_STATUS_e imageSettingForMatrixOemCamera(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                       IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    UINT8 reqCnt = 0;

    *numOfReq = 0;
    switch(action)
    {
        case IMAGE_SETTING_ACTION_GET_CAPABILITY:
        {
            if (TRUE == isTiandyOemCamera(modelNo))
            {
                //http://<ip>:<port>/CGI/Image/channels/1/template/0/type/1
                snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s",
                         MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.imageFolder, MatrixOEM.channelsFolder, "1/template/0/type/1");
            }
            else
            {
                //http://<ip>:<port>/ISAPI/Image/channels/1/capabilities
                snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d%c%s",
                         MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.imageFolder, MatrixOEM.channelsFolder, 1, DIRECTORY_DELIM_CHAR, "capabilities");
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_IMG_APPEARANCE;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            reqCnt++;
        }
        break;

        case IMAGE_SETTING_ACTION_GET_PARAM:
        {
            if (TRUE == isTiandyOemCamera(modelNo))
            {
                //http://<ip>:<port>/CGI/Image/channels/1/template/0/type/1
                snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s",
                         MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.imageFolder, MatrixOEM.channelsFolder, "1/template/0/type/1");
            }
            else
            {
                //http://<ip>:<port>/ISAPI/Image/channels/1
                snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d",
                         MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.imageFolder, MatrixOEM.channelsFolder, 1);
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_IMG_APPEARANCE;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            reqCnt++;
        }
        break;

        case IMAGE_SETTING_ACTION_SET_PARAM:
        {
            CHAR            putBuf[MAX_PUT_REQ_BUF_SIZE];
            size_t          bufSize = 0;
            CHAR            putFileName[MAX_FILE_NAME_LEN];
            INT32           putFileFd = INVALID_FILE_FD;
            static UINT32   putImageSettingFileCnt = 0;

            if (TRUE == isTiandyOemCamera(modelNo))
            {
                bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, TIANDY_IMAGE_SETTING_STR,
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->brightness.value, pImageCapsInfo->brightness.step, pImageCapsInfo->brightness.min, BRIGHTNESS_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->contrast.value, pImageCapsInfo->contrast.step, pImageCapsInfo->contrast.min, CONTRAST_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->saturation.value, pImageCapsInfo->saturation.step, pImageCapsInfo->saturation.min, SATURATION_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->hue.value, pImageCapsInfo->hue.step, pImageCapsInfo->hue.min, HUE_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->sharpness.value, pImageCapsInfo->sharpness.step, pImageCapsInfo->sharpness.min, SHARPNESS_MIN),
                                   imageSettingOemWdrStr[pImageCapsInfo->wdr.mode],
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->wdrStrength.value, pImageCapsInfo->wdrStrength.step, pImageCapsInfo->wdrStrength.min, WDR_STRENGTH_MIN),
                                   imageSettingOemWbStr[pImageCapsInfo->whiteBalance.mode][getOemCameraBrand(modelNo)]);
                if (bufSize > MAX_PUT_REQ_BUF_SIZE)
                {
                    return CMD_PROCESS_ERROR;
                }
            }
            else
            {
                bufSize = snprintf(putBuf, MAX_PUT_REQ_BUF_SIZE, HIKVISION_IMAGE_SETTING_STR, 1,
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->brightness.value, pImageCapsInfo->brightness.step, pImageCapsInfo->brightness.min, BRIGHTNESS_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->contrast.value, pImageCapsInfo->contrast.step, pImageCapsInfo->contrast.min, CONTRAST_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->saturation.value, pImageCapsInfo->saturation.step, pImageCapsInfo->saturation.min, SATURATION_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->hue.value, pImageCapsInfo->hue.step, pImageCapsInfo->hue.min, HUE_MIN),
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->sharpness.value, pImageCapsInfo->sharpness.step, pImageCapsInfo->sharpness.min, SHARPNESS_MIN),
                                   imageSettingOemWdrStr[pImageCapsInfo->wdr.mode],
                                   (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->wdrStrength.value, pImageCapsInfo->wdrStrength.step, pImageCapsInfo->wdrStrength.min, WDR_STRENGTH_MIN),
                                   imageSettingOemWbStr[pImageCapsInfo->whiteBalance.mode][getOemCameraBrand(modelNo)]);
                if (bufSize > MAX_PUT_REQ_BUF_SIZE)
                {
                    return CMD_PROCESS_ERROR;
                }
            }

            snprintf(putFileName, sizeof(putFileName), PUT_IMAGE_SETTING_FILE_NAME, putImageSettingFileCnt++);
            putFileFd = open(putFileName, CREATE_RDWR_MODE, USR_RWE_GRP_RE_OTH_RE);
            if (putFileFd == INVALID_FILE_FD)
            {
                EPRINT(CAMERA_INTERFACE, "fail to open put file: [err=%s]", STR_ERR);
                return CMD_PROCESS_ERROR;
            }

            if (write(putFileFd, putBuf, bufSize) != (ssize_t)bufSize)
            {
                EPRINT(CAMERA_INTERFACE, "failed to write put file: [err=%s]", STR_ERR);
                close(putFileFd);
                return CMD_PROCESS_ERROR;
            }
            close(putFileFd);

            if (TRUE == isTiandyOemCamera(modelNo))
            {
                //http://<ip>:<port>/CGI/Image/channels/1/template/0/type/1
                snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%s",
                         MatrixOEM.rootFolder, MatrixOEM.cgiFolder, MatrixOEM.imageFolder, MatrixOEM.channelsFolder, "1/template/0/type/1");
            }
            else
            {
                //http://<ip>:<port>/ISAPI/Image/channels/1
                snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s%s%s%s%d",
                         MatrixOEM.rootFolder, MatrixOEM.isapiFolder, MatrixOEM.imageFolder, MatrixOEM.channelsFolder, 1);
            }

            urlReqPtr[reqCnt].requestType = CAM_REQ_IMG_APPEARANCE;
            urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
            urlReqPtr[reqCnt].httpRequestType = PUT_REQUEST;
            urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            snprintf(urlReqPtr[reqCnt].fileForPutReq, MAX_FILE_NAME_LENGTH, "%s", putFileName);
            urlReqPtr[reqCnt].sizeOfPutFile = bufSize;
            urlReqPtr[reqCnt].httpContentType = HTTP_CONTENT_TYPE_XML;
            reqCnt++;
        }
        break;

        default:
            return CMD_PROCESS_ERROR;
    }

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   getImageSettingForMatrixOemCamera
 * @param   modelNo
 * @param   urlReqPtr
 * @param   numOfReq
 * @param   action
 * @param   pImageCapsInfo
 * @return
 */
static NET_CMD_STATUS_e imageSettingForMatrixCameraGeneral(CAMERA_MODEL_e modelNo, URL_REQUEST_t *urlReqPtr, UINT8PTR numOfReq,
                                                           IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    UINT8               reqCnt = 0;
    UINT32              outLen = 0;
    MATRIX_SDK_NUM_e    sdk = MATRIX_IP_SDK;

    *numOfReq = 0;
    switch(action)
    {
        case IMAGE_SETTING_ACTION_GET_CAPABILITY:
        {
            for (reqCnt = 0; reqCnt < CAM_REQ_IMG_MAX; reqCnt++)
            {
                outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s",
                                  MatrixIP.rootFolder, MatrixIP.cgiFolder, imageSettingReqStr[reqCnt], URL_DELIM,
                                  MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET_CAP]);
                if (outLen > MAX_CAMERA_URI_WIDTH)
                {
                    EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
                    return CMD_PROCESS_ERROR;
                }

                urlReqPtr[reqCnt].requestType = reqCnt;
                urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
                urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
                urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            }
        }
        break;

        case IMAGE_SETTING_ACTION_GET_PARAM:
        {
            for (reqCnt = 0; reqCnt < CAM_REQ_IMG_MAX; reqCnt++)
            {
                outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s" "%c%s",
                                  MatrixIP.rootFolder, MatrixIP.cgiFolder, imageSettingReqStr[reqCnt], URL_DELIM,
                                  MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_GET],
                                  ARG_DELIM, MATRIX_IP_CAM_IMAGE_SETTING_TEMPLATE);
                if (outLen > MAX_CAMERA_URI_WIDTH)
                {
                    EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
                    return CMD_PROCESS_ERROR;
                }

                urlReqPtr[reqCnt].requestType = reqCnt;
                urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
                urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
                urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
            }
        }
        break;

        case IMAGE_SETTING_ACTION_SET_PARAM:
        {
            CHAR    paramBuf[MAX_PUT_REQ_BUF_SIZE];
            size_t  bufSize = 0;

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_BRIGHTNESS))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_BRIGHTNESS][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->brightness.value, pImageCapsInfo->brightness.step, pImageCapsInfo->brightness.min, BRIGHTNESS_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_CONTRAST))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_CONTRAST][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->contrast.value, pImageCapsInfo->contrast.step, pImageCapsInfo->contrast.min, CONTRAST_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_SATURATION))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_SATURATION][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->saturation.value, pImageCapsInfo->saturation.step, pImageCapsInfo->saturation.min, SATURATION_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_HUE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_HUE][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->hue.value, pImageCapsInfo->hue.step, pImageCapsInfo->hue.min, HUE_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_SHARPNESS))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_SHARPNESS][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->sharpness.value, pImageCapsInfo->sharpness.step, pImageCapsInfo->sharpness.min, SHARPNESS_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_WHITE_BALANCE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_WHITE_BALANCE][sdk], ASSIGN_VAL, pImageCapsInfo->whiteBalance.mode);
            }

            if (bufSize)
            {
                outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s" "%c%s" "%s",
                                  MatrixIP.rootFolder, MatrixIP.cgiFolder, imageSettingReqStr[reqCnt], URL_DELIM,
                                  MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET],
                                  ARG_DELIM, MATRIX_IP_CAM_IMAGE_SETTING_TEMPLATE, paramBuf);
                if (outLen > MAX_CAMERA_URI_WIDTH)
                {
                    EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
                    return CMD_PROCESS_ERROR;
                }

                urlReqPtr[reqCnt].requestType = reqCnt;
                urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
                urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
                urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
                reqCnt++;
            }

            bufSize = 0;
            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_WDR_MODE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_WDR_MODE][sdk], ASSIGN_VAL, pImageCapsInfo->wdr.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_BACKLIGHT))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_BACKLIGHT][sdk], ASSIGN_VAL, pImageCapsInfo->backlightControl.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_WDR_STRENGTH))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_WDR_STRENGTH][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->wdrStrength.value, pImageCapsInfo->wdrStrength.step,
                                                                    pImageCapsInfo->wdrStrength.min, WDR_STRENGTH_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_EXPOSURE_RATIO_MODE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_EXPOSURE_RATIO_MODE][sdk], ASSIGN_VAL, pImageCapsInfo->exposureRatioMode.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_EXPOSURE_RATIO))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_EXPOSURE_RATIO][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureRatio.value, pImageCapsInfo->exposureRatio.step,
                                                                    pImageCapsInfo->exposureRatio.min, EXPOSURE_RATIO_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_EXPOSURE_MODE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_EXPOSURE_MODE][sdk], ASSIGN_VAL, pImageCapsInfo->exposureMode.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_FLICKER))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_FLICKER][sdk], ASSIGN_VAL, pImageCapsInfo->flicker.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_FLICKER_STRENGTH))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_FLICKER_STRENGTH][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->flickerStrength.value, pImageCapsInfo->flickerStrength.step,
                                                                    pImageCapsInfo->flickerStrength.min, FLICKER_STRENGTH_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_HLC))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_HLC][sdk], ASSIGN_VAL, pImageCapsInfo->hlc.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_EXPOSURE_TIME))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_EXPOSURE_TIME][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureTime.value, pImageCapsInfo->exposureTime.step,
                                                                    pImageCapsInfo->exposureTime.min, EXPOSURE_TIME_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_EXPOSURE_GAIN))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_EXPOSURE_GAIN][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureGain.value, pImageCapsInfo->exposureGain.step,
                                                                    pImageCapsInfo->exposureGain.min, EXPOSURE_GAIN_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_EXPOSURE_IRIS))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_EXPOSURE_IRIS][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->exposureIris.value, pImageCapsInfo->exposureIris.step,
                                                                    pImageCapsInfo->exposureIris.min, EXPOSURE_IRIS_MIN));
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_NORMAL_LIGHT_GAIN))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_NORMAL_LIGHT_GAIN][sdk], ASSIGN_VAL, pImageCapsInfo->normalLightGain.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_NORMAL_LIGHT_LUMINANCE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_NORMAL_LIGHT_LUMINANCE][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->normalLightLuminance.value, pImageCapsInfo->normalLightLuminance.step,
                                                                    pImageCapsInfo->normalLightLuminance.min, NORMAL_LIGHT_LUMINANCE_MIN));
            }

            if (bufSize)
            {
                outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s" "%c%s" "%s",
                                  MatrixIP.rootFolder, MatrixIP.cgiFolder, imageSettingReqStr[reqCnt], URL_DELIM,
                                  MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM, MATRIX_IP_CAM_IMAGE_SETTING_TEMPLATE, paramBuf);
                if (outLen > MAX_CAMERA_URI_WIDTH)
                {
                    EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
                    return CMD_PROCESS_ERROR;
                }

                urlReqPtr[reqCnt].requestType = reqCnt;
                urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
                urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
                urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
                reqCnt++;
            }

            bufSize = 0;
            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_LED_MODE))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_LED_MODE][sdk], ASSIGN_VAL, pImageCapsInfo->irLed.mode);
            }

            if (GET_BIT(pImageCapsInfo->imagingCapability, IMG_SETTING_LED_SENSITIVITY))
            {
                bufSize += snprintf(&paramBuf[bufSize], MAX_PUT_REQ_BUF_SIZE - bufSize, "%c%s%c%d",
                                    ARG_DELIM, imageSettingParamStr[IMG_SETTING_LED_SENSITIVITY][sdk], ASSIGN_VAL,
                                    (UINT32)SET_IMAGE_SETTING_VALUE(pImageCapsInfo->irLedSensitivity.value, pImageCapsInfo->irLedSensitivity.step, pImageCapsInfo->irLedSensitivity.min, LED_SENSITIVITY_MIN));
            }

            if (bufSize)
            {
                outLen = snprintf(urlReqPtr[reqCnt].relativeUrl, MAX_CAMERA_URI_WIDTH, "%s" "%s%s%c" "%s%c%s" "%c%s" "%s",
                                  MatrixIP.rootFolder, MatrixIP.cgiFolder, imageSettingReqStr[reqCnt], URL_DELIM,
                                  MatrixIP.action, ASSIGN_VAL, matrixIpCamAction[ACTION_SET], ARG_DELIM, MATRIX_IP_CAM_IMAGE_SETTING_TEMPLATE, paramBuf);
                if (outLen > MAX_CAMERA_URI_WIDTH)
                {
                    EPRINT(CAMERA_INTERFACE, "more buffer required: [outLen=%d]", outLen);
                    return CMD_PROCESS_ERROR;
                }

                urlReqPtr[reqCnt].requestType = reqCnt;
                urlReqPtr[reqCnt].protocolType = CAM_HTTP_PROTOCOL;
                urlReqPtr[reqCnt].httpRequestType = GET_REQUEST;
                urlReqPtr[reqCnt].authMethod = AUTH_TYPE_ANY;
                reqCnt++;
            }
        }
        break;

        default:
            return CMD_PROCESS_ERROR;
    }

    *numOfReq = reqCnt;
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseImageSettingForMatrixCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   action
 * @param   pImageCapsInfo
 * @return
 */
static NET_CMD_STATUS_e parseImageSettingForMatrixCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                         IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    if (TRUE == isMatrixOemCameraModel(modelNo))
    {
        return parseImageSettingForMatrixOemCamera(modelNo, dataSize, data, action, pImageCapsInfo);
    }
    else
    {
        return parseImageSettingForMatrixCameraGeneral(modelNo, dataSize, data, action, pImageCapsInfo);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseImageSettingForMatrixOemCamera
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   action
 * @param   pImageCapsInfo
 * @return
 */
static NET_CMD_STATUS_e parseImageSettingForMatrixOemCamera(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                            IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    CHAR                tagValStr[MAX_SIZE_OF_STRING];
    CHAR                *pSearchTag, *pParamStr;
    UINT8               mode;
    UINT32              paramCnt;
    UINT64              parseValue = 0;
    MATRIX_SDK_NUM_e    sdk = MATRIX_OEM_SDK;

    switch(action)
    {
        case IMAGE_SETTING_ACTION_GET_CAPABILITY:
        {
            for (paramCnt = 0; paramCnt < IMAGE_SETTING_CAPABILITY_MAX; paramCnt++)
            {
                /* Skip param if we don't require */
                if (imageSettingParamStr[paramCnt][sdk][0] == '\0')
                {
                    continue;
                }

                /* Start from begging */
                pSearchTag = data;

                /* Search for param tag */
                if (getXMLvalue(&pSearchTag, imageSettingParamStr[paramCnt][sdk], tagValStr, MAX_SIZE_OF_STRING, TRUE) == FAIL)
                {
                    /* Tag not found */
                    continue;
                }

                /* Tiandy camera doesn't provide the capability in any API and We have limited OEM cameras.
                 * Hence, fixed the capability range for all OEM cameras */
                pImageCapsInfo->imagingCapability |= MX_ADD(paramCnt);
                switch(paramCnt)
                {
                    case IMG_SETTING_BRIGHTNESS:
                    {
                        pImageCapsInfo->brightness.min = imageSettingParamRange[paramCnt].minRange;
                        pImageCapsInfo->brightness.max = imageSettingParamRange[paramCnt].maxRange;
                        pImageCapsInfo->brightness.step = GET_IMAGE_SETTING_STEP_VALUE(pImageCapsInfo->brightness.min, pImageCapsInfo->brightness.max, pImageCapsInfo->brightness.max);
                    }
                    break;

                    case IMG_SETTING_CONTRAST:
                    {
                        pImageCapsInfo->contrast.min = imageSettingParamRange[paramCnt].minRange;
                        pImageCapsInfo->contrast.max = imageSettingParamRange[paramCnt].maxRange;
                        pImageCapsInfo->contrast.step = GET_IMAGE_SETTING_STEP_VALUE(pImageCapsInfo->contrast.min, pImageCapsInfo->contrast.max, pImageCapsInfo->contrast.max);
                    }
                    break;

                    case IMG_SETTING_SATURATION:
                    {
                        pImageCapsInfo->saturation.min = imageSettingParamRange[paramCnt].minRange;
                        pImageCapsInfo->saturation.max = imageSettingParamRange[paramCnt].maxRange;
                        pImageCapsInfo->saturation.step = GET_IMAGE_SETTING_STEP_VALUE(pImageCapsInfo->saturation.min, pImageCapsInfo->saturation.max, pImageCapsInfo->saturation.max);
                    }
                    break;

                    case IMG_SETTING_HUE:
                    {
                        pImageCapsInfo->hue.min = imageSettingParamRange[paramCnt].minRange;
                        pImageCapsInfo->hue.max = imageSettingParamRange[paramCnt].maxRange;
                        pImageCapsInfo->hue.step = GET_IMAGE_SETTING_STEP_VALUE(pImageCapsInfo->hue.min, pImageCapsInfo->hue.max, pImageCapsInfo->hue.max);
                    }
                    break;

                    case IMG_SETTING_SHARPNESS:
                    {
                        pImageCapsInfo->sharpness.min = imageSettingParamRange[paramCnt].minRange;
                        pImageCapsInfo->sharpness.max = imageSettingParamRange[paramCnt].maxRange;
                        pImageCapsInfo->sharpness.step = GET_IMAGE_SETTING_STEP_VALUE(pImageCapsInfo->sharpness.min, pImageCapsInfo->sharpness.max, pImageCapsInfo->sharpness.max);
                    }
                    break;

                    case IMG_SETTING_WHITE_BALANCE:
                    {
                        for (mode = 0; mode < WHITE_BALANCE_MODE_MAX; mode++)
                        {
                            pImageCapsInfo->whiteBalance.modeSupported |= MX_ADD(mode);
                        }
                    }
                    break;

                    case IMG_SETTING_WDR_MODE:
                    {
                        UINT8 maxMode = isTiandyOemCamera(modelNo) ? WDR_MODE_MAX : (WDR_MODE_MAX-1);
                        for (mode = 0; mode < maxMode; mode++)
                        {
                            pImageCapsInfo->wdr.modeSupported |= MX_ADD(mode);
                        }
                    }
                    break;

                    case IMG_SETTING_WDR_STRENGTH:
                    {
                        pImageCapsInfo->wdrStrength.min = imageSettingParamRange[paramCnt].minRange;
                        pImageCapsInfo->wdrStrength.max = imageSettingParamRange[paramCnt].maxRange;
                        pImageCapsInfo->wdrStrength.step = GET_IMAGE_SETTING_STEP_VALUE(pImageCapsInfo->wdrStrength.min, pImageCapsInfo->wdrStrength.max,
                                                                                        (pImageCapsInfo->wdrStrength.max - pImageCapsInfo->wdrStrength.min));
                    }
                    break;
                }
            }
        }
        break;

        case IMAGE_SETTING_ACTION_GET_PARAM:
        {
            for (paramCnt = 0; paramCnt < IMAGE_SETTING_CAPABILITY_MAX; paramCnt++)
            {
                /* Skip param if we don't require */
                if ((imageSettingParamStr[paramCnt][sdk][0] == '\0') || (GET_BIT(pImageCapsInfo->imagingCapability, paramCnt) == 0))
                {
                    continue;
                }

                /* Start from beggining and search required tag */
                pSearchTag = data;
                pParamStr = imageSettingParamStr[paramCnt][sdk];

                /* There is different parsing required for WDR mode */
                if (paramCnt == IMG_SETTING_WDR_MODE)
                {
                    /* Example:
                     * <WDR>
                     *      <mode>close</mode>
                     *      <WDRLevel>50</WDRLevel>
                     * </WDR>
                     */
                    pSearchTag = strstr(pSearchTag, pParamStr);
                    if (pSearchTag == NULL)
                    {
                        continue;
                    }

                    /* Now we need mode tag */
                    pParamStr = "mode";
                }

                /* Search for param tag */
                if (getXMLvalue(&pSearchTag, pParamStr, tagValStr, MAX_SIZE_OF_STRING, TRUE) == FAIL)
                {
                    /* Tag not found */
                    continue;
                }

                /* Range value tag should have ascii value */
                if (TRUE == imageSettingParamRange[paramCnt].isRangeParam)
                {
                    if (FAIL == AsciiToInt(tagValStr, &parseValue))
                    {
                        continue;
                    }
                }

                /* Set parse field value */
                switch(paramCnt)
                {
                    case IMG_SETTING_BRIGHTNESS:
                    {
                        pImageCapsInfo->brightness.value = GET_IMAGE_SETTING_VALUE((float)parseValue, pImageCapsInfo->brightness.step, pImageCapsInfo->brightness.min,
                                                                                   pImageCapsInfo->brightness.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_CONTRAST:
                    {
                        pImageCapsInfo->contrast.value = GET_IMAGE_SETTING_VALUE((float)parseValue, pImageCapsInfo->contrast.step, pImageCapsInfo->contrast.min,
                                                                                 pImageCapsInfo->contrast.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_SATURATION:
                    {
                        pImageCapsInfo->saturation.value = GET_IMAGE_SETTING_VALUE((float)parseValue, pImageCapsInfo->saturation.step, pImageCapsInfo->saturation.min,
                                                                                   pImageCapsInfo->saturation.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_HUE:
                    {
                        pImageCapsInfo->hue.value = GET_IMAGE_SETTING_VALUE((float)parseValue, pImageCapsInfo->hue.step, pImageCapsInfo->hue.min,
                                                                            pImageCapsInfo->hue.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_SHARPNESS:
                    {
                        pImageCapsInfo->sharpness.value = GET_IMAGE_SETTING_VALUE((float)parseValue, pImageCapsInfo->sharpness.step, pImageCapsInfo->sharpness.min,
                                                                                  pImageCapsInfo->sharpness.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_WHITE_BALANCE:
                    {
                        pImageCapsInfo->whiteBalance.mode = imageSettingParamRange[paramCnt].minRange;
                        for (mode = imageSettingParamRange[paramCnt].minRange; mode <= imageSettingParamRange[paramCnt].maxRange; mode++)
                        {
                            if (strcmp(imageSettingOemWbStr[mode][getOemCameraBrand(modelNo)], tagValStr) == 0)
                            {
                                pImageCapsInfo->whiteBalance.mode = mode;
                                break;
                            }
                        }
                    }
                    break;

                    case IMG_SETTING_WDR_MODE:
                    {
                        pImageCapsInfo->wdr.mode = imageSettingParamRange[paramCnt].minRange;
                        for (mode = imageSettingParamRange[paramCnt].minRange; mode <= imageSettingParamRange[paramCnt].maxRange; mode++)
                        {
                            if (strcmp(imageSettingOemWdrStr[mode], tagValStr) == 0)
                            {
                                pImageCapsInfo->wdr.mode = mode;
                                break;
                            }
                        }
                    }
                    break;

                    case IMG_SETTING_WDR_STRENGTH:
                    {
                        pImageCapsInfo->wdrStrength.value = GET_IMAGE_SETTING_VALUE((float)parseValue, pImageCapsInfo->wdrStrength.step, pImageCapsInfo->wdrStrength.min,
                                                                                    pImageCapsInfo->wdrStrength.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;
                }
            }
        }
        break;

        case IMAGE_SETTING_ACTION_SET_PARAM:
        {
            /* Search for response tag */
            pSearchTag = strstr(data, responseStatusStr[sdk]);
            if (pSearchTag == NULL)
            {
                /* Response code tag not found */
                return CMD_PROCESS_ERROR;
            }

            /* Parse resonse code. 0 = success, other = error */
            pSearchTag += (strlen(responseStatusStr[sdk]) + 1);
            if (atoi(pSearchTag) != 1)
            {
                /* Error found in response */
                return CMD_PROCESS_ERROR;
            }
        }
        break;

        default:
        {
            /* Invalid action  found */
            return CMD_PROCESS_ERROR;
        }
    }

    /* Response parsed successfully */
    return CMD_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   parseImageSettingForMatrixCameraGeneral
 * @param   modelNo
 * @param   dataSize
 * @param   data
 * @param   action
 * @param   pImageCapsInfo
 * @return
 */
static NET_CMD_STATUS_e parseImageSettingForMatrixCameraGeneral(CAMERA_MODEL_e modelNo, UINT32 dataSize, CHARPTR data,
                                                                IMAGE_SETTING_ACTION_e action, IMAGE_CAPABILITY_INFO_t *pImageCapsInfo)
{
    CHAR    oneLineData[200];
    CHAR    tagNameStr[30];
    UINT8   tagNameLen;
    UINT32  paramCnt;
    CHARPTR mainData = NULL;
    CHARPTR linePtr = NULL;
    CHARPTR parseData = NULL;
    INT32   convertedData = 0;

    switch(action)
    {
        case IMAGE_SETTING_ACTION_GET_CAPABILITY:
        {
            BOOL isCapabilityFound;
            INT32   min, max, mode;
            UINT32  modeMask;
            mainData = data;
            while((parseData = strstr(mainData, "\n")) != NULL)
            {
                /* Parse response data line by line */
                parseData++;
                snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);
                mainData = data + (strlen(data) - strlen(parseData));
                linePtr = oneLineData;

                /* Parse resonse code. 0 = success, other = error */
                if ((parseData = strstr(linePtr, responseStatusStr[MATRIX_IP_SDK])) != NULL)
                {
                    linePtr += (strlen(responseStatusStr[MATRIX_IP_SDK]) + 1);
                    convertedData = atoi(linePtr);
                    if (convertedData != 0)
                    {
                        /* Error found in response */
                        pImageCapsInfo->imagingCapability = 0;
                        return CMD_FEATURE_NOT_SUPPORTED;
                    }

                    /* Continue for success */
                    continue;
                }

                modeMask = 0;
                min = max = 0;
                isCapabilityFound = FALSE;
                for (paramCnt = 0; paramCnt < IMAGE_SETTING_CAPABILITY_MAX; paramCnt++)
                {
                    /* Check each response tag with required tags */
                    tagNameLen = snprintf(tagNameStr, sizeof(tagNameStr), "%s%c", imageSettingParamStr[paramCnt][MATRIX_IP_SDK], ASSIGN_VAL);
                    if (strncasecmp(linePtr, tagNameStr, tagNameLen) != 0)
                    {
                        /* It is not required tag */
                        continue;
                    }

                    /* Required tag found, now get its value */
                    linePtr += tagNameLen;

                    /* Is it range value tag? (e.g. "brightness=0 - 100") */
                    if (TRUE == imageSettingParamRange[paramCnt].isRangeParam)
                    {
                        /* It must have '-' */
                        if ((parseData = strchr(linePtr, '-')) == NULL)
                        {
                            break;
                        }

                        /* Now parse before and after '-' value */
                        parseData++;
                        min = atoi(linePtr);
                        max = atoi(parseData);

                        /* Validate min and max value */
                        if (IS_VALID_IMAGE_SETTING_RANGE(min, max))
                        {
                            /* Min and max are valid. Hence add in capability */
                            pImageCapsInfo->imagingCapability |= MX_ADD(paramCnt);
                            isCapabilityFound = TRUE;
                        }
                    }
                    else /* It is mode value tag? (e.g. "wb=0,1,2,3|Auto,Fluorescent,Incandescent,Sunny") */
                    {
                        /* It must have ',' */
                        if (strchr(linePtr, ',') == NULL)
                        {
                            break;
                        }

                        /* It should have '|' */
                        if ((parseData = strchr(linePtr, '|')) != NULL)
                        {
                            parseData[0] = '\0';
                        }

                        /* Parse each value separated by ',' */
                        while(linePtr[0] != ',')
                        {
                            /* Get supported mode value */
                            mode = atoi(linePtr);

                            /* If it is valid value then set its bit */
                            if (((UINT32)mode >= imageSettingParamRange[paramCnt].minRange) && ((UINT32)mode <= imageSettingParamRange[paramCnt].maxRange))
                            {
                                modeMask |= MX_ADD(mode);
                            }

                            if ((linePtr = strchr(linePtr, ',')) == NULL)
                            {
                                break;
                            }

                            /* Skip ',' */
                            linePtr++;
                        }

                        /* Do we get any valid mode? */
                        if (modeMask)
                        {
                            /* Atleast one mode is valid. Hence add in capability */
                            pImageCapsInfo->imagingCapability |= MX_ADD(paramCnt);
                            isCapabilityFound = TRUE;
                        }
                    }

                    /* Store parased value */
                    break;
                }

                if (FALSE == isCapabilityFound)
                {
                    /* Required value not found */
                    continue;
                }

                /* Set parse field capabilities */
                switch(paramCnt)
                {
                    case IMG_SETTING_BRIGHTNESS:
                    {
                        pImageCapsInfo->brightness.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, BRIGHTNESS_MAX);
                        pImageCapsInfo->brightness.min = min;
                        pImageCapsInfo->brightness.max = max;
                    }
                    break;

                    case IMG_SETTING_CONTRAST:
                    {
                        pImageCapsInfo->contrast.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, CONTRAST_MAX);
                        pImageCapsInfo->contrast.min = min;
                        pImageCapsInfo->contrast.max = max;
                    }
                    break;

                    case IMG_SETTING_SATURATION:
                    {
                        pImageCapsInfo->saturation.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, SATURATION_MAX);
                        pImageCapsInfo->saturation.min = min;
                        pImageCapsInfo->saturation.max = max;
                    }
                    break;

                    case IMG_SETTING_HUE:
                    {
                        pImageCapsInfo->hue.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, HUE_MAX);
                        pImageCapsInfo->hue.min = min;
                        pImageCapsInfo->hue.max = max;
                    }
                    break;

                    case IMG_SETTING_SHARPNESS:
                    {
                        pImageCapsInfo->sharpness.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, SHARPNESS_MAX);
                        pImageCapsInfo->sharpness.min = min;
                        pImageCapsInfo->sharpness.max = max;
                    }
                    break;

                    case IMG_SETTING_WHITE_BALANCE:
                    {
                        pImageCapsInfo->whiteBalance.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_WDR_MODE:
                    {
                        pImageCapsInfo->wdr.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_WDR_STRENGTH:
                    {
                        pImageCapsInfo->wdrStrength.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (WDR_STRENGTH_MAX - WDR_STRENGTH_MIN));
                        pImageCapsInfo->wdrStrength.min = min;
                        pImageCapsInfo->wdrStrength.max = max;
                    }
                    break;

                    case IMG_SETTING_BACKLIGHT:
                    {
                        pImageCapsInfo->backlightControl.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_RATIO_MODE:
                    {
                        pImageCapsInfo->exposureRatioMode.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_RATIO:
                    {
                        pImageCapsInfo->exposureRatio.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (EXPOSURE_RATIO_MAX - EXPOSURE_RATIO_MIN));
                        pImageCapsInfo->exposureRatio.min = min;
                        pImageCapsInfo->exposureRatio.max = max;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_MODE:
                    {
                        pImageCapsInfo->exposureMode.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_FLICKER:
                    {
                        pImageCapsInfo->flicker.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_FLICKER_STRENGTH:
                    {
                        pImageCapsInfo->flickerStrength.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (FLICKER_STRENGTH_MAX - FLICKER_STRENGTH_MIN));
                        pImageCapsInfo->flickerStrength.min = min;
                        pImageCapsInfo->flickerStrength.max = max;
                    }
                    break;

                    case IMG_SETTING_HLC:
                    {
                        pImageCapsInfo->hlc.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_TIME:
                    {
                        pImageCapsInfo->exposureTime.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (EXPOSURE_TIME_MAX - EXPOSURE_TIME_MIN));
                        pImageCapsInfo->exposureTime.min = min;
                        pImageCapsInfo->exposureTime.max = max;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_GAIN:
                    {
                        pImageCapsInfo->exposureGain.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (EXPOSURE_GAIN_MAX - EXPOSURE_GAIN_MIN));
                        pImageCapsInfo->exposureGain.min = min;
                        pImageCapsInfo->exposureGain.max = max;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_IRIS:
                    {
                        pImageCapsInfo->exposureIris.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (EXPOSURE_IRIS_MAX - EXPOSURE_IRIS_MIN));
                        pImageCapsInfo->exposureIris.min = min;
                        pImageCapsInfo->exposureIris.max = max;
                    }
                    break;

                    case IMG_SETTING_NORMAL_LIGHT_GAIN:
                    {
                        pImageCapsInfo->normalLightGain.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_NORMAL_LIGHT_LUMINANCE:
                    {
                        pImageCapsInfo->normalLightLuminance.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, (NORMAL_LIGHT_LUMINANCE_MAX - NORMAL_LIGHT_LUMINANCE_MIN));
                        pImageCapsInfo->normalLightLuminance.min = min;
                        pImageCapsInfo->normalLightLuminance.max = max;
                    }
                    break;

                    case IMG_SETTING_LED_MODE:
                    {
                        pImageCapsInfo->irLed.modeSupported = modeMask;
                    }
                    break;

                    case IMG_SETTING_LED_SENSITIVITY:
                    {
                        pImageCapsInfo->irLedSensitivity.step = GET_IMAGE_SETTING_STEP_VALUE(min, max, LED_SENSITIVITY_MAX);
                        pImageCapsInfo->irLedSensitivity.min = min;
                        pImageCapsInfo->irLedSensitivity.max = max;
                    }
                    break;
                }
            }
        }
        break;

        case IMAGE_SETTING_ACTION_GET_PARAM:
        {
            INT32 value;

            mainData = data;
            while((parseData = strstr(mainData, "\n")) != NULL)
            {
                /* Parse response data line by line */
                parseData++;
                snprintf(oneLineData, (strlen(mainData) - strlen(parseData))+1, "%s", mainData);
                mainData = data + (strlen(data) - strlen(parseData));
                linePtr = oneLineData;

                /* Parse resonse code. 0 = success, other = error */
                if ((parseData = strstr(linePtr, responseStatusStr[MATRIX_IP_SDK])) != NULL)
                {
                    linePtr += (strlen(responseStatusStr[MATRIX_IP_SDK]) + 1);
                    convertedData = atoi(linePtr);
                    if (convertedData != 0)
                    {
                        /* Error found in response */
                        pImageCapsInfo->imagingCapability = 0;
                        return CMD_FEATURE_NOT_SUPPORTED;
                    }

                    /* Continue for success */
                    continue;
                }

                value = IMAGE_SETTING_PARAM_MODE_INVALID;
                for (paramCnt = 0; paramCnt < IMAGE_SETTING_CAPABILITY_MAX; paramCnt++)
                {
                    /* Check each response tag with required tags */
                    snprintf(tagNameStr, sizeof(tagNameStr), "%s%c", imageSettingParamStr[paramCnt][MATRIX_IP_SDK], ASSIGN_VAL);
                    if ((parseData = strstr(linePtr, tagNameStr)) == NULL)
                    {
                        /* It is not required tags */
                        continue;
                    }

                    /* Did we get capability for this param? */
                    if (GET_BIT(pImageCapsInfo->imagingCapability, paramCnt) == 0)
                    {
                        /* Capability not found */
                        continue;
                    }

                    /* Parse tag value */
                    linePtr += strlen(tagNameStr);
                    value = atoi(linePtr);

                    /* Successfully parased value */
                    break;
                }

                if (paramCnt >= IMAGE_SETTING_CAPABILITY_MAX)
                {
                    /* Required value not found */
                    continue;
                }

                /* Set parse field value */
                switch(paramCnt)
                {
                    case IMG_SETTING_BRIGHTNESS:
                    {
                        pImageCapsInfo->brightness.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->brightness.step, pImageCapsInfo->brightness.min,
                                                                                   pImageCapsInfo->brightness.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_CONTRAST:
                    {
                        pImageCapsInfo->contrast.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->contrast.step, pImageCapsInfo->contrast.min,
                                                                                 pImageCapsInfo->contrast.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_SATURATION:
                    {
                        pImageCapsInfo->saturation.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->saturation.step, pImageCapsInfo->saturation.min,
                                                                                   pImageCapsInfo->saturation.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_HUE:
                    {
                        pImageCapsInfo->hue.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->hue.step, pImageCapsInfo->hue.min,
                                                                            pImageCapsInfo->hue.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_SHARPNESS:
                    {
                        pImageCapsInfo->sharpness.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->sharpness.step, pImageCapsInfo->sharpness.min,
                                                                                  pImageCapsInfo->sharpness.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_WHITE_BALANCE:
                    {
                        pImageCapsInfo->whiteBalance.mode = value;
                    }
                    break;

                    case IMG_SETTING_WDR_MODE:
                    {
                        pImageCapsInfo->wdr.mode = value;
                    }
                    break;

                    case IMG_SETTING_WDR_STRENGTH:
                    {
                        pImageCapsInfo->wdrStrength.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->wdrStrength.step, pImageCapsInfo->wdrStrength.min,
                                                                                    pImageCapsInfo->wdrStrength.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_BACKLIGHT:
                    {
                        pImageCapsInfo->backlightControl.mode = value;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_RATIO_MODE:
                    {
                        pImageCapsInfo->exposureRatioMode.mode = value;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_RATIO:
                    {
                        pImageCapsInfo->exposureRatio.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->exposureRatio.step, pImageCapsInfo->exposureRatio.min,
                                                                                      pImageCapsInfo->exposureRatio.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_MODE:
                    {
                        pImageCapsInfo->exposureMode.mode = value;
                    }
                    break;

                    case IMG_SETTING_FLICKER:
                    {
                        pImageCapsInfo->flicker.mode = value;
                    }
                    break;

                    case IMG_SETTING_FLICKER_STRENGTH:
                    {
                        pImageCapsInfo->flickerStrength.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->flickerStrength.step, pImageCapsInfo->flickerStrength.min,
                                                                                        pImageCapsInfo->flickerStrength.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_HLC:
                    {
                        pImageCapsInfo->hlc.mode = value;
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_TIME:
                    {
                        pImageCapsInfo->exposureTime.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->exposureTime.step, pImageCapsInfo->exposureTime.min,
                                                                                     pImageCapsInfo->exposureTime.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_GAIN:
                    {
                        pImageCapsInfo->exposureGain.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->exposureGain.step, pImageCapsInfo->exposureGain.min,
                                                                                     pImageCapsInfo->exposureGain.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_EXPOSURE_IRIS:
                    {
                        pImageCapsInfo->exposureIris.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->exposureIris.step, pImageCapsInfo->exposureIris.min,
                                                                                     pImageCapsInfo->exposureIris.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_NORMAL_LIGHT_GAIN:
                    {
                        pImageCapsInfo->normalLightGain.mode = value;
                    }
                    break;

                    case IMG_SETTING_NORMAL_LIGHT_LUMINANCE:
                    {
                        pImageCapsInfo->normalLightLuminance.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->normalLightLuminance.step, pImageCapsInfo->normalLightLuminance.min,
                                                                                             pImageCapsInfo->normalLightLuminance.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;

                    case IMG_SETTING_LED_MODE:
                    {
                        pImageCapsInfo->irLed.mode = value;
                    }
                    break;

                    case IMG_SETTING_LED_SENSITIVITY:
                    {
                        pImageCapsInfo->irLedSensitivity.value = GET_IMAGE_SETTING_VALUE((float)value, pImageCapsInfo->irLedSensitivity.step, pImageCapsInfo->irLedSensitivity.min,
                                                                                         pImageCapsInfo->irLedSensitivity.max, imageSettingParamRange[paramCnt].minRange);
                    }
                    break;
                }
            }
        }
        break;

        case IMAGE_SETTING_ACTION_SET_PARAM:
        {
            /* Search for response tag */
            parseData = strstr(data, responseStatusStr[MATRIX_IP_SDK]);
            if (parseData == NULL)
            {
                /* Response code tag not found */
                return CMD_PROCESS_ERROR;
            }

            /* Parse resonse code. 0 = success, other = error */
            parseData += (strlen(responseStatusStr[MATRIX_IP_SDK]) + 1);
            convertedData = atoi(parseData);
            if (convertedData != 0)
            {
                /* Error found in response */
                return CMD_PROCESS_ERROR;
            }
        }
        break;

        default:
        {
            /* Invalid action  found */
            return CMD_PROCESS_ERROR;
        }
    }

    /* Response parsed successfully */
    return CMD_SUCCESS;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
