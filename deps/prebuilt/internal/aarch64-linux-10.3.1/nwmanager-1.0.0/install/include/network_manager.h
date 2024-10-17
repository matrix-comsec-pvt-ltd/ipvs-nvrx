/******************************************************************************
 * Copyright(c), Matrix ComSec Pvt. Ltd.
 *
 * All right reserved. Matrix's source code is an unpublished work and
 * the use of copyright notice does not imply otherwise.
 *
 * This source code contains confidential, trade secret material of MATRIX.
 * Any attempt or participation in deciphering, decoding, reverse engineering
 * or in any way altering the source code is strictly prohibited, unless the
 * prior written consent of Matrix ComSec is obtained.
 *
 *****************************************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file network_manager.h
 * @brief Network Manager API
 *
 * This header file contains the declarations for the Network Manager API,
 * which provides functions to manage network interfaces, configure network
 * settings, and handle network events.
 *
 * It encapsulates declarations for functions that enable developers to interact
 * with network-related functionalities in their applications.
 *
 * By including this header file and utilizing the provided functions, developers can
 * effectively manage network operations, configure network parameters, and respond to
 * various network events within their software systems.
 *
 * @note
 * - These APIs are intended for use with Matrix's embedded Linux-based devices.
 * - All functions in this API should be called after initializing the library
 * using NMLib_Init() and before exiting the library using NMLib_Exit().
 * - Basically there are two types of APIs synchronous and asynchronous.
 * Synchronous APIs will work in blocking manner until given operation in not completed.
 * While asynchronous APIs will return with im-progress status code as soon as
 * operation is accepted and acknowledged by network manager daemon.
 * - In case of asynchronous operations, success or failure status will be provided
 * through application event callback.
 *
 * @warning
 * - If any specific sequence is required to achieve any functionality then
 * network manager will not execute that sequence automatically. Application
 * needs to call the series of APIs as per the behavior on their platform.
 * i.e.
 * If a specific sequence is required to create VLAN on the interface:
 * Down the parent interface -> Create VLAN interface -> Up the parent
 * interface -> Up the VLAN interface, the application needs to call
 * series of APIs.
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#include <stdio.h>
#include <net/if.h>
#include <limits.h>
#include <netdb.h>

#include "nm_stddef.h"
#include "nm_platformdef.h"
#include "nm_iputility.h"

/*****************************************************************************
 * MACROS
 *****************************************************************************/

/**
 * @brief The value representing an invalid interface ID.
 */
#define NM_INVALID_INTERFACE_ID (0xFF)

/**
 * @brief The maximum length of a fully qualified domain name (FQDN) string.
 */
#define NM_FQDN_STR_LEN_MAX (64)

/**
 * @brief The maximum length of a MAC Address string.
 */
#define NM_MAC_ADDR_STR_LEN_MAX (18)

/**
 * @brief The maximum length allowed for a PPP username.
 */
#define NM_PPP_USERNAME_LEN_MAX (61)

/**
 * @brief The maximum length allowed for a PPP password.
 */
#define NM_PPP_PASSWORD_LEN_MAX (100)

/**
 * @brief The maximum length allowed for an 802.1x username used for wired authentication.
 */
#define NM_WIRED_8021X_USERNAME_LEN_MAX (41)

/**
 * @brief The maximum length allowed for an 802.1x password used for wired authentication.
 */
#define NM_WIRED_8021X_PASSWORD_LEN_MAX (100)

/**
 * @brief The maximum interval, in seconds, for status polling during wired 802.1x authentication.
 */
#define NM_WIRED_8021X_POLL_INTERVAL_SEC_MAX (600)

/**
 * @brief The maximum length allowed for the name of a USB modem.
 */
#define NM_USB_MODEM_NAME_LEN_MAX (101)

/**
 * @brief The maximum length allowed for the dial number of a USB modem.
 */
#define NM_USB_MODEM_DIAL_NUM_LEN_MAX (21)

/**
 * @brief The maximum length allowed for the Access Point Name (APN) of a USB modem.
 */
#define NM_USB_MODEM_APN_LEN_MAX (41)

/**
 * @brief The maximum length allowed for the Service Set Identifier (SSID) of a wifi.
 */
#define NM_WIFI_SSID_LEN_MAX (33)

/**
 * @brief The maximum length allowed for a username used for wifi authentication.
 */
#define NM_WIFI_USERNAME_LEN_MAX (51)

/**
 * @brief The maximum length allowed for a password used for wifi authentication.
 */
#define NM_WIFI_PASSWORD_LEN_MAX (65)

/**
 * @brief The maximum number of available wifi network entries that will be provided.
 *
 * @note Currently this value is fixed and not configurable.
 * Contact network manager modules owners if you need more entries of available wifi networks.
 */
#define NM_WIFI_NETWORK_SEARCH_ENTRY_MAX (15)

/**
 * @brief The maximum length of device pin for wifi wps authentication.
 */
#define NM_WIFI_WPS_DEVICE_PIN_LEN_MAX (9)

/**
 * @brief The maximum number of nameservers that can be configured.
 *
 * @note Currently this value is fixed and not configurable. Contact network manager modules owners
 * if you need to support more nameservers.
 */
#define NM_DNS_NAMESERVER_ENTRY_MAX (4)

/**
 * @brief The maximum number of entries that can be configured in /etc/hosts.
 *
 * @note Currently this value is fixed and not configurable. Contact network manager modules owners
 * if you need to support more entries in /etc/hosts file.
 */
#define NM_HOSTS_ENTRY_MAX (8)

/*****************************************************************************
 * ENUMS
 *****************************************************************************/

/**
 * @brief Types of log levels used by the Network Manager.
 */
typedef enum
{
    NM_LOG_OFF,          /**< No logging */
    NM_LOG_FATAL,        /**< Fatal error logging */
    NM_LOG_ERROR,        /**< Error logging */
    NM_LOG_WARN,         /**< Warning logging */
    NM_LOG_INFO,         /**< Informational logging */
    NM_LOG_DEBUG,        /**< Debug logging */
    NM_LOG_TRACE         /**< Trace logging */

} NM_LogLevel_e;

/**
 * @brief Types of log sinks for the Network Manager.
 */
typedef enum
{
    NM_LOG_SINK_NONE,     /**< Don't send logs to anywhere */
    NM_LOG_SINK_CALLBACK, /**< Send logs to a callback function */
    NM_LOG_SINK_SYSLOG    /**< Send logs to syslog server */

} NM_LogSink_e;

/**
 * @brief Type of network devices supported by network manager
 */
typedef enum
{
    NM_DEVICE_TYPE_ETHERNET,  /**< Ethernet device */
    NM_DEVICE_TYPE_VLAN,      /**< VLAN (Virtual LAN) device */
    NM_DEVICE_TYPE_USB_MODEM, /**< USB modem device */
    NM_DEVICE_TYPE_WIFI,      /**< WiFi device */
    NM_DEVICE_TYPE_MAX        /**< Invalid device type */

} NM_DeviceType_e;

/**
 * @brief Authentication types for wired 802.1x security
 */
typedef enum
{
    NM_WIRED_8021X_AUTH_TYPE_MD5,   /**< MD5 authentication */
    NM_WIRED_8021X_AUTH_TYPE_MAX    /**< Invalid authentication type */

} NM_Wired8021xAuthType_e;

/**
 * @brief WiFi security types.
 */
typedef enum
{
    NM_WIFI_SECURITY_TYPE_NO_AUTH,         /**< No authentication (Open Network) */
    NM_WIFI_SECURITY_TYPE_WEP,             /**< WEP security */
    NM_WIFI_SECURITY_TYPE_WPA_PERSONAL,    /**< WPA Personal security */
    NM_WIFI_SECURITY_TYPE_WPA2_PERSONAL,   /**< WPA2 Personal security */
    NM_WIFI_SECURITY_TYPE_WPA_ENTERPRISE,  /**< WPA Enterprise security */
    NM_WIFI_SECURITY_TYPE_WPA2_ENTERPRISE, /**< WPA2 Enterprise security */
    NM_WIFI_SECURITY_TYPE_MAX              /**< Invalid security type */

} NM_WifiSecurityType_e;

/**
 * @brief WPA key encryption types.
 */
