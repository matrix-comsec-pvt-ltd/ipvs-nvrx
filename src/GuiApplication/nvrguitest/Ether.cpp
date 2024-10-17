#include "Ether.h"

static const QString ethStr[] =
{
    LAN1_DEV_LINK_STS_FILE,
    LAN2_DEV_LINK_STS_FILE
};

Ether::Ether(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent, quint8 noOfSubDev):
    HardwareTestControl(hwIndex, parent, noOfSubDev)
{
    QStringList strlst;

    totalLanPort = noOfSubDev;
    for(quint8 index = 0; index < totalLanPort; index++)
    {
        INIT_OBJ(m_fileIO[index]);
        m_fileIO[index] = new FileIO(this, ethStr[index]);
    }

    strlst << "Ethernet1" << "Ethernet2";
    createDefaultComponent(xPos, yPos, totalRow, totalCol, false, false, SUFFIX_FONT_SIZE, strlst, QString("Ethernet"));
}

Ether::~Ether()
{
    for(quint8 index = 0; index < totalLanPort; index++)
    {
        if(IS_VALID_OBJ(m_fileIO[index]))
        {
            DELETE_OBJ(m_fileIO[index]);
        }
    }
}

void Ether::hardwareTestStart()
{    
    for (quint8 index = 0; index < totalLanPort; index++)
    {
        if (m_fileIO[index]->read() == "1\n\n")
        {
            testCondunctResult[index] = HW_TEST_PASS;
        }
        else
        {
            testCondunctResult[index] = HW_TEST_FAIL;
        }
    }
}

void Ether::saveHwTestStatus(CONDUNCT_TEST_e hwIndex)
{
    HardwareTestControl::testResult[hwIndex] = getStatusOfHwTest();
}

