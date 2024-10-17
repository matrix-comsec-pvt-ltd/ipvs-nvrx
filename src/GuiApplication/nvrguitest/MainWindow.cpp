#include "MainWindow.h"

#include <unistd.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include <QStringList>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QFontDatabase>
#include <sys/stat.h>
#include <QDir>

#include "Enumfile.h"
#include "CommonApi.h"
#include "MxGpioDrv.h"

#if defined(RK3568_NVRL)
#include "watchdog.h"
#endif

#ifndef WATCHDOG_DEVICE_NAME
#define WATCHDOG_DEVICE_NAME        "/dev/watchdog"
#endif

#if defined(OEM_JCI)
#define MAC_PREFIX                  "00:50:F9"
#else
#define MAC_PREFIX                  "00:1b:09"
#endif

#define ENTERPRISE_LOGO_IMAGEPATH   ":/Logo/Logo.png"
#define TEST_APP_HEADER_TEXT        ENTERPRISE_STRING " %1 Test Application - V%2R%3"
#define SHUTDOWN_IMAGEPATH          IMAGE_PATH "/Shutdown/"
#define MAX_MANUAL_TIMER_TIMEOUT    (20)

#define MAX_MAC_ADDRESS_WIDTH		18
#define MAX_MAC_ADDR_BYTES			6
#define MAC_DISP_STRING             "%02x:%02x:%02x:%02x:%02x:%02x"
#define SHUTDOWN_STRING             "Shutting Down..."
#define INVALID_BOARD_TYPE_ERR_MSG  "The board type is invalid"
#define INVALID_FIRMWARE_ERR_MSG    "Firmware mismatch detected. Please load the correct firmware or contact support"

typedef enum
{
    #if defined(RK3568_NVRL)
    NVR_MODEL_0801X     = 0,
    NVR_MODEL_1601X     = 1,
    NVR_MODEL_1602X     = 2,
    NVR_MODEL_0801XS    = 3,
    NVR_MODEL_1601XS    = 4,
    NVR_MODEL_0401XS    = 5,
    #elif defined(RK3588_NVRH)
    #if defined(OEM_JCI)
    NVR_MODEL_HRIN_1208 = 0,
    NVR_MODEL_HRIN_2808 = 1,
    NVR_MODEL_HRIN_4808 = 2,
    NVR_MODEL_HRIN_6408 = 3,
    #else
    NVR_MODEL_3202X     = 0,
    NVR_MODEL_3204X     = 1,
    NVR_MODEL_6404X     = 2,
    NVR_MODEL_6408X     = 3,
    NVR_MODEL_9608X     = 4,
    #endif
    #else
    NVR_MODEL_0801X     = 0,
    NVR_MODEL_1601X     = 1,
    NVR_MODEL_1602X     = 2,
    NVR_MODEL_3202X     = 3,
    NVR_MODEL_3204X     = 4,
    NVR_MODEL_6404X     = 5,
    NVR_MODEL_6408X     = 6,
    #endif
    NVR_MODEL_MAX
}NVR_MODEL_e;

typedef enum
{
    LAN1_PORT,
    LAN2_PORT,
    MAX_LAN_PORT
}LAN_CONFIG_e;

