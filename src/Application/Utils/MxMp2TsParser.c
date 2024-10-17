//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file   MxMp2TsParser.c
@brief  This module is used to parse Mpeg2Ts Packet Stream and Provide Audio and Video Frames from
        Mpeg2Ts Packets.
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "MxMp2TsParser.h"
#include "DebugLog.h"
#include "Utils.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define STREAM_TYPE_VIDEO_MPEG1     	0x01
#define STREAM_TYPE_VIDEO_MPEG2     	0x02
#define STREAM_TYPE_AUDIO_MPEG1     	0x03
#define STREAM_TYPE_AUDIO_MPEG2     	0x04
#define STREAM_TYPE_PRIVATE_SECTION 	0x05
#define STREAM_TYPE_PRIVATE_DATA    	0x06
#define STREAM_TYPE_AUDIO_AAC       	0x0f
#define STREAM_TYPE_VIDEO_MJPEG	    	0xA0
#define STREAM_TYPE_VIDEO_MPEG4     	0x10
#define STREAM_TYPE_VIDEO_H264      	0x1b
#define STREAM_TYPE_VIDEO_VC1       	0xea
#define STREAM_TYPE_VIDEO_DIRAC     	0xd1
#define STREAM_TYPE_AUDIO_AC3       	0x81
#define STREAM_TYPE_AUDIO_DTS       	0x8a
#define STREAM_TYPE_AUDIO_HDMV_DTS  	0x82
#define STREAM_TYPE_SUBTITLE_DVB    	0x100

#define CONVERT16BITNUM(a,b) 			((a<<8) + b)
#define CRC_BYTE_SIZE 			 		4
#define MAX_MP2_PACKET_LEN 				188
#define INVALID_PID_NUM					0xFFFF

//FOR PES HEADER
#define PES_PACKET_LENGTH_INDEX  		4
#define PES_STREAM_ID_INDEX				3
#define PES_HEADER_DATA_LENGTH_INDEX  	8

//FOR PMT PACKET
#define PMT_SECTION_LENGTH_INDEX 		1
#define PMT_PROG_NUM_INDEX 		 		3
#define PMT_PROG_INFO_LEN_INDEX	 		10
#define PMT_STRM_TYPE_RELATIVE_INDEX	2
#define PMT_PID_NUM_RELATIVE_INDEX		3

//FOR PAT PACKET
#define PAT_PID_NUM						0x0000
#define PAT_SECTION_LEN_INDEX 			1
#define PAT_PROG_NUM_INDEX 				8

//FOR TES HEADER
#define TES_HEADER_START_CODE_CHECK	    1
#define TES_ADAP_FIELD_CNTRL_INDX		3
#define TES_HEADER_LEN					4

//PACKET IDENTIFICATION CODES
#define START_PACKET_CODE				0x40
#define ADAPT_CNTRL_CODE				0x20
#define VIDEO_FRAME_TYPE_PID			0x1B
#define AUDIO_FRAME_TYPE_PID			0x0F

//STREAM IDENTIFICATION CODE
#define AUDIO_STREAM_ID 				0xC0
#define VIDEO_STREAM_ID 				0xE0

//#################################################################################################
// @DATA TYPES
//#################################################################################################
//AVAILABLE PID TYPES IN PACKET
typedef enum
{
	PAT_PID_TYPE = 0,
	PMT_PID_TYPE,
	VIDEO_PID_TYPE,
	AUDIO_PID_TYPE,
	MAX_PID_TYPE

}PID_TYPE_e;

//PACKET TYPE
typedef enum
{
    MP2_VIDEO_TYPE = 0,
	MP2_AUDIO_TYPE,
	MP2_MAX_FRAME_TYPE,

}MP2_FRAME_TYPE_e;

