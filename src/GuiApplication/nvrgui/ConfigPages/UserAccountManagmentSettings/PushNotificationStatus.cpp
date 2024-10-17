//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "PushNotificationStatus.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
#define PUSH_NOTIFICATION_CELL_HEIGHT   SCALE_HEIGHT(60)
#define PUSH_NOTIFICATION_BGTILE_WIDTH  SCALE_WIDTH(910)

/* Push Notifications status page reply command response */
typedef enum
{
    PUSH_NOTIFICATIONS_LIST_CMD_REPLY_USERNAME = 0,
    PUSH_NOTIFICATIONS_LIST_CMD_REPLY_FCM_TOKEN,
    PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MODEL_NAME,
    PUSH_NOTIFICATIONS_LIST_CMD_REPLY_INACTIVITY_TIMER,
    PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MAX
}PUSH_NOTIFICATION_LIST_RESP_e;

const static QString fieldHeadingStr[PUSH_NOTIFICATION_STR_MAX] =
{
    "Username",
    "Model Name",
    "Inactivity Timer (HHH:MM)",
    "Delete"
};

PushNotification::PushNotification(QString devName, QWidget *parent, DEV_TABLE_INFO_t *tableInfo)
    : ConfigPageControl(devName, parent, PUSH_NOTIFICATION_STR_MAX, tableInfo, MAX_CNFG_BTN_TYPE)
{
    m_topBgTile = NULL;
    m_currElement = 0;

    for (quint8 index = 0; index < PUSH_NOTIFICATION_STR_MAX; index++)
    {
        m_tableCellHeadings[index] = NULL;
        m_tableHeadingTextlabel[index] = NULL;
    }

    for (quint8 index = 0; index < PUSH_NOTIFICATION_RECORD_MAX; index++)
    {
        m_userNameCell[index] = NULL;
        m_modelNameCell[index] = NULL;
        m_inactivityTimerCell[index] = NULL;
        m_deleteCell[index] = NULL;
    }    

    m_userNameList.reserve(PUSH_NOTIFICATION_RECORD_MAX);
    m_fcmTokenList.reserve(PUSH_NOTIFICATION_RECORD_MAX);
    m_inactivityTimerList.reserve(PUSH_NOTIFICATION_RECORD_MAX);
    m_modelNameList.reserve(PUSH_NOTIFICATION_RECORD_MAX);
    m_userNameCellWidth = m_modelNameCellWidth = 0;

    m_currDevName = devName;
    createDefaultComponents();
    payloadLib->setCnfgArrayAtIndex (0,0);
    sendCommand(GET_PUSH_DEV_LIST,1);
}

PushNotification::~PushNotification()
{
    m_userNameList.clear();
    m_fcmTokenList.clear();
    m_inactivityTimerList.clear();
    m_modelNameList.clear();
    DELETE_OBJ(m_topBgTile);

    for (quint8 index = 0; index < PUSH_NOTIFICATION_STR_MAX; index++)
    {
        DELETE_OBJ(m_tableCellHeadings[index]);
        DELETE_OBJ(m_tableHeadingTextlabel[index]);
    }

    for (quint8 index = 0; index < PUSH_NOTIFICATION_RECORD_MAX; index++)
    {
        DELETE_OBJ(m_userNameCell[index]);
        DELETE_OBJ(m_modelNameCell[index]);
        DELETE_OBJ(m_inactivityTimerCell[index]);
        DELETE_OBJ(m_deleteCell[index]);
    }

    for (quint8 index = 1; index < PUSH_NOTIFICATION_RECORD_MAX; index++)
    {
        DELETE_OBJ(m_userNameCellText[index]);
        DELETE_OBJ(m_modelNameCellText[index]);
        DELETE_OBJ(m_inactivityTimerCellText[index]);

        disconnect(m_deleteControlBtn[index],
                  SIGNAL(sigButtonClick(int)),
                  this,
                  SLOT(slotDeleteButtonClick(int)));
        DELETE_OBJ(m_deleteControlBtn[index]);
    }
}

