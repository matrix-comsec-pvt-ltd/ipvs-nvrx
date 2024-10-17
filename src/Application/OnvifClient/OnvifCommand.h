#ifndef ONVIFCOMMAND_H_
#define ONVIFCOMMAND_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		OnvifCommand.h
@brief      This file provides interface between ONVIF stack (soap library) and onvif client module
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "ConfigComnDef.h"
#include "MxOnvifClient.h"

/* Library Includes */
#include "soapH.h"

//#################################################################################################
// @DEFINES AND DATA_TYPES
//#################################################################################################
typedef struct
{
	CHARPTR wsa5__Action;
	CHARPTR wsa5_to;
	CHARPTR addInfo;
}SOAP_WSA5_t;

typedef struct
{
	BOOL 			authReq;
	CHAR 			name[MAX_CAMERA_USERNAME_WIDTH];
	CHAR 			pwd[MAX_CAMERA_PASSWORD_WIDTH];
    CHAR 			ipAddr[IPV6_ADDR_LEN_MAX];
	UINT16 			port;
	CHARPTR 		addr;
    TIME_DIFF_t 	*timeDiff;
}SOAP_USER_DETAIL_t;

typedef struct soap SOAP_t;
#define XSD_BOOL_TRUE													xsd__boolean__true_
#define XSD_BOOL_FALSE													xsd__boolean__false_
#define IPV6_DHCP_OFF                                                   tt__IPv6DHCPConfiguration__Off

/* Namespace :tds :http://www.onvif.org/ver10/device/wsdl */
typedef struct _tds__GetSystemDateAndTime 		  						GET_SYSTEM_DATE_AND_TIME_t;
typedef struct _tds__GetSystemDateAndTimeResponse 						GET_SYSTEM_DATE_AND_TIME_RESPONSE_t;
typedef struct _tds__SetSystemDateAndTime								SET_SYSTEM_DATE_AND_TIME_t;
typedef struct _tds__SetSystemDateAndTimeResponse						SET_SYSTEM_DATE_AND_TIME_RESPONSE_t;
typedef struct _tds__GetDeviceInformation 		  						GET_DEVICE_INFORMATION_t;
typedef struct _tds__GetDeviceInformationResponse						GET_DEVICE_INFORMATION_RESPONSE_t;
typedef struct _tds__SetUser											SET_USER_INFORMATION_t;
typedef struct _tds__SetUserResponse									SET_USER_INFORMATION_RESPONSE_t;
typedef struct _tds__GetCapabilities 									GET_CAPABILITIES_t;
typedef struct _tds__GetCapabilitiesResponse 							GET_CAPABILITIES_RESPONSE_t;
typedef struct _tds__GetServices                                        GET_SERVICES_t;
typedef struct _tds__GetServicesResponse                                GET_SERVICES_RESPONSE_t;
typedef struct _tds__GetRelayOutputs  	  								GET_RELAY_OUTPUTS_t;
typedef struct _tds__GetRelayOutputsResponse 							GET_RELAY_OUTPUTS_RESPONSE_t;
typedef struct _tds__SetRelayOutputState								SET_RELAY_OUTPUT_STATE_t;
typedef struct _tds__SetRelayOutputStateResponse						SET_RELAY_OUTPUT_STATE_RESPONSE_t;
typedef struct _tds__GetNetworkInterfaces                               GET_NETWORK_INTERFACES_t;
typedef struct _tds__GetNetworkInterfacesResponse                       GET_NETWORK_INTERFACESRESPONSE_t;
typedef struct _tds__SetNetworkInterfaces                               SET_NETWORK_INTERFACES_t;
typedef struct _tds__SetNetworkInterfacesResponse                       SET_NETWORK_INTERFACESRESPONSE_t;
typedef struct _tds__SetNetworkDefaultGateway                           SET_NETWORK_DFLT_GATEWAY_t;
typedef struct _tds__SetNetworkDefaultGatewayResponse                   SET_NETWORK_DFLT_GATEWAYRESPONSE_t;
typedef struct _tds__SystemReboot                                       SYSTEM_REBOOT_t;
typedef struct _tds__SystemRebootResponse                               SYSTEM_REBOOTRESPONSE_t;
typedef struct _tds__GetNetworkProtocols                                GET_NETWORK_PROTOCOLS_t;
typedef struct _tds__GetNetworkProtocolsResponse                        GET_NETWORK_PROTOCOLS_RES_t;
typedef struct _tds__SetNetworkProtocols                                SET_NETWORK_PROTOCOLS_t;
typedef struct _tds__SetNetworkProtocolsResponse                        SET_NETWORK_PROTOCOLS_RES_t;
typedef struct _tds__GetDNS                                             GET_DNS_t;
typedef struct _tds__GetDNSResponse                                     GET_DNS_RES_t;
typedef struct _tds__SetDNS                                             SET_DNS_t;
typedef struct _tds__SetDNSResponse                                     SET_DNS_RES_t;
typedef struct _tds__GetNetworkDefaultGateway                           GET_NET_DEFAULT_GATEWAY_t;
typedef struct _tds__GetNetworkDefaultGatewayResponse                   GET_NET_DEFAULT_GATEWAY_RES_t;
typedef struct _tds__GetUsers                                           GET_USER_t;
typedef struct _tds__GetUsersResponse                                   GET_USER_RES_t;
typedef struct _tds__CreateUsers                                        CREATE_USER_t;
typedef struct _tds__CreateUsersResponse                                CREATE_USER_RES_t;
typedef struct _tds__DeleteUsers                                        DELETE_USER_t;
typedef struct _tds__DeleteUsersResponse                                DELETE_USER_RES_t;
typedef struct _tds__SetRelayOutputSettings                             SET_RELAY_OUT_SETTING_t;
typedef struct _tds__SetRelayOutputSettingsResponse                     SET_RELAY_OUT_SETTING_RES_t;

