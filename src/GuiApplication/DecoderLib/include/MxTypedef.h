#if !defined MXTYPEDEF_H
#define MXTYPEDEF_H
///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : NVR (Network Video Recorder)
//   Owner        : Bharat Gohil
//   File         : MxTypedef.h
//   Description  : This file declares all project specific data type, defines
//					and constants that are used in the entire project.
//					All source file (.c) should include this file.
//
/////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"{
#endif

#if defined(OEM_JCI)
#define MATRIX_DIR_PATH         "/nvr"
#else
#define MATRIX_DIR_PATH         "/matrix"
#endif

#define CONFIG_DIR_PATH         MATRIX_DIR_PATH "/config"
#define LANGUAGES_DIR_PATH      MATRIX_DIR_PATH "/languages"
#define LOG_DIR_PATH            MATRIX_DIR_PATH "/log"
#define MEDIA_DIR_PATH          MATRIX_DIR_PATH "/media"

#define KILO_BYTE 				(1024)
#define MEGA_BYTE 				(KILO_BYTE * KILO_BYTE)
#define GIGA_BYTE				(MEGA_BYTE * KILO_BYTE)

// total second in one minute
#define	MSEC_IN_ONE_SEC			(1000)
#define	SEC_IN_ONE_MIN			(60)
#define	MIN_IN_ONE_HOUR			(60)
#define SEC_IN_ONE_HOUR			(MIN_IN_ONE_HOUR * SEC_IN_ONE_MIN)
#define HOUR_IN_ONE_DAY			(24)

// Indicates status ok
#define STATUS_OK				(0)
#define NILL					(-1)
#define INVALID_FILE_FD			(-1)
#define INVALID_CONNECTION		(-1)

//******** Defines and Data Types ****
enum
{
	FALSE = 0,
	TRUE,

	FREE = 0,
	BUSY,
    PENDING,

	FAIL = 0,
	SUCCESS,
};

// Do no use other than these Data types in projects.
typedef unsigned char		BOOL;
typedef char				CHAR;
typedef signed char			INT8;
typedef unsigned char		UINT8;

typedef signed short		INT16;
typedef unsigned short		UINT16;

typedef signed int			INT32;
typedef unsigned int  		UINT32;

typedef signed long long	INT64;
typedef unsigned long long	UINT64;

typedef INT8 *				INT8PTR;
typedef CHAR *				CHARPTR;
typedef UINT8 *				UINT8PTR;

typedef INT16 *				INT16PTR;
typedef UINT16 *			UINT16PTR;

typedef INT32 *				INT32PTR;
typedef UINT32 *			UINT32PTR;

typedef INT64 *				INT64PTR;
typedef UINT64 *			UINT64PTR;

#ifdef __cplusplus
}
#endif
#endif	// MXSYSTYPEDEF_H_
