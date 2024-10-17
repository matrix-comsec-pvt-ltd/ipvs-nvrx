//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		DisplayConfig.cpp
@brief      It manages live view display configuration
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
#include <QFile>
#include "DisplayConfig.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Information file which contains device display configuration */
#define CONFIG_FILE                     CONFIG_FILE_PATH"DisplayConfig.cfg"

/* Display configuration file version */
#define CONFIG_VERSION                  15

/* For backward compatibility */
#define CHANNEL_FOR_SEQ_MAX_VER14       (MAX_DEVICES * CAMERAS_MAX_V1)

/* Channel start from offset for display configuration */
#define CHANNEL_OFFSET  1

#if defined(RK3568_NVRL) || defined(RK3588_NVRH)
#define DEFAULT_MAIN_RESOLUTION         DISPLAY_RESOLUTION_2160P
#else
#define DEFAULT_MAIN_RESOLUTION         DISPLAY_RESOLUTION_1080P
#endif
#define DEFAULT_HDMI_BRIGHTENESS        50
#define DEFAULT_HDMI_CONTRAST           50
#define DEFAULT_HDMI_SATURATION         50
#define DEFAULT_HDMI_HUE                50
#define DEFAULT_TV_ADJUST_PARAM         0
#define DEFAULT_SEQ_INTERVAL            20
#define DEFAULT_SEQ_STATUS              false
#define DEFAULT_SEQ_CHANNEL             0
#define DEFAULT_PAGE                    0
#define DEFAULT_SELECTED_WINDOW         0
#define DEFAULT_CHANNEL(WIN_ID)         (WIN_ID + CHANNEL_OFFSET)
#define DEFAULT_LIVE_VIEW_TYPE          LIVE_VIEW_TYPE_SMOOTH
#define DEFAULT_MAX_WINDOWS             64
#define DEFAULT_OPTIMIZE_BANDWIDTH      true

#define MIN_PAGE_SEQ_INTERVAL           10
#define MAX_PAGE_SEQ_INTERVAL           255

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
    LAYOUT_TYPE_e   layoutId;
    quint16         currPage;
    quint8          seqInterval;
    bool            seqStatus;
    quint16         selectedWindow;
    WINDOW_INFO_t   windowInfo[CHANNEL_FOR_SEQ_MAX_VER14];

}DISPLAY_CONFIG_V14_t;

typedef struct
{
    STYLE_TYPE_e            mainDfltStyle;
    DISPLAY_RESOLUTION_e    mainDfltResolution;
    DISPLAY_PARAM_t         dispParam[PHYSICAL_DISPLAY_TYPE_MAX];
    quint32                 tvAdjustParam;
    DISPLAY_CONFIG_V14_t    dispConfig[MAX_DISPLAY_TYPE][MAX_STYLE_TYPE];
    quint16                 windowCount;
    bool                    optimizeBandwidth;

}LIVE_VIEW_DISP_CONFIG_VER13_t;

typedef struct
{
    STYLE_TYPE_e            mainDfltStyle;
    DISPLAY_RESOLUTION_e    mainDfltResolution;
    DISPLAY_PARAM_t         dispParam;
    quint32                 tvAdjustParam;
    DISPLAY_CONFIG_V14_t    dispConfig[MAX_DISPLAY_TYPE][MAX_STYLE_TYPE];
    quint16                 windowCount;
    bool                    optimizeBandwidth;
    LIVE_VIEW_TYPE_e        liveViewType;

}LIVE_VIEW_DISP_CONFIG_VER14_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const LAYOUT_TYPE_e defaultLayoutForStyle[MAX_STYLE_TYPE] = {FOUR_X_FOUR, ONE_X_ONE, TWO_X_TWO, THREE_X_THREE, FIVE_X_FIVE};

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Display config constructor
 */
