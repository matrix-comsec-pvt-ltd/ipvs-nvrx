#include <QKeyEvent>

#include "LayoutCreator.h"
#include "ApplicationMode.h"
#include "Layout/Layout.h"
#include "ApplController.h"

#define ROW_INDEX           0
#define COL_INDEX           1
#define ROW_SPANING         2
#define COL_SPANING         3

static const quint8 windowInfo[MAX_LAYOUT][MAX_WINDOWS][4] =
{
    //layout 1X1
    {{0,0,5,5}},

    // layout 2x2
    {{0,0,2,2},{0,2,2,2},{2,0,2,2},{2,2,2,2}},

    // layout 1+5
    {{0,0,2,2},{0,2,1,1},{1,2,1,1},{2,0,1,1},{2,1,1,1},{2,2,1,1}},

    // layout 3+4
    {{0,0,2,2},{0,2,2,2},{2,0,2,2},{2,2,1,1},{2,3,1,1},{3,2,1,1},{3,3,1,1}},

    // layout 1+7
    {{0,0,3,3},{0,3,1,1},{1,3,1,1},{2,3,1,1},{3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1}},

    // layout 3x3
    {{0,0,1,1},{0,1,1,1},{0,2,1,1},{1,0,1,1},{1,1,1,1},{1,2,1,1},{2,0,1,1},{2,1,1,1},{2,2,1,1}},

    // layout 2+8
    {{0,0,2,2},{0,2,2,2},{2,0,1,1},{2,1,1,1},{2,2,1,1},{2,3,1,1},{3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1}},

    // layout 1+9
    {{0,0,4,4},{0,4,1,1},{1,4,1,1},{2,4,1,1},{3,4,1,1},{4,0,1,1},{4,1,1,1},{4,2,1,1},{4,3,1,1},{4,4,1,1}},

    // layout 1+12
    {{0,0,2,2},{0,2,1,1},{0,3,1,1},{1,2,1,1},{1,3,1,1},{2,0,1,1},{2,1,1,1},{2,2,1,1},{2,3,1,1},{3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1}},

    // layout 1c+12
    {{0,0,1,1},{0,1,1,1},{0,2,1,1},{0,3,1,1},{1,0,1,1},{1,1,2,2},{1,3,1,1},{2,0,1,1},{2,3,1,1},{3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1}},

    // layout 4+9
    {{0,0,2,2},{0,2,2,2},{0,4,1,1},{1,4,1,1},{2,0,2,2},{2,2,2,2},{2,4,1,1},{3,4,1,1},{4,0,1,1},{4,1,1,1},{4,2,1,1},{4,3,1,1},{4,4,1,1}},

    // layout 2+12
    {{0,0,3,3},{0,3,2,2},{2,3,1,1},{2,4,1,1},{3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1},{3,4,1,1},{4,0,1,1},{4,1,1,1},{4,2,1,1},{4,3,1,1},{4,4,1,1}},

    // layout 4x4
    {{0,0,1,1},{0,1,1,1},{0,2,1,1},{0,3,1,1},
     {1,0,1,1},{1,1,1,1},{1,2,1,1},{1,3,1,1},
     {2,0,1,1},{2,1,1,1},{2,2,1,1},{2,3,1,1},
     {3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1}},

    // layout 5X5
    {{0,0,1,1},{0,1,1,1},{0,2,1,1},{0,3,1,1},{0,4,1,1},
     {1,0,1,1},{1,1,1,1},{1,2,1,1},{1,3,1,1},{1,4,1,1},
     {2,0,1,1},{2,1,1,1},{2,2,1,1},{2,3,1,1},{2,4,1,1},
     {3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1},{3,4,1,1},
     {4,0,1,1},{4,1,1,1},{4,2,1,1},{4,3,1,1},{4,4,1,1}},

    // layout 6X6
    {{0,0,6,6},{0,6,6,6},{0,12,6,6},{0,18,6,6},{0,24,6,6},{0,30,6,6},
     {6,0,6,6},{6,6,6,6},{6,12,6,6},{6,18,6,6},{6,24,6,6},{6,30,6,6},
     {12,0,6,6},{12,6,6,6},{12,12,6,6},{12,18,6,6},{12,24,6,6},{12,30,6,6},
     {18,0,6,6},{18,6,6,6},{18,12,6,6},{18,18,6,6},{18,24,6,6},{18,30,6,6},
     {24,0,6,6},{24,6,6,6},{24,12,6,6},{24,18,6,6},{24,24,6,6},{24,30,6,6},
     {30,0,6,6},{30,6,6,6},{30,12,6,6},{30,18,6,6},{30,24,6,6},{30,30,6,6}},

    // layout 8X8
#if defined(RK3568_NVRL)
    /* Rockchip doesn't have 8x8 layout. Hence added dummy of 1x1 layout */
    {{0,0,5,5}},
#else
    {{0,0,8,8},{0,8,8,8},{0,16,8,8},{0,24,8,8},{0,32,8,8},{0,40,8,8},{0,48,8,8},{0,56,8,8},
     {8,0,8,8},{8,8,8,8},{8,16,8,8},{8,24,8,8},{8,32,8,8},{8,40,8,8},{8,48,8,8},{8,56,8,8},
     {16,0,8,8},{16,8,8,8},{16,16,8,8},{16,24,8,8},{16,32,8,8},{16,40,8,8},{16,48,8,8},{16,56,8,8},
     {24,0,8,8},{24,8,8,8},{24,16,8,8},{24,24,8,8},{24,32,8,8},{24,40,8,8},{24,48,8,8},{24,56,8,8},
     {32,0,8,8},{32,8,8,8},{32,16,8,8},{32,24,8,8},{32,32,8,8},{32,40,8,8},{32,48,8,8},{32,56,8,8},
     {40,0,8,8},{40,8,8,8},{40,16,8,8},{40,24,8,8},{40,32,8,8},{40,40,8,8},{40,48,8,8},{40,56,8,8},
     {48,0,8,8},{48,8,8,8},{48,16,8,8},{48,24,8,8},{48,32,8,8},{48,40,8,8},{48,48,8,8},{48,56,8,8},
     {56,0,8,8},{56,8,8,8},{56,16,8,8},{56,24,8,8},{56,32,8,8},{56,40,8,8},{56,48,8,8},{56,56,8,8}},
#endif

    // layout 1X1 Playback
    {{0,0,5,5}},

    // layout 2X2 Playback
    {{0,0,2,2},{0,2,2,2},{2,0,2,2},{2,2,2,2}},

    //layout 3X3 Playback
    {{0,0,1,1},{0,1,1,1},{0,2,1,1},{1,0,1,1},{1,1,1,1},{1,2,1,1},{2,0,1,1},{2,1,1,1},{2,2,1,1}},

    //layout 4X4 Playback
    {{0,0,1,1},{0,1,1,1},{0,2,1,1},{0,3,1,1},
     {1,0,1,1},{1,1,1,1},{1,2,1,1},{1,3,1,1},
     {2,0,1,1},{2,1,1,1},{2,2,1,1},{2,3,1,1},
     {3,0,1,1},{3,1,1,1},{3,2,1,1},{3,3,1,1}}
};