//PARSER RELATED STRUCTURE
typedef struct
{
	BOOL 				status;
	UINT32				programmeNum;
	UINT32 				pidNum[MAX_PID_TYPE];
	PID_TYPE_e			currPidType;
	UINT32 				packetLen[MAX_STREAM_TYPE];
	BOOL 				frameAvailable;
	UINT8PTR			framePtr[MAX_STREAM_TYPE];
	UINT32 				writeOffSet[MAX_STREAM_TYPE];
	STREAM_CODEC_TYPE_e	codecType[MAX_STREAM_TYPE];
	STREAM_TYPE_e		currStreamType;
	UINT8				prevPacketBuff[MAX_MP2_PACKET_LEN];
	UINT8				prevPacketLen;
	BOOL 				startCodeFnd[MAX_STREAM_TYPE];
	BOOL 				useSamePacket;
	UINT32				nextPacketIndex;

}MP2_PROGRAM_INFO_t;

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL parsePESPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data, UINT32PTR parsedBytes);
//-------------------------------------------------------------------------------------------------
static BOOL parsePMTPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data);
//-------------------------------------------------------------------------------------------------
static void parsePATPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data);
//-------------------------------------------------------------------------------------------------
static BOOL findMp2StartHeader(UINT8PTR data, UINT32PTR index, UINT32 remainingLen);
//-------------------------------------------------------------------------------------------------
static BOOL mp2PacketParser(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data);
//-------------------------------------------------------------------------------------------------
static UINT8PTR getStartPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR *data, UINT32PTR remainingData);
//-------------------------------------------------------------------------------------------------
static BOOL getPIDType(MP2_PROGRAM_INFO_t *progInfo ,UINT8PTR data);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static MP2_PROGRAM_INFO_t mp2ProgramInfo[MAX_MP2_SESSION];

//PARSING REQUEST MUTEX
static pthread_mutex_t    mp2SessionReqMutx;

//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used to initialize sessions for mpeg2Ts parser structures.
 */
