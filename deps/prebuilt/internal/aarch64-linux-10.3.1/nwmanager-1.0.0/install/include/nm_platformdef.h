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
 * @file nm_platformdef.h
 * @brief Network Manager platform dependent paths.
 *
 * This file contains macros defining various platform paths used by
 * the network manager components such as configuration files, scripts, and binaries.
 *
 * @note
 * - Currently only @ref NM_INTERFACE_SUPPORT_MAX is the only macro given as
 * configurable by an application.
 * - @ref NM_INTERFACE_SUPPORT_MAX can be overriden based on total required interfaces
 * while compiling network manager library.
 *
 * @warning
 * - Make sure that all the scripts or programs on the platform that are being used by
 * network manager have executable permissions.
 * - All the configuration files defined in this file are managed by the network manager.
 * So don't edit them directly from the application.
 *
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

/*****************************************************************************
 * MACROS
 *****************************************************************************/

//----------------------------------------------------------------------------
// Network manager defines
//----------------------------------------------------------------------------

// Maximum number of interface to be managed

/**
 * @brief Maximum number of interface to be managed
 *
 * @note
 * - Default value of total interfaces is kept as 4.
 * - Define this macro at library compilation time if you want more number of interfaces.
 * - VLAN will be considered as separate interface other than its parent physical interface.
 */
#ifndef NM_INTERFACE_SUPPORT_MAX
#define NM_INTERFACE_SUPPORT_MAX          (4)
#endif

//----------------------------------------------------------------------------
// Network manager program directories
//----------------------------------------------------------------------------

/**
 * @brief Network manager runtime data directory
 *
 * @note
 * - This directory will be created by network manager daemon on start up.
 * - It will be used to store runtime files like status files, pid files, etc.
 */
#define NW_MANAGER_RUNTIME_DIR              "/tmp/networkmanager"

/**
 * @brief Network manager installation directory
 *
 * @note
 * - This directory must be present on the platform when network manager is installed.
 */
#define NW_MANAGER_INSTALL_DIR              "/etc/networkmanager"

/**
 * @brief Network manager binaries installation directory
 *
 * @note
 * - This directory must be present on the platform when network manager is installed.
 * - All the network manager binaries, scripts will be present in this directory.
 */
#define NW_MANAGER_BIN_DIR                  NW_MANAGER_INSTALL_DIR "/bin"

/**
 * @brief Network manager configuration files installation directory
 *
 * @note
 * - This directory will be created by network manager daemon on start up.
 * - All the network manager operations related configuration files will be present in this directory.
 * - e.g. Dnsmasq and wpa_supplicant configuration files will be created here.
 */
#define NW_MANAGER_CONF_DIR                 NW_MANAGER_INSTALL_DIR "/conf"

//----------------------------------------------------------------------------
// Network manager runtime paths
//----------------------------------------------------------------------------

/**
 * @brief Unix socket for communication between network manager daemon and library
 */
#define NM_UNIX_SOCKET_PATH                 NW_MANAGER_RUNTIME_DIR "/nm-unixsocket"

/**
 * @brief Unix socket for wired 802.1x authentication control interface
 */
#define NM_WPA_SUPPLICANT_CONTROL_INTERFACE NW_MANAGER_RUNTIME_DIR "/wpa_supplicant"

/**
 * @brief File to store wired 802.1x authentication process status
 */
#define NM_WPA_SUPPLICANT_STATUS_FILE_PATH  NW_MANAGER_RUNTIME_DIR "/wpa_supplicant-status-%s"

/**
 * @brief File to store output of wpa_passphrase utility which generates
 * WPA PSK from an ASCII passphrase for a wifi SSID.
 */
#define NM_WPA_PASSPHRASE_FILE_PATH         NW_MANAGER_RUNTIME_DIR "/wpa_passphrase-%s"

/**
 * @brief File to store the result of available wifi networks scan done by wpa_supplicant
 */
#define NM_WPA_CLI_SCAN_RESULT_FILE_PATH    NW_MANAGER_RUNTIME_DIR "/wpa_cli-scan_result-%s"

/**
 * @brief File to store the wps device pin generated by wpa_supplicant
 */
#define NM_WPA_CLI_WPS_PIN_FILE_PATH        NW_MANAGER_RUNTIME_DIR "/wpa_cli-wps_pin-%s"

/**
 * @brief File to store the detailed additional information of wifi networks
 */
