#include "UdevMonitor.h"
#include "MainWindow.h"

udevMonitor* udevMonitor:: udevMonitorIndex = NULL;

#define EHCI_BUS1           "/hiusb-ehci.0/usb1/1-2/1-2:1.0"
#define OHCI_BUS2           "/hiusb-ohci.0/usb2/2-2/2-2:1.0"
#define XHCI_BUS3           "/hiusb-xhci.0/usb3/3-1/3-1:1.0"
#define XHCI_BUS4           "/hiusb-xhci.0/usb4/4-1/4-1:1.0"

#if defined(RK3568_NVRL)
#define INPUT_USB_DEV_PATH  "/devices/platform/fd840000.usb/usb3/3-1/3-1:1.0"
#elif defined(RK3588_NVRH)
#define INPUT_USB_DEV_PATH  "/devices/platform/fc840000.usb/usb3/3-1/3-1:1.0"
#else
#define INPUT_USB_DEV_PATH  "/hiusb-ohci.0/usb2/2-1/2-1:1.0"
#endif

typedef enum
{
    HDD1 = 0,
    HDD2,
    HDD3,
    HDD4,
    HDD5,
    HDD6,
    HDD7,
    HDD8,
    MANUAL_BACKUP_DISK,
    SCHEDULE_BACKUP_DISK,
    USB2,
    MAX_RAW_MEDIA
}STORAGE_MEDIA_TYPE_e;

static const CHAR *rawMediaStr[2][MAX_RAW_MEDIA] =
{
#if defined(RK3568_NVRL)
    {
        "/devices/platform/fc400000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "/devices/platform/fc800000.sata/ata2/host1/target1:0:0/1:0:0:0/",  /* SATA Port 2 */
        "", "", "", "", "", "",                                             /* No SATA Port Present */
        "/devices/platform/usbdrd/fcc00000.dwc3/xhci-hcd.4.auto/",          /* USB3.0 Manual Backup */
        "/devices/platform/fd880000.usb/usb2/2-1/2-1:1.0/",                 /* USB2.0 Scheduled Backup */
        "/devices/platform/fd800000.usb/usb1/1-1/1-1:1.0/",                 /* USB2.0 Mouse Port */
    },
    {   /* For NVR0801XSP2 & NVR1601XSP2 */
        "/devices/platform/fc400000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "", "", "", "", "", "", "",                                         /* No SATA Port Present */
        "/devices/platform/usbdrd/fcc00000.dwc3/xhci-hcd.4.auto/",          /* USB3.0 Manual Backup */
        "/devices/platform/fd800000.usb/usb1/1-1/1-1:1.0/",                 /* USB2.0 Scheduled Backup */
        "",                                                                 /* No USB Port */
    }
#elif defined(RK3588_NVRH)
    {
        "/devices/platform/fe210000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:0:0/1:0:0:0/",  /* SATA Port 2 */
        "", "", "", "", "", "",                                             /* No SATA Port Present */
        "/devices/platform/usbdrd3_0/fc000000.usb/xhci-hcd.5.auto/",        /* USB3.0 Manual Backup */
        "/devices/platform/fc880000.usb/usb2/2-1/2-1:1.0/",                 /* USB2.0 Scheduled Backup */
        "/devices/platform/fc800000.usb/usb1/1-1/1-1:1.0/"                  /* USB2.0 Mouse Port */
    },
    {
        "/devices/platform/fe210000.sata/ata1/host0/target0:0:0/0:0:0:0/",  /* SATA Port 1 */
        "/devices/platform/fe210000.sata/ata1/host0/target0:1:0/0:1:0:0/",  /* SATA Port 2 */
        "/devices/platform/fe210000.sata/ata1/host0/target0:2:0/0:2:0:0/",  /* SATA Port 3 */
        "/devices/platform/fe210000.sata/ata1/host0/target0:3:0/0:3:0:0/",  /* SATA Port 4 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:0:0/1:0:0:0/",  /* SATA Port 5 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:1:0/1:1:0:0/",  /* SATA Port 6 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:2:0/1:2:0:0/",  /* SATA Port 7 */
        "/devices/platform/fe220000.sata/ata2/host1/target1:3:0/1:3:0:0/",  /* SATA Port 8 */
        "/devices/platform/usbdrd3_0/fc000000.usb/xhci-hcd.5.auto/",        /* USB3.0 Manual Backup */
        "/devices/platform/fc880000.usb/usb2/2-1/2-1:1.0/",                 /* USB2.0 Scheduled Backup */
        "/devices/platform/fc800000.usb/usb1/1-1/1-1:1.0/"                  /* USB2.0 Mouse Port */
    }
#else
    {
        "/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:0:0/1:0:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:0:0/2:0:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:0:0/3:0:0:0/",
        "/devices/platform/ahci.0/ata1/host0/target0:1:0/0:1:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:1:0/1:1:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:1:0/2:1:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:1:0/3:1:0:0/",
        "/devices/platform/hiusb-xhci.0/usb3/3-1/3-1:1.0/",
        "/devices/platform/hiusb-ehci.0/usb1/1-2/1-2:1.0/",
        "/devices/platform/hiusb-ehci.0/usb1/1-1/1-1:1.0/"
    },
    {
        "/devices/platform/ahci.0/ata1/host0/target0:0:0/0:0:0:0/",
        "/devices/platform/ahci.0/ata1/host0/target0:1:0/0:1:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:0:0/1:0:0:0/",
        "/devices/platform/ahci.0/ata2/host1/target1:1:0/1:1:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:0:0/2:0:0:0/",
        "/devices/platform/ahci.0/ata3/host2/target2:1:0/2:1:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:0:0/3:0:0:0/",
        "/devices/platform/ahci.0/ata4/host3/target3:1:0/3:1:0:0/",
        "/devices/platform/hiusb-xhci.0/usb3/3-1/3-1:1.0/",
        "/devices/platform/hiusb-ehci.0/usb1/1-2/1-2:1.0/",
        "/devices/platform/hiusb-ehci.0/usb1/1-1/1-1:1.0/"
    }
#endif
};

