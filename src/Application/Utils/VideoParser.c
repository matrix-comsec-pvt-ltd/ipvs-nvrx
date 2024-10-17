//#################################################################################################
// FILE BRIEF
//#################################################################################################
/**
@file		VideoParser.c
@brief      Video codec header paser (e.g NAL header parser etc.)
*/
//#################################################################################################
// @INCLUDES
//#################################################################################################
/* Application Includes */
#include "DebugLog.h"
#include "VideoParser.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
// Coded slice of a non-IDR picture
#define NAL_UNIT_TYPE_CODED_SLICE_NON_IDR       (1)

// Coded slice of an IDR picture
#define NAL_UNIT_TYPE_CODED_SLICE_IDR           (5)

// Sequence parameter set
#define NAL_UNIT_TYPE_SPS                       (7)

// Picture parameter set
#define NAL_UNIT_TYPE_PPS                       (8)
#define MAX_REF_FRAME	                        (15)

#define H265_MAX_SUB_LAYERS                     7

/* Start code length */
#define MPEG4_START_CODE_LEN                    3
#define H264_H265_START_CODE_LEN                4

/* EPB will be there immediately after two consecutive 0x00 bytes in NAL header data 0x0000XX --> 0x000003XX */
#define EMULATION_PREVENTION_BYTE               0x03

//#################################################################################################
// @DATA TYPES
//#################################################################################################
typedef struct
{
	UINT8PTR 	start;
	UINT8PTR 	priv;
	UINT8PTR 	end;
	UINT32	 	bitsLeft;
    BOOL        isEpbIdEnabled; /* Emulation prevention byte identification status (Enable or Disable) */
    UINT8	 	epbIdCnt;       /* EPB is start code sequence breaker byte for NAL header data */

} BIT_STREAM_t;

/* NAL header structure */
typedef struct
{
    INT32 		forbiddenZeroBit;
    INT32 		nalRefIdc;
    INT32 		nalUnitType;

} NAL_t;

/* SPS information as per iso standard but not fully implemented. Only portion that we required to know resolution. */
typedef struct
{
    UINT32 profile_idc;
    UINT32 constraint_set0_flag;
    UINT32 constraint_set1_flag;
    UINT32 constraint_set2_flag;
    UINT32 constraint_set3_flag;
    UINT32 reserved_zero_4bits;
    UINT32 level_idc;
    UINT32 seq_parameter_set_id;
    UINT32 chroma_format_idc;
    UINT32 residual_colour_transform_flag;
    UINT32 bit_depth_luma_minus8;
    UINT32 bit_depth_chroma_minus8;
    UINT32 qpprime_y_zero_transform_bypass_flag;
    UINT32 seq_scaling_matrix_present_flag;
    UINT32 seq_scaling_list_present_flag[12];
    INT32  ScalingList4x4[6][16];
    UINT32 UseDefaultScalingMatrix4x4Flag[6];
    INT32  ScalingList8x8[2][64];
    UINT32 UseDefaultScalingMatrix8x8Flag[2];
    UINT32 log2_max_frame_num_minus4;
    UINT32 pic_order_cnt_type;
    UINT32 log2_max_pic_order_cnt_lsb_minus4;
    UINT32 delta_pic_order_always_zero_flag;
    INT32  offset_for_non_ref_pic;
    INT32  offset_for_top_to_bottom_field;
    UINT32 num_ref_frames_in_pic_order_cnt_cycle;
    INT32  offset_for_ref_frame[256];
    UINT32 num_ref_frames;
    UINT32 gaps_in_frame_num_value_allowed_flag;
    UINT32 pic_width_in_mbs_minus1;
    UINT32 pic_height_in_map_units_minus1;
    UINT32 frame_mbs_only_flag;
    UINT32 mb_adaptive_frame_field_flag;
    UINT32 direct_8x8_inference_flag;
    UINT32 frame_cropping_flag;
    UINT32 frame_crop_left_offset;
    UINT32 frame_crop_right_offset;
    UINT32 frame_crop_top_offset;
    UINT32 frame_crop_bottom_offset;
    UINT32 vui_parameters_present_flag;

} H264_SPS_t;

typedef struct
{
#if 0 /* Unused */
    UINT8 profile_space;
    UINT8 tier_flag;
    UINT8 profile_idc;
    UINT8 profile_compatibility_flag[32];
    UINT8 progressive_source_flag;
    UINT8 interlaced_source_flag;
    UINT8 non_packed_constraint_flag;
    UINT8 frame_only_constraint_flag;
    UINT8 max_12bit_constraint_flag;
    UINT8 max_10bit_constraint_flag;
    UINT8 max_8bit_constraint_flag;
    UINT8 max_422chroma_constraint_flag;
    UINT8 max_420chroma_constraint_flag;
    UINT8 max_monochrome_constraint_flag;
    UINT8 intra_constraint_flag;
    UINT8 one_picture_only_constraint_flag;
    UINT8 lower_bit_rate_constraint_flag;
    UINT8 max_14bit_constraint_flag;
    UINT8 inbld_flag;
    UINT8 level_idc;
#endif
} PTL_COMMON_t;