#define GetSystemDateAndTimeCommand				  						soap_call___tds__GetSystemDateAndTime
#define SetSystemDateAndTimeCommand										soap_call___tds__SetSystemDateAndTime
#define GetDeviceInformationCommand				 						soap_call___tds__GetDeviceInformation
#define SetUserCommand													soap_call___tds__SetUser
#define GetCapabilitiesCommand											soap_call___tds__GetCapabilities
#define GetCapabilitiesCommand2                                         soap_call___tr2__GetServiceCapabilities
#define GetServicesCommand                                              soap_call___tds__GetServices
#define GetRelayOutputsCommand											soap_call___tds__GetRelayOutputs
#define SetRelayOutputStateCommand										soap_call___tds__SetRelayOutputState
#define GetNetworkInterfacesCommand                                     soap_call___tds__GetNetworkInterfaces
#define SetNetworkInterfacesCommand                                     soap_call___tds__SetNetworkInterfaces
#define SetNetworkDfltGatewayCommand                                    soap_call___tds__SetNetworkDefaultGateway
#define SystemRebootCommand                                             soap_call___tds__SystemReboot
#define GetNetworkProtocolsCommand                                      soap_call___tds__GetNetworkProtocols
#define SetNetworkProtocolsCommand                                      soap_call___tds__SetNetworkProtocols
#define GetDnsCommand                                                   soap_call___tds__GetDNS
#define SetDnsCommand                                                   soap_call___tds__SetDNS
#define GetNetworkDefaultGatwayCmd                                      soap_call___tds__GetNetworkDefaultGateway
#define GetUsersCmd                                                     soap_call___tds__GetUsers
#define CreateUsersCmd                                                  soap_call___tds__CreateUsers
#define DeleteUsersCmd                                                  soap_call___tds__DeleteUsers
#define SetRelayOutputSattingCmd                                        soap_call___tds__SetRelayOutputSettings

/* Namespace :trt: http://www.onvif.org/ver10/media/wsdl */
typedef struct _trt__GetProfiles 										GET_PROFILES_t;
typedef struct _trt__GetProfilesResponse 								GET_PROFILES_RESPONSE_t;
typedef struct _trt__GetVideoEncoderConfigurationOptions  				GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_t;
typedef struct _trt__GetVideoEncoderConfigurationOptionsResponse 		GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t;
typedef struct _trt__GetProfile 										GET_PROFILE_t;
typedef struct _trt__GetProfileResponse 								GET_PROFILE_RESPONSE_t;
typedef struct _trt__GetCompatibleVideoSourceConfigurations  			GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_t;
typedef struct _trt__GetCompatibleVideoSourceConfigurationsResponse 	GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t;
typedef struct _trt__GetCompatibleVideoEncoderConfigurations  			GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_t;
typedef struct _trt__GetCompatibleVideoEncoderConfigurationsResponse 	GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_RESPONSE_t;
typedef struct _trt__GetAudioSources  									GET_AUDIO_SOURCES_t;
typedef struct _trt__GetAudioSourcesResponse 							GET_AUDIO_SOURCES_RESPONSE_t;
typedef struct _trt__GetAudioEncoderConfigurations  					GET_AUDIO_ENCODER_CONFIGURATIONS_t;
typedef struct _trt__GetAudioEncoderConfigurationsResponse 				GET_AUDIO_ENCODER_CONFIGURATIONS_RESPONSE_t;
typedef struct _trt__GetAudioSourceConfigurations  						GET_AUDIO_SOURCE_CONFIGURATIONS_t;
typedef struct _trt__GetAudioSourceConfigurationsResponse 				GET_AUDIO_SOURCE_CONFIGURATIONS_RESPONSE_t;
typedef struct _trt__GetVideoSourceConfigurations		   				GET_VIDEO_SOURCE_CONFIGURATIONS_t;
typedef struct _trt__GetVideoSourceConfigurationsResponse  				GET_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t;
typedef struct _trt__AddVideoSourceConfiguration		 				ADD_VIDEO_SOURCE_CONFIGURATION_t;
typedef struct _trt__AddVideoSourceConfigurationResponse 				ADD_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t;
typedef struct _trt__SetVideoEncoderConfiguration		   				SET_VIDEO_ENCODER_CONFIGURATION_t;
typedef struct _trt__SetVideoEncoderConfigurationResponse  				SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t;
typedef struct _trt__GetVideoEncoderConfiguration		   				GET_VIDEO_ENCODER_CONFIGURATION_t;
typedef struct _trt__GetVideoEncoderConfigurationResponse  				GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t;
typedef struct _trt__AddVideoEncoderConfiguration						ADD_VIDEO_ENCODER_CONFIGURATION_t;
typedef struct _trt__AddVideoEncoderConfigurationResponse				ADD_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t;
typedef struct _trt__AddAudioEncoderConfiguration						ADD_AUDIO_ENCODER_CONFIGURATION_t;
typedef struct _trt__AddAudioEncoderConfigurationResponse				ADD_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t;
typedef struct _trt__AddAudioSourceConfiguration						ADD_AUDIO_SOURCE_CONFIGURATION_t;
typedef struct _trt__AddAudioSourceConfigurationResponse				ADD_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t;
typedef struct _trt__RemoveAudioEncoderConfiguration					REMOVE_AUDIO_ENCODER_CONFIGURATION_t;
typedef struct _trt__RemoveAudioEncoderConfigurationResponse			REMOVE_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t;
typedef struct _trt__RemoveAudioSourceConfiguration						REMOVE_AUDIO_SOURCE_CONFIGURATION_t;
typedef struct _trt__RemoveAudioSourceConfigurationResponse				REMOVE_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t;
typedef struct _trt__GetStreamUri										GET_STREAM_URI_t;
typedef struct _trt__GetStreamUriResponse								GET_STREAM_URI_RESPONSE_t;
typedef struct _trt__GetSnapshotUri										GET_SNAPSHOT_URI_t;
typedef struct _trt__GetSnapshotUriResponse								GET_SNAPSHOT_URI_RESPONSE_t;
typedef struct _trt__GetOSDs                                            GET_OSDS_t;
typedef struct _trt__GetOSDsResponse                                    GET_OSDSRESPONSE_t;
typedef struct _trt__SetOSD                                             SET_OSD_t;
typedef struct _trt__SetOSDResponse                                     SET_OSDRESPONSE_t;
typedef struct _trt__CreateOSD                                          CREATE_OSD_t;
typedef struct _trt__CreateOSDResponse                                  CREATE_OSDRESPONSE_t;
typedef struct _trt__DeleteOSD                                          DELETE_OSD_t;
typedef	struct _trt__DeleteOSDResponse                                  DELETE_OSDRESPONSE_t;
typedef struct _trt__GetVideoAnalyticsConfigurations                    GET_VIDEOANALYTICSCONFIGS_t;
typedef struct _trt__GetVideoAnalyticsConfigurationsResponse            GET_VIDEOANALYTICSCONFIGSRESPONSE_t;
typedef struct _trt__GetVideoAnalyticsConfiguration                     GET_VIDEOANALYTICSCONFIG_t;
typedef struct _trt__GetVideoAnalyticsConfigurationResponse             GET_VIDEOANALYTICSCONFIGRESPONSE_t;
typedef struct _trt__SetVideoAnalyticsConfiguration                     SET_VIDEOANALYTICSCONFIG_t;
typedef struct _trt__SetVideoAnalyticsConfigurationResponse             SET_VIDEOANALYTICSCONFIGRESPONSE_t;
typedef struct _trt__GetCompatibleAudioSourceConfigurations             GET_COMPATIBLE_AUDIO_SRC_CONFIG_t;
typedef struct _trt__GetCompatibleAudioSourceConfigurationsResponse     GET_COMPATIBLE_AUDIO_SRC_CONFIG_RESPONSE_t;
typedef struct _trt__GetVideoSourceConfiguration                        GET_VIDEO_SOURCE_CONFIGURATION_t;
typedef struct _trt__GetVideoSourceConfigurationResponse  				GET_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t;
typedef struct _trt__GetVideoSourceConfigurationOptions                 GET_VIDEO_SOURCE_CONFIGURATION_OPTION_t;
typedef struct _trt__GetVideoSourceConfigurationOptionsResponse         GET_VIDEO_SOURCE_CONFIGURATION_OPTION_RES_t;
typedef struct _trt__SetVideoSourceConfiguration                        SET_VIDEO_SRC_CONFIG_t;
typedef struct _trt__SetVideoSourceConfigurationResponse                SET_VIDEO_SRC_CONFIG_RES_t;

