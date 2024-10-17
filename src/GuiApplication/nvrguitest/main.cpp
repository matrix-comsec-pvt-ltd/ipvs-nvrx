//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		main.cpp
@brief		This is main file for nvr test application (Hardware Test).
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <unistd.h>

/* QT Library Includes */
#include <QApplication>

/* Application Includes */
#include "MainWindow.h"
#include "DecDispLib.h"
#include "CommonApi.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define LOG_FILE                LOG_DIR_PATH "/log.testapp"
#define LOG_ROTATION            1 // [n+1] files will be rotated
#define MAX_LOG_FILE_SZ			(1 * MEGA_BYTE)

/* qt kms config file path */
#define QT_KMS_CNFG_FILE_PATH   "/tmp/qt_kms_cnfg.json"

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
MainWindow *m_window = NULL;

//#################################################################################################
// @DEFINATIONS
//#################################################################################################
#if defined(RK3568_NVRL) || defined(RK3588_NVRH)
//-------------------------------------------------------------------------------------------------
/**
 * @brief   Generate Qt kms config file for display
 * @param   dispWidth
 * @param   dispHeight
 * @param   frameRate
 * @return
 */
static BOOL generateQtKmsCnfgFile(UINT32 dispWidth, UINT32 dispHeight, UINT32 frameRate)
{
    /* Remove any existing QT KMS config file */
    unlink(QT_KMS_CNFG_FILE_PATH);

    /* Create QT KMS config file */
    FILE *pFile = fopen(QT_KMS_CNFG_FILE_PATH, "w+");
    if (pFile == NULL)
    {
        EPRINT(GUI_SYS, "unable to create %s file", QT_KMS_CNFG_FILE_PATH);
        return FAIL;
    }

    fprintf(pFile, "{\n\t\"device\": \"/dev/dri/card0\","
                   "\n\t\"hwcursor\": false,"
                   "\n\t\"outputs\": [\n\t\t{ \"name\": \"HDMI1\", \"mode\": \"%dx%d@%d\" }"
                   "\n\t]\n}\n", dispWidth, dispHeight, frameRate);
    fclose(pFile);
    DPRINT(GUI_SYS, "Hardware Test Application started with Resolution: %dx%d@%d", dispWidth, dispHeight, frameRate);
    return SUCCESS;
}
#endif

//-------------------------------------------------------------------------------------------------
/**
 * @brief   The application starts with this function
 * @param   argc
 * @param   argv
 * @return
 */
int main(int argc, char *argv[])
{
    /* Init debug module */
    InitDebugLog();

    /* Restart syslog daemon for hardware test application */
    Utils_ExeCmd((CHARPTR)"killall syslogd");
    Utils_ExeCmd((CHARPTR)QString("syslogd -O %1 -s %2 -b %3 -S").arg(LOG_FILE).arg((MAX_LOG_FILE_SZ/KILO_BYTE)).arg(LOG_ROTATION).toUtf8().constData());

    /* Init display module with FHD */
    EPRINT(GUI_SYS, "NVR HARDWARE TEST APPLICATION: [build=%s %s], [software=V%02dR%02d.%d], [communication=V%02dR%02d]",
           __DATE__, __TIME__, SOFTWARE_VERSION, SOFTWARE_REVISION, PRODUCT_SUB_REVISION, COMMUNICATION_VERSION, COMMUNICATION_REVISION);

    /* Init decoder library for audio test */
    InitDecodeDisplay(DISPLAY_RESOLUTION_1080P);

    #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
    /* Update Configured Display Resolution in case of different resolution is set */
    DISPLAY_RESOLUTION_e    tCurrMainResolution;
    UINT32                  dispWidth, dispHeight, frameRate;

    /* Get current display resolution */
    GetCurrentMainResolution(&tCurrMainResolution, &dispWidth, &dispHeight, &frameRate);

    /* Prepare EGLFS KMS config file for QT display initialization */
    generateQtKmsCnfgFile(dispWidth, dispHeight, frameRate);

    /* Set QT EGLFS environment variables */
    qputenv("QT_QPA_PLATFORM", "eglfs");
    qputenv("QT_QPA_EGLFS_INTEGRATION", "eglfs_kms");
    qputenv("QT_QPA_EGLFS_KMS_CONFIG", QT_KMS_CNFG_FILE_PATH);
    qputenv("QT_QPA_EGLFS_KMS_ATOMIC", "1");
    #endif

    /* Enters the main event loop and waits until exit() is called. */
    QApplication app(argc, argv);
    m_window = new MainWindow();

    /* Exit is called */
    try
    {
        app.exec();
    }
    catch(const std::bad_alloc &e)
    {
        printf(" \n\n ********************** LOW MEMORY [%s] **********************\n\n", e.what());
        return -1;
    }
    catch(const std::exception &e)
    {
        printf(" \n\n ********************** Exception [%s] **********************\n\n", e.what());
        return -2;
    }
    catch(...)
    {
        printf(" \n\n ********************** Unhandled Exception **********************\n\n");
        return -3;
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