typedef struct
{
    PTL_COMMON_t    general_ptl;
    PTL_COMMON_t    sub_layer_ptl[H265_MAX_SUB_LAYERS];

    UINT8           sub_layer_profile_present_flag[H265_MAX_SUB_LAYERS];
    UINT8           sub_layer_level_present_flag[H265_MAX_SUB_LAYERS];

} PTL_t;

/* SPS information as per iso standard but not fully implemented. Only portion that we required to know resolution. */
typedef struct
{
    UINT32  sps_video_parameter_set_id;
    INT32   sps_max_sub_layers_minus1;
    UINT8   sps_temporal_id_nesting_flag;

    /* Profile Tier Level */
    PTL_t   ptl;

    UINT32  sps_seq_parameter_set_id;
    UINT32  chroma_format_idc;
    UINT8   separate_colour_plane_flag;
    UINT32  pic_width_in_luma_samples;
    UINT32  pic_height_in_luma_samples;

#if 0 /* Unused */
	UINT32 conformance_window_flag;

	UINT32 conf_win_left_offset;
	UINT32 conf_win_right_offset;
	UINT32 conf_win_top_offset;
	UINT32 conf_win_bottom_offset;
	UINT32 bit_depth_luma_minus8;
	UINT32 bit_depth_chroma_minus8;
	UINT32 log2_max_pic_order_cnt_lsb_minus4;

	UINT32 sps_sub_layer_ordering_info_present_flag;
	UINT32 sps_max_dec_pic_buffering_minus1[8];
	UINT32 sps_max_num_reorder_pics[8];
	UINT32 sps_max_latency_increase_plus1[8];
	UINT32 log2_min_luma_coding_block_size_minus3;
	UINT32 log2_diff_max_min_luma_coding_block_size;
	UINT32 log2_min_luma_transform_block_size_minus2;
	UINT32 log2_diff_max_min_luma_transform_block_size;
	UINT32 max_transform_hierarchy_depth_inter;
	UINT32 max_transform_hierarchy_depth_intra;
	UINT32 scaling_list_enabled_flag;
	UINT32 sps_scaling_list_data_present_flag;

	//scaling_list_data()
	UINT32 scaling_list_pred_mode_flag[100][100];
	UINT32 scaling_list_pred_matrix_id_delta[100][100];
	UINT32 scaling_list_dc_coef_minus8[100][100];
	UINT32 scaling_list_delta_coef;

	UINT32 amp_enabled_flag;
	UINT32 sample_adaptive_offset_enabled_flag;
	UINT32 pcm_enabled_flag;
	UINT32 pcm_sample_bit_depth_luma_minus1;
	UINT32 pcm_sample_bit_depth_chroma_minus1;
	UINT32 log2_min_pcm_luma_coding_block_size_minus3;
	UINT32 log2_diff_max_min_pcm_luma_coding_block_size;
	UINT32 pcm_loop_filter_disabled_flag;
	UINT32 num_short_term_ref_pic_sets;
	UINT32 long_term_ref_pics_present_flag;
	UINT32 num_long_term_ref_pics_sps;
    UINT32 lt_ref_pic_poc_lsb_sps[100];
    UINT32 used_by_curr_pic_lt_sps_flag[100];
	UINT32 sps_temporal_mvp_enabled_flag;
	UINT32 strong_intra_smoothing_enabled_flag;
	UINT32 vui_parameters_present_flag;
	UINT32 sps_extension_present_flag;
	UINT32 sps_range_extension_flag;
	UINT32 sps_multilayer_extension_flag;
	UINT32 sps_3d_extension_flag;
	UINT32 sps_extension_5bits;
	UINT32 sps_extension_data_flag;
#endif

} H265_SPS_t;

/* VOL(video object layer) information as per iso standard but not fully implemented. Only portion that we required to know resolution. */
typedef struct
{
    UINT32 random_accessible_vol;
    UINT32 video_object_type_indication;
    UINT32 is_object_layer_identifier;
    UINT32 video_object_layer_verid;
    UINT32 video_object_layer_priority;

    UINT32 aspect_ratio_info;
    UINT32 par_width;
    UINT32 par_height;

    UINT32 vol_control_parameters;
    UINT32 chroma_format;
    UINT32 low_delay;
    UINT32 vbv_parameters;

    UINT32 first_half_bit_rate;
    UINT32 marker_bit;
    UINT32 latter_half_bit_rate;
    UINT32 marker_bit1;
    UINT32 first_half_vbv_buffer_size;
    UINT32 marker_bit2;
    UINT32 latter_half_vbv_buffer_size;
    UINT32 first_half_vbv_occupancy;
    UINT32 marker_bit3;
    UINT32 latter_half_vbv_occupancy;
    UINT32 marker_bit4;

    UINT32 video_object_layer_shape;
    UINT32 video_object_layer_shape_extension;
    UINT32 marker_bit5;
    UINT32 vop_time_increment_resolution;
    UINT32 marker_bit6;
    UINT32 fixed_vop_rate;
    UINT32 fixed_vop_time_increment;

    UINT32 marker_bit7;
    UINT32 video_object_layer_width;
    UINT32 marker_bit8;
    UINT32 video_object_layer_height;
    UINT32 marker_bit9;

}VIDEO_OBJET_LAYER_t;

