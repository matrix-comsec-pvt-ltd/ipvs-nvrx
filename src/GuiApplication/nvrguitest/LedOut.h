#ifndef LEDOUT_H
#define LEDOUT_H

#include "HardwareTestControl.h"
#include "FileIO.h"

typedef enum
{
    GREEN_LED_STATE,
    RED_LED_STATE,
    MAX_LED_STATE
}LED_STATUS_e;

class LedOut : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit LedOut(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0);
    ~LedOut();

    void hardwareTestStart();
    void hardwareTestStop();
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);

private:
    FileIO*     m_fileIO;
    quint8      LedState;
    bool        stopTest;

    quint8 getLedState();
protected:
    bool timeOut();
};

#endif // LEDOUT_H
