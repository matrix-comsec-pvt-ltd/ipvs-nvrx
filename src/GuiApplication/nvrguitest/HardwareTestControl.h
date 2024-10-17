#ifndef HARDWARETESTCONTROL_H
#define HARDWARETESTCONTROL_H

#include <QWidget>
#include "Controls/Rect.h"
#include "Controls/TextLabel.h"
#include "Controls/Image.h"
#include "Controls/CnfgButton.h"

#define MAX_SUB_DEVICE      10
#define MAX_ROWS            (3)
#define MAX_COLUMNS         (4)

typedef enum
{
    PLAY_STATE,
    STOP_STATE,
    MAX_PLAY_STATE
}PLAY_BTN_STATE_e;

typedef enum
{    
    HW_TEST_FUNCTION_BUZZER,
    HW_TEST_FUNCTION_LED,
    HW_TEST_FUNCTIONAL_OTHERS,
    HW_TEST_AUTOMATED_ALL,
    MAX_HW_TEST_BLOCK
}HW_TEST_BLOCK_e;

typedef enum
{
    HW_TEST_FAIL,
    HW_TEST_PASS,
    MAX_HW_TEST_RESULT
}TEST_RESULT_e;

typedef enum
{
    /* Functional/Manual Test */
    BUZZER_HW_TEST = 0,
    LED_HW_TEST,
    AUDIO_IN_HW_TEST,
    AUDIO_OUT_HW_TEST,
    HDMI_AUDIO_HW_TEST,
    FUNCTIONAL_TEST_MAX,

    /* Automated Test */
    HDMI_VIDEO_HW_TEST = FUNCTIONAL_TEST_MAX,
    RTC_HW_TEST,
    ETHERNET_HW_TEST,
    SENSOR_IN_ALARM_OUT_HW_TEST,
    HDD_HW_TEST,
    USB_HW_TEST,
    MAX_TEST_HW_CONDUCT
}CONDUNCT_TEST_e;

typedef enum
{
    PLAY_STOP_CLICKED,
    FAIL_CLICKED,
    MAX_UI_CLICKED
}CLICKED_UI_e;

/* Get HwTestType String */
#define GET_HW_TEST_TYPE_STR(__hwTestType__) \
    (((__hwTestType__) == BUZZER_HW_TEST ) ? "BUZZER" : \
    ((__hwTestType__) == LED_HW_TEST) ? "LED" : \
    ((__hwTestType__) == AUDIO_IN_HW_TEST) ? "AUDIO_IN" : \
    ((__hwTestType__) == AUDIO_OUT_HW_TEST) ? "AUDIO_OUT" : \
    ((__hwTestType__) == HDMI_AUDIO_HW_TEST) ? "HDMI_AUDIO" : \
    ((__hwTestType__) == HDMI_VIDEO_HW_TEST) ? "HDMI_VIDEO" : \
    ((__hwTestType__) == RTC_HW_TEST) ? "RTC" : \
    ((__hwTestType__) == ETHERNET_HW_TEST) ? "ETHERNET" : \
    ((__hwTestType__) == SENSOR_IN_ALARM_OUT_HW_TEST) ? "SENSOR_IN_ALARM_OUT" : \
    ((__hwTestType__) == HDD_HW_TEST) ? "HDD" : \
    ((__hwTestType__) == USB_HW_TEST) ? "USB" : \
    "Invld")

/* Get HwTestResult String */
#define GET_HW_TEST_RESULT_STR(__hwTestResult__) (((__hwTestResult__) == HW_TEST_PASS) ? "PASS" : "FAIL")

class HardwareTestControl : public QWidget
{
    Q_OBJECT
public:
    explicit HardwareTestControl(CONDUNCT_TEST_e hwIndex, QWidget *parent = 0, quint8 numOfSubDev=1);
    
    virtual void hardwareTestStart()=0;
    virtual void hardwareTestStop()=0;
    virtual void saveHwTestStatus(CONDUNCT_TEST_e index)=0;

   ~HardwareTestControl();

    void enabledisableControls(bool isEnable);
    void passFailTest(TEST_RESULT_e testResult, bool isApplyToIndicator);

    static bool exitTest;
    static QString ntpServerIpAddr;
    static QString ftpServerIpAddr;
    static QString ftpPort;
    static QString ftpUsername;
    static QString ftpPassword;
    static QString deviceSerialNumber;
    static QString testEmpId;
    static QString macAdd1;
    static QString macAdd2;
    static quint8 reTestCount;
    static QString ntpDate;
    static QString ntpTime;
    static TEST_RESULT_e testResult[MAX_TEST_HW_CONDUCT];
    static QString boardTypeString;
    static struct tm testStartTime;
    static QString testStartEndDiff;

protected:
    void createDefaultComponent(quint16 xPos, quint16 yPos,
                                quint8 totalRow, quint8 totalCol,
                                bool isCtrlBtnNeeded,
                                bool isPlayBtnNeeded,
                                quint8 fontSize,
                                QStringList& lableList,
                                QString testHeader = "");
    TEST_RESULT_e getStatusOfHwTest();
    TEST_RESULT_e testCondunctResult[MAX_SUB_DEVICE];

    CONDUNCT_TEST_e     hwType;
    quint8              m_totalColumns;
    quint8              m_totalElements;
    TextLabel*          m_testHeader;
    Rect*               m_statusIndicator[MAX_ROWS][MAX_COLUMNS];
    TextLabel*          m_statusLable[MAX_ROWS][MAX_COLUMNS];
    Image*              m_playBtn[MAX_ROWS][MAX_COLUMNS];
    PLAY_BTN_STATE_e    m_playBtnState[MAX_ROWS][MAX_COLUMNS];
    CnfgButton*         m_failBtn[MAX_ROWS][MAX_COLUMNS];
    quint8              numOfSubdevices;
    bool                m_isCtrlBtnNeeded;
    bool                m_isPlayBtnNeeded;

signals:
    void sigCtrlBtnClicked(int hwIndex, int ctrlBtnIndex);
    
public slots:
    void slotUIClicked(int ctrlBtnIndex);
    void slotImageClicked(int indexInPage);
};

#endif // HARDWARETESTCONTROL_H