typedef enum
{
    NM_WPA_KEY_ENCRYPTION_TYPE_PSK,     /**< Pre-Shared Key (PSK) */
    NM_WPA_KEY_ENCRYPTION_TYPE_EAP,     /**< Extensible Authentication Protocol (EAP) */
    NM_WPA_KEY_ENCRYPTION_TYPE_MAX      /**< Invalid key encryption type */

} NM_WpaKeyEncryptionType_e;

/**
 * @brief Wifi EAP authentication types.
 */
typedef enum
{
    NM_EAP_AUTH_TYPE_TLS,             /**< Transport Layer Security (TLS) */
    NM_EAP_AUTH_TYPE_LEAP,            /**< Lightweight Extensible Authentication Protocol (LEAP) */
    NM_EAP_AUTH_TYPE_FAST,            /**< Flexible Authentication via Secure Tunneling (FAST) */
    NM_EAP_AUTH_TYPE_TUNNELED_TLS,    /**< Tunneled Transport Layer Security (TTLS) */
    NM_EAP_AUTH_TYPE_PEAP,            /**< Protected Extensible Authentication Protocol (PEAP) */
    NM_EAP_AUTH_TYPE_MAX              /**< Invalid EAP auth type */

} NM_WifiEapAuthType_e;

/**
 * @brief PEAP version for EAP PEAP authentication type.
 */
typedef enum
{
    NM_EAP_PEAP_VERSION_0,         /**< PEAP version 0 */
    NM_EAP_PEAP_VERSION_1,         /**< PEAP version 1 */
    NM_EAP_PEAP_VERSION_LABEL_1,   /**< PEAP label 1 */
    NM_EAP_PEAP_VERSION_MAX        /**< Invalid PEAP version */

} NM_WifiEapPeapVersion_e;

/**
 * @brief Wifi EAP inner auth types.
 */
typedef enum
{
    NM_EAP_INNER_AUTH_MSCHAPV2,    /**< MSCHAPv2 inner authentication */
    NM_EAP_INNER_AUTH_MD5,         /**< MD5 inner authentication */
    NM_EAP_INNER_AUTH_GTC,         /**< GTC inner authentication */
    NM_EAP_INNER_AUTH_MAX          /**< Invalid inner auth type */

} NM_WifiEapInnerAuth_e;

/**
 * @brief Different PAE (Port Access Entity) states for 802.1x security.
 */
typedef enum
{
    NM_8021X_PAE_STATE_LOGOFF,          /**< Logoff state */
    NM_8021X_PAE_STATE_DISCONNECTED,    /**< Disconnected state */
    NM_8021X_PAE_STATE_CONNECTING,      /**< Connecting state */
    NM_8021X_PAE_STATE_AUTHENTICATING,  /**< Authenticating state */
    NM_8021X_PAE_STATE_HELD,            /**< Held state */
    NM_8021X_PAE_STATE_AUTHENTICATED,   /**< Authenticated state */
    NM_8021X_PAE_STATE_RESTART,         /**< Restart state */
    NM_8021X_PAE_STATE_UNKNOWN          /**< Unknown state */

} NM_8021xPaeState_e;

/**
 * @brief Different EAP (Extensible Authentication Protocol) states for 802.1x security.
 */
typedef enum
{
    NM_8021X_EAP_STATE_INITIALIZE,     /**< Initialize state */
    NM_8021X_EAP_STATE_DISABLED,       /**< Disabled state */
    NM_8021X_EAP_STATE_IDLE,           /**< Idle state */
    NM_8021X_EAP_STATE_RECEIVED,       /**< Received state */
    NM_8021X_EAP_STATE_GET_METHOD,     /**< Get method state */
    NM_8021X_EAP_STATE_METHOD,         /**< Method state */
    NM_8021X_EAP_STATE_SEND_RESPONSE,  /**< Send response state */
    NM_8021X_EAP_STATE_DISCARD,        /**< Discard state */
    NM_8021X_EAP_STATE_IDENTITY,       /**< Identity state */
    NM_8021X_EAP_STATE_NOTIFICATION,   /**< Notification state */
    NM_8021X_EAP_STATE_RETRANSMIT,     /**< Retransmit state */
    NM_8021X_EAP_STATE_SUCCESS,        /**< Success state */
    NM_8021X_EAP_STATE_FAILURE,        /**< Failure state */
    NM_8021X_EAP_STATE_UNKNOWN         /**< Unknown state */

} NM_8021xEapState_e;

/**
 * @brief Enumeration of events generated by the Network Manager.
 *
 * Refer to @ref NM_EventCallback to get details about data associated with each event.
 *
 * @note
 * - Events with IDs ranging from 0 to 99 do not have an associated interface ID or event data.
 * - Events with IDs ranging from 100 to 199 are reserved for future use.
 * - Events with IDs ranging from 200 to 299 have an associated interface ID but no event data.
 * - Events with IDs ranging from 300 to 399 have both an associated interface ID and event data.
 */