udevMonitor* udevMonitor::getInstance()
{
    if(udevMonitorIndex == NULL)
    {
        udevMonitorIndex = new udevMonitor();
    }
    return udevMonitorIndex;
}

udevMonitor::udevMonitor(QObject *parent) : QObject(parent)
{
    quint8 blockDevCnt;

    INIT_OBJ(storageStatus);
    INIT_OBJ(moniterTimer);

    storageStatus = new StorageManagment(this);
    if (IS_VALID_OBJ(storageStatus))
    {
        connect(storageStatus,
                 SIGNAL(act1(UDEV_DEVICE_INFO_t)),
                 this,
                 SLOT(slotact1(UDEV_DEVICE_INFO_t)));
    }

    moniterTimer = new QTimer(this);
    if (IS_VALID_OBJ(moniterTimer))
    {
        connect(moniterTimer,
                 SIGNAL(timeout()),
                 storageStatus,
                 SLOT(internalMsgServerLoop()));

        moniterTimer->setInterval(100);
        moniterTimer->start();
    }

    for(blockDevCnt = 0; blockDevCnt < MAX_HDD_PORT; blockDevCnt++)
    {
        m_isHDDConnected[blockDevCnt] = false;
    }

    for(blockDevCnt = 0; blockDevCnt < MAX_USB_PORT; blockDevCnt++)
    {
        m_isUSBConnected[blockDevCnt] = false;
    }
}

