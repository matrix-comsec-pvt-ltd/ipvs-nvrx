//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		OnvifCommand.c
@brief      This file provides interface between ONVIF stack (soap library) and onvif client module
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "OnvifCommand.h"
#include "DebugLog.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_SOAP_RECV_TIME_OUT 		30
#define INVALID_CREDENTIAL_STR      "NotAuthorized"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void fillWsa5InfoToHeader(SOAP_t *soap, SOAP_WSA5_t *wsa5Info);
//-------------------------------------------------------------------------------------------------
static void soapInitialize(SOAP_t *soap, SOAP_USER_DETAIL_t *User);
//-------------------------------------------------------------------------------------------------
static void printSoapError(SOAP_t *soap, const CHAR *function, INT32 retval, CHARPTR ip);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief printSoapError : Print Detailed info of error
 * @param soap           : SOAP Instance
 * @param function       : command name
 * @param retval         : soap code
 * @param ip             : Camera IP
 * @return
 */
static void printSoapError(SOAP_t *soap, const CHAR *function, INT32 retval, CHARPTR ip)
{
    /* Exclude http error and soap success status */
    if ((retval == SOAP_OK) || (retval == 401))
    {
        return;
    }

    if ((soap == NULL) || (soap->fault == NULL))
    {
        //DPRINT(ONVIF_CLIENT, "[ip=%s], [cmd=%s], [err=%d]", ip, function, retval);
        return;
    }

    CHAR debugStr[DEBUG_PRINT_STR_LEN];
    INT32 debugLen = snprintf(debugStr, DEBUG_PRINT_STR_LEN, "[cmd=%s], [ip=%s], [err=%d]", function, ip, retval);

    if (soap->fault->faultstring != NULL)
    {
        debugLen = snprintf(debugStr + debugLen, DEBUG_PRINT_STR_LEN - debugLen, ", [fault=%s]", soap->fault->faultstring);
    }

    if ((soap->fault->SOAP_ENV__Reason != NULL) && (soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text != NULL))
    {
        snprintf(debugStr + debugLen, DEBUG_PRINT_STR_LEN - debugLen, ", [reason=%s]", soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text);
    }

    EPRINT(ONVIF_CLIENT, "onvif cmd status: %s", debugStr);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief fillWsa5InfoToHeader  : Will generate SOAP header with wsa5 fields
 * @param soap                  : SOAP instance
 * @param wsa5Info              : wsa5 details
 * @return
 */
static void fillWsa5InfoToHeader(SOAP_t *soap, SOAP_WSA5_t *wsa5Info)
{
    if(soap->header == NULL)
    {
        soap_header(soap);
    }

    if(wsa5Info->wsa5__Action != NULL)
    {
        soap->header->wsa5__Action = wsa5Info->wsa5__Action;
    }

    if(wsa5Info->wsa5_to != NULL)
    {
        soap->header->wsa5__To = wsa5Info->wsa5_to;
    }

    if(wsa5Info->addInfo != NULL)
    {
        soap->header->addData = wsa5Info->addInfo;
    }

    soap->header->wsa5__MessageID = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief soapInitialize    : init soap instance with soap header
 * @param soap              : soap instance
 * @param User              : user information
 * @return
 */
static void soapInitialize(SOAP_t *soap, SOAP_USER_DETAIL_t *User)
{
    soap->recv_timeout = MAX_SOAP_RECV_TIME_OUT;
    soap->header = NULL;

    if(User->authReq == YES)
    {
        soap_header(soap);
        soap->header->Username = User->name;
        soap->header->Pwd 	   = User->pwd;
        soap->header->timeDIff = User->timeDiff;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSystemDateAndTime  : Get Time and date from camera
 * @param soap
 * @param getSystemDateAndTime_t
 * @param getSystemDateAndTimeResponse_t
 * @param User
 * @return
 */
INT16 GetSystemDateAndTime(SOAP_t *soap, GET_SYSTEM_DATE_AND_TIME_t *getSystemDateAndTime_t,
                           GET_SYSTEM_DATE_AND_TIME_RESPONSE_t *getSystemDateAndTimeResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetSystemDateAndTimeCommand(soap, User->addr, NULL, getSystemDateAndTime_t, getSystemDateAndTimeResponse_t);
        if ((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);

        /* Next try without authentication information*/
        User->authReq = ((User->authReq == YES) ? NO : YES);
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetSystemDateAndTime  : set Date and time to camera
 * @param soap
 * @param setSystemDateAndTime_t
 * @param setSystemDateAndTimeResponse_t
 * @param User
 * @return
 */
INT16 SetSystemDateAndTime(SOAP_t *soap, SET_SYSTEM_DATE_AND_TIME_t *setSystemDateAndTime_t,
                           SET_SYSTEM_DATE_AND_TIME_RESPONSE_t *setSystemDateAndTimeResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetSystemDateAndTimeCommand(soap, User->addr, NULL, setSystemDateAndTime_t, setSystemDateAndTimeResponse_t);
        if ((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetDeviceInformation  : Gets camera information eg Brand and model
 * @param soap
 * @param getDeviceInformation_t
 * @param getDeviceInformationResponse_t
 * @param User
 * @param pIsAuthFail
 * @return
 */
INT16 GetDeviceInformation(SOAP_t *soap, GET_DEVICE_INFORMATION_t *getDeviceInformation_t,
                           GET_DEVICE_INFORMATION_RESPONSE_t *getDeviceInformationResponse_t, SOAP_USER_DETAIL_t *User, BOOL *pIsAuthFail)
{
    INT16   retStatus;
    CHAR 	responseCode[] = "Please create your password to login"; /* For case where there is no user created (After Reset or first time case)*/

    User->authReq = NO;
    soapInitialize(soap, User);
    retStatus =	GetDeviceInformationCommand(soap, User->addr, NULL, getDeviceInformation_t, getDeviceInformationResponse_t);
    if (retStatus == SOAP_OK)
    {
        return SOAP_OK;
    }

    printSoapError(soap, __func__, retStatus, User->ipAddr);
    if (User->pwd[0] == '\0')
    {
        return retStatus;
    }

    User->authReq = YES;
    soapInitialize(soap, User);
    retStatus =	GetDeviceInformationCommand(soap, User->addr, NULL, getDeviceInformation_t, getDeviceInformationResponse_t);

    if ((soap->fault != NULL) && (soap->fault->SOAP_ENV__Reason != NULL) && (soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text != NULL)
            && (soap->fault->SOAP_ENV__Code != NULL) && (soap->fault->SOAP_ENV__Code->SOAP_ENV__Value != NULL))
    {
        /* To identify whether auth fail or not */
        if ((soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode != NULL) && (soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value != NULL))
        {
            if (strstr(soap->fault->SOAP_ENV__Code->SOAP_ENV__Subcode->SOAP_ENV__Value, INVALID_CREDENTIAL_STR) != NULL)
            {
                *pIsAuthFail = TRUE;
            }
        }

        printSoapError(soap, __func__, retStatus, User->ipAddr);

        /* For factory default cases, first new user should be created. Compare camera error response string */
        if(0 == strcmp(responseCode, soap->fault->SOAP_ENV__Reason->SOAP_ENV__Text))
        {
            retStatus = OnvifUnconfigureCamReq(User->ipAddr, User->port, User->name, User->pwd, ONVIF_SET_PASSWORD, NULL);
        }
    }

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief setUser   : Creates new user with username and password
 * @param soap
 * @param setUserPasswd_t
 * @param setUserPasswdResponse_t
 * @param User
 * @return
 */
INT16 setUser(SOAP_t *soap, SET_USER_INFORMATION_t *setUserPasswd_t , SET_USER_INFORMATION_RESPONSE_t *setUserPasswdResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    soapInitialize(soap, User);
    retStatus =	SetUserCommand(soap, User->addr, NULL, setUserPasswd_t, setUserPasswdResponse_t);
    if(retStatus != SOAP_OK)
    {
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCapabilities   : Get camera supported features
 * @param soap
 * @param getCapabilities_t
 * @param getCapabilitiesResponse_t
 * @param User
 * @return
 */
INT16 GetCapabilities(SOAP_t *soap, GET_CAPABILITIES_t *getCapabilities_t,
                      GET_CAPABILITIES_RESPONSE_t *getCapabilitiesResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetCapabilitiesCommand(soap, User->addr, NULL, getCapabilities_t, getCapabilitiesResponse_t);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCapabilities2  : Get camera supported features using media2 API
 * @param soap
 * @param getCapabilities_t
 * @param getCapabilitiesResponse_t
 * @param User
 * @return
 */
INT16 GetCapabilities2(SOAP_t *soap, GET_CAPABILITIES2_t *getCapabilities_t,
                       GET_CAPABILITIES_RESPONSE2_t *getCapabilitiesResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetCapabilitiesCommand2(soap, User->addr, NULL, getCapabilities_t, getCapabilitiesResponse_t);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetServices   : Gets all supported Services from camera
 * @param soap
 * @param getServices
 * @param getServicesResponse
 * @param User
 * @return
 */
INT16 GetServices(SOAP_t *soap, GET_SERVICES_t *getServices, GET_SERVICES_RESPONSE_t *getServicesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetServicesCommand(soap, User->addr, NULL,getServices ,getServicesResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetProfiles   : Gets Media Stream Profile configuration for all profiles
 * @param soap
 * @param getProfiles_t
 * @param getProfilesResponse_t
 * @param User
 * @return
 */
INT16 GetProfiles(SOAP_t *soap, GET_PROFILES_t *getProfiles_t, GET_PROFILES_RESPONSE_t *getProfilesResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetProfilesCommand(soap, User->addr, NULL, getProfiles_t, getProfilesResponse_t);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetProfiles2  : Gets Media Stream Profile configuration using media2
 * @param soap
 * @param getProfiles_t
 * @param getProfilesResponse_t
 * @param User
 * @return
 */
INT16 GetProfiles2(SOAP_t *soap, GET_PROFILES2_t *getProfiles_t, GET_PROFILES_RESPONSE2_t *getProfilesResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetProfilesCommand2(soap, User->addr, NULL, getProfiles_t, getProfilesResponse_t);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetProfile    : Get Stream profile configuration for single profile
 * @param soap
 * @param getProfile_t
 * @param getProfileResponse_t
 * @param User
 * @return
 */
INT16 GetProfile(SOAP_t *soap, GET_PROFILE_t *getProfile_t, GET_PROFILE_RESPONSE_t *getProfileResponse_t, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetProfileCommand(soap, User->addr, NULL, getProfile_t, getProfileResponse_t);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoEncoderConfigurationOptions   : Get Stream profile encoding configuration
 * @param soap
 * @param getVideoEncoderConfigurationOptions
 * @param getVideoEncoderConfigurationOptionsResponse
 * @param User
 * @return
 */
INT16 GetVideoEncoderConfigurationOptions(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_t *getVideoEncoderConfigurationOptions,
                                          GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE_t *getVideoEncoderConfigurationOptionsResponse,
                                          SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoEncoderConfiguratonOptionsCommand(soap, User->addr, NULL, getVideoEncoderConfigurationOptions,
                                                              getVideoEncoderConfigurationOptionsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoEncoderConfigurationOptions2 :Get Stream profile encoding configuration using media2 API
 * @param soap
 * @param getVideoEncoderConfigurationOptions
 * @param getVideoEncoderConfigurationOptionsResponse
 * @param User
 * @return
 */
INT16 GetVideoEncoderConfigurationOptions2(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS2_t *getVideoEncoderConfigurationOptions,
                                           GET_VIDEO_ENCODER_CONFIGURATION_OPTIONS_RESPONSE2_t *getVideoEncoderConfigurationOptionsResponse,
                                           SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoEncoderConfiguratonOptionsCommand2(soap, User->addr, NULL, getVideoEncoderConfigurationOptions,
                                                               getVideoEncoderConfigurationOptionsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCompatibleVideoSourceConfigurations
 * @param soap
 * @param getCompatibleVideoSourceConfigurations
 * @param getCompatibleVideoSourceConfigurationsResponse
 * @param User
 * @return
 */
INT16 GetCompatibleVideoSourceConfigurations(SOAP_t *soap, GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_t *getCompatibleVideoSourceConfigurations,
                                             GET_COMPATIBLE_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t *getCompatibleVideoSourceConfigurationsResponse,
                                             SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetCompatibleVideoSourceConfigurationsCommand(soap, User->addr, NULL, getCompatibleVideoSourceConfigurations,
                                                                  getCompatibleVideoSourceConfigurationsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCompatibleVideoEncoderConfigurations
 * @param soap
 * @param getCompatibleVideoEncoderConfigurations
 * @param getCompatibleVideoEncoderConfigurationsResponse
 * @param User
 * @return
 */
INT16 GetCompatibleVideoEncoderConfigurations(SOAP_t *soap, GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_t *getCompatibleVideoEncoderConfigurations,
                                              GET_COMPATIBLE_VIDEO_ENCODER_CONFIGURATIONS_RESPONSE_t *getCompatibleVideoEncoderConfigurationsResponse,
                                              SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetCompatibleVideoEncoderConfigurationsCommand(soap, User->addr, NULL, getCompatibleVideoEncoderConfigurations,
                                                                   getCompatibleVideoEncoderConfigurationsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
//-------------------------------------------------------------------------------------------------
/**
 * @brief GetAudioSources
 * @param soap
 * @param getAudioSources
 * @param getAudioSourcesResponse
 * @param User
 * @return
 */
INT16 GetAudioSources(SOAP_t *soap, GET_AUDIO_SOURCES_t *getAudioSources, GET_AUDIO_SOURCES_RESPONSE_t *getAudioSourcesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetAudioSourcesCommand(soap, User->addr, NULL, getAudioSources, getAudioSourcesResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
//-------------------------------------------------------------------------------------------------
/**
 * @brief GetAudioSourceConfigurations
 * @param soap
 * @param getAudioSourceConfigurations
 * @param getAudioSourceConfigurationsResponse
 * @param User
 * @return
 */
INT16 GetAudioSourceConfigurations(SOAP_t *soap, GET_AUDIO_SOURCE_CONFIGURATIONS_t *getAudioSourceConfigurations,
                                   GET_AUDIO_SOURCE_CONFIGURATIONS_RESPONSE_t *getAudioSourceConfigurationsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetAudioSourceConfigurationsCommand(soap, User->addr, NULL, getAudioSourceConfigurations, getAudioSourceConfigurationsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
//-------------------------------------------------------------------------------------------------
/**
 * @brief GetAudioEncoderConfigurations
 * @param soap
 * @param getAudioEncoderConfigurations
 * @param getAudioEncoderConfigurationsResponse
 * @param User
 * @return
 */
INT16 GetAudioEncoderConfigurations(SOAP_t *soap, GET_AUDIO_ENCODER_CONFIGURATIONS_t *getAudioEncoderConfigurations,
                                    GET_AUDIO_ENCODER_CONFIGURATIONS_RESPONSE_t *getAudioEncoderConfigurationsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetAudioEncoderConfigurationsCommand(soap, User->addr, NULL, getAudioEncoderConfigurations, getAudioEncoderConfigurationsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetConfigurations
 * @param soap
 * @param getConfigurations
 * @param getConfigurationsResponse
 * @param User
 * @return
 */
INT16 GetConfigurations(SOAP_t *soap, GET_CONFIGURATIONS_t *getConfigurations, GET_CONFIGURATIONS_RESPONSE_t *getConfigurationsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetConfigurationsCommand(soap, User->addr, NULL, getConfigurations, getConfigurationsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetConfigurationOptions
 * @param soap
 * @param getConfigurationOptions
 * @param getConfigurationOptionsResponse
 * @param User
 * @return
 */
INT16 GetConfigurationOptions(SOAP_t *soap, GET_CONFIGURATION_OPTIONS_t *getConfigurationOptions,
                              GET_CONFIGURATION_OPTIONS_RESPONSE_t *getConfigurationOptionsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetConfigurationOptionsCommand(soap, User->addr, NULL, getConfigurationOptions, getConfigurationOptionsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetRelayOutputs
 * @param soap
 * @param getRelayOutputs
 * @param getRelayOutputsResponse
 * @param User
 * @return
 */
INT16 GetRelayOutputs(SOAP_t *soap, GET_RELAY_OUTPUTS_t *getRelayOutputs, GET_RELAY_OUTPUTS_RESPONSE_t *getRelayOutputsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetRelayOutputsCommand(soap, User->addr, NULL, getRelayOutputs, getRelayOutputsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = ((User->authReq == YES) ? NO : YES);
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetMoveOptions
 * @param soap
 * @param getMoveOptions
 * @param getMoveOptionsResponse
 * @param User
 * @return
 */
INT16 GetMoveOptions(SOAP_t *soap, GET_MOVE_OPTIONS_t * getMoveOptions, GET_MOVE_OPTIONS_RESPONSE_t * getMoveOptionsResponse, SOAP_USER_DETAIL_t * User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetMoveOptionsCommand(soap, User->addr, NULL, getMoveOptions, getMoveOptionsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetOptions
 * @param soap
 * @param GetOptions
 * @param GetOptionsResponse
 * @param User
 * @return
 */
INT16 GetOptions(SOAP_t *soap, GET_OPTIONS_t * GetOptions, GET_OPTIONS_RESPONSE_t * GetOptionsResponse, SOAP_USER_DETAIL_t * User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetOptionsCommand(soap, User->addr, NULL, GetOptions, GetOptionsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoSourceConfigurations
 * @param soap
 * @param getVideoSourceConfigurations
 * @param getVideoSourceConfigurationsResponse
 * @param User
 * @return
 */
INT16 GetVideoSourceConfigurations(SOAP_t *soap, GET_VIDEO_SOURCE_CONFIGURATIONS_t *getVideoSourceConfigurations,
                                   GET_VIDEO_SOURCE_CONFIGURATIONS_RESPONSE_t *getVideoSourceConfigurationsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoSourceConfigurationsCommand(soap, User->addr, NULL, getVideoSourceConfigurations, getVideoSourceConfigurationsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AddVideoSourceConfiguration
 * @param soap
 * @param addVideoSourceConfiguration
 * @param addVideoSourceConfigurationResponse
 * @param User
 * @return
 */
INT16 AddVideoSourceConfiguration(SOAP_t *soap, ADD_VIDEO_SOURCE_CONFIGURATION_t *addVideoSourceConfiguration,
                                  ADD_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t *addVideoSourceConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	AddVideoSourceConfigurationCommand(soap, User->addr, NULL, addVideoSourceConfiguration, addVideoSourceConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetVideoEncoderConfiguration
 * @param soap
 * @param setVideoEncoderConfiguration
 * @param setVideoEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 SetVideoEncoderConfiguration(SOAP_t *soap, SET_VIDEO_ENCODER_CONFIGURATION_t *setVideoEncoderConfiguration,
                                   SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t *setVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetVideoEncoderConfigurationCommand(soap, User->addr, NULL, setVideoEncoderConfiguration, setVideoEncoderConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetVideoEncoderConfiguration2
 * @param soap
 * @param setVideoEncoderConfiguration
 * @param setVideoEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 SetVideoEncoderConfiguration2(SOAP_t *soap, SET_VIDEO_ENCODER_CONFIGURATION2_t *setVideoEncoderConfiguration,
                                    SET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t *setVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetVideoEncoderConfigurationCommand2(soap, User->addr, NULL, setVideoEncoderConfiguration, setVideoEncoderConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoEncoderConfiguration
 * @param soap
 * @param getVideoEncoderConfiguration
 * @param getVideoEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 GetVideoEncoderConfiguration(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION_t *getVideoEncoderConfiguration,
                                   GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t *getVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoEncoderConfigurationCommand(soap, User->addr, NULL, getVideoEncoderConfiguration, getVideoEncoderConfigurationResponse);
        if ((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoEncoderConfiguration2
 * @param soap
 * @param getVideoEncoderConfiguration
 * @param getVideoEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 GetVideoEncoderConfiguration2(SOAP_t *soap, GET_VIDEO_ENCODER_CONFIGURATION2_t *getVideoEncoderConfiguration,
                                    GET_VIDEO_ENCODER_CONFIGURATION_RESPONSE2_t *getVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoEncoderConfiguration2Command(soap, User->addr, NULL, getVideoEncoderConfiguration, getVideoEncoderConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AddVideoEncoderConfiguration
 * @param soap
 * @param addVideoEncoderConfiguration
 * @param addVideoEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 AddVideoEncoderConfiguration(SOAP_t *soap, ADD_VIDEO_ENCODER_CONFIGURATION_t *addVideoEncoderConfiguration,
                                   ADD_VIDEO_ENCODER_CONFIGURATION_RESPONSE_t *addVideoEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	AddVideoEncoderConfigurationCommand(soap, User->addr, NULL, addVideoEncoderConfiguration, addVideoEncoderConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AddAudioEncoderConfiguration
 * @param soap
 * @param addAudioEncoderConfiguration
 * @param addAudioEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 AddAudioEncoderConfiguration(SOAP_t *soap, ADD_AUDIO_ENCODER_CONFIGURATION_t *addAudioEncoderConfiguration,
                                   ADD_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t *addAudioEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	AddAudioEncoderConfigurationCommand(soap, User->addr, NULL, addAudioEncoderConfiguration, addAudioEncoderConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AddAudioSourceConfiguration
 * @param soap
 * @param addAudioSourceConfiguration
 * @param addAudioSourceConfigurationResponse
 * @param User
 * @return
 */
INT16 AddAudioSourceConfiguration(SOAP_t *soap, ADD_AUDIO_SOURCE_CONFIGURATION_t *addAudioSourceConfiguration,
                                  ADD_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t *addAudioSourceConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	AddAudioSourceConfigurationCommand(soap, User->addr, NULL, addAudioSourceConfiguration, addAudioSourceConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RemoveAudioEncoderConfiguration
 * @param soap
 * @param removeAudioEncoderConfiguration
 * @param removeAudioEncoderConfigurationResponse
 * @param User
 * @return
 */
INT16 RemoveAudioEncoderConfiguration(SOAP_t *soap, REMOVE_AUDIO_ENCODER_CONFIGURATION_t *removeAudioEncoderConfiguration,
                                      REMOVE_AUDIO_ENCODER_CONFIGURATION_RESPONSE_t *removeAudioEncoderConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	RemoveAudioEncoderConfigurationCommand(soap,User->addr, NULL, removeAudioEncoderConfiguration, removeAudioEncoderConfigurationResponse);

        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RemoveAudioSourceConfiguration
 * @param soap
 * @param removeAudioSourceConfiguration
 * @param removeAudioSourceConfigurationResponse
 * @param User
 * @return
 */
INT16 RemoveAudioSourceConfiguration(SOAP_t *soap, REMOVE_AUDIO_SOURCE_CONFIGURATION_t *removeAudioSourceConfiguration,
                                     REMOVE_AUDIO_SOURCE_CONFIGURATION_RESPONSE_t *removeAudioSourceConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	RemoveAudioSourceConfigurationCommand(soap, User->addr, NULL, removeAudioSourceConfiguration, removeAudioSourceConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetStreamUri
 * @param soap
 * @param getStreamUri
 * @param getStreamUriResponse
 * @param User
 * @return
 */
INT16 GetStreamUri(SOAP_t *soap, GET_STREAM_URI_t *getStreamUri,  GET_STREAM_URI_RESPONSE_t *getStreamUriResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetStreamUriCommand(soap, User->addr, NULL, getStreamUri, getStreamUriResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetMedia2StreamUri
 * @param soap
 * @param getStreamUri
 * @param getStreamUriResponse
 * @param User
 * @return
 */
INT16 GetMedia2StreamUri(SOAP_t *soap, GET_MEDIA2_STREAM_URI_t *getStreamUri,
                         GET_MEDIA2_STREAM_URI_RESPONSE_t *getStreamUriResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetMedia2StreamUriCommand(soap, User->addr, NULL, getStreamUri, getStreamUriResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSnapshotUri
 * @param soap
 * @param getSnapshotUri
 * @param getSnapshotUriResponse
 * @param User
 * @return
 */
INT16 GetSnapshotUri(SOAP_t *soap, GET_SNAPSHOT_URI_t *getSnapshotUri, GET_SNAPSHOT_URI_RESPONSE_t *getSnapshotUriResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetSnapshotUriCommand(soap, User->addr, NULL, getSnapshotUri, getSnapshotUriResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) ||  (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetSnapshotUri2
 * @param soap
 * @param getSnapshotUri
 * @param getSnapshotUriResponse
 * @param User
 * @return
 */
INT16 GetSnapshotUri2(SOAP_t *soap, GET_SNAPSHOT_URI2_t *getSnapshotUri, GET_SNAPSHOT_URI_RESPONSE2_t *getSnapshotUriResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetSnapshotUriCommand2(soap, User->addr, NULL, getSnapshotUri, getSnapshotUriResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) ||  (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetRelayOutputState
 * @param soap
 * @param setRelayOutputState
 * @param setRelayOutputStateResponse
 * @param User
 * @return
 */
INT16 SetRelayOutputState(SOAP_t *soap, SET_RELAY_OUTPUT_STATE_t *setRelayOutputState,
                          SET_RELAY_OUTPUT_STATE_RESPONSE_t *setRelayOutputStateResponse,  SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetRelayOutputStateCommand(soap, User->addr, NULL, setRelayOutputState, setRelayOutputStateResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetImagingSettings
 * @param soap
 * @param getImagingSettings
 * @param getImagingSettingsResponse
 * @param User
 * @return
 */
INT16 GetImagingSettings(SOAP_t *soap, struct _timg__GetImagingSettings *getImagingSettings,
                         struct _timg__GetImagingSettingsResponse *getImagingSettingsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetImagingSettingsCommand(soap, User->addr, NULL, getImagingSettings, getImagingSettingsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetImagingSettings
 * @param soap
 * @param setImagingSettings
 * @param setImagingSettingsResponse
 * @param User
 * @return
 */
INT16 SetImagingSettings(SOAP_t *soap, SET_IMAGING_SETTINGS_t *setImagingSettings,
                         SET_IMAGING_SETTINGS_RESPONSE_t *setImagingSettingsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetImagingSettingsCommand(soap, User->addr, NULL, setImagingSettings, setImagingSettingsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RelativeMove
 * @param soap
 * @param relativeMoveParam
 * @param relativeMoveRespParam
 * @param User
 * @return
 */
INT16 RelativeMove(SOAP_t *soap, RELATIVE_MOVE_t *relativeMoveParam, RELATIVE_MOVE_RESPONSE_t *relativeMoveRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_RelativeMove(soap, User->addr, NULL, relativeMoveParam, relativeMoveRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetPTZStatus
 * @param soap
 * @param getStatusParam
 * @param currStatus
 * @param User
 * @return
 */
INT16 GetPTZStatus(SOAP_t *soap, GET_PTZ_STATUS_t *getStatusParam, GET_PTZ_STATUS_RESPONSE_t *currStatus, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_GetPtzStatus(soap, User->addr, NULL, getStatusParam, currStatus);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AbsoluteMove
 * @param soap
 * @param absoluteMoveParam
 * @param absoluteMoveRespParam
 * @param User
 * @return
 */
INT16 AbsoluteMove(SOAP_t *soap, ABSOLUTE_MOVE_t *absoluteMoveParam, ABSOLUTE_MOVE_RESPONSE_t *absoluteMoveRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_AbsoluteMove(soap, User->addr, NULL, absoluteMoveParam, absoluteMoveRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief ContinuousMove
 * @param soap
 * @param continuousMoveParam
 * @param continuousMoveRespParam
 * @param User
 * @return
 */
INT16 ContinuousMove(SOAP_t *soap, CONTINUOUS_MOVE_t *continuousMoveParam, CONTINUOUS_MOVE_RESPONSE_t *continuousMoveRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_ContinuousMove(soap, User->addr, NULL, continuousMoveParam, continuousMoveRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StopPTZ
 * @param soap
 * @param stopParam
 * @param stopRespParam
 * @param User
 * @return
 */
INT16 StopPTZ(SOAP_t *soap, STOP_PTZ_t *stopParam, STOP_PTZ_RESPONSE_t *stopRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_StopPtz(soap, User->addr, NULL, stopParam, stopRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GoToPreset
 * @param soap
 * @param gotoPresetParam
 * @param gotoPresetRespParam
 * @param User
 * @return
 */
INT16 GoToPreset(SOAP_t *soap, GOTO_PRESET_t *gotoPresetParam, GOTO_PRESET_RESPONSE_t *gotoPresetRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_GotoPreset(soap, User->addr, NULL, gotoPresetParam, gotoPresetRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief RemovePreset
 * @param soap
 * @param removePresetParam
 * @param removePresetRespParam
 * @param User
 * @return
 */
INT16 RemovePreset(SOAP_t *soap, REMOVE_PRESET_t *removePresetParam, REMOVE_PRESET_RESPONSE_t *removePresetRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_RemovePreset(soap, User->addr, NULL, removePresetParam, removePresetRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetPreset
 * @param soap
 * @param setPresetParam
 * @param setPresetRespParam
 * @param User
 * @return
 */
INT16 SetPreset(SOAP_t *soap, SET_PRESET_t *setPresetParam, SET_PRESET_RESPONSE_t *setPresetRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_SetPreset(soap, User->addr, NULL, setPresetParam, setPresetRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetFocusStatus
 * @param soap
 * @param getFocusStatusParam
 * @param currFocusStatus
 * @param User
 * @return
 */
INT16 GetFocusStatus(SOAP_t *soap, GET_FOCUS_STATUS_t *getFocusStatusParam, GET_FOCUS_STATUS_RESPONSE_t	*currFocusStatus, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_GetFocusStatus(soap, User->addr, NULL, getFocusStatusParam, currFocusStatus);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief FocusMove
 * @param soap
 * @param moveParam
 * @param moveRespParam
 * @param User
 * @return
 */
INT16 FocusMove(SOAP_t *soap, MOVE_t *moveParam, MOVE_RESPONSE_t *moveRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_Move(soap, User->addr, NULL, moveParam, moveRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief StopFocus
 * @param soap
 * @param stopFocusParam
 * @param stopFocusRespParam
 * @param User
 * @return
 */
INT16 StopFocus(SOAP_t *soap, STOP_FOCUS_t *stopFocusParam, STOP_FOCUS_RESPONSE_t *stopFocusRespParam, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = soap_call_StopFocus(soap, User->addr, NULL, stopFocusParam, stopFocusRespParam);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CreatePullPtSubscription
 * @param soap
 * @param createPullPtSub
 * @param createPullPtSubResp
 * @param User
 * @param wsa5Info
 * @return
 */
INT16 CreatePullPtSubscription(SOAP_t *soap, CREATE_PULL_PT_SUBSCRIPTION_t *createPullPtSub,
                               CREATE_PULL_PT_SUBSCRIPTION_RESPONSE_t *createPullPtSubResp, SOAP_USER_DETAIL_t *User , SOAP_WSA5_t *wsa5Info)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        fillWsa5InfoToHeader(soap, wsa5Info);
        retStatus = CreatePullPtSubscriptionCommand(soap, User->addr, NULL, createPullPtSub, createPullPtSubResp);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetEventProperties
 * @param soap
 * @param getEvProperties
 * @param getEvPropertiesResp
 * @param User
 * @param wsa5Info
 * @return
 */
INT16 GetEventProperties(SOAP_t *soap, GET_EVENT_PROPERTIES_t *getEvProperties,
                         GET_EVENT_PROPERTIES_RESPONSE_t *getEvPropertiesResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        fillWsa5InfoToHeader(soap, wsa5Info);
        retStatus = GetEventPropertiesCommand(soap, User->addr, NULL, getEvProperties, getEvPropertiesResp);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief SetSynchronizePt
 * @param soap
 * @param setSyncPt
 * @param setSyncPtRef
 * @param User
 * @param wsa5Info
 * @return
 */
INT16 SetSynchronizePt(SOAP_t *soap, SET_SYNC_PT_t *setSyncPt, SET_SYNC_PT_RESPONSE_t *setSyncPtRef, SOAP_USER_DETAIL_t *User,SOAP_WSA5_t *wsa5Info)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        fillWsa5InfoToHeader(soap, wsa5Info);
        retStatus = SetSynchronizePtCommand(soap, User->addr, NULL, setSyncPt, setSyncPtRef);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief PullMessages
 * @param soap
 * @param pullMessages
 * @param pullMessagesResp
 * @param User
 * @param wsa5Info
 * @return
 */
INT16 PullMessages(SOAP_t *soap, PULL_MESSAGES_t *pullMessages, PULL_MESSAGES_RESPONSE_t *pullMessagesResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        fillWsa5InfoToHeader(soap, wsa5Info);
        retStatus = PullMessagesCommand(soap, User->addr, NULL, pullMessages, pullMessagesResp);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Renew
 * @param soap
 * @param renew
 * @param renewResp
 * @param User
 * @param wsa5Info
 * @return
 */
INT16 Renew(SOAP_t *soap, RENEW_t *renew, RENEW_RESPONSE_t *renewResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        fillWsa5InfoToHeader(soap, wsa5Info);
        retStatus = RenewCommand(soap, User->addr, NULL, renew, renewResp);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief UnSubscribe
 * @param soap
 * @param unSubscribe
 * @param unSubscribeResp
 * @param User
 * @param wsa5Info
 * @return
 */
INT16 UnSubscribe(SOAP_t *soap, UNSUBSCRIBE_t *unSubscribe, UNSUBSCRIBE_RESPONSE_t *unSubscribeResp, SOAP_USER_DETAIL_t *User, SOAP_WSA5_t *wsa5Info)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        fillWsa5InfoToHeader(soap, wsa5Info);
        retStatus = UnsubscribeCommand(soap, User->addr, NULL, unSubscribe, unSubscribeResp);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Getosds
 * @param soap
 * @param getOsds
 * @param getOSDsResponse
 * @param User
 * @return
 */
INT16 Getosds(SOAP_t *soap, GET_OSDS_t *getOsds, GET_OSDSRESPONSE_t *getOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetOsdsCommand(soap, User->addr, NULL, getOsds, getOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief Getosds2
 * @param soap
 * @param getOsds
 * @param getOSDsResponse
 * @param User
 * @return
 */
INT16 Getosds2(SOAP_t *soap, GET_OSDS2_t *getOsds, GET_OSDSRESPONSE2_t *getOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetOsdsCommand2(soap, User->addr, NULL, getOsds, getOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetOsd
 * @param soap
 * @param setOsds
 * @param setOSDsResponse
 * @param User
 * @return
 */
INT16 SetOsd(SOAP_t *soap, SET_OSD_t *setOsds, SET_OSDRESPONSE_t *setOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SetOsdCommand(soap, User->addr, NULL, setOsds, setOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief SetOsd2
 * @param soap
 * @param setOsds
 * @param setOSDsResponse
 * @param User
 * @return
 */
INT16 SetOsd2(SOAP_t *soap, SET_OSD2_t *setOsds, SET_OSDRESPONSE2_t *setOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SetOsdCommand2(soap, User->addr, NULL, setOsds, setOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief CreateOsd
 * @param soap
 * @param createOsds
 * @param createOSDsResponse
 * @param User
 * @return
 */
INT16 CreateOsd(SOAP_t *soap, CREATE_OSD_t *createOsds, CREATE_OSDRESPONSE_t *createOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = CreateOsdCommand(soap, User->addr, NULL, createOsds, createOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief CreateOsd2
 * @param soap
 * @param createOsds
 * @param createOSDsResponse
 * @param User
 * @return
 */
INT16 CreateOsd2(SOAP_t *soap, CREATE_OSD2_t *createOsds, CREATE_OSDRESPONSE2_t *createOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = CreateOsdCommand2(soap, User->addr, NULL, createOsds, createOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief DeleteOsd
 * @param soap
 * @param deleteOsds
 * @param deleteOSDsResponse
 * @param User
 * @return
 */
INT16 DeleteOsd(SOAP_t *soap, DELETE_OSD_t *deleteOsds, DELETE_OSDRESPONSE_t *deleteOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = DeleteOsdCommand(soap, User->addr, NULL, deleteOsds, deleteOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief DeleteOsd2
 * @param soap
 * @param deleteOsds
 * @param deleteOSDsResponse
 * @param User
 * @return
 */
INT16 DeleteOsd2(SOAP_t *soap, DELETE_OSD2_t *deleteOsds, DELETE_OSDRESPONSE2_t *deleteOSDsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = DeleteOsdCommand2(soap, User->addr, NULL, deleteOsds, deleteOSDsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoAnaltics  : Gets Video Anlytics configuration, Req for Motion window
 * @param soap
 * @param videoAnalytics
 * @param videoAnalyticsResponse
 * @param User
 * @return
 */
INT16 GetVideoAnaltics(SOAP_t *soap, GET_VIDEOANALYTICSCONFIGS_t *videoAnalytics,
                       GET_VIDEOANALYTICSCONFIGSRESPONSE_t *videoAnalyticsResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetVideoAnalticsConfigsCommand(soap, User->addr, NULL, videoAnalytics, videoAnalyticsResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoAnaltic   : Gets Video Anlytics configuration, Req for Motion window
 * @param soap
 * @param videoAnalytic
 * @param videoAnalyticResponse
 * @param User
 * @return
 */
INT16 GetVideoAnaltic(SOAP_t *soap, GET_VIDEOANALYTICSCONFIG_t *videoAnalytic,
                      GET_VIDEOANALYTICSCONFIGRESPONSE_t *videoAnalyticResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetVideoAnalticsConfigCommand(soap, User->addr, NULL, videoAnalytic, videoAnalyticResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetVideoAnaltic   : Sets motion window configuration
 * @param soap
 * @param videoAnalytic
 * @param videoAnalyticResponse
 * @param User
 * @return
 */
INT16 SetVideoAnaltic(SOAP_t *soap, SET_VIDEOANALYTICSCONFIG_t *videoAnalytic,
                      SET_VIDEOANALYTICSCONFIGRESPONSE_t *videoAnalyticResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SetVideoAnalticsConfigCommand(soap, User->addr, NULL, videoAnalytic, videoAnalyticResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetNetworkInterfaces  : Gets Network congiguration
 * @param soap
 * @param getNetworkInterfaces
 * @param getNetworkInterfacesResponse
 * @param User
 * @return
 */
INT16 GetNetworkInterfaces(SOAP_t *soap, GET_NETWORK_INTERFACES_t *getNetworkInterfaces,
                           GET_NETWORK_INTERFACESRESPONSE_t *getNetworkInterfacesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetNetworkInterfacesCommand(soap, User->addr, NULL, getNetworkInterfaces, getNetworkInterfacesResponse);
        if(( retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = (User->authReq == YES) ? NO : YES;
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetNetworkInterfaces  : Sets nework interface configuration to camera
 * @param soap
 * @param setNetworkInterfaces
 * @param setNetworkInterfacesResponse
 * @param User
 * @return
 */
INT16 SetNetworkInterfaces(SOAP_t *soap, SET_NETWORK_INTERFACES_t *setNetworkInterfaces,
                           SET_NETWORK_INTERFACESRESPONSE_t *setNetworkInterfacesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SetNetworkInterfacesCommand(soap, User->addr, NULL, setNetworkInterfaces, setNetworkInterfacesResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = (User->authReq == YES) ? NO : YES;
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetNetworkDfltGateway : sets Gateway configuration to camera
 * @param soap
 * @param setNetworkDfltGateway
 * @param setNetworkDfltGatewayResponse
 * @param User
 * @return
 */
INT16 SetNetworkDfltGateway(SOAP_t *soap, SET_NETWORK_DFLT_GATEWAY_t *setNetworkDfltGateway,
                            SET_NETWORK_DFLT_GATEWAYRESPONSE_t *setNetworkDfltGatewayResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SetNetworkDfltGatewayCommand(soap, User->addr, NULL, setNetworkDfltGateway, setNetworkDfltGatewayResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = (User->authReq == YES) ? NO : YES;
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SystemReboot  : Will reboot camera
 * @param soap
 * @param systemReboot
 * @param systemRebootResponse
 * @param User
 * @return
 */
INT16 SystemReboot(SOAP_t *soap, SYSTEM_REBOOT_t *systemReboot, SYSTEM_REBOOTRESPONSE_t *systemRebootResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SystemRebootCommand(soap, User->addr, NULL, systemReboot, systemRebootResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = (User->authReq == YES) ? NO : YES;
    }
    while(User->authReq == NO);

    return retStatus;
}


//-------------------------------------------------------------------------------------------------
/**
 * @brief GetNodes
 * @param soap
 * @param getNodes
 * @param getNodesResponse
 * @param User
 * @return
 */
INT16 GetNodes(SOAP_t *soap, GET_NODES_t *getNodes, GET_NODES_RESPONSE_t *getNodesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetNodesCommand(soap, User->addr, NULL, getNodes, getNodesResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief AddPtzConfiguration
 * @param soap
 * @param addPtzConfig
 * @param addPtzConfigResponse
 * @param User
 * @return
 */
INT16 AddPtzConfiguration(SOAP_t *soap, ADD_PTZ_CONFIG_t *addPtzConfig, ADD_PTZ_CONFIG_RESP_t *addPtzConfigResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = AddPtzConfigurationCommand(soap, User->addr, NULL, addPtzConfig, addPtzConfigResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

#if 0
//-------------------------------------------------------------------------------------------------
/**
 * @brief GotoHomePosition
 * @param soap
 * @param gotoHomePos
 * @param gotoHomePosRes
 * @param User
 * @return
 */
INT16 GotoHomePosition(SOAP_t *soap, GOTO_HOME_POS_t *gotoHomePos, GOTO_HOME_POS_RESP_t *gotoHomePosRes, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GotoHomePositionCommand(soap, User->addr, NULL, gotoHomePos, gotoHomePosRes);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------

/**
 * @brief SetHomePosition
 * @param soap
 * @param setHomePos
 * @param setHomePosRes
 * @param User
 * @return
 */
INT16 SetHomePosition(SOAP_t *soap, SET_HOME_POS_t *setHomePos, SET_HOME_POS_RESP_t *setHomePosRes, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = SetHomePostitionCommand(soap, User->addr, NULL, setHomePos, setHomePosRes);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetNetworkDfltGateway
 * @param soap
 * @param getNetworkDfltGateway
 * @param getNetworkDfltGatewayResponse
 * @param User
 * @return
 */
INT16 GetNetworkDfltGateway(SOAP_t *soap, GET_NET_DEFAULT_GATEWAY_t *getNetworkDfltGateway,
                            GET_NET_DEFAULT_GATEWAY_RES_t *getNetworkDfltGatewayResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetNetworkDefaultGatwayCmd(soap, User->addr, NULL, getNetworkDfltGateway, getNetworkDfltGatewayResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = (User->authReq == YES) ? NO : YES;
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetUser
 * @param soap
 * @param getUser
 * @param getUserResp
 * @param User
 * @return
 */
INT16 GetUser(SOAP_t *soap, GET_USER_t *getUser, GET_USER_RES_t *getUserResp, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    soapInitialize(soap, User);
    retStatus =	GetUsersCmd(soap, User->addr, NULL, getUser, getUserResp);
    if(retStatus != SOAP_OK)
    {
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief CreateUser
 * @param soap
 * @param createUser
 * @param createUserResp
 * @param User
 * @return
 */
INT16 CreateUser(SOAP_t *soap, CREATE_USER_t *createUser, CREATE_USER_RES_t *createUserResp, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    soapInitialize(soap, User);
    retStatus =	CreateUsersCmd(soap, User->addr, NULL, createUser, createUserResp);
    if(retStatus != SOAP_OK)
    {
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief DeleteUser
 * @param soap
 * @param deleteUser
 * @param deleteUserResp
 * @param User
 * @return
 */
INT16 DeleteUsers(SOAP_t *soap, DELETE_USER_t *deleteUser, DELETE_USER_RES_t *deleteUserResp, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    soapInitialize(soap, User);
    retStatus =	DeleteUsersCmd(soap, User->addr, NULL, deleteUser, deleteUserResp);
    if(retStatus != SOAP_OK)
    {
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetCompatibleAudioSourceConfigurations
 * @param soap
 * @param getCompatibleAudSrcConfig
 * @param getCompatibleAudSrcConfigResp
 * @param User
 * @return
 */
INT16 GetCompatibleAudioSourceConfigurations(SOAP_t *soap, GET_COMPATIBLE_AUDIO_SRC_CONFIG_t *getCompatibleAudSrcConfig,
                                             GET_COMPATIBLE_AUDIO_SRC_CONFIG_RESPONSE_t *getCompatibleAudSrcConfigResp, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetCompatibleAudioSrcConfigCmd(soap, User->addr, NULL, getCompatibleAudSrcConfig, getCompatibleAudSrcConfigResp);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetRelayOutputSetting
 * @param soap
 * @param setRelayOutputSetting
 * @param setRelayOutputSettingRes
 * @param User
 * @return
 */
INT16 SetRelayOutputSetting(SOAP_t *soap, SET_RELAY_OUT_SETTING_t *setRelayOutputSetting,
                            SET_RELAY_OUT_SETTING_RES_t *setRelayOutputSettingRes, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetRelayOutputSattingCmd(soap, User->addr, NULL, setRelayOutputSetting, setRelayOutputSettingRes);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
        User->authReq = ((User->authReq == YES) ? NO : YES);
    }
    while(User->authReq == NO);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoSourceConfiguration
 * @param soap
 * @param getVideoSourceConfiguration
 * @param getVideoSourceConfigurationResponse
 * @param User
 * @return
 */
INT16 GetVideoSourceConfiguration(SOAP_t *soap, GET_VIDEO_SOURCE_CONFIGURATION_t *getVideoSourceConfiguration,
                                  GET_VIDEO_SOURCE_CONFIGURATION_RESPONSE_t *getVideoSourceConfigurationResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoSourceConfigurationCommand(soap, User->addr, NULL, getVideoSourceConfiguration, getVideoSourceConfigurationResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief GetVideoSourceConfigurationOption
 * @param soap
 * @param input
 * @param response
 * @param User
 * @return
 */
INT16 GetVideoSourceConfigurationOption(SOAP_t *soap, GET_VIDEO_SOURCE_CONFIGURATION_OPTION_t *input,
                                        GET_VIDEO_SOURCE_CONFIGURATION_OPTION_RES_t *response, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	GetVideoSourceConfigurationOptionsCommand(soap, User->addr, NULL, input, response);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief SetVideoSourceConfiguration
 * @param soap
 * @param input
 * @param response
 * @param User
 * @return
 */
INT16 SetVideoSourceConfiguration(SOAP_t *soap, SET_VIDEO_SRC_CONFIG_t *input, SET_VIDEO_SRC_CONFIG_RES_t *response, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus =	SetVideoSourceConfigurationCommand(soap, User->addr, NULL, input, response);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetAnalyticsModules
 * @param   soap
 * @param   getAnalyticsModule
 * @param   getAnalyticsModuleResponse
 * @param   User
 * @return
 */
INT16 GetAnalyticsModules(SOAP_t *soap, GET_ANALYTICS_MODULES_t *getAnalyticsModule,
                          GET_ANALYTICS_MODULES_RESPONSE_t *getAnalyticsModuleResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetAnalyticsModulesCommand(soap, User->addr, NULL, getAnalyticsModule, getAnalyticsModuleResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ModifyAnalyticsModules
 * @param   soap
 * @param   modifyAnalyticsModule
 * @param   modifyAnalyticsModuleResponse
 * @param   User
 * @return
 */
INT16 ModifyAnalyticsModules(SOAP_t *soap, MODIFY_ANALYTICS_MODULES_t *modifyAnalyticsModule,
                             MODIFY_ANALYTICS_MODULES_RESPONSE_t *modifyAnalyticsModuleResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = ModifyAnalyticsModulesCommand(soap, User->addr, NULL, modifyAnalyticsModule, modifyAnalyticsModuleResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetRules
 * @param   soap
 * @param   getRules
 * @param   getRulesResponse
 * @param   User
 * @return
 */
INT16 GetRules(SOAP_t *soap, GET_RULES_t *getRules, GET_RULES_RESPONSE_t *getRulesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = GetRulesCommand(soap, User->addr, NULL, getRules, getRulesResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   ModifyRules
 * @param   soap
 * @param   modifyRules
 * @param   modifyRulesResponse
 * @param   User
 * @return
 */
INT16 ModifyRules(SOAP_t *soap, MODIFY_RULES_t *modifyRules, MODIFY_RULES_RESPONSE_t *modifyRulesResponse, SOAP_USER_DETAIL_t *User)
{
    INT16 retStatus;

    User->authReq = YES;
    do
    {
        soapInitialize(soap, User);
        retStatus = ModifyRulesCommand(soap, User->addr, NULL, modifyRules, modifyRulesResponse);
        if((retStatus == SOAP_OK) || (retStatus == SOAP_ERR) || (retStatus == SOAP_EOF) || (retStatus == SOAP_TCP_ERROR))
        {
            break;
        }
        printSoapError(soap, __func__, retStatus, User->ipAddr);
    }
    while(0);

    return retStatus;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
