#include "SyncPlaybackCropAndBackup.h"
#include <QKeyEvent>
#include "UsbControl.h"
#include "ValidationMessage.h"

#define CROP_AND_BACKUP_PAGE_WIDTH              SCALE_WIDTH(842)
#define CROP_AND_BACKUP_PAGE_HEIGHT             SCALE_HEIGHT(495)
#define CROP_AND_BACKUP_PAGE_LEFT_MARGIN        SCALE_WIDTH(20)
#define CROP_AND_BACKUP_PAGE_TOP_MARGIN         SCALE_HEIGHT(40)
#define CROP_AND_BACKUP_PAGE_INNER_MARGIN       SCALE_WIDTH(10)
#define CROP_AND_BACKUP_PAGE_TILE_HEIGHT        SCALE_HEIGHT(30)
#define CROP_AND_BACKUP_FILECOPTSTATUS_HEIGHT   SCALE_HEIGHT(28)

#define MAX_PAGE                                8
#define EXPORTING_CLIP_TEXT                     "Exporting Clip for Index"
#define MAX_RECORD_INDEX                        64
#define EXPORT_STRING                           "Export"
#define STOP_STRING                             "Stop"

const quint8 tableCellWidth[SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD] = {72, 100, 180, 180, 100, 150};

const QString tableHeaderString[SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD] = {"Index",
                                                                          "Camera",
                                                                          "Start Date Time",
                                                                          "End Date Time",
                                                                          "Remove",
                                                                          "Backup Status"};
const QString exportStatusString[MAX_EXPORT_STATUS] = {"",
                                                       "",
                                                       "Incomplete",
                                                       "In Progress",
                                                       "Complete"};