static const qint8 windowNeighbours[MAX_LAYOUT][MAX_WINDOWS][MAX_NEIGHBOUR] =
{
    //layout 1X1
    {{-1,-1,-1,-1}},

    // layout 2x2
    {{-1,2,-1,1},{-1,3,0,-1},{0,-1,-1,3},{1,-1,2,-1}},

    // layout 1+5
    {{-1,3,-1,1},{-1,2,0,-1},{1,5,0,-1},{0,-1,-1,4},{0,-1,3,5},{2,-1,4,-1}},

    // layout 3+4
    {{-1,2,-1,1},{-1,3,0,-1},{0,-1,-1,3},{1,5,2,4},{1,6,3,-1},{3,-1,2,6},{4,-1,5,-1}},

    // layout 1+7
    {{-1,4,-1,1},{-1,2,0,-1},{1,3,0,-1},{2,7,0,-1},{0,-1,-1,5},{0,-1,4,6},{0,-1,5,7},{3,-1,6,-1}},

    // layout 3x3
    {{-1,3,-1,1},{-1,4,0,2},{-1,5,1,-1},{0,6,-1,4},{1,7,3,5},{2,8,4,-1},{3,-1,-1,7},{4,-1,6,8},{5,-1,7,-1}},

    // layout 2+8
    {{-1,2,-1,1},{-1,4,0,-1},{0,6,-1,3},{0,7,2,4},{1,8,3,5},{1,9,4,-1},{2,-1,-1,7},{3,-1,6,8},{4,-1,7,9},{5,-1,8,-1}},

    // layout 1+9
    {{-1,5,-1,1},{-1,2,0,-1},{1,3,0,-1},{2,4,0,-1},{3,9,0,-1},{0,-1,-1,6},{0,-1,5,7},{0,-1,6,8},{0,-1,7,9},{4,-1,8,-1}},

    // layout 1+12
    {{-1,5,-1,1},{-1,3,0,2},{-1,4,1,-1},{1,7,0,4},{2,8,3,-1},{0,9,-1,6},{0,10,5,7},{3,11,6,8},{4,12,7,-1},{5,-1,-1,10},{6,-1,9,11},{7,-1,10,12},{8,-1,11,-1}},

    // layout 1c+12
    {{-1,4,-1,1},{-1,5,0,2},{-1,5,1,3},{-1,6,2,-1},{0,7,-1,5},{1,10,4,6},{3,8,5,-1},{4,9,-1,5},{6,12,5,-1},{7,-1,-1,10},{5,-1,9,11},{5,-1,10,12},{8,-1,11,-1}},

    // layout 4+9
    {{-1,4,-1,1},{-1,5,0,2},{-1,3,1,-1},{2,6,1,-1},{0,8,-1,5},{1,10,4,6},{3,7,5,-1},{6,12,5,-1},{4,-1,-1,9},{4,-1,8,10},{5,-1,9,11},{5,-1,10,12},{7,-1,11,-1}},

    // layout 2+12
    {{-1,4,-1,1},{-1,2,0,-1},{1,7,0,3},{1,8,2,-1},{0,9,-1,5},{0,10,4,6},{0,11,5,7},{2,12,6,8},{3,13,7,-1},{4,-1,-1,10},{5,-1,9,11},{6,-1,10,12},{7,-1,11,13},{8,-1,12,-1}},

    // layout 4x4
    {{-1,4,-1,1},{-1,5,0,2},{-1,6,1,3},{-1,7,2,-1},
     {0,8,-1,5},{1,9,4,6},{2,10,5,7},{3,11,6,-1},
     {4,12,-1,9},{5,13,8,10},{6,14,9,11},{7,15,10,-1},
     {8,-1,-1,13},{9,-1,12,14},{10,-1,13,15},{11,-1,14,-1}
    },

    // layout 5X5
    {{-1,5,-1,1},{-1,6,0,2},{-1,7,1,3},{-1,8,2,4},{-1,9,3,-1},
     {0,10,-1,6},{1,11,5,7},{2,12,6,8},{3,13,7,9},{4,14,8,-1},
     {5,15,-1,11},{6,16,10,12},{7,17,11,13},{8,18,12,14},{9,19,13,-1},
     {10,20,-1,16},{11,21,15,17},{12,22,16,18},{13,23,17,19},{14,24,18,-1},
     {15,-1,-1,21},{16,-1,20,22},{17,-1,21,23},{18,-1,22,24},{19,-1,23,-1}
    },

    // layout 6X6
    {{-1,6,-1,1},{-1,7,0,2},{-1,8,1,3},{-1,9,2,4},{-1,10,3,5},{-1,11,4,-1},
     {0,12,-1,7},{1,13,6,8},{2,14,7,9},{3,15,8,10},{4,16,9,11},{5,17,10,-1},
     {6,18,-1,13},{7,19,12,14},{8,20,13,15},{9,21,14,16},{10,22,15,17},{11,23,16,-1},
     {12,24,-1,19},{13,25,18,20},{14,26,19,21},{15,27,20,22},{16,28,21,23},{17,29,22,-1},
     {18,30,-1,25},{19,31,24,26},{20,32,25,27},{21,33,26,28},{22,34,27,29},{23,35,28,-1},
     {24,-1,-1,31},{25,-1,30,32},{26,-1,31,33},{27,-1,32,34},{28,-1,33,35},{29,-1,34,-1}
    },

    // layout 8X8
#if defined(RK3568_NVRL)
    /* Rockchip doesn't have 8x8 layout. Hence added dummy of 1x1 layout */
    {{-1,-1,-1,-1}},
#else
    {{-1,8,-1,1},{-1,9,0,2},{-1,10,1,3},{-1,11,2,4},{-1,12,3,5},{-1,13,4,6},{-1,14,5,7},{-1,15,6,-1},
     {0,16,-1,9},{1,17,8,10},{2,18,9,11},{3,19,10,12},{4,20,11,13},{5,21,12,14},{6,22,13,15},{7,23,14,-1},
     {8,24,-1,17},{9,25,16,18},{10,26,17,19},{11,27,18,20},{12,28,19,21},{13,29,20,22},{14,30,21,23},{15,31,22,-1},
     {16,32,-1,25},{17,33,24,26},{18,34,25,27},{19,35,26,28},{20,36,27,29},{21,37,28,30},{22,38,29,31},{23,39,30,-1},
     {24,40,-1,33},{25,41,32,34},{26,42,33,35},{27,43,34,36},{28,44,35,37},{29,45,36,38},{30,46,37,39},{31,47,38,-1},
     {32,48,-1,41},{33,49,40,42},{34,50,41,43},{35,51,42,44},{36,52,43,45},{37,53,44,46},{38,54,45,47},{39,55,46,-1},
     {40,56,-1,49},{41,57,48,50},{42,58,49,51},{43,59,50,52},{44,60,51,53},{45,61,52,54},{46,62,53,55},{47,63,54,-1},
     {48,-1,-1,57},{49,-1,56,58},{50,-1,57,59},{51,-1,58,60},{52,-1,59,61},{53,-1,60,62},{54,-1,61,63},{55,-1,62,-1},
    },
#endif

    //layout 1X1 SYNC_PLAYBACK
    {{-1,-1,-1,-1}},

    // layout 2x2 SYNC_PLAYBACK
    {{-1,2,-1,1},{-1,3,0,-1},
     {0,-1,-1,3},{1,-1,2,-1}},

    //layout 3X3 SYNC_PLAYBACK
    {{-1,3,-1,1},{-1,4,0,2},{-1,5,1,-1},
     {0,6,-1,4},{1,7,3,5},{2,8,4,-1},
     {3,-1,-1,7},{4,-1,6,8},{5,-1,7,-1}},

    //layout 4X4 SYNC_PLAYBACK
    {{-1,4,-1,1},{-1,5,0,2},{-1,6,1,3},{-1,7,2,-1},
     {0,8,-1,5},{1,9,4,6},{2,10,5,7},{3,11,6,-1},
     {4,12,-1,9},{5,13,8,10},{6,14,9,11},{7,15,10,-1},
     {8,-1,-1,13},{9,-1,12,14},{10,-1,13,15},{11,-1,14,-1}}
};

