#ifndef ALARAM_OUTPUT_H
#define ALARAM_OUTPUT_H


#include "ManageMenuOptions.h"
#include "Controls/Bgtile.h"

#define TOP             105
//#define MAX_ALARM       3

typedef enum
{
    ALARM_DEACTIVATE = 0,
    ALARM_ACTIVATE,
    ALARM_UNKNOWN
}ALARM_STATE_e;


class AlaramOutput : public ManageMenuOptions
{
    Q_OBJECT

private:
    BgTile*          m_bgTile[MAX_DEV_ALARM];
    CnfgButton*      m_cnfgButton[MAX_DEV_ALARM];
    Heading*         m_cnfgheading[MAX_DEV_ALARM];
    quint8           alarmStatus[MAX_CAMERAS];
    DEV_TABLE_INFO_t devInfo;
    quint8           m_maxAlarmOutput;

public:
    explicit AlaramOutput(QString devName,
                 QWidget *parent = 0);
    ~AlaramOutput();

    void processDeviceResponse(DevCommParam *param, QString devName);
    void updateStatus(QString devName, qint8 status, qint8 index,quint8 eventType,
                      quint8 eventSubType);

    void changeText(quint8 index);
    void triggerAlaram(QString devName, quint8 index);
    void getAlarmStatus(QString devName);

public slots:
    void slotAlaramActivate(int index);
};

#endif // ALARAM_OUTPUT_H