SyncPlaybackCropAndBackup::SyncPlaybackCropAndBackup(QWidget *parent,
                                                     QList<CROP_AND_BACKUP_DATA_t> cropAndBackupData,
                                                     quint8 recordValueForSearch,
                                                     QString currentDeviceName,
                                                     quint8  recordDriveIndex)
    : BackGround(((parent->width() - CROP_AND_BACKUP_PAGE_WIDTH) / 2),
          ((parent->height() - CROP_AND_BACKUP_PAGE_HEIGHT) / 2),
          CROP_AND_BACKUP_PAGE_WIDTH,
          CROP_AND_BACKUP_PAGE_HEIGHT,
          BACKGROUND_TYPE_3,
          MAX_TOOLBAR_BUTTON,
          parent,
          true)
{
    this->setEnabled(true);
    this->setMouseTracking(true);
    m_cropAndBackupData = cropAndBackupData;
    m_recordValueForSearch = recordValueForSearch;
    m_currentPageIndex = 0;
    pageIndex = 0;
    m_totalPages = ceil((qreal)m_cropAndBackupData.length() / (qreal)MAX_SYNC_DATA_ON_PAGE);
    m_currentRecordIndexForBackup = m_currentRecordIndexForStopBackup = MAX_RECORD_INDEX;
    m_recordDriveIndex = recordDriveIndex;
    m_currentDeviceName = currentDeviceName;
    m_applController = ApplController::getInstance();
    m_payloadLib = new PayloadLib();

    for(quint8 index = 0; index < SYNC_PLAYBACK_CROPANDBACKUP_MAX_ELEMENT; index++)
    {
        m_elementList[index] = NULL;
    }

    m_elementList[SYNC_PLAYBACK_CROPANDBACKUP_CLOSE_BUTTON] = m_mainCloseButton;
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_mainCloseButton->setIndexInPage(SYNC_PLAYBACK_CROPANDBACKUP_CLOSE_BUTTON);

    m_headerTile = new BgTile(CROP_AND_BACKUP_PAGE_LEFT_MARGIN,
                              CROP_AND_BACKUP_PAGE_TOP_MARGIN,
                              (CROP_AND_BACKUP_PAGE_WIDTH - (2 * CROP_AND_BACKUP_PAGE_LEFT_MARGIN)),
                              SCALE_HEIGHT(30),
                              TOP_LAYER,
                              this);

    for(quint8 index = 0; index < SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD; index++)
    {
        quint16 offSet = (index == 0 ?
                              (CROP_AND_BACKUP_PAGE_LEFT_MARGIN + CROP_AND_BACKUP_PAGE_INNER_MARGIN)
                            : (m_headerTableCell[index - 1]->x() + m_headerTableCell[index - 1]->width() - 1));

        m_headerTableCell[index] = new TableCell(offSet,
                                                 (m_headerTile->y() + CROP_AND_BACKUP_PAGE_INNER_MARGIN),
                                                 SCALE_WIDTH(tableCellWidth[index]),
                                                 CROP_AND_BACKUP_PAGE_TILE_HEIGHT,
                                                 this,
                                                 true);

        quint16 offsetForLabel = (index != 4 ? CROP_AND_BACKUP_PAGE_INNER_MARGIN : (m_headerTableCell[index]->width() / 2));
        TEXTLABEL_ALIGNMENT_e alignment = (index != 4 ? ALIGN_START_X_CENTRE_Y : ALIGN_CENTRE_X_CENTER_Y);
        m_tableCellHeadingText[index] = new TextLabel((m_headerTableCell[index]->x() + offsetForLabel),
                                                      (m_headerTableCell[index]->y() + (m_headerTableCell[index]->height() / 2)),
                                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                                      tableHeaderString[index],
                                                      this,
                                                      HIGHLITED_FONT_COLOR,
                                                      NORMAL_FONT_FAMILY,
                                                      alignment);
    }

    for(quint8 rowIndex = 0; rowIndex < MAX_SYNC_DATA_ON_PAGE; rowIndex++)
    {
        m_dataTiles[rowIndex] = new BgTile(CROP_AND_BACKUP_PAGE_LEFT_MARGIN,
                                           (m_headerTile->y() + m_headerTile->height() + (rowIndex * CROP_AND_BACKUP_PAGE_TILE_HEIGHT)),
                                           m_headerTile->width(),
                                           CROP_AND_BACKUP_PAGE_TILE_HEIGHT,
                                           MIDDLE_LAYER,
                                           this);

        for(quint8 colIndex = 0; colIndex < SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD; colIndex++)
        {
            quint16 offSet = (colIndex == 0 ?
                                  (CROP_AND_BACKUP_PAGE_LEFT_MARGIN + CROP_AND_BACKUP_PAGE_INNER_MARGIN)
                                : (m_dataTableCell[rowIndex][colIndex - 1]->x() + m_dataTableCell[rowIndex][colIndex - 1]->width() - 1));

            m_dataTableCell[rowIndex][colIndex] = new TableCell(offSet,
                                                                m_dataTiles[rowIndex]->y(),
                                                                SCALE_WIDTH(tableCellWidth[colIndex]),
                                                                CROP_AND_BACKUP_PAGE_TILE_HEIGHT,
                                                                this);

            m_dataTextLabel[rowIndex][colIndex] = new TextLabel((m_dataTableCell[rowIndex][colIndex]->x() + 10),
                                                                (m_dataTableCell[rowIndex][colIndex]->y() + (m_dataTableCell[rowIndex][colIndex]->height() / 2)),
                                                                NORMAL_FONT_SIZE,
                                                                "",
                                                                this,
                                                                NORMAL_FONT_COLOR,
                                                                NORMAL_FONT_FAMILY,
                                                                ALIGN_START_X_CENTRE_Y);
        }

        m_removeButtons[rowIndex] = new ControlButton(REMOVE_BUTTON_TABLE_INDEX,
                                                      (m_dataTableCell[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_REMOVE]->x() + SCALE_WIDTH(39)),
                                                      m_dataTableCell[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_REMOVE]->y(),
                                                      SCALE_WIDTH(30),
                                                      SCALE_HEIGHT(30),
                                                      this,
                                                      NO_LAYER,
                                                      0,
                                                      "",
                                                      false,
                                                      (SYNC_PLAYBACK_CROPANDBACKUP_REMOVE_BUTTON + rowIndex));
        m_removeButtons[rowIndex]->setVisible(false);

        m_elementList[SYNC_PLAYBACK_CROPANDBACKUP_REMOVE_BUTTON + rowIndex] = m_removeButtons[rowIndex];
        connect(m_removeButtons[rowIndex],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_removeButtons[rowIndex],
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotRemoveButtonClicked(int)));
    }

    m_bottomTile = new BgTile(CROP_AND_BACKUP_PAGE_LEFT_MARGIN,
                              (m_dataTiles[MAX_SYNC_DATA_ON_PAGE - 1]->y() + CROP_AND_BACKUP_PAGE_TILE_HEIGHT),
                              (CROP_AND_BACKUP_PAGE_WIDTH - (2 * CROP_AND_BACKUP_PAGE_LEFT_MARGIN)),
                              CROP_AND_BACKUP_PAGE_TILE_HEIGHT,
                              BOTTOM_LAYER,
                              this);

    m_previousPage = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                       (CROP_AND_BACKUP_PAGE_LEFT_MARGIN + CROP_AND_BACKUP_PAGE_INNER_MARGIN),
                                       m_bottomTile->y(),
                                       SCALE_WIDTH(40),
                                       SCALE_HEIGHT(40),
                                       this,
                                       NO_LAYER,
                                       0,
                                       "Previous",
                                       false,
                                       SYNC_PLAYBACK_CROPANDBACKUP_PREVIOUS_PAGE_BUTTON,
                                       false);
    m_elementList[SYNC_PLAYBACK_CROPANDBACKUP_PREVIOUS_PAGE_BUTTON] = m_previousPage;
    connect(m_previousPage,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_previousPage,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNavigationButtonClicked(int)));

    m_nextPage = new ControlButton(NEXT_BUTTON_INDEX,
                                   (CROP_AND_BACKUP_PAGE_WIDTH - (CROP_AND_BACKUP_PAGE_INNER_MARGIN + CROP_AND_BACKUP_PAGE_LEFT_MARGIN + SCALE_WIDTH(76))),
                                   m_bottomTile->y(),
                                   SCALE_WIDTH(40),
                                   SCALE_HEIGHT(40),
                                   this,
                                   NO_LAYER,
                                   0,
                                   "Next",
                                   false,
                                   SYNC_PLAYBACK_CROPANDBACKUP_NEXT_PAGE_BUTTON);
    m_elementList[SYNC_PLAYBACK_CROPANDBACKUP_NEXT_PAGE_BUTTON] = m_nextPage;
    connect(m_nextPage,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_nextPage,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNavigationButtonClicked(int)));

    m_totalClipTextLable = new TextLabel((CROP_AND_BACKUP_PAGE_WIDTH - SCALE_HEIGHT(25)),
                                         (m_bottomTile->y() + m_bottomTile->height() + SCALE_HEIGHT(25)),
                                         NORMAL_FONT_SIZE,
                                         (Multilang("Total Clips :") + QString(" %1").arg(m_cropAndBackupData.length())),
                                         this,
                                         HIGHLITED_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_END_X_CENTRE_Y, 0, false, (CROP_AND_BACKUP_PAGE_WIDTH/2));
    m_totalClipTextLable->SetBold(true);

    m_expotingTextLable = new TextLabel(CROP_AND_BACKUP_PAGE_LEFT_MARGIN,
                                        (m_bottomTile->y() + m_bottomTile->height() + SCALE_HEIGHT(25)),
                                        NORMAL_FONT_SIZE,
                                        EXPORTING_CLIP_TEXT,
                                        this,
                                        HIGHLITED_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y);
    m_expotingTextLable->setVisible(false);

    m_fileCopyStatus = new SyncPlaybackFileCopyStatus(CROP_AND_BACKUP_PAGE_LEFT_MARGIN,
                                                      (m_totalClipTextLable->y() + m_totalClipTextLable->height() + SCALE_HEIGHT(10)),
                                                      (CROP_AND_BACKUP_PAGE_WIDTH - (2 * CROP_AND_BACKUP_PAGE_LEFT_MARGIN)),
                                                      CROP_AND_BACKUP_FILECOPTSTATUS_HEIGHT,
                                                      this);
    m_fileCopyStatus->setVisible(false);

    m_exportButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                    (CROP_AND_BACKUP_PAGE_WIDTH / 2),
                                    (CROP_AND_BACKUP_PAGE_HEIGHT - SCALE_HEIGHT(32)),
                                    EXPORT_STRING,
                                    this,
                                    SYNC_PLAYBACK_CROPANDBACKUP_EXPORT_BUTTON,
                                    true);
    m_elementList[SYNC_PLAYBACK_CROPANDBACKUP_EXPORT_BUTTON] = m_exportButton;
    connect(m_exportButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_exportButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCnfgButtonClicked(int)));

    m_infoPage = new InfoPage(0, 0,
                              this->width(),
                              this->height(),
                              MAX_INFO_PAGE_TYPE,
                              this,
                              false,
                              false);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageButtonClicked(int)));

    m_processbar = new ProcessBar(0, 0, this->width(), this->height(), 0, this);


    m_currentElement = SYNC_PLAYBACK_CROPANDBACKUP_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();

    fillRecords(0);
    updateNavigationControlStatus();
}