static const quint8 pbToolbarSizeType[MAX_LAYOUT][MAX_WINDOWS] =
{
    //layout 1X1
    {0},
    // layout 2x2
    {0,0,0,0},
    // layout 1+5
    {0,1,1,1,1,1},
    // layout 3+4
    {0,0,0,2,2,2,2},
    // layout 1+7
    {0,2,2,2,2,2,2,2},
    // layout 3x3
    {1,1,1,1,1,1,1,1,1},
    // layout 2+8
    {0,0,2,2,2,2,2,2,2,2},
    // layout 1+9
    {0,3,3,3,3,3,3,3,3,3},
    // layout 1+12
    {0,2,2,2,2,2,2,2,2,2,2,2,2},
    // layout 1c+12
    {2,2,2,2,2,0,2,2,2,2,2,2,2},
    // layout 4+9
    {1,1,3,3,1,1,3,3,3,3,3,3,3},
    // layout 2+12
    {0,1,3,3,3,3,3,3,3,3,3,3,3,3},
    // layout 4x4
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    // layout 5x5
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    // layout 6x6
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    // layout 8x8
#if defined(RK3568_NVRL)
    /* Rockchip doesn't have 8x8 layout. Hence added dummy of 1x1 layout */
    {0},
#else
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
#endif
    //layout 1X1 SYNC_PLAYBACK
    {0},
    // layout 2x2 SYNC_PLAYBACK
    {0,0,0,0},
    //layout 3X3 SYNC_PLAYBACK
    {1,1,1,1,1,1,1,1,1},
    //layout 4X4 SYNC_PLAYBACK
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
};

