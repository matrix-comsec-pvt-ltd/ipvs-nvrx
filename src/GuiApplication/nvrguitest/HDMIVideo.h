#ifndef HDMIVIDEO_H
#define HDMIVIDEO_H

#include "HardwareTestControl.h"

class HDMIVideo : public HardwareTestControl
{
    Q_OBJECT
public:
    explicit HDMIVideo(quint16 xPos, quint16 yPos, quint8 totalRow, quint8 totalCol, CONDUNCT_TEST_e hwIndex, QWidget *parent = 0);
    ~HDMIVideo() { };

    void hardwareTestStart();
    void hardwareTestStop() { };
    void saveHwTestStatus(CONDUNCT_TEST_e hwIndex);
};

#endif // HDMIVIDEO_H