SyncPlaybackCropAndBackup::~SyncPlaybackCropAndBackup()
{
    delete m_processbar;

    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageButtonClicked(int)));
    delete m_infoPage;

    disconnect(m_exportButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_exportButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCnfgButtonClicked(int)));
    delete m_exportButton;

    delete m_fileCopyStatus;

    delete m_expotingTextLable;
    delete m_totalClipTextLable;

    disconnect(m_nextPage,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_nextPage,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNavigationButtonClicked(int)));
    delete m_nextPage;

    disconnect(m_previousPage,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_previousPage,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNavigationButtonClicked(int)));
    delete m_previousPage;

    delete m_bottomTile;

    for(quint8 rowIndex = 0; rowIndex < MAX_SYNC_DATA_ON_PAGE; rowIndex++)
    {
        disconnect(m_removeButtons[rowIndex],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_removeButtons[rowIndex],
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotRemoveButtonClicked(int)));
        delete m_removeButtons[rowIndex];

        for(int colIndex = 0; colIndex < SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD; colIndex++)
        {
            delete m_dataTextLabel[rowIndex][colIndex];
            delete m_dataTableCell[rowIndex][colIndex];
        }

        delete m_dataTiles[rowIndex];
    }

    for(quint8 index = 0; index < SYNC_PLAYBACK_CROPANDBACKUP_MAX_FIELD; index++)
    {
        delete m_tableCellHeadingText[index];
        delete m_headerTableCell[index];
    }

    delete m_headerTile;

    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_payloadLib;
    m_cropAndBackupData.clear();
}