DisplayConfig::DisplayConfig()
{
    memset(&liveViewDispCfg, 0, sizeof(liveViewDispCfg));
    liveViewDispCfg.optimizeBandwidth = DEFAULT_OPTIMIZE_BANDWIDTH;
    liveViewDispCfg.liveViewType = DEFAULT_LIVE_VIEW_TYPE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Display config distructor
 */
DisplayConfig::~DisplayConfig()
{

}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Init display configuration
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::InitConfig(void)
{
    VERSION_e   version = 0;
    QFile       configFile(CONFIG_FILE);

    /* Lock access to configuration */
    cfgAccess.lock();

    /* Open file in read/write mode */
    if (false == configFile.open(QIODevice::ReadWrite))
    {
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to open cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    do
    {
        /* Read version from file */
        if (configFile.read((char *)&version, sizeof(version)) < (qint64)sizeof(version))
        {
            EPRINT(CONFIG_PAGES, "fail to read config version: [path=%s]", CONFIG_FILE);
            break;
        }

        /* Version mismatched? */
        if (version != CONFIG_VERSION)
        {
            EPRINT(CONFIG_PAGES, "cnfg file version mismatched: [path=%s], [fileVer=%d], [sysVer=%d]", CONFIG_FILE, version, CONFIG_VERSION);
            if (false == UpdateOldDispConfig(configFile, version))
            {
                EPRINT(CONFIG_PAGES, "fail to update old config: [path=%s], [version=%d --> %d]", CONFIG_FILE, version, CONFIG_VERSION);
                break;
            }
        }
        else
        {
            /* Read display configuration from file */
            if (configFile.read((char *)&liveViewDispCfg, sizeof(liveViewDispCfg)) < (qint64)sizeof(liveViewDispCfg))
            {
                EPRINT(CONFIG_PAGES, "fail to read config: [path=%s]", CONFIG_FILE);
                break;
            }
        }

        /* Close the file */
        configFile.close();

        /* Unlock access to configuration */
        cfgAccess.unlock();

        /* Config init successfully */
        return true;

    }while(0);

    /* Error in config read or write. Write default config */
    if (false == WriteDefaultDispConfig(configFile))
    {
        /* Close the file */
        configFile.close();
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write default config: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Close the file */
    configFile.close();

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Default config written */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write default display configuration
 * @param   configFile
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteDefaultDispConfig(QFile &configFile)
{
    /* Write default display configuration */
    liveViewDispCfg.mainDfltStyle = STYLE_TYPE_1;
    liveViewDispCfg.mainDfltResolution = DEFAULT_MAIN_RESOLUTION;
    liveViewDispCfg.tvAdjustParam = DEFAULT_TV_ADJUST_PARAM;
    liveViewDispCfg.windowCount = DEFAULT_MAX_WINDOWS;
    liveViewDispCfg.optimizeBandwidth = DEFAULT_OPTIMIZE_BANDWIDTH;
    liveViewDispCfg.liveViewType = DEFAULT_LIVE_VIEW_TYPE;

    liveViewDispCfg.dispParam.brighteness = DEFAULT_HDMI_BRIGHTENESS;
    liveViewDispCfg.dispParam.contrast = DEFAULT_HDMI_CONTRAST;
    liveViewDispCfg.dispParam.saturation = DEFAULT_HDMI_SATURATION;
    liveViewDispCfg.dispParam.hue = DEFAULT_HDMI_HUE;

    /* Update configuration with default values */
    for(quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
    {
        for(quint8 styleIndex = 0; styleIndex < MAX_STYLE_TYPE; styleIndex++)
        {
            DISPLAY_CONFIG_t *pDispStyle = &liveViewDispCfg.dispConfig[displayIndex][styleIndex];
            pDispStyle->layoutId = defaultLayoutForStyle[styleIndex];
            pDispStyle->currPage = DEFAULT_PAGE;
            pDispStyle->seqInterval = DEFAULT_SEQ_INTERVAL;
            pDispStyle->seqStatus = DEFAULT_SEQ_STATUS;
            pDispStyle->selectedWindow = DEFAULT_SELECTED_WINDOW;

            for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
            {
                setDefaultWindowChannelConfig(displayIndex, windowIndex, &pDispStyle->windowInfo[windowIndex]);
            }
        }
    }

    /* Write configuration and update the status */
    return WriteDispConfig(configFile);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set default channel configuration of window
 * @param   displayIndex
 * @param   windowIndex
 * @param   pWindowInfo
 */
void DisplayConfig::setDefaultWindowChannelConfig(quint8 displayIndex, quint16 windowIndex, WINDOW_INFO_t *pWindowInfo)
{
    pWindowInfo->currentChannel = DEFAULT_SEQ_CHANNEL;
    pWindowInfo->sequenceStatus = DEFAULT_SEQ_STATUS;
    pWindowInfo->lastSequenceStatus = DEFAULT_SEQ_STATUS;
    pWindowInfo->sequenceInterval = DEFAULT_WINDOW_SEQ_INTERVAL;

    for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
    {
        /* Set local device name in first max cameras config and remaining set null */
        if ((channelIndex == 0) && (displayIndex == MAIN_DISPLAY) && (windowIndex < MAX_CAMERAS))
        {
            snprintf(pWindowInfo->camInfo[channelIndex].deviceName, MAX_DEVICE_NAME_SIZE, LOCAL_DEVICE_NAME);
            pWindowInfo->camInfo[channelIndex].defChannel = DEFAULT_CHANNEL(windowIndex);
        }
        else
        {
            pWindowInfo->camInfo[channelIndex].deviceName[0] = '\0';
            pWindowInfo->camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Update old display configuration
 * @param   configFile
 * @param   fileVersion
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::UpdateOldDispConfig(QFile &configFile, VERSION_e fileVersion)
{
    bool status = true;
    LIVE_VIEW_DISP_CONFIG_VER13_t *pDispCfg13 = NULL;
    LIVE_VIEW_DISP_CONFIG_VER14_t *pDispCfg14 = NULL;

    /** @note:
     * Here we have allocated dynamic memory because process/thread default stack size is 8MB and this strucure is ~2MB.
     * If you don't do that then application may crash. Hence either we have to alloc memory dynamically or use global variable. */
    do
    {
        pDispCfg13 = (LIVE_VIEW_DISP_CONFIG_VER13_t*)malloc(sizeof(LIVE_VIEW_DISP_CONFIG_VER13_t));
        if (NULL == pDispCfg13)
        {
            status = false;
            break;
        }

        pDispCfg14 = (LIVE_VIEW_DISP_CONFIG_VER14_t*)malloc(sizeof(LIVE_VIEW_DISP_CONFIG_VER14_t));
        if (NULL == pDispCfg14)
        {
            status = false;
            break;
        }

        switch(fileVersion)
        {
            case 13:
            {
                /* We have to read each parameters one by one due to structure padding */
                if (configFile.read((char *)&pDispCfg13->mainDfltStyle, sizeof(pDispCfg13->mainDfltStyle)) < (qint64)sizeof(pDispCfg13->mainDfltStyle))
                {
                    EPRINT(CONFIG_PAGES, "fail to read default style from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }

                if (configFile.read((char *)&pDispCfg13->mainDfltResolution, sizeof(pDispCfg13->mainDfltResolution)) < (qint64)sizeof(pDispCfg13->mainDfltResolution))
                {
                    EPRINT(CONFIG_PAGES, "fail to read default resolution from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }

                if (configFile.read((char *)&pDispCfg13->dispParam, sizeof(pDispCfg13->dispParam)) < (qint64)sizeof(pDispCfg13->dispParam))
                {
                    EPRINT(CONFIG_PAGES, "fail to read display param from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }

                if (configFile.read((char *)&pDispCfg13->tvAdjustParam, sizeof(pDispCfg13->tvAdjustParam)) < (qint64)sizeof(pDispCfg13->tvAdjustParam))
                {
                    EPRINT(CONFIG_PAGES, "fail to read tv adjust param from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }

                if (configFile.read((char *)pDispCfg13->dispConfig, sizeof(pDispCfg13->dispConfig)) < (qint64)sizeof(pDispCfg13->dispConfig))
                {
                    EPRINT(CONFIG_PAGES, "fail to read display style config from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }

                if (configFile.read((char *)&pDispCfg13->windowCount, sizeof(pDispCfg13->windowCount)) < (qint64)sizeof(pDispCfg13->windowCount))
                {
                    EPRINT(CONFIG_PAGES, "fail to read window count from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }

                if (configFile.read((char *)&pDispCfg13->optimizeBandwidth, sizeof(pDispCfg13->optimizeBandwidth)) < (qint64)sizeof(pDispCfg13->optimizeBandwidth))
                {
                    EPRINT(CONFIG_PAGES, "fail to read optimize bandwidth flag from cnfg file: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }
            }
            break;

            case 14:
            {
                /* Read display configuration from file as per version */
                if (configFile.read((char *)pDispCfg14, sizeof(LIVE_VIEW_DISP_CONFIG_VER14_t)) < (qint64)sizeof(LIVE_VIEW_DISP_CONFIG_VER14_t))
                {
                    EPRINT(CONFIG_PAGES, "fail to read config: [version=%d] [path=%s]", fileVersion, CONFIG_FILE);
                    status = false;
                    break;
                }
            }
            break;

            default:
            {
                /* Invalid config version */
                status = false;
            }
            break;
        }

        /* Is error found? */
        if (false == status)
        {
            break;
        }

        /* Earlier config update from old version to new version was not handled properly.
         * It is started from version 13.
         * Take care of default device name and page sequence interval minimum value */
        if (fileVersion == 13)
        {
            for(quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
            {
                for(quint8 styleIndex = 0; styleIndex < MAX_STYLE_TYPE; styleIndex++)
                {
                    /* We have changed minimum page sequence interval from 5 Sec to 10 Sec */
                    DISPLAY_CONFIG_V14_t *pDispStyle = &pDispCfg13->dispConfig[displayIndex][styleIndex];
                    if (pDispStyle->seqInterval < MIN_PAGE_SEQ_INTERVAL)
                    {
                        pDispStyle->seqInterval = MIN_PAGE_SEQ_INTERVAL;
                    }

                    for(quint16 windowIndex = 0; windowIndex < CHANNEL_FOR_SEQ_MAX_VER14; windowIndex++)
                    {
                        WINDOW_INFO_t *pWindowInfo = &pDispStyle->windowInfo[windowIndex];
                        for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
                        {
                            /* We have changed the local display name from "MY DEVICE" to "Matrix-NVR" */
                            if (strcmp(pWindowInfo->camInfo[channelIndex].deviceName, "MY DEVICE") == 0)
                            {
                                snprintf(pWindowInfo->camInfo[channelIndex].deviceName, MAX_DEVICE_NAME_SIZE, LOCAL_DEVICE_NAME);
                            }
                        }
                    }
                }
            }

            pDispCfg14->mainDfltStyle = pDispCfg13->mainDfltStyle;
            pDispCfg14->mainDfltResolution = pDispCfg13->mainDfltResolution;
            pDispCfg14->tvAdjustParam = pDispCfg13->tvAdjustParam;
            pDispCfg14->windowCount = pDispCfg13->windowCount;
            pDispCfg14->optimizeBandwidth = pDispCfg13->optimizeBandwidth;
            memcpy(&pDispCfg14->dispParam, &pDispCfg13->dispParam[HDMI], sizeof(pDispCfg14->dispParam));
            memcpy(&pDispCfg14->dispConfig, pDispCfg13->dispConfig, sizeof(pDispCfg14->dispConfig));
            pDispCfg14->liveViewType = DEFAULT_LIVE_VIEW_TYPE;  /* Added in file version 14 */
            fileVersion++;
        }

        /* Max camera changed from 64 to 96 in UI */
        if (fileVersion == 14)
        {
            liveViewDispCfg.mainDfltStyle = pDispCfg14->mainDfltStyle;
            liveViewDispCfg.mainDfltResolution = pDispCfg14->mainDfltResolution;
            liveViewDispCfg.tvAdjustParam = pDispCfg14->tvAdjustParam;
            liveViewDispCfg.windowCount = pDispCfg14->windowCount;
            liveViewDispCfg.optimizeBandwidth = pDispCfg14->optimizeBandwidth;
            liveViewDispCfg.dispParam = pDispCfg14->dispParam;
            liveViewDispCfg.liveViewType = pDispCfg14->liveViewType;

            for(quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
            {
                for(quint8 styleIndex = 0; styleIndex < MAX_STYLE_TYPE; styleIndex++)
                {
                    DISPLAY_CONFIG_t *pDispStyle = &liveViewDispCfg.dispConfig[displayIndex][styleIndex];
                    pDispStyle->layoutId = pDispCfg14->dispConfig[displayIndex][styleIndex].layoutId;
                    pDispStyle->currPage = pDispCfg14->dispConfig[displayIndex][styleIndex].currPage;
                    pDispStyle->seqInterval = pDispCfg14->dispConfig[displayIndex][styleIndex].seqInterval;
                    pDispStyle->seqStatus = pDispCfg14->dispConfig[displayIndex][styleIndex].seqStatus;
                    pDispStyle->selectedWindow = pDispCfg14->dispConfig[displayIndex][styleIndex].selectedWindow;

                    /* Copy previous device config of 64 cameras, to new device config and set default config in remaing cameras (65 to 96) */
                    for(quint8 deviceIndex = 0; deviceIndex < MAX_DEVICES; deviceIndex++)
                    {
                        for(quint8 cameraIndex = 0; cameraIndex < MAX_CAMERAS; cameraIndex++)
                        {
                            quint16 windowIndex = (deviceIndex * MAX_CAMERAS) + cameraIndex;
                            if (cameraIndex < CAMERAS_MAX_V1)
                            {
                                quint16 oldWindowIndex = (deviceIndex * CAMERAS_MAX_V1) + cameraIndex;
                                pDispStyle->windowInfo[windowIndex] = pDispCfg14->dispConfig[displayIndex][styleIndex].windowInfo[oldWindowIndex];
                            }
                            else
                            {
                                /* Added in file version 15 */
                                setDefaultWindowChannelConfig(displayIndex, windowIndex, &pDispStyle->windowInfo[windowIndex]);
                            }
                        }
                    }
                }
            }

            fileVersion++;
        }

        /* Write configuration and update the status */
        if (false == WriteDispConfig(configFile))
        {
            status = false;
            break;
        }
    } while(0);

    if (pDispCfg13)
    {
        free(pDispCfg13);
    }

    if (pDispCfg14)
    {
        free(pDispCfg14);
    }

    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write display configuration
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteDispConfig(void)
{
    bool    status = true;
    QFile   configFile(CONFIG_FILE);

    do
    {
        /* Open file in write mode */
        status = configFile.open(QIODevice::ReadWrite);
        if (status == false)
        {
            /* Fail to open config file */
            EPRINT(CONFIG_PAGES, "fail to open cnfg file: [path=%s]", CONFIG_FILE);
            break;
        }

        /* Write new configuration to file */
        status = WriteDispConfig(configFile);
        if (status == false)
        {
            /* Failed to write style config */
            EPRINT(CONFIG_PAGES, "fail to write style config in cnfg file: [path=%s]", CONFIG_FILE);
        }

        /* Close the file */
        configFile.close();

    }while(0);

    /* Return status */
    return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write display configuration
 * @param   configFile
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteDispConfig(QFile &configFile)
{
    /* Set permission of file */
    if (false == configFile.setPermissions(CONFIG_FILE_PERIMISSION))
    {
        EPRINT(CONFIG_PAGES, "fail to set file permission for cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Seek file pointer at beginning */
    if (false == configFile.seek(0))
    {
        EPRINT(CONFIG_PAGES, "fail to seek file position for cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Write current version number */
    VERSION_e version = CONFIG_VERSION;
    if (configFile.write((char *)&version, sizeof(version)) < (qint64)sizeof(VERSION_e))
    {
        EPRINT(CONFIG_PAGES, "fail to write cnfg file version: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Write configuration to file */
    if (configFile.write((char *)&liveViewDispCfg, sizeof(liveViewDispCfg)) < (qint64)sizeof(liveViewDispCfg))
    {
        EPRINT(CONFIG_PAGES, "fail to write window count in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write display style configuration
 * @param   dispType
 * @param   pDispCnfg
 * @param   styleIndex
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteConfig(DISPLAY_TYPE_e dispType, DISPLAY_CONFIG_t *pDispCnfg, STYLE_TYPE_e styleIndex)
{
    /* Is display type valie? */
    if (dispType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock access to configuration */
    cfgAccess.lock();

    /* Compare current and new configurations */
    /* PARASOFT : Rule CERT_C-ARR38-a, MISRAC2012-RULE_21_18-a - Considering incorrect flow */
    if (0 == memcmp(&liveViewDispCfg.dispConfig[dispType][styleIndex], pDispCnfg, sizeof(DISPLAY_CONFIG_t)))
    {
        /* Both config are same */
        cfgAccess.unlock();
        return true;
    }

    /* Update local copy of configuration with new one */
    /* PARASOFT : Rule CERT_C-ARR38-a, MISRAC2012-RULE_21_18-a - Considering incorrect flow */
    memcpy(&liveViewDispCfg.dispConfig[dispType][styleIndex], pDispCnfg, sizeof(DISPLAY_CONFIG_t));

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write style config in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read display style configuration
 * @param   dispType
 * @param   pDispCnfg
 * @param   styleIndex
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::ReadConfig(DISPLAY_TYPE_e dispType, DISPLAY_CONFIG_t *pDispCnfg, STYLE_TYPE_e styleIndex)
{
    if (dispType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock and Unlock config and read style configuration */
    cfgAccess.lock();
    /* PARASOFT : Rule CERT_C-STR31-b, MISRAC2012-RULE_21_18-a - Considering incorrect flow (sub activity type READ_DISP_ACTIVITY instead of READ_DFLTSTYLE_ACTIVITY) */
    memcpy(pDispCnfg, &liveViewDispCfg.dispConfig[dispType][styleIndex], sizeof(DISPLAY_CONFIG_t));
    cfgAccess.unlock();
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write default style configuration
 * @param   dispType
 * @param   styleIndex
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteDfltStyle(DISPLAY_TYPE_e dispType, STYLE_TYPE_e styleIndex)
{
    if (dispType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock access to configuration */
    cfgAccess.lock();

    /* Update local copy of configuration with new one */
    liveViewDispCfg.mainDfltStyle = styleIndex;

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write default style config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write default style in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read default style configuration
 * @param   dispType
 * @param   styleIndex
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::ReadDfltStyle(DISPLAY_TYPE_e dispType, STYLE_TYPE_e *styleIndex)
{
    if (dispType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock and Unlock config and read style configuration */
    cfgAccess.lock();
    *styleIndex = liveViewDispCfg.mainDfltStyle;
    cfgAccess.unlock();
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write default display resolution configuration
 * @param   dispType
 * @param   resolutionIndx
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteDfltResolution(DISPLAY_TYPE_e dispType, DISPLAY_RESOLUTION_e *resolutionIndx)
{
    if (dispType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock access to configuration */
    cfgAccess.lock();

    /* Update local copy of configuration with new one */
    liveViewDispCfg.mainDfltResolution = *resolutionIndx;

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write default resolution config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write default resolution in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read default resolution configuration
 * @param   dispType
 * @param   resolutionIndx
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::ReadDfltResolution(DISPLAY_TYPE_e dispType, DISPLAY_RESOLUTION_e *resolutionIndx)
{
    if (dispType >= MAX_DISPLAY_TYPE)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock and Unlock config and read style configuration */
    cfgAccess.lock();
    *resolutionIndx = liveViewDispCfg.mainDfltResolution;
    cfgAccess.unlock();
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write display param configuration
 * @param   dispType
 * @param   param
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteDispParam(PHYSICAL_DISPLAY_TYPE_e dispType, DISPLAY_PARAM_t &param)
{
    /* Is display type valie? */
    if (dispType != HDMI)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock access to configuration */
    cfgAccess.lock();

    /* Compare current and new configurations */
    if (0 == memcmp(&liveViewDispCfg.dispParam, &param, sizeof(DISPLAY_PARAM_t)))
    {
        /* Both config are same */
        cfgAccess.unlock();
        return true;
    }

    /* Update local copy of configuration with new one */
    memcpy(&liveViewDispCfg.dispParam, &param, sizeof(DISPLAY_PARAM_t));

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write display params config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write display param in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read display param configuration
 * @param   dispType
 * @param   resolutionIndx
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::ReadDispParam(PHYSICAL_DISPLAY_TYPE_e dispType, DISPLAY_PARAM_t &param)
{
    if (dispType != HDMI)
    {
        EPRINT(CONFIG_PAGES, "invld display type: [dispType=%d]", dispType);
        return false;
    }

    /* Lock and Unlock config and read style configuration */
    cfgAccess.lock();
    memcpy(&param, &liveViewDispCfg.dispParam, sizeof(DISPLAY_PARAM_t));
    cfgAccess.unlock();
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Delete device from configuration
 * @param   deviceName
 */
void DisplayConfig::DeleteDeviceFromConfig(const char *deviceName)
{
    bool isCnfgChanged = false;

    /* Lock access to configuration */
    cfgAccess.lock();

    for(quint8 displayIndex = 0; displayIndex < MAX_DISPLAY_TYPE; displayIndex++)
    {
        for(quint8 styleIndex = 0; styleIndex < MAX_STYLE_TYPE; styleIndex++)
        {
            for(quint16 windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
            {
                WINDOW_INFO_t *pWindowInfo = &liveViewDispCfg.dispConfig[displayIndex][styleIndex].windowInfo[windowIndex];
                for(quint8 channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
                {
                    if (strcmp(deviceName, pWindowInfo->camInfo[channelIndex].deviceName))
                    {
                        continue;
                    }

                    isCnfgChanged = true;
                    pWindowInfo->camInfo[channelIndex].deviceName[0] = '\0';
                    pWindowInfo->camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                }
            }
        }
    }

    /* Is config change found? */
    if (false == isCnfgChanged)
    {
        /* No change in config */
        cfgAccess.unlock();
        return;
    }

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write updated config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write updated config in cnfg file: [path=%s]", CONFIG_FILE);
        return;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write tv adjust param configuration
 * @param   tvAdjust
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteTvAdjustConfig(quint32 &tvAdjust)
{
    /* Lock access to configuration */
    cfgAccess.lock();

    /* Update local copy of configuration with new one */
    liveViewDispCfg.tvAdjustParam = tvAdjust;

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write tv adjust config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write tv adjust in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read tv adjust param configuration
 * @param   tvAdjust
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::ReadTvAdjustConfig(quint32 &tvAdjust)
{
    cfgAccess.lock();
    tvAdjust = liveViewDispCfg.tvAdjustParam;
    cfgAccess.unlock();
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write max window configuration
 * @param   windowCount
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteMaxWindows(quint16 windowCount)
{
    /* Lock access to configuration */
    cfgAccess.lock();

    /* Is max count already written? */
    if (liveViewDispCfg.windowCount >= windowCount)
    {
        /* Max window count already written in configuration */
        cfgAccess.unlock();
        DPRINT(CONFIG_PAGES, "max window count already added in config: [reqested=%d], [config=%d]", windowCount, liveViewDispCfg.windowCount);
        return true;
    }

    /* Update local copy of configuration with new one */
    liveViewDispCfg.windowCount = windowCount;

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write window count config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write window count in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read max window configuration
 * @param   windowCount
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::ReadMaxWindows(quint16 &windowCount)
{
    cfgAccess.lock();
    windowCount = liveViewDispCfg.windowCount;
    cfgAccess.unlock();
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write optimize bandwidth flag configuration
 * @param   flag
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteBandwidthOptFlag(bool flag)
{
    /* Lock access to configuration */
    cfgAccess.lock();

    /* Update local copy of configuration with new one */
    liveViewDispCfg.optimizeBandwidth = flag;

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write optimize bandwidth config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write optimize bandwidth flag in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get optimize bandwidth flag configuration
 * @return  Returns true if enabled else returns false
 */
bool DisplayConfig::ReadBandwidthOptFlag(void)
{
    return liveViewDispCfg.optimizeBandwidth;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Write live view type configuration
 * @param   liveViewType
 * @return  Returns true on success and false on failure
 */
bool DisplayConfig::WriteLiveViewType(LIVE_VIEW_TYPE_e liveViewType)
{
    /* Lock access to configuration */
    cfgAccess.lock();

    /* Update local copy of configuration with new one */
    liveViewDispCfg.liveViewType = liveViewType;

    /* Write new configuration to file */
    if (false == WriteDispConfig())
    {
        /* Failed to write live view type config */
        cfgAccess.unlock();
        EPRINT(CONFIG_PAGES, "fail to write live view type in cnfg file: [path=%s]", CONFIG_FILE);
        return false;
    }

    /* Unlock access to configuration */
    cfgAccess.unlock();

    /* Config written Successfully */
    return true;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get live view type configuration
 * @return  Returns current set live view type
 */
LIVE_VIEW_TYPE_e DisplayConfig::ReadLiveViewType(void)
{
    return liveViewDispCfg.liveViewType;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
