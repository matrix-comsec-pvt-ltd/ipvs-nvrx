//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		main.cpp
@brief		This is main file for multi NVR Client (GUI).
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/resource.h>

/* Application Includes */
#include "MainWindow.h"
#include "ApplController.h"
#include "../DecoderLib/include/DecDispLib.h"
#include "EnumFile.h"

extern "C" {
#include "DebugLog.h"
#include "DebugLog/UiLogServer.h"
#include "Utils/Utils.h"
}

/* QT Library Includes */
#include <QApplication>

//#################################################################################################
// @DEFINES
//#################################################################################################
#define	APPL_NAME       "nvrAppl.bin"

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static MainWindow       *m_window = NULL;
static ApplController   *m_applController;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void deInitUIApp(int);
//-------------------------------------------------------------------------------------------------
static void setResourceLimit(void);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @DEFINATIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   The application starts with this function
 * @param   argc
 * @param   argv
 * @return
 */
INT32 main(INT32 argc, CHAR *argv[])
{
    DISPLAY_RESOLUTION_e    configResolution, curSetResolution;
    UINT32                  dispWidth, dispHeight, frameRate;

    /* PARASOFT : Rule OWASP2021-A5-c - No need to check errno */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGUSR1, deInitUIApp);

    InitDebugLog();
    InitUiLogServer();

    /* Set application resource limit */
    setResourceLimit();
    EPRINT(GUI_SYS, "NVR GUI APPLICATION: [build=%s %s], [software=V%02dR%02d.%d], [communication=V%02dR%02d]",
           __DATE__, __TIME__, SOFTWARE_VERSION, SOFTWARE_REVISION, PRODUCT_SUB_REVISION, COMMUNICATION_VERSION, COMMUNICATION_REVISION);

    m_applController = ApplController::getInstance();

    /* Read Configured Display Resolution */
    configResolution = m_applController->readResolution(MAIN_DISPLAY);

    /* Initialize Display with configured resolution(Using Default Refresh frequencies) */
    InitDecodeDisplay(configResolution);

    /* Update Configured Display Resolution in case of different resolution is set */
    GetCurrentMainResolution(&curSetResolution, &dispWidth, &dispHeight, &frameRate);

    /* Set current display resolution to highlight in display settings */
    m_applController->setCurrentDisplayResolution(curSetResolution);

    #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
    /* To reduce CPU usage in 4K, We have added patch in linuxfb to send QT graphical data through rockchip graphical layer
     * which is recommended by rockchip. For 4K resolusion, QT layer will run in FHD and video layer will run in 4K.
     * Rockchip GPU will merge both layers and display on the screen. */
    qputenv("QT_QPA_FB_VO", "1");
    if (DISPLAY_RESOLUTION_2160P == curSetResolution)
    {
        dispWidth = 1920;
        dispHeight = 1080;
        qputenv("QT_QPA_FB_4K_DISP", "1");
    }
    #else
    /* In 4K resolution with eglfs, mouse cursor moves very slowly and getting very poor performance.
     * If we use linuxfb in 4K, we are getting too good performance than eglfs. Hence used linuxfb for all resolutions. */
    if (DISPLAY_RESOLUTION_2160P == curSetResolution)
    {
        /* To display proper cursor (48x48 size) in 4K, we have added patch in QT linuxfb for custom cursor */
        qputenv("QT_QPA_FB_SETCURSOR", ":/Images_Nvrx/Mouse_Cursors/custom_cursor.json");
    }
    #endif

    /* Set QT platform as linuxfb and display width-height */
    qputenv("QT_QPA_PLATFORM", QString("linuxfb:size=%1x%2").arg(dispWidth).arg(dispHeight).toUtf8());

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    /* Enters the main event loop and waits until exit() is called */
    QApplication app(argc, argv);
    m_window = new MainWindow();

    /* Exit is called */
    int retCode = 0;
    try
    {
        retCode = app.exec();
    }
    catch (const std::bad_alloc &e)
    {
        std::cout << "\n\n********************** LOW MEMORY [" << e.what() << "] **********************\n\n";
        retCode = -1;
    }
    catch (const std::exception &e)
    {
        std::cout << "\n\n********************** Exception [" << e.what() << "] **********************\n\n";
        retCode = -2;
    }
    catch (...)
    {
        std::cout << "\n\n********************** Unhandled Exception **********************\n\n";
        retCode = -3;
    }

    /* Exit the application */
    return retCode;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   The application exits with this function
 */
static void deInitUIApp(int)
{
    DeInitUiLogServer();
    DELETE_OBJ(m_window);
    DeInitDecDispLib ();
    KillProcess((CHARPTR)APPL_NAME, SIG_USR);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Set FD Resource Limit of GUI Application. Bydefault any application can open 1024 FDs
 *          but for rockchip, GUI application required quite more than 1024 FDs. Hence we have to
 *          set this limit during initialization process.
 */
static void setResourceLimit(void)
{
    struct rlimit resourceLimit;

    /* Get Current FD Limit */
    if (getrlimit(RLIMIT_NOFILE, &resourceLimit))
    {
        EPRINT(GUI_SYS, "fail to get max fd limit: [err=%s]", strerror(errno));
        return;
    }

    /* Set Required FDs Count */
    resourceLimit.rlim_cur = 2048;

    /* Set New FD Limit */
    if (setrlimit(RLIMIT_NOFILE, &resourceLimit))
    {
        EPRINT(GUI_SYS, "fail to set max fd limit: [err=%s]", strerror(errno));
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
