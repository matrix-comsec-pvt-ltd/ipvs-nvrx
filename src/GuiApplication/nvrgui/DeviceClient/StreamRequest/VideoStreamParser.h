#ifndef VIDEOSTREAMPARSER1_H
#define VIDEOSTREAMPARSER1_H

#include "FrameHeader.h"
#include "VideoParser.h"

class VideoStreamParser
{
    public:
        VideoStreamParser();

        static bool getHeightWidth(char *frameStartPos,
                            FRAME_HEADER_t *header,
                            quint16 &width,
                            quint16 &height);
};

#endif // VIDEOPARSER_H


