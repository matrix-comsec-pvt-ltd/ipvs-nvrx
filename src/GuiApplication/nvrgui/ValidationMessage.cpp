#include "ValidationMessage.h"


ValidationMessage* ValidationMessage::m_validationMessage = NULL;

ValidationMessage* ValidationMessage::getInstance()
{
    if(m_validationMessage == NULL)
    {
        m_validationMessage = new ValidationMessage();
    }
    return m_validationMessage;
}

QString ValidationMessage::getValidationMessage(quint32 messageEnum)
{
    return m_validationMessage->loadValidationMessage(messageEnum);
}

QString ValidationMessage::loadValidationMessage(quint32 messageEnum){
    if (m_validationMapStr.contains(messageEnum))
    {
        return m_validationMapStr.value(messageEnum);
    }
    else
    {
        return m_validationMapStr.value(MAX_VALIDATION_MESSAGE);
    }
}

QString ValidationMessage::getDeviceResponceMessage(quint32 messageResponce)
{
    return m_validationMessage->loadDeviceResponceMessage(messageResponce);
}

QString ValidationMessage::loadDeviceResponceMessage(quint32 messageResponce)
{
    if (m_deviceResponceMapStr.contains(messageResponce))
    {
        return m_deviceResponceMapStr.value(messageResponce);
    }
    else
    {
        return m_deviceResponceMapStr.value(CMD_MAX_DEVICE_REPLY);
    }
}
ValidationMessage::ValidationMessage()
{
    //AdativeRecording
    m_validationMapStr[ADAPTREC_EN_ADAPT_REC]                               ="Enabling Adaptive Recording will record only key frames.Configure event and alarm recording to record all frames.";
    //AdvanceCamSearch
    m_validationMapStr[ADV_CAM_SEARCH_STR_END_DIFF]                         ="Start and End IP Address in range should be different";
    m_validationMapStr[ADV_CAM_SEARCH_STR_END_SAME]                         ="Start and End IP Address must be in same subnet";
    //CameraOSDSetting
    m_validationMapStr[CAM_OSD_SETTING_CANNOT_SET_BOTH_OSD_VALUE]           ="Both OSD values cannot be set on same position";
    m_validationMapStr[CAM_OSD_SETTING_ENT_TEXT]                            ="Please enter Text";
    //Camera Search
    m_validationMapStr[CAM_SRCH_CAM_ADD_SUCCESS]                            ="Camera added successfully on index :" + QString(" ");
    m_validationMapStr[CAM_SRCH_AUTO_CONFI]                                 ="Auto configure request received for selected cameras";
    m_validationMapStr[CAM_SRCH_ADD_CAM_REQ_FAIL]                           ="Add Camera Request Failed";
    m_validationMapStr[CAM_SRCH_SELECT_CAM_TO_AUTO_CONFI]                   ="Please select camera(s) to auto configure";
    //Camera Setting
    m_validationMapStr[CAM_SETTING_CAM_NAME]                                ="Please enter Camera Name";
    m_validationMapStr[CAM_SETTING_CONF_CAM_BEF_PROCESS]                    ="Please configure camera before processing";
    m_validationMapStr[CAM_SETTING_MOTION_AREA_SET]                         ="Motion area set Successfully";
    m_validationMapStr[CAM_SETTING_PRIV_AREA_SET]                           ="Privacy area set successfully";
    //IpAddressChang
    m_validationMapStr[IP_ADD_CHANGE_ENT_VALID_SUBNET_MASK]                 ="Please enter valid Subnet Mask";
    m_validationMapStr[IP_ADD_CHANGE_ENT_VALID_DEFAULT_GATEWAY]             ="Please enter valid Default Gateway";
    m_validationMapStr[IP_ADD_CHANGE_IP_ADDR_AND_DEF_GATEWAY_SAME_SUBNET]   ="IP Address and Default Gateway must be in the same subnet";
    //IpCameraSetting
    m_validationMapStr[IP_CAM_SET_ENT_VALID_BRAND_NM]                       ="Please select Brand name";
    m_validationMapStr[IP_CAM_SET_ENT_MODEL_NM]                             ="Please select Model name";
    m_validationMapStr[IP_CAM_SET_CAM_ADD_LEFT_BLK]                         ="Please Enter Camera address";
    m_validationMapStr[IP_CAM_SET_ENT_VALID_PASS]                           ="Please Enter valid Password";
    //IpStreamSetting
    m_validationMapStr[IP_STRM_SET_VIDEO_ENCODE]                            ="Please Enter Video Encoding";
    m_validationMapStr[IP_STRM_SET_RESOLUTION]                              ="Please Enter Resolution";
    m_validationMapStr[IP_STRM_SET_PROFILE_SUCCESS]                         ="Profile created successfully";
    m_validationMapStr[IP_STRM_SET_PROFILE_ERROR]                           ="Cannot save same stream profile in Sub and Main Stream";
    //PresetTour
    m_validationMapStr[PRESET_TOUR_POSTN_AGANT_TWO_NM]                      ="Minimum two Preset Positions : Mandatory";
    //PrivacyMaskSetting
    m_validationMapStr[PRIVACY_MASK_SETTING]                                ="Overall Privacy Mask Area is beyond the limit";
    //PtzSchedule
    m_validationMapStr[PTZSCHD_TOUR_OVERLAP]                                ="Tours cannot be overlapped";
    //Login.cpp
    m_validationMapStr[LOGIN_USER_NAME_ERROR]                               ="Please Enter Username";
    m_validationMapStr[LOGIN_PASSWORD_ERROR]                                ="Please Enter Password";
    m_validationMapStr[LOGIN_INCORRECT_PASS_ERROR]                          ="Incorrect Current Password";
    m_validationMapStr[LOGIN_NEW_PASS_BLANK_ERROR]                          ="Please enter New Password";
    m_validationMapStr[LOGIN_CONFM_PASS_ERROR]                              ="Please enter Confirm Password";
    m_validationMapStr[LOGIN_PASS_MISMATCH_ERROR]                           ="Confirm Password should be same as New Password";
    m_validationMapStr[LOGIN_CURRNT_PASS_ERROR]                             ="Please enter Current Password";
    m_validationMapStr[LOGIN_SUCCESS_MSG]                                   ="Successfully Logged in ";
    m_validationMapStr[LOGIN_LOGOUT_SUCCESS_MSG]                            ="Successfully Logged out ";
    m_validationMapStr[LOGIN_PASS_EXPIRE_ONE_DAY]                           ="Password will expire in 1 day. ";
    //BlockUser.cpp
    m_validationMapStr[BLOCK_USER_SUCCESS_UNBLOCK]                          ="User(s) unblocked successfully";
    //SystemControl.cpp
    m_validationMapStr[SYSTEM_CONTROL_RESTART_MESSAGE]                      ="Do you really want to restart the system?";
    m_validationMapStr[SYSTEM_CONTROL_SHUTDOWN_MESSAGE]                     ="Do you really want to shut down the system?";
    m_validationMapStr[SYSTEM_CONTROL_FACTORY_RESTORE_MESSAGE]              ="Continue with a reboot?";
    //OnlineUser.cpp
    m_validationMapStr[ONLINE_USER_SUCCESS_BLOCK]                           ="User(s) blocked successfully/User(s) unblocked successfully";
    //ModifyPassword.cpp
    m_validationMapStr[MODIFY_PASS_CHANGE_SUCCESS]                          ="Password changed Successfully!";
    m_validationMapStr[MODIFY_PASS_ENT_OLD_PASS]                            ="Please enter Old Password";
    m_validationMapStr[MODIFY_PASS_ENT_NEW_PASS]                            ="Please enter New Password";
    m_validationMapStr[MODIFY_PASS_INCORRECT_PASS]                          ="Please enter valid old password.";
    //syncPlaybackCropAndBackup
    m_validationMapStr[SYNC_PB_CROP_AND_BACKUP]                             ="Please Stop the Export first!";
    //SyncPlayback.cpp
    m_validationMapStr[SYNC_PB_CLIP_MAKE_STARTED]                           ="Clip Making Started";
    m_validationMapStr[SYNC_PB_CLIP_MAKE_STOPPED]                           ="Clip Making Stopped";
    m_validationMapStr[SYNC_PB_DEL_CLIP]                                    ="Delete Some Clips Maximum Clip Limit Reached";
    m_validationMapStr[SYNC_PB_SEL_MIN_EVENT]                               ="Select At least 1 Event for Search!";
    m_validationMapStr[SYNC_PB_SEL_MIN_CAM]                                 ="Select At least 1 Camera for Search!";
    //PlayBackSearch.cpp
    m_validationMapStr[PB_SRCH_MORE_REC_MSG]                                ="More records available.";
    m_validationMapStr[PB_SRCH_MAX_BKUP_MSG]                                ="Sorry You Can't Include More Than 10 Records";
    m_validationMapStr[PB_SRCH_BKUP_MIN_MSG]                                ="Please select atleast one record";
    m_validationMapStr[PB_SRCH_BKUP_DEV_ERR_MSG]                            ="Manual Backup Device Not Found!";
    m_validationMapStr[PB_SRCH_FILE_COPIED_SUCCESS]                         ="File copied successfully";
    m_validationMapStr[PB_SRCH_FILE_COPY_FAIL]                              ="File Copy Failed";

    //QUICK BACKUP//
    m_validationMapStr[QUICK_BKUP_DEV_ERR_MSG]                              ="Backup Location not found";
    m_validationMapStr[QUICK_BKUP_DISK_FULL_MSG]                            ="Error in taking backup. Disk is full";
    m_validationMapStr[QUICK_BKUP_FILE_COPY_FAIL]                           ="File Copy Failed";
    m_validationMapStr[QUICK_BKUP_ON_LOGOUT_MSG]                            ="Please close Quick Backup window";

    //common
    m_validationMapStr[ENT_IP_ADDR_RANGE]                                   ="Please enter IP Address Range";
    m_validationMapStr[SAVE_CHANGES]                                        ="Do you want to save the changes?";
    m_validationMapStr[MAINTAIN_CHANGE]                                     ="Do you want to Maintain Changes?";
    m_validationMapStr[GREATER_START_TM]                                    ="End Time Should be greater than Start Time";
    m_validationMapStr[SCHD_OVRLAP_NT_ALW]                                  ="Schedule overlapping is not allowed";
    m_validationMapStr[FIVE_MIN_DIFF]                                       ="Minimum five minute difference is required between start and end time";
    m_validationMapStr[ENT_EMAIL_ADD]                                       ="Please Enter Email Address";
    m_validationMapStr[ENT_SUBJECT]                                         ="Please Enter Subject";
    m_validationMapStr[ENT_MESS]                                            ="Please enter Message";
    m_validationMapStr[ENT_VAILD_EMAIL_ADD]                                 ="Please Enter Valid Email Address.";
    m_validationMapStr[ENT_FIRST_ALPH]                                      ="First character should be alphanumeric";
    m_validationMapStr[SUCCESS_SAVE_MSG]                                    ="Configuration saved successfully";
    m_validationMapStr[ENT_VALID_USER]                                      ="Please enter valid Username";
    m_validationMapStr[ENT_VALID_MOB_NM]                                    ="Please Enter Valid Mobile Number";
    m_validationMapStr[ENT_VALID_MOB_NM1]                                   ="Please Enter Valid Mobile Number 1";
    m_validationMapStr[ENT_VALID_MOB_NM2]                                   ="Please Enter Valid Mobile Number 2";
    m_validationMapStr[ENT_PULSE_PERIOD_DEF_RANGE]                          ="Please enter Pulse Period in defined range";
    m_validationMapStr[SEQUENCING_ON_MSG]                                   ="Please stop Auto Page Navigation to perform any action";
    m_validationMapStr[WINDOW_SEQUENCING_ON_MSG]                            ="Please stop Window wise sequencing to perform any action";
    m_validationMapStr[WINDOW_EXPANDING_ON_MSG]                             ="Collapse the window to perform any action";
    m_validationMapStr[PB_RUNNING_MESSAGE]                                  ="Please stop the Playback to perform any action";
    m_validationMapStr[ENT_USER_NM]                                         ="Please Enter Username";
    m_validationMapStr[ENT_PASSWORD]                                        ="Please Enter Password";
    m_validationMapStr[ENT_VALID_IP_ADD]                                    ="Please enter valid IP Address";
    m_validationMapStr[START_IP_ADDR_LESS_END_IP_ADDR]                      ="Start IP Address should be less than End IP Address.";
    m_validationMapStr[ENT_IP_ADDR]                                         ="Please enter IP Address";
    m_validationMapStr[ENT_SUBNET_MASK]                                     ="Please Enter Subnet Mask.";
    m_validationMapStr[ENT_DEFAULT_GATEWAY]                                 ="Please Enter Default Gateway.";
    m_validationMapStr[ENT_PREFIX_LEN]                                      ="Please enter valid Prefix Length";
    m_validationMapStr[PASSWORD_RANGE_ERROR]                                ="Please Enter Password Between 4-16 Characters";
    m_validationMapStr[ENT_CONFIRM_PASS]                                    ="Please enter Confirm Password";
    m_validationMapStr[PASS_MISMATCH]                                       ="Password Mismatch";
    m_validationMapStr[START_END_DATE_ERR_MSG]                              ="Start Date-Time should be less than End Date-Time";
    m_validationMapStr[DATE_DIFF_ERR_MSG]                                   ="Time period must not be more than 30 days";
    m_validationMapStr[HTTP_PORT_RANGE]                                     ="Please Enter HTTP port in defined range";
    m_validationMapStr[DEV_NAME]                                            ="Please Enter Device Name";
    m_validationMapStr[ENT_HOSTNAME]                                        ="Please Enter Host Name";
    m_validationMapStr[BUZZER_CONTROL_STOP]                                 ="Buzzer Stopped";
    m_validationMapStr[MSG_SIZE_MAX_LIMIT]                                  ="Message size exceeded max limit";
    m_validationMapStr[ENT_VALUE_DEFI_RANGE]                                ="Please enter value in defined range";
    m_validationMapStr[NO_FREE_CHNL_AVAILABLE]                              ="No free channel available";
    m_validationMapStr[SETTING_SAVE_SUCCESS]                                ="Settings Saved Successfully";
    m_validationMapStr[ERROR_SAVE_MSG]                                      ="Error While Saving Settings";
    m_validationMapStr[START_CHAR_ERROR_MSG]                                ="First Character should be Alphabet";
    m_validationMapStr[DEVICE_FIRMWARE_MISMATCH_SERVER_OLD]                 ="Firmware mismatch detected. Please upgrade firmware";
    m_validationMapStr[DEVICE_FIRMWARE_MISMATCH_SERVER_NEW]                 ="Firmware mismatch detected. Please upgrade network device firmware";

    // Validation message for BasicSettings->AutoConfigureCamera
    m_validationMapStr[AUTO_CONF_IP_STR_END_DIFF]                           ="Start and End IP Address cannot be same";
    m_validationMapStr[AUTO_CONF_IP_STR_END_SAME]                           ="Start and End IP Address must be in same subnet";
    m_validationMapStr[AUTO_CONF_NEW_CAM]                                   ="This IP Address range will be assigned to newly searched camera(s), Are you sure you want to continue?";
    m_validationMapStr[AUTO_CONF_MAIN_GOP]                                  ="Please enter Main-Stream GOP in defined range";
    m_validationMapStr[AUTO_CONF_SUB_GOP]                                   ="Please enter Sub-Stream GOP in defined range";
    m_validationMapStr[AUTO_CONF_IP_TYPE_DIFF]                              ="Start and End IP Address family must be same";

    // Validation message for BasicSettings->DateTimeSetting
    m_validationMapStr[DATE_TIME_SET_SUCCESS]                               ="Date and Time set Successfully";
    m_validationMapStr[DATE_TIME_SPFY_NTP_SRVER]                            ="Please Specify NTP Server";
    m_validationMapStr[DATE_TIME_CNG_CFG_VIDEO_STD]                         ="Changing the configuration will affect the Video Standard Continue with a reboot?";

    // Validation message for BasicSettings->DayLightSaving
    m_validationMapStr[DAY_LIGHT_DIFF_FRWD_REVRSE]                          ="Please enter Different values for Forward and Reverse Clock";

    // Validation message for BasicSettings->GeneralSetting
    m_validationMapStr[GEN_SETT_DEV_RANGE_NO]                               ="Please Enter Device Number in defined range";
    m_validationMapStr[GEN_SETT_SINGLE_FILE_RECORD]                         ="Enter Single File Record Duration within defined range";
    m_validationMapStr[GEN_SETT_TCP_PORT_RANGE]                             ="Please Enter TCP port in defined range";
    m_validationMapStr[GEN_SETT_HTTP_TCP_DIFF]                              ="HTTP port and TCP port should not be identical.";
    m_validationMapStr[GEN_SETT_TCP_NOT_IN_RTP_RANGE]                       ="TCP port should not be in range of RTP port";
    m_validationMapStr[GEN_SETT_HTTP_NOT_IN_RTP_RANGE]                      ="HTTP port should not be in range of RTP port";
    m_validationMapStr[GEN_SETT_INVALID_RTP_START]                          ="Please enter valid even RTP Start Port value";
    m_validationMapStr[GEN_SETT_RTP_RANGE_START_END]                        ="End RTP Port must be greater than Start RTP Port";
    m_validationMapStr[GEN_SETT_VALUE_64_DIFF]                              ="Please enter port value with 64 ports difference.";
    m_validationMapStr[GEN_SETT_CFG_CHANGE_RESTART_SYS]                     ="Changing the configuration will reboot the device. Continue?";
    m_validationMapStr[GEN_SETT_SAMAS_PORT_IN_RANGE]                        ="Please enter SAMAS Port in defined range";
    m_validationMapStr[GEN_SETT_VIDEO_POP_UP_RANGE]                         ="Please enter Video pop-up Duration in defined range";
    m_validationMapStr[GEN_SETT_PRE_VIDEO_LOSS_RANGE]                       ="Please enter Pre Video Loss Duration in defined range";
    m_validationMapStr[GEN_SETT_FORWARD_TCP_PORT_RANGE]                     ="Please Enter Forwarded TCP port in defined range";
    m_validationMapStr[GEN_SETT_FORWARD_TCP_NOT_IN_RTP_RANGE]               ="Forwarded TCP port should not be in range of RTP port";
    m_validationMapStr[GEN_SETT_AUTO_ADD_CAM_TCP_PORT]                      ="Please Enter TCP port in defined range";
    m_validationMapStr[GEN_SETT_AUTO_ADD_CAM_POLL_DURATION]                 ="Please enter valid Poll Duration in defined range";
    m_validationMapStr[GEN_SETT_AUTO_ADD_CAM_POLL_INTERVAL]                 ="Please enter valid Poll Interval in defined range";

    //Validation message for NetworkSettings->BroadbandSettings
    m_validationMapStr[BROAD_SETT_ENT_APN]                                  ="Please enter APN";
    m_validationMapStr[BROAD_SETT_SAVE_CURRENT_CHANGES]                     ="Do you want to save the changes?";
    m_validationMapStr[BROAD_SETT_PROF_ALREADY_EXISTS]                      ="Profile Name already exists";

    //Validation message for NetworkSettings->DDNSClient
    m_validationMapStr[DDNS_CLIENT_UPDATED_SUCCESS]                         ="DDNS Settings updated Successfully";
    m_validationMapStr[DDNS_CLIENT_UPDATE_INT_IN_RANGE]                     ="Please enter Update Interval in range";

    //Validation message for NetworkSettings->EmailClient
    m_validationMapStr[EMAIL_CLIENT_MAIL_SENT_SUCCESS]                      ="Mail Sent Successfully";
    m_validationMapStr[EMAIL_CLIENT_ENT_MAIL_SERVER]                        ="Please enter Mail Server";
    m_validationMapStr[EMAIL_CLIENT_ENT_PORT]                               ="Please Enter Port";
    m_validationMapStr[EMAIL_CLIENT_SENDER_EMAIL_ID]                        ="Please enter Sender's Email ID";
    m_validationMapStr[EMAIL_CLIENT_RECEIVER_EMAIL_ID]                      ="Please enter Receiver's Email ID";

    //Validation message for NetworkSettings->FtpClient
    m_validationMapStr[FTP_CLIENT_FILE_UPLOAD_SUCCESS]                      ="File Uploaded Successfully";
    m_validationMapStr[FTP_CLIENT_ENT_FTP_SERVER_ADDR]                      ="Please enter Valid FTP Server Address";
    m_validationMapStr[FTP_CLIENT_ENT_CORRECT_ADDR]                         ="Please enter correct Address";

    //Validation message for NetworkSettings->IpFiltering
    m_validationMapStr[IP_FILTER_DELETE_IP_RANGE]                           ="Are you sure to delete the selected IP ranges?";
    m_validationMapStr[IP_FILTER_START_OR_START_END_IP_ADDR]                ="Please enter Start IP address or Start and End IP Range";
    m_validationMapStr[IP_FILTER_START_END_IP_ADDR]                         ="Please enter Start and End IP Address";
    m_validationMapStr[IP_FILTER_RANGE]                                     ="This IP filter range already exist";
    m_validationMapStr[IP_FILTER_START_IP_LESS_THAN_END_IP]                 ="Start IP Address must be less than End IP Address";
    m_validationMapStr[IP_FILTER_ENABLE_MODE]                               ="IP Range : Mandatory";
    m_validationMapStr[IP_FILTER_ENT_SAME_IP_VERSION]                       ="Please enter the IP range of same versions";

    //Validation message for NetworkSettings->Lan1Setting, Lan2Setting
    m_validationMapStr[LAN_SETT_ENT_CORRECT_IP_ADDR]                        ="Enter Correct IP Address";
    m_validationMapStr[LAN_SETT_ENT_CORRECT_SUBNET_MASK]                    ="Enter Correct Subnet Mask";
    m_validationMapStr[LAN_SETT_ENT_CORRECT_GATEWAY_MASK]                   ="Invalid Default Gateway";
    m_validationMapStr[LAN_SETT_ENT_CORRECT_PREF_DNS_ADDR]                  ="Please enter valid Preferred DNS";
    m_validationMapStr[LAN_SETT_ENT_CORRECT_ALTR_DNS_ADDR]                  ="Please enter valid Alternate DNS";
    m_validationMapStr[LAN_SETT_ENT_GATEWAY_MASK]                           ="Please Enter Default Gateway.";
    m_validationMapStr[LAN_SETT_ENT_PPPoE_USERNAME]                         ="Please enter PPPoE Username";
    m_validationMapStr[LAN_SETT_ENT_PPPoE_PASSWORD]                         ="Please enter PPPoE Password";
    m_validationMapStr[LAN_SETT_ENT_PREF_DNS_VALUE]                         ="Please Enter Preferred DNS value.";

    //Validation message for NetworkSettings->MatrixDnsClient
    m_validationMapStr[MATRIX_DNS_REG_SUCCESS]                              ="Registration Successful";
    m_validationMapStr[MATRIX_DNS_REG_HOST_NAME]                            ="Please register Host name first";
    m_validationMapStr[MATRIX_DNS_HOST_NAME_ATLEAST_3_CHAR]                 ="Host Name should have atleast 3 Characters";
    m_validationMapStr[MATRIX_DNS_ENT_VALID_PORT]                           ="Please enter valid Port";

    //Validation message for NetworkSettings->MediaFileAccess
    m_validationMapStr[MEDIA_FILE_ENT_FTP_PORT]                             ="Please enter FTP Port within the defined range";

    //Validation message for NetworkSettings->SmsSetting
    m_validationMapStr[SMS_SETT_ENT_VALID_MOBILE_NO]                        ="Please Enter Valid Mobile Number";
    m_validationMapStr[SMS_SETT_ENT_MOBILE_NO]                              ="Please Enter Mobile Number";
    m_validationMapStr[SMS_SETT_ENT_SENDER_ID]                              ="Please Enter Sender ID";
    m_validationMapStr[SMS_SETT_MESSAGE_SENT_SUCCESS]                       ="Your Message has been Successfully sent";

    //Validation message for NetworkSettings->StaticRouting
    m_validationMapStr[STATIC_ROUT_SELECT_VALID_SUBNET]                     ="Please select Subnet Mask";
    m_validationMapStr[STATIC_ROUT_SELECT_VALID_INTERFACE]                  ="Please select Exit Interface";
    m_validationMapStr[STATIC_ROUT_ENT_NW_ADDR]                             ="Please enter Network Address";

    //Validation message for NetworkSettings->TcpClient
    m_validationMapStr[TCP_CLIENT_NOTI_SENT_SUCCESS]                        ="TCP Notification Sent Successfully";
    m_validationMapStr[TCP_CLIENT_ENT_SERVER_VALUE]                         ="Please Enter TCP Server Value";

    //Validation message for Layout
    m_validationMapStr[LAYOUT_STREAM_LIMIT_ERROR_MESSAGE]                   ="Maximum stream limit reached.";
    m_validationMapStr[LAYOUT_NO_FREE_WINDOW_ERROR_MESSAGE]                 ="Free Window Not Available";
    m_validationMapStr[LAYOUT_PB_ERROR_MESSAGE]                             ="Please stop playback to start auto page navigation";
    m_validationMapStr[LAYOUT_WINDOW_SEQENC_ERROR_MESSAGE]                  ="Please stop window sequencing and retry";
    m_validationMapStr[LAYOUT_SEQ_STOP_FOR_MAIN_WINDOW]                     ="Window wise sequencing stopped for Main Display";
    m_validationMapStr[LAYOUT_MANUAL_REC_START]                             ="Manual Recording Started";
    m_validationMapStr[LAYOUT_MANUAL_REC_STOP]                              ="Manual Recording Stopped";
    m_validationMapStr[LAYOUT_PTZ_NOT_SUPPORTED]                            ="PTZ not supported by Camera";

    // DEVICE//
    m_validationMapStr[DEV_SETTING_ENT_VAL_DEV_NAME]                        ="Please enter valid Device name.";
    m_validationMapStr[DEV_SETTING_ENT_PORT_DEFI_RANGE]                     ="Please enter Port in defined range";
    m_validationMapStr[DEV_SETTING_PASS_MIN_4_CHAR]                         ="Password should contain minimum 4 characters";
    m_validationMapStr[DEV_SETTING_ENT_MAC_ADDR]                            ="Please Enter MAC Address";
    m_validationMapStr[DEV_SETTING_DEV_NAME_MATCH_LOCAL]                    ="Device Name cannot be same as Local Device Name";
    m_validationMapStr[DEV_SETTING_ENT_DEV_NAME_ALR_EXIT]                   ="Device name already Exist";
    m_validationMapStr[DEV_SETTING_ENT_DEV_ALR_EXIT]                        ="Device has already been added";
    m_validationMapStr[DEV_SETTING_HOSTNAME_ATLEST_3_CHAR]                  ="Host Name should have atleast 3 Characters";
    m_validationMapStr[DEV_SETTING_DEL_DEVICE_CNFORM]                       ="Do you really want to delete the selected device?";
    m_validationMapStr[DEV_SETTING_END_CHAR_ERROR_MSG]                      ="Last Character should be alphanumeric in Device name";
    m_validationMapStr[DEV_SETTING_DEVICE_ADDED_SUCCESS]                    ="Added Successfully";
    m_validationMapStr[DEV_SETTING_DEVICE_MODIFIED_SUCCESS]                 ="Modified Successfully";
    m_validationMapStr[DEV_SETTING_DEVICE_DEL_SUCCESS]                      ="Deleted Successfully";
    m_validationMapStr[DEV_SETTING_ENT_FORWARDED_TCP_PORT_DEFI_RANGE]       ="Please Enter Forwarded TCP port in defined range";
    m_validationMapStr[DEV_SETTING_DEV_NAME_MATCH_DEFAULT_NAME]             ="Device Name cannot be same as default Local Device Name";

    // STORAGE BACKUP->HDD MANAGEMENT//
    m_validationMapStr[HDD_MANAGE_DO_REALY_WANT_FORMAT]                    ="Do you really want to format?";
    m_validationMapStr[HDD_MANAGE_ALL_REC_DATA_LOST_PLAYBACK_BACKUP_STOP]  ="Recorded data will be lost. Ongoing recording, playback and backup will be stopped. Continue?";
    m_validationMapStr[HDD_MANAGE_PROCESS_NOT_ABORTED_CHANGE_HDD]          ="It is not recommended to abort this process. This will change HDD status to fault. Disk may not be formatted. Continue?";
    m_validationMapStr[HDD_MANAGE_FORMAT_HDD_USE_MAZ_SIZE]                 ="Format the Hard disk to utilize maximum space";

    // STORAGE BACKUP->HDD GROUP MANAGEMENT//
    m_validationMapStr[HDD_GROUP_MANAGE_DISK_ALREADY_ASSIGN]               ="Selected Disk(s) already assigned to another Group. Do you want to assign it to this Group?";
    m_validationMapStr[HDD_GROUP_MANAGE_CAMERA_ALREADY_ASSIGN]             ="Selected Camera(s) already assigned to another Group. Do you want to assign it to this Group?";
    m_validationMapStr[HDD_GROUP_MANAGE_REC_STOP]                          ="All ongoing recording will stop, it will resume as per new configuration. Do you still want to continue?";

    // STORAGE BACKUP->NETWORK DRIVE //
    m_validationMapStr[NW_DRIVE_ENT_NAME]                                  ="Please enter Name";
    m_validationMapStr[NW_DRIVE_ENT_DFL_FOLDER]                            ="Please enter Default Folder";
    m_validationMapStr[NW_DRIVE_TEST_CONN_SUCCESS]                         ="Test connection successful";
    m_validationMapStr[NW_DRIVE_TEST_CONN_FAILED]                          ="Error in connection. Please try again later";

    // STORAGE BACKUP->STORAGE MANANGEMENT //
    m_validationMapStr[STOR_MANAGE_ENT_PERCENT_DEFI_RANGE]                 ="Please enter Percentage in defined range";
    m_validationMapStr[STOR_MANAGE_ENT_SPACE_VALUE_DEFI_RANGE]             ="Please enter Space value in defined range";

    // STORAGE BACKUP->USB MANANGEMENT //
    m_validationMapStr[USB_MANAGE_USB_EJECT_SUCCESS]                       ="USB ejected Successfully!";
    m_validationMapStr[USB_MANAGE_FORMAT_DEV]                              ="Do you really want to format the device?";
    m_validationMapStr[USB_MANAGE_UNPLUG_DEV]                              ="Do you really want to unplug the device?";
    m_validationMapStr[USB_MANAGE_STOP_BACKUP]                             ="Do you want to stop ongoing backup?";

    // USERACCOUNT SETTING->USERACCOUNT MANAGEMENT//
    m_validationMapStr[USR_ACC_MANAGE_MAX_LIMIT_USER]                      ="Maximum Limit is of 128 users";
    m_validationMapStr[USR_ACC_MANAGE_SURE_DELETE_USER]                    ="Are you sure to delete user?";
    m_validationMapStr[USR_ACC_MANAGE_USER_NAME_EXIST]                     ="Username already Exists!";
    m_validationMapStr[USR_ACC_MANAGE_PASS_LESS_THAN_MAX]                  ="Password should be less than 17 characters";
    m_validationMapStr[USR_ACC_MANAGE_ENT_LOGIN_LIM_DEFI_RANGE]            ="Please enter Login limit within range";
    m_validationMapStr[USR_ACC_MANAGE_ENT_LOGIN_LIM_DURATION]              ="Please enter Login limit duration";
    m_validationMapStr[USR_ACC_MANAGE_USER_DEL_SUCCESS]                    ="User deleted Successfully";
    m_validationMapStr[USR_ACC_MANAGE_USER_ADD_SUCCES]                     ="User Added Successfully";
    m_validationMapStr[USR_ACC_MANAGE_USER_MODIFY_SUCCESS]                 ="User Modified Successfully";

    // USER ACCOUNT MANAGEMENT SETTING->PUSH NOTIFICATIONS STATUS
    m_validationMapStr[PUSH_NOTIFICATIONS_DELETE_CLIENT]                   ="Are you sure you want to stop the push notifications for the selected client?";

    // MAIN WINDOW//
    m_validationMapStr[MAIN_WINDOW_INIT_MSG]                               ="Loading...";
    m_validationMapStr[MAIN_WINDOW_DEINIT_MSG]                             ="Shutting Down...";
    m_validationMapStr[MAIN_WINDOW_DISKCLEANUP_WARNING_MESSAGE]            ="Device will restart and may take several minutes";
    m_validationMapStr[MAIN_WINDOW_SNAPSHOT_SUCCESS]                       ="Snapshot taken successfully";
    m_validationMapStr[MAIN_WINDOW_LOGIN_EXPIRED]                          ="Your login session has expired";

    // TOOLBAR //
    m_validationMapStr[TOOLBAR_COLLAPSE_WINDOW_MESSAGE]                     ="Collapse the window to perform any action";
    m_validationMapStr[TOOLBAR_VIDEO_POPUP_WINDOW_MESSAGE]                  ="Exit the window to perform this action";

    // DISPLAY SETTING //
    m_validationMapStr[DISP_SETTING_CUST_SEQ_START_MSG]                     ="Do you want to save changes before start sequencing?";
    m_validationMapStr[DISP_SETTING_SAVE_MSG]                               ="Do you want to save the changes?";
    m_validationMapStr[DISP_SETTING_RES_KEEP_MSG]                           ="Do you want to keep changes?";
    m_validationMapStr[DISP_SETTING_MAIN_DISP_CHANGE]                       ="Changing the configuration will reboot the device. Continue?";
    m_validationMapStr[DISP_SETTING_CONFI_AUTO_PAGE_WINDOW_SEQ]             ="Configure either Auto Page Navigation or Window Sequencing";
    m_validationMapStr[DISP_SETTING_RESTART_UI_APPL_NOTICE]                 ="Relaunch UI application to apply changes?";

    // APPEARANCE SETTING //
    m_validationMapStr[APPEARENCE_SETTING_ERROR_DEFAULT_MSG]                ="Error While Default Settings!";
    m_validationMapStr[APPEARENCE_SETTING_DEFAULT_MSG]                      ="Are you sure you want to default the settings?";

    // PTZ CONTROL //
    m_validationMapStr[PTZ_CONTROL_ENT_POSITION_NAME]                       ="Please Enter Position Name";
    m_validationMapStr[PTZ_CONTROL_POSITION_SAVE_SUCCESS]                   ="Position Saved Successfully";
    m_validationMapStr[PTZ_CONTROL_POSITION_DEL_SUCCESS]                    ="Position Deleted Successfully";
    m_validationMapStr[PTZ_CONTROL_POSITION_NUM_CONGI]                      ="Position number is not configured";
    m_validationMapStr[PTZ_CONTROL_DEL_POSITION]                            ="Do you want to delete this Preset Position?";
    m_validationMapStr[PTZ_PRESET_POS_EMPTY]                                ="Mandatory:Preset Position";
    m_validationMapStr[PTZ_ALREADY_PRESENT_INDEX]                           ="Preset already present at selected index";
    m_validationMapStr[PTZ_PRESET_NAME_ALREADY_PRESENT]                     ="Position name already exists";

    // CONFIGURATION CONTROL
    m_validationMapStr[CONFI_CONTROL_DEFAULT_INFO_MSG]                      ="Are you sure you want to default the settings?";
    m_validationMapStr[CONFI_CONTROL_TEST_CONN_WARNG_MSG]                   ="Please save the changes and retry";
    m_validationMapStr[CONFI_CONTROL_DEVICE_DEFAULT_INFO_MSG]               ="Do you want to change all remote device status to default?";

    // TWO WAY AUDIO
    m_validationMapStr[TWO_WAY_CAM_AUD_STP_PRO_CLNT_AUD]                    ="Camera audio stopped. Processing client audio";
    m_validationMapStr[TWO_WAY_PLYBK_AUD_STP_PRO_CLNT_AUD]                  ="Playback audio stopped. Processing client audio";
    m_validationMapStr[TWO_WAY_PLYBK_AUD_REQ_FAIL_PRO_CLNT_AUD]             ="Playback audio request failed. Processing client audio";

    //LOCAL DECODING
    m_validationMapStr[LOCAL_DECODING_DISABLED]                             ="Live View disabled";
    m_validationMapStr[LOCAL_DECODING_DISABLED_CLICK_ACTION]                ="Enable 'Start Live View' from Basic Settings to perform this action";
    m_validationMapStr[AUTO_ADD_CAM_NO_CAM_SEL]                             ="Please select atleast one camera";

    m_validationMapStr[MAN_BACKUP_VALIDATION]                               ="Change in configuration will stop any ongoing backup. This will require the user to start backup again. Continue?";
    m_validationMapStr[MAN_BACKUP_SAVE_VALIDATION]                          ="Configurations changed Saving shall terminate any ongoing process. Continue?";
    m_validationMapStr[REC_BOTH_WARNING]                                    ="Enabling both recording format will consume more storage.Continue with a reboot?";
    m_validationMapStr[REC_AVI_WARNING]                                     ="Playback and Quick Backup will not function for AVI. Continue with a reboot?";
    m_validationMapStr[REC_FRM_NATIVE_TO_AVI]                               ="This might result in losing current hour's AVI recording Continue with a reboot?";
    m_validationMapStr[CHANGE_PREFEREED_LANGUAGE]                           ="Do you want to set this language as preferred language?";
    m_validationMapStr[FIMRWARE_UPGRADE_NOTE]                               ="Device Firmware will be upgraded shortly";

    //General settings
    m_validationMapStr[GEN_SETTING_LOCAL_NAME_MATCH_REMOTE]                 ="Local Device Name cannot be same as Remote Device Name";

    //MANAGE->RECOVERY PASSWORD
    m_validationMapStr[ENT_EMAIL_ID_OR_ALL_QA]                              ="Please configure Email ID or answer all questions";
    m_validationMapStr[ENT_ALL_QA]                                          ="Please answer all questions";
    m_validationMapStr[ENT_PWD_RST_OTP]                                     ="Please enter valid OTP";

    //Date and Time Settings
    m_validationMapStr[INVALID_DATE_RANGE]                                  ="Please Select Year between 2012 and 2037";
    m_validationMapStr[MAX_VALIDATION_MESSAGE]                              ="Invalid value";


    /* Device response message strings */
    m_deviceResponceMapStr[CMD_SUCCESS]                                     ="";
    m_deviceResponceMapStr[CMD_INVALID_MESSAGE]                             ="Error while connecting.";
    m_deviceResponceMapStr[CMD_INVALID_SESSION]                             ="Error while connecting.";
    m_deviceResponceMapStr[CMD_INVALID_SYNTAX]                              ="Error while connecting.";
    m_deviceResponceMapStr[CMD_IP_BLOCKED]                                  ="IP address is blocked.";
    m_deviceResponceMapStr[CMD_INVALID_CREDENTIAL]                          ="Incorrect User Name or Password";
    m_deviceResponceMapStr[CMD_USER_DISABLED]                               ="User Disabled ";
    m_deviceResponceMapStr[CMD_USER_BLOCKED]                                ="User Blocked";
    m_deviceResponceMapStr[CMD_MULTILOGIN]                                  ="Multi-login is not allowed.";
    m_deviceResponceMapStr[CMD_MAX_USER_SESSION]                            ="Maximum session limit reached";
    m_deviceResponceMapStr[CMD_RESOURCE_LIMIT]                              ="Resource not available.";
    m_deviceResponceMapStr[CMD_NO_PRIVILEGE]                                ="You do not have permissions for this operation.";
    m_deviceResponceMapStr[CMD_INVALID_TABLE_ID]                            ="Configuration mismatch.";
    m_deviceResponceMapStr[CMD_INVALID_INDEX_ID]                            ="Configuration mismatch.";
    m_deviceResponceMapStr[CMD_INVALID_FIELD_ID]                            ="Configuration mismatch.";
    m_deviceResponceMapStr[CMD_INVALID_INDEX_RANGE]                         ="Configuration mismatch.";
    m_deviceResponceMapStr[CMD_INVALID_FIELD_RANGE]                         ="Configuration mismatch.";
    m_deviceResponceMapStr[CMD_INVALID_FIELD_VALUE]                         ="Configuration mismatch.";
    m_deviceResponceMapStr[CMD_MAX_STREAM_LIMIT]                            ="Maximum stream limit reached.";
    m_deviceResponceMapStr[CMD_NO_PTZ_PROTOCOL]                             ="PTZ interface not configured.";
    m_deviceResponceMapStr[CMD_NO_RECORD_FOUND]                             ="No records found.";
    m_deviceResponceMapStr[CMD_SCHEDULE_TOUR_ON]                            ="Schedule tour active. Please try again later";
    m_deviceResponceMapStr[CMD_RECORDING_ON]                                ="Recording. Please try later";
    m_deviceResponceMapStr[CMD_MAX_BUFFER_LIMIT]                            ="Max buffer limit reached.";
    m_deviceResponceMapStr[CMD_PROCESS_ERROR]                               ="Processing Error";
    m_deviceResponceMapStr[CMD_TESTING_ON]                                  ="Test Request already in process.";
    m_deviceResponceMapStr[CMD_RESERVED_26]                                 ="";
    m_deviceResponceMapStr[CMD_NO_MODEM_FOUND]                              ="No modem found.";
    m_deviceResponceMapStr[CMD_SIM_REGISTER_FAIL]                           ="Error in SIM registration.";
    m_deviceResponceMapStr[CMD_TESTING_FAIL]                                ="Cannot establish connection.";
    m_deviceResponceMapStr[CMD_SERVER_DISABLED]                             ="Server Disabled";
    m_deviceResponceMapStr[CMD_CONNECTIVITY_ERROR]                          ="Error in connection. Please try again later";
    m_deviceResponceMapStr[CMD_NO_WRITE_PERMISSION]                         ="No 'Write' permission on FTP.";
    m_deviceResponceMapStr[CMD_INVALID_FILE_NAME]                           ="Incorrect file name.";
    m_deviceResponceMapStr[CMD_CHANNEL_DISABLED]                            ="Camera disabled.";
    m_deviceResponceMapStr[CMD_CHANNEL_BLOCKED]                             ="";
    m_deviceResponceMapStr[CMD_SUB_STREAM_DISABLED]                         ="Sub-stream disabled.";
    m_deviceResponceMapStr[CMD_BACKUP_IN_PROCESS]                           ="Back-up already in progress.";
    m_deviceResponceMapStr[CMD_ALARM_DISABLED]                              ="Alarm output disabled.";
    m_deviceResponceMapStr[CMD_TOUR_NOT_SET]                                ="Manual PTZ tour not configured.";
    m_deviceResponceMapStr[CMD_MAN_RECORD_DISABLED]                         ="Manual recording disabled.";
    m_deviceResponceMapStr[CMD_NO_DISK_FOUND]                               ="No disk found.";
    m_deviceResponceMapStr[CMD_NO_EVENT_FOUND]                              ="No events found.";
    m_deviceResponceMapStr[CMD_ADMIN_CANT_BLOCKED]                          ="Admin user cannot be blocked.";
    m_deviceResponceMapStr[CMD_ADDRESS_ALREADY_ASSIGNED]                    ="Camera Address Already assigned.";
    m_deviceResponceMapStr[CMD_FILE_EXTRACT_ERROR]                          ="Firmware Upgrade failed.";
    m_deviceResponceMapStr[CMD_FILE_MISSING]                                ="Firmware Upgrade failed. File not found";
    m_deviceResponceMapStr[CMD_REQUEST_IN_PROGRESS]                         ="Request already in process.";
    m_deviceResponceMapStr[CMD_NO_EVENTS]                                   ="";
    m_deviceResponceMapStr[CMD_EVENT_AVAILABLE]                             ="";
    m_deviceResponceMapStr[CMD_MORE_DATA]                                   ="More records available.";
    m_deviceResponceMapStr[CMD_MORE_EVENTS]                                 ="More events available. Please change the filter to view more logs.";
    m_deviceResponceMapStr[CMD_AUDIO_DISABLED]                              ="Audio Disabled.Check Stream Settings.";
    m_deviceResponceMapStr[CMD_INVALID_UPDATE_MODE]                         ="Save 'Mode of Update' as 'Manual'";
    m_deviceResponceMapStr[CMD_STREAM_ALREADY_ON]                           ="Stream already ON.";
    m_deviceResponceMapStr[CMD_PLAYBACK_STREAM_ON]                          ="Playback Already ON.";
    m_deviceResponceMapStr[CMD_MANUAL_RECORDING_ON]                         ="Manual Recording already ON.";
    m_deviceResponceMapStr[CMD_MANUAL_TOUR_ON]                              ="Manual Tour already ON.";
    m_deviceResponceMapStr[CMD_MANUAL_RECORDING_OFF]                        ="Manual Recording already OFF.";
    m_deviceResponceMapStr[CMD_MANUAL_TOUR_OFF]                             ="Manual Tour already OFF.";
    m_deviceResponceMapStr[CMD_INVALID_SEQUENCE]                            ="File transfer failed.";
    m_deviceResponceMapStr[CMD_INVALID_DATA_LENGTH]                         ="File transfer failed.";
    m_deviceResponceMapStr[CMD_INVALID_FILE_LENGTH]                         ="File transfer failed.";
    m_deviceResponceMapStr[CMD_INVALID_FILE_SIZE]                           ="File transfer failed. File size too large";
    m_deviceResponceMapStr[CMD_UPGRADE_IN_PROCESS]                          ="Firmware upgrade in progress. Please try later.";
    m_deviceResponceMapStr[CMD_FORMAT_IN_PROCESS]                           ="Disk being formatted.Please try later.";
    m_deviceResponceMapStr[CMD_DDNS_UPDATE_FAILED]                          ="DDNS update failed.";
    m_deviceResponceMapStr[CMD_CAM_REQUEST_IN_PROCESS]                      ="Another request in process. Please try later.";
    m_deviceResponceMapStr[CMD_FEATURE_NOT_SUPPORTED]                       ="Feature not supported by camera";
    m_deviceResponceMapStr[CMD_CODEC_NOT_SUPPORTED]                         ="Stream not available from camera.";
    m_deviceResponceMapStr[CMD_CAM_REQUEST_FAILED]                          ="Request failed.";
    m_deviceResponceMapStr[CMD_CAM_DISCONNECTED]                            ="Camera Offline";
    m_deviceResponceMapStr[CMD_NO_MANUAL_ACTION]                            ="The USB Device is currently in use.";
    m_deviceResponceMapStr[CMD_FTP_INVALID_CREDENTIAL]                      ="Incorrect User Name or Password";
    m_deviceResponceMapStr[CMD_BACKUP_NOT_IN_PROG]                          ="Backup Not in Process";
    m_deviceResponceMapStr[CMD_MAN_TRG_ALREADY_ON]                          ="Manual Trigger Already ON.";
    m_deviceResponceMapStr[CMD_MAN_TRG_ALREADY_OFF]                         ="Manual Trigger Already OFF.";
    m_deviceResponceMapStr[CMD_CONFIG_RESTORE_IN_PROCESS]                   ="Restoring Configuration. Please try later.";
    m_deviceResponceMapStr[CMD_ERROR_SUBMIT_ROUTE_TABLE]                    ="Network Address must not be in LAN or Broadband Subnet";
    m_deviceResponceMapStr[CMD_DEST_ADDR_IS_NOT_NW_ADDR]                    ="Network Address must be within the same subnet/prefix range";
    m_deviceResponceMapStr[CMD_UNABLE_TO_FORMAT]                            ="Unable to format.Another disk is being formatted";
    m_deviceResponceMapStr[CMD_LAN1_LAN2_SAME_SUBNET]                       ="LAN 1 & LAN 2 IP should not be in the same Subnet.";
    m_deviceResponceMapStr[CMD_SNAPSHOT_FAILED]                             ="Snapshot Upload Failed.";
    m_deviceResponceMapStr[CMD_SNAPSHOT_SERVER_DISABLED]                    ="Server Disabled";
    m_deviceResponceMapStr[CMD_SNAPSHOT_CONNECTION_ERROR]                   ="Error in connection. Please try again later";
    m_deviceResponceMapStr[CMD_SNAPSHOT_NO_WRITE_PERMISSION]                ="No 'Write' Permission. Snapshot Upload Failed";
    m_deviceResponceMapStr[CMD_CAM_PARAM_NOT_CONFIGURED]                    ="Request failed.Camera parameters not configured.";
    m_deviceResponceMapStr[CMD_ONVIF_CAM_CAPABILITY_ERROR]                  ="Camera Communication Failed. Please try again later.";
    m_deviceResponceMapStr[CMD_NON_CONFIGURABLE_PARAMETER]                  ="Camera parameter cannot be changed.";
    m_deviceResponceMapStr[CMD_FIRMWARE_NOT_FOUND]                          ="Firmware Not Found";
    m_deviceResponceMapStr[CMD_DEVICE_FIRMWARE_UP_TO_DATE]                  ="Your device firmware is Up to Date";
    m_deviceResponceMapStr[CMD_SMS_ACC_EXPIRED]                             ="SMS User account has expired.";
    m_deviceResponceMapStr[CMD_SMS_ACC_INSUFF_CREDITS]                      ="Insufficient balance.";
    m_deviceResponceMapStr[CMD_INVALID_MOBILE_NO]                           ="Mobile number does not exist.";
    m_deviceResponceMapStr[CMD_IP_SUBNET_MISMATCH]                          ="Start IP Address and LAN IP Address must be in the same network";
    m_deviceResponceMapStr[CMD_PLAYBACK_PROCESS_ERROR]                      ="Playback Processing Error";
    m_deviceResponceMapStr[CMD_INVALID_HOST_SIZE]                           ="Please Enter valid Number of Hosts";
    m_deviceResponceMapStr[CMD_FILE_DOWNGRED_FAIL]                          ="Downgrading to previous version is not supported.";
    m_deviceResponceMapStr[CMD_HOST_NAME_DUPLICATION]                       ="Host Name is already assigned to another device.";
    m_deviceResponceMapStr[CMD_HOST_NAME_REG_FAIL]                          ="Registration failed.";
    m_deviceResponceMapStr[CMD_MAX_CAMERA_CONFIGURED]                       ="Maximum Cameras are already configured";
    m_deviceResponceMapStr[CMD_INVALID_USR_OR_PSWD]                         ="Invalid credentials ";
    m_deviceResponceMapStr[CMD_RESERVED_102]                                ="";
    m_deviceResponceMapStr[CMD_REC_MEDIA_ERR]                               ="Recording media is busy.Please try later.";
    m_deviceResponceMapStr[CMD_REC_MEDIA_FULL]                              ="Recording media is full.";
    m_deviceResponceMapStr[CMD_RESERVED_105]                                ="";
    m_deviceResponceMapStr[CMD_SAME_LAN_IP_ADDR]                            ="LAN1 & LAN2 IP address should not be same.";
    m_deviceResponceMapStr[CMD_BRND_MDL_MIS_MATCH]                          ="Mismatch in camera Brand & Model.";
    m_deviceResponceMapStr[CMD_IP_AND_GATEWAY_SAME_SUBNET]                  ="IP Address and Default Gateway must be in the same subnet";
    m_deviceResponceMapStr[CMD_REC_DRIVE_CONFIG_CHANGES]                    ="Recording Drive configuration changed.Please try later.";
    m_deviceResponceMapStr[CMD_GEN_CAM_STRM_PARAM_CAP_REQ]                  ="Stream Configuration not supported.";
    m_deviceResponceMapStr[CMD_NON_CONFIG_CAM_CAP_REQ]                      ="Unidentified Camera. Stream Configuration not supported.";
    m_deviceResponceMapStr[CMD_SUB_STRM_NOT_SUPPORTED]                      ="Sub Stream not supported for camera.";
    m_deviceResponceMapStr[CMD_MAX_CAM_CONFIGED]                            ="Maximum Cameras are already configured";
    m_deviceResponceMapStr[CMD_RESERVED_114]                                ="";
    m_deviceResponceMapStr[CMD_AVI_SEARCH_NOT_ALLOWED]                      ="";
    m_deviceResponceMapStr[CMD_LOGIN_SESSION_DURATION_OVER]                 ="Your Allowed Access Time elapsed.";
    m_deviceResponceMapStr[CMD_INSTANT_PLAYBACK_FAILED]                     ="Records Unavailable.";
    m_deviceResponceMapStr[CMD_MOTION_WINDOW_FAILED]                        ="Failed to set motion window. Please try again later";
    m_deviceResponceMapStr[CMD_IP_ADDRESS_CHANGE_FAIL]                      ="Request to change IP address failed for camera at index";
    m_deviceResponceMapStr[CMD_CAM_REBOOT_FAILED]                           ="Reboot camera manually to update its IP address";
    m_deviceResponceMapStr[CMD_NO_CAM_FOUND]                                ="No Camera found";
    m_deviceResponceMapStr[CMD_AUTO_SEARCH_COMPLETE]                        ="";
    m_deviceResponceMapStr[CMD_ACTIVE_TOUR_PAUSE]                           ="Active tour Paused";
    m_deviceResponceMapStr[CMD_AUTO_SEARCH_START]                           ="";
    m_deviceResponceMapStr[CMD_AUTO_CONFIG_RANGE_INVALID]                   ="Camera should be auto configured in same subnet as device";
    m_deviceResponceMapStr[CMD_RESET_PASSWORD]                              ="Please set your password  ";
    m_deviceResponceMapStr[CMD_PASSWORD_EXPIRE]                             ="Password expired. Please reset";
    m_deviceResponceMapStr[CMD_USER_ACCOUNT_LOCK]                           ="";
    m_deviceResponceMapStr[CMD_MIN_PASSWORD_CHAR_REQUIRED]                  ="";
    m_deviceResponceMapStr[CMD_HIGH_PASSWORD_SEC_REQ]                       ="Password must contain at least : 1 Uppercase (A-Z) 1 Lowercase (a-z) 1 Number (0-9) and 1 Special Character _.,()[]:@!#$*+/\\";
    m_deviceResponceMapStr[CMD_MEDIUM_PASSWORD_SEC_REQ]                     ="Password must contain at least : 1 Uppercase (A-Z) 1 Lowercase (a-z) and 1 Number (0-9)";
    m_deviceResponceMapStr[CMD_REQ_FAIL_CHNG_AUD_OUT_PRI]                   ="Request failed. Change Audio Out priority";
    m_deviceResponceMapStr[CMD_AUDIO_CHANNEL_BUSY]                          ="Audio channel busy";
    m_deviceResponceMapStr[CMD_NO_AUD_OUT_AVAILABLE]                        ="No Audio Out port available";
    m_deviceResponceMapStr[CMD_AUD_SND_REQ_FAIL]                            ="Audio sending request failed";
    m_deviceResponceMapStr[CMD_AUD_SND_STP_PRO_LCL_CLNT_REQ]                ="Audio sending stopped. Processing local client request";
    m_deviceResponceMapStr[CMD_RAID_TRANSFORM_FAIL]                         ="Please check current HDD combination and try again later";
    m_deviceResponceMapStr[CMD_LOG_VOLUMN_MORE_THEN_8TB]                    ="RAID creation failed. Total logical volume size exceeds maximum allowed size";

    // INTERNAL SERVER CODE //
    m_deviceResponceMapStr[CMD_DEVICE_EXIST]                                ="Device Exist";
    m_deviceResponceMapStr[CMD_DEV_CONNECTED]                               ="Device Connected";
    m_deviceResponceMapStr[CMD_DEV_DISCONNECTED]                            ="Device Disconnected";
    m_deviceResponceMapStr[CMD_DEV_CONFLICT]                                ="Not compatible with the Local Device.";
    m_deviceResponceMapStr[CMD_SERVER_NOT_RESPONDING]                       ="Server not responding.";
    m_deviceResponceMapStr[CMD_STREAM_NORMAL]                               ="Stream Normal.";
    m_deviceResponceMapStr[CMD_STREAM_FILE_ERROR]                           ="Stream file error.";
    m_deviceResponceMapStr[CMD_STREAM_HDD_FORMAT]                           ="Stream HDD Format.";
    m_deviceResponceMapStr[CMD_STREAM_CONFIG_CHANGE]                        ="Stream Configuration Changed.";
    m_deviceResponceMapStr[CMD_STREAM_PLAYBACK_OVER]                        ="Playback Over.";
    m_deviceResponceMapStr[CMD_STREAM_VIDEO_LOSS]                           ="Video Loss";
    m_deviceResponceMapStr[CMD_STREAM_NO_VIDEO_LOSS]                        ="No Video Loss.";
    m_deviceResponceMapStr[CMD_STREAM_STOPPED]                              ="Stream Stopped.";
    m_deviceResponceMapStr[CMD_DUPLICATION_STOPPED]                         ="DUP Stream Stopped.";
    m_deviceResponceMapStr[CMD_DECODER_ERROR]                               ="Maximum Decoding Capacity Reached.";
    m_deviceResponceMapStr[CMD_CONFIG_CHANGED]                              ="Camera Config Changed.";
    m_deviceResponceMapStr[CMD_INTERNAL_RESOURCE_LIMIT]                     ="Resource busy.";
    m_deviceResponceMapStr[CMD_INVALID_REQ_PARAM]                           ="Invalid Request Parameter.";
    m_deviceResponceMapStr[CMD_PLAYBACK_TIME]                               ="";
    m_deviceResponceMapStr[CMD_STREAM_CONNECTING]                           ="";
    m_deviceResponceMapStr[CMD_DEV_LOGGEDOUT]                               ="Device Disconnected";
    m_deviceResponceMapStr[CMD_DEV_DELETED]                                 ="Device Disconnected";
    m_deviceResponceMapStr[CMD_CAM_INDX_NOT_VALID]                          ="";
    m_deviceResponceMapStr[CMD_DISK_CLEANUP_REQUIRED]                       ="Device Disconnected";
    m_deviceResponceMapStr[CMD_REQUEST_NOT_PROCESSED]                       ="Device Disconnected";
    m_deviceResponceMapStr[CMD_MAX_DEVICE_REPLY]                            ="";

    m_deviceResponceMapStr[CMD_SMTP_SERVER_CONNECTION_ERROR]                ="Could not connect to SMTP Server or Port";
    m_deviceResponceMapStr[CMD_SMTP_CONNECTION_REFUSED]                     ="Connection refused by SMTP server";
    m_deviceResponceMapStr[CMD_SMTP_SERVER_UNAVAILABLE]                     ="SMTP Server not available at the moment";
    m_deviceResponceMapStr[CMD_SMTP_RECIPIENT_MAILBOX_FULL]                 ="Recipient mailbox full";
    m_deviceResponceMapStr[CMD_SMTP_RECIPIENT_SERVER_NOT_RESPONDING]        ="Recipient SMTP server not responding";
    m_deviceResponceMapStr[CMD_SMTP_MAIL_NOT_ACCEPTED]                      ="Mail is not accepted";
    m_deviceResponceMapStr[CMD_SMTP_AUTHENTICATION_FAILED]                  ="Authentication Failed. Verify Entered Details.";
    m_deviceResponceMapStr[CMD_SMTP_INVALID_EMAIL_ADDR]                     ="Sender/Recipient Mailbox address invalid or unavailable";
    m_deviceResponceMapStr[CMD_SMTP_SERVER_STORAGE_FULL]                    ="Server storage limit exceeded";
    m_deviceResponceMapStr[CMD_SMTP_TRANSACTION_FAILED]                     ="Transaction failed. Email is Spam/Blacklisted";
    m_deviceResponceMapStr[CMD_INVALID_USERNAME]                            ="Invalid Username";
    m_deviceResponceMapStr[CMD_MISSING_VERIFICATION_DETAILS]                ="Verification details are not found";
    m_deviceResponceMapStr[CMD_MISMATCH_SECURITY_ANSWERS]                   ="Incorrect Answers";
    m_deviceResponceMapStr[CMD_MISMATCH_OTP]                                ="Incorrect OTP";
    m_deviceResponceMapStr[CMD_EMAIL_SERVICE_DISABLED]                      ="Email Service is not enabled";
    m_deviceResponceMapStr[CMD_PWD_RST_SESSION_NOT_AVAILABLE]               ="Password reset session limit exceeded";
    m_deviceResponceMapStr[CMD_PWD_RST_ALREADY_IN_PROGRESS]                 ="Password reset already in progress";
    m_deviceResponceMapStr[CMD_SESSION_EXPIRED]                             ="Session Expired";
}

ValidationMessage::~ValidationMessage()
{
}
