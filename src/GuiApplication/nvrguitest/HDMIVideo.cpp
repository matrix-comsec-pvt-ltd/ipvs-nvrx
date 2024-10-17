#include "HDMIVideo.h"

HDMIVideo::HDMIVideo(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent):
    HardwareTestControl(hwIndex, parent)
{
    QStringList strlst;

    strlst << "HDMI Video" ;
    createDefaultComponent(xPos, yPos, totalRow, totalCol, false, false, TEST_HEADING_FONT_SIZE, strlst);
}

void HDMIVideo::hardwareTestStart()
{
    testCondunctResult[0] = HW_TEST_PASS;
}

void HDMIVideo::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}
