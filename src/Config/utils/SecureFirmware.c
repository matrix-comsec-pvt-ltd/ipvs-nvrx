// #################################################################################################
//  FILE BRIEF
// #################################################################################################
/**
@file		SecureFirmware.c
@brief      This file contains firmware encryption and data signing functions.
*/
// #################################################################################################
//  @INCLUDES
// #################################################################################################
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// #################################################################################################
//  @TYPEDEF
// #################################################################################################
typedef unsigned char      BOOL;
typedef char               CHAR;
typedef unsigned char      UINT8;
typedef unsigned short int UINT16;
typedef int                INT32;
typedef unsigned int       UINT32;

// #################################################################################################
//  @DEFINES
// #################################################################################################
#define SIGN_SERVER_ADDR  "192.168.101.195"
#define SIGN_SERVER_PORT  5123

#define MSG_MAGIC_CODE    "SGN_DATA"
#define MAGIC_CODE_LEN    8
#define RANDOM_KEY_PATH   "/tmp/nvr.tmp"
#define RANDOM_KEY_LEN    64
#define SIGN_DATA_LEN_MAX 256
#define DEBUG_ENABLE      0

#define FALSE             0
#define TRUE              1

#if defined(OEM_JCI)
#define SIGN_PRODUCT_ID    1
#define SIGN_FIRMWARE_TYPE 3

UINT8 obfuscatedKey[RANDOM_KEY_LEN] = {
    0xe1, 0xd3, 0xbe, 0x6b, 0x1e, 0xc3, 0x66, 0xdc,  // First part
    0xc5, 0xcb, 0xc4, 0x5c, 0x86, 0xde, 0x87, 0x75,  // Second part
    0xbe, 0x7b, 0x7a, 0xdf, 0xf6, 0x24, 0x0c, 0x15,  // Third part
    0x41, 0x7e, 0xb7, 0x74, 0x7b, 0xe7, 0x11, 0x91,  // Fourth part
    0x5c, 0xd3, 0xfa, 0xc4, 0x90, 0xd1, 0x34, 0xa1,  // Fifth part
    0x5b, 0x12, 0xdf, 0xcc, 0xb6, 0xda, 0x07, 0x2c,  // Sixth part
    0xb1, 0x21, 0x56, 0x1d, 0x4f, 0x83, 0xcf, 0xb9,  // Seventh part
    0xa2, 0x5f, 0x30, 0x3b, 0x12, 0x7b, 0x38, 0xf5   // Eighth part
};
#else
#error "Invalid build type found"
#endif

// #################################################################################################
//  @DATA TYPES
// #################################################################################################
typedef enum
{
    RSA_SIGN_STATUS_SUCCESS = 0,
    RSA_SIGN_STATUS_PRODUCT_ID_INVALID,
    RSA_SIGN_STATUS_FIRMWARE_TYPE_INVALID,
    RSA_SIGN_STATUS_SIGNING_ERROR,
    RSA_SIGN_STATUS_MAX
} RSA_SIGN_STATUS_e;

typedef struct __attribute__((packed))
{
    CHAR   magicCode[MAGIC_CODE_LEN];
    UINT32 totalMsgLen;
    UINT16 productId;
    UINT16 msgId;
    UINT16 noOfField;
    UINT16 ipAddrLen;
    UINT16 keyTypeLen;
    UINT16 sgnDataLen;
    CHAR   localIpAddr[INET_ADDRSTRLEN];
    UINT16 KeyType;
} RSA_SIGN_MSG_SND_t;

typedef struct __attribute__((packed))
{
    UINT16 productId;
    UINT16 msgId;
    UINT16 noOfField;
    UINT16 respStatusLen;
    UINT16 sgnDataLen;
    UINT16 respStatus;
} RSA_SIGN_MSG_RCV_t;