void InitMp2TsClient(void)
{
	MP2_TS_SESSION sessionIndex;

	for(sessionIndex = 0; sessionIndex < MAX_MP2_SESSION; sessionIndex++)
	{
        mp2ProgramInfo[sessionIndex].status = FREE;
        mp2ProgramInfo[sessionIndex].pidNum[PAT_PID_TYPE] = PAT_PID_NUM;
        mp2ProgramInfo[sessionIndex].pidNum[PMT_PID_TYPE] = INVALID_PID_NUM;
		mp2ProgramInfo[sessionIndex].pidNum[VIDEO_PID_TYPE] = INVALID_PID_NUM;
		mp2ProgramInfo[sessionIndex].pidNum[AUDIO_PID_TYPE] = INVALID_PID_NUM;
	}
    MUTEX_INIT(mp2SessionReqMutx, NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used to provide session handle which is further used to parse Mpeg2Ts stream.
 * @param   sessionIndex
 * @return  SUCCESS / FAIL
 */
BOOL StartMp2TsParser(MP2_TS_SESSION *sessionIndex)
{
	UINT8			cnt;
	MP2_TS_SESSION 	session;

    if(sessionIndex == NULL)
	{
        return FAIL;
    }

    *sessionIndex = INVALID_MP2_SESSION;
    MUTEX_LOCK(mp2SessionReqMutx);
    for(session = 0; session < MAX_MP2_SESSION; session++)
    {
        if(mp2ProgramInfo[session].status == FREE)
        {
            mp2ProgramInfo[session].status = BUSY;
            break;
        }
    }
    MUTEX_UNLOCK(mp2SessionReqMutx);

    if(session >= MAX_MP2_SESSION)
    {
        EPRINT(MP2_TS_PARSER_CLIENT, "no session available for parsing");
        return FAIL;
    }

    mp2ProgramInfo[session].currStreamType = MAX_STREAM_TYPE;
    for(cnt = 0; cnt < MAX_STREAM_TYPE; cnt ++)
    {
        //Frame pointer for both stream
        mp2ProgramInfo[session].framePtr[cnt] = NULL;

        //frame write offset for both stream
        mp2ProgramInfo[session].writeOffSet[cnt] = 0;

        //Keep track whether current stream packet is initiated or not
        mp2ProgramInfo[session].startCodeFnd[cnt] = FALSE;
    }

    mp2ProgramInfo[session].useSamePacket = NO;
    mp2ProgramInfo[session].nextPacketIndex = MAX_MP2_PACKET_LEN;
    mp2ProgramInfo[session].prevPacketLen = 0;
    memset(mp2ProgramInfo[session].prevPacketBuff, 0 , MAX_MP2_PACKET_LEN);

    //Assign session to given session index
    *sessionIndex = session;
    DPRINT(MP2_TS_PARSER_CLIENT, "start mpeg2ts parser: [session=%d]", *sessionIndex);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This API is used to parse Mp2Ts Stream from Provided Data and its data length. It finds
 *          out Mp2Ts Packets from given data and retrieve stream from parsed Mp2ts packets.
 * @param   httpHandle
 * @param   mp2ClientInfo
 * @param   frameCallback
 * @return  SUCCESS / FAIL
 */
BOOL ParseMp2TsData(HTTP_HANDLE httpHandle, MP2_CLIENT_INFO_t *mp2ClientInfo, MP2_TS_CALLBACK frameCallback)
{
	BOOL 	 		status = SUCCESS;
    UINT8PTR 		packetStartPtr; // Packet address which is further used to parse data
	STREAM_TYPE_e 	streamType;

	do
	{
        if(mp2ClientInfo == NULL)
        {
            break;
        }

        if((mp2ClientInfo->session >= MAX_MP2_SESSION) || (mp2ClientInfo->data == NULL))
		{
			//Error condition
			break;
		}

        mp2ProgramInfo[mp2ClientInfo->session].frameAvailable = NO;

        //Get the starting of packet, if packet is not available then return from function.
		if(mp2ClientInfo->dataLen == 0)
		{
			//No More Packets are available to parse, return from function
			break;
		}

		//Get Packet for further parsing
        packetStartPtr = getStartPacket(&mp2ProgramInfo[mp2ClientInfo->session], &mp2ClientInfo->data, &mp2ClientInfo->dataLen);
		if(packetStartPtr == NULL)
		{
			//No full packet available to parse
			break;
		}

		//Parse available packet
		mp2ProgramInfo[mp2ClientInfo->session].useSamePacket = NO;
        status = mp2PacketParser(&mp2ProgramInfo[mp2ClientInfo->session], packetStartPtr);
        if(status == FAIL)
		{
            EPRINT(MP2_TS_PARSER_CLIENT, "fail to parse mpeg2ts packet: [session=%d]", mp2ClientInfo->session);
			break;
		}

		if(mp2ProgramInfo[mp2ClientInfo->session].useSamePacket == NO)
		{
			//Increment data to its next index for further packet parsing
            mp2ClientInfo->data = mp2ClientInfo->data + mp2ProgramInfo[mp2ClientInfo->session].nextPacketIndex;
            mp2ClientInfo->dataLen = mp2ClientInfo->dataLen - mp2ProgramInfo[mp2ClientInfo->session].nextPacketIndex;
		}

		//If Frame available then break the loop and send it to calling module
		if(mp2ProgramInfo[mp2ClientInfo->session].frameAvailable == YES)
		{
			streamType = mp2ProgramInfo[mp2ClientInfo->session].currStreamType;
            mp2ClientInfo->mp2DataInfo.streamType = streamType;
            mp2ClientInfo->mp2DataInfo.framePtr = (mp2ProgramInfo[mp2ClientInfo->session].framePtr[streamType]);
            mp2ClientInfo->mp2DataInfo.frameSize = mp2ProgramInfo[mp2ClientInfo->session].writeOffSet[streamType];
            mp2ClientInfo->mp2DataInfo.codecType = mp2ProgramInfo[mp2ClientInfo->session].codecType[streamType];
			frameCallback(httpHandle, mp2ClientInfo);

			// remove this memory, no longer available
			if(mp2ProgramInfo[mp2ClientInfo->session].framePtr[streamType] != NULL)
			{
				free(mp2ProgramInfo[mp2ClientInfo->session].framePtr[streamType]);
			}

			//Initialize write offset and its stream type
			mp2ProgramInfo[mp2ClientInfo->session].framePtr[streamType] = NULL;
			mp2ProgramInfo[mp2ClientInfo->session].writeOffSet[streamType] = 0;
			mp2ProgramInfo[mp2ClientInfo->session].currStreamType = MAX_STREAM_TYPE;
			break;
		}

    }while(TRUE);//loop breaks only when no packet available for parsing

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function is used to parse Mp2Ts packets. Based on Packet Information it constructs
 *          frames. It also provides the information about whether full packet is parsed or not
 * @param   progInfo
 * @param   data
 * @return  SUCCESS / FAIL
 */
static BOOL mp2PacketParser(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data)
{
	BOOL			startCodeFound = FALSE;
	UINT32  		pesHeaderLen;
	UINT8 			skipData = 0;
	UINT32			remainingData;
	BOOL 			status = SUCCESS;
	STREAM_TYPE_e 	prevStreamType;

	do
	{
		//Detect whether it is a start packet or not.
        if((data[TES_HEADER_START_CODE_CHECK] & START_PACKET_CODE) == START_PACKET_CODE)
		{
			startCodeFound = TRUE;
		}

        //Make next data index further to its 4 bytes of header and adaptation field length (if it is available).
        if(((data[TES_ADAP_FIELD_CNTRL_INDX] & ADAPT_CNTRL_CODE)) == ADAPT_CNTRL_CODE)
		{
			skipData = data[TES_ADAP_FIELD_CNTRL_INDX + 1] + 1;
		}

		data = data + TES_HEADER_LEN + skipData;
        remainingData = (MAX_MP2_PACKET_LEN - (TES_HEADER_LEN + skipData));

        //Now parse packet payload according to its packet type detected in their header.
		if(progInfo->currPidType == MAX_PID_TYPE)
		{
            EPRINT(MP2_TS_PARSER_CLIENT, "invld pid type of packet");
			break;
		}

		if(progInfo->currPidType == PAT_PID_TYPE)
		{
			//Skip 0x00 stuffing byte
			data = data + 1;
			parsePATPacket(progInfo, data);

			//if it is using previous packet
			progInfo->prevPacketLen = 0;
			break;
		}

		if(progInfo->currPidType == PMT_PID_TYPE)
		{
			//Skip 0x00 stuffing bytes
			data = data + 1;
			status = parsePMTPacket(progInfo, data);

			//if it is using previous packet
			progInfo->prevPacketLen = 0;
			break;
		}

        /* If Packet is VIDEO/AUDIO type, first check whether packet is start packet or not. If it is start of packet
         * then first extract its PES header information, from its payload data. And payload data other than PES Header,
         * save as its frame data. If packet is not start packet then its payload represents frame data only and save it. */
		if((progInfo->currPidType == AUDIO_PID_TYPE) || (progInfo->currPidType == VIDEO_PID_TYPE))
		{
			//Retrieve previous stream type
			prevStreamType = progInfo->currStreamType;

			//Now update current stream type
			if(progInfo->currPidType == AUDIO_PID_TYPE)
			{
				progInfo->currStreamType = STREAM_TYPE_AUDIO;
			}
			else
			{
				progInfo->currStreamType = STREAM_TYPE_VIDEO;
			}

			if(startCodeFound == TRUE)
			{
                /* If current stream got start packet earlier and whether its packet len is not available or
                 * its next start packet come earlier before finishing the packet then send its previous frame */
				if(prevStreamType != MAX_STREAM_TYPE)
				{
                    if((progInfo->packetLen[prevStreamType] == 0) || progInfo->startCodeFnd[progInfo->currStreamType] == TRUE)
					{
						progInfo->startCodeFnd[prevStreamType] = FALSE;
						progInfo->frameAvailable = YES;

						//Send Previous frame
						progInfo->currStreamType = prevStreamType;

						//Use same packet for next time parsing
						progInfo->useSamePacket = YES;
						break;
					}
				}

				//Extract PES header length
				pesHeaderLen  = 0;
				if(parsePESPacket(progInfo, data, &pesHeaderLen) == FAIL)
				{
					progInfo->currStreamType = prevStreamType;
					break; //Break function
				}

				//Start Code Found For That Stream
				progInfo->startCodeFnd[progInfo->currStreamType] = TRUE;
				data = data + pesHeaderLen;
				remainingData = remainingData - pesHeaderLen;
			}

			//If packet was of previous buffer then make its length Zero
			progInfo->prevPacketLen = 0;
			if(progInfo->startCodeFnd[progInfo->currStreamType] == FALSE)
			{
                EPRINT(MP2_TS_PARSER_CLIENT, "wrong stream packet: [streamType=%d]", progInfo->currStreamType);
				break;
			}

			if((progInfo->framePtr[progInfo->currStreamType]) == NULL)
			{
				if(remainingData > 0)
				{
					if(progInfo->packetLen[progInfo->currStreamType] > 0)
					{
						//malloc data accroding to available packet len
                        progInfo->framePtr[progInfo->currStreamType] = malloc(progInfo->packetLen[progInfo->currStreamType]);
					}
					else
					{
                        progInfo->framePtr[progInfo->currStreamType] = malloc(remainingData);
					}
				}
				else
				{
                    EPRINT(MP2_TS_PARSER_CLIENT, "remaining data length is 0");
					break;
				}
			}
			else
			{
                if((remainingData + progInfo->writeOffSet[progInfo->currStreamType]) > progInfo->packetLen[progInfo->currStreamType])
				{
                    progInfo->framePtr[progInfo->currStreamType] = realloc(progInfo->framePtr[progInfo->currStreamType],
                            remainingData + progInfo->writeOffSet[progInfo->currStreamType]);
				}
			}

			if((progInfo->framePtr[progInfo->currStreamType]) != NULL)
			{
                /* PARASOFT : Rule CERT_C-STR31-b, 	MISRAC2012-RULE_21_18-a - Ignoring flow outside the function call */
				memcpy(progInfo->framePtr[progInfo->currStreamType] + progInfo->writeOffSet[progInfo->currStreamType], data, remainingData);
				progInfo->writeOffSet[progInfo->currStreamType] += remainingData;
			}
			else
			{
				//failed to malloc / realloc data
				status = FAIL;
                EPRINT(MP2_TS_PARSER_CLIENT, "null frame ptr found");
				break;
			}

            //If frame packet length is available in PES header then, frame is sent when that much of frame data is written to frame pointer.
			if(progInfo->packetLen[progInfo->currStreamType] > 0)
			{
                if(progInfo->writeOffSet[progInfo->currStreamType] >= progInfo->packetLen[progInfo->currStreamType])
				{
					progInfo->startCodeFnd[progInfo->currStreamType] = FALSE;
					progInfo->frameAvailable = YES;
				}
			}
		}

	}while(0);

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function retrives information about stream frame size and PES packet
 * @param   progInfo
 * @param   data
 * @param   parsedBytes
 * @return  SUCCESS / FAIL
 */
static BOOL parsePESPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data, UINT32PTR parsedBytes)
{
	UINT32 	pesPacketLen;

	//chack start bit of PES packet
    if((data[0] != 0x00) || (data[1] != 0x00) || (data[2] != 0x01))
	{
        return FAIL;
    }

    pesPacketLen = CONVERT16BITNUM(data[PES_PACKET_LENGTH_INDEX], data[PES_PACKET_LENGTH_INDEX + 1]);
    progInfo->packetLen[progInfo->currStreamType] = 0;
    if(pesPacketLen > 0)
    {
        // 3 - 2 bytes for PES PACKET LEN + 1 byte for pes header data len
        progInfo->packetLen[progInfo->currStreamType] = pesPacketLen - (data[PES_HEADER_DATA_LENGTH_INDEX] + 3);
    }

    *parsedBytes = PES_HEADER_DATA_LENGTH_INDEX + data[PES_HEADER_DATA_LENGTH_INDEX] + 1;
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function retrives streams associated with specified programme(detected in PAT packet).
 *          Save its streaming information and its streams' PID numbers. These PID numbers are further
 *          used to identify those detected streams' packets. In PMT packet we have to detect perticular
 *          section, in that section it provides that streaming informations.
 * @param   progInfo
 * @param   data
 * @return  SUCCESS / FAIL
 */
static BOOL parsePMTPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data)
{
	INT32 		sectionLen;
	UINT32		programmeNum;
	UINT32 	   	programmeInfoLen;
	UINT8PTR   	sectionStartPtr;
	PID_TYPE_e 	pidType;
	BOOL		status = FAIL;

    sectionLen = CONVERT16BITNUM((data[PMT_SECTION_LENGTH_INDEX] & 0x0F), data[PMT_SECTION_LENGTH_INDEX + 1]);
    programmeNum = CONVERT16BITNUM(data[PMT_PROG_NUM_INDEX], data[PMT_PROG_NUM_INDEX + 1]);
    if(programmeNum != progInfo->programmeNum)
	{
        return FAIL;
    }

    data = data + PMT_PROG_INFO_LEN_INDEX;
    sectionLen = sectionLen - (PMT_PROG_INFO_LEN_INDEX - PMT_PROG_NUM_INDEX + CRC_BYTE_SIZE);
    sectionStartPtr = data ;
    do
    {
        programmeInfoLen = CONVERT16BITNUM((data[0] & 0x0F), data[1]);
        pidType = MAX_PID_TYPE;
        if(data[programmeInfoLen + PMT_STRM_TYPE_RELATIVE_INDEX] == STREAM_TYPE_VIDEO_MPEG4)
        {
            progInfo->codecType[STREAM_TYPE_VIDEO] = VIDEO_MPEG4;
            pidType = VIDEO_PID_TYPE;
        }
        else if(data[programmeInfoLen + PMT_STRM_TYPE_RELATIVE_INDEX] == STREAM_TYPE_VIDEO_H264)
        {
            progInfo->codecType[STREAM_TYPE_VIDEO] = VIDEO_H264;
            pidType = VIDEO_PID_TYPE;
        }
        else if(data[programmeInfoLen + PMT_STRM_TYPE_RELATIVE_INDEX] == STREAM_TYPE_VIDEO_MJPEG)
        {
            progInfo->codecType[STREAM_TYPE_VIDEO] = VIDEO_MJPG;
            pidType = VIDEO_PID_TYPE;
        }
        else if(data[programmeInfoLen + PMT_STRM_TYPE_RELATIVE_INDEX] == STREAM_TYPE_AUDIO_AAC)
        {
            progInfo->codecType[STREAM_TYPE_AUDIO] = AUDIO_AAC;
            pidType = AUDIO_PID_TYPE;
        }

        if(pidType != MAX_PID_TYPE)
        {
            progInfo ->pidNum[pidType] = CONVERT16BITNUM((data[programmeInfoLen + PMT_PID_NUM_RELATIVE_INDEX] & 0x1F),
                    data[programmeInfoLen + PMT_PID_NUM_RELATIVE_INDEX + 1] );
            status = SUCCESS;
        }

        //2 Bytes for programme information length
        data = data + programmeInfoLen + PMT_PID_NUM_RELATIVE_INDEX + 2;

    }while((data - sectionStartPtr) < sectionLen);  //Loop until section doesnt complete.

	return status;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Retrieve section length of PAT packet, in this specified field we have to retrieve
 *          information about available programme number which contains its streaming information
 *          and its pid number which is further used to identify PMT packet.
 * @param   progInfo
 * @param   data
 */
static void parsePATPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR data)
{
    INT32       sectionLen;
	UINT8PTR	startSectionPtr;

    sectionLen = CONVERT16BITNUM((data[PAT_SECTION_LEN_INDEX] & 0x0F), data[PAT_SECTION_LEN_INDEX + 1]);
	data = data + PAT_PROG_NUM_INDEX;
    sectionLen = sectionLen -(PAT_PROG_NUM_INDEX - (PAT_SECTION_LEN_INDEX + 2) + CRC_BYTE_SIZE);
	startSectionPtr = data;

	do
	{
		progInfo->programmeNum = CONVERT16BITNUM(data[0], data[1]);

		//Need only programme number greater than 0 other wise discard it.
		if(progInfo->programmeNum > 0)
		{
            progInfo->pidNum[PMT_PID_TYPE] = CONVERT16BITNUM((data[2] & 0x1F), data[3]);
			break;
		}
		//Make next index to programme num(2 bytes) + pidnum(2 bytes)
		data = data + 4;

	}while((data - startSectionPtr) < sectionLen);
	//Loop until PAT section completes.
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   finds the index of start packet for MP2TS packet.
 * @param   data
 * @param   index
 * @param   remainingLen
 * @return  SUCCESS / FAIL
 */
static BOOL findMp2StartHeader(UINT8PTR data, UINT32PTR index, UINT32 remainingLen)
{
	*index = 0;
    if(data == NULL)
    {
        EPRINT(MP2_TS_PARSER_CLIENT, "data ptr is null");
        return FAIL;
    }

	while(*index < remainingLen)
	{
        if((data[*index]) == 0x47)
        {
            return SUCCESS;
        }
        *index = *index + 1;
	}

    return FAIL;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   stop perticular MP2Ts parser session.
 * @param   session
 * @return  SUCCESS / FAIL
 */
BOOL StopMp2TsParser(MP2_TS_SESSION session)
{
    UINT8		cnt;
	PID_TYPE_e 	pidType;

	if(session >= MAX_MP2_SESSION)
	{
        EPRINT(MP2_TS_PARSER_CLIENT, "invld session found");
        return FAIL;
	}

    MUTEX_LOCK(mp2SessionReqMutx);
    if(mp2ProgramInfo[session].status == FREE)
    {
        MUTEX_UNLOCK(mp2SessionReqMutx);
        EPRINT(MP2_TS_PARSER_CLIENT, "session is already free: [session=%d]", session);
        return FAIL;
    }

    mp2ProgramInfo[session].pidNum[PAT_PID_TYPE] = PAT_PID_NUM;
    for(pidType = PMT_PID_TYPE; pidType < MAX_PID_TYPE; pidType++)
    {
        mp2ProgramInfo[session].pidNum[pidType] = INVALID_PID_NUM;
    }

    for(cnt = STREAM_TYPE_VIDEO; cnt < MAX_STREAM_TYPE; cnt++)
    {
        FREE_MEMORY(mp2ProgramInfo[session].framePtr[cnt]);
    }

    mp2ProgramInfo[session].status = FREE;
    MUTEX_UNLOCK(mp2SessionReqMutx);
    DPRINT(MP2_TS_PARSER_CLIENT, "session stopped successfully: [session=%d]", session);
    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   This Function find out the start packet for mpeg2Ts. If available packet is less than
 *          required packet length then it saves it on previous packet buffer. If previous packet
 *          is available then it provides previous packet as a start packet first.
 * @param   progInfo
 * @param   data
 * @param   remainingData
 * @return  SUCCESS / FAIL
 */
static UINT8PTR getStartPacket(MP2_PROGRAM_INFO_t *progInfo, UINT8PTR *data, UINT32PTR remainingData)
{
	UINT32 	 	incmpPktLen = 0;
	UINT32 	 	index;
	UINT8PTR 	startPacketPtr = NULL;

    //IF PREVIOUS PACKET AVAILABLE THEN FILL THAT PREVIOUS BUFFER
    if(progInfo->prevPacketLen > 0)
    {
        //Condition occurs when same packet is used for parsing again, which is previously buffered.
        if(progInfo->prevPacketLen == MAX_MP2_PACKET_LEN)
        {
            startPacketPtr = progInfo->prevPacketBuff;
            progInfo->prevPacketLen = 0;
            return startPacketPtr;
        }

        //If remaining data is still incomplete then
        if((progInfo->prevPacketLen + *remainingData) < MAX_MP2_PACKET_LEN)
        {
            incmpPktLen = *remainingData;
        }
        else
        {
            //Else provide ptr of previous packet buffer for further parsing
            incmpPktLen = (MAX_MP2_PACKET_LEN - progInfo->prevPacketLen);
            startPacketPtr = progInfo->prevPacketBuff;
        }

        //copy data to previous packet buffer
        memcpy((progInfo->prevPacketBuff + progInfo->prevPacketLen), *data, incmpPktLen);
        progInfo->prevPacketLen = (progInfo->prevPacketLen + incmpPktLen);
        progInfo->nextPacketIndex = incmpPktLen;
        return startPacketPtr;
    }

    //Find the correct start of packet
    do
    {
        if(findMp2StartHeader(*data, &index, *remainingData) == FAIL)
        {
            break;
        }

        *data = *data + index;
        *remainingData = *remainingData - index;

        //To Check Whether Packet is right or not. If Packet is wrong then increment data index by one and find new Packet.
        if((getPIDType(progInfo, *data)) == SUCCESS)
        {
            //If Remaining data is greater then maximum packet length then provide data ptr for parsing Else Save data to previous packet buffer
            if(*remainingData >= MAX_MP2_PACKET_LEN)
            {
                startPacketPtr = *data;
                progInfo->nextPacketIndex 	= MAX_MP2_PACKET_LEN;
            }
            else
            {
                memcpy(progInfo->prevPacketBuff, *data, *remainingData);
                progInfo->prevPacketLen = *remainingData;
            }
            break;
        }
        else
        {
            *data = *data + 1;
            *remainingData = *remainingData - 1;
        }

    }while(TRUE);

	return startPacketPtr;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   From packet header it finds out packet pid type.
 * @param   progInfo
 * @param   data
 * @return  SUCCESS / FAIL
 */
static BOOL getPIDType(MP2_PROGRAM_INFO_t *progInfo ,UINT8PTR data)
{
    UINT32  pidNum;

	progInfo->currPidType = PAT_PID_TYPE;

	//FIND CURRENT PACKETS PID NUMBER
    pidNum = CONVERT16BITNUM((data[TES_HEADER_START_CODE_CHECK] & 0x1F), data[TES_HEADER_START_CODE_CHECK + 1]);

	//FIND PID TYPE FROM GIVEN PID NUMBER
	do
	{
		if(pidNum == progInfo->pidNum[progInfo->currPidType])
		{
            return SUCCESS;
		}
		progInfo->currPidType++;

	}while(progInfo->currPidType < MAX_PID_TYPE);

    return FAIL;
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
