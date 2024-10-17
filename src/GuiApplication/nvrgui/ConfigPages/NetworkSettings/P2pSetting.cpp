#include "P2pSetting.h"

/* The MARGIN indicates the gap between two consecutive elements */
#define HEIGHT_MARGIN                   SCALE_HEIGHT(40)

/* The STATUS_IMAGE_HEIGHT indicates the height of the status image */
#define STATUS_IMAGE_HEIGHT             SCALE_HEIGHT(22)

/* Defines the location of image for status */
#define STATUS_CONNECTED                ":/Images_Nvrx/StatusIcon/Connected.png"
#define STATUS_DISCONNECTED             ":/Images_Nvrx/StatusIcon/Disconnected.png"
#define STATUS_DISABLED                 ":/Images_Nvrx/StatusIcon/Disabled.png"

/* All the elements image height */
#define TOTAL_ELEMENTS_HEIGHT           SCALE_HEIGHT(455)

/* Defines the path of the Qr code from where it will be saved and checked */
#define QRCODE_PATH                     "/tmp/QrCode.png"

/* The path of IOS and Android Image */
#define IOS_IMAGE_PATH                  ":/Images_Nvrx/QR/iOS_app.png"
#define ANDROID_IMAGE_PATH              ":/Images_Nvrx/QR/Android_app.png"

/* Used for PNG generation */
#define INCHES_PER_METER                (100.0/2.54)

/* Define to request the LAN 1 MAC address */
#define LAN1                            1

/* Warning message for relay server */
#define RELAY_SERVER_WARNING_NOTICE     "If P2P connection fails, Matrix Cloud Server will be used. Do you want to continue?"

/* List of controls */
typedef enum
{
    P2P_ENABLE_CHECKBOX,
    P2P_RELAY_SERVER_FALLBACK_CHECKBOX,
    MAX_P2P_STG_ELEMETS

}P2P_STG_ELELIST_e;

/* List of config controls */
typedef enum
{
    P2P_ENABLE,
    P2P_RELAY_SERVER_FALLBACK,
    MAX_P2P_STG_FIELDS

}P2P_STG_FIELDS_e;

/**
 * @brief P2pSetting::P2pSetting
 * @param devName
 * @param parent
 */
P2pSetting::P2pSetting(QString devName, QWidget *parent) : ConfigPageControl(devName, parent, MAX_P2P_STG_ELEMETS)
{
    m_devName = devName;
    m_macAddrString = "";    
    m_qr = NULL;
    m_macQr = NULL;
    m_iosQr = NULL;
    m_androidQr = NULL;
    m_macLabel = NULL;
    m_applLabelAndroid = NULL;
    m_applLabelIos = NULL;
    m_androidAppLabel = NULL;
    m_iosAppLabel = NULL;
    m_enableP2p = NULL;
    m_p2pRelayServerFallback = NULL;

    /* Creating all the default elements while page is opened */
    createDefaultComponent();

    /* Get the restored configuration from server */
    P2pSetting::getConfig();
}

/**
 * @brief This function draws all the default components while the page is opened first time
 */
void P2pSetting::createDefaultComponent()
{
    /* This draws the enable p2p checkbox */
    m_enableP2p = new OptionSelectButton((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                         SCALE_HEIGHT(15),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CHECK_BUTTON_INDEX,
                                         this,COMMON_LAYER,
                                         "Enable P2P","",-1,
                                         P2P_ENABLE_CHECKBOX,
                                         true,-1,
                                         SUFFIX_FONT_COLOR,false);

    /* For the keyboard events and page focus functionality defined in configPageControl Class */
    m_elementList[P2P_ENABLE_CHECKBOX] = m_enableP2p;
    connect(m_enableP2p,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));

    /* This draws the Fallback to Relay Server checkbox */
    m_p2pRelayServerFallback = new OptionSelectButton(m_enableP2p->x(),
                                                      m_enableP2p->y() + m_enableP2p->height(),
                                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      this, COMMON_LAYER,
                                                      "Fallback to Relay Server", "", -1,
                                                      P2P_RELAY_SERVER_FALLBACK_CHECKBOX,
                                                      true, -1,
                                                      SUFFIX_FONT_COLOR, false);

    m_elementList[P2P_RELAY_SERVER_FALLBACK_CHECKBOX] = m_p2pRelayServerFallback;
    connect(m_p2pRelayServerFallback,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));

    /* This element draws the border and the label to display status */
    m_statusLabel = new ElementHeading(m_p2pRelayServerFallback->x(),
                                       m_p2pRelayServerFallback->y() + m_p2pRelayServerFallback->height(),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       "Status",
                                       COMMON_LAYER, this,
                                       false, SCALE_WIDTH(170),NORMAL_FONT_SIZE,false,NORMAL_FONT_COLOR);

    /* Draws the image of tick or cross over the the label */
    m_statusImage = new Image(m_statusLabel->x() + m_statusLabel->width()/2,
                              m_statusLabel->y() + (m_statusLabel->height() - STATUS_IMAGE_HEIGHT)/2,
                              STATUS_DISABLED,
                              this,
                              START_X_START_Y,
                              0,
                              false,
                              true);
}