#define NM_IWLIST_SCAN_RESULT_FILE_PATH    NW_MANAGER_RUNTIME_DIR "/iwlist-scan-%s"

//----------------------------------------------------------------------------
// Network manager bin/script paths
//----------------------------------------------------------------------------

/**
 * @brief An executable or script called by udhcpc (DHCPv4 Client) to notify the status of DHCP operation.
 */
#define NM_DHCPV4_NOTIFY_SCRIPT_PATH        NW_MANAGER_BIN_DIR "/dhcpv4c-notify"

/**
 * @brief An executable or script called by odhcp6c (SLAAC + DHCPv6 Client) to notify the status of DHCP operations.
 */
#define NM_DHCPV6_NOTIFY_SCRIPT_PATH        NW_MANAGER_BIN_DIR "/dhcpv6c-notify"

/**
 * @brief An executable or script called by odhcp6c (SLAAC + DHCPv6 Client) to notify the status of SLAAC operations.
 */
#define NM_SLAAC_NOTIFY_SCRIPT_PATH         NW_MANAGER_BIN_DIR "/slaacv6c-notify"

/**
 * @brief An action script called by wpa_supplicant to notify the updates of  wifi connectivity
 */
#define NM_WIFI_NOTIFY_SCRIPT_PATH          NW_MANAGER_BIN_DIR "/wifi-notify"

/**
 * @brief A program that is called by all the notify scripts like dhcpv4c-notify, dhcpv6c-notify etc.
 * to send the event to the network manager daemon.
 *
 * This program sends the event to the network manager daemon using unix socket.
 */
#define NM_NOTIFY_BIN_PATH                  NW_MANAGER_BIN_DIR "/nmd-notify"

//----------------------------------------------------------------------------
// Network manager conf paths
//----------------------------------------------------------------------------

/**
 * @brief A Directory to store dnsmasq dns server configurations.
 */
#define NW_DNSMASQ_CONF_DIR                 NW_MANAGER_CONF_DIR "/dnsmasq.d"

/**
 * @brief A Dnsmasq dns server configuration file.
 */
#define NM_DNSMASQ_DNS_CONF_FILE_PATH       NW_DNSMASQ_CONF_DIR "/dns-server.conf"

/**
 * @brief A Dnsmasq resolver nameserver file to store the remote dns server address.
 */
#define NM_DNSMASQ_RESOLV_FILE_PATH         NW_DNSMASQ_CONF_DIR "/resolve.dnsmasq"

/**
 * @brief Directory to store wpa_supplicant configurations.
 */
#define NM_WPA_SUPPLICANT_CONF_DIR          NW_MANAGER_CONF_DIR "/wpa_supplicant"

/**
 * @brief A wpa_supplicant supplicant configuration file.
 */
#define NM_WPA_SUPPLICANT_CONF_FILE_PATH    NM_WPA_SUPPLICANT_CONF_DIR "/wpa_supplicant-%s.conf"

//----------------------------------------------------------------------------
// Default standard paths (hostname, local domain)
//----------------------------------------------------------------------------

/**
 * @brief Hosts file for hostname/domain name to IP address mapping.
 *
 * @note Refer @ref NM_HOSTS_ENTRY_MAX macro for the maximum allowed entries.
 */
#define NM_HOSTS_FILE_PATH                  "/etc/hosts"

/**
 * @brief File to update and save system hostname.
 */
#define NM_HOSTNAME_FILE_PATH               "/etc/hostname"

/**
 * @brief Default nameserver file for system dns resolver.
 *
 * @note This file only supports upto 3 remote dns nameservers only.
 *
 * To support more than 3 nameservers, network manager uses dnsmasq dns server
 * which listen for local dns queries and forwards them to remote dns servers.
 * This resolve file will always have the loopback ip address as the dns server
 * where dnsmasq will listen to and process all the queries further.
 */
#define NM_DNS_DEFAULT_RESOLV_FILE_PATH     "/etc/resolv.conf"

//----------------------------------------------------------------------------
// Standard paths (pppd, pppoe)
//----------------------------------------------------------------------------

/**
 * @brief File to provide options to ppp daemon for PPP connection
 */
#define NM_PPP_OPTIONS_FILE_PATH            "/etc/ppp/options.%s"

/**
 * @brief A configuration file that stores PAP secrets for authenticating PPP connections
 */