static const quint8 apToolbarBtnSize[MAX_LAYOUT][MAX_WINDOWS] =
{
    //layout 1X1
    {0},
    // layout 2x2
    {1,1,1,1},
    // layout 1+5
    {1,2,2,2,2,2},
    // layout 3+4
    {1,1,1,3,3,3,3},
    // layout 1+7
    {1,3,3,3,3,3,3,3},
    // layout 3x3
    {2,2,2,2,2,2,2,2,2},
    // layout 2+8
    {1,1,3,3,3,3,3,3,3,3},
    // layout 1+9
    {1,3,3,3,3,3,3,3,3,3},
    // layout 1+12
    {1,3,3,3,3,3,3,3,3,3,3,3,3},
    // layout 1c+12
    {3,3,3,3,3,1,3,3,3,3,3,3,3},
    // layout 4+9
    {1,1,3,3,1,1,3,3,3,3,3,3,3},
    // layout 2+12
    {1,2,3,3,3,3,3,3,3,3,3,3,3,3},
    // layout 4x4
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    // layout 5x5
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    // layout 6x6
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
    // layout 8x8
#if defined(RK3568_NVRL)
    /* Rockchip doesn't have 8x8 layout. Hence added dummy of 1x1 layout */
    {0},
#else
    {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
#endif
    //layout 1X1 SYNC_PLAYBACK
    {0},
    // layout 2x2 SYNC_PLAYBACK
    {1,1,1,1},
    //layout 3X3 SYNC_PLAYBACK
    {2,2,2,2,2,2,2,2,2},
    //layout 4X4 SYNC_PLAYBACK
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
};

LayoutCreator::LayoutCreator(quint32 startx,
                             quint32 starty,
                             quint32 width,
                             quint32 height,
                             QWidget *parent,
                             int indexInPage,
                             WINDOW_TYPE_e windowType,
                             bool isEnabled,
                             quint16 maxWindows)
    : KeyBoard(parent), NavigationControl(indexInPage, isEnabled)
{
    this->setGeometry(startx, starty, width, height);
    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();

    m_selectedWindow = MAX_CHANNEL_FOR_SEQ;
    m_selectedLayout = MAX_LAYOUT;
    m_firstWindowIndex = MAX_CHANNEL_FOR_SEQ;
    m_secondWindowIndex = MAX_CHANNEL_FOR_SEQ;
    m_mouseClicked = m_isDragEvent = false;
    m_proxyWindowCount = 0;
    m_maxWindows = maxWindows;
    m_windowType = windowType;

    m_layoutBg = new QGridLayout();
    this->setLayout(m_layoutBg);
    m_layoutBg->setContentsMargins(0,0,0,0);
    m_layoutBg->setHorizontalSpacing(0);
    m_layoutBg->setVerticalSpacing(0);
    this->setLayout(m_layoutBg);

    for(quint8 index = 0; index < MAX_WINDOWS; index++)
    {
        m_windows[index] = NULL;
    }
    for(quint8 index = 0; index < MAX_PROXY_WINDOW; index++)
    {
        m_proxyWindow[index] = NULL;
    }

    for(quint8 windowIndex = 0; windowIndex < MAX_WINDOWS; windowIndex++)
    {
        m_windows[windowIndex] = new LayoutWindow(windowIndex, this, windowType);
        connect(m_windows[windowIndex],
                SIGNAL(sigWindowSelected(quint8)),
                this,
                SLOT(slotWindowSelected(quint8)));
        connect(m_windows[windowIndex],
                SIGNAL(sigLoadMenuListOptions(quint8)),
                this,
                SLOT(slotLoadMenuListOptions(quint8)));
        connect(m_windows[windowIndex],
                SIGNAL(sigWindowDoubleClicked(quint8)),
                this,
                SLOT(slotWindowDoubleClicked(quint8)));
        connect(m_windows[windowIndex],
                SIGNAL(sigEnterKeyPressed(quint8)),
                this,
                SLOT(slotEnterKeyPressed(quint8)));
        connect(m_windows[windowIndex],
                SIGNAL(sigWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint8)),
                this,
                SLOT(slotWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint8)));
        connect(m_windows[windowIndex],
                SIGNAL(sigWindowImageHover(WINDOW_IMAGE_TYPE_e,quint8,bool)),
                this,
                SLOT(slotWindowImageHover(WINDOW_IMAGE_TYPE_e,quint8,bool)));
        connect(m_windows[windowIndex],
                SIGNAL(sigSwapWindow(quint8,quint8)),
                this,
                SLOT(slotSwapWindow(quint8,quint8)));
        connect(m_windows[windowIndex],
                SIGNAL(sigAPCenterBtnClicked(quint8, quint16)),
                this,
                SLOT(slotAPCenterBtnClicked(quint8, quint16)));

        m_windows[windowIndex]->setGeometry(QRect(0,0,0,0));
    }

    setLayoutStyle(FOUR_X_FOUR, 0);
}

