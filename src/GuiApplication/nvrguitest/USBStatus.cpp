#include "USBStatus.h"
#include "UdevMonitor.h"

USBStatus::USBStatus(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent, quint8 noOfSubDev)
    : HardwareTestControl(hwIndex, parent, noOfSubDev)
{
    QStringList strlst;

    noOfUSB = noOfSubDev;
    hwType = hwIndex;

    strlst << "USB Port 1" << "USB Port 2" << "USB Port 3";
    createDefaultComponent(xPos, yPos, totalRow, totalCol, false, false, SUFFIX_FONT_SIZE, strlst, QString("USB"));
}

void USBStatus::hardwareTestStart()
{
    udevMonitor*    udevMonitorIndex = udevMonitor::getInstance();

    for(quint8 usbCount = 0; usbCount < noOfUSB; usbCount++)
    {
        testCondunctResult[usbCount] = HW_TEST_FAIL;

        if(false == IS_VALID_OBJ(udevMonitorIndex))
        {
            continue;
        }

        if(udevMonitorIndex->m_isUSBConnected[usbCount])
        {
            testCondunctResult[usbCount] = HW_TEST_PASS;
        }
    }
}

void USBStatus::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}
