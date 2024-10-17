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
 * @file nm_iputility.h
 * @brief Network Manager IP address utility functions
 *
 * This header file contains the helper functions related to
 * working with IPv4 and IPv6 addresses.
 *
 * @note This APIs can be used as standalone module without
 * initializing the Network Manager library.
 *
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <netdb.h>

#include "nm_stddef.h"

/*****************************************************************************
 * MACROS
 *****************************************************************************/

/**
 * @brief The loopback hostname.
 */
#define NM_LOOPBACK_HOSTNAME        "localhost"

/**
 * @brief The IPv4 loopback address string.
 */
#define NM_IPV4_LOOPBACK_ADDR_STR   "127.0.0.1"

/**
 * @brief The IPv6 loopback address string.
 */
#define NM_IPV6_LOOPBACK_ADDR_STR   "::1"

/*****************************************************************************
 * ENUMS
 *****************************************************************************/


/**
 * @brief IP address family.
 */
typedef enum
{
    NM_IPADDR_FAMILY_V4,       /**< IPv4 address family */
    NM_IPADDR_FAMILY_V6,       /**< IPv6 address family */
    NM_IPADDR_FAMILY_INVALID   /**< Invalid address family */

} NM_IpAddrFamily_e;

/**
 * @brief IP address scope.
 */
typedef enum
{
    /** @brief Unknown IP address */
    NM_IPADDR_SCOPE_UNKNOWN = 0,

    /** @brief IPv4 normal address (i.e. 192.168.1.1, 10.1.2.3) */
    NM_IPADDR_SCOPE_V4_NORMAL,

     /** @brief IPv4 unspecified address (i.e. 0.0.0.0) */
    NM_IPADDR_SCOPE_V4_UNSPECIFIED,

    /** @brief IPv4 loopback address (i.e. 127.0.0.x) */
    NM_IPADDR_SCOPE_V4_LOOPBACK,

    /** @brief IPv4 multicast address (i.e. 224.x.x.x) */
    NM_IPADDR_SCOPE_V4_MULTICAST,

    /** @brief IPv4 broadcast address (i.e. 255.255.255.255) */
    NM_IPADDR_SCOPE_V4_BROADCAST,

    /** @brief IPv6 global unicast address (i.e. 2000::/3) */
    NM_IPADDR_SCOPE_V6_UNICAST_GLOBAL,

    /** @brief IPv6 link-local unicast address (i.e. fe80::/10) */
    NM_IPADDR_SCOPE_V6_UNICAST_LINK_LOCAL,

    /** @brief IPv6 site-local unicast address (i.e. fec0::/10) */
    NM_IPADDR_SCOPE_V6_UNICAST_SITE_LOCAL,

    /** @brief IPv6 unique-local unicast address (i.e. fc00::/7 or fd00::/7) */
    NM_IPADDR_SCOPE_V6_UNICAST_UNIQUE_LOCAL,

    /** @brief IPv6 unspecified address (i.e. ::) */
    NM_IPADDR_SCOPE_V6_UNSPECIFIED,

    /** @brief IPv6 loopback address (i.e. ::1) */
    NM_IPADDR_SCOPE_V6_LOOPBACK,

    /** @brief IPv6 multicast address (i.e. ff00::/12) */
    NM_IPADDR_SCOPE_V6_MULTICAST,

    /** @brief IPv6 v4 mapped address (i.e. ::/80 or "::ffff:192.168.1.1") */
    NM_IPADDR_SCOPE_V6_V4MAPPED,

    /** @brief IPv6 v4 compatibility address (i.e. ::/80 or "::192.168.1.1") */
    NM_IPADDR_SCOPE_V6_V4COMPAT

} NM_IpAddrScope_e;

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/

//--------------------------------------------------------------------------
// IP Address, Family, Subnet Mask, Prefix Validation Functions
//--------------------------------------------------------------------------

/**
 * @brief Get the IP address family from the given IP address string.
 *
 * @param ipAddrStr The IP address string.
 * @return The IP address family.
 */
NM_IpAddrFamily_e NMIpUtil_GetIpAddrFamily(const mxChar *ipAddrStr);

/**
 * @brief Check if the given IP address string is valid.
 *
 * @param ipAddrStr The IP address string.
 * @return NM_TRUE if the IP address is valid, NM_FALSE otherwise.
 */
mxBool NMIpUtil_IsValidIpAddr(const mxChar *ipAddrStr);

/**
 * @brief Check if the given IPv4 address string is valid.
 *
 * @param ipAddrStr The IPv4 address string.
 * @return NM_TRUE if the IPv4 address is valid, NM_FALSE otherwise.
 */
mxBool NMIpUtil_IsValidIpv4Addr(const mxChar *ipAddrStr);

/**
 * @brief Check if the given IPv4 subnet mask is valid.
 *
 * @param subnetMask The IPv4 subnet mask.
 * @return NM_TRUE if the IPv4 subnet mask is valid, NM_FALSE otherwise.
 */
mxBool NMIpUtil_IsValidIpv4Subnet(const mxChar *subnetMask);

/**
 * @brief Check if the given IPv4 address string is valid.
 *
 * @param ipAddrStr The IPv6 address string.
 * @return NM_TRUE if the IPv6 address is valid, NM_FALSE otherwise.
 */