typedef enum
{
    //--------------------------------------------------------------------------
    // Events without interface id & without payload
    //--------------------------------------------------------------------------

    /**
     * @brief Fatal error occurred in the network manager daemon.
     *
     * In most cases the further communication with daemon can not
     * be possible after this event is generated. Application should restart
     * the network manager and reinitialize network manager library for further use.
     */
    NM_EVENT_ID_DAEMON_FATAL_ERR = 0,

    //--------------------------------------------------------------------------
    // Events without interface id & with payload
    //--------------------------------------------------------------------------

    // Values from 100 to 200 are reserved

    //--------------------------------------------------------------------------
    // Events with interface id & without payload
    //--------------------------------------------------------------------------

    /**
     * @brief Interface physical link is up.
     */
    NM_EVENT_ID_PHY_LINK_UP = 200,

    /**
     * @brief Interface physical link is down.
     */
    NM_EVENT_ID_PHY_LINK_DOWN,

    /**
     * @brief Wifi disconnected with access point.
     */
    NM_EVENT_ID_WIFI_DISCONNECTED,

    /**
     * @brief IPv4 static configuration succeeded.
     *
     * This event is triggered when an IPv4 address is configured using the static method,
     * and a conflict check is requested. If the IPv4 address is found to be non conflicted
     * on the network, this event is generated.
     */
    NM_EVENT_ID_IPV4_STATIC_SUCCESS,

    /**
     * @brief IPv4 static configuration failed.
     *
     * This event is triggered when an IPv4 address is configured using the static method,
     * and a conflict check is requested. If the IPv4 address is found to be non conflicted
     * on the network but the network manager fails to set the IP address on the interface,
     * this event is generated.
     */
    NM_EVENT_ID_IPV4_STATIC_FAIL,

    /**
     * @brief IPv4 DHCP deconfig event.
     *
     * This event is triggered when the IP discovery process starts and
     * when a IP address lease is lost.
     */
    NM_EVENT_ID_IPV4_DHCP_DECONFIG,

    /**
     * @brief IPv4 DHCP lease failed.
     *
     * This event is triggered when IP address lease is lost.
     */
    NM_EVENT_ID_IPV4_DHCP_LEASE_FAIL,

    /**
     * @brief IPv4 PPP link is down.
     *
     * This event is triggered by pppd daemon when the PPP link is no longer
     * available for sending and receiving IP packets.
     */
    NM_EVENT_ID_IPV4_PPP_IP_DOWN,

    /**
     * @brief IPv6 static configuration succeeded.
     *
     * This event is triggered when the manually assigned IP address is set
     * on the interface successfully and IP address is not marked as conflicted
     * address on the network.
     */
    NM_EVENT_ID_IPV6_STATIC_SUCCESS,

    /**
     * @brief IPv6 Duplicate address detected.
     *
     * This event is triggered when the manually assigned IP address is set
     * on the interface successfully but it is marked as conflicted
     * address on the network.
     *
     * @note Here the dad completed flag is set by the kernel for the
     * conflicted IPv6 address on the interface.
     */
    NM_EVENT_ID_IPV6_STATIC_DAD,

    /**
     * @brief The DHCPv6 client has been started and IP discovery has been initiated.
     */
    NM_EVENT_ID_IPV6_DHCP_STARTED,

    /**
     * @brief The DHCPv6 client lost all DHCPv6 servers and will restart.
     */
    NM_EVENT_ID_IPV6_DHCP_UNBOUND,

    /**
     * @brief IPv6 DHCP configuration succeeded.
     *
     * This event is triggered when the DHCPv6 server assigned IP address is set
     * on the interface successfully and IP address is not marked as conflicted
     * address on the network.
     */
    NM_EVENT_ID_IPV6_DHCP_SUCCESS,

    /**
     * @brief IPv6 DHCP Duplicate Address Detection (DAD) completed.
     *
     * This event is triggered when the DHCPv6 server assigned IP address is set
     * on the interface successfully but it is marked as conflicted
     * address on the network.
     *
     * @note Here the dad completed flag is set by the kernel for the
     * conflicted IPv6 address on the interface.
     */
    NM_EVENT_ID_IPV6_DHCP_DAD,

    /**
     * @brief IPv6 SLAAC (Stateless Address Autoconfiguration) client is started
     * and IP discovery has been initiated.
     */
    NM_EVENT_ID_IPV6_SLAAC_STARTED,

    /**
     * @brief IPv6 SLAAC configuration succeeded.
     *
     * This event is triggered when the SLAAC server assigned IP address is set
     * on the interface successfully and IP address is not marked as conflicted
     * address on the network.
     */
    NM_EVENT_ID_IPV6_SLAAC_SUCCESS,

    /**
     * @brief IPv6 SLAAC Duplicate Address Detected
     *
     * This event is triggered when the SLAAC server assigned IP address is set
     * on the interface successfully but it is marked as conflicted
     * address on the network.
     *
     * @note Here the dad completed flag is set by the kernel for the
     * conflicted IPv6 address on the interface.
     */
    NM_EVENT_ID_IPV6_SLAAC_DAD,

    //--------------------------------------------------------------------------
    // Events with interface id & with payload
    //--------------------------------------------------------------------------

    /**
     * @brief Wired 802.1x authentication status.
     *
     * This event will be triggered periodically according to the
     * status poll interval specified by an application.
     */
    NM_EVENT_ID_WIRED_8021X_AUTH_STATUS = 300,

    /**
     * @brief Wifi scan available networks result received.
     */
    NM_EVENT_ID_WIFI_NETWORK_SEARCH_INFO,

    /**
     * @brief Wifi connected with access point.
     *
     * This event will be triggered when the WiFi client is connected to
     * an access point. It includes all connection methods, such as manual,
     * WPS push button, WPS device PIN, and WPS router PIN.
     */
    NM_EVENT_ID_WIFI_CONNECTED,

    /**
     * @brief Wifi access point signal quality information is received.
     */
    NM_EVENT_ID_WIFI_SIGNAL_QUALITY_INFO,

    /**
     * @brief IPv4 static address conflicted on network.
     *
     * This event is triggered when an IPv4 address is set using the static method
     * and conflict check is requested.
     * If the IPv4 address is conflicted on the network, this event is generated.
     */
    NM_EVENT_ID_IPV4_STATIC_CONFLICT,

    /**
     * @brief IPv4 DHCP bound.
     *
     * This event is triggered when the DHCP client state moves from
     * unbound to a bound state. This event denotes that IP address configuration
     * has been received from the DHCP server.
     */
    NM_EVENT_ID_IPV4_DHCP_BOUND,

    /**
     * @brief IPv4 DHCP renewed.
     *
     * This event is triggered when the lease of an IP address is renewed by
     * DHCP client. Usually with this event the IP address will not change but
     * subnet mask, default gateway, dns servers etc. may change.
     */
    NM_EVENT_ID_IPV4_DHCP_RENEW,

    /**
     * @brief IPv4 PPP link is up.
     *
     * This event is triggered when the PPP link is ready and available
     * for sending & receiving IP packets.
     */
    NM_EVENT_ID_IPV4_PPP_IP_UP,

    /**
     * @brief DHCPv6 server was found and addresses or prefixes acquired.
     */
    NM_EVENT_ID_IPV6_DHCP_BOUND,

    /**
     * @brief Updated IPv6  information was received from the DHCPv6 server.
     */
    NM_EVENT_ID_IPV6_DHCP_UPDATED,

    /**
     * @brief The DHCPv6 client switched to another server.
     */
    NM_EVENT_ID_IPV6_DHCP_REBOUND,

    /**
     * @brief IPv6 information received via router advertisement.
     */
    NM_EVENT_ID_IPV6_SLAAC_RA_RECEIVED,

    /**
     * @brief Updated IPv6 information received via router advertisement.
     */
    NM_EVENT_ID_IPV6_SLAAC_RA_UPDATED,

    /**
     * @brief An invalid event ID.
     */
    NM_EVENT_ID_MAX = 400

} NM_EventId_e;

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/
/**
 * @brief Unique handle to be used by an application to manage interface.
 *
 * @warning Avoid using this value for deriving any business logic or as an array index.
 * Utilize it solely for mapping purposes within your application.
 */

typedef mxU8_t NM_InterfaceId;

/**
 * @brief Callback function type for handling network manager events.
 *
 * Refer to @ref NM_EventId_e to determine when the event will be received
 *
 * @param event Unique ID associated with the event.
 * @param ifaceId Valid interface ID if the event is for a specific interface.
 * @param eventData Pointer to the data structure containing the event data.
 *
 * @warning
 * - The event callback will be invoked within the context of the network manager library **THREAD**
 *   which polls for events from the network manager daemon.
 * - The application must typecast the event data structure to the specific structure associated with the event.
 * - Event data will not remain valid after the callback function returns.
 *
 * **Below is a summary of the data expected for each event:**
 *
 * | <b>Event ID</b>                      | <b>Interface ID</b> | <b>Event Data Structure</b>           |
 * | :----------------------------------- | :------------------ | :------------------------------------ |
 * | NM_EVENT_ID_DAEMON_FATAL_ERR         | Invalid             | Invalid                               |
 * | NM_EVENT_ID_PHY_LINK_UP              | Valid               | NULL                                  |
 * | NM_EVENT_ID_PHY_LINK_DOWN            | Valid               | NULL                                  |
 * | NM_EVENT_ID_WIFI_DISCONNECTED        | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV4_STATIC_SUCCESS      | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV4_STATIC_FAIL         | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV4_DHCP_DECONFIG       | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV4_DHCP_LEASE_FAIL     | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV4_PPP_IP_DOWN         | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_STATIC_SUCCESS      | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_STATIC_DAD          | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_DHCP_STARTED        | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_DHCP_UNBOUND        | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_DHCP_SUCCESS        | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_DHCP_DAD            | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_SLAAC_STARTED       | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_SLAAC_SUCCESS       | Valid               | NULL                                  |
 * | NM_EVENT_ID_IPV6_SLAAC_DAD           | Valid               | NULL                                  |
 * | NM_EVENT_ID_WIRED_8021X_AUTH_STATUS  | Valid               | @ref NMEvent_Wired8021xAuthStatus_t   |
 * | NM_EVENT_ID_WIFI_NETWORK_SEARCH_INFO | Valid               | @ref NMEvent_WifiNetworksSearchInfo_t |
 * | NM_EVENT_ID_WIFI_CONNECTED           | Valid               | @ref NMEvent_WifiAccessPointInfo_t    |
 * | NM_EVENT_ID_WIFI_SIGNAL_QUALITY_INFO | Valid               | @ref NMEvent_WifiSignalQualityInfo_t  |
 * | NM_EVENT_ID_IPV4_STATIC_CONFLICT     | Valid               | @ref NMEvent_Ipv4AddrConflict_t       |
 * | NM_EVENT_ID_IPV4_DHCP_BOUND          | Valid               | @ref NMEvent_Ipv4DhcpAddrInfo_t       |
 * | NM_EVENT_ID_IPV4_DHCP_RENEW          | Valid               | @ref NMEvent_Ipv4DhcpAddrInfo_t       |
 * | NM_EVENT_ID_IPV4_PPP_IP_UP           | Valid               | @ref NMEvent_Ipv4PppAddrInfo_t        |
 * | NM_EVENT_ID_IPV6_DHCP_BOUND          | Valid               | @ref NMEvent_Ipv6DhcpAddrInfo_t       |
 * | NM_EVENT_ID_IPV6_DHCP_UPDATED        | Valid               | @ref NMEvent_Ipv6DhcpAddrInfo_t       |
 * | NM_EVENT_ID_IPV6_DHCP_REBOUND        | Valid               | @ref NMEvent_Ipv6DhcpAddrInfo_t       |
 * | NM_EVENT_ID_IPV6_SLAAC_RA_RECEIVED   | Valid               | @ref NMEvent_Ipv6SlaacAddrInfo_t      |
 * | NM_EVENT_ID_IPV6_SLAAC_RA_UPDATED    | Valid               | @ref NMEvent_Ipv6SlaacAddrInfo_t      |
 *
 */