//#################################################################################################
// @STATIC VARIABLES
//#################################################################################################
static const UINT8 h264_h265_StartCode[H264_H265_START_CODE_LEN] = {0x00, 0x00, 0x00, 0x01};
static const UINT8 mpeg4StartCode[MPEG4_START_CODE_LEN] = {0x00, 0x00, 0x01};

//#################################################################################################
// @PROTOTYPES
//#################################################################################################
//-------------------------------------------------------------------------------------------------
static BOOL h264ReadSpsInfo(BIT_STREAM_t *bs, H264_SPS_t *sps);
//-------------------------------------------------------------------------------------------------
static BOOL h265ReadSpsInfo(BIT_STREAM_t *bs, H265_SPS_t *sps);
//-------------------------------------------------------------------------------------------------
static void readScalingList(BIT_STREAM_t *bs, INT32PTR scalingList, UINT32 sizeOfScalingList);
//-------------------------------------------------------------------------------------------------
static INT32 bsReadSe(BIT_STREAM_t *bs);
//-------------------------------------------------------------------------------------------------
static void bsInit(BIT_STREAM_t *bs, UINT8PTR buf, UINT32 size, BOOL isEpbIdEnabled);
//-------------------------------------------------------------------------------------------------
static UINT32 bsReadU(BIT_STREAM_t *bs, INT32 n);
//-------------------------------------------------------------------------------------------------
static BOOL bsEof(BIT_STREAM_t *bs);
//-------------------------------------------------------------------------------------------------
static UINT32 bsReadU1(BIT_STREAM_t *bs);
//-------------------------------------------------------------------------------------------------
static UINT32 bsReadU8(BIT_STREAM_t	*bs);
//-------------------------------------------------------------------------------------------------
static UINT32 bsReadUe(BIT_STREAM_t *bs);
//-------------------------------------------------------------------------------------------------
static UINT32 getLog2(UINT32 data);
//-------------------------------------------------------------------------------------------------
static void readVideoObjectLayer(BIT_STREAM_t *bs, VIDEO_OBJET_LAYER_t *videoObjectLayer);
//-------------------------------------------------------------------------------------------------
//#################################################################################################
// @FUNCTIONS
//#################################################################################################
//-------------------------------------------------------------------------------------------------
/**
 * @brief   This function gets resolution from MJPEG bit stream
 * @param   data
 * @param   dataSize
 * @param   videoInfo
 * @return  SUCCESS/FAIL
 */
