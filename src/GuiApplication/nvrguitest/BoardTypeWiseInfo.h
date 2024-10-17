#ifndef BOARDTYPEWISEINFO_H
#define BOARDTYPEWISEINFO_H

#include <QObject>

typedef enum
{
#if defined(RK3568_NVRL)
    NVR_XP2_BOARD_V1R0 = 0,
    NVR_XP2_BOARD_V1R1 = 1,
#elif defined(RK3588_NVRH)
    NVR_XP2_BOARD_V1R0 = 0,
#else
    NVR_X_BOARD_V1R2 = 0,
    NVR_X_BOARD_V1R3 = 1,
    NVR_X_BOARD_V2R2 = 2,
#endif
    MAX_NVR_X_BOARD_VERSION
}NVR_X_BOARD_VERSION_e;

typedef enum
{
#if defined(OEM_JCI)
    HRIN_1208_18_SR = 0,
    HRIN_2808_18_SR,
    HRIN_4808_18_SR,
    HRIN_6408_18_SR,
#else
    NVR0801X = 0,
    NVR1601X,
    NVR1602X,
    NVR3202X,
    NVR3204X,
    NVR6404X,
    NVR6408X,
    NVR0801XP2,
    NVR1601XP2,
    NVR1602XP2,
    NVR3202XP2,
    NVR3204XP2,
    NVR6404XP2,
    NVR6408XP2,
    NVR0801XSP2,
    NVR1601XSP2,
    NVR0401XSP2,
    NVR9608XP2,
#endif
    VARIANT_MAX
}VARIANT_e;

class BoardTypeWiseInfo : public QObject
{
    Q_OBJECT
public:
    static VARIANT_e    productVariant;
    static quint8       noOfUsb;
    static quint8       noOfHdd;
    static quint8       noOfLan;
    static quint8       noOfSensorInOut;
    static bool         isAudioInOutSupport;
    static bool         isExtRTC;
    static bool         isMultipAvail;
};
#endif // BOARDTYPEWISEINFO_H