typedef void (*NM_EventCallback)(NM_EventId_e event, NM_InterfaceId ifaceId, const void *eventData);

/**
 * @brief Callback function type for receiving all the logs of network manager library
 * as well as daemon.
 *
 * @param level Log severity level
 * @param msg Log message character string with '\0' terminated
 * @param len Length of log message
 *
 * @note As per the design, the network manager daemon's logs are transmitted to the
 * network manager library via Unix socket IPC. Subsequently, the library relays these logs to
 * an application using the provided callback mechanism.
 */
typedef void (*NM_LogCallback)(NM_LogLevel_e level, const mxChar *msg, mxI32_t len);

/*****************************************************************************
 * STRUCTURES
 *****************************************************************************/

/**
 * @brief Network manager logs configuration information.
 *
 * logCallback is valid if sink type is NM_LOG_SINK_CALLBACK.
 *
 * syslogIp, syslogPort are valid if sink type is NM_LOG_SINK_SYSLOG.
 */
typedef struct
{
    NM_LogLevel_e level;                /**< Log level */
    NM_LogSink_e sink;                  /**< Log sink */
    NM_LogCallback logCallback;         /**< Log callback */
    mxChar syslogIp[INET6_ADDRSTRLEN];  /**< Syslog IP address */
    mxU16_t syslogPort;                 /**< Syslog port number */

} NM_LogConfig_t;

/**
 * @brief Interface basic details.
 */
typedef struct
{
    NM_DeviceType_e deviceType;    /**< Device type */
    mxChar ifaceName[IF_NAMESIZE]; /**< Interface name */

} NMIface_InterfaceConfig_t;

/**
 *  @brief MAC address configuration.
 */
typedef struct
{
    mxChar macAddr[NM_MAC_ADDR_STR_LEN_MAX]; /**< MAC address */

} NMIface_MacAddrInfo_t;

/**
 * @brief VLAN interface configuration.
 */
typedef struct
{
    mxChar vlanIfaceName[IF_NAMESIZE]; /**< VLAN interface name */
    mxU16_t vlanId;                    /**< VLAN id */

} NMIface_VlanConfig_t;

/**
 * @brief Ethernet 802.1x authentication configuration.
 */
typedef struct
{
    NM_Wired8021xAuthType_e authType;                   /**< Authentication type */
    mxChar username[NM_WIRED_8021X_USERNAME_LEN_MAX];   /**< Username */
    mxChar password[NM_WIRED_8021X_PASSWORD_LEN_MAX];   /**< Password */
    mxU16_t statusPollIntervalSeconds;                  /**< Interval to periodically poll the authentication status */

} NMIface_Wired8021xAuthConfig_t;

/**
 * @brief Wifi access point settings configuration.
 */
typedef struct
{
    mxChar ssid[NM_WIFI_SSID_LEN_MAX];                    /**< SSID of the Wifi network */
    mxChar username[NM_WIFI_USERNAME_LEN_MAX];            /**< Username for authentication (if applicable) */
    mxChar password[NM_WIFI_PASSWORD_LEN_MAX];            /**< Password/Security key for authentication (if applicable) */
    mxChar anonymousIdentity[NM_WIFI_USERNAME_LEN_MAX];   /**< Anonymous identity for EAP authentication (if applicable) */
    NM_WifiSecurityType_e securityType;                   /**< Security type of the Wifi network */
    NM_WpaKeyEncryptionType_e keyEncryptionType;          /**< Security key encryption type */
    NM_WifiEapAuthType_e eapAuthType;                     /**< Extensible Authentication Protocol (EAP) authentication type */
    NM_WifiEapInnerAuth_e innerAuthType;                  /**< Inner authentication type for EAP */
    NM_WifiEapPeapVersion_e peapVersion;                  /**< Protected Extensible Authentication Protocol (PEAP) version */

} NMIface_WifiAccessPointConfig_t;

/**
 * @brief WiFi available network search configuration.
 */
typedef struct
{
    mxU8_t scanResultWaitSeconds;   /**< Time in seconds to wait for WiFi scan result */

} NMIface_WifiSearchConfig_t;

/**
 * @brief WiFi network signal quality configuration.
 */
typedef struct
{
    mxChar ssid[NM_WIFI_SSID_LEN_MAX];      /**< SSID of the Wifi network */
    mxU8_t scanResultWaitSeconds;           /**< Time in seconds to wait for the scan result */

} NMIface_WifiSignalQualityInfo_t;

/**
 * @brief WiFi WPS AP setup using Router's PIN configuration.
 * @note This PIN is generated by the router.
 */
typedef struct
{
    mxChar bssid[NM_MAC_ADDR_STR_LEN_MAX];       /**< BSSID of the access point */
    mxChar pin[NM_WIFI_WPS_DEVICE_PIN_LEN_MAX];  /**< PIN generated by access point for WPS setup */

} NMIface_WifiWpsAccessPointPinConfig_t;

/**
 * @brief IPv4 address configuration.
 */
typedef struct
{
    mxChar ip[INET_ADDRSTRLEN];     /**< IPv4 address string */
    mxChar subnet[INET_ADDRSTRLEN]; /**< IPv4 subnet mask string */

} NMIpv4_AddrInfo_t;

/**
 * @brief IPv4 gateway configuration.
 */
typedef struct
{
    mxChar gateway[INET_ADDRSTRLEN];    /**< IPv4 gateway address string */

} NMIpv4_GatewayInfo_t;

/**
 * @brief IPv4 static method configuration.
 */
typedef struct
{
    NMIpv4_AddrInfo_t addrInfo;
    mxBool conflictCheck;       /**< Flag to check for conflict prior to apply ip address  */

} NMIpv4_StaticConfig_t;

/**
 * @brief IPv4 PPPoE method configuration
 */
typedef struct
{
    mxChar username[NM_PPP_USERNAME_LEN_MAX];   /**< PPPoE authentication username */
    mxChar password[NM_PPP_PASSWORD_LEN_MAX];   /**< PPPoE authentication password */

} NMIpv4_PppoeConfig_t;

/**
 * @brief IPv4 static route entry configuration
 */
typedef struct
{
    mxChar networkAddr[INET_ADDRSTRLEN]; /**< Network address */
    mxChar subnet[INET_ADDRSTRLEN];      /**< Subnet mask */
    mxChar gateway[INET_ADDRSTRLEN];     /**< Destination gateway address */

} NMIpv4_StaticRouteConfig_t;

/**
 * @brief USB to serial modem configuration
 */
typedef struct
{
    mxChar dialNumber[NM_USB_MODEM_DIAL_NUM_LEN_MAX]; /**< Dial number */
    mxChar apn[NM_USB_MODEM_APN_LEN_MAX];             /**< Access Point Name */
    mxChar username[NM_PPP_USERNAME_LEN_MAX];         /**< Username */
    mxChar password[NM_PPP_PASSWORD_LEN_MAX];         /**< Password */

} NMIpv4_UsbToSerialConfig_t;

/**
 * @brief IPv6 address configuration
 */
typedef struct
{
    mxChar ip[INET6_ADDRSTRLEN]; /**< IPv6 address */
    mxU32_t preferredTime;       /**< IPv6 address preferred time */
    mxU32_t validTime;           /**< IPv6 address valid time */
    mxU8_t prefixLen;            /**< Prefix length */

} NMIpv6_AddrInfo_t;

/**
 * @brief IPv6 gateway configuration
 */
typedef struct
{
    mxChar gateway[INET6_ADDRSTRLEN];   /**< IPv6 gateway address */
    mxU32_t metric;                     /**< Gateway metric */

} NMIpv6_GatewayInfo_t;

/**
 * @brief IPv6 static route entry configuration.
 */
