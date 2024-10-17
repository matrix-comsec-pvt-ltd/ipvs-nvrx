#ifndef ENUMFILE
#define ENUMFILE

#include <QObject>
#include "DebugLog.h"

#define IMAGE_PATH                          ":/Images"
#define HW_TEST_REPORT_PATH                 LOG_DIR_PATH "/"
#define MOUNT_PATH                          MEDIA_DIR_PATH "/"

#define NORMAL_BKG_COLOR                    "#252525"
#define BORDER_2_COLOR                      "#050505"
#define BORDER_1_COLOR                      "#404040"
#define TIME_BAR_COLOR                      "#696868"
#define CLICKED_BKG_COLOR                   "#1a1a1a"
#define HEADER_BG_COLOR                     "#2c2c2c"
#define BODY_BG_COLOR                       "#1c1c1c"
#define LEFT_PANEL_BG_COLOR                 "#242424"
#define DISABLE_FONT_COLOR                  "#606060"
#define HIGHLITED_FONT_COLOR                "#528dc9"
#define SEPERATOR_LINE_COLOR                "#2d2d2d"
#define NORMAL_FONT_COLOR                   "#c8c8c8"
#define MAIN_HEADING_FONT_COLOR             "#b3b3b3"
#define HEADING_FONT_COLOR                  "#757575"
#define SUFFIX_FONT_COLOR                   "#828182"
#define MOUSE_HOWER_COLOR                   "#65affa"
#define GREEN_COLOR                         "#0c822d"
#define RED_COLOR                           "#990303"

#define NORMAL_FONT_FAMILY                  "Century Gothic"
#define WINDOW_FONT_FAMILY                  "LetterGothicStd"

#define LAN1_DEV_LINK_STS_FILE              "/sys/class/net/eth0/carrier"
#define LAN2_DEV_LINK_STS_FILE              "/sys/class/net/eth1/carrier"

#if defined(OEM_JCI)
#define ENTERPRISE_STRING                   "HOLIS ENTERPRISE"
#else
#define ENTERPRISE_STRING                   "SATATYA"
#endif

#define TEST_REPORT_FOLDER                  "NVR_HARDWARE_TEST_REPORT"

#define DELETE_OBJ(obj)      if(obj != NULL){ delete obj;obj = NULL;}
#define IS_VALID_OBJ(obj)    (obj == NULL) ? false : true
#define INIT_OBJ(obj)        (obj = NULL);

#define SYSTEM_COMMAND_SIZE                 500
#define NORMAL_FONT_SIZE                    15
#define SUFFIX_FONT_SIZE                    14
#define SMALL_SUFFIX_FONT_SIZE              12
#define TEST_HEADING_FONT_SIZE              16

const QString asciiset1ValidationStringWithoutSpace = QString("[") + QString("a-zA-Z0-9\\-_.,():@!#$*+\\[\\]/\\\\") + QString("]");

typedef enum
{
    CENTER_X_CENTER_Y,
    START_X_START_Y,
    START_X_CENTER_Y,
    START_X_END_Y,
    END_X_END_Y,
    END_X_CENTER_Y,
    END_X_START_Y,
    CENTER_X_END_Y,
    CENTER_X_START_Y,
    MAX_POINT_PARAM_TYPE
}POINT_PARAM_TYPE_e;

typedef enum
{
    IMAGE_TYPE_NORMAL,
    IMAGE_TYPE_MOUSE_HOVER,
    IMAGE_TYPE_CLICKED,
    IMAGE_TYPE_DISABLE,
    MAX_IMAGE_TYPE
}IMAGE_TYPE_e;

const QString imgTypePath [6] ={"Button_1.png",
                                "Button_2.png",
                                "Button_3.png",
                                "Button_4.png",
                                "Button_5.png",
                                "Button_6.png"};

#endif // ENUMFILE