#define GetProfilesCommand												soap_call___trt__GetProfiles
#define GetVideoEncoderConfiguratonOptionsCommand						soap_call___trt__GetVideoEncoderConfigurationOptions
#define GetProfileCommand												soap_call___trt__GetProfile
#define GetCompatibleVideoSourceConfigurationsCommand					soap_call___trt__GetCompatibleVideoSourceConfigurations
#define GetCompatibleVideoEncoderConfigurationsCommand					soap_call___trt__GetCompatibleVideoEncoderConfigurations
#define GetAudioSourcesCommand											soap_call___trt__GetAudioSources
#define GetAudioEncoderConfigurationsCommand							soap_call___trt__GetAudioEncoderConfigurations
#define GetAudioSourceConfigurationsCommand								soap_call___trt__GetAudioSourceConfigurations
#define GetVideoSourceConfigurationsCommand				   				soap_call___trt__GetVideoSourceConfigurations
#define AddVideoSourceConfigurationCommand				 				soap_call___trt__AddVideoSourceConfiguration
#define SetVideoEncoderConfigurationCommand				   				soap_call___trt__SetVideoEncoderConfiguration
#define GetVideoEncoderConfigurationCommand				   				soap_call___trt__GetVideoEncoderConfiguration
#define AddVideoEncoderConfigurationCommand								soap_call___trt__AddVideoEncoderConfiguration
#define AddAudioEncoderConfigurationCommand								soap_call___trt__AddAudioEncoderConfiguration
#define AddAudioSourceConfigurationCommand								soap_call___trt__AddAudioSourceConfiguration
#define RemoveAudioEncoderConfigurationCommand							soap_call___trt__RemoveAudioEncoderConfiguration
#define RemoveAudioSourceConfigurationCommand							soap_call___trt__RemoveAudioSourceConfiguration
#define GetStreamUriCommand												soap_call___trt__GetStreamUri
#define GetSnapshotUriCommand											soap_call___trt__GetSnapshotUri
#define GetOsdsCommand                                                  soap_call___trt__GetOSDs
#define SetOsdCommand                                                   soap_call___trt__SetOSD
#define CreateOsdCommand                                                soap_call___trt__CreateOSD
#define DeleteOsdCommand                                                soap_call___trt__DeleteOSD
#define GetVideoAnalticsConfigsCommand                                  soap_call___trt__GetVideoAnalyticsConfigurations
#define GetVideoAnalticsConfigCommand                                   soap_call___trt__GetVideoAnalyticsConfiguration
#define SetVideoAnalticsConfigCommand                                   soap_call___trt__SetVideoAnalyticsConfiguration
#define GetCompatibleAudioSrcConfigCmd                                  soap_call___trt__GetCompatibleAudioSourceConfigurations
#define GetVideoSourceConfigurationCommand				   				soap_call___trt__GetVideoSourceConfiguration
#define GetVideoSourceConfigurationOptionsCommand                       soap_call___trt__GetVideoSourceConfigurationOptions
#define SetVideoSourceConfigurationCommand                              soap_call___trt__SetVideoSourceConfiguration

/* Namespace tr2 = http://www.onvif.org/ver20/media/wsdl */
typedef struct _tr2__GetProfiles 										GET_PROFILES2_t;
typedef struct _tr2__GetProfilesResponse 								GET_PROFILES_RESPONSE2_t;
typedef struct _tr2__GetServiceCapabilities                             GET_CAPABILITIES2_t;
typedef struct _tr2__GetServiceCapabilitiesResponse                     GET_CAPABILITIES_RESPONSE2_t;
typedef struct  tr2__GetConfiguration                                   GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS2_t;
typedef struct _tr2__GetVideoEncoderConfigurationOptionsResponse 		GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE2_t;
typedef struct _tr2__SetVideoEncoderConfiguration		   				SET_VIDEO_ENCODER_CONFIGURATION2_t;
typedef struct  tr2__SetConfigurationResponse                           SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t;
typedef struct  tr2__GetConfiguration                                   GET_VIDEO_ENCODER_CONFIGURATION2_t;
typedef struct _tr2__GetVideoEncoderConfigurationsResponse              GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t;
typedef struct _tr2__GetStreamUri										GET_MEDIA2_STREAM_URI_t;
typedef struct _tr2__GetStreamUriResponse								GET_MEDIA2_STREAM_URI_RESPONSE_t;
typedef struct _tr2__GetSnapshotUri                                     GET_SNAPSHOT_URI2_t;
typedef struct _tr2__GetSnapshotUriResponse                             GET_SNAPSHOT_URI_RESPONSE2_t;
typedef struct _tr2__GetOSDs                                            GET_OSDS2_t;
typedef struct _tr2__GetOSDsResponse                                    GET_OSDSRESPONSE2_t;
typedef struct _tr2__GetOSDOptions                                      GET_OSDO_OPTIONS2_t;
typedef struct _tr2__GetOSDOptionsResponse                              GET_OSD_OPT_RES2_t;
typedef struct _tr2__SetOSD                                             SET_OSD2_t;
typedef struct tr2__SetConfigurationResponse                            SET_OSDRESPONSE2_t;
typedef struct _tr2__CreateOSD                                          CREATE_OSD2_t;
typedef struct _tr2__CreateOSDResponse                                  CREATE_OSDRESPONSE2_t;
typedef struct _tr2__DeleteOSD                                          DELETE_OSD2_t;
typedef struct tr2__SetConfigurationResponse                            DELETE_OSDRESPONSE2_t;

