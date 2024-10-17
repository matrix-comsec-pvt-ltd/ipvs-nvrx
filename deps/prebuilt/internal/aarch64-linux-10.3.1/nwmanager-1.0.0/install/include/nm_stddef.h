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
 * @file nm_stddef.h
 * @brief Network Manager data types and return value status codes.
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <inttypes.h>
#include <stddef.h>

/*****************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/**
 * @brief Boolean type, represents true or false.
 */
typedef unsigned char mxBool;

/**
 * @brief Character type.
 */
typedef char mxChar;

/**
 * @brief Unsigned character type
 */
typedef unsigned char mxUchar;

/**
 * @brief 8-bit signed integer
 */
typedef int8_t mxI8_t;

/**
 * @brief 16-bit signed integer
 */
typedef int16_t mxI16_t;

/**
 * @brief 32-bit signed integer
 */
typedef int32_t mxI32_t;

/**
 * @brief 64-bit signed integer
 */
typedef int64_t mxI64_t;

/**
 * @brief 8-bit unsigned integer
 */
typedef uint8_t mxU8_t;

/**
 * @brief 16-bit unsigned integer
 */
typedef uint16_t mxU16_t;

/**
 * @brief 32-bit unsigned integer
 */
typedef uint32_t mxU32_t;

/**
 * @brief 64-bit unsigned integer
 */
typedef uint64_t mxU64_t;

/**
 * @brief Single-precision floating point number
 */
typedef float mxFloat_t;

/**
 * @brief Long integer
 */
typedef long mxLong_t;

/**
 * @brief Double-precision floating point number
 */
typedef double mxDouble_t;

/**
 * @brief Long long integer
 */
typedef long long mxLL_t;

/**
 * @brief Unsigned integer type used to represent sizes
 */
typedef size_t mxSize_t;

/*****************************************************************************
 * ENUMS
 *****************************************************************************/

/**
 * @brief Status codes used in network manager operations.
 */
typedef enum
{
    NMSTS_SUCCESS,                  /**< Operation completed successfully */
    NMSTS_IN_PROGRESS,              /**< Operation is in progress */
    NMSTS_UNKNOWN_ERR,              /**< Error that is not known or categorized */
    NMSTS_UNEXPECTED_ERR,           /**< Unexpected error occurred */
    NMSTS_INVALID_ARG,              /**< Invalid argument provided */
    NMSTS_NO_RESOURCE,              /**< Insufficient resources available */
    NMSTS_RESOURCE_BUSY,            /**< Resource is busy */
    NMSTS_CONNECTION_ERR,           /**< Socket connection error */
    NMSTS_OPERATION_FAIL,           /**< Operation failed */
    NMSTS_OPERATION_TIMEOUT,        /**< Operation timed out */
    NMSTS_UNEXPECTED_ACTION,        /**< Action is not handled */
    NMSTS_ALREADY_INITIALIZED,      /**< Already initialized */
    NMSTS_PROTOCOL_MISMATCH,        /**< Protocol mismatch */
    NMSTS_FILE_IO_ERR,              /**< File I/O error */
    NMSTS_INTERFACE_INVALID,        /**< Invalid interface */
    NMSTS_INTERFACE_LIMIT_EXCEED,   /**< Interface limit exceeded */
    NMSTS_INTERFACE_ALREADY_EXIST,  /**< Interface already exists */
    NMSTS_INTERFACE_NOT_EXIST,      /**< Interface does not exist */
    NMSTS_IPV6_DISABLED,            /**< IPv6 disabled on interface */
    NMSTS_IPV6_NOT_SUPPORTED,       /**< IPv6 not supported on platform */
    NMSTS_HOSTNAME_TOO_BIG,         /**< Hostname is too big */
    NMSTS_VLAN_NOT_ALLOWED          /**< VLAN not allowed on given interface */

} NMSts_e;

/**
 * @brief Boolean status codes.
 */
enum
{
    NM_FALSE = 0,   /**< Represents boolean false */
    NM_TRUE = 1     /**< Represents boolean true */
};

#ifdef __cplusplus
}
#endif
/*****************************************************************************
 * @END OF FILE
 *****************************************************************************/
