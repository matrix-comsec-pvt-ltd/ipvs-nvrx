#if !defined DEVICEDEFINE_H
#define DEVICEDEFINE_H
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DeviceDefine.h
@brief		It provides the device related common defines
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxTypedef.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* GUI client application name */
#define	GUI_APPL_NAME           "multiNvrClient"

/* Firmware and config upgrade related paths */
#define FIRMWARE_FILE           ZIP_DIR_PATH "/NVR_Firmware.zip"
#define CONFIG_ZIP_FILE         ZIP_DIR_PATH "/NVR_Config.zip"

#if defined(HI3536_NVRL) || defined(RK3568_NVRL)
#define MAX_CAMERA              16
#elif defined(HI3536_NVRH)
#define MAX_CAMERA              64
#elif defined(RK3588_NVRH)
#define MAX_CAMERA              96
#endif

#define FILE_VER_REV_SUFFIX     "V%2hdR%2hd.%3s"
#define ZIP_FILE_VER_REV_STR    "V%02dR%02d.zip"
#define FIRMWARE_NAME_PREFIX    DEVICE_NAME_PREFIX "_firmware"
#define CONFIG_NAME_PREFIX      DEVICE_NAME_PREFIX "_config"
#define CONFIG_BACKUP_FILE_NAME CONFIG_NAME_PREFIX "_" ZIP_FILE_VER_REV_STR

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* DEVICEDEFINE_H */