/**
 * @brief This function displays all the QRs and their respective labels
 */
void P2pSetting::createQrComponents(void)
{
    QString androidString = "Android Application";
    QString iosString = "iOS Application";
    QString applString = "SATATYA SIGHT";

    /* Draws Mac Qr and its label*/
    m_macQr = new DrawQr(m_p2pRelayServerFallback->x(),
                         m_statusLabel->y() + m_statusLabel->height() + HEIGHT_MARGIN,
                         QRCODE_PATH,
                         this,m_macAddrString,
                         (m_devName == LOCAL_DEVICE_NAME) ? true : false);

    m_macLabel = new ElementHeading(m_p2pRelayServerFallback->x() - SCALE_WIDTH(15),
                                    m_macQr->y() + m_macQr->height(),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    m_macAddrString,
                                    NO_LAYER,
                                    this,false,0,
                                    NORMAL_FONT_SIZE,false,NORMAL_FONT_COLOR);

    /* Draws Android QR and its labels */
    m_androidQr = new DrawQr(m_p2pRelayServerFallback->x() + m_p2pRelayServerFallback->width() - QR_IMAGE_WIDTH,
                             m_macQr-> y(),
                             ANDROID_IMAGE_PATH,
                             this);

    m_applLabelAndroid = new ElementHeading(m_androidQr->x() - SCALE_WIDTH(4),
                                            m_macLabel->y(),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            applString,
                                            NO_LAYER,
                                            this,false,0,
                                            NORMAL_FONT_SIZE,false,NORMAL_FONT_COLOR);

    m_androidAppLabel= new ElementHeading(m_applLabelAndroid->x() - SCALE_WIDTH(25),
                                          m_applLabelAndroid->y() + m_applLabelAndroid->height() - BGTILE_HEIGHT/2,
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          androidString,
                                          NO_LAYER,
                                          this,false,0,
                                          NORMAL_FONT_SIZE,false,NORMAL_FONT_COLOR);

    /* Draws iOS QR and its labels */
    m_iosQr = new DrawQr(m_p2pRelayServerFallback->x() + m_p2pRelayServerFallback->width() - QR_IMAGE_WIDTH,
                         m_applLabelAndroid-> y() + m_androidQr->height(),
                         IOS_IMAGE_PATH,
                         this);

    m_applLabelIos = new ElementHeading(m_iosQr->x() - SCALE_WIDTH(4),
                                        m_iosQr->y() + m_iosQr->height(),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        applString ,
                                        NO_LAYER,
                                        this,false,0,
                                        NORMAL_FONT_SIZE,false,NORMAL_FONT_COLOR);

    m_iosAppLabel = new ElementHeading(m_applLabelIos->x() - SCALE_WIDTH(6),
                                       m_applLabelIos->y() + m_applLabelIos->height() - BGTILE_HEIGHT/2,
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       iosString ,
                                       NO_LAYER,
                                       this,false,0,
                                       NORMAL_FONT_SIZE,false,NORMAL_FONT_COLOR);
}

/**
 * @brief P2pSetting::~P2pSetting
 */
P2pSetting::~P2pSetting(void)
{
    disconnect(m_enableP2p,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));
    disconnect(m_p2pRelayServerFallback,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));
    DELETE_OBJ (m_iosAppLabel);
    DELETE_OBJ (m_applLabelIos);
    DELETE_OBJ (m_iosQr);
    DELETE_OBJ (m_androidAppLabel);
    DELETE_OBJ (m_applLabelAndroid);
    DELETE_OBJ (m_androidQr);
    DELETE_OBJ (m_macLabel);
    DELETE_OBJ (m_macQr);
    DELETE_OBJ (m_statusImage);
    DELETE_OBJ (m_statusLabel);
    DELETE_OBJ (m_enableP2p);
    DELETE_OBJ (m_p2pRelayServerFallback);
    if(NULL != m_qr)
    {
        QRcode_free(m_qr);
    }
}

/**
 * @brief Redraws the file if not found at the required path
 */
void P2pSetting::validateFileAndRegenerate()
{    
    /* Validates whether this file exists or not */
    QFileInfo pngFile(QString(QRCODE_PATH));
    if(pngFile.exists() == false)
    {
        WPRINT(CONFIG_PAGES,"qr code png file not present");
        if(m_macAddrString != "")
        {
            /* The QRcode_encodeString func generates a bit array of the given string */
            m_qr = QRcode_encodeString(m_macAddrString.toLocal8Bit().constData(),1,QR_ECLEVEL_L,QR_MODE_8,1);
            GeneratePNG(m_qr,QRCODE_PATH);
        }
    }    
}