LayoutCreator::~LayoutCreator()
{
    for(quint8 windowIndex = 0; windowIndex < MAX_WINDOWS; windowIndex++)
    {
        disconnect(m_windows[windowIndex],
                   SIGNAL(sigWindowSelected(quint8)),
                   this,
                   SLOT(slotWindowSelected(quint8)));
        disconnect(m_windows[windowIndex],
                   SIGNAL(sigLoadMenuListOptions(quint8)),
                   this,
                   SLOT(slotLoadMenuListOptions(quint8)));
        disconnect(m_windows[windowIndex],
                   SIGNAL(sigWindowDoubleClicked(quint8)),
                   this,
                   SLOT(slotWindowDoubleClicked(quint8)));
        disconnect(m_windows[windowIndex],
                   SIGNAL(sigEnterKeyPressed(quint8)),
                   this,
                   SLOT(slotEnterKeyPressed(quint8)));
        disconnect(m_windows[windowIndex],
                   SIGNAL(sigWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint8)),
                   this,
                   SLOT(slotWindowImageClicked(WINDOW_IMAGE_TYPE_e,quint8)));
        disconnect(m_windows[windowIndex],
                   SIGNAL(sigWindowImageHover(WINDOW_IMAGE_TYPE_e,quint8,bool)),
                   this,
                   SLOT(slotWindowImageHover(WINDOW_IMAGE_TYPE_e,quint8,bool)));

        if(m_windowType == WINDOW_TYPE_LAYOUT)
        {
            disconnect(m_windows[windowIndex],
                       SIGNAL(sigAPCenterBtnClicked(quint8,quint16)),
                       this,
                       SLOT(slotAPCenterBtnClicked(quint8,quint16)));
        }

        delete m_windows[windowIndex];
    }

    for(quint8 windowIndex = 0; windowIndex < m_proxyWindowCount; windowIndex++)
    {
        DELETE_OBJ(m_proxyWindow[windowIndex]);
    }

    delete m_layoutBg;
}

