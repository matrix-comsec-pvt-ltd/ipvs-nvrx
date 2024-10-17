#include "HDD.h"
#include "UdevMonitor.h"

HDDStatus::HDDStatus(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent,quint8 noOfSubDev)
    : HardwareTestControl(hwIndex, parent, noOfSubDev)
{
    QStringList strlst;

    hwType = hwIndex;
    noOfHDD = noOfSubDev;

    strlst << "HDD1" << "HDD2" << "HDD3" << "HDD4"
           << "HDD5" << "HDD6" << "HDD7" << "HDD8";
    createDefaultComponent(xPos, yPos, totalRow, totalCol, false, false, SUFFIX_FONT_SIZE, strlst, QString("HDD"));
}

void HDDStatus::hardwareTestStart()
{
    udevMonitor*    udevMonitorIndex = udevMonitor::getInstance();

    for(quint8 hddCount=0; hddCount<(noOfHDD); hddCount++)
    {
        testCondunctResult[hddCount] = HW_TEST_FAIL;

        if(false == IS_VALID_OBJ(udevMonitorIndex))
        {
            continue;
        }

        if(udevMonitorIndex->m_isHDDConnected[hddCount])
        {
            testCondunctResult[hddCount] = HW_TEST_PASS;
        }
    }
}

void HDDStatus::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}
