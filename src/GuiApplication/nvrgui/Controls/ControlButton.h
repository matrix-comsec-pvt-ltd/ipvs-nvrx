#ifndef CONTROLBUTTON_H
#define CONTROLBUTTON_H

#include "Controls/TextLabel.h"
#include "NavigationControl.h"
#include "Bgtile.h"
#include <QTimer>

#define CONTROL_BUTTON_IMG_PATH             ":/Images_Nvrx/ControlButtons/"

typedef enum
{
    ABORT_BUTTON_INDEX,
    ADD_BUTTON_INDEX,
    ADD_BUTTON_TABLE_INDEX,
    ADDED_BUTTON_TABLE_INDEX,
    BACKUP_BUTTON_INDEX,
    CHECK_BAL_BUTTON_INDEX,
    DELETE_BUTTON_INDEX,
    EDIT_BUTTON_INDEX,
    EMAIL_BUTTON_INDEX,
    EXPAND_BUTTON_INDEX,
    FIRSTPAGE_BUTTON_INDEX,
    FIRSTPAGE_BUTTON_1_INDEX,
    FORMAT_BUTTON_INDEX,
    FTP_BUTTON_INDEX,
    GO_BUTTON_INDEX,
    LAST_BUTTON_INDEX,
    LAST_BUTTON_1_INDEX,
    NEXT_BUTTON_INDEX,
    NEXT_BUTTON_1_INDEX,
    PICKLIST_BUTTON_INDEX,
    PLAY_BUTTON_INDEX,
    PREVIOUS_BUTTON_INDEX,
    PREVIOUS_BUTTON_1_INDEX,
    REGISTER_BUTTON_INDEX,
    REMOVE_BUTTON_TABLE_INDEX,
    SEARCH_BUTTON_INDEX,
    SET_BUTTON_INDEX,
    STOP_BUTTON_INDEX,
    TEST_CAMERAS_BUTTON_INDEX,
    TEST_CONNECTION_BUTTON_INDEX,
    TEST_SMS_BUTTON_INDEX,
    UNPLUG_BUTTON_INDEX,
    UPDATE_BUTTON_INDEX,
    SYNCPLAYBACK_NEXT_BUTTON_INDEX,
    SYNCPLAYBACK_NEXT_BUTTON_1_INDEX,
    SYNCPLAYBACK_PREV_BUTTON_INDEX,
    SYNCPLAYBACK_PREV_BUTTON_1_INDEX,
    CAMERA_LIST_INDEX,
    MAX_FOLDER_INDEX
}CONTROL_BUTTON_TYPE_e;

const QString controlImageFolderPath[] = { "AbortButton/",
                                           "AddButton/",
                                           "AddButton_table/",
                                           "AddedButton_table/",
                                           "BackupButton/",
                                           "CheckBalanceButton/",
                                           "DeleteButton/",
                                           "EditButton/",
                                           "EmailButton/",
                                           "ExpandButton/",
                                           "FirstPageButton/",
                                           "FirstPageButton_1/",
                                           "FormatButton/",
                                           "FtpButton/",
                                           "GoButton/",
                                           "LastPageButton/",
                                           "LastPageButton_1/",
                                           "NextButton/",
                                           "NextButton_1/",
                                           "PicklistButton/",
                                           "PlayButton/",
                                           "PreviousButton/",
                                           "PreviousButton_1/",
                                           "RegisterButton/",
                                           "RemoveButton_table/",
                                           "SearchButton/",
                                           "SetButton/",
                                           "StopButton/",
                                           "TestCameras/",
                                           "TestConnButton/",
                                           "TestSmsButton/",
                                           "UnplugButton/",
                                           "UpdateButton/",
                                           "SyncPlaybackNextButton/",
                                           "SyncPlaybackNextButton_1/",
                                           "SyncPlaybackPrevButton/",
                                           "SyncPlaybackPrevButton_2/",
                                           "CameraList/"
                                         };

class ControlButton: public BgTile, public NavigationControl
{
    Q_OBJECT

private:
    bool m_isLabel;
    QRect m_imageRect;
    TextLabel * m_textLabel;
    int m_controlButtonType, m_horizontalAlignment;
    int m_currentImageType;
    QString m_label, m_imageSource;
    QPixmap m_iconImage;
    int m_pixelAlign;
    QString m_fontColor;
    QTimer* m_clickEffectTimer;
    quint32 m_leftMarginFromCenter;

public:
    ControlButton(CONTROL_BUTTON_TYPE_e type,
                  int startX, int startY,
                  int width, int height,
                  QWidget *parent = 0,
                  BGTILE_TYPE_e tileType = COMMON_LAYER,
                  int pixelAlign = -1,
                  QString label = "",
                  bool isEnabled = true,
                  int indexInPage = 0,
                  bool isLabel = true,
                  quint32 leftMarginFromCenter = 0);
    ~ControlButton();

    void setGeometryForElements();
    void changeImage(IMAGE_TYPE_e type);
    void selectControl();
    void deSelectControl();
    void changeImageType(CONTROL_BUTTON_TYPE_e type);

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals :
    void sigButtonClick(int index);
    void sigUpdateCurrentElement(int index);

public slots:
    void slotClickEffectTimerTimeout();
};

#endif // CHECKBOX_H