void LayoutCreator::setLayoutStyle(LAYOUT_TYPE_e newLayout, quint16 newSelectedWindow)
{
    quint8 proxyWindowIndex = 0;
    quint8 rowSpaning, colSpaning, rowIndex, colIndex;
    quint16 actualWindowIndex;
    quint16 currentPage;
    quint8 oldSelectedWindow;

    if(m_windowType == WINDOW_TYPE_DISPLAYSETTINGS)
    {
        ApplController* applcontroller = ApplController::getInstance ();
        applcontroller->readMaxWindowsForDisplay (m_maxWindows);
    }

    if((newLayout != m_selectedLayout) && (newLayout < MAX_LAYOUT) && (newLayout >= 0))
    {
        //delete previous layout
        if(m_selectedLayout != MAX_LAYOUT)
        {
            currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
            actualWindowIndex = (currentPage * windowPerPage[m_selectedLayout]);
            for(quint8 windowIndex = 0; windowIndex < windowPerPage[m_selectedLayout]; windowIndex++, actualWindowIndex++)
            {
                if(actualWindowIndex < m_maxWindows)
                {
                    m_layoutBg->removeWidget(m_windows[windowIndex]);
                    m_windows[windowIndex]->setGeometry(0, 0, 0, 0);
                    m_windows[windowIndex]->setIsEnabled(false);
                }
                else
                {
                    m_layoutBg->removeWidget(m_proxyWindow[proxyWindowIndex]);
                    DELETE_OBJ(m_proxyWindow[proxyWindowIndex]);
                    proxyWindowIndex++;
                    m_proxyWindowCount--;
                }
            }
        }

        //create new layout
        proxyWindowIndex = 0;
        currentPage = (newSelectedWindow / windowPerPage[newLayout]);
        actualWindowIndex = (currentPage * windowPerPage[newLayout]);
        for(quint8 windowIndex = 0; windowIndex < windowPerPage[newLayout]; windowIndex++, actualWindowIndex++)
        {
            if (windowIndex >= MAX_WINDOWS)
            {
                break;
            }

            rowSpaning = windowInfo[newLayout][windowIndex][ROW_SPANING];
            colSpaning = windowInfo[newLayout][windowIndex][COL_SPANING];
            rowIndex = windowInfo[newLayout][windowIndex][ROW_INDEX];
            colIndex = windowInfo[newLayout][windowIndex][COL_INDEX];

            if (actualWindowIndex >= m_maxWindows)
            {
                m_proxyWindow[proxyWindowIndex] = new ProxyLayoutWindow(this);
                m_layoutBg->addWidget(m_proxyWindow[proxyWindowIndex++], rowIndex, colIndex, rowSpaning, colSpaning);
                m_proxyWindowCount++;
                continue;
            }

            m_windows[windowIndex]->setAPToolbarSize(apToolbarBtnSize[newLayout][windowIndex]);
            if((rowSpaning == 6) || (colSpaning == 6) || (rowSpaning == 8) || (colSpaning == 8))
            {
                m_windows[windowIndex]->setWindowIconType(ICON_TYPE_8X8);
            }
            else if((rowSpaning == 2) || (rowSpaning == 3) || (colSpaning == 2) || (colSpaning == 3))
            {
                m_windows[windowIndex]->setWindowIconType(ICON_TYPE_4X4);
            }
            else if((rowSpaning == 4) || (colSpaning == 4) || (rowSpaning == 5) || (colSpaning == 5))
            {
                m_windows[windowIndex]->setWindowIconType(ICON_TYPE_5X5);
            }
            else
            {
                m_windows[windowIndex]->setWindowIconType(ICON_TYPE_3X3);
            }

            m_windows[windowIndex]->setPbtoolbarSize(pbToolbarSizeType[newLayout][windowIndex]);
            m_layoutBg->addWidget(m_windows[windowIndex],
                                  rowIndex,
                                  colIndex,
                                  rowSpaning,
                                  colSpaning);
            m_windows[windowIndex]->setIsEnabled(true);
            m_windows[windowIndex]->setWindowNumber(actualWindowIndex);
        }
    }
    else
    {
        if((newLayout >= 0) && (newLayout < MAX_LAYOUT) && (m_selectedLayout >= 0) && (m_selectedLayout < MAX_LAYOUT))
        {
            currentPage = (newSelectedWindow / windowPerPage[newLayout]);
            if((currentPage != (m_selectedWindow / windowPerPage[m_selectedLayout])) || (m_windowType == WINDOW_TYPE_DISPLAYSETTINGS))
            {
                proxyWindowIndex = 0;
                actualWindowIndex = (currentPage * windowPerPage[newLayout]);
                for(quint8 windowIndex = 0; windowIndex < windowPerPage[newLayout]; windowIndex++, actualWindowIndex++)
                {
                    if (windowIndex >= MAX_WINDOWS)
                    {
                        break;
                    }

                    rowSpaning = windowInfo[newLayout][windowIndex][ROW_SPANING];
                    colSpaning = windowInfo[newLayout][windowIndex][COL_SPANING];
                    rowIndex = windowInfo[newLayout][windowIndex][ROW_INDEX];
                    colIndex = windowInfo[newLayout][windowIndex][COL_INDEX];

                    if(actualWindowIndex >= m_maxWindows)
                    {
                        if(m_layoutBg->indexOf(m_proxyWindow[proxyWindowIndex]) == -1)
                        {
                            m_layoutBg->removeWidget(m_windows[windowIndex]);
                            m_windows[windowIndex]->setGeometry(0, 0, 0, 0);
                            m_windows[windowIndex]->setIsEnabled(false);

                            m_proxyWindow[proxyWindowIndex] = new ProxyLayoutWindow(this);
                            m_layoutBg->addWidget(m_proxyWindow[proxyWindowIndex++], rowIndex, colIndex, rowSpaning, colSpaning);
                            m_proxyWindowCount++;
                        }
                        continue;
                    }

                    if(m_layoutBg->indexOf(m_windows[windowIndex]) == -1)
                    {
                        m_layoutBg->removeWidget(m_proxyWindow[proxyWindowIndex]);
                        DELETE_OBJ(m_proxyWindow[proxyWindowIndex]);
                        proxyWindowIndex++;

                        m_windows[windowIndex]->setAPToolbarSize(apToolbarBtnSize[newLayout][windowIndex]);
                        if((rowSpaning == 6) || (colSpaning == 6) || (rowSpaning == 8) || (colSpaning == 8))
                        {
                            m_windows[windowIndex]->setWindowIconType(ICON_TYPE_8X8);
                        }
                        else if((rowSpaning == 2) || (rowSpaning == 3) || (colSpaning == 2) || (colSpaning == 3))
                        {
                            m_windows[windowIndex]->setWindowIconType(ICON_TYPE_4X4);
                        }
                        else if((rowSpaning == 4) || (colSpaning == 4) || (rowSpaning == 5) || (colSpaning == 5))
                        {
                            m_windows[windowIndex]->setWindowIconType(ICON_TYPE_5X5);
                        }
                        else
                        {
                            m_windows[windowIndex]->setWindowIconType(ICON_TYPE_3X3);
                        }

                        m_windows[windowIndex]->setPbtoolbarSize(pbToolbarSizeType[newLayout][windowIndex]);
                        m_layoutBg->addWidget(m_windows[windowIndex], rowIndex, colIndex, rowSpaning, colSpaning);
                        m_windows[windowIndex]->setIsEnabled(true);
                        m_proxyWindowCount--;
                    }
                    m_windows[windowIndex]->setWindowNumber(actualWindowIndex);
                }
            }
        }
    }

    if((m_selectedLayout < MAX_LAYOUT) && (m_selectedLayout >= 0))
    {
        oldSelectedWindow = (m_selectedWindow % windowPerPage[m_selectedLayout]);
        m_windows[oldSelectedWindow]->deselectWindow();
    }
    if((newLayout >= 0) && (newLayout < MAX_LAYOUT))
    {
        actualWindowIndex = (newSelectedWindow % windowPerPage[newLayout]);
        m_windows[actualWindowIndex]->selectWindow();
        m_selectedWindow = newSelectedWindow;
        m_selectedLayout = newLayout;
    }

    emit sigChangePageFromLayout();
}

bool LayoutCreator::isNextPageAllProxyWindow()
{
    quint16 currentPage =(m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 actualWindowIndex = (currentPage * windowPerPage[m_selectedLayout]);
    quint8 proxyWindowCount = 0;

    for(quint8 windowIndex = 0; windowIndex < windowPerPage[m_selectedLayout]; windowIndex++, actualWindowIndex++)
    {
        if(actualWindowIndex >= m_maxWindows)
        {
            proxyWindowCount++;
        }
        else
        {
            proxyWindowCount--;
        }
    }

    if(proxyWindowCount == windowPerPage[m_selectedLayout])
    {
        return true;
    }

    return false;
}

void LayoutCreator::updateWindowData(quint16 windowIndex)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
        m_windows[actualWindowIndex]->updateWindowData(windowIndex);
    }
}

void LayoutCreator::updateWindowHeaderText(quint16 windowIndex, QString windowHeader)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
        m_windows[actualWindowIndex]->updateWindowHeaderText (windowHeader);
    }
}