/**
 * @brief P2pSetting::getMacAddress
 */
void P2pSetting::getMacAddress()
{
    payloadLib->setCnfgArrayAtIndex (0,LAN1);
    sendCommand(GET_MAC,1);
}

/**
 * @brief P2pSetting::getP2pStatus
 */
void P2pSetting::getP2pStatus()
{
    if (m_enableP2p->getCurrentState() == ON_STATE)
    {
        payloadLib->setCnfgArrayAtIndex(0, STATUS_CMD_ID_P2P);
        sendCommand(GET_STATUS, 1);
    }
    else
    {
        /* when samas checkbox is disabled make image disabled */
        m_statusImage->updateImageSource(STATUS_DISABLED, false);
    }
}

/**
 * @brief P2pSetting::sendCommand
 * @param cmdType
 * @param totalfeilds
 */
void P2pSetting:: sendCommand(SET_COMMAND_e cmdType, int totalfeilds)
{
    QString payloadString = payloadLib->createDevCmdPayload(totalfeilds);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;

    /* Load the process bar while waiting for the request response */
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

/**
 * @brief P2pSetting::createPayload
 * @param msgType
 */
void P2pSetting::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             P2P_TABLE_INDEX,
                                                             P2P_RELAY_SERVER_FALLBACK,
                                                             P2P_RELAY_SERVER_FALLBACK,
                                                             P2P_RELAY_SERVER_FALLBACK,
                                                             MAX_P2P_STG_FIELDS,
                                                             MAX_P2P_STG_FIELDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    /* Load the process bar while waiting for the request response */
    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

/**
 * @brief P2pSetting::getConfig
 */
void P2pSetting::getConfig(void)
{
    createPayload(MSG_GET_CFG);
}

/**
 * @brief P2pSetting::defaultConfig
 */
void P2pSetting::defaultConfig(void)
{
    createPayload(MSG_DEF_CFG);
}

/**
 * @brief P2pSetting::saveConfig
 */
void P2pSetting::saveConfig(void)
{
    payloadLib->setCnfgArrayAtIndex(P2P_ENABLE, m_enableP2p->getCurrentState());
    payloadLib->setCnfgArrayAtIndex(P2P_RELAY_SERVER_FALLBACK, m_p2pRelayServerFallback->getCurrentState());
    createPayload(MSG_SET_CFG);
}

/**
 * @brief Responce from server taken care here
 * @param param
 * @param deviceName
 */
void P2pSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName != currDevName)
    {
        return;
    }

    processBar->unloadProcessBar();
    if (param->deviceStatus != CMD_SUCCESS)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);
            if(payloadLib->getcnfgTableIndex () != P2P_TABLE_INDEX)
            {
                break;
            }

            m_enableP2p->changeState((OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex(P2P_ENABLE).toUInt()));
            m_p2pRelayServerFallback->changeState((OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex(P2P_RELAY_SERVER_FALLBACK).toUInt()));
            m_enableP2p->getCurrentState() == ON_STATE ? m_p2pRelayServerFallback->setIsEnabled(true) : m_p2pRelayServerFallback->setIsEnabled(false);

            /* if QR code is not drawn */
            if (m_macQr == NULL)
            {
                /* Get MAC address */
                getMacAddress();
            }

            /* Get p2p status */
            getP2pStatus();
        }
        break;

        case MSG_SET_CFG:
        {
            /* load info page with msg */
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));

            /* Get p2p status */
            getP2pStatus();
        }
        break;

        case MSG_DEF_CFG:
        {
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
                case GET_MAC:
                {
                    payloadLib->parseDevCmdReply(true, param->payload);
                    m_macAddrString = payloadLib->getCnfgArrayAtIndex(0).toString().toUpper();

                    /* Validate file and regenerate if not present */
                    validateFileAndRegenerate();

                    /* Displays the QR codes */
                    createQrComponents();
                }
                break;

                case GET_STATUS:
                {
                    payloadLib->parseDevCmdReply(true, param->payload);
                    if (STATUS_CMD_ID_P2P != (STATUS_CMD_ID_e)payloadLib->getCnfgArrayAtIndex(0).toUInt())
                    {
                        /* It is not for P2P status */
                        break;
                    }

                    quint8 status = (payloadLib->getCnfgArrayAtIndex(1).toUInt());
                    m_statusImage->updateImageSource((status == SUCCESS) ? STATUS_CONNECTED : STATUS_DISCONNECTED, true);
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

/**
 * @brief This function creates a PNG file to the assigned location
 * @param qrcode
 * @param outfile
 */
void P2pSetting::GeneratePNG(const QRcode *qrcode, const char *outfile)
{
    int size = 3;
    int margin = 1;
    int dpi = 72;

    unsigned char fg_color[4] = {0, 0, 0, 255};
    unsigned char bg_color[4] = {255, 255, 255, 255};

    static FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
	png_color palette[2];
    png_byte alpha_values[2];
    unsigned char *row, *dataPtr, *rowPtr;
    int colLoop, rowLoop, colData, rowData, bit;
    int realwidth;

    do
    {
        realwidth = (qrcode->width + margin * 2) * size;
        row = (unsigned char *)malloc((realwidth + 7) / 8);
        if(row == NULL)
        {
            EPRINT(CONFIG_PAGES,"fail to alloc memory");
            break;
        }

        fp = fopen(outfile, "wb");
        if(fp == NULL)
        {
            EPRINT(CONFIG_PAGES,"fail to open file: [path=%s]", outfile);
            break;
        }

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png_ptr == NULL)
        {
            EPRINT(CONFIG_PAGES,"fail to create png writer");
            break;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if(info_ptr == NULL)
        {
            EPRINT(CONFIG_PAGES,"fail to create png info");
            break;
        }

        if(setjmp(png_jmpbuf(png_ptr)))
        {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            EPRINT(CONFIG_PAGES,"fail to write png image");
            break;
        }

        palette[0].red = fg_color[0];
        palette[0].green = fg_color[1];
        palette[0].blue = fg_color[2];
        palette[1].red = bg_color[0];
        palette[1].green = bg_color[1];
        palette[1].blue = bg_color[2];
        alpha_values[0] = fg_color[3];
        alpha_values[1] = bg_color[3];
        png_set_PLTE(png_ptr, info_ptr, palette, 2);
        png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr,
                     realwidth, realwidth,
                     1,
                     PNG_COLOR_TYPE_PALETTE,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_DEFAULT,
                     PNG_FILTER_TYPE_DEFAULT);

        png_set_pHYs(png_ptr, info_ptr,
                     dpi * INCHES_PER_METER,
                     dpi * INCHES_PER_METER,
                     PNG_RESOLUTION_METER);
        png_write_info(png_ptr, info_ptr);

        /* top margin */
        memset(row, 0xff, (realwidth + 7) / 8);
        for(rowLoop = 0; rowLoop < margin * size; rowLoop++)
        {
            png_write_row(png_ptr, row);
        }

        /* data */
        dataPtr = qrcode->data;
        for(rowLoop = 0; rowLoop < qrcode->width; rowLoop++)
        {
            memset(row, 0xff, (realwidth + 7) / 8);
            rowPtr = row;
            rowPtr += margin * size / 8;
            bit = 7 - (margin * size % 8);
            for(colLoop = 0; colLoop < qrcode->width; colLoop++)
            {
                for(colData = 0; colData < size; colData++)
                {
                    *rowPtr ^= (*dataPtr & 1) << bit;
                    bit--;
                    if(bit < 0)
                    {
                        rowPtr++;
                        bit = 7;
                    }
                }
                dataPtr++;
            }
            for(rowData = 0; rowData < size; rowData++)
            {
                png_write_row(png_ptr, row);
            }
        }

        /* bottom margin */
        memset(row, 0xff, (realwidth + 7) / 8);
        for(rowLoop = 0; rowLoop < margin * size; rowLoop++)
        {
            png_write_row(png_ptr, row);
        }

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        DPRINT(CONFIG_PAGES,"Successfully generated PNG File");

	}while(0);

    if(fp != NULL)
        fclose(fp);
    if(row != NULL)
        free(row);
}

void P2pSetting::slotOptionButtonSelected(OPTION_STATE_TYPE_e currentState, int indexInPage)
{
    if (indexInPage == P2P_ENABLE)
    {
        m_p2pRelayServerFallback->setIsEnabled(currentState);
        if (currentState == OFF_STATE)
        {
            /* If p2p flag disabled then also disabled the fallback relay server flag */
            m_p2pRelayServerFallback->changeState(OFF_STATE);
        }
    }
    else if (indexInPage == P2P_RELAY_SERVER_FALLBACK)
    {
        if (currentState == ON_STATE)
        {
            /* Take the consent from the user of relaying data through relay server */
            infoPage->loadInfoPage(RELAY_SERVER_WARNING_NOTICE, true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
        }
    }
}

void P2pSetting::handleInfoPageMessage(int index)
{
    if (index == INFO_CANCEL_BTN)
    {
        if (infoPage->getText() == Multilang(RELAY_SERVER_WARNING_NOTICE))
        {
            m_p2pRelayServerFallback->changeState(OFF_STATE);
        }
    }
}