void PushNotification::createDefaultComponents()
{
    quint16 headerWidthArray[PUSH_NOTIFICATION_STR_MAX] = {330, 270, 220, 70};
    quint16 xPos, yPos, width;

    /* top background tile */
    m_topBgTile = new BgTile(50, 80, PUSH_NOTIFICATION_BGTILE_WIDTH, (6*PUSH_NOTIFICATION_CELL_HEIGHT) + SCALE_HEIGHT(100), TOP_LAYER, this);

    /* draw columns */
    for (quint8 index = 0; index < PUSH_NOTIFICATION_STR_MAX; index++)
    {
        xPos = (index == 0) ? m_topBgTile->x() + SCALE_WIDTH(10) : m_tableCellHeadings[index-1]->x() + m_tableCellHeadings[index-1]->width();
        yPos = (index == 0) ? m_topBgTile->y() + SCALE_HEIGHT(10) : m_tableCellHeadings[index-1]->y();

        /* field heading */
        m_tableCellHeadings[index] = new TableCell(xPos, yPos, (SCALE_WIDTH(headerWidthArray[index]-1)), SCALE_HEIGHT(50), this, true);

        /* field heading labels */
        m_tableHeadingTextlabel[index] = new TextLabel(m_tableCellHeadings[index]->x() + SCALE_WIDTH(10),
                                                       m_tableCellHeadings[index]->y() + (m_tableCellHeadings[index]->height ())/2,
                                                       NORMAL_FONT_SIZE,
                                                       fieldHeadingStr[index],
                                                       this,
                                                       NORMAL_FONT_COLOR,
                                                       NORMAL_FONT_FAMILY,
                                                       ALIGN_START_X_CENTRE_Y, 0, 0, (SCALE_WIDTH(headerWidthArray[index])), 0, true, Qt::AlignVCenter, SCALE_WIDTH(10));
    }

    /* draw rows */
    for (quint8 index = 0; index < PUSH_NOTIFICATION_RECORD_MAX; index++)
    {
        xPos = (index == 0) ? m_tableCellHeadings[index]->x() : m_userNameCell[(index-1)]->x();
        yPos = (index == 0) ? m_tableCellHeadings[index]->y() + m_tableCellHeadings[index]->height() : m_userNameCell[(index-1)]->y() + m_userNameCell[(index-1)]->height();
        width = (index == 0) ? m_tableCellHeadings[index]->width() : m_userNameCell[(index-1)]->width()-1;

        m_userNameCell[index] = new TableCell(xPos, yPos, width, BGTILE_HEIGHT, this);

        xPos = (index == 0) ? m_tableCellHeadings[index+1]->x() : m_modelNameCell[(index -1)]->x();
        yPos = (index == 0) ? m_tableCellHeadings[index+1]->y() + m_tableCellHeadings[index+1]->height() : m_modelNameCell[(index-1)]->y() + m_userNameCell[(index-1)]->height();
        width = (index == 0) ? m_tableCellHeadings[index+1]->width() : m_modelNameCell[(index -1)]->width()-1;

        m_modelNameCell[index] = new TableCell(xPos, yPos, width, BGTILE_HEIGHT, this);

        xPos = (index == 0) ? m_tableCellHeadings[index+2]->x() : m_inactivityTimerCell[(index-1)]->x();
        yPos = (index == 0) ? m_tableCellHeadings[index+2]->y() + m_tableCellHeadings[index+2]->height() : m_inactivityTimerCell[(index-1)]->y() + m_userNameCell[(index-1)]->height();
        width = (index == 0) ? m_tableCellHeadings[index+2]->width() : m_inactivityTimerCell[(index-1)]->width()-1;

        m_inactivityTimerCell[index] = new TableCell(xPos, yPos, width, BGTILE_HEIGHT, this);

        xPos = (index == 0) ? m_tableCellHeadings[index+3]->x() : m_deleteCell[(index-1)]->x();
        yPos = (index == 0) ? m_tableCellHeadings[index+3]->y() + m_tableCellHeadings[index+3]->height() : m_deleteCell[(index-1)]->y() + m_userNameCell[(index-1)]->height();
        width = (index == 0) ? m_tableCellHeadings[index+3]->width() : m_deleteCell[(index-1)]->width()-1;

        m_deleteCell[index] = new TableCell(xPos, yPos, width, BGTILE_HEIGHT, this);
    }

    /* cell text */    
    for (quint8 index = 0; index < PUSH_NOTIFICATION_RECORD_MAX; index++)
    {
        width = (index == 0) ? m_tableCellHeadings[index]->width () : m_userNameCell[(index-1)]->width();
        if (index == 0)
        {
            m_userNameCellWidth = width;
        }

        m_userNameCellText[index] = new TextLabel(m_userNameCell[index]->x () + SCALE_WIDTH(10),
                                                  m_userNameCell[index]->y () + (m_userNameCell[index]->height ())/2,
                                                  NORMAL_FONT_SIZE,
                                                  "",
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, 0, width, 0, true, Qt::AlignVCenter, SCALE_WIDTH(10));

        width = (index == 0) ? m_tableCellHeadings[index+1]->width () : m_modelNameCell[(index-1)]->width();
        if (index == 0)
        {
            m_modelNameCellWidth = width;
        }

        m_modelNameCellText[index] = new TextLabel(m_modelNameCell[index]->x () + SCALE_WIDTH(10),
                                                   m_modelNameCell[index]->y () + (m_modelNameCell[index]->height ())/2,
                                                   NORMAL_FONT_SIZE,
                                                   "",
                                                   this,
                                                   NORMAL_FONT_COLOR,
                                                   NORMAL_FONT_FAMILY,
                                                   ALIGN_START_X_CENTRE_Y, 0, 0, width, 0, true, Qt::AlignVCenter, SCALE_WIDTH(10));

        width = (index == 0) ? m_tableCellHeadings[index+2]->width () : m_inactivityTimerCell[(index-1)]->width();

        m_inactivityTimerCellText[index] = new TextLabel(m_inactivityTimerCell[index]->x () + SCALE_WIDTH(10),
                                                         m_inactivityTimerCell[index]->y () + (m_inactivityTimerCell[index]->height ())/2,
                                                         NORMAL_FONT_SIZE,
                                                         "",
                                                         this,
                                                         NORMAL_FONT_COLOR,
                                                         NORMAL_FONT_FAMILY,
                                                         ALIGN_START_X_CENTRE_Y, 0, 0, width, 0, true, Qt::AlignVCenter, SCALE_WIDTH(10));

        m_deleteControlBtn[index] = new ControlButton(DELETE_BUTTON_INDEX,
                                                      m_deleteCell[index]->x () + SCALE_WIDTH(25),
                                                      m_deleteCell[index]->y (),
                                                      SCALE_WIDTH(60),
                                                      BGTILE_HEIGHT,
                                                      this, NO_LAYER, 0, "", false, index);

        m_elementList[index] =  m_deleteControlBtn[index];

        connect ( m_deleteControlBtn[index],
                  SIGNAL(sigButtonClick(int)),
                  this,
                  SLOT(slotDeleteButtonClick(int)));

        m_deleteControlBtn[index]->setVisible (false);
    }
}