typedef struct
{
    mxChar networkAddr[INET6_ADDRSTRLEN]; /**< Network address */
    mxU8_t prefixLen;                     /**< Prefix length */
    mxChar gateway[INET6_ADDRSTRLEN];     /**< Destination gateway address */

} NMIpv6_StaticRouteConfig_t;

/**
 * @brief System hostname configuration.
 */
typedef struct
{
    mxChar hostname[HOST_NAME_MAX]; /**< Hostname string */

} NM_HostnameInfo_t;

/**
 * @brief Hosts entries configuration for /etc/hosts file.
 */
typedef struct
{
    struct
    {
        mxChar ipAddr[INET6_ADDRSTRLEN];    /**< IPv4/IPv6 address */
        mxChar domain[NM_FQDN_STR_LEN_MAX]; /**< Hostname or domain name */

    } hosts[NM_HOSTS_ENTRY_MAX]; /**< An array of host mappings */

} NM_HostsEntryInfo_t;

/**
 * @brief DNS nameserver address configuration.
 */
typedef struct
{
    mxChar domain[NM_FQDN_STR_LEN_MAX];                                 /**< Domain name */
    mxChar nameServers[NM_DNS_NAMESERVER_ENTRY_MAX][INET6_ADDRSTRLEN];  /**< An array of nameserver addresses */

} NM_DnsServerInfo_t;

/**
 * @brief Wifi WPS device PIN information.
 * @note This PIN is generated by the wifi client device.
 */
typedef struct
{
    mxChar devicePin[NM_WIFI_WPS_DEVICE_PIN_LEN_MAX]; /**< Wifi device generated PIN for WPS setup */

} NMResp_WifiWpsDevicePinInfo_t;

/**
 *  @brief IPv4 address conflict information.
 */
typedef struct
{
    mxChar conflictMacAddr[NM_MAC_ADDR_STR_LEN_MAX];    /**< MAC address of system with ipv4 conflict */

} NMEvent_Ipv4AddrConflict_t;

/**
 *  @brief DHCP IPv4 address assignment information.
 */
typedef struct
{
    NMIpv4_AddrInfo_t ipAddrInfo;               /**< IPv4 address information */
    NMIpv4_GatewayInfo_t gatewayAddrInfo;       /**< Gateway address information */
    mxChar dns1[INET_ADDRSTRLEN];               /**< Primary DNS server */
    mxChar dns2[INET_ADDRSTRLEN];               /**< Secondary DNS server */
    mxChar domain[NM_FQDN_STR_LEN_MAX];         /**< Domain name */
    mxChar httpServer1[NM_FQDN_STR_LEN_MAX];    /**< 1st HTTP server IPv4 address/domain */
    mxChar httpServer2[NM_FQDN_STR_LEN_MAX];    /**< 2nd HTTP server IPv4 address/domain */
    mxChar tftpServer[NM_FQDN_STR_LEN_MAX];     /**< TFTP server IPv4 address/domain */

} NMEvent_Ipv4DhcpAddrInfo_t;

/**
 *  @brief PPP IPv4 address assignment information
 */
typedef struct
{
    mxChar pppIfaceName[IF_NAMESIZE];         /**< PPP interface name */
    NMIpv4_AddrInfo_t ipAddrInfo;             /**< IPv4 address information */
    NMIpv4_GatewayInfo_t gatewayAddrInfo;     /**< Gateway address information */
    mxChar dns1[INET_ADDRSTRLEN];             /**< Primary DNS server */
    mxChar dns2[INET_ADDRSTRLEN];             /**< Secondary DNS server */

} NMEvent_Ipv4PppAddrInfo_t;

/**
 *  @brief DHCP IPv6 address assignment information
 */
typedef struct
{
    NMIpv6_AddrInfo_t ipAddrInfo;               /**< IPv6 address information */
    NMIpv6_GatewayInfo_t gatewayAddrInfo;       /**< Gateway address information */
    mxChar dns1[INET6_ADDRSTRLEN];              /**< Primary DNS server */
    mxChar dns2[INET6_ADDRSTRLEN];              /**< Secondary DNS server */
    mxChar domain[NM_FQDN_STR_LEN_MAX];         /**< Domain */
    mxChar httpServer1[NM_FQDN_STR_LEN_MAX];    /**< 1st HTTP server IPv6 address/domain */
    mxChar httpServer2[NM_FQDN_STR_LEN_MAX];    /**< 2nd HTTP server IPv6 address/domain */
    mxChar tftpServer[NM_FQDN_STR_LEN_MAX];     /**< TFTP server IPv6 address/domain */

} NMEvent_Ipv6DhcpAddrInfo_t;

/**
 *  @brief SLAAC IPv6 address assignment information
 */
typedef struct
{
    NMIpv6_AddrInfo_t ipAddrInfo;           /**< IPv6 address information */
    NMIpv6_GatewayInfo_t gatewayAddrInfo;   /**< Gateway address information */
    mxChar dns1[INET6_ADDRSTRLEN];          /**< Primary DNS server */
    mxChar dns2[INET6_ADDRSTRLEN];          /**< Secondary DNS server */

} NMEvent_Ipv6SlaacAddrInfo_t;

/**
 *  @brief Wired 802.1x authentication status information
 */
typedef struct
{
    NM_8021xPaeState_e paeState; /**< PAE (Port Access Entity) state */
    NM_8021xEapState_e eapState; /**< EAP (Extensible Authentication Protocol) state */

} NMEvent_Wired8021xAuthStatus_t;

/**
 * @brief A single entry of WiFi network search information.
 */
typedef struct
{
    mxChar ssid[NM_WIFI_SSID_LEN_MAX];              /**< SSID of the WiFi network */
    mxChar bssid[NM_MAC_ADDR_STR_LEN_MAX];          /**< BSSID of the WiFi network */
    mxBool secured;                                 /**< Flag indicating whether the network is secured */
    mxBool wpsEnabled;                              /**< Flag indicating whether WPS is enabled */
    mxI32_t signalQuality;                          /**< Signal quality of the network */
    NM_WifiSecurityType_e securityType;             /**< Security type of the network */
    NM_WpaKeyEncryptionType_e keyEncryptionType;    /**< Security key encryption type of the network */

} NMEvent_WifiNetworkSearchEntry_t;

/**
 * @brief WiFi networks search status information.
 */
typedef struct
{
    mxU8_t availableNetworks;                                                       /**< Number of available Wifi networks */
    NMEvent_WifiNetworkSearchEntry_t networkInfo[NM_WIFI_NETWORK_SEARCH_ENTRY_MAX]; /**< Array of Wifi network information */

} NMEvent_WifiNetworksSearchInfo_t;

/**
 * @brief Connected WiFi access point information.
 */
typedef struct
{
    mxChar ssid[NM_WIFI_SSID_LEN_MAX];              /**< SSID of the connected access point */
    mxChar password[NM_WIFI_PASSWORD_LEN_MAX];      /**< Password used for connection */
    NM_WifiSecurityType_e securityType;             /**< Security type of the access point */
    NM_WpaKeyEncryptionType_e keyEncryptionType;    /**< Security key encryption type */

} NMEvent_WifiAccessPointInfo_t;

/**
 * @brief WiFi SSID signal quality information.
 */
typedef struct
{
    mxChar ssid[NM_WIFI_SSID_LEN_MAX];  /**< SSID of the WiFi network */
    mxI32_t signalQuality;              /**< Signal quality in dBm */

} NMEvent_WifiSignalQualityInfo_t;


/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/

//--------------------------------------------------------------------------
// Library Generic Functions
//--------------------------------------------------------------------------

/**
 * @brief Initialize network manager interface library.
 *
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note
 * - This API must be called first prior to any other API.
 * - Network manager daemon must be running prior to calling this API.
 * @warning
 * If you start the network manager daemon from the application and immediately
 * call the @ref NMLib_Init API, it may fail. This is because the network manager
 * daemon requires some time to initialize itself. Initialization tasks include
 * terminating any running dependent utilities, creating directories,
 * generating default configurations, and setting up IPC queues etc.
 */
NMSts_e NMLib_Init(void);

/**
 * @brief Deinitialize network manager interface library.
 *
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMLib_Exit(void);

/**
 * @brief Sets an application callback to handle network manager events.
 *
 * @param eventCallback Callback function address
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note Refer @ref NM_EventId_e and @ref NM_EventCallback for more information.
 */