void SyncPlaybackCropAndBackup::fillRecords(quint8 indexInPage)
{
    clearRecords();

    m_currentPageIndex = indexInPage;
    quint8 startingIndex = m_currentPageIndex * MAX_SYNC_DATA_ON_PAGE;
    quint8 endingIndex = (m_cropAndBackupData.length() < (startingIndex + MAX_SYNC_DATA_ON_PAGE)
                          ? m_cropAndBackupData.length()
                          : (startingIndex + MAX_SYNC_DATA_ON_PAGE));
    for(quint8 index = startingIndex; index < endingIndex; index++)
    {
        quint8 rowIndex = index % MAX_SYNC_DATA_ON_PAGE;
        m_dataTextLabel[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_INDEX]->changeText(QString("%1").arg(index + 1));
        m_dataTextLabel[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_CAMERA_INDEX]->changeText(QString("%1").arg(m_cropAndBackupData.at(index).cameraIndex));
        m_dataTextLabel[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_START_TIME]->changeText(m_cropAndBackupData.at(index).startTime.toString("dd-MM-yyyy hh:mm:ss"));
        m_dataTextLabel[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_END_TIME]->changeText(m_cropAndBackupData.at(index).endTime.toString("dd-MM-yyyy hh:mm:ss"));
        m_dataTextLabel[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_BACKUP_STATUS]->changeText(exportStatusString[m_cropAndBackupData.at(index).exportStatus]);

        switch(m_cropAndBackupData.at(index).exportStatus)
        {
        case EXPORT_STATUS_NONE:
        case EXPORT_STATUS_NOT_EXPORTED:
        case EXPORT_STATUS_INPROGRESS:
            m_removeButtons[rowIndex]->setIsEnabled(true);
            break;

        case EXPORT_STATUS_EXPORTED:
        case EXPORT_STATUS_INCOMPLETE:
            m_removeButtons[rowIndex]->setIsEnabled(false);
            break;

        default:
            break;
        }

        m_removeButtons[rowIndex]->setVisible(true);
    }
}