BOOL GetJpegSize(UINT8PTR data, UINT32 dataSize, VIDEO_INFO_t *videoInfo)
{
    BOOL   retVal = FAIL;
    BOOL   startOfFrame = FALSE;
    UINT32 count = 0;
    UINT32 blockLength = 0;

    /* Validate buffer pointer */
    if ((data == NULL) || (videoInfo == NULL) || (dataSize <= 2))
	{
        return FAIL;
	}

    /* Set invalid frame type bydefault */
    videoInfo->frameType = MAX_FRAME_TYPE;

    while ((count + 2) < dataSize)
    {
        /* Check for valid JPEG image */
        if ((data[count] == 0xFF) && (data[count + 1] == 0xD8))
        {
            count += 2;
            startOfFrame = TRUE;
        }
        else if (startOfFrame == TRUE)
        {
            /* Check that we are truly at the start of another block */
            if (data[count] != 0xFF)
            {
                EPRINT(UTILS, "invld mgpeg frame");
                break;
            }

            /* 0xFFC0 is the "Start of frame" marker which contains the file size */
            if (data[count + 1] == 0xC0)
            {
                /* The structure of the 0xFFC0 block is quite simple
                 * [0xFFC0][ushort length][uchar precision][ushort x][ushort y] */
                videoInfo->height = ((data[count + 5] * 256) + data[count + 6]);
                videoInfo->width = ((data[count + 7] * 256) + data[count + 8]);
                videoInfo->frameType = I_FRAME;
                retVal = SUCCESS;
                break;
            }
            else
            {
                //Skip the block marker
                count += 2;

                //Go to the next block
                blockLength = ((data[count] * 256) + data[count + 1]);

                //Increase the file index to get to the next block
                count += blockLength;
            }
        }
        else
        {
            count++;
        }
	}

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gets resolution as well as frame type from H264 bit stream.
 * @param   frameBuf
 * @param   frameSize
 * @param   videoInfo
 * @param   configPresent
 * @return  SUCCESS/FAIL
 */
BOOL GetH264Info(UINT8PTR frameBuf, UINT32 frameSize, VIDEO_INFO_t *videoInfo, UINT8PTR configPresent)
{
	BOOL			retVal = FAIL;
	UINT32			count = 0;
	UINT32			sliceType;
	BIT_STREAM_t	bitStream;
	NAL_t			nal;

    /* Validate buffer pointer */
    if ((frameBuf == NULL) || (videoInfo == NULL) ||  (frameSize <= H264_H265_START_CODE_LEN))
    {
        return FAIL;
    }

    /* Set default invalid values */
    *configPresent = FALSE;
    videoInfo->frameType = MAX_FRAME_TYPE;

    while((count + H264_H265_START_CODE_LEN) < frameSize)
    {
        /* Check for H264 start code pattern */
        if ((frameBuf[count] != h264_h265_StartCode[0]) || (frameBuf[count+1] != h264_h265_StartCode[1]) ||
                (frameBuf[count+2] != h264_h265_StartCode[2]) || (frameBuf[count+3] != h264_h265_StartCode[3]))
        {
            count++;
            continue;
        }

        /* Skip start code */
        count += H264_H265_START_CODE_LEN;

        bsInit(&bitStream, (frameBuf + count), (frameSize - count), FALSE);
        nal.forbiddenZeroBit = bsReadU(&bitStream, 1);
        nal.nalRefIdc = bsReadU(&bitStream, 2);
        nal.nalUnitType = bsReadU(&bitStream, 5);

        /* Skip nal unit */
        count += 1;

        if ((nal.nalUnitType == NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) || (nal.nalUnitType == NAL_UNIT_TYPE_CODED_SLICE_IDR))
        {
            bsReadUe(&bitStream);
            sliceType = bsReadUe(&bitStream);

            if ((sliceType == 2) || (sliceType == 7))
            {
                videoInfo->frameType = I_FRAME;
            }
            else if ((sliceType == 0) || (sliceType == 5))
            {
                videoInfo->frameType = P_FRAME;
            }
            else if ((sliceType == 1) || (sliceType == 6))
            {
                videoInfo->frameType = B_FRAME;
            }
            retVal = SUCCESS;
            break;
        }
        else if(nal.nalUnitType == NAL_UNIT_TYPE_SPS)
        {
            H264_SPS_t sps;
            if (h264ReadSpsInfo(&bitStream, &sps) == SUCCESS)
            {
                videoInfo->height = ((sps.pic_height_in_map_units_minus1 + 1) * 16) - (sps.frame_crop_bottom_offset * 2)
                                    - (sps.frame_crop_top_offset * 2);
                videoInfo->width = (((2 - sps.frame_mbs_only_flag) * (sps.pic_width_in_mbs_minus1 + 1) * 16)
                                    - (sps.frame_crop_right_offset * 2) - (sps.frame_crop_left_offset * 2));
                if (sps.num_ref_frames < MAX_REF_FRAME)
                {
                    videoInfo->noOfRefFrame = sps.num_ref_frames;
                }

                videoInfo->frameType = SPS_FRAME;
                *configPresent = TRUE;
                retVal = SUCCESS;
            }
            else
            {
                EPRINT(UTILS, "invld h264 frame");
                retVal = FAIL;
            }
            break;
        }
        else if(nal.nalUnitType == NAL_UNIT_TYPE_PPS)
        {
            videoInfo->frameType = PPS_FRAME;
            retVal = SUCCESS;
            break;
        }
    }

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Parse H265 NAL unit
 * @param   frameBuf : NALU data
 * @param   frameSize : NALU size
 * @param   videoInfo : Will be updated after Parsing
 * @param   firstSlice : check if this NALU is first slice or not
 * @return  SUCCESS/FAIL
 */
BOOL GetH265Info(UINT8PTR frameBuf, UINT32 frameSize, VIDEO_INFO_t *videoInfo, UINT8PTR firstSlice)
{
	BOOL			retVal = FAIL;
	UINT32			count = 0;
	BIT_STREAM_t	bitStream;
    UINT8           frameType;

    /* Validate buffer pointer */
    if ((frameBuf == NULL) || (videoInfo == NULL) || (frameSize <= H264_H265_START_CODE_LEN))
    {
        return FAIL;
    }

    /* Set default invalid values */
    *firstSlice = 0;
    videoInfo->frameType = MAX_FRAME_TYPE;

    /* Buffer       : |SC|NALU|
     * NALU         : |NALU Header 2Byte| NALU Payload|
     * NALU Header  : |F 1bit | NAL Type 6 bit | Layrer ID 6 bit | Tid 3 bit |
     * NALU TYPE    : VPS:32, SPS:33, PPP:34, P:01, I:19, SEI:39 (Ref H265 RFC for more info,rfc7798) */
    while ((count + H264_H265_START_CODE_LEN) < frameSize)
    {
        /* Check for H265 start code pattern */
        if ((frameBuf[count] != h264_h265_StartCode[0]) || (frameBuf[count+1] != h264_h265_StartCode[1]) ||
                (frameBuf[count+2] != h264_h265_StartCode[2]) || (frameBuf[count+3] != h264_h265_StartCode[3]))
        {
            count++;
            continue;
        }

        /* Skip start code */
        count += H264_H265_START_CODE_LEN;

        /* Get NAL unit type */
        frameType = (frameBuf[count] >> 1) & 0x3F;

        /* Skip nal unit */
        count += 2;

        if (frameType == 1)
        {
            videoInfo->frameType = P_FRAME;
            *firstSlice = (frameBuf[count] & 0x80) ? TRUE : FALSE;
            retVal = SUCCESS;
            break;
        }
        else if ((frameType >= 16) && (frameType <= 21))
        {
            videoInfo->frameType = I_FRAME;
            // ref 7.3.6.1General slice segment header syntax (ITU High efficiency video coding)
            *firstSlice = (frameBuf[count] & 0x80) ? TRUE : FALSE;
            retVal = SUCCESS;
            break;
        }
        else if (frameType == 32)
        {
            /* Note: For Local Client I frame is: {VPS|SPS|PPS|I}.
             * We need resolution from and it is available in SPS. Hence continue to find SPS in frame */
            #if !defined(GUI_SYSTEM)
            videoInfo->frameType = VPS_FRAME;
            retVal = SUCCESS;
            break;
            #endif
        }
        else if (frameType == 33)
        {
            H265_SPS_t sps;
            bsInit(&bitStream, (frameBuf + count), (frameSize - count), TRUE);

            /* FrameBuf : | startCode | 2Byte H265 Nal Header | H265 Nal Payload
             * Refer    :  ITU-T H.265 ISO standard document */
            h265ReadSpsInfo(&bitStream, &sps);
            videoInfo->height = sps.pic_height_in_luma_samples;
            videoInfo->width  = sps.pic_width_in_luma_samples;
            videoInfo->frameType = SPS_FRAME;
            retVal = SUCCESS;
            break;
        }
        else if (frameType == 34)
        {
            videoInfo->frameType = PPS_FRAME;
            retVal = SUCCESS;
            break;
        }
        else if (frameType == 39)
        {
            #if !defined(GUI_SYSTEM)
            videoInfo->frameType = FRAME_TYPE_SEI;
            retVal = SUCCESS;
            break;
            #endif
        }
    }

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gets resolution as well as frame type from MPEG4 bit stream.
 * @param   frameBuf
 * @param   frameSize
 * @param   videoInfo
 * @param   configPresent
 * @return  SUCCESS/FAIL
 */
BOOL GetMpeg4Info(UINT8PTR frameBuf, UINT32 frameSize, VIDEO_INFO_t *videoInfo, UINT8PTR configPresent)
{
	BOOL					retVal = FAIL;
	UINT32					count = 0;
	UINT32					frameId;
	UINT32					frameType;
    BIT_STREAM_t			bitStream;
	VIDEO_OBJET_LAYER_t		videoObjectLayer;

    /* Validate buffer pointer */
    if ((frameBuf == NULL) || (videoInfo == NULL) || (frameSize <= MPEG4_START_CODE_LEN))
    {
        return FAIL;
    }

    /* Set default invalid values */
    *configPresent = FALSE;
    videoInfo->frameType = MAX_FRAME_TYPE;
    memset(&videoObjectLayer, 0, sizeof(VIDEO_OBJET_LAYER_t));

    while ((count + MPEG4_START_CODE_LEN) < frameSize)
    {
        if ((frameBuf[count] != mpeg4StartCode[0]) || (frameBuf[count+1] != mpeg4StartCode[1]) || (frameBuf[count+2] != mpeg4StartCode[2]))
        {
            count++;
            continue;
        }

        count += MPEG4_START_CODE_LEN;
        bsInit(&bitStream, (frameBuf + count), (frameSize - count), FALSE);
        frameId = bsReadU(&bitStream, 32);
        count += 4;

        if((frameId >= 0x120) && (frameId <= 0x12f))
        {
            readVideoObjectLayer(&bitStream, &videoObjectLayer);
            videoInfo->width = videoObjectLayer.video_object_layer_width;
            videoInfo->height = videoObjectLayer.video_object_layer_height;
            *configPresent = TRUE;
            retVal = SUCCESS;
        }
        else if(frameId == 0x1B6)
        {
            frameType = bsReadU(&bitStream, 2);
            if(frameType == 0x00)
            {
                videoInfo->frameType = I_FRAME;
            }
            else if(frameType == 0x01)
            {
                videoInfo->frameType = P_FRAME;
            }
            retVal = SUCCESS;
            break;
        }
    }

	return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Initialise bit-stream data structure it input frame and frame size
 * @param   bs
 * @param   buf
 * @param   size
 * @param   isEpbIdEnabled: Do EPB identification only for H265 codec.
 * @note    Safer side, we do not want to change implementation of other codec for EPB identification
 */
static void bsInit(BIT_STREAM_t *bs, UINT8PTR buf, UINT32 size, BOOL isEpbIdEnabled)
{
    bs->start = buf;
    bs->priv = buf;
    bs->end = buf + size;
    bs->bitsLeft = 8;
    bs->isEpbIdEnabled = isEpbIdEnabled;
    bs->epbIdCnt = 0;

    /* Confirm whether 1st byte is 0 or not */
    if ((TRUE == bs->isEpbIdEnabled) && (bs->priv[0] == 0x00))
    {
        bs->epbIdCnt++;
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives no of bit as an integer form from current position of bit stream
 * @param   bs
 * @param   n
 * @return  Unsigned integer
 */
static UINT32 bsReadU(BIT_STREAM_t *bs, INT32 n)
{
	UINT32 	retVal = 0;
    INT32 	cnt;

    for (cnt = 0; cnt < n; cnt++)
    {
        retVal |= (bsReadU1(bs) << (n - cnt - 1));
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives indication of end of stream.
 * @param   bs
 * @return  TRUE or FALSE
 */
static BOOL bsEof(BIT_STREAM_t *bs)
{
    if (bs->priv >= bs->end)
	{
        return TRUE;
	}
	else
	{
        return FALSE;
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives only one bit from current bit stream position.
 * @param   bs
 * @return  Single bit
 */
static UINT32 bsReadU1(BIT_STREAM_t *bs)
{
    UINT32 retVal = 0;

    if (bsEof(bs) == TRUE)
    {
        return retVal;
    }

    bs->bitsLeft--;
    retVal = (((*(bs->priv)) >> bs->bitsLeft) & 0x01);
    if (bs->bitsLeft)
    {
        return retVal;
    }

    bs->priv++;
    bs->bitsLeft = 8;

    /* Check EPB enabled or not */
    if (FALSE == bs->isEpbIdEnabled)
    {
        return retVal;
    }

    /* Check 2 consecutive 0x00 found?? */
    if (bs->epbIdCnt >= 2)
    {
        /* Reset EPB identifier */
        bs->epbIdCnt = 0;

        /* If next byte is EPB then skip it */
        if (bs->priv[0] == EMULATION_PREVENTION_BYTE)
        {
            /* Skip EPB */
            bs->priv++;

            /* Is there 0 after EPB */
            if (bs->priv[0] == 0x00)
            {
                /* Reset EPB identifier */
                bs->epbIdCnt++;
            }
        }
    }
    else if (bs->priv[0] == 0x00)
    {
        /* Wait for EPB */
        bs->epbIdCnt++;
    }
    else
    {
        /* Reset EPB identifier */
        bs->epbIdCnt = 0;
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives 8 bit as an integer form from current position of bit stream
 * @param   bs
 * @return  8bits
 */
static UINT32 bsReadU8(BIT_STREAM_t	*bs)
{
    return bsReadU(bs, 8);
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives unsigned integer value as per exp. glombo method from current bit stream position.
 * @param   bs
 * @return  Unsigned integer as per exp.
 */
static UINT32 bsReadUe(BIT_STREAM_t *bs)
{
    INT32  	retVal;
	INT32	cnt = 0;

    while(bsReadU1(bs) == 0 && cnt < 32 && (bsEof(bs) == FALSE))
    {
        cnt++;
    }

    retVal = bsReadU(bs, cnt);
    retVal += (1 << cnt) - 1;
    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gives signed integer value as per exp. glombo method from current bit stream position.
 * @param   bs
 * @return  Signed integer value
 */
static INT32 bsReadSe(BIT_STREAM_t *bs)
{
    INT32 retVal = bsReadUe(bs);

    if (retVal & 0x01)
    {
    	retVal = (retVal+1)/2;
    }
    else
    {
    	retVal = -(retVal/2);
    }

    return retVal;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   h265DecodeProfileTierLevel
 * @param   bs
 * @param   ptl
 */
static void h265DecodeProfileTierLevel(BIT_STREAM_t *bs, PTL_COMMON_t *ptl)
{
    /* Skip 8: profile_space (2bit), tier_flag (1bit), profile_idc (5bit) */
    /* Skip 32: general_profile_compatibility_flag (32bit) */
    /* Skip 4: progressive_source_flag (1bit), interlaced_source_flag (1bit), non_packed_constraint_flag (1bit), frame_only_constraint_flag (1bit) */
    /* Skip 44: max_12bit_constraint_flag to inbld_flag (43bit + 1bit) */
    bsReadU(bs, 88);

    /* Avoid compilation warning */
    (void)ptl;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Read profile level information according to stnadard
 * @param   bs
 * @param   sps
 */
static void h265ProfileTierLevel(BIT_STREAM_t *bs, H265_SPS_t *sps)
{
    INT32 loop;

    /* Get general ptl */
    h265DecodeProfileTierLevel(bs, &sps->ptl.general_ptl);

    /* Skip: general_level_idc (8bit) */
    bsReadU8(bs);

    for (loop = 0; loop < sps->sps_max_sub_layers_minus1; loop++)
    {
        /* Get sub layer profile and level preset flags */
        sps->ptl.sub_layer_profile_present_flag[loop] = bsReadU1(bs);
        sps->ptl.sub_layer_level_present_flag[loop] = bsReadU1(bs);
    }

    if (sps->sps_max_sub_layers_minus1 > 0)
    {
        for (loop = sps->sps_max_sub_layers_minus1; loop < 8; loop++)
        {
            /* Skip: reserved_zero (2bit) */
            bsReadU(bs, 2);
        }
    }

    for (loop = 0; loop < sps->sps_max_sub_layers_minus1; loop++)
    {
        if (sps->ptl.sub_layer_profile_present_flag[loop])
        {
            /* Get sub layer ptl */
            h265DecodeProfileTierLevel(bs, &sps->ptl.sub_layer_ptl[loop]);
        }

        if (sps->ptl.sub_layer_level_present_flag[loop])
        {
            /* Skip: sub layer level_idc (8bit) */
            bsReadU8(bs);
        }
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Refer http://handle.itu.int/11.1002/1000/14107 for ITU ISO standards
 * @param   bs
 * @param   sps
 * @param   flag : Required to extract Profile tier lever information
 * @return  SUCCESS
 */
static BOOL h265ReadSpsInfo(BIT_STREAM_t *bs, H265_SPS_t *sps)
{
    /* Skip: sps_video_parameter_set_id (4bit) */
    bsReadU(bs, 4);

    /* Get max sub layers (3bit) */
    sps->sps_max_sub_layers_minus1 = bsReadU(bs, 3);

    /* Skip: sps_temporal_id_nesting_flag (1bit) */
    bsReadU1(bs);

    /* Get profile tier level */
    h265ProfileTierLevel(bs, sps);

    /* Skip: sps_seq_parameter_set_id (32bit) */
    bsReadUe(bs);

    /* Get chroma_format_idc (32bit) */
    sps->chroma_format_idc = bsReadUe(bs);
	if(sps->chroma_format_idc == 3)
	{
        /* Skip: separate_colour_plane_flag (1bit) */
        bsReadU1(bs);
	}

    /* Get resolution of video */
    sps->pic_width_in_luma_samples =  bsReadUe(bs);
    sps->pic_height_in_luma_samples = bsReadUe(bs);

    /* We got Resolution, no need to parse futher SPS information */
	return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Extract whole SPS (Sequence Parameter Set) from bit stream.
 * @param   bs
 * @param   sps
 * @return  SUCCESS/FAIL
 */
static BOOL h264ReadSpsInfo(BIT_STREAM_t *bs, H264_SPS_t *sps)
{
    UINT32 cnt;

    sps->profile_idc = bsReadU8(bs);
    sps->constraint_set0_flag = bsReadU1(bs);
    sps->constraint_set1_flag = bsReadU1(bs);
    sps->constraint_set2_flag = bsReadU1(bs);
    sps->constraint_set3_flag = bsReadU1(bs);
    sps->reserved_zero_4bits = bsReadU(bs,4);
    sps->level_idc = bsReadU8(bs);
    sps->seq_parameter_set_id = bsReadUe(bs);

    if (sps->profile_idc == 100 || sps->profile_idc == 110 || sps->profile_idc == 122 || sps->profile_idc == 144)
    {
        sps->chroma_format_idc = bsReadUe(bs);
        if(sps->chroma_format_idc == 3)
        {
            sps->residual_colour_transform_flag = bsReadU1(bs);
        }

        sps->bit_depth_luma_minus8 = bsReadUe(bs);
        sps->bit_depth_chroma_minus8 = bsReadUe(bs);
        sps->qpprime_y_zero_transform_bypass_flag = bsReadU1(bs);
        sps->seq_scaling_matrix_present_flag = bsReadU1(bs);
        if (sps->seq_scaling_matrix_present_flag)
        {
            for (cnt = 0; cnt < ((sps->chroma_format_idc != 3) ? 8 : 12); cnt++)
            {
                sps->seq_scaling_list_present_flag[cnt] = bsReadU1(bs);
                if (sps->seq_scaling_list_present_flag[cnt])
                {
                    if (cnt < 6)
                    {
                        readScalingList(bs, sps->ScalingList4x4[cnt], 16);
                    }
                    else
                    {
                        readScalingList(bs, sps->ScalingList8x8[cnt - 6], 64);
                    }
                }
            }
        }
    }

    sps->log2_max_frame_num_minus4 = bsReadUe(bs);
    sps->pic_order_cnt_type = bsReadUe(bs);
    if (sps->pic_order_cnt_type == 0)
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = bsReadUe(bs);
    }
    else if (sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag = bsReadU1(bs);
        sps->offset_for_non_ref_pic = bsReadSe(bs);
        sps->offset_for_top_to_bottom_field = bsReadSe(bs);
        sps->num_ref_frames_in_pic_order_cnt_cycle = bsReadUe(bs);

        if (sps->num_ref_frames_in_pic_order_cnt_cycle >= 256)
        {
           	return FAIL;
        }

        for (cnt = 0; cnt < sps->num_ref_frames_in_pic_order_cnt_cycle; cnt++)
        {
            sps->offset_for_ref_frame[cnt] = bsReadSe(bs);
        }
    }

    sps->num_ref_frames = bsReadUe(bs);
    sps->gaps_in_frame_num_value_allowed_flag = bsReadU1(bs);
    sps->pic_width_in_mbs_minus1 = bsReadUe(bs);
    sps->pic_height_in_map_units_minus1 = bsReadUe(bs);
    sps->frame_mbs_only_flag = bsReadU1(bs);
    if (!sps->frame_mbs_only_flag)
    {
        sps->mb_adaptive_frame_field_flag = bsReadU1(bs);
    }

    sps->direct_8x8_inference_flag = bsReadU1(bs);
    sps->frame_cropping_flag = bsReadU1(bs);
    if (sps->frame_cropping_flag)
    {
        sps->frame_crop_left_offset = bsReadUe(bs);
        sps->frame_crop_right_offset = bsReadUe(bs);
        sps->frame_crop_top_offset = bsReadUe(bs);
        sps->frame_crop_bottom_offset = bsReadUe(bs);
    }
    else
    {
    	sps->frame_crop_left_offset = 0;
		sps->frame_crop_right_offset = 0;
		sps->frame_crop_top_offset = 0;
		sps->frame_crop_bottom_offset = 0;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Gets 4x4 or 8x8 matrix valuse as per define in standard of H264. Please refer 7.3.2.1.1.1 Scaling list syntax
 * @param   bs
 * @param   scalingList
 * @param   sizeOfScalingList
 */
static void readScalingList(BIT_STREAM_t *bs, INT32PTR scalingList, UINT32 sizeOfScalingList)
{
    UINT32  j;
    UINT32  lastScale = 8;
    UINT32  nextScale = 8;
    INT32   delta_scale;

    for (j = 0; j < sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            delta_scale = bsReadSe(bs);
            nextScale = (lastScale + delta_scale + 256) % 256;
        }

        scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[j];
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Get Log2
 * @param   data
 * @return  Log value
 */
static UINT32 getLog2(UINT32 data)
{
    UINT32 logValue = 0;

    while (data)
	{
		data >>= 1;

        if (data)
		{
			logValue++;
		}
	}

    return logValue;
}

//-------------------------------------------------------------------------------------------------
/**
 * @brief   Extracts information of video object layer from bit stream
 * @param   bs
 * @param   videoObjectLayer
 */
static void readVideoObjectLayer(BIT_STREAM_t *bs, VIDEO_OBJET_LAYER_t *videoObjectLayer)
{
    videoObjectLayer->random_accessible_vol = bsReadU(bs, 1);
    videoObjectLayer->video_object_type_indication = bsReadU(bs, 8);
    videoObjectLayer->is_object_layer_identifier = bsReadU(bs, 1);

	if(videoObjectLayer->is_object_layer_identifier)
	{
        videoObjectLayer->video_object_layer_verid = bsReadU(bs, 4);
        videoObjectLayer->video_object_layer_priority = bsReadU(bs, 3);
	}

    videoObjectLayer->aspect_ratio_info = bsReadU(bs, 4);
	if(videoObjectLayer->aspect_ratio_info == 0x0F)
	{
        videoObjectLayer->par_width = bsReadU(bs, 8);
        videoObjectLayer->par_height = bsReadU(bs, 8);
	}

    videoObjectLayer->vol_control_parameters = bsReadU(bs, 1);

	if(videoObjectLayer->vol_control_parameters)
	{
        videoObjectLayer->chroma_format = bsReadU(bs, 2);
        videoObjectLayer->low_delay = bsReadU(bs, 1);
        videoObjectLayer->vbv_parameters = bsReadU(bs, 1);

		if(videoObjectLayer->vbv_parameters)
		{
            videoObjectLayer->first_half_bit_rate = bsReadU(bs, 15);
            videoObjectLayer->marker_bit = bsReadU(bs, 1);
            videoObjectLayer->latter_half_bit_rate = bsReadU(bs, 15);
            videoObjectLayer->marker_bit1 = bsReadU(bs, 1);

            videoObjectLayer->first_half_vbv_buffer_size = bsReadU(bs, 15);

            videoObjectLayer->marker_bit2 = bsReadU(bs, 1);

            videoObjectLayer->latter_half_vbv_buffer_size = bsReadU(bs, 3);
            videoObjectLayer->first_half_vbv_occupancy = bsReadU(bs, 11);
            videoObjectLayer->marker_bit3 = bsReadU(bs, 1);
            videoObjectLayer->latter_half_vbv_occupancy = bsReadU(bs, 15);
            videoObjectLayer->marker_bit4 = bsReadU(bs, 1);
		}
	}

    videoObjectLayer->video_object_layer_shape = bsReadU(bs, 2);

    if((videoObjectLayer->video_object_layer_shape == 3) && (videoObjectLayer->video_object_layer_verid != 1))
	{
        videoObjectLayer->video_object_layer_shape_extension = bsReadU(bs, 4);
	}

    videoObjectLayer->marker_bit5 = bsReadU(bs, 1);
    videoObjectLayer->vop_time_increment_resolution = bsReadU(bs, 16);
    videoObjectLayer->marker_bit6 = bsReadU(bs, 1);
    videoObjectLayer->fixed_vop_rate = bsReadU(bs, 1);

	if(videoObjectLayer->fixed_vop_rate == 1)
	{
        UINT8 log2 = getLog2(videoObjectLayer->vop_time_increment_resolution - 1) + 1;
        if (log2 < 1)
		{
            log2 = 1;
		}
        videoObjectLayer->fixed_vop_time_increment = bsReadU(bs, log2);

	}
	else
	{
        videoObjectLayer->fixed_vop_time_increment = getLog2(videoObjectLayer->vop_time_increment_resolution - 1) + 1;
	}

	if(videoObjectLayer->video_object_layer_shape != 1)
	{
		if(videoObjectLayer->video_object_layer_shape == 0)
		{
            videoObjectLayer->marker_bit7 = bsReadU(bs, 1);
            videoObjectLayer->video_object_layer_width = bsReadU(bs, 13);
            videoObjectLayer->marker_bit8 = bsReadU(bs, 1);
            videoObjectLayer->video_object_layer_height = bsReadU(bs, 13);
            videoObjectLayer->marker_bit9 = bsReadU(bs, 1);
		}
	}
}

//#################################################################################################
// @END OF FILE
//#################################################################################################