NMSts_e NMLib_SetEventCallback(NM_EventCallback eventCallback);

/**
 * @brief Sets an application callback to get network manager logs.
 *
 * @param logConfig Logs configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMLib_ConfigureLogs(const NM_LogConfig_t *logConfig);

//--------------------------------------------------------------------------
// Interface Functions
//--------------------------------------------------------------------------

/**
 * @brief Registers a new network interface with the network manager for management.
 *
 * @param ifaceConfig Interface configuration
 * @param ifaceId Unique interface id assigned to the interface by the network manager.
 * Application must use this interface id to manage the network interface further.
 * @warning Do not use custom data types to store and provide the interface ID.
 * Always use the `@ref NM_InterfaceId` data type for future compatibility.
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @see NM_INTERFACE_SUPPORT_MAX
 */
NMSts_e NMIface_ManageInterface(const NMIface_InterfaceConfig_t *ifaceConfig, NM_InterfaceId *ifaceId);

/**
 * @brief Deregisters a network interface with network manager to release it.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_ReleaseInterface(NM_InterfaceId ifaceId);

/**
 * @brief Makes interface state to UP.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_InterfaceUp(NM_InterfaceId ifaceId);

/**
 * @brief Makes interface state to DOWN.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_InterfaceDown(NM_InterfaceId ifaceId);

/**
 * @brief Creates a VLAN interface on the given parent physical interface.
 *
 * @param parentIfaceId Interface id of parent interface
 * @param vlanInfo VLAN interface configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note
 * - Currently VLAN is supported for Ethernet type of device only.
 * - To use VLAN, parent interface must be registered first with the network manager.
 */
NMSts_e NMIface_CreateVlan(NM_InterfaceId parentIfaceId, const NMIface_VlanConfig_t *vlanInfo);

/**
 * @brief Deletes a VLAN interface.
 *
 * @param parentIfaceId Interface id of parent interface
 * @param vlanInfo VLAN interface configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note Provide same VLAN information same as provided while creating VLAN interface.
 */
NMSts_e NMIface_DeleteVlan(NM_InterfaceId parentIfaceId, const NMIface_VlanConfig_t *vlanInfo);

/**
 * @brief Get current MAC address of the interface.
 *
 * @param ifaceId Interface id
 * @param macAddrInfo MAC address information
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_GetMacAddr(NM_InterfaceId ifaceId, NMIface_MacAddrInfo_t *macAddrInfo);

/**
 * @brief Change MAC address on the interface.
 *
 * @param ifaceId Interface id
 * @param macAddrConfig MAC address configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note This API can be used to achieve MAC cloning feature.
 * @todo What will happen with IPv6 link local address on MAC address change.
 */
NMSts_e NMIface_ChangeMacAddr(NM_InterfaceId ifaceId, const NMIface_MacAddrInfo_t *macAddrConfig);

/**
 * @brief Enables wired 802.1x authentication for port based network access.
 *
 * @param ifaceId Interface id
 * @param authConfig 802.1x authentication configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_EnableWired8021xAuth(NM_InterfaceId ifaceId, const NMIface_Wired8021xAuthConfig_t *authConfig);

/**
 * @brief Disables wired 802.1x authentication for port based network access.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_DisableWired8021xAuth(NM_InterfaceId ifaceId);

/**
 * @brief Reconnects wired 802.1x authentication for port based network access.
 *
 * This function should be called when 802.1x authentication is enabled, and the interface
 * transitions from the down state to the up state. It re-initiates the 802.1x authentication process.
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_ReconnectWired8021xAuth(NM_InterfaceId ifaceId);

/**
 * @brief Disconnects wired 802.1x authentication for port based network access.
 *
 * This function should be called when 802.1x authentication is enabled and the interface goes down.
 * It halts the 802.1x authentication process temporarily.
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_DisconnectWired8021xAuth(NM_InterfaceId ifaceId);

/**
 * @brief Searches for available WiFi networks.
 * @param ifaceId Interface id
 * @param searchConfig Configuration for WiFi network search
 * @return NMSTS_IN_PROGRESS if process to search wifi network is started successfully, @ref NMSts_e otherwise.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_WIFI_NETWORK_SEARCH_INFO
 */
NMSts_e NMIface_SearchWifiNetworks(NM_InterfaceId ifaceId, const NMIface_WifiSearchConfig_t *searchConfig);

/**
 * @brief Connects to wifi access point.
 * @param ifaceId Interface id
 * @param accessPointConfig Wifi access point settings configuration
 * @return NMSTS_IN_PROGRESS if connection to wifi access point started successfully, @ref NMSts_e otherwise.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_WIFI_CONNECTED
 * - @ref NM_EVENT_ID_WIFI_DISCONNECTED
 */
NMSts_e NMIface_ConnectToWifiAccessPoint(NM_InterfaceId ifaceId, const NMIface_WifiAccessPointConfig_t *accessPointConfig);

/**
 * @brief Disconnects from wifi access point.
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIface_DisconnectFromWifiAccessPoint(NM_InterfaceId ifaceId);

/**
 * @brief Gets wifi signal quality information.
 * @param ifaceId Interface id
 * @param signalQualityInfo WiFi network signal quality configuration
 * @return NMSTS_IN_PROGRESS if scan for ssid is started successfully, @ref NMSts_e otherwise.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_WIFI_SIGNAL_QUALITY_INFO
 */
NMSts_e NMIface_GetWifiSignalQuality(NM_InterfaceId ifaceId, const NMIface_WifiSignalQualityInfo_t *signalQualityInfo);

/**
 * @brief Initiates wifi access point configuration through wps push button config.
 * @param ifaceId Interface id
 * @return NMSTS_IN_PROGRESS if process initiated successfully, @ref NMSts_e otherwise.
 * @note Any already connected wifi AP or pending WPS method will be disconnected first
 * when wps push button is initiated.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_WIFI_CONNECTED
 * - @ref NM_EVENT_ID_WIFI_DISCONNECTED
 */
NMSts_e NMIface_SetupWifiWpsPushButton(NM_InterfaceId ifaceId);

/**
 * @brief Initiates wifi access point configuration through wps device pin.
 * @param ifaceId Interface id
 * @param wpsDevicePinInfoResp Generated device PIN information for Wifi WPS device PIN setup
 * @return NMSTS_IN_PROGRESS if process initiated successfully, @ref NMSts_e otherwise.
 * @note
 * - In this method of WPS connection, the generated 8-digit PIN from the client device
 * must be entered into the routers configuration page.
 * - Any already connected wifi AP or pending WPS method will be disconnected first
 * when wps push button is initiated.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_WIFI_CONNECTED
 * - @ref NM_EVENT_ID_WIFI_DISCONNECTED
 */
NMSts_e NMIface_SetupWifiWpsDevicePin(NM_InterfaceId ifaceId, NMResp_WifiWpsDevicePinInfo_t *wpsDevicePinInfoResp);

/**
 * @brief Initiates wifi access point configuration through wps access point pin configuration.
 * @param ifaceId Interface id
 * @param wpsAccessPointPinConfig Router's PIN and BSSID for WiFi WPS access point PIN setup
 * @return NMSTS_IN_PROGRESS if process initiated successfully, @ref NMSts_e otherwise.
 * @note
 * - In this method of WPS connection, the generated 8-digit PIN from the router (AP)
 * and routers BSSID (MAC address) must be provided to this API.
 * - Any already connected wifi AP or pending WPS method will be disconnected first
 * when wps push button is initiated.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_WIFI_CONNECTED
 * - @ref NM_EVENT_ID_WIFI_DISCONNECTED
 */
NMSts_e NMIface_SetupWifiWpsAccessPointPin(NM_InterfaceId ifaceId, const NMIface_WifiWpsAccessPointPinConfig_t *wpsAccessPointPinConfig);

//--------------------------------------------------------------------------
// IPv4 Address Functions
//--------------------------------------------------------------------------

/**
 * @brief Clears all IPv4 addresses from the interface.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv4_ClearAllAddr(NM_InterfaceId ifaceId);

/**
 * @brief Clears IPv4 assignment method on the interface.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note This API may be required in case application needs to disable IPv4 address assignment after
 * the IP address assigned once on the interface with any of the method like Static, DHCP, PPPoE.
 * In other words it will put network manager interface IPv4 in the state same as when the interface
 * is added for management but no IP assignment method is set yet.
 */