void SyncPlaybackCropAndBackup::clearRecords()
{
    for(quint8 index = 0; index < MAX_SYNC_DATA_ON_PAGE; index++)
    {
        m_dataTextLabel[index][SYNC_PLAYBACK_CROPANDBACKUP_INDEX]->changeText("");
        m_dataTextLabel[index][SYNC_PLAYBACK_CROPANDBACKUP_CAMERA_INDEX]->changeText("");
        m_dataTextLabel[index][SYNC_PLAYBACK_CROPANDBACKUP_START_TIME]->changeText("");
        m_dataTextLabel[index][SYNC_PLAYBACK_CROPANDBACKUP_END_TIME]->changeText("");
        m_dataTextLabel[index][SYNC_PLAYBACK_CROPANDBACKUP_BACKUP_STATUS]->changeText("");

        m_removeButtons[index]->setIsEnabled(false);
        m_removeButtons[index]->setVisible(false);
    }
}

void SyncPlaybackCropAndBackup::updateNavigationControlStatus()
{
    bool isPreviousButtonDisable = true;
    bool isNextButtonDisable = true;
    quint8 newCurrentElement = m_currentElement;
    if(m_totalPages > 1)
    {
        isPreviousButtonDisable = (m_currentPageIndex == 0);
        isNextButtonDisable = (m_currentPageIndex == (MAX_PAGE - 1));
    }

    if((m_nextPage->hasFocus())
            && (isNextButtonDisable))
    {
        newCurrentElement = SYNC_PLAYBACK_CROPANDBACKUP_PREVIOUS_PAGE_BUTTON;
    }
    else if((m_previousPage->hasFocus())
            && (isPreviousButtonDisable))
    {
        newCurrentElement = SYNC_PLAYBACK_CROPANDBACKUP_NEXT_PAGE_BUTTON;
    }

    m_previousPage->setIsEnabled(!isPreviousButtonDisable);
    m_nextPage->setIsEnabled(!isNextButtonDisable);
    if(newCurrentElement != m_currentElement)
    {
        m_currentElement = newCurrentElement;
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void SyncPlaybackCropAndBackup::startBackupProcess()
{
    m_currentRecordIndexForBackup = findRecordIndex();
    if(m_currentRecordIndexForBackup != MAX_RECORD_INDEX)
    {
        QString payloadString;
        m_payloadLib->setCnfgArrayAtIndex(0, m_recordValueForSearch);
        m_payloadLib->setCnfgArrayAtIndex(1, m_cropAndBackupData.at(m_currentRecordIndexForBackup).cameraIndex);
        m_payloadLib->setCnfgArrayAtIndex(2, m_cropAndBackupData.at(m_currentRecordIndexForBackup).startTime.toString("ddMMyyyyhhmmss"));
        m_payloadLib->setCnfgArrayAtIndex(3, m_cropAndBackupData.at(m_currentRecordIndexForBackup).endTime.toString("ddMMyyyyhhmmss"));
        m_payloadLib->setCnfgArrayAtIndex(4, m_recordDriveIndex);

        payloadString = m_payloadLib->createDevCmdPayload(5);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = SYNC_CLP_RCD;
        param->payload = payloadString;
        if(m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param))
        {
            m_processbar->loadProcessBar();
        }
    }
    else
    {
        m_expotingTextLable->setVisible(false);
        m_fileCopyStatus->setVisible(false);
        m_fileCopyStatus->changePercentStatus(0);
        m_exportButton->changeText(EXPORT_STRING);
    }
}

void SyncPlaybackCropAndBackup::stopBackupProcess()
{
    m_currentRecordIndexForStopBackup = m_currentRecordIndexForBackup;
    if(m_currentRecordIndexForStopBackup != MAX_RECORD_INDEX)
    {
        QString payloadString;
        m_payloadLib->setCnfgArrayAtIndex(0, 0); //0 for Manual Backup
        payloadString = m_payloadLib->createDevCmdPayload(1);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = STP_BCKUP;
        param->payload = payloadString;
        if(m_applController->processActivity(m_currentDeviceName, DEVICE_COMM, param))
        {
            m_processbar->loadProcessBar();
        }
    }
}

quint8 SyncPlaybackCropAndBackup::findRecordIndex()
{
    quint8 recordIndex = MAX_RECORD_INDEX;
    for(quint8 index = 0; index < m_cropAndBackupData.length(); index++)
    {
        if(m_cropAndBackupData.at(index).exportStatus == EXPORT_STATUS_NOT_EXPORTED)
        {
            recordIndex = index;
            break;
        }
    }
    return recordIndex;
}

void SyncPlaybackCropAndBackup::updateBackupStatusOfRecord(quint8 recordIndex,
                                                           SYNC_PLAYBACK_EXPORT_STATUS_e exportStatus)
{
    CROP_AND_BACKUP_DATA_t data = m_cropAndBackupData.at(recordIndex);
    data.exportStatus = exportStatus;
    m_cropAndBackupData.replace(m_currentRecordIndexForBackup, data);

    quint8 m_pageIndex = recordIndex / MAX_SYNC_DATA_ON_PAGE;
    if(m_pageIndex == m_currentPageIndex)
    {
        quint8 rowIndex = recordIndex % MAX_SYNC_DATA_ON_PAGE;

        m_dataTextLabel[rowIndex][SYNC_PLAYBACK_CROPANDBACKUP_BACKUP_STATUS]->changeText(exportStatusString[exportStatus]);

        switch(exportStatus)
        {
        case EXPORT_STATUS_NONE:
        case EXPORT_STATUS_NOT_EXPORTED:
        case EXPORT_STATUS_INPROGRESS:
            m_removeButtons[rowIndex]->setIsEnabled(true);
            break;

        case EXPORT_STATUS_EXPORTED:
        case EXPORT_STATUS_INCOMPLETE:
            m_removeButtons[rowIndex]->setIsEnabled(false);
            break;

        default:
            break;
        }
    }
}

void SyncPlaybackCropAndBackup::updateManualBackupStatusEventAction(QString deviceName,
                                                                    quint8 copiedPercentage)
{
    if((deviceName != m_currentDeviceName) || (m_currentRecordIndexForBackup == MAX_RECORD_INDEX))
    {
        return;
    }

    m_fileCopyStatus->changePercentStatus(copiedPercentage);
}

void SyncPlaybackCropAndBackup::updateManualBackupSystemEventAction(QString deviceName,
                                                                    LOG_EVENT_STATE_e eventState)
{
    m_processbar->unloadProcessBar();
    if((deviceName != m_currentDeviceName) || (m_currentRecordIndexForBackup == MAX_RECORD_INDEX))
    {
        return;
    }

    switch(eventState)
    {
    case EVENT_START:
        m_expotingTextLable->changeText(EXPORTING_CLIP_TEXT + QString(" ") +QString("%1").arg(m_currentRecordIndexForBackup + 1));
        m_expotingTextLable->update();
        m_expotingTextLable->setVisible(true);
        m_fileCopyStatus->setVisible(true);
        m_fileCopyStatus->changePercentStatus(0);
        updateBackupStatusOfRecord(m_currentRecordIndexForBackup,
                                   EXPORT_STATUS_INPROGRESS);
        m_exportButton->changeText(STOP_STRING);
        break;

    case EVENT_COMPLETE:
        updateBackupStatusOfRecord(m_currentRecordIndexForBackup,
                                   EXPORT_STATUS_EXPORTED);
        startBackupProcess();
        break;

    case EVENT_INCOMPLETE:
        m_expotingTextLable->setVisible(false);
        m_fileCopyStatus->setVisible(false);
        m_fileCopyStatus->changePercentStatus(0);
        m_exportButton->changeText(EXPORT_STRING);
        if((m_currentRecordIndexForBackup == m_currentRecordIndexForStopBackup)
                && (m_currentRecordIndexForStopBackup != MAX_RECORD_INDEX))
        {
            updateBackupStatusOfRecord(m_currentRecordIndexForBackup,
                                       EXPORT_STATUS_NOT_EXPORTED);
            m_currentRecordIndexForStopBackup = m_currentRecordIndexForBackup = MAX_RECORD_INDEX;
        }
        else
        {
            updateBackupStatusOfRecord(m_currentRecordIndexForBackup,
                                       EXPORT_STATUS_INCOMPLETE);
            m_currentRecordIndexForStopBackup = m_currentRecordIndexForBackup = MAX_RECORD_INDEX;
            startBackupProcess();
        }
        break;

    case EVENT_STOP:
        EPRINT(GUI_SYNC_PB_MEDIA, "event stopped recevied: stop backup");
        m_expotingTextLable->setVisible(false);
        m_fileCopyStatus->setVisible(false);
        m_fileCopyStatus->changePercentStatus(0);
        m_exportButton->changeText(EXPORT_STRING);
        updateBackupStatusOfRecord(m_currentRecordIndexForBackup, EXPORT_STATUS_INCOMPLETE);
        m_currentRecordIndexForStopBackup = m_currentRecordIndexForBackup = MAX_RECORD_INDEX;
        break;

    default:
        break;
    }
}

QList<CROP_AND_BACKUP_DATA_t> SyncPlaybackCropAndBackup::getCropAndBackupList()
{
    return m_cropAndBackupData;
}

void SyncPlaybackCropAndBackup::changeBackupExpotStateToStop()
{
    m_processbar->unloadProcessBar();
    m_expotingTextLable->setVisible(false);
    m_fileCopyStatus->setVisible(false);
    m_fileCopyStatus->changePercentStatus(0);
    m_exportButton->changeText(EXPORT_STRING);
    updateBackupStatusOfRecord(m_currentRecordIndexForBackup,
                               EXPORT_STATUS_INCOMPLETE);
    m_currentRecordIndexForBackup = m_currentRecordIndexForStopBackup = MAX_RECORD_INDEX;
}

void SyncPlaybackCropAndBackup::takeLeftKeyAction()
{
    do
    {
        m_currentElement = ((m_currentElement - 1 + SYNC_PLAYBACK_CROPANDBACKUP_MAX_ELEMENT)
                            % SYNC_PLAYBACK_CROPANDBACKUP_MAX_ELEMENT);
    }while((!m_elementList[m_currentElement]->getIsEnabled())
           || (m_elementList[m_currentElement] == NULL));

    m_elementList[m_currentElement]->forceActiveFocus();
}

void SyncPlaybackCropAndBackup::takeRightKeyAction()
{
    do
    {
        m_currentElement = ((m_currentElement + 1) % SYNC_PLAYBACK_CROPANDBACKUP_MAX_ELEMENT);
    }while((!m_elementList[m_currentElement]->getIsEnabled())
           || (m_elementList[m_currentElement] == NULL));

    m_elementList[m_currentElement]->forceActiveFocus();
}

void SyncPlaybackCropAndBackup::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void SyncPlaybackCropAndBackup::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = SYNC_PLAYBACK_CROPANDBACKUP_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void SyncPlaybackCropAndBackup::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void SyncPlaybackCropAndBackup::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void SyncPlaybackCropAndBackup::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void SyncPlaybackCropAndBackup::slotCnfgButtonClicked(int)
{
    if(m_exportButton->getText() == EXPORT_STRING)
    {
        //        if((m_currentDeviceName != LOCAL_DEVICE_NAME)
        //                || (UsbControl::showUsbFlag[USB_TYPE_MANUAL] == true))
        {
            startBackupProcess();
        }
        //        else
        //        {
        //            m_infoPage->loadInfoPage("Manual Backup Device Not Found!");
        //        }
    }
    else
    {
        stopBackupProcess();
    }
}

void SyncPlaybackCropAndBackup::slotInfoPageButtonClicked(int)
{
    m_elementList[m_currentElement]->forceActiveFocus();
}

void SyncPlaybackCropAndBackup::slotRemoveButtonClicked(int index)
{
    if(m_exportButton->getText() == EXPORT_STRING)
    {
        quint8 recordIndex = (index - SYNC_PLAYBACK_CROPANDBACKUP_REMOVE_BUTTON) + (m_currentPageIndex * MAX_SYNC_DATA_ON_PAGE);
        m_cropAndBackupData.removeAt(recordIndex);

        m_totalClipTextLable->changeText(Multilang("Total Clips :") + QString(" %1").arg(m_cropAndBackupData.length()));

        m_totalClipTextLable->update();

        m_totalPages = ceil((qreal)m_cropAndBackupData.length() / (qreal)MAX_SYNC_DATA_ON_PAGE);
        if(m_totalPages < MAX_PAGE)
        {
            fillRecords(0);
        }
        else
        {
            fillRecords(m_currentPageIndex);
        }
        updateNavigationControlStatus();

        quint8 recordsOnPage;
        if(m_currentPageIndex == 0)
        {
            recordsOnPage = ((m_cropAndBackupData.length() < MAX_SYNC_DATA_ON_PAGE) ? m_cropAndBackupData.length() : MAX_SYNC_DATA_ON_PAGE);
        }
        else
        {
            recordsOnPage = m_cropAndBackupData.length() - MAX_SYNC_DATA_ON_PAGE;
        }
        if((index - SYNC_PLAYBACK_CROPANDBACKUP_REMOVE_BUTTON) >= recordsOnPage)
        {
            m_currentElement = (SYNC_PLAYBACK_CROPANDBACKUP_REMOVE_BUTTON + recordsOnPage - 1);
        }
        else
        {
            m_currentElement = index;
        }

        if(m_cropAndBackupData.length() == 0)
        {
            m_exportButton->setIsEnabled(false);
            m_currentElement = SYNC_PLAYBACK_CROPANDBACKUP_CLOSE_BUTTON;
        }
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    else
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SYNC_PB_CROP_AND_BACKUP));
    }
}

