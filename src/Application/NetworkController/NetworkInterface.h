#ifndef _NETWORK_INTERFACE_
#define _NETWORK_INTERFACE_
//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		NetworkInterface.h
@brief      This file provides the interface with network manager library module
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "Config.h"

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
void InitNetworkInterface(void);
//-------------------------------------------------------------------------------------------------
void DeInitNetworkInterface(void);
//-------------------------------------------------------------------------------------------------
void SetNetworkManagerDebugConfig(void);
//-------------------------------------------------------------------------------------------------
const CHAR *GetNetworkPortName(NETWORK_PORT_e portType, CHAR *ifaceName);
//-------------------------------------------------------------------------------------------------
BOOL RegisterNetworkPort(NETWORK_PORT_e portType, const CHAR *ifaceName);
//-------------------------------------------------------------------------------------------------
BOOL DeregisterNetworkPort(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL NetworkPortLinkUp(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL NetworkPortLinkDown(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL GetNetworkPortMacAddr(NETWORK_PORT_e portType, CHAR *macAddr);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv4Addr(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv4Mode(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv4StaticMode(NETWORK_PORT_e portType, const IPV4_NETWORK_PARAMETER_t *staticAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv4DhcpMode(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv4DhcpAddr(NETWORK_PORT_e portType, const IPV4_NETWORK_PARAMETER_t *dhcpAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv4PppoeMode(NETWORK_PORT_e portType, const PPPOE_PARAMETER_t *pppoeInfo);
//-------------------------------------------------------------------------------------------------
BOOL AddIpv4DefaultGateway(NETWORK_PORT_e portType, const IPV4_NETWORK_PARAMETER_t *gatewayAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv4DefaultGateway(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL AddIpv4StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, const CHAR *subnet, const CHAR *gatewayAddr);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv4StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, const CHAR *subnet, const CHAR *gatewayAddr);
//-------------------------------------------------------------------------------------------------
BOOL EnableIpv6OnInterface(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL DisableIpv6OnInterface(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv6Addr(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL UpdateIpv6AddrAttr(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *addrAttrInfo);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv6Mode(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv6StaticMode(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *staticAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv6DhcpMode(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv6DhcpAddr(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *dhcpAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv6SlaacMode(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL SetIpv6SlaacAddr(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *slaacAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL AddIpv6DefaultGateway(NETWORK_PORT_e portType, const IPV6_NETWORK_PARAMETER_t *gatewayAddrInfo);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv6DefaultGateway(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL AddIpv6StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 prefixLen, const CHAR *gateway);
//-------------------------------------------------------------------------------------------------
BOOL RemoveIpv6StaticRoute(NETWORK_PORT_e portType, const CHAR *networkAddr, UINT8 prefixLen, const CHAR *gateway);
//-------------------------------------------------------------------------------------------------
BOOL StartUsbToSerialModem(NETWORK_PORT_e portType, const BROAD_BAND_PROFILE_t *usbModemConfig);
//-------------------------------------------------------------------------------------------------
BOOL StopUsbToSerialModem(NETWORK_PORT_e portType);
//-------------------------------------------------------------------------------------------------
BOOL SetDnsServerAddr(const DNS_PARAMETER_t *ipv4DnsAddr, const DNS_PARAMETER_t *ipv6DnsAddr);
//-------------------------------------------------------------------------------------------------
BOOL SetSystemHostName(const CHAR *hostname);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @END OF FILE
//#################################################################################################
#endif /* _NETWORK_INTERFACE_ */
