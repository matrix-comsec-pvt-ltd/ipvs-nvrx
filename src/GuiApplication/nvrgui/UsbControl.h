#ifndef USBCONTROL_H
#define USBCONTROL_H

#include <QWidget>
#include "Controls/MenuButton.h"
#include "EnumFile.h"
#include "PayloadLib.h"
#include "ApplController.h"

const QString usbEjectMsg[MAX_USB_TYPE] = {"Eject Manual Backup USB",
                                                    "Eject Schedule Backup USB"};

class UsbControl : public QWidget
{
    Q_OBJECT
private:
    int m_startX, m_bottomY, m_currentClickedUsb;
    MenuButton* m_usbEjectButton[MAX_USB_TYPE];

    ApplController* m_applController;
    PayloadLib* m_payloadLib;

public:
    explicit UsbControl( QWidget *parent = 0);
    ~UsbControl();

    static bool showUsbFlag[MAX_USB_TYPE];
    static int currentNoOfUsb;
    static void updateShowUsbFlag(int usbType, bool status);

    void setCurrentUsbStatus(int usbType, bool status);
    void resetGeometry();
    void unPlugUsb(int index);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e index);

public slots:
    void slotEjectButtonClicked(int index);
};

#endif // USBCONTROL_H
