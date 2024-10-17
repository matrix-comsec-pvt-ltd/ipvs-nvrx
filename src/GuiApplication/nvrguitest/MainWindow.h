#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Controls/Rect.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/CnfgButton.h"
#include "DataStructure.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/MxPopUpAlert.h"

#include "FileIO.h"
#include "SensorInAlarmOut.h"
#include "Ether.h"
#include "RTCDisplay.h"
#include "LedOut.h"
#include "RTCTest.h"

#include "USBStatus.h"

#include "HDD.h"
#include "GenrateReport.h"
#include "Report.h"
#include "RunTest.h"
#include "BoardTypeWiseInfo.h"
#include "FileUpload.h"
#include "GenrateReport.h"


#include "Controls/Image.h"
#include "Controls/TextLabel.h"
#include "Controls/MessageBanner.h"
#include "UdevMonitor.h"
#include "VirtualKeypad.h"

typedef enum
{
    SET_RTC,
    SAVE_FTP,
    CANCEL_FTP,
    FTP_DRIVE,
    USB_DRIVE,
    START_RESTART_TEST,
    VIEW_REPORT,
    MANUAL_PASS_BTN,
    MANUAL_FAIL_BTN
}CNFG_BTN_e;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    void setReportBackupSettingFieldValue();
    void setReportBackupSettingParam();

    void backUpToFTPUSBDrive(CNFG_BTN_e index);
    void getMacAddress();
private:
    ~MainWindow();

    RunTest *runFuncTestThreadLed;
    RunTest *runFuncTestThreadBuzzer;
    RunTest *runFuncTestThreadAudio;
    RunTest *runAutoTestThread;


    Rect*           m_mainHeaderRect;
    Rect*           m_userDetailRect;
    Rect*           m_LeftPanelRect;
    Rect*           m_rightPanelRect;
    Rect*           m_seperatorLine;

    //Main Heading
    Image*          m_matrixLogo;
    TextLabel*      m_applicationHeading;
    Image*          m_shutDownImage;

    //LeftPanel
    TextLabel*      m_networkTimeHeading;
    Ipv4TextBox*    m_ntpServerIpAddr;
    CnfgButton*     m_setRtcBtn;
    TextLabel*      m_rtcUpdatedLable;
    TextLabel*      m_currentTimeLable;
    TextLabel*      m_reportBackupHeading;
    Ipv4TextBox*    m_ipAddr;
    TextboxParam*   m_portParam;
    TextBox*        m_portTextBox;
    TextboxParam*   m_ftpUserNameParam;
    TextBox*        m_ftpUserNameTextBox;
    TextboxParam*   m_ftpPasswordParam;
    PasswordTextbox* m_ftpPasswordTextBox;
    CnfgButton*     m_saveBtn;
    CnfgButton*     m_cancelBtn;
    TextLabel*      m_manualBackupHeading;
    CnfgButton*     m_ftpDriveBtn;
    CnfgButton*     m_usbDriveBtn;
    TextLabel*      m_sysTimeLable;
    QTimer*         m_sysTimeTimer;
    QDateTime       m_sysTime;

    //Right Panel
    TextboxParam*   m_devSerialNoParam;
    TextBox*        m_devSerialNoTextBox;
    TextboxParam*   m_testedByParam;
    TextBox*        m_testedByTextBox;
    CnfgButton*     m_startTestBtn;
    CnfgButton*     m_viewReportBtn;
    MxPopUpAlert*   m_popUpAlert;
    TextLabel*      m_functionalTestHeading;
    TextLabel*      m_automatedTestHeading;
    FileUpload*     m_fileUpload;
    GenrateReport*  m_generateReport;
    CnfgButton*     m_manualTestPassBtn;
    CnfgButton*     m_manualTestFailBtn;
    RTCDisplay*     m_rtcDisplay;
    MessageBanner*  m_messageBanner;
    TextLabel*      m_manualTimerLable;

    //End
    VirtualKeypad*      virtualKeypad;
    QTimer*             mainTimer;
    QTimer*             m_manualTestTimer;
    quint8              m_manualTimerCount;
    quint8              m_manuallyFailedCount;
    quint8              m_totalManualTestCnt;
    bool                m_isShowPopupAlert;    
    quint8              m_totalHwTestBlock;
    quint8              m_noOfBlockCompleted;
    udevMonitor*        udevMonitorIndex;
    Rect*               m_reportBgRect;
    Report*             m_report;
    int                 m_watchDogFd;
    quint8              m_countWdUpdate;

    void createDefaultComponents();
    void createLeftPanelComponents();
    void createRightPanelComponents();
    bool UpdateBoardTyeAndVer();
    void showReport();
    void enableDisableElements(bool isEnable);
    void doManualBtnAction(bool isPassBtn, bool isChangeBtnState);
    void showHidePopupAlert(bool isShow,
                            POP_UP_ALERT_NAME_e popupName = MAX_POP_UP_ALERT_NAME,
                            POP_UP_ALERT_MODE_e popupMode = MAX_POP_UP_ALRT_MODE);
    void generateReport();
    BOOL getLan1State();
    void kickWatchdog();
    void paintEvent (QPaintEvent *);   

signals:

public slots:
    void slotTestCompelete(HW_TEST_BLOCK_e testBlock);
    void slotButtonClick(int index);    
    void slotUsbBkp(bool);
    void slotFtpBkp(bool);
    void slotHideReport();
    void slotTimeOut();    
    void slotPopUpAlertClose(int index,bool isCloseAlert);
    void slotLoadNextPage();
    void slotManualTimeout();
    void slotCtrlBtnClicked(HW_TEST_BLOCK_e hwTestBlock, int hwIndex, int ctrlBtnIndex);
    void slotShutDownClicked(int index);
    void slotUpdateSysTime();
};

#endif // MAINWINDOW_H
