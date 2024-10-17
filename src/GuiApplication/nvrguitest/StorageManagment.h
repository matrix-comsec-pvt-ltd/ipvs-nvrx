#ifndef STORAGEMANAGMENT_H
#define STORAGEMANAGMENT_H

#include <QObject>
#include <netinet/in.h>
#include "Enumfile.h"

#define   MAX_TX_SZ                     1024
#define   MAX_RCV_SZ                    16384
#define   FSP                           0x1E
#define	  RECV_MSG_HEADER_LENGTH        25

typedef enum NET_CMD_STATUS_e
{
    CMD_SUCCESS = 0,				// 00 : command success [generic]
    CMD_INVALID_MESSAGE,			// 01 : error 0x01 [network manager]
    CMD_INVALID_SESSION,			// 02 : error 0x02 [network manager]
    CMD_INVALID_SYNTAX  			// 03 : error 0x03 [network manager]
}NET_CMD_STATUS_e;

// This was enum of format type of file system on USB (Backup) device and hard disk
typedef enum
{
    EXT_4,
    FAT,
    NTFS,
    FORMAT_MAX
}RAW_MEDIA_FORMAT_TYPE_e;

typedef struct
{
    char 			subSys[25];
    char 			action[25];
    char 			path[128];
    char 			serial[128];
    char 			baseNode[25];
}UDEV_DEVICE_INFO_t;

class StorageManagment : public QObject
{
    Q_OBJECT
private:
    int internalServerFd;

signals:
    void act1(UDEV_DEVICE_INFO_t devInfo);

public:
    explicit StorageManagment(QObject *parent = 0);

    Q_INVOKABLE void internalMsgServerLoop();

    void CloseSocket(int* connFd);
    bool DetectStorageMedia(UDEV_DEVICE_INFO_t * deviceInfo);
    bool ParseStr(char* * src, char delim, char* dest, unsigned int maxDestSize);
    NET_CMD_STATUS_e FindHeaderIndex(char** tmpBufPtr);
    bool RecvMessage(int connFd, char* rcvMsg, unsigned int* rcvLen, char startOfData,
                     char endOfData, unsigned int maxData, UINT32 timeout);
    unsigned char ConvertStringToIndex(char* strPtr,const char* * strBuffPtr, unsigned char maxIndex);
    INT32 GetPidOfProcess(CHARPTR processName);
    BOOL AsciiToInt(CHARPTR ascii, UINT64PTR integer);

    Q_INVOKABLE bool mountDisk(CHARPTR deviceNode, CHARPTR mountPath, RAW_MEDIA_FORMAT_TYPE_e fsType);
    Q_INVOKABLE bool mountDevice(QString type, QString baseNode);
    Q_INVOKABLE bool unmountDevice(QString type);
    void udev_update_uevent(struct udev_enumerate *udev_enumerate, const char *action);
};

#endif // STORAGEMANAGMENT_H
