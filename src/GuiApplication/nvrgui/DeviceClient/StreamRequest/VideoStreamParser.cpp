#include "VideoStreamParser.h"

VideoStreamParser::VideoStreamParser()
{
}

bool VideoStreamParser::getHeightWidth(char *frameStartPos, FRAME_HEADER_t* header, quint16 &width, quint16 &height)
{
    UINT8           m_configPresent;
    VIDEO_INFO_t    videoInfo;

	memset(&videoInfo, 0, sizeof(videoInfo));

    switch(header->codecType)
    {
        case VIDEO_H264:
        {
            if(GetH264Info((UINT8PTR)(frameStartPos), (header->frameSize - sizeof (FRAME_HEADER_t)), &videoInfo, &m_configPresent) == FAIL)
            {
                return false;
            }
        }
        break;

        case VIDEO_MJPG:
        {
            if(GetJpegSize((UINT8PTR)(frameStartPos), (header->frameSize - sizeof (FRAME_HEADER_t)), &videoInfo) == FAIL)
            {
                return false;
            }
        }
        break;

        case VIDEO_MPEG4:
        {
            if(GetMpeg4Info((UINT8PTR)(frameStartPos), (header->frameSize - sizeof (FRAME_HEADER_t)), &videoInfo, &m_configPresent) == FAIL)
            {
                return false;
            }
        }
        break;

        case VIDEO_H265:
        {
            if(GetH265Info((UINT8PTR)(frameStartPos), (header->frameSize - sizeof (FRAME_HEADER_t)), &videoInfo, &m_configPresent) == FAIL)
            {
                return false;
            }
        }
        break;

        default:
        {
            return false;
        }
    }

    width = videoInfo.width;
    height = videoInfo.height;
    return true;
}
