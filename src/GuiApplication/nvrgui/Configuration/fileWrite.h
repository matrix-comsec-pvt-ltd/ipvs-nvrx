#ifndef FILEWRITE_H
#define FILEWRITE_H
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
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////


#include "EnumFile.h"


//******** Defines and Data Types ****
// configuration file permission
#define CONFIG_FILE_PERIMISSION     (QFile::ReadOwner |     \
    QFile::WriteOwner |    \
    QFile::ReadUser |      \
    QFile::ReadGroup |     \
    QFile::ReadOther)

// configuiration file path
#define CONFIG_FILE_PATH            CONFIG_DIR_PATH "/appConfig/"

// version typedef
typedef quint8 VERSION_e;


//******** Function Prototypes *******



#endif // FILEWRITE_H