void LayoutCreator::setWinHeaderForDispSetting(quint16 windowIndex, QString camName)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
        m_windows[actualWindowIndex]->setWinHeaderTextForDispSetting(camName);
    }
}

void LayoutCreator::getWindowDimensionInfo(quint16 windowIndex, WIN_DIMENSION_INFO_t *info)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
        info->winStartx = m_windows[actualWindowIndex]->x();
        info->winStarty = m_windows[actualWindowIndex]->y();
        info->winWidth = m_windows[actualWindowIndex]->width();
        info->winHeight = m_windows[actualWindowIndex]->height();
    }
}

quint8 LayoutCreator::getPbtoolbarSize(quint16 windowIndex)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
    }
    return m_windows[actualWindowIndex]->getPbToolbarSize();
}

WINDOW_ICON_TYPE_e LayoutCreator::getWindowIconType(quint16 windowIndex)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
    }
    return m_windows[actualWindowIndex]->getWindowIconType();
}

void LayoutCreator::changeSelectedWindow(quint16 windowIndex, bool isFocusToWindow)
{
    quint8 actualWindowIndex = 0;
    quint8 oldActualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout < MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
        oldActualWindowIndex = m_selectedWindow % windowPerPage[m_selectedLayout];
    }
    if(m_selectedWindow != windowIndex)
    {
        m_windows[oldActualWindowIndex]->deselectWindow();
        isFocusToWindow = m_windows[oldActualWindowIndex]->hasFocus();
        m_selectedWindow = windowIndex;
        m_windows[actualWindowIndex]->selectWindow();
    }

    if(isFocusToWindow)
    {
        giveFocusToWindow();
    }
}

void LayoutCreator::giveFocusToWindow()
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout < MAX_LAYOUT))
    {
        actualWindowIndex = (m_selectedWindow % windowPerPage[m_selectedLayout]);
    }
    m_windows[actualWindowIndex]->forceActiveFocus();
}

qint8 LayoutCreator::findNeighbourOfWindow(WIN_NEIGHBOUR_TYPE_e neighbourType)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT) && (actualWindowIndex < MAX_WINDOWS) && (neighbourType < MAX_NEIGHBOUR))
    {
        actualWindowIndex = m_selectedWindow % windowPerPage[m_selectedLayout];
        return (windowNeighbours[m_selectedLayout][actualWindowIndex][neighbourType]);
    }
    else
    {
        return -1;
    }
}

bool LayoutCreator::naviagtionInLayoutCreator(WIN_NEIGHBOUR_TYPE_e neighbourType)
{
    qint8 neighbourWindowIndex = findNeighbourOfWindow(neighbourType);

    if((neighbourWindowIndex >= 0) && (m_selectedLayout >= 0) && (m_selectedLayout < MAX_LAYOUT))
    {
        if(m_windows[neighbourWindowIndex]->getIsEnabled())
        {
            quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
            quint16 newSelectedWindow = ((currentPage * windowPerPage[m_selectedLayout]) + neighbourWindowIndex);
            changeSelectedWindow(newSelectedWindow, true);
            emit sigWindowSelected(newSelectedWindow);
        }
        return true;
    }

    if ((neighbourType == BOTTOM_NEIGHBOUR) || (neighbourType == TOP_NEIGHBOUR))
    {
        return true;
    }

    return false;
}

void LayoutCreator::changeWindowType(quint16 windowIndex, WINDOW_TYPE_e windowType)
{
    if((windowType != WINDOW_TYPE_SYNCPLAYBACK) && (m_selectedLayout >= 0) && (m_selectedLayout < MAX_LAYOUT))
    {
        quint8 actualWindowIndex;
        if ((windowIndex < MAX_WINDOWS) && (WINDOW_TYPE_SYNCPLAYBACK == m_windows[windowIndex]->getWindowType()))
        {
            /* For SyncPlayback we need to update window type irrespective of Layout */
            actualWindowIndex = windowIndex;
        }
        else
        {
            actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
        }

        if(actualWindowIndex < MAX_WINDOWS)
        {
            m_windows[actualWindowIndex]->changeWindowType(windowType);
        }
    }
    else
    {
        if(windowIndex < MAX_WINDOWS)
        {
            m_windows[windowIndex]->changeWindowType(windowType);
        }
    }
}

void LayoutCreator::updateImageMouseHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover)
{
    quint8 actualWindowIndex = 0;
    if(m_selectedLayout< MAX_LAYOUT)
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
    }
    m_windows[actualWindowIndex]->updateImageMouseHover(imageType, isHover);
}

void LayoutCreator::updateSequenceImageType(quint16 windowIndex, WINDOWSEQUENCE_IMAGE_TYPE_e configImageType)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
    }
    m_windows[actualWindowIndex]->updateSequenceImageType(configImageType);
}

void LayoutCreator::forceFocusToPage(bool isFirstElement)
{
    quint16 window[2];
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    Layout::getFirstAndLastWindow(currentPage, m_selectedLayout, window);
    if(isFirstElement)
    {
        changeSelectedWindow(window[0], true);
    }
    else
    {
        changeSelectedWindow(window[1], true);
    }
}

void LayoutCreator::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
            event->accept();
            naviagtionInLayoutCreator(LEFT_NEIGHBOUR);
            break;

        case Qt::Key_Right:
            event->accept();
            naviagtionInLayoutCreator(RIGHT_NEIGHBOUR);
            break;

        case Qt::Key_Up:
            event->accept();
            naviagtionInLayoutCreator(TOP_NEIGHBOUR);

            break;

        case Qt::Key_Down:
            event->accept();
            naviagtionInLayoutCreator(BOTTOM_NEIGHBOUR);
            break;

        default:
            event->accept();
            break;
    }
}

