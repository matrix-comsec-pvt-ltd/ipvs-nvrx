#ifndef UDEVMONITOR_H
#define UDEVMONITOR_H

#include <QObject>
#include <QTimer>
#include "StorageManagment.h"

#define MAX_USB_PORT 3
#define MAX_HDD_PORT 8

class udevMonitor : public QObject
{
    Q_OBJECT
public:
    ~udevMonitor();
    static udevMonitor* getInstance();

    bool getHDDConnectionStatus(quint8 subDeviceIndex);
    bool getUSBConnectionStatus(quint8 subDeviceIndex);

    bool m_isHDDConnected[MAX_HDD_PORT];
    bool m_isUSBConnected[MAX_USB_PORT];

private:
    udevMonitor(QObject *parent = 0);
    void operator = (udevMonitor const &) {}

    StorageManagment*   storageStatus;
    QTimer*             moniterTimer;
    static udevMonitor* udevMonitorIndex;

public slots:
    void slotact1(UDEV_DEVICE_INFO_t devInfo);
};

#endif // UDEVMONITOR_H
