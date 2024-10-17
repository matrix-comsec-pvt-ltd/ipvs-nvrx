#ifndef BUZZERCONTROL_H
#define BUZZERCONTROL_H

#include <QWidget>
#include "ApplController.h"

class BuzzerControl : public QWidget
{
    Q_OBJECT
private:
    ApplController* m_applController;

public:
    explicit BuzzerControl(QWidget *parent = 0);

    void sendStopBuzzerCommand();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);
};

#endif // BUZZERCONTROL_H
