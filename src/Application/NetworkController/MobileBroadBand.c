//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		MobileBroadband.c
@brief      This file implements USB modem Releted Functions.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* OS Includes */
#include <dirent.h>

/* Application Includes */
#include "MobileBroadBand.h"
#include "DebugLog.h"
#include "Utils.h"
#include "InputOutput.h"
#include "TimeZone.h"
#include "NetworkInterface.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define USB_MODE_SWITCH_CMD             "usb_modeswitch -s 20 -v %s -p %s -c %s &"
#if defined(RK3588_NVRH)
#define MODE_SWITCH_FILE_NAME			SCRIPTS_DIR_PATH "/usb_modeswitch.d/%s:%s"
#else
#define MODE_SWITCH_FILE_NAME			SCRIPTS_DIR_PATH "/usb_modeswitch.d/%s_%s"
#endif
#define NEW_DEVICE_ID					"/sys/bus/usb-serial/drivers/option1/new_id"
#define	VENDOR_FILE_NAME                "../idVendor"
#define	PRODUCT_FILE_NAME               "../idProduct"
#define	END_POINT_FILE                  "ep_"
#define	END_POINT_TYPE_FILE             "%s%s/type"
#define	INTERRUPT_TYPE_TTY_NAME         "Interrupt"
#define	USB_POWER_ON_FIRST_TIME			(10)    //in Seconds
#define USB_POWER_RESET_TIMER  			(40)    //in Seconds
#define USB_POWER_ON_TIMER				(5)     //in Seconds
#define TTY_SELECTION_TIME_OUT			(5)     //in Seconds
#define MAX_TTY_NODE					(32)
#define MAX_MODEM_NAME_WIDTH			(100)

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef enum
{
    USB_MODEM_TYPE_TTY,
    USB_MODEM_TYPE_ETH,
    USB_MODEM_TYPE_MAX
}USB_MODEM_TYPE_e;

typedef	enum
{
    USB_MODEM_PORT_USB2_1_1 = 0,    /* USB2.0 Port 1 Node 1 */
    USB_MODEM_PORT_USB2_2_1,        /* USB2.0 Port 2 Node 1 */
    USB_MODEM_PORT_USB2_1_2,        /* USB2.0 Port 1 Node 2 */
    USB_MODEM_PORT_USB2_2_2,        /* USB2.0 Port 2 Node 2 */
    USB_MODEM_PORT_USB3_1_1,        /* USB3.0 Port 1 Node 1 */
    USB_MODEM_PORT_MAX,
}USB_MODEM_PORT_e;

typedef enum
{
    TTY_SELECTION_TIME,
    USB_RESET_TIME,
    MAX_USB_TIMER_TYPE
}TIMER_STATE_e;

typedef struct
{
    CHAR				ttyNodeName[MAX_DEVICE_INFO_LENGTH];
}TTY_NODE_TYPE_t;

typedef	struct
{
    CHAR				vendorId[MAX_DEVICE_INFO_LENGTH];
    CHAR				productId[MAX_DEVICE_INFO_LENGTH];
    UINT8				totalNodes;
    UINT8				currentNode;
    TTY_NODE_TYPE_t		ttyNodeInfo[MAX_TTY_NODE];
    CHAR				devpath[MAX_DEVICE_PATH_LENGTH];
}PPPD_TTY_NODE_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static USB_MODEM_TYPE_e         usbModemType = USB_MODEM_TYPE_MAX;
static CHAR                     modemName[MAX_MODEM_NAME_WIDTH];
static TIMER_HANDLE				usbDevDetectTmrHndl = INVALID_TIMER_HANDLE;
static TIMER_HANDLE				resetUsbTmrHndl = INVALID_TIMER_HANDLE;
static PPPD_TTY_NODE_t			ppdTtyNodeInfo;