void LayoutCreator::mousePressEvent(QMouseEvent *event)
{
    m_isDragEvent = false;
    m_firstWindowIndex = m_secondWindowIndex = MAX_CHANNEL_FOR_SEQ;
    m_firstWindowIndex = LayoutWindow::getFirstClickedWindow();

    if(m_firstWindowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
        quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + m_firstWindowIndex;
        m_firstWindowIndex = arrayWindowIndex;
        m_mouseClicked = true;
    }
    Q_UNUSED(event);
}

void LayoutCreator::mouseReleaseEvent(QMouseEvent *event)
{
    emit sigDragStartStopEvent(false);
    m_mouseClicked = false;
    Q_UNUSED(event);
}

void LayoutCreator::mouseMoveEvent(QMouseEvent *event)
{
    if((m_mouseClicked) && (m_isDragEvent == false))
    {
        m_isDragEvent = true;
        emit sigDragStartStopEvent(true);
    }
    else
    {
        QWidget::mouseMoveEvent(event);
    }
}

QWidget *LayoutCreator::window(quint16 windowIndex)
{
    quint8 actualWindowIndex = 0;
    if((m_selectedLayout >= 0) && (m_selectedLayout< MAX_LAYOUT))
    {
        actualWindowIndex = (windowIndex % windowPerPage[m_selectedLayout]);
    }
    return m_windows[actualWindowIndex];
}

void LayoutCreator::setMaxWinodws(quint16 maxWind)
{
    m_maxWindows = maxWind;
}

void LayoutCreator::slotWindowSelected(quint8 windowIndex)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigWindowSelected(arrayWindowIndex);
    emit sigUpdateCurrentElement(m_indexInPage);
}

void LayoutCreator::slotLoadMenuListOptions(quint8 windowIndex)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigLoadMenuListOptions(arrayWindowIndex);
}

void LayoutCreator::slotWindowDoubleClicked(quint8 windowIndex)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigWindowDoubleClicked(arrayWindowIndex);
}

void LayoutCreator::slotEnterKeyPressed(quint8 windowIndex)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigEnterKeyPressed(arrayWindowIndex);
}

void LayoutCreator::slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint8 windowIndex)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigWindowImageClicked(imageType, arrayWindowIndex);
}

void LayoutCreator::slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint8 windowIndex, bool isHover)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigWindowImageHover(imageType, arrayWindowIndex, isHover);
}

void LayoutCreator::slotSwapWindow(quint8 firstWindow, quint8 secondWindow)
{
    if(false == m_isDragEvent)
    {
        m_mouseClicked = false;
        return;
    }

    m_firstWindowIndex = m_secondWindowIndex = MAX_CHANNEL_FOR_SEQ;
    if(firstWindow < MAX_WINDOWS)
    {
        quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
        quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + firstWindow;
        m_firstWindowIndex = arrayWindowIndex;
    }

    if(secondWindow < MAX_WINDOWS)
    {
        quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
        quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + secondWindow;
        m_secondWindowIndex = arrayWindowIndex;
    }

    if((m_firstWindowIndex < MAX_CHANNEL_FOR_SEQ) && (m_secondWindowIndex < MAX_CHANNEL_FOR_SEQ))
    {
        bool m_firstWindowImgStatus = false;
        bool m_secondWindowImgStatus = false;

        quint8 winIdForFirst = (m_firstWindowIndex % windowPerPage[m_selectedLayout]);
        quint8 winIdForSecond = (m_secondWindowIndex % windowPerPage[m_selectedLayout]);

        // For windowtype DISPLAY Setting not getting windowstatus
        if((winIdForFirst != winIdForSecond) && (m_windowType == WINDOW_TYPE_LAYOUT))
        {
            m_firstWindowImgStatus = m_windows[winIdForFirst]->getApFeatureStatus();
            m_secondWindowImgStatus = m_windows[winIdForSecond]->getApFeatureStatus();
        }

        if((m_firstWindowIndex != m_secondWindowIndex) && ((m_firstWindowImgStatus == false) && (m_secondWindowImgStatus == false)))
        {
            emit sigSwapWindows(m_firstWindowIndex, m_secondWindowIndex);
        }
        else
        {
            emit sigDragStartStopEvent(false);
        }
    }
    else if(m_firstWindowIndex < MAX_CHANNEL_FOR_SEQ)
    {
        emit sigDragStartStopEvent(false);
    }

    m_isDragEvent = false;
    m_mouseClicked = false;
}
void LayoutCreator::slotAPCenterBtnClicked(quint8 index, quint16 windowIndex)
{
    quint16 currentPage = (m_selectedWindow / windowPerPage[m_selectedLayout]);
    quint16 arrayWindowIndex = (currentPage * windowPerPage[m_selectedLayout]) + windowIndex;
    emit sigAPCenterBtnClicked(index,arrayWindowIndex);
}

void LayoutCreator::updateApToolbar(quint16 windowIndex)
{
    quint8 winId = (windowIndex % windowPerPage[m_selectedLayout]);
    m_windows[winId]->updateApToolbar(windowIndex);
}

void LayoutCreator::updateAPNextVideoText(quint16 windowIndex)
{
    quint8 winId = (windowIndex % windowPerPage[m_selectedLayout]);
    m_windows[winId]->updateAPNextVideoText(windowIndex);
}
