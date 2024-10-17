//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   AudioParser.c
@brief  This file is used to get information of audio params (e.g. sampling frequency etc.)
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "Utils.h"
#include "AudioParser.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define MAX_AAC_SAMPLE_FREQ_INDX 16

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const UINT32 aacSampleFreq[MAX_AAC_SAMPLE_FREQ_INDX] =
{
	9600,
	88200,
	64000,
	48000,
	44100,
	32000,
	24000,
	22050,
	16000,
	12000,
	11025,
	8000,
	7350,
	0,
	0,
	0
};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives sampling frequency and AAC header.
 * @param   configStr
 * @param   dataSize
 * @param   audioInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetAACAudioInfo(UINT8PTR configStr, UINT32 dataSize, AAC_AUDIO_INFO_t *audioInfo)
{
    UINT8   audioObjectType;
    UINT8   sampleFreq;
    UINT8   channelConfig;
    UINT8   frameLen = 7;
    UINT8   originalCopy = 0;
    UINT8   home = 0;
    UINT8   copyWriteIdentBit = 0;
    UINT8   copyWriteIdentStart = 0;


    if ((configStr == NULL) || (dataSize < 2) || (audioInfo == NULL))
	{
        EPRINT(UTILS, "no valid input parameter");
        return FAIL;
    }

    audioObjectType = ((configStr[0] & 0xf8) >> 3);
    sampleFreq = (((configStr[0] & 0x07) << 1) | (configStr[1] & 0x80) >> 7);
    channelConfig = ((configStr[1] & 0x78) >> 3);

    audioInfo->samplingFreq = aacSampleFreq[sampleFreq];
    audioInfo->aacHeaderSize = 0;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] = 0xff;
    audioInfo->aacHeaderSize++;
    audioInfo->aacHeader[audioInfo->aacHeaderSize ] = 0xf1;
    audioInfo->aacHeaderSize++;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] = ((audioObjectType - 1) << 6);
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((sampleFreq & 0x0F) << 2);
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((channelConfig & 0x04) >> 2);
    audioInfo->aacHeaderSize++;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] = ((channelConfig & 0x03) << 6);
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((originalCopy & 0x01) << 5);
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((home & 0x01) << 4);
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((copyWriteIdentBit & 0x01) << 3);
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((copyWriteIdentStart & 0x01) << 2);
    //! frame size over last 2 bits
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= ((frameLen & 0x1800) >> 11);
    audioInfo->aacHeaderSize++;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] = ((frameLen & 0x07f8) >> 3);
    audioInfo->aacHeaderSize++;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] = (frameLen & 0x0007) << 5;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] |= 0x1F;
    audioInfo->aacHeaderSize++;
    audioInfo->aacHeader[audioInfo->aacHeaderSize] = 0xF8;
    audioInfo->aacHeaderSize++;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function Apends frame length int AAC header of given buffer
 * @param   data
 * @param   len
 * @return  SUCCESS/FAIL
 */
BOOL ApendAACFrameLen(UINT8PTR data, UINT32 len)
{
    if (data == NULL)
	{
        return FAIL;
    }

    *(data + 3) |= ((len & 0x1800) >> 11);
    *(data + 4) |= ((len & 0x1FF8) >> 3);
    *(data + 5) |= ((len & 0x07) << 5);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gives configuration of AAC decoder from its frame header of ADTS.
 * @param   hdrStr
 * @param   dataSize
 * @param   configData
 * @return  SUCCESS/FAIL
 */
BOOL GetAACAudioConfig(UINT8PTR hdrStr, UINT32 dataSize, UINT32PTR configData)
{
	UINT8	audioObjectType = 0;
	UINT8	sampleFreq = 0;
	UINT8	channelCfg = 0;
	UINT8	frameLengthType = 0;
	UINT8	dependsOnCOreCoder = 0;
	UINT8	extensionFlag = 0;

    if ((hdrStr == NULL) || (dataSize < 7) || (configData == NULL))
	{
        return FAIL;
    }

    audioObjectType = (((hdrStr[2] & 0xC0) >> 6) + 1);
    sampleFreq = ((hdrStr[2] & 0x3C) >> 2 );
    channelCfg = ((hdrStr[2] & 0x01) << 2) | ((hdrStr[3] & 0xC0) >> 6);
    frameLengthType = 0;
    dependsOnCOreCoder = 0;
    if((audioObjectType == 1) || (audioObjectType == 2) || (audioObjectType == 3) || (audioObjectType == 4) || (audioObjectType == 6) || (audioObjectType == 7))
    {
        extensionFlag = 0;
    }

    *configData = 0;
    *configData = ((UINT16)audioObjectType << 11);
    *configData |= ((UINT16)sampleFreq << 7);
    *configData |= (channelCfg << 3);
    *configData |= (frameLengthType << 2);
    *configData |= (dependsOnCOreCoder << 1);
    *configData |= extensionFlag;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   GetAACSamplingFreq
 * @param   data
 * @return  Sampling frequency
 */
UINT32 GetAACSamplingFreq(UINT8PTR data)
{
	UINT8 sampleFreq = 0;

	if(data != NULL)
	{
        sampleFreq = ((data[2] & 0x3C) >> 2);
		if(sampleFreq >= MAX_AAC_SAMPLE_FREQ_INDX)
		{
			sampleFreq = 0;
        }
	}

	return aacSampleFreq[sampleFreq];
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