#define GetProfilesCommand2												soap_call___tr2__GetProfiles
#define GetVideoEncoderConfiguratonOptionsCommand2						soap_call___tr2__GetVideoEncoderConfigurationOptions
#define SetVideoEncoderConfigurationCommand2				   			soap_call___tr2__SetVideoEncoderConfiguration
#define GetVideoEncoderConfiguration2Command                            soap_call___tr2__GetVideoEncoderConfigurations
#define GetMedia2StreamUriCommand										soap_call___tr2__GetStreamUri
#define GetSnapshotUriCommand2											soap_call___tr2__GetSnapshotUri
#define GetOsdsCommand2                                                 soap_call___tr2__GetOSDs
#define GetOsdsOptionsCommand2                                          soap_call___tr2__GetOSDOptions
#define SetOsdCommand2                                                  soap_call___tr2__SetOSD
#define CreateOsdCommand2                                               soap_call___tr2__CreateOSD
#define DeleteOsdCommand2                                               soap_call___tr2__DeleteOSD

/* Namespace ns1 = http://www.onvif.org/ver20/analytics/wsdl */
typedef struct _ns1__GetAnalyticsModules                                GET_ANALYTICS_MODULES_t;
typedef struct _ns1__GetAnalyticsModulesResponse                        GET_ANALYTICS_MODULES_RESPONSE_t;
typedef struct _ns1__ModifyAnalyticsModules                             MODIFY_ANALYTICS_MODULES_t;
typedef struct _ns1__ModifyAnalyticsModulesResponse                     MODIFY_ANALYTICS_MODULES_RESPONSE_t;
typedef struct _ns1__GetRules                                           GET_RULES_t;
typedef struct _ns1__GetRulesResponse                                   GET_RULES_RESPONSE_t;
typedef struct _ns1__ModifyRules                                        MODIFY_RULES_t;
typedef struct _ns1__ModifyRulesResponse                                MODIFY_RULES_RESPONSE_t;

#define GetAnalyticsModulesCommand                                      soap_call___ns1__GetAnalyticsModules
#define ModifyAnalyticsModulesCommand                                   soap_call___ns1__ModifyAnalyticsModules
#define GetRulesCommand                                                 soap_call___ns1__GetRules
#define ModifyRulesCommand                                              soap_call___ns1__ModifyRules

/* Namespace tptz = http://www.onvif.org/ver20/ptz/wsdl */
typedef struct _tptz__GetConfigurations  								GET_CONFIGURATIONS_t;
typedef struct _tptz__GetConfigurationsResponse 						GET_CONFIGURATIONS_RESPONSE_t;
typedef struct _tptz__GetConfigurationOptions  		  					GET_CONFIGURATION_OPTIONS_t;
typedef struct _tptz__GetConfigurationOptionsResponse  					GET_CONFIGURATION_OPTIONS_RESPONSE_t;
typedef struct _tptz__RelativeMove                                      RELATIVE_MOVE_t;
typedef struct _tptz__RelativeMoveResponse                              RELATIVE_MOVE_RESPONSE_t;
typedef struct _tptz__AbsoluteMove                                      ABSOLUTE_MOVE_t;
typedef struct _tptz__AbsoluteMoveResponse                              ABSOLUTE_MOVE_RESPONSE_t;
typedef struct _tptz__GetStatus                                         GET_PTZ_STATUS_t;
typedef struct _tptz__GetStatusResponse                                 GET_PTZ_STATUS_RESPONSE_t;
typedef struct _tptz__ContinuousMove                                    CONTINUOUS_MOVE_t;
typedef struct _tptz__ContinuousMoveResponse                            CONTINUOUS_MOVE_RESPONSE_t;
typedef struct _tptz__Stop                                              STOP_PTZ_t;
typedef struct _tptz__StopResponse                                      STOP_PTZ_RESPONSE_t;
typedef struct _tptz__GotoPreset                                        GOTO_PRESET_t;
typedef struct _tptz__GotoPresetResponse                                GOTO_PRESET_RESPONSE_t;
typedef struct _tptz__RemovePreset                                      REMOVE_PRESET_t;
typedef struct _tptz__RemovePresetResponse                              REMOVE_PRESET_RESPONSE_t;
typedef struct _tptz__SetPreset                                         SET_PRESET_t;
typedef struct _tptz__SetPresetResponse                                 SET_PRESET_RESPONSE_t;
typedef struct _tptz__GetNodes                                          GET_NODES_t;
typedef struct _tptz__GetNodesResponse                                  GET_NODES_RESPONSE_t;
typedef struct _trt__AddPTZConfiguration                                ADD_PTZ_CONFIG_t;
typedef struct _trt__AddPTZConfigurationResponse                        ADD_PTZ_CONFIG_RESP_t;
typedef struct _tptz__GotoHomePosition                                  GOTO_HOME_POS_t;
typedef struct _tptz__GotoHomePositionResponse                          GOTO_HOME_POS_RESP_t;
typedef struct _tptz__SetHomePosition                                   SET_HOME_POS_t;
typedef struct _tptz__SetHomePositionResponse                           SET_HOME_POS_RESP_t;

#define GetConfigurationsCommand										soap_call___tptz__GetConfigurations
#define GetConfigurationOptionsCommand				  					soap_call___tptz__GetConfigurationOptions
#define	soap_call_RelativeMove                                          soap_call___tptz__RelativeMove
#define	soap_call_AbsoluteMove                                          soap_call___tptz__AbsoluteMove
#define soap_call_GetPtzStatus                                          soap_call___tptz__GetStatus
#define	soap_call_ContinuousMove                                        soap_call___tptz__ContinuousMove
#define	soap_call_StopPtz                                               soap_call___tptz__Stop
#define	soap_call_GotoPreset                                            soap_call___tptz__GotoPreset
#define soap_call_RemovePreset                                          soap_call___tptz__RemovePreset
#define soap_call_SetPreset                                             soap_call___tptz__SetPreset
#define GetNodesCommand                                                 soap_call___tptz__GetNodes
#define AddPtzConfigurationCommand                                      soap_call___trt__AddPTZConfiguration
#define GotoHomePositionCommand                                         soap_call___tptz__GotoHomePosition
#define SetHomePostitionCommand                                         soap_call___tptz__SetHomePosition

