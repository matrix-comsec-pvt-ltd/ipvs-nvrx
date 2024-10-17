#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "Buzzer.h"
#include "MxGpioDrv.h"

Buzzer::Buzzer(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent)
    : HardwareTestControl(hwIndex, parent)
{
    QStringList strList;

    stopTest = true;
    strList << "BUZZER";
    m_buzzerDriverFd = open(GPIO_DEVICE_NAME, O_RDWR | O_CLOEXEC);
    createDefaultComponent(xPos, yPos, totalRow, totalCol, true, false, TEST_HEADING_FONT_SIZE, strList);
}

Buzzer::~Buzzer()
{
    if (m_buzzerDriverFd != INVALID_FILE_FD)
    {
        ::close(m_buzzerDriverFd);
    }
}

void Buzzer::hardwareTestStart()
{
    if (m_buzzerDriverFd == INVALID_FILE_FD)
    {
        return;
    }

    /* Making buzzer to beep for 2 seconds */
    updateBuzzer(true);
    sleep(2);
    updateBuzzer(false);
    testCondunctResult[0] = HW_TEST_PASS;
    while(stopTest)
    {
        sleep(1);
    }
    stopTest = true;
}

void Buzzer::hardwareTestStop()
{
    stopTest = false;
}

void Buzzer::updateBuzzer(bool status)
{
    if (m_buzzerDriverFd == INVALID_FILE_FD)
    {
        return;
    }

    if (status == true)
    {
        quint32 buzzerCad = NVRX_TRG_EVT_RESP;
        if(ioctl(m_buzzerDriverFd, MXGPIO_SET_CADENCE, &buzzerCad) < 0)
        {
            EPRINT(GUI_SYS, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", strerror(errno));
        }
    }
    else
    {
        BUZZER_LED_RESP_t buzzerCad = {NVRX_TRG_EVT_RESP, NVRX_TRG_EVT_RESP};
        if(ioctl(m_buzzerDriverFd, MXGPIO_CLR_CADENCE, &buzzerCad) < 0)
        {
            EPRINT(GUI_SYS, "ioctl MXGPIO_SET_CADENCE failed: [err=%s]", strerror(errno));
        }
    }
}

void Buzzer::timeOut(void)
{
    if (m_buzzerDriverFd != INVALID_FILE_FD)
    {
        updateBuzzer(false);
    }
}

void Buzzer::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}