void PushNotification::processDeviceResponse(DevCommParam *param, QString )
{
    processBar->unloadProcessBar();

    /* fail response */
    if (CMD_SUCCESS != param->deviceStatus)
    {
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    /* success response */
    switch(param->cmdType)
    {
        case GET_PUSH_DEV_LIST:
                parseAndUpdateList(param);
                break;

        case DEL_PUSH_DEV:
                payloadLib->parseDevCmdReply(true, param->payload);
                payloadLib->setCnfgArrayAtIndex(0, 0);
                sendCommand(GET_PUSH_DEV_LIST, 1);
                break;

        default:
            break;
    }    
}

void PushNotification::sendCommand(SET_COMMAND_e cmdType, quint32 totalfeilds)
{
    QString payloadString = payloadLib->createDevCmdPayload(totalfeilds);;

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;

    if (applController->processActivity(m_currDevName, DEVICE_COMM, param))
    {
        processBar->loadProcessBar();
    }
}

void PushNotification::parseAndUpdateList(DevCommParam *param)
{
    QString     fcmToken;
    quint32     time;
    quint8      maxListCount, listCount = 0;

    payloadLib->parseDevCmdReply(true, param->payload);    
    maxListCount = (payloadLib->getTotalCmdFields() / PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MAX);

    /* if no list found */
    if (0 == maxListCount)
    {
        return;
    }

    /* clear qstring list of all data */
    clearList();

    /* parse reponse data */
    for (quint8 index = 0; index < maxListCount; index++)
    {
        fcmToken = (payloadLib->getCnfgArrayAtIndex(PUSH_NOTIFICATIONS_LIST_CMD_REPLY_FCM_TOKEN + (index * PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MAX)).toString ());
        if (fcmToken == "")
        {
            continue;
        }

        m_fcmTokenList.append(fcmToken);
        m_userNameList.append(payloadLib->getCnfgArrayAtIndex(PUSH_NOTIFICATIONS_LIST_CMD_REPLY_USERNAME + (index * PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MAX)).toString ());
        m_modelNameList.append(payloadLib->getCnfgArrayAtIndex(PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MODEL_NAME + (index * PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MAX)).toString ());

        /* convert time seconds to HHH:MM format */
        time = payloadLib->getCnfgArrayAtIndex(PUSH_NOTIFICATIONS_LIST_CMD_REPLY_INACTIVITY_TIMER + (index * PUSH_NOTIFICATIONS_LIST_CMD_REPLY_MAX)).toInt();
        if (time == 0)
        {
            m_inactivityTimerList.append(QString("Active"));
        }
        else
        {
            m_inactivityTimerList.append(QString::number(time/3600).rightJustified(3, '0') + ":" + QString::number((time%3600)/60).rightJustified(2, '0'));
        }
        listCount++;
    }

    /* show Push Notification Status on UI */
    updateList(listCount);
}

void PushNotification::clearList()
{
    m_userNameList.clear();
    m_fcmTokenList.clear();
    m_inactivityTimerList.clear();
    m_modelNameList.clear();
}

void PushNotification::updateList(quint8 listCount)
{
    quint8 index;

    for (index = 0; index < listCount; index++)
    {
        m_userNameCellText[index]->changeText(m_userNameList.at(index), m_userNameCellWidth);
        m_userNameCellText[index]->update();

        m_modelNameCellText[index]->changeText(m_modelNameList.at(index), m_modelNameCellWidth);
        m_modelNameCellText[index]->update();

        m_inactivityTimerCellText[index]->changeText(m_inactivityTimerList.at(index));
        m_inactivityTimerCellText[index]->update();

        m_deleteControlBtn[index]->setVisible (true);
        m_deleteControlBtn[index]->setIsEnabled(true);
    }

    /* clear text from cells */
    for (; index < PUSH_NOTIFICATION_RECORD_MAX; index++)
    {
        m_userNameCellText[index]->changeText("");
        m_userNameCellText[index]->update();

        m_modelNameCellText[index]->changeText("");
        m_modelNameCellText[index]->update();

        m_inactivityTimerCellText[index]->changeText("");
        m_inactivityTimerCellText[index]->update();

        m_deleteControlBtn[index]->setVisible (false);
        m_deleteControlBtn[index]->setIsEnabled(false);
    }
}

void PushNotification::slotDeleteButtonClick(int index)
{
    m_currElement = index;
    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PUSH_NOTIFICATIONS_DELETE_CLIENT), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
}

void PushNotification::handleInfoPageMessage(int index)
{
    /* Send device delete command only if "Yes" is pressed in for device delete popup. Ignore other popups. */
    if ((index == INFO_OK_BTN) && (infoPage->getText() == ValidationMessage::getValidationMessage(PUSH_NOTIFICATIONS_DELETE_CLIENT)))
    {
        payloadLib->setCnfgArrayAtIndex (0, m_fcmTokenList.at(m_currElement));
        sendCommand(DEL_PUSH_DEV, 1);
    }
}