// #################################################################################################
//  @PROTOTYPES
// #################################################################################################
//-------------------------------------------------------------------------------------------------
static void printBinaryData(const UINT8 *dataBuff, UINT32 dataLen);
//-------------------------------------------------------------------------------------------------
static void getLocalIpAddr(CHAR *localIpAddr);
//-------------------------------------------------------------------------------------------------
static BOOL calculateFileSha256Hash(const CHAR *filePath, UINT8 *outputHash, UINT32 *outHashLen);
//-------------------------------------------------------------------------------------------------
static INT32 createTcpConnection(const CHAR *domain, INT32 port);
//-------------------------------------------------------------------------------------------------
static BOOL openSslConnection(INT32 *sockFd, SSL_CTX **sslCtx, SSL **ssl);
//-------------------------------------------------------------------------------------------------
static void closeSslConnection(INT32 sockFd, SSL_CTX *sslCtx, SSL *ssl);
//-------------------------------------------------------------------------------------------------
static BOOL getSignedHashFromServer(const UINT8 *dataHash, UINT8 *signedHashData);
//-------------------------------------------------------------------------------------------------
static void prepHashSignRequest(const UINT8 *dataHash, RSA_SIGN_MSG_SND_t *sendMsgInfo, UINT8 *msgHash);
//-------------------------------------------------------------------------------------------------
static BOOL sendHashSignRequest(SSL *ssl, const RSA_SIGN_MSG_SND_t *sendMsgInfo, const UINT8 *dataHash, UINT8 *msgHash);
//-------------------------------------------------------------------------------------------------
static BOOL recvHashSignResponse(SSL *ssl, UINT8 *signedHashData);
//-------------------------------------------------------------------------------------------------
static BOOL prepRandomKeyFile(const CHAR *filename);
//-------------------------------------------------------------------------------------------------
static BOOL appendSignedHashInFirmwareFile(const CHAR *filePath, const UINT8 *signedHashData);
//-------------------------------------------------------------------------------------------------
static BOOL encryptFirmwareFile(const CHAR *filePath, const char *encFilePath);
//-------------------------------------------------------------------------------------------------
// #################################################################################################
// @FUNCTIONS
// #################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief Main function of process
 *
 * @param argc
 * @param argv
 * @return INT32
 */