VARIANT_e   BoardTypeWiseInfo::productVariant = VARIANT_MAX;
quint8      BoardTypeWiseInfo::noOfUsb = 0;
quint8      BoardTypeWiseInfo::noOfHdd = 0;
quint8      BoardTypeWiseInfo::noOfLan = 0;
quint8      BoardTypeWiseInfo::noOfSensorInOut = 0;
bool        BoardTypeWiseInfo::isAudioInOutSupport = true;
bool        BoardTypeWiseInfo::isExtRTC = true;
bool        BoardTypeWiseInfo::isMultipAvail = true;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setObjectName("MainWindow");

    INIT_OBJ(m_mainHeaderRect);
    INIT_OBJ(m_LeftPanelRect);
    INIT_OBJ(m_rightPanelRect);
    INIT_OBJ(m_matrixLogo);
    INIT_OBJ(m_applicationHeading);
    INIT_OBJ(m_shutDownImage);
    //Left Panel
    INIT_OBJ(m_networkTimeHeading);
    INIT_OBJ(m_ntpServerIpAddr);
    INIT_OBJ(m_setRtcBtn);
    INIT_OBJ(m_rtcUpdatedLable);
    INIT_OBJ(m_currentTimeLable);
    INIT_OBJ(m_reportBackupHeading);
    INIT_OBJ(m_ipAddr);
    INIT_OBJ(m_portParam);
    INIT_OBJ(m_portTextBox);
    INIT_OBJ(m_ftpUserNameParam);
    INIT_OBJ(m_ftpUserNameTextBox);
    INIT_OBJ(m_ftpPasswordParam);
    INIT_OBJ(m_ftpPasswordTextBox);
    INIT_OBJ(m_saveBtn);
    INIT_OBJ(m_cancelBtn);
    INIT_OBJ(m_manualBackupHeading);
    INIT_OBJ(m_ftpDriveBtn);
    INIT_OBJ(m_usbDriveBtn);
    INIT_OBJ(m_sysTimeLable);
    INIT_OBJ(m_sysTimeTimer);
    // Right Panel
    INIT_OBJ(m_devSerialNoParam);
    INIT_OBJ(m_devSerialNoTextBox);
    INIT_OBJ(m_testedByParam);
    INIT_OBJ(m_testedByTextBox);
    INIT_OBJ(m_startTestBtn);
    INIT_OBJ(m_viewReportBtn);
    INIT_OBJ(m_popUpAlert);
    INIT_OBJ(m_functionalTestHeading);
    INIT_OBJ(m_automatedTestHeading);
    INIT_OBJ(m_manualTestPassBtn);
    INIT_OBJ(m_manualTestFailBtn);
    INIT_OBJ(m_manualTestTimer);

    INIT_OBJ(runFuncTestThreadLed);
    INIT_OBJ(runFuncTestThreadBuzzer);
    INIT_OBJ(runFuncTestThreadAudio);
    INIT_OBJ(runAutoTestThread);
    INIT_OBJ(mainTimer);
    INIT_OBJ(m_generateReport);
    INIT_OBJ(udevMonitorIndex);
    INIT_OBJ(m_rtcDisplay);
    INIT_OBJ(m_fileUpload);
    INIT_OBJ(m_messageBanner);

    m_noOfBlockCompleted = 0;
    m_totalHwTestBlock = 0;
    m_manuallyFailedCount = 0;
    m_totalManualTestCnt = 0;
    m_isShowPopupAlert = false;

    QFontDatabase::addApplicationFont(":/fonts/Fonts/GOTHIC.TTF");
    QFontDatabase::addApplicationFont(":/fonts/Fonts/GOTHICB.TTF");
    QFontDatabase::addApplicationFont(":/fonts/Fonts/GOTHICBI.TTF");
    QFontDatabase::addApplicationFont(":/fonts/Fonts/GOTHICI.TTF");

    qRegisterMetaType<HW_TEST_BLOCK_e>("HW_TEST_BLOCK_e");

    this->setEnabled(true);
    this->setMouseTracking(true);
    this->setGeometry(QApplication::desktop()->geometry());
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    QApplication::setActiveWindow(this);

    Utils_ExeCmd((CHARPTR)"rm -rf " MOUNT_PATH "*");

    if(!QDir(LOG_DIR_PATH).exists())
    {
        QDir().mkdir(LOG_DIR_PATH);
    }

    for(quint8 hwIndex = 0; hwIndex < MAX_TEST_HW_CONDUCT; hwIndex++)
    {
        HardwareTestControl::testResult[hwIndex] = HW_TEST_PASS;
    }

    UpdateBoardTyeAndVer();
    getMacAddress();

    /* Validate MAC address prefix and ensure a valid board type */
    if ((HardwareTestControl::macAdd1.startsWith(MAC_PREFIX) == false) || (BoardTypeWiseInfo::productVariant == VARIANT_MAX))
    {
        m_mainHeaderRect = new Rect(0, 0, this->width(), this->height(), HEADER_BG_COLOR, this);

        m_applicationHeading = new TextLabel((this->width()/2),
                                             (this->height()/2),
                                             30,
                                             (BoardTypeWiseInfo::productVariant == VARIANT_MAX) ? INVALID_BOARD_TYPE_ERR_MSG : INVALID_FIRMWARE_ERR_MSG,
                                             m_mainHeaderRect,
                                             MAIN_HEADING_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_CENTRE_X_START_Y,
                                             0,
                                             false);
        m_applicationHeading->SetBold(true);
        m_sysTimeTimer = new QTimer(this);

        connect (m_sysTimeTimer,
                 SIGNAL(timeout()),
                 this,
                 SLOT(slotUpdateSysTime()));

        m_sysTimeTimer->setInterval (1000);

        showFullScreen();
        return;
    }
    else
    {
        createDefaultComponents();
    }

    quint8 retryCnt = 0;
    while (FALSE == getLan1State())
    {
        if (++retryCnt >= 10)
        {
            EPRINT(GUI_SYS, "lan1 is not up till timeout");
            break;
        }

        WPRINT(GUI_SYS, "lan1 is not up, waiting for it: [retryCnt=%d]", retryCnt);
        sleep(1);
    }

    m_rtcDisplay = new RTCDisplay();
    if(IS_VALID_OBJ(m_rtcDisplay) && (IS_VALID_OBJ(m_rtcUpdatedLable)))
    {
        quint8 rtcUpdateValue = m_rtcDisplay->getSntpTime();
        if (rtcUpdateValue == 0)
        {
            m_rtcUpdatedLable->changeText("RTC Updated");
        }
        else if (rtcUpdateValue == 1)
        {
            m_rtcUpdatedLable->changeText("NTP Failed");
        }
        else
        {
            m_rtcUpdatedLable->changeText("Failed to\nupdate RTC");
        }
    }

    m_sysTime = QDateTime::currentDateTime();
    m_sysTime = m_sysTime.addSecs(19800);

    if(IS_VALID_OBJ(m_sysTimeTimer))
    {
        m_sysTimeTimer->start();
    }

    QString tempStr =  QString("%1/%2/%3 %4:%5:%6")
            .arg(m_sysTime.date().day(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.date().month(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.date().year(),4,10,QLatin1Char('0'))
            .arg(m_sysTime.time().hour(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.time().minute(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.time().second(),2,10,QLatin1Char('0'));

    m_sysTimeLable->changeText(tempStr);
    m_sysTimeLable->update();

    m_fileUpload = new FileUpload();
    if (IS_VALID_OBJ(m_fileUpload))
    {
        connect (m_fileUpload,
                 SIGNAL(usbBkpRespRcvd(bool)),
                 this,
                 SLOT(slotUsbBkp(bool)));

        connect (m_fileUpload,
                 SIGNAL(ftpWriteStatus(bool)),
                 this,
                 SLOT(slotFtpBkp(bool)));
    }

    m_generateReport = new GenrateReport();

    udevMonitorIndex = udevMonitor::getInstance();

    showFullScreen();

    m_countWdUpdate = 0;
    m_watchDogFd = open(WATCHDOG_DEVICE_NAME, (O_WRONLY | O_CLOEXEC));
    if (m_watchDogFd == INVALID_FILE_FD)
    {
        EPRINT(GUI_SYS, "failed to open watchdog driver: [err=%s]", strerror(errno));
    }
}

MainWindow::~MainWindow()
{
    if (IS_VALID_OBJ(mainTimer))
    {
        if (IS_VALID_OBJ(runAutoTestThread))
        {
            disconnect (mainTimer,
                        SIGNAL(timeout()),
                        runAutoTestThread,
                        SLOT(slotShutDown()));
        }

        if (IS_VALID_OBJ(runFuncTestThreadLed))
        {
            disconnect (mainTimer,
                        SIGNAL(timeout()),
                        runFuncTestThreadLed,
                        SLOT(slotShutDown()));
        }

        if (IS_VALID_OBJ(runFuncTestThreadBuzzer))
        {
            disconnect (mainTimer,
                        SIGNAL(timeout()),
                        runFuncTestThreadBuzzer,
                        SLOT(slotShutDown()));
        }

        if (IS_VALID_OBJ(runFuncTestThreadAudio))
        {
            disconnect (mainTimer,
                        SIGNAL(timeout()),
                        runFuncTestThreadAudio,
                        SLOT(slotShutDown()));
        }

        disconnect (mainTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotTimeOut()));

        DELETE_OBJ(mainTimer);
    }

    if((IS_VALID_OBJ(runAutoTestThread)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        disconnect(runAutoTestThread,
                   SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                   this,
                   SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        disconnect(m_shutDownImage,
                   SIGNAL(sigImageClicked(int)),
                   runAutoTestThread,
                   SLOT(slotShutDown(int)));

        DELETE_OBJ(runAutoTestThread);
    }

    if((IS_VALID_OBJ(runFuncTestThreadLed)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        disconnect(runFuncTestThreadLed,
                   SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                   this,
                   SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        disconnect(m_shutDownImage,
                   SIGNAL(sigImageClicked(int)),
                   runFuncTestThreadLed,
                   SLOT(slotShutDown(int)));

        disconnect(runFuncTestThreadLed,
                   SIGNAL(sigCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                   this,
                   SLOT(slotCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)));

        DELETE_OBJ(runFuncTestThreadLed);
    }

    if((IS_VALID_OBJ(runFuncTestThreadBuzzer)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        disconnect(runFuncTestThreadBuzzer,
                   SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                   this,
                   SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        disconnect(m_shutDownImage,
                   SIGNAL(sigImageClicked(int)),
                   runFuncTestThreadBuzzer,
                   SLOT(slotShutDown(int)));

        disconnect(runFuncTestThreadBuzzer,
                   SIGNAL(sigCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                   this,
                   SLOT(slotCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)));

        DELETE_OBJ(runFuncTestThreadBuzzer);
    }

    if((IS_VALID_OBJ(runFuncTestThreadAudio)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        disconnect(runFuncTestThreadAudio,
                   SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                   this,
                   SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        disconnect(m_shutDownImage,
                   SIGNAL(sigImageClicked(int)),
                   runFuncTestThreadAudio,
                   SLOT(slotShutDown(int)));

        disconnect(runFuncTestThreadAudio,
                   SIGNAL(sigCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                   this,
                   SLOT(slotCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)));

        DELETE_OBJ(runFuncTestThreadAudio);
    }

    DELETE_OBJ(m_mainHeaderRect);
    DELETE_OBJ(m_LeftPanelRect);
    DELETE_OBJ(m_rightPanelRect);
    DELETE_OBJ(m_matrixLogo);
    DELETE_OBJ(m_applicationHeading);

    if(IS_VALID_OBJ(m_shutDownImage))
    {
        disconnect(m_shutDownImage,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotShutDownClicked(int)));
        DELETE_OBJ(m_shutDownImage);
    }

    //Left Panel
    DELETE_OBJ(m_networkTimeHeading);
    DELETE_OBJ(m_ntpServerIpAddr);

    if(IS_VALID_OBJ(m_setRtcBtn))
    {
        disconnect(m_setRtcBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_setRtcBtn);
    }
    DELETE_OBJ(m_rtcUpdatedLable);
    DELETE_OBJ(m_currentTimeLable);
    DELETE_OBJ(m_reportBackupHeading);
    DELETE_OBJ(m_ipAddr);
    DELETE_OBJ(m_portParam);
    DELETE_OBJ(m_portTextBox);
    DELETE_OBJ(m_ftpUserNameParam);
    DELETE_OBJ(m_ftpUserNameTextBox);
    DELETE_OBJ(m_ftpPasswordParam);
    DELETE_OBJ(m_ftpPasswordTextBox);

    if(IS_VALID_OBJ(m_saveBtn))
    {
        disconnect(m_saveBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_saveBtn);
    }

    if(IS_VALID_OBJ(m_cancelBtn))
    {
        disconnect(m_cancelBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_cancelBtn);
    }

    DELETE_OBJ(m_manualBackupHeading);

    if(IS_VALID_OBJ(m_ftpDriveBtn))
    {
        disconnect(m_ftpDriveBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_ftpDriveBtn);
    }

    if(IS_VALID_OBJ(m_usbDriveBtn))
    {
        disconnect(m_usbDriveBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_usbDriveBtn);
    }

    DELETE_OBJ(m_sysTimeLable);

    if(IS_VALID_OBJ(m_sysTimeTimer))
    {
        m_sysTimeTimer->stop();
        disconnect (m_sysTimeTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotUpdateSysTime()));
        DELETE_OBJ(m_sysTimeTimer);
    }

    // Right Panel
    DELETE_OBJ(m_devSerialNoParam);
    DELETE_OBJ(m_devSerialNoTextBox);
    DELETE_OBJ(m_testedByParam);
    DELETE_OBJ(m_testedByTextBox);

    if(IS_VALID_OBJ(m_startTestBtn))
    {
        disconnect(m_startTestBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_startTestBtn);
    }

    if(IS_VALID_OBJ(m_viewReportBtn))
    {
        disconnect(m_viewReportBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_viewReportBtn);
    }

    DELETE_OBJ(m_popUpAlert);
    DELETE_OBJ(m_functionalTestHeading);
    DELETE_OBJ(m_automatedTestHeading);

    if(IS_VALID_OBJ(m_manualTestPassBtn))
    {
        disconnect(m_manualTestPassBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_manualTestPassBtn);
    }

    if(IS_VALID_OBJ(m_manualTestFailBtn))
    {
        disconnect(m_manualTestFailBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(m_manualTestFailBtn);
    }

    if(IS_VALID_OBJ(m_manualTestTimer))
    {
        m_manualTestTimer->stop();
        disconnect(m_manualTestTimer,
                   SIGNAL(timeout()),
                   this,
                   SLOT(slotManualTimeout()));
        DELETE_OBJ(m_manualTestTimer);
    }

    if (IS_VALID_OBJ(m_fileUpload))
    {
        disconnect (m_fileUpload,
                    SIGNAL(usbBkpRespRcvd(bool)),
                    this,
                    SLOT(slotUsbBkp(bool)));

        disconnect (m_fileUpload,
                    SIGNAL(ftpWriteStatus(bool)),
                    this,
                    SLOT(slotFtpBkp(bool)));
        DELETE_OBJ(m_fileUpload);
    }

    DELETE_OBJ(m_generateReport);
    DELETE_OBJ(m_rtcDisplay);
    DELETE_OBJ(udevMonitorIndex);

    if(IS_VALID_OBJ(m_messageBanner))
    {
        disconnect(m_messageBanner,
                   SIGNAL(sigLoadNextPage()),
                   this,
                   SLOT(slotLoadNextPage()));
        DELETE_OBJ(m_messageBanner);
    }
    m_countWdUpdate = 0;
}

void MainWindow::paintEvent (QPaintEvent *)
{

}

void MainWindow::createDefaultComponents ()
{
    m_mainHeaderRect = new Rect(0, 0, this->width(), 90, HEADER_BG_COLOR, this);

    m_LeftPanelRect = new Rect(0,
                               m_mainHeaderRect->height(),
                               420,
                               (this->height() - m_mainHeaderRect->height()),
                               LEFT_PANEL_BG_COLOR,
                               this);


    m_rightPanelRect = new Rect(m_LeftPanelRect->width(),
                                m_mainHeaderRect->height(),
                                (this->width() - m_LeftPanelRect->width()),
                                (this->height() - m_mainHeaderRect->height()),
                                BODY_BG_COLOR,
                                this);

    m_matrixLogo = new Image(22,
                             22,
                             ENTERPRISE_LOGO_IMAGEPATH,
                             m_mainHeaderRect,
                             START_X_START_Y,
                             0,
                             false,
                             true);

    QString hardwareTestHeaderStr = QString(TEST_APP_HEADER_TEXT).arg(HardwareTestControl::boardTypeString)
            .arg(SOFTWARE_VERSION, 2, 10, QLatin1Char('0')).arg(SOFTWARE_REVISION, 2, 10, QLatin1Char('0'));
    if (PRODUCT_SUB_REVISION)
    {
        hardwareTestHeaderStr.append(QString(".%1").arg(PRODUCT_SUB_REVISION));
    }

    m_applicationHeading = new TextLabel((this->width()/2),
                                         28,
                                         18,
                                         hardwareTestHeaderStr,
                                         m_mainHeaderRect,
                                         MAIN_HEADING_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_CENTRE_X_START_Y,
                                         0,
                                         false);
    m_applicationHeading->SetBold(true);

    m_shutDownImage = new Image((m_mainHeaderRect->width() - 40),
                                (m_mainHeaderRect->height()/2),
                                SHUTDOWN_IMAGEPATH,
                                m_mainHeaderRect,
                                END_X_CENTER_Y,
                                0,
                                true);
    if(IS_VALID_OBJ(m_shutDownImage))
    {
        connect(m_shutDownImage,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotShutDownClicked(int)));
    }

    createLeftPanelComponents();
    createRightPanelComponents();

    m_messageBanner =  new MessageBanner(this);
    if(IS_VALID_OBJ(m_messageBanner))
    {
        connect(m_messageBanner,
                SIGNAL(sigLoadNextPage()),
                this,
                SLOT(slotLoadNextPage()));
    }    

    runFuncTestThreadBuzzer = new RunTest(HW_TEST_FUNCTION_BUZZER, this);
    if((IS_VALID_OBJ(runFuncTestThreadBuzzer)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        connect(runFuncTestThreadBuzzer,
                SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                this,
                SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        connect(m_shutDownImage,
                SIGNAL(sigImageClicked(int)),
                runFuncTestThreadBuzzer,
                SLOT(slotShutDown(int)));

        connect(runFuncTestThreadBuzzer,
                SIGNAL(sigCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                this,
                SLOT(slotCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                Qt::QueuedConnection);
    }

    runFuncTestThreadLed = new RunTest(HW_TEST_FUNCTION_LED, this);
    if((IS_VALID_OBJ(runFuncTestThreadLed)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        connect(runFuncTestThreadLed,
                SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                this,
                SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        connect(m_shutDownImage,
                SIGNAL(sigImageClicked(int)),
                runFuncTestThreadLed,
                SLOT(slotShutDown(int)));

        connect(runFuncTestThreadLed,
                SIGNAL(sigCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                this,
                SLOT(slotCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                Qt::QueuedConnection);
    }

    if (true == BoardTypeWiseInfo::isAudioInOutSupport)
    {
        runFuncTestThreadAudio = new RunTest(HW_TEST_FUNCTIONAL_OTHERS, this);
        if((IS_VALID_OBJ(runFuncTestThreadAudio)) && (IS_VALID_OBJ(m_shutDownImage)))
        {
            connect(runFuncTestThreadAudio,
                    SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                    this,
                    SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

            connect(m_shutDownImage,
                    SIGNAL(sigImageClicked(int)),
                    runFuncTestThreadAudio,
                    SLOT(slotShutDown(int)));

            connect(runFuncTestThreadAudio,
                    SIGNAL(sigCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                    this,
                    SLOT(slotCtrlBtnClicked(HW_TEST_BLOCK_e,int,int)),
                    Qt::QueuedConnection);
        }
    }

    runAutoTestThread = new RunTest(HW_TEST_AUTOMATED_ALL,this);
    if((IS_VALID_OBJ(runAutoTestThread)) && (IS_VALID_OBJ(m_shutDownImage)))
    {
        connect(runAutoTestThread,
                SIGNAL(sigTestCompelete(HW_TEST_BLOCK_e)),
                this,
                SLOT(slotTestCompelete(HW_TEST_BLOCK_e)));

        connect(m_shutDownImage,
                SIGNAL(sigImageClicked(int)),
                runAutoTestThread,
                SLOT(slotShutDown(int)));
    }

    mainTimer = new QTimer(this);
    if (IS_VALID_OBJ(mainTimer))
    {
        if (IS_VALID_OBJ(runAutoTestThread))
        {
            connect (mainTimer,
                     SIGNAL(timeout()),
                     runAutoTestThread,
                     SLOT(slotShutDown()));
        }

        if (IS_VALID_OBJ(runFuncTestThreadLed))
        {
            connect (mainTimer,
                     SIGNAL(timeout()),
                     runFuncTestThreadLed,
                     SLOT(slotShutDown()));
        }

        if (IS_VALID_OBJ(runFuncTestThreadBuzzer))
        {
            connect (mainTimer,
                     SIGNAL(timeout()),
                     runFuncTestThreadBuzzer,
                     SLOT(slotShutDown()));
        }

        if (IS_VALID_OBJ(runFuncTestThreadAudio))
        {
            connect (mainTimer,
                     SIGNAL(timeout()),
                     runFuncTestThreadAudio,
                     SLOT(slotShutDown()));
        }

        connect (mainTimer,
                 SIGNAL(timeout()),
                 this,
                 SLOT(slotTimeOut()));

        mainTimer->setInterval((true == BoardTypeWiseInfo::isAudioInOutSupport) ? 12000 : 6000);
    }

    m_manualTimerCount = 0;
    m_manualTestTimer = new QTimer(this);
    if(IS_VALID_OBJ(m_manualTestTimer))
    {
        connect(m_manualTestTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotManualTimeout()));
        m_manualTestTimer->setInterval(1000);
    }
}

void MainWindow::createLeftPanelComponents()
{
    m_networkTimeHeading = new TextLabel(10,
                                         20,
                                         SUFFIX_FONT_SIZE,
                                         QString("Network Time Settings"),
                                         m_LeftPanelRect,
                                         HEADING_FONT_COLOR,
                                         NORMAL_FONT_FAMILY);
    if(m_networkTimeHeading != NULL)
    {
        m_networkTimeHeading->SetBold(true);
    }

    m_ntpServerIpAddr = new Ipv4TextBox(20,
                                      (m_networkTimeHeading->y() + m_networkTimeHeading->height() + 30),
                                      m_LeftPanelRect->width(),
                                      50,
                                      0,
                                      QString("NTP Server IP Address"),
                                      m_LeftPanelRect,
                                      NO_LAYER);
    m_ntpServerIpAddr->setIpaddress(HardwareTestControl::ntpServerIpAddr);

    m_setRtcBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 (m_LeftPanelRect->width()/2 + 45),
                                 (m_ntpServerIpAddr->y() + m_ntpServerIpAddr->height() + 25),
                                 QString("Set RTC"),
                                 m_LeftPanelRect,
                                 SET_RTC);
    if(IS_VALID_OBJ(m_setRtcBtn))
    {
        connect(m_setRtcBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_rtcUpdatedLable = new TextLabel((m_setRtcBtn->x() + m_setRtcBtn->width()),
                                      (m_setRtcBtn->y() + 22),
                                      13,
                                      QString(""),
                                      m_LeftPanelRect,
                                      HEADING_FONT_COLOR,
                                      NORMAL_FONT_FAMILY);

    m_currentTimeLable = new TextLabel((m_setRtcBtn->x() - 2),
                                       (m_setRtcBtn->y() + m_setRtcBtn->height() + 12),
                                       SMALL_SUFFIX_FONT_SIZE,
                                       QString("Current Time"),
                                       m_LeftPanelRect,
                                       HEADING_FONT_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_END_X_CENTRE_Y);

    m_sysTimeLable = new TextLabel((m_setRtcBtn->x() + 12),
                                   (m_setRtcBtn->y() + m_setRtcBtn->height() + 12),
                                   SMALL_SUFFIX_FONT_SIZE,
                                   QString(""),
                                   m_LeftPanelRect,
                                   HEADING_FONT_COLOR,
                                   NORMAL_FONT_FAMILY,
                                   ALIGN_START_X_CENTRE_Y);

    m_sysTimeTimer = new QTimer(this);

    connect (m_sysTimeTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotUpdateSysTime()));

    m_sysTimeTimer->setInterval (1000);

    m_reportBackupHeading = new TextLabel(10,
                                         (m_setRtcBtn->y() + m_setRtcBtn->height() + 50),
                                         SUFFIX_FONT_SIZE,
                                         QString("Report Backup Settings"),
                                         m_LeftPanelRect,
                                         HEADING_FONT_COLOR,
                                         NORMAL_FONT_FAMILY);
    m_reportBackupHeading->SetBold(true);

    m_ipAddr = new Ipv4TextBox(110,
                             (m_reportBackupHeading->y() + m_reportBackupHeading->height() + 30),
                             m_LeftPanelRect->width(),
                             40,
                             0,
                             QString("IP Address"),
                             m_LeftPanelRect,
                             NO_LAYER);

    m_portParam = new TextboxParam();
    m_portParam->labelStr = QString("Port");
    m_portParam->isNumEntry = true;
    m_portParam->minNumValue = 1;
    m_portParam->maxNumValue = 65535;
    m_portParam->maxChar = 5;
    m_portParam->validation = QRegExp(QString("[0-9]"));

    m_portTextBox = new TextBox(161,
                               (m_ipAddr->y() + m_ipAddr->height() + 5),
                                BGTILE_SMALL_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                0,
                                TEXTBOX_LARGE,
                                m_LeftPanelRect,
                                m_portParam,
                                NO_LAYER);

    m_ftpUserNameParam = new TextboxParam();
    m_ftpUserNameParam->labelStr = QString("FTP Username");
    m_ftpUserNameParam->isCentre = true;
    m_ftpUserNameParam->isTotalBlankStrAllow = true;
    m_ftpUserNameParam->maxChar = 40;
    m_ftpUserNameParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_ftpUserNameTextBox = new TextBox(81,
                                       (m_portTextBox->y() + m_portTextBox->height() + 5),
                                       BGTILE_SMALL_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       0,
                                       TEXTBOX_LARGE,
                                       m_LeftPanelRect,
                                       m_ftpUserNameParam,
                                       NO_LAYER);

    m_ftpPasswordParam = new TextboxParam();
    m_ftpPasswordParam->labelStr = QString("FTP Password");
    m_ftpPasswordParam->isCentre =  true;
    m_ftpPasswordParam->isTotalBlankStrAllow = true;
    m_ftpPasswordParam->maxChar  = 24;
    m_ftpPasswordParam->minChar = 4;
    m_ftpPasswordParam->validation = QRegExp(asciiset1ValidationStringWithoutSpace);

    m_ftpPasswordTextBox = new PasswordTextbox(87,
                                               (m_ftpUserNameTextBox->y() + m_ftpUserNameTextBox->height() + 5),
                                               BGTILE_SMALL_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               0,
                                               TEXTBOX_LARGE,
                                               m_LeftPanelRect,
                                               m_ftpPasswordParam,
                                               NO_LAYER);

    m_saveBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 (m_LeftPanelRect->width()/2 - 60),
                                 (m_ftpPasswordTextBox->y() + m_ftpPasswordTextBox->height() + 35),
                                 QString("Save"),
                                 m_LeftPanelRect,
                                 SAVE_FTP);
    if(IS_VALID_OBJ(m_saveBtn))
    {
        connect(m_saveBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_cancelBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 (m_LeftPanelRect->width()/2 + 60),
                                 (m_ftpPasswordTextBox->y() + m_ftpPasswordTextBox->height() + 35),
                                 QString("Cancel"),
                                 m_LeftPanelRect,
                                 CANCEL_FTP);
    if(IS_VALID_OBJ(m_cancelBtn))
    {
        connect(m_cancelBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_manualBackupHeading = new TextLabel(10,
                                         (m_saveBtn->y() + m_saveBtn->height() + 25),
                                         SUFFIX_FONT_SIZE,
                                         QString("Manual Backup"),
                                         m_LeftPanelRect,
                                         HEADING_FONT_COLOR,
                                         NORMAL_FONT_FAMILY);
    m_manualBackupHeading->SetBold(true);

    m_ftpDriveBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                   100,
                                   (m_manualBackupHeading->y() + m_manualBackupHeading->height() + 35),
                                   QString("FTP Drive"),
                                   m_LeftPanelRect,
                                   FTP_DRIVE);
    if(IS_VALID_OBJ(m_ftpDriveBtn))
    {
        connect(m_ftpDriveBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_usbDriveBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                   100,
                                   (m_ftpDriveBtn->y() + m_ftpDriveBtn->height() + 15),
                                   QString("USB Drive"),
                                   m_LeftPanelRect,
                                   USB_DRIVE);
    if(IS_VALID_OBJ(m_usbDriveBtn))
    {
        connect(m_usbDriveBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    setReportBackupSettingFieldValue();
}

void MainWindow::createRightPanelComponents()
{
    m_devSerialNoParam = new TextboxParam();
    m_devSerialNoParam->labelStr = QString("Device Serial Number");
    m_devSerialNoParam->isCentre = true;
    m_devSerialNoParam->isTotalBlankStrAllow = true;
    m_devSerialNoParam->maxChar = 15;
    m_devSerialNoParam->validation = QRegExp(QString("[0-9]"));

    m_devSerialNoTextBox = new TextBox(20,
                                       20,
                                       BGTILE_SMALL_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       0,
                                       TEXTBOX_MEDIAM,
                                       m_rightPanelRect,
                                       m_devSerialNoParam,
                                       NO_LAYER);

    m_testedByParam = new TextboxParam();
    m_testedByParam->labelStr = QString("Tested By");
    m_testedByParam->isCentre = true;
    m_testedByParam->isTotalBlankStrAllow = true;
    m_testedByParam->maxChar = 8;
    m_testedByParam->validation = QRegExp(QString("[0-9]"));

    m_testedByTextBox = new TextBox((m_devSerialNoTextBox->x() + m_devSerialNoTextBox->width() + 30),
                                    20,
                                    BGTILE_SMALL_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    0,
                                    TEXTBOX_SMALL,
                                    m_rightPanelRect,
                                    m_testedByParam,
                                    NO_LAYER);

    m_startTestBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                    (m_testedByTextBox->x() + m_testedByTextBox->width() + 82),
                                    40,
                                    QString("Start Test"),
                                    m_rightPanelRect,
                                    START_RESTART_TEST);
    if(IS_VALID_OBJ(m_startTestBtn))
    {
        connect(m_startTestBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_viewReportBtn = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                     (m_startTestBtn->x() + m_startTestBtn->width() + 120),
                                     40,
                                     QString("View Report"),
                                     m_rightPanelRect,
                                     VIEW_REPORT,
                                     false);
    if(IS_VALID_OBJ(m_viewReportBtn))
    {
        connect(m_viewReportBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_seperatorLine = new Rect(400,
                               70,
                               2,
                               (this->height() - 70),
                               SEPERATOR_LINE_COLOR,
                               m_rightPanelRect);

    m_functionalTestHeading = new TextLabel(m_devSerialNoTextBox->x(),
                                            (m_seperatorLine->y() + 25),
                                            SUFFIX_FONT_SIZE,
                                            QString("FUNCTIONAL TEST"),
                                            m_rightPanelRect,
                                            QString("#898989"),
                                            NORMAL_FONT_FAMILY);
    m_functionalTestHeading->SetBold(true);

    m_automatedTestHeading = new TextLabel((m_devSerialNoTextBox->x() + m_seperatorLine->x() + 5),
                                            (m_seperatorLine->y() + 25),
                                            SUFFIX_FONT_SIZE,
                                            QString("AUTOMATED TEST"),
                                            m_rightPanelRect,
                                            QString("#898989"),
                                            NORMAL_FONT_FAMILY);
    m_automatedTestHeading->SetBold(true);

    m_manualTestPassBtn = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                         (140),
                                         (m_rightPanelRect->height() - 50),
                                         QString("Generate Report"),
                                         m_rightPanelRect,
                                         MANUAL_PASS_BTN,
                                         false);
    if(IS_VALID_OBJ(m_manualTestPassBtn))
    {
        connect(m_manualTestPassBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_manualTestFailBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                         (310),
                                         (m_rightPanelRect->height() - 50),
                                         QString("Fail"),
                                         m_rightPanelRect,
                                         MANUAL_FAIL_BTN,
                                         false);
    if(IS_VALID_OBJ(m_manualTestFailBtn))
    {
        connect(m_manualTestFailBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    }

    m_manualTimerLable = new TextLabel((200),
                                       (m_manualTestFailBtn->y() - 20),
                                       SUFFIX_FONT_SIZE,
                                       QString("Take action within %1s").arg(MAX_MANUAL_TIMER_TIMEOUT),
                                       m_rightPanelRect,
                                       QString("#898989"),
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_CENTER_Y);
    m_manualTimerLable->setVisible(false);
}

void MainWindow::slotTestCompelete(HW_TEST_BLOCK_e testBlock)
{
	DPRINT(GUI_SYS, "HWTest Completed for Block-%d", testBlock);
    m_noOfBlockCompleted++;
    if(m_noOfBlockCompleted >= m_totalHwTestBlock)
    {
        m_manualTestPassBtn->setIsEnabled(true);
        m_manualTestFailBtn->setIsEnabled(true);
        m_manualTimerLable->setVisible(true);
        m_manualTestTimer->start();
    }
}

void MainWindow::slotButtonClick(int index)
{
    quint8 retVal;

    switch(index)
    {
    case SET_RTC:

        if(IS_VALID_OBJ(m_ntpServerIpAddr))
        {
            HardwareTestControl::ntpServerIpAddr = m_ntpServerIpAddr->getIpaddress();
        }

        if(IS_VALID_OBJ(m_rtcDisplay))
        {
            retVal = m_rtcDisplay->getSntpTime();

            if(IS_VALID_OBJ(m_rtcUpdatedLable))
            {
                if( retVal == 0 )
                {
                    m_rtcUpdatedLable->changeText("RTC Updated");
                }
                else if( retVal == 1 )
                {
                    m_rtcUpdatedLable->changeText("NTP Failed");
                }
                else
                {
                    m_rtcUpdatedLable->changeText("Failed to\nupdate RTC");
                }
            }

            m_sysTime = QDateTime::currentDateTime();
            m_sysTime = m_sysTime.addSecs(19800);

            if(IS_VALID_OBJ(m_sysTimeTimer))
            {
                m_sysTimeTimer->stop();
                m_sysTimeTimer->start();
            }

            QString tempStr =  QString("%1/%2/%3 %4:%5:%6")
                    .arg(m_sysTime.date().day(),2,10,QLatin1Char('0'))
                    .arg(m_sysTime.date().month(),2,10,QLatin1Char('0'))
                    .arg(m_sysTime.date().year(),4,10,QLatin1Char('0'))
                    .arg(m_sysTime.time().hour(),2,10,QLatin1Char('0'))
                    .arg(m_sysTime.time().minute(),2,10,QLatin1Char('0'))
                    .arg(m_sysTime.time().second(),2,10,QLatin1Char('0'));

            m_sysTimeLable->changeText(tempStr);
            m_sysTimeLable->update();

            /* sntp is blocking work. Kick watchdog to avoid system reboot */
            kickWatchdog();
        }
        break;

    case SAVE_FTP:
        setReportBackupSettingParam();
        break;

    case CANCEL_FTP:
        setReportBackupSettingFieldValue();
        break;

    case FTP_DRIVE:
        enableDisableElements(false);
        backUpToFTPUSBDrive(FTP_DRIVE);
        break;

    case USB_DRIVE:
        enableDisableElements(false);
        backUpToFTPUSBDrive(USB_DRIVE);
        break;

    case START_RESTART_TEST:
        if(m_devSerialNoTextBox->getInputText() == "")
        {
            m_messageBanner->loadInfoMessage ("Please enter Device Serial Number");
        }
        else if(m_testedByTextBox->getInputText() == "")
        {
            m_messageBanner->loadInfoMessage ("Please enter Employee ID");
        }
        else
        {
            if(IS_VALID_OBJ(m_devSerialNoTextBox))
            {
                 m_devSerialNoTextBox->getInputText(HardwareTestControl::deviceSerialNumber);
            }

            if(IS_VALID_OBJ(m_testedByTextBox))
            {
                 m_testedByTextBox->getInputText(HardwareTestControl::testEmpId);
            }

            m_manuallyFailedCount = 0;
            if(IS_VALID_OBJ(m_manualTestFailBtn))
            {
                m_manualTestFailBtn->changeColor(DISABLE_FONT_COLOR);
                m_manualTestFailBtn->update();
            }

            enableDisableElements(false);
            showHidePopupAlert(false);

            if(IS_VALID_OBJ(m_rtcDisplay))
            {
                m_rtcDisplay->getLocalTime(&HardwareTestControl::testStartTime);
            }

            HardwareTestControl::testStartTime.tm_mon += 1;
            HardwareTestControl::exitTest = true;

            m_totalHwTestBlock = 0;
            m_totalManualTestCnt = 0;
            if (IS_VALID_OBJ(mainTimer))
            {
                mainTimer->start();
            }

            if (IS_VALID_OBJ(runFuncTestThreadLed))
            {
                m_totalHwTestBlock++;
                m_totalManualTestCnt++;
                runFuncTestThreadLed->changeTestIndicatorState(MAX_HW_TEST_RESULT);
                runFuncTestThreadLed->start();
            }

            if (IS_VALID_OBJ(runFuncTestThreadBuzzer))
            {
                m_totalHwTestBlock++;
                m_totalManualTestCnt++;
                runFuncTestThreadBuzzer->changeTestIndicatorState(MAX_HW_TEST_RESULT);
                runFuncTestThreadBuzzer->start();
            }

            if (IS_VALID_OBJ(runAutoTestThread))
            {
                m_totalHwTestBlock++;
                runAutoTestThread->changeTestIndicatorState(MAX_HW_TEST_RESULT);
                runAutoTestThread->start();
            }

            if (IS_VALID_OBJ(runFuncTestThreadAudio))
            {
                m_totalHwTestBlock++;
                m_totalManualTestCnt += 3;
                runFuncTestThreadAudio->changeTestIndicatorState(MAX_HW_TEST_RESULT);
                runFuncTestThreadAudio->start();
            }
        }
        break;

    case VIEW_REPORT:
        showReport();
        break;

    case MANUAL_PASS_BTN:
        doManualBtnAction(true, true);
        break;

    case MANUAL_FAIL_BTN:
        doManualBtnAction(false, true);
        if(IS_VALID_OBJ(m_manualTestFailBtn))
        {
            m_manualTestFailBtn->changeColor(RED_COLOR);
            m_manualTestFailBtn->update();
        }
        break;

    default:
        break;
    }
}

void MainWindow::setReportBackupSettingFieldValue()
{
    if(IS_VALID_OBJ(m_ipAddr))
    {
        m_ipAddr->setIpaddress(HardwareTestControl::ftpServerIpAddr);
    }

    if(IS_VALID_OBJ(m_portTextBox))
    {
        m_portTextBox->setInputText(HardwareTestControl::ftpPort);
    }

    if(IS_VALID_OBJ(m_ftpUserNameTextBox))
    {
        m_ftpUserNameTextBox->setInputText(HardwareTestControl::ftpUsername);
    }

    if(IS_VALID_OBJ(m_ftpPasswordTextBox))
    {
        m_ftpPasswordTextBox->setInputText(HardwareTestControl::ftpPassword);
    }
}

void MainWindow::setReportBackupSettingParam()
{
    if(m_ipAddr->getCurrentIpAddress () == "")
    {
        m_messageBanner->loadInfoMessage ("Please enter IP Address");
    }
    else if(m_portTextBox->getInputText () == "")
    {
        m_messageBanner->loadInfoMessage ("Please enter FTP Port");
    }
    else if(m_ftpUserNameTextBox->getInputText () == "")
    {
         m_messageBanner->loadInfoMessage ("Please enter FTP Username");
    }
    else if(m_ftpPasswordTextBox->getInputText () == "")
    {
         m_messageBanner->loadInfoMessage ("Please enter FTP Password");
    }
    else
    {
        HardwareTestControl::ftpServerIpAddr=m_ipAddr->getCurrentIpAddress();
        HardwareTestControl::ftpPort=m_portTextBox->getInputText();
        HardwareTestControl::ftpUsername=m_ftpUserNameTextBox->getInputText();
        HardwareTestControl::ftpPassword=m_ftpPasswordTextBox->getInputText();
        setReportBackupSettingFieldValue();
    }
}

void MainWindow::backUpToFTPUSBDrive(CNFG_BTN_e index)
{
    char sysCommand[SYSTEM_COMMAND_SIZE];

    QString filePath = "/" TEST_REPORT_FOLDER "/" + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
            + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".pdf");

    QString dirPath = "/" TEST_REPORT_FOLDER "/";

    switch(index)
    {
        case FTP_DRIVE:
        {
            if (IS_VALID_OBJ(m_generateReport))
            {
                m_generateReport->generatePdfFile();
            }
            else
            {
                EPRINT(GUI_SYS, "invld object[m_generateReport]");
            }

            QString ftpUrl = QString("ftp://") +
                    HardwareTestControl::ftpUsername + QString(":") +
                    HardwareTestControl::ftpPassword + QString("@") +
                    HardwareTestControl::ftpServerIpAddr + filePath ;

            QString dirUrl = QString(" ftp://") +
                    HardwareTestControl::ftpUsername + QString(":") +
                    HardwareTestControl::ftpPassword + QString("@") +
                    HardwareTestControl::ftpServerIpAddr + dirPath;

            snprintf(sysCommand, sizeof(sysCommand), "curl --connect-timeout 3 --ftp-create-dirs %s", dirUrl.toUtf8().constData());
            if (FAIL == Utils_ExeCmd(sysCommand))
            {
                EPRINT(GUI_SYS, "fail to create ftp directory for report upload");
                slotFtpBkp(false);
            }
            else
            {
                m_fileUpload->upLoadFileToFtp (ftpUrl);
            }
        }
        break;

        case USB_DRIVE:
        {
            if (IS_VALID_OBJ(m_generateReport))
            {
                m_generateReport->generatePdfFile();
            }
            else
            {
                EPRINT(GUI_SYS, "invld object[m_generateReport]");
            }

            m_fileUpload->upLoadFileToUsb ();
        }
        break;

        default:
            break;
    }
}

bool MainWindow::UpdateBoardTyeAndVer()
{
    INT32	gpioDrvFd = INVALID_CONNECTION;
    UINT32  boardType = 0;
    bool 	status = true;
    UINT32  boardVersion = 0;

    do
    {
        gpioDrvFd = open(GPIO_DEVICE_NAME, O_RDWR | O_CLOEXEC);
        if(gpioDrvFd < 0)
        {
            status = false;
            EPRINT(GUI_SYS, "fail to open gpio driver: [err=%s]", strerror(errno));
            break;
        }

        if(ioctl(gpioDrvFd, MXGPIO_GET_CARD_TYPE, &boardType) < 0)
        {
            status = false;
            EPRINT(GUI_SYS, "ioctl MXGPIO_GET_CARD_TYPE failed");
            ::close(gpioDrvFd);
            break;
        }

        if(ioctl(gpioDrvFd, MXGPIO_GET_BOARD_VERSION, &boardVersion) < STATUS_OK)
        {
            status = false;
            EPRINT(GUI_SYS, "ioctl MXGPIO_GET_BOARD_VERSION failed");
        }

        ::close(gpioDrvFd);
        DPRINT(GUI_SYS, "board info: [boardType=%d], [boardVersion=%d]", boardType, boardVersion);
        switch(boardType)
        {
            #if defined(RK3568_NVRL)
            case NVR_MODEL_0801X:
                BoardTypeWiseInfo::productVariant = NVR0801XP2;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR0801X P2";
                break;

            case NVR_MODEL_1601X:
                BoardTypeWiseInfo::productVariant = NVR1601XP2;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR1601X P2";
                break;

            case NVR_MODEL_1602X:
                BoardTypeWiseInfo::productVariant = NVR1602XP2;
                BoardTypeWiseInfo::noOfHdd = 2;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR1602X P2";
                break;

            case NVR_MODEL_0801XS:
                BoardTypeWiseInfo::productVariant = NVR0801XSP2;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR0801XS P2";
                break;

            case NVR_MODEL_1601XS:
                BoardTypeWiseInfo::productVariant = NVR1601XSP2;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR1601XS P2";
                break;

            case NVR_MODEL_0401XS:
                BoardTypeWiseInfo::productVariant = NVR0401XSP2;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR0401XS P2";
                break;

            #elif defined(RK3588_NVRH)
            #if defined(OEM_JCI)
            case NVR_MODEL_HRIN_1208:
                BoardTypeWiseInfo::productVariant = HRIN_1208_18_SR;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "HRIN-1208-18-SR";
                break;

            case NVR_MODEL_HRIN_2808:
                BoardTypeWiseInfo::productVariant = HRIN_2808_18_SR;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "HRIN-2808-18-SR";
                break;

            case NVR_MODEL_HRIN_4808:
                BoardTypeWiseInfo::productVariant = HRIN_4808_18_SR;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "HRIN-4808-18-SR";
                break;

            case NVR_MODEL_HRIN_6408:
                BoardTypeWiseInfo::productVariant = HRIN_6408_18_SR;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "HRIN-6408-18-SR";
                break;
            #else
            case NVR_MODEL_3202X:
                BoardTypeWiseInfo::productVariant = NVR3202XP2;
                BoardTypeWiseInfo::noOfHdd = 2;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR3202X P2";
                break;

            case NVR_MODEL_3204X:
                BoardTypeWiseInfo::productVariant = NVR3204XP2;
                BoardTypeWiseInfo::noOfHdd = 4;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR3204X P2";
                break;

            case NVR_MODEL_6404X:
                BoardTypeWiseInfo::productVariant = NVR6404XP2;
                BoardTypeWiseInfo::noOfHdd = 4;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR6404X P2";
                break;

            case NVR_MODEL_6408X:
                BoardTypeWiseInfo::productVariant = NVR6408XP2;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR6408X P2";
                break;

            case NVR_MODEL_9608X:
                BoardTypeWiseInfo::productVariant = NVR9608XP2;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR9608X P2";
                break;
            #endif
            #else
            case NVR_MODEL_0801X:
                BoardTypeWiseInfo::productVariant = NVR0801X;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR0801X";
                break;

            case NVR_MODEL_1601X:
                BoardTypeWiseInfo::productVariant = NVR1601X;
                BoardTypeWiseInfo::noOfHdd = 1;
                BoardTypeWiseInfo::noOfLan = 1;
                HardwareTestControl::boardTypeString = "NVR1601X";
                break;

            case NVR_MODEL_1602X:
                BoardTypeWiseInfo::productVariant = NVR1602X;
                BoardTypeWiseInfo::noOfHdd = 2;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR1602X";
                break;

            case NVR_MODEL_3202X:
                BoardTypeWiseInfo::productVariant = NVR3202X;
                BoardTypeWiseInfo::noOfHdd = 2;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR3202X";
                break;

            case NVR_MODEL_3204X:
                BoardTypeWiseInfo::productVariant = NVR3204X;
                BoardTypeWiseInfo::noOfHdd = 4;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR3204X";
                break;

            case NVR_MODEL_6404X:
                BoardTypeWiseInfo::productVariant = NVR6404X;
                BoardTypeWiseInfo::noOfHdd = 4;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR6404X";
                break;

            case NVR_MODEL_6408X:
                BoardTypeWiseInfo::productVariant = NVR6408X;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                HardwareTestControl::boardTypeString = "NVR6408X";
                break;
            #endif

            default:
                BoardTypeWiseInfo::productVariant = VARIANT_MAX;
                BoardTypeWiseInfo::noOfHdd = 8;
                BoardTypeWiseInfo::noOfLan = 2;
                EPRINT(GUI_SYS, "wrong board type detected");
                break;
        }

        switch(boardVersion)
        {
            #if defined(RK3568_NVRL)
            case NVR_XP2_BOARD_V1R0:
            case NVR_XP2_BOARD_V1R1:
                BoardTypeWiseInfo::isExtRTC = true;
                BoardTypeWiseInfo::isMultipAvail = false;
                break;

            #elif defined(RK3588_NVRH)
            case NVR_XP2_BOARD_V1R0:
                BoardTypeWiseInfo::isExtRTC = true;
                #if defined(OEM_JCI)
                BoardTypeWiseInfo::isMultipAvail = true;
                #else
                BoardTypeWiseInfo::isMultipAvail = (boardType == NVR_MODEL_3202X) ?  false : true;  /* We have no SATA multiplier for NVR_MODEL_3202X */
                #endif
                break;

            #else
            case NVR_X_BOARD_V1R2:
                BoardTypeWiseInfo::isExtRTC = true;
                BoardTypeWiseInfo::isMultipAvail = true;
                break;

            case NVR_X_BOARD_V1R3:
                BoardTypeWiseInfo::isExtRTC = false;
                BoardTypeWiseInfo::isMultipAvail = true;
                break;

            case NVR_X_BOARD_V2R2:
                BoardTypeWiseInfo::isExtRTC = false;
                BoardTypeWiseInfo::isMultipAvail = (boardType == NVR_MODEL_6408X) ? true : false;
                break;
            #endif
            default:
                break;
        }

        BoardTypeWiseInfo::isAudioInOutSupport = true;
        BoardTypeWiseInfo::noOfSensorInOut = 3;
        BoardTypeWiseInfo::noOfUsb = 3;

        #if !defined(OEM_JCI)
        if ((BoardTypeWiseInfo::productVariant == NVR0401XSP2)
                || (BoardTypeWiseInfo::productVariant == NVR0801XSP2)
                || (BoardTypeWiseInfo::productVariant == NVR1601XSP2))
        {
            BoardTypeWiseInfo::isAudioInOutSupport = false;
            BoardTypeWiseInfo::noOfSensorInOut = 0;
            BoardTypeWiseInfo::noOfUsb = 2;
        }
        #endif

    }while(0);

    return status;
}

void MainWindow::getMacAddress()
{
    INT32               sockFd;
    struct ifreq        ifr;
    UINT8PTR            hwAddr;
    CHAR                currMacAddr[MAX_LAN_PORT][MAX_MAC_ADDRESS_WIDTH];
    static const CHAR   *pInterfaceNameTab[MAX_LAN_PORT] = {"eth0", "eth1"};

    // Open Socket
    sockFd = socket(AF_PACKET, (SOCK_PACKET | SOCK_CLOEXEC), htons(ETH_P_ALL));
    if(sockFd == INVALID_CONNECTION)
    {
        return;
    }

    for(quint8 lanCount = LAN1_PORT; lanCount <  BoardTypeWiseInfo::noOfLan; lanCount++)
    {
        strcpy(ifr.ifr_name, pInterfaceNameTab[lanCount]);

        // Call Ioctl To Get Mac Addres
        if(ioctl(sockFd, SIOCGIFHWADDR, &ifr) < STATUS_OK)
        {
            EPRINT(GUI_SYS, "ioctl SIOCGIFHWADDR failed");
            continue;
        }

        hwAddr = (UINT8PTR)ifr.ifr_hwaddr.sa_data;
        sprintf(currMacAddr[lanCount], MAC_DISP_STRING, hwAddr[0], hwAddr[1], hwAddr[2], hwAddr[3], hwAddr[4], hwAddr[5]);
        switch(lanCount)
        {
            case LAN1_PORT:
                HardwareTestControl::macAdd1 = QString("%1").arg(currMacAddr[lanCount]);
                break;

            case LAN2_PORT:
                HardwareTestControl::macAdd2 = QString("%1").arg(currMacAddr[lanCount]);
                break;

            default:
                break;
        }
    }

    // Close Socket
    ::close(sockFd);
}

void MainWindow::showReport()
{
    m_reportBgRect = new Rect(m_rightPanelRect->x(),
                              m_rightPanelRect->y(),
                              m_rightPanelRect->width(),
                              m_rightPanelRect->height(),
                              BODY_BG_COLOR,
                              this);

    if(IS_VALID_OBJ(m_reportBgRect))
    {
        m_report = new Report(m_reportBgRect);
        if(IS_VALID_OBJ(m_report))
        {
            connect(m_report,
                    SIGNAL(sigCloseBtnClicked()),
                    this,
                    SLOT(slotHideReport()));
        }
    }
}

void MainWindow::enableDisableElements(bool isEnable)
{
    // Left Panel
    m_ntpServerIpAddr->setIsEnabled(isEnable);
    m_setRtcBtn->setIsEnabled(isEnable);
    m_ipAddr->setIsEnabled(isEnable);
    m_portTextBox->setIsEnabled(isEnable);
    m_ftpUserNameTextBox->setIsEnabled(isEnable);
    m_ftpPasswordTextBox->setIsEnabled(isEnable);
    m_saveBtn->setIsEnabled(isEnable);
    m_cancelBtn->setIsEnabled(isEnable);
    m_ftpDriveBtn->setIsEnabled(isEnable);
    m_usbDriveBtn->setIsEnabled(isEnable);

    //Right Panel
    m_startTestBtn->setIsEnabled(isEnable);
    m_devSerialNoTextBox->setIsEnabled(isEnable);
    m_testedByTextBox->setIsEnabled(isEnable);
    m_viewReportBtn->setIsEnabled(isEnable);
}

void MainWindow::doManualBtnAction(bool isPassBtn, bool isChangeBtnState)
{
    if(isPassBtn)
    {
        m_manualTimerCount = 0;
        m_manualTestTimer->stop();

        if (IS_VALID_OBJ(runFuncTestThreadLed))
        {
            runFuncTestThreadLed->changeTestIndicatorState(HW_TEST_PASS);
        }

        if (IS_VALID_OBJ(runFuncTestThreadBuzzer))
        {
            runFuncTestThreadBuzzer->changeTestIndicatorState(HW_TEST_PASS);
        }

        if (IS_VALID_OBJ(runFuncTestThreadAudio))
        {
            runFuncTestThreadAudio->changeTestIndicatorState(HW_TEST_PASS);
        }

        m_manualTimerLable->setVisible(false);
        m_manuallyFailedCount = 0;
        m_totalManualTestCnt = 0;
        m_noOfBlockCompleted = 0;
        m_totalHwTestBlock = 0;
        m_isShowPopupAlert = true;

        generateReport();

        for(quint8 hwIndex = 0; hwIndex < MAX_TEST_HW_CONDUCT; hwIndex++)
        {
            HardwareTestControl::testResult[hwIndex] = HW_TEST_PASS;
        }
    }
    else
    {
        m_manualTimerLable->setVisible(true);
    }

    m_manualTimerLable->changeText(QString("Take action within %1s").arg(MAX_MANUAL_TIMER_TIMEOUT - m_manualTimerCount));
    m_manualTimerLable->update();

    if((isPassBtn) ||((!isPassBtn) && (m_manuallyFailedCount == 0)))
    {
        m_manualTestPassBtn->setIsEnabled(false);
    }
    else
    {
        m_manualTestPassBtn->setIsEnabled(true);
    }

    if(m_manuallyFailedCount == 0)
    {
        m_manualTestFailBtn->setIsEnabled(false);
    }

    if(isChangeBtnState)
    {
        if (IS_VALID_OBJ(runFuncTestThreadLed))
        {
            runFuncTestThreadLed->enableDisableControls((isPassBtn) ? (false) : (true));
        }

        if (IS_VALID_OBJ(runFuncTestThreadBuzzer))
        {
            runFuncTestThreadBuzzer->enableDisableControls((isPassBtn) ? (false) : (true));
        }

        if (IS_VALID_OBJ(runFuncTestThreadAudio))
        {
            runFuncTestThreadAudio->enableDisableControls((isPassBtn) ? (false) : (true));
        }
    }

    if(isPassBtn)
    {
        backUpToFTPUSBDrive(FTP_DRIVE);
    }
}

void MainWindow::showHidePopupAlert(bool isShow, POP_UP_ALERT_NAME_e popupName, POP_UP_ALERT_MODE_e popupMode)
{
    if(isShow)
    {
        if((IS_VALID_OBJ(m_popUpAlert)) && (m_popUpAlert->getPopUpName() != popupName))
        {
            disconnect(m_popUpAlert,
                       SIGNAL(sigCloseAlert(int,bool)),
                       this,
                       SLOT(slotPopUpAlertClose(int,bool)));
            m_popUpAlert->deleteLater();
            m_popUpAlert = NULL;

        }

        if(!(IS_VALID_OBJ(m_popUpAlert)))
        {
            m_popUpAlert = new MxPopUpAlert(m_rightPanelRect->width(),
                                            m_rightPanelRect->height(),
                                            popupName,
                                            popupMode,
                                            m_rightPanelRect);
            if(IS_VALID_OBJ(m_popUpAlert))
            {
                connect(m_popUpAlert,
                        SIGNAL(sigCloseAlert(int,bool)),
                        this,
                        SLOT(slotPopUpAlertClose(int,bool)));
            }
        }
    }
    else
    {
        if(IS_VALID_OBJ(m_popUpAlert))
        {
            disconnect(m_popUpAlert,
                       SIGNAL(sigCloseAlert(int,bool)),
                       this,
                       SLOT(slotPopUpAlertClose(int,bool)));
            m_popUpAlert->deleteLater();
            m_popUpAlert = NULL;
        }
    }
}

void MainWindow::generateReport()
{
    struct tm tempTime;
    time_t temp1,temp2;    
    m_rtcDisplay->getLocalTime(&tempTime);
    tempTime.tm_mon += 1;
    m_rtcDisplay->ConvertLocalTimeInSec (&HardwareTestControl::testStartTime,&temp1);
    m_rtcDisplay->ConvertLocalTimeInSec (&tempTime,&temp2);
    quint64 diff = (temp2 - temp1);
    quint8 min = diff/60 ;
    quint8 sec = diff%60;
    HardwareTestControl::testStartEndDiff = QString("%1").arg(min,2,10,QLatin1Char('0')) + ":"
            + QString("%1").arg(sec,2,10,QLatin1Char('0')) + QString(" (mm:ss)");

    if(IS_VALID_OBJ(m_generateReport))
    {
        m_generateReport->valueFillInReport();
    }
    else
    {
        EPRINT(GUI_SYS, "invld object[m_generateReport]");
    }

    HardwareTestControl::reTestCount += 1;
}

BOOL MainWindow::getLan1State(void)
{
    INT32   fileFd;
    CHAR 	valBuf[2];

    fileFd = open(LAN1_DEV_LINK_STS_FILE, (O_RDONLY | O_CLOEXEC));
    if (fileFd == INVALID_FILE_FD)
    {
        EPRINT(GUI_SYS, "fail to open lan1 link status file: [file=%s], [err=%s]", LAN1_DEV_LINK_STS_FILE, strerror(errno));
        return FALSE;
    }

    if (read(fileFd, &valBuf, sizeof(valBuf)) < 0)
    {
        EPRINT(GUI_SYS, "fail to read lan1 link status file: [file=%s], [err=%s]", LAN1_DEV_LINK_STS_FILE, strerror(errno));
        ::close(fileFd);
        return FALSE;
    }

    ::close(fileFd);
    return (valBuf[0] == '1') ? TRUE : FALSE;
}

void MainWindow::slotUsbBkp (bool status)
{
    if(status)
    {
        m_messageBanner->loadInfoMessage("Test Completed Successfully Report\ngenerated and saved on USB Drive");
    }
    else
    {
        m_messageBanner->loadInfoMessage("Test Completed Successfully Report\ngenerated but failed to save on USB Drive");
    }

    // After USB report uploaded enable controls
    enableDisableElements(true);
}

void MainWindow::slotFtpBkp(bool status)
{
    if (status)
    {
        if(m_isShowPopupAlert)
        {
            showHidePopupAlert(true, TEST_COMPLETED, ONLY_MSG);
            m_isShowPopupAlert = false;
        }
        else
        {
            m_messageBanner->loadInfoMessage("Test Completed Successfully Report\ngenerated and saved on FTP Drive");
        }
    }
    else
    {
        m_isShowPopupAlert = false;
        m_messageBanner->loadInfoMessage("Test Completed Successfully Report\ngenerated but failed to save on FTP Drive");
    }

    // After Ftp report uploaded enable controls
    enableDisableElements(true);
}

void MainWindow::slotHideReport()
{
    if(IS_VALID_OBJ(m_report))
    {
        disconnect(m_report,
                   SIGNAL(sigCloseBtnClicked()),
                   this,
                   SLOT(slotHideReport()));        

        DELETE_OBJ(m_report);
    }
    DELETE_OBJ(m_reportBgRect);
}

void MainWindow::slotTimeOut()
{
    if (IS_VALID_OBJ(mainTimer))
    {
        mainTimer->stop();
    }
}

void MainWindow::slotPopUpAlertClose(int index, bool isCloseAlert)
{
    Q_UNUSED(index);

    if(isCloseAlert)
    {
        showHidePopupAlert(false);
    }
}

void MainWindow::slotLoadNextPage ()
{
    if(IS_VALID_OBJ(m_messageBanner))
    {
        if(m_messageBanner->getMessageStr() == SHUTDOWN_STRING)
        {
            sleep(1);
            Utils_ExeCmd((char*)"poweroff");
        }
        m_messageBanner->setVisible(false);
    }
}

void MainWindow::slotManualTimeout()
{    
    if(IS_VALID_OBJ(m_manualTestTimer))
    {
        m_manualTimerCount++;
        if(m_manualTimerCount < MAX_MANUAL_TIMER_TIMEOUT)
        {
            m_manualTimerLable->changeText(QString("Take action within %1s").arg(MAX_MANUAL_TIMER_TIMEOUT - m_manualTimerCount));
            m_manualTimerLable->update();
        }
        else
        {
            doManualBtnAction(true, true);
        }
    }
}

void MainWindow::slotCtrlBtnClicked(HW_TEST_BLOCK_e hwTestBlock, int hwIndex, int ctrlBtnIndex)
{
    if(ctrlBtnIndex != FAIL_CLICKED)
    {
        return;
    }

    m_manuallyFailedCount++;
    if(m_manuallyFailedCount >= m_totalManualTestCnt)
    {
        doManualBtnAction(true, false);
    }
    else
    {
        doManualBtnAction(false, false);
    }
    Q_UNUSED(hwTestBlock);
    Q_UNUSED(hwIndex);
}

void MainWindow::slotShutDownClicked(int index)
{
    m_messageBanner->loadInfoMessage(SHUTDOWN_STRING);
    Q_UNUSED(index);
}

void MainWindow::slotUpdateSysTime()
{
    m_sysTime = m_sysTime.addSecs(1);
    QString tempStr =  QString("%1/%2/%3 %4:%5:%6")
            .arg(m_sysTime.date().day(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.date().month(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.date().year(),4,10,QLatin1Char('0'))
            .arg(m_sysTime.time().hour(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.time().minute(),2,10,QLatin1Char('0'))
            .arg(m_sysTime.time().second(),2,10,QLatin1Char('0'));

    m_sysTimeLable->changeText(tempStr);
    m_sysTimeLable->update();

    m_countWdUpdate++;
    if (m_countWdUpdate > 30)
    {
        /* Kick watchdog on regular interval */
        kickWatchdog();
        m_countWdUpdate = 0;
    }
}

void MainWindow::kickWatchdog()
{
    if (m_watchDogFd == INVALID_FILE_FD)
    {
        return;
    }

    #if defined(RK3568_NVRL)
    if ((BoardTypeWiseInfo::productVariant == NVR0401XSP2) || (BoardTypeWiseInfo::productVariant == NVR0801XSP2) || (BoardTypeWiseInfo::productVariant == NVR1601XSP2))
    {
        if (write(m_watchDogFd, "M", 1) < 0)    /* This is dummy value. we have to write some value into file only */
        {
            EPRINT(GUI_SYS, "fail to feed watchdog: [err=%s]", strerror(errno));
        }
    }
    else
    {
        if(ioctl(m_watchDogFd, IOCTL_WATCHDOG_TOGGLE,0) < 0)
        {
            EPRINT(GUI_SYS, "ioctl IOCTL_WATCHDOG_TOGGLE failed");
        }
    }
    #else
    if (write(m_watchDogFd, "M", 1) < 0)    /* This is dummy value. we have to write some value into file only */
    {
        EPRINT(GUI_SYS, "fail to feed watchdog: [err=%s]", strerror(errno));
    }
    #endif
}