/* Namespace timg = http://www.onvif.org/ver20/imaging/wsdl */
typedef struct _timg__GetMoveOptions  									GET_MOVE_OPTIONS_t;
typedef struct _timg__GetMoveOptionsResponse 							GET_MOVE_OPTIONS_RESPONSE_t;
typedef struct _timg__GetOptions  										GET_OPTIONS_t;
typedef struct _timg__GetOptionsResponse 								GET_OPTIONS_RESPONSE_t;
typedef struct _timg__GetImagingSettings								GET_IMAGING_SETTINGS_t;
typedef struct _timg__GetImagingSettingsResponse						GET_IMAGING_SETTINGS_RESPONSE_t;
typedef struct _timg__SetImagingSettings                                SET_IMAGING_SETTINGS_t;
typedef struct _timg__SetImagingSettingsResponse						SET_IMAGING_SETTINGS_RESPONSE_t;
typedef	struct _timg__Move                                              MOVE_t;
typedef	struct _timg__MoveResponse                                      MOVE_RESPONSE_t;
typedef struct _timg__GetStatus                                         GET_FOCUS_STATUS_t;
typedef struct _timg__GetStatusResponse                                 GET_FOCUS_STATUS_RESPONSE_t;
typedef struct _timg__Stop                                              STOP_FOCUS_t;
typedef struct _timg__StopResponse                                      STOP_FOCUS_RESPONSE_t;

#define GetMoveOptionsCommand											soap_call___timg__GetMoveOptions
#define GetOptionsCommand												soap_call___timg__GetOptions
#define GetImagingSettingsCommand										soap_call___timg__GetImagingSettings
#define SetImagingSettingsCommand										soap_call___timg__SetImagingSettings
#define soap_call_Move                                                  soap_call___timg__Move
#define soap_call_GetFocusStatus                                        soap_call___timg__GetStatus
#define	soap_call_StopFocus                                             soap_call___timg__Stop

/* Namespace tev =http://www.onvif.org/ver10/events/wsdl */
typedef struct _tev__CreatePullPointSubscription                        CREATE_PULL_PT_SUBSCRIPTION_t;
typedef struct _tev__CreatePullPointSubscriptionResponse                CREATE_PULL_PT_SUBSCRIPTION_RESPONSE_t;
typedef struct _tev__GetEventProperties                                 GET_EVENT_PROPERTIES_t;
typedef struct _tev__GetEventPropertiesResponse                         GET_EVENT_PROPERTIES_RESPONSE_t;
typedef struct _tev__SetSynchronizationPoint                            SET_SYNC_PT_t;
typedef struct _tev__SetSynchronizationPointResponse                    SET_SYNC_PT_RESPONSE_t;
typedef struct _tev__PullMessages                                       PULL_MESSAGES_t;
typedef struct _tev__PullMessagesResponse                               PULL_MESSAGES_RESPONSE_t;
typedef struct _wsnt__Renew                                             RENEW_t;
typedef struct _wsnt__RenewResponse                                     RENEW_RESPONSE_t;
typedef struct _wsnt__Unsubscribe                                       UNSUBSCRIBE_t;
typedef struct _wsnt__UnsubscribeResponse                               UNSUBSCRIBE_RESPONSE_t;

#define CreatePullPtSubscriptionCommand                                 soap_call___tev__CreatePullPointSubscription
#define GetEventPropertiesCommand                                       soap_call___tev__GetEventProperties
#define SetSynchronizePtCommand                                         soap_call___tev__SetSynchronizationPoint
#define PullMessagesCommand                                             soap_call___tev__PullMessages
#define RenewCommand                                                    soap_call___tev__Renew
#define UnsubscribeCommand                                              soap_call___tev__Unsubscribe

/* Namespace tt = http://www.onvif.org/ver10/schema */
typedef struct tt__VideoEncoderConfiguration							VIDEO_ENCODER_CONFIGURATION_t;
typedef struct tt__VideoEncoder2Configuration							VIDEO_ENCODER2_CONFIGURATION_t;
typedef struct tt__VideoResolution										VIDEO_RESOLUTION_t;
typedef struct tt__VideoResolution2                                     VIDEO_RESOLUTION2_t;
typedef struct tt__StreamSetup											STREAM_SETUP_t;
typedef struct tt__Transport	 										MEDIA_TRANSPORT_t;
typedef struct tt__VideoRateControl                                     VIDEO_RATE_CONTROL_t;
typedef struct tt__VideoRateControl2                                    VIDEO_RATE_CONTROL2_t;
typedef struct tt__Mpeg4Configuration									MPEG4_CONFIGURATION_t;
typedef struct tt__H264Configuration									H264_CONFIGURATION_t;
typedef struct tt__IntRange                                             FRAME_RATE_RANGE_t;
typedef enum   tt__Mpeg4Profile											MPEG4_PROFILE_e;
typedef enum   tt__H264Profile											H264_PROFILE_e;
typedef	struct tt__FocusMove                                            FOCUS_MOVE_t;
typedef	struct tt__AbsoluteFocus                                        ABSOLUTE_FOCUS_t;
typedef	struct tt__RelativeFocus                                        RELATIVE_FOCUS_t;
typedef	struct tt__ContinuousFocus                                      CONTINUOUS_FOCUS_t;
typedef struct tt__PTZVector                                            PTZ_VECTOR_t;
typedef struct tt__PTZSpeed                                             PTZ_SPEED_t;
typedef struct tt__Vector1D                                             VECTOR1D_t;
typedef struct tt__Vector2D                                             VECTOR2D_t;
typedef enum   tt__CapabilityCategory        							CAPABILITY_CATEGORY_e;
typedef struct tt__FocusConfiguration20                                 FOCUS_CONFIGURATION20_t;
typedef struct tt__ImagingSettings20                                    IMAGING_SETTINGS20_t;
typedef struct tt__ImagingOptions20                                     IMAGING_OPTIONS20_t;
typedef	struct tt__VideoAnalyticsConfiguration                          VIDEO_ANALYTICS_CONFIGURATION_t;
typedef	struct _tt__ItemList_SimpleItem                                 ITEMLIST_SIMPLEITEM_t;
typedef struct _tt__ItemList_ElementItem                                ITEMLIST_ELEMENTITEM_t;
typedef struct tt__ItemList                                             ITEMLIST_t;
typedef struct tt__Config                                               CONFIG_t;
typedef struct tt__AnalyticsEngineConfiguration                         ANALYTICSENGINECONFIG_t;
typedef	struct tt__RuleEngineConfiguration                              RULEENGINECONFIG_t;
typedef struct tt__NetworkInterfaceSetConfiguration                     NETWORK_INTERFACE_SET_CONFIG_t;
typedef struct tt__IPv4NetworkInterfaceSetConfiguration                 IPv4_NETWORK_INTERFACE_SET_CONFIG_t;
typedef struct tt__IPv6NetworkInterfaceSetConfiguration                 IPv6_NETWORK_INTERFACE_SET_CONFIG_t;
typedef struct tt__PrefixedIPv4Address                                  PREFIXED_IPv4_ADDR_t;
typedef struct tt__PrefixedIPv6Address                                  PREFIXED_IPv6_ADDR_t;
typedef enum   tt__VideoEncodingProfiles                                VIDEO_ENCODING_PROFILES_e;
typedef enum   tt__VideoEncodingMimeNames                               VIDEO_ENCODING_NAMES_e;

