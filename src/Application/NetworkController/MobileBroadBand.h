#ifndef MOBILEBROADBAND_H
#define MOBILEBROADBAND_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MobileBroadband.h
@brief      This file declares usb modem port related defines and prototypes.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"
#include "NetworkManager.h"
#include "NetworkController.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	MAX_DEVICE_INFO_LENGTH			(16)
#define	MAX_MANUFACTURER_NAME_LENGTH	(50)
#define	MAX_DEVICE_PATH_LENGTH			(128)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    MODEM_NOT_PRESENT,  // Modem Physically not present
    MODEM_DISCONNECTED, // Modem not connected to network
    MODEM_CONNECTED,    // Modem connected to network
    MAX_MODEM_STATUS
}MODEM_STATUS_e;

typedef struct
{
    CHAR action[MAX_DEVICE_INFO_LENGTH];
    CHAR path[MAX_DEVICE_PATH_LENGTH];
    CHAR vendorId[MAX_DEVICE_INFO_LENGTH];
    CHAR productId[MAX_DEVICE_INFO_LENGTH];
    CHAR interfaceClass[MAX_DEVICE_INFO_LENGTH];
    CHAR interfaceSubClass[MAX_DEVICE_INFO_LENGTH];
    CHAR interfaceProtocol[MAX_DEVICE_INFO_LENGTH];
    CHAR interfaceNumber[MAX_DEVICE_INFO_LENGTH];
    CHAR vendor[MAX_DEVICE_INFO_LENGTH];
    CHAR model[MAX_DEVICE_INFO_LENGTH];
    CHAR rev[MAX_DEVICE_INFO_LENGTH];
    CHAR manufacturer[MAX_MANUFACTURER_NAME_LENGTH];
    CHAR product[MAX_MANUFACTURER_NAME_LENGTH];
    CHAR serial[MAX_DEVICE_INFO_LENGTH];
}BROADBAND_DEVICE_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitMobileBroadband(void);
//-------------------------------------------------------------------------------------------------
void MobileBroadbandCfgUpdate(BROAD_BAND_CONFIG_t newCopy, BROAD_BAND_CONFIG_t *oldCopy);
//-------------------------------------------------------------------------------------------------
void DetectBroadBandDevice(BROADBAND_DEVICE_INFO_t *device);
//-------------------------------------------------------------------------------------------------
void DetectTtyDevice(const CHAR *action, const CHAR *devPath, const CHAR *ttyNode);
//-------------------------------------------------------------------------------------------------
void DetectNetDevice(const CHAR *action, const CHAR *devNode);
//-------------------------------------------------------------------------------------------------
void UsbModemStatusChanged(BOOL isStatusConnected);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif  // MOBILEBROADBAND_H