void SyncPlaybackCropAndBackup::slotNavigationButtonClicked(int index)
{
    switch(index)
    {
    case SYNC_PLAYBACK_CROPANDBACKUP_PREVIOUS_PAGE_BUTTON:
        if(pageIndex > 0)
        {
            pageIndex--;
        }
        fillRecords(pageIndex);
        break;

    case SYNC_PLAYBACK_CROPANDBACKUP_NEXT_PAGE_BUTTON:
        if(pageIndex < MAX_PAGE)
        {
            pageIndex++;
            fillRecords(pageIndex);
        }
        break;
    }

    updateNavigationControlStatus();
}

void SyncPlaybackCropAndBackup::slotCloseButtonClicked(int)
{
    if(m_exportButton->getText() == EXPORT_STRING)
    {
        for(qint8 index = 0; index < m_cropAndBackupData.length(); index++)
        {
            if((m_cropAndBackupData.at(index).exportStatus == EXPORT_STATUS_INCOMPLETE)
                    || (m_cropAndBackupData.at(index).exportStatus == EXPORT_STATUS_EXPORTED))
            {
                m_cropAndBackupData.removeAt(index--);
            }
        }
        emit sigClosePage(m_toolbarPageIndex);
    }
    else
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SYNC_PB_CROP_AND_BACKUP));
    }
}

void SyncPlaybackCropAndBackup::closeSyncBackupManual()
{
    for(qint8 index = 0; index < m_cropAndBackupData.length(); index++)
    {
        if((m_cropAndBackupData.at(index).exportStatus == EXPORT_STATUS_INCOMPLETE)
                || (m_cropAndBackupData.at(index).exportStatus == EXPORT_STATUS_EXPORTED))
        {
            m_cropAndBackupData.removeAt(index--);
        }
    }
    emit sigClosePage(m_toolbarPageIndex);
}