static const CHARPTR usbModemDevicePath[USB_MODEM_PORT_MAX] =
{
    #if defined(RK3568_NVRL)
    "/devices/platform/fd800000.usb/usb1/1-1",
    "/devices/platform/fd880000.usb/usb2/2-1",
    "/devices/platform/fd800000.usb/usb1/1-1",
    "/devices/platform/fd880000.usb/usb2/2-1",
    "/devices/platform/usbdrd/fcc00000.dwc3/xhci-hcd.4.auto"
    #elif defined(RK3588_NVRH)
    "/devices/platform/fc800000.usb/usb1/1-1",
    "/devices/platform/fc880000.usb/usb2/2-1",
    "/devices/platform/fc800000.usb/usb1/1-1",
    "/devices/platform/fc880000.usb/usb2/2-1",
    "/devices/platform/usbdrd3_0/fc000000.usb/xhci-hcd.5.auto"
    #else
    "/devices/platform/hiusb-ohci.0/usb2/2-1",
    "/devices/platform/hiusb-ohci.0/usb2/2-2",
    "/devices/platform/hiusb-ehci.0/usb1/1-1",
    "/devices/platform/hiusb-ehci.0/usb1/1-2",
    "/devices/platform/hiusb-xhci.0/usb3/3-1"
    #endif
};