mxBool NMIpUtil_IsValidIpv6Addr(const mxChar *ipAddrStr);

/**
 * @brief Check if the given IPv6 prefix length is valid.
 *
 * @param prefixLength The IPv6 prefix length.
 * @return NM_TRUE if the prefix length is valid, NM_FALSE otherwise.
 */
mxBool NMIpUtil_IsValidIpv6PrefixLen(mxU8_t prefixLength);

//--------------------------------------------------------------------------
// IP Address Scope Functions
//--------------------------------------------------------------------------

/**
 * @brief Get the scope of the given IP address string.
 *
 * @param ipAddrStr The IP address string.
 * @return The IP address scope.
 */
NM_IpAddrScope_e NMIpUtil_GetIpAddrScope(const mxChar *ipAddrStr);

/**
 * @brief Check if the given IP address is within the allowed scope.
 *
 * @param ipAddrStr The IP address string.
 * @param allowUnspecified Whether to allow unspecified IPv4/IPv6 address.
 * @param allowLoopback Whether to allow loopback IPv4/IPv6 address.
 * @param allowMulticast Whether to allow multicast IPv4/IPv6 address.
 * @return NM_TRUE if the IP address is within the allowed scope, NM_FALSE otherwise.
 */
mxBool NMIpUtil_IsIpWithinAllowedScope(const mxChar *ipAddrStr, mxBool allowUnspecified, mxBool allowLoopback, mxBool allowMulticast);

//--------------------------------------------------------------------------
// IP Address Conversion Functions
//--------------------------------------------------------------------------

/**
 * @brief Converts IPv4 and IPv6 addresses from binary to text form for given family.
 *
 * @param family The IP address family.
 * @param ipAddrNw The IPv4/IPv6 network address structure.
 * @param ipAddrStr The buffer to store the IPv4/IPv6 address string.
 * @param strBufLen The size of the IPv4/IPv6 address string buffer.
 * @return NM_TRUE if the conversion is successful, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ConvertNetworkAddrToString(NM_IpAddrFamily_e family, const void *ipAddrNw, mxChar *ipAddrStr, socklen_t strBufLen);

/**
 * @brief Converts IPv4 address from binary to text form.
 *
 * @param ipAddrNw The IPv4 network address structure.
 * @param ipAddrStr The buffer to store the IPv4 address string.
 * @param strBufLen The size of the IPv4 address string buffer.
 * @return NM_TRUE if the conversion is successful, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ConvertIpv4NetworkAddrToString(const void *ipAddrNw, mxChar *ipAddrStr, socklen_t strBufLen);

/**
 * @brief Converts IPv6 address from binary to text form.
 *
 * @param ipAddrNw The IPv6 network address structure.
 * @param ipAddrStr The buffer to store the IPv6 address string.
 * @param strBufLen The size of the IPv6 address string buffer.
 * @return NM_TRUE if the conversion is successful, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ConvertIpv6NetworkAddrToString(const void *ipAddrNw, mxChar *ipAddrStr, socklen_t strBufLen);

/**
 * @brief Converts IPv4 and IPv6 addresses from text to binary form.
 *
 * @param ipAddrStr The IP address string.
 * @param ipAddrNw The buffer to store the IPv4/IPv6 in network address structure.
 *        Size of buffer must be sizeof(struct in_addr) for IPv4 address and sizeof(struct in6_addr) for IPv6 address.
 * @return NM_TRUE if the conversion is successful, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ConvertStringToNetworkAddr(const mxChar *ipAddrStr, void *ipAddrNw);

/**
 * @brief Converts IPv4 address from text to binary form.
 *
 * @param ipAddrStr The IPv4 address string.
 * @param ipAddrNw The buffer to store the IPv4 in network address structure.
 *        Size of buffer must be sizeof(struct in_addr).
 * @return NM_TRUE if the conversion is successful, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ConvertIpv4StringToNetworkAddr(const mxChar *ipAddrStr, void *ipAddrNw);

/**
 * @brief Converts IPv6 address from text to binary form.
 *
 * @param ipAddrStr The IPv6 address string.
 * @param ipAddrNw The buffer to store the IPv6 in network address structure.
 *        Size of buffer must be sizeof(struct in6_addr).
 * @return NM_TRUE if the conversion is successful, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ConvertIpv6StringToNetworkAddr(const mxChar *ipAddrStr, void *ipAddrNw);

//--------------------------------------------------------------------------
// DNS to IP Address Resolved Functions
//--------------------------------------------------------------------------

/**
 * @brief Resolve the domain name to IP addresses.
 *
 * @param domain The hostname or fully qualified domain name.
 * @param ipv4Addr The buffer to store the resolved IPv4 address.
 * Pass NULL if want to resolve IPv6 address only.
 * @param ipv6Addr The buffer to store the resolved IPv6 address.
 * Pass NULL if want to resolve IPv4 address only.
 * @return NM_TRUE if the domain name is resolved successfully, NM_FALSE otherwise.
 */
mxBool NMIpUtil_ResolveDomainName(const mxChar *domain, mxChar *ipv4Addr, mxChar *ipv6Addr);

#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
