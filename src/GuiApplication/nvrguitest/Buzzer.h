#ifndef BUZZER_H
#define BUZZER_H

#include "HardwareTestControl.h"
#include "FileIO.h"

class Buzzer : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit Buzzer(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0);
    ~Buzzer();

    void hardwareTestStart();
    void hardwareTestStop();
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);

private:
    bool    stopTest;
    INT32   m_buzzerDriverFd;

protected:
    void updateBuzzer(bool status);
    void timeOut(void);
};

#endif // BUZZER_H
