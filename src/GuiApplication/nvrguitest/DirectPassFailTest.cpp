#include "DirectPassFailTest.h"

DirectPassFailTest::DirectPassFailTest(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent)
    : HardwareTestControl(hwIndex, parent)
{
    QStringList strlst;

    switch (hwIndex)
    {
        case AUDIO_OUT_HW_TEST:
            strlst << "Audio OUT";
            break;

        case HDMI_AUDIO_HW_TEST:
            strlst << "HDMI Audio";
            break;

        default:
            break;
    }

    createDefaultComponent(xPos, yPos, totalRow, totalCol, true, false, TEST_HEADING_FONT_SIZE, strlst);
}

void DirectPassFailTest::saveHwTestStatus(CONDUNCT_TEST_e index)
{
    HardwareTestControl::testResult[index] = testCondunctResult[0];
}
