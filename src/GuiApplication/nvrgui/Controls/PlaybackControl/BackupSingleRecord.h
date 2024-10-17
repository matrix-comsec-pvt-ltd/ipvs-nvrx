#ifndef BACKUPSINGLERECORD_H
#define BACKUPSINGLERECORD_H

#include "Controls/PlaybackRecordData.h"
#include "NavigationControl.h"
#include "Controls/CnfgButton.h"
#include "Controls/Rectangle.h"
#include "Controls/TextLabel.h"
#include "Controls/CalendarTile.h"
#include "Controls/Clockspinbox.h"
#include "Controls/Closebuttton.h"
#include "Controls/ReadOnlyElement.h"

// List of control
typedef enum
{
    BKUP_SINGLE_REC_CLOSE_BTN,
    BKUP_SINGLE_REC_START_TIME,
    BKUP_SINGLE_REC_END_TIME,
    BKUP_SINGLE_REC_START_BTN,

    MAX_BKUP_SINGLE_REC_ELEMETS
}BKUP_SINGLE_REC_ELE_e;

class BackupSingleRecord : public KeyBoard
{
    Q_OBJECT

public:
    explicit BackupSingleRecord(PlaybackRecordData *recData,
                       QWidget *parent = 0);
    ~BackupSingleRecord();

    void createDefaultComponent ();
    void setPercentage(quint8 percent);
    void updateBackUpLocation(QString str);
    void getDateTime(QString &start,QString &end);
    void fillDateTime();
    void takeLeftKeyAction();
    void takeRightKeyAction();

    void paintEvent (QPaintEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

signals:
    void sigBackUpSingleRecBtnClicked(int index);

public slots:
    void slotClockSpinboxTotalCurrentSec(quint16 currSec,int index);
    void slotButtonClick(int index);
    void slotUpadateCurrentElement(int index);


private:
    quint32         m_currElement;
    QDate           startDate, endDate;

    Rectangle*      m_backGroundRect;
    CloseButtton*   m_closeBtn;
    TextLabel*      m_heading;
    TextLabel*      m_infoString;
    CalendarTile    *m_startDate, *m_endDate;

    ClockSpinbox    *m_startTime, *m_endTime;

    CnfgButton      *m_startBtn;
    PlaybackRecordData *m_recData;

    ReadOnlyElement* m_BackupLocation;
    NavigationControl *m_elementList[MAX_BKUP_SINGLE_REC_ELEMETS];

    TextLabel*      m_backupStatus;
    TextLabel*      m_percentTextlabel;
    Rectangle*      m_statusRect;
    Rectangle*      m_barBackGround;
};

#endif // BACKUPSINGLERECORD_H
