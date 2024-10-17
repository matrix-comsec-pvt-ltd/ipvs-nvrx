#ifndef SYNCPLAYBACKCROPANDBACKUP_H
#define SYNCPLAYBACKCROPANDBACKUP_H

#include "Controls/BackGround.h"
#include "DataStructure.h"
#include "Controls/Bgtile.h"
#include "Controls/TableCell.h"
#include "Controls/CnfgButton.h"
#include "Controls/ControlButton.h"
#include "Controls/TextLabel.h"
#include "Controls/SyncPlayback/SyncPlaybackFileCopyStatus.h"
#include "Controls/InfoPage.h"
#include "PayloadLib.h"
#include "ApplController.h"
#include "Controls/ProcessBar.h"

#define MAX_SYNC_DATA_ON_PAGE        8

typedef enum
{
    SYNC_PLAYBACK_CROPANDBACKUP_CLOSE_BUTTON,
    SYNC_PLAYBACK_CROPANDBACKUP_REMOVE_BUTTON,
    SYNC_PLAYBACK_CROPANDBACKUP_PREVIOUS_PAGE_BUTTON = 9, //8+1
    SYNC_PLAYBACK_CROPANDBACKUP_NEXT_PAGE_BUTTON,
    SYNC_PLAYBACK_CROPANDBACKUP_EXPORT_BUTTON,
    SYNC_PLAYBACK_CROPANDBACKUP_MAX_ELEMENT
}SYNC_PLAYBACK_CROPANDBACKUP_ELEMENT_e;

typedef enum
{
    SYNC_PLAYBACK_CROPANDBACKUP_INDEX,
    SYNC_PLAYBACK_CROPANDBACKUP_CAMERA_INDEX,
    SYNC_PLAYBACK_CROPANDBACKUP_START_TIME,
    SYNC_PLAYBACK_CROPANDBACKUP_END_TIME,
    SYNC_PLAYBACK_CROPANDBACKUP_REMOVE,
    SYNC_PLAYBACK_CROPANDBACKUP_BACKUP_STATUS,
    SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD
}SYNC_PLAYBACK_CROPANDBACKUP_FIELD_e;

class SyncPlaybackCropAndBackup : public BackGround
{
    Q_OBJECT
private:
    QList<CROP_AND_BACKUP_DATA_t> m_cropAndBackupData;
    BgTile* m_headerTile;
    BgTile* m_bottomTile;
    BgTile* m_dataTiles[MAX_SYNC_DATA_ON_PAGE];
    TableCell* m_headerTableCell[SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD];
    TextLabel* m_tableCellHeadingText[SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD];
    TextLabel* m_dataTextLabel[MAX_SYNC_DATA_ON_PAGE][SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD];
    TableCell* m_dataTableCell[MAX_SYNC_DATA_ON_PAGE][SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD];
    CnfgButton* m_exportButton;
    ControlButton* m_nextPage;
    ControlButton* m_previousPage;
    ControlButton* m_removeButtons[MAX_SYNC_DATA_ON_PAGE];
    TextLabel* m_totalClipTextLable;
    TextLabel* m_expotingTextLable;
    SyncPlaybackFileCopyStatus* m_fileCopyStatus;
    InfoPage* m_infoPage;
    ProcessBar* m_processbar;
    quint8 pageIndex;

    ApplController* m_applController;
    PayloadLib* m_payloadLib;

    quint8 m_currentPageIndex, m_totalPages, m_recordValueForSearch;
    quint8 m_currentRecordIndexForBackup, m_currentRecordIndexForStopBackup;
    quint8 m_recordDriveIndex;
    QString m_currentDeviceName;

    int m_currentElement;
    NavigationControl* m_elementList[SYNC_PLAYBACK_CROPANDBACKUP_MAX_ELEMENT];

public:
    SyncPlaybackCropAndBackup(QWidget *parent,
                              QList<CROP_AND_BACKUP_DATA_t> cropAndBackupData,
                              quint8 recordValueForSearch,
                              QString currentDeviceName,
                              quint8  recordDriveIndex = 3);
    ~SyncPlaybackCropAndBackup();

    void fillRecords(quint8 indexInPage);
    void clearRecords();
    void updateNavigationControlStatus();
    void startBackupProcess();
    void stopBackupProcess();
    quint8 findRecordIndex();
    void updateBackupStatusOfRecord(quint8 recordIndex, SYNC_PLAYBACK_EXPORT_STATUS_e exportStatus);
    void updateManualBackupStatusEventAction(QString deviceName, quint8 copiedPercentage);
    void updateManualBackupSystemEventAction(QString deviceName, LOG_EVENT_STATE_e eventState);
    QList<CROP_AND_BACKUP_DATA_t> getCropAndBackupList();
    void changeBackupExpotStateToStop();

    void closeSyncBackupManual();

    void takeLeftKeyAction();
    void takeRightKeyAction();

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

public slots:
    void slotUpdateCurrentElement(int index);
    void slotCnfgButtonClicked(int index);
    void slotInfoPageButtonClicked(int index);
    void slotRemoveButtonClicked(int index);
    void slotNavigationButtonClicked(int index);
    void slotCloseButtonClicked(int indexInPage);
};

#endif // SYNCPLAYBACKCROPANDBACKUP_H