#define NM_PPP_AUTH_PAP_SECRETS_FILE_PATH   "/etc/ppp/pap-secrets"

/**
 * @brief A configuration file that stores CHAP secrets for authenticating PPP connections
 */
#define NM_PPP_AUTH_CHAP_SECRETS_FILE_PATH  "/etc/ppp/chap-secrets"

// Executed when the link is available for sending and receiving IP packets

/**
 * @brief A program or script which is executed when the link is available for sending and receiving
 * IP packets (that is, IPCP has come up).
 *
 * It is invoked with the parameters: interface-name, tty-device speed, local-IP-address, remote-IP-address, ipparam
 */
#define NM_PPP_IPV4_UP_SCRIPT_PATH          "/etc/ppp/ip-up"

// Executed  when the link is no longer available for sending and receiving IP packets

/**
 * @brief A program or script which is executed when the link is no longer available for
 * sending and receiving IP packets. This script can be used for undoing the effects of the
 * /etc/ppp/ip-up and /etc/ppp/ip-pre-up scripts.
 *
 * It is invoked with the parameters: interface-name, tty-device speed, local-IP-address, remote-IP-address, ipparam
 */
#define NM_PPP_IPV4_DOWN_SCRIPT_PATH        "/etc/ppp/ip-down"

/**
 * @brief Usually there is something which needs to be done to prepare the link before the
 * PPP protocol can be started; for instance, with a dial-up modem, commands need to be sent
 * to the modem to dial the appropriate phone number.
 *
 * This option specifies an command for pppd to execute (by passing it to a shell) before attempting
 * to start PPP negotiation. The chat program is often useful here, as it provides a way to send
 * arbitrary strings to a modem and respond to received characters.
 */
#define NM_PPP_CONNECT_SCRIPT_PATH          "/etc/ppp/%s.connect"

/**
 * @brief Execute the command specified by script, by passing it to a shell, after
 * pppd has terminated the link. This command could, for example, issue commands to the modem
 * to cause it to hang up if hardware modem control signals were not available.
 */
#define NM_PPP_DISCONNECT_SCRIPT_PATH       "/etc/ppp/%s.disconnect"

//----------------------------------------------------------------------------
// Default standard paths (binaries)
//----------------------------------------------------------------------------

/**
 * @brief DNS server daemon program.
 */
#define NM_DNSMASQ_DAEMON_PATH              "dnsmasq"

/**
 * @brief Client for DHCPv4 operations.
 */
#define NM_DHCPV4_CLIENT_PROGRAM_PATH       "udhcpc"

/**
 * @brief Client for SLAAC + DHCPv6 operations.
 */
#define NM_DHCPV6_CLIENT_PROGRAM_PATH       "odhcp6c"

/**
 * @brief Daemon program to establish and maintain PPP link with another system.
 */
#define NM_PPP_DAEMON_PROGRAM_PATH          "pppd"

/**
 * @brief pppoe is a user-space client for PPPoE (Point-to-Point Protocol over Ethernet)
 * for Linux and other UNIX systems. pppoe works in concert with the pppd PPP daemon to
 * provide a PPP connection over Ethernet, as is used by many DSL service providers.
 */
#define NM_PPPOE_CLIENT_PROGRAM_PATH        "pppoe"

/**
 * @brief Wi-Fi Protected Access client and IEEE 802.1X supplicant program.
 */
#define NM_WPA_SUPPLICANT_PROGRAM_PATH      "wpa_supplicant"

/**
 * @brief The client program that provides a high-level interface to the functionality
 * of the wpa supplicant daemon.
 */
#define NM_WPA_CLI_PROGRAM_PATH             "wpa_cli"

/**
 * @brief The client program that generates a WPA PSK from an ASCII passphrase for a SSID.
 */
#define NM_WPA_PASSPHRASE_PROGRAM_PATH      "wpa_passphrase"

/**
 * @brief Get more detailed wireless information from a wireless interface.
 */
#define NM_IWLIST_PROGRAM_PATH              "iwlist"

//----------------------------------------------------------------------------
// Other defines
//----------------------------------------------------------------------------

/**
 * @brief Message to add in all the configuration files generated by network manager.
 */
#define NM_AUTO_GENERATED_FILE_NOTE "\n#\n# Auto generated by Matrix Network Manager. Do not edit!\n#\n"


#ifdef __cplusplus
}
#endif
/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