NMSts_e NMIpv4_ClearMethod(NM_InterfaceId ifaceId);

/**
 * @brief Assigns IPv4 address on the interface using static/manual method.
 *
 * @param ifaceId Interface id
 * @param staticConfig IPv4 address configuration
 * @return NMSTS_SUCCESS if conflict flag in FALSE and IP address set successfully
 * @return NMSTS_IN_PROGRESS if conflict flag in TRUE
 * @return @ref NMSts_e otherwise
 * @note
 * - As evident from the return values, this API operates in both synchronous and asynchronous
 * modes based on the conflict check flag input argument.
 * - Any already set IP assignment method will be cleared first before applying this method.
 *
 * If the conflict check flag is set to TRUE, the following events are associated with the
 * response to this API function:
 * - @ref NM_EVENT_ID_IPV4_STATIC_SUCCESS
 * - @ref NM_EVENT_ID_IPV4_STATIC_FAIL
 * - @ref NM_EVENT_ID_IPV4_STATIC_CONFLICT
 */
NMSts_e NMIpv4_SetStaticMethod(NM_InterfaceId ifaceId, const NMIpv4_StaticConfig_t *staticConfig);

/**
 * @brief Assigns IPv4 address on the interface using DHCP method.
 *
 * @param ifaceId Interface id
 * @return NMSTS_IN_PROGRESS if dhcp request is accepted successfully, @ref NMSts_e otherwise.
 * @note Any already set IP assignment method will be cleared first before applying this method.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV4_DHCP_DECONFIG
 * - @ref NM_EVENT_ID_IPV4_DHCP_BOUND
 * - @ref NM_EVENT_ID_IPV4_DHCP_RENEW
 * - @ref NM_EVENT_ID_IPV4_DHCP_LEASE_FAIL
 */
NMSts_e NMIpv4_SetDhcpMethod(NM_InterfaceId ifaceId);

/**
 * @brief Sets DHCP server assigned IPv4 address on the interface.
 *
 * @param ifaceId Interface id
 * @param dhcpAssignedAddr IPv4 address configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv4_SetDhcpAssignedAddr(NM_InterfaceId ifaceId, const NMIpv4_AddrInfo_t *dhcpAssignedAddr);

/**
 * @brief Assigns IPv4 address on the interface using PPPoE method.
 *
 * @param ifaceId Interface id
 * @param pppoeConfig IPv4 pppoe method configuration
 * @return NMSTS_IN_PROGRESS if pppoe request is accepted successfully, @ref NMSts_e otherwise.
 * @note
 * - Any already set IP assignment method will be cleared first before applying this method.
 * - The application will receive the name of the PPP interface (e.g., ppp0, ppp1, etc.)
 * with the data in the event NM_EVENT_ID_IPV4_PPP_IP_UP. The application should not assume
 * the name of the PPP interface as it may vary depending on the interface id.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV4_PPP_IP_UP
 * - @ref NM_EVENT_ID_IPV4_PPP_IP_DOWN
 */
NMSts_e NMIpv4_SetPppoeMethod(NM_InterfaceId ifaceId, const NMIpv4_PppoeConfig_t *pppoeConfig);

/**
 * @brief Sets IPv4 default gateway address.
 *
 * @param ifaceId Interface id
 * @param defaultGatewayConfig IPv4 gateway configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note
 * - This API sets the default route in the route table which is used when the destination
 * IP address in IP communication does not match any of the subnets of the system networks.
 * - It is recommended to clear the existing default gateway before applying a new default gateway.
 */
NMSts_e NMIpv4_SetDefaultGateway(NM_InterfaceId ifaceId, const NMIpv4_GatewayInfo_t *defaultGatewayConfig);

/**
 * @brief Clears IPv4 default gateway address.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv4_ClearDefaultGateway(NM_InterfaceId ifaceId);

/**
 * @brief Adds a static IPv4 route to the route table.
 *
 * @param ifaceId Interface id
 * @param staticRouteConfig IPv4 static route entry configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv4_AddStaticRoute(NM_InterfaceId ifaceId, const NMIpv4_StaticRouteConfig_t *staticRouteConfig);

/**
 * @brief Deletes a static IPv4 route from the route table.
 *
 * @param ifaceId Interface id
 * @param staticRouteConfig IPv4 static route entry configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv4_DeleteStaticRoute(NM_InterfaceId ifaceId, const NMIpv4_StaticRouteConfig_t *staticRouteConfig);

/**
 * @brief Initiates a PPP connection on a USB to serial modem (USB dongle).
 *
 * @param ifaceId Interface id
 * @param usbToSerialConfig USB to serial modem configuration
 * @return NMSTS_IN_PROGRESS if ppp initiation request is accepted successfully, @ref NMSts_e otherwise.
 * @note
 * - This API is useful for USB dongles that communicate over the serial interface (e.g., AT commands).
 * For this type of dongle to work, the application needs to provide the ttyUSB node to the
 * @ref NMIface_ManageInterface API and then call this API to initiate a PPP connection to obtain an
 * IP address on the dongle.
 * - For USB dongles that function as routers, use the @ref NMIpv4_SetDhcpMethod or @ref NMIpv6_SetDhcpMethod
 * APIs to obtain an IP address on the dongle.
 * - Only 4G LTE dongles have been tested for USB to serial types of dongles.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV4_PPP_IP_UP
 * - @ref NM_EVENT_ID_IPV4_PPP_IP_DOWN
 */
NMSts_e NMIpv4_StartUsbModemPppConnection(NM_InterfaceId ifaceId, const NMIpv4_UsbToSerialConfig_t *usbToSerialConfig);

//--------------------------------------------------------------------------
// IPv6 Address Functions
//--------------------------------------------------------------------------

/**
 * @brief Enables the IPv6 stack on the system.
 *
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note
 * - This API enables IPv6 on the system by changing the value of the flag
 * <b>/proc/sys/net/ipv6/conf/all/disable_ipv6</b> to 0.
 * - If this flag is already disabled by default on your system, then
 * you don't need to call this API; otherwise, it is required to use IPv6.
 */
NMSts_e NMIpv6_EnableStack(void);

/**
 * @brief Disables the IPv6 stack on the system.
 *
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv6_DisableStack(void);

/**
 * @brief Enables IPv6 on a specific network interface.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note
 * - When a new network interface is registered with the network manager,
 * IPv6 will be disabled on it by default. To assign an IPv6 address to the interface,
 * you need to enable IPv6 on the interface first.
 * - To enable IPv6 on the interface, IPv6 must be enabled on the system using the API
 * @ref NMIpv6_EnableStack.
 */
NMSts_e NMIpv6_EnableOnInterface(NM_InterfaceId ifaceId);

/**
 * @brief Disables IPv6 on a specific network interface.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note This API can be useful when the system IP assignment mode is changed from Dual stack
 * to IPv4 only mode. Application can disable the IPv6 on the interface without releasing the interface.
 */
NMSts_e NMIpv6_DisableOnInterface(NM_InterfaceId ifaceId);

/**
 * @brief Clears all IPv6 addresses from the interface.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note It will not clear the IPv6 link local ip address.
 */
NMSts_e NMIpv6_ClearAllAddr(NM_InterfaceId ifaceId);

/**
 * @brief Updates the attributes of an IPv6 address assigned to a network interface.
 *
 * @param ifaceId Interface id
 * @param addrAttrConfig IPv6 address configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note This API is useful when application receives the updated preferred and valid lifetime
 * of IPv6 address in SLAAC (RA updated) and DHCPv6 (Renew IP address) methods.
 */
NMSts_e NMIpv6_UpdateAddrAttributes(NM_InterfaceId ifaceId, const NMIpv6_AddrInfo_t *addrAttrConfig);

/**
 * @brief Clears IPv6 assignment method on the interface.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note This API may be required in case application needs to disable IPv6 address assignment after
 * the IP address assigned once on the interface with any of the method like Static, DHCP, PPPoE.
 * In other words it will put network manager interface IPv6 in the state same as when the interface
 * is added for management, IPv6 is enabled on interface but no IP assignment method is set yet.
 */
NMSts_e NMIpv6_ClearMethod(NM_InterfaceId ifaceId);