INT32 main(INT32 argc, CHAR *argv[])
{
    UINT8  dataHash[SHA256_DIGEST_LENGTH];
    UINT8  signedHashData[SIGN_DATA_LEN_MAX];
    UINT32 dataHashLen = 0;
    CHAR  *firmwareFile = NULL;
    CHAR  *encFirmwareFile = NULL;

    // Check if there are enough arguments
    if (argc != 3)
    {
        printf("Usage: %s <Firmware File> <Encrypted Firmware File>\n", argv[0]);
        return 1;
    }

    // Store input and output firmware file name
    firmwareFile = argv[1];
    encFirmwareFile = argv[2];

    // Load encryption and hash algo's in ssl
    SSL_library_init();

    /* Load cryptos, et.al. */
    OpenSSL_add_all_algorithms();

    /* Bring in and register error messages */
    SSL_load_error_strings();

    // Get hash of the input firmware file
    if (FALSE == calculateFileSha256Hash(firmwareFile, dataHash, &dataHashLen))
    {
        return 2;
    }

    // Get signed hash of firmware from server
    if (FALSE == getSignedHashFromServer(dataHash, signedHashData))
    {
        return 3;
    }

    // Print signed hash data
    printBinaryData(signedHashData, sizeof(signedHashData));

    // Append signed hash data to firmware file
    if (FALSE == appendSignedHashInFirmwareFile(firmwareFile, signedHashData))
    {
        return 4;
    }

    // Encrypt the firmware file
    if (FALSE == encryptFirmwareFile(firmwareFile, encFirmwareFile))
    {
        return 5;
    }

    printf("firmware file encrypted successfully\n");
    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Prints binary data for debugging.
 *
 * @param dataBuff
 * @param dataLen
 */
static void printBinaryData(const UINT8 *dataBuff, UINT32 dataLen)
{
#if DEBUG_ENABLE
    UINT32 byteIdx;
    printf("binary data: ");
    for (byteIdx = 0; byteIdx < dataLen; byteIdx++)
    {
        printf("%02x", dataBuff[byteIdx]);
    }
    printf("\n");
#endif
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves the local machine's IPv4 address.
 *
 * @param localIpAddr
 */
static void getLocalIpAddr(CHAR *localIpAddr)
{
    struct ifaddrs *ifap, *ifa;

    if (getifaddrs(&ifap) == -1)
    {
        printf("failed to get iface addrinfo: [err=%s]\n", strerror(errno));
        abort();
    }

    for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
    {
        if ((ifa->ifa_addr == NULL) || (strcmp(ifa->ifa_name, "lo") == 0) || (ifa->ifa_addr->sa_family != AF_INET))
        {
            continue;
        }

        // Get address in string format
        inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, localIpAddr, INET_ADDRSTRLEN);
    }

    // Free all interfaces memory
    freeifaddrs(ifap);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Calculate sha256 hash of given firmware
 *
 * @param filePath
 * @param outputHash
 * @param outHashLen
 * @return BOOL
 */
static BOOL calculateFileSha256Hash(const CHAR *filePath, UINT8 *outputHash, UINT32 *outHashLen)
{
    EVP_MD_CTX *mdCtx;
    FILE       *fileFp;
    UINT8       buffer[PATH_MAX];
    size_t      dataLen = 0;

    fileFp = fopen(filePath, "rb");
    if (fileFp == NULL)
    {
        printf("Failed to open file: [err=%s]\n", strerror(errno));
        return FALSE;
    }

    mdCtx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdCtx, EVP_sha256(), NULL);

    // Read file in chunks and update the hash context
    while ((dataLen = fread(buffer, 1, PATH_MAX, fileFp)) != 0)
    {
        EVP_DigestUpdate(mdCtx, buffer, dataLen);
    }

    EVP_DigestFinal_ex(mdCtx, outputHash, outHashLen);
    EVP_MD_CTX_free(mdCtx);
    fclose(fileFp);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Creates a TCP connection to the specified host and port.
 *
 * @param domainName
 * @param port
 * @return INT32
 */
static INT32 createTcpConnection(const CHAR *domainName, INT32 port)
{
    INT32              sockFd;
    struct sockaddr_in sockAddr;
    struct addrinfo    hints, *result = NULL, *addr = NULL;

    // Set ip address resolve parameters
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    // Get IP addresses from domain address
    if (getaddrinfo(domainName, NULL, &hints, &result) != 0)
    {
        printf("getaddrinfo failed: [domain=%s], [err=%s]\n", domainName, strerror(errno));
        abort();
    }

    for (addr = result; addr != NULL; addr = addr->ai_next)
    {
        // Only check for IPv4 address
        if (addr->ai_family == AF_INET)
        {
            break;
        }
    }

    // Free address info
    freeaddrinfo(result);

    // Is valid addr found?
    if (addr == NULL)
    {
        printf("valid ip addr not found: [domain=%s]\n", domainName);
        abort();
    }

    // Open socket for communication
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1)
    {
        printf("failed to open socket: [err=%s]\n", strerror(errno));
        abort();
    }

    // Set destination address information
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    sockAddr.sin_addr = ((struct sockaddr_in *)addr->ai_addr)->sin_addr;

    // Initiate a connection on a socket
    if (connect(sockFd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) != 0)
    {
        printf("connect failed: [domain=%s], [err=%s]\n", domainName, strerror(errno));
        close(sockFd);
        abort();
    }

    return sockFd;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Create a SSL Connection
 *
 * @param sockFd
 * @param sslCtx
 * @param ssl
 * @return BOOL
 */
static BOOL openSslConnection(INT32 *sockFd, SSL_CTX **sslCtx, SSL **ssl)
{
    // Make TCP connection to server
    *sockFd = createTcpConnection(SIGN_SERVER_ADDR, SIGN_SERVER_PORT);
    if (*sockFd == -1)
    {
        return FALSE;
    }

    /* Create new context */
    *sslCtx = SSL_CTX_new(TLS_client_method());
    if (*sslCtx == NULL)
    {
        ERR_print_errors_fp(stderr);
        return FALSE;
    }

    /* create new SSL connection state */
    *ssl = SSL_new(*sslCtx);

    /* attach the socket descriptor */
    SSL_set_fd(*ssl, *sockFd);

    /* perform the connection */
    if (SSL_connect(*ssl) == -1)
    {
        ERR_print_errors_fp(stderr);
    }

    printf("Connected with %s encryption\n", SSL_get_cipher(*ssl));
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Close the SSL connection
 *
 * @param sockFd
 * @param sslCtx
 * @param ssl
 */
static void closeSslConnection(INT32 sockFd, SSL_CTX *sslCtx, SSL *ssl)
{
    // Release connection state
    if (ssl != NULL)
    {
        SSL_free(ssl);
    }

    // Close socket
    if (sockFd != -1)
    {
        close(sockFd);
    }

    // Release context
    if (sslCtx != NULL)
    {
        SSL_CTX_free(sslCtx);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Get the Signed Hash From Secure Server
 *
 * @param dataHash
 * @param signedHashData
 * @return BOOL
 */
static BOOL getSignedHashFromServer(const UINT8 *dataHash, UINT8 *signedHashData)
{
    SSL               *ssl = NULL;
    SSL_CTX           *sslCtx = NULL;
    INT32              serverFd = -1;
    RSA_SIGN_MSG_SND_t signSendMsg;
    UINT8              msgHash[SHA256_DIGEST_LENGTH];

    // Create SSL connection for communication with the server
    if (FALSE == openSslConnection(&serverFd, &sslCtx, &ssl))
    {
        printf("fail to create ssl connection\n");
        closeSslConnection(serverFd, sslCtx, ssl);
        return FALSE;
    }

    // Prepare send hash sign message
    prepHashSignRequest(dataHash, &signSendMsg, msgHash);

    // Send hash sign request to server
    if (FALSE == sendHashSignRequest(ssl, &signSendMsg, dataHash, msgHash))
    {
        printf("fail to send hash sign request\n");
        closeSslConnection(serverFd, sslCtx, ssl);
        return FALSE;
    }

    // Receive hash sign response from server
    if (FALSE == recvHashSignResponse(ssl, signedHashData))
    {
        printf("fail to recv hash sign response\n");
        closeSslConnection(serverFd, sslCtx, ssl);
        return FALSE;
    }

    // Close the ssl connection
    closeSslConnection(serverFd, sslCtx, ssl);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Prepare message for hash sign request
 *
 * @param dataHash
 * @param sendMsgInfo
 * @param msgHash
 */
static void prepHashSignRequest(const UINT8 *dataHash, RSA_SIGN_MSG_SND_t *sendMsgInfo, UINT8 *msgHash)
{
    UINT32      msgHashLen = 0;
    EVP_MD_CTX *mdCtx;

    memset(sendMsgInfo, 0, sizeof(RSA_SIGN_MSG_SND_t));
    getLocalIpAddr(sendMsgInfo->localIpAddr);
    memcpy(sendMsgInfo->magicCode, MSG_MAGIC_CODE, MAGIC_CODE_LEN);
    sendMsgInfo->productId = SIGN_PRODUCT_ID;
    sendMsgInfo->KeyType = SIGN_FIRMWARE_TYPE;
    sendMsgInfo->msgId = 1;
    sendMsgInfo->noOfField = 3;
    sendMsgInfo->ipAddrLen = sizeof(sendMsgInfo->localIpAddr);
    sendMsgInfo->keyTypeLen = 2;

    // here 8 bytes are subtracted to remove magic code from total length
    sendMsgInfo->sgnDataLen = SHA256_DIGEST_LENGTH;
    sendMsgInfo->totalMsgLen = (sizeof(RSA_SIGN_MSG_SND_t) + SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH - 8 - sizeof(sendMsgInfo->totalMsgLen));

    mdCtx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdCtx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdCtx, sendMsgInfo, sizeof(RSA_SIGN_MSG_SND_t));
    EVP_DigestUpdate(mdCtx, dataHash, SHA256_DIGEST_LENGTH);
    EVP_DigestFinal_ex(mdCtx, msgHash, &msgHashLen);
    EVP_MD_CTX_free(mdCtx);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Sends a hash signing request to the server.
 *
 * @param ssl
 * @param sendMsgInfo
 * @param dataHash
 * @param msgHash
 * @return BOOL
 */
static BOOL sendHashSignRequest(SSL *ssl, const RSA_SIGN_MSG_SND_t *sendMsgInfo, const UINT8 *dataHash, UINT8 *msgHash)
{
    if (SSL_write(ssl, sendMsgInfo, sizeof(RSA_SIGN_MSG_SND_t)) <= 0)
    {
        printf("fail to send message\n");
        return FALSE;
    }

    if (SSL_write(ssl, dataHash, SHA256_DIGEST_LENGTH) <= 0)
    {
        printf("fail to sent data hash\n");
        return FALSE;
    }

    if (SSL_write(ssl, msgHash, SHA256_DIGEST_LENGTH) <= 0)
    {
        printf("fail to sent msg hash\n");
        return FALSE;
    }

    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Receives a hash signing response from the server.
 *
 * @param ssl
 * @param signedHashData
 * @return BOOL
 */
static BOOL recvHashSignResponse(SSL *ssl, UINT8 *signedHashData)
{
    EVP_MD_CTX         *mdCtx;
    CHAR                magicCode[MAGIC_CODE_LEN];
    CHAR                rcvBuff[PATH_MAX];
    RSA_SIGN_MSG_RCV_t *signRecvMsg;
    UINT32              totalMsgLen, recvLen;
    UINT32              tmpReadLen = 0, msgHashLen = 0;
    UINT8               msgHash[SHA256_DIGEST_LENGTH];

    recvLen = SSL_read(ssl, magicCode, MAGIC_CODE_LEN);
    if (recvLen < 0)
    {
        printf("fail to read magic code\n");
        return FALSE;
    }

    if (memcmp(magicCode, MSG_MAGIC_CODE, MAGIC_CODE_LEN) != 0)
    {
        printf("invalid magic code\n");
        return FALSE;
    }

    recvLen = SSL_read(ssl, &totalMsgLen, sizeof(totalMsgLen));
    if (recvLen < 0)
    {
        printf("fail to read msg length\n");
        return FALSE;
    }

    recvLen = 0;
    memset(rcvBuff, 0, sizeof(rcvBuff));
    while (recvLen < totalMsgLen)
    {
        tmpReadLen = SSL_read(ssl, &rcvBuff[recvLen], (totalMsgLen - recvLen));
        if (tmpReadLen < 0)
        {
            printf("fail to read msg data\n");
            return FALSE;
        }
        recvLen += tmpReadLen;
    }

    mdCtx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdCtx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdCtx, magicCode, sizeof(magicCode));
    EVP_DigestUpdate(mdCtx, &totalMsgLen, sizeof(totalMsgLen));
    EVP_DigestUpdate(mdCtx, rcvBuff, (totalMsgLen - SHA256_DIGEST_LENGTH));
    EVP_DigestFinal_ex(mdCtx, msgHash, &msgHashLen);
    EVP_MD_CTX_free(mdCtx);

    if (memcmp(msgHash, &rcvBuff[totalMsgLen - msgHashLen], msgHashLen) != 0)
    {
        printf("hash comparision failed\n");
        return FALSE;
    }

    signRecvMsg = (RSA_SIGN_MSG_RCV_t *)rcvBuff;
    if (signRecvMsg->msgId != 1)
    {
        printf("invalid message id: [msgId=%d]\n", signRecvMsg->msgId);
        return FALSE;
    }

    switch (signRecvMsg->respStatus)
    {
        case RSA_SIGN_STATUS_SUCCESS:
            memcpy(signedHashData, &rcvBuff[sizeof(RSA_SIGN_MSG_RCV_t)], signRecvMsg->sgnDataLen);
            printf("data signed successfully\n");
            return TRUE;

        case RSA_SIGN_STATUS_PRODUCT_ID_INVALID:
            printf("invalid product id\n");
            return FALSE;

        case RSA_SIGN_STATUS_FIRMWARE_TYPE_INVALID:
            printf("invalid firmware type\n");
            return FALSE;

        case RSA_SIGN_STATUS_SIGNING_ERROR:
            printf("failed in any signing process\n");
            return FALSE;

        default:
            printf("unknown response status: [status=%d]\n", signRecvMsg->respStatus);
            return FALSE;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Prepare the random key file from the obfuscated key (XOR the key with a mask to obfuscate it)
 *
 * @param filename
 * @return BOOL
 */
static BOOL prepRandomKeyFile(const CHAR *filename)
{
    UINT8 byteIdx;
    UINT8 mask = 0x55;
    INT32 fileFd;
    UINT8 randomKey[RANDOM_KEY_LEN];

    // XOR each byte with the mask to retrieve the original key
    for (byteIdx = 0; byteIdx < RANDOM_KEY_LEN; byteIdx++)
    {
        randomKey[byteIdx] = obfuscatedKey[byteIdx] ^ mask;
    }

    fileFd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fileFd == -1)
    {
        printf("fail to open file: [file=%s], [err=%s]\n", filename, strerror(errno));
        return FALSE;
    }

    if (write(fileFd, randomKey, sizeof(randomKey)) != sizeof(randomKey))
    {
        printf("fail to write file: [file=%s], [err=%s]\n", filename, strerror(errno));
        close(fileFd);
        return FALSE;
    }

    close(fileFd);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Append signed data to the firmware file
 *
 * @param filePath
 * @param signedHashData
 * @return BOOL
 */
static BOOL appendSignedHashInFirmwareFile(const CHAR *filePath, const UINT8 *signedHashData)
{
    INT32 sockFd;

    // Open the file in write and append mode
    sockFd = open(filePath, O_WRONLY | O_APPEND | O_CLOEXEC);
    if (sockFd == -1)
    {
        return FALSE;
    }

    // Append the signed hash data to the file
    if (write(sockFd, signedHashData, SIGN_DATA_LEN_MAX) != SIGN_DATA_LEN_MAX)
    {
        close(sockFd);
        return FALSE;
    }

    // Close the file
    close(sockFd);
    return TRUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief Encrypt the firmware file
 *
 * @param filePath
 * @param encFilePath
 * @return BOOL
 */
static BOOL encryptFirmwareFile(const CHAR *filePath, const char *encFilePath)
{
    CHAR sysCmd[512];

    // Get the XORed random key
    if (FALSE == prepRandomKeyFile(RANDOM_KEY_PATH))
    {
        return FALSE;
    }

    // Prepare system command to encrypt the firmware
    snprintf(sysCmd, sizeof(sysCmd), "openssl enc -aes-256-cbc -salt -pbkdf2 -in %s -out %s -pass file:%s", filePath, encFilePath, RANDOM_KEY_PATH);

    // Execute system command to encrypt the firmware
    if (system(sysCmd) != 0)
    {
        // Failed to encrypt the firmware
        printf("fail to encrypt the firmware");
        unlink(RANDOM_KEY_PATH);
        return FALSE;
    }

    // Remove temporary random key file
    unlink(RANDOM_KEY_PATH);

    // Firmware encrypted successfully
    return TRUE;
}

// #################################################################################################
//  @END OF FILE
// #################################################################################################