void udevMonitor::slotact1(UDEV_DEVICE_INFO_t devInfo)
{
    quint8  mediaType;
    quint8  usbIndex;
    QString str;
    QString subSys = devInfo.subSys;
    QString action = devInfo.action;
    QString path = devInfo.path;
    QString baseNode = devInfo.baseNode;
    bool    rawMediaStrIdx = BoardTypeWiseInfo::isMultipAvail;

    if (false == IS_VALID_OBJ(storageStatus))
    {
        return;
    }

    DPRINT(GUI_SYS, "[sub_system=%s] [action=%s] [dev_path=%s] [serial=%s] [node=%s]",
           devInfo.subSys, devInfo.action, devInfo.path, devInfo.serial, devInfo.baseNode);

    #if defined(RK3568_NVRL)
    /* We have added device nodes at first index for NVR0801XSP2 & NVR1601XSP2 */
    if ((BoardTypeWiseInfo::productVariant == NVR0401XSP2) || (BoardTypeWiseInfo::productVariant == NVR0801XSP2) || (BoardTypeWiseInfo::productVariant == NVR1601XSP2))
    {
        rawMediaStrIdx = true;
    }
    #endif

    for(mediaType = HDD1; mediaType < MAX_RAW_MEDIA; mediaType++)
    {
        #if defined(RK3568_NVRL) || defined(RK3588_NVRH)
        /* Device path is null then skip this media */
        if (rawMediaStr[rawMediaStrIdx][mediaType][0] == '\0')
        {
            /* This is not a valid media for comparision */
            continue;
        }

        if(path.contains(rawMediaStr[rawMediaStrIdx][mediaType]))
        {
            break;
        }
        #else
        if((path.contains("ahci.0")) || (path.contains("hiusb")))
        {
            if(path.contains(rawMediaStr[rawMediaStrIdx][mediaType]))
            {
                break;
            }
        }

        if((path.contains(XHCI_BUS3)) || (path.contains(XHCI_BUS4)))
        {
            mediaType = MANUAL_BACKUP_DISK;
            break;
        }

        if((path.contains(EHCI_BUS1)) || (path.contains(OHCI_BUS2)))
        {
            mediaType = SCHEDULE_BACKUP_DISK;
            break;
        }
        #endif
    }

    if ((subSys == "input") && (path.contains(INPUT_USB_DEV_PATH)))
    {
        /* We have only two USBs for NVR0801XSP2 & NVR1601XSP2 */
        #if defined(RK3568_NVRL)
        if ((BoardTypeWiseInfo::productVariant == NVR0401XSP2) || (BoardTypeWiseInfo::productVariant == NVR0801XSP2) || (BoardTypeWiseInfo::productVariant == NVR1601XSP2))
        {
            mediaType = SCHEDULE_BACKUP_DISK;
        }
        else
        #endif
        {
            mediaType = USB2;
        }
    }

    if (mediaType >= MAX_RAW_MEDIA)
    {
        return;
    }

    if (mediaType <= HDD8)
    {
        str = "HDD" + QString("%1").arg(mediaType + 1);
        if ((m_isHDDConnected[mediaType] == false) && ((action == "add") || (action == "change")))
        {
            if (true == storageStatus->mountDevice(str, baseNode))
            {
                m_isHDDConnected[mediaType] = true;
                DPRINT(GUI_SYS, "[%s] connected", str.toUtf8().data());
            }
        }
    }
    else
    {
        usbIndex = mediaType - MANUAL_BACKUP_DISK;
        str = "USB" + QString("%1").arg(usbIndex + 1);
        if ((m_isUSBConnected[usbIndex] == false) && ((action == "add") || (action == "change")))
        {
            if ((subSys == "input") || (true == storageStatus->mountDevice(str, baseNode)))
            {
                m_isUSBConnected[usbIndex] = true;
                DPRINT(GUI_SYS, "[%s] connected", str.toUtf8().data());
            }
        }

        if ((m_isUSBConnected[usbIndex] == true) && (action == "remove"))
        {
            if ((subSys == "input") || (true == storageStatus->unmountDevice(str)))
            {
                m_isUSBConnected[usbIndex] = false;
                DPRINT(GUI_SYS, "[%s] disconnected", str.toUtf8().data());
            }
        }
    }
}

udevMonitor::~udevMonitor()
{
    DELETE_OBJ(storageStatus);
    DELETE_OBJ(moniterTimer);
}

bool udevMonitor::getHDDConnectionStatus(quint8 subDeviceIndex)
{
    return m_isHDDConnected[subDeviceIndex];
}

bool udevMonitor::getUSBConnectionStatus(quint8 subDeviceIndex)
{
    return m_isUSBConnected[subDeviceIndex];
}

