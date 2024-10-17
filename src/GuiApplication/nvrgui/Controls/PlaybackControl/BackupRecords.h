#ifndef BACKUPRECORDS_H
#define BACKUPRECORDS_H

#include <QWidget>
#include "NavigationControl.h"
#include "Controls/PlaybackRecordData.h"
#include "Controls/CnfgButton.h"
#include "Controls/Rectangle.h"
#include "Controls/TextLabel.h"
#include "Controls/Closebuttton.h"
#include "Controls/TableCell.h"
#include "Controls/ControlButton.h"
#include "Controls/ReadOnlyElement.h"

#define MAX_BACKUP_HEADING_FIELDS 5
#define MAX_BACKUP_LIST_ROW       10
#define MAX_BACKUP_LIST           10

// List of control
typedef enum
{
    BKUP_REC_CLOSE_BTN,
    BKUP_REC_START_BTN,
    MAX_BKUP_REC_ELEMETS
}BKUP_REC_ELE_e;


class BackupRecords : public KeyBoard
{
    Q_OBJECT

public:
    BackupRecords(QStringList &startDateTimeList,
                  QStringList &endDateTimeList,
                  QStringList &cameraList,
                  QWidget *parent = 0);
    ~BackupRecords();

    void setPercentage(quint8 percent);
    void updateBackUpLocation(QString str);

    void paintEvent (QPaintEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

signals:
    void sigBackUpRecsBtnClicked(int index);

public slots:
    void slotButtonClick(int index);
    void slotUpadateCurrentElement(int index);


private:
    Rectangle*      m_backGroundRect;
    CloseButtton*   m_closeBtn;
    TextLabel*      m_heading;
    TextLabel*      m_infoString;

    ReadOnlyElement* m_BackupLocation;

    CnfgButton*     m_startBtn;

    TableCell*      m_headingTextCell[MAX_BACKUP_HEADING_FIELDS];
    TextLabel*      m_headingTextString[MAX_BACKUP_HEADING_FIELDS];

    TableCell*      m_srNumTextCell[MAX_BACKUP_LIST_ROW];
    TextLabel*      m_srNumTextString[MAX_BACKUP_LIST_ROW];

    TableCell*      m_startTimeTextCell[MAX_BACKUP_LIST_ROW];
    TextLabel*      m_startTimeTextString[MAX_BACKUP_LIST_ROW];

    TableCell*      m_endTimeTextCell[MAX_BACKUP_LIST_ROW];
    TextLabel*      m_endTimeTextString[MAX_BACKUP_LIST_ROW];

    TableCell*      m_cameraNumTextCell[MAX_BACKUP_LIST_ROW];
    TextLabel*      m_cameraNumTextString[MAX_BACKUP_LIST_ROW];

    TableCell*      m_statusTextCell[MAX_BACKUP_LIST_ROW];
    TextLabel*      m_statusTextString[MAX_BACKUP_LIST_ROW];

    NavigationControl *m_elementList[MAX_BKUP_REC_ELEMETS];
    quint32          m_currElement;

    TextLabel*      m_backupStatus;
    TextLabel*      m_percentTextlabel;
    Rectangle*      m_statusRect;
    Rectangle*      m_barBackGround;

    QStringList    m_startDateTimeList;
    QStringList    m_endDateTimeList;
    QStringList    m_cameraList;

    quint8          m_maxPages;
    quint8          m_currentPage;
    quint8          m_currntIndex;

    void createDefaultComponent ();
    void fillRecords();
    void takeLeftKeyAction();
    void takeRightKeyAction();

};

#endif // BACKUPRECORDS_H
