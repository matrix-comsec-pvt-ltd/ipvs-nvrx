#ifndef _DISPLAY_CONFIG_H_
#define _DISPLAY_CONFIG_H_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DisplayConfig.h
@brief      It manages live view display configuration
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
#include <QObject>
#include <QMutex>
#include "EnumFile.h"
#include "DataStructure.h"
#include "fileWrite.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define DEFAULT_WINDOW_SEQ_INTERVAL         10

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    quint8 brighteness;
    quint8 contrast;
    quint8 saturation;
    quint8 hue;

}DISPLAY_PARAM_t;

typedef struct
{
    STYLE_TYPE_e            mainDfltStyle;
    DISPLAY_RESOLUTION_e    mainDfltResolution;
    DISPLAY_PARAM_t         dispParam;
    quint32                 tvAdjustParam;
    DISPLAY_CONFIG_t        dispConfig[MAX_DISPLAY_TYPE][MAX_STYLE_TYPE];
    quint16                 windowCount;
    bool                    optimizeBandwidth;
    LIVE_VIEW_TYPE_e        liveViewType;

}LIVE_VIEW_DISP_CONFIG_t;

//#################################################################################################
// @CLASS
//#################################################################################################
class DisplayConfig : public QObject
{
public:
    // constructor and destructor
    DisplayConfig();
    ~DisplayConfig();

    // initializes configuration
    bool InitConfig(void);

    // write default display configuration
    bool WriteDefaultDispConfig(QFile &configFile);

    // update old display configuration
    bool UpdateOldDispConfig(QFile &configFile, VERSION_e fileVersion);

    // write display configuration
    bool WriteDispConfig(void);
    bool WriteDispConfig(QFile &configFile);

    // read and write style layout configuration
    bool WriteConfig(DISPLAY_TYPE_e dispType, DISPLAY_CONFIG_t *pDispCnfg, STYLE_TYPE_e styleIndex);
    bool ReadConfig(DISPLAY_TYPE_e dispType, DISPLAY_CONFIG_t *pDispCnfg, STYLE_TYPE_e styleIndex);

    // read and write default style configuration
    bool WriteDfltStyle(DISPLAY_TYPE_e dispType, STYLE_TYPE_e styleIndex);
    bool ReadDfltStyle(DISPLAY_TYPE_e dispType, STYLE_TYPE_e *styleIndex);

    // read and write default resolution configuration
    bool WriteDfltResolution(DISPLAY_TYPE_e dispType, DISPLAY_RESOLUTION_e *resolutionIndx);
    bool ReadDfltResolution(DISPLAY_TYPE_e dispType, DISPLAY_RESOLUTION_e *resolutionIndx);

    // read and write display parameter configuration
    bool WriteDispParam(PHYSICAL_DISPLAY_TYPE_e dispType, DISPLAY_PARAM_t &param);
    bool ReadDispParam(PHYSICAL_DISPLAY_TYPE_e dispType, DISPLAY_PARAM_t &param);

    // delete device configuration
    void DeleteDeviceFromConfig(const char *deviceName);

    // read and write tv adjust configuration
    bool WriteTvAdjustConfig(quint32 &tvAdjust);
    bool ReadTvAdjustConfig(quint32 &tvAdjust);

    // read and write max windows configuration
    bool ReadMaxWindows(quint16 &windowCount);
    bool WriteMaxWindows(quint16 windowCount);

    // read and write optimize bandwidth flag configuration
    bool WriteBandwidthOptFlag(bool flag);
    bool ReadBandwidthOptFlag(void);

    // read and write live view type configuration
    bool WriteLiveViewType(LIVE_VIEW_TYPE_e liveViewType);
    LIVE_VIEW_TYPE_e ReadLiveViewType(void);

private:
    // store live view display configuration
    LIVE_VIEW_DISP_CONFIG_t liveViewDispCfg;

    // write default channel config of window
    void setDefaultWindowChannelConfig(quint8 displayIndex, quint16 windowIndex, WINDOW_INFO_t *pWindowInfo);

    // read write access lock for config copy
    QMutex cfgAccess;
};

//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif // _DISPLAY_CONFIG_H_
