#include "RunTest.h"
#include "BoardTypeWiseInfo.h"
#include "LedOut.h"
#include "AudioInOut.h"
#include "DirectPassFailTest.h"
#include "HDMIVideo.h"
#include "RTCTest.h"
#include "Ether.h"
#include "SensorInAlarmOut.h"
#include "HDD.h"
#include "USBStatus.h"
#include "Buzzer.h"

RunTest::RunTest(HW_TEST_BLOCK_e testBlock, QWidget *parent)
    : QThread(parent), hwTestBlock(testBlock)
{
    switch(testBlock)
    {
        case HW_TEST_FUNCTION_BUZZER:
            startTestIndex = BUZZER_HW_TEST;
            stopTestIndex =  LED_HW_TEST;
            break;

        case HW_TEST_FUNCTION_LED:
            startTestIndex = LED_HW_TEST;
            stopTestIndex = AUDIO_IN_HW_TEST;
            break;

        case HW_TEST_FUNCTIONAL_OTHERS:
            startTestIndex = AUDIO_IN_HW_TEST;
            stopTestIndex = HDMI_VIDEO_HW_TEST;
            break;

        case HW_TEST_AUTOMATED_ALL:
            startTestIndex = HDMI_VIDEO_HW_TEST;
            stopTestIndex = MAX_TEST_HW_CONDUCT;
            break;

        default:
            break;
    }

    quint32 yPosAutoTest = 230;
    for (quint8 testCount = startTestIndex; testCount < stopTestIndex; testCount++)
    {
        INIT_OBJ(hwTestControl[testCount]);
        switch (testCount)
        {
            case BUZZER_HW_TEST:
            {
                hwTestControl[testCount] = new Buzzer(440, 235, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                connect(hwTestControl[testCount],
                        SIGNAL(sigCtrlBtnClicked(int,int)),
                        this,
                        SLOT(slotCtrlBtnClicked(int,int)));
            }
            break;

            case LED_HW_TEST:
            {
                hwTestControl[testCount] = new LedOut(440, 340, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                connect(hwTestControl[testCount],
                        SIGNAL(sigCtrlBtnClicked(int,int)),
                        this,
                        SLOT(slotCtrlBtnClicked(int,int)));
            }
            break;

            case AUDIO_IN_HW_TEST:
            {
                if (false == BoardTypeWiseInfo::isAudioInOutSupport)
                {
                    break;
                }

                hwTestControl[testCount] = new AudioInOut(440, 445, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                connect(hwTestControl[testCount],
                        SIGNAL(sigCtrlBtnClicked(int,int)),
                        this,
                        SLOT(slotCtrlBtnClicked(int,int)));
            }
            break;

            case AUDIO_OUT_HW_TEST:
            {
                if (false == BoardTypeWiseInfo::isAudioInOutSupport)
                {
                    break;
                }

                hwTestControl[testCount] = new DirectPassFailTest(440, 550, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                connect(hwTestControl[testCount],
                        SIGNAL(sigCtrlBtnClicked(int,int)),
                        this,
                        SLOT(slotCtrlBtnClicked(int,int)));
            }
            break;

            case HDMI_AUDIO_HW_TEST:
            {
                if (false == BoardTypeWiseInfo::isAudioInOutSupport)
                {
                    break;
                }

                hwTestControl[testCount] = new DirectPassFailTest(440, 650, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                connect(hwTestControl[testCount],
                        SIGNAL(sigCtrlBtnClicked(int,int)),
                        this,
                        SLOT(slotCtrlBtnClicked(int,int)));
            }
            break;

            case HDMI_VIDEO_HW_TEST:
            {
                hwTestControl[testCount] = new HDMIVideo(840, yPosAutoTest, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                yPosAutoTest += 50;
            }
            break;

            case RTC_HW_TEST:
            {
                hwTestControl[testCount] = new RTCTest(840, yPosAutoTest, 1, 1, (CONDUNCT_TEST_e)testCount, parent);
                yPosAutoTest += 50;
            }
            break;

            case ETHERNET_HW_TEST:
            {
                hwTestControl[testCount] = new Ether(871, yPosAutoTest, 1, 2, (CONDUNCT_TEST_e)testCount, parent, BoardTypeWiseInfo::noOfLan);
                yPosAutoTest += 75;
            }
            break;

            case SENSOR_IN_ALARM_OUT_HW_TEST:
            {
                if (BoardTypeWiseInfo::noOfSensorInOut == 0)
                {
                    break;
                }
                hwTestControl[testCount] = new SensorInAlarmOut(871, yPosAutoTest, 3, 1, (CONDUNCT_TEST_e)testCount, parent, BoardTypeWiseInfo::noOfSensorInOut);
                yPosAutoTest += 125;
            }
            break;

            case HDD_HW_TEST:
            {
                hwTestControl[testCount] = new HDDStatus(871, yPosAutoTest, 2, 4, (CONDUNCT_TEST_e)testCount, parent, BoardTypeWiseInfo::noOfHdd);
                yPosAutoTest += ((BoardTypeWiseInfo::noOfHdd > 4) ? 100 : 75); /* 4 HDDs in 1 row */
            }
            break;

            case USB_HW_TEST:
            {
                hwTestControl[testCount] = new USBStatus(871, yPosAutoTest, 1, 3, (CONDUNCT_TEST_e)testCount, parent, BoardTypeWiseInfo::noOfUsb);
                yPosAutoTest += 75;
            }
            break;

            default:
            {
                EPRINT(GUI_SYS, "invld hardware test type: [type=%d]", testCount);
            }
            break;
        }
    }
}

RunTest::~RunTest()
{
    for (quint8 testCount = startTestIndex; testCount < stopTestIndex; testCount++)
    {
        if(IS_VALID_OBJ(hwTestControl[testCount]))
        {
            if(testCount <= HDMI_AUDIO_HW_TEST)
            {
                if(IS_VALID_OBJ(hwTestControl[testCount]))
                {
                    disconnect(hwTestControl[testCount],
                               SIGNAL(sigCtrlBtnClicked(int,int)),
                               this,
                               SLOT(slotCtrlBtnClicked(int,int)));
                }
            }
            DELETE_OBJ(hwTestControl[testCount]);
        }
    }
    hwTestBlock = MAX_HW_TEST_BLOCK;
}

void RunTest::run()
{
    for(quint8 hwIndex = startTestIndex; hwIndex < stopTestIndex; hwIndex++)
    {
        if(IS_VALID_OBJ(hwTestControl[hwIndex]))
        {
            hwTestControl[hwIndex]->hardwareTestStart();
            DPRINT(GUI_SYS, "[%s] HW_Test Started", GET_HW_TEST_TYPE_STR(hwIndex));
            if(testExecuted(hwIndex))
            {
                break;
            }
        }
    }    
    emit sigTestCompelete(hwTestBlock);
}

void RunTest::slotShutDown(int)
{
    DPRINT(GUI_SYS, "Stop all running HW_Test[hwTestBlock=%d]", hwTestBlock);
    for (quint8 testCount = startTestIndex; testCount < stopTestIndex; testCount++)
    {
        if(IS_VALID_OBJ(hwTestControl[testCount]))
        {
            hwTestControl[testCount]->hardwareTestStop();
        }
    }
    HardwareTestControl::exitTest=false;
}

void RunTest::enableDisableControls(bool isEnable)
{
    for (quint8 testCount = startTestIndex; testCount < stopTestIndex; testCount++)
    {
        if(IS_VALID_OBJ(hwTestControl[testCount]))
        {
            hwTestControl[testCount]->enabledisableControls(isEnable);
        }
    }
}

void RunTest::changeTestIndicatorState(TEST_RESULT_e testResult)
{
    for (quint8 testCount = startTestIndex; testCount < stopTestIndex; testCount++)
    {
        if(IS_VALID_OBJ(hwTestControl[testCount]))
        {
            hwTestControl[testCount]->passFailTest(testResult, true);
        }
    }
}

void RunTest::slotCtrlBtnClicked(int hwIndex, int ctrlBtnIndex)
{    
    if(IS_VALID_OBJ(hwTestControl[hwIndex]))
    {
        hwTestControl[hwIndex]->saveHwTestStatus((CONDUNCT_TEST_e)hwIndex);
        DPRINT(GUI_SYS, "[%s] HW_Test Completed [result=%s]", GET_HW_TEST_TYPE_STR(hwIndex),
                GET_HW_TEST_RESULT_STR(HardwareTestControl::testResult[hwIndex]));
    }

    emit sigCtrlBtnClicked(hwTestBlock, hwIndex, ctrlBtnIndex);
}

bool RunTest::testExecuted(quint8 hwIndex)
{
    quint8 isExecutionDone=false;

    do
    {
        if(!IS_VALID_OBJ(hwTestControl[hwIndex]))
        {
            isExecutionDone = true;
            break;
        }

        if ((!HardwareTestControl::exitTest) || (hwIndex >= (quint8)MAX_TEST_HW_CONDUCT))
        {
            isExecutionDone = true;
            break;
        }

        DPRINT(GUI_SYS, "[%s] HW_Test Completed [result=%s]", GET_HW_TEST_TYPE_STR(hwIndex),
                GET_HW_TEST_RESULT_STR(HardwareTestControl::testResult[hwIndex]));
        hwTestControl[hwIndex]->saveHwTestStatus((CONDUNCT_TEST_e)hwIndex);
        hwTestControl[hwIndex]->hardwareTestStop();
    }while(0);

    return isExecutionDone;
}