/**
 * @brief Assigns IPv6 address on the interface using static/manual method.
 *
 * @param ifaceId Interface id
 * @param staticConfig IPv6 address configuration
 * @return NMSTS_IN_PROGRESS if ip address is set successfully and DAD (Duplicate Address Detection) is in progress
 * @return NMSTS_OPERATION_FAIL if failed to set ip address on interface
 * @return @ref NMSts_e otherwise
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV6_STATIC_SUCCESS
 * - @ref NM_EVENT_ID_IPV6_STATIC_DAD
 */
NMSts_e NMIpv6_SetStaticMethod(NM_InterfaceId ifaceId, const NMIpv6_AddrInfo_t *staticConfig);

/**
 * @brief Assigns IPv6 address on the interface using DHCP method.
 *
 * @param ifaceId Interface id
 * @return NMSTS_IN_PROGRESS if dhcp request is accepted successfully, @ref NMSts_e otherwise.
 * @note
 * - <b>For the IPv6 address field</b>,
 * If multiple addresses are available (multiple routers are present on the network),
 * address with a global prefix scope will be prioritized.
 * If no global prefix address is found, first available address regardless of its scope will be selected.
 * - <b>For the gateway address field</b>,
 * If multiple gateway addresses are available, gateway address that matches the prefix
 * of the selected IPv6 address will be prioritized.
 * If no matching gateway address is found, gateway address with the highest priority
 * (indicated by the lowest metric value) will be selected.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV6_DHCP_STARTED
 * - @ref NM_EVENT_ID_IPV6_DHCP_BOUND
 * - @ref NM_EVENT_ID_IPV6_DHCP_UPDATED
 * - @ref NM_EVENT_ID_IPV6_DHCP_REBOUND
 * - @ref NM_EVENT_ID_IPV6_DHCP_UNBOUND
 */
NMSts_e NMIpv6_SetDhcpMethod(NM_InterfaceId ifaceId);

/**
 * @brief Sets DHCP server assigned IPv6 address on the interface.
 *
 * @param ifaceId Interface id
 * @param dhcpAssignedAddr IPv6 address configuration
 * @return NMSTS_IN_PROGRESS if ip address is set successfully and DAD (Duplicate Address Detection) is in progress
 * @return NMSTS_OPERATION_FAIL if failed to set ip address on interface
 * @return @ref NMSts_e otherwise
 * @note Unlike the IPv4 DHCP method, the kernel by default checks for any assigned IPv6 address for DAD
 * (Duplicate Address Detection). So after configuring the IP address on the interface,
 * we can check if the address is found to be a duplicate on the network. Due to this design this API
 * is asynchronous.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV6_DHCP_DAD
 * - @ref NM_EVENT_ID_IPV6_DHCP_SUCCESS
 */
NMSts_e NMIpv6_SetDhcpAssignedAddr(NM_InterfaceId ifaceId, const NMIpv6_AddrInfo_t *dhcpAssignedAddr);

/**
 * @brief Assigns IPv6 address on the interface using SLAAC (Stateless Address Autoconfiguration) method.
 *
 * @param ifaceId Interface id
 * @return NMSTS_IN_PROGRESS if slaac request is accepted successfully, @ref NMSts_e otherwise.
 * @note
 * - With IPv6 SLAAC, the router assigns an IPv6 prefix instead of a complete address.
 * The application needs to use the EUI-64 method with the MAC address to form the full IPv6 address.
 * The open-source tool we use, odhcp6c, handles this task, ensuring that the application
 * receives the fully generated IPv6 address in the case of SLAAC.
 * - <b>For the IPv6 address field</b>,
 * If multiple addresses are available (multiple routes are present on the network),
 * address with a global prefix scope will be prioritized.
 * If no global prefix address is found, first available address regardless of its scope will be selected.
 * - <b>For the gateway address field</b>,
 * If multiple gateway addresses are available, gateway address that matches the prefix
 * of the selected IPv6 address will be prioritized.
 * If no matching gateway address is found, gateway address with the highest priority
 * (indicated by the lowest metric value) will be selected.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV6_SLAAC_STARTED
 * - @ref NM_EVENT_ID_IPV6_SLAAC_RA_RECEIVED
 * - @ref NM_EVENT_ID_IPV6_SLAAC_RA_UPDATED
 */
NMSts_e NMIpv6_SetSlaacMethod(NM_InterfaceId ifaceId);

/**
 * @brief Sets SLAAC generated IPv6 address on the interface.
 *
 * @param ifaceId Interface id
 * @param slaacAssignedAddr IPv6 address configuration
 * @return NMSTS_IN_PROGRESS if ip address is set successfully and DAD (Duplicate Address Detection) is in progress
 * @return NMSTS_OPERATION_FAIL if failed to set ip address on interface
 * @return @ref NMSts_e otherwise
 * @note Here, kernel by default checks for any assigned IPv6 address for DAD
 * (Duplicate Address Detection). So after configuring the IP address on the interface,
 * we can check if the address is found to be a duplicate on the network. Due to this design this API
 * is asynchronous.
 *
 * The following events are associated with the response to this API function:
 * - @ref NM_EVENT_ID_IPV6_SLAAC_DAD
 * - @ref NM_EVENT_ID_IPV6_SLAAC_SUCCESS
 */
NMSts_e NMIpv6_SetSlaacAssignedAddr(NM_InterfaceId ifaceId, const NMIpv6_AddrInfo_t *slaacAssignedAddr);

/**
 * @brief Gets current IPv6 link local address of the interface.
 *
 * @param ifaceId Interface id
 * @param linkLocalAddrInfo IPv6 link local address information
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv6_GetLinkLocalAddr(NM_InterfaceId ifaceId, NMIpv6_AddrInfo_t *linkLocalAddrInfo);

/**
 * @brief Sets IPv6 default gateway address.
 *
 * @param ifaceId Interface id
 * @param defaultGatewayConfig IPv6 gateway configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note It is recommended to clear the existing default gateway before applying a new default gateway.
 */
NMSts_e NMIpv6_SetDefaultGateway(NM_InterfaceId ifaceId, const NMIpv6_GatewayInfo_t *defaultGatewayConfig);

/**
 * @brief Clears IPv6 default gateway address.
 *
 * @param ifaceId Interface id
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv6_ClearDefaultGateway(NM_InterfaceId ifaceId);

/**
 * @brief Adds a static IPv6 route to the route table.
 *
 * @param ifaceId Interface id
 * @param staticRouteConfig IPv6 static route entry configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note In case of IPv6 static route management, interface name is compulsory
 * to make sure kernel knows the physical interface to reach to the destination gateway address.
 */
NMSts_e NMIpv6_AddStaticRoute(NM_InterfaceId ifaceId, const NMIpv6_StaticRouteConfig_t *staticRouteConfig);

/**
 * @brief Deletes a static IPv6 route from the route table.
 *
 * @param ifaceId Interface id
 * @param staticRouteConfig IPv6 static route entry configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMIpv6_DeleteStaticRoute(NM_InterfaceId ifaceId, const NMIpv6_StaticRouteConfig_t *staticRouteConfig);

//--------------------------------------------------------------------------
// DNS Address Functions
//--------------------------------------------------------------------------

/**
 * @brief Updates the hosts file (/etc/hosts) with the provided hosts entry information.
 *
 * @param hostsEntryInfo Hosts entries configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMDns_UpdateHostsFile(const NM_HostsEntryInfo_t* hostsEntryInfo);

/**
 * @brief Sets the remote DNS server addresses.
 *
 * @param dnsInfo DNS nameserver address configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 * @note Network manager uses dnsmasq as a DNS server which stores remote nameservers
 * fo the file @ref NM_DNSMASQ_RESOLV_FILE_PATH instead of /etc/resolv.conf.
 * @see NM_DNS_NAMESERVER_ENTRY_MAX
 */
NMSts_e NMDns_SetRemoteDnsServer(const NM_DnsServerInfo_t *dnsInfo);

//--------------------------------------------------------------------------
// Hostname Functions
//--------------------------------------------------------------------------

/**
 * @brief Sets the hostname of the system.
 *
 * @param hostnameInfo System hostname configuration
 * @return NMSTS_SUCCESS if successful, @ref NMSts_e error codes otherwise
 */
NMSts_e NMHost_SetHostname(const NM_HostnameInfo_t* hostnameInfo);


#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
