///////////////////////////////////////////////////////////////////////////
//
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : SATATYA DEVICES
//   Owner        : Kaushal Patel
//   File         : MxCommandFiels.h
//   Description  : This file contains enum of command fields.
/////////////////////////////////////////////////////////////////////////////


#ifndef COMMANDFIELDS
#define COMMANDFIELDS

typedef enum{

    MX_CMD_FIELDS_CAMERA_STATUS,
    MX_CMD_FIELDS_CAM_NUMBER,
    MX_CMD_FIELDS_CAM_NAME,
    MX_CMD_FIELDS_IPV4_ADDRESS,
    MX_CMD_FIELDS_IPV6_ADDRESS,
    MX_CMD_FIELDS_HTTP_PORT,
    MX_CMD_FIELDS_BRAND_NAME,
    MX_CMD_FIELDS_MODEL_NAME,
    MX_CMD_FIELDS_ONVIF_SUPPORT,
    MX_CMD_FIELDS_ONVIF_PORT,

    MAX_MX_CMD_CAM_SEARCH_FIELDS

}MX_CMD_CAM_SEARCH_FIELDS_e;

typedef enum {

    MX_CMD_AUTO_ADD_FEILDS_CAM_STATUS,
    MX_CMD_AUTO_ADD_FEILDS_CAM_NAME,
    MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS,
    MX_CMD_AUTO_ADD_FEILDS_HTTP_PORT,
    MX_CMD_AUTO_ADD_FEILDS_BRAND_NAME,
    MX_CMD_AUTO_ADD_FEILDS_MODEL_NAME,
    MX_CMD_AUTO_ADD_FEILDS_ONVIF_SUPPORT,
    AUTO_ADD_FEILDS_ONVIF_PORT,
    MAX_MX_CMD_AUTO_ADD_FIELDS

}MX_CMD_CAM_SEARCH_AUTO_ADD_FIELDS_e;

typedef enum{

    MX_CMD_CAM_LIST_MAC_ADDRESS,
    MX_CMD_CAM_LIST_IP_ADDRESS,
    MX_CMD_CAM_LIST_MODEL_NAME,

    MAX_MX_CMD_CAM_LIST_FIELDS

}MX_CMD_AUTOADD_CAM_LIST_FIELDS_e;

#endif // COMMANDFIELDS

