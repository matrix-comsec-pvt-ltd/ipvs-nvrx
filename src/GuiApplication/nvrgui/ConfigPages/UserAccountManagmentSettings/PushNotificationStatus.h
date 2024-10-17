#ifndef PUSHNOTIFICATIONSTATUS_H
#define PUSHNOTIFICATIONSTATUS_H

//#################################################################################################
// @INCLUDES
//#################################################################################################
#include "Controls/ConfigPageControl.h"
#include "Controls/Bgtile.h"
#include "Controls/TableCell.h"
#include "Controls/TextBox.h"
#include "Controls/ControlButton.h"

//#################################################################################################
// @DEFINES
//#################################################################################################
/* Push Notification status columns */
#define PUSH_NOTIFICATION_RECORD_MAX            10

//#################################################################################################
// @TYPEDEF
//#################################################################################################
/*Push Notification Strings*/
typedef enum
{
    PUSH_NOTIFICATION_STR_USERNAME = 0,
    PUSH_NOTIFICATION_STR_MODEL_NAME,
    PUSH_NOTIFICATION_STR_INACTIVITY_TIMER,
    PUSH_NOTIFICATION_STR_DELETE,
    PUSH_NOTIFICATION_STR_MAX
}PUSH_NOTIFICATION_STR_e;

class PushNotification : public ConfigPageControl
{
     Q_OBJECT

public:
    explicit PushNotification(QString devName,QWidget *parent = 0, DEV_TABLE_INFO_t *tableInfo = NULL);
    ~PushNotification();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void handleInfoPageMessage(int);

public slots:    
    void slotDeleteButtonClick(int index);

private:
    /* UI elements */
    QString                 m_currDevName;
    BgTile*                 m_topBgTile;

    TableCell*              m_tableCellHeadings[PUSH_NOTIFICATION_STR_MAX];
    TextLabel*              m_tableHeadingTextlabel[PUSH_NOTIFICATION_STR_MAX];

    TableCell*              m_userNameCell[PUSH_NOTIFICATION_RECORD_MAX];
    TableCell*              m_modelNameCell[PUSH_NOTIFICATION_RECORD_MAX];
    TableCell*              m_inactivityTimerCell[PUSH_NOTIFICATION_RECORD_MAX];
    TableCell*              m_deleteCell[PUSH_NOTIFICATION_RECORD_MAX];

    TextLabel*              m_userNameCellText[PUSH_NOTIFICATION_RECORD_MAX];
    TextLabel*              m_modelNameCellText[PUSH_NOTIFICATION_RECORD_MAX];
    TextLabel*              m_inactivityTimerCellText[PUSH_NOTIFICATION_RECORD_MAX];

    ControlButton*          m_deleteControlBtn[PUSH_NOTIFICATION_RECORD_MAX];
    quint16                 m_userNameCellWidth;
    quint16                 m_modelNameCellWidth;

    /* Push Notify list database */
    QStringList             m_userNameList;
    QStringList             m_fcmTokenList;
    QStringList             m_inactivityTimerList;
    QStringList             m_modelNameList;
    quint8                  m_currElement;

    void createDefaultComponents();
    void sendCommand(SET_COMMAND_e cmdType, quint32 totalfeilds);
    void parseAndUpdateList(DevCommParam *param);
    void clearList(void);
    void updateList(quint8 listCount);
    void updateStatusList();
};

#endif // PUSHNOTIFICATIONSTATUS_H
