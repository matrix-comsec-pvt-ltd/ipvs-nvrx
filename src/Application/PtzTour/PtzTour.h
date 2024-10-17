#ifndef PTZTOUR_H_
#define PTZTOUR_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		PtzTour.h
@brief      PTZ module provides API to start and stop manual PTZ tour as configured for a camera.
            This module periodically checks scheduled time period for PTZ tour of all the cameras.
            If any PTZ tour of a camera matches at the instance to start the configured tour, it
            starts schedule PTZ tour with the configured tour patter and configured preset positions
            and view time of the tour. In the same way, if any PTZ tour for a camera found to end
            the tour at the instance, it ends scheduled tour started.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MANUAL_TOUR_ADV_DETAIL		"User: %s, Tour: %s"
#define SCHEDULE_TOUR_ADV_DETAIL	"Tour: %s"

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
	PTZ_HLT_NO_TOUR = 0,
	PTZ_HLT_SCH_TOUR,
	PTZ_HLT_MAN_TOUR,
	PTZ_HLT_PAUS_TOUR,
	PTZ_HLT_MAX_TOUR
}PTZ_TOUR_HLT_STATUS_e;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitPtzTour(void);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StartManualPtzTour(UINT8 camId, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e StopManualPtzTour(UINT8 camId, CHARPTR advncDetail);
//-------------------------------------------------------------------------------------------------
void UpdateTourCfg(UINT8 camIndex, PRESET_TOUR_CONFIG_t newTourCfg, PRESET_TOUR_CONFIG_t *oldPresetPtr);
//-------------------------------------------------------------------------------------------------
void UpdateTourScheduleCfg(UINT8 camIndex, TOUR_SCHEDULE_CONFIG_t newTourSchCfg, TOUR_SCHEDULE_CONFIG_t *oldSchedule);
//-------------------------------------------------------------------------------------------------
BOOL GetCamTourStatus(UINT8 camId, UINT8 dummy);
//-------------------------------------------------------------------------------------------------
BOOL PausePtzTour(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
NET_CMD_STATUS_e ResumePtzTour(UINT8 camIndex);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* PTZTOUR_H_ */