static const UINT32 usbPowerState[USB_MODEM_PORT_MAX][2] =
{
    {USB2POWER1_OFF, USB2POWER1_ON},
    {USB2POWER2_OFF, USB2POWER2_ON},
    {USB2POWER1_OFF, USB2POWER1_ON},
    {USB2POWER2_OFF, USB2POWER2_ON},
    {USB3POWER_OFF,  USB3POWER_ON},
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static void powerOnAllUsb(UINT32 data);
//-------------------------------------------------------------------------------------------------
static USB_MODEM_PORT_e getUsbPortFromDevPath(const CHAR *devPath);
//-------------------------------------------------------------------------------------------------
static void setUsbDevDetectTimer(UINT32 seconds, BOOL pinState, TIMER_STATE_e timerState, USB_MODEM_PORT_e usbPort);
//-------------------------------------------------------------------------------------------------
static void usbDevDetectTimerCallback(UINT32 data);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   It initialise Mobile Broadband configuration file.
 */
void InitMobileBroadband(void)
{
    TIMER_INFO_t timerInfo;

    memset(&ppdTtyNodeInfo, 0, sizeof(ppdTtyNodeInfo));
    usbDevDetectTmrHndl = INVALID_TIMER_HANDLE;
    resetUsbTmrHndl = INVALID_TIMER_HANDLE; // this handle is use only at init time.

    /* Power off all USB ports */
    UsbControlChangeCmd(USBALLPOWER_OFF);

    //Timer is use for making usb port ON at init time Only.
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(USB_POWER_ON_FIRST_TIME);
    timerInfo.funcPtr = &powerOnAllUsb;
    timerInfo.data = 0;
    StartTimer(timerInfo, &resetUsbTmrHndl);
    DPRINT(ETHERNET, "start timer to enable power of all usb ports");
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function handles powering ON usb port on timeout.
 * @param   data
 */
static void powerOnAllUsb(UINT32 data)
{
    DeleteTimer(&resetUsbTmrHndl);
    UsbControlChangeCmd(ALLUSBPOWER_ON);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   It initialise Mobile Broadband configuration file.
 * @param   newCopy
 * @param   oldCopy
 */
void MobileBroadbandCfgUpdate(BROAD_BAND_CONFIG_t newCopy, BROAD_BAND_CONFIG_t *oldCopy)
{
    /* If tty device is not attached then nothing to do */
    if (usbModemType != USB_MODEM_TYPE_TTY)
    {
        return;
    }

    if (newCopy.activeProfile == 0)
    {
        /* Nothing to do if config cleared but modem is not connected */
        if (GetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM) == MODEM_NOT_PRESENT)
        {
            return;
        }

        /* Stop usb modem communication */
        StopUsbToSerialModem(NETWORK_PORT_USB_MODEM);
        OnIpv4PppoeIpDown(NETWORK_PORT_USB_MODEM);

        /* Deregister USB modem with network manager */
        DeregisterNetworkPort(NETWORK_PORT_USB_MODEM);
        usbModemType = USB_MODEM_TYPE_MAX;
        return;
    }

    if ((newCopy.activeProfile != oldCopy->activeProfile)
            || (memcmp(&newCopy.broadBandCfg[newCopy.activeProfile - 1],
                       &oldCopy->broadBandCfg[oldCopy->activeProfile - 1], sizeof(BROAD_BAND_PROFILE_t))))
    {
        DPRINT(ETHERNET, "active profile parameters changed: [activeProfile=%d]", newCopy.activeProfile);

        if (INVALID_TIMER_HANDLE != usbDevDetectTmrHndl)
        {
            DeleteTimer(&usbDevDetectTmrHndl);
            UsbControlChangeCmd(ALLUSBPOWER_ON);
        }

        ppdTtyNodeInfo.currentNode = 0;
        setUsbDevDetectTimer(USB_POWER_ON_TIMER, OFF, USB_RESET_TIME, getUsbPortFromDevPath(ppdTtyNodeInfo.devpath));
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get USB port from device path
 * @param   devPath
 * @return
 */
static USB_MODEM_PORT_e getUsbPortFromDevPath(const CHAR *devPath)
{
    USB_MODEM_PORT_e usbPort;

    for (usbPort = 0; usbPort < USB_MODEM_PORT_MAX; usbPort++)
    {
        if (NULL != strstr(devPath, usbModemDevicePath[usbPort]))
        {
            break;
        }
    }

    return usbPort;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function will load timer for switching power to Modem USB Port.
 * @param   count
 * @param   pinState
 * @param   timerState
 * @param   usbPort
 */
static void setUsbDevDetectTimer(UINT32 seconds, BOOL pinState, TIMER_STATE_e timerState, USB_MODEM_PORT_e usbPort)
{
    TIMER_INFO_t timerInfo;

    if (usbPort >= USB_MODEM_PORT_MAX)
    {
        return;
    }

    DeleteTimer(&usbDevDetectTmrHndl);
    timerInfo.count = CONVERT_SEC_TO_TIMER_COUNT(seconds);
    timerInfo.data = ((usbPort << 16) | (timerState << 8) | pinState);
    timerInfo.funcPtr = usbDevDetectTimerCallback;
    StartTimer(timerInfo, &usbDevDetectTmrHndl);

    DPRINT(ETHERNET, "start usb device detect timer: [timerState=%d], [pinState=%d], [time=%u sec], [usbPort=%d]",
           timerState, pinState, seconds, usbPort);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function handles powering ON or OFF to usb port on timeout.
 * @param   data
 */
static void usbDevDetectTimerCallback(UINT32 data)
{
    UINT8               pinState = (data & 0xFF);
    TIMER_STATE_e       timerState = ((data >> 8) & 0xFF);
    USB_MODEM_PORT_e    usbPort = (data >> 16) & 0xFF;

    /* Delete cyclic timer */
    DeleteTimer(&usbDevDetectTmrHndl);

    if (timerState == USB_RESET_TIME)
    {
        /* Validate input parameters */
        if ((pinState > 1) || (usbPort >= USB_MODEM_PORT_MAX))
        {
            EPRINT(ETHERNET, "invld usb power port param: [pinState=%d], [usbPort=%d]", pinState, usbPort);
            return;
        }

        DPRINT(ETHERNET, "usb port power: [pinState=%d], [usbPort=%d]", pinState, usbPort);
        UsbControlChangeCmd(usbPowerState[usbPort][pinState]);
        if (pinState == OFF)
        {
            setUsbDevDetectTimer(USB_POWER_ON_TIMER, ON, USB_RESET_TIME, usbPort);
        }
    }
    else
    {
        /* try with each tty node one by one */
        if (ppdTtyNodeInfo.currentNode < ppdTtyNodeInfo.totalNodes)
        {
            BROAD_BAND_CONFIG_t broadBandConfig;

            /* Is usb modem registered with network manager? */
            if (USB_MODEM_TYPE_TTY == usbModemType)
            {
                /* Deregister current node and try next node */
                DeregisterNetworkPort(NETWORK_PORT_USB_MODEM);
                usbModemType = USB_MODEM_TYPE_MAX;
            }

            /* Update modem status as disconnected */
            SetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM, MODEM_DISCONNECTED);

            /* Read broadband config */
            ReadBroadBandConfig(&broadBandConfig);
            if (broadBandConfig.activeProfile == 0)
            {
                EPRINT(ETHERNET, "usb modem profile is not selected");
                return;
            }

            snprintf(modemName, sizeof(modemName), "%s", ppdTtyNodeInfo.ttyNodeInfo[ppdTtyNodeInfo.currentNode].ttyNodeName);
            ppdTtyNodeInfo.currentNode++;

            /* Start usb to serial modem communication */
            DPRINT(ETHERNET, "start usb to serial comm: [ttyName=%s], [time=%dsec]", modemName, USB_POWER_RESET_TIMER);

            /* Is usb modem registered with network manager? */
            if (USB_MODEM_TYPE_MAX == usbModemType)
            {
                /* Register USB modem with network manager */
                RegisterNetworkPort(NETWORK_PORT_USB_MODEM, modemName);
                StartUsbToSerialModem(NETWORK_PORT_USB_MODEM, &broadBandConfig.broadBandCfg[broadBandConfig.activeProfile - 1]);
                usbModemType = USB_MODEM_TYPE_TTY;
                setUsbDevDetectTimer(USB_POWER_RESET_TIMER, OFF, TTY_SELECTION_TIME, usbPort);
            }
        }
        else
        {
            ppdTtyNodeInfo.currentNode = 0;
            DPRINT(ETHERNET, "all tty node has been tried but dongle did not connect. reseting usb port");
            setUsbDevDetectTimer(TTY_SELECTION_TIME_OUT, OFF, USB_RESET_TIME, usbPort);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets detected device and perform usb mode switch for generating tty modem port
 * @param   device
 */
void DetectBroadBandDevice(BROADBAND_DEVICE_INFO_t *device)
{
    CHAR fileName[150];
    CHAR sysCmd[500];
    FILE *pFile;

    /* do noting if action is other than add */
    if (strcmp(device->action, "add") != 0)
    {
        DPRINT(ETHERNET, "broadband device info: [action=%s], [path=%s]", device->action, device->path);
        return;
    }

    DPRINT(ETHERNET, "broadband device info: [action=%s], [path=%s], [vendorId=%s], [productId=%s], "
           "[interfaceNumber=%s], [Class=%s], [SubClass=%s], [Protocol=%s]",
           device->action, device->path, device->vendorId, device->productId, device->interfaceNumber,
           device->interfaceClass, device->interfaceSubClass, device->interfaceProtocol);

    /* class: 0x08: Communications and CDC Data
     * subclass: 0x06: Ethernet Networking Control Model (ENCM)
     * protocol: 0x50: Vendor-specific
     */
    if ((strcmp(device->interfaceClass, "08") != 0) || (strcmp(device->interfaceSubClass, "06") != 0) || (strcmp(device->interfaceProtocol, "50") != 0))
    {
        //EPRINT(ETHERNET, "skip broadband usb interface: unsupported class/subclass/protocol");
        return;
    }

    /* add new vendor/device id to allow serial communication */
    pFile = fopen(NEW_DEVICE_ID, "w");
    if(pFile != NULL)
    {
        DPRINT(ETHERNET, "trigger kernel option driver: [vendorId=%s], [productId=%s]", device->vendorId, device->productId);
        fprintf(pFile, "%s %s", device->vendorId, device->productId);
        fclose(pFile);
    }

    if (strcmp(device->interfaceNumber, "00") != STATUS_OK)
    {
        EPRINT(ETHERNET, "skip broadband usb interface: [interfaceNumber=%s]", device->interfaceNumber);
        return;
    }

    snprintf(fileName, sizeof(fileName), MODE_SWITCH_FILE_NAME, device->vendorId, device->productId);
    if (access(fileName, F_OK) != STATUS_OK)
    {
        EPRINT(ETHERNET, "usb modeswitch config parameter file not present: [file=%s]", fileName);
        return;
    }

    snprintf(sysCmd, sizeof(sysCmd), USB_MODE_SWITCH_CMD, device->vendorId, device->productId, fileName);
    RESET_STR_BUFF(ppdTtyNodeInfo.vendorId);
    RESET_STR_BUFF(ppdTtyNodeInfo.productId);

    if (TRUE != ExeSysCmd(TRUE, sysCmd))
    {
        EPRINT(ETHERNET, "system cmd fail: [cmd=%s]", sysCmd);
    }

    /* reset usb port to detect usb as tty device */
    setUsbDevDetectTimer(USB_POWER_ON_TIMER, OFF, USB_RESET_TIME, getUsbPortFromDevPath(device->path));
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets detected device and start pppd demoan.
 * @param   action
 * @param   devPath
 * @param   ttyNode
 */
void DetectTtyDevice(const CHAR *action, const CHAR *devPath, const CHAR *ttyNode)
{
    CHARPTR             tmpStr = NULL;
    CHAR                nodePath[MAX_DEVICE_PATH_LENGTH] = {'\0'};
    CHAR                fileName[256];
    CHAR                buff[20];
    INT32               fileFd = INVALID_FILE_FD;
    INT32               readCnt = 0;
    DIR                 *dir;
    struct dirent       *entry;
    USB_MODEM_PORT_e    usbPort;
    BOOL                isTtyEpTypeInterrupt = FALSE;

    DPRINT(ETHERNET, "tty device info: [action=%s], [ttyNode=%s], [devPath=%s]", action, ttyNode, devPath);

    tmpStr = strstr(devPath, ttyNode);
    if (tmpStr == NULL)
    {
        EPRINT(ETHERNET, "tty node name doesn't exist in tty path");
        return;
    }

    snprintf(nodePath, MAX_DEVICE_PATH_LENGTH, "/sys");
    snprintf(nodePath + strlen("/sys"), (strlen(devPath) - strlen(tmpStr))+1, "%s", devPath);

    /* i.e. /sys/devices/platform/fd800000.usb/usb1/1-1/1-1:1.0/ */
    DPRINT(ETHERNET, "tty real path: [path=%s]", nodePath);

    /* find the endpoint of type 'Bulk' in device tree */
    if (strcmp(action, "add") == STATUS_OK)
    {
        dir = opendir(nodePath);
        if (dir == NULL)
        {
            EPRINT(ETHERNET, "fail to open dir: [path=%s], [err=%s]", nodePath, STR_ERR);
            return;
        }

        /* store the vendor id and product id only once as it will be same for all tty devices */
        if (ppdTtyNodeInfo.vendorId[0] == '\0')
        {
            snprintf(fileName, sizeof(fileName), "%s%s", nodePath, VENDOR_FILE_NAME);
            fileFd = open(fileName, READ_ONLY_MODE, USR_R_GRP_R_OTH_R);
            if (fileFd != INVALID_FILE_FD)
            {
                RESET_STR_BUFF(ppdTtyNodeInfo.vendorId);
                readCnt = read(fileFd, ppdTtyNodeInfo.vendorId, MAX_DEVICE_INFO_LENGTH);
                if(readCnt > 0)
                {
                    ppdTtyNodeInfo.vendorId[readCnt-1] = '\0';
                }
                close(fileFd);
            }
        }

        if (ppdTtyNodeInfo.productId[0] == '\0')
        {
            snprintf(fileName, sizeof(fileName), "%s%s", nodePath, PRODUCT_FILE_NAME);
            fileFd = open(fileName, READ_ONLY_MODE, USR_R_GRP_R_OTH_R);
            if(fileFd != INVALID_FILE_FD)
            {
                RESET_STR_BUFF(ppdTtyNodeInfo.productId);
                readCnt = read(fileFd, ppdTtyNodeInfo.productId, MAX_DEVICE_INFO_LENGTH);
                if(readCnt > 0)
                {
                    ppdTtyNodeInfo.productId[readCnt-1] = '\0';
                }
                close(fileFd);
            }
        }

        /* find 'ep_' directory */
        while ((entry = readdir(dir)) != NULL)
        {
            if (DT_DIR != entry->d_type)
            {
                continue;
            }

            if (NULL == strstr(entry->d_name, END_POINT_FILE))
            {
                continue;
            }

            /* identify tty node type from file
             * i.e. /sys/devices/platform/fd800000.usb/usb1/1-1/1-1:1.0/ep_02/type
             */
            snprintf(fileName, sizeof(fileName), END_POINT_TYPE_FILE, nodePath, entry->d_name);

            fileFd = open(fileName, READ_ONLY_MODE, USR_R_GRP_R_OTH_R);
            if (fileFd == INVALID_FILE_FD)
            {
                EPRINT(ETHERNET, "failed to open file: [file=%s], [err=%s]", fileName, STR_ERR);
                continue;
            }

            readCnt = read(fileFd, buff, sizeof(buff)-1);
            close(fileFd);

            if (readCnt <= 0)
            {
                EPRINT(ETHERNET, "failed to read file: [file=%s], [err=%s]", fileName, STR_ERR);
                continue;
            }

            buff[readCnt] = '\0';

            if (strstr(buff, INTERRUPT_TYPE_TTY_NAME) != NULL)
            {
                isTtyEpTypeInterrupt = TRUE;
                break;
            }
        }

        closedir(dir);

        /* store tty node of type interrupt only */
        if (isTtyEpTypeInterrupt == FALSE)
        {
            return;
        }

        snprintf(ppdTtyNodeInfo.ttyNodeInfo[ppdTtyNodeInfo.totalNodes].ttyNodeName, MAX_DEVICE_INFO_LENGTH, "%s", ttyNode);
        DPRINT(ETHERNET, "store interrupt type tty node: [ttyIndex=%d], [ttyNode=%s]",
               ppdTtyNodeInfo.totalNodes, ppdTtyNodeInfo.ttyNodeInfo[ppdTtyNodeInfo.totalNodes].ttyNodeName);

        /* increment total tty node count */
        ppdTtyNodeInfo.totalNodes++;

        /* Get usb port from device node path */
        usbPort = getUsbPortFromDevPath(nodePath);
        if (usbPort >= USB_MODEM_PORT_MAX)
        {
            return;
        }

        /* save tty node real path */
        snprintf(ppdTtyNodeInfo.devpath, MAX_DEVICE_PATH_LENGTH, "%s", nodePath);
        setUsbDevDetectTimer(TTY_SELECTION_TIME_OUT, OFF, TTY_SELECTION_TIME, usbPort);
    }
    else if(strcmp(action, "remove") == STATUS_OK)
    {
        /* Is usb modem deregistered with network manager? */
        if (usbModemType == USB_MODEM_TYPE_TTY)
        {
            StopUsbToSerialModem(NETWORK_PORT_USB_MODEM);
            OnIpv4PppoeIpDown(NETWORK_PORT_USB_MODEM);

            /* Deregister USB modem with network manager */
            DeregisterNetworkPort(NETWORK_PORT_USB_MODEM);
            usbModemType = USB_MODEM_TYPE_MAX;
        }

        SetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM, MODEM_NOT_PRESENT);
        RESET_STR_BUFF(modemName);
        memset(&ppdTtyNodeInfo, 0, sizeof(ppdTtyNodeInfo));

        /* Get usb port from device node path */
        usbPort = getUsbPortFromDevPath(nodePath);
        if (usbPort >= USB_MODEM_PORT_MAX)
        {
            return;
        }

        /* Reset USB on modem disconnection */
        setUsbDevDetectTimer(USB_POWER_ON_TIMER, ON, USB_RESET_TIME, usbPort);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Net device detected
 * @param   action
 * @param   devNode
 */
void DetectNetDevice(const CHAR *action, const CHAR *devNode)
{
    DPRINT(ETHERNET, "net device info: [action=%s], [devNode=%s]", action, devNode);

    /* Is it LAN1 interface (eth0)? */
    if (strcmp(devNode, GetLanPortName(LAN1_PORT)) == 0)
    {
        /* Skip this interface */
        DPRINT(ETHERNET, "skipped net device: [devNode=%s]", devNode);
        return;
    }

    /* Is it LAN2 interface (eth1)? */
    if (strcmp(devNode, GetLanPortName(LAN2_PORT)) == 0)
    {
        /* Skip this interface */
        DPRINT(ETHERNET, "skipped net device: [devNode=%s]", devNode);
        return;
    }

    /* Handle add and remove event */
    if (strcmp(action, "add") == STATUS_OK)
    {
        /* Is no usb modem attached? */
        if (usbModemType == USB_MODEM_TYPE_MAX)
        {
            /* Register USB modem with network manager */
            RegisterNetworkPort(NETWORK_PORT_USB_MODEM, devNode);
            SetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM, MODEM_DISCONNECTED);
            NetworkPortLinkUp(NETWORK_PORT_USB_MODEM);
            SetIpv4DhcpMode(NETWORK_PORT_USB_MODEM);
            EnableIpv6OnInterface(NETWORK_PORT_USB_MODEM);
            SetIpv6SlaacMode(NETWORK_PORT_USB_MODEM);
            usbModemType = USB_MODEM_TYPE_ETH;
        }
    }
    else if (strcmp(action, "remove") == STATUS_OK)
    {
        /* Is USB to ETH modem attached? */
        if (usbModemType == USB_MODEM_TYPE_ETH)
        {
            /* Deregister USB modem with network manager */
            OnIpv4DhcpLeaseFail(NETWORK_PORT_USB_MODEM);
            OnIpv6DhcpUnbound(NETWORK_PORT_USB_MODEM);
            DeregisterNetworkPort(NETWORK_PORT_USB_MODEM);
            SetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM, MODEM_NOT_PRESENT);
            usbModemType = USB_MODEM_TYPE_MAX;
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Usb modem communication status changed from connect to disconnect and vice-versa
 * @param   isStatusConnected
 */
void UsbModemStatusChanged(BOOL isStatusConnected)
{
    if (TRUE == isStatusConnected)
    {
        InitAutoTimezoneSrch();
        DeleteTimer(&usbDevDetectTmrHndl);
    }
    else
    {
        DeinitAutoTimezoneSrch();

        if (usbModemType != USB_MODEM_TYPE_TTY)
        {
            return;
        }

        // If modem is present when ppp is down, start timer to reset usb port power
        if (GetNetworkPortLinkStatus(NETWORK_PORT_USB_MODEM) == MODEM_NOT_PRESENT)
        {
            return;
        }

        setUsbDevDetectTimer(USB_POWER_RESET_TIMER, OFF, USB_RESET_TIME, getUsbPortFromDevPath(ppdTtyNodeInfo.devpath));
    }
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