#define ONVIF_AUTO_FOCUS_MODE                                           tt__AutoFocusMode__AUTO
#define	H264_HIGH_PROFILE												tt__H264Profile__High
#define MPEG4_ASP_PROFILE												tt__Mpeg4Profile__ASP
#define VIDEO_ENCODING_JPEG	  											tt__VideoEncoding__JPEG
#define VIDEO_ENCODING_MPEG4  											tt__VideoEncoding__MPEG4
#define VIDEO_ENCODING_H264	  											tt__VideoEncoding__H264
#define STREAM_TYPE_RTP_UNICAST											tt__StreamType__RTP_Unicast
#define TRANSPORT_PROTOCOL_INTERLEAVED									tt__TransportProtocol__RTSP
#define TRANSPORT_PROTOCOL_UDP											tt__TransportProtocol__UDP
#define TRANSPORT_PROTOCOL_HTTP											tt__TransportProtocol__HTTP
#define ALARM_INACTIVE													tt__RelayLogicalState__inactive
#define ALARM_ACTIVE													tt__RelayLogicalState__active

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
INT16 GetSystemDateAndTime(SOAP_t *soap, GET_SYSTEM_DATE_AND_TIME_t *getSystemDateAndTime_t,
                           GET_SYSTEM_DATE_AND_TIME_RESPONSE_t *getSystemDateAndTimeResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetSystemDateAndTime(SOAP_t *soap, SET_SYSTEM_DATE_AND_TIME_t *setSystemDateAndTime_t,
                           SET_SYSTEM_DATE_AND_TIME_RESPONSE_t *setSystemDateAndTimeResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetDeviceInformation(SOAP_t *soap, GET_DEVICE_INFORMATION_t *getDeviceInformation_t,
                           GET_DEVICE_INFORMATION_RESPONSE_t *getDeviceInformationResponse_t, SOAP_USER_DETAIL_t *User, BOOL *pIsAuthFail);
//-------------------------------------------------------------------------------------------------
INT16 GetCapabilities(SOAP_t *soap, GET_CAPABILITIES_t *getCapabilities_t,
                      GET_CAPABILITIES_RESPONSE_t *getCapabilitiesResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetCapabilities2(SOAP_t *soap, GET_CAPABILITIES2_t *getCapabilities_t,
                       GET_CAPABILITIES_RESPONSE2_t *getCapabilitiesResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetServices(SOAP_t *soap,GET_SERVICES_t *getServices, GET_SERVICES_RESPONSE_t *getServicesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetProfiles(SOAP_t *soap, GET_PROFILES_t *getProfiles_t,
                  GET_PROFILES_RESPONSE_t *getProfilesResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetProfiles2(SOAP_t *soap, GET_PROFILES2_t *getProfiles_t,
                   GET_PROFILES_RESPONSE2_t *getProfilesResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetProfile(SOAP_t *soap, GET_PROFILE_t *getProfile_t,
                 GET_PROFILE_RESPONSE_t *getProfileResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoEncoderConfigurationOptions(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_t *getVideoEncoderConfigurationOptions,
                                          GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t *getVideoEncoderConfigurationOptionsResponse,
                                          SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoEncoderConfigurationOptions2(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS2_t *getVideoEncoderConfigurationOptions,
                                           GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE2_t *getVideoEncoderConfigurationOptionsResponse,
                                           SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetCompatibleVideoSourceConfigurations(SOAP_t *soap, GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_t *getCompatibleVideoSourceConfigurations,
                                             GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t *getCompatibleVideoSourceConfigurationsResponse,
                                             SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetCompatibleVideoEncoderConfigurations(SOAP_t *soap, GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_t *getCompatibleVideoEncoderConfigurations,
                                              GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_RESPONSE_t *getCompatibleVideoEncoderConfigurationsResponse,
                                              SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetAudioSources(SOAP_t *soap, GET_AUDIO_SOURCES_t *getAudioSources, GET_AUDIO_SOURCES_RESPONSE_t *getAudioSourcesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetAudioSourceConfigurations(SOAP_t *soap, GET_AUDIO_SOURCE_CONFIGURATIONS_t *getAudioSourceConfigurations,
                                   GET_AUDIO_SOURCE_CONFIGURATIONS_RESPONSE_t *getAudioSourceConfigurationsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetAudioEncoderConfigurations(SOAP_t *soap, GET_AUDIO_ENCODER_CONFIGURATIONS_t *getAudioEncoderConfigurations,
                                    GET_AUDIO_ENCODER_CONFIGURATIONS_RESPONSE_t *getAudioEncoderConfigurationsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetConfigurations(SOAP_t *soap, GET_CONFIGURATIONS_t *getConfigurations, GET_CONFIGURATIONS_RESPONSE_t *getConfigurationsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetConfigurationOptions(SOAP_t *soap, GET_CONFIGURATION_OPTIONS_t *getConfigurationOptions,
                              GET_CONFIGURATION_OPTIONS_RESPONSE_t *getConfigurationOptionsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetRelayOutputs(SOAP_t *soap, GET_RELAY_OUTPUTS_t *getRelayOutputs, GET_RELAY_OUTPUTS_RESPONSE_t *getRelayOutputsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetMoveOptions(SOAP_t *soap, GET_MOVE_OPTIONS_t *getMoveOptions, GET_MOVE_OPTIONS_RESPONSE_t *getMoveOptionsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetOptions(SOAP_t *soap,GET_OPTIONS_t *getOptions, GET_OPTIONS_RESPONSE_t *getOptionsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoSourceConfigurations(SOAP_t *soap, GET_VIDEO_SOURCE_CONFIGURATIONS_t *getVideoSourceConfigurations,
                                   GET_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t *getVideoSourceConfigurationsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 AddVideoSourceConfiguration(SOAP_t *soap, ADD_VIDEO_SOURCE_CONFIGURATION_t *addVideoSourceConfiguration,
                                  ADD_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t *addVideoSourceConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetVideoEncoderConfiguration(SOAP_t *soap, SET_VIDEO_ENCODER_CONFIGURATION_t *setVideoEncoderConfiguration,
                                   SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t *setVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetVideoEncoderConfiguration2(SOAP_t *soap, SET_VIDEO_ENCODER_CONFIGURATION2_t *setVideoEncoderConfiguration,
                                    SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t *setVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoEncoderConfiguration(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION_t *getVideoEncoderConfiguration,
                                   GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t *getVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoEncoderConfiguration2(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION2_t *getVideoEncoderConfiguration,
                                    GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t *getVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 AddVideoEncoderConfiguration(SOAP_t *soap, ADD_VIDEO_ENCODER_CONFIGURATION_t *addVideoEncoderConfiguration,
                                   ADD_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t *addVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 AddAudioEncoderConfiguration(SOAP_t *soap, ADD_AUDIO_ENCODER_CONFIGURATION_t *addAudioEncoderConfiguration,
                                   ADD_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t *addAudioEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 AddAudioSourceConfiguration(SOAP_t *soap, ADD_AUDIO_SOURCE_CONFIGURATION_t *addAudioSourceConfiguration,
                                  ADD_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t *addAudioSourceConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 RemoveAudioEncoderConfiguration(SOAP_t *soap, REMOVE_AUDIO_ENCODER_CONFIGURATION_t *removeAudioEncoderConfiguration,
                                      REMOVE_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t *removeAudioEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 RemoveAudioSourceConfiguration(SOAP_t *soap, REMOVE_AUDIO_SOURCE_CONFIGURATION_t *removeAudioSourceConfiguration,
                                     REMOVE_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t *removeAudioSourceConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetStreamUri(SOAP_t *soap, GET_STREAM_URI_t *getStreamUri,  GET_STREAM_URI_RESPONSE_t *getStreamUriResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetMedia2StreamUri(SOAP_t *soap, GET_MEDIA2_STREAM_URI_t *getStreamUri, GET_MEDIA2_STREAM_URI_RESPONSE_t *getStreamUriResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetSnapshotUri(SOAP_t *soap, GET_SNAPSHOT_URI_t *getSnapshotUri, GET_SNAPSHOT_URI_RESPONSE_t *getSnapshotUriResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetSnapshotUri2(SOAP_t *soap, GET_SNAPSHOT_URI2_t *getSnapshotUri, GET_SNAPSHOT_URI_RESPONSE2_t *getSnapshotUriResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetRelayOutputState(SOAP_t *soap, SET_RELAY_OUTPUT_STATE_t *setRelayOutputState,
                          SET_RELAY_OUTPUT_STATE_RESPONSE_t *setRelayOutputStateResponse,  SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetImagingSettings(SOAP_t *soap, struct _timg__GetImagingSettings *getImagingSettings,
                         struct _timg__GetImagingSettingsResponse *getImagingSettingsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetImagingSettings(SOAP_t *soap, SET_IMAGING_SETTINGS_t *setImagingSettings,
                         SET_IMAGING_SETTINGS_RESPONSE_t *setImagingSettingsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 RelativeMove(SOAP_t *soap, RELATIVE_MOVE_t *relativeMoveParam, RELATIVE_MOVE_RESPONSE_t *relativeMoveRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetPTZStatus(SOAP_t *soap, GET_PTZ_STATUS_t *getStatusParam, GET_PTZ_STATUS_RESPONSE_t *currStatus, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 AbsoluteMove(SOAP_t *soap, ABSOLUTE_MOVE_t *absoluteMoveParam, ABSOLUTE_MOVE_RESPONSE_t *absoluteMoveRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 ContinuousMove(SOAP_t *soap, CONTINUOUS_MOVE_t *continuousMoveParam, CONTINUOUS_MOVE_RESPONSE_t *continuousMoveRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 StopPTZ(SOAP_t *soap, STOP_PTZ_t *stopParam, STOP_PTZ_RESPONSE_t *stopRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GoToPreset(SOAP_t *soap, GOTO_PRESET_t *gotoPresetParam, GOTO_PRESET_RESPONSE_t *gotoPresetRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 RemovePreset(SOAP_t *soap, REMOVE_PRESET_t *removePresetParam, REMOVE_PRESET_RESPONSE_t *removePresetRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetPreset(SOAP_t *soap, SET_PRESET_t *setPresetParam, SET_PRESET_RESPONSE_t *setPresetRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetFocusStatus(SOAP_t *soap, GET_FOCUS_STATUS_t *getFocusStatusParam, GET_FOCUS_STATUS_RESPONSE_t	*currFocusStatus, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 FocusMove(SOAP_t *soap, MOVE_t *moveParam, MOVE_RESPONSE_t *moveRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 StopFocus(SOAP_t *soap, STOP_FOCUS_t *stopFocusParam, STOP_FOCUS_RESPONSE_t *stopFocusRespParam, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 CreatePullPtSubscription(SOAP_t *soap, CREATE_PULL_PT_SUBSCRIPTION_t *createPullPtSub,
                               CREATE_PULL_PT_SUBSCRIPTION_RESPONSE_t *createPullPtSubResp, SOAP_USER_DETAIL_t *User , SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
INT16 GetEventProperties(SOAP_t *soap, GET_EVENT_PROPERTIES_t *getEvProperties,
                         GET_EVENT_PROPERTIES_RESPONSE_t *getEvPropertiesResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
INT16 SetSynchronizePt(SOAP_t *soap, SET_SYNC_PT_t *setSyncPt, SET_SYNC_PT_RESPONSE_t *setSyncPtRef, SOAP_USER_DETAIL_t *User,SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
INT16 PullMessages(SOAP_t *soap, PULL_MESSAGES_t *pullMessages, PULL_MESSAGES_RESPONSE_t *pullMessagesResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
INT16 Renew(SOAP_t *soap, RENEW_t *renew, RENEW_RESPONSE_t *renewResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
INT16 UnSubscribe(SOAP_t *soap, UNSUBSCRIBE_t *unSubscribe, UNSUBSCRIBE_RESPONSE_t *unSubscribeResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
INT16 Getosds(SOAP_t *soap, GET_OSDS_t *getOsds, GET_OSDSRESPONSE_t *getOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 Getosds2(SOAP_t *soap, GET_OSDS2_t *getOsds, GET_OSDSRESPONSE2_t *getOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetOsd(SOAP_t *soap, SET_OSD_t *setOsds, SET_OSDRESPONSE_t *setOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetOsd2(SOAP_t *soap, SET_OSD2_t *setOsds, SET_OSDRESPONSE2_t *setOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 CreateOsd(SOAP_t *soap, CREATE_OSD_t *createOsds, CREATE_OSDRESPONSE_t *createOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 CreateOsd2(SOAP_t *soap, CREATE_OSD2_t *createOsds, CREATE_OSDRESPONSE2_t *createOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 DeleteOsd(SOAP_t *soap, DELETE_OSD_t *deleteOsds, DELETE_OSDRESPONSE_t *deleteOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 DeleteOsd2(SOAP_t *soap, DELETE_OSD2_t *deleteOsds, DELETE_OSDRESPONSE2_t *deleteOSDsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoAnaltics(SOAP_t *soap, GET_VIDEOANALYTICSCONFIGS_t *videoAnalytics,
                       GET_VIDEOANALYTICSCONFIGSRESPONSE_t *videoAnalyticsResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoAnaltic(SOAP_t *soap, GET_VIDEOANALYTICSCONFIG_t *videoAnalytic,
                      GET_VIDEOANALYTICSCONFIGRESPONSE_t *videoAnalyticResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetVideoAnaltic(SOAP_t *soap, SET_VIDEOANALYTICSCONFIG_t *videoAnalytic,
                      SET_VIDEOANALYTICSCONFIGRESPONSE_t *videoAnalyticResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetNetworkInterfaces(SOAP_t *soap, GET_NETWORK_INTERFACES_t *getNetworkInterfaces,
                           GET_NETWORK_INTERFACESRESPONSE_t *getNetworkInterfacesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetNetworkInterfaces(SOAP_t *soap, SET_NETWORK_INTERFACES_t *setNetworkInterfaces,
                           SET_NETWORK_INTERFACESRESPONSE_t *setNetworkInterfacesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetNetworkDfltGateway(SOAP_t *soap, SET_NETWORK_DFLT_GATEWAY_t *setNetworkDfltGateway,
                            SET_NETWORK_DFLT_GATEWAYRESPONSE_t *setNetworkDfltGatewayResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SystemReboot(SOAP_t *soap, SYSTEM_REBOOT_t *systemReboot, SYSTEM_REBOOTRESPONSE_t *systemRebootResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 setUser(SOAP_t *soap, SET_USER_INFORMATION_t *setUserPasswd_t , SET_USER_INFORMATION_RESPONSE_t *setUserPasswdResponse_t, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetNodes(SOAP_t *soap, GET_NODES_t *getNodes, GET_NODES_RESPONSE_t *getNodesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 AddPtzConfiguration(SOAP_t *soap, ADD_PTZ_CONFIG_t *addPtzConfig, ADD_PTZ_CONFIG_RESP_t *addPtzConfigResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GotoHomePosition(SOAP_t *soap, GOTO_HOME_POS_t *gotoHomePos, GOTO_HOME_POS_RESP_t *gotoHomePosRes, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetHomePosition(SOAP_t *soap, SET_HOME_POS_t *setHomePos, SET_HOME_POS_RESP_t *setHomePosRes, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetNetworkDfltGateway(SOAP_t *soap, GET_NET_DEFAULT_GATEWAY_t *getNetworkDfltGateway,
                            GET_NET_DEFAULT_GATEWAY_RES_t *getNetworkDfltGatewayResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetUser(SOAP_t *soap, GET_USER_t *getUser, GET_USER_RES_t *getUserResp, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 CreateUser(SOAP_t *soap, CREATE_USER_t *createUser, CREATE_USER_RES_t *createUserResp, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 DeleteUsers(SOAP_t *soap, DELETE_USER_t *deleteUser, DELETE_USER_RES_t *deleteUserResp, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetCompatibleAudioSourceConfigurations(SOAP_t *soap, GET_COMPATIBLE_AUDIO_SRC_CONFIG_t *getCompatibleAudSrcConfig,
                                             GET_COMPATIBLE_AUDIO_SRC_CONFIG_RESPONSE_t *getCompatibleAudSrcConfigResp, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetRelayOutputSetting(SOAP_t *soap, SET_RELAY_OUT_SETTING_t *setRelayOutputSetting,
                            SET_RELAY_OUT_SETTING_RES_t *setRelayOutputSettingRes, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoSourceConfiguration(SOAP_t *soap, GET_VIDEO_SOURCE_CONFIGURATION_t *getVideoSourceConfiguration,
                                  GET_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t *getVideoSourceConfigurationResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetVideoSourceConfigurationOption(SOAP_t *soap, GET_VIDEO_SOURCE_CONFIGURATION_OPTION_t *input,
                                        GET_VIDEO_SOURCE_CONFIGURATION_OPTION_RES_t *response, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 SetVideoSourceConfiguration(SOAP_t *soap, SET_VIDEO_SRC_CONFIG_t *input, SET_VIDEO_SRC_CONFIG_RES_t *response, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetAnalyticsModules(SOAP_t *soap, GET_ANALYTICS_MODULES_t *getAnalyticsModule,
                          GET_ANALYTICS_MODULES_RESPONSE_t *getAnalyticsModuleResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 ModifyAnalyticsModules(SOAP_t *soap, MODIFY_ANALYTICS_MODULES_t *modifyAnalyticsModule,
                             MODIFY_ANALYTICS_MODULES_RESPONSE_t *modifyAnalyticsModuleResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 GetRules(SOAP_t *soap, GET_RULES_t *getRules, GET_RULES_RESPONSE_t *getRulesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
INT16 ModifyRules(SOAP_t *soap, MODIFY_RULES_t *modifyRules, MODIFY_RULES_RESPONSE_t *modifyRulesResponse, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* ONVIFCOMMAND_H_ */
