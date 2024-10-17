#include <unistd.h>

#include "LedOut.h"

#if defined(RK3568_NVRL) || defined(RK3588_NVRH)
#define GREEN_LED   "/sys/class/gpios/MxGpioDrv/greenled"
#define RED_LED     "/sys/class/gpios/MxGpioDrv/redled"
#else
#define GREEN_LED   "/sys/class/gpio/MxGpioDrv/greenled"
#define RED_LED     "/sys/class/gpio/MxGpioDrv/redled"
#endif

#define LED_ON      "1"
#define LED_OFF     "0"

LedOut::LedOut(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent)
    : HardwareTestControl(hwIndex, parent)
{
    QStringList strlst;

    LedState = MAX_LED_STATE;
    stopTest = true;
    m_fileIO = new FileIO(this,GREEN_LED);

    strlst << "LED" ;
    createDefaultComponent(xPos, yPos, totalRow, totalCol, true, false, TEST_HEADING_FONT_SIZE, strlst);
}

LedOut::~LedOut ()
{
    DELETE_OBJ(m_fileIO);
}

void LedOut::hardwareTestStart()
{
    if (IS_VALID_OBJ(m_fileIO))
    {
        m_fileIO->write(LED_ON);
        testCondunctResult[0] = HW_TEST_PASS;

        while(stopTest)
        {
            sleep(1);
            timeOut();
        }

        LedState = MAX_LED_STATE;
    }
    stopTest = true;
}

void LedOut::hardwareTestStop()
{
    stopTest = false;
}

bool LedOut::timeOut()
{
    LedState = getLedState();

    m_fileIO->write(LED_OFF);

    switch(LedState)
    {
    case RED_LED_STATE:
        m_fileIO->changeSourcePath(RED_LED);
        break;

    case GREEN_LED_STATE:
        m_fileIO->changeSourcePath(GREEN_LED);
        break;

    case MAX_LED_STATE:
    default:
        break;
    }

    m_fileIO->write(LED_ON);
    return true;
}

quint8 LedOut::getLedState()
{
    switch(LedState)
    {
    case MAX_LED_STATE:
        LedState = RED_LED_STATE;
        break;

    case RED_LED_STATE:
        LedState = GREEN_LED_STATE;
        break;

    case GREEN_LED_STATE:
        LedState = RED_LED_STATE;
        break;

    default:
        break;
    }
    return LedState;
}

void LedOut::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}
