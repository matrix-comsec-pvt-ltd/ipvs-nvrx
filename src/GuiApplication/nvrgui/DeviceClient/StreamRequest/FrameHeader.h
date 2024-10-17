#ifndef FRAMEHEADER_H
#define FRAMEHEADER_H
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
//   Project      : NVR ( Network Video Recorder)
//   Owner        : <Owner Name>
//   File         : <File name>
//   Description  : Brief but meaningful description of what file provides.
//
/////////////////////////////////////////////////////////////////////////////

#include <QObject>


//******** Defines and Data Types ****

// structure which holds frame information
typedef struct __attribute__((packed))
{
    quint32 magicCode;          // constant value [0x000001FF]
    quint8 version;             // version of the product
    quint8 productType;         // product type [NVR - DVR]
    quint32 frameSize;          // frame size including FSH size
    quint32 seconds;            // date time in seconds
    quint16 mSec;               // milli seconds
    quint8 channel;             // channel number
    quint8 streamType;          // stream type [audio / video]
    quint8 codecType;           // codec type
                                    // stream type : Video
                                        // [MJPEG, H.264, MPEG-4]
                                    // stream type : Audio
                                        // [G711, G726-8, G726-16, G726-24,
                                        // G726-32, G726-40, AAC]
    quint8 frameRate;           // frame rate
    quint8 frameType;           // frame type [I frame, P frame, B frame]
    quint8 resolution;          // video resolution
    quint8 videoFormat;         // video format [NTSC, PAL]
    quint8 scanType;            // video scan type [interlace - progressive]
    quint8 streamStatus;        // stream status [normal, file error,
    // hdd format, configuration change,
    // playback over]
    quint8 videoLoss;           // video loss, no video loss
    quint16 audioSampleFreq;    // audio sample frequency in hertz
    quint8 noOfRefFrame;        // number of reference frame
    quint8 syncFrameNum;        // sync frame number used at time of seek in sync- pb
    quint8 reserved[6];         // reserved for future use
    quint32 preReserved;        // reserved for PC use
    /* -------------- */
    /* total 40 bytes */

} FRAME_HEADER_t;


//******** Function Prototypes *******




#endif // FRAMEHEADER_H
